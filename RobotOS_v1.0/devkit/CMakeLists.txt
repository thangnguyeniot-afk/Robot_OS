# devkit/CMakeLists.txt
# Phase 5B platform time boundary — STM32F411E-DISCO.
# Target: stm32f411e_disco
# Purpose: runtime skeleton with RobotOS core + event queue + dispatcher integration.
#          Zephyr platform log adapter bridges core to Zephyr logging subsystem.
#          Zephyr platform time adapter bridges devkit_runtime sleep to k_msleep.

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(robotos_devkit)

target_include_directories(app PRIVATE
    ../core
    ../platform
)

target_sources(app PRIVATE
    src/main.c
    src/devkit_status_led.c
    src/devkit_runtime.c
    src/devkit_build_info.c
    src/devkit_fault.c
    ../core/robotos_core.c
    ../core/robotos_event_queue.c
    ../core/robotos_event_dispatcher.c
    ../platform/zephyr/robotos_platform_log_zephyr.c
    ../platform/zephyr/robotos_platform_time_zephyr.c
)
