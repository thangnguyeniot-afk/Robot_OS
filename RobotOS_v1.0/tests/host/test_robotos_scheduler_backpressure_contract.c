/*
 * test_robotos_scheduler_backpressure_contract.c
 * Phase 4J — Scheduler Budget / Backpressure Policy Contract Tests
 *
 * Validates budget/backpressure observability in robotos_core:
 *   - dispatch_budget_per_tick returns ROBOTOS_CORE_MAX_EVENTS_PER_TICK
 *   - backpressure_active rule: pending > budget OR queue is full
 *   - pending/dropped/admission/unhandled/handler_error counters are distinguishable
 *   - tick drains bounded events under backlog
 *   - queue-full behavior is separate from admission rejection
 *   - snapshot backpressure_active matches getter
 *   - repeated init does not clear pending/backpressure state
 *
 * Backpressure is observability only in Phase 4J — it does not throttle
 * producers, adjust priority, or alter scheduler fairness.
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
    .type = ROBOTOS_EVENT_USER, .timestamp_tick = 0, .arg0 = 0, .arg1 = 0
};

static const robotos_event_t EV_NONE = {
    .type = ROBOTOS_EVENT_NONE, .timestamp_tick = 0, .arg0 = 0, .arg1 = 0
};

/* Handler that always returns OK — for testing unhandled/dispatch paths */
static robotos_core_status_t handler_ok(const robotos_event_t *event, void *ctx)
{
    (void)event; (void)ctx;
    return ROBOTOS_CORE_OK;
}

/* Handler that always returns an error — for testing handler_error path */
static robotos_core_status_t handler_err(const robotos_event_t *event, void *ctx)
{
    (void)event; (void)ctx;
    return ROBOTOS_CORE_ERR_INVALID_STATE; /* arbitrary non-OK */
}

/* -------------------------------------------------------------------------- */

