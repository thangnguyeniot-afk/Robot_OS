# Phase 12C -- Framework FSM Event Bridge + Status Model Confirmation

**Status:** `CLOSED_DOCS_ONLY`
**Type:** Docs-only design-confirmation phase. **No source, runtime, test,
CMake, Zephyr, board, host-tool, script, `prj.conf`, DTS overlay, evidence
log, or `framework/` directory change.** No `.h` or `.c` Framework file
created. Phase 12C confirms the four open Framework FSM design decisions
left by Phase 12B and corrects one detail in the Phase 12B evaluation-order
draft.
**Date opened/closed:** 2026-05-12 (same-day docs-only close)
**Branch:** master
**Published baseline at open:** `origin/master = cda3810`
**Prior closed phase:** Phase 12B (`CLOSED_DOCS_ONLY`; FSM API draft).
**Prior runtime implementation phase:** Phase 11D (firmware `2040bfb`).
**Prior hardware evidence phase:** Phase 11E (`10710b3`).
**Companion docs:**
[`PHASE_12B_FRAMEWORK_FSM_API_DRAFT.md`](PHASE_12B_FRAMEWORK_FSM_API_DRAFT.md),
[`PHASE_12A_FRAMEWORK_API_SURFACE_PLANNING.md`](PHASE_12A_FRAMEWORK_API_SURFACE_PLANNING.md),
[`../03_SPECS/FRAMEWORK_FSM_API_DRAFT.md`](../03_SPECS/FRAMEWORK_FSM_API_DRAFT.md)
(spec updated by this phase).

---

## A. Executive Summary

Phase 12C is a targeted docs-only phase that **confirms the four open design
decisions** left by Phase 12B:

1. **Event bridge pattern** — `APPLICATION_OWNED_EVENT_BRIDGE_CONFIRMED`
2. **Status model** — `REUSE_ROBOTOS_CORE_STATUS_T_FOR_PHASE_12D_CONFIRMED`
3. **Payload lifetime** — `PAYLOAD_BORROWED_FOR_DISPATCH_ONLY_CONFIRMED`
4. **Action non-OK semantics** — `ACTION_NON_OK_NO_ROLLBACK_CONFIRMED`

Phase 12C also records **one correction** to the Phase 12B evaluation-order
draft: state update now happens **before** the transition action, not after
(see §G.4). This change is consistent with the "no rollback on action
failure" policy and matches the canonical UML/Harel ordering where actions
run in the post-transition state context.

**Decision result: `PHASE_12C_EVENT_BRIDGE_STATUS_CONFIRMED_DOCS_ONLY`**

Phase 12C introduces **no source, header, or implementation**. It does not
create a `framework/` directory, does not start Phase 12D, does not modify
`devkit_app_state`, does not change command semantics, and does not start
Application/product work.

The long-lived spec
[`../03_SPECS/FRAMEWORK_FSM_API_DRAFT.md`](../03_SPECS/FRAMEWORK_FSM_API_DRAFT.md)
is updated by this phase to move §9 items from `OPEN` to `CONFIRMED` and to
record the §3.2 evaluation-order correction.

---

## B. Baseline Before Phase 12C

| Item | Value |
|---|---|
| `origin/master` at open | `cda3810` (`docs: add Phase 12B framework FSM API draft`) |
| Last closed phase | Phase 12B (`CLOSED_DOCS_ONLY`) |
| Phase 12B decision | `PHASE_12B_FSM_API_DRAFTED_DOCS_ONLY` |
| Robot Framework | **NOT BUILT** — no `framework/` dir, no Framework header, no API |
| Application / product | **NOT BUILT** |
| Framework FSM draft | `DRAFT / NON-FINAL`; §9 had 4 OPEN items |
| Validated command set | **`a / s / r / ? / x / v / L / d / T`** (unchanged) |
| Last runtime implementation phase | Phase 11D (`2040bfb`) |
| Last hardware evidence phase | Phase 11E (`10710b3`) |
| Hardware platform | STM32F411E-DISCO rev D |
| Hardware platform alternatives | F407 / custom board: `HOLD/DEFER` (unchanged) |
| Scheduler | `MAX_EVENTS_PER_TICK = 1`; `QUEUE_CAPACITY = 16`; 7A/7B `DEFER` |
| UART TX scope | Minimal response only; 12 scope-guard constraints intact |
| POST_FLASH_AUTOSTART | `OPEN` / `MITIGATED_BY_WORKFLOW` |
| Devkit event allocation | 100 (legacy), 101 (timer), 102 (button), 103 (UART) |

