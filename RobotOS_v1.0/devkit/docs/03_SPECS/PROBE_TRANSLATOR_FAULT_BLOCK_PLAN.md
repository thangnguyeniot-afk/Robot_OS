# RobotOS — Probe Translator FAULT Block Extension Plan

**Status:** `DRAFT / NON-FINAL`. **No FAULT block implementation
exists.** This document is the execution-ready implementation contract
for **Phase 12J — Probe Translator FAULT Block Extension**, the
additive extension of the Phase 12I host prototype with a FAULT state,
FAULT event, FAULT adapter type, the optional `IDLE + RESET → IDLE`
self-loop, and a wildcard FAULT mapping row.
**Revision:** Phase 12J-pre (2026-05-13, `CLOSED_DOCS_ONLY`;
`PHASE_12J_PRE_PROBE_TRANSLATOR_FAULT_BLOCK_PLAN_CLOSED`) — initial
draft.
**Next revision condition:** Phase 12J (when authorized) — when the
FAULT block is added to `app/probe_translator/` and the host test
passes, this spec upgrades to
`IMPLEMENTED_AT_12J (HOST-TEST EVIDENCE)` and gains the materialized
diff list and evidence cross-references.

> **No FAULT-block implementation exists.** The constants
> `PROBE_TRANSLATOR_STATE_FAULT`, `PROBE_TRANSLATOR_EVT_FAULT`,
> `PROBE_ADAPTER_TYPE_FAULT`, and `PROBE_ADAPTER_ARG_ANY` are
> *reserved at planning depth* and **not declared** in
> `app/probe_translator/probe_translator.h`. No build target yet
> includes a FAULT transition row or FAULT mapping row.

This spec is anchored to
[`../02_PHASE_CLOSEOUTS/PHASE_12J_PRE_PROBE_TRANSLATOR_FAULT_BLOCK_PLAN.md`](../02_PHASE_CLOSEOUTS/PHASE_12J_PRE_PROBE_TRANSLATOR_FAULT_BLOCK_PLAN.md)
and extends
[`PROBE_TRANSLATOR_HOST_PROTOTYPE_PLAN.md`](PROBE_TRANSLATOR_HOST_PROTOTYPE_PLAN.md)
(Phase 12I implementation contract, `IMPLEMENTED_AT_12I`).

---

## 1. Status / Scope

**What this doc is:**
The complete execution-ready implementation contract for Phase 12J.
All numeric values, transition rows, mapping rows, state-def entries,
test case names, and validation gates are locked here. A Phase 12J
implementer should be able to add every required diff solely from
this spec without returning to prior planning docs.

**What this doc is not:**
- A FAULT-block implementation. No `.c` / `.h` diff exists yet.
- A devkit integration spec.
- A command-vocabulary spec. The frozen `a/s/r/?/x/v/L/d/T` devkit
  command set is unchanged.
- A second-application spec.
- A Zephyr / hardware spec. Phase 12J remains host-only.

**Resolved decisions (Phase 12J-pre):**

| Decision | Resolution |
|---|---|
| Numeric value `PROBE_TRANSLATOR_STATE_FAULT` | `4u` (formalized from Phase 12I-pre §E.1 reservation) |
| Numeric value `PROBE_TRANSLATOR_EVT_FAULT` | `5u` (formalized from Phase 12I-pre §E.2 reservation) |
| Numeric value `PROBE_ADAPTER_TYPE_FAULT` | `3u` (formalized from Phase 12I-pre §E.3 reservation) |
| `PROBE_ADAPTER_ARG_ANY` declaration | **Declared** as `((uint32_t)0xFFFFFFFFu)` documentation alias only; actual wildcard semantics come from `match_arg0 == false`. |
| Transition row 5 (`IDLE + RESET → IDLE` self-loop) | **SHIPPED** at Phase 12J (was Phase 12I-deferred). |
| Transition rows 6-9 (FAULT block) | **SHIPPED** at Phase 12J. |
| Mapping row 4 (FAULT wildcard) | **SHIPPED** at Phase 12J. |
| Non-NULL action rows | **None** — all new rows `action=NULL`. |
| New host test target | **None** — extend existing `probe_translator_mapping_contract` target. |
| CMake change | **None** — Phase 12I additive block already references `probe_translator.c`; new content lives in the same compilation unit. |
| Expected new in-binary assertions | **Additive** — Phase 12I 70 + Phase 12J ~30-40 ≈ 100-110 total. |
| Expected ctest target count after Phase 12J | **23/23** (unchanged; no new target). |
| Hardware run | **None.** |

