/*
 * robotos_fw_fsm.h
 *
 * RobotOS Robot Framework — Flat FSM API (Phase 12D header stub).
 *
 * STATUS: DRAFT / EXPERIMENTAL.
 *
 *   - Phase 12D introduces this header as a HEADER STUB ONLY.
 *   - No implementation (.c body) exists at Phase 12D.
 *   - ABI is NOT stable. Function signatures and type layouts may change
 *     before an implementation phase (Phase 12E or later) is authorized.
 *   - This header is the LOCKED-AT-12D surface for the Phase 12B/12C
 *     confirmed flat-FSM contract; it freezes names and parameter shapes,
 *     but not memory layout or behavioral guarantees beyond those documented
 *     in the spec.
 *
 * SPEC ANCHOR:
 *   RobotOS_v1.0/devkit/docs/03_SPECS/FRAMEWORK_FSM_API_DRAFT.md
 *
 * MODEL (Phase 12B/12C confirmed):
 *   - Flat FSM only. No hierarchy, no history, no concurrent regions.
 *   - Table-driven; transitions and state defs are caller-owned static arrays.
 *   - No heap. FSM instance is a caller-owned struct (statically allocated by
 *     the application or test).
 *   - Product-defined state IDs (uint32_t); 0 is reserved as
 *     ROBOTOS_FW_STATE_UNINIT sentinel.
 *   - Product-defined logical event IDs (uint32_t); SEPARATE NAMESPACE from
 *     robotos_event_type_t. No ROBOTOS_EVENT_USER sub-range is allocated for
 *     the FSM core.
 *   - Event bridge is APPLICATION-OWNED. The FSM does NOT register with
 *     robotos_core_register_event_handler(). The application translates
 *     Adapter events (robotos_event_type_t) to robotos_fw_event_id_t and
 *     calls robotos_fw_fsm_dispatch() in thread/dispatch context (never ISR).
 *   - Payload pointer is BORROWED for the duration of dispatch() only;
 *     callbacks must not cache it. Application owns the memory.
 *   - Status type is an alias of robotos_core_status_t (Phase 12C
 *     REUSE_ROBOTOS_CORE_STATUS_T_FOR_PHASE_12D_CONFIRMED).
 *   - Guards return bool only (true = allow, false = skip row).
 *   - Action non-OK return does NOT roll back the transition. State is
 *     already committed before the action runs; entry still executes;
 *     dispatch() returns the action status. (Phase 12C
 *     ACTION_NON_OK_NO_ROLLBACK_CONFIRMED.)
 *   - Evaluation order (Phase 12C):
 *         exit  ->  state_update  ->  action  ->  entry
 *     state_update happens BEFORE action so the action runs in the
 *     post-transition state context.
 *
 * BOUNDARIES:
 *   - No UART TX from FSM or any callback.
 *   - No hardware driver ownership (no GPIO/PWM/I2C/SPI/ADC).
 *   - No core event-handler registration (the application bridges).
 *   - No Zephyr header dependency.
 *   - No devkit dependency.
 *   - No legacy ro_* dependency (this header is Architecture A only).
 *
 * NOT INCLUDED IN PHASE 12D:
 *   - No function bodies.
 *   - No CMake integration.
 *   - No devkit consumer.
 *   - No application/product layer.
 *   - No hardware evidence.
 */

#ifndef ROBOTOS_FW_FSM_H
#define ROBOTOS_FW_FSM_H

#include <stdbool.h>
#include <stdint.h>

#include "robotos_core.h"   /* robotos_core_status_t — Phase 12C status reuse */

