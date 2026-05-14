# RobotOS — Probe Translator Host Prototype Implementation Plan

**Status:** `IMPLEMENTED_AT_12I (HOST-TEST EVIDENCE)`. The Phase 12I
host prototype implementation exists at
`RobotOS_v1.0/app/probe_translator/` and is host-test validated
(23/23 ctest targets PASS; 70/70 assertions PASS in the new
`probe_translator_mapping_contract` target). This document remains
the execution-ready implementation contract for the harness and is
extended at Phase 12I with the materialized file list and the
single deviation from the §6 spec example.
**Revision:** Phase 12I (2026-05-13, `CLOSED_WITH_HOST_TEST_EVIDENCE`;
`PHASE_12I_PROBE_TRANSLATOR_HOST_PROTOTYPE_IMPLEMENTED_VALIDATED`) —
materialized file set; embedded-config deviation recorded in §6.
Phase 12I-pre (2026-05-13, `CLOSED_DOCS_ONLY`;
`PHASE_12I_PRE_PROBE_TRANSLATOR_HOST_PROTOTYPE_PLAN_CLOSED`) —
initial draft.
**Next revision condition:** Future app-behavior phase (FAULT block);
future Architecture-A devkit-integration phase; future hardware-
runnable Zephyr build of `app/probe_translator/`.

> **Phase 12I implementation exists.** The path
> `RobotOS_v1.0/app/probe_translator/` is materialized with
> `probe_translator.{h,c}` + `README.md`; the host test target
> `probe_translator_mapping_contract` is wired via an additive block
> in `tests/host/CMakeLists.txt` (Option A).

This spec is anchored to
[`../02_PHASE_CLOSEOUTS/PHASE_12I_PRE_PROBE_TRANSLATOR_HOST_PROTOTYPE_PLAN.md`](../02_PHASE_CLOSEOUTS/PHASE_12I_PRE_PROBE_TRANSLATOR_HOST_PROTOTYPE_PLAN.md)
and extends
[`PROBE_TRANSLATOR_APP_SKELETON_DRAFT.md`](PROBE_TRANSLATOR_APP_SKELETON_DRAFT.md).

> **FAULT block extension is planned separately.** The Phase 12I
> deferrals (FAULT state / event / adapter type / transition rows
> 5-9 / mapping row 4 / `PROBE_ADAPTER_ARG_ANY`) are converted into
> an execution-ready additive contract by Phase 12J-pre — see
> [`PROBE_TRANSLATOR_FAULT_BLOCK_PLAN.md`](PROBE_TRANSLATOR_FAULT_BLOCK_PLAN.md).
> This Phase 12I spec is **not modified** to record FAULT additions;
> Phase 12J will add a forward-reference cross-link at its close.

---

## 1. Status / Scope

**What this doc is:**
The complete implementation contract for Phase 12I. All numeric
values, struct layouts, API shapes, CMake block, test case names, and
validation gates are locked here. A Phase 12I implementer should be
able to write every required file solely from this spec without
returning to prior planning docs.

**What this doc is not:**
- An application implementation. No `.c` / `.h` / `CMakeLists.txt`
  exists at `app/probe_translator/`.
- A product spec. The harness is product-neutral.
- A devkit integration spec.
- A command-vocabulary spec. The frozen `a/s/r/?/x/v/L/d/T` devkit
  command set is unchanged.

**Resolved decisions (Phase 12I-pre):**

