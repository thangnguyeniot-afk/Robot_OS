# RobotOS — Probe Translator Runtime Admission Plan

**Status:** `IMPLEMENTED_AT_12L (ZEPHYR-BUILD EVIDENCE)`
**Spec type:** Long-lived implementation contract for runtime
admission of `probe_translator` into the devkit runtime.
**Revision:** Phase 12L (2026-05-14, `CLOSED_WITH_BUILD_EVIDENCE`;
`PHASE_12L_PROBE_TRANSLATOR_RUNTIME_ADMISSION_ADAPTER_IMPLEMENTED_VALIDATED`) —
runtime admission landed via new devkit-local adapter
`devkit/src/devkit_probe_adapter.{c,h}`; `west build --pristine` PASS for
`stm32f411e_disco` rev D (FLASH 43,384 B / 8.27%, RAM 12,480 B / 9.52%);
host regression 23/23 PASS; transcript at
`devkit/logs/phase_12L_build_2026-05-14.txt`.
**Previous revision:** Phase 12L-pre (2026-05-14, `CLOSED_DOCS_ONLY`).
**Planning closeout anchor:**
[`../02_PHASE_CLOSEOUTS/PHASE_12L_PRE_PROBE_TRANSLATOR_RUNTIME_ADMISSION_PLAN.md`](../02_PHASE_CLOSEOUTS/PHASE_12L_PRE_PROBE_TRANSLATOR_RUNTIME_ADMISSION_PLAN.md)
**Implementation closeout anchor:**
[`../02_PHASE_CLOSEOUTS/PHASE_12L_PROBE_TRANSLATOR_RUNTIME_ADMISSION_ADAPTER.md`](../02_PHASE_CLOSEOUTS/PHASE_12L_PROBE_TRANSLATOR_RUNTIME_ADMISSION_ADAPTER.md)
**Prior build-admission spec:**
[`PROBE_TRANSLATOR_ZEPHYR_BUILD_ONLY_PLAN.md`](PROBE_TRANSLATOR_ZEPHYR_BUILD_ONLY_PLAN.md)

---

## 1. Status / Scope

**What this doc is:**
The execution-ready implementation contract for Phase 12L —
Probe Translator Runtime Admission. All module boundaries, command
mapping, integration points, file set, dependency rules, validation
gates, and rollback signals are locked here.

**What this doc is not:**
- A UART command expansion spec. The command set `a/s/r/?/x/v/L/d/T`
  is frozen at Phase 12L.
- A hardware validation spec. No RTT, no flash, no J-Link.
- A UART TX response spec. No new UART TX encoding.
- A FAULT source spec. The FAULT adapter type is not wired at Phase 12L.
- A product command mapping spec. Product command mapping requires a
  separate user decision.

---

## 2. Phase 12L Approved File Set

Phase 12L may **only** modify or create the files listed here.

### 2.1 New files (required)

| Path | Purpose |
| --- | --- |
| `RobotOS_v1.0/devkit/src/devkit_probe_adapter.h` | Adapter public API (3 functions). Devkit-local only. |
| `RobotOS_v1.0/devkit/src/devkit_probe_adapter.c` | Adapter implementation. Owns static `probe_translator_t` instance. |
| `RobotOS_v1.0/devkit/logs/phase_12L_build_<YYYY-MM-DD>.txt` | Committed west build transcript. |
| `RobotOS_v1.0/devkit/docs/02_PHASE_CLOSEOUTS/PHASE_12L_PROBE_TRANSLATOR_RUNTIME_ADMISSION.md` | Phase 12L closeout. |

### 2.2 Modified existing files

| Path | Change |
| --- | --- |
| `RobotOS_v1.0/devkit/CMakeLists.txt` | Add `src/devkit_probe_adapter.c` to existing `target_sources` block |
| `RobotOS_v1.0/devkit/src/devkit_runtime.c` | Add `#include "devkit_probe_adapter.h"`; add `devkit_probe_adapter_init()` call + periodic log call |
| `RobotOS_v1.0/devkit/src/devkit_app_state.c` | Add `#include "devkit_probe_adapter.h"`; add `devkit_probe_adapter_dispatch()` calls at 'a'/'s'/'r'/'d' and button transitions |

