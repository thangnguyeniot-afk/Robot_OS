# RobotOS — Current State Snapshot

> **Purpose:** Canonical startup snapshot for agents (Claude, Copilot, GPT, GLM).
> Detailed phase history remains in `RobotOS_v1.0/devkit/docs/DEVKIT_PROGRESS.md`.
> Update this file only at phase close, from validated evidence only.
> Do not record assumptions as fact. If unknown, mark **Pending / Unknown**.

---

## Last Closed Phase

### Phase 9B — Devkit UART RX Producer

- **Commit:** `85389f4`
- **Date:** 2026-05-08
- **Branch:** master
- **Type:** Devkit workload expansion. Second real hardware event source (UART RX); devkit-local; no core/platform/scheduler change.
- **Close status:** `CLOSED`
- **Prior baseline:** Phase 9A-C (`3989ff9`), Phase 6I startup burst gated off

**Phase 9B delivered:**

- `RobotOS_v1.0/devkit/src/devkit_uart_producer.{h,c}` — devkit-local UART RX event producer using Zephyr `uart_irq_callback_set` + `uart_fifo_read`; one byte → one event (USER+3, marker `0x9B0B`).
- `RobotOS_v1.0/devkit/src/devkit_runtime.c` — wired UART producer init, baseline log, and periodic `ROBOTOS_UART` log into the existing observability cadence.
- `RobotOS_v1.0/devkit/CMakeLists.txt` — added `src/devkit_uart_producer.c`.
- `RobotOS_v1.0/devkit/prj.conf` — enabled `CONFIG_SERIAL=y` and `CONFIG_UART_INTERRUPT_DRIVEN=y`; `CONFIG_UART_CONSOLE=n` and `CONFIG_LOG_BACKEND_UART=n` preserved (RTT remains canonical log backend).
- `RobotOS_v1.0/devkit/docs/TELEMETRY_REFERENCE.md` — added ROBOTOS_UART section; updated Telemetry Stack Summary.
- `RobotOS_v1.0/devkit/logs/phase_9B_uart_rtt_2026-05-08.txt` — 60 s RTT capture with `abc123\n` (7 bytes) sent via CP210x USB-UART → PA3 @ 115200; rx=7 ok=7 full=0 handled=7; per-byte handler logs match payload.
- `RobotOS_v1.0/devkit/logs/INDEX.md` — Phase 9B row.
- `RobotOS_v1.0/devkit/docs/DEVKIT_PROGRESS.md` — Phase 9B section.

**Workload anchor expanded:** RobotOS now ingests **two distinct real hardware event sources** — GPIO/EXTI (Phase 9A user button, USER+2) and UART RX (Phase 9B serial bytes, USER+3). Both coexist with Phase 6M timer producer (USER+1). Architecture invariants preserved under combined producer load.

**Phase 9A-C** (prior): Gate Phase 6I startup burst, commit `3989ff9`.
**Phase 9A-B** (prior): Button debounce refinement, commit `92de5e0`.
**Phase 9A-A** (prior): Button EXTI producer, commit `2068180`, CLOSED with BOUNCE_OBSERVED.
**Phase 6O** (prior): Reusable RTT Streaming Capture Harness, commit `6cb979f`.

---

## Validation Evidence (Phase 9B UART workload)