| Decision | Resolution |
|---|---|
| Numeric state values | `IDLE=1u`, `READY=2u`, `ACTIVE=3u` |
| Numeric event values | `CONFIGURED=1u`, `START=2u`, `STOP=3u`, `RESET=4u` |
| Numeric adapter type values | `CONFIG=1u`, `COMMAND=2u` |
| Numeric adapter arg values | `NONE=0u`, `START=1u`, `STOP=2u`, `RESET=3u` |
| `FAULT` block | **DEFERRED** — not declared in Phase 12I |
| Transition row 5 (`IDLE+RESET→IDLE`) | **DEFERRED** — not in Phase 12I transition table |
| `PROBE_ADAPTER_ARG_ANY` | **OMITTED** — no wildcard row needed at Phase 12I |
| `probe_translator_t` ownership | **Embed by value** (FSM + bridge both inside the struct) |
| `probe_translator_snapshot_t` shape | **Combined struct** (`fsm` + `bridge` fields) |
| `probe_translator_config_t` | **Single `user_context` field** (static tables are module-internal) |
| Non-NULL action row | **None** — all rows use `action=NULL` at Phase 12I; TC08 is `REVIEW_VALIDATED` |
| Build strategy | **Option A** — additive block in `tests/host/CMakeLists.txt` |
| Host test case count | **15 items** (TC01–TC15) |
| Expected regression count | **23/23** after Phase 12I adds one new test target |

---

## 2. Phase 12I Materialized File Set

| Path | Status at Phase 12I close |
|---|---|
| `RobotOS_v1.0/app/probe_translator/probe_translator.h` | **MATERIALIZED** — public header. |
| `RobotOS_v1.0/app/probe_translator/probe_translator.c` | **MATERIALIZED** — implementation. |
| `RobotOS_v1.0/app/probe_translator/README.md` | **MATERIALIZED** — short app README. |
| `RobotOS_v1.0/tests/host/test_app_probe_translator_mapping.c` | **MATERIALIZED** — 15-case host test (TC01–TC15; 70 runtime assertions). |
| `RobotOS_v1.0/tests/host/CMakeLists.txt` | **MODIFIED (additive only)** — new `APP_DIR` + `add_executable` + `target_include_directories` + `add_test` block. |
| `RobotOS_v1.0/tests/host/logs/phase_12I_host_2026-05-13.log` | **MATERIALIZED** — host evidence log (23/23 PASS). |
| `RobotOS_v1.0/devkit/docs/02_PHASE_CLOSEOUTS/PHASE_12I_PROBE_TRANSLATOR_HOST_PROTOTYPE.md` | **MATERIALIZED** — Phase 12I closeout. |

**Forbidden at Phase 12I (still held at Phase 12I close):**
`app/probe_translator/CMakeLists.txt`; any Zephyr / devkit / UART /
hardware / Architecture B file.

---

## 3. Numeric ID Plan

### 3.1 State IDs (`robotos_fw_state_id_t`)

```c
#define PROBE_TRANSLATOR_STATE_IDLE   ((robotos_fw_state_id_t)1u)
#define PROBE_TRANSLATOR_STATE_READY  ((robotos_fw_state_id_t)2u)
#define PROBE_TRANSLATOR_STATE_ACTIVE ((robotos_fw_state_id_t)3u)
/* PROBE_TRANSLATOR_STATE_FAULT = 4u — RESERVED; DEFERRED */
```

`0u` (`ROBOTOS_FW_STATE_UNINIT`) is reserved by the Framework and is
never used as a product state. `initial_state = 0u` would cause
`robotos_fw_fsm_init` to return `ERR_INVALID_ARG`.

### 3.2 Event IDs (`robotos_fw_event_id_t`)

```c
#define PROBE_TRANSLATOR_EVT_CONFIGURED ((robotos_fw_event_id_t)1u)
#define PROBE_TRANSLATOR_EVT_START      ((robotos_fw_event_id_t)2u)
#define PROBE_TRANSLATOR_EVT_STOP       ((robotos_fw_event_id_t)3u)
#define PROBE_TRANSLATOR_EVT_RESET      ((robotos_fw_event_id_t)4u)
/* PROBE_TRANSLATOR_EVT_FAULT = 5u — RESERVED; DEFERRED */
```

### 3.3 Adapter type constants (`uint32_t`)

