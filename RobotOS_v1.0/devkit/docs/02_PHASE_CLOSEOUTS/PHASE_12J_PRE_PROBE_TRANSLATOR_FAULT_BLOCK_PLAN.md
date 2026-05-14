# Phase 12J-pre — Probe Translator FAULT Block Plan (docs-only)

**Status:** `CLOSED_DOCS_ONLY`
**Decision:** `PHASE_12J_PRE_PROBE_TRANSLATOR_FAULT_BLOCK_PLAN_CLOSED`
**Date:** 2026-05-13
**Branch / baseline:** `master` at `origin/master = b7fe5e2` (Phase 12I)
**Prior phase anchor:** [`PHASE_12I_PROBE_TRANSLATOR_HOST_PROTOTYPE.md`](PHASE_12I_PROBE_TRANSLATOR_HOST_PROTOTYPE.md)
**New long-lived spec:** [`../03_SPECS/PROBE_TRANSLATOR_FAULT_BLOCK_PLAN.md`](../03_SPECS/PROBE_TRANSLATOR_FAULT_BLOCK_PLAN.md)

---

## A. Executive Summary

Phase 12J-pre is a **docs-only implementation-planning gate** that
converts Phase 12I's deferred FAULT block (state, event, adapter type,
transition rows 5-9, mapping row 4) into an execution-ready
implementation contract for **Phase 12J — Probe Translator FAULT
Block Extension**.

Phase 12J-pre **does**:

- commit the numeric values for `PROBE_TRANSLATOR_STATE_FAULT`
  (`4u`), `PROBE_TRANSLATOR_EVT_FAULT` (`5u`),
  `PROBE_ADAPTER_TYPE_FAULT` (`3u`), and `PROBE_ADAPTER_ARG_ANY`
  (`0xFFFFFFFFu` documentation alias);
- decide ship-or-defer for transition row 5 (`IDLE + RESET → IDLE`
  self-loop): **SHIPPED** at Phase 12J;
- decide ship-or-defer for transition rows 6-9 (FAULT block):
  **SHIPPED** at Phase 12J;
- decide ship-or-defer for mapping row 4 (FAULT wildcard):
  **SHIPPED** at Phase 12J (uses existing Phase 12F
  `match_arg0 == false` wildcard feature; bridge `.c` / `.h`
  zero-diff);
- decide ship-or-defer for `PROBE_ADAPTER_ARG_ANY` header
  declaration: **SHIPPED** at Phase 12J as a documentation alias
  only;
- decide ship-or-defer for sticky-FAULT behavior on
  CONFIG/START/STOP from FAULT: **implicit sticky** — no extra rows
  needed; FSM `no_transition_count++` handles the case;
- define the exact future diff list Phase 12J may make to existing
  files;
- define the exact new file list Phase 12J may create;
- define the exact future host test case list (TC16-TC24, ~9 new
  cases on top of retained Phase 12I TC01-TC15);
- freeze the implementation contract in a new long-lived spec
  [`PROBE_TRANSLATOR_FAULT_BLOCK_PLAN.md`](../03_SPECS/PROBE_TRANSLATOR_FAULT_BLOCK_PLAN.md);
- preserve every other open gate untouched.

Phase 12J-pre **does not**:

- modify any `.c`, `.h`, test source, CMake, devkit runtime,
  framework, core, platform, `prj.conf`, board DTS, overlay, or
  Zephyr config file;
- create or modify `app/probe_translator/CMakeLists.txt`;
- modify any evidence log;
- change `devkit_app_state`;
- change the frozen `a / s / r / ? / x / v / L / d / T` command set;
- run hardware;
- open Phase 12J.

Phase 12J is **not started** at the close of Phase 12J-pre. It may
open only on **explicit user authorization**.

---

## B. Baseline Before Phase 12J-pre

