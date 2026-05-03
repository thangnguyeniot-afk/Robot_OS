/*
 * test_robotos_mixed_event_policy_contract.c
 *
 * Phase 6F -- Mixed Event Policy Smoke Contract Tests
 *
 * Validates that valid, invalid, and full-queue events are handled
 * correctly in a single test run using existing core APIs.
 *
 * Covers three admission/drop paths in one run:
 *   - Valid event (USER type)    -> OK, accepted_count++
 *   - Invalid event (NONE type)  -> ERR_INVALID_ARG, rejected_count++
 *   - Invalid event (type=99)    -> ERR_INVALID_ARG, rejected_count++
 *   - Full queue (valid event)   -> ERR_FULL, dropped_count++
 *   - Invalid while full         -> ERR_INVALID_ARG (not ERR_FULL, admission first)
 *
 * Also exercises retry policy integration:
 *   - ERR_FULL         -> RETRY_AFTER_TICK, retryable
 *   - ERR_INVALID_ARG  -> RETRY_NEVER, not retryable
 *
 * No Zephyr. No hardware. No timing dependencies.
 * Exit non-zero on any failure.
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
 * Static test events
 * -------------------------------------------------------------------------- */

static const robotos_event_t EV_NULL_PTR_SENTINEL = { /* only used for sizeof */ };

static const robotos_event_t EV_NONE = {
    .type = ROBOTOS_EVENT_NONE, .timestamp_tick = 0, .arg0 = 0x6F00u, .arg1 = 0
};

static const robotos_event_t EV_RESERVED_99 = {
    .type = (robotos_event_type_t)99, .timestamp_tick = 0, .arg0 = 0x6F00u, .arg1 = 0
};

