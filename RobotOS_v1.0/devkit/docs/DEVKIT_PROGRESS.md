# DEVKIT_PROGRESS.md — RobotOS Devkit Phase Log

## Phase 3A — Runtime Skeleton

**Date:** 2026-04-30
**Branch:** master
**Baseline commit:** `43de448` — Phase 2 devkit bring-up closed (blink + RTT LOG confirmed)

---

### Purpose

Refactor the minimal bring-up app into a clean runtime skeleton that
establishes file structure for future RobotOS core growth, without
changing observable behavior (blink rate, log output, boot sequence).

---

### Files Added

| File | Role |
|------|------|
| `src/devkit_status_led.h` | LED interface declaration |
| `src/devkit_status_led.c` | Owns `led0` GPIO alias — init and toggle only |
| `src/devkit_runtime.h` | Runtime interface + `DEVKIT_TICK_MS` constant |
| `src/devkit_runtime.c` | Boot banner, periodic tick log, LED orchestration |
| `docs/DEVKIT_PROGRESS.md` | This file |

### Files Modified

| File | Change |
|------|--------|
| `src/main.c` | Reduced to thin entry: init + run, no direct GPIO or LOG |
| `CMakeLists.txt` | Added `devkit_status_led.c` and `devkit_runtime.c` to `target_sources` |

### Files Modified (post-validation fix)

| File       | Change                                                                                    |
| ---------- | ----------------------------------------------------------------------------------------- |
| `prj.conf` | `LOG_DEFAULT_LEVEL` 4→3, added `LOG_PROCESS_THREAD_STACK_SIZE=1536` (see Issues section) |

### Files Unchanged

| File | Reason |
|------|--------|
| `README.md` | Phase 1/2 reference preserved; update deferred to Phase 3A close |

---

### Build Command

```powershell
# From D:\Robot_OS (west workspace root)
$env:ZEPHYR_TOOLCHAIN_VARIANT = "zephyr"
$env:ZEPHYR_SDK_INSTALL_DIR   = "C:\zephyr-sdk-0.17.0"
west build -b stm32f411e_disco RobotOS_v1.0/devkit/ --pristine
```

### Flash Command

```powershell
west flash
# If openocd reports reset issue: manually press RESET on the board after flash
```

---

### Expected Runtime Behavior

Identical to Phase 2 baseline:

```
*** Booting Zephyr OS build v3.6.0 ***
[00:00:00.000,000] <inf> devkit_runtime: RobotOS devkit starting — board: stm32f411e_disco
[00:00:00.000,000] <inf> devkit_runtime: LED blink loop starting
[00:00:00.000,000] <inf> devkit_runtime: tick
[00:00:00.500,000] <inf> devkit_runtime: tick
...
```

**Log module name change:** `devkit_main` → `devkit_runtime` (runtime owns the loop now).
PD13 orange LED blinks at 500ms interval. RTT log active.

---

### Phase 3A Responsibility Map

| Module | Responsibility | Dependencies |
|--------|---------------|--------------|
| `main.c` | Entry point only — init + run | `devkit_runtime.h` |
| `devkit_status_led` | GPIO binding for `led0` alias | Zephyr GPIO driver |
| `devkit_runtime` | Boot sequence, tick loop, LED orchestration | `devkit_status_led`, Zephyr kernel/log |

---

### Issues Found and Fixed During Validation

#### Issue 1 — MemManage fault: log processing thread stack overflow

**Symptom:** LED solid ON, not blinking. CPU halted in MemManage exception
(`xPSR: 0x21000004`, `pc: z_log_msg_local_claim`). RTT ticks logged once
then firmware died.

**Root cause:** `CONFIG_LOG_DEFAULT_LEVEL=4` (DBG) caused the Zephyr kernel
to emit a flood of debug messages (`os`, `mpu` subsystems) during boot into
the deferred log queue. The log processing thread (768 B stack by default)
exhausted its stack servicing this burst, triggering a MemManage fault
via the MPU guard region.

