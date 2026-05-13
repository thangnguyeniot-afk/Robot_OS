# RobotOS — First Application Candidate Draft Spec

**Status:** `IMPLEMENTED_AT_12I (HOST-TEST EVIDENCE)`. The first
application candidate path `RobotOS_v1.0/app/probe_translator/` is
materialized at Phase 12I with `probe_translator.{h,c}` + `README.md`
and is host-test validated (23/23 ctest targets PASS; 70/70 in-binary
assertions in the new `probe_translator_mapping_contract` target).
The detailed implementation contract lives in
[`PROBE_TRANSLATOR_HOST_PROTOTYPE_PLAN.md`](PROBE_TRANSLATOR_HOST_PROTOTYPE_PLAN.md).
The detailed skeleton lock lives in
[`PROBE_TRANSLATOR_APP_SKELETON_DRAFT.md`](PROBE_TRANSLATOR_APP_SKELETON_DRAFT.md).
**Revision:** Phase 12I (2026-05-13, `CLOSED_WITH_HOST_TEST_EVIDENCE`;
`PHASE_12I_PROBE_TRANSLATOR_HOST_PROTOTYPE_IMPLEMENTED_VALIDATED`) —
status upgraded; cross-reference to Phase 12I evidence
[`../02_PHASE_CLOSEOUTS/PHASE_12I_PROBE_TRANSLATOR_HOST_PROTOTYPE.md`](../02_PHASE_CLOSEOUTS/PHASE_12I_PROBE_TRANSLATOR_HOST_PROTOTYPE.md)
and host log
[`../../../../tests/host/logs/phase_12I_host_2026-05-13.log`](../../../../tests/host/logs/phase_12I_host_2026-05-13.log).
Sections 2–13 otherwise unchanged.
Phase 12H (2026-05-13, `CLOSED_DOCS_ONLY`;
`PHASE_12H_PROBE_TRANSLATOR_SKELETON_PLANNED_DOCS_ONLY`) — added
cross-reference to
[`PROBE_TRANSLATOR_APP_SKELETON_DRAFT.md`](PROBE_TRANSLATOR_APP_SKELETON_DRAFT.md).
Phase 12H-pre (2026-05-13, `CLOSED_DOCS_ONLY`;
`PHASE_12H_PRE_RECOMMEND_PROBE_TRANSLATOR_APP`) — initial draft.
**Next revision condition:** future app-behavior phase (FAULT block);
second application under `app/`; future devkit-integration phase.

> **Phase 12I implementation exists.** The path
> `RobotOS_v1.0/app/probe_translator/` is materialized;
> `probe_translator.{h,c}` and `README.md` are tracked and
> host-test-validated. Devkit / UART / Zephyr / hardware /
> Architecture-B boundaries remain unchanged.

This spec is anchored to
[`../02_PHASE_CLOSEOUTS/PHASE_12H_PRE_FIRST_APPLICATION_CANDIDATE_SELECTION.md`](../02_PHASE_CLOSEOUTS/PHASE_12H_PRE_FIRST_APPLICATION_CANDIDATE_SELECTION.md)
and references
[`FRAMEWORK_APPLICATION_BOUNDARY_DRAFT.md`](FRAMEWORK_APPLICATION_BOUNDARY_DRAFT.md),
[`FRAMEWORK_APPLICATION_BRIDGE_DRAFT.md`](FRAMEWORK_APPLICATION_BRIDGE_DRAFT.md),
[`FRAMEWORK_DEVKIT_INTEGRATION_MODE_DRAFT.md`](FRAMEWORK_DEVKIT_INTEGRATION_MODE_DRAFT.md),
and
[`FRAMEWORK_FSM_API_DRAFT.md`](FRAMEWORK_FSM_API_DRAFT.md).

---

## 1. Status / Scope

