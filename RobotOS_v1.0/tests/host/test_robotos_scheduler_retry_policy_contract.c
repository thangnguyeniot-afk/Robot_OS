/*
 * test_robotos_scheduler_retry_policy_contract.c
 *
 * Phase 4L — Scheduler Advisory Retry Decision Policy Contract
 *
 * Tests for robotos_core_retry_decision_for_status() and
 * robotos_core_status_is_retryable(). Both are pure stateless mappings:
 * no lock, no platform call, no queue access, no mutable state.
 *
 * All tests are independent. No init required.
 */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "robotos_core.h"

/* --------------------------------------------------------------------------
 * Minimal test harness
 * -------------------------------------------------------------------------- */

static int s_pass = 0;
static int s_fail = 0;

#define CHECK(cond, name) \
	do { \
		if (cond) { \
			printf("  PASS: %s\n", (name)); \
			s_pass++; \
		} else { \
			printf("  FAIL: %s  (line %d)\n", (name), __LINE__); \
			s_fail++; \
		} \
	} while (0)

/* --------------------------------------------------------------------------
 * TC01 — NULL out pointer -> ERR_NULL
 * -------------------------------------------------------------------------- */
static void tc01_null_out(void)
{
	robotos_core_status_t ret = robotos_core_retry_decision_for_status(
		ROBOTOS_CORE_OK, NULL);
	CHECK(ret == ROBOTOS_CORE_ERR_NULL, "TC01: NULL out -> ERR_NULL");
}

/* --------------------------------------------------------------------------
 * TC02 — OK -> RETRY_NONE, wait=0, drop=false, report=false
 * -------------------------------------------------------------------------- */
static void tc02_ok(void)
{
	robotos_core_retry_decision_t d;
	memset(&d, 0xFF, sizeof(d));
	robotos_core_status_t ret = robotos_core_retry_decision_for_status(
		ROBOTOS_CORE_OK, &d);
	CHECK(ret == ROBOTOS_CORE_OK,               "TC02: OK -> CORE_OK");
	CHECK(d.action == ROBOTOS_CORE_RETRY_NONE,  "TC02: OK -> RETRY_NONE");
	CHECK(d.suggested_wait_ticks == 0u,         "TC02: OK -> wait=0");
	CHECK(d.producer_should_drop   == false,    "TC02: OK -> drop=false");
	CHECK(d.producer_should_report == false,    "TC02: OK -> report=false");
}

/* --------------------------------------------------------------------------
 * TC03 — ERR_FULL -> RETRY_AFTER_TICK, wait=1, drop=false, report=false
 * -------------------------------------------------------------------------- */
static void tc03_full(void)
{
	robotos_core_retry_decision_t d;
	memset(&d, 0xFF, sizeof(d));
	robotos_core_status_t ret = robotos_core_retry_decision_for_status(
		ROBOTOS_CORE_ERR_FULL, &d);
	CHECK(ret == ROBOTOS_CORE_OK,                      "TC03: FULL -> CORE_OK");
	CHECK(d.action == ROBOTOS_CORE_RETRY_AFTER_TICK,   "TC03: FULL -> RETRY_AFTER_TICK");
	CHECK(d.suggested_wait_ticks == 1u,                "TC03: FULL -> wait=1");
	CHECK(d.producer_should_drop   == false,           "TC03: FULL -> drop=false");
	CHECK(d.producer_should_report == false,           "TC03: FULL -> report=false");
}

/* --------------------------------------------------------------------------
 * TC04 — ERR_THROTTLED -> RETRY_AFTER_TICK, wait=1, drop=false, report=false
 * -------------------------------------------------------------------------- */
static void tc04_throttled(void)
{
	robotos_core_retry_decision_t d;
	memset(&d, 0xFF, sizeof(d));
	robotos_core_status_t ret = robotos_core_retry_decision_for_status(
		ROBOTOS_CORE_ERR_THROTTLED, &d);
	CHECK(ret == ROBOTOS_CORE_OK,                      "TC04: THROTTLED -> CORE_OK");
	CHECK(d.action == ROBOTOS_CORE_RETRY_AFTER_TICK,   "TC04: THROTTLED -> RETRY_AFTER_TICK");
	CHECK(d.suggested_wait_ticks == 1u,                "TC04: THROTTLED -> wait=1");
	CHECK(d.producer_should_drop   == false,           "TC04: THROTTLED -> drop=false");
	CHECK(d.producer_should_report == false,           "TC04: THROTTLED -> report=false");
}

/* --------------------------------------------------------------------------
 * TC05 — ERR_INVALID_STATE -> RETRY_SOON, wait=0, drop=false, report=false
 * -------------------------------------------------------------------------- */
