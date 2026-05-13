# RobotOS Framework — Application Boundary Draft Spec

**Status:** `DRAFT / NON-FINAL`. **No application implementation
exists yet.** This document describes the planning-level boundary
between the Framework path (FSM + Application Event Bridge) and the
future application / product layer. No `app/` or `application/`
directory has been created, and no application `.c` / `.h` /
`CMakeLists.txt` / README file exists.
**Revision:** Phase 12G (2026-05-13, `CLOSED_DOCS_ONLY`;
`PHASE_12G_RECOMMEND_APP_PRODUCT_PATH`) — initial draft.
**Next revision condition:** Phase 12H-pre (when authorized) — when
the first concrete product / app name is selected, this spec gains
the resolved `<product>` placeholder and the first minimal mapping
vocabulary.

> **No application implementation exists.** The path
> `RobotOS_v1.0/app/<product>/` is *reserved at planning depth*, not
> created. No file under this path exists. No build target references
> this path. The boundary described here applies to the future
> implementation phase and is not a deployed binding.

This spec is anchored to
[`../02_PHASE_CLOSEOUTS/PHASE_12G_SEPARATE_APPLICATION_BOUNDARY_PLANNING.md`](../02_PHASE_CLOSEOUTS/PHASE_12G_SEPARATE_APPLICATION_BOUNDARY_PLANNING.md)
and references
[`FRAMEWORK_DEVKIT_INTEGRATION_MODE_DRAFT.md`](FRAMEWORK_DEVKIT_INTEGRATION_MODE_DRAFT.md),
[`FRAMEWORK_APPLICATION_BRIDGE_DRAFT.md`](FRAMEWORK_APPLICATION_BRIDGE_DRAFT.md),
and
[`FRAMEWORK_FSM_API_DRAFT.md`](FRAMEWORK_FSM_API_DRAFT.md).

---

## 1. Status / Scope

**What this doc is:**
A long-lived planning-level spec that describes the boundary between
the Framework path and a future product / application layer, under
the Separate Application Mode chosen by Phase 12G-pre. It records the
decision result `PHASE_12G_RECOMMEND_APP_PRODUCT_PATH`, the reserved
directory shape (`RobotOS_v1.0/app/<product>/`), the responsibility
split, and the validation strategy.

**What this doc is not:**
- An application implementation. No `.c` / `.h` / `CMakeLists.txt`
  exists at `app/<product>/`.
- A product vocabulary spec. The first `<product>` placeholder is
  filled in by a future Phase 12H-pre.
- A devkit integration spec. Direct devkit integration remains
  forbidden; the Framework path is consumed by the future
  application layer, not by the devkit firmware.
- A `devkit_app_state` replacement design. `devkit_app_state`
  remains authoritative for the current devkit runtime.
- A command-vocabulary spec. The frozen `a/s/r/?/x/v/L/d/T` devkit
  command set is unchanged.

**Current decision state (Phase 12G):**

| Decision | Status |
|---|---|
| Integration mode (Phase 12G-pre) | `PHASE_12G_PRE_RECOMMEND_SEPARATE_APPLICATION_MODE_PLANNING` |
| Application directory shape | `RobotOS_v1.0/app/<product>/` (reserved at planning depth; **not created**) |
| First product name (`<product>`) | Open — selected at Phase 12H-pre |
| Application layer ownership | Mapping table, FSM instance, bridge instance, vocabulary, validation; see §4 |
| Build separation strategy | Planning-level only; see §6. No CMake change at Phase 12G. |
| Validation strategy | Five-stage; see §7. Phase 12G closes Stage 1 only. |
| `devkit_app_state` | Authoritative for devkit runtime; not touched by Separate Application Mode |
| Command set | `a/s/r/?/x/v/L/d/T` unchanged |
| Architecture B | Frozen; not reachable from `app/<product>/` |

---

## 2. Application Boundary Decision

**Decision:** `PHASE_12G_RECOMMEND_APP_PRODUCT_PATH`.

Future active application code lives at
`RobotOS_v1.0/app/<product>/`. The directory:

- is **reserved at planning depth** by this spec and by the Phase
  12G closeout;
- is **not created** by Phase 12G;
- is **opened** (filled in with a concrete `<product>` name and a
  minimal scaffold) only after Phase 12H-pre — First Application
  Candidate / Product Harness Selection — selects the first
  product;
- does **not** appear in any `CMakeLists.txt`, `prj.conf`, board
  DTS, overlay, or Zephyr config file.

Rejected shapes (full evaluation in
[`../02_PHASE_CLOSEOUTS/PHASE_12G_SEPARATE_APPLICATION_BOUNDARY_PLANNING.md`](../02_PHASE_CLOSEOUTS/PHASE_12G_SEPARATE_APPLICATION_BOUNDARY_PLANNING.md)
§D):

