/*
 * robotos_fw_event_bridge.h
 *
 * RobotOS Robot Framework — Application Event Bridge (Phase 12F host
 * prototype).
 *
 * STATUS: DRAFT / EXPERIMENTAL.
 *
 *   - Phase 12F adds this header and the matching .c body as a
 *     HOST-TESTED PROTOTYPE. No devkit integration. No hardware evidence.
 *     No UART command surface.
 *   - ABI is NOT stable. The public surface matches the §5 names in the
 *     Phase 12F-pre / Phase 12F long-lived spec but may still change.
 *
 * SPEC ANCHOR:
 *   RobotOS_v1.0/devkit/docs/03_SPECS/FRAMEWORK_APPLICATION_BRIDGE_DRAFT.md
 *
 * MODEL (Phase 12C/12F confirmed):
 *   - Bridge is a pure mapping engine: Adapter key (type, arg0) ->
 *     Framework logical event ID (robotos_fw_event_id_t).
 *   - APPLICATION-OWNED: the bridge does NOT register with
 *     robotos_core_register_event_handler(). The application calls
 *     robotos_fw_event_bridge_dispatch() from thread/dispatch context.
 *   - Bridge instance is CALLER-OWNED (statically allocated by the
 *     application or test). Bridge stores a pointer to a caller-owned
 *     config; the config and the mapping rows it points to must outlive
 *     the bridge.
 *   - FSM instance is CALLER-OWNED. The bridge takes a
 *     robotos_fw_fsm_t pointer from the config; the bridge does NOT own,
 *     init, reset, or free the FSM.
 *   - Mapping rows are scanned FIFO first-match. Arg0 may be an exact
 *     match (match_arg0 == true) or a wildcard (match_arg0 == false).
 *     Row order is the precedence rule; the bridge applies no extra
 *     specificity tiebreak.
 *   - Unmapped events: silent OK + unmapped_count++. The FSM is NOT
 *     called for unmapped events; bridge_dispatch() returns
 *     ROBOTOS_CORE_OK.
 *   - Mapped events: bridge calls robotos_fw_fsm_dispatch(fsm,
 *     row.fw_event_id, payload) and returns the FSM's status verbatim.
 *   - Payload is BORROWED for the duration of bridge_dispatch only; the
 *     bridge stores NO payload pointer in its instance struct. The
 *     application owns the memory.
 *   - Status type reuses robotos_core_status_t via the existing
 *     robotos_fw_status_t alias (Phase 12C
 *     REUSE_ROBOTOS_CORE_STATUS_T_FOR_PHASE_12D_CONFIRMED). NO new
 *     status enum is introduced.
 *   - Counter semantics (Phase 12F-pre §11 #6):
 *         event_count    increments on EVERY dispatch attempt.
 *         mapped_count   increments only when a row matched.
 *         unmapped_count increments only when no row matched.
 *         event_count == mapped_count + unmapped_count
 *
 * BOUNDARIES:
 *   - No UART TX from the bridge or any callback the bridge invokes
 *     directly. (FSM callbacks are governed by FSM rules.)
 *   - No hardware driver ownership (no GPIO/PWM/I2C/SPI/ADC).
 *   - No core event-handler registration.
 *   - No Zephyr header dependency.
 *   - No devkit dependency.
 *   - No devkit_app_state coupling.
 *   - No legacy ro_* dependency (Architecture A only).
 *   - No heap allocation.
 *
 * NOT INCLUDED IN PHASE 12F:
 *   - Devkit integration (shadow / replacement / separate application).
 *   - Hardware evidence.
 *   - ISR-context dispatch. Bridge is thread-context only.
 *   - Application/product layer.
 *   - UART command surface change. Frozen 9-command set unchanged.
 */

#ifndef ROBOTOS_FW_EVENT_BRIDGE_H
#define ROBOTOS_FW_EVENT_BRIDGE_H

#include <stdbool.h>
#include <stdint.h>

#include "robotos_fw_fsm.h"   /* robotos_fw_fsm_t, robotos_fw_event_id_t,
                               * robotos_fw_status_t, and (transitively)
                               * robotos_core_status_t for return codes. */

