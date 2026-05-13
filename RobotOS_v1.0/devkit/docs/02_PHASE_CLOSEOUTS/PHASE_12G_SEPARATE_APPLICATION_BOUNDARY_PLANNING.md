# Phase 12G — Separate Application Mode / Application Boundary Planning (docs-only)

**Status:** `CLOSED_DOCS_ONLY`
**Decision:** `PHASE_12G_RECOMMEND_APP_PRODUCT_PATH`
**Date:** 2026-05-13
**Branch / baseline:** `master` at `origin/master = 399e956` (Phase 12G-pre)
**Prior phase anchor:** [`PHASE_12G_PRE_DEVKIT_INTEGRATION_MODE_DECISION.md`](PHASE_12G_PRE_DEVKIT_INTEGRATION_MODE_DECISION.md)
**New long-lived spec:** [`../03_SPECS/FRAMEWORK_APPLICATION_BOUNDARY_DRAFT.md`](../03_SPECS/FRAMEWORK_APPLICATION_BOUNDARY_DRAFT.md)

---

## A. Executive Summary

Phase 12G is a **docs-only architecture planning gate** that defines
the Separate Application Mode boundary recommended by Phase 12G-pre.
It locks the planning-level answer to four questions:

1. **Where** does future application code live?
2. **What** does it own vs. not own?
3. **How** does it consume the Framework path?
4. **What** validation precedes any hardware run?

Phase 12G **does not**:

- create `app/` or `application/` directory;
- write any application `.c` / `.h` file;
- modify any file under `framework/`, `core/`, `platform/`,
  `devkit/src/`, `tests/`, `src/`, or `include/robotos/`;
- modify any `CMakeLists.txt`, `prj.conf`, board DTS, overlay, or
  Zephyr config;
- modify any evidence log;
- change `devkit_app_state`;
- change the frozen `a/s/r/?/x/v/L/d/T` command set;
- run hardware;
- open Phase 12H or any implementation gate.

Phase 12G **does**:

- evaluate five candidate application directory shapes;
- recommend `RobotOS_v1.0/app/<product>/` as the future application
  path;
- enumerate application-layer responsibilities and explicit
  non-responsibilities;
- define event-mapping ownership at the application layer;
- define build-separation strategy at planning depth;
- define staged validation gates before any hardware run;
- freeze the boundary in a new long-lived spec
  (`FRAMEWORK_APPLICATION_BOUNDARY_DRAFT.md`);
- preserve every other open gate untouched.

---

## B. Baseline Before Phase 12G

| Item | Value |
|---|---|
| `origin/master` baseline | `399e956` `docs: add Phase 12G-pre devkit integration mode decision` |
| Phase 12G-pre status | `CLOSED_DOCS_ONLY` / `PHASE_12G_PRE_RECOMMEND_SEPARATE_APPLICATION_MODE_PLANNING` |
| Phase 12F status | `CLOSED_WITH_HOST_TEST_EVIDENCE` / `PHASE_12F_APPLICATION_BRIDGE_HOST_PROTOTYPE_CLOSED` |
| Framework FSM | `framework/robotos_fw_fsm.{h,c}` host-test-validated 93/93 (Phase 12E) |
| Framework Application Bridge | `framework/robotos_fw_event_bridge.{h,c}` host-test-validated 103/103 (Phase 12F); §5 names + signatures `LOCKED-AT-12F` |
| Full host regression | 22/22 PASS under WSL Ubuntu / gcc 13.3.0 |
| Devkit integration of Framework | `NOT_STARTED` |
| Application / product layer | `NOT_STARTED` — no `app/` or `application/` directory |
| Active architecture | A (`core/` + `platform/` + `devkit/` + `framework/`) |
| Legacy architecture | B (`src/` + `include/robotos/` + root `RobotOS_v1.0/CMakeLists.txt`) — frozen at Phase 12D-pre |
| Devkit runtime authority | `devkit_app_state` (Phase 9C; unchanged through Phase 12G-pre) |
| Frozen command set | `a / s / r / ? / x / v / L / d / T` (validated through Phase 11Z) |
| Scope-guard #11 | `devkit_app_state` is devkit-local; not promoted, replaced, copied, or duplicated; preserved through Phase 12G-pre |
| Open gates carried in | ACTIVE disarm `USER_DECISION_REQUIRED_ACTIVE_DISARM`; Scheduler 7A/7B `DEFER`; F407 `HOLD/DEFER`; POST_FLASH_AUTOSTART `OPEN/MITIGATED_BY_WORKFLOW` |

