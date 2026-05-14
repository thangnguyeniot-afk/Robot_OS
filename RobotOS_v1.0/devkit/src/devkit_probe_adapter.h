/*
 * devkit_probe_adapter.h
 *
 * RobotOS devkit -- probe_translator runtime adapter (Phase 12L).
 *
 * Owns the single static probe_translator_t instance inside the devkit and
 * exposes a three-function interface that bridges devkit_app_state command
 * signals to probe_translator adapter-key tuples. Devkit-local only.
 *
 * Spec anchor:
 *   RobotOS_v1.0/devkit/docs/03_SPECS/PROBE_TRANSLATOR_RUNTIME_ADMISSION_PLAN.md
 *
 * Boundaries (Phase 12L):
 *   - No #include of probe_translator.h or framework headers from this
 *     header. The adapter's internal state is owned by .c only.
 *   - No UART TX. No hardware driver API. No Zephyr driver API.
 *   - No new robotos_core_register_event_handler() call.
 *   - All entry points are thread-context only (never ISR).
 *   - Adapter is additive observation: dispatch failures are logged and
 *     not propagated to callers (devkit_app_state must not fail a UART
 *     command due to probe_translator state).
 */

#ifndef DEVKIT_PROBE_ADAPTER_H
#define DEVKIT_PROBE_ADAPTER_H

#include <stdint.h>

#include "robotos_core.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Initialize the probe adapter. Calls probe_translator_init() internally
 * against the module-static probe_translator_t instance.
 *
 * Must be called once from devkit_runtime_init() after
 * devkit_app_state_init() and before any dispatch call.
 *
 * Return:
 *   ROBOTOS_CORE_OK         on success; adapter ready for dispatch.
 *   Any non-OK              from probe_translator_init(); adapter is left
 *                           uninitialized and subsequent dispatch calls
 *                           are no-ops that return ROBOTOS_CORE_ERR_INVALID_STATE.
 *
 * Thread-context only.
 */
robotos_core_status_t devkit_probe_adapter_init(void);

/*
 * Dispatch a synthetic adapter event to the probe translator.
 *
 * adapter_type and adapter_arg0 are PROBE_ADAPTER_* constants defined in
 * app/probe_translator/probe_translator.h. The adapter must not interpret
 * them; it forwards them verbatim through the probe_translator public API.
 *
 * Non-OK return is logged at WRN level and returned to the caller; callers
 * (devkit_app_state) must treat the return as advisory and continue.
 *
 * Return:
 *   ROBOTOS_CORE_OK                 on a mapped or unmapped dispatch.
 *   ROBOTOS_CORE_ERR_INVALID_STATE  if the adapter is not initialized.
 *   Any non-OK from probe_translator_dispatch_adapter_event().
 *
 * Thread-context only.
 */
robotos_core_status_t devkit_probe_adapter_dispatch(
    uint32_t adapter_type, uint32_t adapter_arg0);

/*
 * Emit one ROBOTOS_PROBE log line via Zephyr LOG_INF with the
 * probe_translator FSM + bridge snapshot.
 *
 * Stable single-line format (integer fields only):
 *   ROBOTOS_PROBE state=%u trans=%u events=%u no_trans=%u
 *                 mapped=%u unmapped=%u
 *
 * No-op (silent return) if the adapter is not initialized.
 *
 * Thread-context only (Zephyr LOG_INF deferred path; not safe in ISR).
 */
void devkit_probe_adapter_log_snapshot(void);

#ifdef __cplusplus
}
#endif

#endif /* DEVKIT_PROBE_ADAPTER_H */
