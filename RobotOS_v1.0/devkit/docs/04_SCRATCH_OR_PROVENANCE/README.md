# 04_SCRATCH_OR_PROVENANCE -- Policy

**This directory is for local scratch / provenance snapshots only.**

Anything that lives here is **not canonical** and **not part of the
repository's source of truth**. Canonical phase truth lives in
`../01_PROGRESS/` and `../02_PHASE_CLOSEOUTS/`. Canonical specs live
in `../03_SPECS/`.

## What may exist here

- Working scratch from earlier reorganization or migration passes that
  was useful at the time and is preserved for traceability rather than
  re-derivation.
- Provenance snapshots of files at a known prior state (e.g. before a
  seed-split or before a large reorganization), kept so the change
  history can be audited but not maintained as living docs.

Examples that have lived here historically (may or may not be present
locally; these files are untracked):

- `DEVKIT_PROGRESS_TEMP.md` -- working scratch from an earlier
  reorganization pass; superseded by
  `../01_PROGRESS/DEVKIT_PROGRESS.md` +
  `../01_PROGRESS/DEVKIT_PROGRESS_PHASE_10.md`.
- `DEVKIT_PROGRESS_before.txt` -- provenance snapshot of
  `DEVKIT_PROGRESS.md` from before the seed-split; preserved so the
  split can be audited.

## Rules

1. **Files here are untracked unless explicitly promoted.** Adding
   files to this directory does not put them under version control;
   `git add` is intentionally not the default. A fresh clone of the
   repository will not include them.
2. **Do not link to scratch files from canonical docs.** Canonical
   docs in `../01_PROGRESS/`, `../02_PHASE_CLOSEOUTS/`, and
   `../03_SPECS/` must not depend on the existence of any file in
   this directory. Treat anything here as best-effort local-only.
3. **Promotion is explicit.** If a scratch file becomes worth keeping
   as canonical truth, a dedicated phase must move it to the right
   classification subdir (`01_PROGRESS/` / `02_PHASE_CLOSEOUTS/` /
   `03_SPECS/`) under `git mv`, update all references, and document
   the promotion in `01_PROGRESS/DEVKIT_PROGRESS_PHASE_10.md`.
4. **Do not delete scratch files without explicit user authorization.**
   They are local-only and harmless; deletion can lose audit trail
   that the user may still want.

## What does NOT belong here

- Active spec / reference docs -> `../03_SPECS/`.
- Closed phase evidence / checkpoint docs -> `../02_PHASE_CLOSEOUTS/`.
- Phase history (master or running log) -> `../01_PROGRESS/`.
- Evidence logs (RTT / host transcripts) -> `../../logs/` (not here).
- Source code, scripts, tests, board configs -> none of these belong
  in `docs/` at all.

## Reading from this directory

If you find a reference to a file in this directory from a canonical
doc, treat that as a documentation bug: canonical docs should not
depend on local-only scratch. Open an issue / phase to either (a)
promote the scratch file to a canonical home, or (b) remove the
dangling reference.

## See also

- [`../00_INDEX/README.md`](../00_INDEX/README.md) -- top-level docs
  navigation hub.
- [`../01_PROGRESS/DEVKIT_PROGRESS.md`](../01_PROGRESS/DEVKIT_PROGRESS.md)
  -- historical phase master (canonical).
- [`../01_PROGRESS/DEVKIT_PROGRESS_PHASE_10.md`](../01_PROGRESS/DEVKIT_PROGRESS_PHASE_10.md)
  -- running Phase 10+ log (canonical).
