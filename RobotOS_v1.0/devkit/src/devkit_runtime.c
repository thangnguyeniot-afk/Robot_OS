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
 * Phase 6D: invalid/rejection smoke (arg0=0x6D, NONE + type=99).
 *           Proves ERR_INVALID_ARG + admission_rejected, not dropped_count.
 * Phase 6E: throttle smoke (arg0=0x6E, 3 valid USER events via try_post_event).
 *           seq=1 OK, seq=2 OK (creates throttle condition), seq=3 ERR_THROTTLED.
 *           Proves throttle path: pending > budget, queue not full, no drop.
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
 * Phase 6E: throttled producer smoke
 *
 * Use robotos_core_try_post_event() for all three attempts:
 *   seq=1: OK  -> pending=1, budget=1, throttle=false
 *   seq=2: OK  -> pending=2 > budget=1, throttle=true (but not full)
 *   seq=3: ERR_THROTTLED -> pending stays 2, producer_throttled_count++
 *
 * Handler sees seq=1 and seq=2 only (seq=3 never enters queue).
 * One event per tick drained; after 2 ticks: pending=0, throttle=false.
 * Final summary logged once after handler_called_count == 2.
 * -------------------------------------------------------------------------- */

#define DEVKIT_PHASE6E_MARKER         0x6Eu
#define DEVKIT_PHASE6E_ATTEMPT_COUNT  3u
#define DEVKIT_PHASE6E_EXPECTED_OK    2u

static uint32_t s_throttle_attempted_count;
static uint32_t s_throttle_ok_count;
static uint32_t s_throttle_throttled_count;
static uint32_t s_throttle_full_count;
static uint32_t s_throttle_other_error_count;
static uint32_t s_throttle_handler_called_count;
static uint32_t s_throttle_unexpected_seq_count;
static bool     s_throttle_summary_logged;

static robotos_core_status_t devkit_throttle_handler(
	const robotos_event_t *event, void *user_context)
{
	(void)user_context;
	if (event == NULL) {
		return ROBOTOS_CORE_ERR_NULL;
	}
	if (event->type != ROBOTOS_EVENT_USER) {
		LOG_ERR("Phase 6E: unexpected type=%d", (int)event->type);
		s_throttle_unexpected_seq_count++;
		return ROBOTOS_CORE_ERR_INVALID_ARG;
	}
	if (event->arg0 != DEVKIT_PHASE6E_MARKER) {
		LOG_ERR("Phase 6E: unexpected arg0=0x%x", (unsigned)event->arg0);
		s_throttle_unexpected_seq_count++;
		return ROBOTOS_CORE_ERR_INVALID_ARG;
	}

	uint32_t seq = (uint32_t)event->arg1;

	/* seq=3 should never reach handler — it was throttled before enqueue */
	if (seq == 3u) {
		LOG_ERR("Phase 6E: seq=3 reached handler (should have been throttled)");
		s_throttle_unexpected_seq_count++;
		return ROBOTOS_CORE_ERR_INVALID_STATE;
	}

	s_throttle_handler_called_count++;
	LOG_INF("Phase 6E: handled seq=%u count=%u",
		seq, s_throttle_handler_called_count);
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

	/* Phase 6E: register USER handler */
	core_ret = robotos_core_register_event_handler(ROBOTOS_EVENT_USER,
						       devkit_throttle_handler,
						       NULL);
	if (core_ret != ROBOTOS_CORE_OK) {
		LOG_ERR("Phase 6E handler registration failed: %d",
			(int)core_ret);
	}

	/* Phase 6E: post 3 valid USER events via try_post_event */
	for (uint32_t i = 0u; i < DEVKIT_PHASE6E_ATTEMPT_COUNT; i++) {
		robotos_event_t ev = {
			.type           = ROBOTOS_EVENT_USER,
			.timestamp_tick = 0u,
			.arg0           = DEVKIT_PHASE6E_MARKER,
			.arg1           = (uint32_t)(i + 1u),
		};
		s_throttle_attempted_count++;
		core_ret = robotos_core_try_post_event(&ev);
		if (core_ret == ROBOTOS_CORE_OK) {
			s_throttle_ok_count++;
		} else if (core_ret == ROBOTOS_CORE_ERR_THROTTLED) {
			s_throttle_throttled_count++;
		} else if (core_ret == ROBOTOS_CORE_ERR_FULL) {
			s_throttle_full_count++;
		} else {
			s_throttle_other_error_count++;
			LOG_ERR("Phase 6E: try_post[%u] unexpected err=%d",
				i + 1u, (int)core_ret);
		}
	}

	/* Post summary */
	{
		robotos_core_snapshot_t snap;
		robotos_core_snapshot(&snap);
		LOG_INF("Phase 6E post summary: "
			"attempted=%u ok=%u throttled=%u full=%u other=%u "
			"pending=%u accepted=%u rejected=%u dropped=%u "
			"prod_throttled=%u bp=%d th_active=%d",
			s_throttle_attempted_count,
			s_throttle_ok_count,
			s_throttle_throttled_count,
			s_throttle_full_count,
			s_throttle_other_error_count,
			snap.pending_event_count,
			snap.admission_accepted_count,
			snap.admission_rejected_count,
			snap.dropped_event_count,
			snap.producer_throttled_count,
			(int)snap.backpressure_active,
			(int)snap.producer_throttle_active);
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

		/* Phase 6E: log final summary once after both accepted events handled */
		if (s_throttle_handler_called_count >= DEVKIT_PHASE6E_EXPECTED_OK &&
		    !s_throttle_summary_logged) {
			robotos_core_snapshot_t snap;
			robotos_core_snapshot(&snap);
			LOG_INF("Phase 6E final summary: "
				"handled=%u pending=%u dispatched=%u "
				"accepted=%u rejected=%u dropped=%u "
				"prod_throttled=%u herr=%u unhandled=%u "
				"bp=%d th_active=%d unexpected=%u",
				s_throttle_handler_called_count,
				snap.pending_event_count,
				snap.dispatched_event_count,
				snap.admission_accepted_count,
				snap.admission_rejected_count,
				snap.dropped_event_count,
				snap.producer_throttled_count,
				snap.handler_error_count,
				snap.unhandled_event_count,
				(int)snap.backpressure_active,
				(int)snap.producer_throttle_active,
				s_throttle_unexpected_seq_count);
			s_throttle_summary_logged = true;
		}

		robotos_platform_sleep_ms(DEVKIT_TICK_MS);
	}
}
