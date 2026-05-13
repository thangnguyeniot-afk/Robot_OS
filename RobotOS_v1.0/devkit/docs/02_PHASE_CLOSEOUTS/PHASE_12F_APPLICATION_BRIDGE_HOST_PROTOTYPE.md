# Phase 12F — Framework Application Bridge Host Prototype

**Status:** `CLOSED_WITH_HOST_TEST_EVIDENCE`
**Decision:** `PHASE_12F_APPLICATION_BRIDGE_HOST_PROTOTYPE_CLOSED`
**Date:** 2026-05-13
**Branch / commit base:** `master` at `origin/master = 023987a` (Phase 12F-pre)
**Phase-pre anchor:** [`PHASE_12F_PRE_APPLICATION_BRIDGE_PLANNING.md`](PHASE_12F_PRE_APPLICATION_BRIDGE_PLANNING.md)
**Spec anchor:** [`../03_SPECS/FRAMEWORK_APPLICATION_BRIDGE_DRAFT.md`](../03_SPECS/FRAMEWORK_APPLICATION_BRIDGE_DRAFT.md)

---

## A. Executive Summary

Phase 12F implemented the Framework Application Event Bridge as a
host-only prototype, exactly along the path Phase 12F-pre recommended
(`PHASE_12F_PRE_RECOMMEND_HOST_BRIDGE_PROTOTYPE`).

- **Created:** product-neutral bridge module at
  `RobotOS_v1.0/framework/robotos_fw_event_bridge.{h,c}` and host
  contract test `RobotOS_v1.0/tests/host/test_robotos_fw_event_bridge.c`.
- **Host evidence:** 17 cases / 103 assertions PASS for the bridge
  contract under WSL Ubuntu / gcc 13.3.0. Full host regression 22/22
  PASS (was 21/21 at Phase 12E; Phase 12F adds exactly one ctest
  target).
- **No devkit integration.** No UART command added. No hardware run.
  No change to `devkit_app_state`. No change to the frozen
  `a/s/r/?/x/v/L/d/T` command set. No change to legacy Architecture B.
- **§5 of the bridge spec is now `LOCKED-AT-12F`** for names and
  signatures. ABI memory layout is still `DRAFT / EXPERIMENTAL`.
- **Architecture A only:** the bridge module includes
  `"robotos_fw_fsm.h"` (transitively `"robotos_core.h"`); no Zephyr,
  no devkit, no legacy `ro_*` symbol or include.

---

## B. Baseline Before Phase 12F

| Item | Value |
|---|---|
| `origin/master` baseline | `023987a` docs: add Phase 12F-pre application bridge planning |
| Phase 12F-pre decision | `PHASE_12F_PRE_RECOMMEND_HOST_BRIDGE_PROTOTYPE` |
| Framework FSM status (Phase 12E) | `framework/robotos_fw_fsm.h` (Phase 12D LOCKED) + `framework/robotos_fw_fsm.c` (Phase 12E); host-test-validated 93/93; full regression 21/21 |
| Bridge module status before Phase 12F | spec only; no `.h` / `.c` / test |
| Active architecture | A (`core/` + `platform/` + `devkit/` + `framework/`) |
| Legacy architecture | B (`src/` + `include/robotos/` + root `RobotOS_v1.0/CMakeLists.txt`) — frozen at Phase 12D-pre |
| Command set | `a / s / r / ? / x / v / L / d / T` (9 commands; unchanged) |
| Open gates (preserved into Phase 12F) | ACTIVE disarm `USER_DECISION_REQUIRED_ACTIVE_DISARM`; Scheduler 7A/7B `DEFER`; F407 `HOLD/DEFER`; POST_FLASH_AUTOSTART `OPEN/MITIGATED_BY_WORKFLOW`; Application/product `NOT_STARTED`; Devkit integration of Framework `NOT_STARTED` |

Phase 12F scope was authorized by the Phase 12F-pre planning closeout
§L (Recommended Phase 12F path) and §I (Required behavior coverage).

---

## C. Files Changed

### New (tracked, this phase)

