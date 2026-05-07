/*
 * devkit_observability.c
 * RobotOS devkit observability surface (Phase 6K).
 *
 * Bridges robotos_core_snapshot() to Zephyr LOG_INF for periodic RTT
 * visibility. Pure read-only -- never writes to core state, never
 * participates in scheduling/admission/dispatch/retry decisions.
 *
 * Field name discipline:
 *   - Field names match conventions established by the Phase 6I
 *     final-summary log (e.g. herr, bp, th_active) so existing RTT
 *     parsing tooling continues to work.
 *   - Line prefix is "ROBOTOS_OBS" so the periodic line is easy to
 *     grep separately from one-shot phase summaries.
 *
 * No dynamic allocation. No floating point. No string building on
 * the heap. The Zephyr LOG_INF macro handles deferred formatting.
 */

#include "devkit_observability.h"
#include "robotos_core.h"

#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(devkit_obs, LOG_LEVEL_INF);

/* Map core state enum to a stable, short, parser-friendly name. */
static const char *state_name(robotos_core_state_t s)
{
	switch (s) {
	case ROBOTOS_CORE_STATE_UNINITIALIZED: return "UNINIT";
	case ROBOTOS_CORE_STATE_READY:         return "READY";
	case ROBOTOS_CORE_STATE_ERROR:         return "ERROR";
	default:                               return "UNKNOWN";
	}
}

void devkit_observability_log_snapshot(void)
{
	robotos_core_snapshot_t snap;
	robotos_core_status_t   ret = robotos_core_snapshot(&snap);

	if (ret != ROBOTOS_CORE_OK) {
		LOG_ERR("ROBOTOS_OBS snapshot_failed ret=%d", (int)ret);
		return;
	}

	LOG_INF("ROBOTOS_OBS state=%s ticks=%u pending=%u peak=%u "
		"dropped=%u dispatched=%u herr=%u throttled=%u "
		"rejected=%u accepted=%u unhandled=%u bp=%d th_active=%d",
		state_name(snap.state),
		snap.tick_count,
		snap.pending_event_count,
		snap.peak_queue_depth,
		snap.dropped_event_count,
		snap.dispatched_event_count,
		snap.handler_error_count,
		snap.producer_throttled_count,
		snap.admission_rejected_count,
		snap.admission_accepted_count,
		snap.unhandled_event_count,
		(int)snap.backpressure_active,
		(int)snap.producer_throttle_active);
}
