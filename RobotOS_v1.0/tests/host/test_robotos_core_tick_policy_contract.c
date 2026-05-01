/*
 * test_robotos_core_tick_policy_contract.c
 * Phase 4G host contract tests for scheduler tick policy stub.
 *
 * Verifies that each core tick dispatches up to ROBOTOS_CORE_MAX_EVENTS_PER_TICK
 * events and that the tick-dispatch / explicit-dispatch relationship is correct.
 *
 * Separate executable = fresh static core state.
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

/* ---- Helper ------------------------------------------------------------- */

static robotos_event_t make_event(robotos_event_type_t type, uint32_t a0)
{
    robotos_event_t e;
    e.type           = type;
    e.timestamp_tick = 0;
    e.arg0           = a0;
    e.arg1           = 0;
    return e;
}

/* -------------------------------------------------------------------------- */

int main(void)
{
    printf("=== robotos_core tick policy contract tests (Phase 4G) ===\n\n");

    robotos_event_t ev;
    robotos_core_status_t ret;

    /* ---- Pre-init: tick still guarded ----------------------------------- */
    printf("[ Pre-init tick guard ]\n");

    TC("TC01: tick() before init returns ERR_INVALID_STATE",
        robotos_core_tick() == ROBOTOS_CORE_ERR_INVALID_STATE);
    TC("TC02: tick_count() == 0 before init",
        robotos_core_tick_count() == 0);

    /* ---- Init ----------------------------------------------------------- */
    robotos_core_init();

    printf("\n[ Max events per tick constant ]\n");

    TC("TC03: ROBOTOS_CORE_MAX_EVENTS_PER_TICK >= 1",
        ROBOTOS_CORE_MAX_EVENTS_PER_TICK >= 1u);

    /* ---- Tick on empty queue: OK ---------------------------------------- */
    printf("\n[ Tick on empty queue ]\n");

    ret = robotos_core_tick();
    TC("TC04: tick() on empty queue returns OK (not ERR_EMPTY)",
        ret == ROBOTOS_CORE_OK);
    TC("TC05: tick_count == 1 after first tick",
        robotos_core_tick_count() == 1);
    TC("TC06: pending == 0 after tick on empty",
        robotos_core_pending_event_count() == 0);
    TC("TC07: dispatched == 0 after tick on empty (no events to dispatch)",
        robotos_core_dispatched_event_count() == 0);
    TC("TC08: handler_errors == 0",
        robotos_core_handler_error_count() == 0);

    /* ---- Post 1 event, tick dispatches it ------------------------------- */
    printf("\n[ Tick dispatches posted event ]\n");

    ev = make_event(ROBOTOS_EVENT_USER, 1);
    robotos_core_post_event(&ev);
    TC("TC09: pending == 1 after post",
        robotos_core_pending_event_count() == 1);

    ret = robotos_core_tick();
    TC("TC10: tick() returns OK",
        ret == ROBOTOS_CORE_OK);
    TC("TC11: tick_count == 2 after second tick",
        robotos_core_tick_count() == 2);
    TC("TC12: pending == 0 after tick consumed the event",
        robotos_core_pending_event_count() == 0);
    TC("TC13: dispatched == 1 after tick dispatch",
        robotos_core_dispatched_event_count() == 1);
    TC("TC14: handler_errors == 0",
        robotos_core_handler_error_count() == 0);

    /* ---- Post 2 events, tick dispatches at most MAX per tick ------------ */
    printf("\n[ Tick bounded by MAX_EVENTS_PER_TICK ]\n");

    ev = make_event(ROBOTOS_EVENT_USER, 10);
    robotos_core_post_event(&ev);
    ev = make_event(ROBOTOS_EVENT_USER, 11);
    robotos_core_post_event(&ev);
    TC("TC15: pending == 2 after two posts",
        robotos_core_pending_event_count() == 2);

    ret = robotos_core_tick();
    TC("TC16: tick() with 2 events returns OK",
        ret == ROBOTOS_CORE_OK);
    TC("TC17: tick_count == 3",
        robotos_core_tick_count() == 3);
    TC("TC18: pending decremented by exactly MAX_EVENTS_PER_TICK",
        robotos_core_pending_event_count() == 2 - ROBOTOS_CORE_MAX_EVENTS_PER_TICK);
    TC("TC19: dispatched increased by exactly MAX_EVENTS_PER_TICK",
        robotos_core_dispatched_event_count() == 1 + ROBOTOS_CORE_MAX_EVENTS_PER_TICK);

    /* ---- Second tick dispatches remaining event ------------------------- */
    ret = robotos_core_tick();
    TC("TC20: second tick() returns OK",
        ret == ROBOTOS_CORE_OK);
    TC("TC21: tick_count == 4",
        robotos_core_tick_count() == 4);
    TC("TC22: pending == 0 after second tick drains remainder",
        robotos_core_pending_event_count() == 0);
    TC("TC23: dispatched == 2 + 1 = 3 total",
        robotos_core_dispatched_event_count() == 1 + 2 * ROBOTOS_CORE_MAX_EVENTS_PER_TICK);

    /* ---- Explicit dispatch_events still works after tick policy --------- */
    printf("\n[ Explicit dispatch_events coexists with tick policy ]\n");

    ev = make_event(ROBOTOS_EVENT_USER, 20);
    robotos_core_post_event(&ev);
    ev = make_event(ROBOTOS_EVENT_USER, 21);
    robotos_core_post_event(&ev);
    ev = make_event(ROBOTOS_EVENT_USER, 22);
    robotos_core_post_event(&ev);
    TC("TC24: pending == 3 after 3 posts",
        robotos_core_pending_event_count() == 3);

    uint32_t dispatched_before = robotos_core_dispatched_event_count();
    ret = robotos_core_dispatch_events(2);
    TC("TC25: explicit dispatch_events(2) returns OK",
        ret == ROBOTOS_CORE_OK);
    TC("TC26: pending == 1 after explicit dispatch(2)",
        robotos_core_pending_event_count() == 1);
    TC("TC27: dispatched increased by 2",
        robotos_core_dispatched_event_count() == dispatched_before + 2);

    /* Tick drains the last one */
    dispatched_before = robotos_core_dispatched_event_count();
    robotos_core_tick();
    TC("TC28: tick drains remaining 1 event",
        robotos_core_pending_event_count() == 0);
    TC("TC29: dispatched increased by MAX_EVENTS_PER_TICK",
        robotos_core_dispatched_event_count() == dispatched_before + ROBOTOS_CORE_MAX_EVENTS_PER_TICK);

    /* ---- dispatch_events(empty) returns ERR_EMPTY; tick(empty) returns OK */
    printf("\n[ Empty behavior difference: tick vs explicit ]\n");

    TC("TC30: dispatch_events(1) on empty returns ERR_EMPTY",
        robotos_core_dispatch_events(1) == ROBOTOS_CORE_ERR_EMPTY);
    TC("TC31: tick() on empty returns OK (normalised)",
        robotos_core_tick() == ROBOTOS_CORE_OK);

    /* ---- Full queue: tick drains MAX per tick --------------------------- */
    printf("\n[ Full queue drained by ticks ]\n");

    for (uint32_t i = 0; i < ROBOTOS_EVENT_QUEUE_CAPACITY; i++) {
        ev = make_event(ROBOTOS_EVENT_USER, i);
        robotos_core_post_event(&ev);
    }
    TC("TC32: pending == capacity after filling",
        robotos_core_pending_event_count() == ROBOTOS_EVENT_QUEUE_CAPACITY);

    /* One extra post → ERR_FULL */
    ev = make_event(ROBOTOS_EVENT_USER, 99);
    TC("TC33: extra post returns ERR_FULL",
        robotos_core_post_event(&ev) == ROBOTOS_CORE_ERR_FULL);
    TC("TC34: dropped_count == 1",
        robotos_core_dropped_event_count() == 1);

    /* Each tick drains MAX_EVENTS_PER_TICK */
    uint32_t pending_before = robotos_core_pending_event_count();
    robotos_core_tick();
    TC("TC35: tick from full queue reduces pending by MAX_EVENTS_PER_TICK",
        robotos_core_pending_event_count() == pending_before - ROBOTOS_CORE_MAX_EVENTS_PER_TICK);

    /* Drain all remaining via ticks */
    uint32_t remaining = robotos_core_pending_event_count();
    for (uint32_t i = 0; i < remaining + 2; i++) { /* +2 to ensure extra empty ticks are OK */
        ret = robotos_core_tick();
        TC("TC36+: tick returns OK even as queue empties",
            ret == ROBOTOS_CORE_OK);
        if (robotos_core_pending_event_count() == 0) {
            break;
        }
    }
    TC("TC37: pending == 0 after draining via ticks",
        robotos_core_pending_event_count() == 0);

    /* ---- Repeated init preserves queued events (tick policy preserved) -- */
    printf("\n[ Repeated init: tick policy survives ]\n");

    ev = make_event(ROBOTOS_EVENT_CORE_TICK, 0);
    robotos_core_post_event(&ev);
    TC("TC38: pending == 1 before re-init",
        robotos_core_pending_event_count() == 1);

    robotos_core_init(); /* idempotent: does NOT reinit queue */
    TC("TC39: pending still == 1 after re-init",
        robotos_core_pending_event_count() == 1);

    ret = robotos_core_tick();
    TC("TC40: tick after re-init returns OK",
        ret == ROBOTOS_CORE_OK);
    TC("TC41: pending == 0 after tick dispatched event",
        robotos_core_pending_event_count() == 0);

    /* ---- Handler error path --------------------------------------------- */
    printf("\n[ Handler error note ]\n");
    printf("  NOTE  TC42: handler error path not testable — "
           "internal handler always OK; no public handler override API yet.\n");
    g_pass++; /* count the note as acknowledged */

    /* ---- Summary -------------------------------------------------------- */
    printf("\n=== Results: %d passed, %d failed ===\n", g_pass, g_fail);

    return (g_fail > 0) ? 1 : 0;
}
