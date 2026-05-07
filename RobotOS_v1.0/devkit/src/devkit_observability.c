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
#include "devkit_timer_producer.h"
#include "robotos_core.h"

#include <stdbool.h>
#include <stdint.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(devkit_obs, LOG_LEVEL_INF);

/*
 * ARMv7-M System Control Block fault registers (ARMv7-M ARM, B3.2.10).
 * Memory-mapped at fixed addresses for all Cortex-M3/M4/M7 devices.
 * Read is passive; status bits are write-1-to-clear, so reading does
 * not modify them.
 */
#define DEVKIT_OBS_SCB_CFSR_ADDR 0xE000ED28u
#define DEVKIT_OBS_SCB_HFSR_ADDR 0xE000ED2Cu

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

/*
 * Phase 6L: read CFSR/HFSR directly from the SCB and emit a ROBOTOS_FAULT line.
 *
 * Sticky bits in these registers are write-1-to-clear; a plain read does
 * not modify them and is therefore safe to call repeatedly. No fault
 * recovery, no panic, no register clearing, no scheduling influence.
 *
 * Note: HFSR.DEBUGEVT may be set when a debugger has triggered a halt
 * event, even in the absence of an actual crash. Raw register values
 * are surfaced unfiltered so the operator can inspect the situation.
 */
void devkit_observability_log_fault(void)
{
	const volatile uint32_t *cfsr_reg =
		(const volatile uint32_t *)DEVKIT_OBS_SCB_CFSR_ADDR;
	const volatile uint32_t *hfsr_reg =
		(const volatile uint32_t *)DEVKIT_OBS_SCB_HFSR_ADDR;

	uint32_t cfsr = *cfsr_reg;
	uint32_t hfsr = *hfsr_reg;
	bool     active = (cfsr != 0u) || (hfsr != 0u);

	LOG_INF("ROBOTOS_FAULT active=%d cfsr=0x%08x hfsr=0x%08x context=%s",
		(int)active,
		(unsigned int)cfsr,
		(unsigned int)hfsr,
		active ? "fault" : "none");
}

/*
 * Phase 6M: emit ROBOTOS_PROD summary line.
 *
 * Field name discipline matches Phase 6I/6K conventions: integer counters,
 * one-line, stable key=value shape, grep-friendly. The "type=USER+1"
 * suffix is a literal string identifying the event type used by the
 * producer; it is stable across builds because the producer's event type
 * is a compile-time constant.
 */
void devkit_observability_log_producer_stats(void)
{
	devkit_timer_producer_stats_t st;
	devkit_timer_producer_get_stats(&st);

	LOG_INF("ROBOTOS_PROD attempted=%u ok=%u throttled=%u dropped=%u "
		"invalid=%u other=%u type=USER+1",
		st.attempted,
		st.ok,
		st.throttled,
		st.dropped,
		st.invalid,
		st.other);
}
