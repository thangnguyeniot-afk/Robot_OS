# Phase 12B -- Robot Framework FSM API Draft

**Status:** `CLOSED_DOCS_ONLY`
**Type:** Docs-only API surface draft. **No source, runtime, test, CMake,
Zephyr, board, host-tool, script, `prj.conf`, DTS overlay, evidence log, or
`framework/` directory change.** No `.h` or `.c` Framework file created.
Phase 12B defines the conceptual Robot Framework FSM API surface and
contracts. It does not implement anything, does not create any Framework
source, does not promote `devkit_app_state`, and does not authorize any
implementation phase automatically.
**Date opened/closed:** 2026-05-12 (same-day docs-only close)
**Branch:** master
**Published baseline at open:** `origin/master = 76cb241`
**Prior closed phase:** Phase 12A (`CLOSED_DOCS_ONLY`; Framework API surface
planning; recommended flat Framework State-Machine Abstraction as first slice).
**Prior runtime implementation phase:** Phase 11D (firmware `2040bfb`).
**Prior hardware evidence phase:** Phase 11E (`10710b3`; `CLOSED_WITH_HARDWARE_EVIDENCE`).
**Companion docs:**
[`PHASE_12A_FRAMEWORK_API_SURFACE_PLANNING.md`](PHASE_12A_FRAMEWORK_API_SURFACE_PLANNING.md),
[`PHASE_11Z_COMMAND_SET_CHECKPOINT.md`](PHASE_11Z_COMMAND_SET_CHECKPOINT.md),
[`../03_SPECS/FRAMEWORK_FSM_API_DRAFT.md`](../03_SPECS/FRAMEWORK_FSM_API_DRAFT.md)
(long-lived spec extracted from this closeout).

---

## A. Executive Summary

Phase 12B is a docs-only phase that **drafts the Robot Framework flat FSM
API surface** in enough detail to support a future implementation decision.

It produces two artifacts:

1. **This closeout doc** — rationale, decisions, and design record.
2. **`03_SPECS/FRAMEWORK_FSM_API_DRAFT.md`** — long-lived spec draft that
   will evolve across Phase 12C and later.

**Decision result: `PHASE_12B_FSM_API_DRAFTED_DOCS_ONLY`**

The draft is substantially complete. Two design questions are flagged as
**open decisions** requiring explicit confirmation before any implementation
starts (see §N): the event bridge pattern and the status model choice. Both
have documented recommendations; neither is blocking for Phase 12B close.

Phase 12B:

- Introduces **no source, header, or implementation** of any kind.
- Creates **no `framework/` directory**.
- Changes **no runtime behavior**.
- Does not promote `devkit_app_state` to Framework (§K).
- Does not start Application/product work.
- Does not authorize implementation automatically — implementation belongs
  to a future Phase 12C-class gate.

---

## B. Baseline Before Phase 12B

| Item | Value |
|---|---|
| `origin/master` at open | `76cb241` (`docs: add Phase 12A framework API surface planning`) |
| Last closed phase | Phase 12A (`CLOSED_DOCS_ONLY`) |
| Phase 12A decision | Flat Framework State-Machine Abstraction recommended as first Framework slice |
| Last runtime implementation phase | Phase 11D (`2040bfb`) |
| Last hardware evidence phase | Phase 11E (`10710b3`; `CLOSED_WITH_HARDWARE_EVIDENCE` + `OPERATOR_PHYSICAL_SANITY_CONFIRMED`) |
| Validated command set | **`a / s / r / ? / x / v / L / d / T`** (9 commands; all hardware-evidence-backed) |
| Robot Framework | **NOT BUILT** — no `framework/` dir, no Framework header, no API |
| Application / product | **NOT BUILT** — no product vocabulary, no use-case selection |
| `devkit_app_state` | Devkit-local validation harness; design reference for FSM pattern; not promoted |
| Devkit event type allocation | `ROBOTOS_EVENT_USER + 0` = 100 (runtime legacy), `+1` = 101 (timer), `+2` = 102 (button), `+3` = 103 (UART) |
| Current Adapter limitations | `MAX_EVENTS_PER_TICK=1`; no scheduler mutation authorized; no pool/slab decision; portability undemonstrated |

**Remaining open gates (all preserved unchanged at Phase 12B):**

1. ACTIVE disarm widening — `USER_DECISION_REQUIRED_ACTIVE_DISARM`
2. Scheduler 7A/7B — `DEFER`
3. F407 / custom board — `HOLD/DEFER`
4. POST_FLASH_AUTOSTART root cause — `OPEN` / `MITIGATED_BY_WORKFLOW`
5. Application / product layer — `NOT_STARTED`

---

## C. Why FSM First

Phase 12A §F established the rationale for FSM as the first Framework slice.
This section restates the key points as a design-record so they are not
re-derived in Phase 12C.

**Evidence-reuse:** `devkit_app_state.c` is a validated, hardware-evidence-
backed FSM prototype. The IDLE/ARMED/ACTIVE pattern with transition counting,
ignored-path counting, and source-tracking demonstrates that an event-driven
FSM of this shape works correctly on the target hardware over all Phase 9–11
runs. The Framework FSM generalizes this validated pattern without promoting
or copying devkit-specific vocabulary.

**No new hardware required:** A flat FSM API needs only the existing core event
dispatch substrate. No sensor, no actuator, no I2C/SPI bus access, no PWM
timer. Zero hardware dependency beyond what is proven.