| Path | Purpose |
|---|---|
| `RobotOS_v1.0/framework/robotos_fw_event_bridge.h` | Phase 12F bridge header (declarations + struct layouts). |
| `RobotOS_v1.0/framework/robotos_fw_event_bridge.c` | Phase 12F bridge implementation. |
| `RobotOS_v1.0/tests/host/test_robotos_fw_event_bridge.c` | Phase 12F bridge host contract test (17 cases / 103 assertions). |
| `RobotOS_v1.0/tests/host/logs/host_2026-05-13.log` | Standard daily host log (written by `save_test_log` target; full 22/22 regression). |
| `RobotOS_v1.0/tests/host/logs/phase_12F_host_2026-05-13.log` | Phase-tagged copy of the same run for closeout citation. |
| `RobotOS_v1.0/devkit/docs/02_PHASE_CLOSEOUTS/PHASE_12F_APPLICATION_BRIDGE_HOST_PROTOTYPE.md` | This closeout. |

### Modified

| Path | Change |
|---|---|
| `RobotOS_v1.0/tests/host/CMakeLists.txt` | Additive: one new `add_executable` + `add_test` block for `robotos_fw_event_bridge_contract_test`. No existing target altered. |
| `RobotOS_v1.0/devkit/docs/03_SPECS/FRAMEWORK_APPLICATION_BRIDGE_DRAFT.md` | Status now `IMPLEMENTED_AT_12F (HOST-TEST EVIDENCE)`; §5 marked `LOCKED-AT-12F`; §5.3 added (implementation behavior); §10 host prototype row marked `IMPLEMENTED_AT_12F`; §11 open decisions resolved or deferred per item; §12 revision history updated. |
| `RobotOS_v1.0/devkit/docs/01_PROGRESS/DEVKIT_PROGRESS_PHASE_11_20.md` | Phase 12F index row + `<a id="phase-12f"></a>` narrative section. |
| `RobotOS_v1.0/devkit/docs/00_INDEX/README.md` | Phase 12F closeout link; spec status line updated. |
| `CURRENT_STATE.md` | Latest phase = Phase 12F; bridge `HOST_TEST_VALIDATED_PROTOTYPE`. |

### Intentionally zero-diff in this phase

- `RobotOS_v1.0/framework/robotos_fw_fsm.h` (Phase 12D LOCKED-AT-12D held).
- `RobotOS_v1.0/framework/robotos_fw_fsm.c` (Phase 12E body unchanged).
- `RobotOS_v1.0/framework/README.md`.
- `RobotOS_v1.0/core/*`, `RobotOS_v1.0/platform/*`.
- `RobotOS_v1.0/devkit/src/*`, `RobotOS_v1.0/devkit/CMakeLists.txt`.
- `RobotOS_v1.0/src/*`, `RobotOS_v1.0/include/robotos/*`, root
  `RobotOS_v1.0/CMakeLists.txt` (Architecture B frozen at 12D-pre).
- `RobotOS_v1.0/tests/CMakeLists.txt`.
- All existing host test sources and targets.
- All `prj.conf`, board DTS / overlay, Zephyr config files.

---

## D. Implementation Summary

### D.1 Data model

```c
/* Caller-owned mapping row (static const arrays expected). */
typedef struct {
    uint32_t              adapter_type;
    uint32_t              adapter_arg0;
    bool                  match_arg0;   /* false = wildcard on arg0 */
    robotos_fw_event_id_t fw_event_id;
} robotos_fw_event_bridge_row_t;

/* Caller-owned config (bridge stores by pointer; rows NOT copied). */
typedef struct {
    const robotos_fw_event_bridge_row_t *rows;
    uint32_t                             row_count;
    robotos_fw_fsm_t                    *fsm;          /* caller-owned */
    void                                *user_context; /* reserved */
} robotos_fw_event_bridge_config_t;

/* Caller-owned instance. NO payload field. */
typedef struct robotos_fw_event_bridge {
    const robotos_fw_event_bridge_config_t *config;
    uint32_t              event_count;
    uint32_t              mapped_count;
    uint32_t              unmapped_count;
    uint32_t              last_adapter_type;
    uint32_t              last_adapter_arg0;
    robotos_fw_event_id_t last_fw_event_id;
    robotos_fw_status_t   last_status;
    bool                  initialized;
} robotos_fw_event_bridge_t;
```

The snapshot struct mirrors the instance's counters/last_* fields and
omits the `config` pointer. No payload pointer is exposed anywhere.

### D.2 Public API (four functions, `LOCKED-AT-12F` for names/signatures)