---

## 2. Phase 12J Approved File Set

Phase 12J may **only** modify or create the files listed here. Any
other surface is forbidden.

### 2.1 Modified existing files (additive only)

| Path | Change |
|---|---|
| `RobotOS_v1.0/app/probe_translator/probe_translator.h` | Additive: declare `PROBE_TRANSLATOR_STATE_FAULT`, `PROBE_TRANSLATOR_EVT_FAULT`, `PROBE_ADAPTER_TYPE_FAULT`, `PROBE_ADAPTER_ARG_ANY`. Comments updated from "RESERVED; DEFERRED at Phase 12I" to "Shipped at Phase 12J". Public API unchanged. Struct layout unchanged. |
| `RobotOS_v1.0/app/probe_translator/probe_translator.c` | Additive: 5 new transition rows (row 5 self-loop + rows 6-9 FAULT), 1 new state def for STATE_FAULT, 1 new mapping row (TYPE_FAULT wildcard). `transition_count` bumped 5→10, `state_count` bumped 3→4, `row_count` bumped 4→5 in `probe_translator_init`. No public API change; no struct change. |
| `RobotOS_v1.0/app/probe_translator/README.md` | Additive: FAULT-related symbols appended to the state/event/adapter-key table; "Non-goals" unchanged. |
| `RobotOS_v1.0/tests/host/test_app_probe_translator_mapping.c` | Additive: new TC16..TC24 (~9 cases, ~30-40 assertions). TC01..TC15 retained verbatim. |

### 2.2 New files

| Path | Purpose |
|---|---|
| `RobotOS_v1.0/tests/host/logs/phase_12J_host_<YYYY-MM-DD>.log` | Committed evidence log (full `ctest --verbose` transcript). |
| `RobotOS_v1.0/devkit/docs/02_PHASE_CLOSEOUTS/PHASE_12J_PROBE_TRANSLATOR_FAULT_BLOCK.md` | Phase 12J closeout. |

### 2.3 Doc-sync updates at Phase 12J close

| Path | Change |
|---|---|
| `RobotOS_v1.0/devkit/docs/03_SPECS/PROBE_TRANSLATOR_FAULT_BLOCK_PLAN.md` | Status upgrade `DRAFT / NON-FINAL` → `IMPLEMENTED_AT_12J (HOST-TEST EVIDENCE)`; materialized diff list. |
| `RobotOS_v1.0/devkit/docs/03_SPECS/PROBE_TRANSLATOR_HOST_PROTOTYPE_PLAN.md` | Note FAULT block now shipped; cross-reference Phase 12J evidence. Numeric values / Phase 12I 5-row table preserved verbatim. |
| `RobotOS_v1.0/devkit/docs/03_SPECS/PROBE_TRANSLATOR_APP_SKELETON_DRAFT.md` | §5 / §6 / §7 / §8 update: FAULT block "Optional" → "Shipped at 12J"; transition row 5 likewise. Sections otherwise unchanged. |
| `RobotOS_v1.0/devkit/docs/01_PROGRESS/DEVKIT_PROGRESS_PHASE_11_20.md` | Phase 12J index row + `<a id="phase-12j"></a>` section. |
| `CURRENT_STATE.md` | Phase 12J recorded as latest closed; Phase 12J-pre demoted to prior closed. |
| `RobotOS_v1.0/devkit/docs/00_INDEX/README.md` | Phase 12J closeout link; spec status updates. |

### 2.4 Forbidden at Phase 12J

| Forbidden path | Reason |
|---|---|
| `RobotOS_v1.0/app/probe_translator/CMakeLists.txt` | Option B deferred indefinitely. |
| `RobotOS_v1.0/tests/host/CMakeLists.txt` | **Zero-diff.** The Phase 12I additive block already wires `probe_translator.c`; FAULT block additions live in the same compilation unit. Any change would expand the patch surface beyond the contract. |
| Any Zephyr / `prj.conf` / DTS / overlay file | Host-only at Phase 12J. |
| Any `framework/` / `core/` / `platform/` / `devkit/src/` file | Boundary-preserving phase. |
| Any `src/` / `include/robotos/` file | Architecture B remains frozen. |
| Any new `app/` subdirectory | One product at a time. |
| Any new ctest target | Extend the existing `probe_translator_mapping_contract` target. |
| Any RTT / J-Link / OpenOCD / flashing script | No hardware run. |
| Any UART command byte literal | `a/s/r/?/x/v/L/d/T` frozen surface. |

