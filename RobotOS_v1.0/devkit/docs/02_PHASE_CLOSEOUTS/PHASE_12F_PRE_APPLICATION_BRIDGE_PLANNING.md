# Phase 12F-pre — Application Bridge Planning (`CLOSED_DOCS_ONLY`)

**Status:** `CLOSED_DOCS_ONLY`
**Decision result:** `PHASE_12F_PRE_RECOMMEND_HOST_BRIDGE_PROTOTYPE`
**Type:** Docs-only planning phase. **No source, runtime, test, CMake,
Zephyr, board, `prj.conf`, DTS overlay, evidence log, Framework header,
Framework `.c` file, devkit integration, command-set, or
`devkit_app_state` change.** No file under `framework/`, `tests/`,
`core/`, `platform/`, `devkit/src/`, `src/`, `include/robotos/`
modified.
**Date opened/closed:** 2026-05-12 (same-day docs-only close)
**Published baseline at open:** `origin/master = df9bb8e`
**Prior closed phase:** Phase 12E (`CLOSED_WITH_HOST_TEST_EVIDENCE`;
`PHASE_12E_FSM_HOST_TEST_IMPLEMENTATION_CLOSED`).
**Authorizing user instruction:** "OPEN PHASE 12F-pre — APPLICATION
BRIDGE PLANNING, DOCS ONLY" (this session).

---

## A. Executive Summary

Phase 12F-pre selects the **next implementation gate** for connecting
the host-validated Framework FSM core to a real event source. The FSM
itself was implemented and host-tested in Phase 12E, but it has **no
consumer** — neither the devkit runtime nor an Application layer
dispatches events into it. Phase 12F-pre is the docs-only gate that
decides how the first consumer/bridge should be introduced without
violating scope-guard #11 (`devkit_app_state`), the frozen command set
`a/s/r/?/x/v/L/d/T`, or the `NOT_STARTED` Application/product
boundary.

Five candidate bridge paths are evaluated against safety, feasibility,
boundary impact, and risk to existing devkit runtime. The clear winner
is the **host-only bridge prototype** — a future host-test executable
that exercises Adapter-event → Framework-event-ID translation without
touching any devkit code, UART command, or hardware.

**Decision: `PHASE_12F_PRE_RECOMMEND_HOST_BRIDGE_PROTOTYPE`.** Phase 12F,
when authorized, should add a host-only bridge prototype with its own
host test target inside the existing
`RobotOS_v1.0/tests/host/CMakeLists.txt`. No devkit code. No UART
command. No `devkit_app_state` mutation. No hardware run.

A long-lived bridge spec draft
[`../03_SPECS/FRAMEWORK_APPLICATION_BRIDGE_DRAFT.md`](../03_SPECS/FRAMEWORK_APPLICATION_BRIDGE_DRAFT.md)
is created alongside this planning doc to record the bridge contract
shape under `DRAFT / NON-FINAL`.

---

## B. Baseline Before Phase 12F-pre

| Item | Value |
|---|---|
| Published baseline at open | `origin/master = df9bb8e` |
| Prior closed phase | Phase 12E (`CLOSED_WITH_HOST_TEST_EVIDENCE`) |
| Framework FSM core | **Implemented + host-tested.** `framework/robotos_fw_fsm.c` exists; six public functions implemented; 25/25 Phase 12E-pre contract items satisfied. |
| Host tests | `tests/host/test_robotos_fw_fsm.c` — 20 cases, 93 assertions, targeted PASS; full host regression 21/21 PASS under WSL Ubuntu / gcc 13.3.0. |
| Framework consumer | **None.** No code in the repo calls `robotos_fw_fsm_dispatch()` outside the host test. |
| Active Framework path | `RobotOS_v1.0/framework/` (Architecture A; sibling of `core/`, `platform/`, `devkit/`). |
| devkit integration | `NOT_STARTED` |
| Application / product layer | `NOT_STARTED` |
| Architecture A | `core/` + `platform/` + `devkit/` + `framework/` — canonical active stack |
| Architecture B | `src/` + `include/robotos/` + root `RobotOS_v1.0/CMakeLists.txt` — frozen legacy scaffold |
| Validated command set | `a / s / r / ? / x / v / L / d / T` (unchanged) |
| `devkit_app_state` | Devkit-local state machine; owns IDLE/ARMED/ACTIVE; consumes Phase 9A-B-9D events directly via existing button/UART handlers. **Authoritative for devkit runtime behavior. Scope-guard #11 active.** |
| Existing Adapter event types | `ROBOTOS_EVENT_USER+0` (100, devkit_runtime legacy), `+1` (101, timer producer), `+2` (102, button producer), `+3` (103, UART producer). All in active use; none allocated to Framework. |
| Phase 12C bridge concept | `APPLICATION_OWNED_EVENT_BRIDGE_CONFIRMED` — Application bridge (not FSM) translates Adapter events to Framework event IDs; FSM never calls `robotos_core_register_event_handler` itself. |
| Open gates preserved | ACTIVE disarm widening = `USER_DECISION_REQUIRED_ACTIVE_DISARM`; Scheduler 7A/7B = `DEFER`; F407/custom board = `HOLD/DEFER`; POST_FLASH_AUTOSTART = `OPEN` / `MITIGATED_BY_WORKFLOW`; Application/product layer = `NOT_STARTED`; devkit integration = `NOT_STARTED`. |