| Gate | Result | Detail |
| ---- | ------ | ------ |
| Zephyr build | PASS | FLASH 34448 B (6.57%) / RAM 12224 B (9.33%); +3868 B FLASH and +64 B RAM vs Phase 9A-C (Zephyr serial + STM32 UART driver newly enabled) |
| Flash | PASS | `west flash` wrote 49152 B |
| UART send | PASS | `abc123\n` (7 bytes) written to COM5 (CP210x USB-UART) → board PA3 at 115200 8N1, 12 s into capture |
| RTT capture | PASS | 60.4 s, 20627 bytes; harness exit 0 with Phase 9B-specific patterns |
| Phase 9B uart producer init | FOUND | Init banner shows `type=USER+3 marker=0x9b0b dev=serial@40004400` |
| ROBOTOS_OBS state=READY | FOUND | Baseline + 12 periodic emissions |
| ROBOTOS_FAULT active=0 | FOUND | All 13 emissions; CFSR=0 HFSR=0 throughout (13 occurrences) |
| ROBOTOS_PROD attempted= | FOUND | Phase 6M producer healthy: ticks=120 → attempted=60 ok=60 dropped=0 |
| ROBOTOS_BTN | FOUND | Button producer initialized; idle (no presses): attempted=0 ok=0 |
| ROBOTOS_UART | FOUND | Final at ticks=120: rx=7 ok=7 full=0 invalid=0 other=0 handled=7 last=0x0a |
| `DEVKIT_DIAG phase6i_startup_burst=0` | FOUND | Phase 9A-C gate still in effect; no Phase 6I interference |
| Phase 9B uart handled (per-byte) | FOUND | 7 occurrences in order: 0x61 (`a`), 0x62 (`b`), 0x63 (`c`), 0x31 (`1`), 0x32 (`2`), 0x33 (`3`), 0x0a (LF) — exact match to sent payload |
| CFSR | 0x00000000 throughout | 13 occurrences checked |
| HFSR | 0x00000000 throughout | 13 occurrences checked |
| UART conservation | PASS | ok(7)+full(0)+invalid(0)+other(0) = rx(7) ✓; ok = handled = 7 ✓ |
| Architecture invariants | PASS | accepted=67 dispatched=66 pending=1 (accepted−dispatched=pending ✓); peak=8 (queue absorbed the 7-byte burst); dropped=0; herr=0; unhandled=0; accepted = Phase 6M(60)+UART(7) = 67 ✓ |
| Core/platform sources changed | ZERO | No `core/`, `platform/`, or `tests/` files touched; no Zephyr include in core |

Capture log: `RobotOS_v1.0/devkit/logs/phase_9B_uart_rtt_2026-05-08.txt`

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
| 9B | Devkit UART RX Producer (second real hardware event source; USER+3; rx=7 ok=7 full=0 handled=7) | `85389f4` |
| 9A-C | Gate Phase 6I Startup Burst (devkit-local compile-time gate; default disabled; full 4→0, dropped 13→0) | `3989ff9` |
| 9A-B | Devkit Button Debounce Refinement (30 ms time-guard; full 98→4) | `92de5e0` |
| 9A-A | Devkit Button EXTI Producer (first real hardware workload) | `2068180` |
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
| Phase 9A-B | Devkit button debounce refinement (30 ms time-guard, no core change) | **CLOSED** — full 98→4; debounce=54; see Phase 9A-B section in DEVKIT_PROGRESS.md |
| Phase 9A-C | Gate Phase 6I startup burst (devkit-local compile-time gate; default disabled) | **CLOSED** — full 4→0; dropped 13→0; peak=14; see Phase 9A-C section in DEVKIT_PROGRESS.md |
| Phase 9B | UART RX producer (second real hardware event source; USER+3) | **CLOSED** — rx=7 ok=7 full=0 handled=7; per-byte handler logs match payload `abc123\n`; see Phase 9B section in DEVKIT_PROGRESS.md |
| Phase 9C | Minimal application state machine (compose button + UART into agent demo) | Candidate — two real input sources now available |
| Phase 9D | Workload mode cleanup / second-board sanity | Candidate — only if multi-source workload reveals friction |
| Phase 8A | Custom STM32F407 board bring-up | **Candidate** — retires 25-phase portability debt; use `capture_devkit_rtt.ps1 -OpenOcdConfig <f407.cfg>`; remains HOLD/DEFER until user reopens |
| Phase 7B-1 | Dispatch Budget Test Parameterization | Candidate — only if workload evidence reveals saturation; Phase 9A-C clean baseline shows budget=1 still adequate (peak=14, dropped=0) |
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