---

## C. Application Boundary Problem Statement

The Framework path (FSM + Application Event Bridge) is host-tested and
has a frozen API surface, but has **no runtime consumer**. Phase
12G-pre rejected direct devkit integration and selected Separate
Application Mode as the next direction. Phase 12G must answer where
the future application lives **before** any application source or
CMake exists.

The risks if this question is *not* answered first:

1. **Path drift.** Without a planning-level choice, the first
   application `.c` file lands wherever the implementer feels like —
   inside `devkit/`, inside `framework/`, or under a brand-new
   directory chosen ad hoc. Each of those creates a different
   long-term coupling; the choice should be a planning decision, not
   an implementer reflex.
2. **Devkit blend.** If the application code lives under `devkit/`,
   the devkit validation harness and the application layer become
   indistinguishable. Phase 9 / 10 / 11 evidence covers the devkit
   harness; merging application code into that tree silently
   broadens the evidence claim.
3. **Framework pollution.** If the application code lives under
   `framework/`, the Framework loses its product-neutral guarantee
   (Phase 12B/12C/12F). Any product vocabulary (`IDLE/ARMED/ACTIVE`,
   etc.) appearing in `framework/` would directly violate the
   `LOCKED-AT-12F` boundary.
4. **Legacy reactivation.** If the application code accidentally
   reuses anything under `src/app/` or `include/robotos/app/`,
   Architecture B becomes part of the new application by stealth,
   bypassing Phase 12D-pre's `LEGACY_SCAFFOLD_MARKED_FROZEN_DOCS_ONLY`
   classification.
5. **Build coupling.** If the future application target is added to
   root `RobotOS_v1.0/CMakeLists.txt`, it inherits the legacy
   Architecture B build entry. The application must attach to
   Architecture A only.

Phase 12G resolves all five risks at planning depth, without writing
any source.

---

## D. Candidate Application Directory Shapes

Five candidate shapes are evaluated. Conventions used:

- "Adjacent to Framework" = sibling of `core/`, `platform/`,
  `devkit/`, `framework/` (preferred for Architecture A consumers).
- "Adjacent to devkit harness" = nested inside `devkit/` (mixes
  validation harness with application).
- "Adjacent to legacy" = under `src/` (mixes Architecture B).

### Option 1 — `RobotOS_v1.0/app/<product>/`

| Aspect | Value |
|---|---|
| Purpose | Adjacent-to-Framework sibling directory holding one subdirectory per product / application. Each product owns its own state machine composition, mapping table, vocabulary, and harness integration. |
| Future files likely touched | `RobotOS_v1.0/app/<product>/<product>.c/h`, `RobotOS_v1.0/app/<product>/CMakeLists.txt`, `RobotOS_v1.0/app/<product>/README.md`, plus host tests under `RobotOS_v1.0/tests/host/test_app_<product>_*.c`. |
| Relationship to `framework/` | Consumer-only. Includes `robotos_fw_fsm.h`, `robotos_fw_event_bridge.h`. Does not modify Framework. |
| Relationship to `devkit/` | None. The application does not include any devkit header, does not call any `devkit_*` function, and does not depend on `devkit_app_state`. |
| Relationship to command set | Independent. The application owns its own vocabulary, if any. `a/s/r/?/x/v/L/d/T` remain devkit/probe surface. |
| CMake / build implications | Future `app/<product>/CMakeLists.txt` is a leaf build entry that wires only Architecture A sources. The application build attaches to a future application target — separate from the devkit validation build. Root legacy `RobotOS_v1.0/CMakeLists.txt` is NOT used. |
| Product scalability | Excellent. New products land at `app/<new_product>/` without disturbing existing products. |
| Risk | Low. The path is brand-new, has no legacy collision, and is short enough to be ergonomic. Only risk: a future user might think `app/` is for samples — mitigated by §F responsibility list and §10 of the boundary spec. |
| Recommendation | **RECOMMENDED.** |

### Option 2 — `RobotOS_v1.0/application/<product>/`

