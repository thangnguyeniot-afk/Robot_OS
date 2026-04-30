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

## Expected Serial Output (115200, 8N1)
```
*** Booting Zephyr OS build v3.6.0 ***
[00:00:00.000,000] <inf> devkit_main: RobotOS devkit starting — board: stm32f411e_disco
[00:00:00.000,000] <inf> devkit_main: LED blink loop starting
[00:00:00.000,000] <inf> devkit_main: tick
[00:00:00.500,000] <inf> devkit_main: tick
...
```

## Future Custom Board Migration Checklist
When migrating to custom STM32F407VET6 board:
1. Create `boards/<custom_board_name>.dts` with correct LED and UART pin assignments
2. Create `boards/<custom_board_name>.overlay` if pin reassignment needed
3. Update `west build -b <custom_board_name>` in build commands
4. Validate `stm32f407` HAL is present in `modules/hal/stm32`
5. Confirm `arm-zephyr-eabi-gcc` targets Cortex-M4F (same toolchain, different linker script)
