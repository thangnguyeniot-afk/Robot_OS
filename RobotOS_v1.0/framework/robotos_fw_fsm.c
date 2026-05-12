/*
 * robotos_fw_fsm.c
 *
 * RobotOS Robot Framework — Flat FSM implementation (Phase 12E).
 *
 * STATUS: First implementation behind the Phase 12D-LOCKED header. Behavior
 * is host-test validated only. No devkit integration. No hardware evidence.
 * ABI is still DRAFT / EXPERIMENTAL; the public surface matches the
 * declarations in robotos_fw_fsm.h.
 *
 * Architecture A only:
 *   - Includes "robotos_fw_fsm.h" (the Phase 12D header).
 *   - Includes "robotos_platform_critical.h" for ISR-safe state queries.
 *   - No Zephyr. No devkit. No legacy ro_* HAL.
 *   - No heap. No threads. No timers. No work items. No global singleton
 *     mutable state; everything lives in the caller-owned robotos_fw_fsm_t.
 *
 * Phase 12C behavioral contract (encoded here):
 *   - Flat FSM; product-defined uint32 state and event IDs.
 *   - First-match FIFO scan of the static transition table.
 *   - Guard returns bool. False = skip row; scan continues.
 *   - Evaluation order on a committed transition:
 *         on_exit(from)
 *      -> current_state = next_state    (state update happens BEFORE action)
 *      -> action(from, to, ev, payload, ctx)
 *      -> on_entry(to)
 *   - Action non-OK does NOT roll back state. transition_count++, entry
 *     still runs, last_status records non-OK, dispatch() returns the
 *     action's non-OK value.
 *   - on_exit and on_entry return statuses are observed by the FSM but
 *     are NOT propagated by dispatch(); only the action's return value
 *     determines dispatch's return when an action is present. The Phase
 *     12C spec is silent on entry/exit non-OK; this is the Phase 12E
 *     pragmatic choice and is recorded in the closeout.
 *   - Payload is borrowed for the duration of dispatch only; FSM never
 *     stores it after dispatch returns. This is enforced by NOT having
 *     a payload field in robotos_fw_fsm_t.
 *
 * Status mapping (Phase 12C REUSE_ROBOTOS_CORE_STATUS_T):
 *   ROBOTOS_CORE_OK                 success or no-transition dispatch
 *   ROBOTOS_CORE_ERR_NULL           fsm or config or required pointer is NULL
 *   ROBOTOS_CORE_ERR_INVALID_ARG    transition_count==0 or initial_state==UNINIT
 *   ROBOTOS_CORE_ERR_INVALID_STATE  FSM not initialized
 *
 * Re-init policy: idempotent. A second init() on an already-initialized
 * FSM zeroes all counters and re-runs on_entry(initial_state) if the state
 * def has one. Documented in closeout.
 */

#include "robotos_fw_fsm.h"

#include "robotos_platform_critical.h"

#include <stddef.h>

/* ---------------------------------------------------------------------------
 * Internal helpers
 * ---------------------------------------------------------------------------
 */

/* Locate the state def for state_id within config->states. NULL if not found
 * or if config->states is NULL or config->state_count is zero. */
static const robotos_fw_state_def_t *fw_find_state_def(
    const robotos_fw_fsm_config_t *config,
    robotos_fw_state_id_t          state_id)
{
    if (config == NULL || config->states == NULL) {
        return NULL;
    }
    for (uint32_t i = 0u; i < config->state_count; i++) {
        if (config->states[i].state_id == state_id) {
            return &config->states[i];
        }
    }
    return NULL;
}

/* Zero all counters and runtime fields on *fsm. Leaves config and
 * initialized untouched. Call only with fsm != NULL. */
static void fw_reset_counters(robotos_fw_fsm_t *fsm)
{
    fsm->transition_count     = 0u;
    fsm->event_count          = 0u;
    fsm->guard_rejected_count = 0u;
    fsm->no_transition_count  = 0u;
    fsm->last_event_id        = 0u;
    fsm->last_status          = ROBOTOS_CORE_OK;
}

/* ---------------------------------------------------------------------------
 * Public API — Phase 12D LOCKED, Phase 12E implemented
 * ---------------------------------------------------------------------------
 */

robotos_fw_status_t robotos_fw_fsm_init(
    robotos_fw_fsm_t              *fsm,
    const robotos_fw_fsm_config_t *config)
{
    if (fsm == NULL || config == NULL || config->transitions == NULL) {
        return ROBOTOS_CORE_ERR_NULL;
    }
    if (config->transition_count == 0u ||
        config->initial_state == ROBOTOS_FW_STATE_UNINIT) {
        return ROBOTOS_CORE_ERR_INVALID_ARG;
    }

    fsm->config        = config;
    fsm->current_state = config->initial_state;
    fw_reset_counters(fsm);
    fsm->initialized   = true;

    const robotos_fw_state_def_t *def =
        fw_find_state_def(config, config->initial_state);
    if (def != NULL && def->on_entry != NULL) {
        (void)def->on_entry(config->initial_state, config->user_context);
    }

    return ROBOTOS_CORE_OK;
}

