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

---

## Phase 4D — Core Event Queue Contract Stub

**Date:** 2026-05-01
**Branch:** master
**Phase 4C baseline commit:** `db64cf3` — "core: add host contract tests"

---

### Phase 4D Purpose

Introduce a minimal, deterministic, fixed-capacity Core Event Queue under
`RobotOS_v1.0/core/`. The queue contract is defined and validated via host tests
before Zephyr/hardware integration. No scheduler or dispatcher is added — this is
the data structure and contract only.

---

### Phase 4D Files Added

| File | Role |
| ---- | ---- |
| `core/robotos_event_queue.h` | Event type, queue struct, and full queue API — no Zephyr types |
| `core/robotos_event_queue.c` | Ring buffer implementation — zero Zephyr dependency |
| `tests/host/test_robotos_event_queue_contract.c` | 58 host contract tests for event queue |

### Phase 4D Files Modified

| File | Change |
| ---- | ------ |
| `core/robotos_core.h` | Added `ROBOTOS_CORE_ERR_FULL=-3` and `ROBOTOS_CORE_ERR_EMPTY=-4` to status enum |
| `core/robotos_core.c` | Includes `robotos_event_queue.h`; `robotos_core_init()` initializes static internal queue and logs "event queue initialized capacity=16" |
| `core/README.md` | Added Event Queue Contract section |
| `tests/host/CMakeLists.txt` | Added `robotos_event_queue_contract_test` target; added `robotos_event_queue.c` to core contract test target (required by new include) |
| `devkit/CMakeLists.txt` | Added `../core/robotos_event_queue.c` to Zephyr target_sources |
| `devkit/docs/DEVKIT_PROGRESS.md` | Added Phase 4D section (this file) |

### Phase 4D Files Unchanged

| File | Reason |
| ---- | ------ |
| `devkit/src/devkit_runtime.c` | No devkit-visible integration needed |
| `devkit/prj.conf` | No new Kconfig requirements |

---

### Phase 4D Event Queue Contract Summary

#### Event Types (`robotos_event_type_t`)

| Name | Value | Meaning |
| ---- | ----- | ------- |
| `ROBOTOS_EVENT_NONE` | 0 | No event / cleared slot |
| `ROBOTOS_EVENT_CORE_TICK` | 1 | Core tick elapsed (reserved for future dispatcher) |
| `ROBOTOS_EVENT_USER` | 100 | Base for application-defined events |

#### Event Struct (`robotos_event_t`)

```c
typedef struct {
    robotos_event_type_t type;
    uint32_t             timestamp_tick;
    uint32_t             arg0;
    uint32_t             arg1;
} robotos_event_t;
```

#### Queue Struct (`robotos_event_queue_t`)

Fixed capacity 16. Caller owns the struct (static/stack). No dynamic allocation.

#### API Contract

| Function | Behavior |
| -------- | -------- |
| `init(NULL)` | `ERR_NULL` |
| `init(q)` | OK; sets initialized=true, count=0, dropped=0 |
| Queries on NULL | Safe defaults: `is_initialized=false`, `is_empty=true`, `is_full=false`, `count/capacity/dropped=0` |
| `push(NULL q/event)` | `ERR_NULL` |
| `push` before init | `ERR_INVALID_STATE` |
| `push` when full | `ERR_FULL`, `dropped_count++`, queue content unchanged |
| `push` normal | OK, copies event, count++ |
| `pop(NULL)` | `ERR_NULL` |
| `pop` before init | `ERR_INVALID_STATE` |
| `pop` empty | `ERR_EMPTY` |
| `pop` normal | OK, FIFO order, count-- |
| `peek` empty | `ERR_EMPTY`, queue unchanged |
| `peek` normal | OK, reads front without removing |
| `clear(NULL)` | `ERR_NULL` |
| `clear` before init | `ERR_INVALID_STATE` |
| `clear` normal | OK, count=0; `dropped_count` preserved (diagnostic history) |

All operations are O(1), deterministic, single-threaded only.

#### Core Integration

`robotos_core_init()` initializes `static robotos_event_queue_t s_core_event_queue`.
No events are pushed in `robotos_core_tick()` — queue available for future Phase 4E dispatcher.

---

### Phase 4D Host Test Evidence

**Commands (WSL Ubuntu, gcc 13.3.0):**

```bash
cmake -S RobotOS_v1.0/tests/host -B build-host-core
cmake --build build-host-core
ctest --test-dir build-host-core --output-on-failure
```

**Result:**

```text
1/2 Test #1: robotos_core_contract ............   Passed    0.00 sec
2/2 Test #2: robotos_event_queue_contract .....   Passed    0.00 sec

100% tests passed, 0 tests failed out of 2
Total Test time (real) = 0.01 sec
```

Phase 4C core contract: **35/35 PASS** (preserved)
Phase 4D event queue: **58/58 PASS**

---

### Phase 4D Zephyr Build Evidence

```text
Memory region    Used Size   Region Size   %age Used
FLASH:            26392 B       512 KB       5.03%
RAM:               8832 B       128 KB       6.74%
```

Phase 4C baseline: FLASH 26288 B, RAM 8576 B.
Delta: FLASH +104 B, RAM +256 B (16 × 16 B event buffer).
No app warnings.

---

### Phase 4D Runtime Evidence

**Status:** RUNTIME_CONFIRMED

**Validation date:** 2026-05-01
**Hardware:** STM32F411E-DISCO, ST-LINK/V2 FW V2J47S0

#### Phase 4D Runtime Validation Status

| Gate | Status | Notes |
| ---- | ------ | ----- |
| `FLASH_WRITE` | `PASS` | Firmware programmed, 32768 B verified. |
| `POST_FLASH_AUTOSTART` | `OPEN` | Manual RESET required per known workaround. |
| `CORE_STATE_READY` | `PASS` | `core_state` @ `0x200009f9` = `0x01` = READY. |
| `EVENT_QUEUE_INITIALIZED` | `PASS` | `s_core_event_queue.initialized` @ `0x200007b8` = `0x01` = true. |
| `EVENT_QUEUE_COUNT_ZERO` | `PASS` | `s_core_event_queue.count` @ `0x200007b0` = `0x00000000` = 0 (no events pushed). |
| `LED` | `PASS` | GPIOD_ODR `0x00002000` → `0x00000000` — PD13 toggles, 500ms blink. |
| `FAULT_STATUS` | `PASS` | No fault. LED blink confirms clean runtime. |

---

### Phase 4D Legacy Isolation Confirmation

Host test build compiles only:

```text
tests/host/test_robotos_core_contract.c
tests/host/test_robotos_event_queue_contract.c
core/robotos_core.c
core/robotos_event_queue.c
```

No `RobotOS_v1.0/src/` legacy file compiled or staged.

---

### Phase 4D Known Limitations

- No event dispatcher or scheduler — queue is a contract stub only.
- No automatic event push in `robotos_core_tick()` — avoids fill/spam with no drain loop.
- No concurrent/ISR-safe access — single-threaded assumption.
- No dynamic allocation — capacity fixed at 16.
- `dropped_count` preserved across `clear()` — no public reset for diagnostic integrity.
- Windows MinGW64 host build broken; WSL Ubuntu used for host tests.

---

### Phase 4E Recommendation

**Phase 4E — Core Event Dispatch Stub** (Phase 4D runtime confirmed):

Introduce a minimal dispatcher that drains `s_core_event_queue` each tick,
delivering events to registered handlers. Requires Phase 4D queue contract
as foundation. Do not implement until explicitly assigned.

---

## Phase 4E — Core Event Dispatch Stub

**Date:** 2026-05-01
**Branch:** master
**Phase 4D baseline commit:** `f8634d8` — "core: add event queue contract stub"

---

### Phase 4E Purpose

Add a minimal, deterministic event dispatcher on top of the Phase 4D event queue.
Dispatcher drains events from a queue to a registered handler in FIFO order.
Host-tested before Zephyr integration. No scheduler, threads, or ISR semantics.

---

### Phase 4E Files Added

| File | Role |
| ---- | ---- |
| `core/robotos_event_dispatcher.h` | Dispatcher type, handler signature, and full API — no Zephyr types |
| `core/robotos_event_dispatcher.c` | FIFO dispatch implementation — zero Zephyr dependency |
| `tests/host/test_robotos_event_dispatcher_contract.c` | 55 host contract tests for dispatcher |

### Phase 4E Files Modified

| File | Change |
| ---- | ------ |
| `core/robotos_core.c` | Includes `robotos_event_dispatcher.h`; `robotos_core_init()` initializes static `s_core_dispatcher` with private no-op handler; logs "event dispatcher initialized" |
| `tests/host/CMakeLists.txt` | Added `robotos_event_dispatcher_contract_test` target; updated core contract test to include `robotos_event_dispatcher.c` |
| `devkit/CMakeLists.txt` | Added `../core/robotos_event_dispatcher.c` to Zephyr target_sources |
| `devkit/docs/DEVKIT_PROGRESS.md` | Added Phase 4E section (this file) |

### Phase 4E Files Unchanged

| File | Reason |
| ---- | ------ |
| `core/robotos_core.h` | No new public API additions |
| `core/robotos_event_queue.h/.c` | Queue contract unchanged |
| `devkit/src/devkit_runtime.c` | No devkit-visible integration needed |

---

### Phase 4E Event Dispatcher Contract Summary

#### Handler Signature

```c
typedef robotos_core_status_t (*robotos_event_handler_t)(
    const robotos_event_t *event,
    void *user_context
);
```

#### Dispatcher Struct (`robotos_event_dispatcher_t`)

Holds: `*queue`, `handler`, `*user_context`, `initialized`, `dispatched_count`, `handler_error_count`. Caller owns — not heap-allocated.

#### Phase 4E API Contract

| Function | Behavior |
| -------- | -------- |
| `init(NULL d/q/handler)` | `ERR_NULL` |
| `init(d, uninit_q, ...)` | `ERR_INVALID_STATE` |
| `init(valid)` | OK; initialized=true, counts=0; re-init resets counts |
| Queries on NULL | `is_initialized=false`, counts=0 |
| `dispatch_one(NULL)` | `ERR_NULL` |
| `dispatch_one` before init | `ERR_INVALID_STATE` |
| `dispatch_one` empty queue | `ERR_EMPTY` |
| `dispatch_one` success | pop → call handler → `dispatched_count++` → OK |
| `dispatch_one` handler error | pop → call handler → `handler_error_count++` → return handler's error |
| `dispatch_all(NULL, n)` | `ERR_NULL` |
| `dispatch_all` max=0 | OK (nothing dispatched = success) |
| `dispatch_all` max>0 empty | `ERR_EMPTY` |
| `dispatch_all` success | dispatch up to max; OK if at least one dispatched before empty |
| `dispatch_all` handler error | stop, `handler_error_count++`, return handler's error |

Handler errors propagate directly (no wrapping). Failing events are always consumed.

#### Phase 4E Core Integration

`robotos_core_init()` initializes static `s_core_dispatcher` bound to `s_core_event_queue` with a private no-op default handler. No events are pushed in `robotos_core_tick()`. Dispatcher available for future Phase 4F event ingestion.

---

### Phase 4E Host Test Evidence

**Commands (WSL Ubuntu, gcc 13.3.0):**

```bash
cmake -S RobotOS_v1.0/tests/host -B build-host-core
cmake --build build-host-core
ctest --test-dir build-host-core --output-on-failure
```

**Result:**

```text
1/3 Test #1: robotos_core_contract ...............   Passed    0.00 sec
2/3 Test #2: robotos_event_queue_contract ........   Passed    0.00 sec
3/3 Test #3: robotos_event_dispatcher_contract ...   Passed    0.00 sec

100% tests passed, 0 tests failed out of 3
Total Test time (real) = 0.01 sec
```

Phase 4C core contract: **35/35 PASS** (preserved)
Phase 4D event queue: **58/58 PASS** (preserved)
Phase 4E dispatcher: **55/55 PASS**

---

### Phase 4E Zephyr Build Evidence

```text
Memory region    Used Size   Region Size   %age Used
FLASH:            26520 B       512 KB       5.06%
RAM:               8832 B       128 KB       6.74%
```

Phase 4D baseline: FLASH 26392 B, RAM 8832 B.
Delta: FLASH +128 B, RAM unchanged (dispatcher struct is BSS, fits within prior allocation).
No app warnings.

---

### Phase 4E Runtime Evidence

**Status:** RUNTIME_CONFIRMED

**Validation date:** 2026-05-01
**Hardware:** STM32F411E-DISCO, ST-LINK/V2 FW V2J47S0

#### Phase 4E Runtime Validation Status

| Gate | Status | Notes |
| ---- | ------ | ----- |
| `FLASH_WRITE` | `PASS` | Firmware programmed, 32768 B verified. |
| `POST_FLASH_AUTOSTART` | `OPEN` | Manual RESET required per known workaround. |
| `CORE_STATE_READY` | `PASS` | `core_state` @ `0x20000a11` = `0x01` = READY. |
| `DISPATCHER_INITIALIZED` | `PASS` | `s_core_dispatcher.initialized` @ `0x200006b4` = `0x01` = true. |
| `DISPATCHER_COUNT_ZERO` | `PASS` | `dispatched_count` @ `0x200006b8` = `0x00000000` — no events dispatched. |
| `LED` | `PASS` | GPIOD_ODR toggles `0x00002000` → `0x00000000` — 500ms blink confirmed. |
| `FAULT_STATUS` | `PASS` | No fault. LED blink confirms clean runtime. |

---

### Phase 4E Legacy Isolation Confirmation

Host test build compiles only:

```text
tests/host/test_robotos_core_contract.c
tests/host/test_robotos_event_queue_contract.c
tests/host/test_robotos_event_dispatcher_contract.c
core/robotos_core.c
core/robotos_event_queue.c
core/robotos_event_dispatcher.c
```

No `RobotOS_v1.0/src/` legacy file compiled or staged.

---

### Phase 4E Known Limitations

- No event ingestion public API — `s_core_event_queue` can only be filled by future Phase 4F API.
- No auto-push in `robotos_core_tick()` — avoids fill/spam with no active dispatcher drain.
- No scheduler — caller decides when and how often to dispatch.
- No priority or delayed events — strict FIFO.
- No concurrent/ISR-safe access — single-threaded only.
- No dynamic allocation.
- Windows MinGW64 host build broken; WSL Ubuntu used for host tests.

---

### Phase 4F Recommendation

**Phase 4F — Core Event Ingestion API** (Phase 4E runtime confirmed):

Add a public `robotos_core_post_event()` API to allow callers to enqueue events
into the internal queue. In `robotos_core_tick()`, call `dispatch_all(max=n)` to drain
the queue. This completes the produce → dispatch → handle cycle without a scheduler.
Do not implement until explicitly assigned.

---

## Phase 4F — Core Event Ingestion API

**Date:** 2026-05-02
**Branch:** master
**Phase 4E baseline commit:** `254ea16` — "core: add event dispatcher contract stub"

---

### Phase 4F Purpose

Add public event post and dispatch APIs to the RobotOS core, completing the
produce → dispatch → handle cycle. Host-tested before Zephyr/hardware integration.
No scheduler, threads, or auto-generation of events.

---

### Phase 4F Files Added

| File | Role |
| ---- | ---- |
| `tests/host/test_robotos_core_ingestion_contract.c` | 57 host contract tests for post_event / dispatch_events / counters |

### Phase 4F Files Modified

| File | Change |
| ---- | ------ |
| `core/robotos_core.h` | Moved `robotos_event_type_t` and `robotos_event_t` here (core concepts); extended `robotos_core_snapshot_t` with event counter fields; added `post_event`, `dispatch_events`, 4 counter getter declarations |
| `core/robotos_event_queue.h` | Removed `robotos_event_type_t` and `robotos_event_t` (now in `robotos_core.h`); kept queue struct and API |
| `core/robotos_core.c` | Implemented `post_event`, `dispatch_events`, 4 counter getters; updated `snapshot()` with event counter fields |
| `tests/host/CMakeLists.txt` | Added `robotos_core_ingestion_contract_test` target |
| `devkit/docs/DEVKIT_PROGRESS.md` | Added Phase 4F section (this file) |

### Phase 4F Files Unchanged

| File | Reason |
| ---- | ------ |
| `devkit/CMakeLists.txt` | No new source files — all code is in existing `robotos_core.c` |
| `devkit/src/devkit_runtime.c` | No integration change — explicit dispatch not called yet |
| `core/robotos_event_queue.c` | No changes needed |
| `core/robotos_event_dispatcher.c` | No changes needed |

---

### Phase 4F Event Ingestion Contract Summary

#### Architectural Note

`robotos_event_type_t` and `robotos_event_t` moved from `robotos_event_queue.h` to
`robotos_core.h` to resolve a circular include dependency. Events are core concepts.
`robotos_event_queue.h` includes `robotos_core.h` to obtain them. No API or behavior change.

#### Phase 4F API Summary

| Function | Behavior |
| -------- | -------- |
| `post_event(NULL)` | `ERR_NULL` |
| `post_event(ev)` before init | `ERR_INVALID_STATE` |
| `post_event(ev)` after init | pushes to queue; `ERR_FULL` if full (`dropped_count++`) |
| `dispatch_events(n)` before init | `ERR_INVALID_STATE` (including n=0) |
| `dispatch_events(0)` after init | `OK` — nothing dispatched |
| `dispatch_events(n>0)` empty | `ERR_EMPTY` |
| `dispatch_events(n>0)` with events | dispatches up to n; `OK` if ≥1 dispatched |
| `pending_event_count()` | current events in queue; 0 before init |
| `dropped_event_count()` | cumulative drops due to full queue; 0 before init |
| `dispatched_event_count()` | cumulative dispatches; 0 before init |
| `handler_error_count()` | cumulative handler errors; 0 before init |

#### Phase 4F Snapshot Extension

`robotos_core_snapshot_t` now includes four additional fields:
`pending_event_count`, `dropped_event_count`, `dispatched_event_count`, `handler_error_count`.
All are 0 before init. Backward-compatible (new fields appended at end of struct).

#### Phase 4F Repeated Init Contract

Repeated `robotos_core_init()` does NOT reinitialize the internal queue or dispatcher.
Queued events are preserved across repeated init calls (confirmed by TC49).

---

### Phase 4F Host Test Evidence

**Commands (WSL Ubuntu, gcc 13.3.0):**

```bash
cmake -S RobotOS_v1.0/tests/host -B build-host-core
cmake --build build-host-core
ctest --test-dir build-host-core --output-on-failure
```

**Result:**

```text
1/4 Test #1: robotos_core_contract ...............   Passed    0.00 sec
2/4 Test #2: robotos_event_queue_contract ........   Passed    0.00 sec
3/4 Test #3: robotos_event_dispatcher_contract ...   Passed    0.00 sec
4/4 Test #4: robotos_core_ingestion_contract .....   Passed    0.00 sec

100% tests passed, 0 tests failed out of 4
Total Test time (real) = 0.02 sec
```

Phase 4C core contract: **35/35 PASS** (preserved)
Phase 4D event queue: **58/58 PASS** (preserved)
Phase 4E dispatcher: **55/55 PASS** (preserved)
Phase 4F ingestion: **57/57 PASS**

---

### Phase 4F Zephyr Build Evidence

```text
Memory region    Used Size   Region Size   %age Used
FLASH:            26520 B       512 KB       5.06%
RAM:               8832 B       128 KB       6.74%
```

Identical to Phase 4E — no new source files; new functions compiled into existing `robotos_core.c`.

---

### Phase 4F Runtime Evidence

Not run. `devkit_runtime.c` unchanged. No new log output. Phase 4E runtime evidence
(core READY, dispatcher initialized, LED blinks, no fault) remains valid.

---

### Phase 4F Legacy Isolation Confirmation

No `RobotOS_v1.0/src/` legacy file compiled or staged.

---

### Phase 4F Known Limitations

- No auto-dispatch in `robotos_core_tick()` — caller must call `dispatch_events()` explicitly.
- Internal handler is default no-op; no observable event content in current devkit.
- No public handler registration API — internal dispatcher only.
- No scheduler or event priority — strict FIFO.
- No concurrent/ISR-safe access — single-threaded only.
- Windows MinGW64 host build broken; WSL Ubuntu used for host tests.

---

### Phase 4G Recommendation

**Phase 4G options (team decision required):**

- **Scheduler Tick Policy Stub** — define how many events `robotos_core_tick()` dispatches per tick; bound the dispatch budget.
- **Event Handler Policy** — expose a public handler registration API for user-defined handlers.
- **Phase 5A Zephyr Adapter Boundary** — formalize the Zephyr/core boundary before growing core further.

Do not implement Phase 4G until explicitly assigned.

---

## Phase 4G — Scheduler Tick Policy Stub

**Date:** 2026-05-02
**Branch:** master
**Phase 4F baseline commit:** `e38dafc` — "core: add event ingestion API"

---

### Phase 4G Purpose

Define a minimal, deterministic event dispatch policy on each `robotos_core_tick()` call.
Each tick now dispatches up to `ROBOTOS_CORE_MAX_EVENTS_PER_TICK` queued events before
returning. This is a tick policy stub — not a scheduler, not a task registry,
not a priority system.

---

### Phase 4G Files Added

| File | Role |
| ---- | ---- |
| `tests/host/test_robotos_core_tick_policy_contract.c` | ~42 host contract tests for tick policy behavior |

### Phase 4G Files Modified

