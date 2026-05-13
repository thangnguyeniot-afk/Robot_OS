# Phase 12G-pre — Devkit Integration Mode Decision (docs-only)

**Status:** `CLOSED_DOCS_ONLY`
**Decision:** `PHASE_12G_PRE_RECOMMEND_SEPARATE_APPLICATION_MODE_PLANNING`
**Date:** 2026-05-13
**Branch / baseline:** `master` at `origin/master = 524cb8b` (Phase 12F)
**Prior phase anchor:** [`PHASE_12F_APPLICATION_BRIDGE_HOST_PROTOTYPE.md`](PHASE_12F_APPLICATION_BRIDGE_HOST_PROTOTYPE.md)
**New long-lived spec:** [`../03_SPECS/FRAMEWORK_DEVKIT_INTEGRATION_MODE_DRAFT.md`](../03_SPECS/FRAMEWORK_DEVKIT_INTEGRATION_MODE_DRAFT.md)

---

## A. Executive Summary

Phase 12G-pre is a **docs-only architecture planning gate** that selects
the next direction for connecting the now-host-tested Framework
(FSM + Application Event Bridge) to the running devkit firmware.

Phase 12G-pre **does not**:

- modify any `.c` / `.h` file under `framework/`, `core/`, `platform/`,
  `devkit/src/`, `tests/`, `src/`, or `include/robotos/`;
- modify any `CMakeLists.txt`, `prj.conf`, board DTS, or Zephyr config;
- modify any existing evidence log;
- change `devkit_app_state`;
- change the frozen `a/s/r/?/x/v/L/d/T` command set or its semantics;
- run hardware;
- create an Application / product layer;
- open Phase 12G implementation.

Phase 12G-pre **does**:

- compare five candidate integration modes (HOLD, SHADOW MODE,
  REPLACEMENT MODE, SEPARATE APPLICATION MODE, ADDITIONAL HOST-ONLY
  WORK) against the current repo state;
- recommend `SEPARATE APPLICATION MODE PLANNING` (docs-only) as the
  next gate;
- freeze planning-level direction in the new long-lived spec
  `FRAMEWORK_DEVKIT_INTEGRATION_MODE_DRAFT.md`;
- re-affirm scope-guard #11 (devkit_app_state remains authoritative);
- preserve every other open gate untouched.

---

## B. Baseline Before Phase 12G-pre

| Item | Value |
|---|---|
| `origin/master` baseline | `524cb8b` `feat(framework): add host-tested event bridge prototype` |
| Phase 12F status | `CLOSED_WITH_HOST_TEST_EVIDENCE` / `PHASE_12F_APPLICATION_BRIDGE_HOST_PROTOTYPE_CLOSED` |
| Framework FSM | `framework/robotos_fw_fsm.h` (Phase 12D LOCKED-AT-12D) + `framework/robotos_fw_fsm.c` (Phase 12E); host-test-validated 93/93 |
| Framework Application Bridge | `framework/robotos_fw_event_bridge.h` + `framework/robotos_fw_event_bridge.c` (Phase 12F); host-test-validated 103/103; §5 names + signatures `LOCKED-AT-12F` |
| Full host regression | 22/22 PASS (Phase 12F) under WSL Ubuntu / gcc 13.3.0 |
| Devkit integration of Framework | `NOT_STARTED` — bridge is host-test-only, never linked into the devkit firmware |
| Application / product layer | `NOT_STARTED` — no `app/` or `application/` directory exists |
| Active architecture | A (`core/` + `platform/` + `devkit/` + `framework/`) |
| Legacy architecture | B (`src/` + `include/robotos/` + root `RobotOS_v1.0/CMakeLists.txt`) — frozen at Phase 12D-pre |
| Devkit runtime authority | `devkit_app_state` owns `IDLE/ARMED/ACTIVE`; consumes button (Phase 9A) + UART byte (Phase 9B) events; emits `?` response and `ROBOTOS_APP` log |
| Frozen command set | `a / s / r / ? / x / v / L / d / T` (9 commands; validated through Phase 11Z) |
| Scope-guard #11 | `devkit_app_state` is devkit-local; not promoted, replaced, copied, or duplicated; preserved through Phase 12F |
| Open gates carried in | ACTIVE disarm `USER_DECISION_REQUIRED_ACTIVE_DISARM`; Scheduler 7A/7B `DEFER`; F407 `HOLD/DEFER`; POST_FLASH_AUTOSTART `OPEN/MITIGATED_BY_WORKFLOW` |