**Avoids product semantics:** A generic FSM API (state IDs as product-defined
integers; event IDs as application-defined integers) carries no domain
vocabulary. It does not commit to CNC/printer/arm/mobile states or events.
It is equally usable for a motion controller, a communication protocol FSM,
or a mode-management layer.

**Avoids Scheduler reopening:** A single Framework FSM instance driven by
the existing event dispatch path adds at most one handler invocation per
dispatched event. At `MAX_EVENTS_PER_TICK=1` this fits the current dispatch
budget without mutation. Scheduler 7A/7B remains `DEFER`.

**Decoupled from actuator/PID/motion:** Specifying the FSM API does not
require knowledge of stepper, servo, PID, or trajectory semantics. The FSM
is the control-flow primitive; it is domain-independent.

**`devkit_app_state` provides evidence, not template:** The Framework FSM API
is a new, product-neutral artifact. Lessons extracted from `devkit_app_state`
(explicit transitions, source tracking, idempotency handling, query semantics)
inform the design. The module itself is not renamed, moved, or aliased into
the Framework layer.

---

## D. Layer Boundary

### D.1 Position in layer stack

```
[ Application / product layer    ]  defines product state IDs, event IDs,
                                    guards, actions, and the event-bridge
                                    that translates Adapter events to FSM
                                    logical events

[ Robot Framework layer          ]  robotos_fw_fsm_* API: product-neutral FSM
                                    engine; dispatches logical events;
                                    evaluates transition table; runs callbacks
                                    in dispatch thread context

[ Robot Adapter / runtime        ]  core/: event queue, dispatch, admission
  substrate                         platform/: critical section, time, fault
                                    devkit glue: button/UART/timer producers
                                    (devkit-local; not Framework)

[ Kernel / HW                    ]  Zephyr 3.6.0 + STM32F411E-DISCO
```

### D.2 Framework FSM boundary assertions

- **Framework FSM is above the Adapter/runtime substrate.** It consumes
  Adapter events (timer tick, GPIO input, UART byte) only via a thin
  Application-defined event bridge, not by coupling to devkit event types.

- **Framework FSM must not depend on the devkit UART command vocabulary.**
  `a/s/r/?/x/v/L/d/T` are Adapter/devkit probe commands. The FSM does not
  receive these byte values directly; the Application layer (or a future
  command-to-event mapping) translates them to logical event IDs.

- **Framework FSM must not call UART TX directly.** UART TX remains under the
  12 scope-guard constraints from `PHASE_9EZ_CHECKPOINT.md §H`. The FSM
  produces state transitions and calls action callbacks; actions may
  log via RTT but must not call `uart_poll_out` or any UART TX path.

- **Framework FSM must not own board-specific GPIO/sensor drivers.** It is a
  control-flow engine. Driver access is the Adapter layer's responsibility;
  the FSM may trigger it via action callbacks if the Application layer wires
  that dependency, but the FSM itself has no driver dependency.

- **Framework FSM must be product-neutral.** State IDs and event IDs are
  `uint32_t` values defined by the product/Application layer. The Framework
  FSM engine has no concept of "ARMED", "ACTIVE", "stepper", "CNC", etc.

---

## E. FSM Model Draft

### E.1 Model choice: flat, table-driven FSM

**Chosen: flat FSM, table-driven with per-transition action callbacks.**

| Design axis | Choice | Rationale |
|---|---|---|
| Flat vs hierarchical | **Flat** | No product state machine is known yet; flat is the minimum that generalizes. Hierarchy deferred to later phase. |
| Table-driven vs callback-router | **Table-driven** | Transition table is compile-time-verifiable, auditable, and matches the "explicit over implicit" discipline established by devkit patterns. Callback-routers (big switch) are less auditable. |
| Dynamic vs static config | **Static (compile-time)** | No heap. Config passed at init and not modified at runtime. Matches fixed-capacity ring buffer discipline. |
| Allocation model | **No heap; caller-owned struct** | Consistent with `robotos_event_queue_t`, `robotos_event_dispatcher_t` patterns. |
| State IDs | **Product-defined `uint32_t`** | Framework defines no state vocabulary. 0 = "uninitialized" is reserved by convention only. |
| Event IDs | **Application-defined `uint32_t`** | Completely decoupled from `robotos_event_type_t`; no sub-range allocation of `ROBOTOS_EVENT_USER` needed (see §G). |
| Transition evaluation order | **First-match FIFO** | Scan transition table from index 0; first row matching (current_state, event_id) where guard passes is selected. Deterministic. |
| Concurrency model | **None beyond current dispatch thread** | FSM dispatch always runs in thread context, never in ISR. State query is ISR-safe via critical section. No concurrent dispatch instances. |
| No scheduler mutation | Yes — fits `MAX_EVENTS_PER_TICK=1` budget for single FSM | |

### E.2 Conceptual FSM components

**States:** A state is a named operating mode of the product. Represented as
a `uint32_t` ID. The Framework FSM table maps `(current_state, event_id)` to
`(next_state, action)`. State has optional entry and exit callbacks.

**Events:** A logical trigger. Represented as a `uint32_t` ID. Events are
**not** `robotos_event_type_t` values — they are a separate, application-
defined namespace (see §G). An event may carry an optional const payload
pointer for read-only context data (e.g., sensor reading, byte value).

**Guard:** An optional pure predicate associated with a transition row. A
guard returns `true` (allow) or `false` (reject). Guards must not mutate any
state and must not call UART TX, sensors, or drivers. Guards run in dispatch
thread context.

