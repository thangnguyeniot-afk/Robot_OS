/*
 * test_robotos_event_dispatcher_contract.c
 * Phase 4E host contract tests for robotos_event_dispatcher.
 *
 * Tests the dispatcher contract without Zephyr or hardware.
 * robotos_event_dispatcher.c has zero Zephyr dependency.
 *
 * Build and run via:
 *   cmake -S RobotOS_v1.0/tests/host -B build-host-core
 *   cmake --build build-host-core
 *   ctest --test-dir build-host-core --output-on-failure
 *
 * Exit code 0 = all tests passed.
 * Exit code 1 = one or more tests failed.
 */

#include "robotos_event_dispatcher.h"

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

/* ---- Handler instrumentation --------------------------------------------- */

static int            g_handler_call_count;
static robotos_event_t g_last_event;
static void          *g_last_ctx;
static robotos_core_status_t g_handler_return; /* what handler should return */

static robotos_core_status_t instrumented_handler(const robotos_event_t *event,
                                                    void *ctx)
{
    g_handler_call_count++;
    if (event) g_last_event = *event;
    g_last_ctx = ctx;
    return g_handler_return;
}

static void reset_handler(void)
{
    g_handler_call_count = 0;
    memset(&g_last_event, 0, sizeof(g_last_event));
    g_last_ctx           = NULL;
    g_handler_return     = ROBOTOS_CORE_OK;
}

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

static int ctx_sentinel = 0xBEEF;

/* -------------------------------------------------------------------------- */

