# RobotOS — Probe Translator Zephyr Build-Only Admission Plan

**Status:** `BUILD_ADMITTED_AT_12K (ZEPHYR-BUILD EVIDENCE)`.
This document is the execution-ready implementation contract for
**Phase 12K — Probe Translator Zephyr Build-Only Admission**.
**Revision:** Phase 12K (2026-05-14, `CLOSED_WITH_BUILD_EVIDENCE`;
`PHASE_12K_PROBE_TRANSLATOR_ZEPHYR_BUILD_ONLY_ADMITTED_VALIDATED`) —
Zephyr build admission landed via additive change to
`devkit/CMakeLists.txt`; `west build --pristine` PASS for
`stm32f411e_disco` rev D (FLASH 41,528 B / 7.92%, RAM 12,352 B / 9.42%);
host regression 23/23 PASS; transcript at
`devkit/logs/phase_12K_build_2026-05-14.txt`.
**Previous revision:** Phase 12K-pre (2026-05-14, `CLOSED_DOCS_ONLY`).

> **Zephyr build admission landed at Phase 12K.** `devkit/CMakeLists.txt`
> now lists `framework/robotos_fw_fsm.c`,
> `framework/robotos_fw_event_bridge.c`, and
> `app/probe_translator/probe_translator.c` as sources, with
> `../framework` and `../app/probe_translator` on the include path. No
> runtime wiring, no `devkit_app_state` change, no UART command change,
> no `prj.conf` / DTS / overlay / Kconfig change.

This spec is anchored to
[`../02_PHASE_CLOSEOUTS/PHASE_12K_PRE_PROBE_TRANSLATOR_ZEPHYR_BUILD_ONLY_PLAN.md`](../02_PHASE_CLOSEOUTS/PHASE_12K_PRE_PROBE_TRANSLATOR_ZEPHYR_BUILD_ONLY_PLAN.md)
and extends
[`PROBE_TRANSLATOR_FAULT_BLOCK_PLAN.md`](PROBE_TRANSLATOR_FAULT_BLOCK_PLAN.md)
(Phase 12J implementation contract, `IMPLEMENTED_AT_12J`).

---

## 1. Status / Scope

**What this doc is:**
The complete execution-ready implementation contract for Phase 12K
Zephyr build-only admission. All CMake diff, include path requirements,
dependency audit rules, validation commands, and exit criteria are
locked here.

**What this doc is not:**
- A devkit runtime integration spec. `probe_translator` is NOT wired to
  `devkit_app_state`, UART commands, or any runtime control flow.
- A hardware validation spec. No RTT, no flash, no J-Link.
- A command-vocabulary spec. The frozen `a/s/r/?/x/v/L/d/T` devkit
  command set is unchanged.
- A product integration spec.

---

## 2. Phase 12K Approved File Set

Phase 12K may **only** modify or create the files listed here.

### 2.1 Modified existing files (additive only)

| Path | Change |
| --- | --- |
| `RobotOS_v1.0/devkit/CMakeLists.txt` | Additive: append `../framework/robotos_fw_fsm.c`, `../framework/robotos_fw_event_bridge.c`, `../app/probe_translator/probe_translator.c` to `target_sources(app PRIVATE ...)`; append `../framework`, `../app/probe_translator` to `target_include_directories(app PRIVATE ...)`. |

### 2.2 New files

| Path | Purpose |
| --- | --- |
| `RobotOS_v1.0/devkit/logs/phase_12K_build_<YYYY-MM-DD>.txt` | Committed build evidence (west build output; FLASH/RAM delta). |
| `RobotOS_v1.0/devkit/docs/02_PHASE_CLOSEOUTS/PHASE_12K_PROBE_TRANSLATOR_ZEPHYR_BUILD_ONLY.md` | Phase 12K closeout. |

### 2.3 Doc-sync at Phase 12K close

