# Phase 12H-pre — First Application Candidate / Product Harness Selection (docs-only)

**Status:** `CLOSED_DOCS_ONLY`
**Decision:** `PHASE_12H_PRE_RECOMMEND_PROBE_TRANSLATOR_APP`
**Date:** 2026-05-13
**Branch / baseline:** `master` at `origin/master = 629e062` (Phase 12G)
**Prior phase anchor:** [`PHASE_12G_SEPARATE_APPLICATION_BOUNDARY_PLANNING.md`](PHASE_12G_SEPARATE_APPLICATION_BOUNDARY_PLANNING.md)
**New long-lived spec:** [`../03_SPECS/FIRST_APPLICATION_CANDIDATE_DRAFT.md`](../03_SPECS/FIRST_APPLICATION_CANDIDATE_DRAFT.md)

---

## A. Executive Summary

Phase 12H-pre is a **docs-only product / application planning gate**
that selects the first application candidate / product harness for
the future `RobotOS_v1.0/app/<product>/` boundary reserved by Phase
12G.

Phase 12H-pre **does**:

- evaluate five candidate first-app shapes (probe translator, demo,
  devkit shadow, real product, HOLD);
- recommend `app/probe_translator/` as the first application harness;
- record a planning-level minimal state vocabulary;
- record a planning-level minimal event vocabulary;
- record a planning-level minimal Framework bridge mapping table;
- record a host-first validation strategy and build-strategy
  preference;
- freeze the candidate boundary in a new long-lived spec
  (`FIRST_APPLICATION_CANDIDATE_DRAFT.md`);
- preserve every other open gate untouched.

Phase 12H-pre **does not**:

- create `app/` or `application/` directory;
- create any `.c` / `.h` / `CMakeLists.txt` / README file;
- modify any file under `framework/`, `core/`, `platform/`,
  `devkit/src/`, `tests/`, `src/`, or `include/robotos/`;
- modify any `CMakeLists.txt`, `prj.conf`, board DTS, overlay, or
  Zephyr config;
- modify any evidence log;
- change `devkit_app_state`;
- change the frozen `a/s/r/?/x/v/L/d/T` command set;
- run hardware;
- open Phase 12H or any implementation gate.

Phase 12H is **not started** at the close of Phase 12H-pre. It may
open only on **explicit user authorization** with the recommended
scope in §P.

---

## B. Baseline Before Phase 12H-pre

| Item | Value |
|---|---|
| `origin/master` baseline | `629e062` `docs: add Phase 12G separate application boundary planning` |
| Phase 12G status | `CLOSED_DOCS_ONLY` / `PHASE_12G_RECOMMEND_APP_PRODUCT_PATH` |
| Phase 12G-pre status | `CLOSED_DOCS_ONLY` / `PHASE_12G_PRE_RECOMMEND_SEPARATE_APPLICATION_MODE_PLANNING` |
| Phase 12F status | `CLOSED_WITH_HOST_TEST_EVIDENCE` / 103/103 bridge assertions; full host regression 22/22 |
| Phase 12E status | `CLOSED_WITH_HOST_TEST_EVIDENCE` / 93/93 FSM assertions |
| Future application boundary | `RobotOS_v1.0/app/<product>/` reserved at planning depth |
| `app/` directory | **NOT_CREATED** |
| Application / product implementation | `NOT_STARTED` |
| Framework FSM | host-tested; `framework/robotos_fw_fsm.{h,c}` |
| Framework event bridge | host-tested; `framework/robotos_fw_event_bridge.{h,c}`; §5 names + signatures `LOCKED-AT-12F` |
| Devkit integration of Framework | `NOT_STARTED` |
| Active architecture | A (`core/` + `platform/` + `devkit/` + `framework/`) |
| Legacy architecture | B (`src/` + `include/robotos/` + root `RobotOS_v1.0/CMakeLists.txt`) — frozen at Phase 12D-pre |
| Devkit runtime authority | `devkit_app_state` (Phase 9C; unchanged) |
| Frozen command set | `a / s / r / ? / x / v / L / d / T` (validated through Phase 11Z) |
| Scope-guard #11 | `devkit_app_state` devkit-local; preserved |
| Open gates carried in | ACTIVE disarm `USER_DECISION_REQUIRED_ACTIVE_DISARM`; Scheduler 7A/7B `DEFER`; F407 `HOLD/DEFER`; POST_FLASH_AUTOSTART `OPEN/MITIGATED_BY_WORKFLOW` |

---

## C. First Application Candidate Problem Statement