---

## C. Bridge Problem Statement

The Framework FSM is product-neutral: it consumes `robotos_fw_event_id_t`
(`uint32_t`) values defined by the application. But every real event
source in the active stack delivers `robotos_event_t` (the core's
Adapter event) with `type ∈ {ROBOTOS_EVENT_USER, USER+1, USER+2, USER+3}`
plus `arg0`/`arg1` payload. A consumer of the FSM therefore needs an
**Application bridge** that:

1. Takes a `robotos_event_t` (or an equivalent host-test stand-in) and
   maps it to a logical `robotos_fw_event_id_t`.
2. Calls `robotos_fw_fsm_dispatch(fsm, event_id, payload)`.
3. Returns the dispatch's status to its caller without inventing a
   new public status enum.

Three forces constrain how this bridge is introduced:

- **Scope-guard #11.** `devkit_app_state` currently owns the only
  application-layer behavior in the devkit runtime. Touching, mirroring,
  or replacing it inside a single phase creates duplicate or conflicting
  state truth — a high-risk move when no product is defined.
- **Command-set stability.** `a/s/r/?/x/v/L/d/T` are frozen; the
  bridge must not expose Framework FSM state through UART or via a new
  command in the first implementation.
- **Application-layer boundary.** Phase 12A-12C-12E-pre all explicitly
  classify the application layer as `NOT_STARTED`. Choosing a product
  vocabulary now would prematurely close design decisions that no user
  has authorized.

Phase 12F-pre therefore plans a bridge that is **product-neutral**,
**host-validated only**, and **isolated from devkit_app_state and the
command set**.

---

## D. Candidate Bridge Paths

Each option is evaluated against the same axes. **Future files
touched** describes the **future Phase 12F** patch surface; nothing is
touched in Phase 12F-pre.

### Option 1 — Host-only application bridge prototype

| Axis | Value |
|---|---|
| Objective | A small bridge module + host test that maps synthetic Adapter-style events to Framework event IDs and calls `robotos_fw_fsm_dispatch()`. Validates the translation contract entirely on host. |
| Future files likely touched in Phase 12F | NEW `RobotOS_v1.0/framework/robotos_fw_event_bridge.h` (small mapping API surface) + NEW `RobotOS_v1.0/framework/robotos_fw_event_bridge.c` (caller-owned mapping table; product-neutral); NEW `RobotOS_v1.0/tests/host/test_robotos_fw_event_bridge.c` (host contract test); additive update to `RobotOS_v1.0/tests/host/CMakeLists.txt` (one `add_executable` + one `add_test`); NEW tracked log `RobotOS_v1.0/tests/host/logs/phase_12F_host_<date>.log`; docs (progress + closeout + CURRENT_STATE + INDEX + spec update). Whether a separate bridge module is justified vs. extending the FSM module is a Phase 12F design call; Phase 12F-pre prefers a **separate module** (`framework/robotos_fw_event_bridge.{c,h}`) so the FSM stays product-neutral and the bridge keeps the mapping table. |
| Validation path | Standalone CMake under WSL Ubuntu / gcc 13.3.0 (same as Phase 12E). Host test asserts: mapped event dispatches, unmapped event is silently ignored, payload pass-through, FSM non-OK propagation, mapping determinism, no UART/hardware/dispatcher coupling. Test log captured via existing `save_test_log.cmake` convention. |
| Benefits | (1) Re-uses Architecture-A host test infra. (2) Zero devkit drift. (3) Zero `devkit_app_state` collision. (4) Zero command-set risk. (5) Validates the Phase 12C `APPLICATION_OWNED_EVENT_BRIDGE_CONFIRMED` contract concretely. (6) Resolves the "who owns the FSM pointer" question (bridge owns it via a caller-passed pointer; see §F). |
| Risks | Toolchain dependency on WSL/Linux (mitigation: identical to Phase 12E; documented as the canonical convention). Risk of inventing product vocabulary inside the host test (mitigation: §F requires neutral test events; no `EVT_ARM` interpretation in the bridge module itself — synthetic events live in the test source only). |
| Boundary impact | None to Architecture B. None to devkit. None to runtime. None to command set. None to scheduler/F407. None to `devkit_app_state`. |
| Touches `devkit_app_state`? | **No.** |
| Changes command semantics? | **No.** |
| Requires hardware? | **No.** |
| Recommendation | **STRONG YES.** |