### 2.3 Doc-sync at Phase 12L close

| Path | Change |
| --- | --- |
| `RobotOS_v1.0/devkit/docs/03_SPECS/PROBE_TRANSLATOR_RUNTIME_ADMISSION_PLAN.md` | Status upgrade to `IMPLEMENTED_AT_12L (ZEPHYR-BUILD EVIDENCE)` |
| `RobotOS_v1.0/devkit/docs/01_PROGRESS/DEVKIT_PROGRESS_PHASE_11_20.md` | Phase 12L index row + section |
| `CURRENT_STATE.md` | Phase 12L as latest closed |
| `RobotOS_v1.0/devkit/docs/00_INDEX/README.md` | Phase 12L closeout link + spec status update |

### 2.4 Forbidden at Phase 12L

| Forbidden path | Reason |
| --- | --- |
| `RobotOS_v1.0/devkit/prj.conf` | No new Kconfig dependency. **Zero-diff.** |
| Any `*.dts`, `*.dtsi`, `*.overlay` | No hardware dependency. **Zero-diff.** |
| `RobotOS_v1.0/devkit/src/devkit_uart_producer.{c,h}` | UART command/response surface frozen. **Zero-diff.** |
| `RobotOS_v1.0/devkit/src/devkit_app_state.h` | Public API unchanged. **Zero-diff** (preferred). |
| `RobotOS_v1.0/framework/*.{h,c}` | Framework used as-is. **Zero-diff.** |
| `RobotOS_v1.0/app/probe_translator/probe_translator.{h,c}` | Application used as-is. **Zero-diff.** |
| `RobotOS_v1.0/app/probe_translator/CMakeLists.txt` | Not needed. **NOT CREATED.** |
| `RobotOS_v1.0/app/probe_translator/Kconfig` | No dependency. **NOT CREATED.** |
| `RobotOS_v1.0/core/*` | **Zero-diff.** |
| `RobotOS_v1.0/platform/*` | **Zero-diff.** |
| New `robotos_core_register_event_handler()` call | Not needed; probe_translator does not own a core event registration. |
| New UART TX response | Deferred to product command mapping gate. |
| Hardware log (RTT, J-Link, flash) | Out of Phase 12L scope. **NOT PRODUCED.** |

---

## 3. `devkit_probe_adapter` Module Spec

### 3.1 Purpose

`devkit_probe_adapter` is a **devkit-local** adapter module. It owns
the single static `probe_translator_t` instance inside the devkit and
exposes a three-function interface callable by `devkit_runtime.c` and
`devkit_app_state.c`. It does NOT expose `probe_translator.h` to callers;
callers use only `devkit_probe_adapter.h`.

### 3.2 Header (`devkit_probe_adapter.h`)

```c
/*
 * devkit_probe_adapter.h
 * RobotOS devkit — probe_translator runtime adapter (Phase 12L).
 *
 * Owns the static probe_translator_t instance and bridges devkit_app_state
 * command signals to probe_translator adapter-key tuples. Devkit-local only.
 *
 * Boundaries:
 *   - No #include of probe_translator.h from this header.
 *   - No UART TX. No hardware driver. No Zephyr driver API.
 *   - No core event registration.
 *   - All entry points are thread-context only (never ISR).
 */

#ifndef DEVKIT_PROBE_ADAPTER_H
#define DEVKIT_PROBE_ADAPTER_H

#include <stdint.h>
#include "robotos_core.h"

/*
 * Initialize the probe adapter. Must be called once from devkit_runtime_init()
 * after devkit_app_state_init(). Calls probe_translator_init() internally.
 * Returns ROBOTOS_CORE_OK on success. On failure, the adapter is in an
 * uninitialized state and dispatch calls will be no-ops.
 */
robotos_core_status_t devkit_probe_adapter_init(void);

/*
 * Dispatch a synthetic adapter event to the probe translator.
 * adapter_type and adapter_arg0 are PROBE_ADAPTER_* constants.
 * Returns ROBOTOS_CORE_OK on success; non-OK is logged and not propagated
 * to the caller.
 * Thread-context only.
 */
robotos_core_status_t devkit_probe_adapter_dispatch(
    uint32_t adapter_type, uint32_t adapter_arg0);

/*
 * Emit one ROBOTOS_PROBE log line via Zephyr LOG_INF with the
 * probe_translator FSM + bridge snapshot.
 * Thread-context only.
 */
void devkit_probe_adapter_log_snapshot(void);

#endif /* DEVKIT_PROBE_ADAPTER_H */
```

