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
- Manual RESET required after `west flash` — Phase 3B-R analysis shows `west flash` writes/verifies firmware correctly and issues `reset run`, but post-flash auto-start is unreliable on this board/probe/OpenOCD path. Physical RESET button is the required operational workaround.
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
| FLASH_WRITE | `PASS` |
| RUNTIME_AFTER_MANUAL_RESET | `PASS` |

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

#### Phase 3B Validation Status

| Gate | Status | Notes |
| ---- | ------ | ----- |
| `FLASH_WRITE` | `PASS` | Firmware programmed and verified successfully. |
| `POST_FLASH_AUTOSTART` | `OPEN / UNRELIABLE` | See Open Issue section. Manual RESET is required workaround. |
| `RUNTIME_AFTER_MANUAL_RESET` | `PASS` | All features confirmed: build metadata log, fault visibility, tick counter, LED blink, RTT. |

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

Note: `west flash` writes/verifies firmware and issues `reset run` before shutdown (hardcoded in Zephyr openocd runner).
However, post-flash auto-start is unreliable on this board/probe/OpenOCD path. Physical RESET after flash is currently required before judging runtime behavior.

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

### Open Issue — Post-flash Auto-start Unreliable (ST-LINK/SWD)

**Status: OPEN — no clean fix found. Manual RESET is the required workaround.**

#### Symptom

After `west flash`, the LED does not blink. Board appears partially dead.

Two observed failure modes depending on timing:

| Scenario             | LED State          | Cause                                                        |
|----------------------|--------------------|--------------------------------------------------------------|
| Standard cold flash  | LED solid ON       | Firmware reached GPIO init, then CPU halted before tick loop |
| Double-reset attempt | LED completely OFF | Second reset interrupted firmware before GPIO init           |

Pressing the physical RESET button after flash starts the firmware cleanly in all cases.

#### What `west flash` actually does

The Zephyr OpenOCD runner (`openocd.py:271`) always issues `reset run` before `shutdown`:

```
flash write_image erase → [post_verify cmds] → reset run → shutdown
```

There is no configurable position to inject commands AFTER `reset run`. The `--cmd-post-verify`
hook runs BEFORE `reset run`, not after.

#### Root cause

After `reset run`, the CPU starts booting. OpenOCD then calls `shutdown` immediately — the
ST-LINK USB debug connection is dropped while the MCU is mid-boot. In cold-start conditions
(no prior debug session), the ST-LINK `shutdown` sequence leaves the CPU halted before the
firmware tick loop is reached.

Hardware evidence (DHCSR at `0xE000EDF0`):

| Session state                     | DHCSR value  | Decoded                                    |
|-----------------------------------|--------------|--------------------------------------------|
| After halt command                | `0x00030003` | S_HALT=1, C_HALT=1 (explicitly halted)     |
| After west flash (no halt issued) | `0x00050001` | S_SLEEP=1, S_HALT=0 (in WFI / k_msleep)    |

The `0x00050001` reading (S_SLEEP=1, firmware in WFI) was observed consistently from within
the same debug session chain. In a true cold-start, the CPU may be halted at a different
point — the DHCSR reading is influenced by prior OpenOCD session state.

DEMCR at `0xE000EDFC` = `0x01000000`: TRCENA=1, VC_CORERESET=0. The halt-on-reset bit is
not set, so the halt is not from that mechanism.

#### Attempted fix and why it failed

`app_set_runner_args` in `CMakeLists.txt` was used to inject `reset run + sleep 300`
in `--cmd-post-verify`. This produced a double-reset sequence:

```
flash → reset run (post-verify) → sleep 300 → reset run (built-in) → shutdown
```

Observed output (4 speed-change warnings = 2 resets confirmed):

```
wrote 32768 bytes ...
Info : Unable to match requested speed 2000 kHz, using 1800 kHz   ← reset #1
Info : Unable to match requested speed 2000 kHz, using 1800 kHz
Info : Unable to match requested speed 2000 kHz, using 1800 kHz   ← reset #2
Info : Unable to match requested speed 2000 kHz, using 1800 kHz
shutdown command invoked
```