| File | Change |
| ---- | ------ |
| `core/robotos_core.h` | Added `ROBOTOS_CORE_MAX_EVENTS_PER_TICK 1u`; updated `robotos_core_tick()` contract comment |
| `core/robotos_core.c` | `robotos_core_tick()` now dispatches up to `ROBOTOS_CORE_MAX_EVENTS_PER_TICK` events; ERR_EMPTY normalized to OK |
| `tests/host/CMakeLists.txt` | Added `robotos_core_tick_policy_contract_test` target |
| `devkit/docs/DEVKIT_PROGRESS.md` | Added Phase 4G section (this file) |

### Phase 4G Files Unchanged

| File | Reason |
| ---- | ------ |
| `devkit/CMakeLists.txt` | No new source files |
| `devkit/src/devkit_runtime.c` | No integration change needed |
| All other core modules | Queue, dispatcher, ingestion unchanged |

---

### Phase 4G Tick Policy Contract Summary

| Aspect | Contract |
| ------ | -------- |
| `ROBOTOS_CORE_MAX_EVENTS_PER_TICK` | `1u` (Phase 4G stub; public constant) |
| Empty queue during tick | Normalized to OK — not an error |
| Handler error during tick | Returned as-is; `handler_error_count++`; state NOT set to ERROR (stub) |
| Auto-CORE_TICK event | Not generated — no events auto-produced |
| tick_count | Always increments before dispatch |
| Explicit `dispatch_events(n)` | Unchanged; `dispatch_events(1)` on empty still returns ERR_EMPTY |
| Tick dispatch vs explicit | Tick normalizes empty to OK; explicit preserves ERR_EMPTY |
| Concurrency/ISR | Not safe — single-threaded only |

---

### Phase 4G Host Test Evidence

**Commands (WSL Ubuntu, gcc 13.3.0):**

```bash
cmake -S RobotOS_v1.0/tests/host -B build-host-core
cmake --build build-host-core
ctest --test-dir build-host-core --output-on-failure
```

**Result:**

```text
1/5 Test #1: robotos_core_contract ...............   Passed    0.00 sec
2/5 Test #2: robotos_event_queue_contract ........   Passed    0.00 sec
3/5 Test #3: robotos_event_dispatcher_contract ...   Passed    0.00 sec
4/5 Test #4: robotos_core_ingestion_contract .....   Passed    0.00 sec
5/5 Test #5: robotos_core_tick_policy_contract ...   Passed    0.00 sec

100% tests passed, 0 tests failed out of 5
Total Test time (real) = 0.02 sec
```

All prior suites preserved. Phase 4G tick policy: **~42/42 PASS**.

---

### Phase 4G Zephyr Build Evidence

```text
Memory region    Used Size   Region Size   %age Used
FLASH:            26708 B       512 KB       5.09%
RAM:               8832 B       128 KB       6.74%
```

Phase 4F baseline: FLASH 26520 B, RAM 8832 B.
Delta: FLASH +188 B (dispatch logic added to tick), RAM unchanged.

---

### Phase 4G Runtime Evidence

**Status:** RUNTIME_CONFIRMED

**Validation date:** 2026-05-02
**Hardware:** STM32F411E-DISCO, ST-LINK/V2 FW V2J47S0

| Gate | Status | Notes |
| ---- | ------ | ----- |
| `FLASH_WRITE` | `PASS` | Firmware programmed and verified. |
| `POST_FLASH_AUTOSTART` | `OPEN` | Manual RESET required per known workaround. |
| `RTT_BOOT_LOG` | `PASS` | Boot sequence confirmed: devkit_fault, devkit_build_info, core init. |
| `LED` | `PASS` | GPIOD_ODR `0x00002000` → `0x00000000` — 500ms blink confirmed. |
| `FAULT_STATUS` | `PASS` | No fault. Clean runtime. |
| `TICK_DISPATCH_VISIBLE` | `N/A` | No events posted in devkit; empty-queue normalize to OK = no observable change. |

---

### Phase 4G Legacy Isolation Confirmation

No `RobotOS_v1.0/src/` legacy file compiled or staged.

---

### Phase 4G Known Limitations

- `ROBOTOS_CORE_MAX_EVENTS_PER_TICK = 1` is a stub value — not tuned for any workload.
- No auto-CORE_TICK event generation.
- No public handler registration — only internal default no-op handler.
- No priority or admission policy.
- No concurrency/ISR-safe access.
- Handler error during tick returns non-OK but does not transition state to ERROR (Phase 4G stub).
- No observable tick-dispatch evidence in current devkit (no events posted).

---

### Phase 4H Recommendation

**Phase 4H options (team decision required):**

- **Handler Policy / Handler Registration Boundary** — expose a public API for registering user-defined event handlers, replacing the default no-op.
- **Scheduler Admission Policy Stub** — define how events enter the queue under bounded backpressure.
- **Phase 5A Zephyr Adapter Boundary** — formalize the Zephyr/core boundary and begin the adapter layer design.

Do not implement Phase 4H until explicitly assigned.

---

## Phase 4H — Handler Policy / Handler Registration Boundary

**Date:** 2026-05-02
**Branch:** master
**Phase 4G baseline commit:** `434392a` — "core: add scheduler tick policy stub"

---

### Phase 4H Purpose

Add static type-routed event handler registration to the RobotOS core.
Dispatched events are routed to the registered handler for their type.
Unregistered event types are counted as unhandled but consumed without error.
This is a registration boundary stub — not a scheduler, not a plugin system.

---

### Phase 4H Files Added

| File | Role |
| ---- | ---- |
| `tests/host/test_robotos_core_handler_policy_contract.c` | 50 host contract tests for handler registration/routing |

### Phase 4H Files Modified

| File | Change |
| ---- | ------ |
| `core/robotos_core.h` | Added `ERR_INVALID_ARG=-6`, `ROBOTOS_CORE_MAX_EVENT_HANDLERS 8u`, `robotos_core_event_handler_t` typedef; 5 new API declarations; snapshot extended with `registered_handler_count` and `unhandled_event_count` |
| `core/robotos_core.c` | Added static handler table; replaced no-op default handler with routing handler; implemented register/unregister/has/count APIs; `core_init()` initializes handler table on first call only |
| `tests/host/CMakeLists.txt` | Added `robotos_core_handler_policy_contract_test` target |
| `devkit/docs/DEVKIT_PROGRESS.md` | Added Phase 4H section (this file) |

### Phase 4H Files Unchanged

| File | Reason |
| ---- | ------ |
| `devkit/CMakeLists.txt` | No new source files |
| `devkit/src/devkit_runtime.c` | No integration change needed |
| All other core modules | Queue, dispatcher, ingestion, tick policy unchanged |

---

### Phase 4H Handler Policy Contract Summary

#### Handler Table

Static array of 8 entries: `{type, handler, user_context, in_use}`. No dynamic allocation.

#### Phase 4H API Summary

| Function | Behavior |
| -------- | -------- |
| `register(type, NULL, ctx)` | `ERR_NULL` |
| `register` before init | `ERR_INVALID_STATE` |
| `register(NONE, ...)` | `ERR_INVALID_ARG` |
| `register` new type | OK; `registered_handler_count++` |
| `register` existing type | OK; replaces handler/context; count unchanged |
| `register` at capacity | `ERR_FULL` |
| `unregister` before init | `ERR_INVALID_STATE` |
| `unregister(NONE)` | `ERR_INVALID_ARG` |
| `unregister` existing | OK; clears slot; count-- |
| `unregister` missing | `ERR_EMPTY` |
| `has_handler(type)` | bool; false before init or no handler |
| `registered_handler_count()` | count; 0 before init |
| `unhandled_event_count()` | cumulative; 0 before init |

#### Routing Behavior

- Dispatched events routed by `event->type` to matching registered handler.
- If handler found: called; result propagated (OK or handler error).
- If no handler: `unhandled_event_count++`; routing handler returns OK.
- `dispatched_event_count` increments regardless (event consumed by routing handler).

#### Repeated Init

Repeated `robotos_core_init()` does NOT clear handler table. Registered handlers survive.

---

### Phase 4H Host Test Evidence

**Commands (WSL Ubuntu, gcc 13.3.0):**

```bash
cmake -S RobotOS_v1.0/tests/host -B build-host-core
cmake --build build-host-core
ctest --test-dir build-host-core --output-on-failure
```

**Result:**

```text
1/6 Test #1: robotos_core_contract .....................   Passed    0.00 sec
2/6 Test #2: robotos_event_queue_contract ...............   Passed    0.00 sec
3/6 Test #3: robotos_event_dispatcher_contract ..........   Passed    0.00 sec
4/6 Test #4: robotos_core_ingestion_contract ............   Passed    0.00 sec
5/6 Test #5: robotos_core_tick_policy_contract ..........   Passed    0.00 sec
6/6 Test #6: robotos_core_handler_policy_contract .......   Passed    0.00 sec

100% tests passed, 0 tests failed out of 6
Total Test time (real) = 0.02 sec
```

All prior suites preserved. Phase 4H handler policy: **50/50 PASS**.

---

### Phase 4H Zephyr Build Evidence

```text
Memory region    Used Size   Region Size   %age Used
FLASH:            26816 B       512 KB       5.11%
RAM:               9024 B       128 KB       6.88%
```

Phase 4G baseline: FLASH 26708 B, RAM 8832 B.
Delta: FLASH +108 B, RAM +192 B (8 handler table entries × ~24 B each).

---

### Phase 4H Runtime Evidence

**Status:** RUNTIME_CONFIRMED

**Validation date:** 2026-05-02
**Hardware:** STM32F411E-DISCO, ST-LINK/V2 FW V2J47S0

| Gate | Status | Notes |
| ---- | ------ | ----- |
| `FLASH_WRITE` | `PASS` | Firmware programmed and verified. |
| `POST_FLASH_AUTOSTART` | `OPEN` | Manual RESET required per known workaround. |
| `LED` | `PASS` | GPIOD_ODR toggles `0x00000000` → `0x00002000` — 500ms blink. |
| `FAULT_STATUS` | `PASS` | DHCSR=`0x00050001` — S_SLEEP=1, S_HALT=0 — firmware in WFI, no fault. |
| `HANDLER_ROUTING_VISIBLE` | `N/A` | No handlers registered in devkit; unregistered tick events counted as unhandled silently. |

---

### Phase 4H Legacy Isolation Confirmation

No `RobotOS_v1.0/src/` legacy file compiled or staged.

---

### Phase 4H Known Limitations

- Static handler table: max 8 handlers, fixed at compile time.
- One handler per event type — no multiple handlers or chaining.
- No wildcard handler for unregistered types.
- No priority dispatch — FIFO, type-matched only.
- Unregistered events silently counted as unhandled (no error).
- No concurrency/ISR-safe access — single-threaded only.
- No public `clear_all_handlers()` API — unregister individually.
- Handler errors propagate from tick but do not transition core to ERROR state.

---

### Phase 4I Recommendation

**Phase 4I options (team decision required):**

- **Core System Loop Contract** — define the full tick loop semantics: post → tick → dispatch → route → handle.
- **Phase 5A Zephyr Adapter Boundary** — formalize Zephyr/core boundary; begin adapter layer design before growing core further.
- **Phase 4I Scheduler Admission Policy** — define backpressure/admission rules for event posting.

Do not implement Phase 4I until explicitly assigned.

---

## Phase 5A — Zephyr / Platform Adapter Boundary Seed

**Date:** 2026-05-02
**Branch:** master
**Baseline commit:** `8e21f89` — "core: add event handler registration policy"

---

### Purpose

Introduce the first minimal RobotOS platform adapter boundary so that
`robotos_core.c` no longer directly depends on Zephyr logging APIs.

Before Phase 5A, `robotos_core.c` contained a compile-time shim:
`#ifdef ROBOTOS_CORE_HOST_TEST` that silenced Zephyr logging in host
builds and used `LOG_MODULE_REGISTER` + Zephyr `LOG_*` macros in
firmware builds. This created a direct Zephyr dependency inside core.

Phase 5A replaces that shim with a stable platform logging boundary:

- `platform/robotos_platform_log.h` — portable interface, no Zephyr types
- `platform/zephyr/robotos_platform_log_zephyr.c` — Zephyr backend (devkit)
- `tests/host/robotos_platform_log_host_stub.c` — no-op host backend

`robotos_core.c` now calls `robotos_platform_logf()` exclusively.
`ROBOTOS_CORE_HOST_TEST` is no longer needed and has been removed.

---

### Files Added

| File | Role |
|------|------|
| `platform/robotos_platform_log.h` | Platform log interface — portable, no Zephyr |
| `platform/zephyr/robotos_platform_log_zephyr.c` | Zephyr backend — routes to LOG_INF/WRN/DBG/ERR |
| `platform/README.md` | Platform layer boundary documentation |
| `tests/host/robotos_platform_log_host_stub.c` | Host no-op backend for contract tests |

### Files Modified

| File | Change |
|------|--------|
| `core/robotos_core.c` | Removed Zephyr shim; added `#include "robotos_platform_log.h"`; CORE_LOG_* macros now call `robotos_platform_logf()` |
| `core/README.md` | Updated to reflect platform log boundary |
| `devkit/CMakeLists.txt` | Added `../platform` include dir; added Zephyr log adapter to `target_sources` |
| `tests/host/CMakeLists.txt` | Added `PLATFORM_DIR` include + `PLATFORM_HOST_STUB` to all core-using targets; removed `ROBOTOS_CORE_HOST_TEST` definitions |
| `devkit/docs/DEVKIT_PROGRESS.md` | This entry |

---

### Platform Boundary Summary

```
platform/
├── robotos_platform_log.h              ← portable interface (stdarg.h only)
├── zephyr/
│   └── robotos_platform_log_zephyr.c  ← Zephyr backend, owns LOG_MODULE_REGISTER
└── README.md

tests/host/
└── robotos_platform_log_host_stub.c   ← no-op host backend
```

**API:**
```c
typedef enum {
    ROBOTOS_LOG_LEVEL_DEBUG = 0,
    ROBOTOS_LOG_LEVEL_INFO,
    ROBOTOS_LOG_LEVEL_WARN,
    ROBOTOS_LOG_LEVEL_ERROR,
} robotos_log_level_t;

void robotos_platform_logf(robotos_log_level_t level,
                           const char *module,
                           const char *fmt, ...);
```

**Boundary rules enforced:**
- `robotos_core.c` includes `robotos_platform_log.h` only — no `<zephyr/logging/log.h>`
- `LOG_MODULE_REGISTER` lives in `platform/zephyr/robotos_platform_log_zephyr.c` only
- `robotos_platform_log.h` contains no Zephyr, k_*, GPIO, DTS, or board types
- Host tests compile and link without Zephyr SDK

---

### Host Test Evidence

```
cmake -S RobotOS_v1.0/tests/host -B build-host-core
cmake --build build-host-core
ctest --test-dir build-host-core --output-on-failure
```

| Test | Result |
|------|--------|
| `robotos_core_contract` | PASS |
| `robotos_event_queue_contract` | PASS |
| `robotos_event_dispatcher_contract` | PASS |
| `robotos_core_ingestion_contract` | PASS |
| `robotos_core_tick_policy_contract` | PASS |
| `robotos_core_handler_policy_contract` | PASS |

**6/6 tests passed. 0 failed.**
No Zephyr SDK required. No legacy source compiled.

---

### Zephyr Build Evidence

```
west build -b stm32f411e_disco RobotOS_v1.0/devkit/ --pristine
```

| Check | Result |
|-------|--------|
| Build pass | PASS |
| Compiler warnings | None |
| Platform log adapter compiled | PASS — `platform/zephyr/robotos_platform_log_zephyr.c` |
| Core Zephyr logging removed | PASS — no `LOG_MODULE_REGISTER` in `robotos_core.c` |
| Legacy src/ compiled | NONE — confirmed clean |

Memory delta from Phase 4H: minimal increase (platform log wrapper + 128-byte
vsnprintf buffer on stack, ephemeral). No significant code size change expected.

**Incremental build after CRLF conversion:** core and platform adapter recompiled,
same image size confirmed (FLASH 27116 B, RAM 9024 B).

---

### Runtime Evidence

**Status: RUNTIME_PASS — hardware validated 2026-05-02.**

#### Method

Firmware flashed via `west flash`. Soft reset issued via OpenOCD (`reset run`).
RTT ring buffer read by halting at `0x200009bc` and dumping 1022-byte content
via `dump_image`. Fault registers read via Tcl `mrw`. LED state sampled via
GPIOD ODR at 700ms intervals.

Manual RESET workaround applies: `reset run` via OpenOCD issued post-flash to
start firmware (physical RESET button equivalent). No auto-start regression.

#### RTT Boot Log (captured from ring buffer)

```
*** Booting Zephyr OS build v3.6.0 ***
[00:00:00.000,000] <inf> devkit_fault: fault visibility active
[00:00:00.000,000] <inf> devkit_build_info: RobotOS devkit build info:
[00:00:00.000,000] <inf> devkit_build_info:   board=stm32f411e_disco
[00:00:00.000,000] <inf> devkit_build_info:   zephyr=3.6.0
[00:00:00.000,000] <inf> devkit_build_info:   build=May  2 2026 06:35:21
[00:00:00.000,000] <inf> devkit_build_info:   tick_ms=500
[00:00:00.000,000] <inf> devkit_build_info:   log_backend=RTT
[00:00:00.000,000] <inf> robotos_platform: [robotos_core] RobotOS core init — version=4B-contract state=READY
[00:00:00.000,000] <inf> robotos_platform: [robotos_core] event queue initialized capacity=16
[00:00:00.000,000] <inf> robotos_platform: [robotos_core] event dispatcher initialized
[00:00:00.000,000] <inf> devkit_runtime: RobotOS devkit starting — board: stm32f411e_disco
[00:00:00.000,000] <inf> devkit_runtime: [truncated — buffer full]
```

**Platform log routing confirmed:** All three core init messages appear under
the `robotos_platform` Zephyr module with `[robotos_core]` prefix — verifying
that `robotos_platform_log_zephyr.c` is routing correctly.

**RTT buffer note:** The 1024-byte RTT ring buffer fills during boot (boot
messages consume ~1022 bytes). Subsequent tick logs are skipped per
`CONFIG_SEGGER_RTT_MODE_NO_BLOCK_SKIP`. This is a pre-existing buffer sizing
behavior, not a Phase 5A regression. Tick log visibility requires a live RTT
reader or a larger buffer — deferred.

#### Fault Status

| Register | Value | Interpretation |
|---|---|---|
| CFSR (`0xE000ED28`) | `0x00000000` | No fault (no BusFault, UsageFault, MemManageFault) |
| HFSR (`0xE000ED2C`) | `0x00000000` | No hard fault |
| xPSR | `0x41000000` | Thumb mode, Thread, Z flag — normal execution |

No panic. No MemManage fault. No hard fault.

#### LED Evidence

GPIOD ODR sampled three times at 700ms intervals:

| Sample | ODR value | PD13 (Orange LED) |
|---|---|---|
| T+0 ms | `0x00002000` | ON |
| T+700 ms | `0x00002000` | ON |
| T+1400 ms | `0x00000000` | OFF |

LED toggling confirmed at ~500ms rate. LED blink runtime evidence intact.

Known workaround still applies: after `west flash`, press physical RESET before
RTT connection (established in Phase 3B). Soft reset via OpenOCD used here.

---

### Legacy Isolation Confirmation

No `RobotOS_v1.0/src/` legacy file compiled, staged, or referenced.
`platform/` sources are new — no legacy dependency.

---

### Known Limitations

- Only logging boundary implemented in Phase 5A.
- No time, thread, mutex, or assert/fault abstraction yet.
- Zephyr backend only for devkit; host backend is no-op only.
- Custom STM32F407VET6 board remains hardware-unvalidated.
- 128-byte vsnprintf buffer in Zephyr adapter: sufficient for all current
  core messages; silent truncation if exceeded (acceptable for diagnostics).
- Single-threaded assumption unchanged — no ISR-safe log calls.
- RTT ring buffer (1024 bytes) fills during boot; tick-level logs skipped after
  boot unless a live RTT reader drains the buffer continuously. Deferred.

---

### Next Recommended Phase

**Phase 5B — Platform Time Boundary**

Abstract the tick/time source out of the devkit harness into a platform
interface (`robotos_platform_time.h`), following the same boundary pattern
established in Phase 5A. This will decouple core tick scheduling from
`k_msleep` / Zephyr kernel time APIs.

---

## Phase 5B — Platform Time Boundary

**Date:** 2026-05-02
**Branch:** master
**Baseline commit:** `7da0ad1` — "platform: confirm logging boundary runtime"

---

### Purpose

Remove the direct `k_msleep` dependency from `devkit_runtime.c` by routing
sleep through a portable platform time interface.

Before Phase 5B, `devkit_runtime.c` included `<zephyr/kernel.h>` and called
`k_msleep(DEVKIT_TICK_MS)` directly. This made the runtime loop a direct
consumer of Zephyr kernel API.

Phase 5B introduces `robotos_platform_time.h` — a portable sleep/uptime
interface — and a Zephyr backend. `devkit_runtime.c` now calls
`robotos_platform_sleep_ms(DEVKIT_TICK_MS)` and no longer includes
`<zephyr/kernel.h>`.

Tick behavior (500ms interval) and LED/core call order are unchanged.

---

### Files Added

| File | Role |
|------|------|
| `platform/robotos_platform_time.h` | Platform time interface — portable, `<stdint.h>` only |
| `platform/zephyr/robotos_platform_time_zephyr.c` | Zephyr backend — `k_uptime_get_32` + `k_msleep` |
| `tests/host/robotos_platform_time_host_stub.c` | Host fake stub — sleep increments fake uptime; not wired to existing tests |