Phase 12G reserved `RobotOS_v1.0/app/<product>/` at planning depth
without selecting a `<product>` name. Before any `app/` directory is
created or any application source lands, RobotOS needs a docs-only
selection answering:

1. **Which first `<product>` placeholder?** (e.g., `probe_translator`,
   `demo`, `<real_product_codename>`).
2. **What kind of first harness?** (neutral translator, demo, devkit
   shadow, real product, none).
3. **Minimal state vocabulary?** What states the first application's
   FSM instance needs in order to be a meaningful first consumer.
4. **Minimal event vocabulary?** What product-level event IDs the
   first application defines.
5. **Minimal Framework bridge mapping table?** Which adapter event
   keys feed which product event IDs.
6. **Host-first validation plan?** How a future Phase 12H or 12I
   should validate the mapping under `tests/host/` without touching
   devkit, command set, or hardware.

Risks if these are not selected first:

1. **Path drift.** Without a chosen `<product>` name, the first
   `app/*.c` lands ad-hoc — the chosen name then becomes load-bearing
   without ever being designed.
2. **Premature product commitment.** Picking a real product name at
   this stage couples Framework consumer evidence to product business
   logic and field semantics that have not been defined.
3. **Devkit blend.** Selecting `app/devkit_shadow/` first would
   immediately reintroduce the duplicate-state-truth risk Phase 12G-pre
   rejected.
4. **Framework pollution.** Any state / event vocabulary chosen here
   that leaks into `framework/` would break the Phase 12F
   `LOCKED-AT-12F` boundary; the vocabulary must remain
   application-local.
5. **Validation drift.** Without a host-first plan, the first
   integration may be tempted to bind to devkit producers or to UART —
   both forbidden by Phase 12G.

Phase 12H-pre resolves all five risks at planning depth, without
writing any source.

---

## D. Candidate Application Names / Harness Types

Five candidate first-app shapes are evaluated. The first three are
host-first and product-neutral; the fourth is product-bound; the
fifth is a no-op.

### Candidate 1 — `app/probe_translator/`

| Aspect | Value |
|---|---|
| Purpose | Product-neutral application harness whose only job is to translate **synthetic adapter / probe events** into Framework event IDs and feed them to a Framework FSM instance. Acts as the first Framework consumer at the application layer. |
| Future files likely touched | `RobotOS_v1.0/app/probe_translator/probe_translator.{c,h}`; `RobotOS_v1.0/app/probe_translator/probe_translator_mapping.{c,h}` (optional split); `RobotOS_v1.0/app/probe_translator/CMakeLists.txt`; `RobotOS_v1.0/app/probe_translator/README.md`; future host test `RobotOS_v1.0/tests/host/test_app_probe_translator_mapping.c`. |
| Relationship to `framework/` | Consumer-only. Includes `robotos_fw_fsm.h` and `robotos_fw_event_bridge.h`. Does not modify Framework. |
| Relationship to `devkit/` | None. Does not include `devkit_app_state.h`. Does not read or write `devkit_app_state`. Does not bind to `devkit_button_producer` / `devkit_uart_producer` by name. |
| Relationship to command set | None. Does not introduce a UART command. Does not modify `a/s/r/?/x/v/L/d/T` semantics. |
| Product commitment level | **Minimal.** Translator semantics are neutral; no business logic, no actuator policy, no sensor fusion. |
| Validation strategy | Host-first: synthetic Adapter events → bridge → FSM transitions asserted under `tests/host/`. No devkit, no hardware. |
| Risk | **Low.** Smallest possible first Framework consumer at the application layer. Stays inside every Phase 12G non-goal. |
| Recommendation | **RECOMMENDED.** |

### Candidate 2 — `app/demo/`

| Aspect | Value |
|---|---|
| Purpose | A "first sample" application demonstrating Framework consumption end-to-end. |
| Future files likely touched | `RobotOS_v1.0/app/demo/demo.{c,h}`, `CMakeLists.txt`, `README.md`, plus host tests. |
| Relationship to `framework/` | Consumer-only. |
| Relationship to `devkit/` | None at start; risk of growing toward devkit producers as the "demo" expands in scope. |
| Relationship to command set | None initially. |
| Product commitment level | **Vague.** "Demo" has no contract — its scope drifts as people add things to it. |
| Validation strategy | Host-first possible, but the demo has no fixed acceptance criterion. |
| Risk | **Medium.** Name is too vague; high chance of becoming a scope sink. Better as a later sibling under `RobotOS_v1.0/examples/` (Phase 12G §D Option 5). |
| Recommendation | **REJECTED at Phase 12H-pre.** Useful later as a sample, but not the first application harness. |

