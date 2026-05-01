# tests/host/CMakeLists.txt
# Phase 4C: isolated host build for robotos_core contract tests.
#
# Standalone — does not depend on root CMake, west, or legacy src/.
# Does not require Zephyr SDK or ZEPHYR_BASE.
#
# Build:
#   cmake -S RobotOS_v1.0/tests/host -B build-host-core
#   cmake --build build-host-core
#   ./build-host-core/robotos_core_contract_test  (Linux/Mac)
#   .\build-host-core\Debug\robotos_core_contract_test.exe  (Windows/MSVC)

cmake_minimum_required(VERSION 3.20)
project(robotos_core_host_tests C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Locate core/ relative to this CMakeLists (two levels up from tests/host/)
set(CORE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../core")

add_executable(robotos_core_contract_test
    test_robotos_core_contract.c
    "${CORE_DIR}/robotos_core.c"
)

target_include_directories(robotos_core_contract_test PRIVATE
    "${CORE_DIR}"
)

target_compile_definitions(robotos_core_contract_test PRIVATE
    ROBOTOS_CORE_HOST_TEST=1
)

enable_testing()
add_test(NAME robotos_core_contract COMMAND robotos_core_contract_test)