| Item | Value |
|---|---|
| `origin/master` baseline | `b7fe5e2` `app: add Phase 12I probe translator host prototype` |
| Phase 12I status | `CLOSED_WITH_HOST_TEST_EVIDENCE` / `PHASE_12I_PROBE_TRANSLATOR_HOST_PROTOTYPE_IMPLEMENTED_VALIDATED` |
| `app/probe_translator/` directory | **MATERIALIZED** at Phase 12I (`probe_translator.{h,c}` + `README.md`) |
| `probe_translator.h` FAULT-related state | `PROBE_TRANSLATOR_STATE_FAULT`, `_EVT_FAULT`, `_ADAPTER_TYPE_FAULT` reserved in comments only (`4u/5u/3u`); `PROBE_ADAPTER_ARG_ANY` omitted. **Not declared.** |
| `probe_translator.c` transition table | 5 rows (Phase 12H rows 0-4; no self-loop; no FAULT) |
| `probe_translator.c` mapping table | 4 rows (all `match_arg0 = true`; no wildcard) |
| `probe_translator.c` state defs | 3 entries (`IDLE`, `READY`, `ACTIVE`) |
| `tests/host/test_app_probe_translator_mapping.c` | TC01..TC15 (15 logical cases; 70 in-binary assertions PASS) |
| Full host regression | 23/23 PASS under WSL Ubuntu / gcc 13.3.0 |
| Framework FSM | `framework/robotos_fw_fsm.{h,c}` host-tested (93/93) |
| Framework event bridge | `framework/robotos_fw_event_bridge.{h,c}` host-tested (103/103); §5 `LOCKED-AT-12F`; supports `match_arg0 == false` wildcard |
| Devkit integration | `NOT_STARTED` |
| Frozen command set | `a / s / r / ? / x / v / L / d / T` (unchanged through Phase 12I) |
| Scope-guard #11 | `devkit_app_state` devkit-local; preserved |
| Open gates carried in | ACTIVE disarm `USER_DECISION_REQUIRED_ACTIVE_DISARM`; Scheduler 7A/7B `DEFER`; F407 `HOLD/DEFER`; POST_FLASH_AUTOSTART `OPEN/MITIGATED_BY_WORKFLOW`; FAULT block `DEFERRED` (this phase resolves the planning, not the implementation); bridge ABI memory-layout lock `NOT_STARTED`; hardware-runnable Zephyr build `NOT_STARTED` |

---

## C. Implementation Planning Problem Statement

Phase 12I locked numeric reservations for the FAULT block and the
`IDLE + RESET` self-loop but intentionally left them deferred. Phase
12J cannot proceed without resolving:

1. **Are the reserved numeric values still correct?** The Phase 12I
   header carries `4u/5u/3u` in comments but does not declare them.
   Phase 12J needs the values committed in `#define` form.
2. **Should `PROBE_ADAPTER_ARG_ANY` be declared in the header at all,
   and if so, with what numeric value and what doc-vs-runtime
   semantics?** Phase 12I deferred this entirely.
3. **Should transition row 5 (`IDLE + RESET → IDLE` self-loop)
   ship?** Phase 12I deferred. Phase 12J needs the binary decision
   before writing source.
4. **Should transition rows 6-9 (FAULT block) ship as a group, or in
   smaller increments?** Phase 12I deferred.
5. **Should mapping row 4 use `match_arg0 == false` (wildcard) or
   explicit `(TYPE_FAULT, ARG_NONE)` exact match?** The Phase 12H
   skeleton sketched a wildcard, but Phase 12I-pre §E.4 explicitly
   omitted ARG_ANY without resolving the wildcard direction.
6. **What is the behavior of CONFIG/START/STOP events while in
   STATE_FAULT?** Sticky? Rejected? Unmapped at bridge level?
7. **Does the bridge need a code change to support the wildcard
   row?** This requires checking the locked Phase 12F bridge
   contract.
8. **What is the impact on the existing TC01-TC15 host test
   contract?** Phase 12J must not silently flip Phase 12I
   assertions.