**Fix — `prj.conf`:**
```
CONFIG_LOG_DEFAULT_LEVEL=3          # INF: keeps app LOG_INF, suppresses kernel DBG
CONFIG_LOG_PROCESS_THREAD_STACK_SIZE=1536   # safety margin
```

**Result:** Firmware stable, no MemManage fault. 14 consecutive RTT ticks
captured at exact 500ms intervals. Build delta: FLASH 27252→25520 B.

---

#### Issue 2 — No auto-reset after flash: firmware does not start after `west flash`

**Symptom:** After `west flash`, LED stays OFF. Board appears dead.

**Root cause:** The `west flash` OpenOCD runner programs the firmware and
calls `shutdown` without issuing a reset command. The CPU remains halted
at the pre-flash PC. This is an OpenOCD runner behavior for this board
configuration — not a hardware limitation of the probe.

**Probe identified:** ST-LINK/V2 (`V2J47S0`, API v2). Firmware write and
verify confirmed OK. The probe itself is functional.

**Fix:** Press the physical **RESET button** on the Discovery board after
`west flash` completes. Firmware starts immediately on release.

**Confirmed:** LED blinks at 500ms (PD13 orange) after manual RESET.

---

### Known Limitations

- No RTOS threads — single main-thread blocking loop (`k_msleep`).
- `DEVKIT_TICK_MS = 500` is a compile-time constant; not runtime configurable.
- LED identified by board DTS alias `led0` — no custom board overlay.
- RTT logging only (no UART console on this board revision); RTT log requires ST-Link/SWD debug access.
- No shell, scheduler, event bus, or peripheral drivers at this phase.
- ~~Manual RESET required after every `west flash`~~ — Corrected in Phase 3B-R analysis: `west flash` issues `reset run` before shutdown. The Phase 3A "no blink after flash" was caused by the MemManage fault (LED stuck ON), not a missing reset. Manual RESET is not required with fixed firmware.
- `CONFIG_LOG_DEFAULT_LEVEL=3` (INF) — kernel debug messages suppressed by design.

---

### Legacy Isolation Confirmation

Phase 3A sources contain **no references** to:
- `app_glue_robotos.c`
- `encoder.c`, `servo.c`, `endstop.c`
- Any file under `RobotOS_v1.0/src/` (legacy Opus-generated reference)

CMakeLists.txt explicitly lists only:
```
src/main.c
src/devkit_status_led.c
src/devkit_runtime.c
```

---

### Phase 2 Baseline Reference

| Item | Value |
|------|-------|
| Commit | `43de448` |
| Board | `stm32f411e_disco` |
| Validated | PD13 blink + RTT tick log |
| Structure | Single `main.c`, flat |

---

### Phase 3A Validation Evidence

**Validation date:** 2026-04-30
**Hardware:** STM32F411E-DISCO, ST-LINK/V2, FW V2J47S0

#### Status

| Gate | Result |
| ---- | ------ |
| BUILD | `BUILD_PASS` |
| FLASH | `FLASH_WRITTEN_PASS` (corrected from `FLASH_WRITTEN_RESET_LIMITATION` — see Phase 3B-R analysis) |
| RUNTIME | `RUNTIME_PASS` |

#### Build Memory Summary

Post-fix build (LOG_DEFAULT_LEVEL=3, LOG_PROCESS_THREAD_STACK_SIZE=1536):

```
Memory region    Used Size   Region Size   %age Used
FLASH:            25520 B       512 KB       4.87%
RAM:               8576 B       128 KB       6.54%
```

Initial build before fix (LOG_DEFAULT_LEVEL=4): FLASH 27252 B, RAM 7808 B.

#### Flash Evidence

Command: `west flash` (OpenOCD 0.12.0-01025)

