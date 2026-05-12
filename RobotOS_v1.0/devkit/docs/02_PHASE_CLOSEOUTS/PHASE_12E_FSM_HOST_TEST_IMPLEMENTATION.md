# Phase 12E — Framework FSM Host-Test Implementation (`CLOSED_WITH_HOST_TEST_EVIDENCE`)

**Status:** `CLOSED_WITH_HOST_TEST_EVIDENCE`
**Decision result:** `PHASE_12E_FSM_HOST_TEST_IMPLEMENTATION_CLOSED`
**Type:** First Framework implementation phase. Adds the FSM `.c` body
behind the Phase 12D-LOCKED header, plus a single new host-test target
inside the existing Architecture-A `tests/host/` build, plus a tracked
host log. **No devkit integration. No UART command. No Zephyr config
change. No hardware run. No legacy Architecture B modification.**
**Date opened/closed:** 2026-05-12 (same-day host-test close)
**Published baseline at open:** `origin/master = a8019b5`
**Prior closed phase:** Phase 12E-pre (`CLOSED_DOCS_ONLY`;
`PHASE_12E_RECOMMEND_HOST_UNIT_TEST_CONSUMER`).
**Authorizing user instruction:** "OPEN PHASE 12E — FRAMEWORK FSM
HOST-TEST IMPLEMENTATION" (this session).

---

## A. Executive Summary

Phase 12E implements the six functions declared in the Phase 12D-LOCKED
header `RobotOS_v1.0/framework/robotos_fw_fsm.h` and validates them with
a single host-test executable (`robotos_fw_fsm_contract_test`) wired into
the existing Architecture-A `RobotOS_v1.0/tests/host/CMakeLists.txt`. The
test exercises 20 test cases / **93 assertions** covering every Phase
12B/12C behavioral contract. All 93 assertions pass under WSL Ubuntu 24.04
with gcc 13.3.0 / cmake / ctest. The full pre-existing host test suite
(21 targets, including the new one) passes 100%.

No `.c` file outside `RobotOS_v1.0/framework/` is created or modified
beyond the additive Phase 12E test source. No CMake file outside the
single additive block in `RobotOS_v1.0/tests/host/CMakeLists.txt` is
modified. `RobotOS_v1.0/framework/robotos_fw_fsm.h` is **zero-diff** —
the Phase 12D-LOCKED API surface needed no contract revision. No
devkit integration, no UART command, no hardware evidence, no command-
semantic change, no Architecture B modification.

---

## B. Baseline Before Phase 12E

| Item | Value |
|---|---|
| Published baseline at open | `origin/master = a8019b5` |
| Prior closed phase | Phase 12E-pre (`CLOSED_DOCS_ONLY`) |
| Phase 12E-pre decision | `PHASE_12E_RECOMMEND_HOST_UNIT_TEST_CONSUMER` |
| Phase 12D header stub | `RobotOS_v1.0/framework/robotos_fw_fsm.h` (LOCKED-AT-12D; declarations only) |
| Framework `.c` implementation before 12E | **NOT BUILT** |
| Framework consumer before 12E | None |
| Architecture A | `core/` + `platform/` + `devkit/` + `framework/` — canonical active stack |
| Architecture B | `src/` + `include/robotos/` + root `CMakeLists.txt` — frozen legacy scaffold |
| Architecture A host test infra | `RobotOS_v1.0/tests/host/CMakeLists.txt` — 20 tracked Phase 4-6 contract targets, ctest, `save_test_log.cmake` log convention; documented as needing WSL/Linux on Windows hosts |
| Validated command set | `a / s / r / ? / x / v / L / d / T` (unchanged at this phase) |
| Phase 12D syntax-check finding | `SYNTAX_CHECK_NOT_RUN_TOOLCHAIN_OUTPUT_SUPPRESSED` (local MSYS2 MinGW64 broken; Phase 12E uses WSL Ubuntu as recommended) |
| Open gates preserved | ACTIVE disarm `USER_DECISION_REQUIRED_ACTIVE_DISARM`; Scheduler 7A/7B `DEFER`; F407/custom board `HOLD/DEFER`; POST_FLASH_AUTOSTART `OPEN` / `MITIGATED_BY_WORKFLOW`; Application/product layer `NOT_STARTED`; devkit integration `NOT_STARTED` |