- `robotos_fw_event_bridge_init(bridge, config)`
- `robotos_fw_event_bridge_dispatch(bridge, adapter_type, adapter_arg0, payload)`
- `robotos_fw_event_bridge_reset(bridge)`
- `robotos_fw_event_bridge_get_snapshot(bridge, out)`

### D.3 Mapping behavior (FIFO first-match)

```text
event_count++
last_adapter_type = adapter_type
last_adapter_arg0 = adapter_arg0
for row in config->rows[0 .. row_count):
  if row.adapter_type != adapter_type:                     continue
  if row.match_arg0 && row.adapter_arg0 != adapter_arg0:   continue
  mapped_count++
  last_fw_event_id = row.fw_event_id
  last_status      = robotos_fw_fsm_dispatch(fsm, row.fw_event_id, payload)
  return last_status
unmapped_count++
last_status = ROBOTOS_CORE_OK
return ROBOTOS_CORE_OK
```

### D.4 Wildcard behavior

`match_arg0 == false` makes a row match any `arg0` for its
`adapter_type`. Row order is the only precedence rule — the bridge
applies NO additional specificity tiebreak. Callers ordering wildcard
rows BEFORE exact-match rows for the same type will shadow the
exact-match rows. (Asserted by TC10, TC11, TC12, TC13.)

### D.5 Payload rules

- `payload` is a borrowed `const void *` passed through to
  `robotos_fw_fsm_dispatch()` unchanged.
- The bridge struct has NO payload-storage field by construction. The
  payload pointer is not stored after `dispatch()` returns.
- TC07 verifies the FSM action sees the exact same pointer the caller
  passed. TC08 structurally asserts the bridge struct's field list does
  not include a payload slot.

### D.6 Status mapping (no new enum)

| Condition | Return |
|---|---|
| Mapped dispatch, FSM action OK / no-transition row in FSM | `ROBOTOS_CORE_OK` |
| Mapped dispatch, FSM action returned non-OK | FSM's non-OK status verbatim (FSM state IS committed; Phase 12C no-rollback) |
| Unmapped Adapter event | `ROBOTOS_CORE_OK` (FSM not called) |
| `dispatch` / `reset` / `snapshot` with NULL bridge | `ROBOTOS_CORE_ERR_NULL` |
| `init` with any NULL required pointer (`bridge`, `config`, `config->rows`, `config->fsm`) | `ROBOTOS_CORE_ERR_NULL` |
| `init` with `config->row_count == 0` | `ROBOTOS_CORE_ERR_INVALID_ARG` |
| `dispatch` / `reset` / `snapshot` on uninitialized bridge | `ROBOTOS_CORE_ERR_INVALID_STATE` |

### D.7 Reset policy

`robotos_fw_event_bridge_reset()` zeroes only bridge counters and
`last_*` fields. The bridge stays initialized; the config pointer is
preserved. **The bridge does NOT call `robotos_fw_fsm_reset()`.** FSM
state, FSM counters, and FSM `current_state` survive a bridge reset.
(Asserted by TC15.)

### D.8 Re-init policy

`robotos_fw_event_bridge_init()` is idempotent: a second `init()` on
an already-initialized bridge zeroes counters and re-installs the
config pointer. The FSM is **not** touched by re-init. (Asserted by
TC17.)

### D.9 No heap, no global singleton, no UART, no core registration

- No `malloc`/`free`/`calloc`/`realloc` anywhere in
  `robotos_fw_event_bridge.c`. (`grep -nE
  '(\bmalloc\b|\bfree\b|\bcalloc\b|\brealloc\b)'` returns no matches.)
- No static mutable state in the .c file (all state lives in the
  caller-owned instance).
- No call to `robotos_core_register_event_handler` or any
  `robotos_core_*` registration / dispatcher API.
- No UART API anywhere in the bridge or its test.
- No Zephyr / devkit / legacy `ro_*` include anywhere.

---

## E. Behavior Coverage Matrix

Coverage of the 18 Phase 12F-pre required behaviors:

| # | Required behavior | Verdict | Evidence |
|---|---|---|---|
| 1 | bridge init valid config | `ASSERTED_BY_TEST` | TC01 (8 assertions) |
| 2 | bridge init invalid config | `ASSERTED_BY_TEST` | TC02 (4 NULL paths) + TC03 (`row_count == 0`) |
| 3 | mapped event dispatches FSM transition | `ASSERTED_BY_TEST` | TC05 (FSM transitions IDLE→ARMED; bridge + FSM counters consistent) |
| 4 | unmapped event ignored / no FSM dispatch | `ASSERTED_BY_TEST` | TC06 (FSM `event_count == 0`, bridge `unmapped_count == 1`) |
| 5 | payload pointer passed through as borrowed | `ASSERTED_BY_TEST` | TC07 (FSM action sees exact pointer) |
| 6 | bridge does not retain payload pointer | `ASSERTED_BY_TEST` (structural) | TC08 (sum-of-fields check; no payload slot in `robotos_fw_event_bridge_t` or snapshot) |
| 7 | FSM dispatch non-OK propagated | `ASSERTED_BY_TEST` | TC09 (`ACTION_FAIL_STATUS` returned verbatim; FSM state still committed) |
| 8 | bridge does not call UART/hardware/core dispatcher | `REVIEW_VALIDATED` | `framework/robotos_fw_event_bridge.c` includes only `"robotos_fw_event_bridge.h"` + `<stddef.h>`; grep for `uart`/`gpio`/`pwm`/`i2c`/`spi`/`robotos_core_register_event_handler` returns 0 in bridge `.c` and `.h` |
| 9 | mapping FIFO first-match deterministic | `ASSERTED_BY_TEST` | TC10 (two identical-key rows; row 0 wins) |
| 10 | adapter_arg0 exact match | `ASSERTED_BY_TEST` | TC11 (arg0=A→FW_EV_ARM; arg0=B→FW_EV_START; arg0=C→unmapped) |
| 11 | adapter_arg0 wildcard match | `ASSERTED_BY_TEST` | TC12 (arg0=0 and arg0=7777 both fire the wildcard row) |
| 12 | exact match precedence vs wildcard | `ASSERTED_BY_TEST` | TC13 — chosen rule: **row order is the only precedence**; wildcard row at index 0 wins over an exact-match row at index 1 for the same type |
| 13 | multiple mappings supported | `ASSERTED_BY_TEST` | TC14 (4 mapping rows across 4 distinct adapter types drive a state sequence) |
| 14 | bridge reset resets bridge counters but not FSM state | `ASSERTED_BY_TEST` | TC15 (bridge counters zeroed; FSM `current_state` and `transition_count` preserved; `reset(NULL)` / `reset(uninit)` error paths covered) |
| 15 | snapshot reports counts and last fields | `ASSERTED_BY_TEST` | TC16 (coherence with direct field reads; 3 dispatches; NULL/uninit error paths) |
| 16 | no devkit_app_state modification | `REVIEW_VALIDATED` | bridge `.c` and `.h` and test file do not include `devkit_app_state.h`; grep for `devkit_app_state` returns 0 matches across the three new files |
| 17 | no command semantic change | `REVIEW_VALIDATED` | grep for `'a'`/`'s'`/`'r'`/`'?'`/`'x'`/`'v'`/`'L'`/`'d'`/`'T'` byte handling returns 0 in bridge files; `devkit/src/*` zero-diff |
| 18 | no heap allocation | `REVIEW_VALIDATED` | `grep -nE '(\bmalloc\b\|\bfree\b\|\bcalloc\b\|\brealloc\b)' framework/robotos_fw_event_bridge.c` returns 0 |

Total host-asserted: 14 of 18 (assertions: 103).
Review-validated (architectural / non-runtime): 4 of 18 (items 8, 16, 17, 18).

---

## F. Host Build / Test Result

### F.1 Environment

| Item | Value |
|---|---|
| Host | WSL Ubuntu (per Phase 12E precedent) |
| Compiler | gcc 13.3.0 |
| CMake | system cmake (4.x) |
| Generator | Unix Makefiles |
| MSYS2 MinGW64 | NOT used (Phase 12D documented failure mode preserved) |

### F.2 Commands

```text
wsl bash -lc 'cd /mnt/d/Robot_OS \
  && rm -rf build-phase12f-host \
  && cmake -S RobotOS_v1.0/tests/host -B build-phase12f-host -G "Unix Makefiles"'
wsl bash -lc 'cd /mnt/d/Robot_OS && cmake --build build-phase12f-host'
wsl bash -lc 'cd /mnt/d/Robot_OS && ./build-phase12f-host/robotos_fw_event_bridge_contract_test'
wsl bash -lc 'cd /mnt/d/Robot_OS && ctest --test-dir build-phase12f-host --output-on-failure'
wsl bash -lc 'cd /mnt/d/Robot_OS && cmake --build build-phase12f-host --target save_test_log'
```