```c
#define PROBE_ADAPTER_TYPE_CONFIG  ((uint32_t)1u)
#define PROBE_ADAPTER_TYPE_COMMAND ((uint32_t)2u)
/* PROBE_ADAPTER_TYPE_FAULT = 3u — RESERVED; DEFERRED */
```

### 3.4 Adapter arg constants (`uint32_t`)

```c
#define PROBE_ADAPTER_ARG_NONE  ((uint32_t)0u)
#define PROBE_ADAPTER_ARG_START ((uint32_t)1u)
#define PROBE_ADAPTER_ARG_STOP  ((uint32_t)2u)
#define PROBE_ADAPTER_ARG_RESET ((uint32_t)3u)
/* PROBE_ADAPTER_ARG_ANY — OMITTED; no wildcard row at Phase 12I */
```

---

## 4. Struct / Ownership Plan

### 4.1 `probe_translator_t` (embed by value)

```c
struct probe_translator {
    robotos_fw_fsm_t                 fsm;
    robotos_fw_event_bridge_t        bridge;
    robotos_fw_fsm_config_t          _fsm_cfg;       /* opaque; see §6.4 note */
    robotos_fw_event_bridge_config_t _bridge_cfg;    /* opaque; see §6.4 note */
};
```

Statically allocated by the caller. No heap. The caller is responsible
for the lifetime of the instance and all referenced static tables. The
struct is declared in `probe_translator.h` for static sizing; fields
are opaque — access only through the public API.

**Note (Phase 12I close):** the `_fsm_cfg` / `_bridge_cfg` fields are
added relative to the Phase 12I-pre struct example so that the FSM and
bridge config objects share the harness instance's lifetime. See §6.4
for the rationale; this is a correctness fix to the Phase 12I-pre
example, not an API change.

### 4.2 `probe_translator_config_t`

```c
struct probe_translator_config {
    void *user_context;  /* may be NULL; passed through to FSM config */
};
```

The static tables (transitions, state defs, mapping rows) are
module-internal to `probe_translator.c`. The caller does not provide
them.

### 4.3 `probe_translator_snapshot_t`

```c
typedef struct {
    robotos_fw_fsm_snapshot_t          fsm;
    robotos_fw_event_bridge_snapshot_t bridge;
} probe_translator_snapshot_t;
```

Combined struct — one call fills both FSM and bridge snapshot fields.

### 4.4 Static tables in `probe_translator.c`

All four tables are `static const` and module-internal.

---

## 5. Header Contract Plan

`probe_translator.h` (to be created at Phase 12I):