---

## 3. Numeric ID Plan

All values formalize the Phase 12I-pre reservations.

### 3.1 State IDs

```c
#define PROBE_TRANSLATOR_STATE_IDLE   ((robotos_fw_state_id_t)1u)  /* unchanged */
#define PROBE_TRANSLATOR_STATE_READY  ((robotos_fw_state_id_t)2u)  /* unchanged */
#define PROBE_TRANSLATOR_STATE_ACTIVE ((robotos_fw_state_id_t)3u)  /* unchanged */
#define PROBE_TRANSLATOR_STATE_FAULT  ((robotos_fw_state_id_t)4u)  /* NEW at 12J */
```

### 3.2 Event IDs

```c
#define PROBE_TRANSLATOR_EVT_CONFIGURED ((robotos_fw_event_id_t)1u)  /* unchanged */
#define PROBE_TRANSLATOR_EVT_START      ((robotos_fw_event_id_t)2u)  /* unchanged */
#define PROBE_TRANSLATOR_EVT_STOP       ((robotos_fw_event_id_t)3u)  /* unchanged */
#define PROBE_TRANSLATOR_EVT_RESET      ((robotos_fw_event_id_t)4u)  /* unchanged */
#define PROBE_TRANSLATOR_EVT_FAULT      ((robotos_fw_event_id_t)5u)  /* NEW at 12J */
```

### 3.3 Adapter type constants

```c
#define PROBE_ADAPTER_TYPE_CONFIG  ((uint32_t)1u)  /* unchanged */
#define PROBE_ADAPTER_TYPE_COMMAND ((uint32_t)2u)  /* unchanged */
#define PROBE_ADAPTER_TYPE_FAULT   ((uint32_t)3u)  /* NEW at 12J */
```

### 3.4 Adapter arg constants

```c
#define PROBE_ADAPTER_ARG_NONE  ((uint32_t)0u)           /* unchanged */
#define PROBE_ADAPTER_ARG_START ((uint32_t)1u)           /* unchanged */
#define PROBE_ADAPTER_ARG_STOP  ((uint32_t)2u)           /* unchanged */
#define PROBE_ADAPTER_ARG_RESET ((uint32_t)3u)           /* unchanged */
#define PROBE_ADAPTER_ARG_ANY   ((uint32_t)0xFFFFFFFFu)  /* NEW at 12J; doc alias */
```

**`PROBE_ADAPTER_ARG_ANY` semantics (locked at Phase 12J-pre):**
- Header-level documentation alias for "wildcard arg0 in a mapping
  row".
- The mapping row's `match_arg0` field is the actual switch: when
  `match_arg0 == false`, the bridge ignores the row's `adapter_arg0`
  value entirely (Phase 12F §5.1 / `robotos_fw_event_bridge.h`).
- Choosing `0xFFFFFFFFu` as the sentinel value is a readability /
  inspection aid only; the bridge never compares against it because
  `match_arg0 == false`.
- **No bridge code change is required.** Phase 12F already supports
  `match_arg0 == false`; the Phase 12F bridge contract test
  (`robotos_fw_event_bridge_contract`) already covers wildcard
  semantics. Phase 12J does not modify `framework/`.

---

## 4. State Definition Table

Phase 12J state-def table has **4 entries** (Phase 12I 3 + 1 new).

```c
static const robotos_fw_state_def_t k_probe_translator_state_defs[] = {
    { PROBE_TRANSLATOR_STATE_IDLE,   NULL, NULL },  /* row 0 — unchanged */
    { PROBE_TRANSLATOR_STATE_READY,  NULL, NULL },  /* row 1 — unchanged */
    { PROBE_TRANSLATOR_STATE_ACTIVE, NULL, NULL },  /* row 2 — unchanged */
    { PROBE_TRANSLATOR_STATE_FAULT,  NULL, NULL },  /* row 3 — NEW at 12J */
};
```

All on_entry / on_exit callbacks remain `NULL`. Action-driven
behavior in FAULT is deferred to a future phase.

`state_count` bumps from `3u` to `4u` in `probe_translator_init`.

---

## 5. Transition Table Plan

Phase 12J transition table has **10 rows** (Phase 12I 5 + 5 new).
Row order is FIFO; FIFO order matters when two rows could match the
same (state, event) pair, but no Phase 12J pair has multiple
matching rows.