### Candidate 3 — `app/devkit_shadow/`

| Aspect | Value |
|---|---|
| Purpose | An application that mirrors `devkit_app_state` transitions through the Framework path for cross-check. |
| Future files likely touched | `RobotOS_v1.0/app/devkit_shadow/*.c/h`, plus host tests and possibly a runtime hook into devkit producers. |
| Relationship to `framework/` | Consumer. |
| Relationship to `devkit/` | **Direct dependency.** Would need to consume devkit event keys or borrow producer signatures; trivially creates duplicate state truth in the runtime. |
| Relationship to command set | High risk of accidental coupling. |
| Product commitment level | Low. |
| Validation strategy | Cannot be validated host-first cleanly because it implicitly references devkit producers. |
| Risk | **High.** Re-introduces the Mode 2 (SHADOW) direction Phase 12G-pre explicitly deferred. Selecting it here would override Phase 12G-pre's decision by stealth. |
| Recommendation | **REJECTED.** Only re-evaluate if/when user explicitly authorizes Mode 2 from Phase 12G-pre §D. |

### Candidate 4 — `app/minirobot/` or `app/robot_demo/` (real-product placeholder)

| Aspect | Value |
|---|---|
| Purpose | A first-product placeholder named for an actual product direction (mini-robot, robot demo, etc.). |
| Future files likely touched | `RobotOS_v1.0/app/<product>/*.c/h`, board overlay, `prj.conf` overlay, possibly hardware-runnable Zephyr application. |
| Relationship to `framework/` | Consumer. |
| Relationship to `devkit/` | Indirect via shared hardware, eventually. |
| Relationship to command set | Possibly grows a product command vocabulary, on its own channel. |
| Product commitment level | **High.** Requires actuator / sensor / motion direction that has not been defined; binds the first Framework consumer to product semantics. |
| Validation strategy | Needs product-acceptance criteria that do not yet exist. |
| Risk | **High.** Premature product commitment. The Framework path has never been consumed by anyone; the first consumer should be neutral so failures are debugging, not redesign. |
| Recommendation | **REJECTED at Phase 12H-pre.** Re-evaluate only once a product direction is independently authorized. Tracked as `PHASE_12H_PRE_RECOMMEND_PRODUCT_APP_PENDING_DIRECTION` outcome in §O. |

### Candidate 5 — HOLD / no first app candidate

| Aspect | Value |
|---|---|
| Purpose | Decline to pick a first candidate at all. Phase 12G boundary stays at planning depth indefinitely. |
| Future files | None. |
| Relationship to `framework/` | Framework remains host-tested only with no application consumer. |
| Product commitment level | Zero. |
| Validation strategy | None. |
| Risk | **Low** in the short term; **Medium** long-term: the Framework path keeps accumulating spec surface without a runtime consumer, and any new spec assumption goes unvalidated. |
| Recommendation | **Acceptable fallback** if the user prefers to stop here. Tracked as `PHASE_12H_PRE_RECOMMEND_HOLD` outcome. |

### Comparison summary

| # | Candidate | Product commitment | Validation strategy | Risk | Recommendation |
|---|---|---|---|---|---|
| 1 | `app/probe_translator/` | Minimal (neutral) | Host-first; clean | Low | **RECOMMENDED** |
| 2 | `app/demo/` | Vague | Host-first; weak acceptance criterion | Medium | Rejected (scope sink; better as `examples/`) |
| 3 | `app/devkit_shadow/` | Low | Cannot be host-first cleanly | High | Rejected (overrides Phase 12G-pre Mode 2 DEFER) |
| 4 | `app/<real_product>/` | High | Needs product acceptance criteria | High | Rejected (premature; pending product direction) |
| 5 | HOLD | None | None | Low short / Medium long | Acceptable fallback |

---

## E. Recommended First Candidate

**Decision result:** `PHASE_12H_PRE_RECOMMEND_PROBE_TRANSLATOR_APP`.

### Meaning

- Future first application placeholder is
  `RobotOS_v1.0/app/probe_translator/`.
- **No directory is created in Phase 12H-pre.** The path is reserved
  at planning depth; no source, no header, no `CMakeLists.txt`, no
  README lands.
- `probe_translator` is **not a product**. It is a host-first
  application harness whose only job is to prove application-level
  event mapping into the Framework bridge / FSM.
