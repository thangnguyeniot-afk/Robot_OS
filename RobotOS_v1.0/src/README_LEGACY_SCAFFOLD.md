# `RobotOS_v1.0/src/` — Legacy / Frozen Scaffold

> **NOT THE ACTIVE STACK.** This tree is a frozen, non-authoritative
> scaffold from the original Zephyr devkit bring-up baseline
> (commit `43de448`, 2025-03-05). It is **not** the path used by the
> Phase 1–12 active RobotOS work and has **no hardware evidence**
> from any phase. Do not extend or modify it without an explicit,
> approved reconciliation phase.

---

## Status

| Item | Value |
|---|---|
| Status | **Legacy / frozen scaffold** |
| Baseline commit | `43de448` (2025-03-05) — "feat(devkit): Zephyr devkit bring-up baseline for STM32F411E-DISCO" |
| Source evolution since baseline | **Zero.** No source file under this tree has been modified by any phase since `43de448`. |
| Hardware evidence | **None.** Zero Phase 1–11 evidence log references any file under this tree. |
| Compiled by active devkit build | **No.** The Phase 1–11 hardware-validated build invokes `RobotOS_v1.0/devkit/CMakeLists.txt`, which does not reference this tree. |
| Authoritative status | **Non-authoritative** for current Phase 12+ Robot Framework work. |
| Disposition record | [`RobotOS_v1.0/devkit/docs/02_PHASE_CLOSEOUTS/PHASE_12D_PRE_LEGACY_FRAMEWORK_SCAFFOLD_DISPOSITION.md`](../devkit/docs/02_PHASE_CLOSEOUTS/PHASE_12D_PRE_LEGACY_FRAMEWORK_SCAFFOLD_DISPOSITION.md) |

---

## Two architectures coexist in this repo

| Architecture A — active, evidence-backed | Architecture B — this tree (legacy / frozen) |
|---|---|
| `RobotOS_v1.0/core/`, `RobotOS_v1.0/platform/`, `RobotOS_v1.0/devkit/` | `RobotOS_v1.0/src/`, `RobotOS_v1.0/include/robotos/`, `RobotOS_v1.0/CMakeLists.txt` (root) |
| Status type: `robotos_core_status_t` | Status type: `ro_status_t` |
| Event type: `robotos_event_type_t` / `robotos_event_t` | Queue API: `ro_queue_t` (different model) |
| Platform primitives: `robotos_platform_*` (`time`, `fault`, `critical`, `log`) | HAL primitives: `ro_*` (15 headers under `include/robotos/`) |
| Hardware-validated through Phase 11E on STM32F411E-DISCO rev D | Never built or run on hardware |
| All Phase 12 Framework planning is built on this stack | Not used by Phase 12 planning |

**The two stacks are type-incompatible.** No code in Architecture A
references this tree, and no code in this tree references Architecture A.
Mixing them requires an explicit reconciliation phase that has not been
authorized.

---

## What this notice changes

**Nothing in the source tree.** Phase 12D-pre is docs-only:

- This notice file is added.
- A `DEPRECATED.md` is added under `src/framework/`.
- A `DEPRECATED.md` is added under `include/robotos/`.
- All other files under `src/` and `include/robotos/` are **untouched** —
  no deletion, no move, no rename, no edit.

The classification is recorded so that future agents and contributors do
not mistake this tree for the active Phase 12+ stack and do not silently
extend it.

---

## Rules for this tree

1. **Do not add new files here for Phase 12+ work.** Active Robot Framework
   work goes to `RobotOS_v1.0/framework/` (created in Phase 12D on explicit
   user authorization; not yet created at the time this notice is written).
2. **Do not extend existing files here.** Architecture B is frozen pending
   any future explicit reconciliation phase.
3. **Do not delete files from this tree.** Git history is preserved by
   leaving the files in place; deletion is a separate (and currently
   unauthorized) decision.
4. **Do not invoke the root `RobotOS_v1.0/CMakeLists.txt`** as part of any
   Phase 12+ hardware validation flow. The active build is the devkit
   build at `RobotOS_v1.0/devkit/CMakeLists.txt`.
5. **A future reconciliation phase may revisit this disposition.** It is
   not currently planned or authorized.

---

## Why archive in place

- **Preserves history.** Every file remains on disk; git log under this
  tree is intact.
- **Preserves intent.** The original design (a portable Robot Framework
  layer with a `ro_*` HAL) is visible to future readers as a design
  reference, even if it was not the path the project took.
- **Avoids destructive operations.** No `rm`, no `git rm`, no `git mv` is
  performed by Phase 12D-pre.
- **Avoids CMake churn.** The root `RobotOS_v1.0/CMakeLists.txt` continues
  to reference paths under this tree; that CMakeLists.txt is itself part
  of the legacy scaffold and is not touched by Phase 12D-pre.

---

## Active path

For active Phase 12+ Robot Framework work, use:

| Layer | Active path |
|---|---|
| Runtime substrate | `RobotOS_v1.0/core/` |
| Platform primitives | `RobotOS_v1.0/platform/` |
| Validation harness | `RobotOS_v1.0/devkit/` |
| **Robot Framework (future)** | `RobotOS_v1.0/framework/` (not yet created; pending Phase 12D) |

See [`RobotOS_v1.0/devkit/docs/03_SPECS/FRAMEWORK_FSM_API_DRAFT.md`](../devkit/docs/03_SPECS/FRAMEWORK_FSM_API_DRAFT.md)
for the active Phase 12+ Framework FSM API draft.
