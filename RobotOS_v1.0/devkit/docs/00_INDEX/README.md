# Devkit Docs Index

Navigation hub for `RobotOS_v1.0/devkit/docs/`. Docs are grouped by
artifact type to keep navigation tractable as the project grows. The
docs themselves were not rewritten when grouped; only relative links
were updated to point at the new paths.

Live state lives at the repo root in
[`CURRENT_STATE.md`](../../../../CURRENT_STATE.md).

---

## 01_PROGRESS — running phase logs

Authoritative phase history. Progress files are **grouped by 10-phase
windows** starting at Phase 11, not one file per phase. Phase 1–10
keep their historical naming. Each progress file is a narrative index
(status + short summary + anchors); full-form evidence lives in the
matching closeout docs under `02_PHASE_CLOSEOUTS/`.

See [`../01_PROGRESS/README.md`](../01_PROGRESS/README.md) for the
directory policy (phase-window convention, distinction between
progress streams and closeout docs, editing rules).

| Phase range | File | Status |
|---|---|---|
| Phase 1 through Phase 9E / 9E-Z | [`DEVKIT_PROGRESS.md`](../01_PROGRESS/DEVKIT_PROGRESS.md) | Historical master — **not rewritten** when later phases land. |
| Phase 10 (10A, 10B-*, 10C, ...) | [`DEVKIT_PROGRESS_PHASE_10.md`](../01_PROGRESS/DEVKIT_PROGRESS_PHASE_10.md) | Running log; Phase 10C is latest closed entry. |
| Phase 11 through Phase 20 | [`DEVKIT_PROGRESS_PHASE_11_20.md`](../01_PROGRESS/DEVKIT_PROGRESS_PHASE_11_20.md) | Running log; Phase 12I-pre is latest closed entry (`CLOSED_DOCS_ONLY`; `PHASE_12I_PRE_PROBE_TRANSLATOR_HOST_PROTOTYPE_PLAN_CLOSED`); Phase 12I-pre resolves all open numeric values (`IDLE=1u/READY=2u/ACTIVE=3u`; `CONFIGURED=1u/START=2u/STOP=3u/RESET=4u`; adapter types 1/2; adapter args 0–3), defers FAULT block + transition row 5, locks embed-by-value ownership + combined snapshot + all-NULL-action transition rows + Option A CMake strategy + 15-case host test plan (TC01–TC15) with 14 Phase 12I validation gates; `app/probe_translator/` directory **not created**; Phase 12I pending explicit user authorization. Phase 12H is prior closed entry (`CLOSED_DOCS_ONLY`; `PHASE_12H_PROBE_TRANSLATOR_SKELETON_PLANNED_DOCS_ONLY`); Phase 11A/11B/11C/11D/11E/11Z/12A/12B/12C/12D-pre/12D/12E-pre/12E/12F-pre/12F/12G-pre/12G/12H-pre/12H closed; Phase 11A-11E sensor track complete; Phase 12A-12C Framework planning complete; Phase 12D-pre boundary explicit; Phase 12D header stub in place; Phase 12E FSM implementation host-test-validated (93/93 assertions); Phase 12F bridge host prototype host-test-validated (103/103 assertions, full regression 22/22); bridge §5 names + signatures `LOCKED-AT-12F`; Phase 12G-pre selected SEPARATE APPLICATION MODE; Phase 12G selected application directory path `RobotOS_v1.0/app/<product>/` (`PHASE_12G_RECOMMEND_APP_PRODUCT_PATH`) with new `FRAMEWORK_APPLICATION_BOUNDARY_DRAFT.md` spec; Phase 12H-pre selected `probe_translator` as the first `<product>` placeholder (`PHASE_12H_PRE_RECOMMEND_PROBE_TRANSLATOR_APP`) with new `FIRST_APPLICATION_CANDIDATE_DRAFT.md` spec; Phase 12H locked the `app/probe_translator/` skeleton (file set / API names / state-event-mapping vocab / host test plan / build strategy preference) with new `PROBE_TRANSLATOR_APP_SKELETON_DRAFT.md` spec; `app/` directory and `app/probe_translator/` directory **not created**; Phase 12I-pre (Probe Translator Host Prototype Implementation Plan, docs-only, preferred) or Phase 12I (Probe Translator Host Prototype, implementation) pending explicit user authorization. |
| Phase 21+ | future `DEVKIT_PROGRESS_PHASE_21_30.md`, etc. | Create when the window opens. |

