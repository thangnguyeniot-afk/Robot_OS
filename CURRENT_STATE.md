# RobotOS — Current State Snapshot

> **Purpose:** Canonical startup snapshot for agents (Claude, Copilot, GPT, GLM).
> Detailed phase history remains in `RobotOS_v1.0/devkit/docs/DEVKIT_PROGRESS.md`.
> Update this file only at phase close, from validated evidence only.
> Do not record assumptions as fact. If unknown, mark **Pending / Unknown**.

---

## Last Closed Phase

### Phase 9A-A — Devkit Button EXTI Producer

- **Commit:** (this commit)
- **Date:** 2026-05-08
- **Branch:** master
- **Type:** Devkit workload proof. First real (non-synthetic) hardware event source.
- **Close status:** `CLOSED with BOUNCE_OBSERVED`
- **Prior tooling baseline:** Phase 6O (`6cb979f`), reusable RTT streaming harness

**Phase 9A-A delivered:**

- `RobotOS_v1.0/devkit/src/devkit_button_producer.{h,c}` — devkit-local user-button GPIO/EXTI event producer
- `RobotOS_v1.0/devkit/src/devkit_runtime.c` — wired button producer init, baseline log, and periodic ROBOTOS_BTN log
- `RobotOS_v1.0/devkit/CMakeLists.txt` — added new source file
- `RobotOS_v1.0/devkit/logs/phase_9A_button_rtt_2026-05-08.txt` — 60 s RTT capture with real button presses (37 events handled, 135 ISR firings due to mechanical bounce, CFSR=0 HFSR=0)
- `RobotOS_v1.0/devkit/logs/INDEX.md` — Phase 9A-A row
- `RobotOS_v1.0/devkit/docs/DEVKIT_PROGRESS.md` — Phase 9A-A section (purpose, architecture boundary, bounce analysis, preservation audit)

**Workload anchor:** RobotOS is now confirmed as an event-driven embedded device runtime serving a real hardware input source. Architecture invariants (admission, FIFO, queue capacity, ISR-safety, handler-outside-lock, no fault) all preserved under bounce-heavy concurrent producer load.

**Phase 6O** (prior): Reusable RTT Streaming Capture Harness, commit `6cb979f`, 2026-05-08.
**Phase 6N** (prior): Documentation / Navigation Consolidation, commit `ad52de5`, 2026-05-08.

---

## Validation Evidence (Phase 9A-A button workload)

| Gate | Result | Detail |
| ---- | ------ | ------ |
| Zephyr build | PASS | FLASH 31208 B (5.95%) / RAM 12224 B (9.33%); +1176 B FLASH and +64 B RAM vs Phase 6Z |
| Flash | PASS | `west flash` wrote 32768 B; manual RESET not required |
| RTT capture | PASS | 60.8 s, 21961 bytes captured; ROBOTOS_BTN visible; per-press handler logs visible |
| ROBOTOS_OBS state=READY | FOUND | Baseline + 12 periodic emissions |
| ROBOTOS_FAULT active=0 | FOUND | All 13 emissions |
| ROBOTOS_PROD attempted= | FOUND | Phase 6M producer continues healthy at 60 attempted/60 ok |
| ROBOTOS_BTN attempted= | FOUND | 13 emissions; final attempted=135 ok=37 full=98 handled=37 |
| Phase 9A-A button producer init | FOUND | Init banner present in boot sequence |
| Phase 9A button handled (per-press) | FOUND | 43 occurrences across 60 s |
| Phase 6I final: | MISSING | Bounce displaced some Phase 6I events from queue → handled count never reached 16 → final summary not emitted. Documented as BOUNCE_OBSERVED, not a regression. See DEVKIT_PROGRESS.md Phase 9A-A. |
| CFSR | 0x00000000 throughout | 13 occurrences checked |
| HFSR | 0x00000000 throughout | 13 occurrences checked |
| Architecture invariants | PASS | accepted=112 dispatched=111 pending=1 (accepted-dispatched=pending) ; peak=16=capacity ; herr=0 ; unhandled=0 |
| Core/platform sources changed | ZERO | git diff confirmed no `core/`, `platform/`, or `tests/` files touched |

