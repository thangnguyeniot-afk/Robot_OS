/*
 * devkit_runtime.c
 * Boot sequence, periodic tick log, and status LED orchestration.
 * Phase 5B: sleep path routed through platform time boundary.
 *           Direct k_msleep dependency removed from runtime.
 * Phase 5D: platform critical-section boundary smoke. One enter/exit pair
 *           proves Zephyr irq_lock/irq_unlock backend links and runs correctly.
 * Phase 6A: smoke event handler registered; one USER event posted.
 * Phase 6B: single smoke replaced with burst of 3 USER events (arg0=0x6B).
 *           Proves burst/backpressure behavior.
 * Phase 6C: burst replaced with queue-full/drop smoke (arg0=0x6C, 17 events).
 *           Proves ERR_FULL path and dropped_count semantics.
 * Phase 6D: invalid/rejection smoke (arg0=0x6D, NONE + type=99).
 *           Proves ERR_INVALID_ARG + admission_rejected, not dropped_count.
 * Phase 6E: throttle smoke (arg0=0x6E, 3 valid USER events via try_post_event).
 *           seq=1 OK, seq=2 OK (creates throttle condition), seq=3 ERR_THROTTLED.
 * Phase 6H: ISR/timer producer stress-lite (committed, 8 events at 100ms).
 *           Proved post_event/dispatch path from Zephyr SysTick ISR context.
 * Phase 6F: Mixed event policy smoke (committed, replaced by Phase 6I).
 *           Proved accept/reject/drop in one thread-context boot run.
 * Phase 6I: Timer producer queue-pressure stress (arg0=0x6900).
 *           k_timer fires every 50ms. Attempts 24 events; capacity=16.
 *           Events 1-16 accepted (queue fills). Events 17-24 ERR_FULL.
 *           Producer rate (50ms) > consumer rate (500ms/tick, budget=1).
 *           Proves backpressure_active during burst; ERR_FULL from queue
 *           pressure, not admission failure. Handler dispatches 16 events.
 *           Final summary logged once handled==16.
 *           <zephyr/kernel.h> re-introduced for k_timer; Zephyr type must
 *           not appear in core/ or platform/ headers.
 * Phase 9A-C: Phase 6I synthetic startup burst gated behind compile-time
 *           macro DEVKIT_PHASE6I_STARTUP_BURST_ENABLED. Default 0 (disabled)
 *           so real button workload (Phase 9A-A/9A-B) boots without queue
 *           pressure from the synthetic burst. Re-enable with
 *             -DDEVKIT_PHASE6I_STARTUP_BURST_ENABLED=1
 *           for diagnostic stress runs. Phase 6I source preserved verbatim
 *           inside the #if guard for evidentiary continuity.
 */

/*
 * Phase 9A-C diagnostic gate.
 * Default 0 → Phase 6I synthetic startup burst code is fully compiled out:
 *   - timer ISR + counters not emitted
 *   - USER handler not registered
 *   - timer not started
 *   - "Phase 6I timer producer started" banner suppressed
 *   - "Phase 6I final:" summary suppressed
 *   - "Phase 6I event handled" milestones suppressed
 * Override with -DDEVKIT_PHASE6I_STARTUP_BURST_ENABLED=1 to restore the
 * Phase 6I diagnostic stress fixture exactly as it shipped through 9A-B.
 * No core, platform, or scheduler semantics are affected by this gate.
 */
#ifndef DEVKIT_PHASE6I_STARTUP_BURST_ENABLED
#define DEVKIT_PHASE6I_STARTUP_BURST_ENABLED 0
#endif

#include "devkit_runtime.h"
#include "devkit_build_info.h"
#include "devkit_button_producer.h"
#include "devkit_fault.h"
#include "devkit_observability.h"
#include "devkit_status_led.h"
#include "devkit_timer_producer.h"
#include "devkit_uart_producer.h"
#include "robotos_core.h"
#include "robotos_platform_critical.h"
#include "robotos_platform_time.h"

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(devkit_runtime, LOG_LEVEL_INF);