static void tc05_invalid_state(void)
{
	robotos_core_retry_decision_t d;
	memset(&d, 0xFF, sizeof(d));
	robotos_core_status_t ret = robotos_core_retry_decision_for_status(
		ROBOTOS_CORE_ERR_INVALID_STATE, &d);
	CHECK(ret == ROBOTOS_CORE_OK,                   "TC05: INVALID_STATE -> CORE_OK");
	CHECK(d.action == ROBOTOS_CORE_RETRY_SOON,      "TC05: INVALID_STATE -> RETRY_SOON");
	CHECK(d.suggested_wait_ticks == 0u,             "TC05: INVALID_STATE -> wait=0");
	CHECK(d.producer_should_drop   == false,        "TC05: INVALID_STATE -> drop=false");
	CHECK(d.producer_should_report == false,        "TC05: INVALID_STATE -> report=false");
}

/* --------------------------------------------------------------------------
 * TC06 — ERR_INVALID_ARG -> RETRY_NEVER, drop=true, report=true
 * -------------------------------------------------------------------------- */
static void tc06_invalid_arg(void)
{
	robotos_core_retry_decision_t d;
	memset(&d, 0xFF, sizeof(d));
	robotos_core_status_t ret = robotos_core_retry_decision_for_status(
		ROBOTOS_CORE_ERR_INVALID_ARG, &d);
	CHECK(ret == ROBOTOS_CORE_OK,                   "TC06: INVALID_ARG -> CORE_OK");
	CHECK(d.action == ROBOTOS_CORE_RETRY_NEVER,     "TC06: INVALID_ARG -> RETRY_NEVER");
	CHECK(d.suggested_wait_ticks == 0u,             "TC06: INVALID_ARG -> wait=0");
	CHECK(d.producer_should_drop   == true,         "TC06: INVALID_ARG -> drop=true");
	CHECK(d.producer_should_report == true,         "TC06: INVALID_ARG -> report=true");
}

/* --------------------------------------------------------------------------
 * TC07 — ERR_NULL -> RETRY_NEVER, drop=true, report=true
 * -------------------------------------------------------------------------- */
static void tc07_err_null(void)
{
	robotos_core_retry_decision_t d;
	memset(&d, 0xFF, sizeof(d));
	robotos_core_status_t ret = robotos_core_retry_decision_for_status(
		ROBOTOS_CORE_ERR_NULL, &d);
	CHECK(ret == ROBOTOS_CORE_OK,                   "TC07: ERR_NULL -> CORE_OK");
	CHECK(d.action == ROBOTOS_CORE_RETRY_NEVER,     "TC07: ERR_NULL -> RETRY_NEVER");
	CHECK(d.suggested_wait_ticks == 0u,             "TC07: ERR_NULL -> wait=0");
	CHECK(d.producer_should_drop   == true,         "TC07: ERR_NULL -> drop=true");
	CHECK(d.producer_should_report == true,         "TC07: ERR_NULL -> report=true");
}

/* --------------------------------------------------------------------------
 * TC08 — ERR_EMPTY -> RETRY_NONE, wait=0, drop=false, report=false
 * -------------------------------------------------------------------------- */
static void tc08_empty(void)
{
	robotos_core_retry_decision_t d;
	memset(&d, 0xFF, sizeof(d));
	robotos_core_status_t ret = robotos_core_retry_decision_for_status(
		ROBOTOS_CORE_ERR_EMPTY, &d);
	CHECK(ret == ROBOTOS_CORE_OK,               "TC08: EMPTY -> CORE_OK");
	CHECK(d.action == ROBOTOS_CORE_RETRY_NONE,  "TC08: EMPTY -> RETRY_NONE");
	CHECK(d.suggested_wait_ticks == 0u,         "TC08: EMPTY -> wait=0");
	CHECK(d.producer_should_drop   == false,    "TC08: EMPTY -> drop=false");
	CHECK(d.producer_should_report == false,    "TC08: EMPTY -> report=false");
}

/* --------------------------------------------------------------------------
 * TC09 — unknown status -> ERR_INVALID_ARG, *out zeroed
 * -------------------------------------------------------------------------- */
static void tc09_unknown_status(void)
{
	robotos_core_retry_decision_t d;
	memset(&d, 0xFF, sizeof(d));
	robotos_core_status_t ret = robotos_core_retry_decision_for_status(
		(robotos_core_status_t)(-99), &d);
	CHECK(ret == ROBOTOS_CORE_ERR_INVALID_ARG,    "TC09: unknown -> ERR_INVALID_ARG");
	CHECK(d.action == ROBOTOS_CORE_RETRY_NONE,    "TC09: unknown -> out.action zeroed");
	CHECK(d.suggested_wait_ticks == 0u,           "TC09: unknown -> out.wait zeroed");
	CHECK(d.producer_should_drop   == false,      "TC09: unknown -> out.drop zeroed");
	CHECK(d.producer_should_report == false,      "TC09: unknown -> out.report zeroed");
}