**Action:** An optional callback associated with a transition row. Called
after guard passes, after exit action, before entry action, before state
update. Actions may mutate application context (via the `user_context`
pointer). Actions must not call UART TX directly and must not perform
blocking I/O. Actions run in dispatch thread context.

**Entry action:** Optional per-state callback called when a state is entered
(after transition action and state update). Purpose: initialize state-local
resources.

**Exit action:** Optional per-state callback called when a state is left
(before action). Purpose: clean up state-local resources.

**No-transition path:** If no row matches `(current_state, event_id)` or all
guards reject, the event is silently counted (`no_transition_count++` or
`guard_rejected_count++`) and OK is returned. This is the Framework analog
of `devkit_app_state`'s `ignored_count`.

**Audit counters:** `transition_count`, `event_count`, `guard_rejected_count`,
`no_transition_count`, `last_event_id`, `last_status`. These are the
Framework FSM's telemetry surface (analogous to `devkit_app_state`'s
`transitions`, `uart_count`, `ignored_count`).

### E.3 Transition evaluation sequence

For a given `(fsm, event_id, event_payload)` dispatch call:

```
1.  event_count++
2.  last_event_id = event_id
3.  Scan transition table (index 0 .. transition_count - 1):
    a.  Skip rows where row.current_state != fsm->current_state
    b.  Skip rows where row.event_id != event_id
    c.  If row found and row.guard != NULL:
          if guard(current_state, event_id, payload, user_ctx) == false:
            guard_rejected_count++
            continue to next row (first-match FIFO with guard-skip)
    d.  If row found and guard passes (or guard == NULL):
          Found = TRUE, selected_row = row
          break
4.  If no matching row found after full scan:
    no_transition_count++
    last_status = OK (unhandled is not an error by default)
    return OK
5.  If selected_row found:
    a.  Call on_exit(current_state, user_ctx)   if defined in state table
    b.  Call action(from, to, event, payload, user_ctx)  if non-NULL
    c.  fsm->current_state = selected_row.next_state
    d.  Call on_entry(next_state, user_ctx)     if defined in state table
    e.  transition_count++
    f.  last_status = action_status (or OK if no action)
    g.  return last_status
```

Note: If the action returns non-OK, the transition IS committed (state is
already updated at step 5c). The action error is reported but does not roll
back the transition. This matches the existing core dispatch policy where
handler errors are counted but do not roll back event consumption.

---

## F. Conceptual API Surface

**ALL ITEMS IN THIS SECTION ARE DRAFT / NON-FINAL.**
No `.h` file is created. No function name is final. All names carry the
`robotos_fw_fsm_` prefix as a draft convention subject to change in Phase 12C.

### F.1 Types (conceptual, DRAFT)

```c
/* DRAFT / NON-FINAL — for design discussion only; no .h file created */

/* State and event ID types — product-defined; Framework imposes no values */
typedef uint32_t robotos_fw_state_id_t;
typedef uint32_t robotos_fw_event_id_t;

/* Convention (not enforced by Framework): state ID 0 = "uninitialized" sentinel */
#define ROBOTOS_FW_STATE_UNINIT   ((robotos_fw_state_id_t)0u)

/* Status type — OPEN DECISION: reuse robotos_core_status_t or define new
 * robotos_fw_status_t? Recommendation: reuse robotos_core_status_t.
 * See §N.2 for the unresolved item record.
 *
 * If reused, additional Framework error semantics map as:
 *   guard rejected     -> ROBOTOS_CORE_ERR_INVALID_ARG
 *   no matching row    -> ROBOTOS_CORE_OK (unhandled not an error)
 *   null FSM/config    -> ROBOTOS_CORE_ERR_NULL
 *   not initialized    -> ROBOTOS_CORE_ERR_INVALID_STATE
 */
typedef robotos_core_status_t robotos_fw_status_t;  /* DRAFT; see §N.2 */

/* Guard: pure predicate; must not mutate context; thread context only */
typedef bool (*robotos_fw_guard_fn_t)(
    robotos_fw_state_id_t  current_state,
    robotos_fw_event_id_t  event_id,
    const void            *event_payload,   /* may be NULL */
    void                  *user_context     /* application context */
);

/* Transition action: may mutate user_context; thread context only */
typedef robotos_fw_status_t (*robotos_fw_action_fn_t)(
    robotos_fw_state_id_t  from_state,
    robotos_fw_state_id_t  to_state,
    robotos_fw_event_id_t  event_id,
    const void            *event_payload,   /* may be NULL */
    void                  *user_context
);

/* Entry/exit action per state: thread context only */
typedef robotos_fw_status_t (*robotos_fw_entry_exit_fn_t)(
    robotos_fw_state_id_t  state_id,
    void                  *user_context
);

/* One row in the transition table */
typedef struct {
    robotos_fw_state_id_t   current_state;
    robotos_fw_event_id_t   event_id;
    robotos_fw_guard_fn_t   guard;      /* NULL = no guard; transition always allowed */
    robotos_fw_state_id_t   next_state;
    robotos_fw_action_fn_t  action;     /* NULL = no action */
} robotos_fw_transition_t;

/* One row in the state definition table (for entry/exit) */
typedef struct {
    robotos_fw_state_id_t      state_id;
    robotos_fw_entry_exit_fn_t on_entry;  /* NULL = no entry action */
    robotos_fw_entry_exit_fn_t on_exit;   /* NULL = no exit action */
} robotos_fw_state_def_t;

/* Compile-time FSM configuration — must outlive the FSM object */
typedef struct {
    const robotos_fw_transition_t *transitions;     /* pointer to transition table */
    uint32_t                       transition_count;
    const robotos_fw_state_def_t  *states;          /* NULL = no entry/exit actions */
    uint32_t                       state_count;     /* 0 if states is NULL */
    robotos_fw_state_id_t          initial_state;
    void                          *user_context;    /* passed to all callbacks */
} robotos_fw_fsm_config_t;

/* Audit snapshot filled by robotos_fw_fsm_get_snapshot() */
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

/* FSM runtime object — caller-owned, statically allocated; initialized via init() */
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
```

