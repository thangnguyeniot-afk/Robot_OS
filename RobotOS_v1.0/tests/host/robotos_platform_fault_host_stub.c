/*
 * robotos_platform_fault_host_stub.c
 * RobotOS platform fault — host stub backend.
 *
 * Implements robotos_platform_fault.h for host builds without Zephyr.
 * Tracks fault activity so contract tests can inspect stub state.
 *
 * State tracked:
 *   s_report_count      — total fault_report() calls (includes assert failures)
 *   s_assert_fail_count — assert() calls where condition was false
 *   s_last_severity     — severity of the most recent fault_report() call
 *
 * Call robotos_platform_fault_host_reset() between test cases to clear state.
 * Test-inspection functions are declared in robotos_platform_fault_host_stub.h.
 */

#include <stdint.h>

#include "robotos_platform_fault.h"
#include "robotos_platform_fault_host_stub.h"

static uint32_t                  s_report_count;
static uint32_t                  s_assert_fail_count;
static robotos_fault_severity_t  s_last_severity;

void robotos_platform_fault_report(robotos_fault_severity_t severity,
                                   const char *module,
                                   const char *message)
{
    (void)module;
    (void)message;
    s_last_severity = severity;
    s_report_count++;
}

bool robotos_platform_assert(bool condition,
                              const char *module,
                              const char *message)
{
    if (!condition) {
        s_assert_fail_count++;
        robotos_platform_fault_report(ROBOTOS_FAULT_ERROR, module, message);
    }
    return condition;
}

void robotos_platform_fault_host_reset(void)
{
    s_report_count      = 0u;
    s_assert_fail_count = 0u;
    s_last_severity     = ROBOTOS_FAULT_INFO;
}

uint32_t robotos_platform_fault_host_get_report_count(void)
{
    return s_report_count;
}

uint32_t robotos_platform_fault_host_get_assert_fail_count(void)
{
    return s_assert_fail_count;
}

robotos_fault_severity_t robotos_platform_fault_host_get_last_severity(void)
{
    return s_last_severity;
}