**What this doc is:**
A long-lived planning-level spec describing the first application
candidate (`probe_translator`) for the future
`RobotOS_v1.0/app/<product>/` boundary. It records the candidate's
purpose, minimal state vocabulary, minimal event vocabulary, initial
bridge mapping table, host-first validation plan, build-strategy
preference, and the boundary it preserves against `devkit_app_state`,
the command set, and Architecture B.

**What this doc is not:**
- An application implementation. No `.c` / `.h` / `CMakeLists.txt`
  exists at `app/probe_translator/`.
- A product spec. The `probe_translator` harness is product-neutral
  by design; it does not claim any real product behavior.
- A devkit integration spec. The candidate does not consume devkit
  producers or `devkit_app_state`.
- A command-vocabulary spec. The frozen `a/s/r/?/x/v/L/d/T` devkit
  command set is unchanged.
- A replacement for the application boundary spec. The boundary spec
  (`FRAMEWORK_APPLICATION_BOUNDARY_DRAFT.md`) is the umbrella; this
  spec narrows that boundary to one concrete first candidate.

**Current decision state (Phase 12H-pre):**

| Decision | Status |
|---|---|
| First `<product>` placeholder | `probe_translator` |
| First application path | `RobotOS_v1.0/app/probe_translator/` (reserved at planning depth; **not created**) |
| Harness type | Host-first, product-neutral, synthetic-event translator |
| Source / header / CMake / README | None exist; not created at Phase 12H-pre |
| State vocabulary | Planning-level: `APP_IDLE / APP_READY / APP_ACTIVE` + optional `APP_FAULT` |
| Event vocabulary | Planning-level: `APP_EVT_CONFIGURED / START / STOP / FAULT? / RESET` |
| Mapping table | Planning-level only; conceptual 5-row table |
| Build strategy preference | Option A — host test target inside existing `tests/host/CMakeLists.txt` |
| `devkit_app_state` | Authoritative for devkit runtime; **not** touched |
| Command set | `a/s/r/?/x/v/L/d/T` unchanged |
| Architecture B | Frozen; not reachable from `app/probe_translator/` |

---

## 2. Selected Candidate

**Selected candidate:** `probe_translator` (Phase 12H-pre Candidate 1).

The first `<product>` placeholder under `RobotOS_v1.0/app/<product>/`
is `probe_translator`. Future first application source lives at
`RobotOS_v1.0/app/probe_translator/`. The selection is recorded by
the Phase 12H-pre decision result
`PHASE_12H_PRE_RECOMMEND_PROBE_TRANSLATOR_APP`.

Rejected candidates (full evaluation in
[`../02_PHASE_CLOSEOUTS/PHASE_12H_PRE_FIRST_APPLICATION_CANDIDATE_SELECTION.md`](../02_PHASE_CLOSEOUTS/PHASE_12H_PRE_FIRST_APPLICATION_CANDIDATE_SELECTION.md)
§D):

- `app/demo/` — vague scope; reserved as a future sibling under
  `RobotOS_v1.0/examples/` only.
- `app/devkit_shadow/` — overrides Phase 12G-pre Mode 2 DEFER; not
  re-opened.
- `app/<real_product>/` — premature; needs an independent product
  direction decision.
- HOLD — acceptable fallback if the user prefers to stop.

---

## 3. Proposed Future Path

Planning-level intent (locks at Phase 12H or later):

```text
RobotOS_v1.0/
├── core/
├── platform/
├── devkit/
├── framework/
│   ├── robotos_fw_fsm.{h,c}             (LOCKED-AT-12D names; Phase 12E body)
│   └── robotos_fw_event_bridge.{h,c}    (LOCKED-AT-12F names + signatures)
├── tests/
│   └── host/                             (additive host test target lands here)
├── app/                                  (RESERVED AT PLANNING DEPTH;
│   │                                      NOT CREATED at Phase 12H-pre)
│   └── probe_translator/                 (FIRST CANDIDATE;
│                                          NOT CREATED at Phase 12H-pre)
│       ├── probe_translator.c           (future application source)
│       ├── probe_translator.h           (future application header,
│       │                                 if needed)
│       ├── CMakeLists.txt               (future leaf build entry;
│       │                                 Option B; not preferred at
│       │                                 first implementation)
│       └── README.md                    (future application docs)
├── src/                                  (Architecture B — frozen)
├── include/robotos/                      (Architecture B — frozen)
└── CMakeLists.txt                        (Architecture B — NOT USED by app/)
```

