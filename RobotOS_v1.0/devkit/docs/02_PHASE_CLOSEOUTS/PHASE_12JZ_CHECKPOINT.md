# Phase 12J-Z — Probe Translator App-Layer Checkpoint / Integration Direction Guard

**Status:** `CLOSED_DOCS_ONLY`
**Type:** Docs-only checkpoint / integration direction guard. **No
source, runtime, test, CMake, Zephyr, board, `prj.conf`, DTS overlay,
evidence log, Framework header, Framework `.c` file, devkit runtime,
command-set, or `devkit_app_state` change.**
**Date:** 2026-05-14
**Branch:** master
**HEAD at checkpoint:** `2d9f832` (`app: implement Phase 12J probe
translator FAULT block`)
**origin/master at checkpoint:** `2d9f832` (synced)
**Prior implementation phase:** Phase 12J (`2d9f832`,
`PHASE_12J_FAULT_BLOCK_IMPLEMENTED_VALIDATED`)
**Prior checkpoint:** Phase 11Z command-set checkpoint
([`PHASE_11Z_COMMAND_SET_CHECKPOINT.md`](PHASE_11Z_COMMAND_SET_CHECKPOINT.md))
**Companion docs:**
[`../03_SPECS/PROBE_TRANSLATOR_FAULT_BLOCK_PLAN.md`](../03_SPECS/PROBE_TRANSLATOR_FAULT_BLOCK_PLAN.md),
[`../03_SPECS/PROBE_TRANSLATOR_HOST_PROTOTYPE_PLAN.md`](../03_SPECS/PROBE_TRANSLATOR_HOST_PROTOTYPE_PLAN.md),
[`PHASE_12J_PROBE_TRANSLATOR_FAULT_BLOCK.md`](PHASE_12J_PROBE_TRANSLATOR_FAULT_BLOCK.md),
[`PHASE_9EZ_CHECKPOINT.md`](PHASE_9EZ_CHECKPOINT.md)

---

## A. Executive Summary

Phase 12J-Z is a docs-only checkpoint that **locks the Phase 12J
application-layer behavior baseline** for `app/probe_translator/` and
**guards against premature opening of integration phases** before a
deliberate architectural decision is made.

Phase 12J closed with full host-test validation:
- `probe_translator_mapping_contract`: **132/132** in-binary assertions
  PASS (TC01–TC24)
- Full host regression: **23/23** ctest targets PASS
- Evidence: `tests/host/logs/phase_12J_host_2026-05-14.log`

The `probe_translator` application harness is now host-test-validated
with a complete FAULT block. It remains **host-only**: no Zephyr build,
no devkit runtime wiring, no UART surface, no hardware run, no
`devkit_app_state` coupling.

**App-layer verdict:** `APP_LAYER_BEHAVIOR_COMPLETE_AT_HOST_DEPTH`

**Recommended next move:** HOLD as default safe state. If continuing,
the safest next planned gate is:
`Phase 12K-pre — Probe Translator Zephyr Build-Only Admission Plan`
(docs-only planning; does not wire runtime behavior or devkit state).

Phase 12J-Z introduces **no** runtime behavior, **no** source/config
change, **no** new command, and **no** authorization for any open gate.
Each remaining open gate carries forward in its prior status.

---

## B. Phase 12J Behavior Baseline (locked at 2d9f832)

### B.1 `probe_translator` constants

```c
/* States */
PROBE_TRANSLATOR_STATE_IDLE   = 1u
PROBE_TRANSLATOR_STATE_READY  = 2u
PROBE_TRANSLATOR_STATE_ACTIVE = 3u
PROBE_TRANSLATOR_STATE_FAULT  = 4u   /* Phase 12J */

/* Events */
PROBE_TRANSLATOR_EVT_CONFIGURED = 1u
PROBE_TRANSLATOR_EVT_START      = 2u
PROBE_TRANSLATOR_EVT_STOP       = 3u
PROBE_TRANSLATOR_EVT_RESET      = 4u
PROBE_TRANSLATOR_EVT_FAULT      = 5u  /* Phase 12J */

/* Adapter types */
PROBE_ADAPTER_TYPE_CONFIG   = 1u
PROBE_ADAPTER_TYPE_COMMAND  = 2u
PROBE_ADAPTER_TYPE_FAULT    = 3u      /* Phase 12J */

/* Adapter args */
PROBE_ADAPTER_ARG_NONE  = 0u
PROBE_ADAPTER_ARG_START = 1u
PROBE_ADAPTER_ARG_STOP  = 2u
PROBE_ADAPTER_ARG_RESET = 3u
PROBE_ADAPTER_ARG_ANY   = 0xFFFFFFFFu  /* Phase 12J; doc alias */
```

