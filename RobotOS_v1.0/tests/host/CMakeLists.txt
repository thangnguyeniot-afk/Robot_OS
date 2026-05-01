# tests/host/CMakeLists.txt
# Phase 4D: isolated host build for robotos_core and event queue contract tests.
#
# Standalone — does not depend on root CMake, west, or legacy src/.
# Does not require Zephyr SDK or ZEPHYR_BASE.
#
# Build:
#   cmake -S RobotOS_v1.0/tests/host -B build-host-core
#   cmake --build build-host-core
#   ctest --test-dir build-host-core --output-on-failure
#
# Note: Windows MinGW64 may be broken; use WSL Ubuntu or Linux host.

cmake_minimum_required(VERSION 3.20)
project(robotos_core_host_tests C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Locate core/ relative to this CMakeLists (two levels up from tests/host/)
set(CORE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../core")

# ---- Core contract test (Phase 4C) ----------------------------------------
# robotos_core.c now includes robotos_event_queue.h, so we must compile
# robotos_event_queue.c together with it.
add_executable(robotos_core_contract_test
    test_robotos_core_contract.c
    "${CORE_DIR}/robotos_core.c"
    "${CORE_DIR}/robotos_event_queue.c"
)

target_include_directories(robotos_core_contract_test PRIVATE
    "${CORE_DIR}"
)

# ROBOTOS_CORE_HOST_TEST silences Zephyr logging in robotos_core.c.
# robotos_event_queue.c has no Zephyr dependency — no define needed for it.
target_compile_definitions(robotos_core_contract_test PRIVATE
    ROBOTOS_CORE_HOST_TEST=1
)

# ---- Event queue contract test (Phase 4D) ----------------------------------
add_executable(robotos_event_queue_contract_test
    test_robotos_event_queue_contract.c
    "${CORE_DIR}/robotos_event_queue.c"
)

target_include_directories(robotos_event_queue_contract_test PRIVATE
    "${CORE_DIR}"
)

# No Zephyr shim needed — robotos_event_queue.c has zero Zephyr dependency.

enable_testing()
add_test(NAME robotos_core_contract       COMMAND robotos_core_contract_test)
add_test(NAME robotos_event_queue_contract COMMAND robotos_event_queue_contract_test)
