# RobotOS Framework FSM API — Draft Spec

**Status:** `DRAFT / NON-FINAL`
**Revision:** Phase 12B (2026-05-12, `CLOSED_DOCS_ONLY`)
**Next revision condition:** Phase 12C — confirms event bridge pattern,
status model, payload lifetime, and action return semantics.

> **No `framework/` directory exists. No `.h` or `.c` Framework file
> exists. This document describes a future API that has not been
> implemented.**

This spec is extracted from
[`../02_PHASE_CLOSEOUTS/PHASE_12B_FRAMEWORK_FSM_API_DRAFT.md`](../02_PHASE_CLOSEOUTS/PHASE_12B_FRAMEWORK_FSM_API_DRAFT.md).
All items are DRAFT and subject to revision in Phase 12C before
any header or implementation is written.

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

**Current decision state (Phase 12B):**

| Decision | Status |
|---|---|
| Model: flat FSM, table-driven, no heap | CONFIRMED at Phase 12B |
| State/event IDs: product-defined `uint32_t` | CONFIRMED at Phase 12B |
| No `robotos_event_type_t` sub-range needed | CONFIRMED at Phase 12B |
| Event bridge pattern | OPEN — recommendation documented |
| Status model (reuse `robotos_core_status_t` vs new type) | OPEN — Option A recommended |
| Payload lifetime rules | OPEN — rules documented, confirmation needed |
| Action return semantics on non-OK | OPEN — policy documented, confirmation needed |

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

### 3.2 Evaluation sequence (confirmed at Phase 12B)

```
dispatch(fsm, event_id, payload):
  event_count++
  last_event_id = event_id
  scan transition table (FIFO):
    for each row where current_state matches AND event_id matches:
      if guard != NULL and guard() == false:
        guard_rejected_count++ ; continue
      → on_exit(current_state)  [if state def exists]
      → action(from, to, event, payload, ctx)  [if non-NULL]
      → current_state = row.next_state
      → on_entry(next_state)  [if state def exists]
      → transition_count++
      return action_status (or OK)
  if no row matched or all guards rejected:
    no_transition_count++
    return OK
```

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

## 4. Draft API Surface

**ALL ITEMS IN THIS SECTION ARE DRAFT / NON-FINAL.**

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

### 6.2 Action

- May mutate `user_context`.
- May log via RTT (`LOG_INF`).
- Must not call UART TX, block on I/O, or allocate heap.
- Thread context only.
- Non-OK return propagates as `dispatch()` return; transition is NOT rolled
  back (see §9.4 for OPEN status of this rule).

### 6.3 Entry / exit

- Called on every transition that changes state.
- Same constraints as action (§6.2).
- Entry: called after `current_state` updated.
- Exit: called before action.

---

## 7. Observability Contract

### 7.1 Audit surface

`robotos_fw_fsm_get_snapshot()` provides all audit counters. A future
`ROBOTOS_FW` RTT log stream (not Phase 12B scope) would emit these periodically.

### 7.2 Counter semantics

| Counter | Increments when |
|---|---|
| `transition_count` | A transition row is selected, guard passes, and state changes |
| `event_count` | Any call to `dispatch()` |
| `guard_rejected_count` | A matching row's guard returns false |
| `no_transition_count` | Full table scan finds no matching row (or all guards reject) |

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

## 9. Open Decisions

Items requiring Phase 12C confirmation before any header is written.

### 9.1 Event bridge pattern
**Status:** `OPEN_RECOMMENDATION_PENDING_CONFIRMATION`
**Recommendation:** Application bridge translates Adapter events
(`robotos_event_type_t` 100–103) to `robotos_fw_event_id_t` values and calls
`robotos_fw_fsm_dispatch()`. No new `robotos_event_type_t` allocation needed.
FSM does not register directly with `robotos_core_register_event_handler()`.

### 9.2 Status model
**Status:** `OPEN_RECOMMENDATION_PENDING_CONFIRMATION`
**Recommendation:** Option A — typedef `robotos_core_status_t` as
`robotos_fw_status_t`. Simpler; no new type. Guard rejection maps to
`ERR_INVALID_ARG`; unhandled event is `OK`.

### 9.3 Event payload lifetime
**Status:** `OPEN_RULES_DOCUMENTED_CONFIRMATION_NEEDED`
**Rule documented:** `event_payload` valid only for duration of `dispatch()`.
Callbacks must not cache pointer. Application owns memory.

### 9.4 Action return semantics on non-OK
**Status:** `OPEN_POLICY_DOCUMENTED_CONFIRMATION_NEEDED`
**Policy documented:** Non-OK action return does NOT roll back transition.
State update is committed; action error propagates as `dispatch()` return.

---

## 10. Next Revision Conditions

This spec is revised when:

1. **Phase 12C closes:** All §9 open decisions are confirmed; §9 items move
   from OPEN to CONFIRMED; doc revision note updated.
2. **Phase 12D (implementation) opens:** Header stub is created; API names
   are locked; §4 names move from DRAFT to CONFIRMED-NAME.
3. **A new Framework domain is added:** Section added for new domain (timer
   service, sensor, fault) following the same pattern as §3–§7.

This spec is NOT revised to:
- Record devkit implementation details (devkit docs are authoritative for
  devkit; this spec is authoritative for Framework).
- Track evidence logs (logs live under `../logs/INDEX.md`).
- Record closed phase history (progress file is authoritative for phase
  history).