---

## C. Integration Problem Statement

Phase 12F closed the last in-flight host milestone for the Framework
path. The Framework FSM and Application Event Bridge now exist, are
host-tested, and have a frozen API surface — but they have **no
runtime consumer**. The bridge has never been linked into a firmware
target. No devkit `.c` file currently `#include`s
`robotos_fw_event_bridge.h` or `robotos_fw_fsm.h`.

The next architectural boundary is therefore: **how, or whether, to
connect the Framework path to the devkit runtime.** This is a risky
boundary because a direct connection can:

1. **Create duplicate state truth.** Both `devkit_app_state` and a
   Framework `robotos_fw_fsm_t` instance could claim authority over
   `IDLE/ARMED/ACTIVE`, with no formal resolution rule.
2. **Change command semantics.** The current `a/s/r/?/x/v/L/d/T`
   command set is *validated through Phase 11Z* and is part of the
   devkit probe surface. Any Framework integration that overrides
   `devkit_app_state.on_uart_byte()` or that produces a different `?`
   response is, by definition, a command-semantic change.
3. **Blur the devkit validation harness vs. the Framework /
   Application layer.** `devkit_app_state` exists specifically as a
   devkit validation surface: it is the proof that
   `(button → core → app)` and `(UART → core → app)` round-trip
   correctly. The Framework is product-neutral and lives at a
   different layer; merging the two would create a Frankenlayer with
   no clean ownership.
4. **Force premature hardware evidence.** Once the Framework is in
   firmware, every Framework change requires a hardware run. The
   Framework's host-test discipline (Phase 12E + 12F) was designed
   specifically to avoid that.
5. **Start Application / product work by accident.** A "small"
   integration that names states `IDLE/ARMED/ACTIVE` and maps UART
   bytes to Framework events is, in effect, an Application-layer
   binding. Application / product is `NOT_STARTED`; binding it
   without an explicit Application planning gate skips that question.

**Therefore: integration mode must be selected before any
`devkit/src/` edit, and the selection must be docs-only.** Phase
12G-pre is that gate.

---

## D. Candidate Integration Modes

Five modes are evaluated. Modes 1, 2, 3, 4 come from the Phase 12F-pre
§G enumeration; Mode 5 is added because Phase 12F-pre §11 explicitly
flagged additional host-only work as a viable next gate.

### Mode 1 — HOLD

| Aspect | Value |
|---|---|
| Objective | Stop. Framework is shelved; no consumer added. |
| Future files likely touched | None. |
| Validation path | None — Framework + bridge remain on existing host evidence. |
| Benefits | Zero risk. Preserves clean shelf. No `devkit_app_state` collision. No command-semantic risk. |
| Risks | Framework shelf gradually becomes stale; ABI lock pressure (`LOCKED-AT-12D`, `LOCKED-AT-12F`) builds without a real consumer to validate it. |
| Impact on `devkit_app_state` | None. |
| Impact on command set | None. `a/s/r/?/x/v/L/d/T` unchanged. |
| Hardware required | No. |
| Application / product planning required | No. |
| Recommendation | **ACCEPTABLE if user wants to stop.** Not recommended as the default because the Framework path was deliberately advanced through Phase 12F and now exists in a state where it can answer architectural questions if used; shelving it indefinitely wastes that. |

### Mode 2 — SHADOW MODE