**Remaining open gates (all preserved unchanged at Phase 12C):**

1. ACTIVE disarm widening — `USER_DECISION_REQUIRED_ACTIVE_DISARM`
2. Scheduler 7A/7B — `DEFER`
3. F407 / custom board — `HOLD/DEFER`
4. POST_FLASH_AUTOSTART root cause — `OPEN` / `MITIGATED_BY_WORKFLOW`
5. Application / product layer — `NOT_STARTED`
6. Robot Framework implementation — `NOT_STARTED`; Phase 12D not started

**Note on pre-existing `src/framework/` files:** The repo contains pre-existing
files under `RobotOS_v1.0/src/framework/` (first committed at `43de448`,
the Zephyr devkit bring-up baseline). These are **not** the Robot Framework
described by this spec and are **not** modified by Phase 12C. Phase 12C
confirms that "Robot Framework is NOT BUILT" refers to the Framework FSM API
specified in `FRAMEWORK_FSM_API_DRAFT.md`; the pre-existing `src/framework/`
content is separate and outside Phase 12C scope.

---

## C. Confirmed Event Bridge Pattern

### C.1 Decision

**`APPLICATION_OWNED_EVENT_BRIDGE_CONFIRMED`**

The Application layer owns the translation from Adapter/core events to
Framework FSM logical events. The Framework FSM does not register directly
with `robotos_core_register_event_handler()`.

### C.2 Confirmed rules

- Adapter/core events remain in the existing `robotos_event_type_t` namespace
  (types 100–103 currently allocated by devkit producers; up to 8 handlers
  registrable per `ROBOTOS_CORE_MAX_EVENT_HANDLERS`).
- Framework FSM uses a **separate `robotos_fw_event_id_t` namespace**
  (`uint32_t`; product/application-defined values; `DRAFT / NON-FINAL` type
  name).
- A future application/devkit bridge maps Adapter events to Framework events
  before calling `robotos_fw_fsm_dispatch()`. The bridge runs in dispatch
  thread context (not ISR), reading the `robotos_event_t` it received from
  the core dispatcher.
- The Framework FSM **does not** consume raw `robotos_event_t` or
  `ROBOTOS_EVENT_*` values directly by default. If a future Application layer
  needs the FSM to accept Adapter events directly, that wiring is the
  Application's responsibility and is not part of the Framework FSM API.
- **No `ROBOTOS_EVENT_USER` subrange allocation is required for the FSM core
  itself.** A subrange decision may still be needed later if a specific
  Application bridge or producer registers new `robotos_event_type_t` values,
  but that is out of Phase 12C scope.
- Devkit-local event types (`USER+0`, `USER+1`, `USER+2`, `USER+3`) remain
  unaffected. Phase 12C does not modify any source.

### C.3 Why this design

| Concern | How the confirmed pattern addresses it |
|---|---|
| Avoids collision with devkit-local events | Framework FSM event IDs are a separate `uint32_t` namespace; no overlap with `robotos_event_type_t` values 0..n |
| Preserves Adapter/Framework separation | Adapter dispatch path is unchanged; Framework FSM sits above and consumes only logical events |
| Avoids changing core event enum/range in Phase 12C | Zero source change; `robotos_event_type_t` is read-only at this phase |
| Allows future apps to define logical event IDs without changing core | Each Application chooses its own `robotos_fw_event_id_t` constants; no central registry needed |
| Avoids premature Framework-producer coupling | Bridge is Application code; Framework FSM is product-neutral |

### C.4 What this confirmation does NOT do