| Aspect | Value |
|---|---|
| Purpose | Same idea as Option 1 but with the longer name `application/`. |
| Future files likely touched | Equivalent to Option 1 with longer paths. |
| Relationship to `framework/` | Same as Option 1. |
| Relationship to `devkit/` | Same as Option 1. |
| Relationship to command set | Same as Option 1. |
| CMake / build implications | Same as Option 1 but longer relative paths in include directives and CMake `target_include_directories`. |
| Product scalability | Same as Option 1. |
| Risk | Low. Same structural properties as Option 1. The only material downside is path length and lack of repo precedent — every other top-level directory in `RobotOS_v1.0/` uses a short name (`core/`, `platform/`, `devkit/`, `framework/`, `tests/`, `src/`, `include/`). |
| Recommendation | **REJECTED at Phase 12G.** Same structural choice as Option 1 with worse ergonomics; no repo precedent for the longer form. |

### Option 3 — `RobotOS_v1.0/devkit/app/`

| Aspect | Value |
|---|---|
| Purpose | Place application code inside the devkit tree. |
| Future files likely touched | `RobotOS_v1.0/devkit/app/<product>/*`, `RobotOS_v1.0/devkit/CMakeLists.txt` (would need to add the application target). |
| Relationship to `framework/` | Consumer. |
| Relationship to `devkit/` | **Inside** the devkit. The application and the devkit validation harness share a directory tree; tooling, RTT, log streams, and evidence baselines bleed across. |
| Relationship to command set | Risk of accidental collision: the application might bind to the devkit UART byte stream (`a/s/r/...`) by sharing producers. |
| CMake / build implications | The application would link into the same firmware target as the devkit validation harness. Hardware evidence for the devkit then *includes* application behavior, breaking the Phase 9-11 evidence claim. |
| Product scalability | Poor. Mixing application and validation in a single tree creates a Frankenlayer that grows with both. |
| Risk | **High.** Violates the Phase 12G-pre §C ("blur the devkit validation harness vs. the Framework / Application layer") concern directly. |
| Recommendation | **REJECTED.** |

### Option 4 — `RobotOS_v1.0/framework/app/`

| Aspect | Value |
|---|---|
| Purpose | Place application code inside `framework/`. |
| Future files likely touched | `RobotOS_v1.0/framework/app/<product>/*`, plus Framework `CMakeLists.txt` (would need to add the application target). |
| Relationship to `framework/` | **Inside**. The Framework loses its product-neutral guarantee. Any `IDLE/ARMED/ACTIVE` symbol under `framework/app/` would directly conflict with Phase 12B/12C/12F's frozen product-neutral boundary. |
| Relationship to `devkit/` | Indirect; the framework and devkit would still be siblings, but Framework-internal product symbols would leak into anyone consuming Framework headers. |
| Relationship to command set | Indirect — but the Framework boundary itself is broken. |
| CMake / build implications | Framework would acquire product-specific build entries, breaking its product-neutral build promise. |
| Product scalability | Poor. Adding products grows the Framework tree, defeating the LOCK-AT-12F boundary intent. |
| Risk | **High.** Directly violates `framework/`'s product-neutral guarantee. |
| Recommendation | **REJECTED.** |

### Option 5 — `RobotOS_v1.0/examples/<scenario>/`

| Aspect | Value |
|---|---|
| Purpose | A separate `examples/` tree for sample integrations / demonstration scenarios that exercise the Framework path without claiming to be production application code. |
| Future files likely touched | `RobotOS_v1.0/examples/<scenario>/*.c/h`, `RobotOS_v1.0/examples/<scenario>/CMakeLists.txt`, `RobotOS_v1.0/examples/<scenario>/README.md`. |
| Relationship to `framework/` | Consumer. |
| Relationship to `devkit/` | None. |
| Relationship to command set | None. Examples should not bind to the devkit UART byte stream. |
| CMake / build implications | Examples could be host-only, or have their own firmware target separate from devkit. |
| Product scalability | Good for samples; not authoritative for products. |
| Risk | Low. Useful direction, but does not answer the *product/application* path question. Mixing "real product application" and "demonstration sample" under the same tree would create the same blend problem in a different form. |
| Recommendation | **USEFUL LATER, NOT INSTEAD OF OPTION 1.** Examples can land under `RobotOS_v1.0/examples/` *in addition to* `RobotOS_v1.0/app/<product>/`. Phase 12G does not authorize `examples/` either; it is recorded as a future possibility. |

