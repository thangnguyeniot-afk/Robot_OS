# DEVKIT_PROGRESS_PHASE_11_20.md -- RobotOS Devkit Phase Log, Phase 11-20

**Status:** `RESERVED / NOT_STARTED`

This file is the planned progress stream for Phase 11 through Phase 20.
**Phase 11 is not opened by this scaffold.** No implementation,
checkpoint, or evidence is recorded here yet. Adding content to this
file does not authorize any source, test, CMake, Zephyr, board,
`prj.conf`, host-tool, or script change.

---

## 1. Continuation Note

This file continues the RobotOS devkit phase log after
`DEVKIT_PROGRESS_PHASE_10.md`. Together with the historical master
`DEVKIT_PROGRESS.md`, the three progress streams form the canonical
phase history of the devkit:

| Phase range | File | Status at this scaffold's creation |
|---|---|---|
| Phase 1 through Phase 9E / Phase 9E-Z | [`DEVKIT_PROGRESS.md`](DEVKIT_PROGRESS.md) | Historical master; closed; not rewritten when later phases land. |
| Phase 10 (Phase 10A, 10B-v, 10B-L, 10B-d, 10C, ...) | [`DEVKIT_PROGRESS_PHASE_10.md`](DEVKIT_PROGRESS_PHASE_10.md) | Running log for Phase 10 stream; Phase 10C is the latest closed entry. |
| Phase 11 through Phase 20 | this file | `RESERVED / NOT_STARTED`. |

**Phase-specific closeout / checkpoint docs remain a separate artifact
class** under
[`../02_PHASE_CLOSEOUTS/`](../02_PHASE_CLOSEOUTS/). A phase with
hardware evidence, implementation closeout, checkpoint, direction
guard, architecture/design decision, important validation record, or
release/freeze implication may still publish its own
`PHASE_*_CLOSE.md` / `PHASE_*_CHECKPOINT.md` document under
`02_PHASE_CLOSEOUTS/`. The progress stream file (this file) is **not**
the place for full-form evidence; it is the running narrative index
that points at those closeout docs.

---

## 2. Phase-window convention (carried forward)

Per the docs-only convention recorded at the Phase 11-20 scaffold
opening:

- Progress files are **grouped by 10-phase windows**, not one file per
  phase, starting at Phase 11.
- Phase 11 through Phase 20 share **this** file
  (`DEVKIT_PROGRESS_PHASE_11_20.md`).
- Future windows follow the same rule:
  `DEVKIT_PROGRESS_PHASE_21_30.md`,
  `DEVKIT_PROGRESS_PHASE_31_40.md`, and so on -- created when the
  window opens, not before.
- Phase 1 through Phase 10 keep their historical naming
  (`DEVKIT_PROGRESS.md` and `DEVKIT_PROGRESS_PHASE_10.md`) without
  retroactive rename or reorganization.

This convention applies to **progress stream files only**. It does
**not** prevent or replace separate per-phase closeout / checkpoint
documents under `02_PHASE_CLOSEOUTS/` when a phase warrants one.

---

## 3. Split / Editing Rules (carried forward from Phase 10 stream)

These rules govern how this file and its sibling progress streams
evolve. They mirror `DEVKIT_PROGRESS_PHASE_10.md` §4 so the
discipline does not regress at the window boundary.

1. **History stays put.** `DEVKIT_PROGRESS.md` (Phase 1..9E/9E-Z) and
   `DEVKIT_PROGRESS_PHASE_10.md` (Phase 10) remain frozen except for
   strict link / typo fixes. Do not move, rewrite, reorder, or
   deduplicate old phase sections as part of using this file.
2. **New Phase 11-20 entries land here.** Every Phase 11 through
   Phase 20 entry (including any Phase 11A, 11B-x, 12, ..., 20Z,
   etc.) must be added to this file, not back-propagated to
   `DEVKIT_PROGRESS.md` or `DEVKIT_PROGRESS_PHASE_10.md`.
3. **Manual anchors are mandatory.** Each future phase section must
   place an explicit manual anchor (`<a id="phase-NN[-X]"></a>`) on
   the line immediately before its heading.
4. **Index links must target manual anchors.** Entries in the Phase
   Index table must link to the manual anchor id, not to a heading
   slug.
5. **Do not rely on GitHub auto-generated heading anchors.** Heading
   slugs change when titles are edited; manual anchors do not.
6. **Make non-linear edits explicit.** If a phase is duplicated,
   amended, or inserted out of phase-number order, mark the index
   entry to make the duplication or insertion obvious (e.g. `‡` or
   anchor suffixes like `-1` / `-2`), mirroring the existing
   convention in `DEVKIT_PROGRESS.md`.
