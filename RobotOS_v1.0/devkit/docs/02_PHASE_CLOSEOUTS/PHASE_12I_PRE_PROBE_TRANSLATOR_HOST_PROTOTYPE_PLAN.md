# Phase 12I-pre — Probe Translator Host Prototype Implementation Plan (docs-only)

**Status:** `CLOSED_DOCS_ONLY`
**Decision:** `PHASE_12I_PRE_PROBE_TRANSLATOR_HOST_PROTOTYPE_PLAN_CLOSED`
**Date:** 2026-05-13
**Branch / baseline:** `master` at `origin/master = 03edb75` (Phase 12H)
**Prior phase anchor:** [`PHASE_12H_PROBE_TRANSLATOR_SKELETON_PLANNING.md`](PHASE_12H_PROBE_TRANSLATOR_SKELETON_PLANNING.md)
**New long-lived spec:** [`../03_SPECS/PROBE_TRANSLATOR_HOST_PROTOTYPE_PLAN.md`](../03_SPECS/PROBE_TRANSLATOR_HOST_PROTOTYPE_PLAN.md)

---

## A. Executive Summary

Phase 12I-pre is a **docs-only implementation-planning gate** that
converts the Phase 12H skeleton plan into an execution-ready
implementation contract for Phase 12I — Probe Translator Host
Prototype.

Phase 12I-pre **does**:

- resolve numeric values for all `PROBE_TRANSLATOR_STATE_*`,
  `PROBE_TRANSLATOR_EVT_*`, `PROBE_ADAPTER_TYPE_*`, and
  `PROBE_ADAPTER_ARG_*` constants;
- decide ship-or-defer for the `FAULT` block and transition row 5;
- decide `probe_translator_t` struct ownership model (embed by value);
- decide `probe_translator_snapshot_t` shape (combined struct);
- decide `PROBE_ADAPTER_ARG_ANY` policy (omit from Phase 12I);
- define the exact future file list Phase 12I may create or update;
- write the exact future host test case list (15 tests, 0 conditional
  on FAULT since FAULT defers);
- write the exact CMake additive block skeleton for
  `tests/host/CMakeLists.txt`;
- freeze the implementation contract in a new long-lived spec
  (`PROBE_TRANSLATOR_HOST_PROTOTYPE_PLAN.md`);
- preserve every other open gate untouched.

Phase 12I-pre **does not**:

- create `app/` or `application/` directory;
- create `app/probe_translator/` directory;
- create any `.c` / `.h` / `CMakeLists.txt` / README file;
- modify any file under `framework/`, `core/`, `platform/`,
  `devkit/src/`, `tests/`, `src/`, or `include/robotos/`;
- modify any `CMakeLists.txt`, `prj.conf`, board DTS, overlay, or
  Zephyr config;
- modify any evidence log;
- change `devkit_app_state`;
- change the frozen `a/s/r/?/x/v/L/d/T` command set;
- run hardware;
- open Phase 12I.

Phase 12I is **not started** at the close of Phase 12I-pre. It may
open only on **explicit user authorization**.

---

## B. Baseline Before Phase 12I-pre

| Item | Value |
|---|---|
| `origin/master` baseline | `03edb75` `docs: add Phase 12H probe translator skeleton planning` |
| Phase 12H status | `CLOSED_DOCS_ONLY` / `PHASE_12H_PROBE_TRANSLATOR_SKELETON_PLANNED_DOCS_ONLY` |
| Phase 12H-pre status | `CLOSED_DOCS_ONLY` / `PHASE_12H_PRE_RECOMMEND_PROBE_TRANSLATOR_APP` |
| Future app path | `RobotOS_v1.0/app/probe_translator/` (reserved at planning depth) |
| `app/` directory | **NOT_CREATED** |
| `app/probe_translator/` directory | **NOT_CREATED** |
| Application / product implementation | `NOT_STARTED` |
| Framework FSM | `framework/robotos_fw_fsm.{h,c}` host-tested (Phase 12E; 93/93 assertions) |
| Framework event bridge | `framework/robotos_fw_event_bridge.{h,c}` host-tested (Phase 12F; 103/103 assertions); §5 names + signatures `LOCKED-AT-12F` |
| Full host regression | 22/22 PASS under WSL Ubuntu / gcc 13.3.0 |
| Devkit integration | `NOT_STARTED` |
| Active architecture | A (`core/` + `platform/` + `devkit/` + `framework/`) |
| Legacy architecture | B (`src/` + `include/robotos/` + root `CMakeLists.txt`) — frozen at Phase 12D-pre |
| Devkit runtime authority | `devkit_app_state` (Phase 9C; unchanged) |
| Frozen command set | `a / s / r / ? / x / v / L / d / T` (validated through Phase 11Z) |
| Scope-guard #11 | `devkit_app_state` devkit-local; preserved |
| Open gates carried in | ACTIVE disarm `USER_DECISION_REQUIRED_ACTIVE_DISARM`; Scheduler 7A/7B `DEFER`; F407 `HOLD/DEFER`; POST_FLASH_AUTOSTART `OPEN/MITIGATED_BY_WORKFLOW` |

