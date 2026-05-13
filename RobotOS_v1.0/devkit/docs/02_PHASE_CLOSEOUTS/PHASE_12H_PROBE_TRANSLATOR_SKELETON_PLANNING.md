# Phase 12H — Probe Translator App Skeleton Planning (docs-only, Variant 1)

**Status:** `CLOSED_DOCS_ONLY`
**Decision:** `PHASE_12H_PROBE_TRANSLATOR_SKELETON_PLANNED_DOCS_ONLY`
**Date:** 2026-05-13
**Branch / baseline:** `master` at `origin/master = 45ac339` (Phase 12H-pre)
**Prior phase anchor:** [`PHASE_12H_PRE_FIRST_APPLICATION_CANDIDATE_SELECTION.md`](PHASE_12H_PRE_FIRST_APPLICATION_CANDIDATE_SELECTION.md)
**New long-lived spec:** [`../03_SPECS/PROBE_TRANSLATOR_APP_SKELETON_DRAFT.md`](../03_SPECS/PROBE_TRANSLATOR_APP_SKELETON_DRAFT.md)

---

## A. Executive Summary

Phase 12H is the **docs-only application-skeleton planning gate**
(Variant 1) that locks the future `RobotOS_v1.0/app/probe_translator/`
skeleton boundary recommended by Phase 12H-pre.

Phase 12H **does**:

- enumerate the exact future files that Phase 12I (or a Variant 2
  host-prototype phase) may create;
- freeze the planning-level public API names and shapes for the
  future `probe_translator.h`;
- lock the app-local state IDs and event IDs;
- lock the initial transition table;
- lock the initial bridge mapping table (adapter type / arg0
  conventions and row order);
- lock the host test plan (case list, coverage, grep gates,
  regression preservation rule);
- lock the build strategy preference (Option A — additive entry in
  existing `tests/host/CMakeLists.txt`);
- freeze the skeleton in a new long-lived spec
  (`PROBE_TRANSLATOR_APP_SKELETON_DRAFT.md`);
- preserve every other open gate untouched.

Phase 12H **does not**:

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

Phase 12I is **not started** at the close of Phase 12H. It may open
only on **explicit user authorization**.

---

## B. Baseline Before Phase 12H

| Item | Value |
|---|---|
| `origin/master` baseline | `45ac339` `docs: add Phase 12H-pre first application candidate selection` |
| Phase 12H-pre status | `CLOSED_DOCS_ONLY` / `PHASE_12H_PRE_RECOMMEND_PROBE_TRANSLATOR_APP` |
| Future first app placeholder | `RobotOS_v1.0/app/probe_translator/` (reserved at planning depth) |
| `app/` directory | **NOT_CREATED** |
| `app/probe_translator/` directory | **NOT_CREATED** |
| Application / product implementation | `NOT_STARTED` |
| Framework FSM | `framework/robotos_fw_fsm.{h,c}` host-tested (Phase 12E; 93/93 assertions) |
| Framework event bridge | `framework/robotos_fw_event_bridge.{h,c}` host-tested (Phase 12F; 103/103 assertions); §5 names + signatures `LOCKED-AT-12F` |
| Full host regression | 22/22 PASS under WSL Ubuntu / gcc 13.3.0 |
| Devkit integration | `NOT_STARTED` |
| Active architecture | A (`core/` + `platform/` + `devkit/` + `framework/`) |
| Legacy architecture | B (`src/` + `include/robotos/` + root `RobotOS_v1.0/CMakeLists.txt`) — frozen at Phase 12D-pre |
| Devkit runtime authority | `devkit_app_state` (Phase 9C; unchanged) |
| Frozen command set | `a / s / r / ? / x / v / L / d / T` (validated through Phase 11Z) |
| Scope-guard #11 | `devkit_app_state` devkit-local; preserved |
| Open gates carried in | ACTIVE disarm `USER_DECISION_REQUIRED_ACTIVE_DISARM`; Scheduler 7A/7B `DEFER`; F407 `HOLD/DEFER`; POST_FLASH_AUTOSTART `OPEN/MITIGATED_BY_WORKFLOW` |

---

## C. Skeleton Planning Problem Statement

Phase 12H-pre selected `probe_translator` as the first application
candidate but did **not** define the exact source boundary or
implementation contract. Creating `app/probe_translator/` source
directly at Phase 12H — without first locking which files exist,
which API names they expose, which state / event IDs they declare,
which transition and mapping rows they install, and which host tests
validate them — would risk path drift, API churn, host-test rework,
and accidental coupling to devkit producers.

The risks if these are not locked first:

1. **File-set drift.** The first implementer chooses whether
   `probe_translator.h` exists or whether all symbols stay static
   internal to `probe_translator.c`. Each choice has different
   consequences for test access and future reuse; the choice should
   be a planning decision.
2. **API name churn.** If the public function names are not chosen
   first, a later host-test or runtime consumer is forced to follow
   the implementer's reflex naming. Phase 12F shows the value of
   freezing names before bodies (the bridge `LOCKED-AT-12F` move
   prevented churn during the host prototype).
3. **State / event vocabulary churn.** Phase 12H-pre recorded
   `APP_IDLE/READY/ACTIVE/FAULT?` and `APP_EVT_CONFIGURED/START/STOP/
   FAULT?/RESET` only as concept-level placeholders. Without concrete
   `PROBE_TRANSLATOR_*` names locked here, the first implementer
   picks them ad-hoc and the host test depends on whatever they pick.
4. **Mapping-table specificity-order surprise.** Phase 12F §5.3 made
   FIFO row order the wildcard precedence rule. The first
   implementation needs a row order that matches that rule
   consistently; this is a planning decision, not an implementation
   detail.
5. **Host-test scope drift.** Without a fixed case list, the first
   host test could ship under-covered (just one happy path) or
   over-scoped (binding to devkit producers by mistake). The case
   list should be locked before any test code lands.
6. **Build-wiring drift.** Option A (additive entry in
   `tests/host/CMakeLists.txt`) is preferred per Phase 12H-pre §9.
   Without locking this, an implementer might add a new leaf
   `app/probe_translator/CMakeLists.txt` and double the build
   surface for the first application.