Result: **LED completely OFF** — the second `reset run` interrupted the firmware during early
boot (before `gpio_pin_configure_dt` was called), leaving GPIO in default input/floating
mode. The third `shutdown` then disconnected with no GPIO configured. This was worse than
the original single-reset behavior (LED solid ON).

The double-reset fix was reverted. CMakeLists.txt is clean with no runner overrides.

#### Why physical RESET works

The RESET button drives the MCU's NRST pin directly, bypassing the SWD interface entirely.
After NRST release, the CPU boots with no active debug connection. DEMCR and DHCSR are at
power-on defaults, no debug halt is possible. The firmware runs through its full boot
sequence and enters the tick loop cleanly.

#### Approaches not yet tried

- JLink runner (`west flash -r jlink --reset`): JLink has explicit reset-after-load support
  and generally more reliable auto-start. Requires J-Link software installation.
- `openocd.cfg` overlay with custom `reset-end` event to issue `resume` after reset.
- `adapter srst pulse_width` tuning to extend NRST assertion time.
- Investigation of whether specific ST-LINK firmware versions handle shutdown differently.

#### Workaround

Press the physical **RESET button** on the Discovery board immediately after `west flash`
completes. Firmware starts on button release. LED blinks at 500ms.

---

### Known Limitations

- No watchdog enabled — considered but deferred; no runtime-confirmed need at this phase.
- `devkit_fault_test_panic()` not invoked by default — requires explicit `DEVKIT_FAULT_TEST` define.
- No fatal hook override — Option A chosen (log-only); Zephyr default fatal handler remains active.
- **Manual RESET required after `west flash`** — see Open Issue above.
- RTT server address (`_SEGGER_RTT`) must be re-read from symbol map after pristine build (clean build relocates symbols).
- Custom STM32F407VET6 board remains hardware-unvalidated.
- `CONFIG_LOG_DEFAULT_LEVEL=3` (INF) — kernel debug messages suppressed by design.

---

### Next Recommended Phase

**Phase 4A — Core Bootstrap is unblocked.**

Operational limitation acknowledged: **Manual RESET button is required after `west flash`** before firmware starts.
This does not block core development; future work may investigate post-flash auto-start reliability separately.

Phase 4A scope:

- Introduce `robotos_core` module under `RobotOS_v1.0/core/`.
- Define first RobotOS kernel interface (event queue or scheduler stub).
- devkit becomes integration harness for core validation.

Workflow note: `west build ... && west flash && [press RESET] && [attach RTT]` confirms runtime before core integration.

---

## Phase 4A — Core Bootstrap

**Date:** 2026-05-01
**Branch:** master
**Phase 3B baseline commit:** `6e9c360` — "devkit: clarify post-flash reset limitation"

---

### Phase 4A Purpose

Introduce the first portable RobotOS core module under `RobotOS_v1.0/core/` and connect
it to the devkit runtime harness. This establishes the first clean system boundary between
devkit-specific runtime code and portable RobotOS core logic.

---

### Phase 4A Files Added

| File | Role |
| ---- | ---- |
| `core/robotos_core.h` | Public portable API — no Zephyr or board-specific types |
| `core/robotos_core.c` | Implementation — Zephyr logging used internally |
| `core/README.md` | Core module documentation |

### Phase 4A Files Modified

| File | Change |
| ---- | ------ |
| `devkit/CMakeLists.txt` | Added `../core` include directory; added `../core/robotos_core.c` to `target_sources` |
| `devkit/src/devkit_runtime.c` | Added `robotos_core_init()` call in init path; added `robotos_core_tick()` call in run loop |
| `devkit/docs/DEVKIT_PROGRESS.md` | Added Phase 4A section (this file) |

### Phase 4A Files Unchanged

| File | Reason |
| ---- | ------ |
| `devkit/src/main.c` | Thin entry point; no Phase 4A changes needed |
| `devkit/src/devkit_runtime.h` | No interface changes |
| `devkit/src/devkit_status_led.*` | Unchanged |
| `devkit/src/devkit_build_info.*` | Unchanged |
| `devkit/src/devkit_fault.*` | Unchanged |
| `devkit/prj.conf` | No new Kconfig requirements for Phase 4A |