/* --------------------------------------------------------------------------
 * Phase 6I: Timer producer queue-pressure stress
 *
 * A Zephyr k_timer fires every 50ms from SysTick ISR context.
 * It posts USER events (arg0=0x6900) and stops after 24 attempts.
 *
 * Queue capacity = 16. Events 1-16 succeed; events 17-24 get ERR_FULL
 * (queue pressure -- not admission failure). Tick dispatches 1/500ms.
 *
 * Producer rate (20 events/s) >> consumer rate (2 events/s).
 * This creates genuine queue pressure and proves backpressure_active.
 *
 * ISR contract (Phase 5G):
 *   - no log, no sleep, no tick, no dispatch, no register
 *   - event is a stack local; valid for entire post_event call
 *   - all error returns handled via volatile counters only
 *
 * Expected final state:
 *   attempted=24, ok=16, full=8, invalid=0, other=0
 *   handled=16
 *   dropped=8 (ERR_FULL from queue pressure, admission already passed)
 *   CFSR=0, HFSR=0
 * -------------------------------------------------------------------------- */

#if DEVKIT_PHASE6I_STARTUP_BURST_ENABLED
#define DEVKIT_PHASE6I_MARKER          0x6900u
#define DEVKIT_PHASE6I_ATTEMPT_COUNT     24u   /* more than queue capacity=16 */
#define DEVKIT_PHASE6I_EXPECTED_OK       16u   /* == ROBOTOS_EVENT_QUEUE_CAPACITY */
#define DEVKIT_PHASE6I_EXPECTED_FULL      8u   /* ATTEMPT_COUNT - EXPECTED_OK */
#define DEVKIT_PHASE6I_TIMER_MS          50u   /* 50ms -- faster than Phase 6H's 100ms */

/* volatile: written from ISR (timer callback), read from thread context */
static volatile uint32_t s_prod_attempted;
static volatile uint32_t s_prod_ok;
static volatile uint32_t s_prod_full;
static volatile uint32_t s_prod_invalid;    /* expected 0 -- valid type only */
static volatile uint32_t s_prod_other;      /* expected 0 */

/* thread context only (written/read in run loop, never from ISR) */
static uint32_t s_handled_count;
static bool     s_prod_final_logged;

static struct k_timer s_phase6i_timer;

/* ISR context: called from Zephyr SysTick handler.
 * MUST NOT log, sleep, tick, dispatch, register, or fault. */
static void devkit_phase6i_timer_cb(struct k_timer *timer)
{
	static uint32_t seq = 1u;

	if (seq > DEVKIT_PHASE6I_ATTEMPT_COUNT) {
		return;
	}

	robotos_event_t ev = {
		.type           = ROBOTOS_EVENT_USER,
		.timestamp_tick = 0u,  /* uptime_ms not ISR-safe; omit */
		.arg0           = DEVKIT_PHASE6I_MARKER,
		.arg1           = seq,
	};

	s_prod_attempted++;
	robotos_core_status_t ret = robotos_core_post_event(&ev);
	if (ret == ROBOTOS_CORE_OK) {
		s_prod_ok++;
	} else if (ret == ROBOTOS_CORE_ERR_FULL) {
		s_prod_full++;
	} else if (ret == ROBOTOS_CORE_ERR_INVALID_ARG) {
		s_prod_invalid++;
	} else {
		s_prod_other++;
	}

	seq++;
	if (seq > DEVKIT_PHASE6I_ATTEMPT_COUNT) {
		k_timer_stop(timer);
	}
}

/* Handler runs from thread context (called by tick/dispatch, never from ISR).
 * Logs milestones only to avoid RTT buffer saturation. */
static robotos_core_status_t devkit_phase6i_handler(
	const robotos_event_t *event, void *user_context)
{
	(void)user_context;
	if (event == NULL) {
		return ROBOTOS_CORE_ERR_NULL;
	}
	if (event->type != ROBOTOS_EVENT_USER ||
	    event->arg0 != DEVKIT_PHASE6I_MARKER) {
		LOG_ERR("Phase 6I: unexpected event type=%d arg0=0x%x",
			(int)event->type, (unsigned)event->arg0);
		return ROBOTOS_CORE_ERR_INVALID_ARG;
	}

	s_handled_count++;
	uint32_t seq = (uint32_t)event->arg1;

	/* Milestones: seq=1 (first), seq=8 (mid), seq=16 (last accepted) */
	if (seq == 1u || seq == (DEVKIT_PHASE6I_EXPECTED_OK / 2u) ||
	    seq == DEVKIT_PHASE6I_EXPECTED_OK) {
		LOG_INF("Phase 6I event handled seq=%u count=%u", seq, s_handled_count);
	}
	return ROBOTOS_CORE_OK;
}
#endif /* DEVKIT_PHASE6I_STARTUP_BURST_ENABLED */