```
Info : STLINK V2J47S0 (API v2) VID:PID 0483:3748
Info : Target voltage: 2.925224
Info : [stm32f4x.cpu] Cortex-M4 r0p1 processor detected
Info : device id = 0x10006431
Info : flash size = 512 KiB
wrote 32768 bytes from file zephyr.hex in 1.103585s (28.996 KiB/s)
shutdown command invoked
```

Exit code 1 = OpenOCD runner exited with errors from initial connection (board was running prior firmware).
Firmware write and verify: **confirmed**. `reset run` was issued by the runner but firmware immediately
hit MemManage fault — LED stuck ON, appeared as if reset had not occurred. Manual RESET applied as
workaround; by that time the logging fix was in effect and firmware ran cleanly. Root cause was not
the reset but the MemManage fault. See Phase 3B-R for corrected analysis.

#### RTT Log Evidence

OpenOCD RTT server at `_SEGGER_RTT = 0x20000800`, port 9090.
Captured via Python TCP client (post-fix firmware, after manual RESET).

```
*** Booting Zephyr OS build v3.6.0 ***
[00:00:00.000,000] <inf> devkit_runtime: RobotOS devkit starting — board: stm32f411e_disco
[00:00:00.000,000] <inf> devkit_runtime: LED blink loop starting
[00:00:00.000,000] <inf> devkit_runtime: tick
[00:00:00.500,000] <inf> devkit_runtime: tick
[00:00:01.000,000] <inf> devkit_runtime: tick
[00:00:01.500,000] <inf> devkit_runtime: tick
...
[00:00:06.501,000] <inf> devkit_runtime: tick
```

14 consecutive ticks at exact 500ms intervals. No faults. No log errors.

- Log module: `devkit_runtime` ✓ (changed from `devkit_main` — expected)
- Boot banner: present ✓
- Tick interval: 500ms exact ✓
- LED blink: PD13 orange confirmed visually after manual RESET ✓

---

### Next Recommended Phase

**Phase 3B — Devkit Hardening:** See Phase 3B section below (implemented, runtime validation pending).

**Phase 4A — Core Bootstrap (future):**
- Introduce `robotos_core` module under `RobotOS_v1.0/core/`.
- Define first RobotOS kernel interface (event queue or scheduler stub).
- devkit becomes the integration harness for core validation.

---

## Phase 3B — Devkit Hardening

**Date:** 2026-05-01
**Branch:** master
**Phase 3A baseline commit:** `7163fed` — "devkit: stabilize runtime skeleton logging"

---

### Purpose

Add minimal observability and failure visibility to the devkit validation harness
before introducing RobotOS core modules. Ensures future core/scheduler work can
be diagnosed cleanly from boot logs without requiring a debug session.

---

### Files Added

| File | Role |
|------|------|
| `src/devkit_build_info.h` | Build metadata log interface |
| `src/devkit_build_info.c` | Logs board, Zephyr version, build timestamp, tick interval, log backend |
| `src/devkit_fault.h` | Fault visibility interface (init + guarded test panic) |
| `src/devkit_fault.c` | Logs "fault visibility active" at init; optional test panic off by default |

### Files Modified

| File | Change |
|------|--------|
| `src/devkit_runtime.c` | Added fault_init + build_info_log calls in init; added `tick_count` to run loop |
| `CMakeLists.txt` | Added `devkit_build_info.c` and `devkit_fault.c` to `target_sources` |
| `docs/DEVKIT_PROGRESS.md` | Added Phase 3B section (this file) |

### Files Unchanged

| File | Reason |
|------|--------|
| `src/main.c` | Remains thin entry point; no Phase 3B changes needed |
| `src/devkit_runtime.h` | No interface changes; `DEVKIT_TICK_MS` constant unchanged |
| `src/devkit_status_led.h` | Unchanged |
| `src/devkit_status_led.c` | Unchanged |
| `prj.conf` | No new Kconfig requirements for Phase 3B |

---

### Feature Summary

#### Build/runtime metadata log

