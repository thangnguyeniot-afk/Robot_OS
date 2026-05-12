# Phase 12E-pre — Framework FSM Consumer / Test Plan (`CLOSED_DOCS_ONLY`)

**Status:** `CLOSED_DOCS_ONLY`
**Decision result:** `PHASE_12E_RECOMMEND_HOST_UNIT_TEST_CONSUMER`
**Type:** Docs-only planning phase. **No source, runtime, test, CMake,
Zephyr, board, `prj.conf`, DTS overlay, evidence log, Framework header,
Framework `.c` file, or devkit integration change.**
**Date opened/closed:** 2026-05-12 (same-day docs-only close)
**Published baseline at open:** `origin/master = 87e0626`
**Prior closed phase:** Phase 12D (`CLOSED_HEADER_STUB_ONLY`;
`PHASE_12D_FSM_HEADER_STUB_CREATED`).
**Authorizing user instruction:** "OPEN PHASE 12E-pre — FRAMEWORK FSM
CONSUMER / TEST PLAN, DOCS ONLY" (this session).

---

## A. Executive Summary

Phase 12E-pre selects the **validation and consumer path** for the
first Framework implementation phase (Phase 12E). It is a docs-only
gate — no `.c` file is created, no CMake change, no Framework header
change, no devkit integration, no test directory change. Phase 12E
remains `NOT_STARTED` until the user explicitly authorizes it.

The phase audits the repo's existing host test infrastructure and
evaluates five candidate consumer paths against safety (boundary
fidelity, scope-guard preservation), feasibility (does the validation
toolchain work?), and evidentiary value (does it actually exercise the
FSM contract?). The clear winner is the host unit-test path against
the existing Architecture-A host test build at
`RobotOS_v1.0/tests/host/`.

**Decision: `PHASE_12E_RECOMMEND_HOST_UNIT_TEST_CONSUMER`.** Phase 12E,
when authorized, should add `RobotOS_v1.0/framework/robotos_fw_fsm.c`
and a single new host test target inside the existing
`RobotOS_v1.0/tests/host/CMakeLists.txt`. No devkit integration. No
hardware. No command-set change. No `devkit_app_state` mutation.

---

## B. Baseline Before Phase 12E-pre

| Item | Value |
|---|---|
| Published baseline at open | `origin/master = 87e0626` |
| Prior closed phase | Phase 12D (`CLOSED_HEADER_STUB_ONLY`) |
| Active Framework path | `RobotOS_v1.0/framework/` (exists; sibling of `core/`, `platform/`, `devkit/`) |
| Active Framework header | `RobotOS_v1.0/framework/robotos_fw_fsm.h` (declarations only; LOCKED-AT-12D) |
| Framework implementation | **NOT BUILT.** No `.c` body exists. |
| Framework consumer | **None.** No devkit code, host test, or application uses the header. |
| Phase 12E status | `NOT_STARTED` |
| Architecture A | `core/` + `platform/` + `devkit/` + `framework/` — canonical active stack |
| Architecture B | `src/` + `include/robotos/` + root `CMakeLists.txt` — frozen legacy scaffold |
| Architecture A host tests | `RobotOS_v1.0/tests/host/` — standalone CMake, ~20 tracked contract targets, ctest, log convention; evidence-backed through Phase 6 |
| Architecture B host tests | `RobotOS_v1.0/tests/` root (`test_gcode_parser`, `test_motion_planner`, `test_kinematics_cartesian`, `test_app_sm`) — frozen with the legacy scaffold; uses `include/robotos/` and `src/app/` |
| Validated command set | `a / s / r / ? / x / v / L / d / T` (unchanged at this phase) |
| Open gates preserved | ACTIVE disarm widening = `USER_DECISION_REQUIRED_ACTIVE_DISARM`; Scheduler 7A/7B = `DEFER`; F407 / custom board = `HOLD/DEFER`; POST_FLASH_AUTOSTART = `OPEN` / `MITIGATED_BY_WORKFLOW`; Application/product layer = `NOT_STARTED` |
| Phase 12D syntax check finding | `SYNTAX_CHECK_NOT_RUN_TOOLCHAIN_OUTPUT_SUPPRESSED` — local MSYS2 gcc 15.1.0 returned exit=1 with 0-byte diagnostics on every invocation, including a deliberately broken control source. This makes the local Windows MSYS2 toolchain **not authoritative** for Phase 12E validation. The existing host test CMake explicitly notes "Windows MinGW64 may be broken; use WSL Ubuntu or Linux host" — Phase 12E must use a reliable backend (WSL Ubuntu or Linux). |