### Option 2 — Devkit shadow bridge

| Axis | Value |
|---|---|
| Objective | Devkit creates a Framework FSM instance and mirrors existing devkit_app_state event sources into it **without** replacing devkit_app_state behavior. The Framework FSM observes the same events; `devkit_app_state` remains authoritative; the FSM state is exposed only via RTT for comparison. |
| Future files likely touched in Phase 12F | NEW Framework bridge module under `framework/`; UPDATE `devkit/src/devkit_button_producer.c`, `devkit_uart_producer.c`, or a new `devkit/src/devkit_fw_shadow.c` to mirror events; UPDATE `devkit/CMakeLists.txt` to link the bridge + Framework `.c`; possibly a new RTT log line for FSM state. |
| Validation path | Hardware run on STM32F411E-DISCO: flash, capture RTT, confirm FSM mirrors `devkit_app_state` semantics. Cannot validate on host alone because the shadow exists only in the devkit runtime. |
| Benefits | (1) Tests the bridge against real Adapter events. (2) Verifies FSM state tracks devkit_app_state when given the same inputs. |
| Risks | (1) Duplicate state truth — two FSMs (devkit_app_state and Framework FSM) reach the same conclusion through different code paths; any divergence is a silent bug. (2) Devkit producer files must add a new call site, which modifies devkit source — the boundary it crosses is exactly the one Phase 11A and Phase 12D-pre were careful about. (3) Requires `framework/` to be wired into the devkit build (new CMake change). (4) Requires hardware evidence to be meaningful. (5) `devkit_app_state` is not yet documented to expose its events to a Framework consumer; doing so would expand the devkit-local surface against scope-guard #11 spirit. |
| Boundary impact | High — devkit/CMakeLists + devkit/src + framework link. |
| Touches `devkit_app_state`? | Indirectly — the producer files that call into `devkit_app_state` must also call into the bridge. |
| Changes command semantics? | No (semantically). But it introduces a second state machine fed by the command-driven events. |
| Requires hardware? | Yes, for meaningful evidence. |
| Recommendation | **NO** at Phase 12F. Possibly later, after Option 1 host-validates the bridge contract and after an explicit `devkit_fw_shadow` design phase. |

### Option 3 — Devkit replacement bridge

| Axis | Value |
|---|---|
| Objective | Replace `devkit_app_state` with a Framework FSM. The FSM becomes the new devkit state owner. |
| Future files likely touched in Phase 12F | Major surgery on `devkit/src/devkit_app_state.c/h`; producer files; CMake; possible UART response format changes. |
| Validation path | Hardware run; full devkit regression including the `?` UART response shape and the entire command set. |
| Benefits | Single source of truth for application state. |
| Risks | (1) Direct violation of scope-guard #11 — devkit-local state promoted to Framework. (2) `?` UART response is currently frozen and includes devkit-specific names (IDLE/ARMED/ACTIVE); FSM is product-neutral and uses `uint32_t` IDs only; mapping back to the existing response shape requires either devkit-specific name table inside Framework (impossible — Framework is product-neutral) or maintaining a parallel name mapping in devkit (defeats single-source claim). (3) Requires hardware evidence. (4) Risks command-semantics change. (5) No user authorization to remove `devkit_app_state`. |
| Boundary impact | Very high. Touches devkit_app_state, scope-guard #11, command response shape, command set. |
| Touches `devkit_app_state`? | **Replaces it.** |
| Changes command semantics? | Likely yes (response shape, behavior under edge cases). |
| Requires hardware? | Yes. |
| Recommendation | **NO** at Phase 12F. Explicitly NOT recommended even later without a dedicated migration phase that the user explicitly authorizes. |

### Option 4 — Application-layer bridge skeleton