### F.2 Function signatures (conceptual, DRAFT)

Each function entry specifies: purpose, inputs, outputs, thread-context
assumption, allowed side effects, forbidden side effects, Adapter dependency,
and validation expectation.

---

**`robotos_fw_fsm_init`** — DRAFT / NON-FINAL

```c
robotos_fw_status_t robotos_fw_fsm_init(
    robotos_fw_fsm_t              *fsm,
    const robotos_fw_fsm_config_t *config
);
```

| Aspect | Value |
|---|---|
| Purpose | Initialize an FSM object from a static config. Calls `on_entry` for `initial_state` if defined. |
| Inputs | `fsm`: caller-owned struct; `config`: static compile-time config that must outlive FSM |
| Output / status | `OK` on success; `ERR_NULL` if fsm/config/transitions is NULL; `ERR_INVALID_ARG` if `transition_count == 0` or `initial_state == UNINIT` |
| Thread-context | Thread context only; not ISR-safe |
| Allowed side effects | Sets all FSM fields; may call `on_entry(initial_state)` |
| Forbidden side effects | No UART TX; no sensor/driver call; no heap |
| Adapter dependency | None (pure Framework object init) |
| Validation expectation | Counter values are 0 after init; `current_state == initial_state`; `initialized == true` |

---

**`robotos_fw_fsm_dispatch`** — DRAFT / NON-FINAL

```c
robotos_fw_status_t robotos_fw_fsm_dispatch(
    robotos_fw_fsm_t      *fsm,
    robotos_fw_event_id_t  event_id,
    const void            *event_payload   /* may be NULL */
);
```

| Aspect | Value |
|---|---|
| Purpose | Drive the FSM with a logical event. Evaluates transition table; calls guard, exit, action, entry in sequence if transition found (§E.3). |
| Inputs | `fsm`: initialized FSM; `event_id`: logical event; `event_payload`: optional read-only context pointer |
| Output / status | `OK` on successful transition or unhandled event; `ERR_NULL` if fsm is NULL; `ERR_INVALID_STATE` if not initialized; action's non-OK status on action failure |
| Thread-context | Thread context only (dispatch handler); never ISR |
| Allowed side effects | Updates `current_state`, `transition_count`, `event_count`, `last_event_id`, `last_status`; calls guard/action/entry/exit callbacks |
| Forbidden side effects | No UART TX; no heap; no ISR invocation; no direct sensor/driver call |
| Adapter dependency | `robotos_platform_critical.h` for ISR-safe state snapshot query (not dispatch itself) |
| Validation expectation | After `dispatch`: `event_count` incremented; if transition occurred, `transition_count` incremented and `current_state` updated; if no match, `no_transition_count` incremented |

---

**`robotos_fw_fsm_get_state`** — DRAFT / NON-FINAL

```c
robotos_fw_state_id_t robotos_fw_fsm_get_state(
    const robotos_fw_fsm_t *fsm
);
```

| Aspect | Value |
|---|---|
| Purpose | Return current state ID. ISR-safe (atomic read or under critical section). |
| Inputs | `fsm`: any state (returns `ROBOTOS_FW_STATE_UNINIT` if NULL or not initialized) |
| Output | Current `current_state` value |
| Thread-context | ISR-safe (read-only under critical section) |
| Allowed side effects | None |
| Forbidden side effects | No mutation; no callbacks |
| Adapter dependency | `robotos_platform_critical.h` if state cannot be read atomically on 32-bit MCU (likely a single 32-bit read is atomic on Cortex-M4; but critical section is the correct portable contract) |
| Validation expectation | Returns `initial_state` before any dispatch; reflects latest committed state after dispatch |

---

**`robotos_fw_fsm_reset`** — DRAFT / NON-FINAL

```c
robotos_fw_status_t robotos_fw_fsm_reset(
    robotos_fw_fsm_t *fsm
);
```

| Aspect | Value |
|---|---|
| Purpose | Reset FSM to initial state. Calls `on_exit(current_state)` then `on_entry(initial_state)` if defined. Zeroes all counters. |
| Inputs | `fsm`: initialized FSM |
| Output / status | `OK` on success; `ERR_NULL` if fsm is NULL; `ERR_INVALID_STATE` if not initialized |
| Thread-context | Thread context only |
| Allowed side effects | State and counters reset; may call `on_exit` then `on_entry` |
| Forbidden side effects | No UART TX; no heap |
| Adapter dependency | None |
| Validation expectation | After reset: `current_state == initial_state`; all counters == 0; `initialized == true` |

---

**`robotos_fw_fsm_is_in_state`** — DRAFT / NON-FINAL

```c
bool robotos_fw_fsm_is_in_state(
    const robotos_fw_fsm_t *fsm,
    robotos_fw_state_id_t   state_id
);
```

