# RobotOS — Probe Translator App Skeleton Draft Spec

**Status:** `DRAFT / NON-FINAL`. **No application implementation
exists.** This document describes the planning-level skeleton of the
future `RobotOS_v1.0/app/probe_translator/` harness selected by
Phase 12H-pre and locked at Phase 12H. No `app/probe_translator/`
directory has been created. No `.c` / `.h` / `CMakeLists.txt` /
README file exists.
**Revision:** Phase 12I-pre (2026-05-13, `CLOSED_DOCS_ONLY`;
`PHASE_12I_PRE_PROBE_TRANSLATOR_HOST_PROTOTYPE_PLAN_CLOSED`) — added
cross-reference to the new execution-ready implementation contract
[`PROBE_TRANSLATOR_HOST_PROTOTYPE_PLAN.md`](PROBE_TRANSLATOR_HOST_PROTOTYPE_PLAN.md),
which resolves: numeric values (`IDLE=1u/READY=2u/ACTIVE=3u`;
`CONFIGURED=1u/START=2u/STOP=3u/RESET=4u`; adapter types 1/2;
adapter args 0–3), FAULT block deferred, transition row 5 deferred,
embed-by-value ownership, combined snapshot, `PROBE_ADAPTER_ARG_ANY`
omitted, exact CMake block, and 15-case host test plan (TC01–TC15).
`RobotOS_v1.0/app/probe_translator/` remains `NOT_CREATED`; sections
1–15 of this spec otherwise unchanged from the Phase 12H initial draft.
**Next revision condition:** Phase 12I (when authorized) — when the
first `app/probe_translator/` source and host test are implemented,
this spec upgrades §1 status to `IMPLEMENTED_AT_12I (HOST-TEST
EVIDENCE)` and gains the materialized file list and evidence
cross-references.

> **No application implementation exists.** The path
> `RobotOS_v1.0/app/probe_translator/` is *reserved at planning
> depth*, not created. No file under this path exists. No build
> target references this path.

This spec is anchored to
[`../02_PHASE_CLOSEOUTS/PHASE_12H_PROBE_TRANSLATOR_SKELETON_PLANNING.md`](../02_PHASE_CLOSEOUTS/PHASE_12H_PROBE_TRANSLATOR_SKELETON_PLANNING.md)
and extends
[`FIRST_APPLICATION_CANDIDATE_DRAFT.md`](FIRST_APPLICATION_CANDIDATE_DRAFT.md).
It references
[`FRAMEWORK_APPLICATION_BOUNDARY_DRAFT.md`](FRAMEWORK_APPLICATION_BOUNDARY_DRAFT.md),
[`FRAMEWORK_APPLICATION_BRIDGE_DRAFT.md`](FRAMEWORK_APPLICATION_BRIDGE_DRAFT.md),
and
[`FRAMEWORK_FSM_API_DRAFT.md`](FRAMEWORK_FSM_API_DRAFT.md).

---

## 1. Status / Scope

**What this doc is:**
A long-lived planning-level spec that locks the skeleton of the
first application harness (`probe_translator`): future file set,
public API names and shapes, app-local state / event / adapter-key
vocabulary, transition table, bridge mapping table, host test plan,
and build strategy preference.

**What this doc is not:**
- An application implementation. No `.c` / `.h` / `CMakeLists.txt`
  exists at `app/probe_translator/`.
- A product spec. The harness is product-neutral.
- A devkit integration spec.
- A command-vocabulary spec. The frozen `a/s/r/?/x/v/L/d/T` devkit
  command set is unchanged.
- A replacement for the first-application-candidate spec
  (`FIRST_APPLICATION_CANDIDATE_DRAFT.md`); this spec is the
  narrower skeleton lock under that umbrella.

**Current decision state (Phase 12H):**

