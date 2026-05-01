/*
 * robotos_platform_log_host_stub.c
 * RobotOS platform logging — host (no-op) backend.
 *
 * Used by host contract tests in tests/host/ that compile robotos_core.c.
 * Implements the same robotos_platform_logf() API as the Zephyr backend,
 * but does nothing. No Zephyr headers are included.
 *
 * Purpose: allow core sources to link against the platform log interface
 * during host builds without any Zephyr dependency or log noise in tests.
 */

#include "robotos_platform_log.h"

void robotos_platform_logf(robotos_log_level_t level,
                           const char         *module,
                           const char         *fmt, ...)
{
    (void)level;
    (void)module;
    (void)fmt;
}