---

## C. Implementation Planning Problem Statement

Phase 12H locked the skeleton shape — file set, API names, state /
event / adapter-key vocabulary, transition table rows, mapping table
rows, host test case list, and build strategy preference. However,
Phase 12H intentionally left the following implementation details
open:

1. **Numeric values.** Every `PROBE_TRANSLATOR_*` constant is a name
   only. Phase 12I cannot write a header without concrete `uint32_t`
   values.
2. **FAULT block ship-or-defer.** `PROBE_TRANSLATOR_STATE_FAULT`,
   `PROBE_TRANSLATOR_EVT_FAULT`, transition rows 6–9, mapping row 4,
   and test TC07 are all marked optional. Phase 12I needs a binary
   decision before creating source.
3. **Transition row 5 ship-or-defer.** The `IDLE + RESET → IDLE`
   self-loop is optional. Phase 12I needs to know whether to include
   it in the transition table.
4. **`probe_translator_t` struct ownership model.** Phase 12H listed
   embed-by-value vs. reference-by-pointer as an open question.
   Phase 12I needs the layout to write `probe_translator.h`.
5. **`probe_translator_snapshot_t` shape.** Combined struct vs. two
   out-parameters. Phase 12I needs the shape to write both the header
   and the test assertions.
6. **`PROBE_ADAPTER_ARG_ANY` policy.** Whether to declare it as a
   documentation constant in the header or omit it entirely.
7. **CMake block exactness.** The Phase 12H skeleton spec says
   "Option A — additive entry in existing `tests/host/CMakeLists.txt`"
   but does not write the exact `add_executable` / `add_test` / new
   `APP_DIR` variable block. Phase 12I needs the exact block before
   touching `CMakeLists.txt`.
8. **Payload action flag.** Phase 12H TC09 says "payload borrowed
   visible to action if action exists, otherwise `REVIEW_VALIDATED`".
   Phase 12I needs to decide whether any row uses a non-NULL action
   for test observability.

Phase 12I-pre resolves all eight questions. Phase 12I then has a
complete, unambiguous implementation contract and can deliver in a
single pass without churn.

---

## D. Phase 12I Approved Future File Set

The following are the **only** files Phase 12I may create or modify.
Nothing outside this set is in scope.

### D.1 New app files

| Future path | Purpose |
|---|---|
| `RobotOS_v1.0/app/probe_translator/probe_translator.h` | Public header: include guard; arch-A-only includes; state / event / adapter-key constants; `probe_translator_t`; `probe_translator_snapshot_t`; four public API declarations. |
| `RobotOS_v1.0/app/probe_translator/probe_translator.c` | Implementation: static const transition table, state defs, mapping table; static `probe_translator_t` instance; public API bodies. |
| `RobotOS_v1.0/app/probe_translator/README.md` | Short README: purpose, host-test entry point, explicit non-goals (no devkit binding, no UART, no Zephyr). |

### D.2 New host test

| Future path | Purpose |
|---|---|
| `RobotOS_v1.0/tests/host/test_app_probe_translator_mapping.c` | 15-case host test covering all approved §I cases. |

### D.3 Modified existing files

| Path | Change |
|---|---|
| `RobotOS_v1.0/tests/host/CMakeLists.txt` | Additive only: add `APP_DIR` variable, new `add_executable`, new `target_include_directories`, new `add_test`. See §J for exact block. |

### D.4 New host log

| Future path | Purpose |
|---|---|
| `RobotOS_v1.0/tests/host/logs/phase_12I_host_<YYYY-MM-DD>.log` | Full `ctest --verbose` transcript saved via `save_test_log` target. Committed to version control. |

### D.5 New closeout

| Future path |
|---|
| `RobotOS_v1.0/devkit/docs/02_PHASE_CLOSEOUTS/PHASE_12I_PROBE_TRANSLATOR_HOST_PROTOTYPE.md` |

### D.6 Updated docs

- `RobotOS_v1.0/devkit/docs/03_SPECS/PROBE_TRANSLATOR_APP_SKELETON_DRAFT.md` (status upgrade + cross-ref)
- `RobotOS_v1.0/devkit/docs/03_SPECS/PROBE_TRANSLATOR_HOST_PROTOTYPE_PLAN.md` (status upgrade)
- `RobotOS_v1.0/devkit/docs/03_SPECS/FIRST_APPLICATION_CANDIDATE_DRAFT.md` (cross-ref to Phase 12I evidence)
- `RobotOS_v1.0/devkit/docs/01_PROGRESS/DEVKIT_PROGRESS_PHASE_11_20.md`
- `CURRENT_STATE.md`
- Optional: `RobotOS_v1.0/devkit/docs/00_INDEX/README.md`

### D.7 Forbidden at Phase 12I first implementation