#ifdef __cplusplus
extern "C" {
#endif

/* ---------------------------------------------------------------------------
 * Identifier types (Phase 12B confirmed)
 * ---------------------------------------------------------------------------
 *
 * State IDs and event IDs are unsigned 32-bit integers. Both are
 * application-defined. The FSM imposes no product vocabulary.
 *
 * robotos_fw_event_id_t is a SEPARATE NAMESPACE from robotos_event_type_t
 * (defined in robotos_core.h). The application bridge translates Adapter
 * events to logical Framework event IDs.
 */
typedef uint32_t robotos_fw_state_id_t;
typedef uint32_t robotos_fw_event_id_t;

/* Reserved sentinel: not a valid product state. Used to flag uninitialized
 * FSM instances and to signal "no current state" from get_state() when the
 * FSM pointer is NULL or the instance is not initialized. */
#define ROBOTOS_FW_STATE_UNINIT  ((robotos_fw_state_id_t)0u)

/* ---------------------------------------------------------------------------
 * Status type (Phase 12C confirmed: REUSE_ROBOTOS_CORE_STATUS_T)
 * ---------------------------------------------------------------------------
 *
 * Framework status is an alias of robotos_core_status_t. No new Framework
 * status enum is introduced in Phase 12D. All return codes are interpreted
 * against the robotos_core_status_t value set in robotos_core.h.
 */
typedef robotos_core_status_t robotos_fw_status_t;

/* ---------------------------------------------------------------------------
 * Callback types (Phase 12C confirmed shapes)
 * ---------------------------------------------------------------------------
 *
 * All callbacks run in THREAD/DISPATCH context (never ISR). Callbacks must
 * not call UART TX, must not block on I/O, and must not allocate heap.
 * The event_payload pointer is BORROWED for the duration of dispatch only
 * and must not be cached by any callback.
 */

/* Guard predicate. Returns true to allow the transition, false to skip the
 * row and continue scanning. Pure predicate: no side effects, no mutation
 * of user_context. (Phase 12C GUARD_RETURNS_BOOL_ONLY_CONFIRMED.) */
typedef bool (*robotos_fw_guard_fn_t)(
    robotos_fw_state_id_t current_state,
    robotos_fw_event_id_t event_id,
    const void           *event_payload,
    void                 *user_context);

/* Transition action. Runs AFTER the state update (Phase 12C order). May
 * mutate user_context. Non-OK return does NOT roll back the transition. */
typedef robotos_fw_status_t (*robotos_fw_action_fn_t)(
    robotos_fw_state_id_t from_state,
    robotos_fw_state_id_t to_state,
    robotos_fw_event_id_t event_id,
    const void           *event_payload,
    void                 *user_context);

/* Per-state entry / exit callback.
 *   - on_exit  runs BEFORE state update and BEFORE action; receives the
 *              outgoing state ID as state_id.
 *   - on_entry runs AFTER state update and AFTER action; receives the
 *              incoming state ID as state_id. Entry runs regardless of
 *              the action's status.
 */
typedef robotos_fw_status_t (*robotos_fw_entry_exit_fn_t)(
    robotos_fw_state_id_t state_id,
    void                 *user_context);

/* ---------------------------------------------------------------------------
 * Static config rows (caller-owned; typically declared as const arrays)
 * ---------------------------------------------------------------------------
 */

/* One row of the transition table. First-match FIFO: the order of rows in
 * the array determines priority when multiple rows match. NULL guard means
 * "always allow". NULL action means "no action callback for this row". */
typedef struct {
    robotos_fw_state_id_t  current_state;
    robotos_fw_event_id_t  event_id;
    robotos_fw_guard_fn_t  guard;        /* NULL = always allow */
    robotos_fw_state_id_t  next_state;
    robotos_fw_action_fn_t action;       /* NULL = no action */
} robotos_fw_transition_t;

/* One per-state definition. Both callbacks are optional (NULL = none). */
typedef struct {
    robotos_fw_state_id_t       state_id;
    robotos_fw_entry_exit_fn_t  on_entry; /* NULL = no entry action */
    robotos_fw_entry_exit_fn_t  on_exit;  /* NULL = no exit action */
} robotos_fw_state_def_t;

/* FSM configuration. Pointed to by the FSM instance for its lifetime; the
 * config object and the arrays it references must outlive the FSM. */
typedef struct {
    const robotos_fw_transition_t *transitions;
    uint32_t                       transition_count;
    const robotos_fw_state_def_t  *states;          /* NULL if no entry/exit */
    uint32_t                       state_count;
    robotos_fw_state_id_t          initial_state;
    void                          *user_context;    /* opaque; passed to callbacks */
} robotos_fw_fsm_config_t;

/* ---------------------------------------------------------------------------
 * Snapshot (audit / introspection)
 * ---------------------------------------------------------------------------
 *
 * Returned by robotos_fw_fsm_get_snapshot(). Reflects the FSM state at the
 * call site under critical section. Counter semantics (Phase 12C):
 *
 *   transition_count       committed transitions only (at most one per dispatch)
 *   event_count            calls to dispatch() (one per call)
 *   guard_rejected_count   matching rows whose guard returned false (may
 *                          increment multiple times per dispatch)
 *   no_transition_count    dispatches that ended without a committed
 *                          transition (no matching row OR all guards rejected;
 *                          INDEPENDENT of guard_rejected_count)
 *   last_event_id          event ID of the most recent dispatch
 *   last_status            return status of the most recent dispatch
 *   initialized            true after a successful init() until reset/destroy
 */
typedef struct {
    robotos_fw_state_id_t  current_state;
    uint32_t               transition_count;
    uint32_t               event_count;
    uint32_t               guard_rejected_count;
    uint32_t               no_transition_count;
    robotos_fw_event_id_t  last_event_id;
    robotos_fw_status_t    last_status;
    bool                   initialized;
} robotos_fw_fsm_snapshot_t;

/* ---------------------------------------------------------------------------
 * FSM instance (caller-owned; static allocation expected)
 * ---------------------------------------------------------------------------
 *
 * The application declares a robotos_fw_fsm_t object (statically) and passes
 * its address to init/dispatch/reset/get_*. The Framework does NOT allocate
 * this struct. The Framework does NOT take ownership of the config pointer
 * beyond storing it; the config and the arrays it references must outlive
 * the FSM.
 *
 * Field layout is DRAFT / EXPERIMENTAL. Treat all fields as opaque and
 * access them only through the public API. The struct is declared in the
 * header so the application can statically size it; this is NOT a license
 * to read or write fields directly.
 */
typedef struct robotos_fw_fsm {
    const robotos_fw_fsm_config_t *config;
    robotos_fw_state_id_t          current_state;
    uint32_t                       transition_count;
    uint32_t                       event_count;
    uint32_t                       guard_rejected_count;
    uint32_t                       no_transition_count;
    robotos_fw_event_id_t          last_event_id;
    robotos_fw_status_t            last_status;
    bool                           initialized;
} robotos_fw_fsm_t;

/* ---------------------------------------------------------------------------
 * Public API (Phase 12D LOCKED-AT-12D; bodies are NOT defined in Phase 12D)
 * ---------------------------------------------------------------------------
 *
 * All functions are declared here. No function body exists in Phase 12D.
 * Linking against this header without a Phase 12E (or later) implementation
 * will fail at link time — by design.
 */

/*
 * robotos_fw_fsm_init
 *
 * Purpose:
 *   Initialize a caller-owned FSM instance against a caller-owned config.
 *   Sets current_state to config->initial_state, zeroes all counters, and
 *   marks the instance initialized. If a state def for initial_state has
 *   on_entry, it is called once during init.
 *
 * Context:
 *   Thread context only. Must NOT be called from ISR.
 *
 * Side effects:
 *   Writes *fsm. Does NOT mutate *config. Does NOT register with core.
 *
 * Return:
 *   ROBOTOS_CORE_OK                 on success.
 *   ROBOTOS_CORE_ERR_NULL           if fsm, config, or config->transitions is NULL.
 *   ROBOTOS_CORE_ERR_INVALID_ARG    if config->transition_count == 0 or
 *                                   config->initial_state == ROBOTOS_FW_STATE_UNINIT.
 *
 * Phase 12D: NO IMPLEMENTATION. Declaration only.
 */
robotos_fw_status_t robotos_fw_fsm_init(
    robotos_fw_fsm_t              *fsm,
    const robotos_fw_fsm_config_t *config);

/*
 * robotos_fw_fsm_dispatch
 *
 * Purpose:
 *   Deliver a logical event to the FSM. Scans the transition table FIFO
 *   for the first row whose current_state and event_id match and whose
 *   guard (if any) returns true. On a match the FSM runs:
 *       on_exit(current_state)       (if state def exists)
 *       current_state = next_state   (STATE UPDATE)
 *       action(from, to, event, payload, ctx)   (if non-NULL)
 *       on_entry(next_state)         (if state def exists)
 *   Counters update per the rules in robotos_fw_fsm_snapshot_t.
 *
 * Context:
 *   Thread / dispatch context only. Must NOT be called from ISR.
 *   The application bridge is responsible for marshalling Adapter events
 *   from ISR context into thread context before calling dispatch.
 *
 * Payload lifetime:
 *   event_payload is BORROWED for the duration of this call. The FSM and
 *   all callbacks must not cache or dereference it after dispatch returns.
 *
 * Side effects:
 *   Mutates *fsm (counters, current_state, last_event_id, last_status).
 *   May call any user-provided guard/action/entry/exit callback.
 *
 * Return:
 *   ROBOTOS_CORE_OK                 on a committed transition with OK action,
 *                                   or on a no-transition dispatch (no row
 *                                   matched, or all matching rows had
 *                                   rejecting guards).
 *   Non-OK action status            on a committed transition whose action
 *                                   returned non-OK (state IS committed; no
 *                                   rollback; entry still runs).
 *   ROBOTOS_CORE_ERR_NULL           if fsm is NULL.
 *   ROBOTOS_CORE_ERR_INVALID_STATE  if the FSM is not initialized.
 *
 * Phase 12D: NO IMPLEMENTATION. Declaration only.
 */
robotos_fw_status_t robotos_fw_fsm_dispatch(
    robotos_fw_fsm_t       *fsm,
    robotos_fw_event_id_t   event_id,
    const void             *event_payload);

/*
 * robotos_fw_fsm_get_state
 *
 * Purpose:
 *   Return the current state ID.
 *
 * Context:
 *   ISR-safe. Implementation is expected to take a brief critical section
 *   so that a concurrent dispatch() does not produce a torn read.
 *
 * Return:
 *   The current state ID on success.
 *   ROBOTOS_FW_STATE_UNINIT if fsm is NULL or the instance is not initialized.
 *
 * Phase 12D: NO IMPLEMENTATION. Declaration only.
 */
robotos_fw_state_id_t robotos_fw_fsm_get_state(
    const robotos_fw_fsm_t *fsm);

/*
 * robotos_fw_fsm_reset
 *
 * Purpose:
 *   Reset an initialized FSM to its initial state. Zeroes all counters.
 *   If state defs exist, calls on_exit(current_state) BEFORE the state
 *   update and on_entry(initial_state) AFTER. No transition action runs
 *   during reset; reset is not a transition.
 *
 * Context:
 *   Thread context only. Must NOT be called from ISR.
 *
 * Return:
 *   ROBOTOS_CORE_OK                 on success.
 *   ROBOTOS_CORE_ERR_NULL           if fsm is NULL.
 *   ROBOTOS_CORE_ERR_INVALID_STATE  if the FSM is not initialized.
 *
 * Phase 12D: NO IMPLEMENTATION. Declaration only.
 */
robotos_fw_status_t robotos_fw_fsm_reset(
    robotos_fw_fsm_t *fsm);

/*
 * robotos_fw_fsm_is_in_state
 *
 * Purpose:
 *   Predicate: true if the current state equals state_id.
 *
 * Context:
 *   ISR-safe. Implementation is expected to take a brief critical section.
 *
 * Return:
 *   true  if fsm is initialized and current_state == state_id.
 *   false if fsm is NULL, not initialized, or current_state != state_id.
 *
 * Phase 12D: NO IMPLEMENTATION. Declaration only.
 */
bool robotos_fw_fsm_is_in_state(
    const robotos_fw_fsm_t *fsm,
    robotos_fw_state_id_t   state_id);

/*
 * robotos_fw_fsm_get_snapshot
 *
 * Purpose:
 *   Copy the current state and all counters into *out. Read under critical
 *   section so the snapshot is internally consistent.
 *
 * Context:
 *   Thread context only. Must NOT be called from ISR.
 *
 * Side effects:
 *   Writes *out. Does NOT mutate *fsm.
 *
 * Return:
 *   ROBOTOS_CORE_OK                 on success; *out is filled.
 *   ROBOTOS_CORE_ERR_NULL           if fsm or out is NULL.
 *   ROBOTOS_CORE_ERR_INVALID_STATE  if the FSM is not initialized.
 *
 * Phase 12D: NO IMPLEMENTATION. Declaration only.
 */
robotos_fw_status_t robotos_fw_fsm_get_snapshot(
    const robotos_fw_fsm_t      *fsm,
    robotos_fw_fsm_snapshot_t   *out);

#ifdef __cplusplus
}
#endif

#endif /* ROBOTOS_FW_FSM_H */