### Comparison summary

| # | Option | Adjacent to | Risk | Scalable | Recommendation |
|---|---|---|---|---|---|
| 1 | `RobotOS_v1.0/app/<product>/` | Framework siblings | Low | Yes | **RECOMMENDED** |
| 2 | `RobotOS_v1.0/application/<product>/` | Framework siblings | Low | Yes | Rejected (ergonomics; no repo precedent) |
| 3 | `RobotOS_v1.0/devkit/app/` | Devkit harness | High | No | Rejected (blends validation + application) |
| 4 | `RobotOS_v1.0/framework/app/` | Framework internals | High | No | Rejected (violates Framework product-neutral boundary) |
| 5 | `RobotOS_v1.0/examples/<scenario>/` | Top-level sibling | Low | Sample-only | Useful later; not the product/application path |

---

## E. Recommended Application Boundary

**Decision result:** `PHASE_12G_RECOMMEND_APP_PRODUCT_PATH`.

### Meaning

- Future active application code lives under
  `RobotOS_v1.0/app/<product>/`.
- **No `app/` directory is created in Phase 12G.** The path is
  reserved at planning depth; no source file, README, or
  `CMakeLists.txt` lands.
- **The first product / application name is not selected in Phase
  12G.** That decision is the Phase 12H-pre gate (see §O).
- The path reserves a clean boundary separate from `devkit/` and
  `framework/`. It does not touch `src/`, `include/robotos/`, or any
  legacy artifact.

### Rationale

- The shortest path that does not blend with either the devkit
  validation harness or the Framework product-neutral boundary.
- Has no legacy collision: there is no `app/` under
  `RobotOS_v1.0/` today, and no Architecture B path uses the same
  name at this level.
- Scales to multiple products (`app/<product_a>/`,
  `app/<product_b>/`, …) without disturbing existing products or the
  Framework / devkit trees.
- Easy to attach future host tests at `tests/host/test_app_*.c`
  using the existing host CMake; easy to add a future
  `app/<product>/CMakeLists.txt` that attaches to Architecture A
  only.
- Compatible with the explicit non-goals in §M: no implementation,
  no CMake, no devkit change, no command-set change.

---

## F. Proposed App Layer Responsibility

The future `app/<product>/` directory **may own**:

| Responsibility | Detail |
|---|---|
| Product / application state machine composition | A `robotos_fw_fsm_t` instance with its own transition table, state defs, and vocabulary (`IDLE`, `ARMED`, `ACTIVE`, … — or any product-specific names). |
| Mapping table | An array of `robotos_fw_event_bridge_row_t` translating Adapter event keys (`adapter_type`, `adapter_arg0`) into the application's Framework event IDs. |
| Product event IDs | Application-specific `robotos_fw_event_id_t` values. The Framework imposes no product vocabulary; the application defines its own. |
| Product command vocabulary, if any | The application may grow its own command interface (e.g., a separate UART channel, a network protocol, a button gesture vocabulary). Such a command surface is **separate** from the devkit's `a/s/r/?/x/v/L/d/T`. |
| Product-specific policy around sensors / actuators | Sensor-fusion logic, actuator-control logic, calibration policy — all product-owned. |
| Product-specific build / harness integration | Application-owned `CMakeLists.txt`, `prj.conf` overlay, board DTS overlay (if any). Build is a leaf entry that does not modify the root or devkit CMake. |
| Product-level validation scripts / docs | Application-owned host tests, runbooks, RTT log expectations, and acceptance criteria. |

The future `app/<product>/` directory **must not own**:

| Non-responsibility | Reason |
|---|---|
| Core queue / dispatcher internals | `core/` is canonical; product code is a consumer, not a maintainer. |
| Platform backend primitives | `platform/` owns critical-section, time, fault, log boundaries. |
| Devkit validation command semantics | `a/s/r/?/x/v/L/d/T` belong to `devkit/`; product code does not bind to them. |
| Framework generic FSM / bridge algorithms | `framework/` is product-neutral and `LOCKED-AT-12F`. Product code is a consumer, not a maintainer. |
| Legacy Architecture B | `src/` and `include/robotos/` are frozen at Phase 12D-pre; product code uses Architecture A only. |
| `devkit_app_state` | Devkit-local; scope-guard #11 enforced. Product code does not include, read, write, or duplicate it. |