Phase 12H resolves all six risks at planning depth, without writing
any source. Phase 12I (or a Variant 2 implementation phase) inherits
the locked skeleton.

---

## D. Future Directory and File Plan

The future directory `RobotOS_v1.0/app/probe_translator/` remains
**not created** by Phase 12H. The file list below describes what a
later Phase 12I (docs-only implementation planning) or implementation
variant may create.

### D.1 Recommended minimal future files

| Future path | Purpose | Required / optional |
|---|---|---|
| `RobotOS_v1.0/app/probe_translator/probe_translator.h` | Public app-layer header declaring `probe_translator_*` API + `PROBE_TRANSLATOR_STATE_*` / `PROBE_TRANSLATOR_EVT_*` constants + adapter-key macros for host tests. | **Required** at first implementation. |
| `RobotOS_v1.0/app/probe_translator/probe_translator.c` | Single translation unit: FSM state defs, transition table, bridge mapping table, instance allocation (static), public API bodies. | **Required**. |
| `RobotOS_v1.0/app/probe_translator/README.md` | Short README describing purpose, host-test entry point, and explicit non-goals (no devkit binding, no UART). | **Required**. |
| `RobotOS_v1.0/tests/host/test_app_probe_translator_mapping.c` | New host test target that compiles against `probe_translator.c`, `framework/robotos_fw_event_bridge.c`, `framework/robotos_fw_fsm.c`, and the existing host critical-section stub. | **Required**. |

### D.2 Optional future docs

| Future path | Purpose | When to add |
|---|---|---|
| `RobotOS_v1.0/app/probe_translator/PROBE_TRANSLATOR_SPEC.md` | Long-form implementation spec local to the app. | **Optional.** Only if the planning-depth spec at `RobotOS_v1.0/devkit/docs/03_SPECS/PROBE_TRANSLATOR_APP_SKELETON_DRAFT.md` grows beyond a comfortable size, or if app-local implementation details (memory layout, ABI lock notes) accumulate. Otherwise omit. |

### D.3 Explicitly forbidden at first implementation

The first implementation phase (Phase 12I or its Variant 2
equivalent) **must not** create any of the following:

| Forbidden path | Reason |
|---|---|
| `RobotOS_v1.0/app/probe_translator/CMakeLists.txt` | Build wiring is Option A (additive entry in existing `tests/host/CMakeLists.txt`). A leaf CMake at the app would be Option B, deferred until at least one more app exists. |
| `RobotOS_v1.0/app/probe_translator/probe_translator.prj.conf` / DTS overlay | The harness is host-only at first; no Zephyr build target exists. |
| `RobotOS_v1.0/app/probe_translator/probe_translator_devkit.{c,h}` | No devkit integration at any planned phase before separate authorization. |
| `RobotOS_v1.0/app/probe_translator/probe_translator_uart.{c,h}` | No UART command surface at the application layer at first. |
| `RobotOS_v1.0/app/probe_translator/probe_translator_main.c` (Zephyr main) | No hardware runtime entry-point at first. |
| Any RTT / J-Link / OpenOCD / flashing script under `app/probe_translator/` | No hardware run at any planned phase before separate authorization. |
| Any reference to `RobotOS_v1.0/src/app/` or `include/robotos/app*` | Architecture B is frozen; the new application must use Architecture A only. |

---

## E. Probe Translator Responsibility

### E.1 What `probe_translator/` may own

| Responsibility | Detail |
|---|---|
| App-local state IDs | `PROBE_TRANSLATOR_STATE_*` constants (see §G). |
| App-local event IDs | `PROBE_TRANSLATOR_EVT_*` constants (see §H). |
| Transition table for the probe-translator FSM | A static const `robotos_fw_transition_t[]` (see §I). |
| Bridge mapping table | A static const `robotos_fw_event_bridge_row_t[]` translating synthetic adapter keys to `PROBE_TRANSLATOR_EVT_*` (see §J). |
| App-level init / config binding of FSM + bridge | `probe_translator_init` glues the caller-owned FSM instance, bridge instance, transition table, mapping table, and state defs into one Framework consumer surface. |
| Host-test-only adapter event fixture definitions | The `PROBE_ADAPTER_TYPE_*` / `PROBE_ADAPTER_ARG_*` constants are application-local (see §J); they are not added to `core/`. |
| App-level snapshot / query helper | `probe_translator_get_snapshot` wraps `robotos_fw_fsm_get_snapshot` + `robotos_fw_event_bridge_get_snapshot` for host-test convenience (see §F). |

### E.2 What `probe_translator/` must NOT own

| Non-responsibility | Reason |
|---|---|
| Framework FSM / bridge algorithms | `framework/` is `LOCKED-AT-12D` (FSM names) / `LOCKED-AT-12F` (bridge names + signatures); the app is a consumer only. |
| Core queue / dispatcher internals | `core/` is canonical. |
| Platform backend primitives | `platform/` owns critical-section, time, fault, log boundaries. |
| `devkit_app_state` | Devkit-local; scope-guard #11 enforced. |
| Devkit command semantics (`a/s/r/?/x/v/L/d/T`) | Devkit/probe surface; unchanged. |
| UART TX | No UART surface at the application layer. |
| Zephyr drivers / device-tree references | No hardware binding at first. |
| Legacy Architecture B (`ro_*` HAL, `src/`, `include/robotos/`) | Frozen at Phase 12D-pre. |

---

## F. Proposed Public App API Shape

Phase 12H locks **names and shapes** only. No header file is created.
The API is `DRAFT / NON-FINAL` and may only be locked when the
implementation phase creates `probe_translator.h`.

### F.1 `probe_translator_init`

```c
robotos_fw_status_t probe_translator_init(
    probe_translator_t              *pt,
    const probe_translator_config_t *config);
```

- **Purpose.** Initialize a caller-owned `probe_translator_t`
  instance against a caller-owned config. Wires up FSM init + bridge
  init in one call so the host test does not duplicate framework
  setup.
- **Inputs.**
  - `pt` — caller-owned; statically allocated by the host test.
  - `config` — caller-owned; references the caller-owned FSM
    instance, bridge instance, FSM config (transitions + state defs
    + initial state), and bridge config (rows + fsm pointer +
    user_context).