| Axis | Value |
|---|---|
| Objective | Create a new `RobotOS_v1.0/app/` (or `application/`) top-level path that hosts the bridge + a minimal product. Phase 12F becomes a 2-layer build: bridge + minimal product. |
| Future files likely touched in Phase 12F | NEW `RobotOS_v1.0/app/` top-level directory; NEW Application bridge `.c`/`.h`; NEW minimal product `.c`/`.h`; NEW Application CMake or wiring into existing CMakes; host tests + possibly devkit consumer. |
| Validation path | Composite — host tests for bridge + host or runtime tests for product; possibly devkit. |
| Benefits | Architecturally most correct in the long term — closes the Phase 12A-confirmed Application boundary. |
| Risks | (1) Application/product layer is `NOT_STARTED` and no product has been chosen by the user; opening it inside an implementation phase prematurely commits the repo. (2) Scope explosion — bridge + product + tests in one phase. (3) Creates a new top-level directory with no Architecture A precedent (only `core/`, `platform/`, `devkit/`, `framework/` exist today). (4) Better preceded by a Phase 13-class **Application planning** phase that decides whether the application lives at `RobotOS_v1.0/app/` or elsewhere, what the product is, what its command vocabulary looks like, etc. |
| Boundary impact | Very high — implicitly opens the Application/product layer. |
| Touches `devkit_app_state`? | No directly, but creates a competing application layer. |
| Changes command semantics? | Likely — Application layer typically defines product commands. |
| Requires hardware? | Eventually yes. |
| Recommendation | **NO** at Phase 12F. Possibly a future Phase 13-pre once the user picks a product direction. |

### Option 5 — Hold

| Axis | Value |
|---|---|
| Objective | Keep the Framework FSM core host-tested with no consumer. Defer all bridge work until product direction emerges. |
| Future files likely touched in Phase 12F | None. |
| Validation path | None until a consumer appears. |
| Benefits | Zero risk. Lowest cost. Preserves all gates without any further action. |
| Risks | The bridge contract from Phase 12C remains untested against real (or even synthetic) Adapter-event-to-Framework-ID mapping. The next time someone tries to integrate the FSM, they'll re-derive the contract from scratch. Latent gaps in the spec only surface at implementation time. |
| Boundary impact | None. |
| Touches `devkit_app_state`? | No. |
| Changes command semantics? | No. |
| Requires hardware? | No. |
| Recommendation | **Acceptable as a fallback** if the user wants to wait for product direction before committing to Option 1's bridge module. Not the strongest option because Option 1 has identical safety and resolves the bridge contract. |

---

## E. Recommended Bridge Path

**`PHASE_12F_PRE_RECOMMEND_HOST_BRIDGE_PROTOTYPE`.**

Phase 12F, when authorized, should:

1. Add `RobotOS_v1.0/framework/robotos_fw_event_bridge.h` declaring a
   small, **product-neutral**, **caller-owned** mapping API.
2. Add `RobotOS_v1.0/framework/robotos_fw_event_bridge.c` implementing
   the mapping behind the header.
3. Add `RobotOS_v1.0/tests/host/test_robotos_fw_event_bridge.c`
   exercising every contract item in §I.
4. Add **one** new `add_executable`/`add_test` block inside the
   existing `RobotOS_v1.0/tests/host/CMakeLists.txt` (additive only;
   no existing target touched; no new top-level CMake).
5. Validate on WSL Ubuntu / gcc 13.3.0 — same toolchain as Phase 12E.
6. Capture a tracked test log at
   `RobotOS_v1.0/tests/host/logs/phase_12F_host_<date>.log` via the
   existing `save_test_log.cmake` convention.
7. **Not** modify `devkit_app_state`, devkit producers, devkit
   runtime, devkit CMake, root CMake, Architecture B, or any UART
   command.
8. **Not** create `RobotOS_v1.0/app/` or any Application-layer file.
9. **Not** run hardware.

Rationale: this preserves every open gate from §B, exercises the
Phase 12C `APPLICATION_OWNED_EVENT_BRIDGE_CONFIRMED` contract end-to-end
on host, gives the bridge a real validated implementation surface
without picking a product, and keeps the repo ready for either a
future shadow-bridge phase (Option 2) or a future Application planning
phase (Option 4) without locking either out.

---

## F. Bridge Semantics to Freeze

The Phase 12F host bridge prototype must respect the following
contract. These items are sketched here at planning depth and
expanded in
[`../03_SPECS/FRAMEWORK_APPLICATION_BRIDGE_DRAFT.md`](../03_SPECS/FRAMEWORK_APPLICATION_BRIDGE_DRAFT.md).

### F.1 Ownership

- **Bridge owns the mapping table.** Caller declares a static const
  array of `(adapter_key, robotos_fw_event_id_t)` tuples; the bridge
  scans it on demand. No heap.
- **Caller owns the FSM instance.** Bridge receives a
  `robotos_fw_fsm_t *` pointer; bridge does NOT allocate or store
  ownership of the FSM. This keeps the FSM caller-owned (Phase 12B
  contract) and allows a single FSM to receive events from multiple
  bridges or sources.
