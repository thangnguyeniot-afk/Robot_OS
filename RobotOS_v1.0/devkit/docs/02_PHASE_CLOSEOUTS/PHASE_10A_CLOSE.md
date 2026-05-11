# Phase 10A — Product Command Set Planning (Close)

**Type:** Docs-only planning closeout.
**Status:** `CLOSED_DOCS_ONLY`
**Date opened/closed:** 2026-05-11
**Branch:** master
**Prior runtime checkpoint:** Phase 9E-Z (`30bae27`). Runtime baseline ends
at Phase 9E (`587dab7`).
**Companion docs:**
[`DEVKIT_PROGRESS_PHASE_10.md`](../01_PROGRESS/DEVKIT_PROGRESS_PHASE_10.md) §5 (`<a id="phase-10a"></a>`),
[`COMMAND_SET_DRAFT.md`](../03_SPECS/COMMAND_SET_DRAFT.md).

---

## A. Summary

Phase 10A is a **docs-only planning phase**. It captures product command
vocabulary and workload intent in writing before any Phase 10B-class
implementation is authorized. Phase 10A does not add, change, or imply any
runtime behavior.

Phase 10A produces three planning artifacts:

- `DEVKIT_PROGRESS_PHASE_10.md` §5 — the Phase 10A entry, including the
  command-set planning table.
- `COMMAND_SET_DRAFT.md` — the DRAFT command vocabulary with explicit
  `USER_DECISION_REQUIRED` annotations.
- This closeout document.

No source, runtime, test, CMake, Zephyr, board, or script change occurred.

---

## B. Selected / draft command vocabulary

**Section A — already hardware-proven (reference only):** `a`, `s`, `?`,
`r`, `x` (Phase 9E, commit `587dab7`).

**Section B — DRAFT (`USER_DECISION_REQUIRED`, none approved):**

| Byte | Human meaning | Phase 10B candidate? | New driver/config? |
|---|---|---|---|
| `d` (0x64) | Explicit disarm (`ARMED → IDLE`) | Yes — smallest behavioral surface | No |
| `v` (0x76) | Build / firmware identification query | Yes — smallest implementation surface | No |
| `L` (0x4c) | Onboard LED toggle (single physical effect) | Yes — first physical-effect candidate | Conditional (reuse `devkit_status_led.c`) |
| `T` (0x54) | Onboard sensor read placeholder | Yes — largest surface (driver dependency) | Yes |

**Section C — explicitly rejected at Phase 10A:** multi-byte tokens,
arguments, ACK/sequence protocols, streaming UART telemetry, command
registry, responses larger than 96 bytes, commands that touch `core/` or
`platform/`, ISR-context TX, authentication, fault-injection.

See [`COMMAND_SET_DRAFT.md`](../03_SPECS/COMMAND_SET_DRAFT.md) for the full rationale.

---

## C. Unresolved user decisions

The following remain `USER_DECISION_REQUIRED` after Phase 10A. None are
resolved by this closeout. They map directly to
`PHASE_9EZ_CHECKPOINT.md §L Q1–Q5`.

1. **Product application intent.** What should RobotOS actually do in the
   intended use case?
2. **First real physical-effect command, if any.** Section B's `L` and `T`
   rows are the smallest candidates; neither is approved.
3. **Disarm command introduction.** Whether to introduce `d` given `r`
   already handles `* → IDLE`.
4. **Query-extension policy.** Whether to add `v` (build info) alongside
   `?`, and what fields to expose.
5. **Robot Framework promotion.** Whether `devkit_app_state` should be
   promoted from devkit-local to `core/`-adjacent. Not promoted by Phase
   10A.
6. **Phase 8A (F407) priority.** Not reopened by Phase 10A; remains
   HOLD/DEFER.
7. **Demo-tooling polish (Phase 9F) ordering.** Whether to land
   `run_phase9e_uart_response_demo.ps1` PASS/FAIL polish (or Phase 9G UART
   burst characterization) before any Phase 10B implementation.

---

## D. Implementation candidates that may become Phase 10B (non-binding order)

Each candidate, when opened, must follow Phase 9E-style validation
discipline (pristine build, RTT capture via `capture_devkit_rtt.ps1`, host
transcript, boundary-preservation audit, scope-guard restated, dedicated
closeout document).

