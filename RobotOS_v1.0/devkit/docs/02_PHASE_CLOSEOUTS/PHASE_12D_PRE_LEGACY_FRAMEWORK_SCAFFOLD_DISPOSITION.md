# Phase 12D-pre -- Legacy Framework Scaffold Disposition

**Status:** `CLOSED_DOCS_ONLY`
**Type:** Docs-only architecture-boundary phase. **No source, runtime, test,
CMake, Zephyr, board, host-tool, script, `prj.conf`, DTS overlay, evidence
log change. No source file deleted, moved, or renamed. No new Framework path
created.** Phase 12D-pre is a docs-only classification of the pre-existing
legacy scaffold under `RobotOS_v1.0/src/` and `RobotOS_v1.0/include/robotos/`
that coexists with the Phase 1–12 active stack but has zero source overlap
and zero hardware evidence.
**Date opened/closed:** 2026-05-12 (same-day docs-only close)
**Branch:** master
**Published baseline at open:** `origin/master = c3a9384`
**Prior closed phase:** Phase 12C (`CLOSED_DOCS_ONLY`; FSM event bridge +
status model confirmation).
**Prior runtime implementation phase:** Phase 11D (firmware `2040bfb`).
**Prior hardware evidence phase:** Phase 11E (`10710b3`).
**Companion docs:**
[`PHASE_12C_FSM_EVENT_BRIDGE_STATUS_MODEL.md`](PHASE_12C_FSM_EVENT_BRIDGE_STATUS_MODEL.md),
[`PHASE_12B_FRAMEWORK_FSM_API_DRAFT.md`](PHASE_12B_FRAMEWORK_FSM_API_DRAFT.md),
[`PHASE_12A_FRAMEWORK_API_SURFACE_PLANNING.md`](PHASE_12A_FRAMEWORK_API_SURFACE_PLANNING.md),
[`PHASE_11Z_COMMAND_SET_CHECKPOINT.md`](PHASE_11Z_COMMAND_SET_CHECKPOINT.md),
[`../03_SPECS/FRAMEWORK_FSM_API_DRAFT.md`](../03_SPECS/FRAMEWORK_FSM_API_DRAFT.md).
**New companion notices:**
[`../../../src/README_LEGACY_SCAFFOLD.md`](../../../src/README_LEGACY_SCAFFOLD.md),
[`../../../src/framework/DEPRECATED.md`](../../../src/framework/DEPRECATED.md),
[`../../../include/robotos/DEPRECATED.md`](../../../include/robotos/DEPRECATED.md).

---

## A. Executive Summary

Phase 12D-pre is a docs-only architecture-boundary phase that **classifies
the pre-existing `RobotOS_v1.0/src/` + `RobotOS_v1.0/include/robotos/`
scaffold** (committed at the original `43de448` Zephyr devkit bring-up
baseline) as a **frozen, non-authoritative legacy scaffold** that is not the
active Robot Framework path for Phase 12+ work.

It places three plain-text notices in the legacy tree (one root, one in
`src/framework/`, one in `include/robotos/`) so the architectural status is
visible from any directory listing, and it preserves the disposition in the
docs index so future agents and contributors do not silently re-extend the
legacy scaffold.

**Decision result: `LEGACY_SCAFFOLD_MARKED_FROZEN_DOCS_ONLY`**

Phase 12D-pre:

- Does **not** delete, move, or rename any file in the legacy scaffold.
- Does **not** reconcile the two architectures (Architecture A
  `core/`+`platform/`+`devkit/` vs Architecture B `src/`+`include/robotos/`).
- Does **not** create the new active Framework path
  (`RobotOS_v1.0/framework/`).
- Does **not** create any `.h` or `.c` Framework file.
- Does **not** start Phase 12D.
- Does **not** modify any source, test, CMake, Zephyr, board, `prj.conf`,
  DTS, script, runtime, or evidence-log file.
- Does **not** change any command semantics.
- Does **not** open ACTIVE disarm widening, Scheduler 7A/7B, F407, or
  Application/product work.

It blocks silent architectural drift: Phase 12D may now proceed on
`RobotOS_v1.0/framework/` with the legacy classification explicitly on
record.