| Row | From | Event | To | Source | Status |
|---|---|---|---|---|---|
| 0 | `STATE_IDLE` | `EVT_CONFIGURED` | `STATE_READY` | Phase 12I | unchanged |
| 1 | `STATE_READY` | `EVT_START` | `STATE_ACTIVE` | Phase 12I | unchanged |
| 2 | `STATE_ACTIVE` | `EVT_STOP` | `STATE_READY` | Phase 12I | unchanged |
| 3 | `STATE_READY` | `EVT_RESET` | `STATE_IDLE` | Phase 12I | unchanged |
| 4 | `STATE_ACTIVE` | `EVT_RESET` | `STATE_IDLE` | Phase 12I | unchanged |
| 5 | `STATE_IDLE` | `EVT_RESET` | `STATE_IDLE` | Phase 12J | **NEW** — self-loop |
| 6 | `STATE_IDLE` | `EVT_FAULT` | `STATE_FAULT` | Phase 12J | **NEW** |
| 7 | `STATE_READY` | `EVT_FAULT` | `STATE_FAULT` | Phase 12J | **NEW** |
| 8 | `STATE_ACTIVE` | `EVT_FAULT` | `STATE_FAULT` | Phase 12J | **NEW** |
| 9 | `STATE_FAULT` | `EVT_RESET` | `STATE_IDLE` | Phase 12J | **NEW** — FAULT exit |

All new rows use `guard = NULL` and `action = NULL`.

`transition_count` bumps from `5u` to `10u` in `probe_translator_init`.

### 5.1 Behavior in FAULT for non-RESET events

`STATE_FAULT + EVT_CONFIGURED / EVT_START / EVT_STOP` are
**intentionally absent** from the transition table:

- The bridge mapping table will map a `(TYPE_CONFIG, ARG_NONE)` or
  `(TYPE_COMMAND, ARG_START)` or `(TYPE_COMMAND, ARG_STOP)` adapter
  event to its Framework event ID just as it does in Phase 12I.
- The FSM then scans the transition table FIFO, finds no row with
  `current_state == STATE_FAULT && event_id == EVT_CONFIGURED/START/STOP`,
  increments `fsm.no_transition_count`, and leaves state unchanged.
- Bridge counters: `event_count++`, `mapped_count++`. Bridge
  `unmapped_count` is NOT incremented (the row matched at the bridge
  level even though the FSM had no transition row).
- Return: `ROBOTOS_CORE_OK` (Phase 12F semantics: a mapped dispatch
  whose FSM had no matching transition row still returns OK).

This yields **sticky FAULT** for normal command/config events
without needing explicit sticky-FAULT rows. RESET is the only event
that exits FAULT (row 9).

### 5.2 Behavior of `IDLE + RESET` self-loop (row 5)

Phase 12I left RESET-from-IDLE as a no-transition (FSM
`no_transition_count++`, state unchanged). Phase 12J admits row 5
so that RESET from any state is now a *committed transition* (state
update runs even though current==next; `fsm.transition_count++`).

**Backwards-compatibility check:** Phase 12I tests TC01..TC15 never
issue RESET from IDLE, so adding row 5 does not change any TC01..TC15
counter value. All Phase 12I assertions remain green.

### 5.3 IDLE + FAULT (row 6) edge case

`STATE_IDLE + EVT_FAULT → STATE_FAULT` is shipped intentionally:
treating FAULT-from-IDLE as a valid transition keeps the FAULT block
symmetric. There is no "must come from READY/ACTIVE" precondition
in the host prototype; production sources of FAULT may fire at any
time.

---

## 6. Bridge Mapping Table Plan

Phase 12J mapping table has **5 rows** (Phase 12I 4 + 1 new).

| Row | adapter_type | adapter_arg0 | match_arg0 | fw_event_id | Source | Status |
|---|---|---|---|---|---|---|
| 0 | `TYPE_CONFIG` | `ARG_NONE` | `true` | `EVT_CONFIGURED` | Phase 12I | unchanged |
| 1 | `TYPE_COMMAND` | `ARG_START` | `true` | `EVT_START` | Phase 12I | unchanged |
| 2 | `TYPE_COMMAND` | `ARG_STOP` | `true` | `EVT_STOP` | Phase 12I | unchanged |
| 3 | `TYPE_COMMAND` | `ARG_RESET` | `true` | `EVT_RESET` | Phase 12I | unchanged |
| 4 | `TYPE_FAULT` | `ARG_ANY` | `false` | `EVT_FAULT` | Phase 12J | **NEW** — wildcard |

`row_count` bumps from `4u` to `5u` in `probe_translator_init`.

