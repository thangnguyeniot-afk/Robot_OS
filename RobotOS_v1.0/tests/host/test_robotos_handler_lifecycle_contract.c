/*
 * test_robotos_handler_lifecycle_contract.c
 *
 * Phase 6J-B -- Handler Lifecycle Edge Cases
 *
 * Validates handler table slot management and lifecycle operations NOT
 * covered by the Phase 4H handler policy contract tests. Phase 4H covers:
 *   pre-init guards, register guards, basic routing, basic replacement,
 *   basic unregister, handler error, table capacity + overflow,
 *   repeated-init handler preservation, and snapshot reflection.
 *
 * This file extends coverage with:
 *   - Slot reuse: freed slot is reused by a new type after unregister
 *   - Full table + unregister + new type: slot becomes available
 *   - Unregister-all: all subsequent events become unhandled
 *   - Churn stability: N register/unregister cycles, count remains consistent
 *   - Handler error does not deregister: handler stays registered after error return
 *   - Replacement routes exclusively to the new handler function
 *   - Re-register after unregister dispatches correctly
 *   - Snapshot coherence across lifecycle operations
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

/* ---- Instrumented handlers ------------------------------------------------- */

static uint32_t s_call_a;
static uint32_t s_call_b;
static uint32_t s_call_err;

static robotos_core_status_t handler_a(const robotos_event_t *ev, void *ctx)
{
    (void)ev; (void)ctx;
    s_call_a++;
    return ROBOTOS_CORE_OK;
}

static robotos_core_status_t handler_b(const robotos_event_t *ev, void *ctx)
{
    (void)ev; (void)ctx;
    s_call_b++;
    return ROBOTOS_CORE_OK;
}

static robotos_core_status_t handler_err(const robotos_event_t *ev, void *ctx)
{
    (void)ev; (void)ctx;
    s_call_err++;
    return ROBOTOS_CORE_ERR_INVALID_STATE;
}

static void reset_call_counts(void)
{
    s_call_a   = 0;
    s_call_b   = 0;
    s_call_err = 0;
}

/* ---- Helpers --------------------------------------------------------------- */

static robotos_event_t make_ev(robotos_event_type_t type)
{
    robotos_event_t e;
    e.type           = type;
    e.timestamp_tick = 0;
    e.arg0           = 0;
    e.arg1           = 0;
    return e;
}

static void drain_queue(void)
{
    while (robotos_core_pending_event_count() > 0) {
        robotos_core_dispatch_events(ROBOTOS_EVENT_QUEUE_CAPACITY);
    }
}

/* ---- main ------------------------------------------------------------------ */