---

## B. Baseline Before Phase 12D-pre

| Item | Value |
|---|---|
| `origin/master` at open | `c3a9384` (`docs: confirm Phase 12C FSM bridge and status model`) |
| Last closed phase | Phase 12C (`CLOSED_DOCS_ONLY`) |
| Phase 12C decision | `PHASE_12C_EVENT_BRIDGE_STATUS_CONFIRMED_DOCS_ONLY` |
| Phase 12C confirmations | Event bridge (`APPLICATION_OWNED_EVENT_BRIDGE_CONFIRMED`); status model (`REUSE_ROBOTOS_CORE_STATUS_T_FOR_PHASE_12D_CONFIRMED`); payload lifetime (`PAYLOAD_BORROWED_FOR_DISPATCH_ONLY_CONFIRMED`); action non-OK (`ACTION_NON_OK_NO_ROLLBACK_CONFIRMED`); guard return (`GUARD_RETURNS_BOOL_ONLY_CONFIRMED`); evaluation order corrected to exit → state update → action → entry |
| Phase 12D | **NOT_STARTED**; blocked until Phase 12D-pre closes |
| Robot Framework implementation | **NOT BUILT** |
| Application / product layer | **NOT BUILT** |
| Validated command set | **`a / s / r / ? / x / v / L / d / T`** (9 commands; hardware-evidence-backed) |
| Last runtime implementation phase | Phase 11D (`2040bfb`) |
| Last hardware evidence phase | Phase 11E (`10710b3`; `CLOSED_WITH_HARDWARE_EVIDENCE` + `OPERATOR_PHYSICAL_SANITY_CONFIRMED`) |
| Hardware platform | STM32F411E-DISCO rev D (Architecture A only) |
| Scheduler | `MAX_EVENTS_PER_TICK = 1`; `QUEUE_CAPACITY = 16`; 7A/7B `DEFER` |
| UART TX scope | Minimal response only; 12 scope-guard constraints intact |
| POST_FLASH_AUTOSTART | `OPEN` / `MITIGATED_BY_WORKFLOW` (Phase 6O sidecar `reset run`) |
| Devkit event type allocation | 100 (legacy runtime handler), 101 (timer), 102 (button), 103 (UART) |

**Remaining open gates (all preserved unchanged at Phase 12D-pre):**

1. ACTIVE disarm widening — `USER_DECISION_REQUIRED_ACTIVE_DISARM`
2. Scheduler 7A/7B — `DEFER`
3. F407 / custom board — `HOLD/DEFER`
4. POST_FLASH_AUTOSTART root cause — `OPEN` / `MITIGATED_BY_WORKFLOW`
5. Application / product layer — `NOT_STARTED`
6. Robot Framework implementation — `NOT_STARTED`; Phase 12D blocked pending this disposition

---

## C. Repo Architecture Split Finding