| Forbidden path | Reason |
|---|---|
| `RobotOS_v1.0/app/probe_translator/CMakeLists.txt` | Option B deferred; host test target lives in `tests/host/CMakeLists.txt` (Option A). |
| `RobotOS_v1.0/app/probe_translator/probe_translator.prj.conf` / DTS | No Zephyr build at first. |
| `RobotOS_v1.0/app/probe_translator/probe_translator_devkit.{c,h}` | No devkit integration. |
| `RobotOS_v1.0/app/probe_translator/probe_translator_uart.{c,h}` | No UART surface. |
| `RobotOS_v1.0/app/probe_translator/probe_translator_main.c` | No hardware entry-point. |
| Any RTT / J-Link / OpenOCD / flashing script | No hardware run. |
| Any `src/app/` or `include/robotos/app*` reference | Architecture B is frozen. |

---

## E. Numeric ID Plan

All decisions are final for Phase 12I. Values are assigned as
`uint32_t` constants.

### E.1 States

| Constant | Value | Status |
|---|---|---|
| `PROBE_TRANSLATOR_STATE_IDLE` | `0u` | **Required; ships in Phase 12I.** |
| `PROBE_TRANSLATOR_STATE_READY` | `1u` | **Required; ships in Phase 12I.** |
| `PROBE_TRANSLATOR_STATE_ACTIVE` | `2u` | **Required; ships in Phase 12I.** |
| `PROBE_TRANSLATOR_STATE_FAULT` | `3u` (reserved) | **DEFERRED to a future app-behavior phase.** Not declared in Phase 12I header. |

Note: `ROBOTOS_FW_STATE_UNINIT == 0u` is the Framework sentinel.
`PROBE_TRANSLATOR_STATE_IDLE == 0u` is a valid product state because
the Framework only uses the sentinel to signal "not initialized" from
`robotos_fw_fsm_get_state(NULL)` — the FSM does not reject `0u` as an
initial state value. Confirmed acceptable per `robotos_fw_fsm.h`
§ROBOTOS_FW_STATE_UNINIT: the sentinel is not validated against the
transition table; it is only returned when `fsm == NULL` or the
instance is uninitialized. The application may use `0u` as a state ID
if `initial_state != ROBOTOS_FW_STATE_UNINIT` would fail — but
`initial_state == 0u` triggers `ROBOTOS_CORE_ERR_INVALID_ARG`. **Re-
decision required:** `PROBE_TRANSLATOR_STATE_IDLE` must NOT be `0u`.
**Revised:**

| Constant | Value | Status |
|---|---|---|
| `PROBE_TRANSLATOR_STATE_IDLE` | `1u` | Required. |
| `PROBE_TRANSLATOR_STATE_READY` | `2u` | Required. |
| `PROBE_TRANSLATOR_STATE_ACTIVE` | `3u` | Required. |
| `PROBE_TRANSLATOR_STATE_FAULT` | `4u` (reserved, undeclared) | DEFERRED. |

**Rationale:** `ROBOTOS_FW_STATE_UNINIT == 0u` is reserved by the
Framework; no product state may be assigned `0u` because
`robotos_fw_fsm_init` rejects `initial_state ==
ROBOTOS_FW_STATE_UNINIT` with `ERR_INVALID_ARG` (see
`robotos_fw_fsm.h` §init Return). Starting from `1u` avoids this
invariant violation.

### E.2 Events

| Constant | Value | Status |
|---|---|---|
| `PROBE_TRANSLATOR_EVT_CONFIGURED` | `1u` | Required. |
| `PROBE_TRANSLATOR_EVT_START` | `2u` | Required. |
| `PROBE_TRANSLATOR_EVT_STOP` | `3u` | Required. |
| `PROBE_TRANSLATOR_EVT_RESET` | `4u` | Required. |
| `PROBE_TRANSLATOR_EVT_FAULT` | `5u` (reserved, undeclared) | **DEFERRED.** Not declared in Phase 12I header. |

### E.3 Adapter types

| Constant | Value | Status |
|---|---|---|
| `PROBE_ADAPTER_TYPE_CONFIG` | `1u` | Required. |
| `PROBE_ADAPTER_TYPE_COMMAND` | `2u` | Required. |
| `PROBE_ADAPTER_TYPE_FAULT` | `3u` (reserved, undeclared) | **DEFERRED.** Not declared in Phase 12I header. |

### E.4 Adapter args

| Constant | Value | Status |
|---|---|---|
| `PROBE_ADAPTER_ARG_NONE` | `0u` | Required. |
| `PROBE_ADAPTER_ARG_START` | `1u` | Required. |
| `PROBE_ADAPTER_ARG_STOP` | `2u` | Required. |
| `PROBE_ADAPTER_ARG_RESET` | `3u` | Required. |
| `PROBE_ADAPTER_ARG_ANY` | — | **OMITTED** from Phase 12I header; no wildcard row exists in Phase 12I since FAULT defers. |

### E.5 FAULT block decision