Notes:

- `app/` and `app/probe_translator/` are intentionally **not
  created** by Phase 12H-pre. Only the path is reserved at planning
  depth.
- `app/probe_translator/` is a **sibling** of `framework/`,
  `devkit/`, `core/`, `platform/` — never a child of any of them.
- The first implementation phase decides whether to keep a single
  `.c` file or split mapping / FSM composition into multiple files.
  Phase 12H-pre records only the planning-level intent.

---

## 4. Candidate Purpose

The `probe_translator` candidate is a **host-first, product-neutral,
synthetic-event translation harness**. Its only job is to be the
first application-layer consumer of:

1. `framework/robotos_fw_fsm.{h,c}` (Phase 12E body; LOCKED-AT-12D
   header) — owns one `robotos_fw_fsm_t` instance with its own
   transition table.
2. `framework/robotos_fw_event_bridge.{h,c}` (Phase 12F body; §5
   LOCKED-AT-12F) — owns one `robotos_fw_event_bridge_t` instance
   and one static const mapping table.

The harness proves:

- application-level event mapping works end-to-end (synthetic
  Adapter key → bridge → FSM transition);
- application-local event ID namespace works without coordination
  with `core/`;
- FIFO mapping order works against wildcard arg0 rows;
- bridge counters (`mapped_count`, `unmapped_count`) behave as
  documented in Phase 12F §5.3;
- `robotos_fw_event_bridge_reset` policy (only clears bridge
  counters; does not touch the FSM) holds in practice;
- re-init is idempotent for an application-owned bridge instance.

The harness is **not** a product and never grows business logic. If
real-product behavior is wanted later, it lands as a separate
sibling under `app/`, not by extending `probe_translator/`.

---

## 5. Minimal State Vocabulary

All states live in the `app/probe_translator/` namespace only. They
are application-local and do **not** appear under `framework/`.

| State | Meaning | Why needed | Non-goals |
|---|---|---|---|
| `APP_IDLE` | Default state after FSM init / `APP_EVT_RESET`. | Provides a known starting state; gives `APP_EVT_RESET` a target. | Not equivalent to devkit `IDLE`; not a power state. |
| `APP_READY` | Translator has been configured. | Provides a first observable transition target. | Not equivalent to devkit `ARMED`. |
| `APP_ACTIVE` | Translator has started after configuration. | Lets host tests exercise multi-step paths. | Not equivalent to devkit `ACTIVE`. |
| `APP_FAULT` (optional) | Fault sink. | Provides a negative-path target if needed. | Not a real product fault; no actuator implication. |

The first implementation phase may **drop `APP_FAULT`** if the
host-test plan does not require a fault sink. Phase 12H or later
locks the final list.

State name discipline:

1. The `APP_` prefix marks these as application-local.
2. Name overlap with `devkit_app_state` (`IDLE`, `ACTIVE`) is
   coincidental at the human-readable level only; the constants live
   in different namespaces, and the runtime instances are unrelated.
3. `devkit_app_state` is **not modified** and **not consumed** by
   `probe_translator`.

---

## 6. Minimal Event Vocabulary

All event IDs live in the application namespace only.