Phase 12D-pre formally records the architectural split surfaced by the
Framework Layer Path Decision Audit performed after Phase 12C closed. The
audit found two architecturally distinct, mutually incompatible stacks
rooted at the same baseline commit `43de448` (2025-03-05 "feat(devkit):
Zephyr devkit bring-up baseline for STM32F411E-DISCO").

### C.1 Architecture A — active, evidence-backed, Phase 1–12 canonical

| Aspect | Value |
|---|---|
| Top-level dirs | `RobotOS_v1.0/core/`, `RobotOS_v1.0/platform/`, `RobotOS_v1.0/devkit/` |
| Build entry | `RobotOS_v1.0/devkit/CMakeLists.txt` (Zephyr devkit hardware build) |
| Runtime substrate | `core/robotos_core.{c,h}`, `core/robotos_event_queue.{c,h}`, `core/robotos_event_dispatcher.{c,h}` |
| Platform primitives | `platform/robotos_platform_critical.h`, `platform/robotos_platform_time.h`, `platform/robotos_platform_fault.h`, `platform/robotos_platform_log.h` (+ Zephyr backends under `platform/zephyr/`) |
| Status type | `robotos_core_status_t` (enum with `OK`, `ERR_INVALID_STATE`, `ERR_NULL`, `ERR_FULL`, `ERR_EMPTY`, `ERR_INVALID_ARG`, `ERR_THROTTLED`) |
| Event type | `robotos_event_type_t`, `robotos_event_t` (allocates `100`, `101`, `102`, `103` for devkit producers) |
| Framework layer | **NOT BUILT** at Phase 12D-pre; Phase 12A/12B/12C planning complete; Phase 12D will create `RobotOS_v1.0/framework/` on explicit user authorization |
| Application layer | **NOT BUILT** |
| Hardware evidence | Phase 1–11 RTT logs, host transcripts, `OPERATOR_PHYSICAL_SANITY_CONFIRMED` (Phase 11E), 9-command set validated |
| Last source change | `c3a9384` (Phase 12C, 2026-05-12) — continuous evolution Mar 2025 → present |

### C.2 Architecture B — frozen legacy scaffold

| Aspect | Value |
|---|---|
| Top-level dirs | `RobotOS_v1.0/src/`, `RobotOS_v1.0/include/`, `RobotOS_v1.0/CMakeLists.txt` (root) |
| Build entry | `RobotOS_v1.0/CMakeLists.txt` (root) — Zephyr firmware OR host-test mode; **not invoked by the devkit hardware build** |
| Adapter / HAL | `include/robotos/ro_*.h` (15 headers: `ro_status`, `ro_thread`, `ro_time`, `ro_timer`, `ro_queue`, `ro_pool`, `ro_mutex`, `ro_gpio`, `ro_pwm`, `ro_i2c`, `ro_spi`, `ro_log`, `ro_trace`, `ro_deadline`, `ro_assert`); implementations under `src/adapter/zephyr/` and `src/adapter/host/` |
| Status type | `ro_status_t` (`int32_t`, `RO_OK = 0`, error codes down to `EBOOTSEQ = -23`) |
| Framework layer | `src/framework/{stepper,servo,dcmotor,encoder,endstop,sensor,pid,filter,limiter,robot_sm}.c` + `src/framework/drivers/{stepper_drv,endstop_drv}.c`; 10 framework headers under `include/robotos/` |
| Application layer | `src/app/{app_main,app_sm,gcode_parser,motion_planner,kinematics_cartesian,config_profiles,app_glue_robotos,app_glue_zephyr}.c`; headers under `include/app/` |
| Hardware evidence | **NONE.** Zero Phase 1–11 evidence log references any `src/` or `include/robotos/` file |
| Last source change | `43de448` (Mar 5, 2025) — **frozen at baseline; never modified** in the 14 months of Phase 1–11 work |
| Compiled in devkit build | **NO** — the devkit hardware build invokes only `devkit/CMakeLists.txt`, which pulls in `core/`, `platform/`, and `devkit/src/`; nothing under `src/` or `include/robotos/` is compiled by the validated hardware path |

### C.3 The two stacks are type-incompatible

Architecture A and Architecture B cannot be mixed without an explicit
reconciliation phase:

- `robotos_core_status_t` (A) ≠ `ro_status_t` (B) — different enums, different namespaces.
- `robotos_event_t` (A) ≠ `ro_queue_t` items (B) — different event models.
- `robotos_platform_critical_*` (A) ≠ `ro_mutex_*` (B) — different synchronization primitives.
- `robotos_platform_time.h` (A) ≠ `ro_time.h` (B) — different time APIs.

No source file in A references any header or symbol from B, and vice versa
(verified via grep at Phase 12D-pre open).

### C.4 The name collision

`src/framework/README.md` opens with "**Robot Framework Layer**" and
`include/robotos/robot_sm.h` defines a state-machine API
(`sm_dispatch`/`sm_get_state`/`robot_state_t = {IDLE,HOMING,RUN,FAULT}`).
These constructs share **conceptual** ground with the Phase 12B/12C
`robotos_fw_fsm_*` design but use incompatible types
(`ro_status_t` vs `robotos_core_status_t`) and a different (product-coupled)
state vocabulary. Phase 12D-pre records this as a name-and-concept collision
without authorizing any reconciliation work.

---

## D. Confirmed Disposition

**Decision result:** **`LEGACY_SCAFFOLD_MARKED_FROZEN_DOCS_ONLY`**

### D.1 Confirmed statements

- **Architecture A is canonical** for active RobotOS Phase 12+ work.
- **Architecture B is legacy / frozen / non-authoritative** unless and until
  an explicit reconciliation phase is opened in the future. Phase 12D-pre
  does not open such a phase.
- **`RobotOS_v1.0/src/framework/` is not the active path** for Phase 12+
  Robot Framework implementation.
- **`RobotOS_v1.0/include/robotos/` is not the active namespace** for the
  Phase 12+ Framework FSM. New Framework headers must not be added there
  without an explicit reconciliation phase.
- **`RobotOS_v1.0/CMakeLists.txt` (root)** belongs to the legacy scaffold
  build path. The devkit hardware build (which produces all Phase 1–11
  evidence) uses `RobotOS_v1.0/devkit/CMakeLists.txt`. The root CMakeLists
  is left untouched by Phase 12D-pre but is classified as legacy.
- **Future active Framework path remains `RobotOS_v1.0/framework/`** — a
  new top-level sibling to `core/`, `platform/`, `devkit/`. This path is
  **not created** by Phase 12D-pre; it is created (if at all) by Phase 12D
  on explicit user authorization.

### D.2 Notice files placed in the legacy tree

Phase 12D-pre places three plain-text notices so the architectural status
is visible from any directory listing of the legacy tree:

| Path | Content summary |
|---|---|
| `RobotOS_v1.0/src/README_LEGACY_SCAFFOLD.md` | Whole `src/` tree is the legacy/frozen scaffold; not the active stack; do not modify or extend without an explicit reconciliation phase |
| `RobotOS_v1.0/src/framework/DEPRECATED.md` | This directory is the frozen legacy Robot Framework scaffold; non-authoritative for current Phase 12+ work; type-incompatible with the active stack; future active path is `RobotOS_v1.0/framework/` |
| `RobotOS_v1.0/include/robotos/DEPRECATED.md` | `include/robotos/` namespace is part of the legacy scaffold; not the namespace for new `robotos_fw_fsm.h`; do not add Phase 12+ Framework headers here without an explicit reconciliation phase |

These notices are **classification documents**, not deprecation removals.
No source under `src/` or `include/robotos/` is touched.

---

## E. Why Not Delete / Move / Reconcile Now

Phase 12D-pre is intentionally minimal: classify the boundary, place
notices, commit, and stop. Larger options were considered and rejected:

### E.1 Why not delete the legacy scaffold

- Deletion is destructive. Even if the legacy scaffold has no Phase 1–11
  evidence, it represents real design intent committed by the user at
  `43de448`. Removing it from the working tree on the basis of a docs-only
  phase is too aggressive.
- Git history would preserve the files, but a `git rm -r` operation moves
  beyond "docs-only boundary phase" into "tree mutation".
- The user's stated direction is "do not delete now".

### E.2 Why not move the legacy scaffold

- `git mv RobotOS_v1.0/src/ RobotOS_v1.0/legacy_scaffold_2025_03/` would
  preserve history but produce a noisy diff touching every file in the
  legacy tree.
- It would also break the root `RobotOS_v1.0/CMakeLists.txt` (which
  references `src/adapter/...`, `src/framework/...`, `src/app/...`) unless
  the CMakeLists.txt is updated, which crosses the "no CMake change"
  boundary.
- A move is reasonable as a future reconciliation step but is not
  Phase 12D-pre's responsibility.

### E.3 Why not reconcile the two architectures now

- Reconciling Architecture A and Architecture B (mapping `ro_status_t` to
  `robotos_core_status_t`, mapping `ro_queue` to `robotos_event_queue`,
  unifying `ro_mutex` and `robotos_platform_critical`, deciding whether
  `src/framework/robot_sm.c` should live, etc.) is a substantial
  multi-phase architectural workstream.
- It would invalidate the Phase 12B/12C Framework FSM design surface
  (the new FSM uses `robotos_core_status_t`, not `ro_status_t`).
- Reconciliation belongs in its own future track, not Phase 12D-pre.
- The user's stated direction is "do not reconcile now".

### E.4 Archive-in-place is the right shape

- Preserves every file unchanged.
- Preserves the root `CMakeLists.txt` (which the user may still use for
  legacy host-test builds; this is not for Phase 12D-pre to judge).
- Places clear notices so future readers cannot mistake the legacy
  scaffold for the active stack.
- Records the architectural decision in a docs-only closeout and the
  progress stream.
- Imposes zero scope expansion on Phase 12D when it later opens.

---

## F. Future Framework Path Decision

For the eventual Phase 12D opening (only on explicit user authorization,
not authorized by Phase 12D-pre):

| Item | Value |
|---|---|
| Recommended path | `RobotOS_v1.0/framework/` — new top-level sibling to `core/`, `platform/`, `devkit/` |
| Recommended Phase 12D minimal artifact | `RobotOS_v1.0/framework/README.md` (NEW); `RobotOS_v1.0/framework/robotos_fw_fsm.h` (NEW, header stub only) |
| Phase 12D `.c` body | **NONE.** Header-only. Dispatch logic is later (Phase 12E or 13). |
| Phase 12D CMake | **NONE.** No new `CMakeLists.txt`; no modification of `devkit/CMakeLists.txt`. |
| Phase 12D devkit integration | **NONE.** No devkit consumer; no `devkit_app_state` change. |
| Phase 12D hardware run | **NONE.** Docs + header only. |
| Phase 12D include style | `#include "robotos_fw_fsm.h"` — matches Architecture A relative-include convention; explicitly **not** `<robotos/...>` (which is the legacy namespace) |
| Phase 12D non-goals | No `.c`; no CMake; no devkit integration; no UART command addition; no `devkit_app_state` replacement; no actuator/sensor/PID Framework API; no hardware run; no Scheduler change; no F407; no Application/product work; no modification of Architecture B except referencing its frozen status |

The path `RobotOS_v1.0/framework/` is distinct from the legacy
`RobotOS_v1.0/src/framework/` by parent directory. The two cannot be
conflated.

---

## G. Relationship to Phase 12A / 12B / 12C

Phase 12A, 12B, and 12C described the pre-existing `src/framework/` files
as **"pre-existing unrelated"** and noted they were **untouched** by those
phases. That wording is **operationally true** (Architecture A and B share
zero source code and zero hardware evidence) but **architecturally
incomplete**: it did not record that `src/framework/` is itself a
"Robot Framework Layer" scaffold under a competing path.

Phase 12D-pre corrects this classification without invalidating any prior
phase's design decisions.

### G.1 Design decisions from Phase 12A/12B/12C remain valid

All Phase 12A–12C decisions were taken against Architecture A:

- Phase 12A layer boundary (`devkit_app_state` is not Framework; Framework
  consumes Adapter primitives through explicit contracts; Application not
  built) — still correct.
- Phase 12B FSM model (flat, table-driven, no heap, static config; product-
  defined `uint32_t` state/event IDs; `robotos_fw_event_id_t` decoupled
  from `robotos_event_type_t`) — still correct.
- Phase 12C confirmations (`APPLICATION_OWNED_EVENT_BRIDGE_CONFIRMED`,
  `REUSE_ROBOTOS_CORE_STATUS_T_FOR_PHASE_12D_CONFIRMED`,
  `PAYLOAD_BORROWED_FOR_DISPATCH_ONLY_CONFIRMED`,
  `ACTION_NON_OK_NO_ROLLBACK_CONFIRMED`, `GUARD_RETURNS_BOOL_ONLY_CONFIRMED`,
  evaluation order exit → state update → action → entry) — still correct.

No design decision is changed by Phase 12D-pre.

### G.2 The single correction Phase 12D-pre records

`src/framework/` is now **classified as the legacy scaffold** (frozen,
non-authoritative, type-incompatible) rather than as "pre-existing
unrelated". The new active Framework path will be `RobotOS_v1.0/framework/`
(distinct parent directory).

### G.3 Phase 12A/12B/12C closeouts are not rewritten

Per the progress file rule "phase truth is anchored in `01_PROGRESS/` and
`02_PHASE_CLOSEOUTS/`" and per the convention that closed phase docs are
not rewritten retroactively, Phase 12A, 12B, and 12C closeouts are
preserved as-is. Phase 12D-pre is the canonical record of the corrected
classification; future readers should consult this document.

The long-lived spec [`FRAMEWORK_FSM_API_DRAFT.md`](../03_SPECS/FRAMEWORK_FSM_API_DRAFT.md)
is an evolving artifact; if a small forward-pointer to Phase 12D-pre is
useful there, it is added as a clarification note, not a rewrite.

---

## H. Non-goals

Phase 12D-pre explicitly does not:

- **Delete** any source file under `src/`, `include/`, or anywhere else.
- **Move** any source file or directory (no `git mv`).
- **Rename** any source file or directory.
- **Reconcile** Architecture A and Architecture B (no type mapping; no HAL
  unification; no shared status enum).
- **Compile** any Architecture B target.
- **Create** the new active Framework path `RobotOS_v1.0/framework/`.
- **Create** any Framework `.h` or `.c` source file.
- **Modify** `RobotOS_v1.0/CMakeLists.txt` (root, legacy build entry).
- **Modify** `RobotOS_v1.0/devkit/CMakeLists.txt` (active build entry).
- **Modify** any `core/`, `platform/`, `devkit/src/`, or test source.
- **Modify** `prj.conf`, board DTS, overlay, Kconfig, host tool, runtime
  script, or evidence log.
- **Modify** `DEVKIT_PROGRESS.md` historical master or
  `DEVKIT_PROGRESS_PHASE_10.md`.
- **Modify** any Phase 11A–11Z or Phase 12A–12C closeout (those are closed
  evidence artifacts).
- **Change** any UART command semantics (`a/s/r/?/x/v/L/d/T` unchanged).
- **Add** any new UART command or command candidate.
- **Promote** `devkit_app_state` to Framework (scope-guard #11 unchanged).
- **Promote** `T` to a Framework sensor API.
- **Start** Phase 12D.
- **Start** ACTIVE disarm widening.
- **Reopen** Scheduler 7A/7B.
- **Reopen** F407 / custom board.
- **Start** Application / product work.
- **Perform** any hardware run.
- **Push** to remote.

---

## I. Next Gate

| Item | Value |
|---|---|
| Next possible gate | Phase 12D — Framework FSM Header Stub (docs + single header in `RobotOS_v1.0/framework/`; no `.c`; no CMake) |
| Entry to Phase 12D | **Explicit user authorization required.** Phase 12D-pre does not auto-open Phase 12D. |
| Phase 12D expected path | `RobotOS_v1.0/framework/` |
| Phase 12D expected scope | Header-only FSM stub + `framework/README.md` + Phase 12D closeout + progress entry + spec update from DRAFT to LOCKED-AT-12D for §4 names + CURRENT_STATE + INDEX |
| Phase 12D must not touch Architecture B | Phase 12D may reference legacy/frozen status of `src/framework/` and `include/robotos/` in its closeout but must not modify any file there |
| Alternative | **Hold.** Phase 12B/12C design is durable as a spec; implementation can wait without losing any work. |

---

## J. What this document does not do

- Does not modify any source file under `src/`, `include/`, `core/`,
  `platform/`, or `devkit/src/`.
- Does not delete, move, or rename any source file.
- Does not modify any `CMakeLists.txt`.
- Does not modify `prj.conf`, board DTS, overlay, host tool, runtime
  script, or evidence log.
- Does not create `RobotOS_v1.0/framework/` or any `.h`/`.c` Framework
  file.
- Does not implement any FSM logic.
- Does not promote `devkit_app_state` or `T` to Framework.
- Does not change UART command semantics or add commands.
- Does not start Phase 12D.
- Does not authorize hardware purchase or hardware run.
- Does not authorize ACTIVE disarm widening.
- Does not reopen Scheduler 7A/7B, F407, or POST_FLASH_AUTOSTART
  investigation.
- Does not start Application / product work.
- Does not modify `DEVKIT_PROGRESS.md` historical master.
- Does not modify any evidence log under `RobotOS_v1.0/devkit/logs/`.
- Does not push.