`devkit_build_info_log()` emits at boot:
```
[devkit_build_info] RobotOS devkit build info:
[devkit_build_info]   board=stm32f411e_disco
[devkit_build_info]   zephyr=3.6.0
[devkit_build_info]   build=May  1 2026 <time>
[devkit_build_info]   tick_ms=500
[devkit_build_info]   log_backend=RTT
```

Uses `KERNEL_VERSION_STRING` from Zephyr generated `version.h` (confirmed in build output).
Uses `__DATE__` and `__TIME__` for build timestamp (accepted tradeoff: non-reproducible builds, same as prior art).

#### Fault visibility active

`devkit_fault_init()` emits at boot:
```
[devkit_fault] fault visibility active
```

No fatal hook override. Option A chosen: safe log-only announcement.
Optional test panic (`devkit_fault_test_panic`) is available if `DEVKIT_FAULT_TEST` is defined — not invoked during normal runtime.

#### Runtime tick counter

`tick_count` variable added to `devkit_runtime_run()`. Increments each loop.
Log line changed from `tick` to `tick count=<n>`:
```
[devkit_runtime] tick count=0
[devkit_runtime] tick count=1
...
```

---

### Phase 3B Responsibility Map

| Module | Responsibility | Dependencies |
|--------|---------------|--------------|
| `main.c` | Entry point only — init + run | `devkit_runtime.h` |
| `devkit_status_led` | GPIO binding for `led0` alias | Zephyr GPIO driver |
| `devkit_runtime` | Boot sequence, tick loop + counter, LED orchestration | `devkit_status_led`, `devkit_build_info`, `devkit_fault`, Zephyr kernel/log |
| `devkit_build_info` | Boot-time metadata log | `devkit_runtime.h` (for `DEVKIT_TICK_MS`), generated `version.h` |
| `devkit_fault` | Fault visibility announcement | Zephyr kernel/log |

---

### Build Command

```powershell
$env:ZEPHYR_TOOLCHAIN_VARIANT = "zephyr"
$env:ZEPHYR_SDK_INSTALL_DIR   = "C:\zephyr-sdk-0.17.0"
west build -b stm32f411e_disco RobotOS_v1.0/devkit/ --pristine
```

### Build Memory Summary

Phase 3B post-build (all new modules compiled, no app warnings):

```
Memory region    Used Size   Region Size   %age Used
FLASH:            25848 B       512 KB       4.93%
RAM:               8576 B       128 KB       6.54%
```

Phase 3A baseline (for reference): FLASH 25520 B, RAM 8576 B.
Delta: FLASH +328 B, RAM unchanged.

---

### Flash/Runtime Evidence

Status: RUNTIME_PASS

Validation date: 2026-05-01 — Hardware: STM32F411E-DISCO, ST-LINK/V2, FW V2J47S0 — OpenOCD: 0.12.0-01025-g662bf274f

#### Flash

```
Info : STLINK V2J47S0 (API v2) VID:PID 0483:3748
Info : Target voltage: 2.927284
Info : [stm32f4x.cpu] Cortex-M4 r0p1 processor detected
Info : flash size = 512 KiB
wrote 32768 bytes from file zephyr.hex in 1.104510s (28.972 KiB/s)
shutdown command invoked
```

Note: `west flash` issues `reset run` before shutdown (hardcoded in Zephyr openocd runner).
Firmware auto-starts after flash. Manual RESET is not required with Phase 3B firmware.

#### RTT Log — Captured via Direct Memory Dump

RTT buffer address: `_SEGGER_RTT = 0x20000800` (confirmed from Phase 3B ELF nm).
Method: `dump_image` of up buffer at `0x20000901` (1024 B) after 3s runtime, decoded in Python.

