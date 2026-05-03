# RobotOS — Current State Snapshot

> **Purpose:** Canonical startup snapshot for agents (Claude, Copilot, GPT, GLM).
> Detailed phase history remains in `RobotOS_v1.0/devkit/docs/DEVKIT_PROGRESS.md`.
> Update this file only at phase close, from validated evidence only.
> Do not record assumptions as fact. If unknown, mark **Pending / Unknown**.

---

## Last Closed Phase

### Phase 6I — Timer Producer Queue-Pressure Stress

- **Commit:** `e78e503`
- **Date:** 2026-05-03
- **Branch:** master
- **Remote:** `https://github.com/thangnguyeniot-afk/Robot_OS` (confirmed linked)

---

## Validation Evidence (Phase 6I)

| Gate | Result | Detail |
|------|--------|--------|
| Host tests | PASS | 16/16 suites, 42 new cases, 0 failures |
| Zephyr build | PASS | west build -b stm32f411e_disco |
| FLASH | 28820 B / 524288 B (5.50%) | stm32f411e_disco |
| RAM | 12160 B / 131072 B (9.28%) | stm32f411e_disco |
| RTT pressure | PASS | attempted=24 ok=18 full=6 invalid=0 other=0 handled=16 |
| RTT acceptance | PASS | accepted=18 dropped=6 dispatched=16 herr=0 |
| CFSR | 0x00000000 | no configurable fault |
| HFSR | 0x00000000 | no hard fault |

Host test log: `RobotOS_v1.0/tests/host/logs/host_2026-05-03.log`
RTT log: `RobotOS_v1.0/devkit/logs/phase_6I_rtt_2026-05-03.txt`

**Note on ok/full split:** The ok=18 / full=6 split is timing-dependent. During the 50ms producer burst, a tick freed 2 queue slots, allowing 2 additional events to post successfully. This is expected behavior and was settled cleanly in subsequent ticks.

---

## Behavior Guarantees from Phase 6I

- **No new status codes.** `ROBOTOS_CORE_ERR_THROTTLED = -7` remains the highest-numbered error.
- **No mutable state added.** No new counters in core or snapshot.
- **No auto-retry behavior.** Stress test only; all retry decisions remain producer-owned.
- **Core scheduler semantics unchanged.** Admission gate, queue policy, dispatch budget: all unchanged.
- **Platform boundary unchanged.**
- Existing `post_event` / `try_post_event` semantics unchanged.

---

## Files Changed in Phase 6I

| File | Change |
|------|--------|
| `tests/host/test_robotos_queue_pressure_contract.c` | New -- 42 test cases (50ms producer burst, queue fill + throttle + ok/full split) |
| `tests/host/CMakeLists.txt` | Add `robotos_queue_pressure_contract_test` target |
| `devkit/src/devkit_runtime.c` | Add Phase 6I 50ms producer burst smoke with counter inspection |
| `devkit/docs/DEVKIT_PROGRESS.md` | Phase 6I section |
| `devkit/logs/phase_6I_rtt_2026-05-03.txt` | Hardware RTT evidence |
| `tests/host/logs/host_2026-05-03.log` | Host test log |

---

## Phase History Summary (completed, confirmed from DEVKIT_PROGRESS.md)

The following phases are closed and committed. This list is not exhaustive — refer
to `DEVKIT_PROGRESS.md` for the full history.

| Phase | Description | Commit |
|-------|-------------|--------|
| 6H | ISR/Timer Producer Stress-Lite | `22edfee` |
| 5F/5F-R | Dispatch split + runtime confirm | `b7f08fe`, `460b9f1` |
| 5E | Critical boundary applied to core queue | `ff24147` |
| 5D | Platform critical section boundary | `4187bb3` |
| 6I | Timer Producer Queue-Pressure Stress | `e78e503` |
| 6F | Devkit Mixed Event Policy Smoke | `5bca62f` |
| 6G | Timer producer smoke (ISR context) | `335ee29` |
| 6A–6E | Event pipeline smokes | preceding commits |
| 4K | Scheduler admission + throttle policy | preceding commits |
| 4L | Scheduler advisory retry decision policy | `a3f4369` |

---

## Next Candidate Phases

| Phase | Description | Status |
|-------|-------------|--------|
| Phase 6J | TBD by team | Candidate |

---

## GLM Token Optimization Note

GLM token optimization is intentionally excluded until Level 15-17 stabilization is closed. GLM operates under its own policy and is not subject to the delta-command optimizations being applied to Claude Pro and Copilot.

---

## Maintenance Rule

- Update this file **only at phase close**, from validated evidence only.
- Do not record assumptions as fact.
- If a fact is unknown, mark it **Pending / Unknown**.
- Detailed history (logs, diffs, RTT evidence) remains in `DEVKIT_PROGRESS.md`.
- This file is the lightweight startup signal; `DEVKIT_PROGRESS.md` is the audit trail.
