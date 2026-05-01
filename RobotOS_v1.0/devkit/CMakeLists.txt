# devkit/CMakeLists.txt
# Phase 4D event queue — STM32F411E-DISCO.
# Target: stm32f411e_disco
# Purpose: runtime skeleton with RobotOS core + event queue integration.

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(robotos_devkit)

target_include_directories(app PRIVATE ../core)

target_sources(app PRIVATE
    src/main.c
    src/devkit_status_led.c
    src/devkit_runtime.c
    src/devkit_build_info.c
    src/devkit_fault.c
    ../core/robotos_core.c
    ../core/robotos_event_queue.c
)