---

## C. Candidate Consumer / Test Paths

Each option is evaluated against the same axes. **Files likely touched**
describes the **future Phase 12E** patch surface; nothing is touched in
Phase 12E-pre.

### Option 1 — Host unit-test consumer (extend `tests/host/`)

| Axis | Value |
|---|---|
| Objective | Implement `framework/robotos_fw_fsm.c` and validate it through a single new host-test executable that links against the FSM `.c`, the relevant `core/` source (for `robotos_core_status_t`), and the existing platform host stubs. |
| Files likely touched in future Phase 12E | NEW `RobotOS_v1.0/framework/robotos_fw_fsm.c`; UPDATE `RobotOS_v1.0/tests/host/CMakeLists.txt` (add one `add_executable`/`add_test` block); NEW `RobotOS_v1.0/tests/host/test_robotos_fw_fsm.c`; NEW `RobotOS_v1.0/tests/host/logs/phase_12E_host_<date>.log` (evidence); docs (progress + closeout + CURRENT_STATE + INDEX + spec update). |
| Validation path | Standalone CMake: `cmake -S RobotOS_v1.0/tests/host -B build-host-fw && cmake --build build-host-fw && ctest --test-dir build-host-fw --output-on-failure`. Log captured via existing `save_test_log.cmake` target. Run on WSL Ubuntu or Linux (MSYS2 explicitly noted as unreliable in the host CMake header). |
| Benefits | (1) Reuses proven Architecture-A test infra from Phase 4-6. (2) No devkit drift — host build is isolated from Zephyr, devkit, board files. (3) Direct contract exercise: every Phase 12B/12C decision becomes a test assertion. (4) Catches Phase 12D syntax-check unknown — the host CMake will compile the header through a real translation unit on a working toolchain. (5) Tracked log convention provides durable evidence. |
| Risks | Toolchain dependency on WSL/Linux (mitigation: this is already the documented convention for `tests/host/`, exercised through Phase 6). Possible coupling to `robotos_platform_critical_*` if the FSM uses critical section for ISR-safe getters (mitigation: `tests/host/robotos_platform_critical_host_stub.{c,h}` already exists). |
| Boundary impact | None to Architecture B (`src/`, `include/robotos/`, root CMake untouched). None to devkit. None to runtime. None to command set. None to scheduler/F407. |
| Requires CMake change? | Yes — additive only, inside existing `tests/host/CMakeLists.txt`. No new top-level CMake file. No `framework/CMakeLists.txt` needed (host test can `add_executable(... ../framework/robotos_fw_fsm.c)` directly). |
| Requires devkit integration? | **No.** |
| Risks command semantics? | **No.** No parser, shell, UART command, or response queue touched. |
| Recommendation | **STRONG YES.** |

### Option 2 — Compile-only skeleton consumer

| Axis | Value |
|---|---|
| Objective | Implement `framework/robotos_fw_fsm.c` with no-op or minimal stub function bodies whose only purpose is to satisfy the linker. Compile via host or devkit. No behavioral test cases. |
| Files likely touched in future Phase 12E | NEW `RobotOS_v1.0/framework/robotos_fw_fsm.c` (no-op bodies); UPDATE `tests/host/CMakeLists.txt` to compile the TU (no executable target needed beyond link check); docs. |
| Validation path | Standalone compile via the same host CMake; no `ctest` assertions. |
| Benefits | Closes the link-time gap Phase 12D leaves open. Confirms the header compiles through a real TU. |
| Risks | (1) Implements behavior incorrectly or skips it entirely → dead Framework code with no behavioral validation. (2) Sets the wrong precedent: an "implementation phase" without behavior tests. (3) If never followed by a behavioral phase, the stubs may drift away from spec without detection. (4) Wastes one phase budget on a step that Option 1 absorbs for free. |
| Boundary impact | Same as Option 1 (none to Architecture B, none to devkit). |
| Requires CMake change? | Yes — additive only. |
| Requires devkit integration? | No. |
| Risks command semantics? | No. |
| Recommendation | **NO.** Option 1 includes Option 2's benefit (compile through a TU) without its risk (no behavior validation). |

### Option 3 — Devkit integration consumer