### B.2 FSM state-def table (4 entries)

```
IDLE   READY   ACTIVE   FAULT
```
All `on_entry` / `on_exit` = NULL.

### B.3 FSM transition table (10 rows)

| Row | From | Event | To | Phase |
| --- | --- | --- | --- | --- |
| 0 | IDLE | EVT_CONFIGURED | READY | 12I |
| 1 | READY | EVT_START | ACTIVE | 12I |
| 2 | ACTIVE | EVT_STOP | READY | 12I |
| 3 | READY | EVT_RESET | IDLE | 12I |
| 4 | ACTIVE | EVT_RESET | IDLE | 12I |
| 5 | IDLE | EVT_RESET | IDLE (self-loop) | 12J |
| 6 | IDLE | EVT_FAULT | FAULT | 12J |
| 7 | READY | EVT_FAULT | FAULT | 12J |
| 8 | ACTIVE | EVT_FAULT | FAULT | 12J |
| 9 | FAULT | EVT_RESET | IDLE | 12J |

All guard/action = NULL.

### B.4 Bridge mapping table (5 rows)

| Row | Type | Arg | match_arg0 | fw_event | Phase |
| --- | --- | --- | --- | --- | --- |
| 0 | TYPE_CONFIG | ARG_NONE | true | EVT_CONFIGURED | 12I |
| 1 | TYPE_COMMAND | ARG_START | true | EVT_START | 12I |
| 2 | TYPE_COMMAND | ARG_STOP | true | EVT_STOP | 12I |
| 3 | TYPE_COMMAND | ARG_RESET | true | EVT_RESET | 12I |
| 4 | TYPE_FAULT | ARG_ANY | **false** | EVT_FAULT | 12J |

Row 4 uses `match_arg0=false` — existing Phase 12F bridge wildcard
feature. No bridge code change.

### B.5 Sticky FAULT behavior (implicit)

From FAULT: `CONFIG`, `START`, `STOP` adapter events are mapped by the
bridge (`mapped_count++`) but the FSM has no matching row
(`no_transition_count++`). State remains FAULT. Return: `OK`.
Only `(TYPE_COMMAND, ARG_RESET)` exits FAULT (row 9).

### B.6 Host test coverage (TC01–TC24)

| Range | Cases | Assertions | Phase |
| --- | --- | --- | --- |
| TC01–TC15 | 15 | 70 | 12I (retained) |
| TC16–TC24 | 9 | 62 | 12J (new) |
| **Total** | **24** | **132** | **12J baseline** |

ctest: 23/23 PASS (no new target added at Phase 12J).

---

## C. What Is Now Proven vs. What Remains Unproven

### C.1 Proven (host-test depth)

| Claim | Evidence |
| --- | --- |
| FAULT reachable from IDLE / READY / ACTIVE via TYPE_FAULT adapter event | TC16–TC18 |
| Wildcard `match_arg0=false` maps any arg0 TYPE_FAULT event → EVT_FAULT | TC19 |
| FAULT exit via RESET (row 9) → IDLE | TC20 |
| Sticky FAULT: CONFIG/START/STOP don't exit FAULT | TC21 |
| IDLE+RESET self-loop commits a transition (transition_count++) | TC22 |
| Full IDLE→READY→ACTIVE→FAULT→IDLE path | TC23 |
| Bridge event/mapped/unmapped counters correct with FAULT and unmapped mix | TC24 |
| No devkit/Zephyr/UART dependency | TC12–TC14 (REVIEW_VALIDATED) |
| 23/23 host regression preserved | TC15 + ctest |
| Config lifetime (embedded `_fsm_cfg`/`_bridge_cfg`) | TC03, TC09, TC23 exercised |

### C.2 Unproven (out of scope at Phase 12J)