---

## G. Event Mapping Policy At Application Layer

1. **The application owns the mapping.** The future
   `app/<product>/` defines its mapping table (a static const array
   of `robotos_fw_event_bridge_row_t`) translating Adapter event
   keys (`uint32_t adapter_type, uint32_t adapter_arg0`) into the
   application's Framework event IDs.
2. **The Framework bridge stays product-neutral.** No product
   vocabulary appears in `framework/robotos_fw_event_bridge.{h,c}`.
   This is already locked at Phase 12F (`LOCKED-AT-12F`).
3. **Product event IDs live in the application's namespace.** The
   application defines its own `robotos_fw_event_id_t` constants /
   enums. These do not need to be coordinated with the Framework
   bridge or with any other application.
4. **No new `ROBOTOS_EVENT_USER` subrange is required for the
   Framework core.** Adapter event types remain core-defined
   (`ROBOTOS_EVENT_USER`, `USER+1`, `USER+2`, `USER+3`, …); the
   application translates these to Framework event IDs at the
   bridge.
5. **A future application may define its own event ID ranges.**
   Multiple applications can coexist; each has its own private
   Framework event ID namespace because it owns its own bridge
   instance + FSM instance.
6. **Mapping must be tested host-first before runtime / hardware
   integration.** A new host test target (e.g.,
   `tests/host/test_app_<product>_mapping.c`) validates the mapping
   table against the application's FSM contract. Only after host
   validation may a hardware run be authorized — and that is a later
   phase.

---

## H. Build Separation Strategy

Phase 12G does not introduce any build change. Planning-level
direction:

1. **No build change in Phase 12G.** Zero CMake / `prj.conf` / DTS /
   overlay / Zephyr config diffs.
2. **Future `app/<product>/` builds attach to Architecture A only.**
   The application target must not `add_subdirectory` from the root
   legacy `RobotOS_v1.0/CMakeLists.txt`, and must not include any
   `include/robotos/` header.
3. **Future application builds must be separable** from the devkit
   validation build. Hardware evidence for the devkit and hardware
   evidence for an application target must remain distinct; they
   cannot share the same firmware binary.
4. **Prefer host-first application tests** before any devkit
   hardware build. The first concrete validation of an application's
   mapping table happens under `tests/host/CMakeLists.txt` (or a
   sibling host test build) — the same WSL Ubuntu / gcc 13.3.0
   environment used by Phase 12E / 12F.
5. **Three future CMake options to evaluate** at the relevant
   implementation phase (not in Phase 12G):
   - **Option A:** new host test target inside the existing
     `tests/host/CMakeLists.txt` for the application mapping
     (additive only, like Phase 12E + Phase 12F);
   - **Option B:** a new `app/<product>/CMakeLists.txt` that
     produces a separate (firmware or host) target;
   - **Option C:** devkit integration is *not* authorized — a
     separate phase, after a working host validation, decides whether
     and how the application target ever runs on the board.
6. **Phase 12G does not choose the exact CMake implementation yet.**
   That decision sits with the implementation phase that creates the
   first application source.

---

## I. Validation Strategy

Staged validation, in order. Each stage must close before the next
opens.

| Stage | Gate | Validation | Status at Phase 12G |
|---|---|---|---|
| 1 | Docs-only application boundary plan | This closeout + `FRAMEWORK_APPLICATION_BOUNDARY_DRAFT.md`. | **CLOSED at Phase 12G** |
| 2 | First product selection (docs-only) | Phase 12H-pre — First Application Candidate / Product Harness Selection. Selects the first concrete product name and minimal scenario; no source. | `NOT_STARTED` |
| 3 | Host-only application mapping prototype | First `app/<product>/` source + `tests/host/test_app_<product>_*.c`; host regression PASS under WSL Ubuntu / gcc 13.3.0. | `NOT_STARTED` |
| 4 | Host regression baseline preserved | Full host regression (currently 22/22) must still PASS after Stage 3. | `NOT_STARTED` |
| 5 | Optional devkit shadow (only if user explicitly authorizes Mode 2 from Phase 12G-pre §D) | Phase 12G-pre §G fallback gate; separate spec required. | `NOT_AUTHORIZED` |
| 6 | Hardware evidence | Only after Stages 3-4 are closed AND an explicit runtime integration phase has been opened. | `NOT_AUTHORIZED` |