- **Ownership / lifetime.** The probe-translator instance, its FSM,
  its bridge, and all referenced arrays are caller-owned and must
  outlive `pt`.
- **Side effects.** Calls `robotos_fw_fsm_init` then
  `robotos_fw_event_bridge_init`. No UART, no hardware, no devkit
  call, no Zephyr call.
- **Status behavior.** Returns first non-OK from FSM init or bridge
  init. Returns `ROBOTOS_CORE_ERR_NULL` if `pt` or `config` is NULL.
- **Test expectation.** TC01 (init valid config) + TC02 (init NULL
  config / NULL FSM / NULL bridge / empty mapping).
- **DRAFT / NON-FINAL.**

### F.2 `probe_translator_dispatch_adapter_event`

```c
robotos_fw_status_t probe_translator_dispatch_adapter_event(
    probe_translator_t *pt,
    uint32_t            adapter_type,
    uint32_t            adapter_arg0,
    const void         *payload);
```

- **Purpose.** Forward a synthetic adapter event tuple through the
  app's bridge into its FSM. Thin wrapper over
  `robotos_fw_event_bridge_dispatch` so host tests use a single
  application-layer entry point.
- **Inputs.** Synthetic adapter key + borrowed `payload`.
- **Ownership / lifetime.** `payload` is borrowed for the duration
  of the call only (mirrors Phase 12F §5 semantics).
- **Side effects.** Mutates `*pt`'s embedded bridge + FSM counters.
  No UART, no hardware, no devkit call.
- **Status behavior.** Returns bridge's status verbatim (which is
  either `ROBOTOS_CORE_OK` for unmapped events / OK FSM dispatch, or
  the FSM's non-OK status for an action that returned non-OK).
- **Test expectation.** TC03–TC10 in §K cover this entry point.
- **DRAFT / NON-FINAL.**

### F.3 `probe_translator_reset`

```c
robotos_fw_status_t probe_translator_reset(probe_translator_t *pt);
```

- **Purpose.** Reset the app harness to its initial planning state.
  Calls `robotos_fw_event_bridge_reset` (clears bridge counters) and
  `robotos_fw_fsm_reset` (resets FSM to initial state) in that
  order.
- **Side effects.** Mutates bridge counters and FSM state. No UART,
  no hardware, no devkit call.
- **Status behavior.** Returns first non-OK from
  `robotos_fw_event_bridge_reset` or `robotos_fw_fsm_reset`. Returns
  `ROBOTOS_CORE_ERR_NULL` if `pt` is NULL.
- **Test expectation.** TC11 (reset clears bridge counters + returns
  FSM to `PROBE_TRANSLATOR_STATE_IDLE`).
- **DRAFT / NON-FINAL.**

### F.4 `probe_translator_get_snapshot`

```c
robotos_fw_status_t probe_translator_get_snapshot(
    const probe_translator_t       *pt,
    probe_translator_snapshot_t    *out);
```

- **Purpose.** Combined snapshot returning both FSM and bridge
  snapshots in one call (convenience for host tests).
- **Snapshot shape.**
  - `probe_translator_snapshot_t::fsm` —
    `robotos_fw_fsm_snapshot_t` copy.
  - `probe_translator_snapshot_t::bridge` —
    `robotos_fw_event_bridge_snapshot_t` copy.
- **Side effects.** Writes `*out`. Does not mutate `*pt`. No UART,
  no hardware, no devkit call.
- **Status behavior.** Returns first non-OK from
  `robotos_fw_fsm_get_snapshot` or
  `robotos_fw_event_bridge_get_snapshot`. Returns
  `ROBOTOS_CORE_ERR_NULL` if `pt` or `out` is NULL.
- **Test expectation.** TC10 + TC12 use this to assert state +
  counters at host-test level.
- **DRAFT / NON-FINAL.**

### F.5 Notes on `probe_translator_t` / `probe_translator_config_t`