int main(void)
{
    printf("=== Phase 4J: Scheduler Budget/Backpressure Contract Tests ===\n\n");

    robotos_core_status_t rc;
    robotos_core_snapshot_t snap;

    /* =========================================================================
     * TC01–TC02: Pre-init budget and backpressure
     * ========================================================================= */

    CHECK("TC01 pre-init: dispatch_budget_per_tick == ROBOTOS_CORE_MAX_EVENTS_PER_TICK",
          robotos_core_dispatch_budget_per_tick() == ROBOTOS_CORE_MAX_EVENTS_PER_TICK);

    CHECK("TC02 pre-init: backpressure_active == false",
          robotos_core_backpressure_active() == false);

    /* =========================================================================
     * TC03–TC04: Post-init empty state
     * ========================================================================= */

    rc = robotos_core_init();
    CHECK("init returns OK", rc == ROBOTOS_CORE_OK);

    CHECK("TC03 post-init: pending_event_count == 0",
          robotos_core_pending_event_count() == 0u);

    CHECK("TC04 post-init: backpressure_active == false",
          robotos_core_backpressure_active() == false);

    /* =========================================================================
     * TC05: Post exactly one event — pending == budget, no backpressure
     * budget = 1, pending = 1 → (1 > 1) is false, not full → false
     * ========================================================================= */

    rc = robotos_core_post_event(&EV_USER);
    CHECK("TC05 post 1 event returns OK", rc == ROBOTOS_CORE_OK);
    CHECK("TC05 pending == 1",
          robotos_core_pending_event_count() == 1u);
    CHECK("TC05 backpressure_active == false (pending == budget, not full)",
          robotos_core_backpressure_active() == false);

    /* =========================================================================
     * TC06: Post second event — pending > budget → backpressure active
     * ========================================================================= */

    rc = robotos_core_post_event(&EV_USER);
    CHECK("TC06 post 2nd event returns OK", rc == ROBOTOS_CORE_OK);
    CHECK("TC06 pending == 2",
          robotos_core_pending_event_count() == 2u);
    CHECK("TC06 backpressure_active == true (pending > budget)",
          robotos_core_backpressure_active() == true);

    /* =========================================================================
     * TC07: Tick once — drains budget(1) event, pending becomes 1, pressure off
     * ========================================================================= */

    rc = robotos_core_tick();
    /* tick returns OK (empty queue returns OK from tick, handler error propagates;
       unregistered event → unhandled_count++, routing handler returns OK → tick OK) */
    CHECK("TC07 tick returns OK", rc == ROBOTOS_CORE_OK);
    CHECK("TC07 pending == 1 after tick",
          robotos_core_pending_event_count() == 1u);
    CHECK("TC07 backpressure_active == false (pending == budget again)",
          robotos_core_backpressure_active() == false);

    /* Drain remaining event so next scenario starts clean */
    robotos_core_dispatch_events(1u);
    CHECK("drain: pending == 0", robotos_core_pending_event_count() == 0u);

    /* =========================================================================
     * TC08: Fill queue to capacity → backpressure active, pending == capacity
     * ========================================================================= */

    uint32_t filled = 0u;
    for (uint32_t i = 0u; i < ROBOTOS_EVENT_QUEUE_CAPACITY; i++) {
        if (robotos_core_post_event(&EV_USER) == ROBOTOS_CORE_OK) {
            filled++;
        }
    }
    CHECK("TC08 queue filled to capacity",
          filled == ROBOTOS_EVENT_QUEUE_CAPACITY);
    CHECK("TC08 pending == ROBOTOS_EVENT_QUEUE_CAPACITY",
          robotos_core_pending_event_count() == ROBOTOS_EVENT_QUEUE_CAPACITY);
    CHECK("TC08 backpressure_active == true (full queue)",
          robotos_core_backpressure_active() == true);

    /* =========================================================================
     * TC09: Extra valid post when full — ERR_FULL, dropped++, admitted unchanged,
     *       rejected unchanged, backpressure still true
     * ========================================================================= */

    uint32_t accepted_before = robotos_core_admission_accepted_count();
    uint32_t rejected_before = robotos_core_admission_rejected_count();
    uint32_t dropped_before  = robotos_core_dropped_event_count();

    rc = robotos_core_post_event(&EV_USER);
    CHECK("TC09 post when full returns ERR_FULL", rc == ROBOTOS_CORE_ERR_FULL);
    CHECK("TC09 dropped_count incremented",
          robotos_core_dropped_event_count() == dropped_before + 1u);
    CHECK("TC09 admission_accepted_count NOT incremented (queue push failed)",
          robotos_core_admission_accepted_count() == accepted_before);
    CHECK("TC09 admission_rejected_count NOT incremented (queue-full ≠ admission reject)",
          robotos_core_admission_rejected_count() == rejected_before);
    CHECK("TC09 backpressure_active still true",
          robotos_core_backpressure_active() == true);

    /* =========================================================================
     * TC10: Invalid (NONE) post under pressure — ERR_INVALID_ARG, rejected++,
     *       dropped_count unchanged, backpressure still true
     * ========================================================================= */

    uint32_t dropped_before10 = robotos_core_dropped_event_count();

    rc = robotos_core_post_event(&EV_NONE);
    CHECK("TC10 post NONE under pressure returns ERR_INVALID_ARG",
          rc == ROBOTOS_CORE_ERR_INVALID_ARG);
    CHECK("TC10 admission_rejected_count incremented",
          robotos_core_admission_rejected_count() == rejected_before + 1u);
    CHECK("TC10 dropped_count unchanged (admission reject, not queue drop)",
          robotos_core_dropped_event_count() == dropped_before10);
    CHECK("TC10 backpressure_active still true",
          robotos_core_backpressure_active() == true);

    /* =========================================================================
     * TC11: dispatch_events(large) drains full backlog →
     *       pending == 0, backpressure == false
     * ========================================================================= */

    /* Dispatch more than queue capacity to ensure full drain */
    rc = robotos_core_dispatch_events(ROBOTOS_EVENT_QUEUE_CAPACITY + 4u);
    /* Returns OK (at least one event dispatched before queue emptied) */
    CHECK("TC11 dispatch_events drains backlog returns OK", rc == ROBOTOS_CORE_OK);
    CHECK("TC11 pending == 0 after full drain",
          robotos_core_pending_event_count() == 0u);
    CHECK("TC11 backpressure_active == false after drain",
          robotos_core_backpressure_active() == false);

    /* =========================================================================
     * TC12: Unregistered valid events — dispatched, unhandled_count increments,
     *       backpressure reflects updated pending
     * ========================================================================= */

    uint32_t unhandled_before = robotos_core_unhandled_event_count();

    /* Post two events (no handler registered for USER type from prior drain) */
    robotos_core_post_event(&EV_USER);
    robotos_core_post_event(&EV_USER);
    CHECK("TC12 setup: pending == 2 before dispatch",
          robotos_core_pending_event_count() == 2u);

    /* Dispatch one — unhandled_count goes up, pending drops to 1 */
    robotos_core_dispatch_events(1u);
    CHECK("TC12 unhandled_count incremented after unregistered dispatch",
          robotos_core_unhandled_event_count() == unhandled_before + 1u);
    CHECK("TC12 pending == 1 after partial drain",
          robotos_core_pending_event_count() == 1u);
    CHECK("TC12 backpressure == false (pending 1 == budget 1)",
          robotos_core_backpressure_active() == false);

    /* Drain the last event */
    robotos_core_dispatch_events(1u);

    /* =========================================================================
     * TC13: Handler error — register handler that returns error,
     *       post event, tick returns handler error, event consumed,
     *       handler_error_count increments, pending decreases
     * ========================================================================= */

    /* Register error handler for USER type */
    rc = robotos_core_register_event_handler(ROBOTOS_EVENT_USER, handler_err, NULL);
    CHECK("TC13 register error handler returns OK", rc == ROBOTOS_CORE_OK);

    uint32_t herr_before    = robotos_core_handler_error_count();
    uint32_t pending_before = robotos_core_pending_event_count();

    rc = robotos_core_post_event(&EV_USER);
    CHECK("TC13 post event for handler_err returns OK", rc == ROBOTOS_CORE_OK);

    rc = robotos_core_tick();
    CHECK("TC13 tick propagates handler error (non-OK)",
          rc != ROBOTOS_CORE_OK);
    CHECK("TC13 handler_error_count incremented",
          robotos_core_handler_error_count() == herr_before + 1u);
    CHECK("TC13 event consumed: pending back to pre-post value",
          robotos_core_pending_event_count() == pending_before);
    CHECK("TC13 backpressure updated correctly after error dispatch",
          robotos_core_backpressure_active() == false);

    /* Unregister error handler to clean up */
    robotos_core_unregister_event_handler(ROBOTOS_EVENT_USER);

    /* =========================================================================
     * TC14: Repeated init does NOT clear pending or backpressure
     * ========================================================================= */

    /* Post two events to create pressure state */
    robotos_core_post_event(&EV_USER);
    robotos_core_post_event(&EV_USER);
    CHECK("TC14 setup: pending == 2 before repeated init",
          robotos_core_pending_event_count() == 2u);
    CHECK("TC14 setup: backpressure == true before repeated init",
          robotos_core_backpressure_active() == true);

    uint32_t accepted_14 = robotos_core_admission_accepted_count();
    uint32_t rejected_14 = robotos_core_admission_rejected_count();

    rc = robotos_core_init(); /* repeated init */
    CHECK("TC14 repeated init returns OK", rc == ROBOTOS_CORE_OK);
    CHECK("TC14 pending NOT cleared by repeated init",
          robotos_core_pending_event_count() == 2u);
    CHECK("TC14 backpressure still true after repeated init",
          robotos_core_backpressure_active() == true);
    CHECK("TC14 admission_accepted NOT reset by repeated init",
          robotos_core_admission_accepted_count() == accepted_14);
    CHECK("TC14 admission_rejected NOT reset by repeated init",
          robotos_core_admission_rejected_count() == rejected_14);

    /* Drain to clean up */
    robotos_core_dispatch_events(ROBOTOS_EVENT_QUEUE_CAPACITY);

    /* =========================================================================
     * TC15: Snapshot coherence — backpressure_active, pending, dropped,
     *       admission_accepted, admission_rejected, unhandled, handler_error
     *       all match their getters
     * ========================================================================= */

    /* Register OK handler and post one event to give snapshot non-trivial state */
    rc = robotos_core_register_event_handler(ROBOTOS_EVENT_USER, handler_ok, NULL);
    CHECK("TC15 setup: register OK handler", rc == ROBOTOS_CORE_OK);

    robotos_core_post_event(&EV_USER);
    robotos_core_post_event(&EV_USER); /* 2 pending → pressure */

    rc = robotos_core_snapshot(&snap);
    CHECK("TC15 snapshot returns OK", rc == ROBOTOS_CORE_OK);

    CHECK("TC15 snapshot pending_event_count matches getter",
          snap.pending_event_count == robotos_core_pending_event_count());
    CHECK("TC15 snapshot dropped_event_count matches getter",
          snap.dropped_event_count == robotos_core_dropped_event_count());
    CHECK("TC15 snapshot admission_accepted_count matches getter",
          snap.admission_accepted_count == robotos_core_admission_accepted_count());
    CHECK("TC15 snapshot admission_rejected_count matches getter",
          snap.admission_rejected_count == robotos_core_admission_rejected_count());
    CHECK("TC15 snapshot unhandled_event_count matches getter",
          snap.unhandled_event_count == robotos_core_unhandled_event_count());
    CHECK("TC15 snapshot handler_error_count matches getter",
          snap.handler_error_count == robotos_core_handler_error_count());
    CHECK("TC15 snapshot backpressure_active matches getter",
          snap.backpressure_active == robotos_core_backpressure_active());
    CHECK("TC15 snapshot backpressure_active == true (2 pending > budget 1)",
          snap.backpressure_active == true);

    /* =========================================================================
     * Final summary
     * ========================================================================= */

    printf("\n--- Backpressure Policy Contract: %d passed, %d failed ---\n",
           s_pass, s_fail);
    return (s_fail == 0) ? 0 : 1;
}
