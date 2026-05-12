# Phase 12D — Framework FSM Header Stub (`CLOSED_HEADER_STUB_ONLY`)

**Status:** `CLOSED_HEADER_STUB_ONLY`
**Decision result:** `PHASE_12D_FSM_HEADER_STUB_CREATED`
**Type:** Header stub + docs. **No `.c` body, no CMake change, no devkit
integration, no Zephyr config change, no hardware run, no source modification
outside the two new framework files, no command-semantic change.**
**Date opened/closed:** 2026-05-12 (same-day docs+header-stub close)
**Published baseline at open:** `origin/master = 2385b0f`
**Prior closed phase:** Phase 12D-pre (`CLOSED_DOCS_ONLY`;
`LEGACY_SCAFFOLD_MARKED_FROZEN_DOCS_ONLY`).
**Authorizing user instruction:** "OPEN PHASE 12D — FRAMEWORK FSM HEADER
STUB, HEADER-ONLY + DOCS" (this session).

---

## A. Executive Summary

Phase 12D creates the first artifact of the **active** Robot Framework
layer under the approved canonical path `RobotOS_v1.0/framework/`. Two
files are added:

1. `RobotOS_v1.0/framework/README.md` — Framework layer identity, layer
   boundary, distinction from frozen legacy Architecture B, scope of
   Phase 12D (header stub only).
2. `RobotOS_v1.0/framework/robotos_fw_fsm.h` — Flat-FSM API header stub
   encoding all Phase 12B/12C confirmed design decisions; declarations
   only, no function bodies, no `static inline` definitions.

No `.c` file is created. No CMake integration is added. No devkit code
is modified. No legacy Architecture B file is modified. No command set
change. No telemetry change. No hardware evidence run.