---

### Core API

```c
// robotos_core.h — Phase 4A portable bootstrap API
const char *robotos_core_version(void);  // returns "4A-bootstrap"
int robotos_core_init(void);             // logs init, returns 0
int robotos_core_tick(void);             // increments counter, logs first + every 10th tick
```

No Zephyr types or board types exposed in the public header. Internally uses
`LOG_MODULE_REGISTER(robotos_core, LOG_LEVEL_INF)`.

Tick log policy: first tick and every 10th tick logged to avoid RTT flood.

---

### Integration Points

`devkit_runtime_init()` call order:

```
devkit_fault_init()
devkit_build_info_log()
robotos_core_init()       ← Phase 4A addition
devkit_status_led_init()
```

`devkit_runtime_run()` per-loop:

```
devkit_status_led_toggle()
devkit_runtime: tick count=N    ← existing
robotos_core_tick()             ← Phase 4A addition
k_msleep(DEVKIT_TICK_MS)
```

Core init failure is non-fatal: error logged and LED blink continues.

---

### Phase 4A Build Command

```powershell
$env:ZEPHYR_TOOLCHAIN_VARIANT = "zephyr"
$env:ZEPHYR_SDK_INSTALL_DIR   = "C:\zephyr-sdk-0.17.0"
west build -b stm32f411e_disco RobotOS_v1.0/devkit/ --pristine
```

### Phase 4A Build Memory Summary

Phase 4A post-build (no app warnings):

```
Memory region    Used Size   Region Size   %age Used
FLASH:            26180 B       512 KB       4.99%
RAM:               8576 B       128 KB       6.54%
```

Phase 3B baseline: FLASH 25848 B, RAM 8576 B. Delta: FLASH +332 B, RAM unchanged.

---

### Flash/Runtime Evidence

**Status:** RUNTIME_CONFIRMED

**Validation date:** 2026-05-01
**Hardware:** STM32F411E-DISCO, ST-LINK/V2 FW V2J47S0, OpenOCD 0.12.0-01025-g662bf274f

**Operational workflow:** `west flash` → press physical RESET → attach RTT (post-flash auto-start remains OPEN)

#### Phase 4A Flash Output

```text
Info : STLINK V2J47S0 (API v2) VID:PID 0483:3748
Info : Target voltage: 2.933652
Info : flash size = 512 KiB
wrote 32768 bytes from file zephyr.hex in 1.096093s (29.195 KiB/s)
shutdown command invoked
```

#### Phase 4A Validation Status

| Gate | Status | Notes |
| ---- | ------ | ----- |
| `FLASH_WRITE` | `PASS` | Firmware programmed and verified. |
| `POST_FLASH_AUTOSTART` | `OPEN / UNRELIABLE` | Manual RESET required. See Phase 3B Open Issue. |
| `CORE_INIT` | `PASS` | `robotos_core_init()` confirmed via RTT log and memory read. |
| `CORE_TICK` | `PASS` | `core_tick_count` increments at exact 500ms interval (confirmed via memory read). |
| `RUNTIME_AFTER_MANUAL_RESET` | `PASS` | All Phase 3B features preserved plus core active. |
| `LED` | `PASS` | PD13 blinks 500ms confirmed via GPIOD_ODR toggle. |
| `RTT_LOG` | `PASS` | Boot sequence logged in full. |
| `FAULT_STATUS` | `PASS` | DHCSR=0x00050001 (S_SLEEP=1, S_HALT=0) — firmware in WFI, no fault. |

#### RTT Log (captured via direct memory dump, post-RESET)

RTT buffer address: `_SEGGER_RTT = 0x20000800`, up buffer at `0x20000906` (1024 B).

