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
 *           Proves throttle path: pending > budget, queue not full, no drop.
 * Phase 6G: ISR/timer producer smoke. k_timer callback posts 2 USER events
 *           (arg0=0x0607, seq=1..2) from Zephyr SysTick ISR context using
 *           robotos_core_post_event(). No log/sleep/dispatch/register in
 *           callback. Normal tick() dispatches; handler logs from thread
 *           context. Proves conditional ISR-safe producer contract (Phase 5G).
 *           <zephyr/kernel.h> re-introduced here for k_timer only; Zephyr
 *           timer type must not appear in core or platform headers.
 */

#include "devkit_runtime.h"
#include "devkit_build_info.h"
#include "devkit_fault.h"
#include "devkit_status_led.h"
#include "robotos_core.h"
#include "robotos_platform_critical.h"
#include "robotos_platform_time.h"

/* Phase 6G: k_timer used for ISR-like producer smoke.
 * Zephyr kernel header re-introduced in devkit layer only.
 * core/ and platform/ headers must remain Zephyr-free. */
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(devkit_runtime, LOG_LEVEL_INF);

/* --------------------------------------------------------------------------
 * Phase 6G: ISR/timer producer smoke
 *
 * A Zephyr k_timer fires periodically (100 ms). Its callback runs from the
 * Zephyr SysTick ISR context (IRQ-masked, genuine ISR path on ARMv7-M).
 * The callback posts one USER event per invocation using
 * robotos_core_post_event() and stops itself after 2 attempts.
 *
 * Contract from Phase 5G observed in callback:
 *   - no logging
 *   - no sleep
 *   - no tick / dispatch / register / unregister
 *   - event is a local struct (stack): valid for call duration
 *   - all error returns handled via volatile counters only
 *
 * Normal runtime tick() dispatches the posted events; handler runs from
 * thread context and logs evidence.
 *
 * Expected outcome:
 *   attempted=2, ok=2, full=0, invalid=0, other=0
 *   handled=2, unexpected=0
 *   pending=0, dropped=0, prod_throttled=0, herr=0
 * -------------------------------------------------------------------------- */

#define DEVKIT_PHASE6G_MARKER          0x0607u
#define DEVKIT_PHASE6G_MAX_ATTEMPTS    2u
#define DEVKIT_PHASE6G_EXPECTED_OK     2u
#define DEVKIT_PHASE6G_TIMER_PERIOD_MS 100u

/* volatile: written from ISR (timer callback), read from thread context */
static volatile uint32_t s_timer_attempted_count;
static volatile uint32_t s_timer_ok_count;
static volatile uint32_t s_timer_full_count;
static volatile uint32_t s_timer_invalid_count;
static volatile uint32_t s_timer_other_error_count;
static volatile uint32_t s_timer_handled_count;
static volatile uint32_t s_timer_unexpected_count;
static bool              s_timer_final_logged;

static struct k_timer s_phase6g_timer;

/* ISR context: called from Zephyr SysTick handler.
 * MUST NOT log, sleep, tick, dispatch, register, or fault. */
static void devkit_phase6g_timer_cb(struct k_timer *timer)
{
	/* Static seq persists across invocations; timer callback is
	 * serialized by Zephyr (only one active at a time). */
	static uint32_t seq = 1u;

	if (seq > DEVKIT_PHASE6G_MAX_ATTEMPTS) {
		return;
	}

	/* Local stack event: valid for entire duration of post_event call. */
	robotos_event_t ev = {
		.type           = ROBOTOS_EVENT_USER,
		.timestamp_tick = 0u,  /* uptime_ms not ISR-safe; omit */
		.arg0           = DEVKIT_PHASE6G_MARKER,
		.arg1           = seq,
	};

	s_timer_attempted_count++;
	robotos_core_status_t ret = robotos_core_post_event(&ev);
	if (ret == ROBOTOS_CORE_OK) {
		s_timer_ok_count++;
	} else if (ret == ROBOTOS_CORE_ERR_FULL) {
		s_timer_full_count++;
	} else if (ret == ROBOTOS_CORE_ERR_INVALID_ARG) {
		s_timer_invalid_count++;
	} else {
		s_timer_other_error_count++;
	}

	seq++;
	if (seq > DEVKIT_PHASE6G_MAX_ATTEMPTS) {
		/* Stop timer after 2 attempts; k_timer_stop is safe from callback */
		k_timer_stop(timer);
	}
}