---

## C. Files Changed

| Path | Kind | Notes |
|---|---|---|
| `RobotOS_v1.0/framework/robotos_fw_fsm.c` | NEW source | First Framework `.c` body. ~210 lines. Includes only `"robotos_fw_fsm.h"`, `"robotos_platform_critical.h"`, `<stddef.h>`. No Zephyr, no devkit, no legacy `ro_*`, no heap, no UART, no core dispatcher registration. |
| `RobotOS_v1.0/tests/host/test_robotos_fw_fsm.c` | NEW host test | 20 test cases, 93 assertions. Follows the existing `test_robotos_*_contract.c` style (TC macro, plain C, exit 0 on all-pass). |
| `RobotOS_v1.0/tests/host/CMakeLists.txt` | UPDATE — additive only | Added `set(FRAMEWORK_DIR ...)`; added one `add_executable(robotos_fw_fsm_contract_test ...)` + one matching `add_test(...)` block at end of file before `save_test_log` target. No structural change. No existing test target modified. |
| `RobotOS_v1.0/tests/host/logs/phase_12E_host_2026-05-12.log` | NEW tracked log | ctest verbose output of `robotos_fw_fsm_contract_test`; 160 lines; 93 PASS / 0 FAIL. |
| `RobotOS_v1.0/devkit/docs/02_PHASE_CLOSEOUTS/PHASE_12E_FSM_HOST_TEST_IMPLEMENTATION.md` | NEW closeout | This doc. |
| `RobotOS_v1.0/devkit/docs/03_SPECS/FRAMEWORK_FSM_API_DRAFT.md` | UPDATE — small | Header revision marker advanced to Phase 12E; §1 decision-state table adds `IMPLEMENTED_AT_12E` rows; §10 next-revision-conditions advanced; preserves DRAFT/EXPERIMENTAL ABI warning. |
| `RobotOS_v1.0/devkit/docs/01_PROGRESS/DEVKIT_PROGRESS_PHASE_11_20.md` | UPDATE | Phase 12E index row + `<a id="phase-12e"></a>` section. |
| `CURRENT_STATE.md` | UPDATE | Last closed phase = Phase 12E; framework implementation status = `HOST_TEST_VALIDATED_FSM_CORE`. |
| `RobotOS_v1.0/devkit/docs/00_INDEX/README.md` | UPDATE | Closeout link + progress status row. |

**Files explicitly NOT modified (verified zero-diff):**

- `RobotOS_v1.0/framework/robotos_fw_fsm.h` (LOCKED-AT-12D held).
- `RobotOS_v1.0/framework/README.md`.
- All files under `RobotOS_v1.0/core/`, `RobotOS_v1.0/platform/`,
  `RobotOS_v1.0/devkit/src/`, `RobotOS_v1.0/devkit/boards/`,
  `RobotOS_v1.0/devkit/zephyr/`.
- `RobotOS_v1.0/devkit/CMakeLists.txt`, root `RobotOS_v1.0/CMakeLists.txt`.
- `RobotOS_v1.0/src/`, `RobotOS_v1.0/include/robotos/`,
  `RobotOS_v1.0/include/app/`.
- `RobotOS_v1.0/tests/CMakeLists.txt` (Architecture B legacy test build).
- Any other tracked test source under `RobotOS_v1.0/tests/host/test_*.c`.
- `prj.conf`, DTS files, board overlays, Zephyr workspace files.
- Host tools, runtime scripts, devkit evidence logs.

---

## D. Implementation Summary

### D.1 Functions implemented

All six Phase 12D-LOCKED functions are implemented as plain C99
function bodies; no `static inline`, no macro forwarding, no global
mutable singleton state.