### 3.3 Implementation rules (`devkit_probe_adapter.c`)

- Declares one file-static `probe_translator_t s_probe_translator` and
  one `bool s_initialized` flag.
- `devkit_probe_adapter_init()`:
  - Calls `probe_translator_init(&s_probe_translator, NULL)`.
  - On `ROBOTOS_CORE_OK`: sets `s_initialized = true`; logs
    `"ROBOTOS_PROBE init ok"`.
  - On failure: logs error with return code; `s_initialized` stays false.
  - Returns the `probe_translator_init()` status verbatim.
- `devkit_probe_adapter_dispatch(type, arg0)`:
  - If `!s_initialized`: logs warning; returns `ROBOTOS_CORE_ERR_INVALID_STATE`.
  - Calls `probe_translator_dispatch_adapter_event(&s_probe_translator, type, arg0, NULL)`.
  - On non-OK: logs with `LOG_WRN("ROBOTOS_PROBE dispatch err type=%u arg0=%u ret=%d", ...)`.
  - Returns the dispatch status verbatim.
- `devkit_probe_adapter_log_snapshot()`:
  - If `!s_initialized`: returns silently.
  - Calls `probe_translator_get_snapshot()`.
  - Logs:
    `"ROBOTOS_PROBE state=%u trans=%u events=%u no_trans=%u mapped=%u unmapped=%u"`
    where `state` = `snap.fsm.current_state`, `trans` = `snap.fsm.transition_count`,
    `events` = `snap.fsm.event_count`, `no_trans` = `snap.fsm.no_transition_count`,
    `mapped` = `snap.bridge.mapped_count`, `unmapped` = `snap.bridge.unmapped_count`.
- No heap, no dynamic allocation, no ISR-context call, no `k_sleep`, no
  `k_sem_*`, no new `robotos_core_register_event_handler()` call.
- One `#include <zephyr/logging/log.h>` and `LOG_MODULE_REGISTER(devkit_probe, LOG_LEVEL_INF)`.
- `#include "probe_translator.h"` is in `devkit_probe_adapter.c` only,
  not in `devkit_probe_adapter.h`.

---

## 4. Command Mapping (Phase 12L-pre locked)

| UART byte | `devkit_app_state` action | `devkit_probe_adapter_dispatch` call | probe_translator result |
| --- | --- | --- | --- |
| `'a'` / `'A'` | → ARMED | `(PROBE_ADAPTER_TYPE_CONFIG, PROBE_ADAPTER_ARG_NONE)` | IDLE→READY (or no-op if not IDLE) |
| `'s'` / `'S'` | → ACTIVE | `(PROBE_ADAPTER_TYPE_COMMAND, PROBE_ADAPTER_ARG_START)` | READY→ACTIVE (or no-op if not READY) |
| `'r'` / `'R'` | → IDLE | `(PROBE_ADAPTER_TYPE_COMMAND, PROBE_ADAPTER_ARG_RESET)` | any→IDLE |
| `'d'` / `'D'` (ARMED→IDLE) | ARMED→IDLE | `(PROBE_ADAPTER_TYPE_COMMAND, PROBE_ADAPTER_ARG_RESET)` | any→IDLE |
| button IDLE→ARMED | → ARMED | `(PROBE_ADAPTER_TYPE_CONFIG, PROBE_ADAPTER_ARG_NONE)` | IDLE→READY |
| button ARMED→ACTIVE | → ACTIVE | `(PROBE_ADAPTER_TYPE_COMMAND, PROBE_ADAPTER_ARG_START)` | READY→ACTIVE |
| button ACTIVE→IDLE | → IDLE | `(PROBE_ADAPTER_TYPE_COMMAND, PROBE_ADAPTER_ARG_RESET)` | any→IDLE |

