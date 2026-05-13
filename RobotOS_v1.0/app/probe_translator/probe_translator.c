/*
 * probe_translator.c
 *
 * RobotOS — First application harness (Phase 12I host prototype).
 *
 * Implements the public API declared in probe_translator.h against the
 * Framework FSM (robotos_fw_fsm) and event bridge (robotos_fw_event_bridge)
 * per the implementation contract in PROBE_TRANSLATOR_HOST_PROTOTYPE_PLAN.md.
 *
 * Architecture A only. No devkit, Zephyr, UART, or legacy ro_* dependency.
 */

#include "probe_translator.h"

#include <stddef.h>  /* NULL */

/* ---------------------------------------------------------------------------
 * Static transition table (5 rows; Phase 12I-pre §H.1)
 *
 * All rows use guard = NULL (always allow) and action = NULL.
 * --------------------------------------------------------------------------- */
static const robotos_fw_transition_t k_probe_translator_transitions[] = {
    /* row 0: IDLE + CONFIGURED -> READY */
    { PROBE_TRANSLATOR_STATE_IDLE,   PROBE_TRANSLATOR_EVT_CONFIGURED,
      NULL, PROBE_TRANSLATOR_STATE_READY,  NULL },
    /* row 1: READY + START -> ACTIVE */
    { PROBE_TRANSLATOR_STATE_READY,  PROBE_TRANSLATOR_EVT_START,
      NULL, PROBE_TRANSLATOR_STATE_ACTIVE, NULL },
    /* row 2: ACTIVE + STOP -> READY */
    { PROBE_TRANSLATOR_STATE_ACTIVE, PROBE_TRANSLATOR_EVT_STOP,
      NULL, PROBE_TRANSLATOR_STATE_READY,  NULL },
    /* row 3: READY + RESET -> IDLE */
    { PROBE_TRANSLATOR_STATE_READY,  PROBE_TRANSLATOR_EVT_RESET,
      NULL, PROBE_TRANSLATOR_STATE_IDLE,   NULL },
    /* row 4: ACTIVE + RESET -> IDLE */
    { PROBE_TRANSLATOR_STATE_ACTIVE, PROBE_TRANSLATOR_EVT_RESET,
      NULL, PROBE_TRANSLATOR_STATE_IDLE,   NULL },
};

/* ---------------------------------------------------------------------------
 * Static state definitions (3 entries; on_entry/on_exit NULL at Phase 12I)
 * --------------------------------------------------------------------------- */
static const robotos_fw_state_def_t k_probe_translator_state_defs[] = {
    { PROBE_TRANSLATOR_STATE_IDLE,   NULL, NULL },
    { PROBE_TRANSLATOR_STATE_READY,  NULL, NULL },
    { PROBE_TRANSLATOR_STATE_ACTIVE, NULL, NULL },
};

/* ---------------------------------------------------------------------------
 * Static mapping table (4 rows; Phase 12I-pre §H.1)
 *
 * All rows use exact arg0 match (match_arg0 = true). No wildcard row at
 * Phase 12I (FAULT block deferred).
 * --------------------------------------------------------------------------- */
static const robotos_fw_event_bridge_row_t k_probe_translator_mapping[] = {
    /* row 0: (TYPE_CONFIG, ARG_NONE)  -> EVT_CONFIGURED */
    { PROBE_ADAPTER_TYPE_CONFIG,  PROBE_ADAPTER_ARG_NONE,  true,
      PROBE_TRANSLATOR_EVT_CONFIGURED },
    /* row 1: (TYPE_COMMAND, ARG_START) -> EVT_START */
    { PROBE_ADAPTER_TYPE_COMMAND, PROBE_ADAPTER_ARG_START, true,
      PROBE_TRANSLATOR_EVT_START },
    /* row 2: (TYPE_COMMAND, ARG_STOP)  -> EVT_STOP */
    { PROBE_ADAPTER_TYPE_COMMAND, PROBE_ADAPTER_ARG_STOP,  true,
      PROBE_TRANSLATOR_EVT_STOP },
    /* row 3: (TYPE_COMMAND, ARG_RESET) -> EVT_RESET */
    { PROBE_ADAPTER_TYPE_COMMAND, PROBE_ADAPTER_ARG_RESET, true,
      PROBE_TRANSLATOR_EVT_RESET },
};

/* ---------------------------------------------------------------------------
 * Public API
 * --------------------------------------------------------------------------- */

robotos_fw_status_t probe_translator_init(
    probe_translator_t              *pt,
    const probe_translator_config_t *config)
{
    robotos_fw_status_t st;
    void *user_context;

    if (pt == NULL) {
        return ROBOTOS_CORE_ERR_NULL;
    }

    user_context = (config != NULL) ? config->user_context : NULL;

    pt->_fsm_cfg.transitions      = k_probe_translator_transitions;
    pt->_fsm_cfg.transition_count = 5u;
    pt->_fsm_cfg.states           = k_probe_translator_state_defs;
    pt->_fsm_cfg.state_count      = 3u;
    pt->_fsm_cfg.initial_state    = PROBE_TRANSLATOR_STATE_IDLE;
    pt->_fsm_cfg.user_context     = user_context;

    st = robotos_fw_fsm_init(&pt->fsm, &pt->_fsm_cfg);
    if (st != ROBOTOS_CORE_OK) {
        return st;
    }

    pt->_bridge_cfg.rows         = k_probe_translator_mapping;
    pt->_bridge_cfg.row_count    = 4u;
    pt->_bridge_cfg.fsm          = &pt->fsm;
    pt->_bridge_cfg.user_context = user_context;

    return robotos_fw_event_bridge_init(&pt->bridge, &pt->_bridge_cfg);
}

robotos_fw_status_t probe_translator_dispatch_adapter_event(
    probe_translator_t *pt,
    uint32_t            adapter_type,
    uint32_t            adapter_arg0,
    const void         *payload)
{
    if (pt == NULL) {
        return ROBOTOS_CORE_ERR_NULL;
    }
    return robotos_fw_event_bridge_dispatch(
        &pt->bridge, adapter_type, adapter_arg0, payload);
}

robotos_fw_status_t probe_translator_reset(probe_translator_t *pt)
{
    robotos_fw_status_t st;

    if (pt == NULL) {
        return ROBOTOS_CORE_ERR_NULL;
    }

    st = robotos_fw_event_bridge_reset(&pt->bridge);
    if (st != ROBOTOS_CORE_OK) {
        return st;
    }
    return robotos_fw_fsm_reset(&pt->fsm);
}

robotos_fw_status_t probe_translator_get_snapshot(
    const probe_translator_t    *pt,
    probe_translator_snapshot_t *out)
{
    robotos_fw_status_t st;

    if (pt == NULL || out == NULL) {
        return ROBOTOS_CORE_ERR_NULL;
    }

    st = robotos_fw_fsm_get_snapshot(&pt->fsm, &out->fsm);
    if (st != ROBOTOS_CORE_OK) {
        return st;
    }
    return robotos_fw_event_bridge_get_snapshot(&pt->bridge, &out->bridge);
}