Phase 12G closes **Stage 1 only**.

---

## J. Relationship to `devkit_app_state`

- `devkit_app_state` remains **authoritative** for the current
  devkit runtime state machine. It owns `IDLE/ARMED/ACTIVE` for the
  devkit, the `?` UART response, the `a/s/r/d/t/T` byte handlers,
  the button cycle, and the `ROBOTOS_APP` periodic log line.
- **Separate Application Mode does not replace, shadow, copy, or
  promote `devkit_app_state`.** The application layer may define a
  state machine *with the same names* (e.g., `IDLE/ARMED/ACTIVE`) —
  but it does so inside its own `robotos_fw_fsm_t` instance, in its
  own namespace, in `app/<product>/`. The devkit firmware does not
  consume the application's state.
- **No `?` response change.** The devkit's `?` continues to come
  from `devkit_app_state`, unchanged.
- **No `a/s/r/?/x/v/L/d/T` semantic change.**
- Any future interaction between the application layer and
  `devkit_app_state` (e.g., a shared shadow runtime, or a
  replacement migration) requires a **separate planning phase**.
  Phase 12G does not authorize such interaction.
- **Scope-guard #11 remains active.**

---

## K. Relationship to Command Set

- The current command set `a / s / r / ? / x / v / L / d / T` remains
  the **devkit / probe surface**. Validated through Phase 11Z.
- **Product / application command vocabulary is not defined in Phase
  12G.** If a future application grows a command interface, it does
  so inside its own `app/<product>/` directory, on its own channel,
  with its own framing rules — separate from the devkit UART byte
  stream.
- **Framework is not exposed via UART automatically.** The Framework
  bridge has no UART surface (Phase 12F locked). The application
  layer chooses whether and how to expose any Framework state
  externally, and that exposure is the application's responsibility.
- **Future application commands require a separate product command
  phase.** Such a phase, when authorized, must:
  - list every byte / message it introduces;
  - declare which channel it uses (separate UART, network,
    shared-bus, etc.);
  - state explicitly that it does NOT modify
    `a/s/r/?/x/v/L/d/T` semantics;
  - identify its validation strategy.

---

## L. Relationship to Legacy Architecture B

- `RobotOS_v1.0/src/` and `RobotOS_v1.0/include/robotos/` remain the
  **frozen legacy scaffold**, classified
  `LEGACY_SCAFFOLD_MARKED_FROZEN_DOCS_ONLY` at Phase 12D-pre.
- The future `app/<product>/` directory **does not reuse**
  `src/app/`, `include/robotos/app*`, or any other Architecture B
  artifact.
- The future application uses **Architecture A contracts only**:
  `robotos_core_*` (from `core/`), `robotos_platform_*` (from
  `platform/`), `robotos_fw_*` (from `framework/`). It must not
  include any `ro_*` HAL header.
- **No Architecture A ↔ Architecture B reconciliation in Phase
  12G.** That remains a separate, deferred question.
- `RobotOS_v1.0/src/README_LEGACY_SCAFFOLD.md`,
  `RobotOS_v1.0/src/framework/DEPRECATED.md`, and
  `RobotOS_v1.0/include/robotos/DEPRECATED.md` are not modified by
  Phase 12G.

---

## M. Non-goals (Phase 12G)

| Non-goal | Status |
|---|---|
| `app/` directory created | NO — reserved at planning depth only |
| `application/` directory created | NO |
| Application source file | NO |
| CMake change | NO |
| Framework code change | NO (`framework/` zero-diff) |
| Devkit runtime change | NO (`devkit/src/` zero-diff) |
| `devkit_app_state` change | NO |
| UART command added | NO |
| Command semantic change | NO |
| Hardware run | NO |
| Scheduler 7A/7B reopened | NO — DEFER preserved |
| F407 / custom board reopened | NO — HOLD/DEFER preserved |
| ACTIVE disarm widening started | NO — `USER_DECISION_REQUIRED_ACTIVE_DISARM` preserved |
| POST_FLASH_AUTOSTART change | NO — `OPEN/MITIGATED_BY_WORKFLOW` preserved |
| Legacy Architecture B modification | NO — frozen |
| Evidence logs touched | NO |
| Phase 12H-pre opened | NO — Phase 12H-pre `NOT_STARTED`; requires explicit user authorization |