| Path | Change |
| --- | --- |
| `RobotOS_v1.0/devkit/docs/03_SPECS/PROBE_TRANSLATOR_ZEPHYR_BUILD_ONLY_PLAN.md` | Status upgrade `DRAFT / NON-FINAL` → `BUILD_ADMITTED_AT_12K (ZEPHYR-BUILD EVIDENCE)`. |
| `RobotOS_v1.0/devkit/docs/01_PROGRESS/DEVKIT_PROGRESS_PHASE_11_20.md` | Phase 12K index row + section. |
| `CURRENT_STATE.md` | Phase 12K as latest closed. |
| `RobotOS_v1.0/devkit/docs/00_INDEX/README.md` | Phase 12K closeout link. |

### 2.4 Forbidden at Phase 12K

| Forbidden path | Reason |
| --- | --- |
| `RobotOS_v1.0/app/probe_translator/CMakeLists.txt` | Not needed for Option A (devkit CMake additive). Deferred. |
| `RobotOS_v1.0/app/probe_translator/Kconfig` | No Kconfig dependency in `probe_translator`. |
| `RobotOS_v1.0/devkit/prj.conf` | No new Kconfig flags required. **Zero-diff at Phase 12K.** |
| Any DTS / overlay file | No hardware dependency. **Zero-diff at Phase 12K.** |
| `RobotOS_v1.0/devkit/src/devkit_app_state.*` | Devkit runtime boundary. **Zero-diff at Phase 12K.** |
| Any `RobotOS_v1.0/devkit/src/*.c` other than the CMake change | No runtime wiring. |
| Any `RobotOS_v1.0/framework/*.h` or `framework/*.c` | Framework source is compiled as-is. Zero header diff. |
| Any `RobotOS_v1.0/core/*` | Zero-diff. |
| Any `RobotOS_v1.0/platform/*` | Zero-diff. |
| Any `RobotOS_v1.0/tests/host/*` | Host regression rerun only; no new test target. |
| Any UART command byte / command handler | `a/s/r/?/x/v/L/d/T` frozen surface. |
| Hardware evidence / RTT log | No hardware run. |

---

## 3. Existing Build Topology (confirmed at Phase 12K-pre)

### 3.1 Active Zephyr build

File: `RobotOS_v1.0/devkit/CMakeLists.txt`
Project: `robotos_devkit`
Board target: `stm32f411e_disco`

Current include paths (partial):
```cmake
target_include_directories(app PRIVATE
    ../core
    ../platform
)
```

Current sources (partial):
```cmake
target_sources(app PRIVATE
    src/main.c
    src/devkit_*.c
    ../core/robotos_core.c
    ../core/robotos_event_queue.c
    ../core/robotos_event_dispatcher.c
    ../platform/zephyr/robotos_platform_critical_zephyr.c
    ../platform/zephyr/robotos_platform_log_zephyr.c
    ../platform/zephyr/robotos_platform_fault_zephyr.c
    ../platform/zephyr/robotos_platform_time_zephyr.c
)
```

### 3.2 What is NOT currently in the devkit Zephyr build

- `framework/robotos_fw_fsm.c` — **absent**
- `framework/robotos_fw_event_bridge.c` — **absent**
- `app/probe_translator/probe_translator.c` — **absent**
- No `../framework` include path
- No `../app/probe_translator` include path

### 3.3 Platform critical section (already satisfied)

`framework/robotos_fw_fsm.c` calls `robotos_platform_critical_enter/exit`,
declared in `platform/robotos_platform_critical.h`. The Zephyr
implementation `platform/zephyr/robotos_platform_critical_zephyr.c` is
already in the devkit build. No additional platform file is needed.

### 3.4 `prj.conf` (current, unchanged at Phase 12K)

`RobotOS_v1.0/devkit/prj.conf` sets LOG, RTT, GPIO, SERIAL, SENSOR, I2C
Kconfig flags. None of these are required by `probe_translator` or the
framework FSM/bridge. **`prj.conf` is zero-diff at Phase 12K.**

### 3.5 DTS / overlay (none; zero-diff at Phase 12K)

No DTS overlay files exist in `devkit/`. `probe_translator` has no hardware
dependency. **DTS / overlay are zero-diff at Phase 12K.**

---

## 4. Phase 12K CMake Diff Plan

### 4.1 Strategy: Option A — additive entry in `devkit/CMakeLists.txt`