| Aspect | Value |
|---|---|
| Objective | Run the Framework FSM (and optionally the bridge) **in parallel** with `devkit_app_state` on the device. `devkit_app_state` remains authoritative. The Framework shadow only **observes** events and produces a non-authoritative trace. |
| Future files likely touched | new `devkit/src/devkit_fw_shadow.{c,h}`; small additive hook in `devkit_button_handler` / `devkit_uart_handler` to forward to the shadow; new RTT or log line (`ROBOTOS_FW_SHADOW …`) — strictly non-authoritative. |
| Validation path | Hardware run required: button + UART workload + shadow-trace compare against the authoritative `devkit_app_state` `?` response. Host test for shadow setup possible but does not replace the on-device run. |
| Benefits | First runtime validation of Framework FSM + bridge on actual hardware. Direct comparison with the authoritative state truth. No command-semantic change. Reversible — shadow is purely observational. |
| Risks | **Duplicate state truth labelling.** If the shadow trace is read as "Framework also says ACTIVE", operators / future readers may conflate it with authoritative state. Mitigation: explicit `SHADOW_NON_AUTHORITATIVE` markers everywhere the shadow speaks. **Producer hook risk.** Adding a forward from `devkit_button_handler` to a shadow module touches devkit producers; Phase 9A producer evidence remains the baseline. **Hardware burden.** Every shadow change requires a hardware run; the Framework's host-test discipline is partly lost. |
| Impact on `devkit_app_state` | Read-only consumer (snapshot reads). No mutation. Scope-guard #11 preserved with care. |
| Impact on command set | None. `?` response unchanged. No new UART command. |
| Hardware required | Yes — shadow comparison can only be evidenced on the device with real button + UART workload. |
| Application / product planning required | No — the shadow does not bind a product vocabulary. The Framework states / event IDs are synthetic for the shadow only. |
| Recommendation | **DEFER.** Shadow mode is useful, but is most valuable **after** the separate-application question is decided. Otherwise the shadow trace becomes the only Framework consumer and starts setting Application-layer precedent without a planning gate. Phase 12G — Shadow Spec is reachable from Phase 12G-pre as Plan B if Separate Application Mode is rejected. |

### Mode 3 — REPLACEMENT MODE

| Aspect | Value |
|---|---|
| Objective | Make a Framework FSM instance the authoritative devkit runtime state machine. `devkit_app_state` is rewritten or deleted. |
| Future files likely touched | `devkit/src/devkit_app_state.c` (rewrite or delete), `devkit/src/devkit_uart_producer.c` (response formatting), `devkit/src/devkit_button_producer.c` (handler binding), `devkit/CMakeLists.txt`, every test that reads `devkit_app_state_snapshot`. |
| Validation path | Full hardware regression — every command (`a/s/r/?/x/v/L/d/T`) re-validated; `?` response shape re-validated; ACTIVE disarm decision must be resolved first; POST_FLASH_AUTOSTART must be resolved or formally re-tested. |
| Benefits | Single state truth long-term. Removes `devkit_app_state` / Framework duplication. |
| Risks | **Directly violates scope-guard #11** as currently stated. The `?` response shape (`ROBOTOS_APP state=NAME …`) is part of devkit probe evidence; rewriting its source is a command-semantic change. ACTIVE disarm is still `USER_DECISION_REQUIRED`; replacement forces that decision *now*. Loses Phase 9–11 evidence continuity (every prior probe run was against the `devkit_app_state` implementation; replacing it invalidates that baseline). |
| Impact on `devkit_app_state` | Removed or rewritten. Scope-guard #11 violated unless the migration phase explicitly retires it. |
| Impact on command set | `a/s/r/?/x/v/L/d/T` semantics re-defined; every byte handler re-implemented inside the Framework callback set. |
| Hardware required | Yes — replacement cannot ship without full hardware regression. |
| Application / product planning required | Yes — replacing the devkit state machine implicitly chooses a product vocabulary, which is currently `NOT_STARTED`. |
| Recommendation | **NOT RECOMMENDED at Phase 12G-pre.** Reachable only after (a) the Application / product layer is planned, (b) ACTIVE disarm decision is resolved, and (c) a dedicated migration phase with rollback evidence is opened. Until then this mode is structurally forbidden. |

### Mode 4 — SEPARATE APPLICATION MODE

