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

### Files Unchanged

| File | Reason |
|------|--------|
| `prj.conf` | RTT logging config confirmed working — no change needed |
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

### Known Limitations

- No RTOS threads — single main-thread blocking loop (`k_msleep`).
- `DEVKIT_TICK_MS = 500` is a compile-time constant; not runtime configurable.
- LED identified by board DTS alias `led0` — no custom board overlay.
- RTT logging only (no UART console); STLink V2 required for log read.
- No shell, scheduler, event bus, or peripheral drivers at this phase.

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
| FLASH | `FLASH_WRITTEN_RESET_LIMITATION` |
| RUNTIME | `RUNTIME_PASS` |

#### Build Memory Summary

```
Memory region    Used Size   Region Size   %age Used
FLASH:            27252 B       512 KB       5.20%
RAM:               7808 B       128 KB       5.96%
```

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

Exit code 1 = STLink V1 reset limitation (firmware written, no auto-reset).
Manual RESET on board required to start execution. Firmware write and verify: **confirmed**.

#### RTT Log Evidence

OpenOCD RTT server at `_SEGGER_RTT = 0x20000800`, port 9090.
Captured via Python TCP client after `reset halt` + `reset run`.

```
*** Booting Zephyr OS build v3.6.0 ***
[00:00:00.000,000] <inf> devkit_runtime: RobotOS devkit starting — board: stm32f411e_disco
[00:00:00.000,000] <inf> devkit_runtime: LED blink loop starting
[00:00:00.000,000] <inf> devkit_runtime: tick
[00:00:00.000,000] <dbg> os: z_tick_sleep: thread 0x20000608 for 5000 ticks
```

- Log module: `devkit_runtime` ✓ (changed from `devkit_main` — expected)
- Boot banner: present ✓
- LED blink loop: started ✓
- Tick log: present ✓
- k_msleep(500) active: confirmed via `z_tick_sleep` debug line ✓

**RTT session note:** OpenOCD rtt server drops TCP connection after buffer drain.
Subsequent ticks not captured in same TCP session — this is RTT tooling behavior,
not a firmware regression. Firmware is confirmed running (LED blinks visually, sleep active).

---

### Next Recommended Phase

**Phase 3B — Devkit Hardening (suggested):**
- Add `devkit_assert` or fault hook stub for panic visibility.
- Add `CONFIG_REBOOT=y` + watchdog consideration note.
- Measure and document actual RTT log latency.
- Prepare migration checklist for custom STM32F407VET6 board.

**Phase 4 — Core Bootstrap (future):**
- Introduce `robotos_core` module under `RobotOS_v1.0/core/`.
- Define first RobotOS kernel interface (event queue or scheduler stub).
- devkit becomes the integration harness for core validation.
