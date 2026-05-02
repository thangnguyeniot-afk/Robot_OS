/*
 * test_robotos_scheduler_admission_contract.c
 * Phase 4I — Scheduler Admission Policy Contract Tests
 *
 * Validates admission gate in robotos_core_post_event():
 *   - NONE type rejected
 *   - Reserved range (2–99) rejected
 *   - CORE_TICK accepted
 *   - USER and USER+ accepted
 *   - Rejection increments admission_rejected_count
 *   - Acceptance increments admission_accepted_count
 *   - Queue-full still increments rejected (post-gate drop NOT counted as admitted)
 *   - Snapshot reflects admission counters
 *   - Getters match snapshot
 *
 * Compiled with CORE_SRCS + PLATFORM_HOST_STUB — no Zephyr, no hardware.
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

static const robotos_event_t EV_NONE = {
    .type = ROBOTOS_EVENT_NONE, .timestamp_tick = 0, .arg0 = 0, .arg1 = 0
};

static const robotos_event_t EV_CORE_TICK = {
    .type = ROBOTOS_EVENT_CORE_TICK, .timestamp_tick = 0, .arg0 = 0, .arg1 = 0
};

static const robotos_event_t EV_USER = {
    .type = ROBOTOS_EVENT_USER, .timestamp_tick = 0, .arg0 = 0, .arg1 = 0
};

static const robotos_event_t EV_USER_PLUS = {
    .type = (robotos_event_type_t)(ROBOTOS_EVENT_USER + 1),
    .timestamp_tick = 0, .arg0 = 0, .arg1 = 0
};

static const robotos_event_t EV_RESERVED_2 = {
    .type = (robotos_event_type_t)2, .timestamp_tick = 0, .arg0 = 0, .arg1 = 0
};

static const robotos_event_t EV_RESERVED_50 = {
    .type = (robotos_event_type_t)50, .timestamp_tick = 0, .arg0 = 0, .arg1 = 0
};

static const robotos_event_t EV_RESERVED_99 = {
    .type = (robotos_event_type_t)99, .timestamp_tick = 0, .arg0 = 0, .arg1 = 0
};

/* -------------------------------------------------------------------------- */