`devkit_app_state` is unchanged (scope-guard #11 re-affirmed). The
validated command set `a / s / r / ? / x / v / L / d / T` is unchanged.
All 12 UART TX scope-guard constraints from
[`PHASE_9EZ_CHECKPOINT.md`](PHASE_9EZ_CHECKPOINT.md) §H remain in force.

Phase 12D is **closed at the header surface**. Implementation
(`.c` body, CMake wiring, devkit integration, hardware run) is a future
phase (Phase 12E or later) and requires both explicit user authorization
and a concrete consumer or unit-test plan identified in advance.

---

## B. Baseline Before Phase 12D

| Item | Value |
|---|---|
| Published baseline at open | `origin/master = 2385b0f` |
| Prior closed phase | Phase 12D-pre (`CLOSED_DOCS_ONLY`) |
| Phase 12D-pre decision | `LEGACY_SCAFFOLD_MARKED_FROZEN_DOCS_ONLY` |
| Architecture A | `core/` + `platform/` + `devkit/` — canonical active stack |
| Architecture B | `src/` + `include/robotos/` + root `CMakeLists.txt` — frozen legacy scaffold |
| Future active Framework path | `RobotOS_v1.0/framework/` (NOT yet created at the moment Phase 12D opens) |
| Phase 12C-confirmed event bridge | `APPLICATION_OWNED_EVENT_BRIDGE_CONFIRMED` |
| Phase 12C-confirmed status model | `REUSE_ROBOTOS_CORE_STATUS_T_FOR_PHASE_12D_CONFIRMED` |
| Phase 12C-confirmed payload lifetime | `PAYLOAD_BORROWED_FOR_DISPATCH_ONLY_CONFIRMED` |
| Phase 12C-confirmed action non-OK | `ACTION_NON_OK_NO_ROLLBACK_CONFIRMED` |
| Phase 12C-confirmed guard return type | `GUARD_RETURNS_BOOL_ONLY_CONFIRMED` |
| Phase 12C-confirmed evaluation order | exit → state update → action → entry |
| Robot Framework implementation before 12D | **NOT BUILT** |
| Application / product layer | `NOT_STARTED` |
| Validated command set | `a / s / r / ? / x / v / L / d / T` |
| Open gates preserved | ACTIVE disarm widening, Scheduler 7A/7B = DEFER, F407/custom board = HOLD/DEFER, POST_FLASH_AUTOSTART root cause OPEN/MITIGATED_BY_WORKFLOW |

---

## C. Files Created

| Path | Kind | Purpose |
|---|---|---|
| `RobotOS_v1.0/framework/README.md` | New doc (Framework-layer README) | Identity + boundary statement for the active Framework path; distinction from frozen Architecture B; Phase 12D scope. |
| `RobotOS_v1.0/framework/robotos_fw_fsm.h` | New header (DRAFT/EXPERIMENTAL) | Public surface for the flat FSM API; declarations only; encodes Phase 12B/12C decisions in comments and types. |

Phase 12D creates **no other source files**. No `.c`, no
`CMakeLists.txt`, no subdirectory under `framework/`. No file is
modified outside the docs-update set listed in §J below.

Both files live at `RobotOS_v1.0/framework/` — the canonical active
Framework path recorded in Phase 12D-pre. Neither file is placed under
`src/`, `include/robotos/`, `src/framework/`, or any other legacy
Architecture B path.

---

## D. Header Surface Summary

The header `robotos_fw_fsm.h` is C99-compatible, includes only
`<stdbool.h>`, `<stdint.h>`, and `"robotos_core.h"`, and is marked
`DRAFT / EXPERIMENTAL`. Include guard: `ROBOTOS_FW_FSM_H`. The header is
wrapped in `extern "C"` for accidental C++ inclusion.

### D.1 Types

| Type | Underlying | Role |
|---|---|---|
| `robotos_fw_state_id_t` | `uint32_t` | Product-defined state ID; `0` reserved as `ROBOTOS_FW_STATE_UNINIT` sentinel |
| `robotos_fw_event_id_t` | `uint32_t` | Application-defined logical event ID; **separate namespace from `robotos_event_type_t`** |
| `robotos_fw_status_t` | `robotos_core_status_t` (alias) | Return status, reused per Phase 12C decision |

### D.2 Constants

| Macro | Value | Role |
|---|---|---|
| `ROBOTOS_FW_STATE_UNINIT` | `((robotos_fw_state_id_t)0u)` | Reserved sentinel; not a valid product state |

### D.3 Callback signatures

| Typedef | Return | Notes |
|---|---|---|
| `robotos_fw_guard_fn_t` | `bool` | `true` allows, `false` skips row; pure predicate (Phase 12C confirmed) |
| `robotos_fw_action_fn_t` | `robotos_fw_status_t` | Runs after state update; non-OK does **not** roll back |
| `robotos_fw_entry_exit_fn_t` | `robotos_fw_status_t` | Per-state entry / exit; entry runs regardless of action status |

### D.4 Config / row / def structs (caller-owned, typically `const`)

| Struct | Fields |
|---|---|
| `robotos_fw_transition_t` | `current_state`, `event_id`, `guard` (nullable), `next_state`, `action` (nullable) |
| `robotos_fw_state_def_t` | `state_id`, `on_entry` (nullable), `on_exit` (nullable) |
| `robotos_fw_fsm_config_t` | `transitions`, `transition_count`, `states` (nullable), `state_count`, `initial_state`, `user_context` |

### D.5 Instance struct

`robotos_fw_fsm_t` is caller-owned (static allocation expected). Fields:
`config`, `current_state`, `transition_count`, `event_count`,
`guard_rejected_count`, `no_transition_count`, `last_event_id`,
`last_status`, `initialized`. Layout is DRAFT / EXPERIMENTAL — fields
are documented but the application is expected to access them only
through the public API.

### D.6 Snapshot struct

`robotos_fw_fsm_snapshot_t` mirrors the audit-relevant fields of the
instance (excluding `config`). Returned by `robotos_fw_fsm_get_snapshot()`.

### D.7 Function declarations (all DRAFT; no bodies in Phase 12D)

| Function | Purpose | Context |
|---|---|---|
| `robotos_fw_fsm_init` | Initialize caller-owned FSM against caller-owned config | Thread only |
| `robotos_fw_fsm_dispatch` | Deliver a logical event; scan table; run exit → state update → action → entry | Thread only |
| `robotos_fw_fsm_get_state` | Return current state ID (`ROBOTOS_FW_STATE_UNINIT` on NULL/uninit) | ISR-safe (critical section) |
| `robotos_fw_fsm_reset` | Reset to initial state; zero counters; call exit/entry; no action runs | Thread only |
| `robotos_fw_fsm_is_in_state` | Predicate against current state | ISR-safe (critical section) |
| `robotos_fw_fsm_get_snapshot` | Copy state and counters into a caller-supplied struct | Thread only |

All function bodies are intentionally absent in Phase 12D. Linking the
header without a future Phase 12E (or later) implementation will fail at
link time **by design**.

---

## E. Phase 12C Decision Encoding (how the header records each decision)

| Phase 12C decision | Encoded in header |
|---|---|
| `APPLICATION_OWNED_EVENT_BRIDGE_CONFIRMED` | File-level comment + `dispatch` doc: FSM receives `robotos_fw_event_id_t` only; never calls `robotos_core_register_event_handler`; bridge runs in thread context |
| `REUSE_ROBOTOS_CORE_STATUS_T_FOR_PHASE_12D_CONFIRMED` | `typedef robotos_core_status_t robotos_fw_status_t;` plus include of `"robotos_core.h"`; no new status enum |
| Separate Framework event ID namespace | Comment on `robotos_fw_event_id_t`: explicitly states separation from `robotos_event_type_t`; no `ROBOTOS_EVENT_USER` sub-range allocation |
| `PAYLOAD_BORROWED_FOR_DISPATCH_ONLY_CONFIRMED` | `event_payload` declared `const void *` everywhere; doc comment on `dispatch()` states payload is borrowed and must not be cached |
| `ACTION_NON_OK_NO_ROLLBACK_CONFIRMED` | Doc comment on `dispatch()` and `robotos_fw_action_fn_t`: state is committed before action runs; non-OK action does not roll back; entry runs anyway |
| `GUARD_RETURNS_BOOL_ONLY_CONFIRMED` | `robotos_fw_guard_fn_t` returns `bool`; no status-returning variant |
| Evaluation order exit → state update → action → entry | Spelled out in file-level comment and in `dispatch()` doc comment |
| No heap | Doc comments state FSM and config are caller-owned; no allocator API exposed |
| Thread-context dispatch; ISR-safe queries | Each function's doc comment names the allowed context |
| No direct UART TX from FSM | File-level boundary list explicitly forbids UART TX from FSM or any callback |
| No core event-handler registration by FSM | File-level boundary list explicitly forbids `robotos_core_register_event_handler` calls from FSM |

The header does not declare any API that contradicts a Phase 12B/12C
decision. Where the spec leaves a behavior open (e.g. critical-section
implementation strategy for ISR-safe getters), the doc comment names the
expectation but does not impose a body.

---

## F. Syntax / Validation Result

**Status: `SYNTAX_CHECK_NOT_RUN_TOOLCHAIN_OUTPUT_SUPPRESSED`.**

A scratch translation unit including `robotos_fw_fsm.h` was prepared at
a disposable temporary path outside the repo and removed after the
attempt. Command attempted (under both Bash and PowerShell wrappers):

```
gcc -fsyntax-only -std=c99 -Wall -Wextra \
    -IRobotOS_v1.0/framework -IRobotOS_v1.0/core \
    <scratch>.c
```

Toolchain located:
`C:\msys64\mingw64\bin\gcc.exe (Rev5, Built by MSYS2 project) 15.1.0`.

Result: gcc returned exit code 1 on **every** invocation, including a
control test against a deliberately broken C source. In every case both
stdout and stderr captured 0 bytes when redirected through Bash, through
PowerShell, and through `cmd.exe`. The sandbox suppresses gcc's
diagnostics regardless of the redirection style, which means a non-zero
exit cannot be distinguished from a real header error. The check is
therefore not informative and is reported as `NOT_RUN`.

The scratch file was removed; no scratch file is committed.

Reliance: header surface was reviewed by hand for C99 conformance,
include-guard balance, `extern "C"` balance, completeness of typedefs
and forward references, terminator semicolons on all declarations, and
absence of forbidden includes (no Zephyr, no devkit, no `ro_*` legacy
headers, no app/product headers). Header review is documented in §D.

A future implementation phase (Phase 12E or later) MUST re-run a
toolchain-backed compile (preferably CMake + Ninja under the Architecture A
build path) before declaring the implementation valid. Phase 12D does
not claim such a check.

---

## G. Scope Guard Audit

The following constraints were enforced for the duration of Phase 12D
and verified before commit:

| Guard | Status |
|---|---|
| No `.c` file created anywhere | Verified — diff contains 0 `.c` files |
| No `.h` file outside `RobotOS_v1.0/framework/` | Verified — only `framework/robotos_fw_fsm.h` |
| No CMake file created or modified | Verified — `git diff` includes 0 `CMakeLists.txt` |
| No `prj.conf`, DTS overlay, or board file modified | Verified |
| No Zephyr workspace, runtime script, or host tool modified | Verified |
| No file under `core/`, `platform/`, `devkit/src/` modified | Verified |
| No file under `src/` or `include/robotos/` modified after Phase 12D-pre | Verified |
| No file deleted, moved, or renamed | Verified |
| No evidence log under `RobotOS_v1.0/devkit/logs/` modified | Verified |
| `devkit_app_state` unchanged (scope-guard #11) | Verified |
| Validated command set `a/s/r/?/x/v/L/d/T` unchanged | Verified — no parser/handler change |
| All 12 UART TX scope-guard constraints from `PHASE_9EZ_CHECKPOINT.md §H` | Preserved |
| ACTIVE disarm widening not started | Preserved — `USER_DECISION_REQUIRED_ACTIVE_DISARM` |
| Scheduler 7A/7B remains DEFER | Preserved |
| F407 / custom board remains HOLD/DEFER | Preserved |
| POST_FLASH_AUTOSTART discipline unchanged | Preserved |
| Application/product layer remains NOT_STARTED | Preserved |
| Architecture B legacy notices preserved unchanged | Verified — `src/README_LEGACY_SCAFFOLD.md`, `src/framework/DEPRECATED.md`, `include/robotos/DEPRECATED.md` are zero-diff after Phase 12D-pre |
| Root `RobotOS_v1.0/CMakeLists.txt` (legacy) unchanged | Verified — zero-diff |
| `devkit/CMakeLists.txt` (active) unchanged | Verified — zero-diff |
| No new `ROBOTOS_FW` RTT telemetry stream | Verified — no telemetry code path added |
| No new UART command exposed | Verified |
| No new parser/shell/registry/framing/response queue | Verified |

---

## H. Remaining Non-goals / Next Gate

Phase 12D does **not** authorize, schedule, or imply any of:

- An implementation `.c` body for `robotos_fw_fsm`.
- A `framework/CMakeLists.txt`.
- A `framework/src/` subdirectory.
- A link of the Framework library into `devkit/CMakeLists.txt`.
- An `app/` directory at any level.
- A devkit consumer of the Framework FSM.
- A replacement, copy, or promotion of `devkit_app_state`.
- A UART command bound to FSM dispatch.
- A `ROBOTOS_FW` RTT telemetry stream.
- A hardware run, regression run, or evidence log.
- Any change to Architecture B (the frozen legacy scaffold).
- Any reconciliation phase between Architecture A and Architecture B.
- Any change to the Scheduler 7A/7B status.
- Any change to F407 / custom board status.
- Any ACTIVE disarm widening.

A future phase (Phase 12E or later) may open implementation only on
**explicit user authorization** AND with a concrete consumer or unit-test
plan identified before the phase is opened. Without a concrete consumer,
the implementation has no integration test path, which is a self-imposed
gate to prevent dead-Framework-code drift.

---

## I. Decision Result

**`PHASE_12D_FSM_HEADER_STUB_CREATED`** (`CLOSED_HEADER_STUB_ONLY`).

Active Framework path now exists:

```
RobotOS_v1.0/framework/
├── README.md
└── robotos_fw_fsm.h    (DRAFT / EXPERIMENTAL; declarations only)
```

The Phase 12B/12C-confirmed flat FSM API surface is frozen at this
header as `LOCKED-AT-12D` (function names and parameter shapes), with
the explicit caveat that ABI and behavioral guarantees are NOT stable.
The spec
[`../03_SPECS/FRAMEWORK_FSM_API_DRAFT.md`](../03_SPECS/FRAMEWORK_FSM_API_DRAFT.md)
is updated to reflect the lock-at-12D state.

---

## J. Surfaces Touched

### J.1 New files
- `RobotOS_v1.0/framework/README.md`
- `RobotOS_v1.0/framework/robotos_fw_fsm.h`
- `RobotOS_v1.0/devkit/docs/02_PHASE_CLOSEOUTS/PHASE_12D_FSM_HEADER_STUB.md` (this doc)

### J.2 Doc updates
- `RobotOS_v1.0/devkit/docs/03_SPECS/FRAMEWORK_FSM_API_DRAFT.md` — header revision marker, §1 decision-state, §4 names move from DRAFT → LOCKED-AT-12D, §10 next revision condition updated.
- `RobotOS_v1.0/devkit/docs/01_PROGRESS/DEVKIT_PROGRESS_PHASE_11_20.md` — Phase 12D index row added; `<a id="phase-12d"></a>` anchor + section added.
- `CURRENT_STATE.md` — Last closed phase replaced with Phase 12D; Phase 12D-pre demoted to prior-closed entry; gates preserved.
- `RobotOS_v1.0/devkit/docs/00_INDEX/README.md` — Phase 12D closeout link added; progress status row updated.

### J.3 Untouched surfaces (verified zero-diff)
- All `.c` files everywhere in the repo.
- All `CMakeLists.txt` files (root legacy, devkit, any other).
- `core/`, `platform/`, `devkit/src/`, `devkit/boards/`, `devkit/zephyr/`.
- `src/`, `include/robotos/`, `include/app/`.
- `tests/`, `tests/host/`, `RobotOS_v1.0/devkit/logs/`.
- `prj.conf`, DTS files, overlays, board defconfigs.
- Host tools, runtime scripts.
- All evidence logs.
- All prior closeout docs (none rewritten).

---

## K. Companion Docs

- [`PHASE_12D_PRE_LEGACY_FRAMEWORK_SCAFFOLD_DISPOSITION.md`](PHASE_12D_PRE_LEGACY_FRAMEWORK_SCAFFOLD_DISPOSITION.md) — Phase 12D-pre boundary record that made this header safe to place at `RobotOS_v1.0/framework/`.
- [`PHASE_12C_FSM_EVENT_BRIDGE_STATUS_MODEL.md`](PHASE_12C_FSM_EVENT_BRIDGE_STATUS_MODEL.md) — source of the confirmed event bridge / status / payload / action-non-OK / guard / evaluation-order decisions encoded by the header.
- [`PHASE_12B_FRAMEWORK_FSM_API_DRAFT.md`](PHASE_12B_FRAMEWORK_FSM_API_DRAFT.md) — initial DRAFT FSM API; this header locks the names and signatures from the 12B draft as updated by 12C.
- [`PHASE_12A_FRAMEWORK_API_SURFACE_PLANNING.md`](PHASE_12A_FRAMEWORK_API_SURFACE_PLANNING.md) — layer boundary statement justifying a Framework FSM as the first Framework slice.
- [`PHASE_11Z_COMMAND_SET_CHECKPOINT.md`](PHASE_11Z_COMMAND_SET_CHECKPOINT.md) — frozen 9-command set that remains untouched at Phase 12D.

---

## L. Verdict

**`PHASE_12D_FSM_HEADER_STUB_CREATED` — Phase 12D closed at header surface.**

No runtime behavior change. No hardware evidence. No legacy scaffold
modification. No command semantic change. Active Framework path is now
present in the repo with a single public header. All open gates
preserved unchanged. Future implementation requires explicit user
authorization and a concrete consumer.