7. **No silent baseline change.** Do not alter Phase 10C's recorded
   baseline (or any earlier closed phase) to claim work has
   progressed beyond what has been validated and recorded as a closed
   Phase 11+ entry below.
8. **Closeouts go to `../02_PHASE_CLOSEOUTS/`, not here.** This file
   carries the narrative index and brief per-phase summaries that
   point at closeouts. Full evidence, scope-guard restatements,
   counter tables, and verdict sections belong in the dedicated
   closeout doc.

---

## 4. Phase Index

Entries are added when a phase is formally opened. Until then, the
table contains only the reserved placeholders.

| Phase | Title | Status | Jump |
|-------|-------|--------|------|
| 11 | RESERVED -- not opened | NOT_STARTED | [->](#phase-11) |
| 20Z | RESERVED -- future checkpoint / closeout slot | NOT_STARTED | [->](#phase-20z) |

When future phases are added:

- Insert a new manual-anchor block (`<a id="phase-NN"></a>`)
  immediately before the new phase heading.
- Add a row to the table above linking to that manual anchor.
- Keep the index in insertion / session order, matching the
  convention used in `DEVKIT_PROGRESS_PHASE_10.md` and
  `DEVKIT_PROGRESS.md`. Phase-number order is **not** required.

---

## 5. Phase Sections

<a id="phase-11"></a>
## Phase 11 -- RESERVED / NOT STARTED

**Status:** `NOT_STARTED`

Placeholder only. Phase 11 has not been opened by the user. **Do not
add implementation claims, evidence, or behavioral specification
here until Phase 11 is formally authorized.** Opening Phase 11
requires:

- Explicit user authorization for the specific Phase 11 scope (a
  named, scoped task -- not a vague continuation).
- A clear gate against the still-open Phase 10C decisions:
  - `T` (sensor read) prerequisites: sensor part, driver / `prj.conf`
    change, response format, error variant, fixed-buffer compliance.
  - ACTIVE disarm widening (`USER_DECISION_REQUIRED_ACTIVE_DISARM`).
  - Scheduler 7A/7B remains `DEFER` unless new workload evidence
    appears.
  - F407 / custom board remains `HOLD/DEFER` unless explicit user
    decision.
- Scope-guard restatement carried forward from
  [`../02_PHASE_CLOSEOUTS/PHASE_9EZ_CHECKPOINT.md`](../02_PHASE_CLOSEOUTS/PHASE_9EZ_CHECKPOINT.md)
  section H (12 UART TX scope-guard constraints) before any
  Phase 11 source change is authorized.

This scaffold neither selects nor opens any of those decisions.

---

<a id="phase-20z"></a>
## Phase 20Z -- RESERVED / NOT STARTED

**Status:** `NOT_STARTED`

Placeholder only. Use this only for a future Phase 20 checkpoint /
closeout if approved, mirroring the existing `phase-10z` reserved
slot in `DEVKIT_PROGRESS_PHASE_10.md`. If Phase 20Z is opened, it
should:

- Be `CLOSED_DOCS_ONLY` (a checkpoint, not a runtime change), unless
  the user explicitly authorizes a runtime closeout.
- Carry forward the scope-guard and operational reminders from
  earlier checkpoints (Phase 9E-Z, Phase 10A, Phase 10C).
- Trigger creation of the **next** progress stream file
  (`DEVKIT_PROGRESS_PHASE_21_30.md`) only when Phase 21 is about to
  open.

This scaffold neither opens Phase 20Z nor pre-commits to its content.

---

## 6. What this scaffold does not do

- Does not open Phase 11.
- Does not authorize any source / test / CMake / Zephyr / board /
  `prj.conf` / host-tool / script change.
- Does not modify `DEVKIT_PROGRESS.md` (historical master) or
  `DEVKIT_PROGRESS_PHASE_10.md` (Phase 10 stream).
- Does not change command semantics (`a / s / r / ? / x / v / L / d`
  remain as closed at Phase 10C).
- Does not widen ACTIVE disarm; `USER_DECISION_REQUIRED_ACTIVE_DISARM`
  is preserved.
- Does not open `T` sensor read; `USER_DECISION_REQUIRED` is
  preserved.
- Does not reopen Scheduler 7A/7B or F407 / custom-board work.
- Does not change UART TX scope; all 12 scope-guard constraints
  remain intact.
- Does not change POST_FLASH_AUTOSTART status (root cause `OPEN`;
  `MITIGATED_BY_WORKFLOW` from Phase 6O onward).
- Does not push.