/* --------------------------------------------------------------------------
 * TC10 — is_retryable: true for FULL, THROTTLED, INVALID_STATE
 * -------------------------------------------------------------------------- */
static void tc10_is_retryable_true(void)
{
	CHECK(robotos_core_status_is_retryable(ROBOTOS_CORE_ERR_FULL),
	      "TC10: FULL is retryable");
	CHECK(robotos_core_status_is_retryable(ROBOTOS_CORE_ERR_THROTTLED),
	      "TC10: THROTTLED is retryable");
	CHECK(robotos_core_status_is_retryable(ROBOTOS_CORE_ERR_INVALID_STATE),
	      "TC10: INVALID_STATE is retryable");
}

/* --------------------------------------------------------------------------
 * TC11 — is_retryable: false for OK, INVALID_ARG, ERR_NULL, EMPTY, unknown
 * -------------------------------------------------------------------------- */
static void tc11_is_retryable_false(void)
{
	CHECK(!robotos_core_status_is_retryable(ROBOTOS_CORE_OK),
	      "TC11: OK is not retryable");
	CHECK(!robotos_core_status_is_retryable(ROBOTOS_CORE_ERR_INVALID_ARG),
	      "TC11: INVALID_ARG is not retryable");
	CHECK(!robotos_core_status_is_retryable(ROBOTOS_CORE_ERR_NULL),
	      "TC11: ERR_NULL is not retryable");
	CHECK(!robotos_core_status_is_retryable(ROBOTOS_CORE_ERR_EMPTY),
	      "TC11: EMPTY is not retryable");
	CHECK(!robotos_core_status_is_retryable((robotos_core_status_t)(-99)),
	      "TC11: unknown is not retryable");
}

/* --------------------------------------------------------------------------
 * TC12 — functions work before core init (no init called in this test)
 * -------------------------------------------------------------------------- */
static void tc12_works_before_init(void)
{
	robotos_core_retry_decision_t d;
	memset(&d, 0, sizeof(d));
	robotos_core_status_t ret = robotos_core_retry_decision_for_status(
		ROBOTOS_CORE_ERR_FULL, &d);
	CHECK(ret == ROBOTOS_CORE_OK,                    "TC12: works before init");
	CHECK(d.action == ROBOTOS_CORE_RETRY_AFTER_TICK, "TC12: correct action before init");
	CHECK(robotos_core_status_is_retryable(ROBOTOS_CORE_ERR_FULL),
	      "TC12: is_retryable works before init");
}

/* --------------------------------------------------------------------------
 * TC13 — snapshot counters not mutated by decision function
 * -------------------------------------------------------------------------- */
static void tc13_no_snapshot_mutation(void)
{
	/* init core so snapshot is accessible */
	robotos_core_init();

	robotos_core_snapshot_t snap_before;
	robotos_core_snapshot(&snap_before);

	robotos_core_retry_decision_t d;
	robotos_core_retry_decision_for_status(ROBOTOS_CORE_ERR_FULL, &d);
	robotos_core_retry_decision_for_status(ROBOTOS_CORE_ERR_THROTTLED, &d);
	robotos_core_retry_decision_for_status(ROBOTOS_CORE_ERR_INVALID_ARG, &d);
	(void)robotos_core_status_is_retryable(ROBOTOS_CORE_ERR_FULL);

	robotos_core_snapshot_t snap_after;
	robotos_core_snapshot(&snap_after);

	CHECK(snap_before.pending_event_count   == snap_after.pending_event_count,
	      "TC13: pending unchanged");
	CHECK(snap_before.admission_accepted_count == snap_after.admission_accepted_count,
	      "TC13: accepted unchanged");
	CHECK(snap_before.admission_rejected_count == snap_after.admission_rejected_count,
	      "TC13: rejected unchanged");
	CHECK(snap_before.dropped_event_count   == snap_after.dropped_event_count,
	      "TC13: dropped unchanged");
	CHECK(snap_before.producer_throttled_count == snap_after.producer_throttled_count,
	      "TC13: throttled_count unchanged");
}

/* --------------------------------------------------------------------------
 * main
 * -------------------------------------------------------------------------- */
int main(void)
{
	printf("=== robotos_scheduler_retry_policy_contract ===\n");

	tc01_null_out();
	tc02_ok();
	tc03_full();
	tc04_throttled();
	tc05_invalid_state();
	tc06_invalid_arg();
	tc07_err_null();
	tc08_empty();
	tc09_unknown_status();
	tc10_is_retryable_true();
	tc11_is_retryable_false();
	tc12_works_before_init();
	tc13_no_snapshot_mutation();

	printf("\n%d passed, %d failed\n", s_pass, s_fail);
	return (s_fail == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
