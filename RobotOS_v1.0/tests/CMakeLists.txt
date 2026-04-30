# Tests CMakeLists.txt — Host-only unit test build
#
# Build from project root:
#   mkdir build && cd build
#   cmake -DHOST_BUILD=ON .. && cmake --build .
#   ctest --output-on-failure

cmake_minimum_required(VERSION 3.20)

set(COMMON_INCLUDES
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/robotos
    ${CMAKE_SOURCE_DIR}/include/app
)

set(HOST_ADAPTER_SRCS
    ${CMAKE_SOURCE_DIR}/src/adapter/host/ro_status.c
    ${CMAKE_SOURCE_DIR}/src/adapter/host/ro_thread.c
    ${CMAKE_SOURCE_DIR}/src/adapter/host/ro_time.c
    ${CMAKE_SOURCE_DIR}/src/adapter/host/ro_timer.c
    ${CMAKE_SOURCE_DIR}/src/adapter/host/ro_queue.c
    ${CMAKE_SOURCE_DIR}/src/adapter/host/ro_pool.c
    ${CMAKE_SOURCE_DIR}/src/adapter/host/ro_mutex.c
    ${CMAKE_SOURCE_DIR}/src/adapter/host/ro_gpio.c
    ${CMAKE_SOURCE_DIR}/src/adapter/host/ro_pwm.c
    ${CMAKE_SOURCE_DIR}/src/adapter/host/ro_i2c.c
    ${CMAKE_SOURCE_DIR}/src/adapter/host/ro_spi.c
    ${CMAKE_SOURCE_DIR}/src/adapter/host/ro_log.c
    ${CMAKE_SOURCE_DIR}/src/adapter/host/ro_trace.c
    ${CMAKE_SOURCE_DIR}/src/adapter/host/ro_deadline.c
    ${CMAKE_SOURCE_DIR}/src/adapter/host/ro_assert.c
)

# ── test_gcode_parser ──────────────────────────────────────────
add_executable(test_gcode_parser
    test_gcode_parser.c
    ${CMAKE_SOURCE_DIR}/src/app/gcode_parser.c
)
target_include_directories(test_gcode_parser PRIVATE ${COMMON_INCLUDES})
add_test(NAME test_gcode_parser COMMAND test_gcode_parser)

# ── test_kinematics_cartesian ──────────────────────────────────
add_executable(test_kinematics_cartesian
    test_kinematics_cartesian.c
    ${CMAKE_SOURCE_DIR}/src/app/kinematics_cartesian.c
)
target_include_directories(test_kinematics_cartesian PRIVATE ${COMMON_INCLUDES})
target_link_libraries(test_kinematics_cartesian m)
add_test(NAME test_kinematics_cartesian COMMAND test_kinematics_cartesian)

# ── test_app_sm ────────────────────────────────────────────────
add_executable(test_app_sm
    test_app_sm.c
    ${CMAKE_SOURCE_DIR}/src/app/app_sm.c
    ${HOST_ADAPTER_SRCS}
)
target_include_directories(test_app_sm PRIVATE ${COMMON_INCLUDES})
add_test(NAME test_app_sm COMMAND test_app_sm)

# ── test_motion_planner ────────────────────────────────────────
add_executable(test_motion_planner
    test_motion_planner.c
    ${CMAKE_SOURCE_DIR}/src/app/motion_planner.c
    ${CMAKE_SOURCE_DIR}/src/app/gcode_parser.c
    ${CMAKE_SOURCE_DIR}/src/app/kinematics_cartesian.c
    ${HOST_ADAPTER_SRCS}
)
target_include_directories(test_motion_planner PRIVATE ${COMMON_INCLUDES})
target_link_libraries(test_motion_planner m)
add_test(NAME test_motion_planner COMMAND test_motion_planner)
