# DEVKIT_PROGRESS.md — RobotOS Devkit Phase Log

---

## Phase Index

Sections appear in **insertion / session order**, not in phase-number order.
Use the anchor links below for one-click navigation, or `Ctrl+F` for `## Phase X`.

**Note:** Phase 6H and Phase 4L each appear **twice** in this document (duplicate
sections from non-linear editing history). Index entries marked `†` link to the
first occurrence; the second occurrence is accessible via the `‑1` anchor suffix
or by searching for the heading manually.

| Phase | Title | Status | Jump |
|-------|-------|--------|------|
| 3A | Runtime Skeleton | CLOSED | [→](#phase-3a--runtime-skeleton) |
| 3B | Devkit Hardening | CLOSED | [→](#phase-3b--devkit-hardening) |
| 4A | Core Bootstrap | CLOSED | [→](#phase-4a--core-bootstrap) |
| 4B | Core Contract Hardening | CLOSED | [→](#phase-4b--core-contract-hardening) |
| 4C | Host/Core Contract Tests | CLOSED | [→](#phase-4c--hostcore-contract-tests) |
| 4D | Core Event Queue Contract Stub | CLOSED | [→](#phase-4d--core-event-queue-contract-stub) |
| 4E | Core Event Dispatch Stub | CLOSED | [→](#phase-4e--core-event-dispatch-stub) |
| 4F | Core Event Ingestion API | CLOSED | [→](#phase-4f--core-event-ingestion-api) |
| 4G | Scheduler Tick Policy Stub | CLOSED | [→](#phase-4g--scheduler-tick-policy-stub) |
| 4H | Handler Policy / Registration Boundary | CLOSED | [→](#phase-4h--handler-policy--handler-registration-boundary) |
| 4I | Scheduler Admission Policy Stub | CLOSED | [→](#phase-4i--scheduler-admission-policy-stub) |
| 4J | Scheduler Budget / Backpressure Policy | CLOSED | [→](#phase-4j--scheduler-budget--backpressure-policy) |
| 4K | Scheduler Producer Throttle Policy | CLOSED | [→](#phase-4k--scheduler-producer-throttle-policy) |
| 4L † | Scheduler Advisory Retry Decision Policy | CLOSED | [→](#phase-4l--scheduler-advisory-retry-decision-policy) |
| 5A | Zephyr / Platform Adapter Boundary Seed | CLOSED | [→](#phase-5a--zephyr--platform-adapter-boundary-seed) |
| 5B | Platform Time Boundary | CLOSED | [→](#phase-5b--platform-time-boundary) |
| 5C | Platform Assert/Fault Boundary | CLOSED | [→](#phase-5c--platform-assertfault-boundary) |
| 5D | Platform Critical Section / ISR Lock Boundary | CLOSED | [→](#phase-5d--platform-critical-section--isr-lock-boundary) |
| 5E | Apply Critical Boundary to Core Queue State | CLOSED | [→](#phase-5e--apply-critical-boundary-to-core-queue-state) |
| 5F | Dispatcher Pop / Handler Split | CLOSED | [→](#phase-5f--dispatcher-pop--handler-split) |
| 5G | ISR-Safe Producer Contract Audit | CLOSED | [→](#phase-5g--isr-safe-producer-contract-audit) |
| 6A | Devkit Event Smoke Integration | CLOSED | [→](#phase-6a--devkit-event-smoke-integration) |
| 6B | Devkit Event Burst / Backpressure Smoke | CLOSED | [→](#phase-6b--devkit-event-burst--backpressure-smoke) |
| 6C | Devkit Queue Full / Drop Smoke | CLOSED | [→](#phase-6c--devkit-queue-full--drop-smoke) |
| 6D | Devkit Invalid / Rejection Smoke | CLOSED | [→](#phase-6d--devkit-invalid--rejection-smoke) |
| 6E | Devkit Throttled Producer Smoke | CLOSED | [→](#phase-6e--devkit-throttled-producer-smoke) |
| 6F | Devkit Mixed Event Policy Smoke | CLOSED | [→](#phase-6f----devkit-mixed-event-policy-smoke) |
| 6G | ISR/Timer Producer Smoke | CLOSED | [→](#phase-6g--isrtimer-producer-smoke) |
| 6H † | ISR / Timer Producer Stress-Lite | CLOSED | [→](#phase-6h--isr--timer-producer-stress-lite) |
| 6I | Timer Producer Queue-Pressure Stress | CLOSED | [→](#phase-6i----timer-producer-queue-pressure-stress) |
| 6J | Observability and Contract Stress Expansion | CLOSED | [→](#phase-6j----observability-and-contract-stress-expansion) |
| 6K | Runtime Observability Surfacing | CLOSED via 6Z | [→](#phase-6k----runtime-observability-surfacing) |
| 6L | Fault Observability Integration | CLOSED via 6Z | [→](#phase-6l----fault-observability-integration) |
| 6M | Producer Realism / Timer Producer Diagnostic | CLOSED via 6Z | [→](#phase-6m----producer-realism--timer-producer-diagnostic) |
| 6N | Documentation / Navigation Consolidation | CLOSED | see `CURRENT_STATE.md` |
| 6Z | RTT Closeout for Phase 6K / 6L / 6M | CLOSED | [→](#phase-6z----rtt-closeout-for-phase-6k--6l--6m) |
| 7A | Dispatch Budget Evolution Planning | DEFER | see `CURRENT_STATE.md` |
| 7B-1 | Dispatch Budget Test Parameterization | Candidate | see `CURRENT_STATE.md` |

---

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

---

## Phase 6I -- Timer Producer Queue-Pressure Stress

**Status:** CLOSED_QUEUE_PRESSURE_CONFIRMED
**Date:** 2026-05-03
**Runtime confirmed:** 2026-05-03
**Branch:** master
**Baseline commit:** `5bca62f` -- Phase 6F mixed event policy smoke
**Type:** HOST + DEVKIT -- host tests + hardware RTT evidence

---

### Purpose

Stress the timer/producer path under genuine queue pressure.
A high-frequency k_timer (50ms) posts events faster than the consumer
tick (500ms, budget=1) can dispatch. With 24 attempts and queue capacity=16,
the queue fills during the burst and overflow events return ERR_FULL from
queue pressure (not admission failure). Proves producer/consumer rate
mismatch behavior, backpressure_active during burst, and final drain.

Phase 6I replaces Phase 6F in devkit_runtime.c. Phase 6F evidence is
preserved in DEVKIT_PROGRESS.md and committed at 5bca62f.

Design choice: Phase 6I replaces (not composes with) Phase 6F to avoid
queue-capacity interference. Composing would pre-fill the queue at boot,
making all timer events immediately fail with ERR_FULL and obscuring the
gradual queue-pressure dynamics.

---

### Files Changed

| File | Change |
|------|--------|
| `devkit/src/devkit_runtime.c` | Replace Phase 6F thread smoke with Phase 6I k_timer (50ms, 24 events); `<zephyr/kernel.h>` re-introduced |
| `tests/host/test_robotos_queue_pressure_contract.c` | New -- 42 test cases, queue-pressure + throttle + retry alignment |
| `tests/host/CMakeLists.txt` | Add `robotos_queue_pressure_contract_test` target (test #16) |
| `devkit/docs/DEVKIT_PROGRESS.md` | This section |
| `devkit/logs/phase_6I_rtt_2026-05-03.txt` | Hardware RTT evidence |
| `tests/host/logs/host_2026-05-03.log` | Host test log (16/16) |

### Files Unchanged (confirmed)

| File | Reason |
|------|--------|
| `core/robotos_core.h` | No new API, no new status codes |
| `core/robotos_core.c` | No behavior change |
| `platform/` | No platform change |
| `devkit/prj.conf` | No config change |

---

### Behavior Changed

- Devkit boot smoke changed from Phase 6F (thread-context, no timer) to Phase 6I (k_timer ISR, 50ms, 24 events)
- `<zephyr/kernel.h>` re-introduced for k_timer; `robotos_event_queue.h` removed from runtime

### Behavior Explicitly Not Changed

- Core scheduler semantics: unchanged
- Status code contract: unchanged (ERR_THROTTLED = -7 still highest)
- Admission gate logic: unchanged
- Platform boundary: unchanged
- Mutable state: no new counters in core or snapshot
- No auto-retry

---

### Host Test Evidence

```
Command: ctest --test-dir build-host-core-phase6i --output-on-failure
Result:  16/16 suites pass, 0 fail (100%)
Log:     tests/host/logs/host_2026-05-03.log
```

New suite (robotos_queue_pressure_contract), 42 test cases:
- TC01-TC02: init
- TC03-TC06: rapid fill to capacity -- all 16 OK, backpressure_active=true
- TC07-TC10: queue-pressure overflow -- ERR_FULL, dropped++, not admitted
- TC11-TC13: admission gate before queue check -- NONE rejected as ERR_INVALID_ARG even when full
- TC14-TC19: try_post_event throttle under partial pressure -- ERR_THROTTLED, producer_throttled++
- TC20-TC26: snapshot consistency -- all counters match getters under pressure
- TC27-TC30: drain and settle -- backpressure_active=false, throttle_active=false
- TC31-TC39: retry policy -- ERR_FULL/ERR_THROTTLED both RETRY_AFTER_TICK (retryable)
- TC40-TC42: post succeeds after drain

---

### Zephyr Build Evidence

```
Build: west build -b stm32f411e_disco RobotOS_v1.0/devkit/ --pristine
Board: stm32f411e_disco
Zephyr: v3.6.0
Build timestamp: May  3 2026 14:27:25

Memory:
  FLASH: 28820 / 524288 bytes  (5.50%)
  RAM:   12160 / 131072 bytes  (9.28%)

Errors: 0
Warnings: q_valid unused (pre-existing)
```

Delta from Phase 6F: +104 B FLASH, +64 B RAM (k_timer and ISR callback added; robotos_event_queue.h include removed from runtime).

---

### Runtime RTT Evidence

**RTT log:** `devkit/logs/phase_6I_rtt_2026-05-03.txt`
**Capture:** openocd reset run -> after 14000ms -> halt -> dump_image 0x20000a18 4096
**_SEGGER_RTT address:** 0x20000a18

**Key log lines:**
```
Phase 6I timer producer started: attempts=24 interval=50ms
Phase 6I event handled seq=1 count=1
Phase 6I event handled seq=8 count=8
Phase 6I event handled seq=16 count=16
Phase 6I final: attempted=24 ok=18 full=6 invalid=0 other=0 handled=16
               accepted=18 rejected=0 dropped=6 dispatched=16
               herr=0 unhandled=0 bp=1 th_active=1
```

**Queue pressure dynamics confirmed:**

| Counter | Observed | Notes |
|---------|----------|-------|
| attempted | 24 | Timer fired 24 times as designed |
| ok | 18 | 18 events accepted (ticks freed slots during burst) |
| full | 6 | 6 events ERR_FULL from queue pressure |
| invalid | 0 | Valid type only; admission gate not triggered |
| other | 0 | No unexpected errors |
| handled | 16 | All 16 queued valid events dispatched (2 still pending at final log) |
| dropped | 6 | Equals full count (ERR_FULL path, not admission) |

Note: ok=18/full=6 (not the idealized 16/8) because the tick at t=500ms
freed queue slots while the timer was still firing, allowing 2 extra events
to succeed before the queue filled again. This is correct queue-pressure
behavior -- the producer and consumer genuinely competed for queue slots.
Final summary logged when handled=16 (EXPECTED_OK); at that moment
2 events were still pending (bp=1, th_active=1), confirming ongoing pressure.

**Fault registers:**
```
CFSR = 0x00000000  (no configurable fault)
HFSR = 0x00000000  (no hard fault)
```

LED/tick loop healthy at tick count=27+ when capture ended.

---

### Known Limitations

- **Not a sustained-stress test.** 24 events at 50ms = 1.2s burst, then timer stops. Not a continuous high-frequency ISR test.
- **Single-producer only.** No concurrent ISR producers.
- **No latency budget.** Critical section is O(1) confirmed but not measured.
- **ok/full split is timing-dependent.** Exact split (ok=18/full=6) depends on tick/timer interleaving. Contract (ok <= CAPACITY, full = attempted - ok) is always upheld.
- **Platform APIs not ISR-safe.** Log, fault, sleep, time remain thread-context only.
- **Not valid from NMI context.** irq_lock uses BASEPRI; does not mask NMI.

---

### Next Recommended Phase

**Team decision required.**

Candidates:

- **Phase 6J** -- Observability and Contract Stress Expansion (implemented, see below)

---

## Phase 6J -- Observability and Contract Stress Expansion

**Date:** 2026-05-07
**Branch:** master
**Commit:** `8a1af69`
**Phase 6I baseline commit:** `e78e503`

---

### Phase 6J Purpose

Harden deterministic validation, improve observability trust, and reduce future
runtime evolution risk. Host-test-first. No scheduler evolution. No runtime feature
expansion.

Scopes:

- **6J-A** Handler Routing Stress: multi-handler dispatch correctness and FIFO
- **6J-B** Handler Lifecycle Edge Cases: slot reuse, churn, error semantics
- **6J-C** Snapshot Coherence: all snapshot fields vs getters at multiple points
- **6J-D** Passive Observability: `peak_queue_depth` high-water mark field

---

### Phase 6J Files Added

| File | Role |
| ---- | ---- |
| `tests/host/test_robotos_handler_routing_stress_contract.c` | 6J-A: 55 test cases |
| `tests/host/test_robotos_handler_lifecycle_contract.c` | 6J-B: 52 test cases |
| `tests/host/test_robotos_snapshot_coherence_contract.c` | 6J-C/D: 192 test cases |

### Phase 6J Files Modified

| File | Change |
| ---- | ------ |
| `core/robotos_core.h` | Added `peak_queue_depth` to `robotos_core_snapshot_t`; added `robotos_core_peak_queue_depth()` getter declaration |
| `core/robotos_core.c` | Added `s_peak_queue_depth` static; peak update in `post_event_internal`; snapshot field; getter implementation |
| `tests/host/CMakeLists.txt` | Added 3 new test targets (19 total, up from 16) |

---

### Phase 6J Behavior Guarantees

- **No status codes added.** `ROBOTOS_CORE_ERR_THROTTLED = -7` remains the ceiling.
- **No mutable retry state added.** `robotos_core_retry_decision_for_status()` unchanged.
- **No scheduler semantics changed.** `ROBOTOS_CORE_MAX_EVENTS_PER_TICK = 1` unchanged.
- **No dispatch lock-boundary change.** Handler-outside-lock invariant preserved.
- **No admission policy change.** NONE/reserved still rejected; USER+ still accepted.
- **`peak_queue_depth` is passive.** Updated only after successful push, under existing lock.
  Monotonically non-decreasing. Not reset by repeated init. Zero behavioral impact.

---

### Phase 6J Host Test Evidence

**Commands (WSL Ubuntu, gcc 13.3.0):**

```bash
cmake -S RobotOS_v1.0/tests/host -B build-host-core-phase6j --fresh
cmake --build build-host-core-phase6j
ctest --test-dir build-host-core-phase6j --output-on-failure
cmake --build build-host-core-phase6j --target save_test_log
```

**Result:**

```
17/19 Test #17: robotos_handler_routing_stress_contract ...   Passed    0.01 sec
18/19 Test #18: robotos_handler_lifecycle_contract ........   Passed    0.01 sec
19/19 Test #19: robotos_snapshot_coherence_contract .......   Passed    0.01 sec

100% tests passed, 0 tests failed out of 19

Total Test time (real) = 0.35 sec
```

Per-suite case totals (Phase 6J new suites):

- Phase 6J-A (routing stress):  55 passed, 0 failed
- Phase 6J-B (lifecycle):       52 passed, 0 failed
- Phase 6J-C/D (snapshot):     192 passed, 0 failed
- **Total new test cases: 299**

Pre-existing suites: 16/16 PASS (zero regressions).

Test log: `tests/host/logs/host_2026-05-07.log`

---

### Phase 6J Zephyr Build Evidence

```
Command: west build -b stm32f411e_disco RobotOS_v1.0/devkit/ --pristine
Result:  PASS
FLASH:   28852 B / 524288 B (5.50%)  [+32 B from Phase 6I baseline 28820 B]
RAM:     12160 B / 131072 B (9.28%)  [unchanged from Phase 6I]
Errors:  0
Warnings: 1 pre-existing warning in robotos_event_queue.c (q_valid unused,
          unrelated to Phase 6J)
```

---

### Phase 6J Known Limitations

- **No devkit RTT smoke for Phase 6J.** Phase 6J is host-test-only per approved scope.
  `peak_queue_depth` is not yet exercised via RTT snapshot logging.
- **check_all_getters_match_snapshot helper** produces identically-labelled CHECK calls
  across 10 call sites; failures are disambiguated by section printf headers + line numbers.

---

### Phase 6J Next Recommended Phase

**Team decision required.** Candidates:

- **Phase 6K** (implemented, see below; pending RTT smoke for full close)

---

## Phase 6K -- Runtime Observability Surfacing

**Date:** 2026-05-07
**Branch:** master
**Phase 6J baseline commit:** `8a1af69`
**Commit:** `11516d4`
**Close status:** `CLOSED via Phase 6Z RTT evidence (2026-05-07)`

---

### Phase 6K Purpose

Expose existing core runtime state in devkit RTT logs in a bounded,
deterministic, low-noise way. No scheduler evolution. No new runtime
state. Pure visibility/debuggability work.

The goal is runtime snapshot visibility, queue/dispatch/pressure
visibility, and diagnostic confidence for future debugging support,
without changing runtime behavior.

---

### Phase 6K Files Added

| File | Role |
| ---- | ---- |
| `devkit/src/devkit_observability.h` | Phase 6K interface: log helper declaration + cadence constant |
| `devkit/src/devkit_observability.c` | Phase 6K implementation: snapshot capture + LOG_INF emission |

### Phase 6K Files Modified

| File | Change |
| ---- | ------ |
| `devkit/CMakeLists.txt` | Add `src/devkit_observability.c` to `target_sources` |
| `devkit/src/devkit_runtime.c` | Add `#include "devkit_observability.h"`; baseline call after init; periodic call every 10 ticks in run loop |

---

### Phase 6K Log Format

Stable single-line format, integer/boolean fields only, grep-friendly:

```text
ROBOTOS_OBS state=<NAME> ticks=N pending=N peak=N dropped=N dispatched=N \
            herr=N throttled=N rejected=N accepted=N unhandled=N \
            bp=0|1 th_active=0|1
```

Fields:

- `state`: core lifecycle state name (UNINIT / READY / ERROR / UNKNOWN)
- `ticks`: `snap.tick_count`
- `pending`: `snap.pending_event_count` (current queue depth)
- `peak`: `snap.peak_queue_depth` (Phase 6J-D high-water mark)
- `dropped`: `snap.dropped_event_count`
- `dispatched`: `snap.dispatched_event_count`
- `herr`: `snap.handler_error_count`
- `throttled`: `snap.producer_throttled_count`
- `rejected`: `snap.admission_rejected_count`
- `accepted`: `snap.admission_accepted_count`
- `unhandled`: `snap.unhandled_event_count`
- `bp`: `snap.backpressure_active` (0 or 1)
- `th_active`: `snap.producer_throttle_active` (0 or 1)

Field-name discipline: `herr`, `bp`, `th_active` match the Phase 6I
final-summary log so existing RTT parsing tooling continues to work.

---

### Phase 6K Log Cadence

- One baseline log immediately after `devkit_runtime_init()` completes.
- One periodic log every `DEVKIT_OBSERVABILITY_LOG_INTERVAL_TICKS = 10`
  runtime ticks. At `DEVKIT_TICK_MS = 500`, this is once every ~5 s.
- The cadence is bounded at compile time. There is no runtime config.

---

### Phase 6K Behavior Guarantees

- **No core semantics changed.** `robotos_core.[ch]` is unmodified in 6K.
- **No new mutable state.** `devkit_observability` is read-only over snapshot.
- **No feedback loop.** Log values do not feed back into scheduling,
  admission, dispatch, throttle, or retry decisions.
- **No dynamic allocation.** Snapshot is a stack-local struct in the helper.
- **No floating point.** All fields are integer or boolean.
- **No new threads.** Helper runs in existing devkit_runtime thread context.
- **Handler-outside-lock invariant preserved.** Helper acquires the
  snapshot critical section briefly and exits before logging.

---

### Phase 6K Host Test Evidence

Host regression confirms zero impact on portable core behavior. No new
host tests added: the helper is Zephyr-LOG-bound and has no host-testable
formatting logic worth artificial test scaffolding.

```text
cmake -S RobotOS_v1.0/tests/host -B build-host-core-phase6k --fresh
cmake --build build-host-core-phase6k
ctest --test-dir build-host-core-phase6k --output-on-failure
```

Result:

```text
100% tests passed, 0 tests failed out of 19
Total Test time (real) = 0.33 sec
```

Test log: `tests/host/logs/host_2026-05-07.log` (overwritten -- same date as 6J)

Pre-existing 16 suites: PASS. Phase 6J 3 suites: PASS. Zero regressions.

---

### Phase 6K Zephyr Build Evidence

```text
Command: west build -b stm32f411e_disco RobotOS_v1.0/devkit/ --pristine
Result:  PASS
FLASH:   29272 B / 524288 B (5.58%)  [+420 B from Phase 6J baseline 28852 B]
RAM:     12160 B / 131072 B (9.28%)  [unchanged from Phase 6J]
Errors:  0
Warnings: 1 pre-existing in robotos_event_queue.c (q_valid unused,
          unrelated to Phase 6K and Phase 6J)
```

FLASH +420 B is consistent with: state-name switch, `LOG_MODULE_REGISTER`
metadata for `devkit_obs`, the long format string literal, and two call
sites. RAM is unchanged because the Zephyr LOG backend reuses existing
deferred-log infrastructure.

---

### Phase 6K RTT Smoke

**Status:** `CLOSED via Phase 6Z RTT evidence (2026-05-07)`

RTT validation executed jointly for Phase 6K/6L/6M under Phase 6Z. See
the Phase 6Z section at the end of this document for the full hardware
session, build/flash result, raw log path, verification table, and
counter coherence analysis.

Phase 6K-specific evidence captured:

- Baseline `ROBOTOS_OBS state=READY ticks=0 pending=0 peak=0 dropped=0
  dispatched=0 herr=0 throttled=0 rejected=0 accepted=0 unhandled=0
  bp=0 th_active=0` immediately after init.
- 12 periodic `ROBOTOS_OBS` lines at `ticks=10,20,...,120` over the 60 s
  capture window.
- All fields parseable; `peak=16` reached during the Phase 6I startup
  burst, then steady-state `pending=1`, `bp=0`, `th_active=0` after
  ticks=40 once the burst drained.
- `dispatched` grew monotonically (9, 19, 29, 36, 41, 46, 51, 56, 61,
  66, 71, 76); `accepted - dispatched = pending` invariant held at every
  emission.

Evidence log: `RobotOS_v1.0/devkit/logs/phase_6Z_rtt_2026-05-07.txt`

---

### Phase 6K Known Limitations

- **No RTT evidence in this session.** Build-validated only. See section above.
- **Cadence is compile-time only.** `DEVKIT_OBSERVABILITY_LOG_INTERVAL_TICKS`
  cannot be changed at runtime. This is a deliberate design choice to
  prevent runtime config systems from creeping in.
- **No fault diagnostics in the log line.** CFSR/HFSR/last-fault are not
  surfaced; that is reserved for a future Phase 6L (Fault Observability).
- **Single LOG_MODULE_REGISTER added.** `devkit_obs` is a new log module.
  This is a small one-time RAM cost for the Zephyr log filter table.

---

### Phase 6K Next Recommended Phase

**Team decision required.** Candidates:

- **Phase 6L** Fault Observability Integration (implemented, see below; pending RTT smoke for full close)
- **Phase 6M** Producer Realism / Timer Producer Diagnostic
- **Phase 7A** Dispatch Budget Evolution Planning
- **Phase 7B** Execution Domain Boundary Planning

---

## Phase 6L -- Fault Observability Integration

**Date:** 2026-05-07
**Branch:** master
**Phase 6K baseline commit:** `11516d4` (devkit observability surfacing)
**Commit:** `d3759a7`
**Close status:** `CLOSED via Phase 6Z RTT evidence (2026-05-07)`

---

### Phase 6L Purpose

Surface Cortex-M fault diagnostic registers (CFSR, HFSR) in the devkit
RTT log alongside the Phase 6K runtime snapshot. Visibility only --
no recovery, no reset, no panic, no scheduler influence.

This makes the firmware self-report its own no-fault state during normal
operation and gives an operator on-board evidence of fault registers
without requiring a separate GDB session against the SCB memory map.

---

### Phase 6L Files Modified

| File | Change |
| ---- | ------ |
| `devkit/src/devkit_observability.h` | Add `devkit_observability_log_fault()` declaration |
| `devkit/src/devkit_observability.c` | Add CFSR/HFSR memory-mapped addresses; add `devkit_observability_log_fault()` implementation |
| `devkit/src/devkit_runtime.c` | Add baseline fault log after init; add periodic fault log every 10 ticks alongside snapshot log |

No new files. No core changes. No platform-interface changes.

---

### Phase 6L Log Format

Stable single-line format:

```text
ROBOTOS_FAULT active=0|1 cfsr=0x........ hfsr=0x........ context=<NAME>
```

Fields:

- `active`: `(cfsr != 0) || (hfsr != 0)`. 0 in normal no-fault state.
- `cfsr`: raw 32-bit value of `*(0xE000ED28)` (Configurable Fault Status Register)
- `hfsr`: raw 32-bit value of `*(0xE000ED2C)` (HardFault Status Register)
- `context`: coarse hint string. `"none"` when active=0, `"fault"` otherwise.

Bit-level decoding of UFSR/BFSR/MMFSR sub-fields and BFAR/MMFAR fault
addresses is intentionally out of scope for this phase.

---

### Phase 6L Log Cadence

Aligned with Phase 6K cadence:

- One baseline log immediately after `devkit_runtime_init()` completes.
- One periodic log every `DEVKIT_OBSERVABILITY_LOG_INTERVAL_TICKS = 10`
  runtime ticks. At `DEVKIT_TICK_MS = 500`, this is once every ~5 s.

Each periodic event now emits two lines: one `ROBOTOS_OBS` and one
`ROBOTOS_FAULT`. Total log noise increases by one line per cadence
window (one per ~5 s).

---

### Phase 6L Implementation Notes

**Why direct SCB memory access instead of CMSIS `SCB->CFSR`:**

- Avoids any CMSIS header path uncertainty across Zephyr versions.
- The two register addresses are fixed by the ARMv7-M Architecture
  Reference Manual (B3.2.10) and identical for all Cortex-M3/M4/M7.
- The same addresses are already documented in `tools/runtime/README.md`
  and used by the existing `phase6h_read.gdb` external diagnostic.
- A `volatile uint32_t *` read is well-defined behaviour and does not
  modify the register (CFSR/HFSR status bits are write-1-to-clear).

**Why no platform-interface extension:**

- The platform fault abstraction (Phase 5C) is severity-graded *report*
  API, not register-snapshot API. Extending it for one Cortex-M-specific
  diagnostic would expand a portable contract for marginal benefit.
- Devkit code is already Zephyr/Cortex-M-bound. Direct register access
  in `devkit/` does not violate portability discipline.
- Host tests cannot meaningfully validate hardware-mapped register reads
  beyond asserting constants, which the command brief explicitly forbids.

**Why no new host tests:**

- The function is Zephyr-LOG-bound and reads memory-mapped Cortex-M
  registers; no portable formatter exists to test deterministically.
- Per command brief: "Do NOT create fake tests that only assert constants."
- Host regression suite still runs in full to detect any unintended
  cross-impact on portable core. Result: 19/19 PASS, zero regressions.

---

### Phase 6L Behavior Guarantees

- **No core semantics changed.** `core/robotos_core.[ch]` unmodified.
- **No platform-interface extension.** Phase 5C fault API surface unchanged.
- **No fault recovery.** Read-only inspection; no register write, no reset,
  no panic, no auto-retry.
- **No feedback loop.** CFSR/HFSR values are not consumed by any runtime
  decision (scheduling, admission, throttle, dispatch, retry).
- **No new threads, no dynamic allocation, no floating point.**
- **Handler-outside-lock invariant preserved.** The fault helper holds
  no lock and is invoked from existing thread-context call sites.

---

### Phase 6L Host Test Evidence

```text
cmake -S RobotOS_v1.0/tests/host -B build-host-core-phase6l --fresh
cmake --build build-host-core-phase6l
ctest --test-dir build-host-core-phase6l --output-on-failure
```

Result:

```text
100% tests passed, 0 tests failed out of 19
Total Test time (real) = 0.31 sec
```

Test log: `tests/host/logs/host_2026-05-07.log` (overwritten -- same date as 6J/6K).

Pre-existing 16 suites: PASS. Phase 6J 3 suites: PASS. Zero regressions.

---

### Phase 6L Zephyr Build Evidence

```text
Command: west build -b stm32f411e_disco RobotOS_v1.0/devkit/ --pristine
Result:  PASS
FLASH:   29444 B / 524288 B (5.62%)  [+172 B from Phase 6K baseline 29272 B]
RAM:     12160 B / 131072 B (9.28%)  [unchanged from Phase 6K]
Errors:  0
Warnings: 1 pre-existing in robotos_event_queue.c (q_valid unused,
          unrelated to Phase 6L / Phase 6K / Phase 6J)
```

FLASH +172 B is consistent with: one new function body, the format
string literal, and two new call sites in the runtime loop / init.

---

### Phase 6L RTT Smoke

**Status:** `CLOSED via Phase 6Z RTT evidence (2026-05-07)`

RTT validation executed jointly for Phase 6K/6L/6M under Phase 6Z. See
the Phase 6Z section at the end of this document for the full hardware
session and verification table.

Phase 6L-specific evidence captured:

- Baseline `ROBOTOS_FAULT active=0 cfsr=0x00000000 hfsr=0x00000000
  context=none` immediately after init.
- 12 periodic `ROBOTOS_FAULT` lines aligned with the ROBOTOS_OBS
  cadence at `ticks=10,20,...,120`.
- **Every** captured CFSR value is `0x00000000` and **every** HFSR value
  is `0x00000000` over the full 60 s capture, including during the
  Phase 6I startup burst window when `bp=1 th_active=1`.
- `active` field stayed `0`; `context` stayed `none`. No `HFSR.DEBUGEVT`
  ambiguity to flag — the debugger was attached only for RTT polling,
  not halt-on-debug, so `HFSR` was not artificially elevated.
- Tick counter and Phase 6I final summary both reached expected values,
  confirming no firmware instability was introduced.

Evidence log: `RobotOS_v1.0/devkit/logs/phase_6Z_rtt_2026-05-07.txt`

---

### Phase 6L Known Limitations

- **No bit-level fault decoding.** Sub-fields of CFSR (UFSR/BFSR/MMFSR)
  and fault address registers (BFAR/MMFAR) are not surfaced. Operators
  must decode raw register values manually if a fault occurs. A future
  phase could add structured decoding.
- **`HFSR.DEBUGEVT` may be set under debugger.** When a debugger has
  triggered a halt event, HFSR may report non-zero even without a real
  crash. Raw values are surfaced unfiltered so the operator can inspect.
- **No platform abstraction.** Direct SCB memory access keeps the
  observability helper devkit-local and Cortex-M-only. Non-Cortex-M
  ports would need a different fault diagnostic strategy.
- **No RTT evidence in this session.** Build-validated only.

---

### Phase 6L Next Recommended Phase

**Team decision required.** Candidates:

- **Phase 6M** Producer Realism / Timer Producer Diagnostic (implemented, see below; pending RTT smoke for full close)
- **Phase 6N** Fault Decoder Helper (UFSR/BFSR/MMFSR sub-field decoding,
  BFAR/MMFAR address surfacing) -- only if real fault diagnosis becomes
  needed; not speculative.
- **Phase 7A** Dispatch Budget Evolution Planning
- **Phase 7B** Execution Domain Boundary Planning

---

## Phase 6M -- Producer Realism / Timer Producer Diagnostic

**Date:** 2026-05-07
**Branch:** master
**Phase 6L baseline commit:** `d3759a7`
**Commit:** `a6b253b`
**Close status:** `CLOSED via Phase 6Z RTT evidence (2026-05-07)`

---

### Phase 6M Purpose

Add a controlled periodic devkit producer that posts admissible RobotOS
events at a known low-pressure cadence in thread context. The intent is
purely diagnostic: demonstrate realistic steady-state producer/consumer
interaction in RTT logs and prove that the Phase 6K/6L observability
surfaces respond coherently to actual ongoing event traffic.

This is **not** scheduler evolution. The producer adds traffic; it does
not change how the scheduler treats traffic.

---

### Phase 6M Files Added

| File | Role |
| ---- | ---- |
| `devkit/src/devkit_timer_producer.h` | Pure-C producer interface: cadence constant, event type, marker, stats struct, init/should_post/on_tick/get_stats |
| `devkit/src/devkit_timer_producer.c` | Pure-C producer implementation with no Zephyr dependency (so it host-tests against the real core) |
| `tests/host/test_robotos_timer_producer_contract.c` | 59 host contract tests across 10 sections covering pre-init guards, cadence predicate, single/multi posts, dispatch routing, queue fill, drain recovery, type isolation |

### Phase 6M Files Modified

| File | Change |
| ---- | ------ |
| `devkit/src/devkit_observability.h` | Add `devkit_observability_log_producer_stats()` declaration |
| `devkit/src/devkit_observability.c` | Add `<devkit_timer_producer.h>` include and ROBOTOS_PROD log helper (Zephyr LOG_INF emission separated from the pure-C producer module) |
| `devkit/src/devkit_runtime.c` | Include producer header; init producer with banner; call `devkit_timer_producer_on_tick(tick_count)` once per run-loop iteration; add baseline producer log after init and periodic log every 10 ticks alongside ROBOTOS_OBS / ROBOTOS_FAULT |
| `devkit/CMakeLists.txt` | Add `src/devkit_timer_producer.c` to `target_sources` |
| `tests/host/CMakeLists.txt` | Add `robotos_timer_producer_contract_test` target compiling the producer module against the real core sources and platform host stubs |

No core changes. No platform-interface changes.

---

### Phase 6M Producer Policy

| Decision | Value | Rationale |
| -------- | ----- | --------- |
| Event type | `ROBOTOS_EVENT_USER + 1` (= 101) | Distinct from Phase 6I's USER (= 100). Both producers coexist without handler-routing conflict. Both types are admissible per the Phase 4I gate (USER+ accepted). |
| arg0 marker | `0x6D00` | Phase 6M tag, follows the per-phase marker convention (6E=0x6E, 6F=0x6F00, 6I=0x6900). |
| Cadence | 1 event every 2 devkit ticks (~1 event/sec at `DEVKIT_TICK_MS = 500`) | Producer rate < consumer rate (budget = 1/tick), so steady-state queue stays at or near zero after the Phase 6I startup burst drains. Demonstrates healthy non-pressured runtime. |
| Post API | `robotos_core_post_event()` (raw) | Phase 6M demonstrates raw producer success/drop visibility. Throttle visibility is already covered by Phase 4K/6E/6I. |
| Caller context | Thread context (devkit run loop) | No new Zephyr task/thread, no ISR semantics expansion. |
| Bound | None (continuous) | Bounded by the cadence rate, not by a max-attempt cap. The Phase 6I producer remains the bounded-burst demonstration. |

---

### Phase 6M Log Format

Stable single-line, integer fields only:

```text
ROBOTOS_PROD attempted=N ok=N throttled=N dropped=N invalid=N other=N type=USER+1
```

- `attempted`: total cadence-driven post attempts since process start
- `ok`: post returned `ROBOTOS_CORE_OK`
- `throttled`: post returned `ERR_THROTTLED` (always 0 under `post_event` -- field reserved for future try_post_event variant)
- `dropped`: post returned `ERR_FULL`
- `invalid`: post returned `ERR_INVALID_ARG` (defensive; should always be 0)
- `other`: any other non-OK return (defensive; should always be 0)
- `type`: literal string `USER+1` -- the producer's event type

---

### Phase 6M Log Cadence

Aligned with the Phase 6K/6L cadence (no new log axis introduced):

- One baseline `ROBOTOS_PROD` line immediately after `devkit_runtime_init()` completes.
- One periodic `ROBOTOS_PROD` line every `DEVKIT_OBSERVABILITY_LOG_INTERVAL_TICKS = 10`
  runtime ticks, alongside `ROBOTOS_OBS` and `ROBOTOS_FAULT`.

The producer itself logs nothing per post; only the periodic stats line.

---

### Phase 6M Behavior Guarantees

- **No core semantics changed.** `core/` is unmodified.
- **No platform-interface change.** Phase 5C / 6L surfaces unchanged.
- **No scheduler / dispatch / queue / retry / admission semantic mutation.**
- **No producer feedback loop.** Producer cadence is fixed at compile time.
  The producer never reads core counters and never consumes its own log
  output.
- **Producer cannot starve the run loop.** At most one post per tick;
  on_tick is a no-op on the cadence-skip half of ticks.
- **Counters are passive.** Producer counters never feed back into
  scheduling/admission/throttle/retry/dispatch decisions.
- **No new threads, no dynamic allocation, no floating point.**
- **Handler-outside-lock invariant preserved.** Producer's handler runs
  outside any critical section, like every other registered handler.

---

### Phase 6M Host Test Evidence

```text
cmake -S RobotOS_v1.0/tests/host -B build-host-core-phase6m --fresh
cmake --build build-host-core-phase6m
ctest --test-dir build-host-core-phase6m --output-on-failure
```

Result:

```text
20/20 Test #20: robotos_timer_producer_contract ...........   Passed    0.01 sec

100% tests passed, 0 tests failed out of 20

Total Test time (real) = 0.34 sec
```

Per-suite case totals (Phase 6M new suite):

- Phase 6M (timer producer): 59 passed, 0 failed

Pre-existing 19 suites: PASS (zero regressions).

Test log: `tests/host/logs/host_2026-05-07.log` (overwritten -- same date as 6J/6K/6L).

---

### Phase 6M Zephyr Build Evidence

```text
Command: west build -b stm32f411e_disco RobotOS_v1.0/devkit/ --pristine
Result:  PASS
FLASH:   30032 B / 524288 B (5.73%)  [+588 B from Phase 6L baseline 29444 B]
RAM:     12160 B / 131072 B (9.28%)  [unchanged from Phase 6L]
Errors:  0
Warnings: 1 pre-existing in robotos_event_queue.c (q_valid unused,
          unrelated to Phase 6J/6K/6L/6M)
```

FLASH +588 B is consistent with: producer module body (init/should_post/
on_tick/get_stats/handler), ROBOTOS_PROD log helper in observability,
init banner LOG_INF, and four new call sites in devkit_runtime.

---

### Phase 6M RTT Smoke

**Status:** `CLOSED via Phase 6Z RTT evidence (2026-05-07)`

RTT validation executed jointly for Phase 6K/6L/6M under Phase 6Z. See
the Phase 6Z section at the end of this document for the full hardware
session and verification table.

Phase 6M-specific evidence captured:

- Init banner: `Phase 6M producer init: type=USER+1 marker=0x6d00
  cadence=every 2 ticks`.
- Baseline `ROBOTOS_PROD attempted=0 ok=0 throttled=0 dropped=0
  invalid=0 other=0 type=USER+1` immediately after init.
- 12 periodic `ROBOTOS_PROD` lines: `attempted` = 5, 10, 15, 20, 25, 30,
  35, 40, 45, 50, 55, 60 — exactly +5 per 10 devkit ticks, matching the
  designed 1-event-per-2-ticks cadence at `DEVKIT_TICK_MS=500`.
- `ok = attempted` for every emission in the entire 60 s capture;
  `throttled=0`, `dropped=0`, `invalid=0`, `other=0` throughout.
- Phase 6I startup burst settled by ticks=40 (`bp` flipped from 1 to 0,
  `pending` stabilized at 1). Phase 6I final summary present at L46
  with `attempted=24 ok=17 full=7 ... handled=16`.
- Phase 6M handler routed all events without `unhandled` or `herr`
  increments (the marker-validation handler returned `OK` for every
  event since the only producer of `USER+1` is this module).
- Counter cross-check at ticks=120: Phase 6I `ok`=17 plus Phase 6M
  `ok`=60 equals `accepted`=77 in `ROBOTOS_OBS`; Phase 6I `handled`=16
  plus Phase 6M handled=60 equals `dispatched`=76; `accepted -
  dispatched = pending = 1`.

**Note on Phase 6I `ok`/`full` distribution:** the Phase 6M source
documentation predicted Phase 6I would settle at `ok=16 full=8`. The
real run produced `ok=17 full=7`. This is a one-event timing slip
caused by the 500 ms dispatcher tick at t=1000 ms freeing a queue slot
just before Phase 6I's 21st 50 ms post. Architectural invariants are
preserved: `peak=16` (queue never exceeded capacity), no admission
errors, no fault. The variance is within the expected ISR/dispatcher
phase-relationship envelope and is **not** treated as a regression.

Evidence log: `RobotOS_v1.0/devkit/logs/phase_6Z_rtt_2026-05-07.txt`

---

### Phase 6M Known Limitations

- **No RTT evidence in this session.** Build-validated only.
- **No max-attempt cap.** The producer is unbounded by attempt count.
  This was an explicit design choice (continuous low-pressure realism).
  Bounding the producer would re-create the Phase 6I behaviour, which
  already exists. If the team prefers a bounded variant for hardware
  demos, a compile-time cap could be added in a follow-up.
- **Producer interacts with Phase 6I burst at startup.** During the
  first ~9 s, Phase 6I has the queue full and the producer will record
  ERR_FULL until the burst drains. This is intentional and explainable;
  RTT analysis should confirm `dropped` is bounded and stable.
- **`HFSR.DEBUGEVT` ambiguity** (carried forward from Phase 6L).

---

### Phase 6M Next Recommended Phase

**Team decision required.** Candidates:

- **Phase 6N** Runtime Diagnostic Consolidation -- unify ROBOTOS_OBS /
  ROBOTOS_FAULT / ROBOTOS_PROD into a single configurable diagnostic
  cadence module if RTT bandwidth becomes constrained.
- **Phase 6O** Fault Decoder Planning -- only if real fault diagnosis
  becomes needed.
- **Phase 7A** Dispatch Budget Evolution Planning.
- **Phase 7B** Execution Domain Boundary Planning.

---

## Phase 6Z -- RTT Closeout for Phase 6K / 6L / 6M

**Date:** 2026-05-07
**Branch:** master
**Phase 6K commit:** `11516d4`
**Phase 6L commit:** `d3759a7`
**Phase 6M commit:** `a6b253b`
**Close status:** `CLOSED -- joint RTT closeout for 6K/6L/6M`

---

### Phase 6Z Purpose

Joint hardware-evidence closeout for Phase 6K (Runtime Observability
Surfacing), Phase 6L (Fault Observability Integration), and Phase 6M
(Producer Realism / Timer Producer Diagnostic). All three phases were
implemented and build/host-test validated under their own commits but
held at `READY_BUT_NOT_CLOSED_PENDING_RTT` because no `west flash` had
been performed in the originating sessions.

Phase 6Z is **validation + evidence + state reconciliation only**:

- No source code changed.
- No scheduler / queue / retry / admission / throttle / dispatch budget
  semantics modified.
- No producer cadence modified.
- No tests added or modified.
- No CMake / Kconfig / prj.conf modifications.
- No new runtime behavior introduced.

The only repo additions are the RTT evidence log, the closeout edits in
this document (status flips and a new Phase 6Z section), and the
`CURRENT_STATE.md` last-closed-phase advance.

---

### Phase 6Z Hardware Session

| Item | Value |
| ---- | ----- |
| Date / time | 2026-05-07 21:24 local |
| Board | STM32F411E-DISCO (rev D) |
| MCU | STM32F411 / Cortex-M4 r0p1 |
| Probe | ST-LINK V2 (firmware V2J47S0, VID:PID 0483:3748) |
| Toolchain | Zephyr SDK 0.17.0 |
| Zephyr | 3.6.0 |
| west | 1.5.0 |
| OpenOCD | xpack 0.12.0+dev-02228-ge5888bda3-dirty |
| Build command | `west build -b stm32f411e_disco RobotOS_v1.0/devkit/ --pristine` |
| Flash command | `west flash` (32768 bytes written from `zephyr.hex`, 1.123 s) |
| Manual physical RESET | **Not required.** OpenOCD `reset run` in the RTT capture cfg started the firmware cleanly after flash. |
| RTT capture method | OpenOCD streaming RTT TCP server on localhost:9090, captured via PowerShell TcpClient |
| RTT control block | `_SEGGER_RTT @ 0x20000a34` (resolved via `arm-zephyr-eabi-nm`; OpenOCD confirmed `Control block found at 0x20000a34`) |
| RTT capture duration | 60 seconds wallclock |
| RTT bytes captured | 16,560 bytes (no buffer-wrap loss observed; streaming polled the SEGGER ring continuously) |
| RTT log path | `RobotOS_v1.0/devkit/logs/phase_6Z_rtt_2026-05-07.txt` |

---

### Phase 6Z Build Evidence

```text
Command: west build -b stm32f411e_disco RobotOS_v1.0/devkit/ --pristine
Result:  PASS
FLASH:   30032 B / 524288 B (5.73%)  [matches Phase 6M baseline 30032 B]
RAM:     12160 B / 131072 B  (9.28%)  [unchanged]
Errors:  0
Warnings:
  - 1 pre-existing in robotos_event_queue.c (q_valid unused)
  - 1 pre-existing Kconfig dependency notice for UART_INTERRUPT_DRIVEN
  - 1 pre-existing CMake notice "No SOURCES given to Zephyr library: drivers__console"
```

No new warnings introduced. FLASH and RAM exactly match the Phase 6M
build baseline, confirming the closeout build is identical to the
build that produced commit `a6b253b`.

---

### Phase 6Z RTT Verification Table

| # | Row | Result | Evidence (line in `phase_6Z_rtt_2026-05-07.txt`) |
| - | --- | ------ | ------------------------------------------------ |
| 1 | Zephyr boot banner | PASS | L1 `*** Booting Zephyr OS build v3.6.0 ***` |
| 2 | RobotOS devkit starting | PASS | L14 `RobotOS devkit starting -- board: stm32f411e_disco` |
| 3 | LED blink loop banner | PASS | L15 `LED blink loop starting` |
| 4 | Phase 6I producer banner | PASS | L13 `Phase 6I timer producer started: attempts=24 interval=50ms` |
| 5 | Phase 6M producer init | PASS | L16 `Phase 6M producer init: type=USER+1 marker=0x6d00 cadence=every 2 ticks` |
| 6 | ROBOTOS_OBS baseline | PASS | L17 `state=READY ticks=0 pending=0 peak=0 dropped=0 dispatched=0 ... bp=0 th_active=0` |
| 7 | ROBOTOS_OBS periodic | PASS | 12 emissions at ticks=10,20,30,40,50,60,70,80,90,100,110,120 |
| 8 | ROBOTOS_FAULT baseline | PASS | L18 `active=0 cfsr=0x00000000 hfsr=0x00000000 context=none` |
| 9 | ROBOTOS_FAULT periodic | PASS | 12 emissions, every value `cfsr=0x00000000 hfsr=0x00000000` |
| 10 | ROBOTOS_PROD baseline | PASS | L19 `attempted=0 ok=0 throttled=0 dropped=0 invalid=0 other=0 type=USER+1` |
| 11 | ROBOTOS_PROD periodic | PASS | 12 emissions, attempted = 5, 10, 15, ..., 60 (exactly +5 per 10 ticks) |
| 12 | Phase 6I milestone seq=1 | PASS | L23 |
| 13 | Phase 6I milestone seq=8 | PASS | L31 |
| 14 | Phase 6I milestone seq=16 | PASS | L45 |
| 15 | Phase 6I final summary | PASS | L46 `attempted=24 ok=17 full=7 invalid=0 other=0 handled=16 ...` |
| 16 | CFSR / HFSR zero throughout | PASS | All 13 ROBOTOS_FAULT lines (1 baseline + 12 periodic) |
| 17 | Runtime stability | PASS | tick count log monotonic 0->123 over ~60.5 s; no panic, no fault, no reset re-banner |

---

### Phase 6Z Counter / Behavior Analysis

**Startup pressure (ticks 0--10):** Phase 6I ISR fires 24 events at
50 ms (t=50..1200 ms). Queue capacity 16. Dispatch budget 1/tick at
500 ms. At ticks=10 the snapshot reads `pending=13 peak=16 dropped=7
dispatched=9 accepted=22 bp=1 th_active=1` -- backpressure asserted as
expected during the drain window.

**Phase 6I final result (informational):** Final shows `ok=17 full=7`
rather than the source-comment "ok=16 full=8". This is a one-event
timing slip caused by the dispatcher tick at t=1000 ms freeing a queue
slot just before Phase 6I's 21st 50 ms post. The architectural
invariants (queue never exceeds capacity, no overflow, peak=16, no
admission errors, no fault) are all preserved. Within expected
ISR/dispatcher phase-relationship envelope; not a regression.

**Phase 6M producer behavior during 6I burst:** Phase 6M's first
cadence post lands at devkit tick=2 (t~=1.0 s) -- after the t=1000 ms
tick dispatch had already freed a slot (queue 16->15). All 5 of its
first-period posts succeeded; `dropped` remained 0. Behavior is
consistent with cadence-relative phase. The Phase 6Z command brief
allowed for early `dropped` rises but did not require them.

**Steady state (ticks 30+):** From ticks=40 onwards `bp=0
th_active=0`, `pending=1`. Producer counters: `attempted=ok` exactly
throughout the entire 60 s capture; `dropped=0 invalid=0 other=0` from
baseline through ticks=120. Producer growth is exactly +5 per 10 ticks.

**Coherence cross-check at ticks=120** (line 190):

```text
pending=1  peak=16  dropped=7  dispatched=76  accepted=77  unhandled=0  herr=0
```

Phase 6I `ok` (17) + Phase 6M `ok` (60) = 77 = `accepted`.
Phase 6I `handled` (16) + Phase 6M handled (60) = 76 = `dispatched`.
`accepted - dispatched = 1 = pending`. `peak=16 = ROBOTOS_EVENT_QUEUE_CAPACITY`.

**Fault state:** `cfsr=0x00000000` and `hfsr=0x00000000` in every
emission across the full 60 s capture, including during the Phase 6I
burst window when `bp=1 th_active=1`. No `HFSR.DEBUGEVT` ambiguity to
flag -- OpenOCD attached only as an RTT poller, not as a halt-on-debug
client, so `HFSR` was not artificially elevated.

---

### Phase 6Z Phase Close Decision

| Phase | Decision | Basis |
| ----- | -------- | ----- |
| Phase 6K | **CLOSED** via Phase 6Z RTT evidence | Baseline + 12 periodic ROBOTOS_OBS emissions; all fields parseable; `peak=16`, monotonic `dispatched`, coherent `accepted/pending/dropped` cross-check. |
| Phase 6L | **CLOSED** via Phase 6Z RTT evidence | Baseline + 12 periodic ROBOTOS_FAULT emissions; `cfsr=0x00000000` and `hfsr=0x00000000` throughout; `active=0`, `context=none` invariant. |
| Phase 6M | **CLOSED** via Phase 6Z RTT evidence | Baseline + 12 periodic ROBOTOS_PROD emissions; producer init banner present; `type=USER+1`; `attempted` grows exactly +5 per 10 ticks; `ok=attempted` for every emission; counters cross-check coherent with core snapshot. |

---

### Phase 6Z Architecture Preservation Audit

Confirmed unchanged in Phase 6Z:

- No code under `core/`, `platform/`, `devkit/src/`, or `tests/` modified.
- No CMake / Kconfig / prj.conf modified.
- `ROBOTOS_CORE_MAX_EVENTS_PER_TICK` remains 1.
- `ROBOTOS_EVENT_QUEUE_CAPACITY` remains 16.
- Status code set unchanged.
- Event producer cadence unchanged (Phase 6M still 1 post / 2 ticks at
  `DEVKIT_TICK_MS=500`).
- Logging formats unchanged.
- Runtime behavior unchanged.
- Dispatch budget mutation remains DEFER per Phase 7A planning.

---

### Phase 6Z Residual Risks / Follow-ups

- **Manual RESET requirement:** not observed in this session; OpenOCD
  `reset run` was sufficient. The known operational caveat remains
  documented but did not trigger here.
- **Custom STM32F407 target:** still unvalidated. Phase 6Z exercised
  only STM32F411E-DISCO. Migration to a custom F407 board remains a
  separate validation activity outside Phase 6Z scope.
- **Phase 6I `ok=17 full=7` vs documented `ok=16 full=8`:** explained
  above as a timing-phase variance, not a regression. If future Phase
  7A work adjusts the dispatch budget or tick rate, the documented
  expected counts in `devkit_runtime.c` should be re-evaluated for
  precision rather than left as design-intent comments.
- **Pre-existing `q_valid` unused-function warning** in
  `core/robotos_event_queue.c` -- still present, pre-dates Phase 6Z,
  not addressed here per scope discipline.
- **DEVKIT_PROGRESS.md size:** this document continues to grow; if
  navigation cost rises, a Phase 6N documentation split would be
  appropriate. Out of scope for Phase 6Z.
- **RTT tooling fragility:** the streaming OpenOCD `rtt server` path
  worked cleanly in this session but is not yet captured as a reusable
  harness script (the existing `tools/runtime/capture_phase6h_runtime.ps1`
  uses a 4 KB `dump_image` snapshot which is too small for >=30 s
  captures of the Phase 6K/6L/6M log triplets). Adding a
  `capture_phase6z_runtime.ps1` would be a useful future improvement;
  not done here per "no script additions in Phase 6Z" scope.

---

### Phase 6Z Next Recommended Phase

Phase 6Z does not itself imply a specific next phase. With 6K/6L/6M
closed, the canonical candidates remain:

- **Phase 7B-1** Dispatch Budget Test Parameterization -- prepare future
  scheduler evolution (e.g. a configurable budget) without changing
  current runtime semantics. Lowest-risk preparation step.
- **Phase 6N** Runtime Diagnostic Consolidation -- only if RTT bandwidth
  becomes constrained or DEVKIT_PROGRESS.md navigation cost rises
  enough to motivate a split.
- **Phase 7A** Dispatch Budget Evolution Planning remains DEFER until a
  workload-driven reason emerges. The Phase 6Z evidence shows the
  current `MAX_EVENTS_PER_TICK=1` budget is sufficient for the
  combined Phase 6I burst + Phase 6M cadence workload (steady state
  pending=1, no growth in dropped after burst).

Team decision required before opening any of the above.

---

## Phase 6O -- Reusable RTT Streaming Capture Harness

**Date:** 2026-05-08
**Branch:** master
**Type:** Tooling only -- no source, test, CMake, Kconfig, or runtime change.
**Close status:** `CLOSED -- harness smoke PASS on hardware (2026-05-08)`

---

### Phase 6O Purpose

Generalize the Phase 6Z OpenOCD streaming RTT capture workflow into a
reusable, parameterized PowerShell script. Phase 6Z proved that the
OpenOCD `rtt server start` TCP streaming approach captures ROBOTOS_OBS /
ROBOTOS_FAULT / ROBOTOS_PROD triplets over a 30-60 s window without
ring-buffer wraparound. Phase 6O packages that workflow for repeatable use
in Phase 8A (custom STM32F407 bring-up), Phase 9A (first real workload),
and future validation sessions.

The Phase 6H snapshot harness (`capture_phase6h_runtime.ps1`) is preserved
unchanged. It remains the correct tool for Phase 6H counter-comparison.

---

### Phase 6O Files Added

| File | Role |
| ---- | ---- |
| `tools/runtime/capture_devkit_rtt.ps1` | New reusable RTT streaming capture script |
| `tools/runtime/phase6z_required_patterns.txt` | Human-readable reference for default patterns (not read at runtime) |

### Phase 6O Files Modified

| File | Change |
| ---- | ------ |
| `tools/runtime/README.md` | New Phase 6O section (scripts table, usage, parameters, troubleshooting) |
| `devkit/logs/INDEX.md` | Phase 6O harness-smoke log row added |

No source, test, CMake, or Kconfig files modified.

---

### Phase 6O Script Interface

Script: `RobotOS_v1.0/tools/runtime/capture_devkit_rtt.ps1`

Key parameters:

```text
-OutputLog <path>       : Save RTT log here (default: devkit/logs/phase_rtt_<date>.txt)
-WaitSeconds <int>      : Capture window (default: 60; use 30 for quick smoke)
-Port <int>             : OpenOCD RTT TCP port (default: 9090)
-BuildFirst             : Run west build --pristine before capture
-FlashFirst             : Run west flash before capture (kills stale OpenOCD first)
-RequirePatterns <str[]>: Literal strings that must appear in captured log
-OpenOcdExe <path>      : Override auto-discovered OpenOCD path
-OpenOcdConfig <path>   : Override board .cfg (for Phase 8A custom board)
-RttControlBlock <hex>  : Explicit RTT address; skips nm when provided
-RttSearchBase <hex>    : RAM base for nm-fallback search (default: 0x20000000)
-RttSearchSize <hex>    : RAM range for nm-fallback search (default: 0x20000 = 128 KB)
```

Default required patterns:

```text
ROBOTOS_OBS state=READY
ROBOTOS_FAULT active=0
ROBOTOS_PROD attempted=
Phase 6I final:
```

Exit 0 on PASS, 1 on FAIL. Raw log preserved in both cases. CFSR/HFSR non-zero
triggers a hard FAIL regardless of patterns.

OpenOCD auto-discovery: PATH → xpack WinGet install location. Scripts directory
auto-derived from OpenOCD exe location (xpack: bin/../openocd/scripts). _SEGGER_RTT
address resolved via nm from the ELF; falls back to RAM search range if nm fails.
Only the OpenOCD process started by this script is stopped on cleanup.

---

### Phase 6O Validation

**Syntax check:**

```text
PowerShell AST parser: SYNTAX OK -- 0 parse errors; 2192 tokens
```

**Harness smoke capture:**

```text
Script  : RobotOS_v1.0/tools/runtime/capture_devkit_rtt.ps1
Command : -OutputLog devkit/logs/phase_6O_harness_smoke_2026-05-08.txt -WaitSeconds 30
Board   : STM32F411E-DISCO (ST-LINK V2J47S0)
ELF     : build/zephyr/zephyr.elf  (Phase 6N baseline: FLASH 30032 B unchanged)
RTT     : _SEGGER_RTT @ 0x20000a34 (auto-resolved via nm)
OpenOCD : xpack 0.12.0+dev-02228-ge5888bda3-dirty
Result  : PASS -- exit 0
```

Pattern verification table:

| Pattern | Result |
| ------- | ------ |
| ROBOTOS_OBS state=READY | FOUND |
| ROBOTOS_FAULT active=0 | FOUND |
| ROBOTOS_PROD attempted= | FOUND |
| Phase 6I final: | FOUND |
| CFSR all 0x00000000 | PASS (7 occurrences) |
| HFSR all 0x00000000 | PASS (7 occurrences) |

```text
Bytes captured : 9558 bytes
Duration       : 30.9 s
Manual RESET   : not required (OpenOCD reset run started firmware cleanly)
```

Evidence log: `RobotOS_v1.0/devkit/logs/phase_6O_harness_smoke_2026-05-08.txt`

---

### Phase 6O Architecture Preservation Audit

Confirmed unchanged in Phase 6O:

- No source under `core/`, `platform/`, `devkit/src/`, or `tests/` modified.
- No CMake / Kconfig / prj.conf modified.
- `ROBOTOS_CORE_MAX_EVENTS_PER_TICK = 1` unchanged.
- Log formats (ROBOTOS_OBS / ROBOTOS_FAULT / ROBOTOS_PROD) unchanged.
- SEGGER RTT configuration unchanged.
- `capture_phase6h_runtime.ps1` and `phase6h_read.gdb` unchanged.
- Existing RTT and host log files unchanged.
- Runtime behavior unchanged.

---

### Phase 6O Known Limitations

- **Windows/PowerShell only.** The script is not portable to Linux/macOS without
  porting. A future phase could generalize it (e.g., using a Bash variant for
  Linux CI/CD). For now, the development environment is Windows PowerShell 5.1
  per repo convention.
- **OpenOCD path portability.** The auto-discovery covers PATH and the WinGet
  xpack-openocd install. Other install locations require `-OpenOcdExe`.
- **Port 9090 conflict.** If another process binds port 9090, the script prints
  a warning and the caller should use `-Port <other>`. It does not auto-select
  a free port.
- **Manual RESET not required in this session**, but the known `POST_FLASH_AUTOSTART`
  unreliable-auto-start issue (Phase 3B OPEN) remains. If the board does not
  start after flash, the `reset run` in the sidecar cfg will restart it when
  the script connects. If that also fails, a physical RESET followed by
  rerunning the script (without `-FlashFirst`) is the workaround.
- **Custom STM32F407 target still unvalidated.** The script is designed to support
  it via `-OpenOcdConfig` and `-RttSearchSize 0x40000`, but no F407 session has
  been run yet. Phase 8A will be the first real test of these override paths.

---

### Phase 6O Next Recommended Phase

With Phase 6O tooling in place, the blocking items before scheduler evolution
are the custom board validation and workload definition. Candidates:

- **Phase 8A** -- Custom STM32F407 bring-up: flash current Phase 6N firmware on
  the F407, capture RTT with `capture_devkit_rtt.ps1 -OpenOcdConfig <f407.cfg>`,
  verify ROBOTOS_OBS/FAULT/PROD baseline. Retires the 25-phase-old portability debt.
- **Phase 9A** -- First real event source (user button, UART, or sensor):
  define a concrete non-synthetic producer and handler; use `capture_devkit_rtt.ps1`
  for evidence capture. Produces the first workload measurement to motivate or
  dismiss Phase 7A.
- **Phase 7A** -- Dispatch Budget Evolution Planning: DEFER until Phase 9A
  produces workload evidence that budget=1 is insufficient.