| Function | Lines | Behavior |
|---|---|---|
| `robotos_fw_fsm_init` | ~25 | NULL/INVALID_ARG gates; assigns `config`; `current_state = initial_state`; zeroes counters; sets `initialized = true`; calls `on_entry(initial_state)` if state def exists. **Idempotent re-init** zeroes counters and re-runs entry. |
| `robotos_fw_fsm_dispatch` | ~50 | Thread-context only. Increments `event_count`. Records `last_event_id`. Scans the transition table FIFO: skips rows with mismatched `(current_state, event_id)`; for matching rows, calls guard (if any); if guard rejects, increments `guard_rejected_count` and continues. On the first row whose guard accepts (or is NULL), runs exit → state update → action → entry in that exact order. Updates `transition_count`. Records `last_status = action_status` (OK if no action). Returns the action's status. If no row commits, increments `no_transition_count`, records `last_status = OK`, returns OK. |
| `robotos_fw_fsm_get_state` | ~10 | Critical-section-protected read. Returns `ROBOTOS_FW_STATE_UNINIT` for NULL/uninitialized; `current_state` otherwise. |
| `robotos_fw_fsm_reset` | ~25 | NULL/INVALID_STATE gates. Calls `on_exit(current_state)` if state def exists; sets `current_state = initial_state`; zeroes all counters; calls `on_entry(initial_state)` if state def exists. **No action runs** during reset. |
| `robotos_fw_fsm_is_in_state` | ~10 | Critical-section-protected read. False for NULL/uninitialized; `current_state == state_id` otherwise. |
| `robotos_fw_fsm_get_snapshot` | ~20 | NULL/INVALID_STATE gates. Critical-section-protected copy of all eight visible counters + `current_state` + `last_event_id` + `last_status` + `initialized`. |

### D.2 Static helpers

| Helper | Role |
|---|---|
| `fw_find_state_def(config, state_id)` | Linear scan of `config->states` returning a pointer to the matching def or NULL. Used by init, dispatch (on_exit and on_entry), and reset. |
| `fw_reset_counters(fsm)` | Zeroes `transition_count`, `event_count`, `guard_rejected_count`, `no_transition_count`, `last_event_id`, `last_status`. Called by init and reset. |

Helpers are `static` and not part of the public ABI.

### D.3 Key data flow

The FSM struct is **caller-owned**; the application declares a
`robotos_fw_fsm_t` (typically static) and passes its address into every
public API call. The implementation reads and writes only fields of
`*fsm`. The `config` pointer is stored in `fsm->config` at init time and
borrowed for the FSM's lifetime — the application must keep the config
and its arrays alive. The `event_payload` pointer is borrowed for the
duration of `dispatch()` only and is **never** stored in `*fsm` (the
struct definition in the header has no payload field; the
implementation does not introduce one).

### D.4 Status mapping (Phase 12C reuse)

`robotos_fw_status_t` is the alias of `robotos_core_status_t` set up in
the Phase 12D header. Phase 12E uses the existing enum values
without introducing any new code:

| Condition | Status code |
|---|---|
| Success / no-transition dispatch | `ROBOTOS_CORE_OK` |
| `fsm`, `config`, or `config->transitions` is NULL | `ROBOTOS_CORE_ERR_NULL` |
| `out` snapshot pointer is NULL | `ROBOTOS_CORE_ERR_NULL` |
| `transition_count == 0` at init | `ROBOTOS_CORE_ERR_INVALID_ARG` |
| `initial_state == ROBOTOS_FW_STATE_UNINIT` at init | `ROBOTOS_CORE_ERR_INVALID_ARG` |
| FSM not initialized (`!fsm->initialized`) | `ROBOTOS_CORE_ERR_INVALID_STATE` |
| Action returned non-OK | The action's exact return value (passed through unchanged) |
| Guard rejected the row | Not surfaced as a status; counted in `guard_rejected_count`; dispatch may still return OK from the no-transition path |
| No matching row | Not surfaced as a status; counted in `no_transition_count`; dispatch returns OK |

