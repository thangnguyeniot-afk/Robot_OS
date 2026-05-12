# RobotOS Framework FSM API — Draft Spec

**Status:** `DRAFT / EXPERIMENTAL — IMPLEMENTED_AT_12E (HOST-TEST EVIDENCE)`.
Function names and parameter shapes are frozen at the Phase 12D header
stub (LOCKED-AT-12D). Function bodies exist as of Phase 12E and are
host-test-validated (93/93 assertions). **ABI is still NOT stable**;
the implementation has not been exercised against devkit or hardware,
and behavioral guarantees beyond the Phase 12B/12C contracts plus
Phase 12E §D.5 entry/exit non-OK choice are NOT promised.
**Revision:** Phase 12E (2026-05-12, `CLOSED_WITH_HOST_TEST_EVIDENCE`) —
first implementation `.c` body added at
`RobotOS_v1.0/framework/robotos_fw_fsm.c`; host-test-validated under
WSL Ubuntu / gcc 13.3.0; §1 decision-state table extended with
`IMPLEMENTED_AT_12E` row; entry/exit non-OK behavior committed.
**Prior revision:** Phase 12D (2026-05-12, `CLOSED_HEADER_STUB_ONLY`) —
header stub created; §4 names move from DRAFT to LOCKED-AT-12D.
**Earlier revision:** Phase 12C (2026-05-12, `CLOSED_DOCS_ONLY`) — confirmed
event bridge pattern, status model, payload lifetime, action return
semantics, and corrected evaluation order.
**Original draft:** Phase 12B (2026-05-12, `CLOSED_DOCS_ONLY`) — initial
draft of names and types.
**Next revision condition:** Phase 12F or later — Application bridge
planning, additional FSM host behavior, or hold; devkit integration
remains a multi-phase ask gated by scope-guard #11 (`devkit_app_state`)
and the `NOT_STARTED` status of the Application/product layer.

> **Active Framework path: `RobotOS_v1.0/framework/`** (Architecture A).
> Phase 12D created this directory with `README.md` and the header stub
> `robotos_fw_fsm.h`. **Phase 12E added `robotos_fw_fsm.c`** — the first
> active Framework `.c` body. **No devkit integration exists. No hardware
> evidence exists.** The pre-existing `RobotOS_v1.0/src/framework/*.c`
> files (committed at `43de448` and classified
> `LEGACY_SCAFFOLD_MARKED_FROZEN_DOCS_ONLY` at Phase 12D-pre) are part of
> frozen Architecture B and are not modified by any Phase
> 12A/12B/12C/12D-pre/12D/12E-pre/12E activity.

This spec is extracted from
[`../02_PHASE_CLOSEOUTS/PHASE_12B_FRAMEWORK_FSM_API_DRAFT.md`](../02_PHASE_CLOSEOUTS/PHASE_12B_FRAMEWORK_FSM_API_DRAFT.md)
and updated by
[`../02_PHASE_CLOSEOUTS/PHASE_12C_FSM_EVENT_BRIDGE_STATUS_MODEL.md`](../02_PHASE_CLOSEOUTS/PHASE_12C_FSM_EVENT_BRIDGE_STATUS_MODEL.md).
Behavior contracts are CONFIRMED at Phase 12C; function names and types
remain DRAFT pending Phase 12D.

---

## 1. Status / Scope

**What this doc is:**
A long-lived spec draft for the RobotOS Robot Framework flat FSM API.
It accumulates decisions from Phase 12B, 12C, and beyond until an
implementation phase (Phase 12D-class) is authorized.

**What this doc is not:**
- A final ABI. No function name or type is final until Phase 12C confirms.
- An implementation record. No source file maps to this spec yet.
- A promotion of `devkit_app_state`. That module remains devkit-local;
  this spec describes a new, separate, product-neutral Framework layer.

**Current decision state (Phase 12D):**

