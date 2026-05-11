# Phase 9Z — Workload-Branch Checkpoint Review

**Type:** Audit-only / checkpoint. No source, runtime, test, CMake, or Kconfig change.
**Date:** 2026-05-08
**Branch:** master
**HEAD at checkpoint:** `8e8c801` (Phase 9D commit)
**Tag:** `v0.9d-workload-baseline`

---

## Purpose

Phase 9Z closes the Phase 9 workload-branch stream with a structured audit
and direction checkpoint. All implementations from Phase 9A-A through Phase
9D have been delivered and validated with RTT hardware evidence. This
document records the architectural verdict, open watchpoints, deferred
items, and recommended next-direction options for the team before any
further phase work begins.

---

## A. Phases Delivered in This Stream

| Phase | Title | Commit | Close Status |
|-------|-------|--------|-------------|
| 9A-A | Devkit Button EXTI Producer | `2068180` | CLOSED with BOUNCE_OBSERVED |
| 9A-B | Devkit Button Debounce Refinement (30 ms time-guard) | `92de5e0` | CLOSED |
| 9A-C | Gate Phase 6I Startup Burst (compile-time default=0) | `3989ff9` | CLOSED |
| 9B | Devkit UART RX Producer (second hardware event source) | `85389f4` | CLOSED |
| 9C | Devkit Minimal Application State Machine (IDLE/ARMED/ACTIVE) | `286e61b` | CLOSED |
| 9D | Workload Demo Script & Runbook (tooling/docs only) | `8e8c801` | CLOSED |

All six phases are closed. No phase in this stream is pending, blocked, or in
partial state.

---

## B. Architecture Status — ON_TRACK_WITH_WATCHPOINTS

The 4-layer architecture (Application / Robot Framework / Robot Adapter /
Zephyr) is intact. The Phase 9 stream introduced devkit-layer code only.

**Core layer (`RobotOS_v1.0/core/`):** unchanged throughout all Phase 9 work.
Queue capacity (`ROBOTOS_EVENT_QUEUE_CAPACITY = 16`), dispatch budget
(`ROBOTOS_CORE_MAX_EVENTS_PER_TICK = 1`), handler-outside-lock invariant,
admission policy, and all status codes are identical to their Phase 5F/5G
baseline.

**Platform layer (`RobotOS_v1.0/platform/`):** unchanged.

**Tests layer (`RobotOS_v1.0/tests/`):** unchanged. Host contract tests written
in earlier phases still pass.

**Devkit layer (`RobotOS_v1.0/devkit/`):** extended with four new source
files and one new tool:

| File | Added by | Role |
|------|----------|------|
| `devkit/src/devkit_button_producer.c/.h` | Phase 9A-A / 9A-B | GPIO/EXTI button → event, with 30 ms debounce |
| `devkit/src/devkit_uart_producer.c/.h` | Phase 9B | UART RX IRQ → event (USER+3, one byte per event) |
| `devkit/src/devkit_app_state.c/.h` | Phase 9C | Minimal IDLE/ARMED/ACTIVE state machine |
| `tools/runtime/run_phase9d_demo.ps1` | Phase 9D | Repeatable demo orchestrator (PowerShell 5.1) |

All four source files are devkit-local. None cross the devkit/core or
devkit/platform boundary. None introduce scheduler logic, new queue
semantics, or event-type conflicts.

**Watchpoints (non-blocking; require team awareness):**

1. `ROBOTOS_CORE_MAX_EVENTS_PER_TICK = 1` remains a stub constant. Under
   combined button + UART + Phase 6M producer load, the Phase 9D stress
   run reached `peak=16` and `dropped=3`. The cause was operator-induced
   rapid button input, not steady-state workload pressure. The system
   handled the saturation without fault (`herr=0`, `unhandled=0`,
   CFSR/HFSR=0). No dispatch budget change is justified by this evidence,
   but the budget stub is a known architectural debt marker.

2. Phase 8A (custom STM32F407 board) remains HOLD/DEFER. The `devkit/`
   code has no F407-specific dependency today, but no F407 portability has
   been validated.

3. The `devkit_app_state` module is devkit-local and intentionally not
   promoted to Robot Framework. Future application phases (9E+) or a Robot
   Framework abstraction layer will need an explicit promotion decision.

---

## C. Firmware Baseline at Checkpoint

The active firmware is the Phase 9C build (`286e61b`), flashed and
validated by Phases 9C and 9D.