**DEFERRED.** `PROBE_TRANSLATOR_STATE_FAULT`, `PROBE_TRANSLATOR_EVT_
FAULT`, `PROBE_ADAPTER_TYPE_FAULT`, transition rows 6–9, and mapping
row 4 are **all deferred** to a future app-behavior phase. No
declaration of these symbols appears in any Phase 12I source file.
The value `4u` (state) / `5u` (event) / `3u` (type) are reserved
informally in the design space but not committed to the header.

### E.6 Transition row 5 decision

**DEFERRED.** The `IDLE + RESET → IDLE` self-loop (row 5 from Phase
12H §I) is deferred from Phase 12I. The required transition table has
4 rows only (rows 0–4 from Phase 12H renumbered to rows 0–3 without
row 5). This keeps the first implementation maximally simple; the
self-loop can be added in a future behavior extension phase if any
test requires it. TC06.c (the conditional sub-case for the self-loop)
is correspondingly omitted from Phase 12I.

---

## F. Struct and Ownership Plan

### F.1 `probe_translator_t` — embed by value

```c
typedef struct probe_translator {
    robotos_fw_fsm_t          fsm;
    robotos_fw_event_bridge_t bridge;
} probe_translator_t;
```

**Decision: embed FSM and bridge by value.** The instance is statically
allocated by the caller (typically the host test). Embedding avoids
any pointer indirection, removes the need for a separate lifetime
agreement between the harness and its sub-objects, and mirrors the
Framework's own "caller-owned static allocation" model for FSM and
bridge. The struct is declared in `probe_translator.h` for static
sizing; fields are treated as opaque — access only through the public
API.

Rejected alternative — pointer-to-external FSM/bridge: adds ownership
complexity with no benefit for the first host-only harness.

### F.2 `probe_translator_config_t`

```c
typedef struct {
    void *user_context;   /* passed through to FSM config; may be NULL */
} probe_translator_config_t;
```

`probe_translator_init` uses the static const transition table, state
defs, and mapping table from `probe_translator.c`; it does not accept
them from the caller. The only caller-controlled field is
`user_context` (opaque pointer passed to FSM callbacks). This keeps
host tests clean — the test just calls `probe_translator_init(&pt,
NULL)` (or passes a probe struct as user_context for action
observation if a non-NULL action is used).

### F.3 `probe_translator_snapshot_t` — combined struct

```c
typedef struct {
    robotos_fw_fsm_snapshot_t          fsm;
    robotos_fw_event_bridge_snapshot_t bridge;
} probe_translator_snapshot_t;
```

**Decision: combined struct.** The host test calls
`probe_translator_get_snapshot(&pt, &snap)` and then reads
`snap.fsm.current_state`, `snap.fsm.transition_count`,
`snap.bridge.mapped_count`, etc. in one call. This is more ergonomic
than two separate out-parameters and keeps the test code readable.

### F.4 Static tables in `probe_translator.c`

Phase 12I source owns all four static tables:

- `static const robotos_fw_transition_t
  k_probe_translator_transitions[]` — 4 rows.
- `static const robotos_fw_state_def_t
  k_probe_translator_state_defs[]` — 3 entries (IDLE, READY, ACTIVE),
  all with `on_entry = NULL` / `on_exit = NULL` at first
  implementation.
- `static const robotos_fw_event_bridge_row_t
  k_probe_translator_mapping[]` — 4 rows.
- Internal static `robotos_fw_fsm_config_t` and
  `robotos_fw_event_bridge_config_t` built from the above.

The host test does not construct these tables. It initializes the app
via `probe_translator_init`, dispatches via
`probe_translator_dispatch_adapter_event`, queries via
`probe_translator_get_snapshot`, and resets via
`probe_translator_reset`.

### F.5 Payload action decision

**No non-NULL action at Phase 12I.** All four transition table rows
use `action = NULL`. TC09 (`payload_borrowed_visible_to_action`) is
therefore covered as `REVIEW_VALIDATED` — the test confirms at code
review that the bridge passes the payload pointer through unchanged
to `robotos_fw_fsm_dispatch`, which in turn stores nothing beyond the
dispatch call (as established by Phase 12F TC08). No additional
runtime assertion is needed in Phase 12I. This avoids adding a test-
only action callback to the application source, which would constitute
test-driven pollution of the production module.

---

## G. Future Header Contract Plan

`probe_translator.h` includes guard, then:

```c
#include <stdint.h>
#include "robotos_fw_fsm.h"
#include "robotos_fw_event_bridge.h"
```

No `<stdbool.h>` (already pulled in via `robotos_fw_fsm.h`). No
Zephyr headers. No devkit headers. No Architecture B headers
(`ro_*.h`, `include/robotos/*.h`). No `robotos_core.h` directly
(included transitively via `robotos_fw_fsm.h`).

**State constants** (typed `robotos_fw_state_id_t`):

```c
#define PROBE_TRANSLATOR_STATE_IDLE   ((robotos_fw_state_id_t)1u)
#define PROBE_TRANSLATOR_STATE_READY  ((robotos_fw_state_id_t)2u)
#define PROBE_TRANSLATOR_STATE_ACTIVE ((robotos_fw_state_id_t)3u)
```