- The harness is **separate from devkit runtime and command set**.
- The Framework FSM and bridge remain product-neutral and
  `LOCKED-AT-12F` / `LOCKED-AT-12D` for names and signatures.

### Rationale

- Lowest product commitment of any candidate; lowest risk of
  redirecting the project.
- Host-first by construction; never needs devkit producers,
  `devkit_app_state`, hardware, or a UART surface.
- Synthetic adapter events let us exercise FIFO ordering, arg0
  wildcards, status pass-through, unmapped-event accounting, and
  re-init policy under controlled host inputs.
- Failure of `app/probe_translator/` is debugging of the mapping
  layer, not a product redesign.
- A future real product `app/<product>/` can be added as a sibling
  later; nothing in `probe_translator/` constrains the second
  application.

---

## F. Minimal State Vocabulary

The future `app/probe_translator/` FSM instance uses a minimal,
product-neutral state vocabulary. All names are **application-local**
and live in the `app/probe_translator/` namespace only.

| State | Meaning | Why needed | Non-goals |
|---|---|---|---|
| `APP_IDLE` | Translator is initialized but has not yet seen a `CONFIGURED` event. Default state after `robotos_fw_fsm_init`. | Provides a known starting state and a target for `APP_EVT_RESET`. | Not equivalent to devkit `IDLE`. Not a product idle. Not a power state. |
| `APP_READY` | Translator has received a synthetic `CONFIGURED` event and is ready to translate further events into FSM transitions. | Provides a target state after `APP_EVT_CONFIGURED` so the first transition is observable in a host test. | Not equivalent to devkit `ARMED`. Does not imply hardware readiness. |
| `APP_ACTIVE` | Translator has received `APP_EVT_START` from a synthetic adapter source. | Allows host tests to exercise a multi-step transition path (`IDLE -> READY -> ACTIVE -> READY -> IDLE`). | Not equivalent to devkit `ACTIVE`. Does not imply runtime / hardware activity. |
| `APP_FAULT` (optional) | Translator has received a synthetic fault event. Reached only via `APP_EVT_FAULT`. | Provides a fault sink for negative-path tests. May be omitted at first if the only fault behavior is "stay in current state and increment a counter". | Not equivalent to a real product fault. No actuator / sensor implications. |

State name discipline:

1. The prefix `APP_` makes it obvious these are application-local
   names. They do not collide with `framework/` symbol space (no
   product names appear in `framework/`).
2. Similar names (`IDLE`, `ACTIVE`) to `devkit_app_state` are
   **coincidental** at the human-readable level only. They do not
   imply replacement, shadow, or coupling.
3. `devkit_app_state` keeps the names `DEVKIT_APP_STATE_IDLE / ARMED
   / ACTIVE`. The application uses `APP_IDLE / APP_READY / APP_ACTIVE
   / APP_FAULT`. No code or doc shares constants between them.
4. The future spec is free to drop `APP_FAULT` if the host-test plan
   does not need it. Phase 12H-pre records it as optional only.

---

## G. Minimal Event Vocabulary

Application-local Framework event IDs the first `app/probe_translator/`
defines. These values live in the application namespace and are not
coordinated with the Framework core.

| Event ID | Source class at first | Mapping purpose | Host-only / synthetic? | Maps from |
|---|---|---|---|---|
| `APP_EVT_CONFIGURED` | Synthetic | Drives `APP_IDLE -> APP_READY` transition. | **Yes — synthetic only.** | Synthetic Adapter key `ADAPTER_EVT_CONFIG, arg0=0`. |
| `APP_EVT_START` | Synthetic | Drives `APP_READY -> APP_ACTIVE` transition. | **Yes — synthetic only.** | Synthetic Adapter key `ADAPTER_EVT_COMMAND, arg0=START`. |
| `APP_EVT_STOP` | Synthetic | Drives `APP_ACTIVE -> APP_READY` transition. | **Yes — synthetic only.** | Synthetic Adapter key `ADAPTER_EVT_COMMAND, arg0=STOP`. |
| `APP_EVT_FAULT` (optional) | Synthetic | Drives any state to `APP_FAULT`. Tests wildcard arg0 mapping. | **Yes — synthetic only.** | Synthetic Adapter key `ADAPTER_EVT_FAULT, wildcard arg0`. |
| `APP_EVT_RESET` | Synthetic | Drives any state back to `APP_IDLE`. Tests state reset semantics distinct from `robotos_fw_fsm_reset`. | **Yes — synthetic only.** | Synthetic Adapter key `ADAPTER_EVT_CONFIG, arg0=RESET`. |