```c
/*
 * probe_translator.h
 * First application harness — host prototype (Phase 12I).
 * STATUS: DRAFT / host prototype. No devkit. No Zephyr. No UART.
 * Architecture A only.
 */
#ifndef PROBE_TRANSLATOR_H
#define PROBE_TRANSLATOR_H

#include <stdint.h>
#include "robotos_fw_fsm.h"
#include "robotos_fw_event_bridge.h"

#ifdef __cplusplus
extern "C" {
#endif

/* State IDs */
#define PROBE_TRANSLATOR_STATE_IDLE   ((robotos_fw_state_id_t)1u)
#define PROBE_TRANSLATOR_STATE_READY  ((robotos_fw_state_id_t)2u)
#define PROBE_TRANSLATOR_STATE_ACTIVE ((robotos_fw_state_id_t)3u)

/* Event IDs */
#define PROBE_TRANSLATOR_EVT_CONFIGURED ((robotos_fw_event_id_t)1u)
#define PROBE_TRANSLATOR_EVT_START      ((robotos_fw_event_id_t)2u)
#define PROBE_TRANSLATOR_EVT_STOP       ((robotos_fw_event_id_t)3u)
#define PROBE_TRANSLATOR_EVT_RESET      ((robotos_fw_event_id_t)4u)

/* Adapter keys */
#define PROBE_ADAPTER_TYPE_CONFIG   ((uint32_t)1u)
#define PROBE_ADAPTER_TYPE_COMMAND  ((uint32_t)2u)
#define PROBE_ADAPTER_ARG_NONE      ((uint32_t)0u)
#define PROBE_ADAPTER_ARG_START     ((uint32_t)1u)
#define PROBE_ADAPTER_ARG_STOP      ((uint32_t)2u)
#define PROBE_ADAPTER_ARG_RESET     ((uint32_t)3u)

/* Types */
typedef struct probe_translator        probe_translator_t;
typedef struct probe_translator_config probe_translator_config_t;

struct probe_translator_config {
    void *user_context;
};

typedef struct {
    robotos_fw_fsm_snapshot_t          fsm;
    robotos_fw_event_bridge_snapshot_t bridge;
} probe_translator_snapshot_t;

struct probe_translator {
    robotos_fw_fsm_t          fsm;
    robotos_fw_event_bridge_t bridge;
};

/* Public API */
robotos_fw_status_t probe_translator_init(
    probe_translator_t              *pt,
    const probe_translator_config_t *config);

robotos_fw_status_t probe_translator_dispatch_adapter_event(
    probe_translator_t *pt,
    uint32_t            adapter_type,
    uint32_t            adapter_arg0,
    const void         *payload);

robotos_fw_status_t probe_translator_reset(probe_translator_t *pt);

robotos_fw_status_t probe_translator_get_snapshot(
    const probe_translator_t    *pt,
    probe_translator_snapshot_t *out);

#ifdef __cplusplus
}
#endif
#endif /* PROBE_TRANSLATOR_H */
```

---

## 6. Source Contract Plan

`probe_translator.c` includes only:

```c
#include "probe_translator.h"
#include <stddef.h>  /* NULL */
```

### 6.1 Transition table (4 rows, all guard=NULL, action=NULL)

```c
static const robotos_fw_transition_t k_transitions[] = {
    { PROBE_TRANSLATOR_STATE_IDLE,   PROBE_TRANSLATOR_EVT_CONFIGURED,
      NULL, PROBE_TRANSLATOR_STATE_READY,  NULL },
    { PROBE_TRANSLATOR_STATE_READY,  PROBE_TRANSLATOR_EVT_START,
      NULL, PROBE_TRANSLATOR_STATE_ACTIVE, NULL },
    { PROBE_TRANSLATOR_STATE_ACTIVE, PROBE_TRANSLATOR_EVT_STOP,
      NULL, PROBE_TRANSLATOR_STATE_READY,  NULL },
    { PROBE_TRANSLATOR_STATE_READY,  PROBE_TRANSLATOR_EVT_RESET,
      NULL, PROBE_TRANSLATOR_STATE_IDLE,   NULL },
    { PROBE_TRANSLATOR_STATE_ACTIVE, PROBE_TRANSLATOR_EVT_RESET,
      NULL, PROBE_TRANSLATOR_STATE_IDLE,   NULL },
};
```

### 6.2 State defs (3 entries, all callbacks NULL)

```c
static const robotos_fw_state_def_t k_state_defs[] = {
    { PROBE_TRANSLATOR_STATE_IDLE,   NULL, NULL },
    { PROBE_TRANSLATOR_STATE_READY,  NULL, NULL },
    { PROBE_TRANSLATOR_STATE_ACTIVE, NULL, NULL },
};
```

### 6.3 Mapping table (4 rows)

```c
static const robotos_fw_event_bridge_row_t k_mapping[] = {
    { PROBE_ADAPTER_TYPE_CONFIG,  PROBE_ADAPTER_ARG_NONE,  true,
      PROBE_TRANSLATOR_EVT_CONFIGURED },
    { PROBE_ADAPTER_TYPE_COMMAND, PROBE_ADAPTER_ARG_START, true,
      PROBE_TRANSLATOR_EVT_START },
    { PROBE_ADAPTER_TYPE_COMMAND, PROBE_ADAPTER_ARG_STOP,  true,
      PROBE_TRANSLATOR_EVT_STOP },
    { PROBE_ADAPTER_TYPE_COMMAND, PROBE_ADAPTER_ARG_RESET, true,
      PROBE_TRANSLATOR_EVT_RESET },
};
```