- Does not specify the exact shape of the Application bridge (struct, function
  signature, registration mechanism). Bridge structure is product-specific
  and is not Framework API scope.
- Does not authorize creation of any Application bridge implementation. Bridge
  implementation is part of a future Application/product phase.
- Does not allocate any new `robotos_event_type_t` value.

---

## D. Event Bridge Flow (Conceptual)

The future event flow, confirmed at Phase 12C as the design target:

```
[ Hardware / producer ]
    | ISR posts robotos_event_t to core queue
    v
[ Core event queue ]
    | thread context: robotos_core_tick() -> dispatch
    v
[ Adapter dispatcher / registered handler ]
    | thread context, in user-registered handler for type N
    v
[ Application event bridge ]   <-- product-specific; not Framework
    | translates (robotos_event_t) -> (robotos_fw_event_id_t, payload)
    | thread context, no ISR
    v
[ robotos_fw_fsm_dispatch(fsm, event_id, payload) ]
    | thread context
    | evaluates transition table; calls guard/exit/action/entry
    v
[ Application user_context mutation, RTT log ]
```

### D.1 Confirmed flow contracts

- **Bridge runs in thread/dispatch context, not ISR.** The Application bridge
  must be invoked from a handler registered with
  `robotos_core_register_event_handler()`, which runs in the core dispatch
  thread.
- **Bridge owns translation.** The Application bridge has full freedom in how
  it maps Adapter event metadata (`type`, `arg0`, `arg1`, `timestamp_tick`)
  to a Framework logical event ID and optional payload pointer. Framework FSM
  imposes no mapping convention.
- **Framework FSM sees only product-neutral logical event ID.** The FSM has
  no knowledge of which Adapter event triggered the dispatch.
- **Framework FSM does not call UART TX.** All 12 scope guards from
  `PHASE_9EZ_CHECKPOINT.md §H` remain in force.
- **Framework FSM does not own hardware drivers.** Driver access is Adapter
  layer responsibility; if an action needs sensor/actuator data, the bridge
  attaches it to the payload before dispatch (subject to payload lifetime
  rules, §F).

---

## E. Confirmed Status Model

### E.1 Decision

**`REUSE_ROBOTOS_CORE_STATUS_T_FOR_PHASE_12D_CONFIRMED`**

The repo type name is `robotos_core_status_t` (see `core/robotos_core.h`).
Phase 12D FSM API will reuse this existing type for all FSM return values.

### E.2 Confirmed rules

- Phase 12D FSM API will use `robotos_core_status_t` directly. The
  `robotos_fw_status_t` typedef in the Phase 12B draft is **confirmed as a
  typedef alias** to `robotos_core_status_t` (Option A from Phase 12B §H.2.1).
  An alias is acceptable for clarity; a new enum is not introduced.
- **No new Framework-specific status enum in Phase 12D.** All FSM dispatch
  errors map onto existing `robotos_core_status_t` codes.
- **FSM-specific reasons (guard reject, no transition) are tracked via audit
  counters** (`guard_rejected_count`, `no_transition_count`) rather than
  distinct status codes. Both paths return `ROBOTOS_CORE_OK`.
- **If existing status model is insufficient later,** a future phase may
  introduce a richer `robotos_fw_status_t` enum, but **not in Phase 12C or
  Phase 12D**.
- **Phase 12C does not invent any new status code.**

### E.3 Status mapping draft (Phase 12D target)