| Event ID | Purpose | Source class at first | Maps from |
|---|---|---|---|
| `APP_EVT_CONFIGURED` | Drive `APP_IDLE → APP_READY`. | Synthetic only. | Synthetic Adapter key `ADAPTER_EVT_CONFIG, arg0=0`. |
| `APP_EVT_START` | Drive `APP_READY → APP_ACTIVE`. | Synthetic only. | Synthetic Adapter key `ADAPTER_EVT_COMMAND, arg0=START`. |
| `APP_EVT_STOP` | Drive `APP_ACTIVE → APP_READY`. | Synthetic only. | Synthetic Adapter key `ADAPTER_EVT_COMMAND, arg0=STOP`. |
| `APP_EVT_FAULT` (optional) | Drive any state → `APP_FAULT`. Wildcard arg0 test. | Synthetic only. | Synthetic Adapter key `ADAPTER_EVT_FAULT, wildcard arg0`. |
| `APP_EVT_RESET` | Drive any state → `APP_IDLE`. | Synthetic only. | Synthetic Adapter key `ADAPTER_EVT_CONFIG, arg0=RESET`. |

Event vocabulary discipline:

1. No event maps from a real core event (no `ROBOTOS_EVENT_USER+N`).
2. No event maps from UART, button, or any hardware producer.
3. No new `ROBOTOS_EVENT_USER` subrange is required in `core/`.
4. Numeric values for `APP_EVT_*` are **not frozen** in Phase
   12H-pre; the first implementation phase chooses concrete
   constants.
5. Multiple `probe_translator` instances are not anticipated — the
   harness uses exactly one FSM instance and one bridge instance.

---

## 7. Initial Bridge Mapping Table

Planning-level table. **No source exists.** Numeric values and exact
order are subject to lock at Phase 12H.

| Row | `adapter_type` | `adapter_arg0` | `match_arg0` | `fw_event_id` | Expected transition |
|---|---|---|---|---|---|
| 0 | `ADAPTER_EVT_CONFIG` | `0` (`CONFIGURE`) | `true` | `APP_EVT_CONFIGURED` | `APP_IDLE → APP_READY` |
| 1 | `ADAPTER_EVT_CONFIG` | `1` (`RESET`) | `true` | `APP_EVT_RESET` | any → `APP_IDLE` |
| 2 | `ADAPTER_EVT_COMMAND` | `0` (`START`) | `true` | `APP_EVT_START` | `APP_READY → APP_ACTIVE` |
| 3 | `ADAPTER_EVT_COMMAND` | `1` (`STOP`) | `true` | `APP_EVT_STOP` | `APP_ACTIVE → APP_READY` |
| 4 | `ADAPTER_EVT_FAULT` | (any) | `false` (wildcard) | `APP_EVT_FAULT` | any → `APP_FAULT` (if present) |

Mapping table discipline:

1. **Planning-only.** No `robotos_fw_event_bridge_row_t[]` array is
   declared in any tracked source.
2. **Application-local adapter tags.** `ADAPTER_EVT_CONFIG /
   _COMMAND / _FAULT` are application-local numeric tags; they are
   not allocated in `core/` and do not collide with any
   `ROBOTOS_EVENT_*` constant.
3. **FIFO row order matters.** Per Phase 12F §5.3, the bridge scans
   rows in FIFO order and takes the first match; wildcard precedence
   is determined by row order. Specific rows should remain ahead of
   wildcard rows in practice.
4. **Single FSM / single bridge.** No fan-out (Phase 12F-pre §11 #5
   deferred). The application owns exactly one FSM instance and one
   bridge instance.
5. **No UART byte source.** Rows do not map from `a/s/r/?/x/v/L/d/T`.
6. **No real hardware source.** Rows do not map from `USER+2`
   button, `USER+3` UART byte, or any accelerometer probe key.

---

## 8. Host Mapping Test Plan

If a future Phase 12H is authorized to implement the harness, the
host test plan must include at minimum:

1. **Synthetic dispatch.** `robotos_fw_event_bridge_dispatch` is
   called with synthesized `(adapter_type, adapter_arg0,
   payload=NULL)` tuples; resulting FSM state and bridge counters
   are asserted.
2. **Row-by-row coverage.** Every row in §7 is exercised at least
   once.