```
*** Booting Zephyr OS build v3.6.0 ***
[00:00:00.000,000] <inf> devkit_fault: fault visibility active
[00:00:00.000,000] <inf> devkit_build_info: RobotOS devkit build info:
[00:00:00.000,000] <inf> devkit_build_info:   board=stm32f411e_disco
[00:00:00.000,000] <inf> devkit_build_info:   zephyr=3.6.0
[00:00:00.000,000] <inf> devkit_build_info:   build=May  1 2026 17:31:56
[00:00:00.000,000] <inf> devkit_build_info:   tick_ms=500
[00:00:00.000,000] <inf> devkit_build_info:   log_backend=RTT
[00:00:00.000,000] <inf> robotos_core: RobotOS core init — version=4A-bootstrap
[00:00:00.000,000] <inf> devkit_runtime: RobotOS devkit starting — board: stm32f411e_disco
[00:00:00.0... ← buffer full (1023/1024 B used by boot sequence)
```

Note: RTT buffer fills during boot sequence (same as Phase 3B). Tick messages are logged by
firmware but cannot be captured via memory dump once the buffer is full. Core tick confirmed
via direct memory read instead (see below).

#### Core Tick Confirmation (via direct memory read)

Symbol addresses from Phase 4A ELF (`arm-zephyr-eabi-nm`):

| Symbol | Address | Value (T+2.0s) | Value (T+2.5s) | Conclusion |
| ------ | ------- | -------------- | -------------- | ---------- |
| `core_tick_count` | `0x200006a8` | `0x27b` (635) | `0x27c` (636) | +1 in 500ms — exact tick interval |
| `core_initialized` | `0x200008e0` | `0x101` (non-zero) | — | `true` — init confirmed |

#### LED Confirmation (via GPIOD_ODR)

| Read time | GPIOD_ODR (`0x40020C14`) | PD13 state |
| --------- | ------------------------ | ---------- |
| T+3.0s | `0x00002000` | HIGH (LED ON) |
| T+3.5s | `0x00000000` | LOW (LED OFF) |

Bit 13 toggles — PD13 orange LED blinks at 500ms confirmed.

---

### Phase 4A Legacy Isolation Confirmation

Phase 4A sources contain **no references** to:

- `app_glue_robotos.c`
- `encoder.c`, `servo.c`, `endstop.c`
- Any file under `RobotOS_v1.0/src/` (legacy Opus-generated reference)

CMakeLists.txt source list:

```text
src/main.c
src/devkit_status_led.c
src/devkit_runtime.c
src/devkit_build_info.c
src/devkit_fault.c
../core/robotos_core.c
```

---

### Known Limitations

- Core is bootstrap only — no scheduler, event bus, or peripheral drivers.
- No RTOS threads — core runs synchronously in devkit's main-thread tick loop.
- No custom board support — core is board-agnostic by design.
- `robotos_core_tick()` log policy: first tick and every 10th tick to limit RTT usage.
- RTT buffer fills during boot; tick log messages cannot be captured via memory dump.
  Core tick confirmed via direct memory read (`core_tick_count` symbol address).
- **Manual RESET required after `west flash`** — post-flash auto-start remains OPEN.
- `CONFIG_LOG_DEFAULT_LEVEL=3` (INF) — kernel debug suppressed.

---

### Operational Flash Procedure

```bash
west build -b stm32f411e_disco RobotOS_v1.0/devkit/ --pristine
west flash
[press physical RESET button on board]
[attach RTT: OpenOCD → dump_image 0x20000800 1024]
[read core_tick_count at 0x200006a8 to confirm ticking]
```

---

### Phase 4B Recommendation

**Phase 4B — Core Contract Hardening** (Phase 4A runtime confirmed).

Options for Phase 4B (team decision):

- Define and validate the core module contract more formally (return codes, error states, reset/reinit behavior).
- Introduce the first event queue stub inside `core/`.
- Both options require Phase 4A runtime confirmed — which is now the case.

Do not implement Phase 4B until explicitly assigned.

---

## Phase 4B — Core Contract Hardening

**Date:** 2026-05-01
**Branch:** master
**Phase 4A baseline commit:** `cfb3641` — "core: add minimal RobotOS bootstrap module"

---

### Phase 4B Purpose

Harden the core API contract before introducing scheduler or event queue primitives.
Define and enforce explicit lifecycle semantics so future modules do not depend on
ambiguous or implicit behavior.

---

### Phase 4B Files Modified

