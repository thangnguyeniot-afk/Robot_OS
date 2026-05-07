# RobotOS — Current State Snapshot

> **Purpose:** Canonical startup snapshot for agents (Claude, Copilot, GPT, GLM).
> Detailed phase history remains in `RobotOS_v1.0/devkit/docs/DEVKIT_PROGRESS.md`.
> Update this file only at phase close, from validated evidence only.
> Do not record assumptions as fact. If unknown, mark **Pending / Unknown**.

---

## Last Closed Phase

### Phase 6J — Observability and Contract Stress Expansion

- **Commit:** TBD (pending commit)
- **Date:** 2026-05-07
- **Branch:** master
- **Remote:** `https://github.com/thangnguyeniot-afk/Robot_OS` (confirmed linked)

---

## Validation Evidence (Phase 6J)

| Gate | Result | Detail |
| ---- | ------ | ------ |
| Host tests | PASS | 19/19 suites, 299 new cases, 0 failures |
| Zephyr build | PASS | west build -b stm32f411e_disco |
| FLASH | 28852 B / 524288 B (5.50%) | stm32f411e_disco (+32 B from Phase 6I) |
| RAM | 12160 B / 131072 B (9.28%) | stm32f411e_disco (unchanged) |
| RTT smoke | N/A | Phase 6J is host-test-only per approved scope |
| CFSR | not checked | No devkit smoke in Phase 6J |
| HFSR | not checked | No devkit smoke in Phase 6J |

Host test log: `RobotOS_v1.0/tests/host/logs/host_2026-05-07.log`

**New Phase 6J suite results:**

- Phase 6J-A (routing stress): 55 passed, 0 failed
- Phase 6J-B (lifecycle): 52 passed, 0 failed
- Phase 6J-C/D (snapshot + peak): 192 passed, 0 failed

---

## Behavior Guarantees from Phase 6J

- **No new status codes.** `ROBOTOS_CORE_ERR_THROTTLED = -7` remains the highest-numbered error.
- **No scheduler semantics changed.** `ROBOTOS_CORE_MAX_EVENTS_PER_TICK = 1` unchanged.
- **No auto-retry behavior.** Retry policy remains pure advisory stateless mapping.
- **No dispatch lock-boundary regression.** Handler-outside-lock invariant confirmed preserved.
- **No admission policy change.** NONE/reserved rejected; USER+ accepted: unchanged.
- **`peak_queue_depth` is passive.** Updated only after successful push, under existing lock. Monotonically non-decreasing. Not reset by repeated init. Zero behavioral impact.
- Existing `post_event` / `try_post_event` semantics unchanged.

---

## Files Changed in Phase 6J

| File | Change |
| ---- | ------ |
| `core/robotos_core.h` | Add `peak_queue_depth` to snapshot struct; add `robotos_core_peak_queue_depth()` getter |
| `core/robotos_core.c` | Add `s_peak_queue_depth` static + peak update in `post_event_internal` + getter |
| `tests/host/test_robotos_handler_routing_stress_contract.c` | New -- 55 test cases (6J-A) |
| `tests/host/test_robotos_handler_lifecycle_contract.c` | New -- 52 test cases (6J-B) |
| `tests/host/test_robotos_snapshot_coherence_contract.c` | New -- 192 test cases (6J-C/D) |
| `tests/host/CMakeLists.txt` | Add 3 new test targets (19 total) |
| `devkit/docs/DEVKIT_PROGRESS.md` | Phase 6J section |
| `tests/host/logs/host_2026-05-07.log` | Host test log |

---

## Phase History Summary (completed, confirmed from DEVKIT_PROGRESS.md)

The following phases are closed and committed. This list is not exhaustive — refer
to `DEVKIT_PROGRESS.md` for the full history.

| Phase | Description | Commit |
| ----- | ----------- | ------ |
| 6J | Observability and Contract Stress Expansion | TBD |
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
| Phase 6K | TBD by team | Candidate |

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
