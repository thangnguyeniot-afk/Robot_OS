/*
 * robotos_platform_time_host_stub.c
 * RobotOS platform time — host stub backend.
 *
 * Implements robotos_platform_time.h for host builds without Zephyr.
 * No real sleep occurs. Uptime is simulated: each robotos_platform_sleep_ms()
 * call increments the internal fake uptime by the requested duration.
 *
 * Current host test targets do not call platform time APIs (devkit_runtime.c
 * is not compiled for host tests). This stub is provided for completeness
 * and for future test targets that may exercise time-dependent logic.
 *
 * Limitation: not wired into any existing host CMake target in Phase 5B.
 * Add to target_sources when a test target needs it.
 */

#include "robotos_platform_time.h"

static uint32_t s_fake_uptime_ms;

uint32_t robotos_platform_uptime_ms(void)
{
    return s_fake_uptime_ms;
}

void robotos_platform_sleep_ms(uint32_t duration_ms)
{
    s_fake_uptime_ms += duration_ms;
}