- `RobotOS_v1.0/application/<product>/` — ergonomic downgrade; no
  repo precedent for the longer form.
- `RobotOS_v1.0/devkit/app/` — blends application with devkit
  validation harness; high risk.
- `RobotOS_v1.0/framework/app/` — violates Framework
  product-neutral boundary.
- `RobotOS_v1.0/examples/<scenario>/` — useful for samples later;
  not the authoritative product / application path.

---

## 3. Proposed Directory Shape

Planning-level intent (locks at Phase 12H-pre or later):

```text
RobotOS_v1.0/
├── core/                                (Architecture A)
├── platform/                            (Architecture A)
├── devkit/                              (Architecture A; devkit validation harness)
│   └── src/                             (devkit_app_state lives HERE; unchanged)
├── framework/                           (Architecture A; product-neutral)
│   ├── robotos_fw_fsm.{h,c}             (Phase 12D LOCKED / Phase 12E body)
│   └── robotos_fw_event_bridge.{h,c}    (Phase 12F; LOCKED-AT-12F)
├── tests/
│   ├── host/                            (host test infrastructure;
│   │                                     additive only)
│   └── ...
├── app/                                 (RESERVED AT PLANNING DEPTH;
│   │                                     NOT CREATED at Phase 12G)
│   └── <product>/                       (filled in at Phase 12H-pre
│                                         or later)
│       ├── <product>.c                  (future application source)
│       ├── <product>.h                  (future application header,
│       │                                 if any)
│       ├── CMakeLists.txt               (future leaf build entry;
│       │                                 Architecture A only)
│       └── README.md                    (future application docs)
├── src/                                 (Architecture B — frozen at
│                                         Phase 12D-pre)
├── include/robotos/                     (Architecture B — frozen at
│                                         Phase 12D-pre)
└── CMakeLists.txt                       (Architecture B build entry;
                                          NOT USED by app/<product>/)
```

Notes:

- The `app/` directory is intentionally a **sibling** of `framework/`
  and `devkit/`, not a child of either. This is the structural
  invariant that prevents blend (devkit) or pollution (framework).
- The `<product>` placeholder is a single literal directory name
  chosen at Phase 12H-pre (e.g., `demo`, `probe_translator`, or an
  actual product codename). There is no implicit list of placeholders
  in this spec.
- Multiple products can coexist under `app/` (`app/demo/`,
  `app/<product_a>/`, …) without modifying each other or the
  Framework / devkit trees.

---

## 4. Layer Responsibilities

### 4.1 What `app/<product>/` may own

| Responsibility | Detail |
|---|---|
| Product / application state machine composition | A `robotos_fw_fsm_t` instance, its transition table, its state defs, and its vocabulary (state names, event names — product-defined). |
| Mapping table | Static const array of `robotos_fw_event_bridge_row_t` translating Adapter event keys to the application's Framework event IDs. |
| Product event IDs | Application-defined `robotos_fw_event_id_t` constants / enums. |
| Product command vocabulary, if any | Application's own command surface (separate channel, separate framing); not part of `a/s/r/?/x/v/L/d/T`. |
| Product-specific sensor / actuator policy | Calibration, fusion, control logic. |
| Product-specific build / harness integration | `app/<product>/CMakeLists.txt`, `prj.conf` overlay, board overlay (if any). |
| Product-level validation scripts and docs | Host tests for the mapping table; runbooks; RTT log expectations; acceptance criteria. |

### 4.2 What `app/<product>/` must NOT own

| Non-responsibility | Reason |
|---|---|
| Core queue / dispatcher internals | `core/` is canonical; product code is a consumer. |
| Platform backend primitives | `platform/` owns critical-section, time, fault, log boundaries. |
| Devkit validation command semantics | `a/s/r/?/x/v/L/d/T` belong to `devkit/`. |
| Framework generic FSM / bridge algorithms | `framework/` is `LOCKED-AT-12F` for names + signatures; product code is a consumer. |
| Legacy Architecture B (`ro_*` HAL) | Frozen at Phase 12D-pre. |
| `devkit_app_state` | Devkit-local; scope-guard #11 enforced. |

### 4.3 Layer-by-layer summary

```text
[ app/<product>/                ]  PRODUCT (future; NOT_CREATED at 12G)
   |                                - owns FSM instance, bridge instance,
   |                                  mapping table, vocabulary
   v
[ framework/robotos_fw_event_   ]  FRAMEWORK BRIDGE (product-neutral; LOCKED-AT-12F)
   bridge.{h,c}                     - mapping engine; FIFO first-match;
   |                                  wildcard arg0; borrowed payload
   v
[ framework/robotos_fw_fsm.{h,c}]  FRAMEWORK FSM (product-neutral; LOCKED-AT-12D)
   |                                - flat table-driven FSM;
   |                                  reuse robotos_core_status_t
   v
[ core/ + platform/             ]  ADAPTER / RUNTIME SUBSTRATE
   |                                - robotos_core_*, robotos_platform_*
   v
[ devkit/src/                   ]  DEVKIT VALIDATION HARNESS
                                    - devkit_app_state owns IDLE/ARMED/ACTIVE
                                      for devkit runtime; UNTOUCHED by
                                      Separate Application Mode
```