### 6.1 Wildcard placement and precedence

Row 4 is placed last. FIFO first-match precedence still works
correctly because:

- Rows 0-3 use distinct `(adapter_type, adapter_arg0)` pairs from
  row 4 — no Phase 12I event tuple can ever match row 4.
- Row 4 only matches when `adapter_type == TYPE_FAULT (3u)`, which
  rows 0-3 do not.
- No other type=3 row exists.

Therefore the wildcard does not shadow any specific row, and
placement at index 4 (last) is safe.

### 6.2 Bridge ABI impact

**None.** The wildcard row uses an existing Phase 12F feature
(`match_arg0 == false`). The bridge `.c` and `.h` files are
**zero-diff** at Phase 12J. The bridge contract test
(`robotos_fw_event_bridge_contract`) already exercises wildcard
semantics and remains unchanged. No bridge ABI / memory-layout lock
is triggered.

### 6.3 `unmapped_count` semantics

For an adapter event whose `(adapter_type, adapter_arg0)` matches
*no* row (including the wildcard row, when the type is not
`TYPE_FAULT`):

- Bridge: `event_count++`, `unmapped_count++`, FSM not called,
  return `OK`. (Unchanged Phase 12F behavior.)

For an adapter event whose adapter_type is `TYPE_FAULT` (any arg0):

- Bridge: row 4 matches, `event_count++`, `mapped_count++`, FSM
  called with `EVT_FAULT`, return is the FSM's return.

For an adapter event mapped by rows 0-3 (CONFIG/COMMAND set):

- Same as Phase 12I.

This is symmetric with Phase 12I; only the *set* of mapped events
grows.

---

## 7. Header Diff Plan

`probe_translator.h` Phase 12J diff is **additive only**.

### 7.1 State-IDs block

Replace:
```c
/* PROBE_TRANSLATOR_STATE_FAULT = 4u — RESERVED; DEFERRED at Phase 12I. */
```
with:
```c
#define PROBE_TRANSLATOR_STATE_FAULT  ((robotos_fw_state_id_t)4u)
```

### 7.2 Event-IDs block

Replace:
```c
/* PROBE_TRANSLATOR_EVT_FAULT = 5u — RESERVED; DEFERRED at Phase 12I. */
```
with:
```c
#define PROBE_TRANSLATOR_EVT_FAULT  ((robotos_fw_event_id_t)5u)
```

### 7.3 Adapter-type block

Replace:
```c
/* PROBE_ADAPTER_TYPE_FAULT = 3u — RESERVED; DEFERRED at Phase 12I. */
```
with:
```c
#define PROBE_ADAPTER_TYPE_FAULT  ((uint32_t)3u)
```

### 7.4 Adapter-arg block

Replace:
```c
/* PROBE_ADAPTER_ARG_ANY — OMITTED; no wildcard row at Phase 12I. */
```
with:
```c
#define PROBE_ADAPTER_ARG_ANY  ((uint32_t)0xFFFFFFFFu)
```

### 7.5 Public API surface

**Zero change.** The four public functions retain their Phase 12I
signatures. The struct layout `probe_translator_t` is **zero-diff**.

---

## 8. Source Diff Plan

`probe_translator.c` Phase 12J diff is **additive only** to the three
static const tables and the three count literals in
`probe_translator_init`. Public API bodies are unchanged.

### 8.1 `k_probe_translator_transitions[]` — append 5 rows

Append after the existing row 4:

```c
    /* row 5: IDLE + RESET -> IDLE (self-loop; Phase 12J) */
    { PROBE_TRANSLATOR_STATE_IDLE,   PROBE_TRANSLATOR_EVT_RESET,
      NULL, PROBE_TRANSLATOR_STATE_IDLE,  NULL },
    /* row 6: IDLE + FAULT -> FAULT (Phase 12J) */
    { PROBE_TRANSLATOR_STATE_IDLE,   PROBE_TRANSLATOR_EVT_FAULT,
      NULL, PROBE_TRANSLATOR_STATE_FAULT, NULL },
    /* row 7: READY + FAULT -> FAULT (Phase 12J) */
    { PROBE_TRANSLATOR_STATE_READY,  PROBE_TRANSLATOR_EVT_FAULT,
      NULL, PROBE_TRANSLATOR_STATE_FAULT, NULL },
    /* row 8: ACTIVE + FAULT -> FAULT (Phase 12J) */
    { PROBE_TRANSLATOR_STATE_ACTIVE, PROBE_TRANSLATOR_EVT_FAULT,
      NULL, PROBE_TRANSLATOR_STATE_FAULT, NULL },
    /* row 9: FAULT + RESET -> IDLE (Phase 12J) */
    { PROBE_TRANSLATOR_STATE_FAULT,  PROBE_TRANSLATOR_EVT_RESET,
      NULL, PROBE_TRANSLATOR_STATE_IDLE,  NULL },
```