Capture log: `RobotOS_v1.0/devkit/logs/phase_9A_button_rtt_2026-05-08.txt`

**Phase 6Z verification highlights:**

- Phase 6K: ROBOTOS_OBS baseline + 12 periodic emissions; `peak=16` reached during Phase 6I burst, then steady-state `pending=1` after ticks=40.
- Phase 6L: ROBOTOS_FAULT baseline + 12 periodic emissions; CFSR/HFSR zero throughout; `active=0`, `context=none` invariant.
- Phase 6M: ROBOTOS_PROD baseline + 12 periodic emissions; producer init banner present; `attempted` grew exactly +5 per 10 ticks (60 attempts total over 60 s); `ok=attempted`, `dropped=0` for the entire capture.
- Phase 6I final summary: `attempted=24 ok=17 full=7 handled=16` (one-event timing variance vs documented `ok=16 full=8`; architectural invariants preserved — see Phase 6Z section in `DEVKIT_PROGRESS.md`).
- Counter cross-check at ticks=120: Phase 6I `ok` (17) + Phase 6M `ok` (60) = `accepted` (77); Phase 6I handled (16) + Phase 6M handled (60) = `dispatched` (76); `accepted - dispatched = pending = 1`.

---

## Behavior Guarantees Through Phase 6Z

Phase 6Z is evidence/state-reconciliation only and changed no source.
The following invariants from Phase 6J carry forward unchanged through
Phase 6K / 6L / 6M and are confirmed by the Phase 6Z RTT capture:

- **No new status codes.** `ROBOTOS_CORE_ERR_THROTTLED = -7` remains the highest-numbered error.
- **No scheduler semantics changed.** `ROBOTOS_CORE_MAX_EVENTS_PER_TICK = 1` unchanged.
- **No auto-retry behavior.** Retry policy remains pure advisory stateless mapping.
- **No dispatch lock-boundary regression.** Handler-outside-lock invariant confirmed preserved.
- **No admission policy change.** NONE/reserved rejected; USER+ accepted: unchanged.
- **`peak_queue_depth` is passive.** Updated only after successful push, under existing lock. Monotonically non-decreasing. Not reset by repeated init. Zero behavioral impact. Phase 6Z RTT confirmed `peak=16 = ROBOTOS_EVENT_QUEUE_CAPACITY` reached during the Phase 6I burst.
- Existing `post_event` / `try_post_event` semantics unchanged.
- **Phase 6K/6L/6M observability surfaces are passive.** ROBOTOS_OBS, ROBOTOS_FAULT, and ROBOTOS_PROD logs do not feed into scheduling, admission, throttle, dispatch, or retry decisions. Verified by Phase 6Z behavior coherence check.
- **Phase 6M producer cadence is fixed at compile time.** 1 post / 2 ticks at `DEVKIT_TICK_MS=500`; producer never reads core counters. Verified by Phase 6Z `attempted` growth of exactly +5 per 10 ticks.

---

## Files Changed in Phase 6Z

| File | Change |
| ---- | ------ |
| `RobotOS_v1.0/devkit/logs/phase_6Z_rtt_2026-05-07.txt` | New -- raw RTT capture, 60 s, 16,560 bytes |
| `RobotOS_v1.0/devkit/docs/DEVKIT_PROGRESS.md` | Phase 6K/6L/6M close-status flips; Phase 6Z section appended |
| `CURRENT_STATE.md` | Last-closed phase advanced to Phase 6M (closed via Phase 6Z) |

No source code, tests, CMake, Kconfig, prj.conf, or runtime behavior
modified in Phase 6Z. See `DEVKIT_PROGRESS.md` Phase 6Z Architecture
Preservation Audit for the explicit list.

For prior-phase file history (e.g. Phase 6J/6K/6L/6M originating
commits), refer to `DEVKIT_PROGRESS.md`.

---

## Phase History Summary (completed, confirmed from DEVKIT_PROGRESS.md)

The following phases are closed and committed. This list is not exhaustive — refer
to `DEVKIT_PROGRESS.md` for the full history.