The application reads downward only (consumer of Framework / core /
platform). It never reads from or writes to `devkit/src/`.

---

## 5. Event Mapping Policy

1. **Application owns the mapping.** The future `app/<product>/`
   declares a static const array of
   `robotos_fw_event_bridge_row_t`. Each row maps an Adapter event
   key (`adapter_type`, `adapter_arg0`, `match_arg0`) to a
   product-defined `robotos_fw_event_id_t`.
2. **Framework bridge stays product-neutral.** No product
   vocabulary enters `framework/robotos_fw_event_bridge.{h,c}`.
   Locked at Phase 12F (`LOCKED-AT-12F`).
3. **Product event IDs are application-local.** They live in the
   application's namespace. They are not coordinated with other
   applications or with the Framework core.
4. **No new `ROBOTOS_EVENT_USER` subrange is required.** Adapter
   event types remain core-defined (`ROBOTOS_EVENT_USER`,
   `ROBOTOS_EVENT_USER+1`, etc.); the application translates these
   to Framework event IDs at the bridge.
5. **Multiple applications can coexist** — each has its own private
   Framework event ID namespace because each owns its own bridge
   instance + FSM instance.
6. **Mapping is host-tested before any runtime / hardware run.**
   First validation is a host test target under `tests/host/` (or a
   sibling host test build); only after host validation may a
   runtime / hardware run be authorized.