| FSM situation | Returned `robotos_core_status_t` | Audit counter effect | Notes |
|---|---|---|---|
| OK / transition accepted | `ROBOTOS_CORE_OK` | `transition_count++` | Action returned OK (or no action defined) |
| No transition matched (no row found) | `ROBOTOS_CORE_OK` | `no_transition_count++` | Unhandled event is not an error |
| Guard rejected (returned `false`) | `ROBOTOS_CORE_OK` | `guard_rejected_count++` | Evaluation continues to next matching row; if all reject, `no_transition_count++` (separate counter increments are NOT both performed for the same dispatch — see §G.3) |
| Action returned non-OK | Action's status value (non-OK; e.g., `ROBOTOS_CORE_ERR_INVALID_ARG`) | `transition_count++` (transition already committed; see §G.4) | State is already updated; no rollback (§G) |
| Invalid config (NULL fsm/config/transitions, or `transition_count==0`, or `initial_state==UNINIT`) at init | `ROBOTOS_CORE_ERR_NULL` (for NULL pointers) or `ROBOTOS_CORE_ERR_INVALID_ARG` (for bad values) | None | Detected in `init()` |
| NULL `fsm` argument to any function | `ROBOTOS_CORE_ERR_NULL` | None | Defensive NULL check |
| Uninitialized FSM passed to `dispatch`/`reset`/`get_snapshot` | `ROBOTOS_CORE_ERR_INVALID_STATE` | None | FSM not initialized |
| NULL `out` snapshot pointer in `get_snapshot` | `ROBOTOS_CORE_ERR_NULL` | None | |

The codes referenced (`OK`, `ERR_NULL`, `ERR_INVALID_STATE`, `ERR_INVALID_ARG`)
are all present in `robotos_core_status_t` as defined in
`core/robotos_core.h`. No code mapping requires `STATUS_MAPPING_NEEDS_PHASE_12D_REPO_CHECK`.

### E.4 Guard return-type confirmation

The Phase 12B draft has guards as `typedef bool (*robotos_fw_guard_fn_t)(...)`.
Phase 12C **confirms this signature unchanged**: guards return `bool` only.
The "guard non-OK" concept from the Phase 12C task framing is **not adopted**
because:

- Guards are by contract pure predicates with no side effects (§I).
- Any guard-side error condition (e.g., context inaccessible) is handled
  inside the guard by logging to RTT and returning `false`.
- A status-returning guard would complicate the dispatch logic (mixing
  predicate result with error propagation) without a concrete current use
  case.

If a real use case for guard-side errors emerges later, a status-returning
guard variant can be added as a separate function pointer type without
breaking the existing API. Phase 12C does not introduce that variant.

---

## F. Confirmed Payload Lifetime Rules

### F.1 Decision

**`PAYLOAD_BORROWED_FOR_DISPATCH_ONLY_CONFIRMED`**

### F.2 Confirmed rules

- The `event_payload` parameter to `robotos_fw_fsm_dispatch()` is a
  **borrowed `const void *`**.
- Payload must remain valid only for the duration of the
  `robotos_fw_fsm_dispatch()` call.
- The FSM must not store the payload pointer after `dispatch()` returns. It
  may be passed to guard, exit action, transition action, and entry action,
  all of which complete before `dispatch()` returns.
- Actions and guards may **read** the payload during dispatch.
- If an action needs durable state derived from payload data, the action must
  **copy** the necessary fields into the caller-owned `user_context` struct
  before returning.
- **No heap allocation.** The FSM never allocates payload storage.
- **No framework-owned payload lifetime.** All payload memory is owned by the
  Application/bridge.
- **ISR-owned buffers must not be passed unless either (a) the bridge has
  copied them into a stable buffer before dispatch, or (b) the bridge can
  guarantee the ISR will not free/reuse the buffer until dispatch returns.**
  The bridge is responsible for this safety.

### F.3 Implications by data class

| Data class | Implication |
|---|---|
| Sensor data (e.g. `T` accelerometer reading) | Bridge must read sensor (Adapter-level call), copy reading into a stable struct, then pass pointer-to-struct as payload. Struct memory lives in bridge or user_context. |
| UART command data (single byte) | Bridge can pass the byte value via payload OR encode it into the logical `event_id`. Either is acceptable; convention is product-specific. |
| Timer events | Timer payload (tick number, timer ID) is typically value-encoded into the event itself or read directly from core state; payload pointer may be NULL. |
| Future application bridge | Bridge owns all payload memory; FSM treats payload as opaque read-only. |

### F.4 What this does NOT change

- Does not change the existing core `robotos_event_t` lifetime rule (already
  documented as "event is valid for the duration of the call only" in
  `core/robotos_event_dispatcher.h`).