Phase 12J-pre resolves all eight questions. Phase 12J then has a
complete, unambiguous implementation contract.

---

## D. Phase 12J Approved Future Diff / File Set

The following are the **only** files Phase 12J may create or modify.
Nothing outside this set is in scope.

### D.1 Modified existing files (additive only)

| Path | Change at Phase 12J |
|---|---|
| `RobotOS_v1.0/app/probe_translator/probe_translator.h` | Additive: declare `PROBE_TRANSLATOR_STATE_FAULT (4u)`, `_EVT_FAULT (5u)`, `_ADAPTER_TYPE_FAULT (3u)`, `_ADAPTER_ARG_ANY (0xFFFFFFFFu)`. Comments updated. Public API and struct layout **zero-diff**. |
| `RobotOS_v1.0/app/probe_translator/probe_translator.c` | Additive: 5 new transition rows; 1 new state-def entry; 1 new mapping row; `transition_count` 5→10; `state_count` 3→4; `row_count` 4→5 in `probe_translator_init`. Public API bodies and struct layout **zero-diff**. |
| `RobotOS_v1.0/app/probe_translator/README.md` | Additive: FAULT-related symbols appended to the state/event/adapter-key table; "Non-goals" unchanged. |
| `RobotOS_v1.0/tests/host/test_app_probe_translator_mapping.c` | Additive: TC16-TC24 appended. TC01-TC15 retained verbatim. |

### D.2 New files

| Path | Purpose |
|---|---|
| `RobotOS_v1.0/tests/host/logs/phase_12J_host_<YYYY-MM-DD>.log` | Full `ctest --verbose` transcript; committed evidence. |
| `RobotOS_v1.0/devkit/docs/02_PHASE_CLOSEOUTS/PHASE_12J_PROBE_TRANSLATOR_FAULT_BLOCK.md` | Phase 12J closeout. |

### D.3 Doc-sync updates at Phase 12J close

- `RobotOS_v1.0/devkit/docs/03_SPECS/PROBE_TRANSLATOR_FAULT_BLOCK_PLAN.md`
  (status `DRAFT / NON-FINAL` → `IMPLEMENTED_AT_12J (HOST-TEST EVIDENCE)`)
- `RobotOS_v1.0/devkit/docs/03_SPECS/PROBE_TRANSLATOR_HOST_PROTOTYPE_PLAN.md`
  (forward-reference to FAULT block now shipped; Phase 12I content
  preserved verbatim)
- `RobotOS_v1.0/devkit/docs/03_SPECS/PROBE_TRANSLATOR_APP_SKELETON_DRAFT.md`
  (§5 / §6 / §7 / §8 update: FAULT block + transition row 5 marked
  shipped)
- `RobotOS_v1.0/devkit/docs/01_PROGRESS/DEVKIT_PROGRESS_PHASE_11_20.md`
- `CURRENT_STATE.md`
- `RobotOS_v1.0/devkit/docs/00_INDEX/README.md`

### D.4 Forbidden at Phase 12J

| Forbidden | Reason |
|---|---|
| `RobotOS_v1.0/app/probe_translator/CMakeLists.txt` | Option B remains deferred. |
| `RobotOS_v1.0/tests/host/CMakeLists.txt` change | **Must be zero-diff.** The Phase 12I additive block already compiles `probe_translator.c`. FAULT block additions live in the same compilation unit; CMake automatically picks them up. Any CMake edit would expand the patch surface. |
| Any `framework/` / `core/` / `platform/` / `devkit/src/` file | The bridge's existing `match_arg0 == false` semantics are what the wildcard row uses; no Framework change needed. |
| Any `src/` / `include/robotos/` file | Architecture B frozen. |
| Any Zephyr / `prj.conf` / DTS / overlay file | Host-only at Phase 12J. |
| Any new `app/<product>/` subdirectory | One product at a time. |
| Any new ctest target | Extend `probe_translator_mapping_contract`. |
| Any RTT / J-Link / OpenOCD / pyOCD / flashing script | No hardware run. |
| Any UART command byte literal | `a/s/r/?/x/v/L/d/T` frozen surface. |
| Any change to `devkit_app_state.{c,h}` | Scope-guard #11. |