### 8.2 `k_probe_translator_state_defs[]` — append 1 entry

```c
    { PROBE_TRANSLATOR_STATE_FAULT,  NULL, NULL },
```

### 8.3 `k_probe_translator_mapping[]` — append 1 row

```c
    /* row 4: (TYPE_FAULT, *) -> EVT_FAULT (wildcard; Phase 12J) */
    { PROBE_ADAPTER_TYPE_FAULT, PROBE_ADAPTER_ARG_ANY, false,
      PROBE_TRANSLATOR_EVT_FAULT },
```

### 8.4 `probe_translator_init` — count literals

```c
    pt->_fsm_cfg.transition_count = 10u;  /* was 5u */
    pt->_fsm_cfg.state_count      = 4u;   /* was 3u */
    ...
    pt->_bridge_cfg.row_count     = 5u;   /* was 4u */
```

No other change to `init`. The user-context, initial-state, FSM
pointer, and bridge wiring are unchanged.

### 8.5 Other public functions

`probe_translator_dispatch_adapter_event`, `probe_translator_reset`,
and `probe_translator_get_snapshot` are **zero-diff** at Phase 12J.

---

## 9. Host Test Contract

Test file: `RobotOS_v1.0/tests/host/test_app_probe_translator_mapping.c`

**TC01..TC15:** retained verbatim. All Phase 12I in-binary assertions
remain green at Phase 12J close. (See §11 below for verification
reasoning.)

**TC16..TC24:** new at Phase 12J. ~30-40 in-binary assertions
expected.

| # | Test name | Key assertions |
|---|---|---|
| TC16 | `fault_from_idle` | After reset, dispatch `(TYPE_FAULT, ARG_ANY, NULL)`; snapshot shows `STATE_FAULT`; `fsm.transition_count == 1`; `bridge.mapped_count == 1`; `bridge.unmapped_count == 0`; `bridge.last_fw_event_id == EVT_FAULT`. |
| TC17 | `fault_from_ready` | Drive IDLE→READY via CONFIG; dispatch `(TYPE_FAULT, 0xDEADBEEF, NULL)` (arbitrary arg0 to exercise wildcard); snapshot shows `STATE_FAULT`; `bridge.mapped_count == 2` (config + fault); `bridge.last_adapter_arg0 == 0xDEADBEEF`. |
| TC18 | `fault_from_active` | Drive IDLE→READY→ACTIVE; dispatch `(TYPE_FAULT, 0u, NULL)`; snapshot shows `STATE_FAULT`. |
| TC19 | `fault_wildcard_arg0_variants` | After reset, dispatch `(TYPE_FAULT, 0u, NULL)` then `(TYPE_FAULT, 0xFFFFFFFFu, NULL)` then `(TYPE_FAULT, 42u, NULL)`; each transitions IDLE→FAULT or stays in FAULT; mapped_count increments by 3; unmapped_count stays 0. |
| TC20 | `reset_from_fault_to_idle` | Drive any path to FAULT; dispatch `(TYPE_COMMAND, ARG_RESET, NULL)`; snapshot shows `STATE_IDLE`; FAULT-exit transition counted. |
| TC21 | `fault_sticky_for_normal_events` | In FAULT, dispatch CONFIG, START, STOP in sequence. After each: snapshot still shows `STATE_FAULT`; `bridge.mapped_count++` (bridge maps each); `fsm.no_transition_count++` (FSM has no row); `bridge.unmapped_count` unchanged. |
| TC22 | `idle_reset_self_loop_admitted` | After reset, dispatch `(TYPE_COMMAND, ARG_RESET, NULL)` from IDLE; snapshot shows `STATE_IDLE` (unchanged) AND `fsm.transition_count == 1` (committed, per row 5). |
| TC23 | `full_path_with_fault` | Sequence: CONFIG → START → FAULT → RESET. State trail: `IDLE → READY → ACTIVE → FAULT → IDLE`. `fsm.transition_count == 4`. `bridge.mapped_count == 4`. `bridge.unmapped_count == 0`. |
| TC24 | `snapshot_counts_with_fault_and_unmapped` | After reset, dispatch unmapped event `(0xDEADBEEF, 0xCAFEBABE, NULL)` (no match — neither rows 0-3 nor wildcard row 4 since type != TYPE_FAULT); then `(TYPE_FAULT, 0u, NULL)`. Assert `bridge.event_count == 2`, `bridge.mapped_count == 1`, `bridge.unmapped_count == 1`, state `STATE_FAULT`. |

