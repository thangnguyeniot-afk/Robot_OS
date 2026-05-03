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
 * Phase 6H: ISR/timer producer stress-lite (committed, replaced by Phase 6F).
 *           k_timer callback posted 8 USER events from SysTick ISR context.
 *           Proven via hardware evidence (GDB counter read, 2026-05-03).
 * Phase 6F: Mixed event policy smoke (arg0=0x6F00).
 *           One boot run exercises:
 *             - NONE type event -> ERR_INVALID_ARG (admission_rejected++)
 *             - reserved type=99 event -> ERR_INVALID_ARG (admission_rejected++)
 *             - CAPACITY valid USER events posted -> all OK (queue full)
 *             - one more valid event -> ERR_FULL (dropped_count++)
 *           Proves accept/reject/drop paths in a single run.
 *           Phase 6H k_timer removed; <zephyr/kernel.h> no longer needed.
 */

#include "devkit_runtime.h"
#include "devkit_build_info.h"
#include "devkit_fault.h"
#include "devkit_status_led.h"
#include "robotos_core.h"
#include "robotos_event_queue.h"
#include "robotos_platform_critical.h"
#include "robotos_platform_time.h"

#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(devkit_runtime, LOG_LEVEL_INF);

/* --------------------------------------------------------------------------
 * Phase 6F: Mixed event policy smoke
 *
 * Run once at boot (devkit_runtime_init). Posts events covering three paths:
 *   1. NONE type        -> ERR_INVALID_ARG  (admission gate, rejected_count++)
 *   2. reserved type=99 -> ERR_INVALID_ARG  (admission gate, rejected_count++)
 *   3. CAPACITY valid   -> ROBOTOS_CORE_OK  (queue fills, accepted_count+=16)
 *   4. one more valid   -> ERR_FULL         (dropped_count++)
 *
 * The 16 queued valid events are dispatched by the tick loop (one per tick).
 * Handler logs first/mid/last milestones and final summary after all handled.
 *
 * Expected final state:
 *   accepted=16, rejected=2, dropped=1, handled=16
 *   CFSR=0, HFSR=0, no faults
 * -------------------------------------------------------------------------- */

#define DEVKIT_PHASE6F_MARKER           0x6F00u
#define DEVKIT_PHASE6F_CAPACITY         ROBOTOS_EVENT_QUEUE_CAPACITY  /* 16 */
#define DEVKIT_PHASE6F_EXPECTED_HANDLED DEVKIT_PHASE6F_CAPACITY

/* Boot-time smoke result counters (set in init, read in run) */
static uint32_t s_smoke_accepted;
static uint32_t s_smoke_rejected;
static uint32_t s_smoke_dropped;

/* Dispatch-time tracking (written/read in run loop thread context only) */
static uint32_t s_handled_count;
static bool     s_final_logged;

/* --------------------------------------------------------------------------
 * Phase 6F handler
 * Receives valid USER events (arg0 == DEVKIT_PHASE6F_MARKER, arg1 == seq 1..N).
 * Runs from tick/dispatch thread context only — never from ISR.
 * -------------------------------------------------------------------------- */
static robotos_core_status_t devkit_phase6f_handler(
	const robotos_event_t *event, void *user_context)
{
	(void)user_context;
	if (event == NULL) {
		return ROBOTOS_CORE_ERR_NULL;
	}
	if (event->type != ROBOTOS_EVENT_USER ||
	    event->arg0 != DEVKIT_PHASE6F_MARKER) {
		LOG_ERR("Phase 6F: unexpected event type=%d arg0=0x%x",
			(int)event->type, (unsigned)event->arg0);
		return ROBOTOS_CORE_ERR_INVALID_ARG;
	}

	s_handled_count++;
	uint32_t seq = (uint32_t)event->arg1;

	/* Log milestones only to avoid RTT spam */
	if (seq == 1u || seq == (DEVKIT_PHASE6F_CAPACITY / 2u) || seq == DEVKIT_PHASE6F_CAPACITY) {
		LOG_INF("Phase 6F event handled seq=%u count=%u", seq, s_handled_count);
	}
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
	 * Not wired into core queue -- single-threaded assumption unchanged. */
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

	/* Phase 6F: register USER handler from thread context (before smoke) */
	core_ret = robotos_core_register_event_handler(ROBOTOS_EVENT_USER,
						       devkit_phase6f_handler,
						       NULL);
	if (core_ret != ROBOTOS_CORE_OK) {
		LOG_ERR("Phase 6F handler registration failed: %d", (int)core_ret);
	}

	/* Phase 6F: mixed event policy smoke
	 *
	 * Path 1: NONE type -> ERR_INVALID_ARG (admission gate rejects)
	 * Path 2: reserved type=99 -> ERR_INVALID_ARG (admission gate rejects)
	 * Path 3: CAPACITY valid USER events -> all OK (queue fills)
	 * Path 4: one more valid USER event -> ERR_FULL (dropped_count++)
	 */
	{
		robotos_event_t ev;

		/* Path 1: invalid type NONE */
		ev.type           = ROBOTOS_EVENT_NONE;
		ev.timestamp_tick = 0u;
		ev.arg0           = DEVKIT_PHASE6F_MARKER;
		ev.arg1           = 0u;
		core_ret = robotos_core_post_event(&ev);
		if (core_ret == ROBOTOS_CORE_ERR_INVALID_ARG) {
			s_smoke_rejected++;
		}

		/* Path 2: invalid reserved type=99 */
		ev.type = (robotos_event_type_t)99;
		core_ret = robotos_core_post_event(&ev);
		if (core_ret == ROBOTOS_CORE_ERR_INVALID_ARG) {
			s_smoke_rejected++;
		}

		/* Path 3: fill queue with CAPACITY valid events */
		ev.type = ROBOTOS_EVENT_USER;
		for (uint32_t seq = 1u; seq <= DEVKIT_PHASE6F_CAPACITY; seq++) {
			ev.arg1 = seq;
			core_ret = robotos_core_post_event(&ev);
			if (core_ret == ROBOTOS_CORE_OK) {
				s_smoke_accepted++;
			}
		}

		/* Path 4: one more valid event -> ERR_FULL */
		ev.arg1 = DEVKIT_PHASE6F_CAPACITY + 1u;
		core_ret = robotos_core_post_event(&ev);
		if (core_ret == ROBOTOS_CORE_ERR_FULL) {
			s_smoke_dropped++;
		}
	}

	LOG_INF("Phase 6F smoke: accepted=%u rejected=%u dropped=%u",
		s_smoke_accepted, s_smoke_rejected, s_smoke_dropped);

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

		/* Phase 6F: log final summary once after all events dispatched */
		if (s_handled_count >= DEVKIT_PHASE6F_EXPECTED_HANDLED &&
		    !s_final_logged) {
			robotos_core_snapshot_t snap;
			robotos_core_snapshot(&snap);
			LOG_INF("Phase 6F final: "
				"smoke_accepted=%u smoke_rejected=%u smoke_dropped=%u "
				"handled=%u "
				"accepted=%u rejected=%u dropped=%u "
				"dispatched=%u herr=%u unhandled=%u "
				"bp=%d",
				s_smoke_accepted,
				s_smoke_rejected,
				s_smoke_dropped,
				s_handled_count,
				snap.admission_accepted_count,
				snap.admission_rejected_count,
				snap.dropped_event_count,
				snap.dispatched_event_count,
				snap.handler_error_count,
				snap.unhandled_event_count,
				(int)snap.backpressure_active);
			s_final_logged = true;
		}

		robotos_platform_sleep_ms(DEVKIT_TICK_MS);
	}
}
