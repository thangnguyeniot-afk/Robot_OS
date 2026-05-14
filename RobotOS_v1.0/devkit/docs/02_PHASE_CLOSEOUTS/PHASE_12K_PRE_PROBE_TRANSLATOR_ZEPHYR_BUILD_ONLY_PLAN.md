# Phase 12K-pre — Probe Translator Zephyr Build-Only Admission Plan

**Status:** `CLOSED_DOCS_ONLY`
**Decision:** `PHASE_12K_PRE_PROBE_TRANSLATOR_ZEPHYR_BUILD_ONLY_PLAN_CLOSED`
**Date:** 2026-05-14
**Branch / baseline:** `master` at `origin/master = e935dd1` (Phase 12J-Z)
**Prior phase anchor:** [`PHASE_12JZ_CHECKPOINT.md`](PHASE_12JZ_CHECKPOINT.md)
**New long-lived spec:** [`../03_SPECS/PROBE_TRANSLATOR_ZEPHYR_BUILD_ONLY_PLAN.md`](../03_SPECS/PROBE_TRANSLATOR_ZEPHYR_BUILD_ONLY_PLAN.md)

---

## A. Executive Summary

Phase 12K-pre is a **docs-only implementation-planning gate** that defines
the execution-ready contract for **Phase 12K — Probe Translator Zephyr
Build-Only Admission**: admitting `app/probe_translator/` and the
Architecture-A Framework modules (`framework/robotos_fw_fsm.c`,
`framework/robotos_fw_event_bridge.c`) into the existing devkit Zephyr
build without any runtime wiring.

Phase 12K-pre **does**:

- audit the existing devkit Zephyr build topology (`devkit/CMakeLists.txt`
  project `robotos_devkit`) and confirm it currently excludes `framework/`
  and `app/`;
- confirm `probe_translator.h` depends only on `<stdint.h>`,
  `robotos_fw_fsm.h`, `robotos_fw_event_bridge.h` — no Zephyr, no devkit,
  no hardware;
- confirm the platform critical section boundary is already satisfied by
  `platform/zephyr/robotos_platform_critical_zephyr.c` already in the
  devkit build;
- confirm no `prj.conf`, DTS, overlay, or Kconfig change is required;
- lock the CMake admission strategy as Option A (additive entry in
  `devkit/CMakeLists.txt`);
- define the exact future CMake diff, dependency audit rules, validation
  commands, evidence policy, and closeout criteria for Phase 12K;
- freeze the implementation contract in
  [`PROBE_TRANSLATOR_ZEPHYR_BUILD_ONLY_PLAN.md`](../03_SPECS/PROBE_TRANSLATOR_ZEPHYR_BUILD_ONLY_PLAN.md);
- preserve every other open gate untouched.

Phase 12K-pre **does not**:

- modify any `.c`, `.h`, CMake, Kconfig, `prj.conf`, DTS, overlay, or
  Zephyr config file;
- admit `probe_translator` into the Zephyr build;
- wire `probe_translator` to devkit runtime or `devkit_app_state`;
- add any UART command;
- change the frozen `a / s / r / ? / x / v / L / d / T` command set;
- run Zephyr build, hardware, RTT, or J-Link;
- open Phase 12K.

---

## B. Baseline Audit

### B.1 Build topology facts (confirmed from repo)

| Fact | Status |
| --- | --- |
| Active Zephyr build entry: `devkit/CMakeLists.txt` project `robotos_devkit` | **CONFIRMED** |
| Board target: `stm32f411e_disco` | **CONFIRMED** |
| Devkit include paths: `../core`, `../platform` | **CONFIRMED** |
| `framework/robotos_fw_fsm.c` in devkit Zephyr build | **ABSENT** |
| `framework/robotos_fw_event_bridge.c` in devkit Zephyr build | **ABSENT** |
| `app/probe_translator/probe_translator.c` in devkit Zephyr build | **ABSENT** |
| `../framework` include path in devkit CMake | **ABSENT** |
| `../app/probe_translator` include path in devkit CMake | **ABSENT** |
| `platform/zephyr/robotos_platform_critical_zephyr.c` in devkit build | **PRESENT** (satisfies framework dependency) |
| `devkit/prj.conf` Kconfig flags needed by `probe_translator` | **NONE** (no new Kconfig required) |
| DTS / overlay files in `devkit/` | **NONE** (no hardware dependency in `probe_translator`) |
| `app/probe_translator/CMakeLists.txt` exists | **NO** |
| `app/probe_translator/Kconfig` exists | **NO** |
| `app/probe_translator/prj.conf` exists | **NO** |
| Architecture-B root `CMakeLists.txt` is the active Zephyr build | **NO** (frozen; devkit uses its own `devkit/CMakeLists.txt`) |
| West build convention | `west build --pristine=always -d build-phase12k -b stm32f411e_disco RobotOS_v1.0/devkit` |

