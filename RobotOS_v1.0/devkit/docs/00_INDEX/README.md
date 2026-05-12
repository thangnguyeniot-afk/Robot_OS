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
| Phase 11 through Phase 20 | [`DEVKIT_PROGRESS_PHASE_11_20.md`](../01_PROGRESS/DEVKIT_PROGRESS_PHASE_11_20.md) | Running log; Phase 11E is latest closed entry (`CLOSED_WITH_HARDWARE_EVIDENCE`); Phase 11A/11B/11C/11D/11E closed; Phase 11A-11E sensor track complete. |
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

Future `PHASE_*_CLOSE.md` and `PHASE_*_CHECKPOINT.md` docs belong here.

## 03_SPECS — long-lived design / reference artifacts

Specs, reference docs, and runbooks that are long-lived design
artifacts (not snapshots of a single phase). They get updated in place
as the underlying contracts evolve.

- [`COMMAND_SET_DRAFT.md`](../03_SPECS/COMMAND_SET_DRAFT.md) -- product command vocabulary table (Sections A / B / C).
- [`TELEMETRY_REFERENCE.md`](../03_SPECS/TELEMETRY_REFERENCE.md) -- ROBOTOS_OBS / FAULT / PROD / BTN / UART / APP telemetry line definitions.
- [`WORKLOAD_DEMO_9D.md`](../03_SPECS/WORKLOAD_DEMO_9D.md) -- Phase 9D workload-demo runbook (hardware setup, sequence, expected counters).

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
