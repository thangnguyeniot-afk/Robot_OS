/*
 * test_robotos_scheduler_throttle_contract.c
 * Phase 4K — Scheduler Producer Throttle Policy Contract Tests
 *
 * Validates producer throttle semantics in robotos_core:
 *   - try_post_event returns ERR_THROTTLED when pending > budget and queue not full
 *   - throttled events do not enter queue
 *   - throttled events do not increment admission_accepted/rejected or dropped
 *   - post_event bypasses throttle (raw ingestion path unchanged)
 *   - queue-full through post_event returns ERR_FULL + dropped (not throttled)
 *   - queue-full through try_post_event also returns ERR_FULL + dropped (not throttled)
 *   - producer_throttle_active getter matches throttle condition
 *   - snapshot reports producer_throttle_active and producer_throttled_count
 *   - repeated init does not reset producer_throttled_count
 *   - tick drains events and clears throttle when pending drops to budget
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
    printf("=== Phase 4K: Scheduler Producer Throttle Contract Tests ===\n\n");

    robotos_core_status_t rc;
    robotos_core_snapshot_t snap;

    /* =========================================================================
     * TC01: Pre-init throttle state
     * ========================================================================= */

    CHECK("TC01 pre-init: producer_throttle_active == false",
          robotos_core_producer_throttle_active() == false);
    CHECK("TC01 pre-init: producer_throttled_count == 0",
          robotos_core_producer_throttled_count() == 0u);

    /* =========================================================================
     * TC02: try_post_event before init returns ERR_INVALID_STATE (not throttled)
     * ========================================================================= */

    rc = robotos_core_try_post_event(&EV_USER);
    CHECK("TC02 try_post_event before init returns ERR_INVALID_STATE",
          rc == ROBOTOS_CORE_ERR_INVALID_STATE);
    CHECK("TC02 throttled_count unchanged (state check fires before throttle)",
          robotos_core_producer_throttled_count() == 0u);

    /* =========================================================================
     * TC03: Post-init empty state
     * ========================================================================= */

    rc = robotos_core_init();
    CHECK("init returns OK", rc == ROBOTOS_CORE_OK);

    CHECK("TC03 post-init: throttle_active == false",
          robotos_core_producer_throttle_active() == false);
    CHECK("TC03 post-init: throttled_count == 0",
          robotos_core_producer_throttled_count() == 0u);
    CHECK("TC03 post-init: pending == 0",
          robotos_core_pending_event_count() == 0u);

    /* =========================================================================
     * TC04: try_post_event NULL -> ERR_NULL, throttled_count unchanged
     * ========================================================================= */

    rc = robotos_core_try_post_event(NULL);
    CHECK("TC04 try_post_event NULL returns ERR_NULL",
          rc == ROBOTOS_CORE_ERR_NULL);
    CHECK("TC04 throttled_count unchanged",
          robotos_core_producer_throttled_count() == 0u);

    /* =========================================================================
     * TC05: try_post_event invalid type -> ERR_INVALID_ARG,
     *       admission_rejected++, throttled_count unchanged
     * ========================================================================= */

    uint32_t rejected_before = robotos_core_admission_rejected_count();
    uint32_t throttled_before = robotos_core_producer_throttled_count();

    rc = robotos_core_try_post_event(&EV_NONE);
    CHECK("TC05 try_post_event NONE returns ERR_INVALID_ARG",
          rc == ROBOTOS_CORE_ERR_INVALID_ARG);
    CHECK("TC05 admission_rejected incremented",
          robotos_core_admission_rejected_count() == rejected_before + 1u);
    CHECK("TC05 throttled_count unchanged (admission fires before throttle)",
          robotos_core_producer_throttled_count() == throttled_before);

    /* =========================================================================
     * TC06: try_post_event event #1 -> OK
     *       pending=1, budget=1 -> pending == budget -> throttle=false
     * ========================================================================= */

    uint32_t accepted_before = robotos_core_admission_accepted_count();

    rc = robotos_core_try_post_event(&EV_USER);
    CHECK("TC06 try_post_event #1 returns OK", rc == ROBOTOS_CORE_OK);
    CHECK("TC06 pending == 1", robotos_core_pending_event_count() == 1u);
    CHECK("TC06 accepted incremented",
          robotos_core_admission_accepted_count() == accepted_before + 1u);
    CHECK("TC06 throttle_active == false (pending == budget, not > budget)",
          robotos_core_producer_throttle_active() == false);
    CHECK("TC06 backpressure_active == false",
          robotos_core_backpressure_active() == false);
    CHECK("TC06 throttled_count still 0",
          robotos_core_producer_throttled_count() == 0u);

    /* =========================================================================
     * TC07: try_post_event event #2 -> OK (causes backpressure AND throttle)
     *       pending=2 > budget=1 -> backpressure=true, throttle=true
     *       The event causing the transition IS accepted.
     * ========================================================================= */

    uint32_t accepted_07 = robotos_core_admission_accepted_count();

    rc = robotos_core_try_post_event(&EV_USER);
    CHECK("TC07 try_post_event #2 returns OK (transition event is accepted)",
          rc == ROBOTOS_CORE_OK);
    CHECK("TC07 pending == 2", robotos_core_pending_event_count() == 2u);
    CHECK("TC07 accepted incremented for event #2",
          robotos_core_admission_accepted_count() == accepted_07 + 1u);
    CHECK("TC07 backpressure_active == true (pending > budget)",
          robotos_core_backpressure_active() == true);
    CHECK("TC07 throttle_active == true (pending > budget, queue not full)",
          robotos_core_producer_throttle_active() == true);
    CHECK("TC07 throttled_count still 0 (event #2 was accepted, not throttled)",
          robotos_core_producer_throttled_count() == 0u);

    /* =========================================================================
     * TC08: try_post_event event #3 while pending=2 -> ERR_THROTTLED
     *       pending unchanged, accepted unchanged, rejected unchanged,
     *       dropped unchanged, throttled_count incremented
     * ========================================================================= */

    uint32_t pending_08   = robotos_core_pending_event_count();
    uint32_t accepted_08  = robotos_core_admission_accepted_count();
    uint32_t rejected_08  = robotos_core_admission_rejected_count();
    uint32_t dropped_08   = robotos_core_dropped_event_count();
    uint32_t throttled_08 = robotos_core_producer_throttled_count();

    rc = robotos_core_try_post_event(&EV_USER);
    CHECK("TC08 try_post_event #3 returns ERR_THROTTLED",
          rc == ROBOTOS_CORE_ERR_THROTTLED);
    CHECK("TC08 pending unchanged", robotos_core_pending_event_count() == pending_08);
    CHECK("TC08 accepted unchanged", robotos_core_admission_accepted_count() == accepted_08);
    CHECK("TC08 rejected unchanged", robotos_core_admission_rejected_count() == rejected_08);
    CHECK("TC08 dropped unchanged", robotos_core_dropped_event_count() == dropped_08);
    CHECK("TC08 throttled_count incremented",
          robotos_core_producer_throttled_count() == throttled_08 + 1u);
    CHECK("TC08 throttle_active still true",
          robotos_core_producer_throttle_active() == true);

    /* Multiple throttled events accumulate */
    rc = robotos_core_try_post_event(&EV_USER);
    CHECK("TC08b second throttled -> ERR_THROTTLED",
          rc == ROBOTOS_CORE_ERR_THROTTLED);
    CHECK("TC08b throttled_count == 2",
          robotos_core_producer_throttled_count() == throttled_08 + 2u);

    /* =========================================================================
     * TC09: Tick drains one event -> pending=1 == budget -> throttle clears
     * ========================================================================= */

    rc = robotos_core_tick();
    CHECK("TC09 tick returns OK", rc == ROBOTOS_CORE_OK);
    CHECK("TC09 pending == 1 after tick", robotos_core_pending_event_count() == 1u);
    CHECK("TC09 throttle_active == false (pending == budget)",
          robotos_core_producer_throttle_active() == false);
    CHECK("TC09 backpressure_active == false",
          robotos_core_backpressure_active() == false);

    /* =========================================================================
     * TC10: try_post_event succeeds after throttle clears
     * ========================================================================= */

    uint32_t accepted_10 = robotos_core_admission_accepted_count();

    rc = robotos_core_try_post_event(&EV_USER);
    CHECK("TC10 try_post_event succeeds after throttle clear",
          rc == ROBOTOS_CORE_OK);
    CHECK("TC10 pending == 2 again",
          robotos_core_pending_event_count() == 2u);
    CHECK("TC10 accepted incremented",
          robotos_core_admission_accepted_count() == accepted_10 + 1u);
    CHECK("TC10 throttle_active == true again",
          robotos_core_producer_throttle_active() == true);

    /* Drain to clean up */
    robotos_core_dispatch_events(ROBOTOS_EVENT_QUEUE_CAPACITY);
    CHECK("TC10 drain: pending == 0",
          robotos_core_pending_event_count() == 0u);

    /* =========================================================================
     * TC11: post_event (raw) fills queue to capacity — bypasses throttle.
     *       ERR_FULL returned on extra valid event, dropped_count increments.
     * ========================================================================= */

    uint32_t filled = 0u;
    for (uint32_t i = 0u; i < ROBOTOS_EVENT_QUEUE_CAPACITY; i++) {
        if (robotos_core_post_event(&EV_USER) == ROBOTOS_CORE_OK) {
            filled++;
        }
    }
    CHECK("TC11 post_event (raw) fills queue to capacity",
          filled == ROBOTOS_EVENT_QUEUE_CAPACITY);
    CHECK("TC11 pending == ROBOTOS_EVENT_QUEUE_CAPACITY",
          robotos_core_pending_event_count() == ROBOTOS_EVENT_QUEUE_CAPACITY);

    uint32_t dropped_11   = robotos_core_dropped_event_count();
    uint32_t throttled_11 = robotos_core_producer_throttled_count();

    rc = robotos_core_post_event(&EV_USER);
    CHECK("TC11 extra post_event when full returns ERR_FULL",
          rc == ROBOTOS_CORE_ERR_FULL);
    CHECK("TC11 dropped_count incremented",
          robotos_core_dropped_event_count() == dropped_11 + 1u);
    CHECK("TC11 throttled_count NOT incremented (post_event bypasses throttle)",
          robotos_core_producer_throttled_count() == throttled_11);

    /* =========================================================================
     * TC12: try_post_event when queue is full -> ERR_FULL (not throttled)
     *       Queue-full takes priority over throttle in try_post_event.
     * ========================================================================= */

    uint32_t dropped_12   = robotos_core_dropped_event_count();
    uint32_t throttled_12 = robotos_core_producer_throttled_count();

    rc = robotos_core_try_post_event(&EV_USER);
    CHECK("TC12 try_post_event when full returns ERR_FULL (not ERR_THROTTLED)",
          rc == ROBOTOS_CORE_ERR_FULL);
    CHECK("TC12 dropped_count incremented",
          robotos_core_dropped_event_count() == dropped_12 + 1u);
    CHECK("TC12 throttled_count NOT incremented (full queue is not throttle path)",
          robotos_core_producer_throttled_count() == throttled_12);
    CHECK("TC12 throttle_active == false when queue is full",
          robotos_core_producer_throttle_active() == false);
    CHECK("TC12 backpressure_active == true (queue full)",
          robotos_core_backpressure_active() == true);

    /* Drain full queue */
    robotos_core_dispatch_events(ROBOTOS_EVENT_QUEUE_CAPACITY + 4u);
    CHECK("TC12 drain: pending == 0", robotos_core_pending_event_count() == 0u);

    /* =========================================================================
     * TC13: Snapshot reports throttle fields correctly
     * ========================================================================= */

    rc = robotos_core_register_event_handler(ROBOTOS_EVENT_USER, handler_ok, NULL);
    CHECK("TC13 setup: register OK handler", rc == ROBOTOS_CORE_OK);

    /* Create throttle state: pending=2 */
    robotos_core_try_post_event(&EV_USER);
    robotos_core_try_post_event(&EV_USER);

    rc = robotos_core_snapshot(&snap);
    CHECK("TC13 snapshot returns OK", rc == ROBOTOS_CORE_OK);
    CHECK("TC13 snapshot.producer_throttle_active == true",
          snap.producer_throttle_active == true);
    CHECK("TC13 snapshot.producer_throttle_active matches getter",
          snap.producer_throttle_active == robotos_core_producer_throttle_active());
    CHECK("TC13 snapshot.producer_throttled_count matches getter",
          snap.producer_throttled_count == robotos_core_producer_throttled_count());
    CHECK("TC13 snapshot.backpressure_active == true",
          snap.backpressure_active == true);

    /* Drain to clear */
    robotos_core_dispatch_events(ROBOTOS_EVENT_QUEUE_CAPACITY);

    rc = robotos_core_snapshot(&snap);
    CHECK("TC13b snapshot after drain: throttle_active == false",
          snap.producer_throttle_active == false);
    CHECK("TC13b snapshot after drain: backpressure_active == false",
          snap.backpressure_active == false);

    /* =========================================================================
     * TC14: Repeated init does NOT reset producer_throttled_count
     *       (consistent with admission_accepted/rejected behavior)
     * ========================================================================= */

    /* Create some throttle events */
    robotos_core_try_post_event(&EV_USER);
    robotos_core_try_post_event(&EV_USER);
    robotos_core_try_post_event(&EV_USER); /* throttled */

    uint32_t throttled_before14 = robotos_core_producer_throttled_count();

    rc = robotos_core_init(); /* repeated init */
    CHECK("TC14 repeated init returns OK", rc == ROBOTOS_CORE_OK);
    CHECK("TC14 producer_throttled_count NOT reset by repeated init",
          robotos_core_producer_throttled_count() == throttled_before14);

    /* Drain remaining events */
    robotos_core_dispatch_events(ROBOTOS_EVENT_QUEUE_CAPACITY);

    /* =========================================================================
     * TC15: dispatch_events drains backlog and clears throttle condition
     * ========================================================================= */

    robotos_core_try_post_event(&EV_USER);
    robotos_core_try_post_event(&EV_USER);
    CHECK("TC15 setup: throttle active with 2 pending",
          robotos_core_producer_throttle_active() == true);

    rc = robotos_core_dispatch_events(ROBOTOS_EVENT_QUEUE_CAPACITY);
    CHECK("TC15 dispatch_events returns OK", rc == ROBOTOS_CORE_OK);
    CHECK("TC15 pending == 0 after full drain",
          robotos_core_pending_event_count() == 0u);
    CHECK("TC15 throttle_active == false after drain",
          robotos_core_producer_throttle_active() == false);

    /* =========================================================================
     * Final summary
     * ========================================================================= */

    printf("\n--- Throttle Policy Contract: %d passed, %d failed ---\n",
           s_pass, s_fail);
    return (s_fail == 0) ? 0 : 1;
}