| Aspect | Value |
|---|---|
| Objective | Treat the Framework FSM + bridge as the substrate for a **future, separate application / product layer** (e.g. `RobotOS_v1.0/app/<product>/`). The devkit firmware keeps `devkit_app_state` as its validation harness; the new application layer is a *separate* firmware target / harness consumer. |
| Future files likely touched | `RobotOS_v1.0/app/<product>/<product>.c/h` (when authorized; not in this phase); new `app/<product>/CMakeLists.txt`; possibly a separate Zephyr application directory; new bridge / FSM instance owned by the application. |
| Validation path | Host test for the application's mapping table first; eventual hardware run on a dedicated application build, kept separate from the devkit validation build. |
| Benefits | Clean long-term boundary. Devkit validation harness stays put. Framework gets a real consumer that exercises bridge mapping + FSM transition + payload-borrowed semantics in a non-trivial way. Scope-guard #11 preserved automatically — `devkit_app_state` is not touched. ACTIVE disarm / POST_FLASH_AUTOSTART decisions can stay deferred because the application is a separate target. |
| Risks | Application planning is `NOT_STARTED`; opening Phase 12G as direct application implementation would skip that question. Mitigation: Phase 12G-pre's recommendation is *application boundary planning*, not direct application source. **Repo shape drift**: introducing `RobotOS_v1.0/app/` is a long-term architecture decision; needs a planning gate that explicitly chooses the directory shape and build wiring policy. Mitigation: that planning is exactly what Phase 12G — Separate Application Mode Planning would do (still docs-only). |
| Impact on `devkit_app_state` | None. `devkit_app_state` continues as the devkit validation harness. |
| Impact on command set | None. The new application layer would define **its own** command vocabulary if/when it grows one — the devkit command set is unchanged. |
| Hardware required | Eventually yes — but not for the planning gate. Hardware lives downstream of an application implementation phase that is at least two gates away. |
| Application / product planning required | Yes — that is exactly what Phase 12G — Separate Application Mode Planning would deliver. |
| Recommendation | **RECOMMENDED.** This is the only mode that resolves the integration question without forcing a `devkit_app_state` change, a command-semantic change, an ACTIVE disarm decision, or a hardware run. It also gives the Framework path its first non-trivial concrete consumer (the future application). The next step after Phase 12G-pre would be Phase 12G — Separate Application Mode Planning (still docs-only). |

### Mode 5 — ADDITIONAL HOST-ONLY WORK