| Attribute | Value |
|-----------|-------|
| Target | STM32F411E-DISCO (Cortex-M4 @ 96 MHz, Zephyr 3.6.0) |
| FLASH usage | ≈ 35.7 KB |
| RAM usage | ≈ 12 KB |
| Logging backend | SEGGER RTT (canonical) |
| UART backend | USART2, PA3 RX, 115200 8N1 (IRQ-driven, no console) |
| Button | PA0 / sw0 (GPIO_ACTIVE_HIGH, 30 ms debounce) |
| Phase 6I gate | `DEVKIT_PHASE6I_STARTUP_BURST_ENABLED = 0` (default off) |
| Phase 6M producer | 1 event / 2 ticks (500 ms cadence) |
| Dispatch budget | `ROBOTOS_CORE_MAX_EVENTS_PER_TICK = 1` |
| Queue capacity | `ROBOTOS_EVENT_QUEUE_CAPACITY = 16` |

---

## D. RTT Evidence Summary

All Phase 9 sub-phases produced hardware-validated RTT evidence on the
STM32F411E-DISCO. Key captures:

| Phase | Log File | Result |
|-------|----------|--------|
| 9A-A | `phase_9A_button_rtt_2026-05-08.txt` | CLOSED with BOUNCE_OBSERVED — 37 button events, bounce present |
| 9A-B | `phase_9B_debounce_rtt_2026-05-08.txt` | PASS — attempted=94 ok=36 debounce=54 conservation ✓ |
| 9A-C | `phase_9C_no_burst_button_rtt_2026-05-08.txt` | PASS — phase6i_startup_burst=0 banner; burst absent; OBS invariants ✓ |
| 9B | `phase_9B_uart_rtt_2026-05-08.txt` | PASS — rx=7 ok=7 handled=7; per-byte match to `abc123\n` ✓ |
| 9C | `phase_9C_app_state_rtt_2026-05-08.txt` | PASS — transitions=23 button=20 uart=3 ignored=0; conservation ✓ |
| 9D | `phase_9D_workload_demo_2026-05-08.txt` | PASS (operator-timing deviation noted) — all 12 patterns FOUND; conservation ✓; CFSR=0/HFSR=0 throughout |

Phase 9D deviation documented: transitions=35/button=32/uart=3/ignored=0
instead of the canonical 6/3/3/0. Cause: operator pressed button during
boot-settle window and continued through the demo. Firmware behavior is
correct; the capture also demonstrated safe queue saturation handling
(`peak=16`, `dropped=3`, `herr=0`, `unhandled=0`).

---

## E. Scheduler Status — 7A / 7B-1 DEFER (unchanged)

Phase 9 workload evidence was examined against the scheduler-evolution
question:

- Phase 9C steady-state: `peak=4`, `dropped=0` — well within budget=1.
- Phase 9D stress segment: `peak=16`, `dropped=3` — operator-induced burst,
  not steady-state pressure. Even at full queue saturation, `herr=0`,
  `unhandled=0`, CFSR/HFSR=0. The system drained cleanly and reached the
  correct final state.

**Verdict:** No workload-driven justification for Phase 7A (budget evolution
planning) or Phase 7B-1 (dispatch budget parameterization). Both remain
DEFER. Re-examine if a future real workload produces sustained non-operator-
induced saturation.

---

## F. Phase 6I Gate Status — CORRECT, PRESERVED

- Default: `DEVKIT_PHASE6I_STARTUP_BURST_ENABLED = 0` (in `devkit_runtime.c`).
- Boot banner: `DEVKIT_DIAG phase6i_startup_burst=0` emitted at every boot.
- Opt-in: set `DEVKIT_PHASE6I_STARTUP_BURST_ENABLED=1` in CMake/prj.conf to
  re-enable the synthetic burst for diagnostic purposes.
- Phase 9A-C and Phase 9D evidence both confirm the gate works correctly and
  the Phase 6I burst is absent from production captures.

---

## G. UART Path Status

- UART RX producer (Phase 9B) is fully validated; one-byte-per-event model
  works correctly with Phase 5G ISR-safe `robotos_core_post_event()`.
- `CONFIG_SERIAL=y` and `CONFIG_UART_INTERRUPT_DRIVEN=y` are in `prj.conf`.
  RTT remains the canonical log backend; UART is pure event input.
- UART TX path does not exist. Phase 9D's `'?'` query is logged to RTT only.
  A TX path is Phase 9E scope (candidate, not approved).
- External USB-UART adapter (CP210x or equivalent) is required for any UART
  RX demo. The board has no on-board USB-VCP.

---

## H. Application State Machine Status

The Phase 9C `devkit_app_state` module is a minimal, devkit-local three-state
machine. It is intentionally **not** a Robot Framework abstraction.