| Phase | Description | Commit |
| ----- | ----------- | ------ |
| 9A-A | Devkit Button EXTI Producer (first real hardware workload) | (this commit) |
| 6O | Reusable RTT Streaming Capture Harness (tooling) | `6cb979f` |
| 6N | Documentation / Navigation Consolidation (docs-only) | `ad52de5` |
| 6Z | RTT closeout for 6K/6L/6M (docs/evidence only) | `4ec5b86` |
| 6M | Producer Realism / Timer Producer Diagnostic | `a6b253b` |
| 6L | Fault Observability Integration | `d3759a7` |
| 6K | Runtime Observability Surfacing | `11516d4` |
| 6J | Observability and Contract Stress Expansion | `8a1af69` |
| 6I | Timer Producer Queue-Pressure Stress | `e78e503` |
| 6H | ISR/Timer Producer Stress-Lite | `22edfee` |
| 6G | Timer producer smoke (ISR context) | `335ee29` |
| 6F | Devkit Mixed Event Policy Smoke | `5bca62f` |
| 6A–6E | Event pipeline smokes | preceding commits |
| 5F/5F-R | Dispatch split + runtime confirm | `b7f08fe`, `460b9f1` |
| 5E | Critical boundary applied to core queue | `ff24147` |
| 5D | Platform critical section boundary | `4187bb3` |
| 4K | Scheduler admission + throttle policy | preceding commits |
| 4L | Scheduler advisory retry decision policy | `a3f4369` |

---

## Next Candidate Phases

| Phase | Description | Status |
| ----- | ----------- | ------ |
| Phase 9A-B | Devkit button debounce refinement (devkit-local time-guard, no core change) | **Optional** — only if bounce traffic is operationally noisy |
| Phase 8A | Custom STM32F407 board bring-up | **Candidate** — retires 25-phase portability debt; use `capture_devkit_rtt.ps1 -OpenOcdConfig <f407.cfg>`; remains HOLD/DEFER until user reopens |
| Phase 9B | Second real event source (UART or sensor) | Candidate — depends on application direction |
| Phase 7B-1 | Dispatch Budget Test Parameterization | Candidate — only if Phase 9A-A workload evidence reveals saturation; current data shows budget=1 still adequate |
| Phase 7A | Dispatch Budget Evolution Planning | DEFER — Phase 9A-A workload data shows ~112 events / 60 s sustained, no workload-driven reason for budget mutation |
| Custom STM32F407 target | Migration / validation | HOLD/DEFER — not yet exercised; Phase 8A addresses when reopened |

**Primary workload anchor:** event-driven embedded device runtime, validated with the Phase 9A-A user-button input source. Future workloads can be layered using the same producer/handler pattern (see `devkit_button_producer.c` as the reference template).

Dispatch budget remains `ROBOTOS_CORE_MAX_EVENTS_PER_TICK = 1`. Any
mutation requires explicit team decision and a workload-driven
justification beyond what Phase 6Z evidence currently supports.

---

## GLM Token Optimization Note

GLM token optimization is intentionally excluded until Level 15-17 stabilization is closed. GLM operates under its own policy and is not subject to the delta-command optimizations being applied to Claude Pro and Copilot.

---

## Operating Notes

**Token/context baseline (2026-05-03):** Closed for now; monitor only. Future agents should use `CURRENT_STATE.md` as lightweight startup signal and `PHASE_CLOSE_TEMPLATE.md` as closeout structure to reduce repeated context reconstruction.

**Instruction discipline update:** Copilot/Claude instruction update committed at `6227e17` — *do not reduce validation depth for token savings*. Keep correctness checks even when optimizing context overhead. Caveat: commit `6227e17` included 131 pre-existing lines of GLM Evidence Logging content alongside new material; no revert planned.

**Validation preservation:** Never skip build/test/hardware validation steps to save tokens. Token optimization applies to repo documentation state-tracking, not to correctness gates.

**GLM status:** Excluded from token-shortening until Level 15–17 stabilization closes. GLM operates under separate policy constraints.

---

## Maintenance Rule

- Update this file **only at phase close**, from validated evidence only.
- Do not record assumptions as fact.
- If a fact is unknown, mark it **Pending / Unknown**.
- Detailed history (logs, diffs, RTT evidence) remains in `DEVKIT_PROGRESS.md`.
- This file is the lightweight startup signal; `DEVKIT_PROGRESS.md` is the audit trail.