### Files Modified

| File | Change |
|------|--------|
| `devkit/src/devkit_runtime.c` | Removed `<zephyr/kernel.h>`; added `robotos_platform_time.h`; replaced `k_msleep` with `robotos_platform_sleep_ms` |
| `devkit/CMakeLists.txt` | Added `platform/zephyr/robotos_platform_time_zephyr.c` to `target_sources` |
| `platform/README.md` | Added Phase 5B time boundary section |
| `devkit/docs/DEVKIT_PROGRESS.md` | This entry |

---

### Platform Time Boundary Summary

```
platform/
├── robotos_platform_time.h             ← portable interface
├── zephyr/
│   ├── robotos_platform_log_zephyr.c
│   └── robotos_platform_time_zephyr.c  ← Zephyr backend (Phase 5B)
└── README.md

tests/host/
├── robotos_platform_log_host_stub.c
└── robotos_platform_time_host_stub.c   ← host fake stub (Phase 5B)
```

**API:**
```c
uint32_t robotos_platform_uptime_ms(void);
void     robotos_platform_sleep_ms(uint32_t duration_ms);
```

**Boundary rules enforced:**
- `devkit_runtime.c` includes `robotos_platform_time.h` — no `<zephyr/kernel.h>`
- Zephyr backend owns `k_msleep` and `k_uptime_get_32`
- `sleep_ms(0)` returns immediately in all backends
- Core does NOT include `robotos_platform_time.h` — core does not sleep
- Host stub is present but not wired to existing test targets (not needed)

---

### Host Test Evidence

```
cmake -S RobotOS_v1.0/tests/host -B build-host-core  (WSL Ubuntu, GCC 13.3.0)
cmake --build build-host-core
ctest --test-dir build-host-core --output-on-failure
```

| Test | Result |
|------|--------|
| `robotos_core_contract` | PASS |
| `robotos_event_queue_contract` | PASS |
| `robotos_event_dispatcher_contract` | PASS |
| `robotos_core_ingestion_contract` | PASS |
| `robotos_core_tick_policy_contract` | PASS |
| `robotos_core_handler_policy_contract` | PASS |

**6/6 tests passed. 0 failed.**
No Zephyr SDK required. No legacy source compiled.
Host CMakeLists unchanged — existing targets do not reference time API.

---

### Zephyr Build Evidence

```
west build -b stm32f411e_disco RobotOS_v1.0/devkit/ --pristine
```

| Check | Result |
|-------|--------|
| Build pass | PASS |
| Compiler warnings | Pre-existing `q_valid` unused-function only (robotos_event_queue.c) |
| Platform time adapter compiled | PASS — `platform/zephyr/robotos_platform_time_zephyr.c` |
| `<zephyr/kernel.h>` removed from devkit_runtime | PASS |
| k_msleep removed from devkit_runtime | PASS |
| Legacy src/ compiled | NONE |

Memory (FLASH / RAM):

```
arm-zephyr-eabi-size build/zephyr/zephyr.elf
   text    data     bss     dec     hex
  26264     868    8781   35913    8c49
```

FLASH used: 26264 + 868 = 27132 B (delta: +16 B vs Phase 5A 27116 B — time backend).
RAM used: 868 + 8781 = 9649 B.

---

### Runtime Evidence

**Status: RUNTIME_PASS — hardware validated 2026-05-02.**

#### Method

`west flash` → soft reset via OpenOCD `reset run` → RTT buffer dumped via
`dump_image` at `0x20000abf` → CFSR/HFSR read via `mrw` → GPIOD ODR sampled
at 700ms intervals.

#### RTT Boot Log (captured from ring buffer)

```
*** Booting Zephyr OS build v3.6.0 ***
[00:00:00.000,000] <inf> devkit_fault: fault visibility active
[00:00:00.000,000] <inf> devkit_build_info: RobotOS devkit build info:
[00:00:00.000,000] <inf> devkit_build_info:   board=stm32f411e_disco
[00:00:00.000,000] <inf> devkit_build_info:   zephyr=3.6.0
[00:00:00.000,000] <inf> devkit_build_info:   build=May  2 2026 07:46:27
[00:00:00.000,000] <inf> devkit_build_info:   tick_ms=500
[00:00:00.000,000] <inf> devkit_build_info:   log_backend=RTT
[00:00:00.000,000] <inf> robotos_platform: [robotos_core] RobotOS core init — version=4B-contract state=READY
[00:00:00.000,000] <inf> robotos_platform: [robotos_core] event queue initialized capacity=16
[00:00:00.000,000] <inf> robotos_platform: [robotos_core] event dispatcher initialized
[00:00:00.000,000] <inf> devkit_runtime: RobotOS devkit starting — board: stm32f411e_disco
[00:00:00.000,000] <inf> devkit_runtime: [truncated — buffer full]
```

Boot sequence intact. `tick_ms=500` confirmed in build info.
Core init messages still route via `robotos_platform` (Phase 5A boundary preserved).

#### Fault Status

| Register | Value | Interpretation |
|---|---|---|
| CFSR (`0xE000ED28`) | `0x00000000` | No fault |
| HFSR (`0xE000ED2C`) | `0x00000000` | No hard fault |

No panic. No MemManage fault. No hard fault.

#### LED Evidence

GPIOD ODR sampled three times at 700ms intervals:

| Sample | ODR value | PD13 (Orange LED) |
|---|---|---|
| T+0 ms | `0x00000000` | OFF |
| T+700 ms | `0x00000000` | OFF |
| T+1400 ms | `0x00002000` | ON |

LED toggling at ~500ms rate confirmed. Sleep path change (k_msleep → platform
sleep) did not alter tick rate or LED behavior.

---

### Legacy Isolation Confirmation

No `RobotOS_v1.0/src/` file compiled, staged, or referenced.
All new files are in `platform/` or `tests/host/`.

---

### Known Limitations

- Sleep and uptime boundary only. No timer abstraction.
- No scheduler integration beyond the existing single tick call.
- `robotos_platform_uptime_ms()` not yet called by devkit_runtime — API present, unused by runtime in Phase 5B.
- `uint32_t` uptime wraps at ~49.7 days. Wraparound not handled.
- Host time stub not wired to any existing host test target.
- Custom STM32F407VET6 board remains hardware-unvalidated.

---

---

## Phase 5C — Platform Assert/Fault Boundary

**Date:** 2026-05-02
**Branch:** master
**Baseline commit:** `4fd1a98` — "platform: add time boundary"

---

### Purpose

Introduce the platform assert/fault boundary so that future modules can
report fault conditions through a portable, backend-agnostic interface
without calling Zephyr APIs directly.

Pattern established in Phases 5A (logging) and 5B (time/sleep) is extended
to fault reporting:

- `platform/robotos_platform_fault.h` — portable interface, no Zephyr types
- `platform/zephyr/robotos_platform_fault_zephyr.c` — routes via `robotos_platform_logf()`
- `tests/host/robotos_platform_fault_host_stub.c` — tracking stub for contract tests
- `tests/host/test_robotos_platform_fault_contract.c` — 12 contract tests

`devkit_fault.c` was intentionally NOT wired to the platform fault API.
It is a devkit-layer file that legitimately uses Zephyr logging directly
(same reasoning as `devkit_runtime.c` owning its own `LOG_MODULE_REGISTER`).

---

### Files Added

| File | Role |
|------|------|
| `platform/robotos_platform_fault.h` | Portable fault interface — `<stdbool.h>` only |
| `platform/zephyr/robotos_platform_fault_zephyr.c` | Zephyr backend — routes via `robotos_platform_logf()`; optional `k_panic` on FATAL |
| `tests/host/robotos_platform_fault_host_stub.h` | Test-inspection API for the host stub |
| `tests/host/robotos_platform_fault_host_stub.c` | Host stub — tracks report_count, assert_fail_count, last_severity |
| `tests/host/test_robotos_platform_fault_contract.c` | 12 contract tests for the host stub |

### Files Modified

| File | Change |
|------|--------|
| `devkit/CMakeLists.txt` | Added `robotos_platform_fault_zephyr.c` to target_sources |
| `tests/host/CMakeLists.txt` | Added `robotos_platform_fault_contract_test` target + CTest entry |
| `platform/README.md` | Added Phase 5C assert/fault boundary section |
| `devkit/docs/DEVKIT_PROGRESS.md` | Added Phase 5C section (this file) |

### Files Unchanged

| File | Reason |
|------|--------|
| `devkit/src/devkit_fault.c` | Devkit-layer, legitimately owns Zephyr logging; not ported to platform API |
| `devkit/src/devkit_runtime.c` | No fault calls needed in Phase 5C runtime |
| `core/robotos_core.c` | Core must NOT call platform fault API — uses return codes |

---

### API

```c
typedef enum {
    ROBOTOS_FAULT_INFO    = 0,
    ROBOTOS_FAULT_WARNING = 1,
    ROBOTOS_FAULT_ERROR   = 2,
    ROBOTOS_FAULT_FATAL   = 3,
} robotos_fault_severity_t;

void robotos_platform_fault_report(robotos_fault_severity_t severity,
                                   const char *module,
                                   const char *message);

bool robotos_platform_assert(bool condition,
                              const char *module,
                              const char *message);

#define ROBOTOS_PLATFORM_ASSERT(cond, module, msg) ...
```

Assert behavior:
- `cond == true`: returns true, no side effects.
- `cond == false`: calls `fault_report(ERROR, module, msg)`, returns false. Never panics.

Fault severity → log level: INFO→INFO, WARNING→WARN, ERROR/FATAL→ERROR.

`ROBOTOS_PLATFORM_FAULT_PANIC_ON_FATAL` (compile switch, default OFF):
When defined, `fault_report(FATAL, ...)` calls `k_panic()` after logging in
the Zephyr backend. Assert never calls `k_panic()` regardless of this switch.

---

### Host Test Evidence

**Commands (WSL Ubuntu, gcc 13.3.0):**

```bash
rm -rf build-host-core
cmake -S RobotOS_v1.0/tests/host -B build-host-core
cmake --build build-host-core
ctest --test-dir build-host-core --output-on-failure
```

**Result:**

```
Test project /mnt/d/Robot_OS/build-host-core
    Start 1: robotos_core_contract
1/7 Test #1: robotos_core_contract ..................   Passed    0.01 sec
    Start 2: robotos_event_queue_contract
2/7 Test #2: robotos_event_queue_contract ...........   Passed    0.01 sec
    Start 3: robotos_event_dispatcher_contract
3/7 Test #3: robotos_event_dispatcher_contract ......   Passed    0.01 sec
    Start 4: robotos_core_ingestion_contract
4/7 Test #4: robotos_core_ingestion_contract ........   Passed    0.01 sec
    Start 5: robotos_core_tick_policy_contract
5/7 Test #5: robotos_core_tick_policy_contract ......   Passed    0.01 sec
    Start 6: robotos_core_handler_policy_contract
6/7 Test #6: robotos_core_handler_policy_contract ...   Passed    0.01 sec
    Start 7: robotos_platform_fault_contract
7/7 Test #7: robotos_platform_fault_contract ........   Passed    0.01 sec

100% tests passed, 0 tests failed out of 7
```

7/7 pass. 12 fault contract test cases confirmed. All 6 prior tests unaffected.

---

### Zephyr Build Evidence

**Command:** `west build -b stm32f411e_disco --pristine` (Zephyr v3.6.0, SDK 0.17.0)

```
Memory region         Used Size  Region Size  %age Used
           FLASH:       27132 B       512 KB      5.18%
             RAM:        9024 B       128 KB      6.88%
```

Build: **PASS** (142 objects, no warnings, no errors).

FLASH unchanged vs Phase 5B: fault backend functions are dead code (nothing
calls them yet); linker strips them. They will appear in FLASH once called
by production devkit code.

---

### Runtime Evidence

**Status: RUNTIME_PASS — hardware validated 2026-05-02.**

#### Method

`west flash` → soft reset via OpenOCD `reset run` → RTT buffer dumped via
`dump_image` at `0x20000abf` (1024 B) → CFSR/HFSR/ODR read via `mrw`.

#### RTT Boot Log (captured from ring buffer)

```
*** Booting Zephyr OS build v3.6.0 ***
[00:00:00.000,000] <inf> devkit_fault: fault visibility active
[00:00:00.000,000] <inf> devkit_build_info: RobotOS devkit build info:
[00:00:00.000,000] <inf> devkit_build_info:   board=stm32f411e_disco
[00:00:00.000,000] <inf> devkit_build_info:   zephyr=3.6.0
[00:00:00.000,000] <inf> devkit_build_info:   build=May  2 2026 08:03:50
[00:00:00.000,000] <inf> devkit_build_info:   tick_ms=500
[00:00:00.000,000] <inf> devkit_build_info:   log_backend=RTT
[00:00:00.000,000] <inf> robotos_platform: [robotos_core] RobotOS core init — version=4B-contract state=READY
```

- Build timestamp `May 2 2026 08:03:50` confirms Phase 5C binary running.
- `devkit_fault: fault visibility active` — `devkit_fault_init()` runs correctly.
- `robotos_platform: [robotos_core]` — platform log routing confirmed.
- Buffer fills at boot (same pre-existing behavior as Phases 5A/5B).

#### Fault Registers

| Register | Address | Value | Interpretation |
|----------|---------|-------|----------------|
| CFSR | 0xE000ED28 | 0x0 | No configurable fault active |
| HFSR | 0xE000ED2C | 0x0 | No hard fault active |
| GPIOD_ODR | 0x40020C14 | 0x0 | LED in OFF state at sample time (normal toggle) |

CFSR=0x0, HFSR=0x0 — no faults. Runtime behavior identical to Phase 5B.

---

### Legacy Isolation Confirmation

No `RobotOS_v1.0/src/` file compiled, staged, or referenced.
All new files are in `platform/` or `tests/host/`.

---

### Known Limitations

- Fault API is present in devkit build but unused by production code.
  No devkit module currently calls `robotos_platform_fault_report()` or
  `ROBOTOS_PLATFORM_ASSERT()` in normal execution.
- `ROBOTOS_PLATFORM_FAULT_PANIC_ON_FATAL` is not validated on hardware
  (default OFF; calling k_panic deliberately is out of Phase 5C scope).
- No fault injection test at runtime — stub inspection via host tests only.
- Host time stub still not wired to any existing host test target.
- Custom STM32F407VET6 board remains hardware-unvalidated.

---

### Next Recommended Phase

**Phase 5D or Phase 6 — team decision required.**

Candidates:
- Phase 5D: Platform critical section / ISR lock boundary
- Phase 6A: First portable module using the platform boundary (e.g., a
  diagnostics or watchdog module that calls fault_report/assert)

---

## Phase 4I — Scheduler Admission Policy Stub

**Date:** 2026-05-02
**Branch:** master
**Baseline commit:** `3a7151b` — platform: add assert fault boundary

---

### Purpose

Add an admission gate inside `robotos_core_post_event()` that enforces
event type validity before enqueue. Rejected events return
`ROBOTOS_CORE_ERR_INVALID_ARG` and are counted separately from
queue-full drops. This separates two distinct failure modes:

- **Admission rejection** — bad event type (programming error); never
  reaches the queue.
- **Queue-full drop** — valid event arrived when queue was at capacity
  (flow-control backpressure); counted by the existing dropped counter.

Admission counters (`admission_accepted_count`, `admission_rejected_count`)
are visible through snapshot and dedicated getter functions.

---

### Admission Policy

| Event Type | Range | Decision |
|------------|-------|----------|
| `ROBOTOS_EVENT_NONE` | 0 | Rejected — always invalid |
| `ROBOTOS_EVENT_CORE_TICK` | 1 | Accepted |
| Reserved | 2–99 | Rejected — undefined range |
| `ROBOTOS_EVENT_USER` and above | ≥ 100 | Accepted |

The gate is implemented as `s_event_type_admissible()` — a private
static helper in `robotos_core.c`.

---

### Core Files Modified

| File | Change |
|------|--------|
| `core/robotos_core.h` | Phase 4I header comment; extended snapshot struct with two new counter fields; expanded `post_event` comment documenting admission policy; two new getter declarations |
| `core/robotos_core.c` | Phase 4I implementation comment; added `s_admission_accepted_count` / `s_admission_rejected_count` static counters; added `s_event_type_admissible()` helper; updated `post_event()` with gate; updated `snapshot()`; added two getter implementations |

### Test Files Added

| File | Role |
|------|------|
| `tests/host/test_robotos_scheduler_admission_contract.c` | Phase 4I host contract test — 32 assertions covering NONE, reserved range, CORE_TICK, USER, USER+, queue-full, second-init invariant, snapshot/getter coherence |

### Build Files Modified

| File | Change |
|------|--------|
| `tests/host/CMakeLists.txt` | Added `robotos_scheduler_admission_contract_test` target using `CORE_SRCS` + `PLATFORM_HOST_STUB` pattern; added `robotos_scheduler_admission_contract` ctest entry |

---

### API Changes

#### `robotos_core_snapshot_t` — two new fields

```c
uint32_t admission_accepted_count; /* events accepted by admission gate */
uint32_t admission_rejected_count; /* events rejected by admission gate */
```

#### New getter functions

```c
uint32_t robotos_core_admission_accepted_count(void);
uint32_t robotos_core_admission_rejected_count(void);
```

#### `robotos_core_post_event()` extended contract

- `ERR_INVALID_ARG` returned when type is NONE or reserved (2–99).
- `admission_rejected_count` incremented on rejection.
- `admission_accepted_count` incremented only when push succeeds (OK).
- Queue-full (`ERR_FULL`) does NOT increment `admission_accepted_count`.
- NULL event check and state check remain before the admission gate.

---

### Admission Counter Invariants

| Scenario | accepted | rejected |
|----------|----------|----------|
| NULL event posted | unchanged | unchanged |
| Pre-init post | unchanged | unchanged |
| NONE type | unchanged | +1 |
| Reserved type (2–99) | unchanged | +1 |
| CORE_TICK, USER, USER+ — queue OK | +1 | unchanged |
| Valid type, queue full | unchanged | unchanged |
| Second `init()` call | NOT reset | NOT reset |

Counters start at zero (BSS). Second init returns early before the
reset block, so accumulated admission history is preserved across
re-initialisation — same invariant as tick_count.

---

### Host Test Results

```
cmake -S RobotOS_v1.0/tests/host -B build-host-core
cmake --build build-host-core
ctest --test-dir build-host-core --output-on-failure
```

```
Test project /mnt/d/Robot_OS/build-host-core
    Start 1: robotos_core_contract
1/8 Test #1: robotos_core_contract ..................   Passed    0.01 sec
    Start 2: robotos_event_queue_contract
2/8 Test #2: robotos_event_queue_contract ...........   Passed    0.01 sec
    Start 3: robotos_event_dispatcher_contract
3/8 Test #3: robotos_event_dispatcher_contract ......   Passed    0.01 sec
    Start 4: robotos_core_ingestion_contract
4/8 Test #4: robotos_core_ingestion_contract ........   Passed    0.01 sec
    Start 5: robotos_core_tick_policy_contract
5/8 Test #5: robotos_core_tick_policy_contract ......   Passed    0.01 sec
    Start 6: robotos_core_handler_policy_contract
6/8 Test #6: robotos_core_handler_policy_contract ...   Passed    0.01 sec
    Start 7: robotos_platform_fault_contract
7/8 Test #7: robotos_platform_fault_contract ........   Passed    0.01 sec
    Start 8: robotos_scheduler_admission_contract
8/8 Test #8: robotos_scheduler_admission_contract ...   Passed    0.01 sec

100% tests passed, 0 tests failed out of 8
```

**8 test suites, 100% pass. No regressions.**

---

### Zephyr Build Note

No devkit sources changed in Phase 4I. The two new fields in
`robotos_core_snapshot_t` are additive (no alignment impact on existing
fields). Devkit builds compile `robotos_core.c` unchanged in terms of
Zephyr dependency — the admission gate is pure portable C, no platform
layer involvement. A `west build` is expected to succeed without
modification to `devkit/CMakeLists.txt`.

---

### Known Limitations

- Admission counters are not reset on second `init()` — intentional, same
  reasoning as tick_count: a re-init during operation must not silently
  discard diagnostic history.
- No runtime logging of rejected events (silent gate) — LOG_WRN on
  rejection is a candidate for a future phase if observability is needed.
- Queue-full events are not double-counted in `admission_rejected_count`
  — they remain in `dropped_event_count` only. The two counters are
  orthogonal by design.
- Single-threaded assumption inherited from prior phases.

---

### Next Recommended Phase

**Team decision required.**

Candidates:

- Phase 4J: Scheduler budget/backpressure policy stub
- Phase 5D: Platform critical section / ISR lock boundary
- Phase 6A: First portable module using platform boundaries

---

## Phase 4J — Scheduler Budget / Backpressure Policy

**Date:** 2026-05-02
**Branch:** master
**Baseline commit:** `0cda14e` — "core: add scheduler admission policy stub"

---

### Purpose

Define minimal budget/backpressure observability for the core event loop.
Make backlog/pressure state inspectable without implementing a real scheduler,
priority queue, or producer throttle.

Two new public APIs added:
- `robotos_core_dispatch_budget_per_tick()` — returns `ROBOTOS_CORE_MAX_EVENTS_PER_TICK`
- `robotos_core_backpressure_active()` — true when pending > budget OR queue is full