| Decision | Status |
|---|---|
| Model: flat FSM, table-driven, no heap | CONFIRMED at Phase 12B |
| State/event IDs: product-defined `uint32_t` | CONFIRMED at Phase 12B |
| No `robotos_event_type_t` sub-range needed for FSM core | CONFIRMED at Phase 12B |
| Event bridge pattern | CONFIRMED at Phase 12C — `APPLICATION_OWNED_EVENT_BRIDGE_CONFIRMED` |
| Status model — reuse `robotos_core_status_t` | CONFIRMED at Phase 12C — `REUSE_ROBOTOS_CORE_STATUS_T_FOR_PHASE_12D_CONFIRMED` |
| Payload lifetime rules | CONFIRMED at Phase 12C — `PAYLOAD_BORROWED_FOR_DISPATCH_ONLY_CONFIRMED` |
| Action return semantics on non-OK | CONFIRMED at Phase 12C — `ACTION_NON_OK_NO_ROLLBACK_CONFIRMED` |
| Guard return type (`bool` only) | CONFIRMED at Phase 12C — `GUARD_RETURNS_BOOL_ONLY_CONFIRMED` |
| Evaluation order (exit → state update → action → entry) | CONFIRMED at Phase 12C — corrects Phase 12B draft order |
| §4 API surface (names, parameter shapes) | **LOCKED-AT-12D** — header at `RobotOS_v1.0/framework/robotos_fw_fsm.h`; ABI still not stable |
| Implementation (`.c` body) | **`IMPLEMENTED_AT_12E`** — first body at `RobotOS_v1.0/framework/robotos_fw_fsm.c`; host-test-validated (93/93 assertions) under WSL Ubuntu / gcc 13.3.0 |
| Entry / exit non-OK behavior | **`IMPLEMENTED_AT_12E`** — entry/exit return values observed by FSM but **not propagated** by `dispatch()`/`init()`/`reset()`; action return is the sole driver of `last_status` and `dispatch()` return. Phase 12E design decision; revisable in a future API revision phase. |
| Devkit integration | `NOT_STARTED` — gated by scope-guard #11 (`devkit_app_state`) and Application/product layer `NOT_STARTED` |
| Hardware evidence | `NOT_STARTED` — Phase 12E close is `CLOSED_WITH_HOST_TEST_EVIDENCE`, not hardware |

---

## 2. Layer Boundary

```
[ Application / product layer    ]  defines state IDs, event IDs, guards,
                                    actions; owns event bridge

[ Robot Framework layer          ]  robotos_fw_fsm_* — product-neutral
    [this spec]                     FSM engine; no domain vocabulary

[ Robot Adapter / runtime        ]  core/ + platform/ + devkit hardware glue
  substrate                         (devkit_app_state is NOT Framework)

[ Kernel / HW                    ]  Zephyr + STM32F411E-DISCO
```

**Boundary assertions:**

- Framework FSM does not receive `robotos_event_t` events directly. It
  receives `robotos_fw_event_id_t` logical events from an Application bridge.
- Framework FSM must not call UART TX (all 12 scope guards from
  `PHASE_9EZ_CHECKPOINT.md §H` remain in force).
