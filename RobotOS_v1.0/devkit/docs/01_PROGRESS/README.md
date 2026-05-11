# 01_PROGRESS -- Devkit Progress Streams

This directory holds the **running phase logs** for the RobotOS
devkit. Each file is a narrative index of the phases in its window:
status, brief summary, and pointers to the per-phase closeout docs
under [`../02_PHASE_CLOSEOUTS/`](../02_PHASE_CLOSEOUTS/) when those
exist.

Progress files in this directory are canonical phase truth. The
top-level [`../00_INDEX/README.md`](../00_INDEX/README.md) and
[`../../../../CURRENT_STATE.md`](../../../../CURRENT_STATE.md) point
back here for that reason.

---

## Phase-window convention

Starting at Phase 11, **progress is recorded per 10-phase window**,
not per phase:

| Phase range | Progress file |
|---|---|
| Phase 1 through Phase 9E / 9E-Z | [`DEVKIT_PROGRESS.md`](DEVKIT_PROGRESS.md) (historical master) |
| Phase 10 (10A, 10B-*, 10C, 10Z, ...) | [`DEVKIT_PROGRESS_PHASE_10.md`](DEVKIT_PROGRESS_PHASE_10.md) |
| Phase 11 through Phase 20 | [`DEVKIT_PROGRESS_PHASE_11_20.md`](DEVKIT_PROGRESS_PHASE_11_20.md) (currently `RESERVED / NOT_STARTED`) |
| Phase 21 through Phase 30 | future `DEVKIT_PROGRESS_PHASE_21_30.md` -- create when Phase 21 opens |
| Phase 31 through Phase 40 | future `DEVKIT_PROGRESS_PHASE_31_40.md` -- create when Phase 31 opens |

The convention applies to **progress stream files only**. It does
**not** prevent or replace per-phase closeout / checkpoint documents
under `../02_PHASE_CLOSEOUTS/` -- a phase with hardware evidence,
implementation closeout, checkpoint, direction guard, architecture /
design decision, important validation record, or release / freeze
implication may (and usually should) still publish its own dedicated
`PHASE_*_CLOSE.md` or `PHASE_*_CHECKPOINT.md`.

Phase 1 through Phase 10 keep their historical naming
(`DEVKIT_PROGRESS.md` and `DEVKIT_PROGRESS_PHASE_10.md`) without
retroactive rename or reorganization.

---

## Distinction: progress streams vs. closeout docs

| Aspect | Progress stream (this directory) | Closeout / checkpoint doc (`../02_PHASE_CLOSEOUTS/`) |
|---|---|---|
| Number of files | One file per 10-phase window (from Phase 11 onward); two historical files for Phase 1-10. | One file per phase that warrants one. Optional but encouraged for any phase with evidence or a design decision. |
| Form | Narrative index: status, short summary, anchors, links. | Full-form evidence: scope-guard restatement, counter tables, host transcripts, RTT counters, verdict, what-the-doc-does-not-do. |
| Rewrite policy | Append-only after a phase closes. Manual anchors are mandatory so links survive heading edits. | Frozen after a phase closes. Each closeout stands on its own captured run; do not rewrite another phase's evidence. |
| Index location | Each progress file maintains its own internal phase index. Cross-window navigation lives in `../00_INDEX/README.md`. | Each closeout file is reachable from `../00_INDEX/README.md` and from the corresponding progress-stream entry. |

---

## Editing rules (apply to every progress file in this directory)

These rules are reproduced in each progress file (see
`DEVKIT_PROGRESS_PHASE_10.md` §4 and
`DEVKIT_PROGRESS_PHASE_11_20.md` §3). They are also restated here so
the directory's policy is visible without opening any progress file:

1. **History stays put.** Closed phases in any progress file remain
   frozen except for strict link / typo fixes.
2. **New phases land in the current window's file.** Do not back-
   propagate Phase 11+ work into `DEVKIT_PROGRESS.md` or
   `DEVKIT_PROGRESS_PHASE_10.md`.
3. **Manual anchors are mandatory** (`<a id="phase-NN[-X]"></a>` on
   the line immediately before each phase heading).
4. **Index links target manual anchors**, not heading slugs.
5. **Do not rely on GitHub auto-generated heading anchors.**
6. **Make non-linear edits explicit** (`‡` mark, anchor suffix, etc.)
   when a phase is amended or inserted out of order.
7. **No silent baseline change.**
8. **Closeouts go to `../02_PHASE_CLOSEOUTS/`, not here.** This
   directory carries the narrative index; full evidence lives in the
   closeout docs.

---

## When to create the next-window progress file

The next-window file (e.g. `DEVKIT_PROGRESS_PHASE_21_30.md`) should
be created **when Phase 21 is about to open**, not preemptively. The
trigger is the same as for the Phase 11-20 scaffold:

- A reserved-only scaffold (status `RESERVED / NOT_STARTED`).
- An explicit user authorization to open the next window.
- A docs-only commit; no source / test / config change.
- The previous window's progress file (e.g.
  `DEVKIT_PROGRESS_PHASE_11_20.md`) remains active until its window
  is genuinely closed -- the new file does not retroactively migrate
  phases from the previous window.

The Phase 9E-Z direction-guard discipline (separating scaffold
creation from phase opening) is the rule: scaffolds carry no
implementation claim and require no validation; opening a phase
inside a scaffold requires the full Phase 10B-style discipline.