/* Handler runs from thread context (called by tick/dispatch, never from ISR). */
static robotos_core_status_t devkit_phase6g_handler(
	const robotos_event_t *event, void *user_context)
{
	(void)user_context;
	if (event == NULL) {
		return ROBOTOS_CORE_ERR_NULL;
	}
	if (event->type != ROBOTOS_EVENT_USER) {
		LOG_ERR("Phase 6G: unexpected type=%d", (int)event->type);
		s_timer_unexpected_count++;
		return ROBOTOS_CORE_ERR_INVALID_ARG;
	}
	if (event->arg0 != DEVKIT_PHASE6G_MARKER) {
		LOG_ERR("Phase 6G: unexpected arg0=0x%x", (unsigned)event->arg0);
		s_timer_unexpected_count++;
		return ROBOTOS_CORE_ERR_INVALID_ARG;
	}

	uint32_t seq = (uint32_t)event->arg1;
	if (seq < 1u || seq > DEVKIT_PHASE6G_MAX_ATTEMPTS) {
		LOG_ERR("Phase 6G: unexpected seq=%u", seq);
		s_timer_unexpected_count++;
		return ROBOTOS_CORE_ERR_INVALID_ARG;
	}

	s_timer_handled_count++;
	LOG_INF("Phase 6G timer event handled seq=%u count=%u",
		seq, (uint32_t)s_timer_handled_count);
	return ROBOTOS_CORE_OK;
}

/* -------------------------------------------------------------------------- */

int devkit_runtime_init(void)
{
	int ret;
	robotos_core_status_t core_ret;

	devkit_fault_init();
	devkit_build_info_log();

	/* Phase 5D: one-time critical-section boundary smoke.
	 * Proves Zephyr irq_lock/irq_unlock backend links and executes.
	 * Not wired into core queue — single-threaded assumption unchanged. */
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

	/* Phase 6G: register USER handler from thread context (before timer start) */
	core_ret = robotos_core_register_event_handler(ROBOTOS_EVENT_USER,
						       devkit_phase6g_handler,
						       NULL);
	if (core_ret != ROBOTOS_CORE_OK) {
		LOG_ERR("Phase 6G handler registration failed: %d",
			(int)core_ret);
	}

	/* Phase 6G: init and start periodic 100ms timer.
	 * Timer callback is called from Zephyr SysTick ISR context.
	 * It stops itself after 2 attempts. */
	k_timer_init(&s_phase6g_timer, devkit_phase6g_timer_cb, NULL);
	k_timer_start(&s_phase6g_timer,
		      K_MSEC(DEVKIT_PHASE6G_TIMER_PERIOD_MS),
		      K_MSEC(DEVKIT_PHASE6G_TIMER_PERIOD_MS));
	LOG_INF("Phase 6G timer producer started");

	LOG_INF("RobotOS devkit starting -- board: %s", CONFIG_BOARD);

	ret = devkit_status_led_init();
	if (ret < 0) {
		LOG_ERR("Status LED init failed: %d", ret);
		return ret;
	}

	LOG_INF("LED blink loop starting");

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

		/* Phase 6G: log final summary once after both timer events handled */
		if ((uint32_t)s_timer_handled_count >= DEVKIT_PHASE6G_EXPECTED_OK &&
		    !s_timer_final_logged) {
			robotos_core_snapshot_t snap;
			robotos_core_snapshot(&snap);
			LOG_INF("Phase 6G final summary: "
				"attempted=%u ok=%u full=%u invalid=%u other=%u "
				"handled=%u unexpected=%u "
				"pending=%u dispatched=%u accepted=%u rejected=%u "
				"dropped=%u prod_throttled=%u herr=%u unhandled=%u "
				"bp=%d th_active=%d",
				(uint32_t)s_timer_attempted_count,
				(uint32_t)s_timer_ok_count,
				(uint32_t)s_timer_full_count,
				(uint32_t)s_timer_invalid_count,
				(uint32_t)s_timer_other_error_count,
				(uint32_t)s_timer_handled_count,
				(uint32_t)s_timer_unexpected_count,
				snap.pending_event_count,
				snap.dispatched_event_count,
				snap.admission_accepted_count,
				snap.admission_rejected_count,
				snap.dropped_event_count,
				snap.producer_throttled_count,
				snap.handler_error_count,
				snap.unhandled_event_count,
				(int)snap.backpressure_active,
				(int)snap.producer_throttle_active);
			s_timer_final_logged = true;
		}

		robotos_platform_sleep_ms(DEVKIT_TICK_MS);
	}
}