**Event constants** (typed `robotos_fw_event_id_t`):

```c
#define PROBE_TRANSLATOR_EVT_CONFIGURED ((robotos_fw_event_id_t)1u)
#define PROBE_TRANSLATOR_EVT_START      ((robotos_fw_event_id_t)2u)
#define PROBE_TRANSLATOR_EVT_STOP       ((robotos_fw_event_id_t)3u)
#define PROBE_TRANSLATOR_EVT_RESET      ((robotos_fw_event_id_t)4u)
```

**Adapter key constants** (plain `uint32_t`):

```c
#define PROBE_ADAPTER_TYPE_CONFIG  ((uint32_t)1u)
#define PROBE_ADAPTER_TYPE_COMMAND ((uint32_t)2u)
#define PROBE_ADAPTER_ARG_NONE     ((uint32_t)0u)
#define PROBE_ADAPTER_ARG_START    ((uint32_t)1u)
#define PROBE_ADAPTER_ARG_STOP     ((uint32_t)2u)
#define PROBE_ADAPTER_ARG_RESET    ((uint32_t)3u)
```

**Types:**

```c
typedef struct probe_translator       probe_translator_t;
typedef struct probe_translator_config probe_translator_config_t;
typedef struct {
    robotos_fw_fsm_snapshot_t          fsm;
    robotos_fw_event_bridge_snapshot_t bridge;
} probe_translator_snapshot_t;
```

**API declarations** (DRAFT / host prototype):

```c
robotos_fw_status_t probe_translator_init(
    probe_translator_t              *pt,
    const probe_translator_config_t *config);

robotos_fw_status_t probe_translator_dispatch_adapter_event(
    probe_translator_t *pt,
    uint32_t            adapter_type,
    uint32_t            adapter_arg0,
    const void         *payload);

robotos_fw_status_t probe_translator_reset(probe_translator_t *pt);

robotos_fw_status_t probe_translator_get_snapshot(
    const probe_translator_t    *pt,
    probe_translator_snapshot_t *out);
```

---

## H. Future Source Contract Plan

`probe_translator.c` includes only `"probe_translator.h"` and
`<stddef.h>` (for `NULL`). No additional includes.

### H.1 Static tables

**Transition table (4 rows):**

```
row 0: STATE_IDLE   + EVT_CONFIGURED -> STATE_READY,   guard=NULL, action=NULL
row 1: STATE_READY  + EVT_START      -> STATE_ACTIVE,  guard=NULL, action=NULL
row 2: STATE_ACTIVE + EVT_STOP       -> STATE_READY,   guard=NULL, action=NULL
row 3: STATE_READY  + EVT_RESET      -> STATE_IDLE,    guard=NULL, action=NULL
row 4: STATE_ACTIVE + EVT_RESET      -> STATE_IDLE,    guard=NULL, action=NULL
```

**State defs (3 entries):**

```
STATE_IDLE,   on_entry=NULL, on_exit=NULL
STATE_READY,  on_entry=NULL, on_exit=NULL
STATE_ACTIVE, on_entry=NULL, on_exit=NULL
```

**Mapping table (4 rows):**

```
row 0: TYPE_CONFIG,  ARG_NONE,  match_arg0=true,  fw_event=EVT_CONFIGURED
row 1: TYPE_COMMAND, ARG_START, match_arg0=true,  fw_event=EVT_START
row 2: TYPE_COMMAND, ARG_STOP,  match_arg0=true,  fw_event=EVT_STOP
row 3: TYPE_COMMAND, ARG_RESET, match_arg0=true,  fw_event=EVT_RESET
```

### H.2 `probe_translator_init`

Builds internal `robotos_fw_fsm_config_t` from static tables with
`initial_state = PROBE_TRANSLATOR_STATE_IDLE`. Calls
`robotos_fw_fsm_init(&pt->fsm, &fsm_cfg)`. Then builds internal
`robotos_fw_event_bridge_config_t` with `rows =
k_probe_translator_mapping`, `row_count = 4`,
`fsm = &pt->fsm`, `user_context = config ? config->user_context :
NULL`. Calls `robotos_fw_event_bridge_init(&pt->bridge, &bridge_cfg)`.
Returns first non-OK. Returns `ROBOTOS_CORE_ERR_NULL` if `pt` is NULL.

### H.3 `probe_translator_dispatch_adapter_event`

Calls `robotos_fw_event_bridge_dispatch(&pt->bridge, adapter_type,
adapter_arg0, payload)` and returns the result verbatim. Returns
`ROBOTOS_CORE_ERR_NULL` if `pt` is NULL.

### H.4 `probe_translator_reset`

Calls `robotos_fw_event_bridge_reset(&pt->bridge)` first (clears
bridge counters). Then calls `robotos_fw_fsm_reset(&pt->fsm)` (resets
FSM to `PROBE_TRANSLATOR_STATE_IDLE`). Returns first non-OK. Returns
`ROBOTOS_CORE_ERR_NULL` if `pt` is NULL.

