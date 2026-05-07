/*
 * test_robotos_timer_producer_contract.c
 *
 * Phase 6M -- Devkit Timer Producer Contract Tests
 *
 * The producer module is intentionally pure C (no Zephyr dependency) so
 * it can be host-validated end-to-end against the real core sources.
 * These tests exercise:
 *
 *   - pre-init guards: should_post / on_tick are no-ops before init
 *   - cadence predicate: should_post(tick) deterministic modulo PERIOD
 *   - single post path: on_tick(post-tick) increments counters and queue
 *   - cadence skip: on_tick(non-post-tick) is a strict no-op
 *   - multiple posts accumulate in attempted/ok and core admission
 *   - dispatch routes producer events to the producer handler with no
 *     handler error and no unhandled count growth
 *   - queue fill from producer alone exercises ERR_FULL and producer.dropped
 *   - producer.dropped equals core dropped_event_count (single producer)
 *   - drain restores the post-success path
 *   - type isolation: events of other types do NOT route to the producer
 *     handler (USER routes nowhere here, USER+1 routes to producer)
 *
 * No Zephyr. No hardware. No timing. Exit non-zero on any failure.
 */

#include "robotos_core.h"
#include "robotos_event_queue.h"
#include "devkit_timer_producer.h"

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/* ---- Test harness ---------------------------------------------------------- */

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

/* ---- Helpers --------------------------------------------------------------- */

static void drain_queue(void)
{
    while (robotos_core_pending_event_count() > 0u) {
        robotos_core_dispatch_events(ROBOTOS_EVENT_QUEUE_CAPACITY);
    }
}

/* ---- main ------------------------------------------------------------------ */

