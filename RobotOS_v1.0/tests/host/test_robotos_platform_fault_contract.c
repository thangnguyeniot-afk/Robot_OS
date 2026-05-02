/*
 * test_robotos_platform_fault_contract.c
 * Phase 5C: contract test for the robotos_platform_fault host stub.
 *
 * Verifies the stub correctly tracks fault activity so that future test
 * targets exercising fault-reporting modules can rely on the inspection API.
 *
 * 12 test cases covering: initial state, all severity levels, last_severity
 * tracking, assert(true) side-effect-free behavior, assert(false) return
 * value, assert(false) counter increments, assert(false) severity, and reset.
 */

#include <stdio.h>
#include <stdbool.h>

#include "robotos_platform_fault.h"
#include "robotos_platform_fault_host_stub.h"

static int s_pass;
static int s_fail;

static void check(const char *name, bool result)
{
    if (result) {
        printf("PASS: %s\n", name);
        s_pass++;
    } else {
        printf("FAIL: %s\n", name);
        s_fail++;
    }
}

int main(void)
{
    /* TC01: report_count starts at 0 after reset */
    robotos_platform_fault_host_reset();
    check("TC01 report_count starts at 0 after reset",
          robotos_platform_fault_host_get_report_count() == 0u);

    /* TC02: fault_report INFO increments report_count */
    robotos_platform_fault_host_reset();
    robotos_platform_fault_report(ROBOTOS_FAULT_INFO, "test", "info msg");
    check("TC02 fault_report INFO increments report_count",
          robotos_platform_fault_host_get_report_count() == 1u);

    /* TC03: fault_report WARNING increments report_count */
    robotos_platform_fault_host_reset();
    robotos_platform_fault_report(ROBOTOS_FAULT_WARNING, "test", "warn msg");
    check("TC03 fault_report WARNING increments report_count",
          robotos_platform_fault_host_get_report_count() == 1u);

    /* TC04: fault_report ERROR increments report_count */
    robotos_platform_fault_host_reset();
    robotos_platform_fault_report(ROBOTOS_FAULT_ERROR, "test", "error msg");
    check("TC04 fault_report ERROR increments report_count",
          robotos_platform_fault_host_get_report_count() == 1u);

    /* TC05: fault_report FATAL increments report_count */
    robotos_platform_fault_host_reset();
    robotos_platform_fault_report(ROBOTOS_FAULT_FATAL, "test", "fatal msg");
    check("TC05 fault_report FATAL increments report_count",
          robotos_platform_fault_host_get_report_count() == 1u);

    /* TC06: last_severity reflects most recent fault_report call */
    robotos_platform_fault_host_reset();
    robotos_platform_fault_report(ROBOTOS_FAULT_INFO,  "test", "first");
    robotos_platform_fault_report(ROBOTOS_FAULT_FATAL, "test", "second");
    check("TC06 last_severity reflects most recent report",
          robotos_platform_fault_host_get_last_severity() == ROBOTOS_FAULT_FATAL);

    /* TC07: assert(true) returns true */
    robotos_platform_fault_host_reset();
    check("TC07 assert(true) returns true",
          robotos_platform_assert(true, "test", "no trigger") == true);

    /* TC08: assert(true) does not increment report_count */
    robotos_platform_fault_host_reset();
    robotos_platform_assert(true, "test", "no trigger");
    check("TC08 assert(true) does not increment report_count",
          robotos_platform_fault_host_get_report_count() == 0u);

    /* TC09: assert(false) returns false */
    robotos_platform_fault_host_reset();
    check("TC09 assert(false) returns false",
          robotos_platform_assert(false, "test", "expected fail") == false);

    /* TC10: assert(false) increments assert_fail_count */
    robotos_platform_fault_host_reset();
    robotos_platform_assert(false, "test", "expected fail");
    check("TC10 assert(false) increments assert_fail_count",
          robotos_platform_fault_host_get_assert_fail_count() == 1u);

    /* TC11: assert(false) reports ERROR severity */
    robotos_platform_fault_host_reset();
    robotos_platform_assert(false, "test", "expected fail");
    check("TC11 assert(false) reports ERROR severity",
          robotos_platform_fault_host_get_last_severity() == ROBOTOS_FAULT_ERROR);

    /* TC12: assert(false) increments report_count */
    robotos_platform_fault_host_reset();
    robotos_platform_assert(false, "test", "expected fail");
    check("TC12 assert(false) increments report_count",
          robotos_platform_fault_host_get_report_count() == 1u);

    printf("\n%d passed, %d failed\n", s_pass, s_fail);
    return (s_fail == 0) ? 0 : 1;
}