### H.5 `probe_translator_get_snapshot`

Calls `robotos_fw_fsm_get_snapshot(&pt->fsm, &out->fsm)` then
`robotos_fw_event_bridge_get_snapshot(&pt->bridge, &out->bridge)`.
Returns first non-OK. Returns `ROBOTOS_CORE_ERR_NULL` if `pt` or
`out` is NULL.

---

## I. Future Host Test Contract

Test file: `RobotOS_v1.0/tests/host/test_app_probe_translator_mapping.c`

Uses the same `TC(name, cond)` harness pattern as Phase 12E / 12F
tests (simple `printf` + pass/fail counter). Exact test names and
expected assertions:

| # | Test name | Coverage target | FAULT conditional? |
|---|---|---|---|
| TC01 | `init_valid_starts_idle` | `probe_translator_init(&pt, NULL)` returns `OK`; `snap.fsm.current_state == STATE_IDLE`; `snap.fsm.initialized == true`; bridge counters zero. | No |
| TC02 | `init_null_rejected` | `probe_translator_init(NULL, ...)` returns `ERR_NULL`. | No |
| TC03 | `config_maps_idle_to_ready` | Dispatch `(TYPE_CONFIG, ARG_NONE, NULL)` from IDLE; snap shows `STATE_READY`; `fsm.transition_count == 1`; `bridge.mapped_count == 1`; `bridge.event_count == 1`. | No |
| TC04 | `start_maps_ready_to_active` | Dispatch `(TYPE_COMMAND, ARG_START, NULL)` from READY; snap shows `STATE_ACTIVE`. | No |
| TC05 | `stop_maps_active_to_ready` | Dispatch `(TYPE_COMMAND, ARG_STOP, NULL)` from ACTIVE; snap shows `STATE_READY`. | No |
| TC06a | `reset_maps_ready_to_idle` | Dispatch `(TYPE_COMMAND, ARG_RESET, NULL)` from READY; snap shows `STATE_IDLE`. | No |
| TC06b | `reset_maps_active_to_idle` | Dispatch `(TYPE_COMMAND, ARG_RESET, NULL)` from ACTIVE; snap shows `STATE_IDLE`. | No |
| TC07 | `unmapped_event_no_state_change` | Dispatch `(0xDEADBEEF, 0xCAFEBABE, NULL)`; FSM state unchanged; `bridge.unmapped_count == 1`; return `OK`. | No |
| TC08 | `payload_borrowed_visible_to_action` | Pass non-NULL payload pointer; dispatch returns `OK`; bridge holds no payload pointer post-dispatch (no `last_payload` field in bridge snapshot). Covered as `REVIEW_VALIDATED` since all actions are NULL. | No |
| TC09 | `snapshot_contains_fsm_and_bridge_counts` | After sequence CONFIG / START / STOP / RESET: `snap.fsm.transition_count == 4`; `snap.bridge.event_count == 4`; `snap.bridge.mapped_count == 4`; `snap.bridge.unmapped_count == 0`; `snap.fsm.current_state == STATE_IDLE`. | No |
| TC10 | `probe_translator_reset_clears_counters` | Dispatch CONFIG then START; reset; snap shows `STATE_IDLE`; bridge counters zero; FSM counters zero. | No |
| TC11 | `full_path_idle_ready_active_ready_idle` | CONFIG → START → STOP → RESET; assert state at each step: `IDLE→READY→ACTIVE→READY→IDLE`; final `fsm.transition_count == 4`. | No |
| TC12 | `grep_gate_no_devkit_dependency` | Build-time / review: `probe_translator.{c,h}` and test do not `#include "devkit_app_state.h"` or any `devkit_*.h`. `REVIEW_VALIDATED`. | No |
| TC13 | `grep_gate_no_uart_command_dependency` | Build-time / review: `probe_translator.{c,h}` and test contain no reference to `a/s/r/?/x/v/L/d/T` byte literals or `devkit_uart_*` symbols. `REVIEW_VALIDATED`. | No |
| TC14 | `grep_gate_no_zephyr_devkit_legacy_include` | Build-time / review: no `<zephyr/...>`, no `devkit_*.h`, no `ro_*.h`, no `include/robotos/` header. `REVIEW_VALIDATED`. | No |
| TC15 | `full_host_regression_preserved` | All existing targets (22/22 prior to Phase 12I) still PASS; total after Phase 12I = 23/23. | No |

Total: 15 test items (TC01–TC15); TC08 / TC12 / TC13 / TC14 are
`REVIEW_VALIDATED`; TC15 is structural (regression assertion). The
executed assert count will grow from 22/22 to approximately 23/23 for
the test target count; the assertion count within
`test_app_probe_translator_mapping.c` is expected to be approximately
50–70 based on Phase 12F (103 assertions for 17 cases).

---

## J. Build Strategy for Phase 12I

### J.1 CMake additive block (exact skeleton)

