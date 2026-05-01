/*
 * test_robotos_event_queue_contract.c
 * Phase 4D host contract tests for robotos_event_queue.
 *
 * Tests the fixed-capacity ring buffer contract without Zephyr or hardware.
 * robotos_event_queue.c has no Zephyr dependency — no shim define needed.
 *
 * Build and run via:
 *   cmake -S RobotOS_v1.0/tests/host -B build-host-core
 *   cmake --build build-host-core
 *   ctest --test-dir build-host-core --output-on-failure
 *
 * Exit code 0 = all tests passed.
 * Exit code 1 = one or more tests failed.
 */

#include "robotos_event_queue.h"

#include <stdio.h>
#include <string.h>
#include <stdint.h>

/* ---- Minimal test harness (same style as Phase 4C) ----------------------- */

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

/* ---- Helper to make a test event ----------------------------------------- */
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
    printf("=== robotos_event_queue contract tests (Phase 4D) ===\n\n");

    robotos_event_queue_t q;
    robotos_event_t       ev;
    robotos_core_status_t ret;

    /* ---- Null-pointer safety for query functions ------------------------- */
    printf("[ NULL-safe queries ]\n");

    TC("TC01: is_initialized(NULL) == false",
        robotos_event_queue_is_initialized(NULL) == false);
    TC("TC02: is_empty(NULL) == true",
        robotos_event_queue_is_empty(NULL) == true);
    TC("TC03: is_full(NULL) == false",
        robotos_event_queue_is_full(NULL) == false);
    TC("TC04: count(NULL) == 0",
        robotos_event_queue_count(NULL) == 0);
    TC("TC05: capacity(NULL) == 0",
        robotos_event_queue_capacity(NULL) == 0);
    TC("TC06: dropped_count(NULL) == 0",
        robotos_event_queue_dropped_count(NULL) == 0);

    /* ---- Pre-init contract ----------------------------------------------- */
    printf("\n[ Pre-init contract ]\n");

    /* Zeroed struct is NOT initialized */
    memset(&q, 0, sizeof(q));
    TC("TC07: fresh zeroed queue is_initialized == false",
        robotos_event_queue_is_initialized(&q) == false);

    TC("TC08: init(NULL) returns ERR_NULL",
        robotos_event_queue_init(NULL) == ROBOTOS_CORE_ERR_NULL);

    ev = make_event(ROBOTOS_EVENT_CORE_TICK, 0, 0, 0);

    TC("TC09: push before init returns ERR_INVALID_STATE",
        robotos_event_queue_push(&q, &ev) == ROBOTOS_CORE_ERR_INVALID_STATE);
    TC("TC10: pop before init returns ERR_INVALID_STATE",
        robotos_event_queue_pop(&q, &ev) == ROBOTOS_CORE_ERR_INVALID_STATE);
    TC("TC11: peek before init returns ERR_INVALID_STATE",
        robotos_event_queue_peek(&q, &ev) == ROBOTOS_CORE_ERR_INVALID_STATE);
    TC("TC12: clear before init returns ERR_INVALID_STATE",
        robotos_event_queue_clear(&q) == ROBOTOS_CORE_ERR_INVALID_STATE);

    /* ---- Init ------------------------------------------------------------ */
    printf("\n[ Init ]\n");

    TC("TC13: init(valid) returns OK",
        robotos_event_queue_init(&q) == ROBOTOS_CORE_OK);
    TC("TC14: after init: is_initialized == true",
        robotos_event_queue_is_initialized(&q) == true);
    TC("TC15: after init: is_empty == true",
        robotos_event_queue_is_empty(&q) == true);
    TC("TC16: after init: is_full == false",
        robotos_event_queue_is_full(&q) == false);
    TC("TC17: after init: count == 0",
        robotos_event_queue_count(&q) == 0);
    TC("TC18: after init: capacity == 16",
        robotos_event_queue_capacity(&q) == 16u);
    TC("TC19: after init: dropped_count == 0",
        robotos_event_queue_dropped_count(&q) == 0);

    /* ---- Push null checks ------------------------------------------------ */
    printf("\n[ NULL guards ]\n");

    TC("TC20: push(NULL queue) returns ERR_NULL",
        robotos_event_queue_push(NULL, &ev) == ROBOTOS_CORE_ERR_NULL);
    TC("TC21: push(NULL event) returns ERR_NULL",
        robotos_event_queue_push(&q, NULL) == ROBOTOS_CORE_ERR_NULL);
    TC("TC22: pop(NULL out) returns ERR_NULL",
        robotos_event_queue_pop(&q, NULL) == ROBOTOS_CORE_ERR_NULL);
    TC("TC23: peek(NULL out) returns ERR_NULL",
        robotos_event_queue_peek(&q, NULL) == ROBOTOS_CORE_ERR_NULL);
    TC("TC24: clear(NULL) returns ERR_NULL",
        robotos_event_queue_clear(NULL) == ROBOTOS_CORE_ERR_NULL);

    /* ---- Single push/peek/pop ------------------------------------------- */
    printf("\n[ Single push / peek / pop ]\n");

    robotos_event_t e0 = make_event(ROBOTOS_EVENT_CORE_TICK, 1, 42, 99);

    TC("TC25: push one event returns OK",
        robotos_event_queue_push(&q, &e0) == ROBOTOS_CORE_OK);
    TC("TC26: count == 1 after push",
        robotos_event_queue_count(&q) == 1);
    TC("TC27: is_empty == false after push",
        robotos_event_queue_is_empty(&q) == false);

    ret = robotos_event_queue_peek(&q, &ev);
    TC("TC28: peek returns OK",
        ret == ROBOTOS_CORE_OK);
    TC("TC29: peek returns correct event type",
        ev.type == ROBOTOS_EVENT_CORE_TICK);
    TC("TC30: peek returns correct timestamp",
        ev.timestamp_tick == 1);
    TC("TC31: peek returns correct arg0",
        ev.arg0 == 42);
    TC("TC32: count still 1 after peek (not consumed)",
        robotos_event_queue_count(&q) == 1);

    ret = robotos_event_queue_pop(&q, &ev);
    TC("TC33: pop returns OK",
        ret == ROBOTOS_CORE_OK);
    TC("TC34: popped event matches pushed event",
        ev.type == ROBOTOS_EVENT_CORE_TICK &&
        ev.timestamp_tick == 1 && ev.arg0 == 42 && ev.arg1 == 99);
    TC("TC35: count == 0 after pop",
        robotos_event_queue_count(&q) == 0);
    TC("TC36: is_empty == true after pop",
        robotos_event_queue_is_empty(&q) == true);

    /* ---- FIFO order ------------------------------------------------------- */
    printf("\n[ FIFO order ]\n");

    robotos_event_t ea = make_event(ROBOTOS_EVENT_USER, 10, 1, 0);
    robotos_event_t eb = make_event(ROBOTOS_EVENT_USER, 11, 2, 0);
    robotos_event_t ec = make_event(ROBOTOS_EVENT_USER, 12, 3, 0);

    robotos_event_queue_push(&q, &ea);
    robotos_event_queue_push(&q, &eb);
    robotos_event_queue_push(&q, &ec);

    robotos_event_queue_pop(&q, &ev);
    TC("TC37: FIFO pop 1 — arg0 == 1",   ev.arg0 == 1);
    robotos_event_queue_pop(&q, &ev);
    TC("TC38: FIFO pop 2 — arg0 == 2",   ev.arg0 == 2);
    robotos_event_queue_pop(&q, &ev);
    TC("TC39: FIFO pop 3 — arg0 == 3",   ev.arg0 == 3);

    /* ---- Ring buffer wrap-around ----------------------------------------- */
    printf("\n[ Ring buffer wrap-around ]\n");

    /* Push 14 events, pop 14 — head and tail both at slot 14 */
    for (uint32_t i = 0; i < 14; i++) {
        robotos_event_t tmp = make_event(ROBOTOS_EVENT_USER, i, i, 0);
        robotos_event_queue_push(&q, &tmp);
    }
    for (uint32_t i = 0; i < 14; i++) {
        robotos_event_queue_pop(&q, &ev);
    }
    TC("TC40: after push14/pop14 queue is empty",
        robotos_event_queue_is_empty(&q));

    /* Push 5 more — tail wraps past end of buffer */
    for (uint32_t i = 0; i < 5; i++) {
        robotos_event_t tmp = make_event(ROBOTOS_EVENT_USER, 100 + i, 100 + i, 0);
        robotos_event_queue_push(&q, &tmp);
    }
    TC("TC41: count == 5 after wrap push",
        robotos_event_queue_count(&q) == 5);

    int wrap_ok = 1;
    for (uint32_t i = 0; i < 5; i++) {
        robotos_event_queue_pop(&q, &ev);
        if (ev.arg0 != 100u + i) {
            wrap_ok = 0;
        }
    }
    TC("TC42: wrap-around FIFO order preserved",
        wrap_ok);
    TC("TC43: count == 0 after draining wrapped events",
        robotos_event_queue_count(&q) == 0);

    /* ---- Fill to capacity / overflow ------------------------------------- */
    printf("\n[ Capacity / overflow / dropped ]\n");

    for (uint32_t i = 0; i < ROBOTOS_EVENT_QUEUE_CAPACITY; i++) {
        robotos_event_t tmp = make_event(ROBOTOS_EVENT_USER, i, i, 0);
        robotos_event_queue_push(&q, &tmp);
    }
    TC("TC44: after filling: is_full == true",
        robotos_event_queue_is_full(&q));
    TC("TC45: after filling: count == capacity",
        robotos_event_queue_count(&q) == ROBOTOS_EVENT_QUEUE_CAPACITY);

    robotos_event_t overflow_ev = make_event(ROBOTOS_EVENT_USER, 999, 999, 0);
    ret = robotos_event_queue_push(&q, &overflow_ev);
    TC("TC46: push when full returns ERR_FULL",
        ret == ROBOTOS_CORE_ERR_FULL);
    TC("TC47: dropped_count == 1 after overflow",
        robotos_event_queue_dropped_count(&q) == 1);
    TC("TC48: count unchanged after overflow push",
        robotos_event_queue_count(&q) == ROBOTOS_EVENT_QUEUE_CAPACITY);

    /* Pop all and verify first event still correct (not overwritten) */
    robotos_event_queue_pop(&q, &ev);
    TC("TC49: first popped event has arg0==0 (overflow did not overwrite)",
        ev.arg0 == 0);

    /* Drain remaining */
    for (uint32_t i = 1; i < ROBOTOS_EVENT_QUEUE_CAPACITY; i++) {
        robotos_event_queue_pop(&q, &ev);
    }
    TC("TC50: queue empty after draining all",
        robotos_event_queue_is_empty(&q));

    /* ---- Pop/peek empty -------------------------------------------------- */
    printf("\n[ Empty queue behavior ]\n");

    TC("TC51: pop empty returns ERR_EMPTY",
        robotos_event_queue_pop(&q, &ev) == ROBOTOS_CORE_ERR_EMPTY);
    TC("TC52: peek empty returns ERR_EMPTY",
        robotos_event_queue_peek(&q, &ev) == ROBOTOS_CORE_ERR_EMPTY);

    /* ---- Clear ----------------------------------------------------------- */
    printf("\n[ Clear ]\n");

    robotos_event_t ex = make_event(ROBOTOS_EVENT_USER, 0, 77, 0);
    robotos_event_queue_push(&q, &ex);
    robotos_event_queue_push(&q, &ex);
    TC("TC53: count == 2 before clear",
        robotos_event_queue_count(&q) == 2);

    uint32_t dropped_before = robotos_event_queue_dropped_count(&q);
    ret = robotos_event_queue_clear(&q);
    TC("TC54: clear returns OK",
        ret == ROBOTOS_CORE_OK);
    TC("TC55: count == 0 after clear",
        robotos_event_queue_count(&q) == 0);
    TC("TC56: is_empty == true after clear",
        robotos_event_queue_is_empty(&q) == true);
    TC("TC57: dropped_count preserved after clear",
        robotos_event_queue_dropped_count(&q) == dropped_before);
    TC("TC58: still initialized after clear",
        robotos_event_queue_is_initialized(&q) == true);

    /* ---- Summary -------------------------------------------------------- */
    printf("\n=== Results: %d passed, %d failed ===\n", g_pass, g_fail);

    return (g_fail > 0) ? 1 : 0;
}