---

## N. Decision Result

**`PHASE_12G_RECOMMEND_APP_PRODUCT_PATH`**

Future active application code lives under
`RobotOS_v1.0/app/<product>/`. The directory is not created in Phase
12G; the path is reserved at planning depth. The first product /
application name is selected at Phase 12H-pre (see §O).

---

## O. Next Gate Recommendation

**Phase 12H-pre — First Application Candidate / Product Harness
Selection**

| Field | Value |
|---|---|
| Title | Phase 12H-pre — First Application Candidate / Product Harness Selection |
| Classification | Docs-only product / application planning |
| Purpose | Decide the first concrete product / app harness name (the `<product>` placeholder in `app/<product>/`), the minimal scenario it covers, the minimal event vocabulary, and the host mapping test plan, BEFORE any `app/<product>/` directory is created. |
| In-scope | Selection of the first product name (e.g., `app/demo/`, `app/probe_translator/`, `app/<actual_product_name>/`); enumeration of the minimal state vocabulary; enumeration of the minimal Adapter-event-to-Framework-event mapping rows; host test plan for that mapping; relationship to existing devkit producers (USER+2 button / USER+3 UART) — read-only consumer surface only. |
| Non-goals | No source files. No `app/<product>/` directory creation. No `devkit/src/` change. No `devkit_app_state` change. No command-set change. No CMake change. No hardware run. No actual application implementation. No Framework code change. |
| Exit criteria | A `PHASE_12H_PRE_*.md` closeout that selects the first product / app name and freezes the planning depth contract. A revision of `FRAMEWORK_APPLICATION_BOUNDARY_DRAFT.md` that fills in the `<product>` placeholder for the first instance. |
| Authorization | Must be opened only on **explicit user approval**. Phase 12G's recommendation is not authorization. |

Alternative if no product direction is available:

- **HOLD.** No next phase. Phase 12G shelves the boundary spec at
  planning depth; the `<product>` placeholder remains unbound. The
  Framework path stays host-tested only.

---

## P. Cross-references

- Phase 12G-pre integration-mode decision:
  [`PHASE_12G_PRE_DEVKIT_INTEGRATION_MODE_DECISION.md`](PHASE_12G_PRE_DEVKIT_INTEGRATION_MODE_DECISION.md)
- Long-lived integration-mode spec:
  [`../03_SPECS/FRAMEWORK_DEVKIT_INTEGRATION_MODE_DRAFT.md`](../03_SPECS/FRAMEWORK_DEVKIT_INTEGRATION_MODE_DRAFT.md)
- Phase 12F bridge host prototype:
  [`PHASE_12F_APPLICATION_BRIDGE_HOST_PROTOTYPE.md`](PHASE_12F_APPLICATION_BRIDGE_HOST_PROTOTYPE.md)
- Long-lived bridge spec:
  [`../03_SPECS/FRAMEWORK_APPLICATION_BRIDGE_DRAFT.md`](../03_SPECS/FRAMEWORK_APPLICATION_BRIDGE_DRAFT.md)
- Phase 12E FSM implementation:
  [`PHASE_12E_FSM_HOST_TEST_IMPLEMENTATION.md`](PHASE_12E_FSM_HOST_TEST_IMPLEMENTATION.md)
- Long-lived FSM spec:
  [`../03_SPECS/FRAMEWORK_FSM_API_DRAFT.md`](../03_SPECS/FRAMEWORK_FSM_API_DRAFT.md)
- Phase 12D-pre legacy disposition:
  [`PHASE_12D_PRE_LEGACY_FRAMEWORK_SCAFFOLD_DISPOSITION.md`](PHASE_12D_PRE_LEGACY_FRAMEWORK_SCAFFOLD_DISPOSITION.md)
- Phase 11Z command-set checkpoint:
  [`PHASE_11Z_COMMAND_SET_CHECKPOINT.md`](PHASE_11Z_COMMAND_SET_CHECKPOINT.md)
- New long-lived application boundary spec:
  [`../03_SPECS/FRAMEWORK_APPLICATION_BOUNDARY_DRAFT.md`](../03_SPECS/FRAMEWORK_APPLICATION_BOUNDARY_DRAFT.md)
- Live state: [`../../../../CURRENT_STATE.md`](../../../../CURRENT_STATE.md)