| Decision | Status |
|---|---|
| First `<product>` placeholder | `probe_translator` (Phase 12H-pre) |
| First application path | `RobotOS_v1.0/app/probe_translator/` (reserved at planning depth; **not created**) |
| Skeleton planning | `PHASE_12H_PROBE_TRANSLATOR_SKELETON_PLANNED_DOCS_ONLY` |
| Public API names | Locked at Phase 12H (`probe_translator_init / _dispatch_adapter_event / _reset / _get_snapshot`) |
| Public API numeric / layout | Open (Phase 12I or later) |
| State ID names | Locked at Phase 12H |
| Event ID names | Locked at Phase 12H |
| Numeric values | Open (Phase 12I-pre) |
| `PROBE_TRANSLATOR_STATE_FAULT` block | **Optional**; ship-or-defer decided at Phase 12I-pre |
| Transition row 5 (`IDLE + RESET → IDLE`) | **Optional**; ship-or-defer decided at Phase 12I-pre |
| Build strategy preference | Option A (additive entry in existing `tests/host/CMakeLists.txt`) |
| `devkit_app_state` | Authoritative for devkit runtime; **not** touched |
| Command set | `a/s/r/?/x/v/L/d/T` unchanged |
| Architecture B | Frozen; not reachable from `app/probe_translator/` |

---

## 2. Future Directory and Files

Planning-level intent (locks at Phase 12I-pre or Phase 12I). The
directory `RobotOS_v1.0/app/probe_translator/` is **not created** by
Phase 12H.

### 2.1 Recommended minimal future file set

```text
RobotOS_v1.0/
├── core/
├── platform/
├── devkit/
├── framework/
│   ├── robotos_fw_fsm.{h,c}                 (LOCKED-AT-12D names)
│   └── robotos_fw_event_bridge.{h,c}        (LOCKED-AT-12F names + signatures)
├── tests/
│   └── host/
│       ├── CMakeLists.txt                   (host build wiring; Option A patch
│       │                                     planned but NOT YET APPLIED)
│       ├── test_robotos_fw_fsm.c            (existing; zero-diff)
│       ├── test_robotos_fw_event_bridge.c   (existing; zero-diff)
│       └── test_app_probe_translator_mapping.c   (FUTURE; NOT CREATED)
├── app/                                     (RESERVED AT PLANNING DEPTH;
│   │                                         NOT CREATED at Phase 12H)
│   └── probe_translator/                    (FIRST CANDIDATE;
│                                             NOT CREATED at Phase 12H)
│       ├── probe_translator.c               (FUTURE — public API bodies,
│       │                                     transition + mapping tables,
│       │                                     static instances)
│       ├── probe_translator.h               (FUTURE — public API
│       │                                     declarations, PROBE_TRANSLATOR_*
│       │                                     constants, adapter-key macros)
│       └── README.md                        (FUTURE — short app README;
│                                             purpose, host-test entry, explicit
│                                             non-goals)
├── src/                                     (Architecture B — frozen)
├── include/robotos/                         (Architecture B — frozen)
└── CMakeLists.txt                           (Architecture B — NOT USED by app/)
```

### 2.2 Optional future docs

| Future path | Add when |
|---|---|
| `RobotOS_v1.0/app/probe_translator/PROBE_TRANSLATOR_SPEC.md` | Only if this spec grows beyond a comfortable size or app-local implementation details (memory layout, ABI lock notes) accumulate. Otherwise omit. |

### 2.3 Forbidden at first implementation

- `RobotOS_v1.0/app/probe_translator/CMakeLists.txt` — defer to a
  later phase (Option B) once at least one more application exists.
- `RobotOS_v1.0/app/probe_translator/probe_translator.prj.conf` and
  any DTS overlay — host-only at first; no Zephyr build target.
- `RobotOS_v1.0/app/probe_translator/probe_translator_devkit.{c,h}` —
  no devkit integration before separate authorization.
- `RobotOS_v1.0/app/probe_translator/probe_translator_uart.{c,h}` —
  no UART surface.
- `RobotOS_v1.0/app/probe_translator/probe_translator_main.c` (Zephyr
  main) — no hardware runtime entry-point.
- Any RTT / J-Link / OpenOCD / flashing script under
  `app/probe_translator/` — no hardware run before separate
  authorization.
- Any include of `RobotOS_v1.0/src/app/` or `include/robotos/app*`
  — Architecture B is frozen.

---

## 3. Responsibility Boundary

### 3.1 What `probe_translator/` may own