| Axis | Value |
|---|---|
| Objective | Wire the FSM into the devkit runtime. Phase 12E adds a Framework FSM instance to devkit, possibly displacing or shadowing `devkit_app_state`, and dispatches Adapter events through the new FSM. |
| Files likely touched in future Phase 12E | NEW `framework/robotos_fw_fsm.c`; likely NEW `framework/CMakeLists.txt`; MODIFICATIONS to `devkit/CMakeLists.txt`; MODIFICATIONS to devkit runtime files; possible duplicate FSM state alongside `devkit_app_state`; possible command/UART surface change to expose FSM observability. |
| Validation path | Hardware: flash STM32F411E-DISCO, run UART session, capture RTT log. Requires preflight, hardware availability, J-Link, OpenOCD, etc. |
| Benefits | High end-to-end realism. Validates Application bridge concept against real Adapter events. |
| Risks | (1) **Scope-guard #11 collision** — `devkit_app_state` is documented as devkit-local, not Framework; this option creates pressure to promote or replace it. (2) Command semantics drift — once FSM observes events, exposing its state via `?` or a new UART command is the natural next ask, but the command set is frozen. (3) Hardware-evidence phase scope — implementation + integration + hardware run is too much for one phase. (4) Premature: Application/product layer is `NOT_STARTED`; this option implicitly creates an application bridge with no product. (5) Toolchain risk on Windows is the same as the Phase 12D syntax-check failure but for a runtime build. |
| Boundary impact | High. Touches devkit + command set + possibly Application layer. |
| Requires CMake change? | Yes, in devkit/CMakeLists.txt (forbidden by Phase 12E-pre boundary). |
| Requires devkit integration? | Yes, by definition. |
| Risks command semantics? | Yes — high. |
| Recommendation | **NO** at Phase 12E. Possibly later, after host-tested implementation + an explicit Application bridge planning phase. |

### Option 4 — Application bridge prototype

| Axis | Value |
|---|---|
| Objective | Create an explicit Application bridge translating `robotos_event_t` (Adapter) → `robotos_fw_event_id_t` (Framework). Phase 12E becomes a 3-layer build: FSM impl + bridge + minimal product. |
| Files likely touched in future Phase 12E | NEW `framework/robotos_fw_fsm.c`; NEW Application-layer code (likely under a new `RobotOS_v1.0/app/` path that **does not yet exist**); CMake changes to wire it all; devkit consumer to drive it. |
| Validation path | Composite — host tests for FSM, host or runtime tests for bridge, devkit run for end-to-end. |
| Benefits | Closes Phase 12C's stated bridge concept end-to-end. |
| Risks | Application/product layer is `NOT_STARTED` and no product has been chosen. Creating a "minimal product" inside Phase 12E commits the repo to a specific product semantics with no design phase. Scope explosion — three layers in one phase. |
| Boundary impact | Very high. Implicitly opens the Application layer. |
| Requires CMake change? | Yes, in multiple places. |
| Requires devkit integration? | Yes (for end-to-end). |
| Risks command semantics? | Yes — Application layer typically defines commands. |
| Recommendation | **NO** at Phase 12E. Likely a future Phase 13-class plan. |

### Option 5 — Hold

| Axis | Value |
|---|---|
| Objective | Keep `robotos_fw_fsm.h` as a design artifact. Do not implement until a concrete external consumer (hypothetical product) requires it. |
| Files likely touched in future Phase 12E | None — the phase is deferred indefinitely. |
| Validation path | None until the consumer appears. |
| Benefits | Zero risk of dead Framework code. Lowest cost. Preserves all gates without action. |
| Risks | The header remains link-broken with no behavioral validation. The Phase 12D `SYNTAX_CHECK_NOT_RUN` finding is never resolved. The Phase 12B/12C decisions remain untested against a working implementation, so latent contract errors (e.g. the Phase 12B → 12C evaluation-order correction) may have analogues that surface only at implementation time. Indefinite hold has its own cost. |
| Boundary impact | None. |
| Requires CMake change? | No. |
| Requires devkit integration? | No. |
| Risks command semantics? | No. |
| Recommendation | **Acceptable as a fallback** if user wants to defer further. Not the strongest option because the host-test path is low-risk and resolves the open syntax-check question with high value. |

---

## D. Recommended Path

**`PHASE_12E_RECOMMEND_HOST_UNIT_TEST_CONSUMER`.**

Phase 12E, when authorized, should:

1. Implement `RobotOS_v1.0/framework/robotos_fw_fsm.c` against the
   Phase 12D header surface, encoding the Phase 12C evaluation order
   (exit → state update → action → entry) and the Phase 12C action
   non-OK no-rollback rule.