### F.3 Results

- Configure: PASS. gcc 13.3.0 detected; no missing dependency.
- Build: 22 host targets built clean. Bridge target
  `robotos_fw_event_bridge_contract_test` linked successfully against
  `framework/robotos_fw_event_bridge.c` + `framework/robotos_fw_fsm.c`
  + `robotos_platform_critical_host_stub.c`. No warnings.
- Bridge direct run: **103/103 PASS, 0 FAIL** across 17 cases (TC01
  through TC17).
- Full regression: **22/22 PASS, 0 FAIL** (was 21/21 at Phase 12E;
  the only delta is the new `robotos_fw_event_bridge_contract` test;
  every Phase 4–6 / 12E test still passes).
- Log persistence: written to
  `RobotOS_v1.0/tests/host/logs/host_2026-05-13.log` by the existing
  `save_test_log` cmake target; phase-tagged copy at
  `RobotOS_v1.0/tests/host/logs/phase_12F_host_2026-05-13.log`.

### F.4 Log path

`RobotOS_v1.0/tests/host/logs/phase_12F_host_2026-05-13.log`
(80 KB; full `ctest --output-on-failure --verbose` transcript;
includes all 22 ctest entries and the per-assertion PASS lines for
the bridge contract test).

---

## G. Scope Guard Audit

| Guard | Status | Evidence |
|---|---|---|
| No devkit integration | HELD | `devkit/src/*` and `devkit/CMakeLists.txt` zero-diff; bridge never `#include`s any devkit header; bridge never calls a `devkit_*` function. |
| No UART command added | HELD | Frozen command set `a/s/r/?/x/v/L/d/T` unchanged; bridge `.c`/`.h`/test contain no UART byte handler. |
| No command semantic change | HELD | `devkit_uart_*.c` zero-diff; `devkit_app_state.c` zero-diff. |
| No hardware run | HELD | No `west flash`, no RTT, no J-Link, no OpenOCD invoked in this phase. Host-only validation under WSL/gcc. |
| No `devkit_app_state` change | HELD | `grep -n devkit_app_state framework/robotos_fw_event_bridge.{h,c} tests/host/test_robotos_fw_event_bridge.c` returns 0; `devkit/src/devkit_app_state.c` zero-diff. Scope-guard #11 re-affirmed. |
| No `devkit_app_state` promotion / replacement / duplication | HELD | No new symbol, file, or path duplicates `devkit_app_state` responsibility. |
| No Scheduler 7A/7B work | HELD | DEFER preserved. |
| No F407 / custom board work | HELD | HOLD/DEFER preserved. |
| No ACTIVE disarm widening | HELD | `USER_DECISION_REQUIRED_ACTIVE_DISARM` preserved. |
| No POST_FLASH_AUTOSTART change | HELD | `OPEN/MITIGATED_BY_WORKFLOW` preserved; no boot path touched. |
| No Application / product source | HELD | No `app/` directory created; no product vocabulary added. |
| No legacy Architecture B change | HELD | `src/*`, `include/robotos/*`, root `RobotOS_v1.0/CMakeLists.txt` zero-diff. |
| No root CMake change | HELD | `RobotOS_v1.0/CMakeLists.txt` and `RobotOS_v1.0/devkit/CMakeLists.txt` zero-diff. |
| No core/platform mutation | HELD | `core/*` and `platform/*` zero-diff. Bridge compiles against existing core types via the FSM header chain only. |
| No Zephyr / prj.conf / board change | HELD | `prj.conf`, board DTS, overlay files zero-diff. |
| No heap allocation | HELD | `grep -nE '(\bmalloc\b\|\bfree\b\|\bcalloc\b\|\brealloc\b)'` in `framework/robotos_fw_event_bridge.c` returns 0. |
| No Zephyr / devkit / legacy `ro_*` include in bridge | HELD | Only `#include "robotos_fw_event_bridge.h"` + `<stddef.h>` in `.c`; only `<stdbool.h>` + `<stdint.h>` + `"robotos_fw_fsm.h"` in `.h`. |
| No UART/hardware/core dispatcher registration | HELD | No `robotos_core_register_event_handler` or `robotos_core_*` registration call anywhere in bridge or test. |
| Devkit hardware evidence logs not touched | HELD | `RobotOS_v1.0/devkit/docs/logs/*` zero-diff. |
| Runtime scripts not touched | HELD | No script in `RobotOS_v1.0/devkit/` or repo root modified. |