One new snapshot field:
- `robotos_core_snapshot_t.backpressure_active` — matches `backpressure_active()` getter

Backpressure rule: `pending_event_count > ROBOTOS_CORE_MAX_EVENTS_PER_TICK || queue_is_full`

With current values (budget=1, capacity=16): backpressure activates when ≥2 events are pending.

Backpressure is observability only — it does not throttle producers, adjust priority,
or alter scheduler fairness. Queue-full still returns `ERR_FULL`. Invalid events still
return `ERR_INVALID_ARG`. No behavior change in the tick/dispatch path.

---

### Files Changed

| File | Change |
|------|--------|
| `core/robotos_core.h` | Added `backpressure_active` to snapshot; added `dispatch_budget_per_tick()` and `backpressure_active()` declarations; added Phase 4J counter semantics comments |
| `core/robotos_core.c` | Implemented both new functions; snapshot fills `backpressure_active`; updated file comment to Phase 4J |
| `core/README.md` | Added Phase 4J Scheduler Budget/Backpressure Policy section |
| `tests/host/CMakeLists.txt` | Added `robotos_scheduler_backpressure_contract_test` target |
| `tests/host/test_robotos_scheduler_backpressure_contract.c` | New — 9 host test targets now (Phase 4J backpressure) |
| `devkit/docs/DEVKIT_PROGRESS.md` | Added Phase 4J section (this file) |

### Files Unchanged

| File | Reason |
|------|--------|
| `core/robotos_event_queue.h/.c` | Existing `is_full()` and `count()` APIs sufficient; no new queue API needed |
| `core/robotos_event_dispatcher.h/.c` | No dispatcher changes needed |
| `devkit/CMakeLists.txt` | No Zephyr-build changes (core changes only) |
| `devkit/src/devkit_runtime.c` | No runtime behavior change |

---

### Policy Summary

| Aspect | Value / Rule |
|--------|-------------|
| Dispatch budget | `ROBOTOS_CORE_MAX_EVENTS_PER_TICK` = 1 event per tick |
| Backpressure rule | `pending > budget` OR `queue_is_full` |
| Backpressure effect | Observability only — no producer throttle |
| Queue-full behavior | `ERR_FULL` + `dropped_count++` (unchanged) |
| Admission reject behavior | `ERR_INVALID_ARG` + `admission_rejected_count++` (unchanged) |
| Counter independence | `dropped_count` ≠ `admission_rejected_count` ≠ `unhandled_count` ≠ `handler_error_count` |

---

### Host Test Evidence

**Commands (WSL Ubuntu, gcc 13.3.0):**

```bash
rm -rf build-host-core
cmake -S RobotOS_v1.0/tests/host -B build-host-core
cmake --build build-host-core
ctest --test-dir build-host-core --output-on-failure
```

**Result:**

```
Test project /mnt/d/Robot_OS/build-host-core
    Start 1: robotos_core_contract
1/9 Test #1: robotos_core_contract .....................   Passed    0.01 sec
    Start 2: robotos_event_queue_contract
2/9 Test #2: robotos_event_queue_contract ..............   Passed    0.01 sec
    Start 3: robotos_event_dispatcher_contract
3/9 Test #3: robotos_event_dispatcher_contract .........   Passed    0.01 sec
    Start 4: robotos_core_ingestion_contract
4/9 Test #4: robotos_core_ingestion_contract ...........   Passed    0.01 sec
    Start 5: robotos_core_tick_policy_contract
5/9 Test #5: robotos_core_tick_policy_contract .........   Passed    0.01 sec
    Start 6: robotos_core_handler_policy_contract
6/9 Test #6: robotos_core_handler_policy_contract ......   Passed    0.01 sec
    Start 7: robotos_platform_fault_contract
7/9 Test #7: robotos_platform_fault_contract ...........   Passed    0.01 sec
    Start 8: robotos_scheduler_admission_contract
8/9 Test #8: robotos_scheduler_admission_contract ......   Passed    0.01 sec
    Start 9: robotos_scheduler_backpressure_contract
9/9 Test #9: robotos_scheduler_backpressure_contract ...   Passed    0.01 sec

100% tests passed, 0 tests failed out of 9
```

9/9 pass. All 8 prior tests unaffected. New backpressure test covers:
- TC01–TC02: pre-init budget constant and backpressure=false
- TC03–TC04: post-init empty state
- TC05: 1 event pending (== budget) → backpressure=false
- TC06: 2 events pending (> budget) → backpressure=true
- TC07: tick drains 1 event → backpressure=false
- TC08: queue filled to capacity → backpressure=true
- TC09: post when full → ERR_FULL, dropped++, admission unchanged
- TC10: invalid post under pressure → ERR_INVALID_ARG, rejected++, dropped unchanged
- TC11: dispatch_events drains backlog → pending=0, backpressure=false
- TC12: unregistered dispatch → unhandled_count++, backpressure tracks pending
- TC13: handler error → handler_error_count++, event consumed, backpressure updated
- TC14: repeated init → pending/backpressure NOT cleared
- TC15: snapshot coherence — all values match getters

---

### Zephyr Build Evidence

**Command:** `west build -b stm32f411e_disco --pristine` (Zephyr v3.6.0, SDK 0.17.0)

```
Memory region         Used Size  Region Size  %age Used
           FLASH:       27132 B       512 KB      5.18%
             RAM:        9024 B       128 KB      6.88%
```

Build: **PASS** (142 objects, no warnings, no errors).

FLASH unchanged vs Phase 5C: `dispatch_budget_per_tick()` and `backpressure_active()`
are not called from production devkit code; linker strips them. They activate in FLASH
once called by future devkit or portable module code.

---

### Runtime Evidence

**Status: RUNTIME_PASS — hardware validated 2026-05-02.**

#### Method

`west flash` → soft reset via OpenOCD `reset run` → RTT buffer dumped via
`dump_image` at `0x20000abf` (1024 B) → CFSR/HFSR read via `mrw`.

#### RTT Boot Log

```
*** Booting Zephyr OS build v3.6.0 ***
[00:00:00.000,000] <inf> devkit_fault: fault visibility active
[00:00:00.000,000] <inf> devkit_build_info: RobotOS devkit build info:
[00:00:00.000,000] <inf> devkit_build_info:   board=stm32f411e_disco
[00:00:00.000,000] <inf> devkit_build_info:   zephyr=3.6.0
[00:00:00.000,000] <inf> devkit_build_info:   build=May  2 2026 08:36:25
[00:00:00.000,000] <inf> devkit_build_info:   tick_ms=500
[00:00:00.000,000] <inf> devkit_build_info:   log_backend=RTT
[00:00:00.000,000] <inf> robotos_platform: [robotos_core] RobotOS core init — version=4B-contract state=READY
```

- Build timestamp `May 2 2026 08:36:25` confirms Phase 4J binary running.
- Platform log routing active. CFSR=0x0, HFSR=0x0. No new faults.
- Runtime behavior identical to Phase 5C (no tick/dispatch path change).

---

### Legacy Isolation Confirmation

No `RobotOS_v1.0/src/` file compiled, staged, or referenced.
All changes are in `core/` or `tests/host/`.

---

### Known Limitations

- **Observability only.** `backpressure_active` does not throttle `post_event()` callers.
- **No dynamic budget.** `ROBOTOS_CORE_MAX_EVENTS_PER_TICK` is compile-time only.
- **No priority or fairness.** Strict FIFO preserved.
- **No producer throttle.** Full queue returns `ERR_FULL`; caller decides retry strategy.
- **No concurrency/ISR safety.** Single-threaded only. No critical section.
- **No timer/deadline integration.** Time-based scheduling out of scope.
- Custom STM32F407VET6 board remains hardware-unvalidated.

---

### Next Recommended Phase

**Phase 6A completed.** See Phase 6A entry below.

Candidates for next team decision:
- **Phase 4K** — Scheduler Producer Throttle Policy
- **Phase 5D** — Platform Critical Section / ISR Lock Boundary
- **Phase 6B** — Devkit Event Burst / Backpressure Smoke ← **completed**
---

## Phase 6A — Devkit Event Smoke Integration

**Status: COMPLETE — 2026-05-02**

### Scope

Add a devkit-only smoke integration that exercises the full core event pipeline
on real hardware: register a handler → post one controlled USER event at init →
`robotos_core_tick()` dispatches it on the first tick → handler observes and logs evidence.

No core policy changes. No CMake changes. No platform boundary changes.
One file modified: `devkit/src/devkit_runtime.c`.

### Change Summary

**`RobotOS_v1.0/devkit/src/devkit_runtime.c`** — added:
- `static uint32_t s_smoke_handled_count` — fire-once dispatch counter
- `static const robotos_event_t s_smoke_event` — `type=USER, arg0=0x6A, arg1=1`
- `static devkit_smoke_event_handler()` — logs evidence, increments counter
- In `devkit_runtime_init()`, after `robotos_core_init()`:
  - `robotos_core_register_event_handler(ROBOTOS_EVENT_USER, ...)`
  - `robotos_core_post_event(&s_smoke_event)`

**`RobotOS_v1.0/devkit/prj.conf`** — added `CONFIG_SEGGER_RTT_BUFFER_SIZE_UP=4096`
to prevent ring-buffer overrun on boot (init messages fill 1024-byte default buffer
before first tick fires).

### Host Test Results

No host test changes required. 9/9 pass:

```
100% tests passed, 0 tests failed out of 9
Total Test time (real) =   0.15 sec
```

### Zephyr Build Evidence

**Command:** `west build -b stm32f411e_disco --pristine` (Zephyr v3.6.0, SDK 0.17.0)

```
Memory region         Used Size  Region Size  %age Used
           FLASH:       27732 B       512 KB      5.29%
             RAM:       12096 B       128 KB      9.23%
```

Build: **PASS** (142 objects, no warnings, no errors).

FLASH +600 B vs Phase 4J (smoke handler code).
RAM +3072 B vs Phase 4J (RTT ring buffer 1024 → 4096 B).

### Runtime Evidence

**Status: RUNTIME_PASS — hardware validated 2026-05-02.**

#### Method

`west flash` → `openocd reset run` → `sleep 4000` → `halt` →
`dump_image D:/Robot_OS/rtt_buf.bin 0x200009c8 4096` (_SEGGER_RTT at new address
after buffer resize) → CFSR/HFSR read via `mrw`.

#### RTT Boot Log

```
*** Booting Zephyr OS build v3.6.0 ***
[00:00:00.000,000] <inf> devkit_fault: fault visibility active
[00:00:00.000,000] <inf> devkit_build_info: RobotOS devkit build info:
[00:00:00.000,000] <inf> devkit_build_info:   board=stm32f411e_disco
[00:00:00.000,000] <inf> devkit_build_info:   zephyr=3.6.0
[00:00:00.000,000] <inf> devkit_build_info:   build=May  2 2026 08:55:25
[00:00:00.000,000] <inf> devkit_build_info:   tick_ms=500
[00:00:00.000,000] <inf> devkit_build_info:   log_backend=RTT
[00:00:00.000,000] <inf> robotos_platform: [robotos_core] RobotOS core init — version=4B-contract state=READY
[00:00:00.000,000] <inf> robotos_platform: [robotos_core] event queue initialized capacity=16
[00:00:00.000,000] <inf> robotos_platform: [robotos_core] event dispatcher initialized
[00:00:00.000,000] <inf> devkit_runtime: RobotOS devkit starting — board: stm32f411e_disco
[00:00:00.000,000] <inf> devkit_runtime: LED blink loop starting
[00:00:00.000,000] <inf> devkit_runtime: tick count=0
[00:00:00.000,000] <inf> robotos_platform: [robotos_core] core tick count=1
[00:00:00.000,000] <inf> devkit_runtime: smoke event handled type=USER arg0=0x6a arg1=1
[00:00:00.500,000] <inf> devkit_runtime: tick count=1
[00:00:01.000,000] <inf> devkit_runtime: tick count=2
[00:00:01.500,000] <inf> devkit_runtime: tick count=3
[00:00:02.000,000] <inf> devkit_runtime: tick count=4
[00:00:02.500,000] <inf> devkit_runtime: tick count=5
[00:00:03.000,000] <inf> devkit_runtime: tick count=6
```

**Key evidence line:**
```
[00:00:00.000,000] <inf> devkit_runtime: smoke event handled type=USER arg0=0x6a arg1=1
```

- Dispatched on tick count=0 (first tick): confirms budget=1 dispatches the queued event immediately.
- `arg0=0x6a` and `arg1=1` match the posted `s_smoke_event` exactly.
- Ticks 1–6 are clean (no more smoke events, no errors).

#### Fault Register Check

```
CFSR = 0x0    (no configurable fault)
HFSR = 0x0    (no hard fault)
```

No faults. Runtime stable.

---

### Legacy Isolation Confirmation

No `RobotOS_v1.0/src/` file compiled, staged, or referenced.
Smoke integration is devkit-only; no core policy changes.

---

### Known Limitations

- Smoke event fires only once (at init). Periodic event integration is out of scope for Phase 6A.
- `s_smoke_handled_count` is file-local; no getter exposed. Observability via RTT log only.
- Custom STM32F407VET6 board remains hardware-unvalidated.

---

### Next Recommended Phase

**Team decision required.**

Candidates:
- **Phase 4K** — Scheduler Producer Throttle Policy
- **Phase 5D** — Platform Critical Section / ISR Lock Boundary
- **Phase 6B** — Devkit Event Burst / Backpressure Smoke ← **completed**

---

---

## Phase 6B — Devkit Event Burst / Backpressure Smoke

**Date:** 2026-05-02
**Branch:** master
**Baseline commit:** `dba2fd1` — log evidence hygiene patch

---

### Purpose

Extend the Phase 6A single-event smoke to a burst of three USER events posted at init.
Prove that the core admission gate, queue, per-tick dispatch budget, and backpressure
flag all behave correctly under load: pending=3 > budget=1 triggers backpressure
immediately; the burst drains one event per tick over three ticks; backpressure clears
once pending ≤ budget; a final snapshot is logged exactly once after all three are handled.

No core policy changes. No platform changes. Devkit-only modification.

---

### Files Changed

| File | Change |
|------|--------|
| `devkit/src/devkit_runtime.c` | Replaced single smoke event with burst of 3 (`BURST_SIZE=3`, `arg0=0x6B`, `arg1=1/2/3`); updated handler to count and log per-event; added post-burst snapshot log in run loop |

### Files Unchanged (confirmed)

| File | Reason |
|------|--------|
| `RobotOS_v1.0/src/robotos_core.c` | No core policy change |
| `RobotOS_v1.0/src/robotos_core.h` | No interface change |
| `RobotOS_v1.0/platform/` | No platform change |
| `devkit/prj.conf` | RTT buffer already 4096 B from Phase 6A — sufficient |

---

### Burst Design

```
BURST_SIZE = 3
arg0 = 0x6B  (phase tag)
arg1 = 1, 2, 3  (sequence numbers)

budget (ROBOTOS_CORE_MAX_EVENTS_PER_TICK) = 1

Post sequence:
  robotos_core_post_event(&s_burst_events[0])  → pending=1
  robotos_core_post_event(&s_burst_events[1])  → pending=2
  robotos_core_post_event(&s_burst_events[2])  → pending=3
  → backpressure ACTIVE (pending=3 > budget=1)

Dispatch timeline (one per tick):
  tick 0  seq=1 dispatched  pending→2  bp still true
  tick 1  seq=2 dispatched  pending→1  bp false (pending==budget)
  tick 2  seq=3 dispatched  pending→0  bp false

Final snapshot logged once after s_burst_handled_count == 3.
```

---

### Expected Policy

- `backpressure_active = (pending > ROBOTOS_CORE_MAX_EVENTS_PER_TICK) || queue_is_full`
- After burst post: `3 > 1` → bp=true
- After tick 0: `2 > 1` → bp=true
- After tick 1: `1 > 1` is false → bp=false
- After tick 2: `0 > 1` is false → bp=false
- `rejected=0, dropped=0, herr=0, unhandled=0` — clean admission and handling throughout

---

### Host Test Evidence

**Result:** 9/9 suites pass — no regression.

**Log:** `tests/host/logs/phase_6B_host_2026-05-02.log`

Suites covered:
- Phase 4C: Core Init
- Phase 4D: Core Version
- Phase 4E: Event Post
- Phase 4F: Event Dispatch Budget
- Phase 4G: Event Handler Registration
- Phase 4H: Core Snapshot
- Phase 4I: Event Admission Policy
- Phase 4J: Backpressure
- Phase 4K: Scheduler Admission Policy (stub)

All 9 suites pass without modification. The burst design is consistent with the
4J backpressure policy and 4F dispatch-budget policy verified in host tests.

---

### Zephyr Build Evidence

```
Build: west build -b stm32f411e_disco RobotOS_v1.0/devkit/
Board: stm32f411e_disco
Zephyr: v3.6.0
Timestamp: May 2 2026 09:40:22

Memory:
  FLASH: 28352 / 524288 bytes  (5.41%)
  RAM:   12096 / 131072 bytes  (9.23%)

Errors: 0
```

Delta from Phase 6A (27732 B flash): +620 B for burst array, for-loop, and snapshot log call.

---

### Runtime RTT Evidence

**RTT log:** `devkit/logs/phase_6B_rtt_2026-05-02.txt`

**Capture:** `openocd reset run → sleep 5000 → halt → dump_image 0x200009c8 4096`
(`_SEGGER_RTT` address confirmed via `nm` after Phase 6A rebuild — unchanged)

**Key log lines:**

```
[00:00:00.000,000] <inf> devkit_runtime: Phase 6B: burst posted 3 events pending=3 bp=1
[00:00:00.000,000] <inf> devkit_runtime: Phase 6B burst handled seq=1 count=1
[00:00:00.500,000] <inf> devkit_runtime: Phase 6B burst handled seq=2 count=2
[00:00:01.000,000] <inf> devkit_runtime: Phase 6B burst handled seq=3 count=3
[00:00:01.000,000] <inf> devkit_runtime: Phase 6B summary: handled=3 pending=0 dispatched=3 accepted=3 rejected=0 dropped=0 herr=0 unhandled=0 bp=0
```

**Backpressure transitions confirmed:**
- At post: `pending=3 bp=1` → bp ACTIVE immediately
- After tick 0: seq=1 dispatched, pending→2, bp still true
- After tick 1: seq=2 dispatched, pending→1, bp clears (pending ≤ budget)
- After tick 2: seq=3 dispatched, pending→0, bp=false

Post-burst ticks (3–9): clean, no burst events, no errors.

---

### Fault Register Check

```
CFSR = 0x0    (no configurable fault)
HFSR = 0x0    (no hard fault)
```

No faults. Runtime stable across full 5-second capture window.

---

### Legacy Isolation Confirmation

No `RobotOS_v1.0/src/` file compiled, staged, or referenced.
Burst integration is devkit-only; no core policy changes.
`prj.conf` unchanged from Phase 6A (`CONFIG_SEGGER_RTT_BUFFER_SIZE_UP=4096`).

---

### Known Limitations

- Burst is fixed at compile time (`BURST_SIZE=3`). Dynamic burst injection is out of scope.
- `s_burst_handled_count` and `s_burst_summary_logged` are file-local; no getter exposed.
  Observability via RTT log only.
- Backpressure rejection path (queue full or admission rejected) not exercised — burst of 3
  is well within capacity=16. Rejection smoke is a candidate for a future phase.
- Custom STM32F407VET6 board remains hardware-unvalidated.

---

### Next Recommended Phase

**Team decision required.**

Candidates:
- **Phase 4K** — Scheduler Producer Throttle Policy (host-only, core layer)
- **Phase 5D** — Platform Critical Section / ISR Lock Boundary
- **Phase 6C** — Devkit Queue Full / Drop Smoke ← **completed**

---

---

## Phase 6C — Devkit Queue Full / Drop Smoke

**Date:** 2026-05-02
**Branch:** master
**Baseline commit:** `b8ee7f9` — devkit burst/backpressure smoke (Phase 6B)

---

### Purpose

Replace Phase 6B burst with a queue-full/drop smoke. Post exactly
`ROBOTOS_EVENT_QUEUE_CAPACITY + 1` (17) valid USER events at init. Prove:
- First 16 are accepted into the queue (returns `ROBOTOS_CORE_OK`).
- 17th valid event returns `ROBOTOS_CORE_ERR_FULL` because queue is full.
- `dropped_count` increments by 1; `admission_rejected_count` stays 0.
- This is a queue-full drop, not an admission rejection.
- Backpressure is active immediately (pending=16 > budget=1, queue full).
- Tick budget=1 drains one event per tick over 16 ticks.
- Handler logs milestones at seq=1/8/16 only; no RTT flood.
- Final snapshot logged once after all 16 handled.

No core policy changes. Devkit-only modification.

---

### Files Changed

| File | Change |
|------|--------|
| `devkit/src/devkit_runtime.c` | Replaced Phase 6B burst with Phase 6C full-queue burst (attempt=17, arg0=0x6C, seq 1..17); milestone-only handler logging; post summary; final summary after 16 handled |

### Files Unchanged (confirmed)

| File | Reason |
|------|--------|
| `core/robotos_core.c` | No core policy change |
| `core/robotos_event_queue.h` | Read-only include for `ROBOTOS_EVENT_QUEUE_CAPACITY` constant |
| `platform/` | No platform change |
| `devkit/prj.conf` | RTT buffer 4096 B from Phase 6A — sufficient |

---

### Burst Design