| File | Change |
| ---- | ------ |
| `core/robotos_core.h` | Added `robotos_core_status_t`, `robotos_core_state_t`, `robotos_core_snapshot_t`; replaced `int` returns with typed status; added `robotos_core_state()`, `robotos_core_tick_count()`, `robotos_core_snapshot()` |
| `core/robotos_core.c` | Implemented contract semantics: state machine, init idempotency, tick-before-init guard, snapshot; version updated to `4B-contract` |
| `core/README.md` | Full contract documentation: lifecycle states, API rules, limitations, next phase |
| `devkit/src/devkit_runtime.c` | Updated core call sites to use `robotos_core_status_t core_ret`; compare against `ROBOTOS_CORE_OK` |
| `devkit/docs/DEVKIT_PROGRESS.md` | Added Phase 4B section (this file) |

### Phase 4B Files Unchanged

| File | Reason |
| ---- | ------ |
| `devkit/CMakeLists.txt` | No new source files added |
| `devkit/src/devkit_runtime.h` | No interface changes |
| `devkit/src/main.c` | Thin entry point; unchanged |
| `devkit/prj.conf` | No new Kconfig requirements |

---

### Phase 4B Core Contract Summary

#### Status Codes (`robotos_core_status_t`)

| Code | Value | Meaning |
| ---- | ----- | ------- |
| `ROBOTOS_CORE_OK` | 0 | Success |
| `ROBOTOS_CORE_ERR_INVALID_STATE` | -1 | Wrong lifecycle state (e.g. tick before init) |
| `ROBOTOS_CORE_ERR_NULL` | -2 | NULL pointer passed to snapshot |

#### Lifecycle States (`robotos_core_state_t`)

| State | Value | Meaning |
| ----- | ----- | ------- |
| `ROBOTOS_CORE_STATE_UNINITIALIZED` | 0 | Power-on default |
| `ROBOTOS_CORE_STATE_READY` | 1 | Initialized, accepting ticks |
| `ROBOTOS_CORE_STATE_ERROR` | 2 | Reserved — not yet triggered |

#### Snapshot Struct (`robotos_core_snapshot_t`)

```c
typedef struct {
    robotos_core_state_t state;
    uint32_t             tick_count;
    uint32_t             init_count;
    const char          *version;
} robotos_core_snapshot_t;
```

No locking — single-threaded contexts only.

#### API

| Function | Contract |
| -------- | -------- |
| `robotos_core_version()` | Returns `"4B-contract"`. Never NULL. |
| `robotos_core_init()` | Idempotent. First: state→READY, tick_count=0, init_count=1, LOG_INF. Repeat: init_count++, LOG_DBG. |
| `robotos_core_tick()` | Requires READY. Increments tick_count. Logs tick 1 + every 10th. Returns ERR_INVALID_STATE if not READY; warns once only. |
| `robotos_core_state()` | Returns current state enum. |
| `robotos_core_tick_count()` | Returns current tick_count. |
| `robotos_core_snapshot(out)` | Copies {state, tick_count, init_count, version} to `out`. Returns ERR_NULL if out==NULL. |

---

### Phase 4B Build Command

```powershell
$env:ZEPHYR_TOOLCHAIN_VARIANT = "zephyr"
$env:ZEPHYR_SDK_INSTALL_DIR   = "C:\zephyr-sdk-0.17.0"
west build -b stm32f411e_disco RobotOS_v1.0/devkit/ --pristine
```

### Phase 4B Build Memory Summary

Phase 4B post-build (no app warnings):

```text
Memory region    Used Size   Region Size   %age Used
FLASH:            26288 B       512 KB       5.01%
RAM:               8576 B       128 KB       6.54%
```

Phase 4A baseline: FLASH 26180 B, RAM 8576 B. Delta: FLASH +108 B, RAM unchanged.

---

### Phase 4B Flash/Runtime Evidence

**Status:** RUNTIME_CONFIRMED

**Validation date:** 2026-05-01
**Hardware:** STM32F411E-DISCO, ST-LINK/V2 FW V2J47S0, OpenOCD 0.12.0-01025-g662bf274f

**Operational workflow:** `west flash` → press physical RESET → attach RTT + memory reads

#### Phase 4B Validation Status