Event vocabulary discipline:

1. None of these events map from a **real** core event type (no
   `ROBOTOS_EVENT_USER+1/+2/+3`). Phase 12H-pre uses synthetic adapter
   types reserved at the application layer only.
2. None of these events maps from a UART byte. The application does
   not see UART input.
3. None of these events maps from a button or hardware source.
4. No new `ROBOTOS_EVENT_USER` subrange is required for `core/`.
   Adapter event types remain core-defined; the application defines
   its own translation domain.

---

## H. Minimal Bridge Mapping Table

Planning-level initial mapping table for the first
`app/probe_translator/`. **No source is created at Phase 12H-pre.**
The table below is conceptual only.

| Row | `adapter_type` | `adapter_arg0` | `match_arg0` | `fw_event_id` | Expected transition |
|---|---|---|---|---|---|
| 0 | `ADAPTER_EVT_CONFIG` | `0` | `true` | `APP_EVT_CONFIGURED` | `APP_IDLE -> APP_READY` |
| 1 | `ADAPTER_EVT_CONFIG` | `1` (`RESET`) | `true` | `APP_EVT_RESET` | any -> `APP_IDLE` |
| 2 | `ADAPTER_EVT_COMMAND` | `0` (`START`) | `true` | `APP_EVT_START` | `APP_READY -> APP_ACTIVE` |
| 3 | `ADAPTER_EVT_COMMAND` | `1` (`STOP`) | `true` | `APP_EVT_STOP` | `APP_ACTIVE -> APP_READY` |
| 4 | `ADAPTER_EVT_FAULT` | (any) | `false` (wildcard) | `APP_EVT_FAULT` | any -> `APP_FAULT` (if present) |

Mapping table discipline:

1. **Planning-only.** No `robotos_fw_event_bridge_row_t[]` array
   exists in any source file.
2. **No core event type is allocated.** `ADAPTER_EVT_CONFIG`,
   `ADAPTER_EVT_COMMAND`, `ADAPTER_EVT_FAULT` are application-local
   numeric tags chosen at the implementation phase, not reservations
   in `core/`.
3. **No UART command is mapped.** The application does not bind to
   `a/s/r/?/x/v/L/d/T`.
4. **No real hardware source is mapped.** No `USER+2` button, no
   `USER+3` UART byte, no accelerometer probe key.
5. **FIFO order matters.** Per Phase 12F-pre §11 and Phase 12F §5.3,
   wildcard precedence is FIFO row order only; if Row 4 were placed
   before Row 0, then `ADAPTER_EVT_FAULT` with arg0=0 would beat the
   `CONFIG` mapping — the table must keep specific rows ahead of
   wildcard rows in practice.