---

## E. Numeric ID Plan

All values formalize the Phase 12I-pre reservations. They are final
for Phase 12J.

### E.1 State

| Constant | Value | Status at Phase 12J |
|---|---|---|
| `PROBE_TRANSLATOR_STATE_IDLE` | `1u` | unchanged (Phase 12I) |
| `PROBE_TRANSLATOR_STATE_READY` | `2u` | unchanged |
| `PROBE_TRANSLATOR_STATE_ACTIVE` | `3u` | unchanged |
| `PROBE_TRANSLATOR_STATE_FAULT` | `4u` | **NEW** — formalized from Phase 12I-pre §E.1 reservation |

### E.2 Events

| Constant | Value | Status at Phase 12J |
|---|---|---|
| `PROBE_TRANSLATOR_EVT_CONFIGURED` | `1u` | unchanged |
| `PROBE_TRANSLATOR_EVT_START` | `2u` | unchanged |
| `PROBE_TRANSLATOR_EVT_STOP` | `3u` | unchanged |
| `PROBE_TRANSLATOR_EVT_RESET` | `4u` | unchanged |
| `PROBE_TRANSLATOR_EVT_FAULT` | `5u` | **NEW** — formalized from Phase 12I-pre §E.2 reservation |

### E.3 Adapter types

| Constant | Value | Status at Phase 12J |
|---|---|---|
| `PROBE_ADAPTER_TYPE_CONFIG` | `1u` | unchanged |
| `PROBE_ADAPTER_TYPE_COMMAND` | `2u` | unchanged |
| `PROBE_ADAPTER_TYPE_FAULT` | `3u` | **NEW** — formalized from Phase 12I-pre §E.3 reservation |

### E.4 Adapter args

| Constant | Value | Status at Phase 12J |
|---|---|---|
| `PROBE_ADAPTER_ARG_NONE` | `0u` | unchanged |
| `PROBE_ADAPTER_ARG_START` | `1u` | unchanged |
| `PROBE_ADAPTER_ARG_STOP` | `2u` | unchanged |
| `PROBE_ADAPTER_ARG_RESET` | `3u` | unchanged |
| `PROBE_ADAPTER_ARG_ANY` | `0xFFFFFFFFu` | **NEW** — declared as documentation alias only |

**`PROBE_ADAPTER_ARG_ANY` semantics:** This is a header-level
readability aid for the wildcard mapping row. The bridge never
compares against it; wildcard semantics come from
`row.match_arg0 == false` (Phase 12F §5.1). Choosing the all-bits-set
sentinel value is for inspectability when reading source / logs only.

---

## F. Transition Table Plan

Phase 12J ships the additive rows below. Final transition table is
**10 rows**.

| Row | From | Event | To | Source | Notes |
|---|---|---|---|---|---|
| 0 | `STATE_IDLE` | `EVT_CONFIGURED` | `STATE_READY` | Phase 12I | unchanged |
| 1 | `STATE_READY` | `EVT_START` | `STATE_ACTIVE` | Phase 12I | unchanged |
| 2 | `STATE_ACTIVE` | `EVT_STOP` | `STATE_READY` | Phase 12I | unchanged |
| 3 | `STATE_READY` | `EVT_RESET` | `STATE_IDLE` | Phase 12I | unchanged |
| 4 | `STATE_ACTIVE` | `EVT_RESET` | `STATE_IDLE` | Phase 12I | unchanged |
| 5 | `STATE_IDLE` | `EVT_RESET` | `STATE_IDLE` | **Phase 12J** | self-loop; admitted |
| 6 | `STATE_IDLE` | `EVT_FAULT` | `STATE_FAULT` | **Phase 12J** | FAULT entry from IDLE |
| 7 | `STATE_READY` | `EVT_FAULT` | `STATE_FAULT` | **Phase 12J** | FAULT entry from READY |
| 8 | `STATE_ACTIVE` | `EVT_FAULT` | `STATE_FAULT` | **Phase 12J** | FAULT entry from ACTIVE |
| 9 | `STATE_FAULT` | `EVT_RESET` | `STATE_IDLE` | **Phase 12J** | FAULT exit |

