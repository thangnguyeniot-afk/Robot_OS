/*
 * test_robotos_queue_pressure_contract.c
 *
 * Phase 6I -- Timer Producer Queue-Pressure Stress Contract Tests
 *
 * Validates the behavior of the core scheduler when a rapid producer fills
 * the queue beyond consumer rate. No hardware, no timing dependencies.
 *
 * Simulates queue-pressure scenario:
 *   - Rapid-post CAPACITY events (all accepted, queue full)
 *   - Post more valid events  -> ERR_FULL (dropped_count++)
 *   - Post invalid while full -> ERR_INVALID_ARG (admission gate first)
 *   - Dispatch one           -> try_post_event -> ERR_THROTTLED (pending > budget)
 *   - Verify backpressure_active during pressure, false after drain
 *
 * Also exercises retry policy alignment under pressure:
 *   - ERR_FULL      -> RETRY_AFTER_TICK, retryable
 *   - ERR_THROTTLED -> RETRY_AFTER_TICK, retryable
 *
 * No Zephyr. No hardware. No timing race. Exit non-zero on any failure.
 */

#include "robotos_core.h"
#include "robotos_event_queue.h"

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/* --------------------------------------------------------------------------
 * Test harness
 * -------------------------------------------------------------------------- */

static int s_pass = 0;
static int s_fail = 0;

#define CHECK(label, expr)                                              \
    do {                                                                \
        if (expr) {                                                     \
            printf("[PASS] %s\n", (label));                            \
            s_pass++;                                                   \
        } else {                                                        \
            printf("[FAIL] %s  (line %d)\n", (label), __LINE__);      \
            s_fail++;                                                   \
        }                                                               \
    } while (0)

/* --------------------------------------------------------------------------
 * Test events
 * -------------------------------------------------------------------------- */

static const robotos_event_t EV_USER = {
    .type = ROBOTOS_EVENT_USER, .timestamp_tick = 0,
    .arg0 = 0x6900u, .arg1 = 0
};

static const robotos_event_t EV_NONE = {
    .type = ROBOTOS_EVENT_NONE, .timestamp_tick = 0,
    .arg0 = 0, .arg1 = 0
};

/* --------------------------------------------------------------------------
 * Helpers
 * -------------------------------------------------------------------------- */

static void drain_queue(void)
{
    while (robotos_core_pending_event_count() > 0) {
        robotos_core_dispatch_events(ROBOTOS_EVENT_QUEUE_CAPACITY);
    }
}

static void fill_queue_to_capacity(uint32_t *accepted_out)
{
    uint32_t accepted = 0;
    for (uint32_t i = 0; i < ROBOTOS_EVENT_QUEUE_CAPACITY; i++) {
        if (robotos_core_post_event(&EV_USER) == ROBOTOS_CORE_OK) {
            accepted++;
        }
    }
    if (accepted_out) *accepted_out = accepted;
}

/* --------------------------------------------------------------------------
 * main
 * -------------------------------------------------------------------------- */

