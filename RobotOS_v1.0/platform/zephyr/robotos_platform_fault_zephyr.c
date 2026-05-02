/*
 * robotos_platform_fault_zephyr.c
 * RobotOS platform fault backend — Zephyr RTOS.
 *
 * Phase 5C: implements robotos_platform_fault.h for devkit/Zephyr builds.
 *
 * Fault reports are routed through robotos_platform_logf() at the
 * appropriate log level. No Zephyr fault handler is overridden.
 *
 * ROBOTOS_PLATFORM_FAULT_PANIC_ON_FATAL (default OFF):
 *   When defined at build time, fault_report() calls k_panic() after logging
 *   if severity == ROBOTOS_FAULT_FATAL. This switch does NOT affect
 *   robotos_platform_assert() — assert never calls k_panic().
 *
 * Severity → log level mapping:
 *   INFO    → ROBOTOS_LOG_LEVEL_INFO
 *   WARNING → ROBOTOS_LOG_LEVEL_WARN
 *   ERROR   → ROBOTOS_LOG_LEVEL_ERROR
 *   FATAL   → ROBOTOS_LOG_LEVEL_ERROR  (plus optional k_panic)
 */

#include "robotos_platform_fault.h"
#include "robotos_platform_log.h"

#ifdef ROBOTOS_PLATFORM_FAULT_PANIC_ON_FATAL
#include <zephyr/kernel.h>
#endif

static robotos_log_level_t severity_to_log_level(robotos_fault_severity_t severity)
{
    switch (severity) {
    case ROBOTOS_FAULT_INFO:    return ROBOTOS_LOG_LEVEL_INFO;
    case ROBOTOS_FAULT_WARNING: return ROBOTOS_LOG_LEVEL_WARN;
    case ROBOTOS_FAULT_ERROR:   return ROBOTOS_LOG_LEVEL_ERROR;
    case ROBOTOS_FAULT_FATAL:   return ROBOTOS_LOG_LEVEL_ERROR;
    default:                    return ROBOTOS_LOG_LEVEL_ERROR;
    }
}

void robotos_platform_fault_report(robotos_fault_severity_t severity,
                                   const char *module,
                                   const char *message)
{
    robotos_log_level_t level = severity_to_log_level(severity);

    robotos_platform_logf(level, module, "FAULT[%d]: %s", (int)severity, message);

#ifdef ROBOTOS_PLATFORM_FAULT_PANIC_ON_FATAL
    if (severity == ROBOTOS_FAULT_FATAL) {
        k_panic();
    }
#endif
}

bool robotos_platform_assert(bool condition,
                              const char *module,
                              const char *message)
{
    if (!condition) {
        robotos_platform_fault_report(ROBOTOS_FAULT_ERROR, module, message);
    }
    return condition;
}