/* -------------------------------------------------------------------------- */

int devkit_runtime_init(void)
{
	int ret;
	robotos_core_status_t core_ret;

	devkit_fault_init();
	devkit_build_info_log();

	/* Phase 5D: one-time critical-section boundary smoke. */
	{
		robotos_platform_critical_token_t cs_tok =
			robotos_platform_critical_enter();
		robotos_platform_critical_exit(cs_tok);
		LOG_INF("platform critical smoke ok");
	}

	core_ret = robotos_core_init();
	if (core_ret != ROBOTOS_CORE_OK) {
		LOG_ERR("Core init failed: %d -- continuing", (int)core_ret);
	}

	/* Phase 9A-C: announce diagnostic gate state once at boot. Grep targets:
	 *   "DEVKIT_DIAG phase6i_startup_burst=0|1"
	 *   "Phase 6I startup burst disabled" (gate=0 only)
	 * The Phase 6I synthetic burst ran unconditionally through Phase 9A-B.
	 * Default in 9A-C is 0 so real button workload boots without queue
	 * pressure from the synthetic stress fixture. Re-enable with
	 *   -DDEVKIT_PHASE6I_STARTUP_BURST_ENABLED=1
	 * for stress diagnostic captures. */
#if DEVKIT_PHASE6I_STARTUP_BURST_ENABLED
	LOG_INF("DEVKIT_DIAG phase6i_startup_burst=1");

	/* Phase 6I: register USER handler before timer start */
	core_ret = robotos_core_register_event_handler(ROBOTOS_EVENT_USER,
						       devkit_phase6i_handler,
						       NULL);
	if (core_ret != ROBOTOS_CORE_OK) {
		LOG_ERR("Phase 6I handler registration failed: %d", (int)core_ret);
	}

	/* Phase 6I: start periodic 50ms timer.
	 * Callback posts from Zephyr SysTick ISR context.
	 * Stops itself after DEVKIT_PHASE6I_ATTEMPT_COUNT (24) posts. */
	k_timer_init(&s_phase6i_timer, devkit_phase6i_timer_cb, NULL);
	k_timer_start(&s_phase6i_timer,
		      K_MSEC(DEVKIT_PHASE6I_TIMER_MS),
		      K_MSEC(DEVKIT_PHASE6I_TIMER_MS));
	LOG_INF("Phase 6I timer producer started: attempts=%u interval=%ums",
		DEVKIT_PHASE6I_ATTEMPT_COUNT, DEVKIT_PHASE6I_TIMER_MS);
#else
	LOG_INF("DEVKIT_DIAG phase6i_startup_burst=0");
	LOG_INF("Phase 6I startup burst disabled (Phase 9A-C default; "
		"-DDEVKIT_PHASE6I_STARTUP_BURST_ENABLED=1 to restore)");
#endif

	LOG_INF("RobotOS devkit starting -- board: %s", CONFIG_BOARD);

	ret = devkit_status_led_init();
	if (ret < 0) {
		LOG_ERR("Status LED init failed: %d", ret);
		return ret;
	}

	LOG_INF("LED blink loop starting");

	/* Phase 6M: register the periodic producer's USER+1 handler.
	 * Distinct from the Phase 6I producer's USER handler so the two
	 * coexist without routing conflict. Failure is logged and ignored:
	 * the rest of the runtime continues without the diagnostic producer. */
	{
		robotos_core_status_t prod_ret = devkit_timer_producer_init();
		if (prod_ret != ROBOTOS_CORE_OK) {
			LOG_ERR("Phase 6M producer init failed: %d", (int)prod_ret);
		} else {
			LOG_INF("Phase 6M producer init: type=USER+1 marker=0x%x "
				"cadence=every %u ticks",
				(unsigned)DEVKIT_TIMER_PRODUCER_MARKER,
				(unsigned)DEVKIT_TIMER_PRODUCER_TICK_PERIOD);
		}
	}

	/* Phase 9A-A: register the user-button USER+2 handler and enable EXTI.
	 * Distinct from Phase 6I (USER) and Phase 6M (USER+1) so all three
	 * coexist without routing conflict. Failure is logged and ignored:
	 * the rest of the runtime continues without the button workload. */
	{
		robotos_core_status_t btn_ret = devkit_button_producer_init();
		if (btn_ret != ROBOTOS_CORE_OK) {
			LOG_ERR("Phase 9A-A button init failed: %d", (int)btn_ret);
		}
	}

	/* Phase 9B: register the UART RX USER+3 handler and enable RX IRQ.
	 * Distinct from Phase 6I (USER), Phase 6M (USER+1), Phase 9A-A (USER+2)
	 * so all real producers coexist without routing conflict. Failure is
	 * logged and ignored: the rest of the runtime continues without UART. */
	{
		robotos_core_status_t uart_ret = devkit_uart_producer_init();
		if (uart_ret != ROBOTOS_CORE_OK) {
			LOG_ERR("Phase 9B uart init failed: %d", (int)uart_ret);
		}
	}

	/* Phase 6K: emit one baseline observability snapshot after init */
	devkit_observability_log_snapshot();
	/* Phase 6L: emit one baseline fault diagnostic after init */
	devkit_observability_log_fault();
	/* Phase 6M: emit one baseline producer stats line after init */
	devkit_observability_log_producer_stats();
	/* Phase 9A-A: emit one baseline button stats line after init */
	devkit_button_producer_log_stats();
	/* Phase 9B: emit one baseline uart stats line after init */
	devkit_uart_producer_log_stats();

	return 0;
}