| Gate | Status | Notes |
| ---- | ------ | ----- |
| `FLASH_WRITE` | `PASS` | Firmware programmed and verified. |
| `POST_FLASH_AUTOSTART` | `OPEN / UNRELIABLE` | Manual RESET required. See Phase 3B Open Issue. |
| `CORE_STATE_READY` | `PASS` | `core_state` = `0x01` = `ROBOTOS_CORE_STATE_READY` confirmed. |
| `CORE_INIT_COUNT` | `PASS` | `core_init_count` = `1` — initialized exactly once. |
| `CORE_TICK` | `PASS` | `core_tick_count`: 86 → 87 in 500ms — exact interval confirmed. |
| `TICK_WARN_EMITTED` | `PASS` | `core_tick_warn_emitted` = `0x00` — no tick-before-init in normal path. |
| `RUNTIME_AFTER_MANUAL_RESET` | `PASS` | All Phase 4A features preserved plus contract API active. |
| `LED` | `PASS` | PD13 blinks 500ms via GPIOD_ODR toggle. |
| `RTT_LOG` | `PASS` | Boot sequence logged, `version=4B-contract state=READY` confirmed. |
| `FAULT_STATUS` | `PASS` | DHCSR=`0x00050001` — S_SLEEP=1, S_HALT=0 — firmware in WFI, no fault. |

#### Phase 4B RTT Log (captured via direct memory dump, post-RESET)

```text
*** Booting Zephyr OS build v3.6.0 ***
[00:00:00.000,000] <inf> devkit_fault: fault visibility active
[00:00:00.000,000] <inf> devkit_build_info: RobotOS devkit build info:
[00:00:00.000,000] <inf> devkit_build_info:   board=stm32f411e_disco
[00:00:00.000,000] <inf> devkit_build_info:   zephyr=3.6.0
[00:00:00.000,000] <inf> devkit_build_info:   build=May  1 2026 18:24:53
[00:00:00.000,000] <inf> devkit_build_info:   tick_ms=500
[00:00:00.000,000] <inf> devkit_build_info:   log_backend=RTT
[00:00:00.000,000] <inf> robotos_core: RobotOS core init — version=4B-contract state=READY
[00:00:00.000,000] <inf> devkit_runtime: RobotOS devkit starting — board: stm32f411e_disco
[00:00:00.0... ← buffer full (1022/1024 B used by boot sequence)
```

#### Phase 4B Memory Read Evidence

Symbol addresses from Phase 4B ELF (`arm-zephyr-eabi-nm`):

| Symbol | Address | Value | Decoded |
| ------ | ------- | ----- | ------- |
| `core_init_count` | `0x200006a8` | `0x00000001` | init_count = 1 — initialized once |
| `core_tick_count` (T+2.0s) | `0x200006ac` | `0x00000056` (86) | ticking |
| `core_tick_count` (T+2.5s) | `0x200006ac` | `0x00000057` (87) | +1 in 500ms — exact interval |
| `core_state` | `0x200008e5` | `0x01` | `ROBOTOS_CORE_STATE_READY` |
| `core_tick_warn_emitted` | `0x200008e4` | `0x00` | false — no tick-before-init warning |

#### Phase 4B LED Confirmation (via GPIOD_ODR)

| Read time | GPIOD_ODR (`0x40020C14`) | PD13 state |
| --------- | ------------------------ | ---------- |
| T+2.0s | `0x00002000` | HIGH (LED ON) |
| T+2.5s | `0x00000000` | LOW (LED OFF) |

Bit 13 toggles — PD13 orange LED blinks at 500ms confirmed.

---

### Phase 4B Legacy Isolation Confirmation

Phase 4B modifies only core and devkit harness files. No legacy references added.

CMakeLists.txt source list unchanged from Phase 4A:

```text
src/main.c
src/devkit_status_led.c
src/devkit_runtime.c
src/devkit_build_info.c
src/devkit_fault.c
../core/robotos_core.c
```

---

### Phase 4B Known Limitations

