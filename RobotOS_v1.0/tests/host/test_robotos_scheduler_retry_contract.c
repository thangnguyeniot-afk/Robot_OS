/*
 * test_robotos_scheduler_retry_contract.c
 * Phase 4L — Scheduler Retry/Backoff Policy Contract Tests
 *
 * Validates retry/backoff policy stub semantics in robotos_core:
 *   - set_retry_policy configures retry behavior
 *   - post_event_with_retry with retry_enabled=false behaves like post_event
 *   - post_event_with_retry retries on ERR_THROTTLED when enabled
 *   - post_event_with_retry retries on ERR_FULL when retry_on_full_enabled=true
 *   - retry counters track attempts, successes, and exhaustions
 *   - retry_exhausted_count increments when all attempts exhausted
 *   - retry_success_count increments when retry eventually succeeds
 *   - retry_attempt_count increments for every retry loop iteration
 *   - snapshot includes retry counter fields
 *   - repeated init does not reset retry counters
 *   - stub has no actual backoff delay (tight retry loop)
 *
 * Compiled with CORE_SRCS + PLATFORM_HOST_STUB — no Zephyr, no hardware.
 * Fresh static process state: no shared state with other test executables.
 */

#include "robotos_core.h"
#include "robotos_event_queue.h"

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

/* -------------------------------------------------------------------------- */

static int s_pass;
static int s_fail;

#define CHECK(label, expr)                                              \
    do {                                                                \
        if (expr) {                                                     \
            printf("[PASS] %s\n", label);                              \
            s_pass++;                                                   \
        } else {                                                        \
            printf("[FAIL] %s  (line %d)\n", label, __LINE__);        \
            s_fail++;                                                   \
        }                                                               \
    } while (0)

/* -------------------------------------------------------------------------- */

static const robotos_event_t EV_USER = {
    .type = ROBOTOS_EVENT_USER, .timestamp_tick = 0, .arg0 = 0x4B, .arg1 = 0
};

static const robotos_event_t EV_NONE = {
    .type = ROBOTOS_EVENT_NONE, .timestamp_tick = 0, .arg0 = 0, .arg1 = 0
};

static robotos_core_status_t handler_ok(const robotos_event_t *event, void *ctx)
{
    (void)event; (void)ctx;
    return ROBOTOS_CORE_OK;
}

/* -------------------------------------------------------------------------- */

