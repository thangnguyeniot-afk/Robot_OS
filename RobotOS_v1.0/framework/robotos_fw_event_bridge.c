/*
 * robotos_fw_event_bridge.c
 *
 * RobotOS Robot Framework — Application Event Bridge implementation
 * (Phase 12F host prototype).
 *
 * STATUS: First implementation of the Phase 12F-pre / Phase 12F bridge
 * contract. Behavior is host-test validated only. No devkit integration.
 * No hardware evidence. No UART command surface. ABI is still
 * DRAFT / EXPERIMENTAL.
 *
 * Architecture A only:
 *   - Includes "robotos_fw_event_bridge.h" (which transitively pulls in
 *     robotos_fw_fsm.h and, indirectly, robotos_core.h for status codes).
 *   - No Zephyr. No devkit. No legacy ro_* HAL.
 *   - No heap. No threads. No timers. No work items. No global mutable
 *     singleton state; everything lives in the caller-owned
 *     robotos_fw_event_bridge_t.
 *   - No critical section is taken by this module (Phase 12F-pre §11 #8:
 *     thread-context only). robotos_fw_fsm_dispatch may take one inside
 *     the FSM, but the bridge does not add one.
 *
 * Behavioral contract (encoded here):
 *   - Bridge stores config by pointer; mapping rows are never copied.
 *   - Bridge stores NO payload pointer beyond the duration of one
 *     dispatch call (enforced by NOT having a payload field in the
 *     instance struct).
 *   - Bridge does NOT register with robotos_core_register_event_handler.
 *   - Bridge does NOT call any UART, GPIO, sensor, or driver API.
 *   - Bridge does NOT touch devkit_app_state symbols.
 *
 * Dispatch flow (FIFO first-match):
 *   event_count++
 *   last_adapter_type = adapter_type
 *   last_adapter_arg0 = adapter_arg0
 *   for each row in config->rows[0 .. row_count):
 *     if row.adapter_type != adapter_type:                continue
 *     if row.match_arg0 && row.adapter_arg0 != adapter_arg0: continue
 *     -> mapped_count++
 *     -> last_fw_event_id = row.fw_event_id
 *     -> last_status     = robotos_fw_fsm_dispatch(fsm, row.fw_event_id, payload)
 *     -> return last_status
 *   unmapped_count++
 *   last_status = ROBOTOS_CORE_OK
 *   return ROBOTOS_CORE_OK
 *
 * Status mapping (Phase 12C REUSE_ROBOTOS_CORE_STATUS_T):
 *   ROBOTOS_CORE_OK                 success, no-row, or FSM-OK
 *   ROBOTOS_CORE_ERR_NULL           bridge / config / rows / fsm NULL on init,
 *                                   or bridge NULL on dispatch / reset / snapshot
 *   ROBOTOS_CORE_ERR_INVALID_ARG    row_count == 0 on init
 *   ROBOTOS_CORE_ERR_INVALID_STATE  bridge not initialized on dispatch /
 *                                   reset / snapshot
 */

#include "robotos_fw_event_bridge.h"

#include <stddef.h>

/* ---------------------------------------------------------------------------
 * Internal helpers
 * ---------------------------------------------------------------------------
 */

/* Zero all counter and last_* fields on *bridge. Leaves config and
 * initialized untouched. Call only with bridge != NULL. */
static void bridge_reset_counters(robotos_fw_event_bridge_t *bridge)
{
    bridge->event_count       = 0u;
    bridge->mapped_count      = 0u;
    bridge->unmapped_count    = 0u;
    bridge->last_adapter_type = 0u;
    bridge->last_adapter_arg0 = 0u;
    bridge->last_fw_event_id  = 0u;
    bridge->last_status       = ROBOTOS_CORE_OK;
}

/* ---------------------------------------------------------------------------
 * Public API
 * ---------------------------------------------------------------------------
 */

robotos_fw_status_t robotos_fw_event_bridge_init(
    robotos_fw_event_bridge_t              *bridge,
    const robotos_fw_event_bridge_config_t *config)
{
    if (bridge == NULL || config == NULL ||
        config->rows == NULL || config->fsm == NULL) {
        return ROBOTOS_CORE_ERR_NULL;
    }
    if (config->row_count == 0u) {
        return ROBOTOS_CORE_ERR_INVALID_ARG;
    }

    bridge->config = config;
    bridge_reset_counters(bridge);
    bridge->initialized = true;

    return ROBOTOS_CORE_OK;
}

robotos_fw_status_t robotos_fw_event_bridge_dispatch(
    robotos_fw_event_bridge_t *bridge,
    uint32_t                   adapter_type,
    uint32_t                   adapter_arg0,
    const void                *payload)
{
    if (bridge == NULL) {
        return ROBOTOS_CORE_ERR_NULL;
    }
    if (!bridge->initialized) {
        return ROBOTOS_CORE_ERR_INVALID_STATE;
    }

    const robotos_fw_event_bridge_config_t *cfg = bridge->config;

    bridge->event_count++;
    bridge->last_adapter_type = adapter_type;
    bridge->last_adapter_arg0 = adapter_arg0;

    for (uint32_t i = 0u; i < cfg->row_count; i++) {
        const robotos_fw_event_bridge_row_t *row = &cfg->rows[i];

        if (row->adapter_type != adapter_type) {
            continue;
        }
        if (row->match_arg0 && row->adapter_arg0 != adapter_arg0) {
            continue;
        }

        /* Mapped: invoke the FSM. Payload is passed through borrowed and
         * not retained by the bridge after this call returns. */
        bridge->mapped_count++;
        bridge->last_fw_event_id = row->fw_event_id;

        robotos_fw_status_t fsm_status =
            robotos_fw_fsm_dispatch(cfg->fsm, row->fw_event_id, payload);

        bridge->last_status = fsm_status;
        return fsm_status;
    }

    /* Unmapped: silent OK + counter. FSM is NOT called. */
    bridge->unmapped_count++;
    bridge->last_status = ROBOTOS_CORE_OK;
    return ROBOTOS_CORE_OK;
}

robotos_fw_status_t robotos_fw_event_bridge_reset(
    robotos_fw_event_bridge_t *bridge)
{
    if (bridge == NULL) {
        return ROBOTOS_CORE_ERR_NULL;
    }
    if (!bridge->initialized) {
        return ROBOTOS_CORE_ERR_INVALID_STATE;
    }

    /* Resets bridge counters only. FSM state is untouched (spec §5.2). */
    bridge_reset_counters(bridge);
    return ROBOTOS_CORE_OK;
}

robotos_fw_status_t robotos_fw_event_bridge_get_snapshot(
    const robotos_fw_event_bridge_t    *bridge,
    robotos_fw_event_bridge_snapshot_t *out)
{
    if (bridge == NULL || out == NULL) {
        return ROBOTOS_CORE_ERR_NULL;
    }
    if (!bridge->initialized) {
        return ROBOTOS_CORE_ERR_INVALID_STATE;
    }

    out->event_count       = bridge->event_count;
    out->mapped_count      = bridge->mapped_count;
    out->unmapped_count    = bridge->unmapped_count;
    out->last_adapter_type = bridge->last_adapter_type;
    out->last_adapter_arg0 = bridge->last_adapter_arg0;
    out->last_fw_event_id  = bridge->last_fw_event_id;
    out->last_status       = bridge->last_status;
    out->initialized       = bridge->initialized;

    return ROBOTOS_CORE_OK;
}