| Aspect | Value |
|---|---|
| Purpose | Predicate: true if FSM is currently in the given state. ISR-safe. |
| Inputs | `fsm`, `state_id` |
| Output | `true` if `fsm->current_state == state_id`; `false` on NULL or not initialized |
| Thread-context | ISR-safe |
| Allowed side effects | None |
| Forbidden side effects | No mutation; no callbacks |
| Adapter dependency | Same critical section contract as `get_state` |
| Validation expectation | Same as `get_state` |

---

**`robotos_fw_fsm_get_snapshot`** — DRAFT / NON-FINAL

```c
robotos_fw_status_t robotos_fw_fsm_get_snapshot(
    const robotos_fw_fsm_t    *fsm,
    robotos_fw_fsm_snapshot_t *out
);
```

| Aspect | Value |
|---|---|
| Purpose | Copy all audit counters and state into a caller-owned snapshot struct. Thread context only (not ISR-safe due to multi-field copy). |
| Inputs | `fsm`: initialized FSM; `out`: caller-owned snapshot buffer |
| Output / status | `OK` on success; `ERR_NULL` if fsm or out is NULL; `ERR_INVALID_STATE` if not initialized |
| Thread-context | Thread context only |
| Allowed side effects | Fills `*out`; no FSM mutation |
| Forbidden side effects | No callbacks; no UART TX; no heap |
| Adapter dependency | None beyond consistent reading (same thread context means no concurrent mutation during copy) |
| Validation expectation | `out->transition_count == fsm->transition_count`; `out->current_state == fsm->current_state` at time of call |

---

## G. Event Type / Event Range Planning

### G.1 Existing event type allocation (confirmed from source)

| Offset | Type value | Module | Notes |
|---|---|---|---|
| `ROBOTOS_EVENT_USER + 0` | 100 | `devkit_runtime.c` | Legacy Phase 6I; handler still registered but superseded |
| `ROBOTOS_EVENT_USER + 1` | 101 | `devkit_timer_producer.c` | Periodic tick event |
| `ROBOTOS_EVENT_USER + 2` | 102 | `devkit_button_producer.c` | Button EXTI event |
| `ROBOTOS_EVENT_USER + 3` | 103 | `devkit_uart_producer.c` | UART byte event |

4 of 8 available handler slots (`ROBOTOS_CORE_MAX_EVENT_HANDLERS = 8`) are
currently occupied.

### G.2 Design choice: FSM event IDs are decoupled from `robotos_event_type_t`

**Key decision:** The Framework FSM does **not** receive events via direct
registration with `robotos_core_register_event_handler()`. Instead:

- The Application layer registers an Adapter event handler (e.g., for type 102
  or 103) with the core.
- That handler translates the raw Adapter event into a logical
  `robotos_fw_event_id_t` value (e.g., `CYCLE_EVENT = 1`,
  `STOP_EVENT = 2`, `FAULT_EVENT = 3`) defined by the Application.
- The handler then calls `robotos_fw_fsm_dispatch(fsm, logical_event, payload)`.

**Consequence:** The Framework FSM's `robotos_fw_event_id_t` values occupy a
**separate namespace** from `robotos_event_type_t`. There is no collision risk
with devkit event types 100–103. No `ROBOTOS_EVENT_USER` sub-range needs to
be reserved for Framework FSM use.

**Exception (future, flagged):** If the FSM needs to **post** events back into
the core queue (e.g., "FSM transitioned to state X" observability event), it
would need a `robotos_event_type_t` allocation. Phase 12B does not design this
direction; it is a deferred concern for a future observability planning phase.

### G.3 Event bridge pattern (OPEN DECISION — recommendation documented)

The event bridge is the Application-layer mechanism that translates Adapter
events to Framework logical events. Phase 12B recommends this shape:

```
                       +------------------------+
Adapter event          |   Application          |   Framework FSM
(type=102, button)  -->|   event bridge         |-->  robotos_fw_fsm_dispatch()
(type=103, UART)    -->|   (product-specific)   |     logical_event_id (uint32_t)
(type=101, timer)   -->|                        |     event_payload (const void*)
                       +------------------------+
```

The event bridge is **not** a Framework component — it belongs to the
Application layer and is product-specific. Phase 12B does not specify the
bridge's internal structure, only its interface contract:

- Input: one Adapter event (received from the core dispatcher handler callback)
- Output: call to `robotos_fw_fsm_dispatch(fsm, logical_id, payload)` or no-op

This pattern is a **recommendation pending explicit confirmation in Phase 12C
as part of the event bridge spec.** See §N.1.

### G.4 Conclusion

No source change to event type constants is needed or authorized in Phase 12B.
The Framework FSM uses its own `robotos_fw_event_id_t` (uint32_t) namespace
that is completely independent of `robotos_event_type_t`. No sub-range
allocation is required before implementation — the Application layer assigns
event ID values.

---

## H. State / Event Representation Decisions

### H.1 Resolved choices

| Design question | Decision | Rationale |
|---|---|---|
| State ID type | `uint32_t` aliased as `robotos_fw_state_id_t` | Portable; matches existing `robotos_event_type_t` pattern; no Zephyr dependency |
| Event ID type | `uint32_t` aliased as `robotos_fw_event_id_t` | Product-defined; no Framework-imposed values |
| Reserved state ID | `0` = "uninitialized" sentinel by convention | Consistent with Zephyr and RobotOS zero-init policy; enforced at `init()` |
| Event payload | `const void *` pointer; application-managed lifetime; may be NULL | Avoids fixed payload size; passes only metadata; Framework FSM does not own or copy payload |
| User context | `void *user_context` in config; passed to all callbacks | Matches `robotos_event_dispatcher_t` and `robotos_event_handler_t` pattern |
| Allocation model | No heap; caller-owned `robotos_fw_fsm_t` | Consistent with all Adapter/core data structures |
| Transition table | `const robotos_fw_transition_t[]`; compile-time static; pointer in config | No dynamic mutation; auditable at review time |
| Evaluation order | First-match FIFO | Simple, deterministic; matches devkit `switch` dispatch discipline |

