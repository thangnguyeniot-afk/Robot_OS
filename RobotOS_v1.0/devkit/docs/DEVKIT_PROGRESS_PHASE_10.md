# DEVKIT_PROGRESS_PHASE_10.md — RobotOS Devkit Phase Log, Phase 10+

---

## 1. Continuation Note

This file continues the RobotOS devkit phase log after `DEVKIT_PROGRESS.md`.

- `DEVKIT_PROGRESS.md` remains the **historical master** for Phase 1 through
  Phase 9E / Phase 9E-Z. It must not be rewritten, reordered, or deduplicated
  as part of this seed split.
- All **Phase 10 and later** phase entries (including any Phase 10A, 10B, 10Z,
  Phase 11+, etc.) must be added to this file (`DEVKIT_PROGRESS_PHASE_10.md`).
- This file is seeded as a documentation scaffold. Phase 10 itself has **not**
  been opened. No Phase 10 implementation work has started.

---

## 2. Current Baseline Before Phase 10

The following facts describe the confirmed baseline that exists **before** any
Phase 10 work begins. They are reproduced here only for orientation; the
authoritative live snapshot lives in `CURRENT_STATE.md` and the authoritative
phase history lives in `DEVKIT_PROGRESS.md`.

- Baseline is **closed at Phase 9E / Phase 9E-Z**.
- Phase 9E proved the first minimal host-commanded embedded runtime loop:
  host UART command → UART RX producer → core event queue → app state machine →
  UART TX minimal response → host received response.
- Phase 9E-Z is the post-9E checkpoint / direction guard.
- **Scheduler 7A/7B** remains `DEFER`. No scheduler admission change is
  approved as part of seeding this file.
- **STM32F407 / custom board** migration remains `HOLD/DEFER`. No board
  migration is approved as part of seeding this file.
- **UART TX** scope remains **minimal response only**. No shell, parser,
  protocol expansion, or product command-set implementation is approved as
  part of seeding this file.
- The next real decision gate is **product command set / workload intent
  direction**, not blind implementation. Phase 10 must not be opened until
  that direction decision is made.

---

## 3. Phase Index

Phase entries below use **manual anchors** (`<a id="..."></a>`) placed
immediately before each phase heading. Index entries link to those manual
anchors so navigation does not depend solely on GitHub auto-generated heading
anchors.

| Phase | Title | Status | Jump |
|-------|-------|--------|------|
| 10 | RESERVED — not yet opened | NOT_STARTED | [→](#phase-10) |
| 10Z | RESERVED — future checkpoint / closeout slot | NOT_STARTED | [→](#phase-10z) |

When future phases are added:

- Insert a new manual-anchor block (`<a id="phase-XY"></a>`) immediately
  before the new phase heading.
- Add a row to the table above linking to that manual anchor.
- Keep the index in insertion / session order, matching the convention used
  in `DEVKIT_PROGRESS.md`. Phase-number order is **not** required.

---

## 4. Split / Editing Rules

These rules govern how this file and `DEVKIT_PROGRESS.md` evolve together.

1. **History stays put.** Phase 1 through Phase 9E / Phase 9E-Z history
   remains in `DEVKIT_PROGRESS.md`. Do not move, rewrite, reorder, or
   deduplicate old phase sections as part of using this file.
2. **New phases land here.** Every Phase 10+ entry must be added to
   `DEVKIT_PROGRESS_PHASE_10.md`, not appended to `DEVKIT_PROGRESS.md`.
3. **Manual anchors are mandatory.** Each future phase section must place an
   explicit manual anchor (`<a id="phase-XY"></a>`) on the line immediately
   before its heading.
4. **Index links must target manual anchors.** Entries in the Phase Index
   table must link to the manual anchor id, not to a heading slug.
5. **Do not rely on GitHub auto-generated heading anchors.** Heading slugs
   change when titles are edited; manual anchors do not.
6. **Make non-linear edits explicit.** If a phase is duplicated, amended, or
   inserted out of phase-number order, mark the index entry to make the
   duplication or insertion obvious (for example, `†` for the first
   occurrence and `‑1` / `‑2` anchor suffixes for subsequent ones, mirroring
   the convention used in `DEVKIT_PROGRESS.md`).
7. **No silent baseline change.** Do not alter the "Current Baseline Before
   Phase 10" section to claim work has progressed beyond Phase 9E / 9E-Z
   unless that progression has been validated and recorded as a closed
   Phase 10+ entry below.

---

## 5. Phase Sections

<a id="phase-10"></a>
## Phase 10 — RESERVED / NOT STARTED

Status: NOT_STARTED

Placeholder only. Do not add implementation claims here until Phase 10 is
formally opened.

---

<a id="phase-10z"></a>
## Phase 10Z — RESERVED / NOT STARTED

Status: NOT_STARTED

Placeholder only. Use this only for a future Phase 10 checkpoint/closeout if
approved.