| Aspect | Value |
|---|---|
| Objective | Stay on host. Extend the bridge / FSM with additional behavior (e.g., `arg1` matching, fan-out, unmapped-event diagnostic hook from Phase 12F-pre §11 #5 / #7). |
| Future files likely touched | `framework/robotos_fw_event_bridge.{h,c}` (extension), `tests/host/test_robotos_fw_event_bridge.c` (extension), spec update. |
| Validation path | Host test (WSL Ubuntu / gcc 13.3.0). |
| Benefits | Lowest runtime risk. Improves bridge expressiveness if a future consumer needs `arg1` or fan-out. Keeps host-test discipline intact. |
| Risks | Speculative. Phase 12F resolved every concrete bridge requirement currently on the table. Adding behavior without a concrete consumer risks shaping the bridge for hypothetical needs. |
| Impact on `devkit_app_state` | None. |
| Impact on command set | None. |
| Hardware required | No. |
| Application / product planning required | No. |
| Recommendation | **LOWER PRIORITY THAN MODE 4.** Acceptable as a parallel track, but Mode 4 (Separate Application Mode planning) is the more productive next gate because it surfaces the *consumer* shape, which is what would tell us whether `arg1` / fan-out are actually needed. Mode 5 may follow Phase 12G — Separate Application Mode Planning if that planning gate identifies concrete bridge gaps. |

### Comparison summary

| # | Mode | Touches `devkit_app_state` | Changes command set | Hardware required | App / product planning required | Recommendation at Phase 12G-pre |
|---|---|---|---|---|---|---|
| 1 | HOLD | No | No | No | No | Acceptable fallback |
| 2 | SHADOW | Read-only | No | Yes | No | Defer until after Mode 4 decision |
| 3 | REPLACEMENT | Yes (rewrite) | Yes | Yes | Yes (implicit) | Not recommended; structurally forbidden until ACTIVE disarm + app planning resolved |
| 4 | SEPARATE APPLICATION | No | No | Eventually | Yes (next gate is exactly this) | **RECOMMENDED** |
| 5 | HOST-ONLY EXTENSION | No | No | No | No | Lower priority than Mode 4 |

---

## E. Recommended Decision

**Decision result:** `PHASE_12G_PRE_RECOMMEND_SEPARATE_APPLICATION_MODE_PLANNING`.

### Rationale

- The Framework path should not replace `devkit_app_state`. Phase 12D-pre,
  Phase 12F, and every preceding scope-guard #11 re-affirmation have
  established that `devkit_app_state` is the devkit validation
  authority, not a generic state machine to be retired.
- The devkit command set is **validation / probe surface**, not
  product vocabulary. Integration modes that change `a/s/r/?/x/v/L/d/T`
  are architectural regressions, not advances.
- Application / product layer is `NOT_STARTED`. The next clean
  forward step is to plan *where the application lives* before
  building any application — and that is exactly what Separate
  Application Mode Planning would do.
- Shadow mode (Mode 2) is useful, but premature. Without an
  Application decision, the shadow trace becomes the only Framework
  consumer on the device, and the labeling discipline required to
  keep it non-authoritative is fragile (one drift in a future log
  line and the shadow starts being read as truth).
- Replacement mode (Mode 3) is structurally forbidden at this gate
  for the reasons enumerated in §D Mode 3.
- HOLD (Mode 1) is acceptable if the user wants to stop, but is
  inferior to Mode 4 as a default because Mode 4 *also* preserves the
  shelf while adding direction.
- Additional host-only work (Mode 5) is acceptable as a parallel
  track but is lower-priority than answering the integration-mode
  question; Mode 5 risks shaping the bridge for hypothetical needs
  in the absence of a concrete consumer.

---

## F. If Separate Application Mode Is Recommended (frozen planning direction)

Planning-level direction frozen by this phase (subject to refinement
by a future Phase 12G — Separate Application Mode Planning):

1. **Current `devkit_app_state` remains authoritative** for the
   devkit validation harness. It owns `IDLE/ARMED/ACTIVE`, the `?`
   UART response, the `a/s/r/d/t/T` byte handling, and the button
   cycle. Scope-guard #11 unchanged.
2. **Framework FSM + Application Event Bridge are intended to be
   consumed by a future, separate application / product harness**,
   not by the devkit validation firmware.
3. **The future application layer will live at a new path** (proposal
   anchor: `RobotOS_v1.0/app/<product>/`). The exact directory shape
   is a Phase 12G decision, not a Phase 12G-pre decision.
4. **The future application layer will own a mapping table** of
   adapter event keys → Framework event IDs, an application-owned
   FSM instance, and an application-owned bridge instance. The
   bridge module itself stays product-neutral (already frozen at
   `LOCKED-AT-12F`).
5. **No current UART command semantics change.**
   `a/s/r/?/x/v/L/d/T` remain the devkit/probe surface.
6. **No `?` response change.** The future application layer, if it
   exposes a status surface, does so through its own RTT / log /
   command stream, not through the existing devkit `?` byte.
7. **No `devkit_app_state` replacement, copy, or promotion.**
8. **No hardware run** until the future application implementation
   phase produces a build target that runs on the board.
9. **Build separation:** the future application build is expected to
   be a separate firmware target from the devkit validation build.
   The exact split (separate `west` board overlay vs. separate
   Zephyr application directory vs. shared base + selectable
   `CONFIG_*` switches) is a Phase 12G decision.

These nine items become the input to Phase 12G — Separate Application
Mode Planning. They are not implementation commitments.

---

## G. If Shadow Mode Is Recommended (fallback direction; not selected)

If a future user decision overrides §F and selects Shadow Mode as the
next gate, the planning constraints would be:

1. The shadow Framework FSM must be **explicitly labelled
   non-authoritative** at every output surface. Suggested log prefix:
   `ROBOTOS_FW_SHADOW …`. Suggested explicit `_NON_AUTHORITATIVE`
   token in every shadow line that mentions a state name.
2. **No command output change.** The `?` UART response continues to
   come from `devkit_app_state` only. The shadow never emits to the
   devkit's UART TX path.
3. **No `devkit_app_state` replacement.** The shadow reads
   `devkit_app_state_get_snapshot()` (or a future read-only equivalent)
   but never writes through that module.
4. **Evidence required before any shadow runtime run:**
   - host test that the shadow's mapping table produces the same
     state sequence as `devkit_app_state` for a canonical workload;
   - a docs-only spec naming exactly which events the shadow
     observes and which it ignores;
   - explicit decision on shadow log volume vs. existing RTT
     budget.
5. **A dedicated hardware-evidence phase is required** before the
   shadow ships. The shadow does not bypass Phase 11D / 11E hardware
   discipline.
6. Scope-guard #11 remains active: the shadow may not be promoted to
   "authoritative" without a dedicated REPLACEMENT MODE migration
   phase, which is gated by §D Mode 3's preconditions.

These items are not in scope for Phase 12G-pre. They are recorded
here only as the fallback direction if the user overrides §F.

---

## H. Relationship to `devkit_app_state`

- `devkit_app_state` remains **authoritative** for devkit runtime
  state and command-byte handling. It is the canonical implementation
  of `IDLE/ARMED/ACTIVE`, the `?` response, the `a/s/r/d/t/T` byte
  handlers, the button cycle, and the `ROBOTOS_APP` periodic log
  line.
- Phase 12G-pre **does not modify it** and does not propose to modify
  it. No read path, no write path, no header change.
- **Any future integration mode that touches `devkit_app_state` must
  list, before opening:**
  - migration risk (which Phase 9 / 10 / 11 evidence is invalidated?);
  - rollback plan (how is the prior `devkit_app_state` re-instated
    if the migration fails?);
  - evidence requirements (which hardware probes must rerun?);
  - command impact (which of `a/s/r/?/x/v/L/d/T` change semantics?);
  - relationship to ACTIVE disarm `USER_DECISION_REQUIRED` and
    POST_FLASH_AUTOSTART `OPEN`.
- **Scope-guard #11 remains active** through Phase 12G-pre.
  `devkit_app_state` is devkit-local; not promoted, copied, replaced,
  or duplicated.

---

## I. Relationship to Command Set

- **`a / s / r / ? / x / v / L / d / T` remain unchanged.**
- No Framework integration mode evaluated in §D automatically
  exposes new UART commands.
- The frozen 9-command set is **devkit / probe surface**, validated
  through Phase 11Z. It is not the product / application command
  vocabulary.
- A future application layer (Phase 12G or later) will define **its
  own** command vocabulary if it grows one. That vocabulary lives in
  the application path (`app/<product>/` proposed), not in
  `devkit/src/`.
- All 12 UART TX scope-guard constraints from
  [`PHASE_9EZ_CHECKPOINT.md`](PHASE_9EZ_CHECKPOINT.md) §H remain
  preserved.

---

## J. Non-goals (Phase 12G-pre)

| Non-goal | Status |
|---|---|
| Integration implementation | NOT STARTED |
| Framework code change | NOT STARTED (`framework/` zero-diff) |
| Devkit runtime change | NOT STARTED (`devkit/src/` zero-diff) |
| `devkit_app_state` change | NOT STARTED |
| UART command added | NOT STARTED |
| Command semantic change | NOT STARTED |
| Hardware run | NOT STARTED |
| Application / product source | NOT STARTED (no `app/` directory) |
| Scheduler 7A/7B reopened | NO — DEFER preserved |
| F407 / custom board reopened | NO — HOLD/DEFER preserved |
| ACTIVE disarm widening started | NO — `USER_DECISION_REQUIRED_ACTIVE_DISARM` preserved |
| POST_FLASH_AUTOSTART change | NO — `OPEN/MITIGATED_BY_WORKFLOW` preserved |
| Legacy Architecture B modification | NO — frozen |
| CMake / test change | NO |
| Evidence log change | NO |
| Phase 12G opened | NO — Phase 12G remains `NOT_STARTED` and requires explicit user authorization |

---

## K. Decision Result

**`PHASE_12G_PRE_RECOMMEND_SEPARATE_APPLICATION_MODE_PLANNING`**

The recommended next gate is **Phase 12G — Separate Application Mode
Planning** (docs-only architecture planning). All other modes are
explicitly deferred or not recommended at this gate; their fallback
direction is recorded in §G (Shadow), §D Mode 3 (Replacement), §D
Mode 1 (Hold), and §D Mode 5 (Host-only extension) so the trail is
preserved if the user overrides this recommendation.

---

## L. Next Gate Recommendation

**Phase 12G — Separate Application Mode / Application Boundary
Planning**

| Field | Value |
|---|---|
| Title | Phase 12G — Separate Application Mode / Application Boundary Planning |
| Classification | Docs-only architecture planning |
| Purpose | Define where a future application harness lives, how it consumes the Framework Application Event Bridge + FSM, and how it remains separate from the devkit validation harness. |
| In-scope | Choice of application directory shape (e.g. `RobotOS_v1.0/app/<product>/`); build separation strategy (separate Zephyr app dir vs. shared base + `CONFIG_*` selection vs. overlay); event-mapping policy between adapter event keys and Framework event IDs at the application layer; relationship to `devkit_app_state` (explicit non-overlap); validation strategy (host test for the application's mapping table; eventual hardware run on a separate firmware target). |
| Non-goals | No source files. No devkit/src/ change. No `devkit_app_state` change. No command-set change. No CMake / `prj.conf` / board change. No hardware run. No actual application implementation. No Framework code change. |
| Exit criteria | A new `PHASE_12G_*.md` closeout selects the application boundary, names the future file path, defines the event mapping policy, and sets the validation strategy. The bridge spec gains a §13 referring to the application boundary; the integration-mode spec gains a §11 marking the chosen mode as `PLANNED_AT_12G`. |
| Authorization | Must be opened only on **explicit user approval**. Phase 12G-pre's recommendation is not authorization. |

If the user prefers any of the alternative modes, the corresponding
fallback gate is:

- **Mode 1 (HOLD):** no next phase; shelf is stable.
- **Mode 2 (SHADOW):** Phase 12G — Devkit Shadow Integration Spec
  (docs-only implementation spec; see §G for constraints).
- **Mode 3 (REPLACEMENT):** structurally forbidden at this gate;
  requires prior resolution of ACTIVE disarm + Application planning +
  dedicated migration phase.
- **Mode 5 (HOST-ONLY EXTENSION):** Phase 12G — Bridge / FSM Host
  Extension Planning (low priority; reachable only with a concrete
  consumer-driven gap).

---

## M. Cross-references

- Phase 12F closeout:
  [`PHASE_12F_APPLICATION_BRIDGE_HOST_PROTOTYPE.md`](PHASE_12F_APPLICATION_BRIDGE_HOST_PROTOTYPE.md)
- Phase 12F-pre planning:
  [`PHASE_12F_PRE_APPLICATION_BRIDGE_PLANNING.md`](PHASE_12F_PRE_APPLICATION_BRIDGE_PLANNING.md)
- Phase 12E implementation closeout:
  [`PHASE_12E_FSM_HOST_TEST_IMPLEMENTATION.md`](PHASE_12E_FSM_HOST_TEST_IMPLEMENTATION.md)
- Phase 12C event bridge + status confirmation:
  [`PHASE_12C_FSM_EVENT_BRIDGE_STATUS_MODEL.md`](PHASE_12C_FSM_EVENT_BRIDGE_STATUS_MODEL.md)
- Phase 12D-pre legacy disposition:
  [`PHASE_12D_PRE_LEGACY_FRAMEWORK_SCAFFOLD_DISPOSITION.md`](PHASE_12D_PRE_LEGACY_FRAMEWORK_SCAFFOLD_DISPOSITION.md)
- Phase 11Z command set checkpoint:
  [`PHASE_11Z_COMMAND_SET_CHECKPOINT.md`](PHASE_11Z_COMMAND_SET_CHECKPOINT.md)
- Phase 9EZ UART TX scope-guard:
  [`PHASE_9EZ_CHECKPOINT.md`](PHASE_9EZ_CHECKPOINT.md)
- Long-lived bridge spec:
  [`../03_SPECS/FRAMEWORK_APPLICATION_BRIDGE_DRAFT.md`](../03_SPECS/FRAMEWORK_APPLICATION_BRIDGE_DRAFT.md)
- Long-lived FSM spec:
  [`../03_SPECS/FRAMEWORK_FSM_API_DRAFT.md`](../03_SPECS/FRAMEWORK_FSM_API_DRAFT.md)
- New long-lived integration-mode spec:
  [`../03_SPECS/FRAMEWORK_DEVKIT_INTEGRATION_MODE_DRAFT.md`](../03_SPECS/FRAMEWORK_DEVKIT_INTEGRATION_MODE_DRAFT.md)
- Live state: [`../../../../CURRENT_STATE.md`](../../../../CURRENT_STATE.md)