static const robotos_event_t EV_USER = {
    .type = ROBOTOS_EVENT_USER, .timestamp_tick = 0, .arg0 = 0x6F00u, .arg1 = 0
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

/* --------------------------------------------------------------------------
 * main
 * -------------------------------------------------------------------------- */

int main(void)
{
    printf("=== Phase 6F: Mixed Event Policy Smoke Contract Tests ===\n\n");

    robotos_core_status_t         rc;
    robotos_core_snapshot_t       snap;
    robotos_core_retry_decision_t rd;

    /* ---- TC01: init -------------------------------------------------------- */
    printf("[ Init ]\n");

    rc = robotos_core_init();
    CHECK("TC01: init returns OK", rc == ROBOTOS_CORE_OK);
    CHECK("TC02: state == READY after init",
          robotos_core_state() == ROBOTOS_CORE_STATE_READY);

    /* ---- TC03-04: Invalid event paths ------------------------------------- */
    printf("\n[ Invalid events (admission gate) ]\n");

    rc = robotos_core_post_event(NULL);
    CHECK("TC03: post NULL -> ERR_NULL", rc == ROBOTOS_CORE_ERR_NULL);
    CHECK("TC04: rejected_count unchanged after NULL post",
          robotos_core_admission_rejected_count() == 0);
    CHECK("TC05: accepted_count unchanged after NULL post",
          robotos_core_admission_accepted_count() == 0);

    rc = robotos_core_post_event(&EV_NONE);
    CHECK("TC06: post NONE -> ERR_INVALID_ARG", rc == ROBOTOS_CORE_ERR_INVALID_ARG);
    CHECK("TC07: rejected_count == 1 after NONE post",
          robotos_core_admission_rejected_count() == 1);
    CHECK("TC08: accepted_count still 0 after NONE post",
          robotos_core_admission_accepted_count() == 0);
    CHECK("TC09: dropped_count still 0 (admission reject, not drop)",
          robotos_core_dropped_event_count() == 0);

    rc = robotos_core_post_event(&EV_RESERVED_99);
    CHECK("TC10: post reserved type 99 -> ERR_INVALID_ARG",
          rc == ROBOTOS_CORE_ERR_INVALID_ARG);
    CHECK("TC11: rejected_count == 2 after reserved type post",
          robotos_core_admission_rejected_count() == 2);
    CHECK("TC12: dropped_count still 0 (admission reject, not drop)",
          robotos_core_dropped_event_count() == 0);

    /* ---- TC13-16: Valid event path ---------------------------------------- */
    printf("\n[ Valid event (accepted) ]\n");

    rc = robotos_core_post_event(&EV_USER);
    CHECK("TC13: post USER -> OK", rc == ROBOTOS_CORE_OK);
    CHECK("TC14: accepted_count == 1 after USER post",
          robotos_core_admission_accepted_count() == 1);
    CHECK("TC15: pending == 1 after USER post",
          robotos_core_pending_event_count() == 1);
    CHECK("TC16: dropped_count still 0 after valid post",
          robotos_core_dropped_event_count() == 0);

    /* Fill to capacity: 1 event already in queue, need (capacity-1) more */
    for (uint32_t i = 1; i < ROBOTOS_EVENT_QUEUE_CAPACITY; i++) {
        robotos_core_post_event(&EV_USER);
    }
    CHECK("TC17: pending == capacity after fill",
          robotos_core_pending_event_count() == ROBOTOS_EVENT_QUEUE_CAPACITY);
    CHECK("TC18: accepted_count == capacity after fill",
          robotos_core_admission_accepted_count() == ROBOTOS_EVENT_QUEUE_CAPACITY);

    /* ---- TC19-22: Full-queue path ----------------------------------------- */
    printf("\n[ Full queue (drop path) ]\n");

    rc = robotos_core_post_event(&EV_USER);
    CHECK("TC19: post USER to full queue -> ERR_FULL", rc == ROBOTOS_CORE_ERR_FULL);
    CHECK("TC20: dropped_count == 1 after overflow",
          robotos_core_dropped_event_count() == 1);
    CHECK("TC21: pending unchanged == capacity after overflow",
          robotos_core_pending_event_count() == ROBOTOS_EVENT_QUEUE_CAPACITY);
    CHECK("TC22: accepted_count unchanged after overflow (not admitted)",
          robotos_core_admission_accepted_count() == ROBOTOS_EVENT_QUEUE_CAPACITY);

    /* ---- TC23: Invalid-while-full path (admission gate before queue check) - */
    printf("\n[ Invalid event while queue full ]\n");

    rc = robotos_core_post_event(&EV_NONE);
    CHECK("TC23: post NONE while full -> ERR_INVALID_ARG (not ERR_FULL)",
          rc == ROBOTOS_CORE_ERR_INVALID_ARG);
    CHECK("TC24: rejected_count == 3 (admission checked before queue)",
          robotos_core_admission_rejected_count() == 3);
    CHECK("TC25: dropped_count still 1 (NONE not queued, not dropped)",
          robotos_core_dropped_event_count() == 1);

    /* ---- TC26: Snapshot reflects all three paths in one run --------------- */
    printf("\n[ Snapshot: all three paths in one run ]\n");

    rc = robotos_core_snapshot(&snap);
    CHECK("TC26: snapshot returns OK", rc == ROBOTOS_CORE_OK);
    CHECK("TC27: snapshot.accepted == capacity",
          snap.admission_accepted_count == ROBOTOS_EVENT_QUEUE_CAPACITY);
    CHECK("TC28: snapshot.rejected == 3",
          snap.admission_rejected_count == 3u);
    CHECK("TC29: snapshot.dropped == 1",
          snap.dropped_event_count == 1u);
    CHECK("TC30: snapshot.pending == capacity",
          snap.pending_event_count == ROBOTOS_EVENT_QUEUE_CAPACITY);
    /* Getters must match snapshot */
    CHECK("TC31: accepted getter matches snapshot",
          robotos_core_admission_accepted_count() == snap.admission_accepted_count);
    CHECK("TC32: rejected getter matches snapshot",
          robotos_core_admission_rejected_count() == snap.admission_rejected_count);
    CHECK("TC33: dropped getter matches snapshot",
          robotos_core_dropped_event_count() == snap.dropped_event_count);
    /* All three paths exercised: nonzero accepted, nonzero rejected, nonzero dropped */
    CHECK("TC34: accepted > 0 (valid path exercised)",
          snap.admission_accepted_count > 0u);
    CHECK("TC35: rejected > 0 (invalid path exercised)",
          snap.admission_rejected_count > 0u);
    CHECK("TC36: dropped > 0 (full-queue path exercised)",
          snap.dropped_event_count > 0u);

    /* ---- TC37-38: Dispatch clears queue ----------------------------------- */
    printf("\n[ Dispatch ]\n");

    drain_queue();
    CHECK("TC37: pending == 0 after drain",
          robotos_core_pending_event_count() == 0);
    CHECK("TC38: dispatched_count == capacity after drain",
          robotos_core_dispatched_event_count() == ROBOTOS_EVENT_QUEUE_CAPACITY);

    /* ---- TC39-42: Retry policy alignment ---------------------------------- */
    printf("\n[ Retry policy alignment ]\n");

    memset(&rd, 0, sizeof(rd));
    rc = robotos_core_retry_decision_for_status(ROBOTOS_CORE_ERR_FULL, &rd);
    CHECK("TC39: retry_decision(ERR_FULL) returns OK", rc == ROBOTOS_CORE_OK);
    CHECK("TC40: ERR_FULL -> action == RETRY_AFTER_TICK",
          rd.action == ROBOTOS_CORE_RETRY_AFTER_TICK);
    CHECK("TC41: ERR_FULL is retryable",
          robotos_core_status_is_retryable(ROBOTOS_CORE_ERR_FULL));

    memset(&rd, 0, sizeof(rd));
    rc = robotos_core_retry_decision_for_status(ROBOTOS_CORE_ERR_INVALID_ARG, &rd);
    CHECK("TC42: retry_decision(ERR_INVALID_ARG) returns OK", rc == ROBOTOS_CORE_OK);
    CHECK("TC43: ERR_INVALID_ARG -> action == RETRY_NEVER",
          rd.action == ROBOTOS_CORE_RETRY_NEVER);
    CHECK("TC44: ERR_INVALID_ARG -> producer_should_drop == true",
          rd.producer_should_drop);
    CHECK("TC45: ERR_INVALID_ARG is not retryable",
          !robotos_core_status_is_retryable(ROBOTOS_CORE_ERR_INVALID_ARG));

    /* ---- Summary ---------------------------------------------------------- */
    printf("\n=== Phase 6F Results: %d passed, %d failed ===\n", s_pass, s_fail);
    return (s_fail > 0) ? 1 : 0;
}
