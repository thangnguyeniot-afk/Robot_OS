# RobotOS Framework — Devkit Integration Mode Draft Spec

**Status:** `DRAFT / NON-FINAL`. No integration implementation exists.
This document describes the **planning-level direction** for how the
Framework path (FSM + Application Event Bridge) connects, if at all,
to the devkit firmware. Function paths, directory layouts, and
runtime wiring are all subject to change until an implementation
phase locks them.
**Revision:** Phase 12G-pre (2026-05-13, `CLOSED_DOCS_ONLY`) —
initial draft created alongside the Phase 12G-pre integration-mode
decision closeout.
**Next revision condition:** Phase 12G (`CLOSED_DOCS_ONLY` at
2026-05-13; `PHASE_12G_RECOMMEND_APP_PRODUCT_PATH`) selected the
SEPARATE APPLICATION MODE direction; the application boundary
spec lives at
[`FRAMEWORK_APPLICATION_BOUNDARY_DRAFT.md`](FRAMEWORK_APPLICATION_BOUNDARY_DRAFT.md).
Further revision happens at the next implementation phase when a
concrete application source / build wiring is created.

> **No integration implementation exists.** No `devkit/src/*.c` file
> currently `#include`s `robotos_fw_event_bridge.h` or
> `robotos_fw_fsm.h`. No `app/` or `application/` directory exists.
> This spec describes the *intended layering*, not a deployed
> binding.

This spec is anchored to
[`../02_PHASE_CLOSEOUTS/PHASE_12G_PRE_DEVKIT_INTEGRATION_MODE_DECISION.md`](../02_PHASE_CLOSEOUTS/PHASE_12G_PRE_DEVKIT_INTEGRATION_MODE_DECISION.md)
and references the bridge spec
[`FRAMEWORK_APPLICATION_BRIDGE_DRAFT.md`](FRAMEWORK_APPLICATION_BRIDGE_DRAFT.md)
and the FSM spec
[`FRAMEWORK_FSM_API_DRAFT.md`](FRAMEWORK_FSM_API_DRAFT.md).

---

## 1. Status / Scope

**What this doc is:**
A long-lived planning-level spec describing the candidate ways the
Framework path can connect to the devkit firmware, the recommended
mode at Phase 12G-pre, and the boundaries that any future integration
must preserve.

**What this doc is not:**
- An implementation plan. No file path, function signature, or build
  rule listed here is final.
- A `devkit_app_state` replacement design. `devkit_app_state` remains
  authoritative; this spec describes *how the Framework path stays
  separate*, not how it absorbs the devkit state machine.
- An Application / product layer spec. The Application layer is
  `NOT_STARTED` and is a separate planning question.
- A command-vocabulary spec. The frozen `a/s/r/?/x/v/L/d/T` devkit
  command set is unchanged; future application command vocabulary
  lives outside this doc.

**Current decision state (Phase 12G-pre):**

| Decision | Status |
|---|---|
| Integration mode selected for next gate | `PHASE_12G_PRE_RECOMMEND_SEPARATE_APPLICATION_MODE_PLANNING` |
| Devkit runtime authority | `devkit_app_state` (unchanged) |
| Framework runtime consumer | NONE (Framework is host-tested only) |
| Future application path | Proposal anchor: `RobotOS_v1.0/app/<product>/` (locks at Phase 12G when authorized) |
| Shadow mode | DEFER (fallback direction if Mode 4 is rejected) |
| Replacement mode | NOT RECOMMENDED; structurally forbidden until ACTIVE disarm + Application planning resolved |
| Hold mode | Acceptable fallback if user wants to stop |
| Additional host-only work | Lower priority than the integration-mode question |

---

## 2. Current Runtime Authority

Today, the devkit firmware runtime is owned by **`devkit_app_state`**
(`RobotOS_v1.0/devkit/src/devkit_app_state.{c,h}`, Phase 9C). Its
contract:

- States: `DEVKIT_APP_STATE_IDLE / ARMED / ACTIVE`.
- Inputs:
  - button event (USER+2; consumed by `devkit_button_handler`); cycles
    `IDLE -> ARMED -> ACTIVE -> IDLE`.
  - UART byte event (USER+3; consumed by `devkit_uart_handler`);
    drives the `a/s/r/d/t/T/?` byte handlers.
- Outputs:
  - `?` UART response (the `ROBOTOS_APP state=NAME …` log line);
  - `ROBOTOS_APP` periodic snapshot log;
  - counters: `transitions`, `button_count`, `uart_count`,
    `ignored_count`, `last_src`, `last_byte`.
- Threading: single-threaded thread-context-only; no locks.
- Validation evidence: Phase 9A-C / Phase 10A-D / Phase 11D-E
  hardware probes are the canonical evidence baseline.

**`devkit_app_state` is the devkit validation harness.** It exists to
prove that `(button → core → app)` and `(UART → core → app)`
round-trip correctly. Its `IDLE/ARMED/ACTIVE` naming is a *probe
vocabulary*, not a product vocabulary.

