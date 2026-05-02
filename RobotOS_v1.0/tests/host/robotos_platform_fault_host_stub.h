/*
 * robotos_platform_fault_host_stub.h
 * Test-inspection API for the platform fault host stub.
 *
 * Not part of the production platform interface.
 * Include only in host test code alongside robotos_platform_fault_host_stub.c.
 *
 * Provides reset and getter functions to inspect stub state between test cases.
 */

#ifndef ROBOTOS_PLATFORM_FAULT_HOST_STUB_H
#define ROBOTOS_PLATFORM_FAULT_HOST_STUB_H

#include <stdint.h>
#include "robotos_platform_fault.h"

/* Reset all stub counters and state. Call between test cases. */
void robotos_platform_fault_host_reset(void);

/* Return total fault_report() call count since last reset. */
uint32_t robotos_platform_fault_host_get_report_count(void);

/* Return count of assert() calls where condition was false since last reset. */
uint32_t robotos_platform_fault_host_get_assert_fail_count(void);

/* Return severity of the most recent fault_report() call since last reset. */
robotos_fault_severity_t robotos_platform_fault_host_get_last_severity(void);

#endif /* ROBOTOS_PLATFORM_FAULT_HOST_STUB_H */