- Core contract is validated only on-hardware via devkit integration. No host-test layer exists yet.
- `robotos_core_snapshot()` is not thread-safe — single-threaded assumption documented in `core/README.md`.
- No scheduler, event bus, or RTOS threads.
- RTT buffer fills during boot; tick log confirmed via direct memory read.
- **Manual RESET required after `west flash`** — post-flash auto-start remains OPEN.

---

### Phase 4C Recommendation

**Phase 4C options (team decision required):**

- **Host/Core Contract Tests** — verify init idempotency, tick-before-init, snapshot semantics without hardware. Enables CI-level validation of the core contract.
- **Event Queue Stub** — introduce the first async primitive inside `core/`. Requires Phase 4B contract as foundation.

Do not implement Phase 4C until explicitly assigned.

---

## Phase 4C — Host/Core Contract Tests

**Date:** 2026-05-01
**Branch:** master
**Phase 4B baseline commit:** `08dadcc` — "core: harden bootstrap lifecycle contract"

---

### Phase 4C Purpose

Lock the `robotos_core` API contract via host-side tests that run without Zephyr,
without `west build`, and without hardware. Establishes Tier 1 validation: fastest
feedback loop for core contract regressions.

---

### Phase 4C Files Added

| File | Role |
| ---- | ---- |
| `tests/host/test_robotos_core_contract.c` | 35 sequential contract tests in plain C |
| `tests/host/CMakeLists.txt` | Isolated host build — no dependency on legacy root CMake or `src/` |

### Phase 4C Files Modified

| File | Change |
| ---- | ------ |
| `core/robotos_core.c` | Added `ROBOTOS_CORE_HOST_TEST` logging shim; added `<stdbool.h>` and `<stddef.h>` explicit includes for host-build portability |
| `core/README.md` | Added Host Contract Tests section with shim explanation, test cases, and run instructions |
| `devkit/docs/DEVKIT_PROGRESS.md` | Added Phase 4C section (this file) |

### Phase 4C Files Unchanged

| File | Reason |
| ---- | ------ |
| `core/robotos_core.h` | No changes needed — already free of Zephyr types; only `<stdint.h>` included |
| `devkit/CMakeLists.txt` | Not affected by host test infrastructure |
| `devkit/src/devkit_runtime.c` | No changes needed |

---

### Phase 4C Host Test Strategy

**Shim approach:** `robotos_core.c` detects host test mode via `ROBOTOS_CORE_HOST_TEST=1`
compile definition. In that mode, internal logging macros (`CORE_LOG_INF/WRN/DBG`) are
no-ops and Zephyr headers are not included. Standard headers `<stdbool.h>` and `<stddef.h>`
are included explicitly (normally provided transitively by Zephyr).

The shim is confined to the top of `robotos_core.c`. The public header `robotos_core.h`
is unchanged.

**Test layout:** `tests/host/CMakeLists.txt` is standalone — it does not include or depend
on the legacy root `CMakeLists.txt` or `src/` files. Build produces one executable:
`robotos_core_contract_test`.

**Execution environment:** Windows native host build blocked by broken MinGW64 installation
on dev machine. Tests compiled and run via WSL Ubuntu (gcc 13.3.0) as equivalent host.

---

### Phase 4C Contract Cases (35 tests)

| Group | Test Cases |
| ----- | ---------- |
| Pre-init | TC01–TC12: version, state=UNINITIALIZED, tick_count=0, snapshot(NULL)=ERR_NULL, tick before init=ERR_INVALID_STATE, snapshot before init |
| First init | TC13–TC17: init() returns OK, state=READY, tick_count=0, init_count=1 |
| Ticks after init | TC18–TC22: tick returns OK, tick_count increments, 10 ticks monotonic |
| Second init (idempotency) | TC23–TC27: init again=OK, state still READY, tick_count NOT reset, init_count=2 |
| Ticks after re-init | TC28–TC30: tick continues monotonically after re-init |
| Final snapshot | TC31–TC35: state=READY, tick_count=16, init_count=2, version="4B-contract" |

---

### Phase 4C Build and Test Commands

```bash
# Host build (WSL Ubuntu / Linux)
cmake -S RobotOS_v1.0/tests/host -B build-host-core
cmake --build build-host-core
ctest --test-dir build-host-core --output-on-failure
```