The Framework FSM is **structurally similar** (also a flat,
table-driven, thread-context-only state machine) but lives at a
different architectural layer: it is product-neutral and has no
inherent vocabulary. The Framework Application Event Bridge
translates adapter event keys to Framework logical events.

The two are **not interchangeable** by design. `devkit_app_state` is
the devkit's validation harness; the Framework path is the substrate
for a future application / product layer that has not yet been
planned.

---

## 3. Candidate Integration Modes

Detailed evaluation lives in
[`../02_PHASE_CLOSEOUTS/PHASE_12G_PRE_DEVKIT_INTEGRATION_MODE_DECISION.md`](../02_PHASE_CLOSEOUTS/PHASE_12G_PRE_DEVKIT_INTEGRATION_MODE_DECISION.md)
§D. Summary:

| # | Mode | One-line summary | Recommended at 12G-pre |
|---|---|---|---|
| 1 | HOLD | Shelf Framework; no consumer added. | Acceptable fallback |
| 2 | SHADOW | Framework FSM observes devkit events in parallel; `devkit_app_state` authoritative. | DEFER |
| 3 | REPLACEMENT | Framework FSM replaces `devkit_app_state`. | NOT RECOMMENDED (structurally forbidden until ACTIVE disarm + Application planning resolved) |
| 4 | SEPARATE APPLICATION | Framework consumed by a future separate application / product harness; devkit untouched. | **RECOMMENDED** |
| 5 | HOST-ONLY EXTENSION | Stay on host; extend bridge / FSM behavior. | Lower priority |

---

## 4. Recommended Mode

**Mode 4 — SEPARATE APPLICATION MODE** is the recommended next gate
direction, captured as the Phase 12G-pre decision
`PHASE_12G_PRE_RECOMMEND_SEPARATE_APPLICATION_MODE_PLANNING`.

Intended layering (planning-level only; locks at Phase 12G when
authorized):

```text
[ Application / product layer       ]  RobotOS_v1.0/app/<product>/
   (NOT_STARTED; future)               - defines mapping table
                                       - owns FSM instance
                                       - owns bridge instance
                                       - chooses product vocabulary

[ Framework Application Bridge      ]  RobotOS_v1.0/framework/
   (Phase 12F; LOCKED-AT-12F)          robotos_fw_event_bridge.{h,c}

[ Robot Framework FSM               ]  RobotOS_v1.0/framework/
   (Phase 12D/12E; LOCKED-AT-12D)      robotos_fw_fsm.{h,c}

[ Robot Adapter / runtime substrate ]  RobotOS_v1.0/core/ + platform/
                                       + devkit/ (devkit_app_state
                                       lives HERE as the devkit
                                       validation harness; SEPARATE
                                       from the future application
                                       layer)

[ Kernel / HW                       ]  Zephyr + STM32F411E-DISCO
```

**Key property:** the future application layer is a **sibling** of
`devkit/`, not a child. The devkit firmware and the application
firmware can be separate build targets, separate `west` builds, or a
shared base with `CONFIG_*`-selectable layers — the exact split is a
Phase 12G decision.

---

## 5. `devkit_app_state` Boundary

The following hold under Mode 4 and are **invariants** for every
mode short of full Replacement (Mode 3), which is structurally
forbidden at this gate:

1. `devkit_app_state` is **authoritative** for devkit runtime state.
2. `devkit_app_state` is **devkit-local**. It is not moved, copied,
   promoted, renamed, or duplicated.
3. `devkit_app_state` is **not consumed** by `framework/*` code
   (and `framework/*` does not include `devkit_app_state.h`).
4. The future application layer (if it implements a similar state
   surface) does so using its own Framework FSM instance — not by
   reading or writing `devkit_app_state`.
5. **Scope-guard #11** remains active.

A future Replacement mode (Mode 3) phase, if ever opened, must list:

- which Phase 9 / 10 / 11 evidence is invalidated;
- a rollback plan to re-instate `devkit_app_state`;
- which hardware probes must rerun;
- which of `a/s/r/?/x/v/L/d/T` change semantics;
- the resolution of ACTIVE disarm `USER_DECISION_REQUIRED` and
  POST_FLASH_AUTOSTART `OPEN`.

Replacement mode is **not** opened by Phase 12G-pre.

---

## 6. Command Set Boundary

1. `a / s / r / ? / x / v / L / d / T` remain **unchanged**. They are
   devkit / probe surface, validated through Phase 11Z.
2. **No Framework integration mode automatically exposes a new UART
   command.**
3. The future application layer, if it grows a command vocabulary,
   defines that vocabulary at its own layer — not in `devkit/src/`,
   not by reusing the devkit byte stream, and not by overriding the
   `?` response shape.
4. All 12 UART TX scope-guard constraints from Phase 9EZ §H remain
   preserved.