| Claim | Status |
| --- | --- |
| `probe_translator` compiles under Zephyr's build system | NOT_PROVEN — no Zephyr build target exists |
| `probe_translator` works in a Zephyr application context | NOT_STARTED |
| `probe_translator` can be wired to devkit runtime / `devkit_app_state` | NOT_STARTED |
| FAULT adapter events can be sourced from real hardware/firmware signals | NOT_STARTED |
| Non-NULL action / on_entry / on_exit callbacks are safe | NOT_STARTED |
| Bridge ABI memory layout is stable across compiler/ABI boundaries | NOT_LOCKED |
| Multi-product coordination rules are safe | NOT_STARTED |

---

## D. Integration Boundaries Still Closed at Phase 12J-Z

| Boundary | Status |
| --- | --- |
| Devkit runtime (`devkit_app_state`, `devkit_uart_handler`) | **CLOSED** |
| UART command surface (`a/s/r/?/x/v/L/d/T`) | **FROZEN** |
| Zephyr build admission for `app/probe_translator/` | **NOT_STARTED** |
| Hardware validation for `app/probe_translator/` | **NOT_STARTED** |
| Framework bridge ABI lock | **NOT_STARTED** |
| `src/` / `include/robotos/` (Architecture B) | **FROZEN** |
| `core/` / `platform/` / `framework/` | **BOUNDARY_PRESERVED** |
| Scheduler 7A/7B | **DEFER** |
| F407 / custom board | **HOLD/DEFER** |
| ACTIVE disarm widening | **USER_DECISION_REQUIRED** |
| POST_FLASH_AUTOSTART root cause | **OPEN / MITIGATED_BY_WORKFLOW** |

---

## E. Candidate Next Gate Matrix

| # | Candidate | Type | Likely Surface | Risk to Guards | Validation Path | Recommendation |
| --- | --- | --- | --- | --- | --- | --- |
| 1 | **HOLD** | HOLD | none | none | n/a | **HOLD_RECOMMENDED** |
| 2 | Zephyr build-only admission planning (Phase 12K-pre) | docs-only | new spec doc | none | docs review | **SAFE_NEXT** |
| 3 | Zephyr build-only admission (Phase 12K) | Zephyr build-only | `app/probe_translator/CMakeLists.txt`, `prj.conf` | low (no runtime wiring) | `west build`, no flash | **SAFE_AFTER_PRE_DOC** |
| 4 | Devkit integration planning | docs-only | new spec doc | none (docs only) | docs review | **SAFE_AFTER_PRE_DOC** |
| 5 | Devkit runtime integration of `probe_translator` | devkit runtime | `devkit_app_state.c/.h`, UART handler | **HIGH** — touches devkit runtime + UART semantics | Zephyr build + RTT evidence | **HIGH_RISK_NEEDS_CONTRACT** |
| 6 | Bridge ABI memory-layout lock planning | docs-only | `FRAMEWORK_APPLICATION_BRIDGE_DRAFT.md` update | none | docs review | **SAFE_AFTER_PRE_DOC** |
| 7 | `examples/` directory planning | docs-only | new `examples/` spec | none | docs review | **SAFE_NEXT** (low priority) |
| 8 | Second app/product planning | docs-only | new `app/<product>/` spec | none | docs review | **SAFE_AFTER_PRE_DOC** |
| 9 | Product command mapping / UART expansion | command-interface | `devkit_uart_handler.c`, command set docs | **HIGH** — UART freeze, devkit runtime | Zephyr build + RTT | **USER_DECISION_REQUIRED** |
| 10 | ACTIVE disarm widening | command-interface + devkit | `devkit_app_state.c`, UART handler | **HIGH** — ACTIVE disarm guard | Zephyr build + RTT | **USER_DECISION_REQUIRED** |
| 11 | Scheduler 7A/7B | scheduler/execution | `core/`, `platform/`, `devkit/src/` | **HIGH** — Scheduler DEFER | host tests + Zephyr build + RTT | **DEFER** |
| 12 | F407 / custom board | hardware | `boards/`, DTS, overlay, `prj.conf` | **HIGH** — F407 HOLD | hardware build + flash + RTT | **DEFER** |
| 13 | POST_FLASH_AUTOSTART root-cause work | hardware | `prj.conf`, Zephyr config | medium | hardware + RTT | **DEFER** |

### E.1 Detailed notes on top candidates

#### Candidate 2 — Phase 12K-pre (Zephyr Build-Only Admission Planning)