int main(void)
{
    printf("=== Phase 6J-B: Handler Lifecycle Edge Cases ===\n\n");

    robotos_core_status_t rc;
    robotos_event_t       ev;

    rc = robotos_core_init();
    CHECK("TC01: init returns OK", rc == ROBOTOS_CORE_OK);

    /* ---- Section 1: Slot reuse after unregister ----------------------------- */
    printf("\n[ Slot reuse after unregister ]\n");

    rc = robotos_core_register_event_handler(ROBOTOS_EVENT_USER, handler_a, NULL);
    CHECK("TC02: register USER returns OK", rc == ROBOTOS_CORE_OK);
    CHECK("TC03: count == 1", robotos_core_registered_handler_count() == 1u);

    rc = robotos_core_unregister_event_handler(ROBOTOS_EVENT_USER);
    CHECK("TC04: unregister USER returns OK", rc == ROBOTOS_CORE_OK);
    CHECK("TC05: count == 0 after unregister", robotos_core_registered_handler_count() == 0u);

    /* Register a different type — freed slot must be reusable */
    robotos_event_type_t new_type = (robotos_event_type_t)(ROBOTOS_EVENT_USER + 50u);
    rc = robotos_core_register_event_handler(new_type, handler_a, NULL);
    CHECK("TC06: register new type after unregister returns OK (slot reused)",
          rc == ROBOTOS_CORE_OK);
    CHECK("TC07: count == 1 after new type registration",
          robotos_core_registered_handler_count() == 1u);

    /* Confirm the new type dispatches to handler_a */
    reset_call_counts();
    ev = make_ev(new_type);
    robotos_core_post_event(&ev);
    robotos_core_dispatch_events(1u);
    CHECK("TC08: handler_a dispatched via new type (slot reuse confirmed)",
          s_call_a == 1u);

    robotos_core_unregister_event_handler(new_type);
    CHECK("TC09: count == 0 after unregister new_type",
          robotos_core_registered_handler_count() == 0u);

    /* ---- Section 2: Full table + unregister + new type ---------------------- */
    printf("\n[ Full table: unregister one slot, then register new type ]\n");

    robotos_event_type_t slot_types[ROBOTOS_CORE_MAX_EVENT_HANDLERS];
    slot_types[0] = ROBOTOS_EVENT_CORE_TICK;
    for (uint32_t i = 1u; i < ROBOTOS_CORE_MAX_EVENT_HANDLERS; i++) {
        slot_types[i] = (robotos_event_type_t)(ROBOTOS_EVENT_USER + i);
    }
    for (uint32_t i = 0u; i < ROBOTOS_CORE_MAX_EVENT_HANDLERS; i++) {
        robotos_core_register_event_handler(slot_types[i], handler_a, NULL);
    }
    CHECK("TC10: all slots filled (count == max)",
          robotos_core_registered_handler_count() == ROBOTOS_CORE_MAX_EVENT_HANDLERS);

    /* New type must fail: table full */
    robotos_event_type_t overflow_type = (robotos_event_type_t)(ROBOTOS_EVENT_USER + 100u);
    rc = robotos_core_register_event_handler(overflow_type, handler_a, NULL);
    CHECK("TC11: register beyond capacity returns ERR_FULL", rc == ROBOTOS_CORE_ERR_FULL);
    CHECK("TC12: count unchanged after overflow attempt",
          robotos_core_registered_handler_count() == ROBOTOS_CORE_MAX_EVENT_HANDLERS);

    /* Unregister one slot, then register the new type */
    rc = robotos_core_unregister_event_handler(slot_types[0]);
    CHECK("TC13: unregister one slot returns OK", rc == ROBOTOS_CORE_OK);
    CHECK("TC14: count == max-1 after one unregister",
          robotos_core_registered_handler_count() == ROBOTOS_CORE_MAX_EVENT_HANDLERS - 1u);

    rc = robotos_core_register_event_handler(overflow_type, handler_b, NULL);
    CHECK("TC15: register new type succeeds after freeing one slot",
          rc == ROBOTOS_CORE_OK);
    CHECK("TC16: count back to max",
          robotos_core_registered_handler_count() == ROBOTOS_CORE_MAX_EVENT_HANDLERS);

    /* Confirm dispatch routes correctly to the newly registered type */
    reset_call_counts();
    ev = make_ev(overflow_type);
    robotos_core_post_event(&ev);
    robotos_core_dispatch_events(1u);
    CHECK("TC17: handler_b dispatched via overflow_type (new slot fill confirmed)",
          s_call_b == 1u);

    /* Clear all registrations for subsequent sections */
    for (uint32_t i = 1u; i < ROBOTOS_CORE_MAX_EVENT_HANDLERS; i++) {
        robotos_core_unregister_event_handler(slot_types[i]);
    }
    robotos_core_unregister_event_handler(overflow_type);
    CHECK("TC18: count == 0 after clearing all registrations",
          robotos_core_registered_handler_count() == 0u);

    /* ---- Section 3: Unregister-all → all events become unhandled ------------ */
    printf("\n[ Unregister-all makes all events unhandled ]\n");

    robotos_core_register_event_handler(ROBOTOS_EVENT_USER, handler_a, NULL);
    robotos_core_register_event_handler(ROBOTOS_EVENT_CORE_TICK, handler_b, NULL);

    /* Confirm dispatch works before unregister */
    reset_call_counts();
    ev = make_ev(ROBOTOS_EVENT_USER);      robotos_core_post_event(&ev);
    ev = make_ev(ROBOTOS_EVENT_CORE_TICK); robotos_core_post_event(&ev);
    robotos_core_dispatch_events(2u);
    CHECK("TC19: handler_a called before unregister", s_call_a == 1u);
    CHECK("TC20: handler_b called before unregister", s_call_b == 1u);

    /* Unregister both */
    robotos_core_unregister_event_handler(ROBOTOS_EVENT_USER);
    robotos_core_unregister_event_handler(ROBOTOS_EVENT_CORE_TICK);
    CHECK("TC21: count == 0 after unregister-all",
          robotos_core_registered_handler_count() == 0u);

    uint32_t unhandled_before = robotos_core_unhandled_event_count();
    reset_call_counts();
    ev = make_ev(ROBOTOS_EVENT_USER);      robotos_core_post_event(&ev);
    ev = make_ev(ROBOTOS_EVENT_CORE_TICK); robotos_core_post_event(&ev);
    robotos_core_dispatch_events(2u);
    CHECK("TC22: handler_a NOT called after unregister-all", s_call_a == 0u);
    CHECK("TC23: handler_b NOT called after unregister-all", s_call_b == 0u);
    CHECK("TC24: unhandled_count +2 (both events unhandled)",
          robotos_core_unhandled_event_count() == unhandled_before + 2u);

    /* ---- Section 4: Churn stability ---------------------------------------- */
    printf("\n[ Register/unregister churn stability ]\n");

    for (uint32_t i = 0u; i < 10u; i++) {
        robotos_core_register_event_handler(ROBOTOS_EVENT_USER, handler_a, NULL);
        robotos_core_unregister_event_handler(ROBOTOS_EVENT_USER);
    }
    CHECK("TC25: count == 0 after 10 register/unregister cycles",
          robotos_core_registered_handler_count() == 0u);
    CHECK("TC26: has_handler(USER) == false after churn",
          robotos_core_has_event_handler(ROBOTOS_EVENT_USER) == false);

    /* Registration after churn must work and dispatch correctly */
    rc = robotos_core_register_event_handler(ROBOTOS_EVENT_USER, handler_a, NULL);
    CHECK("TC27: register after churn returns OK", rc == ROBOTOS_CORE_OK);
    reset_call_counts();
    ev = make_ev(ROBOTOS_EVENT_USER);
    robotos_core_post_event(&ev);
    robotos_core_dispatch_events(1u);
    CHECK("TC28: handler_a dispatched correctly after churn", s_call_a == 1u);

    /* ---- Section 5: Handler error does not deregister ----------------------- */
    printf("\n[ Handler error does not deregister handler ]\n");

    /* Replace USER registration with error handler */
    rc = robotos_core_register_event_handler(ROBOTOS_EVENT_USER, handler_err, NULL);
    CHECK("TC29: replace with error handler returns OK (replacement)",
          rc == ROBOTOS_CORE_OK);

    ev = make_ev(ROBOTOS_EVENT_USER);
    robotos_core_post_event(&ev);
    reset_call_counts();
    rc = robotos_core_dispatch_events(1u);
    CHECK("TC30: dispatch returns handler's error", rc == ROBOTOS_CORE_ERR_INVALID_STATE);
    CHECK("TC31: error handler was called", s_call_err == 1u);

    /* Handler must remain registered after the error */
    CHECK("TC32: has_handler(USER) still true after handler error",
          robotos_core_has_event_handler(ROBOTOS_EVENT_USER) == true);
    CHECK("TC33: count still 1 after handler error",
          robotos_core_registered_handler_count() == 1u);

    /* Second dispatch on same type fires the same handler again */
    ev = make_ev(ROBOTOS_EVENT_USER);
    robotos_core_post_event(&ev);
    reset_call_counts();
    rc = robotos_core_dispatch_events(1u);
    CHECK("TC34: error handler fires again on second dispatch (not auto-deregistered)",
          s_call_err == 1u && rc == ROBOTOS_CORE_ERR_INVALID_STATE);

    robotos_core_unregister_event_handler(ROBOTOS_EVENT_USER);

    /* ---- Section 6: Replacement routes exclusively to new handler ----------- */
    printf("\n[ Replacement routes to new handler, not old ]\n");

    rc = robotos_core_register_event_handler(ROBOTOS_EVENT_USER, handler_a, NULL);
    CHECK("TC35: register USER with handler_a returns OK", rc == ROBOTOS_CORE_OK);

    /* Replace with handler_b */
    rc = robotos_core_register_event_handler(ROBOTOS_EVENT_USER, handler_b, NULL);
    CHECK("TC36: replace USER with handler_b returns OK", rc == ROBOTOS_CORE_OK);
    CHECK("TC37: count still 1 after replacement (no new slot)",
          robotos_core_registered_handler_count() == 1u);

    reset_call_counts();
    ev = make_ev(ROBOTOS_EVENT_USER);
    robotos_core_post_event(&ev);
    rc = robotos_core_dispatch_events(1u);
    CHECK("TC38: dispatch returns OK after replacement", rc == ROBOTOS_CORE_OK);
    CHECK("TC39: handler_b called (new handler)", s_call_b == 1u);
    CHECK("TC40: handler_a NOT called (old handler replaced)", s_call_a == 0u);

    /* ---- Section 7: Re-register after unregister dispatches correctly -------- */
    printf("\n[ Re-register after unregister ]\n");

    /* Current: USER registered with handler_b */
    rc = robotos_core_unregister_event_handler(ROBOTOS_EVENT_USER);
    CHECK("TC41: unregister USER returns OK", rc == ROBOTOS_CORE_OK);
    CHECK("TC42: has_handler(USER) == false after unregister",
          robotos_core_has_event_handler(ROBOTOS_EVENT_USER) == false);

    /* Re-register with handler_a */
    rc = robotos_core_register_event_handler(ROBOTOS_EVENT_USER, handler_a, NULL);
    CHECK("TC43: re-register USER with handler_a returns OK", rc == ROBOTOS_CORE_OK);

    reset_call_counts();
    ev = make_ev(ROBOTOS_EVENT_USER);
    robotos_core_post_event(&ev);
    rc = robotos_core_dispatch_events(1u);
    CHECK("TC44: dispatch routes to handler_a after re-register",
          s_call_a == 1u && rc == ROBOTOS_CORE_OK);
    CHECK("TC45: handler_b NOT called after re-register", s_call_b == 0u);

    /* ---- Section 8: Snapshot coherence through lifecycle -------------------- */
    printf("\n[ Snapshot coherence through lifecycle ]\n");

    drain_queue();
    robotos_core_snapshot_t snap;
    rc = robotos_core_snapshot(&snap);
    CHECK("TC46: snapshot returns OK", rc == ROBOTOS_CORE_OK);
    CHECK("TC47: snapshot.registered_handler_count matches getter",
          snap.registered_handler_count == robotos_core_registered_handler_count());
    CHECK("TC48: snapshot.unhandled_event_count matches getter",
          snap.unhandled_event_count == robotos_core_unhandled_event_count());
    CHECK("TC49: snapshot.handler_error_count matches getter",
          snap.handler_error_count == robotos_core_handler_error_count());
    CHECK("TC50: snapshot.dispatched_event_count matches getter",
          snap.dispatched_event_count == robotos_core_dispatched_event_count());
    CHECK("TC51: snapshot.state == READY", snap.state == ROBOTOS_CORE_STATE_READY);
    CHECK("TC52: snapshot.pending == 0 (queue drained)",
          snap.pending_event_count == 0u);

    /* ---- Summary ------------------------------------------------------------- */
    printf("\n=== Phase 6J-B Results: %d passed, %d failed ===\n", s_pass, s_fail);
    return (s_fail > 0) ? 1 : 0;
}