Append after existing `target_sources(app PRIVATE ...)` block:

```cmake
# Architecture-A Framework (Phase 12K build admission)
target_sources(app PRIVATE
    ../framework/robotos_fw_fsm.c
    ../framework/robotos_fw_event_bridge.c
    ../app/probe_translator/probe_translator.c
)

target_include_directories(app PRIVATE
    ../framework
    ../app/probe_translator
)
```

### 4.2 Rejected strategies

| Strategy | Reason rejected |
| --- | --- |
| New `app/probe_translator/CMakeLists.txt` | Requires touching devkit CMake anyway (`add_subdirectory`). No isolation gain at build-only depth. Deferred to devkit integration phase if needed. |
| Root `RobotOS_v1.0/CMakeLists.txt` | Root CMake is Architecture-B scaffold (frozen). Not the active devkit build path. |
| New standalone west application | Over-engineered for build-only admission. Would require separate `prj.conf`, west manifest update. Not necessary. |
| `app/probe_translator/Kconfig` | No Kconfig flag required. Creating a Kconfig guard for no real reason adds complexity. |

### 4.3 Why `prj.conf` is unchanged

`probe_translator.c` uses only `<stdint.h>`, `robotos_fw_fsm.h`, and
`robotos_fw_event_bridge.h`. None of these introduce new Kconfig
requirements. No new `CONFIG_*` flag is needed. `prj.conf` remains
zero-diff at Phase 12K.

---

## 5. Dependency Audit Rules

Phase 12K must confirm all of the following before closeout:

| # | Gate | Check method |
| --- | --- | --- |
| 1 | No `<zephyr/...>` include in `app/probe_translator/` | `grep -r "#include.*zephyr" app/probe_translator/` returns no match |
| 2 | No `devkit_*.h` or `devkit_app_state.h` include | `grep -r "devkit_" app/probe_translator/` returns no match in actual includes |
| 3 | No UART command byte literal | No `a/s/r/?/x/v/L/d/T` byte literal in `app/probe_translator/` |
| 4 | No heap dependency | No `malloc`, `free`, `calloc` in `probe_translator.c` or framework files |
| 5 | No thread/scheduler dependency | No `k_sleep`, `k_thread_create`, `k_sem_*`, or OS scheduler calls |
| 6 | No hardware HAL dependency | No `gpio_pin_*`, `uart_*`, `i2c_*`, `sensor_*` calls |
| 7 | Framework include paths visible | `robotos_fw_fsm.h` and `robotos_fw_event_bridge.h` reachable via `../framework` |
| 8 | Core include paths visible | `robotos_core_status_t` reachable via `../core` (already in devkit build) |
| 9 | Platform critical boundary satisfied | `robotos_platform_critical_zephyr.c` already in build — confirmed by Phase 12K-pre audit |
| 10 | No duplicate symbol | Neither `robotos_fw_fsm.c` nor `robotos_fw_event_bridge.c` is in the current devkit CMake — confirmed by Phase 12K-pre audit |

---

## 6. Validation Gates for Phase 12K (10 gates)

| # | Gate | Command / Evidence |
| --- | --- | --- |
| 1 | CMake diff scope | `git diff -- devkit/CMakeLists.txt` — exactly the additive block in §4.1 |
| 2 | `prj.conf` zero-diff | `git diff -- devkit/prj.conf` — empty |
| 3 | DTS/overlay zero-diff | `git diff -- "*.overlay" "*.dts" "*.dtsi"` — empty |
| 4 | `devkit/src/` zero-diff | `git diff -- devkit/src/` — empty |
| 5 | `framework/` zero-diff | `git diff -- framework/` — empty |
| 6 | `core/ platform/` zero-diff | `git diff -- core/ platform/` — empty |
| 7 | `probe_translator.{h,c}` zero-diff | `git diff -- app/probe_translator/probe_translator.*` — empty |
| 8 | Zephyr build PASS | `west build --pristine=always -d build-phase12k -b stm32f411e_disco RobotOS_v1.0/devkit` — no errors, 0 warnings beyond pre-existing baseline |
| 9 | Host regression PASS | `cmake -S RobotOS_v1.0/tests/host -B build-phase12k-host && cmake --build build-phase12k-host && ctest --test-dir build-phase12k-host --output-on-failure` — 23/23 PASS |
| 10 | Dependency grep gates | All 10 checks in §5 return no forbidden matches |

