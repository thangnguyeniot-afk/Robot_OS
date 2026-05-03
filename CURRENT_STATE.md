# RobotOS — Current State Snapshot

> **Purpose:** Canonical startup snapshot for agents (Claude, Copilot, GPT, GLM).
> Detailed phase history remains in `RobotOS_v1.0/devkit/docs/DEVKIT_PROGRESS.md`.
> Update this file only at phase close, from validated evidence only.
> Do not record assumptions as fact. If unknown, mark **Pending / Unknown**.

---

## Last Closed Phase

### Phase 6F — Devkit Mixed Event Policy Smoke

- **Commit:** `5bca62f`
- **Date:** 2026-05-03
- **Branch:** master
- **Remote:** `https://github.com/thangnguyeniot-afk/Robot_OS` (confirmed linked)

---

## Validation Evidence (Phase 6F)

| Gate | Result | Detail |
|------|--------|--------|
| Host tests | PASS | 15/15 suites, 45 cases, 0 failures |
| Zephyr build | PASS | west build -b stm32f411e_disco |
| FLASH | 28716 B / 524288 B (5.48%) | stm32f411e_disco |
| RAM | 12096 B / 131072 B (9.23%) | stm32f411e_disco |
| RTT evidence | PASS | accepted=16 rejected=2 dropped=1 handled=16 herr=0 |
| CFSR | 0x00000000 | no configurable fault |
| HFSR | 0x00000000 | no hard fault |

Host test log: `RobotOS_v1.0/tests/host/logs/host_2026-05-03.log`
RTT log: `RobotOS_v1.0/devkit/logs/phase_6F_rtt_2026-05-03.txt`

---

## Behavior Guarantees from Phase 6F

- **No new status codes.** `ROBOTOS_CORE_ERR_THROTTLED = -7` remains the highest-numbered error.
- **No mutable state added.** No new counters in core or snapshot.
- **No auto-retry behavior.** Smoke only; all retry decisions remain producer-owned.
- **Core scheduler semantics unchanged.** Admission gate, queue policy, dispatch budget: all unchanged.
- **Platform boundary unchanged.**
- Existing `post_event` / `try_post_event` semantics unchanged.

---

## Files Changed in Phase 6F

| File | Change |
|------|--------|
| `devkit/src/devkit_runtime.c` | Replace Phase 6H k_timer smoke with Phase 6F mixed policy smoke; `<zephyr/kernel.h>` removed |
| `tests/host/test_robotos_mixed_event_policy_contract.c` | New -- 45 test cases (valid/invalid/full-queue + retry alignment) |
| `tests/host/CMakeLists.txt` | Add `robotos_mixed_event_policy_contract_test` target |
| `devkit/docs/DEVKIT_PROGRESS.md` | Phase 6F section |
| `devkit/logs/phase_6F_rtt_2026-05-03.txt` | Hardware RTT evidence |
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
| 6F | Devkit Mixed Event Policy Smoke | `5bca62f` |
| 6G | Timer producer smoke (ISR context) | `335ee29` |
| 6A–6E | Event pipeline smokes | preceding commits |
| 4K | Scheduler admission + throttle policy | preceding commits |
| 4L | Scheduler advisory retry decision policy | `a3f4369` |

---

## Next Candidate Phases

| Phase | Description | Status |
|-------|-------------|--------|
| Phase 6I | Timer Producer Queue-Pressure Stress | Candidate |

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