7. **No fan-out to multiple FSMs at Phase 12G.** Phase 12F-pre §11
   #5 already deferred fan-out; an application's mapping table
   feeds exactly one FSM instance (the application's own).

---

## 6. Build Separation Strategy

Planning-level direction (locks at the relevant implementation
phase, not at Phase 12G):

1. **No build change at Phase 12G.** Zero CMake / `prj.conf` / DTS
   / overlay / Zephyr config diffs.
2. **Future `app/<product>/` builds attach to Architecture A only.**
   They must not `add_subdirectory` from the root legacy
   `RobotOS_v1.0/CMakeLists.txt` and must not include any
   `include/robotos/` header.
3. **Application builds are separable from the devkit validation
   build.** Hardware evidence for the devkit and any future hardware
   evidence for an application target are produced from distinct
   firmware binaries.
4. **Prefer host-first application tests.** The first concrete
   validation of an application's mapping table happens under
   `tests/host/CMakeLists.txt` (or a sibling host test build) — same
   WSL Ubuntu / gcc 13.3.0 environment as Phase 12E / 12F.
5. **Three future CMake options** (decided at the implementation
   phase that adds the first application source — not at Phase
   12G):

   - **Option A:** new host test target inside the existing
     `tests/host/CMakeLists.txt` (additive only).
   - **Option B:** new `app/<product>/CMakeLists.txt` producing a
     separate target (host-only at first; possibly firmware later
     under explicit authorization).
   - **Option C:** Devkit integration is *not* authorized;
     hardware evidence for an application target requires a
     separate, later phase.

6. **Phase 12G does not choose the exact CMake implementation.**

---

## 7. Validation Strategy

Staged validation; each stage closes before the next opens.

| Stage | Gate | Required artifact | Status at Phase 12G |
|---|---|---|---|
| 1 | Docs-only application boundary plan | This spec + the Phase 12G closeout. | **CLOSED at Phase 12G** |
| 2 | First product selection | Phase 12H-pre — First Application Candidate / Product Harness Selection (docs-only). Picks `<product>` and minimal scenario. | NOT_STARTED |
| 3 | Host-only application mapping prototype | First `app/<product>/` source + `tests/host/test_app_<product>_*.c`; host regression PASS under WSL Ubuntu / gcc 13.3.0. | NOT_STARTED |
| 4 | Host regression baseline preserved | Full host regression (currently 22/22) still PASS after Stage 3. | NOT_STARTED |
| 5 | Optional devkit shadow | Only if user explicitly authorizes Mode 2 from Phase 12G-pre §D; separate spec required. | NOT_AUTHORIZED |
| 6 | Hardware evidence | Only after Stages 3–4 and an explicit runtime-integration phase. | NOT_AUTHORIZED |

Phase 12G closes Stage 1 only.

---

## 8. `devkit_app_state` Boundary

- `devkit_app_state` is **authoritative** for the current devkit
  runtime. It owns `IDLE/ARMED/ACTIVE` for the devkit, the `?` UART
  response, the `a/s/r/d/t/T` byte handlers, the button cycle, and
  the `ROBOTOS_APP` periodic log line.
- **Separate Application Mode does not replace, shadow, copy, or
  promote `devkit_app_state`.** An application may define a state
  machine with overlapping names — but it does so in its own
  `robotos_fw_fsm_t` instance, in `app/<product>/`. The devkit
  firmware does not consume the application's state.
- **No `?` response change.**
- **No `a/s/r/?/x/v/L/d/T` semantic change.**
- Any future interaction between the application layer and
  `devkit_app_state` (shadow runtime; replacement migration)
  requires a **separate planning phase** beyond Phase 12H.
- **Scope-guard #11 remains active.**

---

## 9. Command Set Boundary

- The current command set `a / s / r / ? / x / v / L / d / T` remains
  the **devkit / probe surface**. Validated through Phase 11Z.
- **Product / application command vocabulary is not defined in Phase
  12G.** If an application grows a command interface, it does so
  inside `app/<product>/`, on its own channel, with its own framing.
- **Framework is not exposed via UART automatically.** The bridge
  has no UART surface (Phase 12F locked). The application decides
  whether and how to expose any state externally; that is the
  application's responsibility.
- **Future application command introduction requires a separate
  product-command phase.** Such a phase must list every byte / message
  it introduces, declare the channel, state explicitly that
  `a/s/r/?/x/v/L/d/T` semantics are unchanged, and provide a
  validation strategy.
- All 12 UART TX scope-guard constraints from Phase 9EZ §H preserved.

---

## 10. Legacy Architecture B Boundary

- `RobotOS_v1.0/src/` and `RobotOS_v1.0/include/robotos/` remain
  frozen at `LEGACY_SCAFFOLD_MARKED_FROZEN_DOCS_ONLY` (Phase 12D-pre).
- The future `app/<product>/` directory **does not reuse**
  `src/app/`, `include/robotos/app*`, or any other Architecture B
  artifact.
- The future application uses **Architecture A contracts only**:
  `robotos_core_*` (from `core/`), `robotos_platform_*` (from
  `platform/`), `robotos_fw_*` (from `framework/`).
- The future application **must not include** any `ro_*` HAL header
  or any header in `include/robotos/`.
- **No Architecture A ↔ Architecture B reconciliation in Phase
  12G.** That remains a separate, deferred question.

---

## 11. Open Decisions

Items intentionally left open at Phase 12G:

| # | Open question | Latest at |
|---|---|---|
| 1 | First `<product>` placeholder name (`demo`, `probe_translator`, actual codename, …) | Phase 12H-pre |
| 2 | Minimal initial state vocabulary for the first product | Phase 12H-pre |
| 3 | Minimal initial mapping table for the first product | Phase 12H-pre |
| 4 | First application host test name and location | Phase 12H-pre or first implementation phase |
| 5 | Exact CMake wiring choice (Option A / B / C from §6) | First application implementation phase |
| 6 | Whether the first application is also a hardware-runnable Zephyr application or strictly host-only at first | First application implementation phase |
| 7 | Whether to introduce `RobotOS_v1.0/examples/` for sample integrations in addition to `app/<product>/` | Future docs-only phase |
| 8 | Multi-product coordination rules (cross-app event ID coordination, shared utility code, etc.) | Future phase, only after at least two applications exist |
| 9 | Bridge ABI memory-layout lock — should it close before or after the first application proves the shape | First application implementation phase (or later) |

---

## 12. Next Revision Conditions

This spec is revised when:

1. **Phase 12H-pre opens (First Application Candidate / Product
   Harness Selection).** §2 gains a sentence recording the chosen
   `<product>`. §3 directory shape updates to reference the real
   product name. §11 #1, #2, #3 are resolved. §11 #4, #5 are
   either resolved or explicitly carried forward.
2. **The first `app/<product>/` source is created (post-Phase
   12H-pre).** §3 directory shape gains the materialized file list.
   §6 CMake option is selected. Bridge spec gains a §13 referring
   to the application as the first concrete consumer.
3. **A second application is added under `app/`.** §11 #8 becomes
   the lead question; multi-product coordination rules are added.
4. **A future phase introduces `RobotOS_v1.0/examples/`.** §11 #7
   is resolved; §3 directory shape adds the `examples/` sibling.
5. **A future phase reconciles or removes Architecture B.** §10 is
   updated to reflect the resolution.

This spec is **NOT** revised to:

- Record devkit implementation details (devkit docs are
  authoritative for devkit; this spec is authoritative for the
  application boundary only).
- Track per-phase test results (closeouts are authoritative).
- Pick a product's specific business logic.
- Change command semantics (frozen at 11Z).
- Promote `devkit_app_state` (scope-guard #11 enforced).
- Reactivate Architecture B (frozen at 12D-pre).