- The instance struct embeds (or references) the caller-owned FSM
  and bridge instances. The exact layout (embed vs. reference) is an
  open decision for Phase 12I (see §S #1).
- Memory layout is `DRAFT / EXPERIMENTAL`; treat all fields as
  opaque until ABI is locked by a later phase.
- **No header is created in Phase 12H.** Locking the API beyond names
  + shapes requires a future implementation phase that produces
  `probe_translator.h`.

---

## G. App-Local State Vocabulary

The future FSM uses the following planning-level state IDs. Names
are locked at Phase 12H. Numeric values are open (deferred to Phase
12I).

| State ID | Meaning | Entry condition | Exit condition | Non-goals | Relation to devkit states |
|---|---|---|---|---|---|
| `PROBE_TRANSLATOR_STATE_IDLE` | Initial state after `probe_translator_init` and after `PROBE_TRANSLATOR_EVT_RESET`. | `probe_translator_init` success; `PROBE_TRANSLATOR_EVT_RESET` from any state. | `PROBE_TRANSLATOR_EVT_CONFIGURED`. | Not a power state; not a devkit idle. | Coincidental name overlap with `DEVKIT_APP_STATE_IDLE`; no shared constants, no shared instance. |
| `PROBE_TRANSLATOR_STATE_READY` | Translator is configured and able to translate further events. | `PROBE_TRANSLATOR_EVT_CONFIGURED` from `IDLE`; `PROBE_TRANSLATOR_EVT_STOP` from `ACTIVE`. | `PROBE_TRANSLATOR_EVT_START` or `PROBE_TRANSLATOR_EVT_RESET`. | Not a devkit armed state; no hardware readiness implication. | Coincidental name overlap with `DEVKIT_APP_STATE_ARMED` at the *behavior level* (configured-but-idle). No shared constants. |
| `PROBE_TRANSLATOR_STATE_ACTIVE` | Translator has started and is processing synthetic adapter events. | `PROBE_TRANSLATOR_EVT_START` from `READY`. | `PROBE_TRANSLATOR_EVT_STOP`, `PROBE_TRANSLATOR_EVT_RESET`, or `PROBE_TRANSLATOR_EVT_FAULT` (if `FAULT` is included). | Not a devkit active state; no runtime hardware activity. | Coincidental name overlap with `DEVKIT_APP_STATE_ACTIVE`. No shared constants. |
| `PROBE_TRANSLATOR_STATE_FAULT` *(optional)* | Translator has received a synthetic fault event. | `PROBE_TRANSLATOR_EVT_FAULT` from any state. | `PROBE_TRANSLATOR_EVT_RESET`. | Not a real product fault; no actuator implication. | No devkit equivalent (devkit has no `FAULT` state in `devkit_app_state`). |

State vocabulary discipline:

1. The `PROBE_TRANSLATOR_STATE_` prefix marks these as
   application-local. They do not collide with `framework/` symbol
   space (no product names appear in `framework/`).
2. **Name overlap with `devkit_app_state` is coincidental.** The
   constants live in different namespaces; the runtime instances are
   unrelated; the application does not include
   `devkit_app_state.h`.
3. **`PROBE_TRANSLATOR_STATE_FAULT` may be deferred to Phase 12I**
   if the first implementation prefers a smaller initial coverage
   matrix. The skeleton spec marks it optional.

---

## H. App-Local Event Vocabulary

Planning-level event IDs. Names are locked at Phase 12H. Numeric
values are open (deferred to Phase 12I).

| Event ID | Source class | Host-synthetic? | Mapping purpose | Transition effect | Non-goals |
|---|---|---|---|---|---|
| `PROBE_TRANSLATOR_EVT_CONFIGURED` | Synthetic adapter (`PROBE_ADAPTER_TYPE_CONFIG`, `PROBE_ADAPTER_ARG_NONE`). | Yes. | Drive `IDLE -> READY` transition. | `PROBE_TRANSLATOR_STATE_IDLE -> READY`. | Not a UART command; not a devkit producer signal. |
| `PROBE_TRANSLATOR_EVT_START` | Synthetic adapter (`PROBE_ADAPTER_TYPE_COMMAND`, `PROBE_ADAPTER_ARG_START`). | Yes. | Drive `READY -> ACTIVE`. | `PROBE_TRANSLATOR_STATE_READY -> ACTIVE`. | Not a UART command. |
| `PROBE_TRANSLATOR_EVT_STOP` | Synthetic adapter (`PROBE_ADAPTER_TYPE_COMMAND`, `PROBE_ADAPTER_ARG_STOP`). | Yes. | Drive `ACTIVE -> READY`. | `PROBE_TRANSLATOR_STATE_ACTIVE -> READY`. | Not a UART command. |
| `PROBE_TRANSLATOR_EVT_RESET` | Synthetic adapter (`PROBE_ADAPTER_TYPE_COMMAND`, `PROBE_ADAPTER_ARG_RESET`). | Yes. | Drive any state -> `IDLE`. Tests state-reset semantics distinct from `robotos_fw_fsm_reset`. | `READY -> IDLE`; `ACTIVE -> IDLE`; (if FAULT) `FAULT -> IDLE`. | Not a UART command. |
| `PROBE_TRANSLATOR_EVT_FAULT` *(optional)* | Synthetic adapter (`PROBE_ADAPTER_TYPE_FAULT`, wildcard `arg0`). | Yes. | Drive any state -> `FAULT`. Tests wildcard arg0 mapping. | Any state -> `PROBE_TRANSLATOR_STATE_FAULT`. | Not a UART command; not a real fault path. |

Event vocabulary discipline:

1. **No event maps from a real core event** (no
   `ROBOTOS_EVENT_USER+N`). All five are synthetic-only.
2. **No event maps from a UART byte.** The application does not see
   UART input.
3. **No event maps from a button or hardware source.**
4. **No new `ROBOTOS_EVENT_USER` subrange is required** in `core/`.
5. **`PROBE_TRANSLATOR_EVT_FAULT` is optional.** If the
   implementation ships without it, row #4 in the mapping table
   (§J) is also dropped, and tests TC07 (FAULT wildcard) and TC10
   (FAULT-related counter checks) become not-applicable.

---

## I. Initial Transition Table Plan

Planning-level transition rows for the future
`probe_translator.c` translation unit. Row order is FIFO; the FSM
takes the first matching row whose guard (if any) returns true.

| Row | From state | Event | To state | Guard | Action | Required / optional |
|---|---|---|---|---|---|---|
| 0 | `PROBE_TRANSLATOR_STATE_IDLE` | `PROBE_TRANSLATOR_EVT_CONFIGURED` | `PROBE_TRANSLATOR_STATE_READY` | NULL (always allow) | NULL (no action) | **Required** |
| 1 | `PROBE_TRANSLATOR_STATE_READY` | `PROBE_TRANSLATOR_EVT_START` | `PROBE_TRANSLATOR_STATE_ACTIVE` | NULL | NULL | **Required** |
| 2 | `PROBE_TRANSLATOR_STATE_ACTIVE` | `PROBE_TRANSLATOR_EVT_STOP` | `PROBE_TRANSLATOR_STATE_READY` | NULL | NULL | **Required** |
| 3 | `PROBE_TRANSLATOR_STATE_READY` | `PROBE_TRANSLATOR_EVT_RESET` | `PROBE_TRANSLATOR_STATE_IDLE` | NULL | NULL | **Required** |
| 4 | `PROBE_TRANSLATOR_STATE_ACTIVE` | `PROBE_TRANSLATOR_EVT_RESET` | `PROBE_TRANSLATOR_STATE_IDLE` | NULL | NULL | **Required** |
| 5 | `PROBE_TRANSLATOR_STATE_IDLE` | `PROBE_TRANSLATOR_EVT_RESET` | `PROBE_TRANSLATOR_STATE_IDLE` | NULL | NULL | **Optional** (idempotent self-loop; aids `no_transition` accounting tests) |
| 6 | `PROBE_TRANSLATOR_STATE_IDLE` | `PROBE_TRANSLATOR_EVT_FAULT` | `PROBE_TRANSLATOR_STATE_FAULT` | NULL | NULL | **Optional** (only if `FAULT` shipped) |
| 7 | `PROBE_TRANSLATOR_STATE_READY` | `PROBE_TRANSLATOR_EVT_FAULT` | `PROBE_TRANSLATOR_STATE_FAULT` | NULL | NULL | **Optional** (only if `FAULT` shipped) |
| 8 | `PROBE_TRANSLATOR_STATE_ACTIVE` | `PROBE_TRANSLATOR_EVT_FAULT` | `PROBE_TRANSLATOR_STATE_FAULT` | NULL | NULL | **Optional** (only if `FAULT` shipped) |
| 9 | `PROBE_TRANSLATOR_STATE_FAULT` | `PROBE_TRANSLATOR_EVT_RESET` | `PROBE_TRANSLATOR_STATE_IDLE` | NULL | NULL | **Optional** (only if `FAULT` shipped) |

Row-discipline notes:

1. **No guards at first.** Phase 12H locks guard-NULL across all
   rows. Guard usage is open for Phase 12I or later.
2. **No actions at first.** Phase 12H locks action-NULL across all
   rows. The host tests do not depend on action side effects. Action
   usage is open for Phase 12I or later.
3. **Rows 6–9 form the fault block.** If `PROBE_TRANSLATOR_STATE_
   FAULT` is dropped at implementation, rows 6–9 are dropped
   together.
4. **Row 5 is idempotent.** Including it lets a host test assert
   that a RESET arriving in IDLE produces a committed self-transition
   rather than a `no_transition_count++`. The test choice is open
   (TC06.b in §K is conditional on this row).

---

## J. Initial Bridge Mapping Table Plan

Planning-level adapter-key conventions and mapping rows. Numeric
values are open (deferred to Phase 12I). Names are locked here.

### J.1 Application-local adapter type constants

| Constant | Meaning |
|---|---|
| `PROBE_ADAPTER_TYPE_CONFIG` | Synthetic config-channel adapter. |
| `PROBE_ADAPTER_TYPE_COMMAND` | Synthetic command-channel adapter. |
| `PROBE_ADAPTER_TYPE_FAULT` *(optional)* | Synthetic fault-channel adapter. Only declared if `FAULT` is shipped. |

These are **application-local** `uint32_t` tags. They are not
allocated in `core/`, do not collide with any
`ROBOTOS_EVENT_*` constant, and are never registered with
`robotos_core_register_event_handler` (the application owns the
bridge per Phase 12F §5).

### J.2 Application-local adapter arg0 constants

| Constant | Meaning |
|---|---|
| `PROBE_ADAPTER_ARG_NONE` | Default arg0 for config / no-payload events. |
| `PROBE_ADAPTER_ARG_START` | START command arg0. |
| `PROBE_ADAPTER_ARG_STOP` | STOP command arg0. |
| `PROBE_ADAPTER_ARG_RESET` | RESET command arg0. |
| `PROBE_ADAPTER_ARG_ANY` *(documentation alias)* | Conceptual placeholder for wildcard rows; not stored — wildcard semantics come from `row.match_arg0 == false` (Phase 12F §5). The constant `PROBE_ADAPTER_ARG_ANY` is documentation only and may be omitted from the header. |

### J.3 Mapping rows

| Row | `adapter_type` | `adapter_arg0` | `match_arg0` | `fw_event_id` | Expected transition (via §I) | Required / optional |
|---|---|---|---|---|---|---|
| 0 | `PROBE_ADAPTER_TYPE_CONFIG` | `PROBE_ADAPTER_ARG_NONE` | `true` | `PROBE_TRANSLATOR_EVT_CONFIGURED` | `IDLE -> READY` | **Required** |
| 1 | `PROBE_ADAPTER_TYPE_COMMAND` | `PROBE_ADAPTER_ARG_START` | `true` | `PROBE_TRANSLATOR_EVT_START` | `READY -> ACTIVE` | **Required** |
| 2 | `PROBE_ADAPTER_TYPE_COMMAND` | `PROBE_ADAPTER_ARG_STOP` | `true` | `PROBE_TRANSLATOR_EVT_STOP` | `ACTIVE -> READY` | **Required** |
| 3 | `PROBE_ADAPTER_TYPE_COMMAND` | `PROBE_ADAPTER_ARG_RESET` | `true` | `PROBE_TRANSLATOR_EVT_RESET` | any -> `IDLE` | **Required** |
| 4 | `PROBE_ADAPTER_TYPE_FAULT` | (ignored) | `false` (wildcard) | `PROBE_TRANSLATOR_EVT_FAULT` | any -> `FAULT` | **Optional** (only if `FAULT` shipped) |

Mapping discipline:

1. **Specific rows before wildcard rows.** Per Phase 12F §5.3, FIFO
   row order is the precedence rule. The required rows (0–3) all use
   `match_arg0 == true`. Row 4 is the only wildcard row, and it
   targets a distinct `adapter_type` (`PROBE_ADAPTER_TYPE_FAULT`),
   so the wildcard does not collide with the specific rows.
2. **No UART bytes mapped.** No row's `adapter_type` corresponds to
   `a/s/r/?/x/v/L/d/T`.
3. **No real hardware source mapped.** No row corresponds to
   `USER+1 / +2 / +3` core event types.
4. **One bridge instance, one FSM instance.** No fan-out (Phase
   12F-pre §11 #5 deferred).
5. **Unmapped events.** Any adapter event whose
   `(adapter_type, adapter_arg0)` does not match a row leaves the FSM
   state unchanged and increments `bridge.unmapped_count` (Phase 12F
   §5.3). TC08 in §K asserts this.

---

## K. Host Test Plan

The future host test
`RobotOS_v1.0/tests/host/test_app_probe_translator_mapping.c`
covers the following cases. Phase 12H locks the list and the names;
exact assertion counts are open until Phase 12I.

| # | Case | Coverage target |
|---|---|---|
| TC01 | `probe_translator_init` valid config | Init returns `OK`; FSM in `STATE_IDLE`; bridge counters zero; `initialized == true`. |
| TC02 | `probe_translator_init` invalid config | NULL pt → `ERR_NULL`; NULL config → `ERR_NULL`; empty mapping (row_count == 0) → `ERR_INVALID_ARG`; NULL FSM in config → `ERR_NULL`; NULL transition table → `ERR_NULL`. |
| TC03 | CONFIG → CONFIGURED → IDLE → READY | Adapter (`TYPE_CONFIG`, `ARG_NONE`) maps to `EVT_CONFIGURED`; FSM transitions `IDLE → READY`; counters: `event=1`, `mapped=1`, `unmapped=0`. |
| TC04 | COMMAND/START → START → READY → ACTIVE | Adapter (`TYPE_COMMAND`, `ARG_START`) maps to `EVT_START`; FSM transitions `READY → ACTIVE`. |
| TC05 | COMMAND/STOP → STOP → ACTIVE → READY | Adapter (`TYPE_COMMAND`, `ARG_STOP`) maps to `EVT_STOP`; FSM transitions `ACTIVE → READY`. |
| TC06 | COMMAND/RESET → RESET → {READY, ACTIVE} → IDLE | Two sub-cases: TC06.a `READY → IDLE`; TC06.b `ACTIVE → IDLE`. Optional sub-case TC06.c (`IDLE → IDLE` via row 5) only if row 5 is included. |
| TC07 | FAULT wildcard → any state → FAULT *(conditional)* | Only runs if `PROBE_TRANSLATOR_STATE_FAULT` is included. Three sub-cases: `IDLE`, `READY`, `ACTIVE` each → `FAULT`. Asserts wildcard arg0 row matches regardless of `adapter_arg0` value. |
| TC08 | Unmapped adapter event | Adapter (`0xDEADBEEF`, `0xCAFEBABE`) does not match any row; FSM state unchanged; `bridge.unmapped_count == 1`; `bridge.event_count` increments; return value `OK`. |
| TC09 | Payload borrowed through bridge to FSM | Pass a non-NULL payload pointer through `probe_translator_dispatch_adapter_event` and confirm the bridge does not cache it (snapshot has no payload field); confirm host-test-owned payload memory is untouched after dispatch returns. |
| TC10 | Counters via `probe_translator_get_snapshot` | After a sequence `CONFIG / START / STOP / RESET`, snapshot reports `fsm.event_count = 4`, `fsm.transition_count = 4`, `bridge.event_count = 4`, `bridge.mapped_count = 4`, `bridge.unmapped_count = 0`, `fsm.current_state == STATE_IDLE`. |
| TC11 | `probe_translator_reset` | Reset clears `bridge.event_count / mapped_count / unmapped_count` and returns FSM to `STATE_IDLE` regardless of prior state. FSM counters also reset per Phase 12E semantics. |
| TC12 | Full path `IDLE → READY → ACTIVE → READY → IDLE` | One continuous dispatch sequence; asserts state at each step + final counters. |
| TC13 | Grep gate: no `devkit_app_state.h` include | Build-time + manual review: `probe_translator.{c,h}` and `test_app_probe_translator_mapping.c` must not include `devkit_app_state.h`. Recorded as REVIEW_VALIDATED. |
| TC14 | Grep gate: no `a/s/r/?/x/v/L/d/T` reference | `probe_translator.{c,h}` and the host test must not reference the devkit command bytes or any `devkit_uart_*` symbol. REVIEW_VALIDATED. |
| TC15 | Grep gate: no Zephyr / devkit / legacy `ro_*` include | `probe_translator.{c,h}` and the host test must not include any `<zephyr/...>` header, any `devkit_*.h`, or any `ro_*.h` from `include/robotos/`. REVIEW_VALIDATED. |
| TC16 | Full host regression preserved | Adding this target keeps the full host suite passing at ≥22/22 (Phase 12F baseline) → ≥23/23 once the new target is added. |

Test-discipline notes:

1. **Coverage of every required mapping row (J §J.3 rows 0–3).** TC03,
   TC04, TC05, TC06 each exercise one row.
2. **Coverage of every required transition row (I §I rows 0–4).**
   TC03, TC04, TC05, TC06.a, TC06.b each exercise one row.
3. **Wildcard precedence.** TC07 implicitly covers wildcard behavior;
   no need to repeat the Phase 12F TC12 wildcard-beats-specific test
   here, since the application's mapping table places the wildcard
   row last (per §J.3) and at a distinct `adapter_type`.
4. **No new framework regression.** TC16 enforces that the bridge and
   FSM tests still pass with the new target attached.
5. **Negative-path tests (TC02, TC08, TC13–TC15)** are required at
   the first implementation.

---

## L. Build Strategy

Phase 12H locks the **preferred build strategy** at planning depth.
**No build change at Phase 12H.**

1. **No build change at Phase 12H.** Zero CMake / `prj.conf` / DTS /
   overlay / Zephyr config diffs.
2. **Preferred at first implementation: Option A.** Add a new
   `add_executable` + `add_test` block inside the existing
   `tests/host/CMakeLists.txt`, compiling:
   - `RobotOS_v1.0/app/probe_translator/probe_translator.c` (when
     created);
   - `RobotOS_v1.0/framework/robotos_fw_event_bridge.c`;
   - `RobotOS_v1.0/framework/robotos_fw_fsm.c`;
   - `RobotOS_v1.0/tests/host/test_app_probe_translator_mapping.c`;
   - the existing host critical-section stub
     (`robotos_platform_critical_host_stub.c`).
   With `target_include_directories` covering `framework/`, `core/`,
   `platform/`, `tests/host/`, and the new `app/probe_translator/`.
3. **Not preferred at first implementation: Option B (new
   `app/probe_translator/CMakeLists.txt`).** Acceptable once at
   least one more application exists; introduces a second build
   entry-point that is overkill for the first candidate.
4. **Not authorized: Option C (devkit integration).** Hardware
   evidence for the application target is a separate, later phase
   gated by explicit user authorization.
5. **No root CMake.** `RobotOS_v1.0/CMakeLists.txt` (Architecture B
   entry) is not used by the application target.
6. **No devkit CMake.** `RobotOS_v1.0/devkit/CMakeLists.txt` is not
   modified.
7. **Host environment.** WSL Ubuntu / gcc 13.3.0 — same as Phase 12E
   / 12F. MSYS2 MinGW64 remains broken and is not used.
8. **No hardware build at any planned phase before separate
   authorization.**

---

## M. Relationship to `devkit_app_state`

- `devkit_app_state` remains **authoritative** for the current
  devkit runtime. Phase 12H does **not** touch it.
- `probe_translator/`, when created, **must not** include
  `devkit_app_state.h`, must not call any `devkit_*` function, and
  must not read or write `devkit_app_state` snapshots.
- The application's `PROBE_TRANSLATOR_STATE_IDLE / READY / ACTIVE /
  FAULT` states are application-local; name overlap with devkit's
  `DEVKIT_APP_STATE_IDLE / ARMED / ACTIVE` is human-readable only.
- **No `?` UART response change.**
- **No `a/s/r/?/x/v/L/d/T` semantic change.**
- Any future devkit ↔ application interaction (shadow,
  replacement) requires a separate planning phase — not Phase 12H
  and not Phase 12I.
- **Scope-guard #11 remains active.**

---

## N. Relationship to Command Set

- The current command set `a / s / r / ? / x / v / L / d / T` remains
  the **devkit / probe surface**. Validated through Phase 11Z.
- `probe_translator/` **does not** define any UART command. It has
  no UART surface in the first host implementation phase.
- The Framework bridge has no UART surface (Phase 12F locked).
- A future application command vocabulary requires a separate
  product-command planning phase. Such a phase must:
  - list every byte / message it introduces;
  - declare which channel it uses (separate UART, network, etc.);
  - state explicitly that `a/s/r/?/x/v/L/d/T` semantics are
    unchanged;
  - provide its own validation strategy.
- All 12 UART TX scope-guard constraints from Phase 9EZ §H preserved.

---

## O. Relationship to Legacy Architecture B

- `RobotOS_v1.0/src/` and `RobotOS_v1.0/include/robotos/` remain
  frozen at `LEGACY_SCAFFOLD_MARKED_FROZEN_DOCS_ONLY` (Phase
  12D-pre).
- `probe_translator/`, when created, **must not** reuse `src/app/`,
  `include/robotos/app*`, or any other Architecture B artifact.
- `probe_translator/` uses **Architecture A contracts only**:
  `robotos_fw_*` (from `framework/`) and `robotos_core_status_t`
  (from `core/`).
- `probe_translator/` **must not include** any `ro_*` HAL header or
  any header in `include/robotos/`.
- **No Architecture A ↔ Architecture B reconciliation in Phase 12H.**

---

## P. Non-goals (Phase 12H)

| Non-goal | Status |
|---|---|
| `app/` directory created | NO |
| `application/` directory created | NO |
| `app/probe_translator/` directory created | NO |
| `probe_translator.{c,h}` / README created | NO |
| `CMakeLists.txt` change anywhere | NO |
| Framework code change | NO (`framework/` zero-diff) |
| Tests change | NO (`tests/` zero-diff) |
| Devkit runtime change | NO (`devkit/src/` zero-diff) |
| `devkit_app_state` change | NO |
| UART command added | NO |
| Command semantic change | NO |
| `prj.conf` / DTS / overlay change | NO |
| Zephyr config change | NO |
| Evidence log touched | NO |
| Hardware run | NO |
| Scheduler 7A/7B reopened | NO — `DEFER` preserved |
| F407 / custom board reopened | NO — `HOLD/DEFER` preserved |
| ACTIVE disarm widening started | NO — `USER_DECISION_REQUIRED_ACTIVE_DISARM` preserved |
| POST_FLASH_AUTOSTART change | NO — `OPEN/MITIGATED_BY_WORKFLOW` preserved |
| Legacy Architecture B modification | NO — frozen |
| Phase 12I started | NO — `NOT_STARTED` |
| Parser / shell / registry / framing / response queue added | NO |

---

## Q. Decision Result

**`PHASE_12H_PROBE_TRANSLATOR_SKELETON_PLANNED_DOCS_ONLY`**

The future `RobotOS_v1.0/app/probe_translator/` skeleton boundary is
locked at planning depth. The directory remains **not created**.
Phase 12I — Probe Translator Host Prototype Implementation Plan
(docs-only) or Phase 12I — Probe Translator Host Prototype
(implementation) may open only on **explicit user authorization**
with the recommended scope in §R.

Alternative outcome considered but **not** selected:

- `PHASE_12H_BLOCKED_NEEDS_APP_CONTRACT_DECISION` — not applicable;
  every required planning question (file set, API, state /
  event / transition / mapping vocab, host test, build) is answered
  in this closeout. No upstream contract is missing.

---

## R. Next Gate Recommendation

**Phase 12I-pre — Probe Translator Host Prototype Implementation
Plan (docs-only)**

| Field | Value |
|---|---|
| Title | Phase 12I-pre — Probe Translator Host Prototype Implementation Plan (docs-only) |
| Classification | Docs-only implementation planning |
| Purpose | Convert the Phase 12H skeleton (file list, API, state/event/mapping vocab, test plan, build strategy) into a concrete pre-implementation checklist for the host prototype: numeric constants for `PROBE_TRANSLATOR_STATE_*` / `_EVT_*` / `PROBE_ADAPTER_TYPE_*` / `_ARG_*`; final decision on `PROBE_TRANSLATOR_STATE_FAULT` (ship or defer); final decision on row 5 (`IDLE + RESET → IDLE`); host-test file naming pinned; CMake patch sketched in docs but not applied. |
| In-scope | Numeric values; ship-or-defer of optional pieces (FAULT block, row 5); locked test case count; locked grep gates; docs-only CMake patch sketch. |
| Non-goals | No `app/` directory creation. No source / header / CMake / README. No devkit change. No `devkit_app_state` change. No command-set change. No hardware run. |
| Exit criteria | A `PHASE_12I_PRE_PROBE_TRANSLATOR_IMPLEMENTATION_PLAN.md` closeout that resolves the open decisions in §S below. A revision of `PROBE_TRANSLATOR_APP_SKELETON_DRAFT.md` upgrading §4–§7 from "planning-level (names locked)" to "pre-implementation (names + values locked)". |
| Authorization | Must be opened only on **explicit user approval**. |

**Variant — Phase 12I (host prototype implementation)**

| Field | Value |
|---|---|
| Title | Phase 12I — Probe Translator Host Prototype |
| Classification | Host-first implementation |
| Purpose | Implement `app/probe_translator/probe_translator.{c,h}` + README, add a new host test target inside the existing `tests/host/CMakeLists.txt` (Option A), and produce host evidence under WSL Ubuntu / gcc 13.3.0. |
| In-scope | New `RobotOS_v1.0/app/probe_translator/` directory; first application source pair; new `tests/host/test_app_probe_translator_mapping.c`; additive entry in `tests/host/CMakeLists.txt`; new host log under `tests/host/logs/phase_12I_host_<YYYY-MM-DD>.log`. |
| Non-goals | No devkit change. No `devkit_app_state` change. No UART command. No hardware run. No legacy Architecture B modification. No Framework code change. |
| Exit criteria | All §K coverage targets met. Full host regression preserved (≥23/23 after the new target). New host test passes 100%. A `PHASE_12I_PROBE_TRANSLATOR_HOST_PROTOTYPE.md` closeout exists. `PROBE_TRANSLATOR_APP_SKELETON_DRAFT.md` upgrades to `IMPLEMENTED_AT_12I (HOST-TEST EVIDENCE)`. |
| Authorization | Must be opened only on **explicit user approval**. Recommended only if the user is comfortable skipping the Phase 12I-pre docs-only gate. |

**Preferred:** Phase 12I-pre docs-only unless the user explicitly
authorizes implementation directly.

**Alternative — HOLD.** If the user does not want to advance, the
phase chain stops. Framework path stays host-tested only;
`app/probe_translator/` remains uncreated; the skeleton spec sits at
planning depth indefinitely.

---

## S. Open Decisions Carried Forward

Items intentionally left open at Phase 12H:

| # | Open question | Latest at |
|---|---|---|
| 1 | Concrete numeric values for `PROBE_TRANSLATOR_STATE_*` and `PROBE_TRANSLATOR_EVT_*` | Phase 12I-pre |
| 2 | Concrete numeric values for `PROBE_ADAPTER_TYPE_*` and `PROBE_ADAPTER_ARG_*` | Phase 12I-pre |
| 3 | Whether `PROBE_TRANSLATOR_STATE_FAULT` + rows 6–9 + TC07 ship in the first implementation | Phase 12I-pre |
| 4 | Whether row 5 (`IDLE + RESET → IDLE`) ships in the first implementation | Phase 12I-pre |
| 5 | Whether `probe_translator_t` embeds the FSM + bridge instances by value or references them by pointer | Phase 12I |
| 6 | Whether `PROBE_ADAPTER_ARG_ANY` is declared as a documentation constant in the header or omitted entirely | Phase 12I |
| 7 | Whether `probe_translator_get_snapshot` returns a combined struct or two out-parameters | Phase 12I |
| 8 | Bridge ABI memory-layout lock (still open from Phase 12G §11 #9 / 12H-pre §13 #9) | Future application implementation phase or later |
| 9 | Whether the first application is also a hardware-runnable Zephyr application | Future runtime-integration phase |
| 10 | Whether to introduce `RobotOS_v1.0/examples/` for sample integrations | Future docs-only phase |

---

## T. Cross-references

- Phase 12H-pre first application candidate selection:
  [`PHASE_12H_PRE_FIRST_APPLICATION_CANDIDATE_SELECTION.md`](PHASE_12H_PRE_FIRST_APPLICATION_CANDIDATE_SELECTION.md)
- First-application-candidate spec:
  [`../03_SPECS/FIRST_APPLICATION_CANDIDATE_DRAFT.md`](../03_SPECS/FIRST_APPLICATION_CANDIDATE_DRAFT.md)
- Phase 12G separate application boundary planning:
  [`PHASE_12G_SEPARATE_APPLICATION_BOUNDARY_PLANNING.md`](PHASE_12G_SEPARATE_APPLICATION_BOUNDARY_PLANNING.md)
- Application boundary spec:
  [`../03_SPECS/FRAMEWORK_APPLICATION_BOUNDARY_DRAFT.md`](../03_SPECS/FRAMEWORK_APPLICATION_BOUNDARY_DRAFT.md)
- Phase 12G-pre devkit integration mode decision:
  [`PHASE_12G_PRE_DEVKIT_INTEGRATION_MODE_DECISION.md`](PHASE_12G_PRE_DEVKIT_INTEGRATION_MODE_DECISION.md)
- Long-lived devkit integration mode spec:
  [`../03_SPECS/FRAMEWORK_DEVKIT_INTEGRATION_MODE_DRAFT.md`](../03_SPECS/FRAMEWORK_DEVKIT_INTEGRATION_MODE_DRAFT.md)
- Phase 12F bridge host prototype:
  [`PHASE_12F_APPLICATION_BRIDGE_HOST_PROTOTYPE.md`](PHASE_12F_APPLICATION_BRIDGE_HOST_PROTOTYPE.md)
- Long-lived bridge spec:
  [`../03_SPECS/FRAMEWORK_APPLICATION_BRIDGE_DRAFT.md`](../03_SPECS/FRAMEWORK_APPLICATION_BRIDGE_DRAFT.md)
- Phase 12E FSM implementation:
  [`PHASE_12E_FSM_HOST_TEST_IMPLEMENTATION.md`](PHASE_12E_FSM_HOST_TEST_IMPLEMENTATION.md)
- Long-lived FSM spec:
  [`../03_SPECS/FRAMEWORK_FSM_API_DRAFT.md`](../03_SPECS/FRAMEWORK_FSM_API_DRAFT.md)
- Phase 11Z command-set checkpoint:
  [`PHASE_11Z_COMMAND_SET_CHECKPOINT.md`](PHASE_11Z_COMMAND_SET_CHECKPOINT.md)
- New long-lived probe translator skeleton spec:
  [`../03_SPECS/PROBE_TRANSLATOR_APP_SKELETON_DRAFT.md`](../03_SPECS/PROBE_TRANSLATOR_APP_SKELETON_DRAFT.md)
- Live state: [`../../../../CURRENT_STATE.md`](../../../../CURRENT_STATE.md)