void devkit_runtime_run(void)
{
	int ret;
	robotos_core_status_t core_ret;
	uint32_t tick_count = 0;

	while (1) {
		ret = devkit_status_led_toggle();
		if (ret < 0) {
			LOG_ERR("LED toggle failed: %d", ret);
		}
		LOG_INF("tick count=%u", tick_count++);

		core_ret = robotos_core_tick();
		if (core_ret != ROBOTOS_CORE_OK) {
			LOG_ERR("Core tick failed: %d", (int)core_ret);
		}

		/* Phase 6M: cadence-driven post. on_tick is a no-op when the
		 * cadence predicate is false; one post at most per call. */
		devkit_timer_producer_on_tick(tick_count);

		/* Phase 6K/6L/6M/9A-A/9B: periodic observability snapshot + fault +
		 * producer + button + uart log. tick_count was post-incremented in
		 * the LOG_INF above, so it already reflects the next iteration
		 * index here. Fire every N. */
		if ((tick_count % DEVKIT_OBSERVABILITY_LOG_INTERVAL_TICKS) == 0u) {
			devkit_observability_log_snapshot();
			devkit_observability_log_fault();
			devkit_observability_log_producer_stats();
			devkit_button_producer_log_stats();
			devkit_uart_producer_log_stats();
		}

#if DEVKIT_PHASE6I_STARTUP_BURST_ENABLED
		/* Log final summary once after all accepted events are handled */
		if (s_handled_count >= DEVKIT_PHASE6I_EXPECTED_OK &&
		    !s_prod_final_logged) {
			robotos_core_snapshot_t snap;
			robotos_core_snapshot(&snap);
			LOG_INF("Phase 6I final: "
				"attempted=%u ok=%u full=%u invalid=%u other=%u "
				"handled=%u "
				"accepted=%u rejected=%u dropped=%u "
				"dispatched=%u herr=%u unhandled=%u "
				"bp=%d th_active=%d",
				(uint32_t)s_prod_attempted,
				(uint32_t)s_prod_ok,
				(uint32_t)s_prod_full,
				(uint32_t)s_prod_invalid,
				(uint32_t)s_prod_other,
				s_handled_count,
				snap.admission_accepted_count,
				snap.admission_rejected_count,
				snap.dropped_event_count,
				snap.dispatched_event_count,
				snap.handler_error_count,
				snap.unhandled_event_count,
				(int)snap.backpressure_active,
				(int)snap.producer_throttle_active);
			s_prod_final_logged = true;
		}
#endif /* DEVKIT_PHASE6I_STARTUP_BURST_ENABLED */

		robotos_platform_sleep_ms(DEVKIT_TICK_MS);
	}
}