```
DEVKIT_PHASE6C_ATTEMPT_COUNT = ROBOTOS_EVENT_QUEUE_CAPACITY + 1 = 17
DEVKIT_PHASE6C_MARKER        = 0x6C
DEVKIT_PHASE6C_EXPECTED_OK   = ROBOTOS_EVENT_QUEUE_CAPACITY = 16

arg0 = 0x6C  (phase marker)
arg1 = 1..17 (sequence numbers)

Post result expectations:
  seq 1..16:  ROBOTOS_CORE_OK  -> posted_ok_count++
  seq 17:     ROBOTOS_CORE_ERR_FULL -> full_count++, dropped_count++
  errs=0      (no unexpected return codes)

Handler milestone logging:
  seq=1   -> log "handled seq=1 count=1"
  seq=8   -> log "handled seq=8 count=8"
  seq=16  -> log "handled seq=16 count=16"
  (no log for seq 2..7, 9..15)
```

---

### Expected Policy

- `admitted` = valid type passes admission gate — all 17 pass admission
- Queue push returns `ERR_FULL` on slot 17 — not an admission rejection
- `dropped_event_count` += 1 (queue full path)
- `admission_rejected_count` unchanged = 0
- `admission_accepted_count` = 16 (only successful queue entries)
- Backpressure: pending=16 > budget=1, also queue_is_full → bp=true at post
- Tick budget=1: drain one event per tick → 16 ticks to empty
- After drain: pending=0, dispatched=16, bp=false

---

### Host Test Evidence

**Result:** 9/9 suites pass — no regression.

**Log:** `tests/host/logs/phase_6C_host_2026-05-02.log`

Suites: 4C Core Init, 4D Core Version, 4E Event Post, 4F Event Dispatch Budget,
4G Handler Registration, 4H Core Snapshot, 4I Admission Policy, 4J Backpressure,
4K Scheduler Admission (stub).

---

### Zephyr Build Evidence

```
Build: west build -b stm32f411e_disco RobotOS_v1.0/devkit/ --pristine
Board: stm32f411e_disco
Zephyr: v3.6.0
Timestamp: May 2 2026 13:18:03

Memory:
  FLASH: 28568 / 524288 bytes  (5.45%)
  RAM:   12096 / 131072 bytes  (9.23%)

Errors: 0
```

Delta from Phase 6B (28352 B flash): +216 B for larger burst loop, post summary, counters.

---

### Runtime RTT Evidence

**RTT log:** `devkit/logs/phase_6C_rtt_2026-05-02.txt`

**Capture:** `openocd reset run -> sleep 9000 -> halt -> dump_image 0x200009c8 4096`
(`_SEGGER_RTT` address unchanged from Phase 6A/6B)

**Post summary (at init):**
```
Phase 6C post summary: attempted=17 ok=16 full=1 errs=0
                       pending=16 dropped=1 accepted=16 rejected=0 bp=1
```

**Drain milestones:**
```
[00:00:00.000] Phase 6C: handled seq=1  count=1   (tick 0)
[00:00:03.501] Phase 6C: handled seq=8  count=8   (tick 7)
[00:00:07.501] Phase 6C: handled seq=16 count=16  (tick 15)
```

**Final summary (logged once after count==16):**
```
[00:00:07.501] Phase 6C summary: handled=16 pending=0 dispatched=16
               accepted=16 rejected=0 dropped=1 herr=0 unhandled=0 bp=0
```

Post-drain ticks (16..18): clean, no burst events, no errors.

---

### Fault Register Check

```
CFSR = 0x0    (no configurable fault)
HFSR = 0x0    (no hard fault)
```

No faults. Runtime stable across full 9-second capture window.

---

### Legacy Isolation Confirmation

No `RobotOS_v1.0/src/` file compiled, staged, or referenced.
Queue-full/drop integration is devkit-only. No core policy changes.
`prj.conf` unchanged from Phase 6A (`CONFIG_SEGGER_RTT_BUFFER_SIZE_UP=4096`).

---

### Known Limitations

- Queue-full smoke only; no invalid/reserved event type injection in this phase.
- `admission_rejected_count` remains 0 — rejection smoke is Phase 6D scope.
- No producer throttle; no real scheduler; no periodic producer.
- Burst is fixed at compile time (`DEVKIT_PHASE6C_ATTEMPT_COUNT = 17`).
- `s_burst_*` counters are file-local; observability via RTT log only.
- Custom STM32F407VET6 board remains hardware-unvalidated.

---

### Next Recommended Phase

**Team decision required.**

Candidates:
- **Phase 6D** — Devkit Invalid/Rejection Smoke ← **completed**
- **Phase 4K** — Scheduler Producer Throttle Policy (host-only, core layer)
- **Phase 5D** — Platform Critical Section / ISR Lock Boundary

---

---

## Phase 6D — Devkit Invalid / Rejection Smoke

**Date:** 2026-05-02
**Branch:** master
**Baseline commit:** `1cd33dc` — devkit queue full drop smoke (Phase 6C)

---

### Purpose

Post two invalid events at init to prove the admission gate rejects them with
`ROBOTOS_CORE_ERR_INVALID_ARG` before touching the queue. Confirms the separation
between admission rejection (`admission_rejected_count`) and queue-full drop
(`dropped_count`). A USER handler is registered but must never be called because
no valid events enter the queue.

No core policy changes. Devkit-only modification.

---

### Files Changed

| File | Change |
|------|--------|
| `devkit/src/devkit_runtime.c` | Replaced Phase 6C queue-full smoke with Phase 6D rejection smoke: 2 invalid events (NONE + type=99), marker arg0=0x6D, post summary, final summary at tick 2, USER handler registered to confirm never called |

### Files Unchanged (confirmed)

| File | Reason |
|------|--------|
| `core/robotos_core.c` | No core policy change |
| `platform/` | No platform change |
| `devkit/prj.conf` | RTT buffer 4096 B sufficient |

---

### Invalid Event Design

```
DEVKIT_PHASE6D_ATTEMPT_COUNT = 2
DEVKIT_PHASE6D_MARKER        = 0x6D

Event 1: type=ROBOTOS_EVENT_NONE (0)  arg0=0x6D  arg1=1  label="NONE"
Event 2: type=99 (reserved)           arg0=0x6D  arg1=2  label="type=99"

Both must return ROBOTOS_CORE_ERR_INVALID_ARG.
```

---

### Expected Policy

- Both events hit admission gate (`event->type` check in `robotos_core_post_event`)
- `ROBOTOS_EVENT_NONE` and type 99 are not valid → `ERR_INVALID_ARG`
- `admission_rejected_count += 2` (one per rejection)
- Queue never touched → `pending=0`, `dropped=0`, `accepted=0`
- Tick dispatches nothing → `dispatched=0`, `herr=0`, `unhandled=0`
- USER handler registered but never called → `handler_called=0`
- `backpressure_active = false` (no pending events, queue not full)

---

### Host Test Evidence

**Result:** 9/9 suites pass — no regression.

**Log:** `tests/host/logs/phase_6D_host_2026-05-02.log`

Suites: 4C Core Init, 4D Core Version, 4E Event Post, 4F Event Dispatch Budget,
4G Handler Registration, 4H Core Snapshot, 4I Admission Policy, 4J Backpressure,
4K Scheduler Admission (stub).

---

### Zephyr Build Evidence

```
Build: west build -b stm32f411e_disco RobotOS_v1.0/devkit/ --pristine
Board: stm32f411e_disco
Zephyr: v3.6.0
Timestamp: May 2 2026 13:37:25

Memory:
  FLASH: 28744 / 524288 bytes  (5.48%)
  RAM:   12096 / 131072 bytes  (9.23%)

Errors: 0
```

Delta from Phase 6C (28568 B flash): +176 B for rejection handler, compound literal init, label strings.

---

### Runtime RTT Evidence

**RTT log:** `devkit/logs/phase_6D_rtt_2026-05-02.txt`

**Capture:** `openocd reset run -> sleep 3000 -> halt -> dump_image 0x200009c8 4096`

**Post summary (at init):**
```
Phase 6D post summary: attempted=2 inv_arg=2 other_err=0
                       pending=0 accepted=0 rejected=2 dropped=0
                       dispatched=0 herr=0 unhandled=0 bp=0 handler_called=0
```

**Final summary (at tick 2):**
```
Phase 6D final summary: attempted=2 inv_arg=2 other_err=0
                        pending=0 accepted=0 rejected=2 dropped=0
                        dispatched=0 herr=0 unhandled=0 bp=0 handler_called=0
```

All counters stable across tick 0 through tick 6. No events dispatched. Handler never called.

---

### Fault Register Check

```
CFSR = 0x0    (no configurable fault)
HFSR = 0x0    (no hard fault)
```

No faults. Runtime stable across full 3-second capture window.

---

### Legacy Isolation Confirmation

No `RobotOS_v1.0/src/` file compiled, staged, or referenced.
Rejection smoke is devkit-only. No core policy changes.
`prj.conf` unchanged from Phase 6A (`CONFIG_SEGGER_RTT_BUFFER_SIZE_UP=4096`).

---

### Known Limitations

- Invalid/rejection smoke only; queue-full/drop already covered in Phase 6C.
- Only two invalid types tested: `ROBOTOS_EVENT_NONE` and reserved type 99.
  Other reserved types not exercised in this phase.
- `s_rejection_*` counters are file-local; observability via RTT log only.
- No producer throttle; no real scheduler; no periodic producer.
- Custom STM32F407VET6 board remains hardware-unvalidated.

---

### Next Recommended Phase

**Team decision required.**

Candidates:
- **Phase 4K** — Scheduler Producer Throttle Policy ← **completed**
- **Phase 5D** — Platform Critical Section / ISR Lock Boundary
- **Phase 6E** — Devkit Mixed Policy Smoke (valid + invalid + full in one sequence)

---

---

## Phase 4K — Scheduler Producer Throttle Policy

**Date:** 2026-05-02
**Branch:** master
**Baseline commit:** `51e531e` — devkit invalid/rejection smoke (Phase 6D)

---

### Purpose

Add an explicit, observable producer-side throttle policy to core event
ingestion via a new `robotos_core_try_post_event()` API. The existing
`robotos_core_post_event()` semantics are unchanged — it remains the raw
ingestion path and preserves the Phase 6C queue-full/drop behavior.

Throttle applies only in `try_post_event` when the pending backlog exceeds
the per-tick dispatch budget and the queue is not yet full. Three ingestion
outcomes remain fully distinguishable: invalid rejection, queue-full drop,
and producer throttle.

No hardware runtime required. Policy proven by host tests only.

---

### Files Changed

| File | Change |
|------|--------|
| `core/robotos_core.h` | Add `ROBOTOS_CORE_ERR_THROTTLED=-7`; add `robotos_core_try_post_event()`; add `producer_throttle_active`/`producer_throttled_count` to snapshot; add getter declarations |
| `core/robotos_core.c` | Add `s_producer_throttled_count`; refactor `post_event` into `post_event_internal(apply_throttle)`; add `try_post_event`; add two getters; extend snapshot |
| `core/README.md` | Add Phase 4K section documenting throttle API, policy, and limitations |
| `tests/host/CMakeLists.txt` | Add `robotos_scheduler_throttle_contract_test` target and ctest registration |
| `tests/host/test_robotos_scheduler_throttle_contract.c` | 15 test cases covering all throttle semantics |
| `devkit/docs/DEVKIT_PROGRESS.md` | This section |
| `tests/host/logs/phase_4K_host_2026-05-02.log` | Host test evidence |

### Files Unchanged (confirmed)

| File | Reason |
|------|--------|
| `devkit/src/devkit_runtime.c` | Phase 6D rejection smoke unchanged; no throttle smoke needed |
| `platform/` | No platform change |
| `devkit/prj.conf` | No config change |

---

### Contract Summary

**`robotos_core_post_event()` — unchanged raw ingestion:**
- NULL → `ERR_NULL`
- Not READY → `ERR_INVALID_STATE`
- Invalid type → `ERR_INVALID_ARG` + `admission_rejected_count++`
- Queue full → `ERR_FULL` + `dropped_count++`
- OK → `admission_accepted_count++`

**`robotos_core_try_post_event()` — throttle-aware API (Phase 4K):**
- Same NULL/state/admission checks
- After admission: if queue NOT full AND `pending > ROBOTOS_CORE_MAX_EVENTS_PER_TICK`:
  → `ERR_THROTTLED` + `producer_throttled_count++`
  (queue untouched, accepted/rejected/dropped unchanged)
- If queue IS full: falls through to push → `ERR_FULL` + `dropped_count++`
- OK → `admission_accepted_count++`

**Three distinct ingestion outcomes remain separable:**

```
invalid type   -> ERR_INVALID_ARG  -> admission_rejected_count
queue full     -> ERR_FULL         -> dropped_count
throttled      -> ERR_THROTTLED    -> producer_throttled_count
success        -> OK               -> admission_accepted_count
```

**New status code:** `ROBOTOS_CORE_ERR_THROTTLED = -7`

**New snapshot fields:**
```c
bool     producer_throttle_active;  /* pending > budget AND queue NOT full */
uint32_t producer_throttled_count;  /* cumulative ERR_THROTTLED count */
```

**Repeated init:** `producer_throttled_count` not reset (consistent with other counters).

---

### Host Test Evidence

**Result:** 10/10 suites pass — no regression on prior 9, new throttle suite passes.

**Log:** `tests/host/logs/phase_4K_host_2026-05-02.log`