int main(void)
{
    printf("=== Phase 6M: Devkit Timer Producer Contract Tests ===\n\n");

    robotos_core_status_t         rc;
    devkit_timer_producer_stats_t st;

    /* ---- Section 1: Pre-init guards ----------------------------------------- */
    printf("[ Pre-init guards ]\n");

    CHECK("TC01: should_post(0) false before init",
          devkit_timer_producer_should_post(0u) == false);
    CHECK("TC02: should_post(2) false before init",
          devkit_timer_producer_should_post(2u) == false);
    CHECK("TC03: should_post(100) false before init",
          devkit_timer_producer_should_post(100u) == false);

    devkit_timer_producer_get_stats(&st);
    CHECK("TC04: stats.attempted == 0 before init", st.attempted == 0u);
    CHECK("TC05: stats.ok == 0 before init",        st.ok == 0u);
    CHECK("TC06: stats.dropped == 0 before init",   st.dropped == 0u);
    CHECK("TC07: stats.throttled == 0 before init", st.throttled == 0u);

    /* on_tick before init must be a strict no-op */
    devkit_timer_producer_on_tick(0u);
    devkit_timer_producer_on_tick(2u);
    devkit_timer_producer_get_stats(&st);
    CHECK("TC08: on_tick before init: attempted unchanged", st.attempted == 0u);
    CHECK("TC09: on_tick before init: no event queued",
          robotos_core_pending_event_count() == 0u);

    /* get_stats with NULL is a strict no-op (defensive) */
    devkit_timer_producer_get_stats(NULL);
    CHECK("TC10: get_stats(NULL) does not crash and is no-op",
          true /* survival is the assertion */);

    /* ---- Section 2: Init ---------------------------------------------------- */
    printf("\n[ Init ]\n");

    rc = robotos_core_init();
    CHECK("TC11: robotos_core_init returns OK", rc == ROBOTOS_CORE_OK);

    rc = devkit_timer_producer_init();
    CHECK("TC12: producer init returns OK", rc == ROBOTOS_CORE_OK);

    CHECK("TC13: handler registered for USER+1",
          robotos_core_has_event_handler(DEVKIT_TIMER_PRODUCER_TYPE) == true);
    CHECK("TC14: registered_handler_count == 1",
          robotos_core_registered_handler_count() == 1u);

    /* Re-init must be idempotent: still OK, count still 1 (replacement) */
    rc = devkit_timer_producer_init();
    CHECK("TC15: producer init idempotent (re-init returns OK)",
          rc == ROBOTOS_CORE_OK);
    CHECK("TC16: registered_handler_count still 1 after re-init",
          robotos_core_registered_handler_count() == 1u);

    /* ---- Section 3: Cadence predicate ---------------------------------------- */
    printf("\n[ Cadence predicate ]\n");

    CHECK("TC17: should_post(0)   true  (0 %% 2 == 0)",
          devkit_timer_producer_should_post(0u) == true);
    CHECK("TC18: should_post(1)   false (1 %% 2 != 0)",
          devkit_timer_producer_should_post(1u) == false);
    CHECK("TC19: should_post(2)   true",
          devkit_timer_producer_should_post(2u) == true);
    CHECK("TC20: should_post(3)   false",
          devkit_timer_producer_should_post(3u) == false);
    CHECK("TC21: should_post(100) true",
          devkit_timer_producer_should_post(100u) == true);
    CHECK("TC22: should_post(101) false",
          devkit_timer_producer_should_post(101u) == false);
    CHECK("TC23: should_post(UINT32_MAX) false (odd)",
          devkit_timer_producer_should_post(UINT32_MAX) == false);
    CHECK("TC24: should_post(UINT32_MAX-1) true (even)",
          devkit_timer_producer_should_post(UINT32_MAX - 1u) == true);

    /* ---- Section 4: First post ---------------------------------------------- */
    printf("\n[ First post ]\n");

    uint32_t accepted_before = robotos_core_admission_accepted_count();

    devkit_timer_producer_on_tick(2u);
    devkit_timer_producer_get_stats(&st);
    CHECK("TC25: on_tick(2): attempted == 1",   st.attempted == 1u);
    CHECK("TC26: on_tick(2): ok == 1",          st.ok == 1u);
    CHECK("TC27: on_tick(2): dropped == 0",     st.dropped == 0u);
    CHECK("TC28: pending == 1 (event queued)",
          robotos_core_pending_event_count() == 1u);
    CHECK("TC29: core accepted +1",
          robotos_core_admission_accepted_count() == accepted_before + 1u);

    /* ---- Section 5: Cadence skip is a strict no-op -------------------------- */
    printf("\n[ Cadence skip is a strict no-op ]\n");

    accepted_before = robotos_core_admission_accepted_count();
    uint32_t pending_before = robotos_core_pending_event_count();

    devkit_timer_producer_on_tick(3u);
    devkit_timer_producer_on_tick(5u);
    devkit_timer_producer_on_tick(7u);
    devkit_timer_producer_on_tick(99u);
    devkit_timer_producer_get_stats(&st);
    CHECK("TC30: attempted unchanged after odd ticks", st.attempted == 1u);
    CHECK("TC31: pending unchanged after odd ticks",
          robotos_core_pending_event_count() == pending_before);
    CHECK("TC32: core accepted unchanged after odd ticks",
          robotos_core_admission_accepted_count() == accepted_before);

    /* ---- Section 6: Multiple posts accumulate ------------------------------- */
    printf("\n[ Multiple posts accumulate ]\n");

    devkit_timer_producer_on_tick(4u);
    devkit_timer_producer_on_tick(6u);
    devkit_timer_producer_get_stats(&st);
    CHECK("TC33: attempted == 3 after two more posts", st.attempted == 3u);
    CHECK("TC34: ok == 3 (queue not full)", st.ok == 3u);
    CHECK("TC35: pending == 3 (no dispatch yet)",
          robotos_core_pending_event_count() == 3u);

    /* ---- Section 7: Dispatch routes to the producer handler ----------------- */
    printf("\n[ Dispatch routes correctly ]\n");

    uint32_t dispatched_before  = robotos_core_dispatched_event_count();
    uint32_t herr_before        = robotos_core_handler_error_count();
    uint32_t unhandled_before   = robotos_core_unhandled_event_count();

    rc = robotos_core_dispatch_events(3u);
    CHECK("TC36: dispatch_events(3) returns OK", rc == ROBOTOS_CORE_OK);
    CHECK("TC37: pending == 0 after drain",
          robotos_core_pending_event_count() == 0u);
    CHECK("TC38: dispatched_count +3",
          robotos_core_dispatched_event_count() == dispatched_before + 3u);
    CHECK("TC39: handler_error_count unchanged (marker valid)",
          robotos_core_handler_error_count() == herr_before);
    CHECK("TC40: unhandled_event_count unchanged (handler matched)",
          robotos_core_unhandled_event_count() == unhandled_before);

    /* ---- Section 8: Queue fill exercises ERR_FULL --------------------------- */
    printf("\n[ Queue fill from producer alone ]\n");

    /* Fire enough posts to exceed capacity. CAPACITY = 16; we already posted 3.
     * Burst CAPACITY + 5 more attempts at even ticks. Without dispatching,
     * the first CAPACITY of these will queue, the rest will hit ERR_FULL. */
    uint32_t prod_attempted_pre = st.attempted;
    uint32_t prod_dropped_pre   = st.dropped;
    uint32_t core_dropped_pre   = robotos_core_dropped_event_count();

    for (uint32_t k = 0u; k < ROBOTOS_EVENT_QUEUE_CAPACITY + 5u; k++) {
        devkit_timer_producer_on_tick(100u + 2u * k); /* always even */
    }
    devkit_timer_producer_get_stats(&st);

    CHECK("TC41: attempted advanced by CAPACITY+5",
          st.attempted == prod_attempted_pre + (ROBOTOS_EVENT_QUEUE_CAPACITY + 5u));
    CHECK("TC42: ok advanced by CAPACITY (queue filled exactly to cap)",
          st.ok - 3u == ROBOTOS_EVENT_QUEUE_CAPACITY);
    CHECK("TC43: dropped == 5 (CAPACITY+5 attempts vs CAPACITY queued)",
          st.dropped == prod_dropped_pre + 5u);
    CHECK("TC44: pending == CAPACITY (queue full)",
          robotos_core_pending_event_count() == ROBOTOS_EVENT_QUEUE_CAPACITY);
    CHECK("TC45: throttled == 0 (post_event path, not try_post_event)",
          st.throttled == 0u);
    CHECK("TC46: invalid == 0 (marker always valid)",
          st.invalid == 0u);
    CHECK("TC47: producer.dropped equals core dropped delta (single producer)",
          (robotos_core_dropped_event_count() - core_dropped_pre) ==
          (st.dropped - prod_dropped_pre));

    /* ---- Section 9: Drain restores post-success path ------------------------ */
    printf("\n[ Drain restores post-success ]\n");

    drain_queue();
    CHECK("TC48: pending == 0 after drain",
          robotos_core_pending_event_count() == 0u);

    uint32_t attempted_pre = st.attempted;
    uint32_t ok_pre        = st.ok;
    uint32_t dropped_pre   = st.dropped;

    devkit_timer_producer_on_tick(1000u); /* even -> post */
    devkit_timer_producer_get_stats(&st);
    CHECK("TC49: attempted +1 after drain",
          st.attempted == attempted_pre + 1u);
    CHECK("TC50: ok +1 after drain",
          st.ok == ok_pre + 1u);
    CHECK("TC51: dropped unchanged after drain post",
          st.dropped == dropped_pre);
    CHECK("TC52: pending == 1 again",
          robotos_core_pending_event_count() == 1u);

    /* ---- Section 10: Type isolation ----------------------------------------- */
    printf("\n[ Type isolation: USER does not route to producer handler ]\n");

    drain_queue();
    uint32_t unhandled_pre  = robotos_core_unhandled_event_count();
    uint32_t herr_pre       = robotos_core_handler_error_count();

    /* USER (type=100) has no handler in this test. Posting and dispatching
     * a USER event must increment unhandled_event_count, not handler_error_count,
     * and must not influence the producer's stats. */
    robotos_event_t user_ev = {
        .type           = ROBOTOS_EVENT_USER, /* 100 -- distinct from USER+1 */
        .timestamp_tick = 0u,
        .arg0           = 0xBEEFu,
        .arg1           = 0u,
    };
    rc = robotos_core_post_event(&user_ev);
    CHECK("TC53: post USER event returns OK (admission passes)",
          rc == ROBOTOS_CORE_OK);

    rc = robotos_core_dispatch_events(1u);
    CHECK("TC54: dispatch USER event returns OK", rc == ROBOTOS_CORE_OK);
    CHECK("TC55: USER event went unhandled (no handler for type=USER)",
          robotos_core_unhandled_event_count() == unhandled_pre + 1u);
    CHECK("TC56: handler_error_count unchanged (no handler matched)",
          robotos_core_handler_error_count() == herr_pre);

    /* Producer stats must be untouched by the USER post/dispatch */
    devkit_timer_producer_stats_t st_after_user;
    devkit_timer_producer_get_stats(&st_after_user);
    CHECK("TC57: producer.attempted unaffected by USER traffic",
          st_after_user.attempted == st.attempted);
    CHECK("TC58: producer.ok unaffected by USER traffic",
          st_after_user.ok == st.ok);
    CHECK("TC59: producer.dropped unaffected by USER traffic",
          st_after_user.dropped == st.dropped);

    /* ---- Summary ------------------------------------------------------------- */
    printf("\n=== Phase 6M Producer Results: %d passed, %d failed ===\n",
           s_pass, s_fail);
    return (s_fail > 0) ? 1 : 0;
}
