# devkit/CMakeLists.txt
# Minimal Zephyr application for STM32F411E-DISCO devkit bring-up.
# Target: stm32f411e_disco
# Purpose: blink LED, LOG_INF over UART — validate toolchain + Zephyr layer.

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(robotos_devkit)

target_sources(app PRIVATE src/main.c)