All new rows: `guard = NULL`, `action = NULL`.

`transition_count` bumps `5u → 10u`.

### F.1 Sticky FAULT for non-RESET events

No row matches `(STATE_FAULT, EVT_CONFIGURED)`, `(STATE_FAULT,
EVT_START)`, or `(STATE_FAULT, EVT_STOP)`. The FSM scan returns
no-transition; bridge return is `OK`; FSM `no_transition_count++`.
This yields **implicit sticky FAULT** without dedicated rows. RESET
is the only event that exits FAULT.

### F.2 Compatibility check with Phase 12I tests

The Phase 12I tests TC01-TC15 never issue RESET from IDLE and never
issue FAULT. Adding rows 5-9 is therefore behavior-neutral for the
Phase 12I assertions (proof carried in spec §11).

---

## G. Mapping Table Plan

Phase 12J ships **1 new mapping row**. Final mapping table is **5
rows**.

| Row | adapter_type | adapter_arg0 | match_arg0 | fw_event_id | Source |
|---|---|---|---|---|---|
| 0 | `TYPE_CONFIG` | `ARG_NONE` | `true` | `EVT_CONFIGURED` | Phase 12I |
| 1 | `TYPE_COMMAND` | `ARG_START` | `true` | `EVT_START` | Phase 12I |
| 2 | `TYPE_COMMAND` | `ARG_STOP` | `true` | `EVT_STOP` | Phase 12I |
| 3 | `TYPE_COMMAND` | `ARG_RESET` | `true` | `EVT_RESET` | Phase 12I |
| 4 | `TYPE_FAULT` | `ARG_ANY` | `false` | `EVT_FAULT` | **Phase 12J** |

`row_count` bumps `4u → 5u`.

### G.1 Wildcard placement / precedence

Row 4 placed last. FIFO first-match is safe because rows 0-3 use
distinct `adapter_type` values from row 4 (`TYPE_FAULT == 3u`); no
specific row competes for a `TYPE_FAULT` adapter event.

### G.2 Bridge ABI impact: NONE