- **Caller owns the mapping table memory.** Bridge stores a pointer;
  caller keeps the array alive for the bridge's lifetime.

### F.2 Adapter key shape

- **Primary discriminator:** `robotos_event_type_t` (or, in the host
  prototype, an integer stand-in equivalent to it).
- **Optional sub-discriminator:** `uint32_t arg0` from
  `robotos_event_t` (the UART byte value, button sequence, etc.).
- The bridge's key shape is **`(type, optional arg0 match)`**.
  Mapping rows that omit `arg0` match any value; rows that include
  `arg0` match exactly. The bridge scans rows FIFO (same convention
  as the FSM transition table) and dispatches the first match.

### F.3 Unmapped events

- Unmapped events return OK silently and increment a
  `unmapped_count` counter on the bridge instance.
- Bridge does **not** call `robotos_fw_fsm_dispatch()` for unmapped
  events. This avoids inventing a "default" Framework event ID.
- This rule must be testable: the host test will assert that an
  unmapped event leaves the FSM's `event_count` and counters
  unchanged.

### F.4 Payload pass-through

- The bridge passes the caller-provided payload pointer **unchanged**
  to `robotos_fw_fsm_dispatch()`.
- The bridge does **not** synthesize a new payload struct in the host
  prototype. (A future production bridge may copy `arg0`/`arg1` into
  a typed payload, but that is out of scope for Phase 12F.)
- The bridge **does not store** the payload after the dispatch call
  returns. The bridge instance struct must not contain a payload
  field. (Same rule as the FSM, per Phase 12C
  `PAYLOAD_BORROWED_FOR_DISPATCH_ONLY_CONFIRMED`.)

### F.5 Status propagation

- The bridge's `dispatch` entry point returns whatever the FSM's
  `robotos_fw_fsm_dispatch()` returned (using `robotos_core_status_t`
  through the `robotos_fw_status_t` alias — same as Phase 12E).