| Responsibility | Detail |
|---|---|
| App-local state IDs | `PROBE_TRANSLATOR_STATE_*` constants. |
| App-local event IDs | `PROBE_TRANSLATOR_EVT_*` constants. |
| Transition table | Static const `robotos_fw_transition_t[]`. |
| Bridge mapping table | Static const `robotos_fw_event_bridge_row_t[]`. |
| FSM + bridge instance allocation | Static `robotos_fw_fsm_t` and `robotos_fw_event_bridge_t` — caller-owned at app scope, lifetime-tied to the harness. |
| Public API bodies | `probe_translator_init / _dispatch_adapter_event / _reset / _get_snapshot`. |
| Host-test fixture constants | `PROBE_ADAPTER_TYPE_*` and `PROBE_ADAPTER_ARG_*`. |
| App-local README | Short purpose + entry-point + non-goals. |

### 3.2 What `probe_translator/` must NOT own

| Non-responsibility | Reason |
|---|---|
| Framework FSM / bridge algorithms | `framework/` is `LOCKED-AT-12D` / `LOCKED-AT-12F`. |
| Core queue / dispatcher internals | `core/` is canonical. |
| Platform backend primitives | `platform/` owns critical-section / time / fault / log boundaries. |
| `devkit_app_state` | Devkit-local; scope-guard #11 enforced. |
| Devkit command semantics (`a/s/r/?/x/v/L/d/T`) | Devkit/probe surface; unchanged. |
| UART TX | No UART at the application layer. |
| Zephyr drivers / device-tree | No hardware binding at first. |
| Legacy Architecture B (`ro_*`, `src/`, `include/robotos/`) | Frozen at Phase 12D-pre. |

---

## 4. Public App API Draft

Names are locked at Phase 12H. Numeric values, exact struct layout,
and ABI memory placement are open. **No header file is created.**

### 4.1 `probe_translator_init`

```c
robotos_fw_status_t probe_translator_init(
    probe_translator_t              *pt,
    const probe_translator_config_t *config);
```

Initialize a caller-owned harness instance. Wires up the embedded
FSM + bridge in one call. Returns first non-OK from FSM init or
bridge init. `ROBOTOS_CORE_ERR_NULL` on NULL inputs.

### 4.2 `probe_translator_dispatch_adapter_event`

```c
robotos_fw_status_t probe_translator_dispatch_adapter_event(
    probe_translator_t *pt,
    uint32_t            adapter_type,
    uint32_t            adapter_arg0,
    const void         *payload);
```

Forward a synthetic adapter event tuple through the bridge into the
FSM. Returns bridge's status verbatim. `payload` is borrowed for the
call only.

### 4.3 `probe_translator_reset`

```c
robotos_fw_status_t probe_translator_reset(probe_translator_t *pt);
```

Reset bridge counters and FSM state (in that order). Returns first
non-OK from `robotos_fw_event_bridge_reset` or
`robotos_fw_fsm_reset`. `ROBOTOS_CORE_ERR_NULL` on NULL input.

### 4.4 `probe_translator_get_snapshot`

```c
robotos_fw_status_t probe_translator_get_snapshot(
    const probe_translator_t    *pt,
    probe_translator_snapshot_t *out);
```

Combined FSM + bridge snapshot for host-test convenience.

### 4.5 Open layout decisions

- Whether `probe_translator_t` embeds the FSM and bridge by value or
  references them by pointer is open at Phase 12I.
- Whether `probe_translator_snapshot_t` returns a combined struct or
  two out-parameters is open at Phase 12I.
- Whether `PROBE_ADAPTER_ARG_ANY` is declared in the header (as a
  documentation constant) or omitted is open at Phase 12I.

---

## 5. State Vocabulary

| State | Required / optional |
|---|---|
| `PROBE_TRANSLATOR_STATE_IDLE` | Required. |
| `PROBE_TRANSLATOR_STATE_READY` | Required. |
| `PROBE_TRANSLATOR_STATE_ACTIVE` | Required. |
| `PROBE_TRANSLATOR_STATE_FAULT` | **Optional.** Ship-or-defer at Phase 12I-pre. |

Discipline:

- All names are application-local (`PROBE_TRANSLATOR_STATE_` prefix).
- Name overlap with `DEVKIT_APP_STATE_IDLE / ARMED / ACTIVE` is
  coincidental at the human-readable level only.
- Numeric values are open. Phase 12I-pre or Phase 12I assigns them.
  `ROBOTOS_FW_STATE_UNINIT == 0` (Framework reserved sentinel) — no
  application state may use the value `0`.

---

## 6. Event Vocabulary

| Event | Required / optional |
|---|---|
| `PROBE_TRANSLATOR_EVT_CONFIGURED` | Required. |
| `PROBE_TRANSLATOR_EVT_START` | Required. |
| `PROBE_TRANSLATOR_EVT_STOP` | Required. |
| `PROBE_TRANSLATOR_EVT_RESET` | Required. |
| `PROBE_TRANSLATOR_EVT_FAULT` | **Optional.** Ship-or-defer at Phase 12I-pre. |

Discipline:

- All events are synthetic-only at first.
- No event maps from a UART byte, button, hardware producer, or any
  `ROBOTOS_EVENT_USER+N` core event.
- No new `ROBOTOS_EVENT_USER` subrange is required in `core/`.
- Numeric values are open. Phase 12I-pre or Phase 12I assigns them.

---

## 7. Transition Table Plan

Planning-level rows. Row order is FIFO (the FSM takes the first
matching row whose guard returns true).

| Row | From | Event | To | Required / optional |
|---|---|---|---|---|
| 0 | `STATE_IDLE` | `EVT_CONFIGURED` | `STATE_READY` | Required |
| 1 | `STATE_READY` | `EVT_START` | `STATE_ACTIVE` | Required |
| 2 | `STATE_ACTIVE` | `EVT_STOP` | `STATE_READY` | Required |
| 3 | `STATE_READY` | `EVT_RESET` | `STATE_IDLE` | Required |
| 4 | `STATE_ACTIVE` | `EVT_RESET` | `STATE_IDLE` | Required |
| 5 | `STATE_IDLE` | `EVT_RESET` | `STATE_IDLE` | Optional (self-loop) |
| 6 | `STATE_IDLE` | `EVT_FAULT` | `STATE_FAULT` | Optional (if FAULT shipped) |
| 7 | `STATE_READY` | `EVT_FAULT` | `STATE_FAULT` | Optional (if FAULT shipped) |
| 8 | `STATE_ACTIVE` | `EVT_FAULT` | `STATE_FAULT` | Optional (if FAULT shipped) |
| 9 | `STATE_FAULT` | `EVT_RESET` | `STATE_IDLE` | Optional (if FAULT shipped) |

All rows use `guard = NULL` and `action = NULL` at first
implementation. Guard / action usage is open for later phases.

---

## 8. Bridge Mapping Table Plan

### 8.1 Application-local adapter types

| Constant | Required / optional |
|---|---|
| `PROBE_ADAPTER_TYPE_CONFIG` | Required. |
| `PROBE_ADAPTER_TYPE_COMMAND` | Required. |
| `PROBE_ADAPTER_TYPE_FAULT` | Optional (only if FAULT shipped). |

### 8.2 Application-local adapter args

| Constant | Purpose |
|---|---|
| `PROBE_ADAPTER_ARG_NONE` | Default arg0 for config events. |
| `PROBE_ADAPTER_ARG_START` | START command arg0. |
| `PROBE_ADAPTER_ARG_STOP` | STOP command arg0. |
| `PROBE_ADAPTER_ARG_RESET` | RESET command arg0. |
| `PROBE_ADAPTER_ARG_ANY` | Documentation alias only (wildcard semantics come from `row.match_arg0 == false`); may be omitted from header. |

### 8.3 Mapping rows

| Row | adapter_type | adapter_arg0 | match_arg0 | fw_event_id | Required / optional |
|---|---|---|---|---|---|
| 0 | `TYPE_CONFIG` | `ARG_NONE` | `true` | `EVT_CONFIGURED` | Required |
| 1 | `TYPE_COMMAND` | `ARG_START` | `true` | `EVT_START` | Required |
| 2 | `TYPE_COMMAND` | `ARG_STOP` | `true` | `EVT_STOP` | Required |
| 3 | `TYPE_COMMAND` | `ARG_RESET` | `true` | `EVT_RESET` | Required |
| 4 | `TYPE_FAULT` | (ignored) | `false` (wildcard) | `EVT_FAULT` | Optional (if FAULT shipped) |

