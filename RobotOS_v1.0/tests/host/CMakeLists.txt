# tests/host/CMakeLists.txt
# Phase 5E: isolated host build — core, queue, dispatcher, ingestion, tick policy, handler policy,
#           platform fault contract, platform critical section contract,
#           core critical boundary contract,
#           scheduler admission policy, scheduler backpressure policy,
#           scheduler producer throttle policy.
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
# Build and run:
#   cmake -S RobotOS_v1.0/tests/host -B build-host-core
#   cmake --build build-host-core
#   ctest --test-dir build-host-core --output-on-failure
#
# Save test log to version-controlled logs/:
#   cmake --build build-host-core --target save_test_log
#   (writes tests/host/logs/host_<date>.log — commit this file)
#
# Note: Windows MinGW64 may be broken; use WSL Ubuntu or Linux host.

cmake_minimum_required(VERSION 3.20)
project(robotos_core_host_tests C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Locate core/ relative to this CMakeLists (two levels up from tests/host/)
set(CORE_DIR    "${CMAKE_CURRENT_SOURCE_DIR}/../../core")
set(PLATFORM_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../platform")

# Host platform stubs — compiled into any target that includes robotos_core.c.
# Phase 5E: robotos_core.c now calls robotos_platform_critical_enter/exit,
# so the critical stub must be linked alongside the log stub for all core tests.
set(PLATFORM_HOST_STUB
    "${CMAKE_CURRENT_SOURCE_DIR}/robotos_platform_log_host_stub.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/robotos_platform_critical_host_stub.c"
)

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
    "${CMAKE_CURRENT_SOURCE_DIR}"
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
    "${CMAKE_CURRENT_SOURCE_DIR}"
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
    "${CMAKE_CURRENT_SOURCE_DIR}"
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
    "${CMAKE_CURRENT_SOURCE_DIR}"
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

# ---- Platform critical section contract test (Phase 5D) ---------------------
# Tests enter/exit counter semantics via host stub. No Zephyr. No core sources.
add_executable(robotos_platform_critical_contract_test
    test_robotos_platform_critical_contract.c
    robotos_platform_critical_host_stub.c
)

target_include_directories(robotos_platform_critical_contract_test PRIVATE
    "${PLATFORM_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}"
)

add_test(NAME robotos_platform_critical_contract COMMAND robotos_platform_critical_contract_test)

# ---- Core critical boundary contract test (Phase 5E) -----------------------
# Tests that core state transitions are balanced enter/exit and that
# handler callbacks run with depth == 0 (no critical section held).
# Needs all three core sources + platform log stub + platform critical stub.
add_executable(robotos_core_critical_boundary_contract_test
    test_robotos_core_critical_boundary_contract.c
    ${CORE_SRCS}
    ${PLATFORM_HOST_STUB}
)

target_include_directories(robotos_core_critical_boundary_contract_test PRIVATE
    "${CORE_DIR}"
    "${PLATFORM_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}"
)

add_test(NAME robotos_core_critical_boundary_contract COMMAND robotos_core_critical_boundary_contract_test)

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
    "${CMAKE_CURRENT_SOURCE_DIR}"
)

add_test(NAME robotos_scheduler_admission_contract COMMAND robotos_scheduler_admission_contract_test)

# ---- Scheduler backpressure policy contract test (Phase 4J) -----------------
# Tests budget/backpressure observability in robotos_core.
# Needs all three core sources + platform log stub.
add_executable(robotos_scheduler_backpressure_contract_test
    test_robotos_scheduler_backpressure_contract.c
    ${CORE_SRCS}
    ${PLATFORM_HOST_STUB}
)

target_include_directories(robotos_scheduler_backpressure_contract_test PRIVATE
    "${CORE_DIR}"
    "${PLATFORM_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}"
)

add_test(NAME robotos_scheduler_backpressure_contract COMMAND robotos_scheduler_backpressure_contract_test)

# ---- Scheduler producer throttle policy contract test (Phase 4K) ------------
# Tests try_post_event throttle behavior in robotos_core.
# Needs all three core sources + platform log stub.
add_executable(robotos_scheduler_throttle_contract_test
    test_robotos_scheduler_throttle_contract.c
    ${CORE_SRCS}
    ${PLATFORM_HOST_STUB}
)

target_include_directories(robotos_scheduler_throttle_contract_test PRIVATE
    "${CORE_DIR}"
    "${PLATFORM_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}"
)

add_test(NAME robotos_scheduler_throttle_contract COMMAND robotos_scheduler_throttle_contract_test)

# ---- Core dispatch critical split contract test (Phase 5F) ------------------
# Tests that pop/handler-lookup/handler-invoke are split correctly and that
# no critical section is held during handler callback.
# Needs all three core sources + platform log stub + platform critical stub.
add_executable(robotos_core_dispatch_critical_contract_test
    test_robotos_core_dispatch_critical_contract.c
    ${CORE_SRCS}
    ${PLATFORM_HOST_STUB}
)

target_include_directories(robotos_core_dispatch_critical_contract_test PRIVATE
    "${CORE_DIR}"
    "${PLATFORM_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}"
)

add_test(NAME robotos_core_dispatch_critical_contract
         COMMAND robotos_core_dispatch_critical_contract_test)

# ---- Scheduler advisory retry policy contract test (Phase 4L) ---------------
# Tests robotos_core_retry_decision_for_status() and
# robotos_core_status_is_retryable(). Pure stateless mapping; no init required
# for most tests (TC12 explicitly exercises the pre-init path).
add_executable(robotos_scheduler_retry_policy_contract_test
    test_robotos_scheduler_retry_policy_contract.c
    ${CORE_SRCS}
    ${PLATFORM_HOST_STUB}
)

target_include_directories(robotos_scheduler_retry_policy_contract_test PRIVATE
    "${CORE_DIR}"
    "${PLATFORM_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}"
)

add_test(NAME robotos_scheduler_retry_policy_contract
         COMMAND robotos_scheduler_retry_policy_contract_test)

# ---- Log persistence target --------------------------------------------------
# Saves ctest output to a timestamped file in the version-controlled logs/
# directory alongside this CMakeLists.txt.
#
# Usage (from repo root, WSL/Linux):
#   cmake --build build-host-core --target save_test_log
#
# The generated log file should be committed to version control.
# Naming convention: host_YYYY-MM-DD.log
# If a file for today already exists it is overwritten (re-run on same day).
set(LOG_DIR "${CMAKE_CURRENT_SOURCE_DIR}/logs")
add_custom_target(save_test_log
    COMMAND ${CMAKE_COMMAND} -E make_directory "${LOG_DIR}"
    COMMAND ${CMAKE_COMMAND} -E echo "Running tests and saving log to ${LOG_DIR}/"
    COMMAND ${CMAKE_COMMAND}
        -DLOG_DIR=${LOG_DIR}
        -DCTEST_BINARY_DIR=${CMAKE_BINARY_DIR}
        -P "${CMAKE_CURRENT_SOURCE_DIR}/save_test_log.cmake"
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
    COMMENT "Saving host test log to tests/host/logs/"
    VERBATIM
)