2. Add a single host test target inside the existing
   `RobotOS_v1.0/tests/host/CMakeLists.txt` (no new top-level CMake
   file; no `framework/CMakeLists.txt`). The new target compiles the
   FSM `.c` plus a new `tests/host/test_robotos_fw_fsm.c` plus the
   `robotos_platform_critical_host_stub.{c,h}` already in the test
   tree.
3. Validate on a reliable toolchain (WSL Ubuntu or Linux), **not**
   the local MSYS2 MinGW64 path that Phase 12D found unusable.
4. Capture the test log via the tracked `save_test_log.cmake`
   convention and commit it as evidence.
5. **Not** integrate with devkit. **Not** modify `devkit_app_state`.
   **Not** modify `devkit/CMakeLists.txt`. **Not** add or remove
   UART commands. **Not** change command semantics.

Rationale: this is the lowest-risk path that resolves Phase 12D's open
syntax-check question, exercises every confirmed Phase 12B/12C
behavioral decision, reuses an evidence-backed test infrastructure
proven through Phase 4-6, and preserves every open gate without
expansion.

---

## E. Proposed Phase 12E Scope (If Host Unit Test Is Chosen)

This section describes the **future** Phase 12E boundary. Phase 12E-pre
creates none of these files.

### E.1 Future approved files for Phase 12E

| Path | Kind | Notes |
|---|---|---|
| `RobotOS_v1.0/framework/robotos_fw_fsm.c` | NEW source | Implementation of the six Phase 12D-LOCKED functions; no other public symbols; static helpers permitted. |
| `RobotOS_v1.0/tests/host/test_robotos_fw_fsm.c` | NEW host test | Test harness exercising every contract item in §F. |
| `RobotOS_v1.0/tests/host/CMakeLists.txt` | UPDATE (additive only) | Add one `add_executable` block + one `add_test` line. Pattern: identical to the existing 20-target host test convention. **No structural change.** |
| `RobotOS_v1.0/tests/host/logs/phase_12E_host_<YYYY-MM-DD>.log` | NEW tracked log | Evidence; produced by `save_test_log.cmake`. |
| `RobotOS_v1.0/devkit/docs/02_PHASE_CLOSEOUTS/PHASE_12E_FSM_HOST_TEST_IMPLEMENTATION.md` | NEW closeout | Phase 12E evidence record. |
| `RobotOS_v1.0/devkit/docs/01_PROGRESS/DEVKIT_PROGRESS_PHASE_11_20.md` | UPDATE | Add Phase 12E index row + anchor + section. |
| `RobotOS_v1.0/devkit/docs/03_SPECS/FRAMEWORK_FSM_API_DRAFT.md` | UPDATE (small) | Mark §4 as `IMPLEMENTED_AT_12E` for the names that gained bodies; preserve LOCKED-AT-12D wording for compatibility. |
| `RobotOS_v1.0/devkit/docs/00_INDEX/README.md` | OPTIONAL UPDATE | Add Phase 12E closeout link. |
| `CURRENT_STATE.md` | UPDATE | Last closed phase = Phase 12E. |

### E.2 Files NOT touched by Phase 12E

- `RobotOS_v1.0/framework/robotos_fw_fsm.h` (LOCKED-AT-12D; Phase 12E
  may rely on the header but must not change it; if a change is
  required, that is a header-API revision phase, not Phase 12E).
- `RobotOS_v1.0/framework/README.md`.
- Anything under `RobotOS_v1.0/core/`, `RobotOS_v1.0/platform/`,
  `RobotOS_v1.0/devkit/src/`, `RobotOS_v1.0/devkit/boards/`,
  `RobotOS_v1.0/devkit/zephyr/`, `RobotOS_v1.0/devkit/CMakeLists.txt`.
- Anything under `RobotOS_v1.0/src/`, `RobotOS_v1.0/include/`.
- Root `RobotOS_v1.0/CMakeLists.txt` (legacy Architecture B).
- `RobotOS_v1.0/tests/CMakeLists.txt` (legacy Architecture B test
  build for `src/app/`; frozen alongside the legacy scaffold).
- `RobotOS_v1.0/tests/test_*.c` legacy tests (frozen).
- `prj.conf`, DTS files, board overlays.
- Host tools and runtime scripts.
- Any existing evidence log (new Phase 12E log added; no log modified).

### E.3 Future Phase 12E validation surface

