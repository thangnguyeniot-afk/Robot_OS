/*
 * robotos_platform_log_zephyr.c
 * RobotOS platform logging backend — Zephyr RTOS.
 *
 * Phase 5A: routes robotos_platform_logf() calls to the Zephyr logging
 * subsystem. This is the ONLY file in Phase 5A that includes
 * <zephyr/logging/log.h>. Core and other portable modules must not.
 *
 * LOG_MODULE_REGISTER is placed here, not in core, ensuring that Zephyr
 * logging module ownership lives at the platform boundary.
 *
 * Log level mapping:
 *   ROBOTOS_LOG_LEVEL_DEBUG -> LOG_DBG
 *   ROBOTOS_LOG_LEVEL_INFO  -> LOG_INF
 *   ROBOTOS_LOG_LEVEL_WARN  -> LOG_WRN
 *   ROBOTOS_LOG_LEVEL_ERROR -> LOG_ERR
 *
 * The module label is "robotos_platform". Module-name strings passed by
 * callers (e.g. "robotos_core") are embedded in the formatted message so
 * the origin is visible in RTT/UART output.
 *
 * Buffer size: 128 bytes. Sufficient for all current core log messages.
 * If a formatted string exceeds this, vsnprintf truncates silently —
 * this is acceptable for diagnostic output.
 */

#include <stdio.h>
#include <zephyr/logging/log.h>

#include "robotos_platform_log.h"

LOG_MODULE_REGISTER(robotos_platform, LOG_LEVEL_INF);

#define PLATFORM_LOG_BUF_SIZE 128

void robotos_platform_logf(robotos_log_level_t level,
                           const char         *module,
                           const char         *fmt, ...)
{
    char buf[PLATFORM_LOG_BUF_SIZE];
    va_list args;

    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    switch (level) {
    case ROBOTOS_LOG_LEVEL_DEBUG:
        LOG_DBG("[%s] %s", module, buf);
        break;
    case ROBOTOS_LOG_LEVEL_WARN:
        LOG_WRN("[%s] %s", module, buf);
        break;
    case ROBOTOS_LOG_LEVEL_ERROR:
        LOG_ERR("[%s] %s", module, buf);
        break;
    case ROBOTOS_LOG_LEVEL_INFO:
    default:
        LOG_INF("[%s] %s", module, buf);
        break;
    }
}