```
*** Booting Zephyr OS build v3.6.0 ***
[00:00:00.000,000] <inf> devkit_fault: fault visibility active
[00:00:00.000,000] <inf> devkit_build_info: RobotOS devkit build info:
[00:00:00.000,000] <inf> devkit_build_info:   board=stm32f411e_disco
[00:00:00.000,000] <inf> devkit_build_info:   zephyr=3.6.0
[00:00:00.000,000] <inf> devkit_build_info:   build=May  1 2026 10:12:30
[00:00:00.000,000] <inf> devkit_build_info:   tick_ms=500
[00:00:00.000,000] <inf> devkit_build_info:   log_backend=RTT
[00:00:00.000,000] <inf> devkit_runtime: RobotOS devkit starting — board: stm32f411e_disco
[00:00:00.000,000] <inf> devkit_runtime: LED blink loop starting
[00:00:00.000,000] <inf> devkit_runtime: tick count=0
[00:00:00.500,000] <inf> devkit_runtime: tick count=1
[00:00:01.000,000] <inf> devkit_runtime: tick count=2
[00:00:01.500,000] <inf> devkit_runtime: tick count=3
[00:00:02.000,000] <inf> dev  ← buffer full (1018/1024 B)
```

4 consecutive ticks at exact 500ms intervals confirmed. Buffer fills after ~4 ticks because
Phase 3B boot metadata (~550 B) leaves ~470 B for tick messages. Not a runtime blocker.
RTT buffer WrOff=1018, RdOff=0 verified via `mdw 0x20000800`.

#### LED

GPIOD_ODR at `0x40020C14` read at two 500ms intervals after firmware start:

| Halt    | ODR Value    | PD13 State    |
|---------|--------------|---------------|
| T+1.25s | `0x00000000` | LOW (LED OFF) |
| T+1.75s | `0x00002000` | HIGH (LED ON) |

Bit 13 toggles between reads — PD13 orange LED blinks at 500ms confirmed.

#### Fault Status

Normal execution context at both halts:

- `xPSR: 0x41000000` — Thread mode, no exception active
- No MemManage fault
- No unexpected panic
- No RTT flood

---

### Legacy Isolation Confirmation

Phase 3B sources contain **no references** to:

- `app_glue_robotos.c`
- `encoder.c`, `servo.c`, `endstop.c`
- Any file under `RobotOS_v1.0/src/` (legacy Opus-generated reference)

CMakeLists.txt explicitly lists only:

```
src/main.c
src/devkit_status_led.c
src/devkit_runtime.c
src/devkit_build_info.c
src/devkit_fault.c
```

---

### Known Limitations

- No watchdog enabled — considered but deferred; no runtime-confirmed need at this phase.
- `devkit_fault_test_panic()` not invoked by default — requires explicit `DEVKIT_FAULT_TEST` define.
- No fatal hook override — Option A chosen (log-only); Zephyr default fatal handler remains active.
- `west flash` auto-resets via `reset run` (hardcoded in Zephyr openocd runner `openocd.py:271`); manual RESET is not required with Phase 3B firmware. The Phase 3A "manual RESET required" note was a misdiagnosis — the real cause was the MemManage fault making the LED appear stuck rather than the reset failing. Manual RESET may still help recover from edge-case exit-code-1 flash failures.
- RTT server address (`_SEGGER_RTT`) must be re-read from symbol map after pristine build (clean build relocates symbols).
- Custom STM32F407VET6 board remains hardware-unvalidated.
- `CONFIG_LOG_DEFAULT_LEVEL=3` (INF) — kernel debug messages suppressed by design.

---

### Next Recommended Phase

Phase 4A — Core Bootstrap (if Phase 3B runtime confirmed):

- Introduce `robotos_core` module under `RobotOS_v1.0/core/`.
- Define first RobotOS kernel interface (event queue or scheduler stub).
- devkit becomes integration harness for core validation.

Phase 3B-R — Runtime Follow-up (if runtime pending or fails):

- Flash Phase 3B firmware, capture RTT boot log, confirm tick_count progression.
- Confirm no MemManage fault from additional log modules.
- Update this section with `RUNTIME_PASS` evidence and close 3B.