- No new public status enum is introduced.
- Unmapped events return `ROBOTOS_CORE_OK` to differentiate from
  "FSM ran but failed" (which would be the FSM's non-OK return).
- Bridge NULL guards return `ROBOTOS_CORE_ERR_NULL`.
- Bridge uninitialized → `ROBOTOS_CORE_ERR_INVALID_STATE`.

### F.6 Threading and context

- Bridge runs in **thread context only**. Never ISR. This matches the
  Phase 12C contract and the existing producer convention (ISR posts,
  thread dispatcher delivers).
- Bridge does **not** start threads, timers, or work items.
- Bridge does **not** require any platform critical section because
  the FSM's get_state/is_in_state/get_snapshot already handle the
  ISR-safe queries internally; the bridge merely sequences calls.

### F.7 Forbidden surface (must be testable)

- Bridge does **not** call any UART TX function.
- Bridge does **not** call any GPIO/PWM/I2C/SPI/sensor driver.
- Bridge does **not** call `robotos_core_register_event_handler()`.
- Bridge does **not** include any Zephyr, devkit, app, or legacy
  `ro_*` header.
- Bridge does **not** allocate heap.
- Bridge does **not** depend on `devkit_app_state.h`.

### F.8 Synthetic test vocabulary

The host test may use neutral test event names such as
`EVT_HOST_INPUT_A`, `EVT_HOST_INPUT_B`, `EVT_HOST_TICK`. These are
**host-test-synthetic only**; they are not product commands and have
no relationship to the devkit `a/s/r/?/x/v/L/d/T` set. The bridge
module itself contains no such names — it is a pure mapping engine.

---

## G. Relationship to `devkit_app_state`

This section is required by the Phase 12F-pre task brief.

- **`devkit_app_state` remains authoritative** for the devkit runtime
  state machine (`IDLE → ARMED → ACTIVE`). It owns the response to
  `?`, the UART byte handling for `a/s/r/d/t/T`, and the button cycle
  semantics. Phase 12F-pre confirms no change.
- **Phase 12F (host bridge prototype) does not replace, promote, copy,
  shadow, or duplicate `devkit_app_state`.** The host bridge prototype
  exists entirely in a separate translation unit, exercised only by a
  host test executable. No devkit producer file calls into the bridge
  in Phase 12F.
- **Future devkit integration paths** are explicitly enumerated as
  three distinct modes; the choice between them is **a separate
  authorization** at a future phase boundary:
  - **Shadow mode.** Add bridge call sites in devkit producers
    alongside `devkit_app_state` calls; Framework FSM observes the
    same events; `devkit_app_state` remains authoritative; FSM state
    exposed only via RTT. Requires explicit user authorization at a
    future Phase 12G-pre (or similar) gate. **Not Phase 12F.**
  - **Replacement mode.** Replace `devkit_app_state` with Framework
    FSM. Requires explicit user authorization and a dedicated
    migration phase that includes UART response-shape compatibility,
    full devkit regression, and `?` response equivalence. **Not Phase
    12F. Not Phase 12G. Likely never without a strong product reason.**
  - **Separate application mode.** Create
    `RobotOS_v1.0/app/` (or equivalent) and bridge into the FSM from
    there; `devkit_app_state` keeps owning devkit-only behavior; the
    Application owns product behavior. Requires explicit user
    authorization AND a chosen product. **Not Phase 12F.**
- **No Phase 12F implementation may modify `devkit_app_state`.** Any
  future need to do so triggers a separate authorized phase.
- **Scope-guard #11 is preserved unchanged by Phase 12F-pre and by
  the recommended Phase 12F.**

---

## H. Relationship to Command Set

This section is required by the Phase 12F-pre task brief.

- **`a / s / r / ? / x / v / L / d / T` remain the devkit/probe
  command surface.** Phase 12F-pre confirms no change; the
  recommended Phase 12F also makes no change.
- **The bridge does not add UART commands.** The bridge has no UART
  TX call and no UART RX consumer. The host bridge test does not run
  any UART code.
- **The Framework FSM is not exposed via UART automatically.** Any
  future decision to expose FSM state through the UART would belong
  to:
  - Shadow mode + a new telemetry decision (separate authorization), or
  - Replacement mode + a `?` response migration (separate
    authorization), or
  - Separate Application mode + an Application-defined product
    command vocabulary (separate authorization).
- **Any future command vocabulary belongs to the Application/product
  phase, not the bridge prototype.** The bridge is a pure mapping
  engine; product semantics live above it.
- All 12 UART TX scope-guard constraints from
  [`PHASE_9EZ_CHECKPOINT.md`](PHASE_9EZ_CHECKPOINT.md) §H remain
  preserved.

---

## I. Required Future Phase 12F Coverage (If Host Bridge Is Chosen)

Phase 12F may close as `CLOSED_WITH_HOST_TEST_EVIDENCE` only when the
new bridge test target passes the following 12 contract cases plus
review-validated items. Cases map to §F semantics.

| # | Test | Maps to |
|---|---|---|
| 1 | Mapped event dispatches the FSM transition (FSM `event_count` and `transition_count` increment as expected). | §F.2 |
| 2 | Unmapped event is silently ignored: no `dispatch()` call, FSM counters unchanged, bridge `unmapped_count++`. | §F.3 |
| 3 | Payload pointer is passed through to the FSM dispatch unchanged. | §F.4 |
| 4 | FSM dispatch non-OK (action returns non-OK) is propagated through the bridge unchanged. | §F.5 |
| 5 | Bridge does **not** retain the payload after dispatch (bridge struct has no payload field). | §F.4 |
| 6 | Bridge does **not** call UART TX (grep: 0 matches for `printf`/`uart`/`tx` in `robotos_fw_event_bridge.c`). | §F.7 |
| 7 | Bridge does **not** call `robotos_core_register_event_handler` (grep: 0 matches). | §F.7 |
| 8 | Bridge mapping is deterministic: two consecutive `dispatch()` calls with the same input produce identical FSM state changes. | §F.2 |
| 9 | Bridge supports multiple mapping rows (test with at least 3 distinct `(type, arg0)` → `event_id` mappings). | §F.2 |
| 10 | Bridge can be tested with host-only synthetic events (no `robotos_event_t` from the core module is required; the bridge takes a small input struct or `type+arg0` pair). | §F.8 |
| 11 | No `devkit_app_state` symbol is referenced by the bridge module (grep: 0 matches). | §G |
| 12 | Command set `a/s/r/?/x/v/L/d/T` zero-diff in the Phase 12F commit. | §H |

Review-validated items (recorded in the Phase 12F closeout):

- R1: No heap (grep `malloc/calloc/realloc/free` returns 0 in the
  bridge `.c`).
- R2: No Zephyr / devkit / legacy `ro_*` includes in the bridge
  module.
- R3: Bridge instance struct definition has no payload field
  (verified by struct definition inspection).
- R4: Public-symbol surface matches the Phase 12F-LOCKED bridge API
  (verified by `nm`/`grep` of static-vs-public symbols).
- R5: Full host suite regression passes (22/22 expected: 21 existing
  Phase 4-6 + Phase 12E + the new Phase 12F bridge target).

---

## J. Non-goals

Phase 12F-pre does **not**:

- Create any `.c` or `.h` source file.
- Create the bridge module.
- Modify `RobotOS_v1.0/framework/robotos_fw_fsm.h` or
  `robotos_fw_fsm.c`.
- Modify `RobotOS_v1.0/framework/README.md`.
- Modify any CMake file.
- Modify any tracked test file.
- Modify or extend any evidence log.
- Modify `devkit_app_state`.
- Modify any devkit runtime / producer file.
- Move, copy, or promote `devkit_app_state`.
- Change command semantics or add UART commands.
- Integrate Framework with devkit.
- Start ACTIVE disarm widening.
- Reopen Scheduler 7A/7B.
- Reopen F407 / custom board.
- Start Application/product source under `RobotOS_v1.0/app/` (or
  anywhere else).
- Create the Application bridge directory.
- Add parser, shell, registry, framing, or response queue.
- Modify Architecture B (`src/`, `include/robotos/`, root CMake).
- Open Phase 12F (Phase 12F remains `NOT_STARTED`).

---

## K. Decision Result

**`PHASE_12F_PRE_RECOMMEND_HOST_BRIDGE_PROTOTYPE`.**

Phase 12F, when authorized, should follow Option 1 (§D.1): a
host-only bridge prototype module + test inside the existing
`tests/host/` build. No devkit integration. No `devkit_app_state`
modification. No command-set change. No hardware run. No Application/
product layer creation.

Alternate acceptable result if the user wants to defer indefinitely:
`PHASE_12F_PRE_RECOMMEND_HOLD`. All other options are explicitly not
recommended at Phase 12F.

---

## L. Phase 12F Recommendation

### Title
**Phase 12F — Framework Application Bridge Host Prototype**

### Classification
`HOST_TEST_IMPLEMENTATION_DOCS_AND_EVIDENCE`. Adds:

- a new bridge module under `RobotOS_v1.0/framework/`,
- a new host test executable under `RobotOS_v1.0/tests/host/`,
- additive update to the existing `tests/host/CMakeLists.txt`,
- a tracked test log under `tests/host/logs/`,
- closeout + progress + CURRENT_STATE + spec + index docs.

No devkit / runtime / hardware / command / Architecture-B / Application-
layer impact.

### Likely future approved files

| Path | Kind |
|---|---|
| `RobotOS_v1.0/framework/robotos_fw_event_bridge.h` | NEW public bridge header (small mapping API; product-neutral; `DRAFT/EXPERIMENTAL`) |
| `RobotOS_v1.0/framework/robotos_fw_event_bridge.c` | NEW bridge implementation (mapping table scan; caller-owned mapping array; no heap; no UART; no devkit; no core register) |
| `RobotOS_v1.0/tests/host/test_robotos_fw_event_bridge.c` | NEW host contract test (12 cases + review items per §I) |
| `RobotOS_v1.0/tests/host/CMakeLists.txt` | UPDATE — additive only (one `add_executable` + one `add_test` block) |
| `RobotOS_v1.0/tests/host/logs/phase_12F_host_<YYYY-MM-DD>.log` | NEW tracked test log |
| `RobotOS_v1.0/devkit/docs/02_PHASE_CLOSEOUTS/PHASE_12F_APPLICATION_BRIDGE_HOST_PROTOTYPE.md` | NEW closeout |
| `RobotOS_v1.0/devkit/docs/01_PROGRESS/DEVKIT_PROGRESS_PHASE_11_20.md` | UPDATE (Phase 12F index row + anchor + section) |
| `RobotOS_v1.0/devkit/docs/03_SPECS/FRAMEWORK_APPLICATION_BRIDGE_DRAFT.md` | UPDATE — small (mark bridge `IMPLEMENTED_AT_12F`) |
| `CURRENT_STATE.md` | UPDATE (Phase 12F as latest closed; bridge status) |
| `RobotOS_v1.0/devkit/docs/00_INDEX/README.md` | OPTIONAL UPDATE (closeout link) |

### Validation

- Host tests only. WSL Ubuntu / gcc 13.3.0 (same toolchain as Phase
  12E).
- `ctest -R robotos_fw_event_bridge_contract --output-on-failure`
  reports PASS.
- Full host suite regression: 22/22 expected.
- Test log captured via `save_test_log.cmake` and committed.

### Non-goals (Phase 12F)

- No `devkit_app_state` change.
- No devkit integration.
- No UART command added or modified.
- No hardware run.
- No Application/product source.
- No `RobotOS_v1.0/app/` directory.
- No new top-level CMake.
- No Architecture B modification.
- No Scheduler 7A/7B work.
- No F407 / custom board work.
- No ACTIVE disarm widening.
- No POST_FLASH_AUTOSTART change.

### Exit criteria

`PHASE_12F_APPLICATION_BRIDGE_HOST_PROTOTYPE_CLOSED` is reachable when:

1. `framework/robotos_fw_event_bridge.h` and
   `robotos_fw_event_bridge.c` exist and compile cleanly on WSL/Linux.
2. `tests/host/test_robotos_fw_event_bridge.c` exists and exercises
   the 12 contract cases.
3. `ctest --test-dir <build> -R robotos_fw_event_bridge_contract`
   reports PASS.
4. Full host suite regression: PASS (22/22).
5. Test log committed at
   `tests/host/logs/phase_12F_host_<date>.log`.
6. Review items R1-R5 from §I recorded as evidence in the Phase 12F
   closeout.
7. All gates from §B preserved unchanged.

### Rollback / blocker behavior

- If the bridge contract cannot be implemented without modifying
  `framework/robotos_fw_fsm.h`, Phase 12F must close
  `BLOCKED_FSM_HEADER_CONTRACT_REVISION_NEEDED` and a small header
  revision phase must precede a retry.
- If a §I case reveals an ambiguity in
  `FRAMEWORK_APPLICATION_BRIDGE_DRAFT.md`, that spec wins until a
  clarification phase corrects it.
- Implementation-only commit without ctest evidence is not
  acceptable. Test log must show PASS.

---

## M. Companion Docs

- [`PHASE_12E_FSM_HOST_TEST_IMPLEMENTATION.md`](PHASE_12E_FSM_HOST_TEST_IMPLEMENTATION.md) — Framework FSM core implementation this bridge will dispatch into. Phase 12F-pre's design is a strict superset that calls into Phase 12E's `robotos_fw_fsm_dispatch()`.
- [`PHASE_12E_PRE_FSM_CONSUMER_TEST_PLAN.md`](PHASE_12E_PRE_FSM_CONSUMER_TEST_PLAN.md) — The plan Phase 12E executed. Phase 12F-pre is the analogous planning phase for the next layer up.
- [`PHASE_12D_FSM_HEADER_STUB.md`](PHASE_12D_FSM_HEADER_STUB.md) — Phase 12D-LOCKED FSM API surface that the bridge consumes.
- [`PHASE_12D_PRE_LEGACY_FRAMEWORK_SCAFFOLD_DISPOSITION.md`](PHASE_12D_PRE_LEGACY_FRAMEWORK_SCAFFOLD_DISPOSITION.md) — Architecture A/B boundary that keeps Phase 12F away from `src/`, `include/robotos/`, root `CMakeLists.txt`, and the legacy `tests/` root.
- [`PHASE_12C_FSM_EVENT_BRIDGE_STATUS_MODEL.md`](PHASE_12C_FSM_EVENT_BRIDGE_STATUS_MODEL.md) — Source of `APPLICATION_OWNED_EVENT_BRIDGE_CONFIRMED`; this phase is the first concrete planning of that bridge.
- [`PHASE_12B_FRAMEWORK_FSM_API_DRAFT.md`](PHASE_12B_FRAMEWORK_FSM_API_DRAFT.md) — Origin of `APPLICATION_OWNED_EVENT_BRIDGE`.
- [`PHASE_11Z_COMMAND_SET_CHECKPOINT.md`](PHASE_11Z_COMMAND_SET_CHECKPOINT.md) — Frozen 9-command set Phase 12F must not touch.
- [`PHASE_9EZ_CHECKPOINT.md`](PHASE_9EZ_CHECKPOINT.md) — 12 UART TX scope guards that constrain Phase 12F §F.7.
- [`../03_SPECS/FRAMEWORK_FSM_API_DRAFT.md`](../03_SPECS/FRAMEWORK_FSM_API_DRAFT.md) — FSM spec; Phase 12F may add a small cross-reference to the bridge draft when 12F is opened.
- [`../03_SPECS/FRAMEWORK_APPLICATION_BRIDGE_DRAFT.md`](../03_SPECS/FRAMEWORK_APPLICATION_BRIDGE_DRAFT.md) — Long-lived bridge spec draft created alongside this planning doc; `DRAFT / NON-FINAL`.

---

## N. Verdict

**`PHASE_12F_PRE — CLOSED_DOCS_ONLY`** with recommendation
`PHASE_12F_PRE_RECOMMEND_HOST_BRIDGE_PROTOTYPE`.

Phase 12F **remains `NOT_STARTED`** until the user explicitly
authorizes opening it. No file under `framework/`, `tests/host/`,
`core/`, `platform/`, `devkit/`, `src/`, or `include/` is modified by
this phase. All gates from §B are preserved unchanged.