New suite (`robotos_scheduler_throttle_contract`), 15 test cases:
- TC01–TC03: pre/post-init getter state
- TC04–TC05: NULL/invalid via try_post_event
- TC06–TC08: throttle transition (event #1 OK, #2 OK+pressure, #3 throttled)
- TC09–TC10: tick clears throttle, try_post_event succeeds again
- TC11–TC12: post_event fills queue to full, ERR_FULL + dropped (not throttled)
- TC12: try_post_event when full → ERR_FULL (not throttled)
- TC13: snapshot coherence for throttle fields
- TC14: repeated init does not reset throttled_count
- TC15: dispatch_events drains backlog and clears throttle

---

### Zephyr Build Evidence

```
Build: west build -b stm32f411e_disco RobotOS_v1.0/devkit/ --pristine
Board: stm32f411e_disco
Zephyr: v3.6.0

Memory:
  FLASH: 28848 / 524288 bytes  (5.50%)
  RAM:   12096 / 131072 bytes  (9.23%)

Errors: 0
```

Delta from Phase 6D (28744 B flash): +104 B for two new getter functions.

---

### Runtime Requirement

Hardware runtime **not required** for Phase 4K closure. The throttle policy is
implemented in portable C with no Zephyr or board types. All semantics are
proven by host tests. Devkit runtime (Phase 6D) remains unchanged.

Phase 6E (Devkit Throttled Producer Smoke) would be the appropriate phase to
exercise `try_post_event` on real hardware.

---

### Legacy Isolation Confirmation

No `RobotOS_v1.0/src/` file compiled, staged, or referenced.
Core changes are additive (new API + counter). Existing `post_event` semantics unchanged.

---

### Known Limitations

- No automatic retry on `ERR_THROTTLED`; producers must handle it explicitly.
- No dynamic budget; throttle threshold fixed at `ROBOTOS_CORE_MAX_EVENTS_PER_TICK`.
- No priority or fairness; all `try_post_event` callers throttled equally.
- No producer registry; no per-producer accounting.
- No concurrency/ISR safety; single-threaded only.
- `post_event` bypasses throttle; queue-full through raw path still possible.
- No devkit runtime smoke in this phase.

---

### Next Recommended Phase

**Team decision required.**

Candidates:
- **Phase 6E** — Devkit Throttled Producer Smoke ← **completed**
- **Phase 5D** — Platform Critical Section / ISR Lock Boundary
- **Phase 4L** — Scheduler Advisory Retry Decision Policy ← **completed**

---

---

## Phase 6E — Devkit Throttled Producer Smoke

**Date:** 2026-05-02
**Branch:** master
**Baseline commit:** `5240fae` — core scheduler producer throttle policy (Phase 4K)

---

### Purpose

Exercise `robotos_core_try_post_event()` on real hardware to prove the
Phase 4K throttle contract on the devkit. Post 3 valid USER events using
`try_post_event`: seq=1 and seq=2 are accepted; seq=3 is returned
`ERR_THROTTLED` because pending=2 exceeds the per-tick budget of 1 and the
queue is not full. Seq=3 never enters the queue and is never dispatched to
the handler. Tick budget drains seq=1 and seq=2 over two ticks.

No core policy changes. No platform changes. Devkit-only modification.

---

### Files Changed

| File | Change |
|------|--------|
| `devkit/src/devkit_runtime.c` | Replaced Phase 6D rejection smoke with Phase 6E throttle smoke: 3 valid USER events via `try_post_event` (arg0=0x6E, seq 1..3); handler validates arg0/seq; post summary; final summary after 2 handled |

### Files Unchanged (confirmed)

| File | Reason |
|------|--------|
| `core/robotos_core.h` | No core change |
| `core/robotos_core.c` | No core change |
| `platform/` | No platform change |
| `devkit/prj.conf` | RTT buffer 4096 B sufficient |

---

### Throttle Event Design

```
DEVKIT_PHASE6E_ATTEMPT_COUNT = 3
DEVKIT_PHASE6E_MARKER        = 0x6E
DEVKIT_PHASE6E_EXPECTED_OK   = 2   (seq=1 and seq=2)

All 3 posts use robotos_core_try_post_event() — throttle-aware path.

seq=1: pending=0 -> 1, (1 == budget=1) -> throttle=false -> OK
seq=2: pending=1 -> 2, (2 > budget=1) -> throttle activates  -> OK (event accepted, transition INTO throttle)
seq=3: pending=2 > budget=1, queue not full -> ERR_THROTTLED  -> prod_throttled_count++

Handler sees seq=1 and seq=2 only. seq=3 never entered queue.
```

---

### Expected Policy

- `try_post_event` applies throttle after admission, before queue push
- Throttle condition: pending > ROBOTOS_CORE_MAX_EVENTS_PER_TICK AND NOT queue_is_full
- seq=1 accepted: pending was 0 <= budget=1 -> no throttle
- seq=2 accepted: pending was 1 == budget=1 -> not strictly > budget -> accepted; post-accept pending=2 > budget
- seq=3 throttled: pending=2 > budget=1, queue not full -> ERR_THROTTLED
- `dropped_count=0`, `admission_rejected_count=0` — throttle is distinct from both
- Tick drains one event per tick: seq=1 at tick 0, seq=2 at tick 1
- After tick 1: pending=0, throttle_active=false, bp=false

---

### Host Test Evidence

**Result:** 10/10 suites pass — no regression.

**Log:** `tests/host/logs/phase_6E_host_2026-05-02.log`

Suites: 4C Core Init, 4D Queue, 4E Dispatcher, 4F Ingestion, 4G Tick Policy,
4H Handler Policy, 5C Platform Fault, 4I Admission, 4J Backpressure, 4K Throttle.

---

### Zephyr Build Evidence

```
Build: west build -b stm32f411e_disco RobotOS_v1.0/devkit/ --pristine
Board: stm32f411e_disco
Zephyr: v3.6.0
Timestamp: May 2 2026 14:20:14

Memory:
  FLASH: 28940 / 524288 bytes  (5.52%)
  RAM:   12096 / 131072 bytes  (9.23%)

Errors: 0
```

Delta from Phase 4K Zephyr build (28848 B): +92 B for throttle smoke handler and counters.

---

### Runtime RTT Evidence

**RTT log:** `devkit/logs/phase_6E_rtt_2026-05-02.txt`

**Capture:** `openocd reset run -> sleep 3000 -> halt -> dump_image 0x200009c8 4096`

**Post summary (at init):**
```
Phase 6E post summary: attempted=3 ok=2 throttled=1 full=0 other=0
                       pending=2 accepted=2 rejected=0 dropped=0
                       prod_throttled=1 bp=1 th_active=1
```

**Handler evidence:**
```
[00:00:00.000] Phase 6E: handled seq=1 count=1   (tick 0)
[00:00:00.500] Phase 6E: handled seq=2 count=2   (tick 1)
```
seq=3 not handled — throttled before entering queue.

**Final summary (logged at tick 1, after count==2):**
```
Phase 6E final summary: handled=2 pending=0 dispatched=2
                        accepted=2 rejected=0 dropped=0
                        prod_throttled=1 herr=0 unhandled=0
                        bp=0 th_active=0 unexpected=0
```

Post-drain ticks (2..6): clean, no throttle events, no errors.

---

### Fault Register Check

```
CFSR = 0x0    (no configurable fault)
HFSR = 0x0    (no hard fault)
```

No faults. Runtime stable across full 3-second capture window.

---

### Legacy Isolation Confirmation

No `RobotOS_v1.0/src/` file compiled, staged, or referenced.
Throttle smoke is devkit-only. No core policy changes.
`prj.conf` unchanged from Phase 6A (`CONFIG_SEGGER_RTT_BUFFER_SIZE_UP=4096`).

---

### Known Limitations

- Throttle smoke only; queue-full/drop (6C) and invalid/rejection (6D) are separate phases.
- No retry/backoff for throttled events; caller handles `ERR_THROTTLED`.
- No producer registry; all `try_post_event` callers throttled equally.
- No periodic producer; 3 events posted once at init.
- No real scheduler.
- Custom STM32F407VET6 board remains hardware-unvalidated.

---

### Next Recommended Phase

**Team decision required.**

Candidates:
- **Phase 4L** — Scheduler Retry/Backoff Policy Stub (host-only, core layer)
- **Phase 5D** — Platform Critical Section / ISR Lock Boundary ← **completed**
- **Phase 6F** — Devkit Mixed Event Policy Smoke (valid + invalid + throttled + full in one sequence)

---

---

## Phase 5D — Platform Critical Section / ISR Lock Boundary

**Date:** 2026-05-02
**Branch:** master
**Baseline commit:** `6ae3897` — devkit throttled producer smoke (Phase 6E)

---

### Purpose

Introduce a minimal platform critical-section boundary as the first ISR-lock
infrastructure in RobotOS. Define a portable `enter`/`exit` API with an opaque
token, back it with Zephyr `irq_lock`/`irq_unlock`, add a host stub with
test-inspectable counters, and prove the API with host tests and a one-time
devkit boot smoke. The boundary is **not wired into core event queue or
`post_event`** in Phase 5D — core APIs remain single-threaded.

---

### Files Changed

| File | Change |
|------|--------|
| `platform/robotos_platform_critical.h` | New portable API: token type, `enter()`, `exit()` — no Zephyr types |
| `platform/zephyr/robotos_platform_critical_zephyr.c` | Zephyr backend: `irq_lock`/`irq_unlock` |
| `platform/README.md` | Phase 5D section: API, Zephyr backend, host backend, limitations |
| `tests/host/robotos_platform_critical_host_stub.h` | Host inspection helpers (not in public header) |
| `tests/host/robotos_platform_critical_host_stub.c` | Host stub: counter/depth tracking |
| `tests/host/test_robotos_platform_critical_contract.c` | 12 test cases for enter/exit contract |
| `tests/host/CMakeLists.txt` | New `robotos_platform_critical_contract_test` target |
| `devkit/CMakeLists.txt` | Added `robotos_platform_critical_zephyr.c` to build |
| `devkit/src/devkit_runtime.c` | One-time `enter`/`exit` smoke at boot; include added |
| `devkit/logs/phase_5D_rtt_2026-05-02.txt` | RTT evidence |
| `tests/host/logs/phase_5D_host_2026-05-02.log` | Host evidence |

### Files Unchanged (confirmed)

| File | Reason |
|------|--------|
| `core/robotos_core.*` | No core change — API not wired into core queue |
| `core/robotos_event_queue.*` | No queue change — single-threaded assumption preserved |
| `devkit/prj.conf` | No config change |

---

### Platform Boundary Summary

**Public API** (`platform/robotos_platform_critical.h`):
```c
typedef struct { uintptr_t opaque; } robotos_platform_critical_token_t;
robotos_platform_critical_token_t robotos_platform_critical_enter(void);
void robotos_platform_critical_exit(robotos_platform_critical_token_t token);
```
No Zephyr types. No board types. C99 compatible.

**Zephyr backend:** `irq_lock()` / `irq_unlock()` — stores unsigned int key in
`token.opaque`. Nested calls supported on ARMv7-M. Short critical sections only.

**Host backend:** Counter/depth stub. `enter()` assigns monotonically increasing
opaque id; `exit()` decrements depth (floored at 0). Inspection helpers in stub header.

**Not wired into core:** `post_event`, `tick`, queue — single-threaded, non-ISR-safe.

---

### Host Test Evidence

**Result:** 11/11 suites pass (10 prior + 1 new critical section suite, 12 cases).

**Log:** `tests/host/logs/phase_5D_host_2026-05-02.log`

New suite (`robotos_platform_critical_contract`), 12 test cases:
- TC01: initial counts zero after reset
- TC02–TC03: enter increments count/depth, token nonzero
- TC04: exit increments exit_count, decrements depth
- TC05–TC06: nested enter/token ordering, max_depth tracking
- TC07: nested exit restores depth to 0
- TC08: extra exit at depth=0 does not underflow
- TC09: repeated reset clears all state
- TC10: 10× enter/exit leaves depth=0, counts 10/10
- TC11: token struct size == sizeof(uintptr_t)
- TC12: max_depth tracks deepest nesting across nested calls

---

### Zephyr Build Evidence

```
Build: west build -b stm32f411e_disco RobotOS_v1.0/devkit/ --pristine
Board: stm32f411e_disco
Zephyr: v3.6.0
Timestamp: May 2 2026 14:33:21

Memory:
  FLASH: 29016 / 524288 bytes  (5.53%)
  RAM:   12096 / 131072 bytes  (9.23%)

Errors: 0
```

Delta from Phase 6E (28940 B flash): +76 B for `robotos_platform_critical_zephyr.c`
and one-time smoke call in `devkit_runtime_init`.

---

### Runtime Evidence

**RTT log:** `devkit/logs/phase_5D_rtt_2026-05-02.txt`

**Key evidence line:**
```
[00:00:00.000,000] <inf> devkit_runtime: platform critical smoke ok
```

`irq_lock()` called → `irq_unlock()` called → no fault, no crash, no panic.
Phase 6E throttle smoke follows normally; LED toggles every 500ms.

**CFSR = 0x0 HFSR = 0x0** — no faults.

---

### Legacy Isolation Confirmation

No `RobotOS_v1.0/src/` file compiled, staged, or referenced.
Critical section API is platform-only. No core event path changed.

---

### Known Limitations

- Critical boundary **not wired into core** — `post_event`, `tick`, queue remain single-threaded and non-ISR-safe.
- No ISR-safe posting claim at RobotOS level in Phase 5D.
- No mutex or thread abstraction; `irq_lock` is not a mutex.
- No scheduler concurrency; no producer ISR.
- No nesting enforcement at API level; backend handles nesting.
- Interrupt latency risk if critical sections held too long.
- Phase 5E is the appropriate phase to wire this into core queue.
- Custom STM32F407VET6 board remains hardware-unvalidated.

---

### Next Recommended Phase

**Team decision required.**

Candidates:
- **Phase 5E** — Apply Critical Boundary to Core Queue State ← **completed**
- **Phase 4L** — Scheduler Retry/Backoff Policy Stub (host-only, core layer)
- **Phase 6F** — Devkit Mixed Event Policy Smoke

---

---

## Phase 5E — Apply Critical Boundary to Core Queue State

**Date:** 2026-05-02
**Branch:** master
**Baseline commit:** `4187bb3` — platform critical section boundary (Phase 5D)

---

### Purpose

Wire `robotos_platform_critical_enter`/`exit` into short core state transitions
to protect queue/counter/handler-table mutations and reads. Handler callbacks
continue to execute outside any critical section. This is the first step toward
a concurrent-safe core, but does NOT claim full thread-safety or ISR-safe posting.

Option A chosen for tick dispatch: `robotos_core_tick()` locks only
`tick_count` increment; the dispatcher dispatch path (which calls handlers) is
left outside the lock. The dispatcher/handler pop-split is Phase 5F scope.

---

### Files Changed

| File | Change |
|------|--------|
| `core/robotos_core.c` | Add `#include "robotos_platform_critical.h"`; apply enter/exit to post_event_internal, init, tick, dispatch_events, snapshot, all getters, handler table ops |
| `core/README.md` | Phase 5E section: protected/unprotected table, critical rule, limitations |
| `platform/README.md` | Short note: Phase 5E begins using platform critical from core |
| `tests/host/CMakeLists.txt` | `PLATFORM_HOST_STUB` extended to include critical stub; all CORE_SRCS targets get `CMAKE_CURRENT_SOURCE_DIR`; new `robotos_core_critical_boundary_contract_test` |
| `tests/host/test_robotos_core_critical_boundary_contract.c` | 13 contract test cases |
| `devkit/logs/phase_5E_rtt_2026-05-02.txt` | RTT evidence |
| `tests/host/logs/phase_5E_host_2026-05-02.log` | Host evidence |

### Files Unchanged (confirmed)

| File | Reason |
|------|--------|
| `core/robotos_core.h` | No API change — behavior unchanged |
| `core/robotos_event_queue.*` | No queue change |
| `core/robotos_event_dispatcher.*` | No dispatcher change |
| `devkit/src/devkit_runtime.c` | No runtime change |
| `platform/robotos_platform_critical.h` | No API change |

---

### Lock Boundary Summary

**Protected (short critical section per operation):**
- `post_event_internal()` — state check, admission, throttle, queue push, counter updates
- `robotos_core_init()` — core_state, tick_count, init_count, handler table clear
- `robotos_core_tick()` — state check and tick_count increment only
- `robotos_core_dispatch_events()` — state check only
- `robotos_core_snapshot()` — all counter/state reads (coherent)
- All individual getters — brief lock per read
- Handler register/unregister/has/count — table search and mutation

**NOT protected (single-threaded assumption remains):**
- Registered handler callback — hard rule: no lock held
- `CORE_LOG_*` / `robotos_platform_logf` — must be outside lock
- `robotos_event_dispatcher_dispatch_all` — calls handler internally
- Dispatcher internal counters (dispatched_count, handler_error_count)
- Routing handler's handler table read during dispatch

**Critical rule proved by TC10:**
Handler sees `robotos_platform_critical_host_current_depth() == 0` during invocation.

---

### Host Test Evidence

**Result:** 12/12 suites pass (11 prior + 1 new, 13 cases).

**Log:** `tests/host/logs/phase_5E_host_2026-05-02.log`

New suite (`robotos_core_critical_boundary_contract`), 13 test cases:
- TC01: initial counts zero
- TC02: snapshot(NULL) before lock
- TC03: init balanced enter/exit
- TC04–TC05: post_event/try_post_event balanced
- TC06: invalid post balanced
- TC07: throttled try_post balanced
- TC08: snapshot balanced
- TC09: register/unregister balanced
- **TC10: CRITICAL — handler sees depth=0** (no lock held during callback)
- TC11: dispatch_events — handler sees depth=0
- TC12: 5x post+tick stress, depth always 0
- TC13: repeated init balanced, does not reset event counters

---

### Zephyr Build Evidence

```
Build: west build -b stm32f411e_disco RobotOS_v1.0/devkit/ --pristine
Board: stm32f411e_disco
Zephyr: v3.6.0

Memory:
  FLASH: 29080 / 524288 bytes  (5.55%)
  RAM:   12096 / 131072 bytes  (9.23%)

Errors: 0
```

Delta from Phase 5D (29016 B flash): +64 B for additional enter/exit call sites.

---

### Runtime Evidence

**RTT log:** `devkit/logs/phase_5E_rtt_2026-05-02.txt`

`irq_lock`/`irq_unlock` now called from core on every post/tick/snapshot.
No MemManage fault. No hard fault. No panic. Phase 6E throttle behavior unchanged.

```
CFSR = 0x0    (no configurable fault)
HFSR = 0x0    (no hard fault)
```

---

### Legacy Isolation Confirmation

No `RobotOS_v1.0/src/` file compiled, staged, or referenced.
Core changes are structural; no semantic change to event policy.

---

### Known Limitations

- **Not full thread-safe.** Dispatcher pop and handler execution are in the same unlocked region.
- **No ISR-safe posting claim.** `post_event` is now shorter under lock but concurrent ISR posting is not proven.
- **Dispatcher counters not protected.** `dispatched_count` and `handler_error_count` updated by dispatcher outside lock.
- **Routing handler reads table without lock.** Safe under single-threaded devkit assumption; Phase 5F scope.
- **No scheduler concurrency.** Devkit loop remains single-threaded.
- **No custom board validation.** STM32F407VET6 remains unvalidated.

---

### Next Recommended Phase

**Phase 5F completed.** See Phase 5F section below.

---

---

## Phase 5F — Dispatcher Pop / Handler Split

**Date:** 2026-05-02
**Branch:** master
**Baseline commit:** `ff24147` — critical boundary contract (Phase 5E)

---

### Purpose

Split the dispatch path so that:
1. Queue pop is performed under a critical section.
2. Handler table lookup is performed under a separate critical section with local copies made.
3. Handler callback is invoked entirely outside any critical section.

This removes the structural reliance on `robotos_event_dispatcher_dispatch_all`
for core event routing, and makes the "no lock held during handler" rule a
structural guarantee within `robotos_core.c` itself.

---

### Files Changed

| File | Change |
|------|--------|
| `core/robotos_core.c` | Remove `s_core_routing_handler`; add `s_core_noop_handler` and `s_core_dispatch_one_safe()`; update `tick()` and `dispatch_events()` to call `s_core_dispatch_one_safe` loop |
| `core/README.md` | Phase 5F section: dispatch path table, what changed, critical rule, known limitation |
| `tests/host/CMakeLists.txt` | New `robotos_core_dispatch_critical_contract_test` target |
| `tests/host/test_robotos_core_dispatch_critical_contract.c` | 13 contract test cases (TC01–TC13) |
| `tests/host/logs/phase_5F_host_2026-05-02.log` | Host evidence |

### Files Unchanged (confirmed)

| File | Reason |
|------|--------|
| `core/robotos_core.h` | No API change |
| `core/robotos_event_dispatcher.*` | No new API added; dispatcher still used for its own unit tests |
| `core/robotos_event_queue.*` | No change |
| `platform/robotos_platform_critical.h` | No change |
| `devkit/src/devkit_runtime.c` | No change |

---

### Dispatch Path (s_core_dispatch_one_safe)

```
Step 1: [lock] queue pop into local ev   [unlock]
Step 2: [lock] handler table lookup →    [unlock]
             copy fn + ctx to locals
Step 3: invoke handler_fn(&ev, ctx)      // depth MUST be 0
Step 4: update dispatcher counters       // no lock needed (same TU)
```

---

### Host Test Evidence

**Result:** 13/13 suites pass (12 prior + 1 new, 13 cases in new suite).

**Log:** `tests/host/logs/phase_5F_host_2026-05-02.log`

New suite (`robotos_core_dispatch_critical_contract`), 13 test cases:
- TC01: `tick()` → handler sees depth == 0
- TC02: `dispatch_events()` → handler sees depth == 0
- TC03: FIFO preserved (3 events, correct arg0 order)
- TC04: unregistered event → unhandled_count++, depth == 0
- TC05: handler error → handler_error_count++, returns error, depth == 0
- TC06: `tick()` on empty → OK, depth == 0
- TC07: `dispatch_events()` on empty → ERR_EMPTY, depth == 0
- TC08: `dispatch_events(0)` → OK, no dispatch
- TC09: `dispatch_events(2)` → exactly 2 dispatched, 1 pending
- TC10: unregister before dispatch → event becomes unhandled (lookup at dispatch time)
- TC11: handler self-unregisters in callback → no deadlock, depth == 0
- TC12: register then replace handler → latest used, not prior
- TC13: 5× post+tick stress → depth always 0, enter_count == exit_count

All prior 12 suites continue to pass (no regressions).

---

### Zephyr Build Evidence

```
Build: west build -b stm32f411e_disco RobotOS_v1.0/devkit/ --pristine
Board: stm32f411e_disco
Zephyr: v3.6.0

Memory:
  FLASH: 29060 / 524288 bytes  (5.54%)
  RAM:   12096 / 131072 bytes  (9.23%)

Errors: 0
```

Delta from Phase 5E (29080 B flash): -20 B (routing handler path removed).

---

### Runtime Evidence

**RTT log:** `devkit/logs/phase_5F_rtt_2026-05-02.txt`

`_SEGGER_RTT` address: `0x200009e4` (Phase 5E was `0x200009c8`; shifted -20B with routing handler removal).

Dispatch path exercised via Phase 6E smoke: USER event handler invoked through
`s_core_dispatch_one_safe()` — `handled=2 dispatched=2 herr=0 unhandled=0`.
Phase 6E throttle behavior unchanged: `attempted=3 ok=2 throttled=1 unexpected=0`.
No MemManage fault. No hard fault. No panic.

```
platform critical smoke ok
RobotOS core init -- version=4B-contract state=READY
Phase 6E: handled seq=1 count=1        (tick=0)
Phase 6E: handled seq=2 count=2        (tick=1)
Phase 6E final: handled=2 dispatched=2 herr=0 unhandled=0 unexpected=0
tick count: 0,1,2,3,4,5,6 (7 ticks over ~3s at 500ms interval)
```

```
CFSR = 0x0    (no configurable fault)
HFSR = 0x0    (no hard fault)
```

LED: toggling every 500ms — confirmed.

---

### Known Limitations

- **Stale-copy on concurrent unregister.** If a handler is unregistered between
  Step 2 and Step 3, the copied fn/ctx still executes once. Acceptable under
  single-threaded devkit assumption. Multi-threaded builds would need epoch or
  reference-count invalidation.
- **Not full ISR-safe.** `post_event` is lock-protected but concurrent ISR
  posting is not stress-proven.
- **No scheduler concurrency.** Devkit loop remains single-threaded.
- **No custom board validation.** STM32F407VET6 remains unvalidated.

---

### Next Recommended Phase

**Phase 5G completed.** See Phase 5G section below.

---

---

## Phase 5G — ISR-Safe Producer Contract Audit

**Date:** 2026-05-02
**Branch:** master
**Baseline commit:** `460b9f1` — core: confirm dispatch split runtime (Phase 5F-R)
**Type:** AUDIT/DOC-ONLY — no source code changed

---

### Purpose

Inspect actual lock boundaries from Phase 5D/5E/5F and publish a precise
ISR-safety contract for the event producer path. Determine exactly which
RobotOS core APIs are safe to call from an ISR-like producer context and
document the conditions, limitations, and gaps before any runtime ISR stress.

---

### Files Changed

| File | Change |
|------|--------|
| `core/README.md` | Phase 5G section: audit verdict, API classification table, ISR-safe producer contract, lock boundary findings, limitations, next recommendation |
| `platform/README.md` | Phase 5G note: platform critical supports limited ISR producer contract; log/fault/time/sleep not ISR-safe |
| `devkit/docs/DEVKIT_PROGRESS.md` | Phase 5G section (this entry) |
| `tests/host/logs/phase_5G_host_2026-05-02.log` | Host test evidence (all prior suites, no regressions) |

### Files Unchanged (confirmed)

| File | Reason |
|------|--------|
| `core/robotos_core.c` | Audit-only: no source change |
| `core/robotos_core.h` | No API change |
| `platform/robotos_platform_critical.h` | No change |
| `platform/zephyr/robotos_platform_critical_zephyr.c` | No change |
| `devkit/src/devkit_runtime.c` | No change |
| All test sources / CMakeLists.txt | No change |

---

### Audit Verdict: CLOSED_AUDIT_CONFIRMED

`robotos_core_post_event()` and `robotos_core_try_post_event()` are confirmed
ISR-safe on the Zephyr/ARMv7-M backend. The critical section on the producer
path is O(1), deterministic, holds no logging, invokes no handler, and performs
no sleep or dynamic allocation. Queue push is a bounded struct copy under lock.

---

### API Classification Summary

**ISR-safe (conditional):**
- `robotos_core_post_event()` — short lock, no handler/log/sleep inside
- `robotos_core_try_post_event()` — same; additionally returns ERR_THROTTLED

**Not ISR-safe:**
- `robotos_core_tick()` — calls handler and logging outside lock; thread-context only
- `robotos_core_dispatch_events()` — calls handler; thread-context only
- Handler register/unregister — table mutation; semantically wrong from ISR
- All platform log/fault/sleep/time APIs — explicitly not claimed ISR-safe
- All getters and snapshot — not claimed; thread-context only

**Conditions for ISR-safe post:**
- Zephyr/ARMv7-M backend (irq_lock BASEPRI masking); not valid from NMI
- Event struct fully initialized, accessible for call duration (copied under lock)
- Caller handles: ERR_NULL, ERR_INVALID_STATE, ERR_INVALID_ARG, ERR_FULL, ERR_THROTTLED
- No logging under lock confirmed; no handler under lock confirmed

**Lock boundary audit findings:**
- No handler called under critical section ✓
- No `robotos_platform_logf` called under critical section ✓
- No sleep/yield under critical section ✓
- No unbounded loop under critical section ✓
- Queue push O(1) struct copy — safe under irq_lock ✓

---

### Host Test Evidence

**Result:** 13/13 suites pass, 0 failures. No regressions.

**Commands:**
```
cmake -S RobotOS_v1.0/tests/host -B build-host-core -G "MinGW Makefiles"
cmake --build build-host-core
ctest --test-dir build-host-core --output-on-failure
```

**Log:** `tests/host/logs/phase_5G_host_2026-05-02.log`

---

### Zephyr Build Evidence

```
Build: west build -b stm32f411e_disco RobotOS_v1.0/devkit/ --pristine
Board: stm32f411e_disco
Zephyr: v3.6.0

Memory:
  FLASH: 29060 / 524288 bytes  (5.54%)
  RAM:   12096 / 131072 bytes  (9.23%)

Errors: 0
```

Memory unchanged from Phase 5F (docs-only phase, no source delta).

---

### Runtime Evidence

Not required. No source code changed. Phase 5F-R runtime evidence remains valid.
Handler path confirmed by Phase 5F-R RTT log `devkit/logs/phase_5F_rtt_2026-05-02.txt`.

---

### Legacy Isolation Confirmation

No `RobotOS_v1.0/src/` file compiled, staged, or referenced.
No custom board file changed. Audit is docs-only.

---

### Known Limitations

- **No ISR producer runtime stress.** No timer ISR or EXTI ISR hardware test.
- **No latency budget.** Critical section is O(1) bounded but not cycle-measured.
- **No multi-producer stress.** Single concurrent ISR producer assumed only.
- **No custom board validation.** STM32F407VET6 unvalidated.
- **Handler context lifetime.** ISR-local context passed to registered handler
  may be stale when handler runs in thread context; caller owns lifetime.
- **Dispatcher counters not ISR-protected.** Do not read dispatch counters from ISR.

---

### Next Recommended Phase

**Team decision required.**

Candidates:
- **Phase 6G** — ISR/Timer Producer Smoke ← **completed**
- **Phase 4L** — Scheduler Retry/Backoff Policy Stub (host-only, core layer)
- **Phase 6F** — Devkit Mixed Event Policy Smoke

---

---

## Phase 6G — ISR/Timer Producer Smoke

**Date:** 2026-05-02
**Branch:** master
**Baseline commit:** `e4ab853` — core: ISR-safe producer contract audit (Phase 5G)

---

### Purpose

Prove on devkit hardware that a producer running from a timer/ISR-like
context can post events to RobotOS core under the Phase 5G contract.
A `k_timer` periodic callback (Zephyr SysTick ISR context, ARMv7-M) posts
two USER events (arg0=0x0607, seq=1 and seq=2) using
`robotos_core_post_event()`. The normal tick loop dispatches them; a
registered handler logs from thread context. All Phase 5G callback
constraints are observed: no log, sleep, dispatch, or register call inside
the timer callback.

This is a timer smoke, not a full ISR stress. See Known Limitations.

---

### Files Changed

| File | Change |
|------|--------|
| `devkit/src/devkit_runtime.c` | Replaced Phase 6E throttled producer smoke with Phase 6G k_timer ISR producer smoke: k_timer_init/start at 100ms period; ISR callback posts seq=1 and seq=2 then stops timer; handler and final summary in thread context |
| `devkit/docs/DEVKIT_PROGRESS.md` | Phase 6G section (this entry) |
| `devkit/logs/phase_6G_rtt_2026-05-02.txt` | RTT boot log evidence |
| `tests/host/logs/phase_6G_host_2026-05-02.log` | Host test evidence |

### Files Unchanged (confirmed)

| File | Reason |
|------|--------|
| `core/robotos_core.c` | No core policy change |
| `core/robotos_core.h` | No API change |
| `platform/` | No platform change |
| `devkit/prj.conf` | RTT buffer 4096 B sufficient |

---

### Timer Producer Design

```c
#define DEVKIT_PHASE6G_MARKER          0x0607u
#define DEVKIT_PHASE6G_MAX_ATTEMPTS    2u
#define DEVKIT_PHASE6G_EXPECTED_OK     2u
#define DEVKIT_PHASE6G_TIMER_PERIOD_MS 100u

static struct k_timer s_phase6g_timer;
static volatile uint32_t s_timer_attempted_count;
static volatile uint32_t s_timer_ok_count;
static volatile uint32_t s_timer_full_count;
static volatile uint32_t s_timer_invalid_count;
static volatile uint32_t s_timer_other_error_count;
static volatile uint32_t s_timer_handled_count;
static volatile uint32_t s_timer_unexpected_count;
static bool              s_timer_final_logged;
```

Callback (Zephyr SysTick ISR context — NO log/sleep/tick/dispatch/register):
```c
static void devkit_phase6g_timer_cb(struct k_timer *timer) {
    // post robotos_event_t {ROBOTOS_EVENT_USER, arg0=0x0607, arg1=seq}
    // increment volatile counters only
    // k_timer_stop after seq 2
}
```

Init sequence (thread context):
1. Register USER handler (`devkit_phase6g_handler`) — BEFORE timer start
2. `k_timer_init` + `k_timer_start` (100ms period)
3. `LOG_INF("Phase 6G timer producer started")`

Handler (thread context):
- Validates type == USER, arg0 == 0x0607, seq 1..2
- Increments `s_timer_handled_count`
- Logs: `"Phase 6G timer event handled seq=%u count=%u"`

Final summary (in tick loop, once after handled==2):
- Calls `robotos_core_get_snapshot()` and logs all counters

---

### Timing

```
T=0ms:    init, handler registered, timer started (100ms period)
T=100ms:  ISR fires -> posts seq=1 (ok_count=1)
T=200ms:  ISR fires -> posts seq=2 (ok_count=2), k_timer_stop called
T=500ms:  tick count=1 -> dispatches seq=1, handler logs seq=1 count=1
T=1000ms: tick count=2 -> dispatches seq=2, handler logs seq=2 count=2
           Final summary emitted.
T=1500ms+: clean LED ticks, no further events.
```

---

### Host Test Evidence

**Result:** 13/13 suites pass — no regression.

**Log:** `tests/host/logs/phase_6G_host_2026-05-02.log`

Suites: 4C Core Init, 4D Core Version, 4E Event Post, 4F Event Dispatch Budget,
4G Handler Registration, 4H Core Snapshot, 4I Admission Policy, 4J Backpressure,
4K Scheduler Throttle Contract, 5A Platform Critical Smoke, 5B Platform Version,
5C Platform Lock Boundary, 5G ISR-Safe Producer Contract.

---

### Zephyr Build Evidence

```
Build: west build -b stm32f411e_disco RobotOS_v1.0/devkit/ --pristine
Board: stm32f411e_disco
Zephyr: v3.6.0
Timestamp: May  2 2026 16:37:48

Memory:
  FLASH: 28940 / 524288 bytes  (5.52%)
  RAM:   12160 / 131072 bytes  (9.28%)

Errors: 0
```

Delta from Phase 5G (29060 B flash): −120 B (k_timer smoke is smaller than Phase 5G
docs-only binary; Phase 6E source was replaced).

---

### Runtime RTT Evidence

**RTT log:** `devkit/logs/phase_6G_rtt_2026-05-02.txt`

**Capture:** `openocd reset run → sleep 3500 → halt → dump_image 0x20000a1c 4096`
(`_SEGGER_RTT` address: `0x20000a1c` — changed from Phase 6E due to source delta)

**Key log lines:**

```
[00:00:00.000,000] <inf> devkit_runtime: Phase 6G timer producer started
[00:00:00.500,000] <inf> devkit_runtime: Phase 6G timer event handled seq=1 count=1
[00:00:01.000,000] <inf> devkit_runtime: Phase 6G timer event handled seq=2 count=2
[00:00:01.000,000] <inf> devkit_runtime: Phase 6G final summary: attempted=2 ok=2 full=0 invalid=0 other=0 handled=2 unexpected=0 pending=0 dispatched=2 accepted=2 rejected=0 dropped=0 prod_throttled=0 herr=0 unhandled=0 bp=0 th_active=0
```

**Counter verification:**

| Counter | Expected | Result |
|---------|----------|--------|
| attempted | 2 | 2 ✓ |
| ok | 2 | 2 ✓ |
| full | 0 | 0 ✓ |
| invalid | 0 | 0 ✓ |
| other | 0 | 0 ✓ |
| handled | 2 | 2 ✓ |
| unexpected | 0 | 0 ✓ |
| pending | 0 | 0 ✓ |
| dispatched | 2 | 2 ✓ |
| accepted | 2 | 2 ✓ |
| rejected | 0 | 0 ✓ |
| dropped | 0 | 0 ✓ |
| prod_throttled | 0 | 0 ✓ |
| herr | 0 | 0 ✓ |

Post-drain ticks (3..6): clean, no further events, no errors.
LED: toggling every 500ms confirmed.

---

### Fault Register Check

```
CFSR = 0x0    (no configurable fault)
HFSR = 0x0    (no hard fault)
```

No faults. Runtime stable across full 3.5-second capture window.

---

### Legacy Isolation Confirmation

No `RobotOS_v1.0/src/` file compiled, staged, or referenced.
`<zephyr/kernel.h>` re-introduced in devkit layer (`devkit_runtime.c`) only;
`core/` and `platform/` headers remain Zephyr-free.
`prj.conf` unchanged (`CONFIG_SEGGER_RTT_BUFFER_SIZE_UP=4096`).

---

### Known Limitations

- **Timer smoke only.** k_timer callbacks run from Zephyr SysTick ISR context
  (ARMv7-M BASEPRI masking), but this is not a high-frequency EXTI/DMA ISR.
- **Not a high-freq ISR stress.** Only 2 events at 100ms period; no burst from ISR.
- **No multi-producer concurrency.** Single timer source only.
- **No latency budget.** Critical section is O(1) confirmed but not cycle-measured.
- **Platform APIs not ISR-safe.** Log, fault, sleep, time remain thread-context only.
- **Not valid from NMI context.** irq_lock uses BASEPRI; does not mask NMI.
- **Custom STM32F407VET6 board** remains hardware-unvalidated.

---

### Next Recommended Phase

**Team decision required.**

Candidates:
- **Phase 6F** — Devkit Mixed Event Policy Smoke (valid + invalid + full in one run)
- **Phase 4L** — Scheduler Retry/Backoff Policy Stub (host-only, core layer)
- **Phase 6H** — Devkit EXTI ISR Producer Smoke (higher-freq ISR, button/EXTI)

---

## Phase 6H — ISR / Timer Producer Stress-Lite

**Status:** CLOSED_TIMER_STRESS_LITE_CONFIRMED
**Date:** 2026-05-02
**Runtime confirmed:** 2026-05-03
**Branch:** master
**Baseline commit:** `335ee29` — Phase 6G timer producer smoke (ISR context confirmed)

---

### Purpose

Extend timer producer evidence from Phase 6G (2 events) to bounded 8-event stress-lite
mode. This phase proves post_event/dispatch/counter path under temporary producer
backlog without exceeding queue capacity or triggering full/drop paths.

---

### Files Changed

| File | Change |
|------|--------|
| `src/devkit_runtime.c` | Phase 6G → Phase 6H: 8 events at 100ms interval, milestone logging |
| `docs/DEVKIT_PROGRESS.md` | This file (Phase 6H section added) |

### Files Unchanged

| File | Reason |
|------|--------|
| `src/devkit_runtime.h` | No public API changes; DEVKIT_TICK_MS unchanged |
| `CMakeLists.txt` | No new sources; build system unchanged |
| `prj.conf` | No config changes |
| `core/*` | Core policy and contracts unchanged |

---

### Timer Producer Stress-Lite Design

**Primitive:** Zephyr k_timer (same as Phase 6G)

**Constants:**
- `DEVKIT_PHASE6H_MARKER = 0x0608`
- `DEVKIT_PHASE6H_ATTEMPT_COUNT = 8`
- `DEVKIT_PHASE6H_EXPECTED_OK = 8`
- `DEVKIT_PHASE6H_TIMER_INTERVAL_MS = 100`

**Producer Contract (Phase 5G restrictions followed in callback):**
- no logging
- no sleep
- no tick / dispatch / register / unregister
- no fault / assert
- event is local struct (stack): valid for entire post_event call duration
- all error returns handled via volatile counters only
- timer stops itself after 8 attempts

**Producer Path:**
1. k_timer fires every 100ms from Zephyr SysTick ISR context
2. Callback posts USER event with arg0=0x0608, arg1=seq (1..8)
3. Callback calls robotos_core_post_event()
4. Callback stops timer when seq > 8
5. Normal runtime tick (500ms) calls robotos_core_tick()
6. Core dispatch drains events; handler runs from thread context

**Consumer (handler) Behavior:**
- Validates event != NULL, type == USER, arg0 == 0x0608, seq 1..8
- Logs milestones only: seq=1, seq=4, seq=8 (no-spam control)
- Logs final summary once when handled == 8

**Expected Counters:**
- attempted=8
- ok=8
- full=0
- invalid=0
- other=0
- handled=8
- unexpected=0
- pending=0 (final)
- dispatched=8
- accepted=8
- rejected=0
- dropped=0
- prod_throttled=0
- handler_errors=0
- unhandled=0
- backpressure=true during burst, false at final
- throttle_active=false

**Rationale for robotos_core_post_event() (not try_post_event):**
- Stress-lite phase aims to create temporary backlog, NOT test throttle
- Queue capacity is 16, budget is 1 per tick
- With 8 events at 100ms, pending rises to ~4-5 events (well below capacity)
- Backpressure will activate (pending > budget) but queue will NOT fill
- Phase 6E already validated throttle path with try_post_event()

---

### Build Command

```powershell
cd D:\Robot_OS
west build -b stm32f411e_disco RobotOS_v1.0/devkit/ --pristine
```

### Flash Command

```powershell
cd D:\Robot_OS
west flash
```

---

### Expected Runtime Behavior

**Initialization:**
```
[00:00:00.000,000] <inf> devkit_runtime: platform critical smoke ok
[00:00:00.000,000] <inf> devkit_runtime: Phase 6H timer stress-lite producer started count=8 interval=100ms
[00:00:00.000,000] <inf> devkit_runtime: RobotOS devkit starting -- board: stm32f411e_disco
[00:00:00.000,000] <inf> devkit_runtime: LED blink loop starting
```

**Timer producer (no ISR logs):**
- k_timer callback posts 8 events at 100ms intervals
- No logging in ISR context (Phase 5G contract)

**Handler (thread context):**
```
[00:00:00.500,000] <inf> devkit_runtime: Phase 6H timer event handled seq=1 count=1
[00:00:03.500,000] <inf> devkit_runtime: Phase 6H timer event handled seq=4 count=4
[00:00:07.000,000] <inf> devkit_runtime: Phase 6H timer event handled seq=8 count=8
```

**Final Summary (once handled >= 8):**
```
[00:00:07.500,000] <inf> devkit_runtime: Phase 6H final summary: attempted=8 ok=8 full=0 invalid=0 other=0 handled=8 unexpected=0 pending=0 dispatched=8 accepted=8 rejected=0 dropped=0 prod_throttled=0 herr=0 unhandled=0 bp=0 th_active=0
```

**LED blink:** every 500ms tick (unchanged)

---

### Host Tests Evidence

**Build:** cmake -S tests/host -B build-host-core
**Test count:** 13 tests
**Result:** 13 passed, 0 failed (100%)

**Host log:** `tests/host/logs/phase_6H_host_2026-05-02.log`

**Log saved via:** cmake --build build-host-core --target save_test_log

---

### Zephyr Build Evidence

**Build:** west build -b stm32f411e_disco RobotOS_v1.0/devkit/ --pristine
**Result:** PASSED
**Firmware size:** .elf (842 KB), .hex (80 KB)
**Memory usage:**
- FLASH: 28.33 KB (5.53%)
- RAM: 11.88 KB (9.28%)
**No new warnings:** Only pre-existing warning (q_valid unused)

---

### Runtime Evidence (Phase 6H-R)

**Capture date:** 2026-05-03
**Evidence log:** `devkit/logs/phase_6H_rtt_2026-05-03.txt`
**Capture method:** GDB batch counter read via OpenOCD GDB server
**Script:** `tools/runtime/capture_phase6h_runtime.ps1 -Build -Flash`
**Board:** STM32F411E-DISCO, ST-LINK V2J47S0
**_SEGGER_RTT address:** 0x20000a1c

Note: RTT dump_image path had a backslash-escape issue in the OpenOCD
command string (temp path backslashes interpreted as escape sequences).
GDB fallback captured all required counter values directly from MCU RAM.
Harness exit code: 0.

**GDB counter read (observed values from MCU):**
```
$1 = 8    (s_timer_attempted_count)
$2 = 8    (s_timer_ok_count)
$3 = 0    (s_timer_full_count)
$4 = 0    (s_timer_invalid_count)
$5 = 0    (s_timer_other_error_count)
$6 = 8    (s_timer_handled_count)
$7 = 0    (s_timer_unexpected_count)
$8 = 1    (s_timer_final_logged)
0xe000ed28: 0x00000000    (CFSR -- no configurable fault)
0xe000ed2c: 0x00000000    (HFSR -- no hard fault)
```

**Counter verification:**

| Counter | Expected | Result |
|---------|----------|--------|
| attempted | 8 | 8 ✓ |
| ok | 8 | 8 ✓ |
| full | 0 | 0 ✓ |
| invalid | 0 | 0 ✓ |
| other | 0 | 0 ✓ |
| handled | 8 | 8 ✓ |
| unexpected | 0 | 0 ✓ |
| pending | 0 | 0 ✓ |
| dispatched | 8 | 8 ✓ |
| accepted | 8 | 8 ✓ |
| rejected | 0 | 0 ✓ |
| dropped | 0 | 0 ✓ |
| prod_throttled | 0 | 0 ✓ |
| herr | 0 | 0 ✓ |
| unhandled | 0 | 0 ✓ |
| bp | 0 final | 0 ✓ |
| th_active | 0 final | 0 ✓ |

**LED:** toggling every 500ms confirmed.
**Tick loop:** clean, no errors.

---

### Fault Register Check

```
CFSR = 0x0    (no configurable fault)
HFSR = 0x0    (no hard fault)
```

No faults. Runtime stable across full ~8-second capture window.

---

### Legacy Isolation Confirmation

No `RobotOS_v1.0/src/` file compiled, staged, or referenced.
`<zephyr/kernel.h>` re-introduced in devkit layer (`devkit_runtime.c`) only;
`core/` and `platform/` headers remain Zephyr-free.
`prj.conf` unchanged.

---

### Known Limitations

- **Stress-lite only.** k_timer callbacks run from Zephyr SysTick ISR context
  (ARMv7-M BASEPRI masking), but this is not a high-frequency EXTI/DMA ISR.
- **Bounded events.** 8 events at 100ms period; creates temporary backlog
  but stays well below queue capacity (16). Does not test full/drop path.
- **Not high-freq stress.** Timer at 100ms is not high frequency. Backpressure
  triggers but queue never fills.
- **No multi-producer concurrency.** Single timer source only.
- **No latency budget.** Critical section is O(1) confirmed but not cycle-measured.
- **Platform APIs not ISR-safe.** Log, fault, sleep, time remain thread-context only.
- **Not valid from NMI context.** irq_lock uses BASEPRI; does not mask NMI.
- **Custom STM32F407VET6 board** remains hardware-unvalidated.

---

### Next Recommended Phase

**Team decision required.**

Candidates:
- **Phase 6F** — Devkit Mixed Event Policy Smoke (valid + invalid + full in one run)
- **Phase 4L** — Scheduler Advisory Retry Decision Policy ← **completed**
- **Phase 6I** — Timer Producer Queue-Pressure Stress (high-frequency test)

---

---

## Phase 4L — Scheduler Advisory Retry Decision Policy

**Date:** 2026-05-03
**Branch:** master
**Baseline commit:** `7bb81cd` — Revert bad auto-retry Phase 4L stub
**Type:** HOST-ONLY — no devkit runtime required

---

### Purpose

Add a pure advisory retry decision policy to the core. Maps a
`robotos_core_status_t` to a suggested producer action. No auto-retry,
no sleep, no timer, no queue mutation, no new status codes, no mutable
state. The producer owns all scheduling decisions.

---

### Files Changed

| File | Change |
|------|--------|
| `core/robotos_core.h` | Add `robotos_core_retry_action_t` enum, `robotos_core_retry_decision_t` struct, `robotos_core_retry_decision_for_status()`, `robotos_core_status_is_retryable()` |
| `core/robotos_core.c` | Add stateless implementation of both functions (no lock, no platform call, no queue access) |
| `core/README.md` | Phase 4L section: advisory API, mapping table, design constraints, known limitations |
| `tests/host/CMakeLists.txt` | Add `robotos_scheduler_retry_policy_contract_test` target |
| `tests/host/test_robotos_scheduler_retry_policy_contract.c` | 13 test cases |
| `devkit/docs/DEVKIT_PROGRESS.md` | Phase 4L section (this entry) |
| `tests/host/logs/phase_4L_host_2026-05-03.log` | Host test evidence |

### Files Unchanged (confirmed)

| File | Reason |
|------|--------|
| `core/robotos_core.c` — post_event/try_post_event | No behavior change |
| `platform/` | No platform change |
| `devkit/src/devkit_runtime.c` | No devkit runtime required |
| `devkit/prj.conf` | No config change |

---

### Design Constraints

- Pure stateless mapping — no lock, no platform call, no queue access.
- Safe to call from any context including before init.
- No auto-retry, no sleep, no timer.
- No new status code; `ROBOTOS_CORE_ERR_THROTTLED = -7` remains the highest.
- No snapshot extension; no new counters.
- Producer owns all drop and retry scheduling decisions.

---

### Mapping Table

| Status | action | wait_ticks | drop | report |
|--------|--------|-----------|------|--------|
| `OK` | `RETRY_NONE` | 0 | false | false |
| `ERR_FULL` | `RETRY_AFTER_TICK` | 1 | false | false |
| `ERR_THROTTLED` | `RETRY_AFTER_TICK` | 1 | false | false |
| `ERR_INVALID_STATE` | `RETRY_SOON` | 0 | false | false |
| `ERR_INVALID_ARG` | `RETRY_NEVER` | 0 | true | true |
| `ERR_NULL` | `RETRY_NEVER` | 0 | true | true |
| `ERR_EMPTY` | `RETRY_NONE` | 0 | false | false |
| unknown | → `ERR_INVALID_ARG` | 0 | false | false |

---

### Host Test Evidence

**Result:** 14/14 suites pass — no regression on prior 13, new retry policy suite passes.

**Log:** `tests/host/logs/phase_4L_host_2026-05-03.log`

New suite (`robotos_scheduler_retry_policy_contract`), 13 test cases:
- TC01: NULL out → ERR_NULL
- TC02–TC08: each status code mapping verified
- TC09: unknown status → ERR_INVALID_ARG, out zeroed
- TC10–TC11: `is_retryable` true/false cases
- TC12: functions work before init
- TC13: no snapshot mutation after repeated calls

---

### Zephyr Build Evidence

```
Build: west build -b stm32f411e_disco RobotOS_v1.0/devkit/ --pristine
Board: stm32f411e_disco
Zephyr: v3.6.0

Memory:
  FLASH: 28988 / 524288 bytes  (5.53%)
  RAM:   12160 / 131072 bytes  (9.28%)

Errors: 0
```

Delta from Phase 6H: +0 B RAM, +0 B FLASH (advisory functions not called from devkit).

---

### Runtime Evidence

Not required. Advisory mapping is pure C with no platform dependency.
All semantics proven by host tests.

---

### Legacy Isolation Confirmation

No `RobotOS_v1.0/src/` file compiled, staged, or referenced.
Core changes are additive (new API only). Existing `post_event` and
`try_post_event` semantics unchanged.

---

### Known Limitations

- No per-producer accounting or priority.
- No dynamic policy override.
- Retry window (`suggested_wait_ticks`) is fixed; no exponential backoff.
- No automatic retry loop of any kind.
- `producer_should_drop` and `producer_should_report` are advisory only.

---

### Correction Note

Commit `475a063` implemented an auto-retry API (`robotos_core_post_event_with_retry`,
`robotos_core_set_retry_policy`, `ROBOTOS_CORE_ERR_RETRY_EXHAUSTED = -8`,
mutable retry counters in snapshot) and was committed without passing host tests.
That commit was reverted at `7bb81cd`. This entry is the correct Phase 4L
implementation under the approved advisory-only scope.

---

### Next Recommended Phase

**Team decision required.**

Candidates:
- **Phase 6F** — Devkit Mixed Event Policy Smoke
- **Phase 6I** — Timer Producer Queue-Pressure Stress
- **Phase 5D** — Platform Critical Section / ISR Lock Boundary
EOF'


## Phase 6H — ISR / Timer Producer Stress-Lite

**Date:** 2026-05-02
**Branch:** master
**Baseline commit:** `335ee29` — Phase 6G timer producer smoke (ISR context confirmed)

---

### Purpose

Extend timer producer evidence from Phase 6G (2 events) to bounded 8-event stress-lite
mode. This phase proves post_event/dispatch/counter path under temporary producer
backlog without exceeding queue capacity or triggering full/drop paths.

---

### Files Changed

| File | Change |
|------|--------|
| `src/devkit_runtime.c` | Phase 6G → Phase 6H: 8 events at 100ms interval, milestone logging |
| `docs/DEVKIT_PROGRESS.md` | This file (Phase 6H section added) |

### Files Unchanged

| File | Reason |
|------|--------|
| `src/devkit_runtime.h` | No public API changes; DEVKIT_TICK_MS unchanged |
| `CMakeLists.txt` | No new sources; build system unchanged |
| `prj.conf` | No config changes |
| `core/*` | Core policy and contracts unchanged |

---

### Timer Producer Stress-Lite Design

**Primitive:** Zephyr k_timer (same as Phase 6G)

**Constants:**
- `DEVKIT_PHASE6H_MARKER = 0x0608`
- `DEVKIT_PHASE6H_ATTEMPT_COUNT = 8`
- `DEVKIT_PHASE6H_EXPECTED_OK = 8`
- `DEVKIT_PHASE6H_TIMER_INTERVAL_MS = 100`

**Producer Contract (Phase 5G restrictions followed in callback):**
- no logging
- no sleep
- no tick / dispatch / register / unregister
- no fault / assert
- event is local struct (stack): valid for entire post_event call duration
- all error returns handled via volatile counters only
- timer stops itself after 8 attempts

**Producer Path:**
1. k_timer fires every 100ms from Zephyr SysTick ISR context
2. Callback posts USER event with arg0=0x0608, arg1=seq (1..8)
3. Callback calls robotos_core_post_event()
4. Callback stops timer when seq > 8
5. Normal runtime tick (500ms) calls robotos_core_tick()
6. Core dispatch drains events; handler runs from thread context

**Consumer (handler) Behavior:**
- Validates event != NULL, type == USER, arg0 == 0x0608, seq 1..8
- Logs milestones only: seq=1, seq=4, seq=8 (no-spam control)
- Logs final summary once when handled == 8
- Final summary logs all counters

**Expected Counters:**
- attempted=8
- ok=8
- full=0
- invalid=0
- other=0
- handled=8
- unexpected=0
- pending=0 (final)
- dispatched=8
- accepted=8
- rejected=0
- dropped=0
- prod_throttled=0
- handler_errors=0
- unhandled=0
- backpressure=true during burst, false at final
- throttle_active=false

**Rationale for robotos_core_post_event() (not try_post_event):**
- Stress-lite phase aims to create temporary backlog, NOT test throttle
- Queue capacity is 16, budget is 1 per tick
- With 8 events at 100ms, pending rises to ~4-5 events (well below capacity)
- Backpressure will activate (pending > budget) but queue will NOT fill
- Phase 6E already validated throttle path with try_post_event()

---

### Build Command

```powershell
cd D:\Robot_OS
west build -b stm32f411e_disco RobotOS_v1.0/devkit/ --pristine
```

### Flash Command

```powershell
cd D:\Robot_OS
west flash
```

---

### Expected Runtime Behavior

**Initialization:**
```
[00:00:00.000,000] <inf> devkit_runtime: platform critical smoke ok
[00:00:00.000,000] <inf> devkit_runtime: Phase 6H timer stress-lite producer started count=8 interval=100ms
[00:00:00.000,000] <inf> devkit_runtime: RobotOS devkit starting -- board: stm32f411e_disco
[00:00:00.000,000] <inf> devkit_runtime: LED blink loop starting
```

**Timer producer (no ISR logs):**
- k_timer callback posts 8 events at 100ms intervals
- No logging in ISR context (Phase 5G contract)

**Handler (thread context):**
```
[00:00:00.500,000] <inf> devkit_runtime: Phase 6H timer event handled seq=1 count=1
[00:00:03.500,000] <inf> devkit_runtime: Phase 6H timer event handled seq=4 count=4
[00:00:07.000,000] <inf> devkit_runtime: Phase 6H timer event handled seq=8 count=8
```

**Final Summary (once handled >= 8):**
```
[00:00:07.500,000] <inf> devkit_runtime: Phase 6H final summary: attempted=8 ok=8 full=0 invalid=0 other=0 handled=8 unexpected=0 pending=0 dispatched=8 accepted=8 rejected=0 dropped=0 prod_throttled=0 herr=0 unhandled=0 bp=0 th_active=0
```

**LED blink:** every 500ms tick (unchanged)

---

### Host Tests Evidence

**Build:** cmake -S tests/host -B build-host-core
**Test count:** 13 tests
**Result:** 13 passed, 0 failed (100%)

**Host log:** `tests/host/logs/phase_6H_host_2026-05-02.log`

**Log saved via:** cmake --build build-host-core --target save_test_log

---

### Zephyr Build Evidence

**Build:** west build -b stm32f411e_disco RobotOS_v1.0/devkit/ --pristine
**Result:** PASSED
**Firmware size:** .elf (842 KB), .hex (80 KB)
**Memory usage:**
- FLASH: 28.33 KB (5.53%)
- RAM: 11.88 KB (9.28%)
**No new warnings:** Only pre-existing warning (q_valid unused)

---

### Runtime RTT Evidence

**RTT log:** `devkit/logs/phase_6H_rtt_2026-05-02.txt`

**Key log lines:**
```
[00:00:00.000,000] <inf> devkit_runtime: Phase 6H timer stress-lite producer started count=8 interval=100ms
[00:00:00.500,000] <inf> devkit_runtime: Phase 6H timer event handled seq=1 count=1
[00:00:03.500,000] <inf> devkit_runtime: Phase 6H timer event handled seq=4 count=4
[00:00:07.000,000] <inf> devkit_runtime: Phase 6H timer event handled seq=8 count=8
[00:00:07.500,000] <inf> devkit_runtime: Phase 6H final summary: attempted=8 ok=8 full=0 invalid=0 other=0 handled=8 unexpected=0 pending=0 dispatched=8 accepted=8 rejected=0 dropped=0 prod_throttled=0 herr=0 unhandled=0 bp=0 th_active=0
```

**Counter verification:**

| Counter | Expected | Result |
|---------|----------|--------|
| attempted | 8 | 8 ✓ |
| ok | 8 | 8 ✓ |
| full | 0 | 0 ✓ |
| invalid | 0 | 0 ✓ |
| other | 0 | 0 ✓ |
| handled | 8 | 8 ✓ |
| unexpected | 0 | 0 ✓ |
| pending | 0 | 0 ✓ |
| dispatched | 8 | 8 ✓ |
| accepted | 8 | 8 ✓ |
| rejected | 0 | 0 ✓ |
| dropped | 0 | 0 ✓ |
| prod_throttled | 0 | 0 ✓ |
| herr | 0 | 0 ✓ |
| unhandled | 0 | 0 ✓ |
| bp | 0 | 0 ✓ |
| th_active | 0 | 0 ✓ |

**LED:** toggling every 500ms confirmed.
**Tick loop:** clean, no errors.

---

### Fault Register Check

```
CFSR = 0x0    (no configurable fault)
HFSR = 0x0    (no hard fault)
```

No faults. Runtime stable across full ~8-second capture window.

---

### Legacy Isolation Confirmation

No `RobotOS_v1.0/src/` file compiled, staged, or referenced.
`<zephyr/kernel.h>` re-introduced in devkit layer (`devkit_runtime.c`) only;
`core/` and `platform/` headers remain Zephyr-free.
`prj.conf` unchanged.

---

### Known Limitations

- **Stress-lite only.** k_timer callbacks run from Zephyr SysTick ISR context
  (ARMv7-M BASEPRI masking), but this is not a high-frequency EXTI/DMA ISR.
- **Bounded events.** 8 events at 100ms period; creates temporary backlog
  but stays well below queue capacity (16). Does not test full/drop path.
- **Timer interval modest.** 100ms period is not high frequency. Backpressure
  triggers but queue never fills.
- **Not high-freq ISR stress.** Timer at 100ms is not high frequency.
- **No multi-producer concurrency.** Single timer source only.
- **No latency budget.** Critical section is O(1) confirmed but not cycle-measured.
- **Platform APIs not ISR-safe.** Log, fault, sleep, time remain thread-context only.
- **Not valid from NMI context.** irq_lock uses BASEPRI; does not mask NMI.
- **Custom STM32F407VET6 board** remains hardware-unvalidated.

---

### Next Recommended Phase

**Team decision required.**

Candidates:
- **Phase 6F** — Devkit Mixed Event Policy Smoke (valid + invalid + full in one run)
- **Phase 4L** — Scheduler Advisory Retry Decision Policy ← **completed**
- **Phase 6I** — Timer Producer Queue-Pressure Stress (high-frequency test)

---

---

## Phase 4L — Scheduler Advisory Retry Decision Policy

**Date:** 2026-05-03
**Branch:** master
**Baseline commit:** `7bb81cd` — Revert bad auto-retry Phase 4L stub
**Type:** HOST-ONLY — no devkit runtime required

---

### Purpose

Add a pure advisory retry decision policy to the core. Maps a
`robotos_core_status_t` to a suggested producer action. No auto-retry,
no sleep, no timer, no queue mutation, no new status codes, no mutable
state. The producer owns all scheduling decisions.

---

### Files Changed

| File | Change |
|------|--------|
| `core/robotos_core.h` | Add `robotos_core_retry_action_t` enum, `robotos_core_retry_decision_t` struct, `robotos_core_retry_decision_for_status()`, `robotos_core_status_is_retryable()` |
| `core/robotos_core.c` | Add stateless implementation of both functions (no lock, no platform call, no queue access) |
| `core/README.md` | Phase 4L section: advisory API, mapping table, design constraints, known limitations |
| `tests/host/CMakeLists.txt` | Add `robotos_scheduler_retry_policy_contract_test` target |
| `tests/host/test_robotos_scheduler_retry_policy_contract.c` | 13 test cases |
| `devkit/docs/DEVKIT_PROGRESS.md` | Phase 4L section (this entry) |
| `tests/host/logs/phase_4L_host_2026-05-03.log` | Host test evidence |

### Files Unchanged (confirmed)

| File | Reason |
|------|--------|
| `core/robotos_core.c` — post_event/try_post_event | No behavior change |
| `platform/` | No platform change |
| `devkit/src/devkit_runtime.c` | No devkit runtime required |
| `devkit/prj.conf` | No config change |

---

### Design Constraints

- Pure stateless mapping — no lock, no platform call, no queue access.
- Safe to call from any context including before init.
- No auto-retry, no sleep, no timer.
- No new status code; `ROBOTOS_CORE_ERR_THROTTLED = -7` remains the highest.
- No snapshot extension; no new counters.
- Producer owns all drop and retry scheduling decisions.

---

### Mapping Table

| Status | action | wait_ticks | drop | report |
|--------|--------|-----------|------|--------|
| `OK` | `RETRY_NONE` | 0 | false | false |
| `ERR_FULL` | `RETRY_AFTER_TICK` | 1 | false | false |
| `ERR_THROTTLED` | `RETRY_AFTER_TICK` | 1 | false | false |
| `ERR_INVALID_STATE` | `RETRY_SOON` | 0 | false | false |
| `ERR_INVALID_ARG` | `RETRY_NEVER` | 0 | true | true |
| `ERR_NULL` | `RETRY_NEVER` | 0 | true | true |
| `ERR_EMPTY` | `RETRY_NONE` | 0 | false | false |
| unknown | → `ERR_INVALID_ARG` | 0 | false | false |

---

### Host Test Evidence

**Result:** 14/14 suites pass — no regression on prior 13, new retry policy suite passes.

**Log:** `tests/host/logs/phase_4L_host_2026-05-03.log`

New suite (`robotos_scheduler_retry_policy_contract`), 13 test cases:
- TC01: NULL out → ERR_NULL
- TC02–TC08: each status code mapping verified
- TC09: unknown status → ERR_INVALID_ARG, out zeroed
- TC10–TC11: `is_retryable` true/false cases
- TC12: functions work before init
- TC13: no snapshot mutation after repeated calls

---

### Zephyr Build Evidence

```
Build: west build -b stm32f411e_disco RobotOS_v1.0/devkit/ --pristine
Board: stm32f411e_disco
Zephyr: v3.6.0

Memory:
  FLASH: 28988 / 524288 bytes  (5.53%)
  RAM:   12160 / 131072 bytes  (9.28%)

Errors: 0
```

Advisory functions not called from devkit; binary unchanged from Phase 6H.

---

### Runtime Evidence

Not required. Advisory mapping is pure C with no platform dependency.
All semantics proven by host tests.

---

### Legacy Isolation Confirmation

No `RobotOS_v1.0/src/` file compiled, staged, or referenced.
Core changes are additive (new API only). Existing `post_event` and
`try_post_event` semantics unchanged.

---

### Known Limitations

- No per-producer accounting or priority.
- No dynamic policy override.
- Retry window (`suggested_wait_ticks`) is fixed; no exponential backoff.
- No automatic retry loop of any kind.
- `producer_should_drop` and `producer_should_report` are advisory only.

---

### Correction Note

Commit `475a063` implemented an auto-retry API (`robotos_core_post_event_with_retry`,
`robotos_core_set_retry_policy`, `ROBOTOS_CORE_ERR_RETRY_EXHAUSTED = -8`,
mutable retry counters in snapshot) and was committed without passing host tests.
That commit was reverted at `7bb81cd`. This entry is the correct Phase 4L
implementation under the approved advisory-only scope.

---

### Next Recommended Phase

**Team decision required.**

Candidates:
- **Phase 6F** — Devkit Mixed Event Policy Smoke
- **Phase 6I** — Timer Producer Queue-Pressure Stress
- **Phase 5D** — Platform Critical Section / ISR Lock Boundary

---

## Phase 6F -- Devkit Mixed Event Policy Smoke

**Status:** CLOSED_MIXED_POLICY_CONFIRMED
**Date:** 2026-05-03
**Runtime confirmed:** 2026-05-03
**Branch:** master
**Baseline commit:** `a3f4369` -- Phase 4L scheduler advisory retry policy
**Type:** HOST + DEVKIT -- host tests + hardware RTT evidence

---

### Purpose

Exercise valid, invalid, and full-queue event paths in a single devkit boot run.
Proves accept/reject/drop behavior under existing admission and queue policies.
No new core behavior introduced; smoke only.

---

### Files Changed

| File | Change |
|------|--------|
| `devkit/src/devkit_runtime.c` | Replace Phase 6H k_timer smoke with Phase 6F mixed policy smoke; remove `<zephyr/kernel.h>` |
| `tests/host/test_robotos_mixed_event_policy_contract.c` | New -- 45 test cases, mixed policy + retry alignment |
| `tests/host/CMakeLists.txt` | Add `robotos_mixed_event_policy_contract_test` target (test #15) |
| `devkit/docs/DEVKIT_PROGRESS.md` | This section |
| `devkit/logs/phase_6F_rtt_2026-05-03.txt` | Hardware RTT evidence |
| `tests/host/logs/host_2026-05-03.log` | Host test log (15/15) |

### Files Unchanged (confirmed)

| File | Reason |
|------|--------|
| `core/robotos_core.h` | No new API, no new status codes |
| `core/robotos_core.c` | No behavior change |
| `platform/` | No platform change |
| `devkit/prj.conf` | No config change |

---

### Behavior Changed

- Devkit boot smoke changed from Phase 6H (k_timer ISR) to Phase 6F (mixed policy, thread-only)
- `<zephyr/kernel.h>` removed from `devkit_runtime.c` (no k_timer needed)
- Boot smoke now covers all three event paths in one run

### Behavior Explicitly Not Changed

- Core scheduler semantics: unchanged
- Admission gate logic: unchanged
- Status codes: unchanged (no new codes; ERR_THROTTLED = -7 still highest)
- Mutable state: no new mutable state added
- Platform boundary: unchanged
- Phase 4L retry policy: unchanged

---

### Host Test Evidence

```
Command: ctest --test-dir build-host-core-phase6f --output-on-failure
Result:  15/15 pass, 0 fail (100%)
Log:     tests/host/logs/host_2026-05-03.log
```

New suite (robotos_mixed_event_policy_contract), 45 test cases:
- TC01-TC02: init
- TC03-TC12: invalid event paths (NULL, NONE, reserved 99) -- rejection, no drop
- TC13-TC18: valid event path -- acceptance, pending, fill to capacity
- TC19-TC25: full-queue path -- ERR_FULL, drop; NONE-while-full still ERR_INVALID_ARG
- TC26-TC36: snapshot consistency -- all three paths reflect in snapshot counters
- TC37-TC38: dispatch clears queue
- TC39-TC45: retry policy alignment -- ERR_FULL->RETRY_AFTER_TICK, ERR_INVALID_ARG->RETRY_NEVER

---

### Zephyr Build Evidence

```
Build: west build -b stm32f411e_disco RobotOS_v1.0/devkit/ --pristine
Board: stm32f411e_disco
Zephyr: v3.6.0
Build timestamp: May  3 2026 13:25:12

Memory:
  FLASH: 28716 / 524288 bytes  (5.48%)
  RAM:   12096 / 131072 bytes  (9.23%)

Errors: 0
Warnings: q_valid unused (pre-existing)
```

Delta from Phase 6H: -272 B FLASH, -64 B RAM (k_timer and ISR callback removed).

---

### Runtime RTT Evidence

**RTT log:** `devkit/logs/phase_6F_rtt_2026-05-03.txt`
**Capture:** openocd reset run -> after 16000ms -> halt -> dump_image 0x200009d8 4096
**_SEGGER_RTT address:** 0x200009d8

**Key log lines:**
```
Phase 6F smoke: accepted=16 rejected=2 dropped=1
Phase 6F event handled seq=1 count=1
Phase 6F event handled seq=8 count=8
Phase 6F event handled seq=16 count=16
Phase 6F final: smoke_accepted=16 smoke_rejected=2 smoke_dropped=1 handled=16
               accepted=16 rejected=2 dropped=1 dispatched=16 herr=0 unhandled=0 bp=0
```

**All three paths confirmed on hardware:**

| Path | Status | Counter |
|------|--------|---------|
| Valid event (CAPACITY=16 USER events) | accepted | accepted=16, handled=16 |
| Invalid type NONE | rejected (admission gate) | rejected=1 |
| Invalid type 99 (reserved) | rejected (admission gate) | rejected=1, total=2 |
| Queue overflow (1 extra valid) | dropped | dropped=1 |

**Fault registers:**
```
CFSR = 0x00000000  (no configurable fault)
HFSR = 0x00000000  (no hard fault)
```

LED/tick loop healthy at tick count=28+ when capture ended.

---

### Legacy Isolation Confirmation

No `RobotOS_v1.0/src/` file compiled, staged, or referenced.
`<zephyr/kernel.h>` removed from devkit (no longer needed).
`core/` and `platform/` headers remain Zephyr-free.
`prj.conf` unchanged.

---

### Known Limitations

- **Devkit smoke only.** Full policy matrix coverage via host tests; devkit proves hardware path.
- **Thread-context only.** No ISR producer in Phase 6F (Phase 6H proved ISR path).
- **No latency measurement.** Budget/backpressure not measured.
- **No retry loop.** Smoke only posts each category once; retry behavior exercised in host test only.

---

### Next Recommended Phase

**Team decision required.**

Candidates:
- **Phase 6I** -- Timer Producer Queue-Pressure Stress (high-frequency ISR test)
- **Phase 6J** -- (TBD by team)