All 20 scope guards held.

---

## H. Remaining Gaps / Next Gate

### H.1 What is still NOT_STARTED

- **Devkit integration** of the Framework FSM + bridge. Three modes
  are enumerated in Phase 12F-pre §G — shadow, replacement, separate
  application — and **none of them is in scope for Phase 12F**. Each
  requires its own future planning gate.
- **Application / product layer.** No `app/<product>/` directory
  exists. Product vocabulary (`IDLE/ARMED/ACTIVE`, etc.) lives only
  inside `devkit_app_state.c` and host test source. The bridge is
  product-neutral by design.
- **Hardware evidence for the Framework path.** Phase 12F adds zero
  hardware runs. The Framework FSM + bridge have host-test evidence
  only. The first hardware run of the Framework stack will be at the
  earliest devkit-integration phase (Phase 12G or later, not yet
  authorized).
- **ABI lock.** Memory layout of the bridge struct is still
  `DRAFT / EXPERIMENTAL`. Only names and signatures are locked.

### H.2 Recommended next gate

**Recommend: HOLD or a docs-only Phase 12G-pre devkit-integration-mode
decision.** Do not open direct devkit integration without an explicit
planning gate that chooses among shadow / replacement / separate
application. The Phase 12F-pre §G enumeration is the input; the
output is a Phase 12G-pre closeout that names the mode, the surface
to be touched, and the boundary against `devkit_app_state`.

A second viable next gate is **additional host-only bridge behavior**
(e.g., `arg1` matching, an unmapped-event diagnostic hook, fan-out).
This would extend the Phase 12F prototype without crossing the
devkit boundary, and would not require a new planning gate as
intrusive as 12G-pre. It is, however, lower-priority than the
integration-mode question.

**Not recommended for the next gate:** any direct devkit edit, any
UART command addition, any hardware run, or any ACTIVE-disarm /
Scheduler / F407 work. Those remain explicitly gated.

---

## I. Decision Result

**`PHASE_12F_APPLICATION_BRIDGE_HOST_PROTOTYPE_CLOSED`**

- Bridge implementation host-test-validated (103/103, 17 cases).
- Full host regression PASS (22/22).
- §5 of the bridge spec is now `LOCKED-AT-12F` for names and
  signatures.
- No devkit integration. No hardware evidence. No command-set
  change. No `devkit_app_state` change. No Architecture B change.

---

## J. Cross-references

- Phase 12F-pre planning closeout:
  [`PHASE_12F_PRE_APPLICATION_BRIDGE_PLANNING.md`](PHASE_12F_PRE_APPLICATION_BRIDGE_PLANNING.md)
- Long-lived bridge spec:
  [`../03_SPECS/FRAMEWORK_APPLICATION_BRIDGE_DRAFT.md`](../03_SPECS/FRAMEWORK_APPLICATION_BRIDGE_DRAFT.md)
- Long-lived FSM spec:
  [`../03_SPECS/FRAMEWORK_FSM_API_DRAFT.md`](../03_SPECS/FRAMEWORK_FSM_API_DRAFT.md)
- Phase 12E implementation closeout:
  [`PHASE_12E_FSM_HOST_TEST_IMPLEMENTATION.md`](PHASE_12E_FSM_HOST_TEST_IMPLEMENTATION.md)
- Phase 12C event bridge + status model confirmation:
  [`PHASE_12C_FSM_EVENT_BRIDGE_STATUS_MODEL.md`](PHASE_12C_FSM_EVENT_BRIDGE_STATUS_MODEL.md)
- Phase 12D-pre legacy disposition:
  [`PHASE_12D_PRE_LEGACY_FRAMEWORK_SCAFFOLD_DISPOSITION.md`](PHASE_12D_PRE_LEGACY_FRAMEWORK_SCAFFOLD_DISPOSITION.md)
- Live state:
  [`../../../../CURRENT_STATE.md`](../../../../CURRENT_STATE.md)