3. **Wildcard precedence test.** A test confirms that a wildcard row
   placed before a specific row wins (matching Phase 12F TC12).
4. **Unmapped-event test.** An adapter event with no matching row
   increments `unmapped_count` and does not transition the FSM.
5. **Full transition path.** `APP_IDLE → APP_READY → APP_ACTIVE →
   APP_READY → APP_IDLE` is exercised in one continuous host test.
6. **Re-init idempotence.** Re-initializing the bridge is observable
   in counters but does not touch the FSM (matches Phase 12F TC17).
7. **Bridge reset policy.** `robotos_fw_event_bridge_reset` clears
   bridge counters only; FSM state is unchanged.
8. **No `devkit_app_state` dependency.** Application source and host
   test both fail a grep gate if they include `devkit_app_state.h`.
9. **No command-set dependency.** Application source and host test
   both fail a grep gate if they reference
   `a/s/r/?/x/v/L/d/T` semantics.
10. **No hardware dependency.** Application source and host test do
    not include any Zephyr header.
11. **Host regression preserved.** Full host suite still passes
    ≥22/22 after the new target is added.

The test file naming convention is
`tests/host/test_app_probe_translator_mapping.c`. The build wiring
choice is Option A (additive entry inside
`tests/host/CMakeLists.txt`).

---

## 9. Build Strategy

Planning-level direction (locks at the first implementation phase,
not at Phase 12H-pre):

1. **No build change at Phase 12H-pre.** Zero CMake / `prj.conf` /
   DTS / overlay / Zephyr config diffs.
2. **Preferred: Option A (additive host test target).** Phase 12H or
   12I adds a new `add_executable` + `add_test` block inside the
   existing `tests/host/CMakeLists.txt`, compiling the application
   source pair plus the existing `framework/robotos_fw_event_bridge.c`
   and `framework/robotos_fw_fsm.c`, against the existing host
   critical-section stub.
3. **Acceptable but not preferred: Option B (new
   `app/probe_translator/CMakeLists.txt`).** Useful once at least
   one more application exists; introduces a second build entry-point
   that is overkill for the first candidate.
4. **Not authorized: Option C (devkit integration).** Hardware
   evidence for the application target is a separate, later phase
   gated by explicit user authorization.
5. **WSL Ubuntu / gcc 13.3.0 host environment.** Same as Phase 12E /
   12F. MSYS2 MinGW64 remains broken and is not used.
6. **No `prj.conf`, board DTS, overlay, Zephyr config** change at
   any planned phase before hardware authorization.

---

## 10. `devkit_app_state` Boundary

- `devkit_app_state` is **authoritative** for the current devkit
  runtime. Phase 12H-pre does **not** touch it.
- `app/probe_translator/`, when created, **must not** include
  `devkit_app_state.h`, must not call any `devkit_*` function, and
  must not read or write `devkit_app_state` snapshots.
- The application's `APP_IDLE / APP_READY / APP_ACTIVE / APP_FAULT`
  states are application-local; name overlap with devkit's
  `DEVKIT_APP_STATE_IDLE / ARMED / ACTIVE` is human-readable only.
- **No `?` UART response change.** Devkit's `?` continues to come
  from `devkit_app_state`, unchanged.
- **No `a/s/r/?/x/v/L/d/T` semantic change.**
- Any future devkit ↔ application interaction (shadow, replacement)
  requires a separate planning phase. Phase 12H-pre does not open
  one.
- **Scope-guard #11 remains active.**

---

## 11. Command Set Boundary

- The current command set `a / s / r / ? / x / v / L / d / T` remains
  the **devkit / probe surface**.
- `app/probe_translator/` **does not** define a UART command. It has
  no UART surface in the first host implementation phase.
- The Framework bridge has no UART surface (Phase 12F locked).
- A future product command vocabulary, if ever added, requires a
  separate product-command planning phase. Such a phase must:
  - list every byte / message it introduces;
  - declare which channel it uses (separate UART, network, etc.);
  - state explicitly that `a/s/r/?/x/v/L/d/T` semantics are
    unchanged;
  - provide its own validation strategy.
