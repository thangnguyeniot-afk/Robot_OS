# ============================================================================
# RobotOS v1.0 — Root CMakeLists.txt
# ============================================================================
# Hybrid build system:
#   - Zephyr mode:  west build (auto-detected via ZEPHYR_BASE)
#   - Host mode:    cmake -B build_host -DROBOTOS_HOST_BUILD=ON
# ============================================================================

cmake_minimum_required(VERSION 3.20)

# ---------- Detect build mode ------------------------------------------------
if(DEFINED ENV{ZEPHYR_BASE} OR DEFINED ZEPHYR_BASE)
    # ======== ZEPHYR FIRMWARE BUILD ========
    find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
    project(robotos VERSION 1.0.0 LANGUAGES C)

    # Adapter layer (Zephyr backend)
    target_sources(app PRIVATE
        src/adapter/zephyr/ro_status.c
        src/adapter/zephyr/ro_thread.c
        src/adapter/zephyr/ro_time.c
        src/adapter/zephyr/ro_timer.c
        src/adapter/zephyr/ro_queue.c
        src/adapter/zephyr/ro_pool.c
        src/adapter/zephyr/ro_mutex.c
        src/adapter/zephyr/ro_gpio.c
        src/adapter/zephyr/ro_pwm.c
        src/adapter/zephyr/ro_i2c.c
        src/adapter/zephyr/ro_spi.c
        src/adapter/zephyr/ro_log.c
        src/adapter/zephyr/ro_trace.c
        src/adapter/zephyr/ro_deadline.c
    )

    # Framework layer
    target_sources(app PRIVATE
        src/framework/stepper.c
        src/framework/servo.c
        src/framework/dcmotor.c
        src/framework/encoder.c
        src/framework/endstop.c
        src/framework/sensor.c
        src/framework/pid.c
        src/framework/filter.c
        src/framework/limiter.c
        src/framework/robot_sm.c
        src/framework/drivers/stepper_drv.c
        src/framework/drivers/endstop_drv.c
    )

    # Application layer — Core (OS-agnostic)
    target_sources(app PRIVATE
        src/app/core/gcode_parser.c
        src/app/core/motion_planner.c
        src/app/core/kinematics_cartesian.c
        src/app/core/app_sm.c
        src/app/core/config_profiles.c
    )

    # Application layer — Glue (RobotOS)
    target_sources(app PRIVATE
        src/app/glue/app_glue_robotos.c
        src/app/app_main.c
    )

    target_include_directories(app PRIVATE
        include
        src/app/core
    )

else()
    # ======== HOST BUILD (unit tests + development) ========
    project(robotos_host VERSION 1.0.0 LANGUAGES C)
    set(CMAKE_C_STANDARD 11)
    set(CMAKE_C_STANDARD_REQUIRED ON)

    option(ROBOTOS_HOST_BUILD "Build for host (PC) with stubs" ON)
    option(ROBOTOS_BUILD_TESTS "Build unit tests" ON)

    # Common include path
    include_directories(include src/app/core)

    # Host adapter stubs library
    add_library(ro_adapter_host STATIC
        src/adapter/host/ro_status.c
        src/adapter/host/ro_thread.c
        src/adapter/host/ro_time.c
        src/adapter/host/ro_timer.c
        src/adapter/host/ro_queue.c
        src/adapter/host/ro_pool.c
        src/adapter/host/ro_mutex.c
        src/adapter/host/ro_gpio.c
        src/adapter/host/ro_pwm.c
        src/adapter/host/ro_i2c.c
        src/adapter/host/ro_spi.c
        src/adapter/host/ro_log.c
        src/adapter/host/ro_trace.c
        src/adapter/host/ro_deadline.c
    )
    target_compile_definitions(ro_adapter_host PUBLIC ROBOTOS_HOST_BUILD=1)

    # Framework library
    add_library(ro_framework STATIC
        src/framework/stepper.c
        src/framework/servo.c
        src/framework/dcmotor.c
        src/framework/encoder.c
        src/framework/endstop.c
        src/framework/sensor.c
        src/framework/pid.c
        src/framework/filter.c
        src/framework/limiter.c
        src/framework/robot_sm.c
    )
    target_link_libraries(ro_framework PUBLIC ro_adapter_host)

    # App Core library (OS-agnostic — no adapter dependency!)
    add_library(app_core STATIC
        src/app/core/gcode_parser.c
        src/app/core/motion_planner.c
        src/app/core/kinematics_cartesian.c
        src/app/core/app_sm.c
        src/app/core/config_profiles.c
    )
    # App Core must NOT link to any OS library — pure C only
    target_include_directories(app_core PUBLIC src/app/core)

    # ---- Unit Tests ----
    if(ROBOTOS_BUILD_TESTS)
        enable_testing()
        add_subdirectory(tests/host)
    endif()
endif()