**Existing TC12-TC14 grep gates** are unchanged in label but cover
the additional FAULT-related declarations and mapping row — the
gates still pass because Phase 12J adds no `devkit_*`, no UART
byte literal, no Zephyr / `ro_*` / `include/robotos/` include.

**TC15 regression** still expects 23/23 ctest targets.

---

## 10. Build Strategy

### 10.1 CMake change: **NONE**

The Phase 12I additive block in `tests/host/CMakeLists.txt` lists
`${APP_DIR}/probe_translator.c` as a source. Phase 12J adds new
content **inside** that same `.c` file. CMake automatically rebuilds
the target on next configure/build. No `add_executable`,
`target_include_directories`, or `add_test` change.

### 10.2 Build commands (WSL Ubuntu / gcc 13.3.0)

Reusable from Phase 12I:

```bash
rm -rf build-phase12j-host
cmake -S RobotOS_v1.0/tests/host -B build-phase12j-host
cmake --build build-phase12j-host
ctest --test-dir build-phase12j-host --output-on-failure -V
cmake --build build-phase12j-host --target save_test_log
cp RobotOS_v1.0/tests/host/logs/host_<date>.log \
   RobotOS_v1.0/tests/host/logs/phase_12J_host_<date>.log
```

### 10.3 Expected outcome

- 23 ctest targets configured / built / run.
- `probe_translator_mapping_contract` PASS with extended in-binary
  count (~100-110 assertions).
- All 22 prior targets unchanged → PASS.
- `100% tests passed, 0 tests failed out of 23`.

---

## 11. Phase 12I Regression Safety (TC01..TC15)

This section is the audit-time justification that Phase 12J's
additive changes do **not** flip any Phase 12I assertion.

| Phase 12I case | Phase 12J behavior | Safety |
|---|---|---|
| TC01 init / idle / counters zero | Init runs the same code; new state-def/transition/mapping rows are appended but not exercised | green |
| TC02 init_null_rejected | Unchanged | green |
| TC03 config_maps_idle_to_ready | Mapping row 0 unchanged; transition row 0 unchanged | green |
| TC04 start_maps_ready_to_active | Mapping row 1 unchanged; transition row 1 unchanged | green |
| TC05 stop_maps_active_to_ready | Mapping row 2 / transition row 2 unchanged | green |
| TC06a reset_maps_ready_to_idle | Mapping row 3 / transition row 3 unchanged | green |
| TC06b reset_maps_active_to_idle | Mapping row 3 / transition row 4 unchanged | green |
| TC07 unmapped_event_no_state_change | `(0xDEADBEEF, 0xCAFEBABE)` still does not match any row (rows 0-3 require specific types; row 4 requires `TYPE_FAULT==3u`, not 0xDEADBEEF). `bridge.unmapped_count` still increments. | green |
| TC08 payload_borrowed_review_validated | Action rows still all NULL | green |
| TC09 snapshot_with_counts_4 | Sequence CONFIG→START→STOP→RESET still hits rows 0/1/2/3 (RESET from READY matches row 3 before row 5 could ever fire because state is READY, not IDLE) | green |
| TC10 reset_clears_counters | `reset` body unchanged | green |
| TC11 full_path | Same as TC09 reasoning | green |
| TC12-14 grep gates | Phase 12J adds no `devkit_*` / UART / Zephyr / legacy includes | green |
| TC15 23/23 regression | Phase 12J adds no new ctest target | green |

---

## 12. Validation and Exit Criteria (Phase 12J §K — 14 gates)