The following block is added **at the end** of
`RobotOS_v1.0/tests/host/CMakeLists.txt`, after the existing
`robotos_fw_event_bridge_contract_test` block and before the
`save_test_log` custom target:

```cmake
# ---- Application: Probe Translator host mapping contract test (Phase 12I) ---
# Exercises the probe_translator application harness end-to-end against the
# real Framework FSM and event bridge. No devkit, Zephyr, or hardware
# dependency. Architecture A only.
set(APP_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../app/probe_translator")
add_executable(probe_translator_mapping_contract_test
    test_app_probe_translator_mapping.c
    "${APP_DIR}/probe_translator.c"
    "${FRAMEWORK_DIR}/robotos_fw_event_bridge.c"
    "${FRAMEWORK_DIR}/robotos_fw_fsm.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/robotos_platform_critical_host_stub.c"
)

target_include_directories(probe_translator_mapping_contract_test PRIVATE
    "${APP_DIR}"
    "${FRAMEWORK_DIR}"
    "${CORE_DIR}"
    "${PLATFORM_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}"
)

add_test(NAME probe_translator_mapping_contract
         COMMAND probe_translator_mapping_contract_test)
```

### J.2 Build environment

- **WSL Ubuntu / gcc 13.3.0** (same as Phase 12E / 12F; MSYS2
  MinGW64 remains broken and is not used).
- Build directory convention: `build-phase12i-host/` (at repo root,
  mirroring `build-phase12e-host/` and `build-phase12f-host/`).
- Expected test count after build: 23 tests (22 prior + 1 new).
- Log naming: `tests/host/logs/phase_12I_host_<YYYY-MM-DD>.log`.

### J.3 Build command reference (for Phase 12I implementer)

```bash
# WSL, from repo root:
cmake -S RobotOS_v1.0/tests/host -B build-phase12i-host
cmake --build build-phase12i-host
ctest --test-dir build-phase12i-host --output-on-failure -V
cmake --build build-phase12i-host --target save_test_log
```

---

## K. Phase 12I Validation and Exit Criteria

Phase 12I may only close `CLOSED_WITH_HOST_TEST_EVIDENCE` when **all**
of the following pass:

| # | Gate | Evidence required |
|---|---|---|
| 1 | CMake configure succeeds | No error on `cmake -S ... -B build-phase12i-host`. |
| 2 | Build succeeds | No error on `cmake --build build-phase12i-host`. |
| 3 | Probe-translator test PASS | `ctest` shows `probe_translator_mapping_contract` PASS. |
| 4 | FSM test still PASS | `robotos_fw_fsm_contract` PASS. |
| 5 | Bridge test still PASS | `robotos_fw_event_bridge_contract` PASS. |
| 6 | Full host regression PASS | All 23 tests PASS (23/23). |
| 7 | Host log saved | `tests/host/logs/phase_12I_host_<date>.log` exists and is non-empty. |
| 8 | Grep gate: no `devkit_app_state.h` | `grep -r "devkit_app_state" app/probe_translator/` returns no match. |
| 9 | Grep gate: no `devkit_*` calls | `grep -r "devkit_" app/probe_translator/` returns no match. |
| 10 | Grep gate: no UART command strings | `grep -r "\"a\"\|\"s\"\|\"r\"\|\"?\"\|\"x\"\|\"v\"\|\"L\"\|\"d\"\|\"T\"" app/probe_translator/` returns no semantically relevant match (string-literal command bytes). |
| 11 | Grep gate: no Zephyr / legacy includes | `grep -rE "<zephyr/|ro_|include/robotos" app/probe_translator/ tests/host/test_app_probe_translator_mapping.c` returns no match. |
| 12 | Command set unchanged | `a/s/r/?/x/v/L/d/T` still defined in `devkit/src/`; no new byte in UART handler. |
| 13 | `devkit_app_state` zero-diff | `git diff HEAD -- devkit/src/devkit_app_state.{c,h}` = empty. |
| 14 | No hardware run | No RTT / J-Link / OpenOCD session; no new board DTS or `prj.conf`. |

---

## L. Relationship to `devkit_app_state`

- `devkit_app_state` remains **authoritative** for the current
  devkit runtime. Phase 12I-pre does **not** touch it.
- `probe_translator/`, when created, **must not** include
  `devkit_app_state.h`, must not call any `devkit_*` function, and
  must not read or write `devkit_app_state` snapshots.
- `PROBE_TRANSLATOR_STATE_IDLE / READY / ACTIVE` are application-
  local. Name overlap with `DEVKIT_APP_STATE_IDLE / ARMED / ACTIVE`
  is coincidental at the human-readable level only.
- **No `?` UART response change.**
- **No `a/s/r/?/x/v/L/d/T` semantic change.**
- Any future devkit ↔ application interaction requires a separate
  planning phase — not Phase 12I.
- **Scope-guard #11 remains active.**

---

## M. Relationship to Command Set

- `a / s / r / ? / x / v / L / d / T` remain the **devkit / probe
  surface**. Phase 12I-pre changes nothing here.