The wildcard is implemented entirely by the existing Phase 12F
bridge feature `match_arg0 == false`. The bridge `.c` / `.h` files
are **zero-diff** at Phase 12J. The bridge contract test
(`robotos_fw_event_bridge_contract`) already covers wildcard
semantics and remains green at Phase 12J. The bridge ABI
memory-layout lock (open gate from Phase 12G §11 #9) is **not
triggered** by Phase 12J.

### G.3 `unmapped_count` semantics

- `(adapter_type != TYPE_FAULT)` AND no matching specific row → no
  match → `unmapped_count++`, FSM not called. (Phase 12I behavior
  preserved.)
- `(adapter_type == TYPE_FAULT, any arg0)` → row 4 matches →
  `mapped_count++`, FSM dispatched.

---

## H. State Definitions Plan

| Index | state_id | on_entry | on_exit | Source |
|---|---|---|---|---|
| 0 | `STATE_IDLE` | NULL | NULL | Phase 12I |
| 1 | `STATE_READY` | NULL | NULL | Phase 12I |
| 2 | `STATE_ACTIVE` | NULL | NULL | Phase 12I |
| 3 | `STATE_FAULT` | NULL | NULL | **Phase 12J** |

`state_count` bumps `3u → 4u`.

All callbacks remain `NULL`. Action-driven behavior and on_entry /
on_exit fault-cause latching are deferred to a future app-behavior
phase.

---

## I. Host Test Contract (Phase 12J)

Test file: `RobotOS_v1.0/tests/host/test_app_probe_translator_mapping.c`

- **TC01..TC15** retained verbatim from Phase 12I. All assertions
  remain green (see spec §11).
- **TC16..TC24** appended at Phase 12J (~9 cases, ~30-40 in-binary
  assertions).

| # | Test name | Key assertions |
|---|---|---|
| TC16 | `fault_from_idle` | `(TYPE_FAULT, ARG_ANY, NULL)` from IDLE → `STATE_FAULT`; `fsm.transition_count == 1`; `bridge.mapped_count == 1`; `bridge.unmapped_count == 0`; `bridge.last_fw_event_id == EVT_FAULT`. |
| TC17 | `fault_from_ready` | CONFIG then `(TYPE_FAULT, 0xDEADBEEFu, NULL)` from READY → `STATE_FAULT`; `bridge.last_adapter_arg0 == 0xDEADBEEFu`. |
| TC18 | `fault_from_active` | CONFIG + START then `(TYPE_FAULT, 0u, NULL)` from ACTIVE → `STATE_FAULT`. |
| TC19 | `fault_wildcard_arg0_variants` | After reset, three FAULT dispatches with arg0 = `0u`, `0xFFFFFFFFu`, `42u`; each maps; `bridge.mapped_count += 3`; `bridge.unmapped_count == 0`. |
| TC20 | `reset_from_fault_to_idle` | From any path to FAULT, dispatch `(TYPE_COMMAND, ARG_RESET, NULL)`; → `STATE_IDLE`. |
| TC21 | `fault_sticky_for_normal_events` | In FAULT, dispatch CONFIG, START, STOP in turn. State stays `STATE_FAULT`; `bridge.mapped_count += 3` (rows 0/1/2 match); `fsm.no_transition_count += 3` (no FSM row). |
| TC22 | `idle_reset_self_loop_admitted` | After reset, `(TYPE_COMMAND, ARG_RESET, NULL)` from IDLE → `STATE_IDLE`; `fsm.transition_count == 1` (committed via row 5). |
| TC23 | `full_path_with_fault` | Sequence CONFIG → START → FAULT → RESET; state trail IDLE→READY→ACTIVE→FAULT→IDLE; `fsm.transition_count == 4`; `bridge.mapped_count == 4`; `bridge.unmapped_count == 0`. |
| TC24 | `snapshot_counts_with_fault_and_unmapped` | After reset, dispatch `(0xDEADBEEFu, 0xCAFEBABEu, NULL)` (no row matches; not TYPE_FAULT) then `(TYPE_FAULT, 0u, NULL)`. `bridge.event_count == 2`; `bridge.mapped_count == 1`; `bridge.unmapped_count == 1`; state `STATE_FAULT`. |

**Grep gates TC12-14 and regression TC15** are inherited from Phase
12I unchanged. They still pass at Phase 12J because:
- TC12-13-14: Phase 12J adds no `devkit_*`, UART byte, or
  Zephyr/legacy include.
- TC15: Phase 12J adds no new ctest target; full regression count
  remains 23/23.

**Estimated final assertion total in the binary at Phase 12J close:**
~100-110.

---

## J. Build Strategy

### J.1 CMake change: NONE

`tests/host/CMakeLists.txt` was modified at Phase 12I to wire the
`probe_translator_mapping_contract_test` target with
`probe_translator.c` as a source. Phase 12J extends the same `.c`
file (and the same test `.c` file). No CMake change.

### J.2 Build environment

- WSL Ubuntu 24.04 / gcc 13.3.0 / cmake 3.28.3 (same as Phase 12I).
- Build directory: `build-phase12j-host/`.
- Log naming: `tests/host/logs/phase_12J_host_<YYYY-MM-DD>.log`.

### J.3 Build commands

```bash
rm -rf build-phase12j-host
cmake -S RobotOS_v1.0/tests/host -B build-phase12j-host
cmake --build build-phase12j-host
ctest --test-dir build-phase12j-host --output-on-failure -V
cmake --build build-phase12j-host --target save_test_log
cp RobotOS_v1.0/tests/host/logs/host_<date>.log \
   RobotOS_v1.0/tests/host/logs/phase_12J_host_<date>.log
```

---

## K. Phase 12J Validation and Exit Criteria (14 gates)

Phase 12J may close `CLOSED_WITH_HOST_TEST_EVIDENCE` only when all
of the following pass.

| # | Gate | Evidence |
|---|---|---|
| 1 | CMake configure succeeds | no error |
| 2 | Build succeeds | no error, no warning on new code |
| 3 | `probe_translator_mapping_contract` PASS | TC01..TC24; in-binary total ~100-110 |
| 4 | `robotos_fw_fsm_contract` still PASS | unchanged |
| 5 | `robotos_fw_event_bridge_contract` still PASS | unchanged |
| 6 | Full host regression | 23/23 PASS |
| 7 | Host log saved | `phase_12J_host_<date>.log` non-empty |
| 8 | Grep gate: no `devkit_app_state` | clean |
| 9 | Grep gate: no `devkit_*` symbol | clean |
| 10 | Grep gate: no UART command byte | clean |
| 11 | Grep gate: no Zephyr / legacy include | clean |
| 12 | Command set unchanged | `devkit/src/` zero-diff |
| 13 | `devkit_app_state` zero-diff | confirmed |
| 14 | No hardware run | confirmed |

Plus a Phase-12J-specific structural check:
`probe_translator_init` sets `transition_count = 10u`,
`state_count = 4u`, `row_count = 5u`. Verified implicitly via TC23
(full_path_with_fault) and TC22 (idle_reset_self_loop) — the FAULT
path and self-loop path must be reachable.

---

## L. Relationship to `devkit_app_state`

Unchanged from Phase 12I:

- `devkit_app_state` remains authoritative for the devkit runtime.
- `app/probe_translator/` must not include `devkit_app_state.h`,
  must not call any `devkit_*` function, must not read or write
  `devkit_app_state` snapshots.
- `PROBE_TRANSLATOR_STATE_FAULT` is application-local; name overlap
  with any `DEVKIT_*` symbol is coincidental at the human-readable
  level only.
- No `?` UART response change. No `a/s/r/?/x/v/L/d/T` semantic
  change.
- Scope-guard #11 remains active.

---

## M. Relationship to Command Set

Unchanged from Phase 12I:

- `a / s / r / ? / x / v / L / d / T` remain devkit / probe surface.
- `probe_translator/` defines no UART command at Phase 12J.
- Framework bridge has no UART surface.
- All 12 UART TX scope-guard constraints from Phase 9EZ §H preserved.

---

## N. Relationship to Legacy Architecture B

Unchanged from Phase 12I:

- `src/` and `include/robotos/` frozen at
  `LEGACY_SCAFFOLD_MARKED_FROZEN_DOCS_ONLY` (Phase 12D-pre).
- `app/probe_translator/` uses Architecture A contracts only.
- No `ro_*` include.

---

## O. Non-goals (Phase 12J-pre)

| Non-goal | Status |
|---|---|
| Modify `.c` files | NO |
| Modify `.h` files | NO |
| Modify test source | NO |
| Modify CMake | NO |
| Framework / core / platform / devkit/src/ change | NO |
| Devkit runtime change | NO |
| `devkit_app_state` change | NO |
| UART command added or changed | NO |
| Command semantic change | NO |
| Hardware run | NO |
| Scheduler 7A/7B reopened | NO — `DEFER` preserved |
| F407 / custom board reopened | NO — `HOLD/DEFER` preserved |
| ACTIVE disarm widening started | NO — `USER_DECISION_REQUIRED_ACTIVE_DISARM` preserved |
| POST_FLASH_AUTOSTART change | NO — `OPEN/MITIGATED_BY_WORKFLOW` preserved |
| Legacy Architecture B modification | NO |
| Evidence logs touched | NO |
| Bridge ABI memory-layout lock change | NO (Phase 12J uses existing Phase 12F wildcard feature) |
| `app/` second product introduced | NO |
| Examples directory introduced | NO |
| Phase 12J started | NO — `NOT_STARTED` |

---

## P. Decision Result

**`PHASE_12J_PRE_PROBE_TRANSLATOR_FAULT_BLOCK_PLAN_CLOSED`**

The implementation contract for Phase 12J is complete. Phase 12J may
open only on **explicit user authorization** with the scope in §Q.

---

## Q. Next Gate Recommendation

**Phase 12J — Probe Translator FAULT Block Extension**

| Field | Value |
|---|---|
| Title | Phase 12J — Probe Translator FAULT Block Extension |
| Classification | Host-only additive extension; no devkit / Zephyr / hardware |
| In-scope | Additive diff to `app/probe_translator/probe_translator.{h,c}` (FAULT constants, 5 new transition rows, 1 new state def, 1 new mapping row, count bumps); additive TC16-TC24 in existing host test; updated README; new host log `phase_12J_host_<date>.log`; Phase 12J closeout; doc-sync updates. |
| Non-goals | No CMake change. No Framework / core / platform / devkit-runtime / `devkit_app_state` change. No UART command. No Zephyr / `prj.conf` / DTS / overlay. No hardware run. No legacy Architecture B touch. No second `app/<product>/`. |
| Exit criteria | All 14 §K gates pass. Host test PASS (TC01-TC24, ~100-110 assertions). Full regression 23/23. Log committed. |
| Authorization | Must be opened only on **explicit user approval**. |

**Alternative — HOLD.** If the user does not want to implement the
FAULT block yet, Phase 12J-pre sits at planning depth indefinitely.
No regression risk; Phase 12I baseline remains.

---

## R. Cross-references

- Phase 12I implementation closeout:
  [`PHASE_12I_PROBE_TRANSLATOR_HOST_PROTOTYPE.md`](PHASE_12I_PROBE_TRANSLATOR_HOST_PROTOTYPE.md)
- Phase 12I-pre planning closeout:
  [`PHASE_12I_PRE_PROBE_TRANSLATOR_HOST_PROTOTYPE_PLAN.md`](PHASE_12I_PRE_PROBE_TRANSLATOR_HOST_PROTOTYPE_PLAN.md)
- Phase 12I long-lived implementation-plan spec:
  [`../03_SPECS/PROBE_TRANSLATOR_HOST_PROTOTYPE_PLAN.md`](../03_SPECS/PROBE_TRANSLATOR_HOST_PROTOTYPE_PLAN.md)
- Phase 12H skeleton spec:
  [`../03_SPECS/PROBE_TRANSLATOR_APP_SKELETON_DRAFT.md`](../03_SPECS/PROBE_TRANSLATOR_APP_SKELETON_DRAFT.md)
- Framework FSM spec:
  [`../03_SPECS/FRAMEWORK_FSM_API_DRAFT.md`](../03_SPECS/FRAMEWORK_FSM_API_DRAFT.md)
- Framework Bridge spec:
  [`../03_SPECS/FRAMEWORK_APPLICATION_BRIDGE_DRAFT.md`](../03_SPECS/FRAMEWORK_APPLICATION_BRIDGE_DRAFT.md)
- Application boundary spec:
  [`../03_SPECS/FRAMEWORK_APPLICATION_BOUNDARY_DRAFT.md`](../03_SPECS/FRAMEWORK_APPLICATION_BOUNDARY_DRAFT.md)
- New long-lived FAULT-block plan spec:
  [`../03_SPECS/PROBE_TRANSLATOR_FAULT_BLOCK_PLAN.md`](../03_SPECS/PROBE_TRANSLATOR_FAULT_BLOCK_PLAN.md)
- Live state: [`../../../../CURRENT_STATE.md`](../../../../CURRENT_STATE.md)