- Does not introduce a payload-copy convention in the Framework FSM API
  (copying is Application bridge responsibility).

---

## G. Confirmed Action / Guard Return Semantics

### G.1 Decision

**`ACTION_NON_OK_NO_ROLLBACK_CONFIRMED`** plus a Phase 12C correction to the
evaluation-order draft from Phase 12B §E.3 (see §G.4 below).

### G.2 Confirmed guard rules

- **Guard returns `bool`** (Phase 12B signature confirmed unchanged).
- **Guard returns `true`:** transition proceeds (subject to remaining
  pipeline).
- **Guard returns `false`:** no transition is taken for this matching row;
  `guard_rejected_count++`; evaluation continues to the next matching row.
- If all matching rows have rejecting guards, the dispatch ends with
  `no_transition_count++` and returns `ROBOTOS_CORE_OK`.
- Guards must not mutate `user_context`, must not call UART TX, must not
  block, must not allocate heap, and must not call sensor/driver APIs
  (§I in `PHASE_12B_FRAMEWORK_FSM_API_DRAFT.md`).

### G.3 Counter semantic clarification (Phase 12C)

The Phase 12B draft `FRAMEWORK_FSM_API_DRAFT.md §7.2` left two counters
adjacent:

- `guard_rejected_count` — incremented per guard rejection
- `no_transition_count` — incremented when the dispatch finds no transition

Phase 12C **confirms** the per-dispatch semantic:

- If at least one matching row has a passing guard → transition runs; only
  `transition_count++`. Rejected guards on earlier rows in the same dispatch
  **still increment** `guard_rejected_count` per rejection.
- If no matching row found at all → only `no_transition_count++`.
- If matching rows existed but all guards rejected → `no_transition_count++`
  **and** `guard_rejected_count` is incremented once per rejection during the
  scan.

Both counters are **independent**; they count distinct events and may both
increment in a single dispatch when matching rows exist but all guards reject.

### G.4 Phase 12C correction to evaluation order

The Phase 12B draft (`PHASE_12B_FRAMEWORK_FSM_API_DRAFT.md §E.3` step 5 and
`FRAMEWORK_FSM_API_DRAFT.md §3.2`) had:

> exit → **action** → **state update** → entry

Phase 12C **corrects** this to:

> exit → **state update** → **action** → entry

### G.4.1 Rationale for the correction

- **Consistency with no-rollback policy.** "If action fails after state
  update, FSM remains in the target state" is the natural reading only if
  state update happens before action runs. With Phase 12B's old order, the
  action runs while `current_state` is still the old value — making the
  phrase "after state update" semantically awkward.
- **Action sees post-transition state.** When the action callback queries
  the FSM (e.g., for observability or to post a follow-up event), it sees
  the new state, which matches caller intuition.
- **Entry action invariant preserved.** Entry action still runs in
  `next_state`; the only change is that the transition action now also runs
  in `next_state`, immediately before entry.

### G.4.2 Confirmed evaluation order (Phase 12C; supersedes Phase 12B §E.3)

```
robotos_fw_fsm_dispatch(fsm, event_id, payload):

  event_count++
  last_event_id = event_id

  scan transition table (FIFO, index 0..N-1):
    for each row where row.current_state == fsm->current_state
                   AND row.event_id      == event_id:
      if row.guard != NULL and row.guard(...) == false:
        guard_rejected_count++
        continue                     -- try next matching row
      -- Guard passed (or no guard). Commit the transition.
      from_state = fsm->current_state
      if state_def[from_state].on_exit != NULL:
        on_exit(from_state, user_ctx)
      fsm->current_state = row.next_state            -- (A) state update
      if row.action != NULL:
        action_status = action(from, row.next_state, event, payload, user_ctx)
      else:
        action_status = OK
      if state_def[row.next_state].on_entry != NULL:
        on_entry(row.next_state, user_ctx)           -- runs regardless of action_status
      transition_count++
      last_status = action_status
      return action_status

  -- No row matched, or all matching rows had rejecting guards
  no_transition_count++
  last_status = OK
  return OK
```

Key invariants:

| Invariant | Holds because |
|---|---|
| Action runs in new state context | `current_state` set at (A) before action call |
| Action failure does not roll back state | No reverse of (A) on action non-OK |
| Entry runs after action regardless of action result | Entry call is unconditional once transition is committed |
| `transition_count` reflects committed transitions only | Increment is after (A) and inside the for-row block |
| Per-dispatch single transition | Loop returns after first committed transition |

### G.5 Behavior on action failure (no rollback)

- Action returns non-OK after the state update at (A):
  - `current_state` remains at the new target state.
  - Entry action still runs.
  - `transition_count++`.
  - `last_status = action_status`.
  - `dispatch()` returns the action's non-OK status to the caller.
- The Application is responsible for monitoring `last_status` (or the
  `dispatch()` return value) and taking corrective action if needed.
- **No retry inside Framework FSM.** Retry is the Application's
  responsibility.
- **No implicit fault escalation.** Phase 12C does not authorize automatic
  transition to a fault state on action failure. A product that wants such
  behavior must define an explicit fault state and a transition row to it.

---

## H. Transition Ordering / Atomicity

Phase 12C confirms the following ordering and atomicity constraints
(consistent with Phase 12B and the §G.4 correction):

| Constraint | Status |
|---|---|
| Deterministic first-match order during table scan | Confirmed |
| Flat FSM only — no hierarchy | Confirmed |
| No concurrent regions | Confirmed |
| No scheduler mutation by FSM dispatch | Confirmed |
| `dispatch()` is **not** ISR-safe | Confirmed; dispatch runs in thread context only |
| State query / snapshot may use critical section for ISR-safe read | Confirmed; future implementation will use `robotos_platform_critical_enter/exit` for `get_state` and `is_in_state` only |
| State update + counter increments **inside the dispatch loop body** may use a critical section if FSM is shared across contexts | Confirmed as Phase 12D implementation discretion; not required at API level |
| No long critical section around action callbacks | Confirmed — actions run **outside** any critical section; only the field-update steps may be protected |

---

## I. Observability Confirmation

Phase 12C confirms what Phase 12D may track conceptually (already drafted in
Phase 12B `FRAMEWORK_FSM_API_DRAFT.md §7.2`; restated here as confirmed for
Phase 12D):

| Counter | Increments when | Phase 12C status |
|---|---|---|
| `current_state` | (Not a counter; state field) | Confirmed |
| `transition_count` | Committed transition (after state update) | Confirmed |
| `event_count` | Every `dispatch()` call | Confirmed |
| `last_event_id` | Set on every `dispatch()` call | Confirmed |
| `last_status` | Set on every `dispatch()` return | Confirmed |
| `guard_rejected_count` | Each guard rejection during scan | Confirmed (§G.3) |
| `no_transition_count` | Dispatch ends with no committed transition | Confirmed (§G.3) |

### I.1 Optional `previous_state` field

The Phase 12C closeout records as **deferred**: a `previous_state` field on
the FSM struct that records the last from-state. Reason: not strictly
required for Phase 12D minimal viability; can be added later without
breaking the API. Not in scope for Phase 12C confirmation.

### I.2 What is NOT changed by Phase 12C

- **No UART query expansion.** The `?` response shape is frozen at Phase 11Z
  and unchanged by Phase 12C.
- **No telemetry schema mutation.** `ROBOTOS_OBS`, `ROBOTOS_APP`,
  `ROBOTOS_FAULT`, `ROBOTOS_PROD` log lines are unchanged.
- **No new RTT fields.** No `ROBOTOS_FW` log stream is created by Phase 12C.
- **Observability remains internal API/snapshot draft only.** A future phase
  (post-12D implementation) may add a `ROBOTOS_FW` periodic log line.

---

## J. Update to FRAMEWORK_FSM_API_DRAFT.md

Phase 12C updates `RobotOS_v1.0/devkit/docs/03_SPECS/FRAMEWORK_FSM_API_DRAFT.md`
as follows (no other spec file changed):