### H.2 Open decisions (flagged for Phase 12C)

**H.2.1 Status model (OPEN — recommendation documented)**

Two options:
- **Option A (Recommended):** Reuse `robotos_core_status_t`. Advantages: no
  new type; existing error codes cover all Framework FSM failure modes
  (`ERR_NULL`, `ERR_INVALID_STATE`, `ERR_INVALID_ARG`). Disadvantage: no
  Framework-specific error code for "guard rejected" vs "action failed".
- **Option B:** Define `robotos_fw_status_t` as a Framework-specific enum
  extending `robotos_core_status_t` with additional codes (`FW_ERR_GUARD_REJECTED`,
  `FW_ERR_NO_TRANSITION`). Advantage: richer diagnostics. Disadvantage: type
  proliferation; requires new enum in a future header.

**Recommendation: Option A (reuse `robotos_core_status_t`) for Phase 12C
first implementation.** Guard rejection maps to `ERR_INVALID_ARG`; no
matching transition is OK (not an error). If diagnostics later require
finer distinction, add a `robotos_fw_status_t` extension in a later phase.

Phase 12C must confirm this decision before writing the header.

**H.2.2 Event payload lifetime (OPEN — rules documented, confirmation needed)**

Rule as drafted: `event_payload` is a `const void *` pointer valid only for
the duration of the `dispatch()` call. Callbacks must not cache the pointer
after returning. The Application layer owns the payload memory.

This rule mirrors the core event pointer semantics:
`event is valid for the duration of the call only` (`robotos_event_handler_t`
comment in `robotos_event_dispatcher.h`).

Phase 12C must confirm this rule and document it in the header comment.

**H.2.3 Action return semantics on non-OK (OPEN — policy documented, confirmation needed)**

Policy as drafted in §E.3: if the action returns non-OK, the transition IS
committed. The state update is not rolled back. Non-OK propagates to the
`dispatch()` return value and increments no specific rollback counter.

Rationale: rollback-on-error requires tracking pre-transition state, which
adds complexity. Since the Framework is not a transactional system, a failed
action in the middle of a transition is better treated as an error that the
Application monitors (via `last_status`) rather than a rollback trigger.

Phase 12C must confirm this policy.

---

## I. Action / Guard Contract

### I.1 Guard contract

A guard function is a **pure predicate**: it inspects context, does not
mutate any state, and returns `true` (allow) or `false` (reject).

| Guard rule | Detail |
|---|---|
| No mutation | Must not modify `user_context`, FSM fields, or any shared state |
| No blocking | Must return promptly; no sleep, no I/O wait |
| No UART TX | Must not call `uart_poll_out` or any UART TX path |
| No heap | No dynamic allocation |
| No ISR execution | Called from dispatch, which is thread context only |
| No driver call | Must not call Zephyr driver APIs directly |
| Return `true` = allow | Transition proceeds |
| Return `false` = reject | `guard_rejected_count++`; evaluation continues to next matching row (first-match with guard-skip); if all matching rows reject, `no_transition_count++` |

### I.2 Action contract

An action function is a **transition side-effect handler**. It may mutate
the Application context.

| Action rule | Detail |
|---|---|
| May mutate `user_context` | The Application owns `user_context`; it may be a struct pointer that the action updates |
| May log via RTT | `robotos_platform_log.h` calls (LOG_INF equivalent) are permitted; RTT is the canonical logging backend |
| Must return status | `ROBOTOS_CORE_OK` on success; non-OK propagates up |
| No UART TX | Must not call `uart_poll_out` or TX path |
| No blocking I/O | Must not sleep or wait on hardware |
| No heap | No dynamic allocation |
| No ISR execution | Called from dispatch (thread context only) |
| No direct sensor/driver call | Unless a future Phase 12C+ decision explicitly allows it (e.g., a Framework sensor action contract); not authorized in Phase 12B |
| No state rollback on failure | If action returns non-OK, transition is already committed (§H.2.3) |

### I.3 Entry / exit action contract

Entry and exit actions follow the same constraints as transition actions
(§I.2), plus:

| Rule | Detail |
|---|---|
| Entry called after transition | `on_entry(new_state, user_ctx)` is called after `current_state` is updated |
| Exit called before transition | `on_exit(old_state, user_ctx)` is called before `action` and before `current_state` update |
| Optional | NULL means no entry/exit for that state |
| On reset | `on_exit(current_state)` called before `on_entry(initial_state)` |

---

## J. Observability / Debug Contract

### J.1 What the Framework FSM exposes for observability

The `robotos_fw_fsm_snapshot_t` struct (§F.1) captures all audit counters
and current state. A Framework observability module (future) would call
`robotos_fw_fsm_get_snapshot()` periodically and emit a `ROBOTOS_FW` RTT
log line analogous to the existing `ROBOTOS_APP` log line.

**Illustrative future log line (NOT produced by Phase 12B; design placeholder):**