Discipline:

- Specific rows precede the wildcard row in declaration order. The
  wildcard row uses a distinct `adapter_type`
  (`PROBE_ADAPTER_TYPE_FAULT`), so it does not collide with the
  specific rows.
- No UART bytes mapped (no `a/s/r/?/x/v/L/d/T`).
- No real hardware source mapped (no `USER+1/+2/+3`).
- One bridge instance, one FSM instance — no fan-out (Phase 12F-pre
  §11 #5 deferred).
- Unmapped events leave FSM state unchanged and increment
  `bridge.unmapped_count` per Phase 12F §5.3.

---

## 9. Host Test Plan

Locked case list (full text in
[`../02_PHASE_CLOSEOUTS/PHASE_12H_PROBE_TRANSLATOR_SKELETON_PLANNING.md`](../02_PHASE_CLOSEOUTS/PHASE_12H_PROBE_TRANSLATOR_SKELETON_PLANNING.md)
§K):

| # | Case |
|---|---|
| TC01 | `probe_translator_init` valid config |
| TC02 | `probe_translator_init` invalid config (NULL pt / NULL config / NULL FSM / NULL transitions / row_count == 0) |
| TC03 | CONFIG / NONE → CONFIGURED → IDLE → READY |
| TC04 | COMMAND / START → START → READY → ACTIVE |
| TC05 | COMMAND / STOP → STOP → ACTIVE → READY |
| TC06 | COMMAND / RESET → RESET → {READY, ACTIVE} → IDLE; optional TC06.c if row 5 ships |
| TC07 | FAULT wildcard → any → FAULT (conditional on FAULT block) |
| TC08 | Unmapped adapter event |
| TC09 | Payload borrowed through bridge to FSM |
| TC10 | Counters via `probe_translator_get_snapshot` |
| TC11 | `probe_translator_reset` clears bridge counters and FSM state |
| TC12 | Full path `IDLE → READY → ACTIVE → READY → IDLE` |
| TC13 | Grep gate: no `devkit_app_state.h` include |
| TC14 | Grep gate: no `a/s/r/?/x/v/L/d/T` reference |
| TC15 | Grep gate: no Zephyr / devkit / legacy `ro_*` include |
| TC16 | Full host regression preserved (≥23/23 after new target) |

Test naming convention:
`RobotOS_v1.0/tests/host/test_app_probe_translator_mapping.c`.

---

## 10. Build Strategy

Preferred (locks at Phase 12I-pre or Phase 12I, not at Phase 12H):

1. **No build change at Phase 12H.** Zero CMake / `prj.conf` / DTS /
   overlay / Zephyr config diffs.
2. **Option A — additive entry in existing
   `tests/host/CMakeLists.txt`.** Preferred at first
   implementation. Compiles:
   - `RobotOS_v1.0/app/probe_translator/probe_translator.c`;
   - `RobotOS_v1.0/framework/robotos_fw_event_bridge.c`;
   - `RobotOS_v1.0/framework/robotos_fw_fsm.c`;
   - `RobotOS_v1.0/tests/host/test_app_probe_translator_mapping.c`;
   - existing `robotos_platform_critical_host_stub.c`.
3. **Option B — new `app/probe_translator/CMakeLists.txt`.**
   Acceptable later, not preferred at first.
4. **Option C — devkit integration.** NOT authorized.
5. **Host environment.** WSL Ubuntu / gcc 13.3.0 (Phase 12E / 12F
   baseline).
6. **No root CMake.** Architecture B build entry not used.
7. **No devkit CMake change.**
8. **No hardware build** at any planned phase before separate
   authorization.

---

## 11. `devkit_app_state` Boundary

- `devkit_app_state` remains authoritative for the devkit runtime.
- `probe_translator/`, when created, **must not** include
  `devkit_app_state.h`, must not call any `devkit_*` function, and
  must not read or write `devkit_app_state` snapshots.
- `PROBE_TRANSLATOR_STATE_IDLE / READY / ACTIVE / FAULT` are
  application-local. Name overlap with `DEVKIT_APP_STATE_IDLE /
  ARMED / ACTIVE` is human-readable only.
- No `?` UART response change. No `a/s/r/?/x/v/L/d/T` semantic
  change.
- Future devkit ↔ application interaction requires a separate
  planning phase.
- Scope-guard #11 remains active.

---

## 12. Command Set Boundary

- `a / s / r / ? / x / v / L / d / T` remain the devkit / probe
  surface.
- `probe_translator/` defines no UART command.
- Framework bridge has no UART surface (Phase 12F locked).
- Future application command vocabulary requires a separate product-
  command planning phase.
- All 12 UART TX scope-guard constraints from Phase 9EZ §H preserved.

---

## 13. Legacy Architecture B Boundary

- `RobotOS_v1.0/src/` and `RobotOS_v1.0/include/robotos/` remain
  frozen at `LEGACY_SCAFFOLD_MARKED_FROZEN_DOCS_ONLY` (Phase
  12D-pre).
- `probe_translator/` must not reuse `src/app/`,
  `include/robotos/app*`, or any other Architecture B artifact.
- `probe_translator/` uses Architecture A contracts only
  (`robotos_fw_*` + `robotos_core_status_t`).
- No `ro_*` HAL include.
- No Architecture A ↔ Architecture B reconciliation in Phase 12H.

---

## 14. Open Decisions

| # | Open question | Latest at |
|---|---|---|
| 1 | Numeric values for `PROBE_TRANSLATOR_STATE_*` and `_EVT_*` | Phase 12I-pre |
| 2 | Numeric values for `PROBE_ADAPTER_TYPE_*` and `_ARG_*` | Phase 12I-pre |
| 3 | Whether `PROBE_TRANSLATOR_STATE_FAULT` + rows 6–9 + TC07 ship in the first implementation | Phase 12I-pre |
| 4 | Whether row 5 (`IDLE + RESET → IDLE`) ships in the first implementation | Phase 12I-pre |
| 5 | Whether `probe_translator_t` embeds or references the FSM + bridge | Phase 12I |
| 6 | Whether `PROBE_ADAPTER_ARG_ANY` is declared as a header constant | Phase 12I |
| 7 | Whether `probe_translator_get_snapshot` returns a combined struct or two out-params | Phase 12I |
| 8 | Bridge ABI memory-layout lock (still open from Phase 12G §11 #9) | Future implementation phase |
| 9 | Whether the first application is also a hardware-runnable Zephyr application | Future runtime-integration phase |
| 10 | Whether to introduce `RobotOS_v1.0/examples/` for sample integrations | Future docs-only phase |

---

## 15. Next Revision Conditions

This spec is revised when:

1. **Phase 12I-pre opens (docs-only implementation plan).** §4–§7
   upgrade from "names locked, values open" to "names + values
   locked". §14 #1–#4 are resolved.
2. **Phase 12I opens (host prototype implementation).** §1 status
   becomes `IMPLEMENTED_AT_12I (HOST-TEST EVIDENCE)`. §2 lists the
   materialized file set. §4 references `probe_translator.h`. §9
   references the materialized host test file. §14 #5–#7 are
   resolved.
3. **A user override picks Phase 12I directly (skipping 12I-pre).**
   §14 #1–#4 must be resolved as part of Phase 12I's implementation
   plan.
4. **A second application is added under `app/`.** §14 #10 triggers
   updates; multi-application coordination rules added.
5. **A future phase reconciles or removes Architecture B.** §13 is
   updated.
6. **A user override picks `HOLD`.** §1 status updates to record the
   override; no further revision required.

This spec is **NOT** revised to:

- Record devkit implementation details (devkit docs are authoritative
  for devkit).
- Track per-phase test results (closeouts are authoritative).
- Pick a product's specific business logic.
- Change command semantics (frozen at 11Z).
- Promote `devkit_app_state` (scope-guard #11 enforced).
- Reactivate Architecture B (frozen at 12D-pre).