| Attribute | Value |
|-----------|-------|
| States | IDLE, ARMED, ACTIVE |
| Button semantics | Cycles IDLE → ARMED → ACTIVE → IDLE |
| UART semantics | `a`/`A` → ARMED, `s`/`S` → ACTIVE, `r`/`R` → IDLE, `?` → query (no state change), others → ignored |
| Redundant-command handling | `ignored++`; no crash, no fault |
| Integration | Option A (producer handlers call app hooks directly) |
| Layer | Devkit only; no promotion to Robot Framework |

The module has no concurrency hazard: all hook calls arrive from the
dispatcher thread (handler-outside-lock invariant preserved from Phase 5F).

---

## I. Phase 8A (STM32F407) Status — HOLD/DEFER

Phase 8A (custom STM32F407 board bring-up) remains HOLD/DEFER. No F407-
specific work was done in this stream. The devkit code has no F407 dependency
today. Reopening Phase 8A requires an explicit user decision.

---

## J. Next-Direction Options

Three non-exclusive paths are available. All require explicit user approval
before implementation begins.

### Option 1 — Continue Phase 9 application evolution (Phase 9E+)
Add UART TX response (`'?'` echo or richer app feedback), additional app
states or commands, or mode-driven LED reactions. Requires adding a TX path
to `devkit_uart_producer` (currently RX-only). Low portability impact; still
devkit-local.

### Option 2 — Promote producer/handler pattern to Robot Framework
Lift the button/UART producer and application state patterns from devkit-local
to a Robot Framework abstraction layer. This is a boundary-crossing change
requiring an architecture decision. Phase 9 work provides the validated
implementation pattern for this promotion.

### Option 3 — Phase 8A — Custom STM32F407 bring-up
Flash the current Phase 9C firmware on the F407 target. This retires the
25-phase portability debt item and validates hardware portability. Requires
an F407 board, an appropriate OpenOCD config, and a Phase 6O harness
run with `-OpenOcdConfig <f407.cfg>`.

These options are not ordered by priority. The user and GPT should decide
before Phase 9E or any other implementation work starts.

---

## K. Risk and Watchpoint Register

| ID | Watchpoint | Severity | Status |
|----|-----------|----------|--------|
| W1 | `ROBOTOS_CORE_MAX_EVENTS_PER_TICK = 1` is a stub; not a final scheduler design | LOW — no workload-driven saturation observed in steady-state | MONITOR |
| W2 | `devkit_app_state` is devkit-local and not promoted to Robot Framework | LOW — this is intentional scope; promotion requires explicit decision | KNOWN |
| W3 | UART TX path absent; `'?'` query has no wire response | LOW — documented; Phase 9E scope | KNOWN |
| W4 | External USB-UART adapter required for UART demo; not in standard kit | LOW — documented in runbook §7 | KNOWN |
| W5 | Phase 8A (F407) portability unvalidated | MEDIUM — 25-phase debt; remains DEFER | DEFERRED |
| W6 | Phase 9D canonical-target deviation (35/32/3/0 vs 6/3/3/0) | LOW — operator-timing; fully explainable; firmware correct | DOCUMENTED |

---

## L. Architecture Verdict

**ON_TRACK_WITH_WATCHPOINTS.**

The Phase 9 workload stream delivered a validated, real-hardware event
pipeline with two independent hardware event sources (GPIO EXTI button and
UART RX IRQ) feeding a minimal application state machine, with repeatable
demo tooling and full RTT evidence. The core/platform boundary was not
breached. No scheduler change was introduced or required. All Phase 9 phases
closed with hardware evidence.

The system is ready for a product-direction decision (options J above) before
further implementation begins.

---

## M. Checkpoint Summary

| Item | Status |
|------|--------|
| Phase 9A-A through 9D | All CLOSED |
| Architecture boundary | INTACT — core/platform unchanged |
| Hardware evidence | All phases have RTT logs on STM32F411E-DISCO |
| Scheduler evolution | DEFER — no workload justification |
| Phase 6I gate | CORRECT — default=0, opt-in for diagnostics |
| UART TX path | ABSENT — Phase 9E candidate (not approved) |
| Phase 8A (F407) | HOLD/DEFER |
| Next step | Await user product-direction decision |
| Baseline tag | `v0.9d-workload-baseline` at `8e8c801` |

---

## Cross-references

- Phase history: [`DEVKIT_PROGRESS.md`](../01_PROGRESS/DEVKIT_PROGRESS.md) Phases 9A-A through 9D
- Workload demo runbook: [`WORKLOAD_DEMO_9D.md`](../03_SPECS/WORKLOAD_DEMO_9D.md)
- RTT log index: [`../logs/INDEX.md`](../../logs/INDEX.md)
- Live phase state: [`../../../CURRENT_STATE.md`](../../../../CURRENT_STATE.md)
- Telemetry reference: [`TELEMETRY_REFERENCE.md`](../03_SPECS/TELEMETRY_REFERENCE.md)
