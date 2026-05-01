/*
 * test_robotos_core_ingestion_contract.c
 * Phase 4F host contract tests for core event ingestion API.
 *
 * Tests post_event / dispatch_events / counter introspection.
 * Runs as a separate executable to get fresh static core state.
 * No Zephyr. No hardware. Exit non-zero on failure.
 *
 * Build and run via:
 *   cmake -S RobotOS_v1.0/tests/host -B build-host-core
 *   cmake --build build-host-core
 *   ctest --test-dir build-host-core --output-on-failure
 */

#include "robotos_core.h"
#include "robotos_event_queue.h"  /* for ROBOTOS_EVENT_QUEUE_CAPACITY */

#include <stdio.h>
#include <string.h>
#include <stdint.h>

/* ---- Test harness -------------------------------------------------------- */

static int g_pass;
static int g_fail;

#define TC(name, cond) do { \
    if (cond) { \
        printf("  PASS  " name "\n"); \
        g_pass++; \
    } else { \
        printf("  FAIL  " name "\n"); \
        g_fail++; \
    } \
} while (0)

/* ---- Helpers ------------------------------------------------------------- */

static robotos_event_t make_event(robotos_event_type_t type, uint32_t tick,
                                   uint32_t a0, uint32_t a1)
{
    robotos_event_t e;
    e.type           = type;
    e.timestamp_tick = tick;
    e.arg0           = a0;
    e.arg1           = a1;
    return e;
}

/* -------------------------------------------------------------------------- */