int main(void)
{
    printf("=== robotos_event_dispatcher contract tests (Phase 4E) ===\n\n");

    robotos_event_queue_t      q;
    robotos_event_dispatcher_t d;
    robotos_event_queue_t      uninit_q; /* deliberately NOT initialized */
    robotos_core_status_t      ret;
    robotos_event_t            ev;

    memset(&uninit_q, 0, sizeof(uninit_q)); /* zeroed = not initialized */

    /* ---- NULL / pre-init guards ----------------------------------------- */
    printf("[ NULL and pre-init guards ]\n");

    TC("TC01: init(NULL dispatcher) returns ERR_NULL",
        robotos_event_dispatcher_init(NULL, &uninit_q, instrumented_handler, NULL)
            == ROBOTOS_CORE_ERR_NULL);

    TC("TC02: init with NULL queue returns ERR_NULL",
        robotos_event_dispatcher_init(&d, NULL, instrumented_handler, NULL)
            == ROBOTOS_CORE_ERR_NULL);

    TC("TC03: init with NULL handler returns ERR_NULL",
        robotos_event_dispatcher_init(&d, &uninit_q, NULL, NULL)
            == ROBOTOS_CORE_ERR_NULL);

    TC("TC04: init with uninitialized queue returns ERR_INVALID_STATE",
        robotos_event_dispatcher_init(&d, &uninit_q, instrumented_handler, NULL)
            == ROBOTOS_CORE_ERR_INVALID_STATE);

    /* NULL-safe queries before init */
    TC("TC05: is_initialized(NULL) == false",
        robotos_event_dispatcher_is_initialized(NULL) == false);
    TC("TC06: dispatched_count(NULL) == 0",
        robotos_event_dispatcher_dispatched_count(NULL) == 0);
    TC("TC07: handler_error_count(NULL) == 0",
        robotos_event_dispatcher_handler_error_count(NULL) == 0);

    /* dispatch before init (zeroed struct is not initialized) */
    memset(&d, 0, sizeof(d));
    TC("TC08: dispatch_one(NULL) returns ERR_NULL",
        robotos_event_dispatcher_dispatch_one(NULL) == ROBOTOS_CORE_ERR_NULL);
    TC("TC09: dispatch_one before init returns ERR_INVALID_STATE",
        robotos_event_dispatcher_dispatch_one(&d) == ROBOTOS_CORE_ERR_INVALID_STATE);
    TC("TC10: dispatch_all(NULL) returns ERR_NULL",
        robotos_event_dispatcher_dispatch_all(NULL, 4) == ROBOTOS_CORE_ERR_NULL);
    TC("TC11: dispatch_all before init returns ERR_INVALID_STATE",
        robotos_event_dispatcher_dispatch_all(&d, 4) == ROBOTOS_CORE_ERR_INVALID_STATE);

    /* ---- Valid init ------------------------------------------------------- */
    printf("\n[ Valid init ]\n");

    robotos_event_queue_init(&q);
    reset_handler();

    ret = robotos_event_dispatcher_init(&d, &q, instrumented_handler, &ctx_sentinel);
    TC("TC12: valid init returns OK",
        ret == ROBOTOS_CORE_OK);
    TC("TC13: after init: is_initialized == true",
        robotos_event_dispatcher_is_initialized(&d) == true);
    TC("TC14: after init: dispatched_count == 0",
        robotos_event_dispatcher_dispatched_count(&d) == 0);
    TC("TC15: after init: handler_error_count == 0",
        robotos_event_dispatcher_handler_error_count(&d) == 0);

    /* ---- dispatch_one on empty queue ------------------------------------- */
    printf("\n[ dispatch_one — empty queue ]\n");

    TC("TC16: dispatch_one on empty queue returns ERR_EMPTY",
        robotos_event_dispatcher_dispatch_one(&d) == ROBOTOS_CORE_ERR_EMPTY);
    TC("TC17: handler not called when queue empty",
        g_handler_call_count == 0);

    /* ---- dispatch_one with one event ------------------------------------- */
    printf("\n[ dispatch_one — one event ]\n");

    ev = make_event(ROBOTOS_EVENT_CORE_TICK, 42, 11, 22);
    robotos_event_queue_push(&q, &ev);
    reset_handler();

    ret = robotos_event_dispatcher_dispatch_one(&d);
    TC("TC18: dispatch_one with event returns OK",
        ret == ROBOTOS_CORE_OK);
    TC("TC19: handler called exactly once",
        g_handler_call_count == 1);
    TC("TC20: handler received correct event type",
        g_last_event.type == ROBOTOS_EVENT_CORE_TICK);
    TC("TC21: handler received correct timestamp",
        g_last_event.timestamp_tick == 42);
    TC("TC22: handler received correct arg0",
        g_last_event.arg0 == 11);
    TC("TC23: handler received correct arg1",
        g_last_event.arg1 == 22);
    TC("TC24: user_context passed correctly",
        g_last_ctx == (void *)&ctx_sentinel);
    TC("TC25: event consumed from queue",
        robotos_event_queue_count(&q) == 0);
    TC("TC26: dispatched_count == 1",
        robotos_event_dispatcher_dispatched_count(&d) == 1);
    TC("TC27: handler_error_count unchanged == 0",
        robotos_event_dispatcher_handler_error_count(&d) == 0);

    /* ---- FIFO order across dispatch_one ---------------------------------- */
    printf("\n[ FIFO order via dispatch_one ]\n");

    for (uint32_t i = 0; i < 3; i++) {
        robotos_event_t tmp = make_event(ROBOTOS_EVENT_USER, i, i, 0);
        robotos_event_queue_push(&q, &tmp);
    }

    uint32_t fifo_ok = 1;
    for (uint32_t i = 0; i < 3; i++) {
        reset_handler();
        robotos_event_dispatcher_dispatch_one(&d);
        if (g_last_event.arg0 != i) fifo_ok = 0;
    }
    TC("TC28: FIFO order preserved across 3 dispatch_one calls",
        fifo_ok == 1);
    TC("TC29: dispatched_count == 4 after 4 total successes",
        robotos_event_dispatcher_dispatched_count(&d) == 4);

    /* ---- dispatch_all: max_events=0 -------------------------------------- */
    printf("\n[ dispatch_all — edge cases ]\n");

    TC("TC30: dispatch_all with max_events=0 returns OK",
        robotos_event_dispatcher_dispatch_all(&d, 0) == ROBOTOS_CORE_OK);

    /* ---- dispatch_all: empty queue --------------------------------------- */
    TC("TC31: dispatch_all with max_events>0 on empty queue returns ERR_EMPTY",
        robotos_event_dispatcher_dispatch_all(&d, 4) == ROBOTOS_CORE_ERR_EMPTY);

    /* ---- dispatch_all: fewer events than max ----------------------------- */
    printf("\n[ dispatch_all — normal dispatch ]\n");

    for (uint32_t i = 0; i < 3; i++) {
        robotos_event_t tmp = make_event(ROBOTOS_EVENT_USER, i, i, 0);
        robotos_event_queue_push(&q, &tmp);
    }
    reset_handler();
    uint32_t dc_before = robotos_event_dispatcher_dispatched_count(&d);

    ret = robotos_event_dispatcher_dispatch_all(&d, 10); /* max > available */
    TC("TC32: dispatch_all drains fewer than max, returns OK",
        ret == ROBOTOS_CORE_OK);
    TC("TC33: dispatch_all dispatched exactly 3 events",
        robotos_event_dispatcher_dispatched_count(&d) == dc_before + 3);
    TC("TC34: queue empty after dispatch_all",
        robotos_event_queue_is_empty(&q));

    /* ---- dispatch_all: exactly max events -------------------------------- */
    for (uint32_t i = 0; i < 6; i++) {
        robotos_event_t tmp = make_event(ROBOTOS_EVENT_USER, i, i, 0);
        robotos_event_queue_push(&q, &tmp);
    }
    dc_before = robotos_event_dispatcher_dispatched_count(&d);

    ret = robotos_event_dispatcher_dispatch_all(&d, 4); /* max < available */
    TC("TC35: dispatch_all stops at max_events",
        ret == ROBOTOS_CORE_OK);
    TC("TC36: exactly max_events dispatched",
        robotos_event_dispatcher_dispatched_count(&d) == dc_before + 4);
    TC("TC37: 2 events remain in queue after limited dispatch_all",
        robotos_event_queue_count(&q) == 2);

    robotos_event_queue_clear(&q);

    /* ---- Handler error behavior ----------------------------------------- */
    printf("\n[ Handler error behavior ]\n");

    /* Push 3 events; second will trigger error */
    robotos_event_t ea = make_event(ROBOTOS_EVENT_USER, 1, 100, 0);
    robotos_event_t eb = make_event(ROBOTOS_EVENT_USER, 2, 200, 0); /* will fail */
    robotos_event_t ec = make_event(ROBOTOS_EVENT_USER, 3, 300, 0);
    robotos_event_queue_push(&q, &ea);
    robotos_event_queue_push(&q, &eb);
    robotos_event_queue_push(&q, &ec);

    /* First dispatch succeeds */
    g_handler_return = ROBOTOS_CORE_OK;
    robotos_event_dispatcher_dispatch_one(&d);
    uint32_t ec_before = robotos_event_dispatcher_handler_error_count(&d);

    /* Second dispatch: handler returns error */
    g_handler_return = ROBOTOS_CORE_ERR_INVALID_STATE;
    ret = robotos_event_dispatcher_dispatch_one(&d);
    TC("TC38: dispatch_one returns handler's non-OK status",
        ret == ROBOTOS_CORE_ERR_INVALID_STATE);
    TC("TC39: handler_error_count incremented",
        robotos_event_dispatcher_handler_error_count(&d) == ec_before + 1);
    TC("TC40: failing event was consumed (ec still in queue, count=1)",
        robotos_event_queue_count(&q) == 1);

    /* Third event still in queue, can be dispatched after error */
    g_handler_return = ROBOTOS_CORE_OK;
    reset_handler();
    robotos_event_dispatcher_dispatch_one(&d);
    TC("TC41: event after error-event still dispatched correctly",
        g_last_event.arg0 == 300);

    /* ---- dispatch_all stops on handler error ----------------------------- */
    robotos_event_queue_clear(&q);
    for (uint32_t i = 0; i < 4; i++) {
        robotos_event_t tmp = make_event(ROBOTOS_EVENT_USER, i, i, 0);
        robotos_event_queue_push(&q, &tmp);
    }

    /* Handler succeeds for first two, fails on third */
    static uint32_t g_call_num;
    g_call_num = 0;

    /* Use a lambda-style approach via g_handler_return toggling */
    /* Set up: succeed for first 2, fail on 3rd by pre-loading events */
    dc_before  = robotos_event_dispatcher_dispatched_count(&d);
    ec_before  = robotos_event_dispatcher_handler_error_count(&d);

    /* Temporarily swap to a counting handler to control per-call behavior */
    /* Simple approach: just test with dispatch_all, error on first call */
    robotos_event_queue_clear(&q);
    for (uint32_t i = 0; i < 3; i++) {
        robotos_event_t tmp = make_event(ROBOTOS_EVENT_USER, i, i, 0);
        robotos_event_queue_push(&q, &tmp);
    }
    g_handler_return = ROBOTOS_CORE_ERR_INVALID_STATE; /* fail all */
    dc_before  = robotos_event_dispatcher_dispatched_count(&d);
    ec_before  = robotos_event_dispatcher_handler_error_count(&d);

    ret = robotos_event_dispatcher_dispatch_all(&d, 10);
    TC("TC42: dispatch_all stops on first handler error",
        ret == ROBOTOS_CORE_ERR_INVALID_STATE);
    TC("TC43: dispatched_count unchanged (first call failed)",
        robotos_event_dispatcher_dispatched_count(&d) == dc_before);
    TC("TC44: handler_error_count incremented by 1",
        robotos_event_dispatcher_handler_error_count(&d) == ec_before + 1);
    TC("TC45: 2 remaining events still in queue",
        robotos_event_queue_count(&q) == 2);

    /* ---- Re-init resets counters ----------------------------------------- */
    printf("\n[ Re-init ]\n");

    int ctx2 = 0xCAFE;
    robotos_event_queue_init(&q); /* fresh queue */
    g_handler_return = ROBOTOS_CORE_OK;

    ret = robotos_event_dispatcher_init(&d, &q, instrumented_handler, &ctx2);
    TC("TC46: re-init returns OK",
        ret == ROBOTOS_CORE_OK);
    TC("TC47: dispatched_count reset to 0",
        robotos_event_dispatcher_dispatched_count(&d) == 0);
    TC("TC48: handler_error_count reset to 0",
        robotos_event_dispatcher_handler_error_count(&d) == 0);

    /* Verify new context is used */
    ev = make_event(ROBOTOS_EVENT_CORE_TICK, 0, 0, 0);
    robotos_event_queue_push(&q, &ev);
    reset_handler();
    robotos_event_dispatcher_dispatch_one(&d);
    TC("TC49: re-init context is passed to handler",
        g_last_ctx == (void *)&ctx2);

    /* ---- queue dropped_count unaffected by dispatch ---------------------- */
    printf("\n[ Queue state after dispatch ]\n");

    robotos_event_queue_init(&q);
    robotos_event_dispatcher_init(&d, &q, instrumented_handler, NULL);
    g_handler_return = ROBOTOS_CORE_OK;

    /* Fill queue, overflow to create dropped events */
    for (uint32_t i = 0; i < ROBOTOS_EVENT_QUEUE_CAPACITY + 2; i++) {
        robotos_event_t tmp = make_event(ROBOTOS_EVENT_USER, i, i, 0);
        robotos_event_queue_push(&q, &tmp);
    }
    uint32_t dropped_before = robotos_event_queue_dropped_count(&q);
    TC("TC50: dropped_count > 0 after overflow",
        dropped_before == 2);

    robotos_event_dispatcher_dispatch_all(&d, 4);
    TC("TC51: queue dropped_count unchanged after dispatch",
        robotos_event_queue_dropped_count(&q) == dropped_before);

    /* ---- dispatch after clear returns ERR_EMPTY -------------------------- */
    printf("\n[ Dispatch after clear ]\n");

    robotos_event_queue_clear(&q);
    TC("TC52: dispatch_one after clear returns ERR_EMPTY",
        robotos_event_dispatcher_dispatch_one(&d) == ROBOTOS_CORE_ERR_EMPTY);

    /* ---- Multiple event types dispatched correctly ----------------------- */
    printf("\n[ Multiple event types ]\n");

    robotos_event_t en = make_event(ROBOTOS_EVENT_NONE, 0, 0, 0);
    robotos_event_t et = make_event(ROBOTOS_EVENT_CORE_TICK, 1, 0, 0);
    robotos_event_t eu = make_event(ROBOTOS_EVENT_USER, 2, 42, 0);
    robotos_event_queue_push(&q, &en);
    robotos_event_queue_push(&q, &et);
    robotos_event_queue_push(&q, &eu);

    reset_handler();
    robotos_event_dispatcher_dispatch_one(&d);
    TC("TC53: NONE event dispatched correctly",
        g_last_event.type == ROBOTOS_EVENT_NONE);
    reset_handler();
    robotos_event_dispatcher_dispatch_one(&d);
    TC("TC54: CORE_TICK event dispatched correctly",
        g_last_event.type == ROBOTOS_EVENT_CORE_TICK);
    reset_handler();
    robotos_event_dispatcher_dispatch_one(&d);
    TC("TC55: USER event dispatched with correct arg0",
        g_last_event.type == ROBOTOS_EVENT_USER && g_last_event.arg0 == 42);

    /* ---- Summary -------------------------------------------------------- */
    printf("\n=== Results: %d passed, %d failed ===\n", g_pass, g_fail);

    return (g_fail > 0) ? 1 : 0;
}