Build is **proof of compile/link admission only**. No flash. No RTT. No hardware.

---

## 7. Evidence Policy

### 7.1 Evidence log to commit

```
RobotOS_v1.0/devkit/logs/phase_12K_build_<YYYY-MM-DD>.txt
```

Content: full `west build --pristine` transcript including FLASH/RAM
summary. Follow existing naming convention from Phase 11D/11E:
`phase_11D_accel_probe_host_2026-05-12.txt`.

### 7.2 Host regression log

If Phase 12K reruns the host ctest, save the result alongside the build
log or as `tests/host/logs/phase_12K_host_<date>.log`. This is optional
but recommended for full regression proof.

### 7.3 No hardware evidence

No RTT log. No flash. No hardware log of any kind.

---

## 8. Closeout Criteria for Phase 12K

Phase 12K closeout must report:

1. **Files changed:** `devkit/CMakeLists.txt` (additive only); evidence log (new).
2. **CMake diff:** exact appended `target_sources` / `target_include_directories` block.
3. **`prj.conf` decision:** zero-diff confirmed.
4. **DTS/overlay decision:** zero-diff confirmed; no hardware dependency.
5. **Zephyr build result:** `west build --pristine` exit code 0; FLASH/RAM delta reported.
6. **Host regression result:** 23/23 PASS (or evidence that prior PASS still holds).
7. **No runtime wiring confirmed:** `devkit_app_state` not called; no UART command added.
8. **No UART command semantic change:** `a/s/r/?/x/v/L/d/T` frozen.
9. **No hardware behavior:** no flash, no RTT, no board run.
10. **Boundaries preserved:** all 10 dependency audit gates PASS.

---

## 9. Risk and Rollback Signals

| Risk | Signal | Rollback |
| --- | --- | --- |
| Include path not visible | Build error: cannot find `robotos_fw_fsm.h` | Check `target_include_directories` relative path from `devkit/` |
| Framework source missing from link | Linker error: undefined reference to `robotos_fw_fsm_*` | Confirm `.c` paths in `target_sources` are correct relative to `devkit/` |
| Duplicate symbol | Linker error: multiple definition | Check if framework files were already added by another path (unlikely — confirmed absent at Phase 12K-pre) |
| Kconfig overreach | New `CONFIG_*` flag required for compile | Stop and report `PHASE_12K_BLOCKED_KCONFIG_REQUIRED`; plan Kconfig gate first |
| `prj.conf` mutation | `git diff -- devkit/prj.conf` non-empty | Revert and stop |
| Accidental runtime coupling | `probe_translator_init` called from `main.c` or `devkit_app_state.c` | Hard stop — not an implementation phase; revert `devkit/src/` changes |
| Accidental DTS change | `git diff -- "*.overlay"` non-empty | Revert and stop |
| Host regression breaks | ctest < 23/23 | Do not close Phase 12K; diagnose first |
| FLASH/RAM delta unacceptable | West build reports FLASH > threshold | Report delta; user decides if acceptable |

---

## 10. Open Decisions (after Phase 12K)

| # | Open question | Status |
| --- | --- | --- |
| 1 | Whether to create `app/probe_translator/CMakeLists.txt` for app-module isolation | Deferred to devkit integration planning phase |
| 2 | Whether to create `app/probe_translator/Kconfig` | Deferred (no current dependency) |
| 3 | Devkit runtime integration of `probe_translator` | NOT_STARTED; separate planning gate required |
| 4 | Hardware-runnable Zephyr application with `probe_translator` | NOT_STARTED; depends on devkit integration |
| 5 | Bridge ABI memory-layout lock | NOT_STARTED |
| 6 | Non-NULL action / on_entry / on_exit for FAULT | NOT_STARTED |
| 7 | Product command mapping | NOT_STARTED; USER_DECISION_REQUIRED |
| 8 | Multi-product coordination | NOT_STARTED |