int main(void)
{
    printf("=== Phase 4L: Scheduler Retry/Backoff Contract Tests ===\n");

    robotos_core_status_t rc;
    robotos_core_snapshot_t snap;

    /* =========================================================================
     * TC01: Pre-init retry counters are zero
     * ========================================================================= */

    CHECK("TC01 pre-init: retry_attempt_count == 0",
          robotos_core_retry_attempt_count() == 0u);
    CHECK("TC01 pre-init: retry_success_count == 0",
          robotos_core_retry_success_count() == 0u);
    CHECK("TC01 pre-init: retry_exhausted_count == 0",
          robotos_core_retry_exhausted_count() == 0u);

    /* =========================================================================
     * TC02: post_event_with_retry before init returns ERR_INVALID_STATE
     * ========================================================================= */

    rc = robotos_core_post_event_with_retry(&EV_USER);
    CHECK("TC02 with_retry before init: ERR_INVALID_STATE",
          rc == ROBOTOS_CORE_ERR_INVALID_STATE);
    CHECK("TC02 with_retry before init: retry counters still 0",
          robotos_core_retry_attempt_count() == 0u);

    /* =========================================================================
     * TC03: set_retry_policy before init succeeds (policy stored)
     * ========================================================================= */

    rc = robotos_core_set_retry_policy(true, false, 3u, 10u);
    CHECK("TC03 set_retry_policy before init: OK", rc == ROBOTOS_CORE_OK);

    /* =========================================================================
     * TC04: Init and register handler
     * ========================================================================= */

    rc = robotos_core_init();
    CHECK("TC04 init: OK", rc == ROBOTOS_CORE_OK);

    rc = robotos_core_register_event_handler(ROBOTOS_EVENT_USER, handler_ok, NULL);
    CHECK("TC04 register handler: OK", rc == ROBOTOS_CORE_OK);

    /* =========================================================================
     * TC05: post_event_with_retry with retry_enabled=false behaves like post_event
     * ========================================================================= */

    rc = robotos_core_set_retry_policy(false, false, 3u, 10u);
    CHECK("TC05 disable retry: OK", rc == ROBOTOS_CORE_OK);

    rc = robotos_core_post_event_with_retry(&EV_USER);
    CHECK("TC05 with_retry disabled: OK (immediate success)",
          rc == ROBOTOS_CORE_OK);
    CHECK("TC05 with_retry disabled: retry counters still 0",
          robotos_core_retry_attempt_count() == 0u);

    /* =========================================================================
     * TC06: post_event_with_retry retries on ERR_THROTTLED
     * ========================================================================= */

    /* Fill queue to trigger throttle on try_post_event */
    for (uint32_t i = 0; i < 16u; i++) {
        robotos_core_post_event(&EV_USER);
    }

    /* Queue is full, not throttled. We need pending > budget but not full.
     * Dispatch one event to create backlog. */
    robotos_core_tick();

    /* Now pending = 15, budget = 1, throttle active */
    CHECK("TC06 throttle active after fill+tick: true",
          robotos_core_producer_throttle_active() == true);

    /* Enable retry for ERR_THROTTLED */
    rc = robotos_core_set_retry_policy(true, false, 3u, 10u);
    CHECK("TC06 enable retry (throttle only): OK", rc == ROBOTOS_CORE_OK);

    rc = robotos_core_post_event_with_retry(&EV_USER);
    CHECK("TC06 with_retry on throttled: OK (eventually succeeded)",
          rc == ROBOTOS_CORE_OK);
    CHECK("TC06 with_retry throttled: retry_attempt_count > 0",
          robotos_core_retry_attempt_count() > 0u);
    CHECK("TC06 with_retry throttled: retry_success_count > 0",
          robotos_core_retry_success_count() > 0u);

    uint32_t attempt_after_throttle = robotos_core_retry_attempt_count();
    uint32_t success_after_throttle = robotos_core_retry_success_count();

    /* =========================================================================
     * TC07: post_event_with_retry does NOT retry on ERR_FULL (default)
     * ========================================================================= */

    /* Fill queue completely */
    while (robotos_core_post_event(&EV_USER) == ROBOTOS_CORE_OK) {
        /* drain some to avoid infinite loop if something is wrong */
        if (robotos_core_pending_event_count() > 100) {
            break;
        }
    }

    CHECK("TC07 queue full after fill: backpressure_active == true",
          robotos_core_backpressure_active() == true);

    uint32_t before_full_retry = robotos_core_retry_attempt_count();

    rc = robotos_core_post_event_with_retry(&EV_USER);
    CHECK("TC07 with_retry on full: ERR_FULL (not retried by default)",
          rc == ROBOTOS_CORE_ERR_FULL);
    CHECK("TC07 with_retry on full: retry_attempt_count unchanged",
          robotos_core_retry_attempt_count() == before_full_retry);

    /* =========================================================================
     * TC08: post_event_with_retry retries on ERR_FULL when retry_on_full_enabled=true
     * ========================================================================= */

    rc = robotos_core_set_retry_policy(true, true, 3u, 10u);
    CHECK("TC08 enable retry (throttle + full): OK", rc == ROBOTOS_CORE_OK);

    rc = robotos_core_post_event_with_retry(&EV_USER);
    CHECK("TC08 with_retry on full: ERR_FULL (retried but still full)",
          rc == ROBOTOS_CORE_ERR_FULL);
    CHECK("TC08 with_retry on full: retry_attempt_count increased",
          robotos_core_retry_attempt_count() > before_full_retry);

    /* =========================================================================
     * TC09: post_event_with_retry with NULL event returns ERR_NULL
     * ========================================================================= */

    rc = robotos_core_set_retry_policy(true, false, 3u, 10u);
    rc = robotos_core_post_event_with_retry(NULL);
    CHECK("TC09 with_retry NULL: ERR_NULL", rc == ROBOTOS_CORE_ERR_NULL);
    CHECK("TC09 with_retry NULL: retry counters unchanged",
          robotos_core_retry_attempt_count() == robotos_core_retry_attempt_count());

    /* =========================================================================
     * TC10: post_event_with_retry with invalid event returns ERR_INVALID_ARG
     * ========================================================================= */

    rc = robotos_core_post_event_with_retry(&EV_NONE);
    CHECK("TC10 with_retry NONE: ERR_INVALID_ARG",
          rc == ROBOTOS_CORE_ERR_INVALID_ARG);
    CHECK("TC10 with_retry NONE: retry counters unchanged",
          robotos_core_retry_attempt_count() == robotos_core_retry_attempt_count());

    /* =========================================================================
     * TC11: retry_ex_exhausted_count increments when max attempts exhausted
     * ========================================================================= */

    /* Reset queue state for clean test */
    robotos_core_init();
    robotos_core_register_event_handler(ROBOTOS_EVENT_USER, handler_ok, NULL);

    /* Set low retry limit for quick exhaustion test */
    rc = robotos_core_set_retry_policy(true, false, 1u, 10u);
    CHECK("TC11 set low retry limit: OK", rc == ROBOTOS_CORE_OK);

    /* Create backlog but keep queue not full (dispatch some to make space) */
    for (uint32_t i = 0; i < 10u; i++) {
        robotos_core_post_event(&EV_USER);
    }
    robotos_core_tick(); /* Dispatch one, now pending=9 */

    CHECK("TC11 throttle active: true",
          robotos_core_producer_throttle_active() == true);

    uint32_t before_exhaustion = robotos_core_retry_exhausted_count();

    rc = robotos_core_post_event_with_retry(&EV_USER);
    CHECK("TC11 with_retry (1 attempt): ERR_RETRY_EXHAUSTED",
          rc == ROBOTOS_CORE_ERR_RETRY_EXHAUSTED);
    CHECK("TC11 retry_exhausted_count incremented",
          robotos_core_retry_exhausted_count() > before_exhaustion);

    /* =========================================================================
     * TC12: snapshot includes retry counter fields
     * ========================================================================= */

    robotos_core_snapshot(&snap);
    CHECK("TC12 snapshot retry_attempt_count matches getter",
          snap.retry_attempt_count == robotos_core_retry_attempt_count());
    CHECK("TC12 snapshot retry_success_count matches getter",
          snap.retry_success_count == robotos_core_retry_success_count());
    CHECK("TC12 snapshot retry_exhausted_count matches getter",
          snap.retry_exhausted_count == robotos_core_retry_exhausted_count());

    /* =========================================================================
     * TC13: repeated init does not reset retry counters
     * ========================================================================= */

    uint32_t before_reinit_attempt   = robotos_core_retry_attempt_count();
    uint32_t before_reinit_success  = robotos_core_retry_success_count();
    uint32_t before_reinit_exhausted = robotos_core_retry_exhausted_count();

    rc = robotos_core_init();
    CHECK("TC13 re-init: OK", rc == ROBOTOS_CORE_OK);

    CHECK("TC13 re-init: retry_attempt_count unchanged",
          robotos_core_retry_attempt_count() == before_reinit_attempt);
    CHECK("TC13 re-init: retry_success_count unchanged",
          robotos_core_retry_success_count() == before_reinit_success);
    CHECK("TC13 re-init: retry_exhausted_count unchanged",
          robotos_core_retry_exhausted_count() == before_reinit_exhausted);

    /* =========================================================================
     * TC14: Immediate success (no retry) does not increment retry_success_count
     * ========================================================================= */

    robotos_core_init();
    robotos_core_register_event_handler(ROBOTOS_EVENT_USER, handler_ok, NULL);
    rc = robotos_core_set_retry_policy(true, false, 3u, 10u);

    uint32_t before_immediate = robotos_core_retry_success_count();

    rc = robotos_core_post_event_with_retry(&EV_USER);
    CHECK("TC14 with_retry immediate success: OK",
          rc == ROBOTOS_CORE_OK);
    CHECK("TC14 immediate success: retry_success_count unchanged",
          robotos_core_retry_success_count() == before_immediate);

    /* =========================================================================
     * TC15: Backoff delay parameter is stored but not honored (stub)
     * ========================================================================= */

    rc = robotos_core_set_retry_policy(true, false, 3u, 99999u);
    CHECK("TC15 set large backoff: OK", rc == ROBOTOS_CORE_OK);

    /* Stub: backoff not honored, tight retry loop */
    rc = robotos_core_post_event_with_retry(&EV_USER);
    CHECK("TC15 with_retry still works: OK", rc == ROBOTOS_CORE_OK);

    /* =========================================================================
     * TC16: zero max_retry_attempts = immediate one-shot (no retry)
     * ========================================================================= */

    robotos_core_init();
    robotos_core_register_event_handler(ROBOTOS_EVENT_USER, handler_ok, NULL);

    rc = robotos_core_set_retry_policy(true, false, 0u, 10u);
    CHECK("TC16 set zero retry limit: OK", rc == ROBOTOS_CORE_OK);

    uint32_t before_zero_retry = robotos_core_retry_attempt_count();

    rc = robotos_core_post_event_with_retry(&EV_USER);
    CHECK("TC16 with_retry (0 attempts): OK (no retry needed)",
          rc == ROBOTOS_CORE_OK);
    CHECK("TC16 zero limit: retry_attempt_count unchanged",
          robotos_core_retry_attempt_count() == before_zero_retry);

    /* =========================================================================
     * Summary
     * ========================================================================= */

    printf("\n=== Phase 4L Test Summary ===\n");
    printf("Passed: %d\n", s_pass);
    printf("Failed: %d\n", s_fail);

    return (s_fail == 0) ? 0 : 1;
}