5. The Framework bridge does not call any UART TX function (Phase 12F
   contract).

---

## 7. Application / Product Boundary

Planning-level intent (not implementation):

1. The future application / product layer lives at a **separate
   directory**, proposed at `RobotOS_v1.0/app/<product>/`. The exact
   shape is a Phase 12G decision.
2. The application layer **owns**:
   - the mapping table (`robotos_fw_event_bridge_row_t[]`);
   - its own `robotos_fw_fsm_t` instance (caller-owned, statically
     allocated);
   - its own `robotos_fw_event_bridge_t` instance;
   - its own product vocabulary (state names, event names, command
     vocabulary if any);
   - its own validation strategy (host test first; hardware later
     under a separate firmware target).
3. The application layer **does not**:
   - include `devkit_app_state.h`;
   - read or write `devkit_app_state` snapshots;
   - depend on devkit producers (`devkit_button_producer`,
     `devkit_uart_producer`) by name;
   - share its firmware target with the devkit validation build.
4. The bridge module (`framework/robotos_fw_event_bridge.{c,h}`)
   stays **product-neutral** and is shared by all future application
   patterns.
5. The FSM module (`framework/robotos_fw_fsm.{c,h}`) stays
   **product-neutral** and is shared by all future application
   patterns.

---

## 8. Future Evidence Requirements

| Integration mode | Required evidence before opening |
|---|---|
| HOLD | None. Existing Phase 12F evidence (103/103, 22/22) stands. |
| SHADOW | A docs-only spec naming exact shadow event coverage + shadow log discipline + read-only `devkit_app_state` consumer surface; a host test that the shadow mapping produces the same state sequence as `devkit_app_state` for a canonical workload; a planned hardware evidence phase distinct from devkit probe phases. |
| REPLACEMENT | All of: resolved ACTIVE disarm decision; opened Application / product planning; dedicated migration phase with rollback plan; identification of every command-semantic change in `a/s/r/?/x/v/L/d/T`; identification of every Phase 9 / 10 / 11 evidence run that becomes obsolete. |
| SEPARATE APPLICATION | Phase 12G — Separate Application Mode / Application Boundary Planning closeout (docs-only). Selects directory shape, build separation, event-mapping policy at application layer, validation strategy. |
| HOST-ONLY EXTENSION | A concrete consumer-driven gap. Without one, this mode is speculative. |

---

## 9. Open Decisions

Items intentionally left open at Phase 12G-pre:

| # | Open question | Latest at |
|---|---|---|
| 1 | Final application directory shape (`RobotOS_v1.0/app/<product>/` vs. alternative) | Phase 12G |
| 2 | Build separation strategy (separate Zephyr app dir vs. shared base + selectable `CONFIG_*` switches vs. board overlay) | Phase 12G |
| 3 | Whether the first application is a real product or a demonstration application (e.g. `app/demo/`) | Phase 12G or 13-pre |
| 4 | Whether any UART command vocabulary belongs to the future application or to a future shell layer above it | Phase 12G or 13 |
| 5 | Whether the Framework FSM / bridge ABI memory layout should be locked before the first application implementation, or after the first application proves the shape | Phase 12G or later |
| 6 | If the user overrides Mode 4 and chooses SHADOW: exact shadow log line schema and shadow event coverage | Future Phase 12G — Shadow Spec |
| 7 | If the user overrides to REPLACEMENT: full migration plan, rollback plan, evidence retraversal list | Future REPLACEMENT migration phase (not reachable until preconditions cleared) |

---

## 10. Next Revision Conditions

This spec is revised when:

1. **Phase 12G opens (Separate Application Mode Planning).** §3 marks
   Mode 4 as `PLANNED_AT_12G`. §4 gains the locked application
   directory shape and build separation strategy. §7 gains the locked
   event-mapping policy and validation strategy. §9 §1–§5 are
   resolved or explicitly deferred.
2. **A user override selects Mode 1 (HOLD).** §3 marks Mode 4 as
   `NOT_OPENED_USER_HOLD`. No further structural change.
3. **A user override selects Mode 2 (SHADOW).** A new section
   describing the shadow constraints is added; §3 marks Mode 2 as
   `PLANNED_AT_12G_SHADOW`. §9 #6 becomes the lead question for a
   Phase 12G — Devkit Shadow Integration Spec.
4. **A user override selects Mode 5 (HOST-ONLY EXTENSION).** §3 marks
   Mode 5 as `PLANNED_AT_12G_HOST_EXTENSION`. Bridge spec gets the
   relevant §5 extension at the corresponding implementation phase.

This spec is **NOT** revised to:

- Record devkit implementation details (devkit docs are authoritative
  for devkit; this spec is authoritative for the integration boundary
  only).
- Track per-phase test results (closeouts are authoritative).
- Pick a product vocabulary (Application / product phase territory).
- Change command semantics (frozen at 11Z).
- Promote `devkit_app_state` (scope-guard #11 enforced).