| Layer | Tool | Pass criterion |
|---|---|---|
| Header reachability | `gcc` on WSL Ubuntu via the host CMake | `#include "robotos_fw_fsm.h"` compiles cleanly with `-Wall -Wextra -std=c99` |
| Implementation link | CMake host test target | `add_executable(test_robotos_fw_fsm test_robotos_fw_fsm.c ../../framework/robotos_fw_fsm.c ...)` links without unresolved symbols |
| Behavioral contract | ctest | Every contract item in §F passes |
| Determinism | ctest | Two consecutive test runs produce identical pass/fail set |
| Memory | code review (no heap is part of the contract) | No `malloc`/`calloc`/`realloc`/`free` reference in `robotos_fw_fsm.c` |
| ABI surface | grep of public symbols | Only the six Phase 12D-LOCKED functions are exported |

### E.4 Future Phase 12E non-goals (deferred)

| Item | Deferred to |
|---|---|
| `devkit_app_state` replacement / promotion | Not part of Phase 12E. Out of scope at least through Phase 13. |
| Application bridge prototype | Future planning phase (12F-pre or 13A). |
| UART command exposure of FSM state | Out of scope; command set is frozen. |
| Hardware validation of FSM | Future phase once a devkit consumer exists. |
| Framework sensor / actuator / PID / motion APIs | Out of scope; Phase 12 series is FSM only. |
| `ROBOTOS_FW` RTT telemetry stream | Future phase. |
| Scheduler 7A/7B | DEFER. |
| F407 / custom board | HOLD/DEFER. |
| ACTIVE disarm widening | USER_DECISION_REQUIRED_ACTIVE_DISARM. |

---

## F. Minimal Behavior Coverage Required Before Phase 12E Can Close

Phase 12E may close as `CLOSED_WITH_HOST_TEST_EVIDENCE` only when the
host test target passes every case below. Cases map directly to
Phase 12B/12C decisions so coverage is auditable.