**No new status enum was introduced.** The existing `robotos_core_status_t`
codes cover every Phase 12E exit. The "no match" and "guard rejected"
outcomes are intentionally observability counters rather than return
codes — this matches Phase 12C `GUARD_RETURNS_BOOL_ONLY_CONFIRMED` and
the Phase 12B counter model.

### D.5 Entry / exit non-OK behavior (Phase 12E design decision)

Phase 12B/12C are silent on what to do when `on_entry` or `on_exit`
returns non-OK. Phase 12E commits to: **entry and exit return values
are observed but not propagated**. `dispatch()` returns the action's
status (or OK if no action ran); `init()` and `reset()` return OK
regardless of entry/exit return. `last_status` records only the action's
status (or OK on no-transition). Rationale: action is the
state-mutating callback, so its return value carries the dispatch
outcome; entry/exit are side-effect callbacks whose failures are
recoverable per state. This is the most conservative interpretation of
the Phase 12C `ACTION_NON_OK_NO_ROLLBACK_CONFIRMED` rule and avoids
silently changing the rollback semantics under entry/exit failure.

If a future product needs entry/exit failure propagation, that is a
header-API revision phase, not Phase 12E.

### D.6 No heap, no UART, no core register

`robotos_fw_fsm.c` contains:
- 0 calls to `malloc`/`calloc`/`realloc`/`free`.
- 0 calls to any `printf`/`puts`/UART/TX function.
- 0 calls to `robotos_core_register_event_handler` or any other core
  dispatcher API.
- 0 thread/timer/work-item creation calls.
- 0 references to `<zephyr/...>`, `<kernel.h>`, devkit headers, or
  legacy `ro_*` headers.

Verified by grep (`grep -E 'malloc|calloc|realloc|free\(|printf|uart|robotos_core_register_event_handler|<zephyr|<kernel|ro_status_t' RobotOS_v1.0/framework/robotos_fw_fsm.c` produced no matches).

### D.7 Critical section usage

`get_state`, `is_in_state`, and `get_snapshot` wrap their reads in
`robotos_platform_critical_enter()` / `robotos_platform_critical_exit()`.
Under host build, this is the host stub from
`tests/host/robotos_platform_critical_host_stub.c` (deterministic
counter-only). Under the future devkit build (not Phase 12E), the
Zephyr backend would lock IRQs. Phase 12E test TC20 verifies the
enter/exit counts are balanced (3 enters, 3 exits across the three
calls).

---

## E. Behavior Coverage Matrix

The 25 contract items from Phase 12E-pre §F are tracked against the
host test executable:

| # | Contract item | Maps to | Verdict |
|---|---|---|---|
| 1 | init valid config | TC01 | `ASSERTED_BY_TEST` |
| 2 | init NULL guards | TC02 | `ASSERTED_BY_TEST` |
| 3 | init invalid args (`transition_count==0`, `initial==UNINIT`) | TC03 | `ASSERTED_BY_TEST` |
| 4 | dispatch matched transition | TC05 | `ASSERTED_BY_TEST` |
| 5 | first-match FIFO determinism | TC07 | `ASSERTED_BY_TEST` |
| 6 | guard false / guard rejected | TC09, TC10 | `ASSERTED_BY_TEST` |
| 7 | exit → state update → action → entry order | TC06, TC12 | `ASSERTED_BY_TEST` (seq tag `X1a2E2`) |
| 8 | action non-OK no rollback | TC12 | `ASSERTED_BY_TEST` (state stays at S_B; entry runs; dispatch returns non-OK) |
| 9 | entry non-OK behavior | TC01, TC13 | `REVIEW_VALIDATED` (Phase 12E design: entry status observed but not propagated; documented in §D.5) |
| 10 | reset behavior | TC13 | `ASSERTED_BY_TEST` |
| 11 | get_state | TC14 | `ASSERTED_BY_TEST` |
| 12 | is_in_state | TC15 | `ASSERTED_BY_TEST` |
| 13 | get_snapshot | TC16 | `ASSERTED_BY_TEST` |
| 14 | transition_count semantics | TC05, TC08, TC12, TC17 | `ASSERTED_BY_TEST` |
| 15 | event_count per dispatch | TC17 | `ASSERTED_BY_TEST` |
| 16 | last_event_id | TC05, TC08, TC16 | `ASSERTED_BY_TEST` |
| 17 | payload borrowed not cached | TC18 | `ASSERTED_BY_TEST` (action sees payload; snapshot exposes no payload field) |
| 18 | guard observes pre-transition state | TC11, TC06 | `ASSERTED_BY_TEST` (seq tag `g1`; pre-state matches) |
| 19 | action observes post-transition state | TC06, TC12 | `ASSERTED_BY_TEST` (seq tag `a2`; state already updated) |
| 20 | entry observes post-transition state | TC06, TC12 | `ASSERTED_BY_TEST` (seq tag `E2`) |
| 21 | exit observes pre-transition state | TC06, TC12 | `ASSERTED_BY_TEST` (seq tag `X1`) |
| 22 | re-init policy (idempotent) | TC19 | `ASSERTED_BY_TEST` |
| 23 | no heap allocation | grep | `REVIEW_VALIDATED` (0 matches for `malloc`/`calloc`/`realloc`/`free` in `robotos_fw_fsm.c`) |
| 24 | no UART TX, no core dispatcher registration | grep | `REVIEW_VALIDATED` (0 matches for `printf`/`uart`/`robotos_core_register_event_handler` in `robotos_fw_fsm.c`) |
| 25 | public-symbol surface matches LOCKED-AT-12D | nm/grep | `REVIEW_VALIDATED` (only the 6 declared functions are non-static; helpers `fw_find_state_def`, `fw_reset_counters` are `static`) |

**Coverage summary:** 21 items `ASSERTED_BY_TEST`, 4 items
`REVIEW_VALIDATED`, 0 items `BLOCKED`. All 25 contract items from the
Phase 12E-pre plan are satisfied.

---

## F. Host Build/Test Result

### F.1 Environment

| Item | Value |
|---|---|
| OS | Ubuntu 24.04 LTS under WSL2 on Windows 11 (per the Phase 12E-pre recommendation and the existing `tests/host/CMakeLists.txt` note) |
| Compiler | gcc 13.3.0 (Ubuntu 13.3.0-6ubuntu2~24.04) |
| CMake | system cmake (apt) |
| Generator | Unix Makefiles |
| Build dir | `build-phase12e-host/` (untracked, gitignored by convention) |
| Local MSYS2 MinGW64 | **Not used.** Phase 12D recorded this toolchain as silent-failure unreliable; Phase 12E-pre §G mandated WSL/Linux for this phase. |

### F.2 Commands

```sh
# Configure
cmake -S RobotOS_v1.0/tests/host -B build-phase12e-host

# Build (whole tree to confirm no regression; FSM target alone also works)
cmake --build build-phase12e-host

# Run only the new target (and capture log)
ctest --test-dir build-phase12e-host \
      -R robotos_fw_fsm_contract \
      --output-on-failure --verbose \
  > RobotOS_v1.0/tests/host/logs/phase_12E_host_2026-05-12.log 2>&1

# Run full suite (regression confirmation)
ctest --test-dir build-phase12e-host --output-on-failure
```

### F.3 Results

| Run | Result |
|---|---|
| Configure (`cmake -S ... -B ...`) | PASS — `Configuring done`, `Generating done`, `Build files have been written to:` |
| Build all targets | PASS — `[100%] Built target robotos_fw_fsm_contract_test` plus all 20 prior targets; a "clock skew detected" cmake warning was emitted (WSL/Windows filesystem time-stamp drift between the WSL clock and the NTFS file metadata); this is environment-only and does not affect compilation correctness — every translation unit recompiled cleanly. |
| FSM contract test (targeted run) | PASS — 93 assertions PASS / 0 FAIL across 20 test cases. ctest reports `100% tests passed, 0 tests failed out of 1`. |
| Full host suite regression | PASS — `100% tests passed, 0 tests failed out of 21`. All 20 pre-existing Phase 4-6 contract tests still pass; new `robotos_fw_fsm_contract` joins the suite. |