int main(void)
{
    printf("=== robotos_core ingestion contract tests (Phase 4F) ===\n\n");

    robotos_event_t         ev;
    robotos_core_snapshot_t snap;
    robotos_core_status_t   ret;

    /* ---- Pre-init behavior ---------------------------------------------- */
    printf("[ Pre-init ]\n");

    TC("TC01: pending_event_count() == 0 before init",
        robotos_core_pending_event_count() == 0);
    TC("TC02: dropped_event_count() == 0 before init",
        robotos_core_dropped_event_count() == 0);
    TC("TC03: dispatched_event_count() == 0 before init",
        robotos_core_dispatched_event_count() == 0);
    TC("TC04: handler_error_count() == 0 before init",
        robotos_core_handler_error_count() == 0);

    ev = make_event(ROBOTOS_EVENT_USER, 0, 0, 0);
    TC("TC05: post_event(NULL) returns ERR_NULL",
        robotos_core_post_event(NULL) == ROBOTOS_CORE_ERR_NULL);
    TC("TC06: post_event(valid) before init returns ERR_INVALID_STATE",
        robotos_core_post_event(&ev) == ROBOTOS_CORE_ERR_INVALID_STATE);
    TC("TC07: dispatch_events(0) before init returns ERR_INVALID_STATE",
        robotos_core_dispatch_events(0) == ROBOTOS_CORE_ERR_INVALID_STATE);
    TC("TC08: dispatch_events(1) before init returns ERR_INVALID_STATE",
        robotos_core_dispatch_events(1) == ROBOTOS_CORE_ERR_INVALID_STATE);

    /* ---- Snapshot before init ------------------------------------------- */
    ret = robotos_core_snapshot(&snap);
    TC("TC09: snapshot before init returns OK",
        ret == ROBOTOS_CORE_OK);
    TC("TC10: snapshot.state == UNINITIALIZED",
        snap.state == ROBOTOS_CORE_STATE_UNINITIALIZED);
    TC("TC11: snapshot.pending_event_count == 0",
        snap.pending_event_count == 0);
    TC("TC12: snapshot.dropped_event_count == 0",
        snap.dropped_event_count == 0);
    TC("TC13: snapshot.dispatched_event_count == 0",
        snap.dispatched_event_count == 0);
    TC("TC14: snapshot.handler_error_count == 0",
        snap.handler_error_count == 0);

    /* ---- Init and post-init counters ------------------------------------ */
    printf("\n[ After init ]\n");

    TC("TC15: init() returns OK",
        robotos_core_init() == ROBOTOS_CORE_OK);
    TC("TC16: state == READY",
        robotos_core_state() == ROBOTOS_CORE_STATE_READY);
    TC("TC17: pending == 0 after init",
        robotos_core_pending_event_count() == 0);
    TC("TC18: dropped == 0 after init",
        robotos_core_dropped_event_count() == 0);
    TC("TC19: dispatched == 0 after init",
        robotos_core_dispatched_event_count() == 0);
    TC("TC20: handler_errors == 0 after init",
        robotos_core_handler_error_count() == 0);

    /* ---- post_event NULL after init ------------------------------------- */
    TC("TC21: post_event(NULL) after init returns ERR_NULL",
        robotos_core_post_event(NULL) == ROBOTOS_CORE_ERR_NULL);
    TC("TC22: pending unchanged after NULL post",
        robotos_core_pending_event_count() == 0);

    /* ---- Post one event ------------------------------------------------- */
    printf("\n[ Post and dispatch ]\n");

    ev = make_event(ROBOTOS_EVENT_USER, 1, 42, 0);
    TC("TC23: post one USER event returns OK",
        robotos_core_post_event(&ev) == ROBOTOS_CORE_OK);
    TC("TC24: pending == 1 after one post",
        robotos_core_pending_event_count() == 1);
    TC("TC25: dropped unchanged == 0",
        robotos_core_dropped_event_count() == 0);
    TC("TC26: dispatched unchanged == 0",
        robotos_core_dispatched_event_count() == 0);

    /* ---- dispatch_events(0): nothing dispatched ------------------------- */
    ret = robotos_core_dispatch_events(0);
    TC("TC27: dispatch_events(0) returns OK",
        ret == ROBOTOS_CORE_OK);
    TC("TC28: pending unchanged == 1 after dispatch(0)",
        robotos_core_pending_event_count() == 1);
    TC("TC29: dispatched unchanged == 0 after dispatch(0)",
        robotos_core_dispatched_event_count() == 0);

    /* ---- dispatch_events(1): consumes one event ------------------------- */
    ret = robotos_core_dispatch_events(1);
    TC("TC30: dispatch_events(1) returns OK",
        ret == ROBOTOS_CORE_OK);
    TC("TC31: pending == 0 after dispatch(1)",
        robotos_core_pending_event_count() == 0);
    TC("TC32: dispatched == 1 after dispatch(1)",
        robotos_core_dispatched_event_count() == 1);
    TC("TC33: handler_errors unchanged == 0",
        robotos_core_handler_error_count() == 0);

    /* ---- dispatch_events when empty ------------------------------------- */
    ret = robotos_core_dispatch_events(1);
    TC("TC34: dispatch_events on empty queue returns ERR_EMPTY",
        ret == ROBOTOS_CORE_ERR_EMPTY);
    TC("TC35: dispatched unchanged == 1",
        robotos_core_dispatched_event_count() == 1);

    /* ---- Post 3 events, dispatch 2 then rest ---------------------------- */
    printf("\n[ Multiple post/dispatch ]\n");

    for (uint32_t i = 0; i < 3; i++) {
        robotos_event_t tmp = make_event(ROBOTOS_EVENT_USER, i, i, 0);
        robotos_core_post_event(&tmp);
    }
    TC("TC36: pending == 3 after 3 posts",
        robotos_core_pending_event_count() == 3);

    robotos_core_dispatch_events(2);
    TC("TC37: pending == 1 after dispatch(2)",
        robotos_core_pending_event_count() == 1);
    TC("TC38: dispatched == 3 after dispatch(2)",
        robotos_core_dispatched_event_count() == 3);

    robotos_core_dispatch_events(10); /* drains the last 1 */
    TC("TC39: pending == 0 after dispatch remaining",
        robotos_core_pending_event_count() == 0);
    TC("TC40: dispatched == 4 total",
        robotos_core_dispatched_event_count() == 4);

    /* ---- Full queue behavior -------------------------------------------- */
    printf("\n[ Full queue ]\n");

    for (uint32_t i = 0; i < ROBOTOS_EVENT_QUEUE_CAPACITY; i++) {
        robotos_event_t tmp = make_event(ROBOTOS_EVENT_USER, i, i, 0);
        robotos_core_post_event(&tmp);
    }
    TC("TC41: pending == capacity after filling",
        robotos_core_pending_event_count() == ROBOTOS_EVENT_QUEUE_CAPACITY);

    ev = make_event(ROBOTOS_EVENT_USER, 99, 99, 0);
    ret = robotos_core_post_event(&ev);
    TC("TC42: post to full queue returns ERR_FULL",
        ret == ROBOTOS_CORE_ERR_FULL);
    TC("TC43: dropped == 1 after overflow",
        robotos_core_dropped_event_count() == 1);
    TC("TC44: pending unchanged == capacity after overflow",
        robotos_core_pending_event_count() == ROBOTOS_EVENT_QUEUE_CAPACITY);

    /* Drain all capacity events */
    ret = robotos_core_dispatch_events(ROBOTOS_EVENT_QUEUE_CAPACITY);
    TC("TC45: dispatch_events(capacity) returns OK",
        ret == ROBOTOS_CORE_OK);
    TC("TC46: pending == 0 after draining all",
        robotos_core_pending_event_count() == 0);
    uint32_t dispatched_after_fill = robotos_core_dispatched_event_count();
    TC("TC47: dispatched increased by capacity",
        dispatched_after_fill == 4 + ROBOTOS_EVENT_QUEUE_CAPACITY);

    /* ---- Repeated init preserves queued events -------------------------- */
    printf("\n[ Repeated init preserves queue ]\n");

    ev = make_event(ROBOTOS_EVENT_CORE_TICK, 0, 7, 0);
    robotos_core_post_event(&ev);
    TC("TC48: pending == 1 before re-init",
        robotos_core_pending_event_count() == 1);

    robotos_core_init(); /* second init — should not drop queued events */
    TC("TC49: pending still == 1 after re-init",
        robotos_core_pending_event_count() == 1);

    ret = robotos_core_dispatch_events(1);
    TC("TC50: dispatch after re-init returns OK",
        ret == ROBOTOS_CORE_OK);
    TC("TC51: pending == 0 after dispatching",
        robotos_core_pending_event_count() == 0);

    /* ---- Snapshot after events ----------------------------------------- */
    printf("\n[ Snapshot after events ]\n");

    ev = make_event(ROBOTOS_EVENT_USER, 0, 0, 0);
    robotos_core_post_event(&ev);
    robotos_core_post_event(&ev);
    robotos_core_dispatch_events(1);

    ret = robotos_core_snapshot(&snap);
    TC("TC52: snapshot returns OK",
        ret == ROBOTOS_CORE_OK);
    TC("TC53: snapshot.state == READY",
        snap.state == ROBOTOS_CORE_STATE_READY);
    TC("TC54: snapshot.pending_event_count == 1",
        snap.pending_event_count == 1);
    TC("TC55: snapshot.dropped_event_count == 1 (from earlier overflow)",
        snap.dropped_event_count == 1);

    /* dispatched_event_count and handler_error_count verified via individual getters */
    TC("TC56: handler_error_count == 0 throughout",
        robotos_core_handler_error_count() == 0);
    TC("TC57: snapshot.version == \"4B-contract\"",
        snap.version != NULL && strcmp(snap.version, "4B-contract") == 0);

    /* ---- Summary -------------------------------------------------------- */
    printf("\n=== Results: %d passed, %d failed ===\n", g_pass, g_fail);

    return (g_fail > 0) ? 1 : 0;
}