### B.2 `probe_translator` dependency audit (confirmed from source)

| Dependency | Value | Zephyr-safe? |
| --- | --- | --- |
| `probe_translator.h` includes | `<stdint.h>`, `robotos_fw_fsm.h`, `robotos_fw_event_bridge.h` | **YES** |
| `probe_translator.c` includes | `probe_translator.h`, `<stddef.h>` | **YES** |
| Framework dependency | `robotos_platform_critical_*` via `robotos_fw_fsm.c` | **SATISFIED** (already in devkit build) |
| `devkit_app_state.h` or `devkit_*.h` included | **NONE** | **SAFE** |
| `<zephyr/...>` includes | **NONE** | **SAFE** |
| `include/robotos/` (`ro_*.h`) includes | **NONE** | **SAFE** |
| Heap allocation (`malloc`/`free`) | **NONE** | **SAFE** |
| OS thread / scheduler calls | **NONE** | **SAFE** |
| Hardware HAL calls | **NONE** | **SAFE** |
| C standard used | C11 | **SAFE** (devkit uses C standard via Zephyr toolchain) |
| UART command byte literals | **NONE** | **SAFE** |

### B.3 No blocking conditions found

- `PHASE_12K_PRE_BLOCKED_BUILD_TOPOLOGY_AMBIGUITY` — **NOT triggered**
  (build topology fully resolved from repo)
- `PHASE_12K_PRE_BLOCKED_BUILD_RUNTIME_COUPLING` — **NOT triggered**
  (probe_translator has zero runtime coupling to devkit or Zephyr)
- `PHASE_12K_PRE_BLOCKED_DOC_PATH_AMBIGUITY` — **NOT triggered**
  (naming follows Phase 12J-pre precedent)

---

## C. Phase 12K Contract Summary

### C.1 Build admission objective

Phase 12K proves:
- `app/probe_translator/probe_translator.c` compiles in Zephyr context
- `framework/robotos_fw_fsm.c` compiles in Zephyr context
- `framework/robotos_fw_event_bridge.c` compiles in Zephyr context
- All three link into the `robotos_devkit` firmware image without errors

Phase 12K does **not** prove:
- Runtime behavior of `probe_translator`
- Hardware or board behavior
- UART command surface
- Devkit state machine integration

### C.2 Allowed/Forbidden/Conditional file set for Phase 12K