### F.4 Log

`RobotOS_v1.0/tests/host/logs/phase_12E_host_2026-05-12.log` — 160 lines,
160-line ctest verbose transcript of the FSM contract run, ending with:

```
=== Summary: 93 passed, 0 failed ===
1/1 Test #21: robotos_fw_fsm_contract ..........   Passed    0.06 sec
100% tests passed, 0 tests failed out of 1
```

The log is tracked and committed alongside the Phase 12E artifacts.

### F.5 Phase 12D syntax-check finding resolution

Phase 12D recorded `SYNTAX_CHECK_NOT_RUN_TOOLCHAIN_OUTPUT_SUPPRESSED` due
to the local MSYS2 MinGW64 silent-failure issue. Phase 12E uses gcc
13.3.0 under WSL Ubuntu, which produces real diagnostics and a real
binary. The header's C99 compatibility is now compile-validated by
transitive inclusion in `robotos_fw_fsm.c` and `test_robotos_fw_fsm.c`.
The Phase 12D open question is resolved.

---

## G. Scope Guard Audit

| Guard | Status |
|---|---|
| No devkit integration | Verified — no file under `devkit/src/` modified; no link from `devkit/CMakeLists.txt` to `framework/` |
| No UART command added or modified | Verified — no parser/shell change; command set `a/s/r/?/x/v/L/d/T` zero-diff |
| No hardware run | Verified — Phase 12E close is `CLOSED_WITH_HOST_TEST_EVIDENCE`, not `CLOSED_WITH_HARDWARE_EVIDENCE` |
| No Scheduler 7A/7B work | Verified — `DEFER` preserved |
| No F407 / custom board work | Verified — `HOLD/DEFER` preserved |
| No legacy Architecture B modification | Verified — `src/`, `include/robotos/`, root `CMakeLists.txt` zero-diff |
| No `core/` or `platform/` mutation | Verified — both zero-diff |
| No Zephyr / `prj.conf` / board change | Verified — zero-diff |
| No `devkit_app_state` change (scope-guard #11) | Verified — file zero-diff; not promoted, not replaced, not copied |
| All 12 UART TX scope-guard constraints from `PHASE_9EZ_CHECKPOINT.md §H` | Preserved |
| `framework/robotos_fw_fsm.h` zero-diff (LOCKED-AT-12D held) | Verified — `git diff` shows zero hunks on the header |
| `framework/README.md` zero-diff | Verified |
| ACTIVE disarm widening not started | Preserved — `USER_DECISION_REQUIRED_ACTIVE_DISARM` |
| POST_FLASH_AUTOSTART discipline unchanged | Verified |
| Application / product layer remains NOT_STARTED | Verified |
| `tests/CMakeLists.txt` (Architecture B legacy) unchanged | Verified — zero-diff |
| `devkit/CMakeLists.txt` (Architecture A active build entry) unchanged | Verified — zero-diff |
| Root `RobotOS_v1.0/CMakeLists.txt` unchanged | Verified — zero-diff |

---

## H. Remaining Gaps / Next Gate

### H.1 What remains before devkit integration

- A Phase 12F-pre (or 13A-pre) **Application bridge planning** phase is
  needed before any devkit consumer of the Framework FSM is built. The
  bridge translates `robotos_event_t` (Adapter events 100-103) into
  `robotos_fw_event_id_t` per Phase 12C `APPLICATION_OWNED_EVENT_BRIDGE_CONFIRMED`.
  Phase 12E does not build this bridge.
- A concrete product/Application layer is still `NOT_STARTED`. Without a
  product, the Framework FSM has no production caller. Phase 12E
  deliberately stops at the host test surface.

### H.2 Phase 12F candidates

| Candidate | Posture |
|---|---|
| **Phase 12F-pre — Application bridge planning** | Recommended next gate when the user is ready to move toward devkit integration. Plans the bridge from Adapter events to Framework event IDs without writing any `.c` file. |
| **Phase 12F — additional FSM host behavior** (extended snapshots, RTT telemetry stream, additional contract cases) | Acceptable if more contract surface is desired before integration; lower priority because the 25-item coverage from Phase 12E-pre is already met. |
| **Phase 12F — small API cleanup** | Acceptable only if the LOCKED-AT-12D header reveals an ergonomic friction during the eventual bridge design. None observed during Phase 12E implementation. |
| **Hold** | Acceptable. Phase 12E delivers a complete, host-test-validated FSM core. The repo can sit here indefinitely until a product is chosen. |

### H.3 Non-recommendations

- **Do not** open devkit integration as the next phase. That would
  collide with scope-guard #11 (`devkit_app_state`) and the
  Application-layer-NOT_STARTED constraint.
- **Do not** open a Phase 12F that touches `framework/robotos_fw_fsm.h`
  unless a concrete consumer reveals an API issue. The header is
  LOCKED-AT-12D and Phase 12E found no need to change it.

---

## I. Decision Result

**`PHASE_12E_FSM_HOST_TEST_IMPLEMENTATION_CLOSED`**
(`CLOSED_WITH_HOST_TEST_EVIDENCE`).

The Framework FSM has its first `.c` body, host-test evidence at 93/93
passing assertions across 20 test cases, no devkit/hardware exposure,
and all Phase 12E-pre gates preserved. Phase 12F is `NOT_STARTED`;
opening it requires explicit user authorization and a chosen direction
from §H.2.

---

## J. Companion Docs

- [`PHASE_12E_PRE_FSM_CONSUMER_TEST_PLAN.md`](PHASE_12E_PRE_FSM_CONSUMER_TEST_PLAN.md) — Plan this phase executes. §F coverage map cross-references this closeout's §E coverage matrix.
- [`PHASE_12D_FSM_HEADER_STUB.md`](PHASE_12D_FSM_HEADER_STUB.md) — Header surface this phase implements. `SYNTAX_CHECK_NOT_RUN` from §F is resolved here.
- [`PHASE_12D_PRE_LEGACY_FRAMEWORK_SCAFFOLD_DISPOSITION.md`](PHASE_12D_PRE_LEGACY_FRAMEWORK_SCAFFOLD_DISPOSITION.md) — Architecture A/B boundary that keeps this phase away from `src/`, `include/robotos/`, root `CMakeLists.txt`, and `RobotOS_v1.0/tests/` legacy root.
- [`PHASE_12C_FSM_EVENT_BRIDGE_STATUS_MODEL.md`](PHASE_12C_FSM_EVENT_BRIDGE_STATUS_MODEL.md) — Source of every contract item in §E.
- [`PHASE_12B_FRAMEWORK_FSM_API_DRAFT.md`](PHASE_12B_FRAMEWORK_FSM_API_DRAFT.md) — Source of init/snapshot/reset contracts (§E items 1-3, 10-13).
- [`PHASE_11Z_COMMAND_SET_CHECKPOINT.md`](PHASE_11Z_COMMAND_SET_CHECKPOINT.md) — Frozen 9-command set unchanged by this phase.
- [`PHASE_9EZ_CHECKPOINT.md`](PHASE_9EZ_CHECKPOINT.md) — 12 UART TX scope guards preserved.
- [`../03_SPECS/FRAMEWORK_FSM_API_DRAFT.md`](../03_SPECS/FRAMEWORK_FSM_API_DRAFT.md) — Long-lived spec; revision marker advanced to Phase 12E; §1 decision-state table includes `IMPLEMENTED_AT_12E` row.

---

## K. Verdict

**`PHASE_12E — CLOSED_WITH_HOST_TEST_EVIDENCE`** with decision
`PHASE_12E_FSM_HOST_TEST_IMPLEMENTATION_CLOSED`. Framework FSM core is
implemented, host-test-validated at 93/93 assertions, free of
heap/UART/dispatcher/legacy coupling, and entirely isolated from
devkit, hardware, and command-set surface. All open gates from §B are
preserved unchanged. Phase 12F remains `NOT_STARTED`.