robotos_fw_status_t robotos_fw_fsm_dispatch(
    robotos_fw_fsm_t      *fsm,
    robotos_fw_event_id_t  event_id,
    const void            *event_payload)
{
    if (fsm == NULL) {
        return ROBOTOS_CORE_ERR_NULL;
    }
    if (!fsm->initialized) {
        return ROBOTOS_CORE_ERR_INVALID_STATE;
    }

    fsm->event_count++;
    fsm->last_event_id = event_id;

    const robotos_fw_fsm_config_t *cfg = fsm->config;
    const robotos_fw_state_id_t    from = fsm->current_state;
    bool any_matching_row = false;

    for (uint32_t i = 0u; i < cfg->transition_count; i++) {
        const robotos_fw_transition_t *row = &cfg->transitions[i];

        if (row->current_state != from || row->event_id != event_id) {
            continue;
        }
        any_matching_row = true;

        if (row->guard != NULL &&
            !row->guard(from, event_id, event_payload, cfg->user_context)) {
            fsm->guard_rejected_count++;
            continue;
        }

        /* Committed transition: exit -> state update -> action -> entry. */
        const robotos_fw_state_def_t *def_from = fw_find_state_def(cfg, from);
        if (def_from != NULL && def_from->on_exit != NULL) {
            (void)def_from->on_exit(from, cfg->user_context);
        }

        const robotos_fw_state_id_t to = row->next_state;
        fsm->current_state = to;

        robotos_fw_status_t action_status = ROBOTOS_CORE_OK;
        if (row->action != NULL) {
            action_status = row->action(from, to, event_id,
                                        event_payload, cfg->user_context);
        }

        const robotos_fw_state_def_t *def_to = fw_find_state_def(cfg, to);
        if (def_to != NULL && def_to->on_entry != NULL) {
            (void)def_to->on_entry(to, cfg->user_context);
        }

        fsm->transition_count++;
        fsm->last_status = action_status;
        return action_status;
    }

    /* No row committed: either no match at all, or all matching rows had
     * rejecting guards. Counter semantics are independent (Phase 12C). */
    (void)any_matching_row;
    fsm->no_transition_count++;
    fsm->last_status = ROBOTOS_CORE_OK;
    return ROBOTOS_CORE_OK;
}

robotos_fw_state_id_t robotos_fw_fsm_get_state(const robotos_fw_fsm_t *fsm)
{
    if (fsm == NULL) {
        return ROBOTOS_FW_STATE_UNINIT;
    }
    robotos_platform_critical_token_t tok = robotos_platform_critical_enter();
    robotos_fw_state_id_t result =
        fsm->initialized ? fsm->current_state : ROBOTOS_FW_STATE_UNINIT;
    robotos_platform_critical_exit(tok);
    return result;
}

robotos_fw_status_t robotos_fw_fsm_reset(robotos_fw_fsm_t *fsm)
{
    if (fsm == NULL) {
        return ROBOTOS_CORE_ERR_NULL;
    }
    if (!fsm->initialized) {
        return ROBOTOS_CORE_ERR_INVALID_STATE;
    }

    const robotos_fw_fsm_config_t *cfg = fsm->config;
    const robotos_fw_state_id_t    prev = fsm->current_state;

    const robotos_fw_state_def_t *def_prev = fw_find_state_def(cfg, prev);
    if (def_prev != NULL && def_prev->on_exit != NULL) {
        (void)def_prev->on_exit(prev, cfg->user_context);
    }

    fsm->current_state = cfg->initial_state;
    fw_reset_counters(fsm);

    const robotos_fw_state_def_t *def_init =
        fw_find_state_def(cfg, cfg->initial_state);
    if (def_init != NULL && def_init->on_entry != NULL) {
        (void)def_init->on_entry(cfg->initial_state, cfg->user_context);
    }

    return ROBOTOS_CORE_OK;
}

bool robotos_fw_fsm_is_in_state(
    const robotos_fw_fsm_t *fsm,
    robotos_fw_state_id_t   state_id)
{
    if (fsm == NULL) {
        return false;
    }
    robotos_platform_critical_token_t tok = robotos_platform_critical_enter();
    bool result = fsm->initialized && fsm->current_state == state_id;
    robotos_platform_critical_exit(tok);
    return result;
}

robotos_fw_status_t robotos_fw_fsm_get_snapshot(
    const robotos_fw_fsm_t    *fsm,
    robotos_fw_fsm_snapshot_t *out)
{
    if (fsm == NULL || out == NULL) {
        return ROBOTOS_CORE_ERR_NULL;
    }
    if (!fsm->initialized) {
        return ROBOTOS_CORE_ERR_INVALID_STATE;
    }

    robotos_platform_critical_token_t tok = robotos_platform_critical_enter();
    out->current_state        = fsm->current_state;
    out->transition_count     = fsm->transition_count;
    out->event_count          = fsm->event_count;
    out->guard_rejected_count = fsm->guard_rejected_count;
    out->no_transition_count  = fsm->no_transition_count;
    out->last_event_id        = fsm->last_event_id;
    out->last_status          = fsm->last_status;
    out->initialized          = fsm->initialized;
    robotos_platform_critical_exit(tok);

    return ROBOTOS_CORE_OK;
}