int main(void)
{
    printf("=== Phase 6I: Queue Pressure Producer Contract Tests ===\n\n");

    robotos_core_status_t         rc;
    robotos_core_snapshot_t       snap;
    robotos_core_retry_decision_t rd;
    uint32_t                      filled;

    /* ---- TC01-02: Init ---------------------------------------------------- */
    printf("[ Init ]\n");
    rc = robotos_core_init();
    CHECK("TC01: init returns OK", rc == ROBOTOS_CORE_OK);
    CHECK("TC02: state == READY", robotos_core_state() == ROBOTOS_CORE_STATE_READY);

    /* ---- TC03-06: Rapid fill to capacity ---------------------------------- */
    printf("\n[ Rapid fill to capacity ]\n");

    fill_queue_to_capacity(&filled);
    CHECK("TC03: all CAPACITY posts succeed", filled == ROBOTOS_EVENT_QUEUE_CAPACITY);
    CHECK("TC04: accepted_count == CAPACITY",
          robotos_core_admission_accepted_count() == ROBOTOS_EVENT_QUEUE_CAPACITY);
    CHECK("TC05: pending == CAPACITY (queue full)",
          robotos_core_pending_event_count() == ROBOTOS_EVENT_QUEUE_CAPACITY);

    /* Backpressure must be active: pending == CAPACITY > budget=1, queue is full */
    rc = robotos_core_snapshot(&snap);
    CHECK("TC06: backpressure_active == true when queue full",
          snap.backpressure_active);

    /* ---- TC07-10: Queue-pressure drop path -------------------------------- */
    printf("\n[ Queue-pressure overflow (ERR_FULL) ]\n");

    rc = robotos_core_post_event(&EV_USER);
    CHECK("TC07: post to full queue -> ERR_FULL", rc == ROBOTOS_CORE_ERR_FULL);
    CHECK("TC08: dropped_count == 1 after overflow",
          robotos_core_dropped_event_count() == 1);
    CHECK("TC09: pending unchanged == CAPACITY after overflow",
          robotos_core_pending_event_count() == ROBOTOS_EVENT_QUEUE_CAPACITY);
    CHECK("TC10: accepted_count unchanged (ERR_FULL not admitted)",
          robotos_core_admission_accepted_count() == ROBOTOS_EVENT_QUEUE_CAPACITY);

    /* ---- TC11-12: Admission gate runs before queue check while full ------- */
    printf("\n[ Admission gate before queue check ]\n");

    rc = robotos_core_post_event(&EV_NONE);
    CHECK("TC11: post NONE while full -> ERR_INVALID_ARG (not ERR_FULL)",
          rc == ROBOTOS_CORE_ERR_INVALID_ARG);
    CHECK("TC12: dropped_count still 1 (NONE rejected before queue check)",
          robotos_core_dropped_event_count() == 1);
    CHECK("TC13: rejected_count == 1 (NONE rejected by admission)",
          robotos_core_admission_rejected_count() == 1);

    /* ---- TC14-16: try_post_event throttle under partial pressure ---------- */
    printf("\n[ try_post_event throttle under partial pressure ]\n");

    /* Dispatch one event: queue goes from CAPACITY (full) to CAPACITY-1 (not full) */
    rc = robotos_core_dispatch_events(1);
    CHECK("TC14: dispatch 1 returns OK", rc == ROBOTOS_CORE_OK);
    CHECK("TC15: pending == CAPACITY-1 after dispatch",
          robotos_core_pending_event_count() == ROBOTOS_EVENT_QUEUE_CAPACITY - 1u);

    /* Now: not full, pending = CAPACITY-1 = 15 > budget=1 -> ERR_THROTTLED */
    rc = robotos_core_try_post_event(&EV_USER);
    CHECK("TC16: try_post_event with pending>budget and not full -> ERR_THROTTLED",
          rc == ROBOTOS_CORE_ERR_THROTTLED);
    CHECK("TC17: producer_throttled_count == 1",
          robotos_core_producer_throttled_count() == 1u);
    CHECK("TC18: pending unchanged after ERR_THROTTLED",
          robotos_core_pending_event_count() == ROBOTOS_EVENT_QUEUE_CAPACITY - 1u);
    CHECK("TC19: dropped_count still 1 (throttle != drop)",
          robotos_core_dropped_event_count() == 1u);

    /* ---- TC20-23: Snapshot consistency under pressure -------------------- */
    printf("\n[ Snapshot consistency under pressure ]\n");

    rc = robotos_core_snapshot(&snap);
    CHECK("TC20: snapshot returns OK", rc == ROBOTOS_CORE_OK);
    CHECK("TC21: snap.accepted == CAPACITY",
          snap.admission_accepted_count == ROBOTOS_EVENT_QUEUE_CAPACITY);
    CHECK("TC22: snap.rejected == 1",
          snap.admission_rejected_count == 1u);
    CHECK("TC23: snap.dropped == 1",
          snap.dropped_event_count == 1u);
    CHECK("TC24: snap.producer_throttled_count == 1",
          snap.producer_throttled_count == 1u);
    /* Backpressure: pending = CAPACITY-1 = 15 > budget=1, not full */
    CHECK("TC25: backpressure_active == true (pending > budget)",
          snap.backpressure_active);
    CHECK("TC26: producer_throttle_active == true (pending > budget, not full)",
          snap.producer_throttle_active);

    /* ---- TC27-30: Drain and settle --------------------------------------- */
    printf("\n[ Drain and settle ]\n");

    drain_queue();
    CHECK("TC27: pending == 0 after drain",
          robotos_core_pending_event_count() == 0);
    CHECK("TC28: dispatched_count == CAPACITY (all queued events dispatched)",
          robotos_core_dispatched_event_count() == ROBOTOS_EVENT_QUEUE_CAPACITY);

    rc = robotos_core_snapshot(&snap);
    CHECK("TC29: backpressure_active == false after drain",
          !snap.backpressure_active);
    CHECK("TC30: producer_throttle_active == false after drain",
          !snap.producer_throttle_active);

    /* ---- TC31-34: Retry policy alignment under queue pressure ------------ */
    printf("\n[ Retry policy alignment ]\n");

    memset(&rd, 0, sizeof(rd));
    rc = robotos_core_retry_decision_for_status(ROBOTOS_CORE_ERR_FULL, &rd);
    CHECK("TC31: retry_decision(ERR_FULL) returns OK", rc == ROBOTOS_CORE_OK);
    CHECK("TC32: ERR_FULL -> RETRY_AFTER_TICK",
          rd.action == ROBOTOS_CORE_RETRY_AFTER_TICK);
    CHECK("TC33: ERR_FULL -> wait_ticks == 1", rd.suggested_wait_ticks == 1u);
    CHECK("TC34: ERR_FULL is retryable",
          robotos_core_status_is_retryable(ROBOTOS_CORE_ERR_FULL));

    memset(&rd, 0, sizeof(rd));
    rc = robotos_core_retry_decision_for_status(ROBOTOS_CORE_ERR_THROTTLED, &rd);
    CHECK("TC35: retry_decision(ERR_THROTTLED) returns OK", rc == ROBOTOS_CORE_OK);
    CHECK("TC36: ERR_THROTTLED -> RETRY_AFTER_TICK",
          rd.action == ROBOTOS_CORE_RETRY_AFTER_TICK);
    CHECK("TC37: ERR_THROTTLED -> wait_ticks == 1", rd.suggested_wait_ticks == 1u);
    CHECK("TC38: ERR_THROTTLED is retryable",
          robotos_core_status_is_retryable(ROBOTOS_CORE_ERR_THROTTLED));

    /* Both queue-pressure error codes map to the same retry guidance */
    CHECK("TC39: ERR_FULL and ERR_THROTTLED both RETRY_AFTER_TICK -- same guidance",
          robotos_core_status_is_retryable(ROBOTOS_CORE_ERR_FULL) &&
          robotos_core_status_is_retryable(ROBOTOS_CORE_ERR_THROTTLED));

    /* ---- TC40-42: Producer succeeds once queue drains to safe level ------- */
    printf("\n[ Post succeeds after drain ]\n");

    uint32_t prev_accepted = robotos_core_admission_accepted_count();
    rc = robotos_core_post_event(&EV_USER);
    CHECK("TC40: post succeeds after drain", rc == ROBOTOS_CORE_OK);
    CHECK("TC41: accepted_count incremented after post",
          robotos_core_admission_accepted_count() == prev_accepted + 1u);
    drain_queue();
    CHECK("TC42: pending == 0 after final drain",
          robotos_core_pending_event_count() == 0);

    /* ---- Summary --------------------------------------------------------- */
    printf("\n=== Phase 6I Results: %d passed, %d failed ===\n", s_pass, s_fail);
    return (s_fail > 0) ? 1 : 0;
}
