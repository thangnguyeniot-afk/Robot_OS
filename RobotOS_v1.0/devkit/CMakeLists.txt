# devkit/CMakeLists.txt
# Phase 3A runtime skeleton — STM32F411E-DISCO.
# Target: stm32f411e_disco
# Purpose: structured devkit runtime with status LED and tick loop.

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(robotos_devkit)

target_sources(app PRIVATE
    src/main.c
    src/devkit_status_led.c
    src/devkit_runtime.c
)