### 6.4 `probe_translator_init`

```c
robotos_fw_status_t probe_translator_init(
    probe_translator_t              *pt,
    const probe_translator_config_t *config)
{
    robotos_fw_status_t st;
    void               *user_context;

    if (!pt) return ROBOTOS_CORE_ERR_NULL;

    user_context = (config != NULL) ? config->user_context : NULL;

    pt->_fsm_cfg.transitions      = k_transitions;
    pt->_fsm_cfg.transition_count = 5u;
    pt->_fsm_cfg.states           = k_state_defs;
    pt->_fsm_cfg.state_count      = 3u;
    pt->_fsm_cfg.initial_state    = PROBE_TRANSLATOR_STATE_IDLE;
    pt->_fsm_cfg.user_context     = user_context;

    st = robotos_fw_fsm_init(&pt->fsm, &pt->_fsm_cfg);
    if (st != ROBOTOS_CORE_OK) return st;

    pt->_bridge_cfg.rows         = k_mapping;
    pt->_bridge_cfg.row_count    = 4u;
    pt->_bridge_cfg.fsm          = &pt->fsm;
    pt->_bridge_cfg.user_context = user_context;

    return robotos_fw_event_bridge_init(&pt->bridge, &pt->_bridge_cfg);
}
```

**Deviation note (Phase 12I close).** The Phase 12I-pre draft of this
section showed `robotos_fw_fsm_config_t fsm_cfg;` and
`robotos_fw_event_bridge_config_t bridge_cfg;` as **stack-local**
variables. That pattern is incorrect: `robotos_fw_fsm_init` and
`robotos_fw_event_bridge_init` both store the supplied config object
**by pointer** (`fsm->config = config` / `bridge->config = config`)
and the Framework headers document that the config must outlive the
instance. A stack-local config goes out of scope when `init` returns,
leaving a dangling pointer that the next dispatch dereferences. The
Phase 12I implementation embeds `_fsm_cfg` and `_bridge_cfg` into
`probe_translator_t` (see §4.1) so their lifetime matches the harness
instance. Public API is unchanged.

### 6.5 `probe_translator_dispatch_adapter_event`

```c
robotos_fw_status_t probe_translator_dispatch_adapter_event(
    probe_translator_t *pt,
    uint32_t            adapter_type,
    uint32_t            adapter_arg0,
    const void         *payload)
{
    if (!pt) return ROBOTOS_CORE_ERR_NULL;
    return robotos_fw_event_bridge_dispatch(
        &pt->bridge, adapter_type, adapter_arg0, payload);
}
```

### 6.6 `probe_translator_reset`

```c
robotos_fw_status_t probe_translator_reset(probe_translator_t *pt)
{
    robotos_fw_status_t st;
    if (!pt) return ROBOTOS_CORE_ERR_NULL;
    st = robotos_fw_event_bridge_reset(&pt->bridge);
    if (st != ROBOTOS_CORE_OK) return st;
    return robotos_fw_fsm_reset(&pt->fsm);
}
```

### 6.7 `probe_translator_get_snapshot`

```c
robotos_fw_status_t probe_translator_get_snapshot(
    const probe_translator_t    *pt,
    probe_translator_snapshot_t *out)
{
    robotos_fw_status_t st;
    if (!pt || !out) return ROBOTOS_CORE_ERR_NULL;
    st = robotos_fw_fsm_get_snapshot(&pt->fsm, &out->fsm);
    if (st != ROBOTOS_CORE_OK) return st;
    return robotos_fw_event_bridge_get_snapshot(&pt->bridge, &out->bridge);
}
```

