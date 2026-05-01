/*
 * test_robotos_core_handler_policy_contract.c
 * Phase 4H host contract tests for event handler registration policy.
 *
 * Tests register / unregister / routing / unhandled / error behavior.
 * Fresh executable = fresh static core state.
 * No Zephyr. No hardware. Exit non-zero on failure.
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

/* ---- Handler instrumentation --------------------------------------------- */

static int            g_handler_call_count;
static robotos_event_t g_last_event;
static void          *g_last_ctx;
static robotos_core_status_t g_handler_return;

static robotos_core_status_t instrumented_handler(const robotos_event_t *e, void *ctx)
{
    g_handler_call_count++;
    if (e) g_last_event = *e;
    g_last_ctx = ctx;
    return g_handler_return;
}

static void reset_handler_state(void)
{
    g_handler_call_count = 0;
    memset(&g_last_event, 0, sizeof(g_last_event));
    g_last_ctx       = NULL;
    g_handler_return = ROBOTOS_CORE_OK;
}

/* Secondary handler for replacement tests */
static robotos_core_status_t alt_handler(const robotos_event_t *e, void *ctx)
{
    (void)e; (void)ctx;
    return ROBOTOS_CORE_OK;
}

static int ctx_sentinel = 0xC0DE;

/* Helpers */
static robotos_event_t make_ev(robotos_event_type_t type, uint32_t a0)
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
    printf("=== robotos_core handler policy contract tests (Phase 4H) ===\n\n");

    robotos_event_t         ev;
    robotos_core_snapshot_t snap;
    robotos_core_status_t   ret;

    reset_handler_state();

    /* ---- Pre-init state ------------------------------------------------- */
    printf("[ Pre-init guards ]\n");

    TC("TC01: registered_handler_count == 0 before init",
        robotos_core_registered_handler_count() == 0);
    TC("TC02: unhandled_event_count == 0 before init",
        robotos_core_unhandled_event_count() == 0);
    TC("TC03: has_handler(USER) == false before init",
        robotos_core_has_event_handler(ROBOTOS_EVENT_USER) == false);
    TC("TC04: register before init returns ERR_INVALID_STATE",
        robotos_core_register_event_handler(ROBOTOS_EVENT_USER,
            instrumented_handler, NULL) == ROBOTOS_CORE_ERR_INVALID_STATE);
    TC("TC05: unregister before init returns ERR_INVALID_STATE",
        robotos_core_unregister_event_handler(ROBOTOS_EVENT_USER)
            == ROBOTOS_CORE_ERR_INVALID_STATE);

    /* ---- Init ----------------------------------------------------------- */
    robotos_core_init();
    printf("\n[ After init ]\n");

    TC("TC06: registered_handler_count == 0 after init",
        robotos_core_registered_handler_count() == 0);
    TC("TC07: unhandled_event_count == 0 after init",
        robotos_core_unhandled_event_count() == 0);

    /* ---- Register guards ------------------------------------------------ */
    printf("\n[ Register guards ]\n");

    TC("TC08: register NULL handler returns ERR_NULL",
        robotos_core_register_event_handler(ROBOTOS_EVENT_USER, NULL, NULL)
            == ROBOTOS_CORE_ERR_NULL);
    TC("TC09: register NONE type returns ERR_INVALID_ARG",
        robotos_core_register_event_handler(ROBOTOS_EVENT_NONE,
            instrumented_handler, NULL) == ROBOTOS_CORE_ERR_INVALID_ARG);

    /* ---- Valid registration --------------------------------------------- */
    printf("\n[ Valid registration ]\n");

    TC("TC10: register USER handler returns OK",
        robotos_core_register_event_handler(ROBOTOS_EVENT_USER,
            instrumented_handler, &ctx_sentinel) == ROBOTOS_CORE_OK);
    TC("TC11: has_handler(USER) == true",
        robotos_core_has_event_handler(ROBOTOS_EVENT_USER) == true);
    TC("TC12: registered_handler_count == 1",
        robotos_core_registered_handler_count() == 1);
    TC("TC13: has_handler(CORE_TICK) == false",
        robotos_core_has_event_handler(ROBOTOS_EVENT_CORE_TICK) == false);

    /* ---- Handler replacement -------------------------------------------- */
    printf("\n[ Replacement ]\n");

    TC("TC14: register USER again returns OK",
        robotos_core_register_event_handler(ROBOTOS_EVENT_USER,
            alt_handler, NULL) == ROBOTOS_CORE_OK);
    TC("TC15: count still 1 after replacement",
        robotos_core_registered_handler_count() == 1);
    TC("TC16: has_handler(USER) still true",
        robotos_core_has_event_handler(ROBOTOS_EVENT_USER) == true);

    /* Restore instrumented handler for further tests */
    robotos_core_register_event_handler(ROBOTOS_EVENT_USER,
        instrumented_handler, &ctx_sentinel);

    /* ---- Dispatch routes to handler ------------------------------------- */
    printf("\n[ Dispatch routing ]\n");

    ev = make_ev(ROBOTOS_EVENT_USER, 77);
    robotos_core_post_event(&ev);
    reset_handler_state();

    ret = robotos_core_tick();
    TC("TC17: tick() returns OK",
        ret == ROBOTOS_CORE_OK);
    TC("TC18: handler called exactly once",
        g_handler_call_count == 1);
    TC("TC19: handler received correct event type",
        g_last_event.type == ROBOTOS_EVENT_USER);
    TC("TC20: handler received correct arg0",
        g_last_event.arg0 == 77);
    TC("TC21: handler received registered user_context",
        g_last_ctx == (void *)&ctx_sentinel);
    TC("TC22: dispatched_count incremented",
        robotos_core_dispatched_event_count() == 1);
    TC("TC23: unhandled_event_count still 0 (event was handled)",
        robotos_core_unhandled_event_count() == 0);

    /* ---- Explicit dispatch routes same way ------------------------------ */
    printf("\n[ Explicit dispatch routing ]\n");

    ev = make_ev(ROBOTOS_EVENT_USER, 88);
    robotos_core_post_event(&ev);
    reset_handler_state();

    ret = robotos_core_dispatch_events(1);
    TC("TC24: dispatch_events(1) returns OK",
        ret == ROBOTOS_CORE_OK);
    TC("TC25: handler called once via explicit dispatch",
        g_handler_call_count == 1);
    TC("TC26: unhandled still 0",
        robotos_core_unhandled_event_count() == 0);

    /* ---- Unregistered event type → unhandled_count --------------------- */
    printf("\n[ Unregistered event type ]\n");

    uint32_t unhandled_before = robotos_core_unhandled_event_count();
    ev = make_ev(ROBOTOS_EVENT_CORE_TICK, 0); /* no handler for CORE_TICK */
    robotos_core_post_event(&ev);

    ret = robotos_core_tick();
    TC("TC27: tick with unregistered type returns OK",
        ret == ROBOTOS_CORE_OK);
    TC("TC28: unhandled_event_count incremented",
        robotos_core_unhandled_event_count() == unhandled_before + 1);
    TC("TC29: dispatch_count still incremented (event consumed by routing)",
        robotos_core_dispatched_event_count() == 3);

    /* Future type also counts as unhandled */
    ev = make_ev((robotos_event_type_t)999, 0);
    robotos_core_post_event(&ev);
    robotos_core_tick();
    TC("TC30: future/unknown type also increments unhandled_count",
        robotos_core_unhandled_event_count() == unhandled_before + 2);

    /* ---- Unregister ----------------------------------------------------- */
    printf("\n[ Unregister ]\n");

    TC("TC31: unregister NONE returns ERR_INVALID_ARG",
        robotos_core_unregister_event_handler(ROBOTOS_EVENT_NONE)
            == ROBOTOS_CORE_ERR_INVALID_ARG);

    TC("TC32: unregister USER returns OK",
        robotos_core_unregister_event_handler(ROBOTOS_EVENT_USER) == ROBOTOS_CORE_OK);
    TC("TC33: has_handler(USER) == false after unregister",
        robotos_core_has_event_handler(ROBOTOS_EVENT_USER) == false);
    TC("TC34: registered_handler_count == 0",
        robotos_core_registered_handler_count() == 0);

    TC("TC35: unregister USER again returns ERR_EMPTY",
        robotos_core_unregister_event_handler(ROBOTOS_EVENT_USER)
            == ROBOTOS_CORE_ERR_EMPTY);

    /* Post USER event after unregister → unhandled */
    uint32_t unhandled_after_unreg = robotos_core_unhandled_event_count();
    ev = make_ev(ROBOTOS_EVENT_USER, 0);
    robotos_core_post_event(&ev);
    ret = robotos_core_tick();
    TC("TC36: tick after unregister returns OK",
        ret == ROBOTOS_CORE_OK);
    TC("TC37: unhandled_event_count incremented after unregister",
        robotos_core_unhandled_event_count() == unhandled_after_unreg + 1);

    /* ---- Handler error path --------------------------------------------- */
    printf("\n[ Handler error ]\n");

    robotos_core_register_event_handler(ROBOTOS_EVENT_USER,
        instrumented_handler, NULL);
    g_handler_return = ROBOTOS_CORE_ERR_INVALID_STATE;

    ev = make_ev(ROBOTOS_EVENT_USER, 0);
    robotos_core_post_event(&ev);
    uint32_t errors_before = robotos_core_handler_error_count();
    reset_handler_state();
    g_handler_return = ROBOTOS_CORE_ERR_INVALID_STATE;

    ret = robotos_core_tick();
    TC("TC38: tick returns handler's non-OK status",
        ret == ROBOTOS_CORE_ERR_INVALID_STATE);
    TC("TC39: handler_error_count incremented",
        robotos_core_handler_error_count() == errors_before + 1);
    TC("TC40: core state remains READY",
        robotos_core_state() == ROBOTOS_CORE_STATE_READY);

    /* Reset to OK for further tests */
    g_handler_return = ROBOTOS_CORE_OK;

    /* ---- Table capacity ------------------------------------------------- */
    printf("\n[ Handler table capacity ]\n");

    /* Clear all slots first */
    robotos_core_unregister_event_handler(ROBOTOS_EVENT_USER);

    /* Register ROBOTOS_CORE_MAX_EVENT_HANDLERS distinct types */
    uint32_t filled = 0;
    robotos_event_type_t types[ROBOTOS_CORE_MAX_EVENT_HANDLERS];
    types[0] = ROBOTOS_EVENT_CORE_TICK;
    for (uint32_t i = 1; i < ROBOTOS_CORE_MAX_EVENT_HANDLERS; i++) {
        types[i] = (robotos_event_type_t)(ROBOTOS_EVENT_USER + i);
    }
    for (uint32_t i = 0; i < ROBOTOS_CORE_MAX_EVENT_HANDLERS; i++) {
        ret = robotos_core_register_event_handler(types[i], alt_handler, NULL);
        if (ret == ROBOTOS_CORE_OK) filled++;
    }
    TC("TC41: all MAX slots filled",
        filled == ROBOTOS_CORE_MAX_EVENT_HANDLERS &&
        robotos_core_registered_handler_count() == ROBOTOS_CORE_MAX_EVENT_HANDLERS);

    /* One more distinct type → ERR_FULL */
    ret = robotos_core_register_event_handler(
        (robotos_event_type_t)(ROBOTOS_EVENT_USER + ROBOTOS_CORE_MAX_EVENT_HANDLERS + 1),
        alt_handler, NULL);
    TC("TC42: register beyond capacity returns ERR_FULL",
        ret == ROBOTOS_CORE_ERR_FULL);
    TC("TC43: count unchanged after FULL",
        robotos_core_registered_handler_count() == ROBOTOS_CORE_MAX_EVENT_HANDLERS);

    /* Replacement at capacity succeeds */
    ret = robotos_core_register_event_handler(types[0], instrumented_handler, NULL);
    TC("TC44: replacement at full capacity returns OK",
        ret == ROBOTOS_CORE_OK);
    TC("TC45: count unchanged after replacement at capacity",
        robotos_core_registered_handler_count() == ROBOTOS_CORE_MAX_EVENT_HANDLERS);

    /* ---- Repeated init preserves handler table -------------------------- */
    printf("\n[ Repeated init preserves handlers ]\n");

    uint32_t count_before_reinit = robotos_core_registered_handler_count();
    robotos_core_init(); /* repeated init — should NOT clear table */
    TC("TC46: handler count preserved after re-init",
        robotos_core_registered_handler_count() == count_before_reinit);
    TC("TC47: has_handler(CORE_TICK) still true after re-init",
        robotos_core_has_event_handler(ROBOTOS_EVENT_CORE_TICK) == true);

    /* ---- Snapshot reflects handler state -------------------------------- */
    printf("\n[ Snapshot ]\n");

    robotos_core_snapshot(&snap);
    TC("TC48: snapshot.registered_handler_count == current count",
        snap.registered_handler_count == robotos_core_registered_handler_count());
    TC("TC49: snapshot.unhandled_event_count == current unhandled",
        snap.unhandled_event_count == robotos_core_unhandled_event_count());
    TC("TC50: snapshot.state == READY",
        snap.state == ROBOTOS_CORE_STATE_READY);

    /* ---- Summary -------------------------------------------------------- */
    printf("\n=== Results: %d passed, %d failed ===\n", g_pass, g_fail);

    return (g_fail > 0) ? 1 : 0;
}