**State divergence rule:** If `probe_translator` receives a dispatch that
does not match its current state (e.g., `ARG_START` while in IDLE because
's' was sent without 'a' first), the FSM increments `no_transition_count`
and returns `ROBOTOS_CORE_OK`. This is expected and correct behavior. No
error is raised. The two FSMs track separate concerns.

**FAULT adapter type:** Not wired at Phase 12L. `PROBE_ADAPTER_TYPE_FAULT`
and `PROBE_ADAPTER_ARG_ANY` are defined in `probe_translator.h` and the
mapping table handles them. A future phase will wire a hardware fault
signal source. No action required at Phase 12L.

---

## 5. Integration Rules

### 5.1 `devkit_runtime.c` changes

**In `devkit_runtime_init()`:**

```c
/* Phase 12L: initialize probe adapter after app state init. */
{
    robotos_core_status_t probe_ret = devkit_probe_adapter_init();
    if (probe_ret != ROBOTOS_CORE_OK) {
        LOG_ERR("Phase 12L probe adapter init failed: %d", (int)probe_ret);
    }
}
```

Place immediately after the `devkit_app_state_init()` call and before the
producer inits. Return value is logged and ignored (error-log-and-continue
pattern, consistent with existing Phase 6M/9A/9B producer inits).

**In `devkit_runtime_run()` observability block:**

```c
devkit_probe_adapter_log_snapshot();
```

Place inside the `if ((tick_count % DEVKIT_OBSERVABILITY_LOG_INTERVAL_TICKS) == 0u)`
block alongside the existing `devkit_app_state_log_snapshot()` call.

### 5.2 `devkit_app_state.c` changes

**Dispatch placement rules:**

- The `devkit_probe_adapter_dispatch()` call must be placed **after** the
  `devkit_app_state` transition in each case handler (the state change
  is committed first; probe dispatch is secondary).
- On no-op paths (e.g., 'a' when already ARMED), do **not** dispatch.
  Only dispatch when a state transition actually occurs.

**Specific placement:**

```c
case 'a':
    if (s_state != DEVKIT_APP_STATE_ARMED) {
        transition(DEVKIT_APP_STATE_ARMED, ...);
        devkit_probe_adapter_dispatch(PROBE_ADAPTER_TYPE_CONFIG, PROBE_ADAPTER_ARG_NONE);
    } else { ... }

case 's':
    if (s_state != DEVKIT_APP_STATE_ACTIVE) {
        transition(DEVKIT_APP_STATE_ACTIVE, ...);
        devkit_probe_adapter_dispatch(PROBE_ADAPTER_TYPE_COMMAND, PROBE_ADAPTER_ARG_START);
    } else { ... }

case 'r':
    if (s_state != DEVKIT_APP_STATE_IDLE) {
        transition(DEVKIT_APP_STATE_IDLE, ...);
        devkit_probe_adapter_dispatch(PROBE_ADAPTER_TYPE_COMMAND, PROBE_ADAPTER_ARG_RESET);
    } else { ... }

case 'd':
    if (s_state == DEVKIT_APP_STATE_ARMED) {
        transition(DEVKIT_APP_STATE_IDLE, ...);
        devkit_probe_adapter_dispatch(PROBE_ADAPTER_TYPE_COMMAND, PROBE_ADAPTER_ARG_RESET);
    } else { ... }
```