```text
ROBOTOS_FW fsm_id=N state=N transitions=N events=N rejected=N no_trans=N last_event=N
```

This would be emitted by a future Framework observability module at the same
cadence as `ROBOTOS_APP` (every `DEVKIT_OBSERVABILITY_LOG_INTERVAL_TICKS` =
10 ticks).

### J.2 Mapping to existing RTT discipline

| FSM counter | Analog in devkit_app_state | Purpose |
|---|---|---|
| `transition_count` | `transitions` | Audit of state changes |
| `event_count` | `uart_count` (total events seen) | Volume metric |
| `guard_rejected_count` | n/a (new) | Transition filtering metric |
| `no_transition_count` | `ignored_count` | Unhandled-event metric |
| `current_state` | `state` | Current operating mode |
| `last_event_id` | `last_byte` | Last trigger |
| `last_status` | n/a (new) | Last action outcome |

### J.3 Optional observer callback

If a product requires synchronous notification on every state transition, a
future `robotos_fw_fsm_set_observer()` API could register a callback. This is
not in the Phase 12B scope — it is noted here as a future extension point.

### J.4 What must NOT be added yet

- No new UART query field (`?` response shape is frozen).
- No telemetry schema mutation to existing `ROBOTOS_OBS`/`ROBOTOS_APP` lines.
- No `ROBOTOS_FW` RTT log stream (planned but not implemented in Phase 12B).
- No response queue addition.

---

## K. Relationship to `devkit_app_state`

This section is required per the task spec. It states clearly and completely
the relationship between the Framework FSM API and the existing devkit module.

### K.1 `devkit_app_state` remains devkit-local

- `devkit_app_state.c` and `devkit_app_state.h` are not modified by Phase 12B.
- They are not moved, copied, renamed, or aliased into the Framework layer.
- Scope-guard #11 from `PHASE_9EZ_CHECKPOINT.md §H` remains in force:
  "No promotion of `devkit_app_state` from devkit-local to Robot Framework."
- Phase 12B does not lift this guard.

### K.2 Design lessons extracted (not copied)

Phase 12B extracts design lessons from `devkit_app_state` as a validated
concrete example of the FSM pattern. These lessons inform the Framework FSM
API design but do not constitute promotion.

| `devkit_app_state` pattern | Framework FSM design lesson extracted |
|---|---|
| Three explicit state enum values (IDLE/ARMED/ACTIVE) | States are `uint32_t` product-defined values; 0 = uninit sentinel by convention |
| Explicit `transition()` helper with prev/next log | `transition_count++` and `on_exit`/`on_entry` callbacks make transitions observable |
| `ignored_count` for unhandled commands | `no_transition_count` covers the same "event had no handler" diagnostic case |
| `uart_count` counting all events | `event_count` covers all dispatched events to the FSM |
| State-local `state_name()` for logging | Framework FSM returns `current_state` as integer; naming belongs to the Application layer |
| Query response (`?` command) reports state + counters | `robotos_fw_fsm_get_snapshot()` serves the same audit purpose |
| No locks (single-thread discipline) | Framework FSM also dispatch-thread-only; `get_state` is ISR-safe via critical section |
| No heap | Framework FSM also no-heap; static allocation |

### K.3 What is explicitly NOT carried over

- `DEVKIT_APP_STATE_IDLE`, `DEVKIT_APP_STATE_ARMED`, `DEVKIT_APP_STATE_ACTIVE`
  state IDs: the Framework FSM has no built-in state vocabulary. These devkit
  state names do not appear in any Framework API.
- `devkit_app_src_t` (BTN/UART source tracking): the Framework FSM has no
  event-source concept at API level; the Application bridge is responsible for
  tracking event sources.
- `on_uart_byte()` / `on_button()` entry points: the Framework FSM has a single
  `dispatch(event_id, payload)` entry point; event-source routing is the
  Application bridge's responsibility.
- `devkit_app_state_log_snapshot()` as Zephyr `LOG_INF`: the Framework FSM
  logs nothing itself; observability is via `get_snapshot()` + a future
  `ROBOTOS_FW` log module.

---

## L. Relationship to Command Set

- **`a/s/r/?/x/v/L/d/T` remain the devkit/probe command surface.** Phase 12B
  does not add, remove, or redefine any UART command.

- **Framework FSM API is not automatically exposed via UART.** The FSM is a
  pure in-process control-flow primitive. Exposing it over UART (e.g., a new
  command that queries FSM state) requires a separate Application command
  phase that evaluates the new command against the 12 UART TX scope guards.

- **New product command vocabulary requires a separate Application/product
  phase.** The current UART vocabulary is a devkit probe surface, not a
  product surface. Phase 12B does not start that planning.

- **`T` remains an Adapter probe evidence command, not a Framework sensor API.**
  `T` drives the `lis2dh` accelerometer read at Adapter level. A Framework
  sensor API would be a separate abstraction at the Framework layer; it is
  planned (Phase 12A §E.3) but not started here.

---

## M. Non-goals

Phase 12B explicitly does not:

- Create a `framework/` directory.
- Create any `.h` or `.c` Framework file.
- Modify any source, test, CMake, Zephyr, board, `prj.conf`, DTS overlay, or
  script file.
- Modify `devkit_app_state` or any devkit module.
- Change any UART command semantics (`a/s/r/?/x/v/L/d/T` unchanged).
- Add any new UART command or command candidate to `COMMAND_SET_DRAFT.md`.
- Expose the Framework FSM through a new UART query command.
- Add a parser, shell, registry, framing protocol, or response queue.
- Mutate the scheduler (`MAX_EVENTS_PER_TICK`, `QUEUE_CAPACITY`, or any
  admission-gate constant).