6. **Single FSM instance.** The application owns exactly one
   `robotos_fw_fsm_t` and exactly one `robotos_fw_event_bridge_t`. No
   fan-out (Phase 12F-pre §11 #5 deferred).

---

## I. Future Host Mapping Test Plan

If Phase 12H is later authorized to implement the harness, the host
test plan should validate at least:

1. **Application defines product-local event IDs.** A static `enum`
   or `#define` set inside the application's translation unit holds
   `APP_EVT_*` constants.
2. **Application defines transition table.** A static const array of
   `robotos_fw_fsm_transition_t` (or equivalent) inside the
   application binds each `APP_EVT_*` to the expected state
   transition.
3. **Application owns the bridge mapping table.** A static const
   array of `robotos_fw_event_bridge_row_t` inside the application
   maps Adapter event keys to `APP_EVT_*`.
4. **Host test dispatches synthetic adapter events.** A new test file
   (e.g., `tests/host/test_app_probe_translator_mapping.c`) calls
   `robotos_fw_event_bridge_dispatch(...)` with synthesized
   `(adapter_type, adapter_arg0, payload=NULL)` tuples and asserts
   the resulting FSM state, the bridge mapped/unmapped counters, and
   the last `fw_event_id`.
5. **No `devkit_app_state` dependency.** The new test file does not
   `#include` `devkit_app_state.h`; the application source does not
   either. A grep gate enforces this.
6. **No command-set dependency.** No reference to `a/s/r/?/x/v/L/d/T`
   in the new application source or test.
7. **No hardware dependency.** No `prj.conf`, no board DTS overlay,
   no Zephyr include, no RTT call.
8. **Coverage targets** (planning-level only):
   - all five rows in §H exercised;
   - at least one wildcard-row test;
   - at least one unmapped-event test (event not in table);
   - at least one re-init / reset test (bridge counters cleared, FSM
     not touched);
   - at least one full transition path
     `IDLE -> READY -> ACTIVE -> READY -> IDLE`;
   - if `APP_FAULT` is included: at least one fault test from each
     other state.

Phase 12H-pre **does not** create this test file. The plan above is
the contract that Phase 12H — if authorized — must honor.

---

## J. Build Strategy

Phase 12H-pre records the preferred build strategy at planning
depth only. **No CMake change at Phase 12H-pre.**

1. **No build change at Phase 12H-pre.** Zero CMake / `prj.conf` /
   DTS / overlay / Zephyr config diffs.
2. **Future first implementation should be host-first.** The first
   `app/probe_translator/` source pair should compile under the
   existing host test environment (WSL Ubuntu / gcc 13.3.0; Phase 12E
   / 12F baseline).
3. **Preferred first build option = Option A from Phase 12G §6 / §H
   of the application boundary spec:**
   add a new host test target under `tests/host/CMakeLists.txt` that
   compiles:
   - `RobotOS_v1.0/app/probe_translator/probe_translator.c` (when
     created);
   - `RobotOS_v1.0/framework/robotos_fw_event_bridge.c` (existing);
   - `RobotOS_v1.0/framework/robotos_fw_fsm.c` (existing);
   - the existing host critical-section stub.
4. **Do NOT use** root legacy `RobotOS_v1.0/CMakeLists.txt`. Do NOT
   use `devkit/CMakeLists.txt`. The first harness does not link into
   the devkit firmware target.
5. **No hardware build at Phase 12H or 12I.** Hardware build is a
   later phase, separate from this gate.
6. **Option B (a new `app/probe_translator/CMakeLists.txt`) remains
   available** as a future refactor once at least one more application
   exists; not preferred at the first implementation phase because
   host-first additive wiring inside `tests/host/CMakeLists.txt` is
   the lowest-risk path.

---

## K. Relationship to `devkit_app_state`

- `devkit_app_state` remains **authoritative** for the current devkit
  runtime. Phase 12H-pre does **not** modify, replace, shadow, copy,
  or promote it.
- `app/probe_translator/`, when created, **must not** include
  `devkit_app_state.h`, must not call any `devkit_*` function, and
  must not read or write `devkit_app_state` snapshots.
- The application's `APP_IDLE / APP_READY / APP_ACTIVE / APP_FAULT`
  states are **application-local only**. Name similarity to devkit's
  `DEVKIT_APP_STATE_IDLE / ARMED / ACTIVE` is human-readable
  coincidence; it does not imply replacement.
- **No `?` UART response change.** Devkit's `?` continues to come
  from `devkit_app_state`, unchanged.
- **No `a/s/r/?/x/v/L/d/T` semantic change.**
- Any future devkit ↔ application interaction (shadow, replacement)
  is a **separate planning phase**, not Phase 12H or Phase 12H-pre.
- **Scope-guard #11 remains active.**

---

## L. Relationship to Command Set

- The current command set `a / s / r / ? / x / v / L / d / T` remains
  the **devkit / probe surface**. Validated through Phase 11Z. Phase
  12H-pre changes nothing here.
- `app/probe_translator/` **must not** define a UART command. It has
  no UART surface at all in the first host implementation phase.
- The Framework bridge has no UART surface (Phase 12F locked). The
  application does not expose Framework state through UART
  automatically.
- A future application command vocabulary (for `probe_translator/`
  or any later product) requires a **separate product-command
  planning phase**. Such a phase must:
  - list every byte / message it introduces;
  - declare which channel it uses (separate UART, network, etc.);
  - state explicitly that `a/s/r/?/x/v/L/d/T` semantics are
    unchanged;
  - provide its own validation strategy.
- All 12 UART TX scope-guard constraints from Phase 9EZ §H preserved.

---

## M. Relationship to Legacy Architecture B

- `RobotOS_v1.0/src/` and `RobotOS_v1.0/include/robotos/` remain
  frozen at `LEGACY_SCAFFOLD_MARKED_FROZEN_DOCS_ONLY` (Phase 12D-pre).
- The future `app/probe_translator/` directory **does not reuse**
  `src/app/`, `include/robotos/app*`, or any other Architecture B
  artifact.
- The future application uses **Architecture A contracts only**:
  `robotos_fw_*` (from `framework/`), and if needed
  `robotos_core_status_t` (from `core/`).
- The future application **must not include** any `ro_*` HAL header
  or any header from `include/robotos/`.
- **No Architecture A ↔ Architecture B reconciliation in Phase
  12H-pre.** That remains a separate, deferred question.
- `RobotOS_v1.0/src/README_LEGACY_SCAFFOLD.md`,
  `RobotOS_v1.0/src/framework/DEPRECATED.md`, and
  `RobotOS_v1.0/include/robotos/DEPRECATED.md` are not modified by
  Phase 12H-pre.

---

## N. Non-goals (Phase 12H-pre)

| Non-goal | Status |
|---|---|
| `app/` directory created | NO — reserved at planning depth only |
| `application/` directory created | NO |
| `app/probe_translator/` directory created | NO |
| Application source / header file | NO |
| Application `CMakeLists.txt` | NO |
| Application README | NO |
| CMake change anywhere | NO |
| Framework code change | NO (`framework/` zero-diff) |
| Devkit runtime change | NO (`devkit/src/` zero-diff) |
| `devkit_app_state` change | NO |
| UART command added | NO |
| Command semantic change | NO |
| `prj.conf` / DTS / overlay change | NO |
| Zephyr config change | NO |
| Test added or modified | NO (`tests/` zero-diff) |
| Evidence log touched | NO |
| Hardware run | NO |
| Scheduler 7A/7B reopened | NO — `DEFER` preserved |
| F407 / custom board reopened | NO — `HOLD/DEFER` preserved |
| ACTIVE disarm widening started | NO — `USER_DECISION_REQUIRED_ACTIVE_DISARM` preserved |
| POST_FLASH_AUTOSTART change | NO — `OPEN/MITIGATED_BY_WORKFLOW` preserved |
| Legacy Architecture B modification | NO — frozen |
| Phase 12H opened | NO — `NOT_STARTED`; requires explicit user authorization |
| Replacement / shadow / direct devkit integration unlocked | NO |

---

## O. Decision Result

**`PHASE_12H_PRE_RECOMMEND_PROBE_TRANSLATOR_APP`**

The first `<product>` placeholder under `RobotOS_v1.0/app/<product>/`
is selected as `probe_translator`. The future first application
harness lives at `RobotOS_v1.0/app/probe_translator/` and is a
host-first, product-neutral, synthetic-event translation harness
proving Framework consumption at the application layer.

Phase 12H-pre **does not** create the directory or any file under
it. Phase 12H — if authorized — opens the first implementation
phase with the scope in §P.

Alternative outcomes considered but **not** selected:

- `PHASE_12H_PRE_RECOMMEND_DEMO_APP` — rejected; "demo" lacks a
  fixed acceptance criterion and risks becoming a scope sink. Better
  reserved for a future `RobotOS_v1.0/examples/` tree.
- `PHASE_12H_PRE_RECOMMEND_PRODUCT_APP_PENDING_DIRECTION` —
  rejected; premature. Re-evaluate only once a product direction is
  independently authorized.
- `PHASE_12H_PRE_RECOMMEND_HOLD` — acceptable fallback if the user
  prefers to stop here; not selected by Phase 12H-pre's
  recommendation.
- `PHASE_12H_PRE_BLOCKED_NEEDS_PRODUCT_DIRECTION` — not applicable;
  the recommendation deliberately decouples the first Framework
  consumer from product direction.

---

## P. Next Gate Recommendation

**Phase 12H — Probe Translator App Skeleton Planning or Host
Prototype**

Phase 12H opens only on **explicit user authorization**. Two shapes
are possible; the docs-only variant is preferred unless the user
authorizes implementation directly.

### Variant 1 — Phase 12H — Probe Translator Skeleton Planning (docs-only)

| Field | Value |
|---|---|
| Title | Phase 12H — Probe Translator App Skeleton Planning (docs-only) |
| Classification | Docs-only application implementation planning |
| Purpose | Freeze the exact file list, function signatures, build wiring choice (Option A vs. Option B from §J), and host test case list for `app/probe_translator/` before any source lands. Lock the `APP_EVT_*` numeric values, the mapping table row order, and the test naming convention. |
| In-scope | File list freeze; function signature freeze; CMake-option freeze (Option A preferred); host test case list freeze; consumer / non-consumer surface list freeze. |
| Non-goals | No `app/` directory creation. No source / header / CMake / README. No devkit change. No `devkit_app_state` change. No command-set change. No hardware run. |
| Exit criteria | A `PHASE_12H_PROBE_TRANSLATOR_SKELETON_PLANNING.md` closeout that locks the planning depth contract above. A revision of `FIRST_APPLICATION_CANDIDATE_DRAFT.md` upgrading §3-§7 from "planning-level" to "frozen-at-12H planning". |
| Authorization | Must be opened only on **explicit user approval**. Phase 12H-pre's recommendation is not authorization. |

### Variant 2 — Phase 12H — Probe Translator Host Prototype (host-first implementation)

| Field | Value |
|---|---|
| Title | Phase 12H — Probe Translator Host Prototype |
| Classification | Host-first implementation |
| Purpose | Implement `app/probe_translator/probe_translator.{c,h}`, add a new host test target inside the existing `tests/host/CMakeLists.txt` (Option A), and produce host evidence under WSL Ubuntu / gcc 13.3.0. |
| In-scope | New `RobotOS_v1.0/app/probe_translator/` directory; first application source pair; new `tests/host/test_app_probe_translator_mapping.c`; additive entry in `tests/host/CMakeLists.txt`; new host log under `tests/host/logs/phase_12H_host_<YYYY-MM-DD>.log`. |
| Non-goals | No devkit change. No `devkit_app_state` change. No UART command. No hardware run. No legacy Architecture B modification. No Framework code change. |
| Exit criteria | All §I coverage targets met. Full host regression preserved at ≥22/22. New host test passes 100%. A `PHASE_12H_PROBE_TRANSLATOR_HOST_PROTOTYPE.md` closeout exists. `FIRST_APPLICATION_CANDIDATE_DRAFT.md` upgrades to `IMPLEMENTED_AT_12H (HOST-TEST EVIDENCE)`. |
| Authorization | Must be opened only on **explicit user approval**. Recommended only if the user is comfortable skipping the Variant 1 docs-only gate. |

### Alternative — HOLD

If the user does not want to create an app candidate yet, the
phase chain stops. The Framework path stays host-tested only;
`app/` remains uncreated; the candidate spec
(`FIRST_APPLICATION_CANDIDATE_DRAFT.md`) sits at planning depth
indefinitely. No regression risk; no further action required.

---

## Q. Cross-references

- Phase 12G separate application boundary planning:
  [`PHASE_12G_SEPARATE_APPLICATION_BOUNDARY_PLANNING.md`](PHASE_12G_SEPARATE_APPLICATION_BOUNDARY_PLANNING.md)
- Application boundary spec:
  [`../03_SPECS/FRAMEWORK_APPLICATION_BOUNDARY_DRAFT.md`](../03_SPECS/FRAMEWORK_APPLICATION_BOUNDARY_DRAFT.md)
- Phase 12G-pre devkit integration mode decision:
  [`PHASE_12G_PRE_DEVKIT_INTEGRATION_MODE_DECISION.md`](PHASE_12G_PRE_DEVKIT_INTEGRATION_MODE_DECISION.md)
- Long-lived devkit integration mode spec:
  [`../03_SPECS/FRAMEWORK_DEVKIT_INTEGRATION_MODE_DRAFT.md`](../03_SPECS/FRAMEWORK_DEVKIT_INTEGRATION_MODE_DRAFT.md)
- Phase 12F bridge host prototype:
  [`PHASE_12F_APPLICATION_BRIDGE_HOST_PROTOTYPE.md`](PHASE_12F_APPLICATION_BRIDGE_HOST_PROTOTYPE.md)
- Long-lived bridge spec:
  [`../03_SPECS/FRAMEWORK_APPLICATION_BRIDGE_DRAFT.md`](../03_SPECS/FRAMEWORK_APPLICATION_BRIDGE_DRAFT.md)
- Phase 12E FSM implementation:
  [`PHASE_12E_FSM_HOST_TEST_IMPLEMENTATION.md`](PHASE_12E_FSM_HOST_TEST_IMPLEMENTATION.md)
- Long-lived FSM spec:
  [`../03_SPECS/FRAMEWORK_FSM_API_DRAFT.md`](../03_SPECS/FRAMEWORK_FSM_API_DRAFT.md)
- Phase 11Z command-set checkpoint:
  [`PHASE_11Z_COMMAND_SET_CHECKPOINT.md`](PHASE_11Z_COMMAND_SET_CHECKPOINT.md)
- New long-lived first-application-candidate spec:
  [`../03_SPECS/FIRST_APPLICATION_CANDIDATE_DRAFT.md`](../03_SPECS/FIRST_APPLICATION_CANDIDATE_DRAFT.md)
- Live state: [`../../../../CURRENT_STATE.md`](../../../../CURRENT_STATE.md)
