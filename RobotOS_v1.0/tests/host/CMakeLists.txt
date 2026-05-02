# tests/host/CMakeLists.txt
# Phase 4I: isolated host build — core, queue, dispatcher, ingestion, tick policy, handler policy,
#           platform fault contract, scheduler admission policy.
#
# Platform logging: each target that compiles robotos_core.c also compiles
# the no-op host stub (robotos_platform_log_host_stub.c) and adds the
# platform include directory. No Zephyr dependency.
#
# ROBOTOS_CORE_HOST_TEST is no longer needed — the logging shim has been
# replaced by the platform log boundary introduced in Phase 5A.
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
set(CORE_DIR    "${CMAKE_CURRENT_SOURCE_DIR}/../../core")
set(PLATFORM_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../platform")

# Host no-op platform log backend — compiled into any target that includes core.
set(PLATFORM_HOST_STUB "${CMAKE_CURRENT_SOURCE_DIR}/robotos_platform_log_host_stub.c")

# Common core sources shared across tests that include robotos_core.h dependency chain.
set(CORE_SRCS
    "${CORE_DIR}/robotos_core.c"
    "${CORE_DIR}/robotos_event_queue.c"
    "${CORE_DIR}/robotos_event_dispatcher.c"
)

# ---- Core lifecycle contract test (Phase 4C) --------------------------------
# robotos_core.c includes event_queue and dispatcher headers.
# Platform log stub satisfies robotos_platform_logf() symbol.
add_executable(robotos_core_contract_test
    test_robotos_core_contract.c
    ${CORE_SRCS}
    ${PLATFORM_HOST_STUB}
)

target_include_directories(robotos_core_contract_test PRIVATE
    "${CORE_DIR}"
    "${PLATFORM_DIR}"
)

# ---- Event queue contract test (Phase 4D) -----------------------------------
# Only needs robotos_event_queue.c (no core or dispatcher dependency).
# Does not compile robotos_core.c — no platform stub needed.
add_executable(robotos_event_queue_contract_test
    test_robotos_event_queue_contract.c
    "${CORE_DIR}/robotos_event_queue.c"
)

target_include_directories(robotos_event_queue_contract_test PRIVATE "${CORE_DIR}")

# ---- Event dispatcher contract test (Phase 4E) ------------------------------
# Needs queue and dispatcher; does not include robotos_core.c.
# No platform stub needed.
add_executable(robotos_event_dispatcher_contract_test
    test_robotos_event_dispatcher_contract.c
    "${CORE_DIR}/robotos_event_queue.c"
    "${CORE_DIR}/robotos_event_dispatcher.c"
)

target_include_directories(robotos_event_dispatcher_contract_test PRIVATE "${CORE_DIR}")

# ---- Core ingestion contract test (Phase 4F) --------------------------------
# Separate executable = fresh static core state. Needs all three core sources.
add_executable(robotos_core_ingestion_contract_test
    test_robotos_core_ingestion_contract.c
    ${CORE_SRCS}
    ${PLATFORM_HOST_STUB}
)

target_include_directories(robotos_core_ingestion_contract_test PRIVATE
    "${CORE_DIR}"
    "${PLATFORM_DIR}"
)

# ---- Tick policy contract test (Phase 4G) -----------------------------------
add_executable(robotos_core_tick_policy_contract_test
    test_robotos_core_tick_policy_contract.c
    ${CORE_SRCS}
    ${PLATFORM_HOST_STUB}
)

target_include_directories(robotos_core_tick_policy_contract_test PRIVATE
    "${CORE_DIR}"
    "${PLATFORM_DIR}"
)

# ---- Handler policy contract test (Phase 4H) --------------------------------
add_executable(robotos_core_handler_policy_contract_test
    test_robotos_core_handler_policy_contract.c
    ${CORE_SRCS}
    ${PLATFORM_HOST_STUB}
)

target_include_directories(robotos_core_handler_policy_contract_test PRIVATE
    "${CORE_DIR}"
    "${PLATFORM_DIR}"
)

enable_testing()
add_test(NAME robotos_core_contract               COMMAND robotos_core_contract_test)
add_test(NAME robotos_event_queue_contract        COMMAND robotos_event_queue_contract_test)
add_test(NAME robotos_event_dispatcher_contract   COMMAND robotos_event_dispatcher_contract_test)
add_test(NAME robotos_core_ingestion_contract     COMMAND robotos_core_ingestion_contract_test)
add_test(NAME robotos_core_tick_policy_contract   COMMAND robotos_core_tick_policy_contract_test)
add_test(NAME robotos_core_handler_policy_contract COMMAND robotos_core_handler_policy_contract_test)

# ---- Platform fault contract test (Phase 5C) --------------------------------
# Tests the host stub inspection API. No core sources needed.
# Includes tests/host/ for robotos_platform_fault_host_stub.h.
add_executable(robotos_platform_fault_contract_test
    test_robotos_platform_fault_contract.c
    robotos_platform_fault_host_stub.c
)

target_include_directories(robotos_platform_fault_contract_test PRIVATE
    "${PLATFORM_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}"
)

add_test(NAME robotos_platform_fault_contract COMMAND robotos_platform_fault_contract_test)

# ---- Scheduler admission policy contract test (Phase 4I) --------------------
# Tests admission gate in robotos_core_post_event().
# Needs all three core sources + platform log stub.
add_executable(robotos_scheduler_admission_contract_test
    test_robotos_scheduler_admission_contract.c
    ${CORE_SRCS}
    ${PLATFORM_HOST_STUB}
)

target_include_directories(robotos_scheduler_admission_contract_test PRIVATE
    "${CORE_DIR}"
    "${PLATFORM_DIR}"
)

add_test(NAME robotos_scheduler_admission_contract COMMAND robotos_scheduler_admission_contract_test)
