/*
 * devkit_runtime.c
 * Boot sequence, periodic tick log, and status LED orchestration.
 * Phase 5B: sleep path routed through platform time boundary.
 *           Direct k_msleep dependency removed from runtime.
 * Phase 6A: smoke event handler registered; one USER event posted.
 * Phase 6B: single smoke replaced with burst of 3 USER events (arg0=0x6B,
 *           arg1=1..3). Proves burst/backpressure behavior: pending=3 > budget=1
 *           on first tick, drains one per tick, backpressure clears after tick 2.
 *           Final snapshot logged once after all 3 handled.
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
 * Phase 6B: burst event integration
 *
 * Three USER events are posted at init (arg0=0x6B, arg1=1/2/3).
 * budget=1 means one dispatched per tick — takes 3 ticks to drain.
 * Backpressure is active while pending > 1 (after post: true; clears
 * after tick 2 when pending drops to 1 == budget).
 * Final snapshot is logged once after all 3 events are handled.
 * -------------------------------------------------------------------------- */

#define BURST_SIZE 3u

static uint32_t s_burst_handled_count;
static bool     s_burst_summary_logged;

static const robotos_event_t s_burst_events[BURST_SIZE] = {
	{ .type = ROBOTOS_EVENT_USER, .timestamp_tick = 0u, .arg0 = 0x6Bu, .arg1 = 1u },
	{ .type = ROBOTOS_EVENT_USER, .timestamp_tick = 0u, .arg0 = 0x6Bu, .arg1 = 2u },
	{ .type = ROBOTOS_EVENT_USER, .timestamp_tick = 0u, .arg0 = 0x6Bu, .arg1 = 3u },
};

static robotos_core_status_t devkit_smoke_event_handler(
	const robotos_event_t *event, void *user_context)
{
	(void)user_context;
	if (event == NULL) {
		return ROBOTOS_CORE_ERR_NULL;
	}
	if (event->type != ROBOTOS_EVENT_USER) {
		return ROBOTOS_CORE_ERR_INVALID_ARG;
	}
	s_burst_handled_count++;
	LOG_INF("Phase 6B burst handled seq=%u count=%u",
		(unsigned)event->arg1, s_burst_handled_count);
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
		LOG_ERR("Core init failed: %d — continuing", (int)core_ret);
	}

	/* Phase 6B: register handler then post burst of BURST_SIZE events */
	core_ret = robotos_core_register_event_handler(ROBOTOS_EVENT_USER,
						       devkit_smoke_event_handler, NULL);
	if (core_ret != ROBOTOS_CORE_OK) {
		LOG_ERR("burst handler registration failed: %d", (int)core_ret);
	}

	for (uint32_t i = 0u; i < BURST_SIZE; i++) {
		core_ret = robotos_core_post_event(&s_burst_events[i]);
		if (core_ret != ROBOTOS_CORE_OK) {
			LOG_ERR("burst event post [%u] failed: %d", i, (int)core_ret);
		}
	}
	LOG_INF("Phase 6B: burst posted %u events pending=%u bp=%d",
		BURST_SIZE,
		robotos_core_pending_event_count(),
		(int)robotos_core_backpressure_active());

	LOG_INF("RobotOS devkit starting — board: %s", CONFIG_BOARD);

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

		/* Phase 6B: log final snapshot once after all burst events handled */
		if (s_burst_handled_count >= BURST_SIZE && !s_burst_summary_logged) {
			robotos_core_snapshot_t snap;
			robotos_core_snapshot(&snap);
			LOG_INF("Phase 6B summary: handled=%u pending=%u "
				"dispatched=%u accepted=%u rejected=%u "
				"dropped=%u herr=%u unhandled=%u bp=%d",
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
