/*
 * robotos_platform_time_zephyr.c
 * RobotOS platform time backend — Zephyr RTOS.
 *
 * Phase 5B: implements robotos_platform_time.h using Zephyr kernel APIs.
 * This is the only Phase 5B file that includes <zephyr/kernel.h>.
 * devkit/runtime and other portable modules must not call k_msleep or
 * Zephyr time APIs directly; they use this interface instead.
 *
 * uptime:  k_uptime_get_32() — uint32_t ms, wraps ~49.7 days.
 * sleep:   k_msleep(duration_ms) — blocks calling thread for duration.
 *          duration_ms == 0 returns immediately without calling k_msleep,
 *          since k_msleep(0) behavior may vary by Zephyr version.
 */

#include <zephyr/kernel.h>

#include "robotos_platform_time.h"

uint32_t robotos_platform_uptime_ms(void)
{
    return k_uptime_get_32();
}

void robotos_platform_sleep_ms(uint32_t duration_ms)
{
    if (duration_ms == 0u) {
        return;
    }
    k_msleep((int32_t)duration_ms);
}
