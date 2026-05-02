/*
 * devkit_runtime.c
 * Boot sequence, periodic tick log, and status LED orchestration.
 * Phase 5B: sleep path routed through platform time boundary.
 *           Direct k_msleep dependency removed from runtime.
 * Phase 6A: smoke event handler registered; one USER event posted.
 * Phase 6B: single smoke replaced with burst of 3 USER events (arg0=0x6B).
 *           Proves burst/backpressure behavior.
 * Phase 6C: burst replaced with queue-full/drop smoke (arg0=0x6C, 17 events).
 *           Proves ERR_FULL path and dropped_count semantics.
 * Phase 6D: replaced with invalid/rejection smoke (arg0=0x6D, 2 invalid events).
 *           Posts ROBOTOS_EVENT_NONE and a reserved type (99).
 *           Proves ERR_INVALID_ARG + admission_rejected_count, NOT dropped_count.
 *           Handler registered for USER but never called (no valid events posted).
 */

#include "devkit_runtime.h"
#include "devkit_build_info.h"
#include "devkit_fault.h"
#include "devkit_status_led.h"
#include "robotos_core.h"
#include "robotos_platform_time.h"

#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(devkit_runtime, LOG_LEVEL_INF);

/* --------------------------------------------------------------------------
 * Phase 6D: invalid / rejection smoke
 *
 * Post 2 invalid events once at init:
 *   1. ROBOTOS_EVENT_NONE  (type=0)         -> ERR_INVALID_ARG
 *   2. reserved type 99                     -> ERR_INVALID_ARG
 * Both are rejected at admission gate before touching the queue.
 * Expected after posts:
 *   invalid_arg_count = 2
 *   rejected          = 2   (admission_rejected_count)
 *   pending           = 0
 *   accepted          = 0
 *   dropped           = 0
 *   dispatched        = 0
 *   handler_called    = 0   (USER handler registered but never invoked)
 *   unhandled         = 0
 *   backpressure      = false
 * Final summary logged once after tick 2.
 * -------------------------------------------------------------------------- */

#define DEVKIT_PHASE6D_MARKER         0x6Du
#define DEVKIT_PHASE6D_ATTEMPT_COUNT  2u
#define DEVKIT_PHASE6D_RESERVED_TYPE  ((robotos_event_type_t)99)
#define DEVKIT_PHASE6D_SUMMARY_TICK   2u

static uint32_t s_rejection_attempted_count;
static uint32_t s_rejection_invalid_arg_count;
static uint32_t s_rejection_other_error_count;
static uint32_t s_rejection_handler_called_count;
static bool     s_rejection_summary_logged;

static robotos_core_status_t devkit_rejection_handler(
	const robotos_event_t *event, void *user_context)
{
	/* This handler should NEVER be called in Phase 6D — no valid events posted */
	(void)user_context;
	s_rejection_handler_called_count++;
	LOG_ERR("Phase 6D: handler unexpectedly called! type=%d arg0=0x%x",
		event ? (int)event->type : -1,
		event ? (unsigned)event->arg0 : 0u);
	return ROBOTOS_CORE_ERR_INVALID_STATE;
}

/* -------------------------------------------------------------------------- */

int devkit_runtime_init(void)
{
	int ret;
	robotos_core_status_t core_ret;

	devkit_fault_init();
	devkit_build_info_log();

	core_ret = robotos_core_init();
	if (core_ret != ROBOTOS_CORE_OK) {
		LOG_ERR("Core init failed: %d -- continuing", (int)core_ret);
	}

	/* Phase 6D: register USER handler — should never be called this phase */
	core_ret = robotos_core_register_event_handler(ROBOTOS_EVENT_USER,
						       devkit_rejection_handler,
						       NULL);
	if (core_ret != ROBOTOS_CORE_OK) {
		LOG_ERR("Phase 6D handler registration failed: %d",
			(int)core_ret);
	}

	/* Phase 6D: post 2 invalid events */
	const struct {
		robotos_event_type_t type;
		uint32_t             arg1;
		const char          *label;
	} invalid_events[DEVKIT_PHASE6D_ATTEMPT_COUNT] = {
		{ ROBOTOS_EVENT_NONE,              1u, "NONE"     },
		{ DEVKIT_PHASE6D_RESERVED_TYPE,    2u, "type=99"  },
	};

	for (uint32_t i = 0u; i < DEVKIT_PHASE6D_ATTEMPT_COUNT; i++) {
		robotos_event_t ev = {
			.type           = invalid_events[i].type,
			.timestamp_tick = 0u,
			.arg0           = DEVKIT_PHASE6D_MARKER,
			.arg1           = invalid_events[i].arg1,
		};
		s_rejection_attempted_count++;
		core_ret = robotos_core_post_event(&ev);
		if (core_ret == ROBOTOS_CORE_ERR_INVALID_ARG) {
			s_rejection_invalid_arg_count++;
		} else {
			s_rejection_other_error_count++;
			LOG_ERR("Phase 6D: post[%s] unexpected result=%d",
				invalid_events[i].label, (int)core_ret);
		}
	}

	/* Post summary */
	{
		robotos_core_snapshot_t snap;
		robotos_core_snapshot(&snap);
		LOG_INF("Phase 6D post summary: "
			"attempted=%u inv_arg=%u other_err=%u "
			"pending=%u accepted=%u rejected=%u "
			"dropped=%u dispatched=%u herr=%u "
			"unhandled=%u bp=%d handler_called=%u",
			s_rejection_attempted_count,
			s_rejection_invalid_arg_count,
			s_rejection_other_error_count,
			snap.pending_event_count,
			snap.admission_accepted_count,
			snap.admission_rejected_count,
			snap.dropped_event_count,
			snap.dispatched_event_count,
			snap.handler_error_count,
			snap.unhandled_event_count,
			(int)snap.backpressure_active,
			s_rejection_handler_called_count);
	}

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

		/* Phase 6D: log final summary once after DEVKIT_PHASE6D_SUMMARY_TICK */
		if (tick_count >= DEVKIT_PHASE6D_SUMMARY_TICK &&
		    !s_rejection_summary_logged) {
			robotos_core_snapshot_t snap;
			robotos_core_snapshot(&snap);
			LOG_INF("Phase 6D final summary: "
				"attempted=%u inv_arg=%u other_err=%u "
				"pending=%u accepted=%u rejected=%u "
				"dropped=%u dispatched=%u herr=%u "
				"unhandled=%u bp=%d handler_called=%u",
				s_rejection_attempted_count,
				s_rejection_invalid_arg_count,
				s_rejection_other_error_count,
				snap.pending_event_count,
				snap.admission_accepted_count,
				snap.admission_rejected_count,
				snap.dropped_event_count,
				snap.dispatched_event_count,
				snap.handler_error_count,
				snap.unhandled_event_count,
				(int)snap.backpressure_active,
				s_rejection_handler_called_count);
			s_rejection_summary_logged = true;
		}

		robotos_platform_sleep_ms(DEVKIT_TICK_MS);
	}
}