| Section | Phase 12C edit |
|---|---|
| Header revision line | Updated from "Phase 12B" to "Phase 12C" with date and `CLOSED_DOCS_ONLY` |
| §1 Decision state table | All four §9 items moved from `OPEN` to `CONFIRMED at Phase 12C` |
| §3.2 Evaluation sequence | Reordered to exit → state update → action → entry per §G.4 correction; pseudo-code updated |
| §6.2 Action contract | "Non-OK action propagates" sentence updated to clarify no-rollback per Phase 12C confirmation |
| §6.3 Entry / exit | "Entry called after state update" wording made consistent with new order |
| §7.2 Counter semantics | Added Phase 12C clarification on dual-increment of `guard_rejected_count` and `no_transition_count` |
| §9 Open decisions | All four items moved to CONFIRMED status with Phase 12C reference |
| §10 Next revision conditions | Updated to mark Phase 12C as "current revision"; Phase 12D listed as next revision trigger |

**The spec retains its `DRAFT / NON-FINAL` label.** Phase 12C does not freeze
function names or types — that is Phase 12D scope. Phase 12C only confirms
the *behavior contracts*.

---

## K. Non-goals

Phase 12C explicitly does not:

- Create a `framework/` directory.
- Create any `.h` or `.c` Framework file.
- Modify any source, test, CMake, Zephyr, board, `prj.conf`, DTS overlay, or
  script file.
- Modify any pre-existing `src/framework/*.c` file (these pre-existing files
  are outside Phase 12C scope; see §B).
- Modify `devkit_app_state` or any devkit module.
- Move or alias `devkit_app_state` into Framework.
- Change any UART command semantics (`a/s/r/?/x/v/L/d/T` unchanged).
- Add any new UART command or command candidate.
- Expose the Framework FSM through a new UART query command.
- Add a parser, shell, registry, framing protocol, or response queue.
- Mutate scheduler (`MAX_EVENTS_PER_TICK`, `QUEUE_CAPACITY`).
- Reopen F407 / custom board.
- Start ACTIVE disarm widening.
- Start Application/product semantics.
- Implement actuator, PID, motion, or sensor Framework APIs.
- Mutate telemetry schema (`ROBOTOS_OBS`/`ROBOTOS_APP`/`ROBOTOS_FAULT`
  unchanged; no `ROBOTOS_FW` stream created).
- Perform any hardware run.
- Modify any evidence log.
- Modify `DEVKIT_PROGRESS.md` historical master.
- Start Phase 12D.
- Push to remote.

---

## L. Decision Result

**`PHASE_12C_EVENT_BRIDGE_STATUS_CONFIRMED_DOCS_ONLY`**

All four open decisions from Phase 12B are now CONFIRMED:

1. Event bridge pattern → `APPLICATION_OWNED_EVENT_BRIDGE_CONFIRMED` (§C)
2. Status model → `REUSE_ROBOTOS_CORE_STATUS_T_FOR_PHASE_12D_CONFIRMED` (§E)
3. Payload lifetime → `PAYLOAD_BORROWED_FOR_DISPATCH_ONLY_CONFIRMED` (§F)
4. Action non-OK semantics → `ACTION_NON_OK_NO_ROLLBACK_CONFIRMED` (§G)

Additional Phase 12C confirmation:
- Guard return type → `GUARD_RETURNS_BOOL_ONLY_CONFIRMED` (§E.4)
- Evaluation order → corrected to exit → state update → action → entry
  (§G.4)

`FRAMEWORK_FSM_API_DRAFT.md §9` reflects all CONFIRMED entries. The spec
remains labeled `DRAFT / NON-FINAL` because function names and types are not
locked until Phase 12D.

---

## M. Phase 12D Recommendation

### M.1 Recommended next gate

**Phase 12D — Framework FSM Header Stub / Compile-only Skeleton.**

All four decision prerequisites are CONFIRMED. Phase 12D may proceed
**only on explicit user authorization**, since it would be the first source/
header file to enter the Framework layer.

### M.2 Entry criteria for Phase 12D