### Phase 4C Host Test Evidence

```text
=== robotos_core contract tests (Phase 4C) ===

[ Pre-init ]
  PASS  TC01: version() != NULL
  PASS  TC02: version() == "4B-contract"
  PASS  TC03: initial state == UNINITIALIZED
  PASS  TC04: initial tick_count() == 0
  PASS  TC05: snapshot(NULL) returns ERR_NULL
  PASS  TC06: tick() before init returns ERR_INVALID_STATE
  PASS  TC07: tick_count() unchanged after invalid tick
  PASS  TC08: snapshot() before init returns OK
  PASS  TC09: snapshot.state == UNINITIALIZED before init
  PASS  TC10: snapshot.tick_count == 0 before init
  PASS  TC11: snapshot.init_count == 0 before init
  PASS  TC12: snapshot.version == current version string

[ First init ]
  PASS  TC13: first init() returns OK
  PASS  TC14: state == READY after init
  PASS  TC15: tick_count() == 0 after first init
  PASS  TC16: snapshot.init_count == 1 after first init
  PASS  TC17: snapshot.state == READY after first init

[ Ticks after init ]
  PASS  TC18: first tick() returns OK
  PASS  TC19: tick_count() == 1 after first tick
  PASS  TC20: second tick() returns OK
  PASS  TC21: tick_count() == 2 after second tick
  PASS  TC22: tick_count() == 10 after 10 total ticks

[ Second init (idempotency) ]
  PASS  TC23: second init() returns OK
  PASS  TC24: state still READY after second init
  PASS  TC25: tick_count() unchanged after second init (== 10)
  PASS  TC26: snapshot.init_count == 2 after second init
  PASS  TC27: snapshot.tick_count unchanged == 10

[ Ticks after re-init ]
  PASS  TC28: tick() after second init returns OK
  PASS  TC29: tick_count() == 11 — monotonic after re-init
  PASS  TC30: tick_count() increments monotonically over 5 ticks

[ Final snapshot ]
  PASS  TC31: final snapshot() returns OK
  PASS  TC32: final snapshot.state == READY
  PASS  TC33: final snapshot.tick_count == 16
  PASS  TC34: final snapshot.init_count == 2
  PASS  TC35: final snapshot.version == "4B-contract"

=== Results: 35 passed, 0 failed ===
```

ctest result:

```text
100% tests passed, 0 tests failed out of 1
Total Test time (real) =   0.01 sec
```

### Phase 4C Zephyr Build Evidence

Zephyr devkit build unchanged after Phase 4C additions (no devkit source changes):

```text
Memory region    Used Size   Region Size   %age Used
FLASH:            26288 B       512 KB       5.01%
RAM:               8576 B       128 KB       6.54%
```

Identical to Phase 4B — `<stdbool.h>` and `<stddef.h>` inclusions are harmless under Zephyr (provided transitively). No app warnings.

---

### Phase 4C Runtime Evidence

Not run in Phase 4C. Zephyr firmware is unchanged from Phase 4B (no devkit source changes). Runtime evidence from Phase 4B remains valid.

---

### Phase 4C Legacy Isolation Confirmation

Host test build compiles only:

```text
tests/host/test_robotos_core_contract.c
core/robotos_core.c
```

No `RobotOS_v1.0/src/` legacy file compiled or staged.

---

### Phase 4C Known Limitations

- Windows native host build requires a working MinGW/MSVC host toolchain. Broken MinGW64 on this dev machine; WSL Ubuntu used as equivalent.
- Tests are sequential in a single process — no public reset API; order-dependent by design.
- No concurrency/thread-safety validation.
- Log behavior (one-shot WRN for tick-before-init) is tested indirectly via `core_tick_warn_emitted` memory read in devkit but not in host tests (private static).

---

### Phase 4D Recommendation

**Phase 4D — team decision required:**

- **Event Queue Contract Stub** — introduce the first async primitive inside `core/`. Phase 4C host test framework serves as the validation foundation.
- **Core Concurrency/Host Test Expansion** — extend host tests to cover future thread-safety requirements before introducing RTOS threads.

Do not implement Phase 4D until explicitly assigned.