**In `devkit_app_state_on_button()`:**

```c
switch (s_state) {
case DEVKIT_APP_STATE_IDLE:
    next = DEVKIT_APP_STATE_ARMED;
    transition(next, ...);
    devkit_probe_adapter_dispatch(PROBE_ADAPTER_TYPE_CONFIG, PROBE_ADAPTER_ARG_NONE);
    break;
case DEVKIT_APP_STATE_ARMED:
    next = DEVKIT_APP_STATE_ACTIVE;
    transition(next, ...);
    devkit_probe_adapter_dispatch(PROBE_ADAPTER_TYPE_COMMAND, PROBE_ADAPTER_ARG_START);
    break;
case DEVKIT_APP_STATE_ACTIVE:
    next = DEVKIT_APP_STATE_IDLE;
    transition(next, ...);
    devkit_probe_adapter_dispatch(PROBE_ADAPTER_TYPE_COMMAND, PROBE_ADAPTER_ARG_RESET);
    break;
default:
    next = DEVKIT_APP_STATE_IDLE;
    transition(next, ...);
    break;
}
```

### 5.3 `devkit/CMakeLists.txt` change

Add to the existing devkit `target_sources` block (not the Phase 12K
Architecture-A Framework block):

```cmake
src/devkit_probe_adapter.c
```

---

## 6. Dependency Audit Rules

Phase 12L must confirm all of the following before closeout:

| # | Gate | Check method |
| --- | --- | --- |
| 1 | No `#include <zephyr/...>` in `devkit_probe_adapter.h` | `grep "#include.*zephyr" devkit/src/devkit_probe_adapter.h` — no match |
| 2 | No `#include "probe_translator.h"` in `devkit_probe_adapter.h` | Confirmed by review |
| 3 | No heap in `devkit_probe_adapter.c` | `grep "malloc\|free\|calloc" devkit/src/devkit_probe_adapter.c` — no match |
| 4 | No `robotos_core_register_event_handler` call | `grep "register_event_handler" devkit/src/devkit_probe_adapter.*` — no match |
| 5 | No scheduler dependency | `grep "k_sleep\|k_thread\|k_sem_" devkit/src/devkit_probe_adapter.*` — no match |
| 6 | No hardware HAL dependency | `grep "gpio_pin\|uart_\|i2c_\|sensor_" devkit/src/devkit_probe_adapter.*` — no match |
| 7 | `prj.conf` zero-diff | `git diff -- devkit/prj.conf` — empty |
| 8 | DTS/overlay zero-diff | `git diff -- "*.overlay" "*.dts" "*.dtsi"` — empty |
| 9 | `devkit_uart_producer.*` zero-diff | `git diff -- devkit/src/devkit_uart_producer.*` — empty |
| 10 | `framework/` zero-diff | `git diff -- framework/` — empty |
| 11 | `app/probe_translator/probe_translator.*` zero-diff | `git diff -- app/probe_translator/probe_translator.*` — empty |
| 12 | `app/probe_translator/CMakeLists.txt` not created | Confirmed by file listing |

---

## 7. Validation Gates for Phase 12L (13 gates)