**Why safe next:** Proving `probe_translator` compiles under Zephyr's build
system without any runtime wiring is a bounded, low-risk step. It does NOT
require:
- wiring `probe_translator` to `devkit_app_state`
- adding a UART command
- running hardware
- changing devkit runtime behavior

Build-only admission means: create a minimal `app/probe_translator/CMakeLists.txt`
and `prj.conf` that include `probe_translator.c` in a Zephyr target (no app
main, no hardware init, just compilation proof). CMake only — no runtime wiring.

**Contract required:** A docs-only Phase 12K-pre planning gate locks:
- exact new file set (at minimum: `app/probe_translator/CMakeLists.txt`,
  `prj.conf`)
- whether a DTS or overlay is needed
- what `Kconfig` / `CONFIG_*` entries are required
- whether the Framework `.c` files need a Zephyr-side wrapper
- that devkit runtime is NOT wired at Phase 12K

#### Candidate 5 — Devkit Runtime Integration (HIGH RISK)

**Why not now:** Devkit runtime integration requires modifying
`devkit_app_state.c`, the UART command handler, and possibly introducing
new event sources (`PROBE_ADAPTER_TYPE_FAULT` fired from real hardware
signals). These touch:
- `devkit_app_state` (scope-guard #11)
- UART command semantics (frozen `a/s/r/?/x/v/L/d/T`)
- ACTIVE disarm logic
- Zephyr build context

This must not happen without a dedicated planning contract that explicitly
addresses each boundary. A separate Phase 12K+1-pre or similar planning
gate is required first.

#### Candidates 9, 10 — UART expansion and ACTIVE disarm

Both require deliberate product decisions. Command set freeze from Phase
9E-Z (`UART_TX_SCOPE_FROZEN`) and Phase 11Z (`9-command set validated`)
is still active. These may only open on explicit user authorization with
a product-direction decision memo.

---

## F. Recommended Next Gate

**Default (safe):** `HOLD`

If the user wants to continue on the `probe_translator` integration track:

> **`Phase 12K-pre — Probe Translator Zephyr Build-Only Admission Plan`**

This is a **docs-only** planning gate that:
- contracts the exact file set for Zephyr build admission
- confirms whether DTS / overlay / Kconfig changes are needed
- explicitly excludes devkit runtime wiring, UART command changes, and hardware runs
- creates an implementation contract for Phase 12K (build-only, no flash)

This gate is the minimal safe step to prove Zephyr compilation without
widening any integration boundary.

**Do not recommend directly opening Phase 12K (implementation) without
Phase 12K-pre (planning contract) first.**

---

## G. Carried-Forward Open Gates

All gates carried forward unchanged from Phase 12J:

| Gate | Status |
| --- | --- |
| ACTIVE disarm widening | `USER_DECISION_REQUIRED_ACTIVE_DISARM` |
| Scheduler 7A/7B | `DEFER` |
| F407 / custom board | `HOLD/DEFER` |
| POST_FLASH_AUTOSTART root cause | `OPEN / MITIGATED_BY_WORKFLOW` |
| Non-NULL action / on_entry / on_exit for FAULT | `OPEN (future app-behavior phase)` |
| Devkit integration of `probe_translator` | `NOT_STARTED` |
| Bridge ABI memory-layout lock | `NOT_STARTED` |
| Hardware-runnable Zephyr build of `app/probe_translator/` | `NOT_STARTED` |
| `RobotOS_v1.0/examples/` | `NOT_STARTED` |
| Multi-product coordination rules | `NOT_STARTED` |
| Zephyr build-only admission for `probe_translator` | `NOT_STARTED` |

---

## H. What This Checkpoint Does NOT Open

Phase 12J-Z **does not** open or authorize:

- devkit runtime integration of `probe_translator`
- any UART command semantic change
- Scheduler 7A/7B
- F407 / custom board work
- ACTIVE disarm widening
- POST_FLASH_AUTOSTART root-cause investigation
- hardware validation (RTT, J-Link, OpenOCD, flashing)
- Zephyr build implementation (Phase 12K)
- second `app/<product>/` directory
- product command mapping

**Only a subsequent explicit user command may open any of these.**

---

## I. Phase 12J-Z Decision

`PHASE_12J_Z_APP_LAYER_CHECKPOINT_CLOSED`

Baseline locked. Open gates carried forward. Recommended next gate:
`Phase 12K-pre — Probe Translator Zephyr Build-Only Admission Plan`
(or HOLD).