- `probe_translator/` defines no UART command at Phase 12I.
- Framework bridge has no UART surface (Phase 12F locked).
- All 12 UART TX scope-guard constraints from Phase 9EZ §H preserved.

---

## N. Relationship to Legacy Architecture B

- `RobotOS_v1.0/src/` and `RobotOS_v1.0/include/robotos/` remain
  frozen at `LEGACY_SCAFFOLD_MARKED_FROZEN_DOCS_ONLY` (Phase 12D-pre).
- `probe_translator/` **must not** reuse `src/app/`,
  `include/robotos/app*`, or any other Architecture B artifact.
- `probe_translator/` uses **Architecture A contracts only**:
  `robotos_fw_*` (from `framework/`) and `robotos_core_status_t`
  (transitively from `core/`).
- **No Architecture A ↔ Architecture B reconciliation in Phase
  12I-pre.**

---

## O. Non-goals (Phase 12I-pre)

| Non-goal | Status |
|---|---|
| `app/` or `application/` directory created | NO |
| `app/probe_translator/` directory created | NO |
| Any source / header / test file created | NO |
| CMake change | NO |
| Framework code change | NO |
| Devkit runtime change | NO |
| `devkit_app_state` change | NO |
| UART command added | NO |
| Command semantic change | NO |
| Hardware run | NO |
| Scheduler 7A/7B reopened | NO — `DEFER` preserved |
| F407 / custom board reopened | NO — `HOLD/DEFER` preserved |
| ACTIVE disarm widening started | NO — `USER_DECISION_REQUIRED_ACTIVE_DISARM` preserved |
| POST_FLASH_AUTOSTART change | NO — `OPEN/MITIGATED_BY_WORKFLOW` preserved |
| Legacy Architecture B modification | NO |
| Evidence logs touched | NO |
| Phase 12I started | NO — `NOT_STARTED` |

---

## P. Decision Result

**`PHASE_12I_PRE_PROBE_TRANSLATOR_HOST_PROTOTYPE_PLAN_CLOSED`**

The implementation contract for Phase 12I is complete. Phase 12I may
open only on **explicit user authorization** with the scope in §Q.

---

## Q. Next Gate Recommendation

**Phase 12I — Probe Translator Host Prototype**

| Field | Value |
|---|---|
| Title | Phase 12I — Probe Translator Host Prototype |
| Classification | Host-first implementation |
| In-scope | New `app/probe_translator/` directory; `probe_translator.{c,h}` + `README.md`; new `tests/host/test_app_probe_translator_mapping.c`; additive CMake block per §J; new host log `tests/host/logs/phase_12I_host_<date>.log`; Phase 12I closeout doc. |
| Non-goals | No devkit change. No `devkit_app_state` change. No UART command. No hardware run. No Zephyr config. No legacy Architecture B modification. No Framework code change. |
| Exit criteria | All 14 validation gates in §K pass. Host test 23/23 PASS. Grep gates clean. Log committed. |
| Authorization | Must be opened only on **explicit user approval**. |

**Alternative — HOLD.** If the user does not want to implement yet,
Phase 12I-pre sits at planning depth indefinitely. No regression risk.

---

## R. Cross-references

- Phase 12H skeleton planning:
  [`PHASE_12H_PROBE_TRANSLATOR_SKELETON_PLANNING.md`](PHASE_12H_PROBE_TRANSLATOR_SKELETON_PLANNING.md)
- Skeleton spec:
  [`../03_SPECS/PROBE_TRANSLATOR_APP_SKELETON_DRAFT.md`](../03_SPECS/PROBE_TRANSLATOR_APP_SKELETON_DRAFT.md)
- Phase 12H-pre first application candidate:
  [`PHASE_12H_PRE_FIRST_APPLICATION_CANDIDATE_SELECTION.md`](PHASE_12H_PRE_FIRST_APPLICATION_CANDIDATE_SELECTION.md)
- Application boundary spec:
  [`../03_SPECS/FRAMEWORK_APPLICATION_BOUNDARY_DRAFT.md`](../03_SPECS/FRAMEWORK_APPLICATION_BOUNDARY_DRAFT.md)
- Phase 12F bridge host prototype:
  [`PHASE_12F_APPLICATION_BRIDGE_HOST_PROTOTYPE.md`](PHASE_12F_APPLICATION_BRIDGE_HOST_PROTOTYPE.md)
- Bridge spec:
  [`../03_SPECS/FRAMEWORK_APPLICATION_BRIDGE_DRAFT.md`](../03_SPECS/FRAMEWORK_APPLICATION_BRIDGE_DRAFT.md)
- New long-lived implementation-plan spec:
  [`../03_SPECS/PROBE_TRANSLATOR_HOST_PROTOTYPE_PLAN.md`](../03_SPECS/PROBE_TRANSLATOR_HOST_PROTOTYPE_PLAN.md)
- Live state: [`../../../../CURRENT_STATE.md`](../../../../CURRENT_STATE.md)