int main(void)
{
    printf("=== Phase 4I: Scheduler Admission Policy Contract Tests ===\n\n");

    robotos_core_status_t rc;
    robotos_core_snapshot_t snap;

    /* --- Pre-init baseline ------------------------------------------------ */
    CHECK("pre-init: admission_accepted_count == 0",
          robotos_core_admission_accepted_count() == 0u);
    CHECK("pre-init: admission_rejected_count == 0",
          robotos_core_admission_rejected_count() == 0u);

    /* post before init should return INVALID_STATE, not INVALID_ARG */
    rc = robotos_core_post_event(&EV_USER);
    CHECK("pre-init: post returns ERR_INVALID_STATE",
          rc == ROBOTOS_CORE_ERR_INVALID_STATE);
    CHECK("pre-init: admission counts unchanged after pre-init post",
          robotos_core_admission_accepted_count() == 0u &&
          robotos_core_admission_rejected_count() == 0u);

    /* --- Init ------------------------------------------------------------- */
    rc = robotos_core_init();
    CHECK("init returns OK", rc == ROBOTOS_CORE_OK);
    CHECK("post-init: admission_accepted == 0",
          robotos_core_admission_accepted_count() == 0u);
    CHECK("post-init: admission_rejected == 0",
          robotos_core_admission_rejected_count() == 0u);

    /* --- NULL event ------------------------------------------------------- */
    rc = robotos_core_post_event(NULL);
    CHECK("post NULL returns ERR_NULL", rc == ROBOTOS_CORE_ERR_NULL);
    CHECK("NULL: admission counts unchanged",
          robotos_core_admission_accepted_count() == 0u &&
          robotos_core_admission_rejected_count() == 0u);

    /* --- NONE type rejected ----------------------------------------------- */
    rc = robotos_core_post_event(&EV_NONE);
    CHECK("post NONE returns ERR_INVALID_ARG", rc == ROBOTOS_CORE_ERR_INVALID_ARG);
    CHECK("NONE: rejected_count == 1",  robotos_core_admission_rejected_count() == 1u);
    CHECK("NONE: accepted_count == 0",  robotos_core_admission_accepted_count() == 0u);

    /* --- Reserved range rejected ------------------------------------------ */
    rc = robotos_core_post_event(&EV_RESERVED_2);
    CHECK("post reserved(2) returns ERR_INVALID_ARG", rc == ROBOTOS_CORE_ERR_INVALID_ARG);
    CHECK("reserved(2): rejected_count == 2", robotos_core_admission_rejected_count() == 2u);

    rc = robotos_core_post_event(&EV_RESERVED_50);
    CHECK("post reserved(50) returns ERR_INVALID_ARG", rc == ROBOTOS_CORE_ERR_INVALID_ARG);
    CHECK("reserved(50): rejected_count == 3", robotos_core_admission_rejected_count() == 3u);

    rc = robotos_core_post_event(&EV_RESERVED_99);
    CHECK("post reserved(99) returns ERR_INVALID_ARG", rc == ROBOTOS_CORE_ERR_INVALID_ARG);
    CHECK("reserved(99): rejected_count == 4", robotos_core_admission_rejected_count() == 4u);
    CHECK("after reserved posts: accepted_count still 0",
          robotos_core_admission_accepted_count() == 0u);

    /* --- CORE_TICK accepted ------------------------------------------------ */
    rc = robotos_core_post_event(&EV_CORE_TICK);
    CHECK("post CORE_TICK returns OK", rc == ROBOTOS_CORE_OK);
    CHECK("CORE_TICK: accepted_count == 1", robotos_core_admission_accepted_count() == 1u);
    CHECK("CORE_TICK: rejected_count unchanged (4)",
          robotos_core_admission_rejected_count() == 4u);

    /* Drain the queue so it doesn't fill up mid-test */
    robotos_core_dispatch_events(1u);

    /* --- USER accepted ----------------------------------------------------- */
    rc = robotos_core_post_event(&EV_USER);
    CHECK("post USER returns OK", rc == ROBOTOS_CORE_OK);
    CHECK("USER: accepted_count == 2", robotos_core_admission_accepted_count() == 2u);

    robotos_core_dispatch_events(1u);

    /* --- USER+1 accepted --------------------------------------------------- */
    rc = robotos_core_post_event(&EV_USER_PLUS);
    CHECK("post USER+1 returns OK", rc == ROBOTOS_CORE_OK);
    CHECK("USER+1: accepted_count == 3", robotos_core_admission_accepted_count() == 3u);

    robotos_core_dispatch_events(1u);

    /* --- Queue-full: accepted event blocked by full queue ------------------ */
    /* Fill the queue to capacity */
    uint32_t filled = 0u;
    for (uint32_t i = 0; i < ROBOTOS_EVENT_QUEUE_CAPACITY; i++) {
        rc = robotos_core_post_event(&EV_USER);
        if (rc == ROBOTOS_CORE_OK) {
            filled++;
        }
    }
    CHECK("queue filled to capacity", filled == ROBOTOS_EVENT_QUEUE_CAPACITY);

    uint32_t accepted_before_full = robotos_core_admission_accepted_count();

    /* One more USER-type event — passes admission gate but queue is full */
    rc = robotos_core_post_event(&EV_USER);
    CHECK("post USER when full returns ERR_FULL", rc == ROBOTOS_CORE_ERR_FULL);
    CHECK("queue-full: accepted_count NOT incremented",
          robotos_core_admission_accepted_count() == accepted_before_full);
    CHECK("queue-full: rejected_count NOT incremented (queue-full is not admission reject)",
          robotos_core_admission_rejected_count() == 4u);

    /* NONE still rejected even when queue is full */
    rc = robotos_core_post_event(&EV_NONE);
    CHECK("post NONE when full still returns ERR_INVALID_ARG",
          rc == ROBOTOS_CORE_ERR_INVALID_ARG);
    CHECK("NONE when full: rejected_count == 5",
          robotos_core_admission_rejected_count() == 5u);

    /* Drain queue */
    for (uint32_t i = 0; i < ROBOTOS_EVENT_QUEUE_CAPACITY; i++) {
        robotos_core_dispatch_events(1u);
    }

    /* --- Second init does NOT reset admission counters -------------------- */
    uint32_t accepted_snap = robotos_core_admission_accepted_count();
    uint32_t rejected_snap = robotos_core_admission_rejected_count();

    rc = robotos_core_init();
    CHECK("second init returns OK", rc == ROBOTOS_CORE_OK);
    CHECK("second init: accepted_count NOT reset",
          robotos_core_admission_accepted_count() == accepted_snap);
    CHECK("second init: rejected_count NOT reset",
          robotos_core_admission_rejected_count() == rejected_snap);

    /* --- Snapshot reflects admission counters ----------------------------- */
    rc = robotos_core_snapshot(&snap);
    CHECK("snapshot returns OK", rc == ROBOTOS_CORE_OK);
    CHECK("snapshot: admission_accepted_count matches getter",
          snap.admission_accepted_count == robotos_core_admission_accepted_count());
    CHECK("snapshot: admission_rejected_count matches getter",
          snap.admission_rejected_count == robotos_core_admission_rejected_count());

    /* --- Final summary ----------------------------------------------------- */
    printf("\n--- Admission Policy Contract: %d passed, %d failed ---\n",
           s_pass, s_fail);
    return (s_fail == 0) ? 0 : 1;
}
