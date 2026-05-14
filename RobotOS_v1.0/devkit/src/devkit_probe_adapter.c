/*
 * devkit_probe_adapter.c
 *
 * RobotOS devkit -- probe_translator runtime adapter (Phase 12L).
 *
 * Implements the public API declared in devkit_probe_adapter.h. Owns the
 * module-static probe_translator_t instance and bridges devkit-side command
 * signals to the probe_translator public API.
 *
 * Spec anchor:
 *   RobotOS_v1.0/devkit/docs/03_SPECS/PROBE_TRANSLATOR_RUNTIME_ADMISSION_PLAN.md
 *
 * Implementation notes:
 *   - The probe_translator is build-admitted to the devkit Zephyr image
 *     at Phase 12K. This module is the runtime admission point (Phase 12L).
 *   - The static probe_translator_t lives for the lifetime of the devkit
 *     process; its embedded fsm + bridge configs satisfy the Framework
 *     lifetime contract by construction.
 *   - No heap. No scheduler call. No ISR-context call. No UART include.
 *   - All entry points are thread-context only. Callers are the core
 *     dispatcher thread (devkit_app_state on_uart_byte / on_button) and
 *     the devkit runtime thread (init / periodic log).
 */

#include "devkit_probe_adapter.h"

#include <stdbool.h>

#include "probe_translator.h"

#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(devkit_probe, LOG_LEVEL_INF);

static probe_translator_t s_probe_translator;
static bool               s_initialized;

robotos_core_status_t devkit_probe_adapter_init(void)
{
	robotos_core_status_t st;

	s_initialized = false;

	st = probe_translator_init(&s_probe_translator, NULL);
	if (st != ROBOTOS_CORE_OK) {
		LOG_ERR("ROBOTOS_PROBE init failed: %d", (int)st);
		return st;
	}

	s_initialized = true;
	LOG_INF("ROBOTOS_PROBE init ok");
	return ROBOTOS_CORE_OK;
}

robotos_core_status_t devkit_probe_adapter_dispatch(
	uint32_t adapter_type, uint32_t adapter_arg0)
{
	robotos_core_status_t st;

	if (!s_initialized) {
		LOG_WRN("ROBOTOS_PROBE dispatch skipped: not initialized "
			"(type=%u arg0=%u)",
			(unsigned)adapter_type, (unsigned)adapter_arg0);
		return ROBOTOS_CORE_ERR_INVALID_STATE;
	}

	st = probe_translator_dispatch_adapter_event(
		&s_probe_translator, adapter_type, adapter_arg0, NULL);
	if (st != ROBOTOS_CORE_OK) {
		LOG_WRN("ROBOTOS_PROBE dispatch err type=%u arg0=%u ret=%d",
			(unsigned)adapter_type, (unsigned)adapter_arg0, (int)st);
	}
	return st;
}

void devkit_probe_adapter_log_snapshot(void)
{
	probe_translator_snapshot_t snap;
	robotos_core_status_t st;

	if (!s_initialized) {
		return;
	}

	st = probe_translator_get_snapshot(&s_probe_translator, &snap);
	if (st != ROBOTOS_CORE_OK) {
		LOG_WRN("ROBOTOS_PROBE snapshot err: %d", (int)st);
		return;
	}

	LOG_INF("ROBOTOS_PROBE state=%u trans=%u events=%u no_trans=%u "
		"mapped=%u unmapped=%u",
		(unsigned)snap.fsm.current_state,
		(unsigned)snap.fsm.transition_count,
		(unsigned)snap.fsm.event_count,
		(unsigned)snap.fsm.no_transition_count,
		(unsigned)snap.bridge.mapped_count,
		(unsigned)snap.bridge.unmapped_count);
}