- Phase 12C is `CLOSED_DOCS_ONLY` (this document).
- Event bridge pattern: CONFIRMED (§C).
- Status model: CONFIRMED (§E).
- Payload lifetime: CONFIRMED (§F).
- Action return semantics: CONFIRMED (§G).
- **Explicit user authorization required** to introduce the first
  Framework-layer source/header file.
- User direction on the exact Framework layer path. Options to discuss
  before opening Phase 12D:
  - Path A: `RobotOS_v1.0/framework/` — new top-level Framework layer
    directory parallel to `core/`, `platform/`, `devkit/`.
  - Path B: under an existing path (only if a clear repo convention dictates;
    this requires user decision because the pre-existing `src/framework/`
    is unrelated and must not be conflated).

### M.3 Likely files touched by Phase 12D

| File | Change |
|---|---|
| `<framework-dir>/robotos_fw_fsm.h` | NEW — compile-only header stub; locked names; behavior contracts in comments only; no `.c` body |
| `02_PHASE_CLOSEOUTS/PHASE_12D_FSM_HEADER_STUB.md` | New closeout |
| `03_SPECS/FRAMEWORK_FSM_API_DRAFT.md` | Update §4 names from DRAFT to LOCKED-AT-12D |
| `01_PROGRESS/DEVKIT_PROGRESS_PHASE_11_20.md` | Phase 12D entry + anchor |
| `CURRENT_STATE.md` | Update latest phase |
| `00_INDEX/README.md` | Phase 12D closeout link |

### M.4 Validation path for Phase 12D

- `git diff --check` clean.
- Exactly one new `.h` file under the agreed Framework path; **no `.c` body**.
- Header compiles standalone (compile-only check, not link). The compile
  check is a **bounded** Phase 12D validation: it confirms type definitions
  parse correctly, not behavior.
- No `core/`, `platform/`, or `devkit/` source modified.
- No `prj.conf`, CMake, DTS, board, or script change.
- No evidence log change.
- `devkit_app_state` unchanged.
- Validated command set unchanged.
- All 12 UART TX scope guards intact.

### M.5 Explicit non-goals for Phase 12D

- **No `.c` implementation file.** Phase 12D is header stub only — no FSM
  dispatch logic body, no transition evaluation code.
- **No devkit_app_state replacement.** The Framework FSM does not replace,
  shadow, or alias `devkit_app_state`. `devkit_app_state` remains as Phase
  11Z left it.
- **No UART command integration.** No new UART command exposes Framework
  FSM state.
- **No hardware run.** Phase 12D is build-time-only.
- **No scheduler change.** `MAX_EVENTS_PER_TICK` and `QUEUE_CAPACITY`
  unchanged.
- **No product behavior.** The header stub defines API surface; it does not
  bind to any specific product state machine.
- **No `.c` body** even as a stub. Phase 12D is header only; the
  implementation phase (12E or later) is separate.

### M.6 Alternative recommendation: Hold

If the user does not wish to introduce a Framework source/header file at this
time, **Hold** is fully acceptable. Phase 12B and Phase 12C have brought the
Framework FSM design to a state where header creation is the next mechanical
step — but holding does not invalidate any prior work. The design is durable;
the implementation can wait.

---

## N. What this document does not do

- Does not create any Framework `.h` or `.c` file.
- Does not create a `framework/` directory.
- Does not modify any source, test, CMake, Zephyr, board, `prj.conf`, DTS
  overlay, runtime script, host tool, or evidence log.
- Does not modify any pre-existing `src/framework/*.c` file.
- Does not implement the FSM API.
- Does not modify `devkit_app_state` or promote it to Framework.
- Does not change UART command semantics or add new commands.
- Does not start Phase 12D.
- Does not authorize hardware purchase.
- Does not authorize ACTIVE disarm widening.
- Does not reopen Scheduler 7A/7B.
- Does not reopen F407 / custom board.
- Does not change POST_FLASH_AUTOSTART status.
- Does not start Application/product semantics.
- Does not modify `DEVKIT_PROGRESS.md` historical master.
- Does not modify any evidence log under `RobotOS_v1.0/devkit/logs/`.
- Does not push.
