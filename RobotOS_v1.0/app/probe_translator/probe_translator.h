/*
 * probe_translator.h
 *
 * RobotOS — First application harness (Phase 12I host prototype).
 *
 * STATUS: DRAFT / host prototype. Architecture A only.
 *
 *   - Host-only first application harness implementing the contract locked
 *     by Phase 12I-pre (see PROBE_TRANSLATOR_HOST_PROTOTYPE_PLAN.md).
 *   - No devkit dependency. No UART command surface. No Zephyr binding.
 *   - No hardware runtime entry-point.
 *
 * SPEC ANCHOR:
 *   RobotOS_v1.0/devkit/docs/03_SPECS/PROBE_TRANSLATOR_HOST_PROTOTYPE_PLAN.md
 *
 * BOUNDARIES:
 *   - No #include of devkit_app_state.h or any devkit_*.h.
 *   - No #include of <zephyr/...>, include/robotos/*, or ro_*.h.
 *   - No reference to a/s/r/?/x/v/L/d/T UART command bytes.
 *   - No core event-handler registration; the bridge is application-owned.
 */

#ifndef PROBE_TRANSLATOR_H
#define PROBE_TRANSLATOR_H

#include <stdint.h>

#include "robotos_fw_fsm.h"
#include "robotos_fw_event_bridge.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ---------------------------------------------------------------------------
 * Application-local state IDs (Phase 12I-pre §E.1)
 * --------------------------------------------------------------------------- */
#define PROBE_TRANSLATOR_STATE_IDLE   ((robotos_fw_state_id_t)1u)
#define PROBE_TRANSLATOR_STATE_READY  ((robotos_fw_state_id_t)2u)
#define PROBE_TRANSLATOR_STATE_ACTIVE ((robotos_fw_state_id_t)3u)
#define PROBE_TRANSLATOR_STATE_FAULT  ((robotos_fw_state_id_t)4u)

/* ---------------------------------------------------------------------------
 * Application-local event IDs (Phase 12I-pre §E.2)
 * --------------------------------------------------------------------------- */
#define PROBE_TRANSLATOR_EVT_CONFIGURED ((robotos_fw_event_id_t)1u)
#define PROBE_TRANSLATOR_EVT_START      ((robotos_fw_event_id_t)2u)
#define PROBE_TRANSLATOR_EVT_STOP       ((robotos_fw_event_id_t)3u)
#define PROBE_TRANSLATOR_EVT_RESET      ((robotos_fw_event_id_t)4u)
#define PROBE_TRANSLATOR_EVT_FAULT  ((robotos_fw_event_id_t)5u)

/* ---------------------------------------------------------------------------
 * Adapter key constants (Phase 12I-pre §E.3 / §E.4)
 * --------------------------------------------------------------------------- */
#define PROBE_ADAPTER_TYPE_CONFIG   ((uint32_t)1u)
#define PROBE_ADAPTER_TYPE_COMMAND  ((uint32_t)2u)
#define PROBE_ADAPTER_TYPE_FAULT  ((uint32_t)3u)

#define PROBE_ADAPTER_ARG_NONE      ((uint32_t)0u)
#define PROBE_ADAPTER_ARG_START     ((uint32_t)1u)
#define PROBE_ADAPTER_ARG_STOP      ((uint32_t)2u)
#define PROBE_ADAPTER_ARG_RESET     ((uint32_t)3u)
#define PROBE_ADAPTER_ARG_ANY  ((uint32_t)0xFFFFFFFFu)

/* ---------------------------------------------------------------------------
 * Types (Phase 12I-pre §F)
 *
 * Field layout is DRAFT / EXPERIMENTAL. The struct body is declared here so
 * the caller can statically size the instance; access only through the
 * public API.
 * --------------------------------------------------------------------------- */

typedef struct probe_translator_config {
    void *user_context;  /* may be NULL; passed through to FSM config */
} probe_translator_config_t;

typedef struct {
    robotos_fw_fsm_snapshot_t          fsm;
    robotos_fw_event_bridge_snapshot_t bridge;
} probe_translator_snapshot_t;

/*
 * The FSM and bridge stores its config by pointer (see robotos_fw_fsm.h /
 * robotos_fw_event_bridge.h: "the config object and the arrays it
 * references must outlive the FSM"). To keep the harness self-contained
 * the configs are embedded here so their lifetime matches the harness
 * instance. All fields remain opaque — access only through the public API.
 */
typedef struct probe_translator {
    robotos_fw_fsm_t                 fsm;
    robotos_fw_event_bridge_t        bridge;
    robotos_fw_fsm_config_t          _fsm_cfg;
    robotos_fw_event_bridge_config_t _bridge_cfg;
} probe_translator_t;

/* ---------------------------------------------------------------------------
 * Public API (Phase 12I-pre §G)
 * --------------------------------------------------------------------------- */

/*
 * probe_translator_init
 *
 * Initialize a caller-owned harness instance against module-internal static
 * tables. Wires the embedded FSM (initial_state = STATE_IDLE) and event
 * bridge in one call.
 *
 *   pt      — caller-owned; must outlive any use of the harness.
 *   config  — optional; NULL is allowed (treated as user_context == NULL).
 *
 * Return:
 *   ROBOTOS_CORE_OK            on success.
 *   ROBOTOS_CORE_ERR_NULL      if pt is NULL.
 *   First non-OK from robotos_fw_fsm_init / robotos_fw_event_bridge_init.
 */
robotos_fw_status_t probe_translator_init(
    probe_translator_t              *pt,
    const probe_translator_config_t *config);

/*
 * probe_translator_dispatch_adapter_event
 *
 * Forward a synthetic adapter event tuple through the bridge into the FSM.
 * Returns the bridge's status verbatim. payload is borrowed for the call
 * only and is not cached by the bridge.
 *
 * Return:
 *   ROBOTOS_CORE_OK            on a mapped or unmapped dispatch.
 *   ROBOTOS_CORE_ERR_NULL      if pt is NULL.
 *   Any non-OK status from the bridge / FSM.
 */
robotos_fw_status_t probe_translator_dispatch_adapter_event(
    probe_translator_t *pt,
    uint32_t            adapter_type,
    uint32_t            adapter_arg0,
    const void         *payload);

/*
 * probe_translator_reset
 *
 * Reset the bridge counters first, then the FSM (state -> STATE_IDLE,
 * counters cleared). Returns the first non-OK from either reset call.
 */
robotos_fw_status_t probe_translator_reset(probe_translator_t *pt);

/*
 * probe_translator_get_snapshot
 *
 * Copy FSM and bridge snapshots into out in one call. Returns the first
 * non-OK from either get_snapshot.
 */
robotos_fw_status_t probe_translator_get_snapshot(
    const probe_translator_t    *pt,
    probe_translator_snapshot_t *out);

#ifdef __cplusplus
}
#endif

#endif /* PROBE_TRANSLATOR_H */