| # | Test | Maps to |
|---|---|---|
| 1 | `init` with valid config returns OK; `initialized` becomes true; `current_state == initial_state`; entry callback (if any) called once with `initial_state`. | Phase 12B init contract |
| 2 | `init` with NULL fsm / NULL config / NULL transitions returns `ROBOTOS_CORE_ERR_NULL`. | Phase 12B init guards |
| 3 | `init` with `transition_count == 0` or `initial_state == ROBOTOS_FW_STATE_UNINIT` returns `ROBOTOS_CORE_ERR_INVALID_ARG`. | Phase 12B init guards |
| 4 | `dispatch` of a matched event with no guard runs `exit(from) → state update → action(from, to, ev, payload, ctx) → entry(to)` in that order. | Phase 12C evaluation order |
| 5 | First-match FIFO: when multiple rows match, the first row in the array wins; subsequent rows are not evaluated. | Phase 12B determinism |
| 6 | Guard returning `false` skips the row; `guard_rejected_count++`; scan continues to next row. | Phase 12C guard semantics |
| 7 | No matching row at all → `no_transition_count++`; returns `ROBOTOS_CORE_OK`; `transition_count` unchanged; `current_state` unchanged. | Phase 12C no-transition path |
| 8 | All matching rows have rejecting guards → `guard_rejected_count` incremented per rejection; `no_transition_count++` once at the end; counters are **independent**. | Phase 12C counter independence |
| 9 | Action returns non-OK → state IS already updated; entry runs anyway; `transition_count++`; `dispatch()` returns the non-OK status; `last_status` records it. | Phase 12C `ACTION_NON_OK_NO_ROLLBACK_CONFIRMED` |
| 10 | `reset` from any state returns to `initial_state`, zeroes all counters, calls `exit(prev)` then `entry(initial)`; no action runs during reset. | Phase 12B reset contract |
| 11 | `get_state` on uninitialized / NULL → returns `ROBOTOS_FW_STATE_UNINIT`. | Phase 12B query safety |
| 12 | `is_in_state` matches `get_state`; returns false for NULL / uninit. | Phase 12B query safety |
| 13 | `get_snapshot` copies all eight visible counters + `current_state` + `last_event_id` + `last_status` + `initialized`; on NULL out → `ROBOTOS_CORE_ERR_NULL`. | Phase 12B snapshot contract |
| 14 | `transition_count` increments by exactly one per committed transition; never on no-transition or guard-rejected paths. | Phase 12C counter semantics |
| 15 | `event_count` increments once per `dispatch()` call regardless of match. | Phase 12C counter semantics |
| 16 | `last_event_id` after `dispatch(fsm, EV, _)` equals `EV`. | Phase 12B observability |
| 17 | Payload pointer is NOT cached: pass a payload, return from dispatch, mutate the buffer; subsequent `get_snapshot` shows no stored payload reference. (Cannot inspect callbacks' historical pointer directly, but the FSM struct must not contain a `void *last_payload` field; verified by struct inspection and the absence of any payload-read API.) | Phase 12C `PAYLOAD_BORROWED_FOR_DISPATCH_ONLY_CONFIRMED` |
| 18 | Guard sees the **pre-transition** `current_state` (= `from`). | Phase 12C order |
| 19 | Action sees the **post-transition** `current_state` (= `to`); `from_state` parameter equals the pre-transition state. | Phase 12C order |
| 20 | Entry sees the **post-transition** `current_state` (= `to`). | Phase 12C order |
| 21 | Exit sees the **pre-transition** `current_state` (= `from`). | Phase 12C order |
| 22 | Re-initializing an already-initialized FSM is either explicitly defined (idempotent or error) — Phase 12E must commit to one and document it. | Phase 12E design decision |
| 23 | No heap: `robotos_fw_fsm.c` does not include `<stdlib.h>` (or includes it only for `size_t` and uses no allocator); reviewed by grep. | Phase 12B no-heap constraint |
| 24 | No UART TX call in `robotos_fw_fsm.c` (grep for `printf`, `puts`, `uart`, `tx`). | Phase 9EZ §H scope guards |
| 25 | No `robotos_core_register_event_handler` call from FSM code. | Phase 12C `APPLICATION_OWNED_EVENT_BRIDGE_CONFIRMED` |

Cases 22-25 are review-driven; the rest are runtime-asserted by the
host test executable. Pass criterion: ctest reports all asserted cases
pass; review cases are recorded in the Phase 12E closeout with grep
output as evidence.

---

## G. Test Infrastructure Audit Requirement

**Finding: `TEST_INFRA_AUDIT_NOT_NEEDED — EXISTING_HOST_TESTS_SUFFICE`.**

A repo audit was performed at Phase 12E-pre and confirms:

- `RobotOS_v1.0/tests/host/CMakeLists.txt` exists, is tracked, is
  standalone (no Zephyr / west / legacy `src/` dependency), and is
  documented as the canonical Architecture-A host test build.
- The file contains ~20 `add_executable` + `add_test` blocks for
  Phase 4-6 contract tests (`robotos_core_contract_test`,
  `robotos_event_queue_contract_test`,
  `robotos_event_dispatcher_contract_test`,
  `robotos_platform_critical_contract_test`,
  `robotos_platform_fault_contract_test`,
  `robotos_scheduler_admission_contract_test`,
  `robotos_handler_routing_stress_contract_test`,
  `robotos_handler_lifecycle_contract_test`,
  `robotos_queue_pressure_contract_test`, and more).
- Tracked test logs under `RobotOS_v1.0/tests/host/logs/` demonstrate
  the convention works:
  `host_2026-05-03.log`, `host_2026-05-07.log`,
  `phase_4K_host_2026-05-02.log`, `phase_4L_host_2026-05-03.log`,
  `phase_5D_host_2026-05-02.log` ... `phase_6H_host_2026-05-02.log`,
  `phases_4C-4J_host_2026-05-02.log`.
- The log-capture pattern (`save_test_log.cmake`) is tracked.
- Platform host stubs (`robotos_platform_critical_host_stub.{c,h}`,
  `robotos_platform_fault_host_stub.{c,h}`,
  `robotos_platform_log_host_stub.c`,
  `robotos_platform_time_host_stub.c`) are tracked and reusable.
- The CMake header explicitly states "Windows MinGW64 may be broken;
  use WSL Ubuntu or Linux host" — same finding as the Phase 12D
  syntax check. Phase 12E validation must use a reliable backend.

The four untracked tests under `RobotOS_v1.0/tests/host/`
(`README.md`, `GLM_TEST_PROPOSAL.md`, `glm_compiled_test_draft_not_wired.c`,
`glm_test_skeleton_not_compiled.c`) are **not authoritative** for this
plan (per task instructions). The recommendation is grounded only in
tracked files.

The four legacy host tests at `RobotOS_v1.0/tests/` root
(`test_app_sm.c`, `test_gcode_parser.c`, `test_kinematics_cartesian.c`,
`test_motion_planner.c`) and `RobotOS_v1.0/tests/CMakeLists.txt` belong
to Architecture B (they consume `include/robotos/` and `src/app/`) and
are **classified as part of the frozen legacy scaffold** by extension
of the Phase 12D-pre disposition. Phase 12E must not modify them; they
are not the active Architecture-A test infrastructure.

**Conclusion:** Phase 12E can proceed directly. No Phase 12E-test-pre
infrastructure-creation phase is needed. The recommended Phase 12E
patch surface is purely additive (one `add_executable`/`add_test`
inside an existing file, one new `.c` test, one new FSM `.c`, one new
log) and follows the established convention exactly.

---

## H. Non-goals

Phase 12E-pre does **not**:

- Create any `.c` or `.h` source file.
- Modify `RobotOS_v1.0/framework/robotos_fw_fsm.h`.
- Modify `RobotOS_v1.0/framework/README.md`.
- Add or modify any CMake file (root, devkit, tests, framework).
- Add or modify any Zephyr config (`prj.conf`, board DTS, overlays).
- Modify any tracked test file.
- Modify or extend any evidence log.
- Modify `devkit_app_state`.
- Move, copy, or promote `devkit_app_state`.
- Change command semantics or add UART commands.
- Integrate Framework with devkit.
- Start ACTIVE disarm widening.
- Reopen Scheduler 7A/7B.
- Reopen F407 / custom board.
- Start Application/product work.
- Add parser, shell, registry, framing, or response queue.
- Modify Architecture B (`src/`, `include/robotos/`, root CMake).
- Open Phase 12E (Phase 12E remains `NOT_STARTED`).

---

## I. Decision Result

**`PHASE_12E_RECOMMEND_HOST_UNIT_TEST_CONSUMER`.**

Phase 12E, when authorized, should follow Option 1: implement
`framework/robotos_fw_fsm.c`, add a single host test target inside the
existing `tests/host/CMakeLists.txt`, and validate on WSL Ubuntu or
Linux. No devkit integration. No `devkit_app_state` modification. No
command-set change. No hardware run. No Application/product work.

Alternate acceptable result if the user wants to defer indefinitely:
`PHASE_12E_RECOMMEND_HOLD`. All other options are not recommended.

---

## J. Phase 12E Recommendation

### Title
**Phase 12E — Framework FSM Host-Test Implementation**

### Implementation classification
`HOST_TEST_IMPLEMENTATION_DOCS_AND_EVIDENCE`. Adds:

- the first `.c` body inside `RobotOS_v1.0/framework/`,
- a single new host test executable inside the existing host test
  CMake,
- a tracked test log produced by the existing log convention.

No runtime / hardware / devkit / command / Architecture-B impact.

### Approved files (future Phase 12E)

- NEW: `RobotOS_v1.0/framework/robotos_fw_fsm.c`
- UPDATE (additive only): `RobotOS_v1.0/tests/host/CMakeLists.txt`
- NEW: `RobotOS_v1.0/tests/host/test_robotos_fw_fsm.c`
- NEW: `RobotOS_v1.0/tests/host/logs/phase_12E_host_<YYYY-MM-DD>.log`
- NEW: `RobotOS_v1.0/devkit/docs/02_PHASE_CLOSEOUTS/PHASE_12E_FSM_HOST_TEST_IMPLEMENTATION.md`
- UPDATE: `RobotOS_v1.0/devkit/docs/01_PROGRESS/DEVKIT_PROGRESS_PHASE_11_20.md`
- UPDATE (small): `RobotOS_v1.0/devkit/docs/03_SPECS/FRAMEWORK_FSM_API_DRAFT.md`
- UPDATE: `CURRENT_STATE.md`
- OPTIONAL UPDATE: `RobotOS_v1.0/devkit/docs/00_INDEX/README.md`

### Tests required (Phase 12E exit criteria)

Cases 1-21 in §F asserted by the host test executable and passing
under `ctest --output-on-failure` on WSL Ubuntu or Linux. Cases 22-25
recorded by grep / review evidence in the Phase 12E closeout. Test log
captured via `save_test_log.cmake` and committed.

### CMake boundary

Additive only:

- ONE new `add_executable(test_robotos_fw_fsm ... )` block inside the
  existing `RobotOS_v1.0/tests/host/CMakeLists.txt`.
- ONE matching `add_test(NAME test_robotos_fw_fsm COMMAND test_robotos_fw_fsm)`.
- ZERO new top-level CMake files.
- ZERO change to `RobotOS_v1.0/devkit/CMakeLists.txt`.
- ZERO change to root `RobotOS_v1.0/CMakeLists.txt`.
- ZERO new `framework/CMakeLists.txt`.

### Non-goals (Phase 12E)

- No devkit integration.
- No `devkit_app_state` change.
- No UART command added or modified.
- No `ROBOTOS_FW` RTT telemetry stream.
- No hardware run.
- No Zephyr config change.
- No Architecture B modification.
- No Application/product layer code.
- No Scheduler 7A/7B work.
- No F407 / custom board work.
- No ACTIVE disarm widening.
- No POST_FLASH_AUTOSTART change.

### Exit criteria

`PHASE_12E_FSM_HOST_TEST_IMPLEMENTED` is reachable when:

1. `framework/robotos_fw_fsm.c` exists and compiles cleanly on WSL
   Ubuntu via the existing host CMake.
2. `tests/host/test_robotos_fw_fsm.c` exists and exercises §F cases
   1-21.
3. `ctest --test-dir <build> --output-on-failure` shows
   `test_robotos_fw_fsm` PASS.
4. Test log committed at `tests/host/logs/phase_12E_host_<date>.log`.
5. Review evidence for §F cases 22-25 recorded in the Phase 12E
   closeout.
6. All gates from §B preserved unchanged.

### Rollback / blocker behavior

- If the host build cannot be made to pass on WSL Ubuntu, Phase 12E
  must close `BLOCKED_HOST_TOOLCHAIN` and not commit a half-working
  implementation. Implementation-only commit without ctest evidence is
  not acceptable.
- If a Phase 12B/12C contract item turns out to be implementable only
  with a header change, Phase 12E must close
  `BLOCKED_HEADER_CONTRACT_REVISION_NEEDED` and a small header-revision
  phase (e.g. Phase 12D.1) must run before Phase 12E retries.
- If a §F case reveals a Phase 12B/12C spec ambiguity, the spec is
  authoritative — propose a clarification phase rather than divergent
  implementation choice.

---

## K. Companion Docs

- [`PHASE_12D_FSM_HEADER_STUB.md`](PHASE_12D_FSM_HEADER_STUB.md) — Header stub locked at LOCKED-AT-12D; this plan is the natural successor.
- [`PHASE_12D_PRE_LEGACY_FRAMEWORK_SCAFFOLD_DISPOSITION.md`](PHASE_12D_PRE_LEGACY_FRAMEWORK_SCAFFOLD_DISPOSITION.md) — Architecture A/B boundary that keeps Phase 12E away from `src/`, `include/robotos/`, root CMake, and `RobotOS_v1.0/tests/` root.
- [`PHASE_12C_FSM_EVENT_BRIDGE_STATUS_MODEL.md`](PHASE_12C_FSM_EVENT_BRIDGE_STATUS_MODEL.md) — Source of the contract items in §F (cases 4, 6-9, 14-21).
- [`PHASE_12B_FRAMEWORK_FSM_API_DRAFT.md`](PHASE_12B_FRAMEWORK_FSM_API_DRAFT.md) — Init/snapshot/reset contracts (§F cases 1-3, 10-13, 22-25).
- [`PHASE_11Z_COMMAND_SET_CHECKPOINT.md`](PHASE_11Z_COMMAND_SET_CHECKPOINT.md) — Frozen command set `a/s/r/?/x/v/L/d/T` that Phase 12E must not touch.
- [`PHASE_9EZ_CHECKPOINT.md`](PHASE_9EZ_CHECKPOINT.md) — 12 UART TX scope guards that constrain §F case 24.
- [`../03_SPECS/FRAMEWORK_FSM_API_DRAFT.md`](../03_SPECS/FRAMEWORK_FSM_API_DRAFT.md) — Long-lived spec; §4 is LOCKED-AT-12D; Phase 12E may mark individual functions `IMPLEMENTED_AT_12E` once their `.c` bodies are committed.

---

## L. Verdict

**`PHASE_12E_PRE — CLOSED_DOCS_ONLY`** with recommendation
`PHASE_12E_RECOMMEND_HOST_UNIT_TEST_CONSUMER`.

Phase 12E **remains `NOT_STARTED`** until the user explicitly authorizes
opening it. No file under `framework/`, `tests/host/`, `core/`,
`platform/`, `devkit/`, `src/`, or `include/` is touched by this phase.
All gates from §B are preserved unchanged.