- All 12 UART TX scope-guard constraints from Phase 9EZ §H preserved.

---

## 12. Legacy Architecture B Boundary

- `RobotOS_v1.0/src/` and `RobotOS_v1.0/include/robotos/` remain
  frozen at `LEGACY_SCAFFOLD_MARKED_FROZEN_DOCS_ONLY` (Phase
  12D-pre).
- `app/probe_translator/`, when created, **must not** reuse
  `src/app/`, `include/robotos/app*`, or any other Architecture B
  artifact.
- `app/probe_translator/` uses **Architecture A contracts only**:
  `robotos_fw_*` (from `framework/`) and, if needed,
  `robotos_core_status_t` (from `core/`).
- `app/probe_translator/` **must not include** any `ro_*` HAL
  header.
- **No Architecture A ↔ Architecture B reconciliation in Phase
  12H-pre.**

---

## 13. Open Decisions

Items intentionally left open at Phase 12H-pre:

| # | Open question | Latest at |
|---|---|---|
| 1 | Concrete numeric values for `APP_EVT_*` constants | Phase 12H |
| 2 | Whether `APP_FAULT` ships in the first implementation or is deferred | Phase 12H |
| 3 | Whether to split `probe_translator.c` and `probe_translator_mapping.c` into two TUs or keep a single TU | Phase 12H |
| 4 | Final names for `ADAPTER_EVT_CONFIG / _COMMAND / _FAULT` and arg0 enum constants | Phase 12H |
| 5 | Whether to introduce an application-local header (`probe_translator.h`) or keep static-only internal symbols | Phase 12H |
| 6 | Whether Phase 12H is docs-only (Variant 1) or implementation (Variant 2) | User decision before Phase 12H opens |
| 7 | Whether `RobotOS_v1.0/examples/` will host a future demo as a sibling tree | Future docs-only phase |
| 8 | When `app/probe_translator/` should ever run on hardware, and through which firmware target | Future runtime-integration phase (not Phase 12H, not Phase 12I) |
| 9 | Bridge ABI memory-layout lock (still open from Phase 12G §11 #9) | First or later application implementation phase |

---

## 14. Next Revision Conditions

This spec is revised when:

1. **Phase 12H opens (docs-only skeleton planning — Variant 1).**
   §3 gains the frozen file list. §5 / §6 / §7 upgrade from
   "planning-level" to "frozen-at-12H planning". §13 #1–#5 are
   resolved or explicitly carried forward.
2. **Phase 12H opens (host prototype — Variant 2).** §1 status
   becomes `IMPLEMENTED_AT_12H (HOST-TEST EVIDENCE)`. §7 mapping
   table is mirrored by a static const array in the new application
   source. §8 host mapping test plan is mirrored by
   `tests/host/test_app_probe_translator_mapping.c`. §13 #1–#5 are
   resolved. Application boundary spec
   (`FRAMEWORK_APPLICATION_BOUNDARY_DRAFT.md`) gains a §3 reference
   to the materialized file list.
3. **A second application is added under `app/`.** §13 #7 and #8
   trigger updates; multi-application coordination rules are added.
4. **A future phase reconciles or removes Architecture B.** §12 is
   updated to reflect the resolution.
5. **A user override picks `PHASE_12H_PRE_RECOMMEND_HOLD`.** §1 /
   §2 status updates record the override; no further revision is
   required until a future first candidate is selected.

This spec is **NOT** revised to:

- Record devkit implementation details (devkit docs are authoritative
  for devkit; this spec is authoritative for the first application
  candidate only).
- Track per-phase test results (closeouts are authoritative).
- Pick a product's specific business logic.
- Change command semantics (frozen at 11Z).
- Promote `devkit_app_state` (scope-guard #11 enforced).
- Reactivate Architecture B (frozen at 12D-pre).