| # | Gate | Command / Evidence |
| --- | --- | --- |
| 1 | CMake diff scope | `git diff -- devkit/CMakeLists.txt` — only `src/devkit_probe_adapter.c` added |
| 2 | `prj.conf` zero-diff | `git diff -- devkit/prj.conf` — empty |
| 3 | DTS/overlay zero-diff | `git diff -- "*.overlay" "*.dts" "*.dtsi"` — empty |
| 4 | `devkit_uart_producer.*` zero-diff | `git diff -- devkit/src/devkit_uart_producer.*` — empty |
| 5 | `framework/` zero-diff | `git diff -- framework/` — empty |
| 6 | `probe_translator.{h,c}` zero-diff | `git diff -- app/probe_translator/probe_translator.*` — empty |
| 7 | `app/probe_translator/CMakeLists.txt` not created | `ls app/probe_translator/CMakeLists.txt` — no such file |
| 8 | `app/probe_translator/Kconfig` not created | `ls app/probe_translator/Kconfig` — no such file |
| 9 | Zephyr build PASS | `py -m west build --pristine=always -d build-phase12l -b stm32f411e_disco RobotOS_v1.0/devkit` — exit 0; FLASH/RAM delta reported |
| 10 | Host regression PASS | `cmake -S RobotOS_v1.0/tests/host -B build-phase12l-host && cmake --build build-phase12l-host && ctest --test-dir build-phase12l-host` — **23/23 PASS** |
| 11 | Dependency grep gates | All 12 checks in §6 return no forbidden matches |
| 12 | `git diff --check` PASS | EXIT:0 |
| 13 | No runtime wiring to UART TX | `grep "devkit_probe_adapter" devkit/src/devkit_uart_producer.c` — no match |

---

## 8. Evidence Policy

### 8.1 Zephyr build evidence log

```
RobotOS_v1.0/devkit/logs/phase_12L_build_<YYYY-MM-DD>.txt
```

Content: full `west build --pristine` transcript including FLASH/RAM
summary and delta from Phase 12K baseline. Follow naming convention from
Phase 11D/11E/12K.

### 8.2 Host regression

No new log required if all 23 tests still PASS and no new host test is
added. If a new host test is added (optional), save as
`tests/host/logs/phase_12L_host_<date>.log`.

### 8.3 No hardware evidence

No RTT log. No flash. No hardware log of any kind.

---

## 9. Closeout Criteria for Phase 12L

Phase 12L closeout must report:

1. **Files changed:** `devkit_probe_adapter.{c,h}` (new); `devkit/CMakeLists.txt` (additive); `devkit_runtime.c` (additive); `devkit_app_state.c` (additive); evidence log (new).
2. **Command mapping applied:** confirm §4 command mapping was implemented exactly.
3. **`prj.conf` decision:** zero-diff confirmed.
4. **DTS/overlay decision:** zero-diff confirmed; no hardware dependency.
5. **`devkit_uart_producer.*` decision:** zero-diff confirmed.
6. **Zephyr build result:** `west build --pristine` exit code 0; FLASH/RAM delta reported.
7. **Host regression result:** 23/23 PASS (or evidence prior PASS holds unchanged).
8. **No new UART command confirmed:** `a/s/r/?/x/v/L/d/T` frozen.
9. **No UART TX change confirmed:** `devkit_uart_producer.c` zero-diff.
10. **No hardware behavior:** no flash, no RTT, no board run.
11. **Boundaries preserved:** all 12 dependency audit gates PASS.
12. **Boundary inclusion check:** `devkit_probe_adapter.h` does not include `probe_translator.h`.

---

## 10. Open Decisions (after Phase 12L)

| # | Open question | Status |
| --- | --- | --- |
| 1 | FAULT adapter type wiring (hardware fault signal source) | `NOT_STARTED` |
| 2 | UART TX response encoding for probe_translator snapshot | `NOT_STARTED; USER_DECISION_REQUIRED` |
| 3 | Non-NULL action / on_entry / on_exit callbacks | `NOT_STARTED` |
| 4 | ACTIVE disarm widening | `USER_DECISION_REQUIRED_ACTIVE_DISARM` |
| 5 | Hardware-runnable Zephyr application with probe_translator | `NOT_STARTED` (runtime-wired at 12L; not hardware-proven) |
| 6 | Bridge ABI memory-layout lock | `NOT_STARTED` |
| 7 | Product command mapping / UART expansion | `NOT_STARTED; USER_DECISION_REQUIRED` |
| 8 | `app/probe_translator/CMakeLists.txt` for app-module isolation | `DEFERRED` |
| 9 | Scheduler 7A/7B | `DEFER` |
| 10 | F407 / custom board | `HOLD/DEFER` |
| 11 | Multi-product coordination | `NOT_STARTED` |
