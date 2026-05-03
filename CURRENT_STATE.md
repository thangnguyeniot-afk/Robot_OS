# RobotOS — Current State Snapshot

> **Purpose:** Canonical startup snapshot for agents (Claude, Copilot, GPT, GLM).
> Detailed phase history remains in `RobotOS_v1.0/devkit/docs/DEVKIT_PROGRESS.md`.
> Update this file only at phase close, from validated evidence only.
> Do not record assumptions as fact. If unknown, mark **Pending / Unknown**.

---

## Last Closed Phase

**Phase 4L — Scheduler Advisory Retry Decision Policy**

- **Commit:** `a3f4369`
- **Date:** 2026-05-03
- **Branch:** master
- **Remote:** `https://github.com/thangnguyeniot-afk/Robot_OS` (confirmed linked)

---

## Validation Evidence (Phase 4L)

| Gate | Result | Detail |
|------|--------|--------|
| Host tests | PASS | 14/14 suites, 0 failures |
| Zephyr build | PASS | west build -b stm32f411e_disco |
| FLASH | 28988 B / 524288 B (5.53%) | stm32f411e_disco |
| RAM | 12160 B / 131072 B (9.28%) | stm32f411e_disco |
| Runtime evidence | Not required | Advisory mapping is pure C, no platform dependency |

Host test log: `RobotOS_v1.0/tests/host/logs/phase_4L_host_2026-05-03.log`

---

## Behavior Guarantees from Phase 4L

- **No new status codes.** `ROBOTOS_CORE_ERR_THROTTLED = -7` remains the highest-numbered error.
- **No mutable retry state.** No retry counters, no per-producer state added to core or snapshot.
- **No auto-retry behavior.** `robotos_core_retry_decision_for_status()` and `robotos_core_status_is_retryable()` are pure advisory mappings. The producer owns all scheduling decisions.
- **No lock, no platform call, no queue access** in the new functions.
- **Safe to call before init** and from any context.
- Existing `post_event` / `try_post_event` semantics unchanged.

---

## Files Changed in Phase 4L

| File | Change |
|------|--------|
| `core/robotos_core.h` | Add `robotos_core_retry_action_t` enum, `robotos_core_retry_decision_t` struct, `robotos_core_retry_decision_for_status()`, `robotos_core_status_is_retryable()` |
| `core/robotos_core.c` | Stateless implementation of both advisory functions |
| `core/README.md` | Phase 4L section: advisory API, mapping table, design constraints, limitations |
| `tests/host/CMakeLists.txt` | Add `robotos_scheduler_retry_policy_contract_test` target |
| `tests/host/test_robotos_scheduler_retry_policy_contract.c` | 13 test cases |
| `RobotOS_v1.0/devkit/docs/DEVKIT_PROGRESS.md` | Phase 4L section |
| `tests/host/logs/phase_4L_host_2026-05-03.log` | Host test evidence |

---

## Retry Policy Mapping Table (Phase 4L)

| Status | action | wait_ticks | drop | report |
|--------|--------|-----------|------|--------|
| `OK` | `RETRY_NONE` | 0 | false | false |
| `ERR_FULL` | `RETRY_AFTER_TICK` | 1 | false | false |
| `ERR_THROTTLED` | `RETRY_AFTER_TICK` | 1 | false | false |
| `ERR_INVALID_STATE` | `RETRY_SOON` | 0 | false | false |
| `ERR_INVALID_ARG` | `RETRY_NEVER` | 0 | true | true |
| `ERR_NULL` | `RETRY_NEVER` | 0 | true | true |
| `ERR_EMPTY` | `RETRY_NONE` | 0 | false | false |
| unknown | -> `ERR_INVALID_ARG` | 0 | false | false |

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
| 6G | Timer producer smoke (ISR context) | `335ee29` |
| 6A–6E | Event pipeline smokes | preceding commits |
| 4K | Scheduler admission + throttle policy | preceding commits |
| 4L | Scheduler advisory retry decision policy | `a3f4369` |

---

## Next Candidate Phases

| Phase | Description | Status |
|-------|-------------|--------|
| Phase 6F | Devkit Mixed Event Policy Smoke | Candidate |
| Phase 6I | Timer Producer Queue-Pressure Stress | Candidate |
| Phase 5D | Platform Critical Section / ISR Lock Boundary | **Pending / Verify** — `4187bb3` in git log may indicate already completed; confirm before scheduling |

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