- Framework FSM must not own GPIO/sensor/actuator drivers.
- Framework FSM is product-neutral: no built-in state or event vocabulary.
- `devkit_app_state` is NOT Framework (scope-guard #11 not lifted).

---

## 3. FSM Concepts

### 3.1 Core concepts

| Concept | Description |
|---|---|
| **State** | Named operating mode; `uint32_t` ID; product-defined; 0 = uninitialized sentinel |
| **Event** | Logical trigger; `uint32_t` ID; application-defined; no Framework-imposed values |
| **Guard** | Optional pure predicate on a transition row; returns `true` (allow) / `false` (reject); no side effects |
| **Action** | Optional callback on a transition row; may mutate `user_context`; returns status |
| **Entry action** | Optional per-state callback; called after state update |
| **Exit action** | Optional per-state callback; called before action |
| **Transition row** | Tuple: `(current_state, event_id, guard, next_state, action)` |
| **No-transition** | No matching row found or all guards reject; event counted but OK returned |

### 3.2 Evaluation sequence (Phase 12C confirmed; corrects Phase 12B order)

**Phase 12C correction:** state update happens **before** the transition
action (not after, as drafted in Phase 12B). Rationale recorded in
[`../02_PHASE_CLOSEOUTS/PHASE_12C_FSM_EVENT_BRIDGE_STATUS_MODEL.md §G.4`](../02_PHASE_CLOSEOUTS/PHASE_12C_FSM_EVENT_BRIDGE_STATUS_MODEL.md):
consistent with no-rollback policy; action runs in post-transition state
context.

```
dispatch(fsm, event_id, payload):
  event_count++
  last_event_id = event_id
  scan transition table (FIFO):
    for each row where current_state matches AND event_id matches:
      if guard != NULL and guard() == false:
        guard_rejected_count++ ; continue
      → on_exit(current_state)              [if state def exists]
      → current_state = row.next_state      ← STATE UPDATE (Phase 12C order)
      → action(from, to, event, payload, ctx)   [if non-NULL]
      → on_entry(next_state)                [if state def exists]
      → transition_count++
      → last_status = action_status (or OK)
      return action_status (or OK)
  if no row matched or all matching rows had rejecting guards:
    no_transition_count++
    last_status = OK
    return OK
```

Key invariants (Phase 12C):
- Action runs in **new** state context (state already updated).
- Action failure does **not** roll back state.
- Entry action runs regardless of action status.
- `transition_count` reflects committed transitions only.
- `guard_rejected_count` and `no_transition_count` are **independent**;
  both may increment in a single dispatch when matching rows exist but all
  guards reject.

### 3.3 Model constraints (confirmed at Phase 12B)

- **Flat only.** No nested states, no history, no concurrent regions.
- **Static config.** Transition table is compile-time array; no runtime
  addition or removal of rows.
- **No heap.** FSM object is caller-owned static struct.
- **No scheduler mutation.** Single FSM + `MAX_EVENTS_PER_TICK=1` fits
  current dispatch budget.
- **Thread-context dispatch.** `dispatch()` always called from Adapter event
  handler (thread context); never from ISR.
- **ISR-safe state query.** `get_state()` and `is_in_state()` use critical
  section for safe concurrent read.
- **First-match FIFO.** Deterministic; the order of transition table rows
  determines priority when multiple rows match.

---

## 4. API Surface — LOCKED-AT-12D

**Phase 12D status:** names and parameter shapes in this section are
**LOCKED-AT-12D** and exist as a header stub at
`RobotOS_v1.0/framework/robotos_fw_fsm.h`. The header declares all
symbols listed below; **no function body exists**. ABI is **NOT** stable.
Behavioral guarantees beyond the Phase 12B/12C contracts are **NOT**
promised. An implementation `.c` body is a future phase (Phase 12E or
later) gated on explicit user authorization and a concrete consumer or
unit test.

> The illustrative C blocks below are reproduced from the Phase 12B
> draft for reference. The authoritative header is
> `RobotOS_v1.0/framework/robotos_fw_fsm.h`; if this spec and that header
> ever disagree, the header wins until a future revision phase resolves
> the conflict.

### 4.1 Types

```c
/* DRAFT / NON-FINAL */

typedef uint32_t robotos_fw_state_id_t;
typedef uint32_t robotos_fw_event_id_t;

/* Reserved state ID (uninit sentinel): */
#define ROBOTOS_FW_STATE_UNINIT  ((robotos_fw_state_id_t)0u)

/* Status type — OPEN DECISION: see §9.2 */
typedef robotos_core_status_t robotos_fw_status_t;  /* Option A; pending Phase 12C */

typedef bool (*robotos_fw_guard_fn_t)(
    robotos_fw_state_id_t current_state,
    robotos_fw_event_id_t event_id,
    const void *event_payload,
    void *user_context);

typedef robotos_fw_status_t (*robotos_fw_action_fn_t)(
    robotos_fw_state_id_t from_state,
    robotos_fw_state_id_t to_state,
    robotos_fw_event_id_t event_id,
    const void *event_payload,
    void *user_context);

typedef robotos_fw_status_t (*robotos_fw_entry_exit_fn_t)(
    robotos_fw_state_id_t state_id,
    void *user_context);

typedef struct {
    robotos_fw_state_id_t  current_state;
    robotos_fw_event_id_t  event_id;
    robotos_fw_guard_fn_t  guard;       /* NULL = always allow */
    robotos_fw_state_id_t  next_state;
    robotos_fw_action_fn_t action;      /* NULL = no action */
} robotos_fw_transition_t;

typedef struct {
    robotos_fw_state_id_t      state_id;
    robotos_fw_entry_exit_fn_t on_entry; /* NULL = no entry action */
    robotos_fw_entry_exit_fn_t on_exit;  /* NULL = no exit action */
} robotos_fw_state_def_t;

typedef struct {
    const robotos_fw_transition_t *transitions;
    uint32_t                       transition_count;
    const robotos_fw_state_def_t  *states;       /* NULL if no entry/exit */
    uint32_t                       state_count;
    robotos_fw_state_id_t          initial_state;
    void                          *user_context;
} robotos_fw_fsm_config_t;

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

### 4.2 Functions

```c
/* DRAFT / NON-FINAL */

/* Initialize FSM. Calls on_entry(initial_state) if state def exists.
 * Returns ERR_NULL if fsm/config/transitions NULL.
 * Returns ERR_INVALID_ARG if transition_count==0 or initial_state==UNINIT.
 * Returns OK on success. Thread context only. */
robotos_fw_status_t robotos_fw_fsm_init(
    robotos_fw_fsm_t *fsm,
    const robotos_fw_fsm_config_t *config);

/* Dispatch logical event to FSM. See §3.2 for evaluation sequence.
 * Returns OK on success or unhandled event.
 * Returns ERR_NULL if fsm NULL. Returns ERR_INVALID_STATE if not init.
 * Returns action status on action failure (transition committed).
 * Thread context only; never ISR. */
robotos_fw_status_t robotos_fw_fsm_dispatch(
    robotos_fw_fsm_t *fsm,
    robotos_fw_event_id_t event_id,
    const void *event_payload);

/* Return current state. ISR-safe (under critical section).
 * Returns ROBOTOS_FW_STATE_UNINIT if fsm NULL or not initialized. */
robotos_fw_state_id_t robotos_fw_fsm_get_state(
    const robotos_fw_fsm_t *fsm);

/* Reset FSM to initial_state. Zeroes all counters.
 * Calls on_exit(current_state) then on_entry(initial_state) if defined.
 * Thread context only. */
robotos_fw_status_t robotos_fw_fsm_reset(
    robotos_fw_fsm_t *fsm);

/* ISR-safe predicate: true if current_state == state_id.
 * Returns false if fsm NULL or not initialized. */
bool robotos_fw_fsm_is_in_state(
    const robotos_fw_fsm_t *fsm,
    robotos_fw_state_id_t state_id);

/* Copy all counters and state into snapshot. Thread context only.
 * Returns ERR_NULL if fsm or out NULL.
 * Returns ERR_INVALID_STATE if fsm not initialized. */
robotos_fw_status_t robotos_fw_fsm_get_snapshot(
    const robotos_fw_fsm_t *fsm,
    robotos_fw_fsm_snapshot_t *out);
```

---

## 5. Event / State Representation

### 5.1 Event ID namespace

`robotos_fw_event_id_t` is a `uint32_t` defined by the Application layer.
It occupies a **separate namespace** from `robotos_event_type_t`. No
`ROBOTOS_EVENT_USER` sub-range allocation is required. The Application bridge
translates Adapter events (type 100–103) to Framework logical event IDs.

### 5.2 Existing devkit event type allocation (reference, read-only)

| `robotos_event_type_t` value | Module | Status |
|---|---|---|
| 100 (`USER + 0`) | `devkit_runtime.c` (legacy) | Active (legacy handler) |
| 101 (`USER + 1`) | `devkit_timer_producer.c` | Active |
| 102 (`USER + 2`) | `devkit_button_producer.c` | Active |
| 103 (`USER + 3`) | `devkit_uart_producer.c` | Active |

Framework FSM does not use any of these directly.

### 5.3 State ID conventions

- `0` = `ROBOTOS_FW_STATE_UNINIT` sentinel; not a valid product state.
- Product states start at `1`; values are product-chosen.
- Framework FSM has no constraint on maximum state count beyond memory.

### 5.4 Payload rules (OPEN — see §9.3)

`event_payload` is `const void *`. Lifetime rule: valid only for the duration
of `dispatch()`. Callbacks must not cache the pointer. Application owns memory.
Pending Phase 12C confirmation.

---

## 6. Guard / Action Contract

### 6.1 Guard

- Pure predicate: no mutation, no blocking, no UART TX, no heap, no driver.
- Returns `true` = allow; `false` = skip this row (evaluation continues).
- Thread context only.

### 6.2 Action (Phase 12C confirmed)

- May mutate `user_context`.
- May log via RTT (`LOG_INF`).
- Must not call UART TX, block on I/O, or allocate heap.
- Thread context only.
- **Runs in post-transition state context** (state already updated; Phase
  12C order).
- **Non-OK return does NOT roll back the transition.** State stays at target;
  `dispatch()` returns the action's non-OK status. Entry action still runs.
- No retry inside Framework FSM. Application monitors `last_status` or the
  `dispatch()` return for failure handling.
- No implicit fault escalation. Products that want a fault-on-action-fail
  behavior must define an explicit fault state and a transition row.

### 6.3 Entry / exit (Phase 12C confirmed)

- Called on every committed transition.
- Same constraints as action (§6.2).
- **Exit:** called **before** state update and **before** action; sees
  `from_state` as `current_state`.
- **Entry:** called **after** state update and **after** action; sees
  `next_state` as `current_state`. Runs regardless of action's status.

---

## 7. Observability Contract

### 7.1 Audit surface

`robotos_fw_fsm_get_snapshot()` provides all audit counters. A future
`ROBOTOS_FW` RTT log stream (not Phase 12B scope) would emit these periodically.

### 7.2 Counter semantics (Phase 12C confirmed)

| Counter | Increments when |
|---|---|
| `transition_count` | A committed transition completes (after state update; once per dispatch maximum) |
| `event_count` | Any call to `dispatch()` (once per call) |
| `guard_rejected_count` | A matching row's guard returns false (may increment multiple times per dispatch if multiple matching rows have rejecting guards) |
| `no_transition_count` | Dispatch ends without a committed transition (either no matching row OR all matching rows had rejecting guards) |

**Phase 12C clarification:** `guard_rejected_count` and `no_transition_count`
are **independent**. In a single `dispatch()` where matching rows exist but
all guards reject, `guard_rejected_count` is incremented once per rejection
during the scan, **and** `no_transition_count` is incremented once at the
end. See
[`../02_PHASE_CLOSEOUTS/PHASE_12C_FSM_EVENT_BRIDGE_STATUS_MODEL.md §G.3`](../02_PHASE_CLOSEOUTS/PHASE_12C_FSM_EVENT_BRIDGE_STATUS_MODEL.md).

### 7.3 What is NOT changed

- `ROBOTOS_OBS`, `ROBOTOS_APP`, `ROBOTOS_FAULT` log lines: unchanged.
- `?` UART response shape: unchanged.
- No new `ROBOTOS_FW` RTT stream yet (future Phase 12C+ work).

---

## 8. Non-goals

This spec does not cover:

- Hierarchical FSM, history states, concurrent FSM regions.
- Dynamic state/transition registration at runtime.
- Heap allocation in FSM internals.
- UART TX from FSM or action callbacks.
- Parser/shell/registry/framing/response queue additions.
- Sensor, actuator, PID, or motion Framework APIs.
- Application/product state vocabulary.
- F407 portability.
- Scheduler mutation.
- Any hardware run.
- Any `framework/` source directory.
- Any `.h`/`.c` implementation files.
- The `?` UART query command shape (frozen).
- Telemetry schema mutation.

---

## 9. Decisions (CONFIRMED at Phase 12C)

All items in this section are **CONFIRMED at Phase 12C** for Phase 12D
implementation entry. Function names and types in §4 remain DRAFT until
Phase 12D locks them.

### 9.1 Event bridge pattern
**Status:** **`CONFIRMED at Phase 12C`** — `APPLICATION_OWNED_EVENT_BRIDGE_CONFIRMED`
**Confirmed rule:** Application bridge translates Adapter events
(`robotos_event_type_t` 100–103) to `robotos_fw_event_id_t` values and calls
`robotos_fw_fsm_dispatch()`. No new `robotos_event_type_t` allocation needed
for the FSM core itself. FSM does not register directly with
`robotos_core_register_event_handler()`. Bridge runs in thread/dispatch
context, not ISR.

### 9.2 Status model
**Status:** **`CONFIRMED at Phase 12C`** — `REUSE_ROBOTOS_CORE_STATUS_T_FOR_PHASE_12D_CONFIRMED`
**Confirmed rule:** Reuse `robotos_core_status_t` (the exact repo type name
in `core/robotos_core.h`). The `robotos_fw_status_t` typedef in §4.1 is
confirmed as an alias to `robotos_core_status_t`. No new Framework-specific
status enum is introduced in Phase 12C or Phase 12D. Status mapping is
documented in
[`../02_PHASE_CLOSEOUTS/PHASE_12C_FSM_EVENT_BRIDGE_STATUS_MODEL.md §E.3`](../02_PHASE_CLOSEOUTS/PHASE_12C_FSM_EVENT_BRIDGE_STATUS_MODEL.md).

### 9.3 Event payload lifetime
**Status:** **`CONFIRMED at Phase 12C`** — `PAYLOAD_BORROWED_FOR_DISPATCH_ONLY_CONFIRMED`
**Confirmed rule:** `event_payload` is a borrowed `const void *`. Valid only
for the duration of `robotos_fw_fsm_dispatch()`. FSM must not cache the
pointer. Application owns all payload memory. Actions copy fields into
`user_context` if durable state is needed. No heap allocation.

### 9.4 Action return semantics on non-OK
**Status:** **`CONFIRMED at Phase 12C`** — `ACTION_NON_OK_NO_ROLLBACK_CONFIRMED`
**Confirmed rule:** Action's non-OK return does NOT roll back the
transition. State is already updated (Phase 12C order: state update before
action). `transition_count++`, entry action still runs, `last_status`
records the non-OK value, and `dispatch()` returns it. No retry inside FSM.
No implicit fault escalation.

### 9.5 Guard return type
**Status:** **`CONFIRMED at Phase 12C`** — `GUARD_RETURNS_BOOL_ONLY_CONFIRMED`
**Confirmed rule:** Guards return `bool` only. `false` = skip row (evaluation
continues to next matching row). No status-returning guard variant in Phase
12C or Phase 12D. Any guard-side error is handled inside the guard (log to
RTT, return `false`).

### 9.6 Evaluation order
**Status:** **`CONFIRMED at Phase 12C`** — corrects Phase 12B draft order
**Confirmed rule:** exit → state update → action → entry. See §3.2 for the
authoritative pseudo-code.

---

## 10. Next Revision Conditions

This spec is revised when:

1. ~~**Phase 12C closes:** All §9 open decisions are confirmed.~~ **DONE at
   Phase 12C** (2026-05-12). §9 items moved from OPEN to CONFIRMED;
   evaluation order corrected; counter independence clarified.
2. ~~**Phase 12D opens (header stub):** Header stub `.h` file is created in
   an agreed Framework path. §4 names move from DRAFT to LOCKED-AT-12D.~~
   **DONE at Phase 12D** (2026-05-12). Header stub created at
   `RobotOS_v1.0/framework/robotos_fw_fsm.h`. §4 names are now
   LOCKED-AT-12D. Active Framework path established under Architecture A.
   **No `.c` body. No CMake integration. No devkit consumer.**
3. ~~**Phase 12E or later (implementation):** Header gains a `.c` body;
   dispatch logic, transition evaluation, counters, and critical-section
   protection are implemented and unit-tested. §3.2 pseudo-code maps to
   real code.~~ **DONE at Phase 12E** (2026-05-12). First implementation
   `.c` body at `RobotOS_v1.0/framework/robotos_fw_fsm.c`;
   host-test-validated (93/93 assertions, 20 test cases) under WSL Ubuntu /
   gcc 13.3.0; entry/exit non-OK behavior committed to "observed but not
   propagated". **No devkit integration. No hardware evidence.**
4. **Phase 12F-pre or later (Application bridge planning):** Plans the
   Application bridge translating `robotos_event_t` (Adapter events
   100-103) to `robotos_fw_event_id_t`. Docs-only gate; required before
   any devkit consumer of the Framework FSM is built. Phase 12F-pre
   requires explicit user authorization.
5. **A new Framework domain is added:** Section added for new domain (timer
   service, sensor, fault) following the same pattern as §3–§7. Each new
   domain is its own revision trigger.

This spec is NOT revised to:

- Record devkit implementation details (devkit docs are authoritative for
  devkit; this spec is authoritative for Framework).
- Track evidence logs (logs live under `../logs/INDEX.md`).
- Record closed phase history (progress file is authoritative for phase
  history).
- Track pre-existing `src/framework/*.c` content (unrelated; see header
  note).