---

## 7. Host Test Contract

Test file: `tests/host/test_app_probe_translator_mapping.c`

Uses the same `TC(name, cond)` harness as Phase 12E / 12F. Expected
total assertions: approximately 50–70 (modelled on Phase 12F 17
cases / 103 assertions).

| # | Test name | Key assertions |
|---|---|---|
| TC01 | `init_valid_starts_idle` | `probe_translator_init` → `OK`; snapshot `fsm.current_state == STATE_IDLE`; `fsm.initialized == true`; `bridge.event_count == 0`. |
| TC02 | `init_null_rejected` | `probe_translator_init(NULL, ...)` → `ERR_NULL`. |
| TC03 | `config_maps_idle_to_ready` | Dispatch `(TYPE_CONFIG, ARG_NONE, NULL)` → `OK`; `fsm.current_state == STATE_READY`; `fsm.transition_count == 1`; `bridge.mapped_count == 1`; `bridge.event_count == 1`. |
| TC04 | `start_maps_ready_to_active` | Dispatch `(TYPE_COMMAND, ARG_START, NULL)` from READY → `STATE_ACTIVE`. |
| TC05 | `stop_maps_active_to_ready` | Dispatch `(TYPE_COMMAND, ARG_STOP, NULL)` from ACTIVE → `STATE_READY`. |
| TC06a | `reset_maps_ready_to_idle` | Dispatch `(TYPE_COMMAND, ARG_RESET, NULL)` from READY → `STATE_IDLE`. |
| TC06b | `reset_maps_active_to_idle` | Dispatch `(TYPE_COMMAND, ARG_RESET, NULL)` from ACTIVE → `STATE_IDLE`. |
| TC07 | `unmapped_event_no_state_change` | Dispatch `(0xDEADBEEFu, 0xCAFEBABEu, NULL)`; `bridge.unmapped_count == 1`; FSM state unchanged; return `OK`. |
| TC08 | `payload_borrowed_review_validated` | REVIEW_VALIDATED: `action=NULL` on all rows; bridge stores no `last_payload`; design-level confirmation that payload pointer is not cached post-dispatch. |
| TC09 | `snapshot_contains_fsm_and_bridge_counts` | CONFIG→START→STOP→RESET; `fsm.transition_count == 4`; `bridge.event_count == 4`; `bridge.mapped_count == 4`; `bridge.unmapped_count == 0`; `fsm.current_state == STATE_IDLE`. |
| TC10 | `probe_translator_reset_clears_counters` | CONFIG then START; reset; `bridge.event_count == 0`; `bridge.mapped_count == 0`; `fsm.current_state == STATE_IDLE`. |
| TC11 | `full_path_idle_ready_active_ready_idle` | CONFIG→START→STOP→RESET; state at each step: `IDLE→READY→ACTIVE→READY→IDLE`; `fsm.transition_count == 4`. |
| TC12 | `grep_gate_no_devkit_dependency` | REVIEW_VALIDATED: no `devkit_app_state.h` / `devkit_*.h` in `probe_translator.{c,h}` or test. |
| TC13 | `grep_gate_no_uart_command_dependency` | REVIEW_VALIDATED: no `devkit_uart_*` symbols or UART command byte literals in `probe_translator.{c,h}` or test. |
| TC14 | `grep_gate_no_zephyr_devkit_legacy_include` | REVIEW_VALIDATED: no `<zephyr/...>`, `devkit_*.h`, `ro_*.h`, or `include/robotos/` headers. |
| TC15 | `full_host_regression_preserved` | All 23 test targets PASS (22 prior + 1 new). |

---

## 8. Build Strategy

### 8.1 CMake additive block

Added after the `robotos_fw_event_bridge_contract_test` block in
`tests/host/CMakeLists.txt`, before the `save_test_log` custom target:

```cmake
# ---- Application: Probe Translator host mapping contract test (Phase 12I) ---
set(APP_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../app/probe_translator")
add_executable(probe_translator_mapping_contract_test
    test_app_probe_translator_mapping.c
    "${APP_DIR}/probe_translator.c"
    "${FRAMEWORK_DIR}/robotos_fw_event_bridge.c"
    "${FRAMEWORK_DIR}/robotos_fw_fsm.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/robotos_platform_critical_host_stub.c"
)

target_include_directories(probe_translator_mapping_contract_test PRIVATE
    "${APP_DIR}"
    "${FRAMEWORK_DIR}"
    "${CORE_DIR}"
    "${PLATFORM_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}"
)

add_test(NAME probe_translator_mapping_contract
         COMMAND probe_translator_mapping_contract_test)
```

### 8.2 Build commands (WSL Ubuntu / gcc 13.3.0)

```bash
cmake -S RobotOS_v1.0/tests/host -B build-phase12i-host
cmake --build build-phase12i-host
ctest --test-dir build-phase12i-host --output-on-failure -V
cmake --build build-phase12i-host --target save_test_log
```

---

## 9. Validation and Exit Criteria

Phase 12I may close `CLOSED_WITH_HOST_TEST_EVIDENCE` when all 14 gates
in §K of `PHASE_12I_PRE_PROBE_TRANSLATOR_HOST_PROTOTYPE_PLAN.md` pass.
Key gates: CMake configure/build PASS; all 23 tests PASS; grep gates
clean; log committed; command set unchanged; `devkit_app_state`
zero-diff; no hardware run.

---

## 10. `devkit_app_state` Boundary

`devkit_app_state` remains authoritative. `probe_translator/` must not
include `devkit_app_state.h`, call any `devkit_*` function, or read /
write `devkit_app_state` snapshots. `PROBE_TRANSLATOR_STATE_IDLE /
READY / ACTIVE` are application-local. Scope-guard #11 active.

---

## 11. Command Set Boundary

`a / s / r / ? / x / v / L / d / T` remain devkit / probe surface.
`probe_translator/` defines no UART command. Framework bridge has no
UART surface (Phase 12F locked). All 12 UART TX scope-guard
constraints from Phase 9EZ §H preserved.

---

## 12. Legacy Architecture B Boundary

`src/` and `include/robotos/` frozen at `LEGACY_SCAFFOLD_MARKED_
FROZEN_DOCS_ONLY` (Phase 12D-pre). `probe_translator/` uses
Architecture A contracts only. No `ro_*` HAL include.

---

## 13. Open Decisions

| # | Open question | Latest at |
|---|---|---|
| 1 | Whether `PROBE_TRANSLATOR_STATE_FAULT` + FAULT event / adapter type / mapping row 4 / transition rows 6–9 / TC07 ship in a future behavior phase | Future app-behavior phase |
| 2 | Whether transition row 5 (`IDLE + RESET → IDLE`) ships in a future behavior phase | Future behavior phase |
| 3 | Whether `probe_translator/` ever runs as a hardware-runnable Zephyr application | Future runtime-integration phase |
| 4 | Whether to introduce `RobotOS_v1.0/examples/` for sample integrations | Future docs-only phase |
| 5 | Bridge ABI memory-layout lock (still open from Phase 12G §11 #9) | Future phase |

---

## 14. Next Revision Conditions

1. **Phase 12I opens (host prototype implementation).** §1 status
   becomes `IMPLEMENTED_AT_12I (HOST-TEST EVIDENCE)`. §2 updates with
   materialized file list. §7 / §9 cross-reference the evidence log.
2. **A future behavior phase ships the FAULT block.** §3 / §7 gain
   fault-related rows and test cases.
3. **A user override picks HOLD.** §1 records the hold; no further
   revision required until a future phase opens.

This spec is **NOT** revised to record devkit implementation details,
track per-phase test results, change command semantics, promote
`devkit_app_state`, or reactivate Architecture B.