**Per-phase closeout / checkpoint docs remain a separate artifact
class** under `../02_PHASE_CLOSEOUTS/`. A phase with hardware
evidence, implementation closeout, checkpoint, direction guard,
architecture / design decision, important validation record, or
release / freeze implication may still publish its own
`PHASE_*_CLOSE.md` / `PHASE_*_CHECKPOINT.md` regardless of which
progress-window file holds its narrative entry.

## 02_PHASE_CLOSEOUTS — phase evidence and checkpoint docs

One file per closed phase or docs-only checkpoint. Each closeout
captures source delta, scope-guard restatement, hardware evidence (if
applicable), and verdict. **These are evidence artifacts and are not
rewritten after a phase closes.**

- [`PHASE_9EZ_CHECKPOINT.md`](../02_PHASE_CLOSEOUTS/PHASE_9EZ_CHECKPOINT.md) -- Phase 9E-Z command-loop direction guard (audit checkpoint).
- [`PHASE_9Z_CHECKPOINT.md`](../02_PHASE_CLOSEOUTS/PHASE_9Z_CHECKPOINT.md) -- Phase 9Z workload-branch checkpoint review.
- [`PHASE_9G_CLOSE.md`](../02_PHASE_CLOSEOUTS/PHASE_9G_CLOSE.md) -- bounded UART burst characterization.
- [`PHASE_10A_CLOSE.md`](../02_PHASE_CLOSEOUTS/PHASE_10A_CLOSE.md) -- product command set planning (docs-only).
- [`PHASE_10B_V_CLOSE.md`](../02_PHASE_CLOSEOUTS/PHASE_10B_V_CLOSE.md) -- `v` build/version query command.
- [`PHASE_10B_L_CLOSE.md`](../02_PHASE_CLOSEOUTS/PHASE_10B_L_CLOSE.md) -- `L` LED physical-effect command (+ operator-visual confirmation).
- [`PHASE_10B_D_CLOSE.md`](../02_PHASE_CLOSEOUTS/PHASE_10B_D_CLOSE.md) -- `d` explicit disarm command.
- [`PHASE_10C_COMMAND_SET_CHECKPOINT.md`](../02_PHASE_CLOSEOUTS/PHASE_10C_COMMAND_SET_CHECKPOINT.md) -- post-10B-d command-set checkpoint (docs-only).
- [`PHASE_11A_ADAPTER_BOUNDARY_SENSOR_SURFACE.md`](../02_PHASE_CLOSEOUTS/PHASE_11A_ADAPTER_BOUNDARY_SENSOR_SURFACE.md) -- Adapter API surface inventory + sensor surface classification (docs-only).
- [`PHASE_11B_DEVICE_DRIVER_FEASIBILITY.md`](../02_PHASE_CLOSEOUTS/PHASE_11B_DEVICE_DRIVER_FEASIBILITY.md) -- Device / driver feasibility gate (docs-only audit; `FEASIBILITY_CONFIRMED_ONBOARD_MEMS`).
- [`PHASE_11C_ACCEL_PROBE_SPEC.md`](../02_PHASE_CLOSEOUTS/PHASE_11C_ACCEL_PROBE_SPEC.md) -- On-board MEMS accelerometer probe spec (docs-only; `PHASE_11C_SPEC_FROZEN_ACCEL_LIS2DH_REV_D`).
- [`PHASE_11D_ACCEL_PROBE_IMPLEMENTATION.md`](../02_PHASE_CLOSEOUTS/PHASE_11D_ACCEL_PROBE_IMPLEMENTATION.md) -- On-board MEMS accelerometer probe implementation (firmware; build-validated).
- [`PHASE_11E_ACCEL_PROBE_EVIDENCE.md`](../02_PHASE_CLOSEOUTS/PHASE_11E_ACCEL_PROBE_EVIDENCE.md) -- On-board MEMS accelerometer probe evidence closeout (`CLOSED_WITH_HARDWARE_EVIDENCE`; `OPERATOR_PHYSICAL_SANITY_CONFIRMED`).
- [`PHASE_11Z_COMMAND_SET_CHECKPOINT.md`](../02_PHASE_CLOSEOUTS/PHASE_11Z_COMMAND_SET_CHECKPOINT.md) -- Post-Phase-11E command-set checkpoint (`CLOSED_DOCS_ONLY`; validated 9-command set `a/s/r/?/x/v/L/d/T`; Phase 11A–11E sensor-probe track shelved as complete).
- [`PHASE_12A_FRAMEWORK_API_SURFACE_PLANNING.md`](../02_PHASE_CLOSEOUTS/PHASE_12A_FRAMEWORK_API_SURFACE_PLANNING.md) -- Robot Framework API surface planning gate (`CLOSED_DOCS_ONLY`; layer boundary established; 9 candidate domains evaluated; FSM abstraction recommended as first Framework slice).
- [`PHASE_12B_FRAMEWORK_FSM_API_DRAFT.md`](../02_PHASE_CLOSEOUTS/PHASE_12B_FRAMEWORK_FSM_API_DRAFT.md) -- Robot Framework FSM API draft (`CLOSED_DOCS_ONLY`; `PHASE_12B_FSM_API_DRAFTED_DOCS_ONLY`; flat table-driven FSM model; draft `robotos_fw_fsm_*` API surface; event bridge and status model decisions open for Phase 12C).
- [`PHASE_12C_FSM_EVENT_BRIDGE_STATUS_MODEL.md`](../02_PHASE_CLOSEOUTS/PHASE_12C_FSM_EVENT_BRIDGE_STATUS_MODEL.md) -- Framework FSM event bridge + status model confirmation (`CLOSED_DOCS_ONLY`; `PHASE_12C_EVENT_BRIDGE_STATUS_CONFIRMED_DOCS_ONLY`; all four Phase 12B open decisions confirmed; evaluation order corrected; Phase 12D pending explicit user authorization).
- [`PHASE_12D_PRE_LEGACY_FRAMEWORK_SCAFFOLD_DISPOSITION.md`](../02_PHASE_CLOSEOUTS/PHASE_12D_PRE_LEGACY_FRAMEWORK_SCAFFOLD_DISPOSITION.md) -- Legacy Framework scaffold disposition (`CLOSED_DOCS_ONLY`; `LEGACY_SCAFFOLD_MARKED_FROZEN_DOCS_ONLY`; classifies `RobotOS_v1.0/src/` + `include/robotos/` as legacy/frozen/non-authoritative; future active Framework path = `RobotOS_v1.0/framework/`; Phase 12D unblocked at the boundary level but still requires explicit user authorization).
- [`PHASE_12D_FSM_HEADER_STUB.md`](../02_PHASE_CLOSEOUTS/PHASE_12D_FSM_HEADER_STUB.md) -- Framework FSM header stub (`CLOSED_HEADER_STUB_ONLY`; `PHASE_12D_FSM_HEADER_STUB_CREATED`; creates active Framework path `RobotOS_v1.0/framework/` with `README.md` + `robotos_fw_fsm.h`; declarations only — no `.c` body, no CMake, no devkit integration; §4 of `FRAMEWORK_FSM_API_DRAFT.md` now `LOCKED-AT-12D`; Phase 12E implementation pending explicit user authorization + concrete consumer/test).
- [`PHASE_12E_PRE_FSM_CONSUMER_TEST_PLAN.md`](../02_PHASE_CLOSEOUTS/PHASE_12E_PRE_FSM_CONSUMER_TEST_PLAN.md) -- Framework FSM consumer / test plan (`CLOSED_DOCS_ONLY`; `PHASE_12E_RECOMMEND_HOST_UNIT_TEST_CONSUMER`; recommended Phase 12E path = implement `framework/robotos_fw_fsm.c` + extend existing `tests/host/CMakeLists.txt` with one new target; no devkit integration; no command-set change; validates 21 runtime cases + 4 review cases mapping to Phase 12B/12C decisions; Phase 12E remains `NOT_STARTED`).
- [`PHASE_12E_FSM_HOST_TEST_IMPLEMENTATION.md`](../02_PHASE_CLOSEOUTS/PHASE_12E_FSM_HOST_TEST_IMPLEMENTATION.md) -- Framework FSM host-test implementation (`CLOSED_WITH_HOST_TEST_EVIDENCE`; `PHASE_12E_FSM_HOST_TEST_IMPLEMENTATION_CLOSED`; first Framework `.c` body at `framework/robotos_fw_fsm.c`; host test `tests/host/test_robotos_fw_fsm.c` passes 93/93 assertions across 20 cases under WSL Ubuntu / gcc 13.3.0; full host suite regression 21/21; tracked log at `tests/host/logs/phase_12E_host_2026-05-12.log`; header zero-diff (LOCKED-AT-12D held); no devkit integration; no hardware evidence; Phase 12F NOT_STARTED).
- [`PHASE_12F_PRE_APPLICATION_BRIDGE_PLANNING.md`](../02_PHASE_CLOSEOUTS/PHASE_12F_PRE_APPLICATION_BRIDGE_PLANNING.md) -- Framework Application Bridge planning (`CLOSED_DOCS_ONLY`; `PHASE_12F_PRE_RECOMMEND_HOST_BRIDGE_PROTOTYPE`; recommended Phase 12F path = host-only bridge prototype at `framework/robotos_fw_event_bridge.{h,c}` + new host test target; 12 runtime cases + 5 review items mapped; bridge contract frozen at planning depth in long-lived spec; no devkit integration; no `devkit_app_state` change; no command-set change; Phase 12F remains `NOT_STARTED` at the close of this planning gate).
- [`PHASE_12I_PRE_PROBE_TRANSLATOR_HOST_PROTOTYPE_PLAN.md`](../02_PHASE_CLOSEOUTS/PHASE_12I_PRE_PROBE_TRANSLATOR_HOST_PROTOTYPE_PLAN.md) -- Probe Translator Host Prototype Implementation Plan (`CLOSED_DOCS_ONLY`; `PHASE_12I_PRE_PROBE_TRANSLATOR_HOST_PROTOTYPE_PLAN_CLOSED`; resolves all Phase 12H open decisions: numeric IDs locked, FAULT block deferred, row 5 deferred, embed-by-value struct, combined snapshot, `PROBE_ADAPTER_ARG_ANY` omitted, 15-case host test plan, Option A CMake block, 14 Phase 12I exit gates; `app/probe_translator/` NOT created; Phase 12I NOT_STARTED).
- [`PHASE_12H_PROBE_TRANSLATOR_SKELETON_PLANNING.md`](../02_PHASE_CLOSEOUTS/PHASE_12H_PROBE_TRANSLATOR_SKELETON_PLANNING.md) -- Probe Translator App Skeleton Planning (`CLOSED_DOCS_ONLY`; `PHASE_12H_PROBE_TRANSLATOR_SKELETON_PLANNED_DOCS_ONLY`; Variant 1 docs-only skeleton lock; future file set locked = `app/probe_translator/probe_translator.{c,h}` + `README.md` + `tests/host/test_app_probe_translator_mapping.c`; public API names locked = `probe_translator_init / _dispatch_adapter_event / _reset / _get_snapshot`; state vocab locked = `PROBE_TRANSLATOR_STATE_IDLE/READY/ACTIVE/FAULT?`; event vocab locked = `PROBE_TRANSLATOR_EVT_CONFIGURED/START/STOP/RESET/FAULT?`; adapter-key vocab locked = `PROBE_ADAPTER_TYPE_CONFIG/COMMAND/FAULT?` + `PROBE_ADAPTER_ARG_NONE/START/STOP/RESET/ANY?`; transition table rows 0-4 required + 5-9 optional; mapping table rows 0-3 required + 4 optional (wildcard for FAULT); 16-case host test plan (TC01-TC16 incl. three grep gates); preferred build = Option A additive entry in existing `tests/host/CMakeLists.txt`; new long-lived `PROBE_TRANSLATOR_APP_SKELETON_DRAFT.md` spec frozen at planning depth; `app/probe_translator/` directory **NOT created** at Phase 12H; numeric values for `PROBE_TRANSLATOR_*` constants + ship-or-defer of FAULT block + transition row 5 open for Phase 12I-pre; `devkit_app_state` remains authoritative; scope-guard #11 preserved; command set `a/s/r/?/x/v/L/d/T` unchanged; no application implementation; no Framework / devkit / CMake / test / log change; Phase 12I-pre (docs-only implementation plan, preferred next gate) and Phase 12I (host prototype implementation) both remain `NOT_STARTED`).
- [`PHASE_12H_PRE_FIRST_APPLICATION_CANDIDATE_SELECTION.md`](../02_PHASE_CLOSEOUTS/PHASE_12H_PRE_FIRST_APPLICATION_CANDIDATE_SELECTION.md) -- First Application Candidate / Product Harness Selection (`CLOSED_DOCS_ONLY`; `PHASE_12H_PRE_RECOMMEND_PROBE_TRANSLATOR_APP`; five candidate first-app shapes evaluated -- `app/probe_translator/` / `app/demo/` / `app/devkit_shadow/` / `app/<real_product>/` / HOLD; `app/probe_translator/` recommended as first application harness; directory **NOT created** at Phase 12H-pre; new long-lived `FIRST_APPLICATION_CANDIDATE_DRAFT.md` spec frozen at planning depth; minimal state vocabulary `APP_IDLE/READY/ACTIVE/FAULT?` and minimal event vocabulary `APP_EVT_CONFIGURED/START/STOP/FAULT?/RESET` defined planning-level only; 5-row mapping table sketched planning-level only; host-first validation plan recorded; preferred build = Option A additive entry in existing `tests/host/CMakeLists.txt`; `devkit_app_state` remains authoritative; scope-guard #11 preserved; command set `a/s/r/?/x/v/L/d/T` unchanged; no application implementation; no Framework / devkit / CMake / test / log change; Phase 12H (Probe Translator App Skeleton Planning Variant 1 docs-only OR Probe Translator Host Prototype Variant 2) remains `NOT_STARTED`).
- [`PHASE_12G_SEPARATE_APPLICATION_BOUNDARY_PLANNING.md`](../02_PHASE_CLOSEOUTS/PHASE_12G_SEPARATE_APPLICATION_BOUNDARY_PLANNING.md) -- Separate Application Mode / Application Boundary Planning (`CLOSED_DOCS_ONLY`; `PHASE_12G_RECOMMEND_APP_PRODUCT_PATH`; five candidate application directory shapes evaluated -- `app/<product>/` / `application/<product>/` / `devkit/app/` / `framework/app/` / `examples/<scenario>/`; `RobotOS_v1.0/app/<product>/` recommended as future active application path; directory **NOT created** at Phase 12G; new long-lived `FRAMEWORK_APPLICATION_BOUNDARY_DRAFT.md` spec frozen at planning depth; application layer responsibilities + non-responsibilities + event-mapping policy + build separation strategy + staged validation strategy defined; `devkit_app_state` remains authoritative; scope-guard #11 preserved; command set `a/s/r/?/x/v/L/d/T` unchanged; no application implementation; no Framework / devkit / CMake / test / log change; Phase 12H-pre (First Application Candidate / Product Harness Selection) remains `NOT_STARTED`).
- [`PHASE_12G_PRE_DEVKIT_INTEGRATION_MODE_DECISION.md`](../02_PHASE_CLOSEOUTS/PHASE_12G_PRE_DEVKIT_INTEGRATION_MODE_DECISION.md) -- Devkit Integration Mode decision (`CLOSED_DOCS_ONLY`; `PHASE_12G_PRE_RECOMMEND_SEPARATE_APPLICATION_MODE_PLANNING`; five candidate modes evaluated -- HOLD / SHADOW / REPLACEMENT / SEPARATE APPLICATION / HOST-ONLY EXTENSION; SEPARATE APPLICATION MODE recommended; new long-lived `FRAMEWORK_DEVKIT_INTEGRATION_MODE_DRAFT.md` spec frozen at planning depth; `devkit_app_state` remains authoritative; scope-guard #11 preserved; command set `a/s/r/?/x/v/L/d/T` unchanged; no integration implementation; no Framework/devkit/CMake/test/log change; Phase 12G (Separate Application Mode Planning) remains `NOT_STARTED`).
- [`PHASE_12F_APPLICATION_BRIDGE_HOST_PROTOTYPE.md`](../02_PHASE_CLOSEOUTS/PHASE_12F_APPLICATION_BRIDGE_HOST_PROTOTYPE.md) -- Framework Application Bridge host prototype (`CLOSED_WITH_HOST_TEST_EVIDENCE`; `PHASE_12F_APPLICATION_BRIDGE_HOST_PROTOTYPE_CLOSED`; first Framework bridge `.c` body at `framework/robotos_fw_event_bridge.c` + new header `framework/robotos_fw_event_bridge.h`; host test `tests/host/test_robotos_fw_event_bridge.c` passes 103/103 assertions across 17 cases under WSL Ubuntu / gcc 13.3.0; full host suite regression 22/22; tracked log at `tests/host/logs/phase_12F_host_2026-05-13.log`; FSM header + body zero-diff; bridge spec §5 names + signatures `LOCKED-AT-12F`; no devkit integration; no `devkit_app_state` change; no UART command added; no hardware run; no legacy Architecture B change; Phase 12G / 12G-pre `NOT_STARTED`).

Future `PHASE_*_CLOSE.md` and `PHASE_*_CHECKPOINT.md` docs belong here.

## 03_SPECS — long-lived design / reference artifacts

Specs, reference docs, and runbooks that are long-lived design
artifacts (not snapshots of a single phase). They get updated in place
as the underlying contracts evolve.

- [`COMMAND_SET_DRAFT.md`](../03_SPECS/COMMAND_SET_DRAFT.md) -- product command vocabulary table (Sections A / B / C).
- [`TELEMETRY_REFERENCE.md`](../03_SPECS/TELEMETRY_REFERENCE.md) -- ROBOTOS_OBS / FAULT / PROD / BTN / UART / APP telemetry line definitions.
- [`WORKLOAD_DEMO_9D.md`](../03_SPECS/WORKLOAD_DEMO_9D.md) -- Phase 9D workload-demo runbook (hardware setup, sequence, expected counters).
- [`FRAMEWORK_FSM_API_DRAFT.md`](../03_SPECS/FRAMEWORK_FSM_API_DRAFT.md) -- Robot Framework flat FSM API spec (`DRAFT / EXPERIMENTAL — IMPLEMENTED_AT_12E (HOST-TEST EVIDENCE)`; Phase 12B origin; §4 names LOCKED-AT-12D; `.c` body implemented at Phase 12E; host-test-validated 93/93 assertions; ABI still not stable).
- [`FRAMEWORK_APPLICATION_BRIDGE_DRAFT.md`](../03_SPECS/FRAMEWORK_APPLICATION_BRIDGE_DRAFT.md) -- Framework Application Bridge spec (`DRAFT / EXPERIMENTAL — IMPLEMENTED_AT_12F (HOST-TEST EVIDENCE)`; Phase 12F-pre origin; §5 names + signatures `LOCKED-AT-12F`; mapping engine from Adapter event keys `(type, arg0)` to Framework event IDs; caller-owned bridge instance, caller-owned FSM, caller-owned mapping table; FIFO first-match, optional `arg0` wildcard; no heap, no UART, no devkit/legacy includes; `.c` body implemented at Phase 12F; host-test-validated 103/103 assertions; ABI memory layout still not stable).
- [`FRAMEWORK_APPLICATION_BOUNDARY_DRAFT.md`](../03_SPECS/FRAMEWORK_APPLICATION_BOUNDARY_DRAFT.md) -- Framework application boundary spec (`DRAFT / NON-FINAL`; Phase 12G origin; `PHASE_12G_RECOMMEND_APP_PRODUCT_PATH`; reserves `RobotOS_v1.0/app/<product>/` as future application path at planning depth; defines application layer responsibilities, event-mapping policy, build separation strategy, staged validation strategy; `app/` directory NOT created; first `<product>` placeholder resolved at Phase 12H-pre as `probe_translator` -- see `FIRST_APPLICATION_CANDIDATE_DRAFT.md`; no implementation exists; `devkit_app_state` and command set unchanged; Architecture B frozen).
- [`FIRST_APPLICATION_CANDIDATE_DRAFT.md`](../03_SPECS/FIRST_APPLICATION_CANDIDATE_DRAFT.md) -- First application candidate spec (`DRAFT / NON-FINAL`; Phase 12H-pre origin; revised at Phase 12H with cross-reference to the skeleton spec; `PHASE_12H_PRE_RECOMMEND_PROBE_TRANSLATOR_APP`; first `<product>` placeholder = `probe_translator`; reserves `RobotOS_v1.0/app/probe_translator/` as future first application harness path at planning depth; host-first, product-neutral, synthetic-event-driven; concept-level state and event vocabulary (concrete `PROBE_TRANSLATOR_*` names now live in `PROBE_TRANSLATOR_APP_SKELETON_DRAFT.md`); host mapping test plan; preferred build = Option A additive entry in existing `tests/host/CMakeLists.txt`; `app/probe_translator/` directory NOT created; no implementation exists; `devkit_app_state` and command set unchanged; Architecture B frozen).
- [`PROBE_TRANSLATOR_HOST_PROTOTYPE_PLAN.md`](../03_SPECS/PROBE_TRANSLATOR_HOST_PROTOTYPE_PLAN.md) -- Probe translator host prototype implementation plan (`DRAFT / NON-FINAL`; Phase 12I-pre origin; execution-ready contract for Phase 12I; all numeric values locked; struct/header/source/CMake/test/validation contracts locked; `app/probe_translator/` NOT created; FAULT block + row 5 deferred; Phase 12I NOT_STARTED).
- [`PROBE_TRANSLATOR_APP_SKELETON_DRAFT.md`](../03_SPECS/PROBE_TRANSLATOR_APP_SKELETON_DRAFT.md) -- Probe translator app skeleton spec (`DRAFT / NON-FINAL`; Phase 12H origin; `PHASE_12H_PROBE_TRANSLATOR_SKELETON_PLANNED_DOCS_ONLY`; future file set locked = `app/probe_translator/probe_translator.{c,h}` + `README.md` + `tests/host/test_app_probe_translator_mapping.c`; public API names locked = `probe_translator_init / _dispatch_adapter_event / _reset / _get_snapshot`; state vocab locked = `PROBE_TRANSLATOR_STATE_IDLE/READY/ACTIVE/FAULT?`; event vocab locked = `PROBE_TRANSLATOR_EVT_CONFIGURED/START/STOP/RESET/FAULT?`; adapter-key vocab locked = `PROBE_ADAPTER_TYPE_CONFIG/COMMAND/FAULT?` + `PROBE_ADAPTER_ARG_NONE/START/STOP/RESET/ANY?`; transition table rows 0-4 required + 5-9 optional (FAULT block + IDLE+RESET self-loop); mapping table rows 0-3 required + 4 optional (wildcard arg0 for FAULT); 16-case host test plan (TC01-TC16 incl. three grep gates against `devkit_app_state.h` / command set / Zephyr-devkit-legacy includes); preferred build = Option A; `app/probe_translator/` directory NOT created; numeric values + FAULT-block ship-or-defer + transition row 5 ship-or-defer + struct-layout-by-value-vs-pointer open for Phase 12I-pre / 12I; no implementation exists; `devkit_app_state` and command set unchanged; Architecture B frozen).
- [`FRAMEWORK_DEVKIT_INTEGRATION_MODE_DRAFT.md`](../03_SPECS/FRAMEWORK_DEVKIT_INTEGRATION_MODE_DRAFT.md) -- Framework devkit integration-mode spec (`DRAFT / NON-FINAL`; Phase 12G-pre origin; five candidate modes -- HOLD / SHADOW / REPLACEMENT / SEPARATE APPLICATION / HOST-ONLY EXTENSION; recommended mode = SEPARATE APPLICATION; `devkit_app_state` remains authoritative; scope-guard #11 preserved; command set `a/s/r/?/x/v/L/d/T` unchanged; no integration implementation exists; Phase 12G (Separate Application Mode Planning) remains `NOT_STARTED`).

## 04_SCRATCH_OR_PROVENANCE — non-canonical scratch

**Not canonical.** Files in this directory exist for traceability or
scratch work during prior sessions. **Do not cite them as
source-of-truth.** Files here are not promoted to canonical without an
explicit phase that promotes them.

See [`../04_SCRATCH_OR_PROVENANCE/README.md`](../04_SCRATCH_OR_PROVENANCE/README.md)
for the directory policy (this is the only tracked file in the
directory).

Examples of files that have lived here historically (may or may not be
present locally; **these files are untracked in git and will not
appear on a fresh clone**):

- `DEVKIT_PROGRESS_TEMP.md` -- working scratch from an earlier reorganization pass; superseded by [`../01_PROGRESS/DEVKIT_PROGRESS.md`](../01_PROGRESS/DEVKIT_PROGRESS.md) + [`../01_PROGRESS/DEVKIT_PROGRESS_PHASE_10.md`](../01_PROGRESS/DEVKIT_PROGRESS_PHASE_10.md).
- `DEVKIT_PROGRESS_before.txt` -- provenance snapshot of `DEVKIT_PROGRESS.md` from before the seed-split; preserved locally so the split can be audited but not maintained.

The scratch files themselves are **untracked** in git by design. They
are physically present on disk only on the machines that produced
them; a fresh clone will see only the policy README in this directory.

---

## Conventions carried forward

- **Phase truth is anchored in `01_PROGRESS/` and `02_PHASE_CLOSEOUTS/`.**
  Any other doc that disagrees should be treated as stale until that
  conflict is resolved.
- **Manual anchors** (`<a id="phase-XY"></a>`) are mandatory in
  `01_PROGRESS/DEVKIT_PROGRESS_PHASE_10.md` so index links survive
  heading edits. See §4 of that file for the rules.
- **Evidence logs** live separately under `../logs/` and are indexed
  in `../logs/INDEX.md`. The closeouts in `02_PHASE_CLOSEOUTS/` cite
  those logs by relative path.
- **No file in `02_PHASE_CLOSEOUTS/` rewrites another's evidence.**
  Each phase closeout stands on its own captured run.

---

## Files NOT in this directory

For completeness, these related artifacts live elsewhere:

- [`../../../CURRENT_STATE.md`](../../../../CURRENT_STATE.md) -- repo-root live state snapshot.
- `../logs/INDEX.md` -- evidence log index.
- `../logs/phase_*_{rtt,host}_*.txt` -- raw RTT captures and host
  transcripts (canonical evidence).
- `../../README.md`, `../../../README.md`, `../../core/README.md`,
  `../../tools/runtime/README.md` -- module-level READMEs that may
  link back into this docs tree.
