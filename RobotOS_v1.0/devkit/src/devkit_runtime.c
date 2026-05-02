/*
 * devkit_runtime.c
 * Boot sequence, periodic tick log, and status LED orchestration.
 * Phase 5B: sleep path routed through platform time boundary.
 *           Direct k_msleep dependency removed from runtime.
 * Phase 6A: smoke event handler registered; one USER event posted.
 * Phase 6B: single smoke replaced with burst of 3 USER events (arg0=0x6B,
 *           arg1=1..3). Proves burst/backpressure behavior.
 * Phase 6C: burst replaced with queue-full/drop smoke (arg0=0x6C,
 *           arg1=1..17). Posts ROBOTOS_EVENT_QUEUE_CAPACITY+1 valid USER
 *           events. Proves 16 accepted, 17th returns ERR_FULL, dropped_count
 *           increments, admission_rejected stays 0.
 */

#include "devkit_runtime.h"
#include "devkit_build_info.h"
#include "devkit_fault.h"
#include "devkit_status_led.h"
#include "robotos_core.h"
#include "robotos_event_queue.h"
#include "robotos_platform_time.h"

#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(devkit_runtime, LOG_LEVEL_INF);

/* --------------------------------------------------------------------------
 * Phase 6C: queue-full / drop smoke
 *
 * Attempt ROBOTOS_EVENT_QUEUE_CAPACITY + 1 valid USER posts at init.
 * First 16 must be accepted (CORE_OK).
 * 17th must be rejected with ERR_FULL, incrementing dropped_count.
 * admission_rejected_count must remain 0 throughout.
 * Backpressure is active while queue is full (pending==16 > budget==1).
 * Tick budget=1 drains one event per tick -> 16 ticks to empty queue.
 * Handler logs milestones at seq=1, seq=8, seq=16 only (no RTT flood).
 * Final summary logged once after handled_count == 16.
 * -------------------------------------------------------------------------- */

#define DEVKIT_PHASE6C_ATTEMPT_COUNT  (ROBOTOS_EVENT_QUEUE_CAPACITY + 1u)
#define DEVKIT_PHASE6C_MARKER         0x6Cu
#define DEVKIT_PHASE6C_EXPECTED_OK    ROBOTOS_EVENT_QUEUE_CAPACITY

static uint32_t s_burst_attempted_count;
static uint32_t s_burst_posted_ok_count;
static uint32_t s_burst_full_count;
static uint32_t s_burst_post_error_count;
static uint32_t s_burst_handled_count;
static bool     s_burst_summary_logged;

static robotos_core_status_t devkit_smoke_event_handler(
	const robotos_event_t *event, void *user_context)
{
	(void)user_context;
	if (event == NULL) {
		return ROBOTOS_CORE_ERR_NULL;
	}
	if (event->type != ROBOTOS_EVENT_USER) {
		LOG_ERR("Phase 6C: unexpected event type=%d", (int)event->type);
		return ROBOTOS_CORE_ERR_INVALID_ARG;
	}
	if (event->arg0 != DEVKIT_PHASE6C_MARKER) {
		LOG_ERR("Phase 6C: unexpected arg0=0x%x", (unsigned)event->arg0);
		return ROBOTOS_CORE_ERR_INVALID_ARG;
	}

	s_burst_handled_count++;

	/* Log milestones only to avoid RTT flood */
	uint32_t seq = (uint32_t)event->arg1;
	if (seq == 1u || seq == 8u || seq == DEVKIT_PHASE6C_EXPECTED_OK) {
		LOG_INF("Phase 6C: handled seq=%u count=%u",
			seq, s_burst_handled_count);
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

	core_ret = robotos_core_init();
	if (core_ret != ROBOTOS_CORE_OK) {
		LOG_ERR("Core init failed: %d -- continuing", (int)core_ret);
	}

	/* Phase 6C: register handler */
	core_ret = robotos_core_register_event_handler(ROBOTOS_EVENT_USER,
						       devkit_smoke_event_handler,
						       NULL);
	if (core_ret != ROBOTOS_CORE_OK) {
		LOG_ERR("Phase 6C handler registration failed: %d",
			(int)core_ret);
	}

	/* Phase 6C: post ROBOTOS_EVENT_QUEUE_CAPACITY+1 valid USER events */
	for (uint32_t i = 0u; i < DEVKIT_PHASE6C_ATTEMPT_COUNT; i++) {
		robotos_event_t ev = {
			.type           = ROBOTOS_EVENT_USER,
			.timestamp_tick = 0u,
			.arg0           = DEVKIT_PHASE6C_MARKER,
			.arg1           = (uint32_t)(i + 1u),
		};
		s_burst_attempted_count++;
		core_ret = robotos_core_post_event(&ev);
		if (core_ret == ROBOTOS_CORE_OK) {
			s_burst_posted_ok_count++;
		} else if (core_ret == ROBOTOS_CORE_ERR_FULL) {
			s_burst_full_count++;
		} else {
			s_burst_post_error_count++;
			LOG_ERR("Phase 6C: post[%u] unexpected err=%d",
				i, (int)core_ret);
		}
	}

	/* Post summary */
	{
		robotos_core_snapshot_t snap;
		robotos_core_snapshot(&snap);
		LOG_INF("Phase 6C post summary: "
			"attempted=%u ok=%u full=%u errs=%u "
			"pending=%u dropped=%u accepted=%u rejected=%u bp=%d",
			s_burst_attempted_count,
			s_burst_posted_ok_count,
			s_burst_full_count,
			s_burst_post_error_count,
			snap.pending_event_count,
			snap.dropped_event_count,
			snap.admission_accepted_count,
			snap.admission_rejected_count,
			(int)snap.backpressure_active);
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

		/* Phase 6C: log final summary once after all 16 events handled */
		if (s_burst_handled_count >= DEVKIT_PHASE6C_EXPECTED_OK &&
		    !s_burst_summary_logged) {
			robotos_core_snapshot_t snap;
			robotos_core_snapshot(&snap);
			LOG_INF("Phase 6C summary: "
				"handled=%u pending=%u dispatched=%u "
				"accepted=%u rejected=%u dropped=%u "
				"herr=%u unhandled=%u bp=%d",
				s_burst_handled_count,
				snap.pending_event_count,
				snap.dispatched_event_count,
				snap.admission_accepted_count,
				snap.admission_rejected_count,
				snap.dropped_event_count,
				snap.handler_error_count,
				snap.unhandled_event_count,
				(int)snap.backpressure_active);
			s_burst_summary_logged = true;
		}

		robotos_platform_sleep_ms(DEVKIT_TICK_MS);
	}
}