- Reopen F407 / custom board.
- Start ACTIVE disarm widening.
- Start Application/product semantics.
- Implement actuator, PID, motion primitive, or sensor Framework APIs.
- Mutate telemetry schema (`ROBOTOS_OBS`, `ROBOTOS_APP`, `ROBOTOS_FAULT` lines
  are frozen; no `ROBOTOS_FW` stream is created by Phase 12B).
- Perform any hardware run.
- Modify any evidence log.
- Modify `DEVKIT_PROGRESS.md` historical master.
- Push to remote.

---

## N. Decision Result and Open Items

### N.1 Decision result

**`PHASE_12B_FSM_API_DRAFTED_DOCS_ONLY`**

The Framework FSM API surface is defined at design level in this document and
in `FRAMEWORK_FSM_API_DRAFT.md`. The draft is substantially complete. Two
sub-decisions require confirmation before any Phase 12C implementation:

### N.2 Open decision 1 — Status model (EVENT_BRIDGE_PATTERN_SPEC_NEEDED)

**Item:** The Application-layer event bridge pattern (how Adapter events are
translated to `robotos_fw_event_id_t` values and delivered to
`robotos_fw_fsm_dispatch()`) is documented as a recommendation (§G.3) but has
not been confirmed as the adopted design.

**Impact:** Before Phase 12C writes the `robotos_fw_fsm_t` header and any
Application bridge stub, the event bridge interface contract must be confirmed
so the FSM's `dispatch()` call site is stable.

**Recommendation recorded:** Application-layer bridge receives Adapter events
from `robotos_core_register_event_handler()` callbacks and calls
`robotos_fw_fsm_dispatch()` with product-defined `robotos_fw_event_id_t`
values. No new `robotos_event_type_t` allocation required.

**Status:** `OPEN_RECOMMENDATION_PENDING_CONFIRMATION`

### N.3 Open decision 2 — Status model (`STATUS_MODEL_RECOMMENDATION_PENDING_CONFIRMATION`)

**Item:** Whether to reuse `robotos_core_status_t` (Option A, recommended)
or define a new `robotos_fw_status_t` (Option B) for the FSM return type.

**Impact:** The typedef `typedef robotos_core_status_t robotos_fw_status_t`
(Option A) can be written in the header without a new enum; switching to
Option B later would change the header. Phase 12C must confirm before writing
the header.

**Recommendation recorded:** Option A — reuse `robotos_core_status_t`. If
richer Framework-specific error codes are needed in a later phase, a new
`robotos_fw_status_t` enum can be added alongside without breaking the alias.

**Status:** `OPEN_RECOMMENDATION_PENDING_CONFIRMATION`

---

## O. Phase 12C Recommendation

### O.1 Recommended next gate

**Phase 12C — Framework FSM Event Bridge Spec + Status Model Confirmation
(docs-only).**

Phase 12C is a targeted docs-only phase that closes the two open decisions
from §N before any implementation. It does NOT create source files; it
produces a confirmed-decision addendum to `FRAMEWORK_FSM_API_DRAFT.md`.

### O.2 Entry criteria for Phase 12C

- Phase 12B is `CLOSED_DOCS_ONLY` (this document).
- Explicit user authorization to open Phase 12C.
- No blocking open decision on any other gate (Scheduler, F407, ACTIVE disarm
  are all decoupled).

### O.3 Exit criteria for Phase 12C

- Event bridge pattern confirmed: exact interface between Application Adapter
  event handler and `robotos_fw_fsm_dispatch()` call specified, including
  whether a separate Application bridge struct or simple inline call is used.
- Status model confirmed: `robotos_core_status_t` reuse (Option A) or
  `robotos_fw_status_t` new enum (Option B) decided.
- Event payload lifetime rule confirmed (§H.2.2).
- Action return semantics on non-OK confirmed (§H.2.3).
- `FRAMEWORK_FSM_API_DRAFT.md` Section 9 (Open Decisions) updated with
  all-confirmed status.
- `CLOSED_DOCS_ONLY`.

### O.4 Files likely touched by Phase 12C

| File | Change |
|---|---|
| `02_PHASE_CLOSEOUTS/PHASE_12C_FSM_EVENT_BRIDGE_STATUS_CONFIRMATION.md` | New |
| `03_SPECS/FRAMEWORK_FSM_API_DRAFT.md` | Update §9 open decisions to confirmed |
| `01_PROGRESS/DEVKIT_PROGRESS_PHASE_11_20.md` | Phase 12C entry + anchor |
| `CURRENT_STATE.md` | Update latest phase |
| `00_INDEX/README.md` | Phase 12C closeout link |

### O.5 Validation path for Phase 12C

- Docs-only: no source/config/evidence-log changes.
- No `framework/` dir created.
- No `.h`/`.c` files created.
- All open decisions in `FRAMEWORK_FSM_API_DRAFT.md §9` resolved.
- `CLOSED_DOCS_ONLY`.

### O.6 Explicit non-goals for Phase 12C

- Does not create `robotos_fw_fsm.h` or `.c` (that is Phase 12D-class).
- Does not create `framework/` dir.
- Does not implement FSM logic.
- Does not modify devkit source.
- Does not change UART commands.
- Does not start actuator/sensor/PID Framework work.
- Does not start Application/product layer.
