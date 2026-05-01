# devkit/CMakeLists.txt
# Phase 3B devkit hardening — STM32F411E-DISCO.
# Target: stm32f411e_disco
# Purpose: runtime skeleton with build metadata, fault visibility, and tick counter.

cmake_minimum_required(VERSION 3.20.0)

# Issue an explicit reset-run and wait 300ms before the built-in reset run.
# This gives the firmware time to boot and the SWD state to settle before
# the ST-LINK disconnects, preventing the CPU from being left halted after
# west flash (which requires a manual RESET button press to recover).
macro(app_set_runner_args)
    board_runner_args(openocd "--cmd-post-verify=reset run")
    board_runner_args(openocd "--cmd-post-verify=sleep 300")
endmacro()

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(robotos_devkit)

target_sources(app PRIVATE
    src/main.c
    src/devkit_status_led.c
    src/devkit_runtime.c
    src/devkit_build_info.c
    src/devkit_fault.c
)