| # | Gate | Evidence required |
|---|---|---|
| 1 | CMake configure succeeds | No error on `cmake -S ... -B build-phase12j-host`. |
| 2 | Build succeeds | No error on `cmake --build build-phase12j-host`. |
| 3 | `probe_translator_mapping_contract` test PASS | ctest shows PASS; in-binary assertion total in the 100-110 range; TC01..TC24 each line is PASS. |
| 4 | FSM test still PASS | `robotos_fw_fsm_contract` PASS. |
| 5 | Bridge test still PASS | `robotos_fw_event_bridge_contract` PASS. |
| 6 | Full host regression PASS | 23/23 (`100% tests passed, 0 tests failed out of 23`). |
| 7 | Host log saved | `tests/host/logs/phase_12J_host_<date>.log` exists and is non-empty. |
| 8 | Grep gate: no `devkit_app_state.h` | `grep -r "devkit_app_state" app/probe_translator/` returns no match. |
| 9 | Grep gate: no `devkit_*` calls | `grep -r "devkit_" app/probe_translator/` returns no match. |
| 10 | Grep gate: no UART command bytes | No `a/s/r/?/x/v/L/d/T` byte literal in `app/probe_translator/` or the new test. |
| 11 | Grep gate: no Zephyr / legacy includes | No `<zephyr/...>` / `ro_*` / `include/robotos/` include. |
| 12 | Command set unchanged | `devkit/src/` zero-diff; no new byte in UART handler. |
| 13 | `devkit_app_state` zero-diff | `git diff HEAD -- devkit/src/devkit_app_state.{c,h}` = empty. |
| 14 | No hardware run | No RTT / J-Link / OpenOCD session; no new board DTS or `prj.conf`. |

**Plus the Phase-12J-specific count-bump check:**

`probe_translator_init` after Phase 12J must set:
- `pt->_fsm_cfg.transition_count == 10u`
- `pt->_fsm_cfg.state_count == 4u`
- `pt->_bridge_cfg.row_count == 5u`

A snapshot read in TC23 (`full_path_with_fault`) implicitly proves
these are correct because the FAULT row and the FAULT mapping row
must be reachable.

---

## 13. `devkit_app_state` Boundary

Unchanged from Phase 12I:

- `devkit_app_state` remains authoritative for the devkit runtime.
- `app/probe_translator/` must not include `devkit_app_state.h`,
  call any `devkit_*` function, or read/write `devkit_app_state`
  snapshots.
- The new `STATE_FAULT` symbol is **application-local**. Name
  overlap with any `DEVKIT_APP_STATE_*` symbol is coincidental at
  the human-readable level only.
- No `?` UART response change. No `a/s/r/?/x/v/L/d/T` semantic
  change.
- Scope-guard #11 remains active.

---

## 14. Command Set Boundary

Unchanged from Phase 12I:

- `a / s / r / ? / x / v / L / d / T` remain devkit / probe surface.
- `probe_translator/` defines no UART command.
- Framework bridge has no UART surface (Phase 12F locked).
- All 12 UART TX scope-guard constraints from Phase 9EZ §H preserved.

---

## 15. Legacy Architecture B Boundary

Unchanged from Phase 12I:

- `src/` and `include/robotos/` frozen at
  `LEGACY_SCAFFOLD_MARKED_FROZEN_DOCS_ONLY` (Phase 12D-pre).
- `app/probe_translator/` uses Architecture A contracts only.
- No `ro_*` HAL include.

---

## 16. Open Decisions (after Phase 12J)

| # | Open question | Latest at |
|---|---|---|
| 1 | Non-NULL action callbacks (e.g., fault logging via action) | Future app-behavior phase |
| 2 | on_entry / on_exit callbacks for FAULT (e.g., latch fault cause via user_context) | Future app-behavior phase |
| 3 | Multi-source FAULT cause encoding (currently any TYPE_FAULT event is a single EVT_FAULT) | Future app-behavior phase |
| 4 | Whether `probe_translator/` ever runs as a hardware-runnable Zephyr application | Future runtime-integration phase |
| 5 | `RobotOS_v1.0/examples/` for sample integrations | Future docs-only phase |
| 6 | Bridge ABI memory-layout lock (still open from Phase 12G §11 #9) | Future phase |
| 7 | Devkit integration of `probe_translator` (separate planning phase) | NOT_STARTED |
| 8 | Multi-product coordination (second `app/<product>/`) | NOT_STARTED |

---

## 17. Next Revision Conditions

1. **Phase 12J opens (FAULT block implementation).** §1 status
   becomes `IMPLEMENTED_AT_12J (HOST-TEST EVIDENCE)`. §2 updates
   with materialized diff list. §9 / §12 cross-reference the
   evidence log.
2. **A future behavior phase ships non-NULL FAULT actions or
   on_entry FAULT-cause latching.** §3 / §5 / §9 gain rows / cases.
3. **A user override picks HOLD.** §1 records the hold; no further
   revision required until a future phase opens.

This spec is **NOT** revised to record devkit implementation
details, track per-phase test results, change command semantics,
promote `devkit_app_state`, or reactivate Architecture B.