1. **Phase 10B-`v`** — build query. Smallest surface; no driver dependency.
2. **Phase 10B-`d`** — explicit disarm. Smallest behavioral surface.
3. **Phase 10B-`L`** — LED toggle. First physical-effect command; requires
   reconciliation with existing blink semantics.
4. **Phase 10B-`T`** — sensor read. Largest surface; new driver dependency.

Direction-independent alternatives the user may prefer first:

- **Phase 9F** — demo script PASS/FAIL polish (tooling only).
- **Phase 9G** — bounded UART burst characterization (evidence only).

Neither consumes the Phase 10 opening.

---

## E. Architecture preservation (Phase 10A docs-only)

Confirmed unchanged at Phase 10A close:

- `core/` — zero diff
- `platform/` — zero diff
- `tests/` — zero diff
- `devkit/src/` — zero diff
- `devkit/CMakeLists.txt`, `devkit/prj.conf` — zero diff
- `tools/runtime/capture_devkit_rtt.ps1`,
  `tools/runtime/run_phase9e_uart_response_demo.ps1`,
  `tools/runtime/run_phase9d_demo.ps1` — zero diff
- Scheduler constants (`ROBOTOS_CORE_MAX_EVENTS_PER_TICK = 1`,
  `ROBOTOS_EVENT_QUEUE_CAPACITY = 16`) — unchanged
- Event-type contract — unchanged
- UART TX scope guard (12 constraints, `PHASE_9EZ_CHECKPOINT.md §H`) —
  intact
- POST_FLASH_AUTOSTART discipline — unchanged; root cause OPEN;
  mitigated-by-workflow from Phase 6O onward; manual RESET retained as
  fallback; plain `west flash` alone is not runtime-start evidence
- RTT canonical log backend — unchanged

`DEVKIT_PROGRESS.md` is not modified by Phase 10A; the historical master
remains the authoritative phase log through Phase 9E / 9E-Z.

---

## F. Explicit confirmations

- **Docs-only.** Phase 10A touched only `.md` files under
  `RobotOS_v1.0/devkit/docs/` and `CURRENT_STATE.md`.
- **Phase 10B not started.** No `devkit/src/` or `core/` or `platform/`
  file modified. No source/test/CMake/Zephyr/board/script/runtime file
  staged or committed by Phase 10A.
- **No runtime implementation introduced.** Hardware behavior is identical
  to the Phase 9E baseline.
- **Scheduler 7A / 7B-1 remains DEFER.**
- **F407 / custom board remains HOLD/DEFER.**
- **UART TX remains minimal response only.** All twelve scope-guard
  constraints intact.
- **POST_FLASH_AUTOSTART remains OPEN root cause + MITIGATED_BY_WORKFLOW
  from Phase 6O onward.**

---

## G. Next gate

Before any Phase 10B opening, the user must explicitly select:

- a row from Section B (with that row's `USER_DECISION_REQUIRED` notes
  answered); **or**
- a direction-independent supporting phase (Phase 9F demo polish or Phase 9G
  UART burst characterization); **or**
- a continued hold (no Phase 10B opening).

Phase 10A does not authorize any of these by itself.

---

## H. Cross-references

- Phase 9E-Z direction guard: [`PHASE_9EZ_CHECKPOINT.md`](PHASE_9EZ_CHECKPOINT.md)
- Phase history (Phase 9E and earlier): [`DEVKIT_PROGRESS.md`](../01_PROGRESS/DEVKIT_PROGRESS.md)
- Phase 10+ log: [`DEVKIT_PROGRESS_PHASE_10.md`](../01_PROGRESS/DEVKIT_PROGRESS_PHASE_10.md)
- DRAFT command vocabulary: [`COMMAND_SET_DRAFT.md`](../03_SPECS/COMMAND_SET_DRAFT.md)
- Live state snapshot: [`../../../CURRENT_STATE.md`](../../../../CURRENT_STATE.md)
- Telemetry reference: [`TELEMETRY_REFERENCE.md`](../03_SPECS/TELEMETRY_REFERENCE.md)
- Runtime tools: [`../../tools/runtime/README.md`](../../tools/runtime/README.md)