| File | Classification | Reason |
| --- | --- | --- |
| `RobotOS_v1.0/devkit/CMakeLists.txt` | **ALLOWED (additive)** | Only existing Zephyr build entry; minimal change |
| `RobotOS_v1.0/app/probe_translator/CMakeLists.txt` | **FORBIDDEN at Phase 12K** | Not needed for Option A; deferred |
| `RobotOS_v1.0/app/probe_translator/Kconfig` | **FORBIDDEN at Phase 12K** | No Kconfig dependency |
| `RobotOS_v1.0/app/CMakeLists.txt` | **FORBIDDEN** | Does not exist; creating would require root CMake wiring |
| `RobotOS_v1.0/app/Kconfig` | **FORBIDDEN** | No app-level Kconfig at Phase 12K |
| `RobotOS_v1.0/devkit/prj.conf` | **FORBIDDEN (zero-diff)** | No new Kconfig flags required |
| DTS / overlay files | **FORBIDDEN (zero-diff)** | No hardware dependency |
| `RobotOS_v1.0/devkit/src/devkit_app_state.*` | **FORBIDDEN (zero-diff)** | Devkit runtime boundary (scope-guard #11) |
| Any `RobotOS_v1.0/devkit/src/*.c` other than CMake wiring | **FORBIDDEN** | No runtime wiring |
| `RobotOS_v1.0/framework/*.h` or `*.c` | **FORBIDDEN (content zero-diff)** | Framework compiled as-is |
| `RobotOS_v1.0/core/*` | **FORBIDDEN (zero-diff)** | Core boundary preserved |
| `RobotOS_v1.0/platform/*` | **FORBIDDEN (zero-diff)** | Platform boundary preserved |
| `RobotOS_v1.0/tests/host/*` | **CONDITIONAL** | Host regression rerun allowed (read + test only; no new test target) |
| Root `RobotOS_v1.0/CMakeLists.txt` | **FORBIDDEN** | Architecture-B scaffold (frozen) |

### C.3 CMake diff plan (Option A)

Append to `devkit/CMakeLists.txt` after existing `target_sources` block:

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

This is the **only** file change for Phase 12K source/build.

### C.4 Dependency audit rules

Phase 12K must pass all 10 dependency audit gates before closeout:

1. No `#include <zephyr/...>` in `app/probe_translator/`
2. No `#include "devkit_*.h"` in `app/probe_translator/`
3. No UART command byte literal in `app/probe_translator/`
4. No heap calls in `probe_translator.c` or framework
5. No thread/scheduler OS calls
6. No hardware HAL calls
7. `robotos_fw_fsm.h` and `robotos_fw_event_bridge.h` reachable via `../framework`
8. `robotos_core_status_t` reachable via `../core`
9. Platform critical section already in devkit build
10. No duplicate symbol (framework not in current devkit build)

### C.5 Validation gates for Phase 12K

1. `git diff -- devkit/CMakeLists.txt` — exactly the additive block in C.3
2. `git diff -- devkit/prj.conf` — **EMPTY**
3. `git diff -- "*.overlay" "*.dts" "*.dtsi"` — **EMPTY**
4. `git diff -- devkit/src/` — **EMPTY**
5. `git diff -- framework/ core/ platform/` — **EMPTY**
6. `git diff -- app/probe_translator/probe_translator.*` — **EMPTY**
7. `west build --pristine=always -d build-phase12k -b stm32f411e_disco RobotOS_v1.0/devkit` — **PASS** (0 errors)
8. Host regression 23/23 PASS (cmake + ctest in existing `tests/host/`)
9. All 10 dependency grep gates PASS
10. Build FLASH/RAM delta reported (for reference)

### C.6 Evidence policy

- **Required:** `devkit/logs/phase_12K_build_<date>.txt` — west build transcript
- **Recommended:** `tests/host/logs/phase_12K_host_<date>.log` — host regression
- **Forbidden:** RTT log, hardware log, board output

### C.7 Closeout criteria

Phase 12K must report:
1. Files changed (only `devkit/CMakeLists.txt` + evidence logs + docs)
2. Exact CMake diff
3. `prj.conf` zero-diff confirmed
4. DTS/overlay zero-diff confirmed
5. `west build` result: exit code, FLASH/RAM delta
6. Host regression: 23/23 PASS (or evidence pre-existing)
7. No runtime wiring
8. No UART command change
9. No hardware behavior
10. All 10 dependency audit gates PASS

### C.8 Risk and rollback signals

| Risk | Signal | Rollback |
| --- | --- | --- |
| Include path not visible | Build error: cannot find `robotos_fw_fsm.h` | Fix relative path in `target_include_directories` |
| Missing source link | Linker: undefined `robotos_fw_fsm_*` | Fix path in `target_sources` |
| Duplicate symbol | Linker: multiple definition | Investigate — framework should not be in current build |
| Kconfig required | New `CONFIG_*` error | Report `PHASE_12K_BLOCKED_KCONFIG_REQUIRED`; plan Kconfig gate |
| `prj.conf` mutated | `git diff` non-empty | Revert immediately |
| Runtime coupling | `main.c` or `devkit_app_state.c` modified | Hard stop; revert `devkit/src/` |
| DTS change | `git diff -- "*.overlay"` non-empty | Revert; stop |
| Host regression < 23/23 | ctest failure | Do not close; diagnose first |

---

## D. Open Gates (unchanged at Phase 12K-pre)

All gates carried forward from Phase 12J-Z:

| Gate | Status |
| --- | --- |
| ACTIVE disarm widening | `USER_DECISION_REQUIRED_ACTIVE_DISARM` |
| Scheduler 7A/7B | `DEFER` |
| F407 / custom board | `HOLD/DEFER` |
| POST_FLASH_AUTOSTART root cause | `OPEN / MITIGATED_BY_WORKFLOW` |
| Non-NULL action / on_entry / on_exit for FAULT | `OPEN (future app-behavior phase)` |
| Devkit runtime integration of `probe_translator` | `NOT_STARTED` |
| Bridge ABI memory-layout lock | `NOT_STARTED` |
| Hardware-runnable Zephyr build of `app/probe_translator/` | `NOT_STARTED` |
| Product command mapping | `NOT_STARTED; USER_DECISION_REQUIRED` |
| `RobotOS_v1.0/examples/` | `NOT_STARTED` |
| Multi-product coordination | `NOT_STARTED` |
| Zephyr build-only admission for `probe_translator` | `NOT_STARTED → opens at Phase 12K` |

---

## E. What Phase 12K-pre Does NOT Open

Phase 12K-pre does **not** open or authorize:

- Zephyr build admission (Phase 12K)
- Devkit runtime integration of `probe_translator`
- Any `devkit_app_state` modification
- Any UART command semantic change
- Scheduler 7A/7B
- F407 / custom board
- ACTIVE disarm widening
- POST_FLASH_AUTOSTART root-cause work
- Hardware validation (RTT, J-Link, OpenOCD, flashing)
- Product command mapping

**Only a subsequent explicit user command authorizing Phase 12K may open
Zephyr build admission.**
