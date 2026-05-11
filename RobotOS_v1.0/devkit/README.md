# devkit/ — RobotOS Bringup Application (STM32F411E-DISCO)

## Purpose
Minimal Zephyr application for hardware validation.
Phase 1 goal: blink LED + LOG_INF over UART. No RobotOS framework.

## Board Target
`stm32f411e_disco` (confirmed in Zephyr v3.6.0 tree)

## Hardware (STM32F411E-DISCO)
- LED0 = PD13 (orange) — GPIO_ACTIVE_HIGH — alias `led0` in board DTS
- Console UART = USART2 (PA2=TX, PA3=RX) — 115200 baud
- **No custom overlay needed** — board DTS already defines all aliases

## Build
```powershell
# From D:\Robot_OS (west workspace root)
$env:ZEPHYR_TOOLCHAIN_VARIANT = "zephyr"
$env:ZEPHYR_SDK_INSTALL_DIR   = "C:\zephyr-sdk-0.17.0"
west build -b stm32f411e_disco RobotOS_v1.0/devkit/ --pristine
```

## Flash (STLINKv2 on disco board)
```powershell
west flash
```

## Expected Serial Output (RTT, Phase 6M+)

Output is via SEGGER RTT (captured by OpenOCD or J-Link RTTLogger).
The following lines appear at boot, taken verbatim from
`devkit/logs/phase_6Z_rtt_2026-05-07.txt`:

```text
*** Booting Zephyr OS build v3.6.0 ***
[00:00:00.000,000] <inf> devkit_runtime: Phase 6I timer producer started: attempts=24 interval=50ms
[00:00:00.000,000] <inf> devkit_runtime: RobotOS devkit starting -- board: stm32f411e_disco
[00:00:00.000,000] <inf> devkit_runtime: LED blink loop starting
[00:00:00.000,000] <inf> devkit_runtime: Phase 6M producer init: type=USER+1 marker=0x6d00 cadence=every 2 ticks
[00:00:00.000,000] <inf> devkit_obs: ROBOTOS_OBS state=READY ticks=0 pending=0 peak=0 dropped=0 dispatched=0 herr=0 throttled=0 rejected=0 accepted=0 unhandled=0 bp=0 th_active=0
[00:00:00.000,000] <inf> devkit_obs: ROBOTOS_FAULT active=0 cfsr=0x00000000 hfsr=0x00000000 context=none
[00:00:00.000,000] <inf> devkit_obs: ROBOTOS_PROD attempted=0 ok=0 throttled=0 dropped=0 invalid=0 other=0 type=USER+1
[00:00:00.000,000] <inf> devkit_runtime: tick count=0
...
```

Periodic ROBOTOS_OBS / ROBOTOS_FAULT / ROBOTOS_PROD triplets appear every 10
runtime ticks (~5 s at `DEVKIT_TICK_MS = 500`). For full telemetry field
definitions see `devkit/docs/03_SPECS/TELEMETRY_REFERENCE.md`.

## Future Custom Board Migration Checklist
When migrating to custom STM32F407VET6 board:
1. Create `boards/<custom_board_name>.dts` with correct LED and UART pin assignments
2. Create `boards/<custom_board_name>.overlay` if pin reassignment needed
3. Update `west build -b <custom_board_name>` in build commands
4. Validate `stm32f407` HAL is present in `modules/hal/stm32`
5. Confirm `arm-zephyr-eabi-gcc` targets Cortex-M4F (same toolchain, different linker script)