#ifdef __cplusplus
extern "C" {
#endif

/* ---------------------------------------------------------------------------
 * Adapter key shape (Phase 12F-pre §4.1 / spec §5.1)
 * ---------------------------------------------------------------------------
 *
 * The bridge accepts a small (type, arg0) pair rather than a full
 * robotos_event_t. This keeps the bridge module testable on host without
 * forcing the test to build the full core event struct. Production callers
 * translate robotos_event_t to (type, arg0) at the call site:
 *
 *     bridge_dispatch(bridge,
 *                     (uint32_t)event->type,
 *                     event->arg0,
 *                     payload);
 *
 * Both fields are sized as uint32_t to keep the bridge core-event-type
 * agnostic.
 */

/* ---------------------------------------------------------------------------
 * Mapping row (caller-owned; typically declared as a const static array)
 * ---------------------------------------------------------------------------
 *
 * A row matches an Adapter event when:
 *   - row.adapter_type == event_type, AND
 *   - row.match_arg0 == false   (wildcard arg0)
 *       OR
 *     row.match_arg0 == true && row.adapter_arg0 == event_arg0
 *
 * Rows are scanned FIFO; the first row that matches wins. The bridge
 * applies no other specificity tiebreak: if a wildcard row precedes an
 * exact-match row for the same type, the wildcard row wins.
 */
typedef struct {
    uint32_t              adapter_type;
    uint32_t              adapter_arg0;
    bool                  match_arg0;   /* false = wildcard on arg0 */
    robotos_fw_event_id_t fw_event_id;
} robotos_fw_event_bridge_row_t;

/* ---------------------------------------------------------------------------
 * Bridge configuration (caller-owned; pointer stored by the bridge)
 * ---------------------------------------------------------------------------
 *
 * The application owns the config object and the mapping array. Both must
 * outlive the bridge. The bridge stores `config` by pointer and never
 * copies the rows.
 *
 *   rows         pointer to a caller-owned array of mapping rows.
 *   row_count    number of rows in `rows`. Must be > 0.
 *   fsm          caller-owned FSM instance. Bridge does NOT own/init/reset
 *                the FSM. Must be non-NULL and already initialized.
 *   user_context opaque; reserved for future use. Not dereferenced by
 *                Phase 12F. The bridge does not pass user_context to FSM
 *                callbacks (the FSM has its own user_context in its config).
 */
typedef struct {
    const robotos_fw_event_bridge_row_t *rows;
    uint32_t                             row_count;
    robotos_fw_fsm_t                    *fsm;
    void                                *user_context;
} robotos_fw_event_bridge_config_t;

/* ---------------------------------------------------------------------------
 * Snapshot (audit / introspection)
 * ---------------------------------------------------------------------------
 *
 * Returned by robotos_fw_event_bridge_get_snapshot(). Counter semantics:
 *
 *   event_count       calls to bridge_dispatch() (one per attempted call).
 *   mapped_count      dispatches that matched a row and invoked the FSM.
 *   unmapped_count    dispatches that matched no row.
 *   last_adapter_type adapter_type of the most recent dispatch.
 *   last_adapter_arg0 adapter_arg0 of the most recent dispatch.
 *   last_fw_event_id  fw_event_id of the most recent MAPPED dispatch.
 *                     Zero if no mapped dispatch has occurred yet.
 *   last_status       return status of the most recent dispatch (the FSM's
 *                     return for a mapped event, or OK for unmapped).
 *   initialized       true after a successful init() until destroy/reinit.
 */
typedef struct {
    uint32_t              event_count;
    uint32_t              mapped_count;
    uint32_t              unmapped_count;
    uint32_t              last_adapter_type;
    uint32_t              last_adapter_arg0;
    robotos_fw_event_id_t last_fw_event_id;
    robotos_fw_status_t   last_status;
    bool                  initialized;
} robotos_fw_event_bridge_snapshot_t;

/* ---------------------------------------------------------------------------
 * Bridge instance (caller-owned; static allocation expected)
 * ---------------------------------------------------------------------------
 *
 * The application declares a robotos_fw_event_bridge_t object statically
 * and passes its address to the bridge API. The Framework does NOT
 * allocate this struct. Field layout is DRAFT / EXPERIMENTAL; treat all
 * fields as opaque and access them only through the public API. There is
 * intentionally NO payload pointer in this struct — the bridge does not
 * cache payload pointers after dispatch() returns.
 */
typedef struct robotos_fw_event_bridge {
    const robotos_fw_event_bridge_config_t *config;
    uint32_t              event_count;
    uint32_t              mapped_count;
    uint32_t              unmapped_count;
    uint32_t              last_adapter_type;
    uint32_t              last_adapter_arg0;
    robotos_fw_event_id_t last_fw_event_id;
    robotos_fw_status_t   last_status;
    bool                  initialized;
} robotos_fw_event_bridge_t;

/* ---------------------------------------------------------------------------
 * Public API (Phase 12F; ABI still DRAFT / EXPERIMENTAL)
 * ---------------------------------------------------------------------------
 */

/*
 * robotos_fw_event_bridge_init
 *
 * Purpose:
 *   Initialize a caller-owned bridge instance against a caller-owned
 *   config. Stores config by pointer, zeroes all counters, sets
 *   last_fw_event_id and last_status to their zero / OK defaults, and
 *   marks the bridge initialized. Does NOT init the FSM (the FSM must
 *   already be initialized by the caller).
 *
 * Input ownership / lifetime:
 *   *bridge — caller-owned; must outlive any use of the bridge.
 *   *config — caller-owned; must outlive the bridge. The mapping array
 *             that config->rows points to must also outlive the bridge.
 *   config->fsm — caller-owned; must already be initialized by the caller
 *             before the first dispatch call (init does not validate FSM
 *             state).
 *
 * Context:
 *   Thread context only. Must NOT be called from ISR.
 *
 * Side effects:
 *   Writes *bridge. Does NOT mutate *config. Does NOT touch the FSM.
 *   Does NOT call UART, hardware, or core dispatcher.
 *
 * Return:
 *   ROBOTOS_CORE_OK                 on success.
 *   ROBOTOS_CORE_ERR_NULL           if bridge, config, config->rows, or
 *                                   config->fsm is NULL.
 *   ROBOTOS_CORE_ERR_INVALID_ARG    if config->row_count == 0.
 *
 * Re-init policy: idempotent. A second init() on an already-initialized
 * bridge zeroes all counters and re-installs the config pointer.
 */
robotos_fw_status_t robotos_fw_event_bridge_init(
    robotos_fw_event_bridge_t              *bridge,
    const robotos_fw_event_bridge_config_t *config);

/*
 * robotos_fw_event_bridge_dispatch
 *
 * Purpose:
 *   Deliver an Adapter event key (adapter_type, adapter_arg0) plus a
 *   borrowed payload pointer through the bridge. Scans the mapping table
 *   FIFO. On the first match, calls
 *       robotos_fw_fsm_dispatch(bridge->config->fsm,
 *                               row.fw_event_id,
 *                               payload)
 *   and returns the FSM's status. On no match, returns OK and
 *   increments unmapped_count without calling the FSM.
 *
 * Input ownership / lifetime:
 *   *bridge — caller-owned; must be initialized.
 *   payload — borrowed const pointer; bridge does NOT cache it. The FSM
 *             may pass it to a dispatch action callback, but the bridge
 *             stores nothing referencing it once dispatch returns.
 *
 * Context:
 *   Thread / dispatch context only. Must NOT be called from ISR. The
 *   application is responsible for marshalling Adapter events from ISR
 *   into thread context before calling dispatch.
 *
 * Side effects:
 *   Mutates *bridge (counters, last_* fields). May call
 *   robotos_fw_fsm_dispatch which in turn may call user-provided FSM
 *   callbacks. Bridge itself does NOT call UART, hardware, or core
 *   dispatcher.
 *
 * Return:
 *   ROBOTOS_CORE_OK                 on a mapped dispatch whose FSM
 *                                   action returned OK, on a mapped
 *                                   dispatch that matched no transition
 *                                   row inside the FSM (FSM returns OK
 *                                   per the no-transition rule), or on
 *                                   an unmapped Adapter event.
 *   Any non-OK FSM status           on a mapped dispatch whose FSM
 *                                   action returned non-OK. The FSM's
 *                                   status is propagated verbatim.
 *   ROBOTOS_CORE_ERR_NULL           if bridge is NULL.
 *   ROBOTOS_CORE_ERR_INVALID_STATE  if bridge is not initialized.
 */
robotos_fw_status_t robotos_fw_event_bridge_dispatch(
    robotos_fw_event_bridge_t *bridge,
    uint32_t                   adapter_type,
    uint32_t                   adapter_arg0,
    const void                *payload);

/*
 * robotos_fw_event_bridge_reset
 *
 * Purpose:
 *   Reset bridge counters (event_count, mapped_count, unmapped_count,
 *   last_*) to their zero / OK defaults. Leaves the config pointer
 *   installed and the bridge initialized. Does NOT reset, init, or
 *   modify the FSM. (FSM reset is the caller's responsibility via
 *   robotos_fw_fsm_reset.)
 *
 * Context:
 *   Thread context only. Must NOT be called from ISR.
 *
 * Side effects:
 *   Mutates *bridge counters and last_* fields. Does NOT call UART,
 *   hardware, FSM, or core dispatcher.
 *
 * Return:
 *   ROBOTOS_CORE_OK                 on success.
 *   ROBOTOS_CORE_ERR_NULL           if bridge is NULL.
 *   ROBOTOS_CORE_ERR_INVALID_STATE  if bridge is not initialized.
 */
robotos_fw_status_t robotos_fw_event_bridge_reset(
    robotos_fw_event_bridge_t *bridge);

/*
 * robotos_fw_event_bridge_get_snapshot
 *
 * Purpose:
 *   Copy bridge counters and last_* fields into a caller-supplied
 *   snapshot struct.
 *
 * Context:
 *   Thread context only. Phase 12F is thread-context only and does NOT
 *   take a critical section here (Phase 12F-pre §11 #8). A future phase
 *   may add critical-section protection if ISR access is added.
 *
 * Side effects:
 *   Writes *out. Does NOT mutate *bridge. Does NOT call UART, hardware,
 *   FSM, or core dispatcher.
 *
 * Return:
 *   ROBOTOS_CORE_OK                 on success; *out is filled.
 *   ROBOTOS_CORE_ERR_NULL           if bridge or out is NULL.
 *   ROBOTOS_CORE_ERR_INVALID_STATE  if bridge is not initialized.
 */
robotos_fw_status_t robotos_fw_event_bridge_get_snapshot(
    const robotos_fw_event_bridge_t    *bridge,
    robotos_fw_event_bridge_snapshot_t *out);

#ifdef __cplusplus
}
#endif

#endif /* ROBOTOS_FW_EVENT_BRIDGE_H */
