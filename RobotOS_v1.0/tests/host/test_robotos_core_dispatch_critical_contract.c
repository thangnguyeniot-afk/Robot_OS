/*
 * test_robotos_core_dispatch_critical_contract.c
 * Phase 5F -- Dispatcher Pop / Handler Split Critical Contract Tests
 *
 * Validates that after the Phase 5F refactor:
 *   - Handler callbacks always see critical depth == 0 (no lock held)
 *   - FIFO order is preserved across the pop/lookup/invoke split
 *   - unhandled, handler_error, dispatched counters behave correctly
 *   - explicit dispatch_events semantics preserved
 *   - tick empty/handler-error semantics preserved
 *   - register/replace/unregister routing remains deterministic
 *
 * Uses robotos_platform_critical_host_stub for enter/exit instrumentation.
 * No Zephyr. No hardware. Single process with sequential test cases.
 * Fresh static core state shared across tests; queue drained between cases.
 *
 * Critical rule (tested in TC01, TC02):
 *   handler sees robotos_platform_critical_host_current_depth() == 0
 *   confirming no lock is held during handler invocation.
 */

#include "robotos_core.h"
#include "robotos_event_queue.h"
#include "robotos_platform_critical_host_stub.h"

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

/* -------------------------------------------------------------------------- */

static int s_pass;
static int s_fail;

#define CHECK(label, expr)                                                  \
    do {                                                                    \
        if (expr) {                                                         \
            printf("[PASS] %s\n", label);                                  \
            s_pass++;                                                       \
        } else {                                                            \
            printf("[FAIL] %s  (line %d)\n", label, __LINE__);            \
            s_fail++;                                                       \
        }                                                                   \
    } while (0)

/* -------------------------------------------------------------------------- */

/* Test event types (ROBOTOS_EVENT_USER = 100) */
#define EV_A  ROBOTOS_EVENT_USER          /* 100 */
#define EV_B  (ROBOTOS_EVENT_USER + 1u)   /* 101 */
#define EV_C  (ROBOTOS_EVENT_USER + 2u)   /* 102 */

/* ---- Shared handler state ------------------------------------------------ */
static unsigned int s_handler_depth_in_cb;  /* depth observed inside handler  */
static uint32_t     s_handler_call_count;
static uint32_t     s_fifo_log[16];         /* recorded arg0 sequence         */
static uint32_t     s_fifo_idx;
static bool         s_handler_b_called;
static bool         s_handler_b2_called;

/* ---- Drain helper -------------------------------------------------------- */
/*
 * Drain all pending events (up to queue capacity) via dispatch_events.
 * Ignores return value. Used for inter-test cleanup.
 */
static void drain_all(void)
{
    (void)robotos_core_dispatch_events(ROBOTOS_EVENT_QUEUE_CAPACITY);
}

/* ---- Handler implementations --------------------------------------------- */

/* Records critical depth inside callback */
static robotos_core_status_t handler_depth_check(const robotos_event_t *ev,
                                                   void *ctx)
{
    (void)ev; (void)ctx;
    s_handler_depth_in_cb = robotos_platform_critical_host_current_depth();
    s_handler_call_count++;
    return ROBOTOS_CORE_OK;
}

/* Records arg0 for FIFO order check */
static robotos_core_status_t handler_fifo_record(const robotos_event_t *ev,
                                                   void *ctx)
{
    (void)ctx;
    s_handler_depth_in_cb = robotos_platform_critical_host_current_depth();
    if (s_fifo_idx < 16u) {
        s_fifo_log[s_fifo_idx++] = ev->arg0;
    }
    s_handler_call_count++;
    return ROBOTOS_CORE_OK;
}

/* Returns handler error -- for testing handler_error_count */
static robotos_core_status_t handler_return_error(const robotos_event_t *ev,
                                                    void *ctx)
{
    (void)ev; (void)ctx;
    s_handler_depth_in_cb = robotos_platform_critical_host_current_depth();
    s_handler_call_count++;
    return ROBOTOS_CORE_ERR_INVALID_STATE;
}

/* Unregisters itself during callback -- tests no-deadlock + depth == 0 */
static robotos_core_status_t handler_self_unregister(const robotos_event_t *ev,
                                                       void *ctx)
{
    (void)ctx;
    s_handler_depth_in_cb = robotos_platform_critical_host_current_depth();
    s_handler_call_count++;
    /* Handler runs outside lock; unregister enters a fresh (non-nested) lock */
    (void)robotos_core_unregister_event_handler(ev->type);
    return ROBOTOS_CORE_OK;
}

/* Handler B -- for replace test */
static robotos_core_status_t handler_B(const robotos_event_t *ev, void *ctx)
{
    (void)ev; (void)ctx;
    s_handler_b_called    = true;
    s_handler_depth_in_cb = robotos_platform_critical_host_current_depth();
    s_handler_call_count++;
    return ROBOTOS_CORE_OK;
}

/* Handler B2 -- replacement for same type */
static robotos_core_status_t handler_B2(const robotos_event_t *ev, void *ctx)
{
    (void)ev; (void)ctx;
    s_handler_b2_called   = true;
    s_handler_depth_in_cb = robotos_platform_critical_host_current_depth();
    s_handler_call_count++;
    return ROBOTOS_CORE_OK;
}

/* Counter-only OK handler */
static robotos_core_status_t handler_count_only(const robotos_event_t *ev,
                                                  void *ctx)
{
    (void)ev; (void)ctx;
    s_handler_depth_in_cb = robotos_platform_critical_host_current_depth();
    s_handler_call_count++;
    return ROBOTOS_CORE_OK;
}

/* -------------------------------------------------------------------------- */

int main(void)
{
    printf("=== Phase 5F: Dispatch Critical Split Contract Tests ===\n\n");

    /* One-time init; static core state shared across all tests */
    robotos_core_init();

    robotos_core_status_t   rc;
    robotos_core_snapshot_t snap;

    /* =========================================================================
     * TC01: Handler sees depth == 0 when dispatched by tick()
     * ========================================================================= */
    {
        robotos_core_register_event_handler(EV_A, handler_depth_check, NULL);

        const robotos_event_t ev = { .type = EV_A, .timestamp_tick = 0,
                                     .arg0 = 1, .arg1 = 0 };
        robotos_core_post_event(&ev);

        robotos_platform_critical_host_reset(); /* reset after setup */
        s_handler_depth_in_cb = 0xFFu;
        s_handler_call_count  = 0;

        rc = robotos_core_tick();

        CHECK("TC01 tick returns OK",             rc == ROBOTOS_CORE_OK);
        CHECK("TC01 handler called",              s_handler_call_count == 1u);
        CHECK("TC01 depth inside handler == 0",   s_handler_depth_in_cb == 0u);
        CHECK("TC01 depth after tick == 0",
              robotos_platform_critical_host_current_depth() == 0u);
        CHECK("TC01 enter == exit",
              robotos_platform_critical_host_enter_count() ==
              robotos_platform_critical_host_exit_count());
    }

    /* =========================================================================
     * TC02: Handler sees depth == 0 when dispatched by dispatch_events()
     * ========================================================================= */
    {
        /* EV_A handler still registered from TC01 */
        const robotos_event_t ev = { .type = EV_A, .timestamp_tick = 0,
                                     .arg0 = 2, .arg1 = 0 };
        robotos_core_post_event(&ev);

        robotos_platform_critical_host_reset();
        s_handler_depth_in_cb = 0xFFu;
        s_handler_call_count  = 0;

        rc = robotos_core_dispatch_events(1);

        CHECK("TC02 dispatch_events returns OK",  rc == ROBOTOS_CORE_OK);
        CHECK("TC02 handler called",              s_handler_call_count == 1u);
        CHECK("TC02 depth inside handler == 0",   s_handler_depth_in_cb == 0u);
        CHECK("TC02 depth after dispatch == 0",
              robotos_platform_critical_host_current_depth() == 0u);
        CHECK("TC02 enter == exit",
              robotos_platform_critical_host_enter_count() ==
              robotos_platform_critical_host_exit_count());
    }

    /* =========================================================================
     * TC03: FIFO preserved with multiple events after split
     * ========================================================================= */
    {
        robotos_core_register_event_handler(EV_A, handler_fifo_record, NULL);
        s_fifo_idx            = 0;
        s_handler_call_count  = 0;
        s_handler_depth_in_cb = 0xFFu;

        const robotos_event_t e1 = { .type = EV_A, .arg0 = 100 };
        const robotos_event_t e2 = { .type = EV_A, .arg0 = 200 };
        const robotos_event_t e3 = { .type = EV_A, .arg0 = 300 };
        robotos_core_post_event(&e1);
        robotos_core_post_event(&e2);
        robotos_core_post_event(&e3);

        rc = robotos_core_dispatch_events(3);

        CHECK("TC03 dispatch_events(3) returns OK",  rc == ROBOTOS_CORE_OK);
        CHECK("TC03 handler called 3 times",          s_handler_call_count == 3u);
        CHECK("TC03 FIFO: first arg0 == 100",         s_fifo_log[0] == 100u);
        CHECK("TC03 FIFO: second arg0 == 200",        s_fifo_log[1] == 200u);
        CHECK("TC03 FIFO: third arg0 == 300",         s_fifo_log[2] == 300u);
        CHECK("TC03 depth inside handlers == 0",      s_handler_depth_in_cb == 0u);
    }

    /* =========================================================================
     * TC04: Unregistered event type -> unhandled_count++, returns OK; depth == 0
     * ========================================================================= */
    {
        robotos_core_unregister_event_handler(EV_A);

        robotos_core_snapshot(&snap);
        uint32_t unhandled_before = snap.unhandled_event_count;

        robotos_platform_critical_host_reset();

        const robotos_event_t ev = { .type = EV_A, .arg0 = 4 };
        robotos_core_post_event(&ev);
        rc = robotos_core_dispatch_events(1);
        robotos_core_snapshot(&snap);

        CHECK("TC04 dispatch returns OK",
              rc == ROBOTOS_CORE_OK);
        CHECK("TC04 unhandled_count incremented",
              snap.unhandled_event_count == unhandled_before + 1u);
        CHECK("TC04 depth after dispatch == 0",
              robotos_platform_critical_host_current_depth() == 0u);
        CHECK("TC04 enter == exit",
              robotos_platform_critical_host_enter_count() ==
              robotos_platform_critical_host_exit_count());
    }

    /* =========================================================================
     * TC05: Handler error -> handler_error_count++, returns error; depth == 0
     * ========================================================================= */
    {
        robotos_core_register_event_handler(EV_A, handler_return_error, NULL);
        robotos_core_snapshot(&snap);
        uint32_t err_before = snap.handler_error_count;

        robotos_platform_critical_host_reset();
        s_handler_call_count  = 0;
        s_handler_depth_in_cb = 0xFFu;

        const robotos_event_t ev = { .type = EV_A, .arg0 = 5 };
        robotos_core_post_event(&ev);
        rc = robotos_core_dispatch_events(1);
        robotos_core_snapshot(&snap);

        CHECK("TC05 dispatch returns handler error",
              rc == ROBOTOS_CORE_ERR_INVALID_STATE);
        CHECK("TC05 handler_error_count incremented",
              snap.handler_error_count == err_before + 1u);
        CHECK("TC05 handler was called",          s_handler_call_count == 1u);
        CHECK("TC05 depth inside handler == 0",   s_handler_depth_in_cb == 0u);
        CHECK("TC05 depth after dispatch == 0",
              robotos_platform_critical_host_current_depth() == 0u);
    }

    /* =========================================================================
     * TC06: tick with empty queue -> OK; depth == 0
     * ========================================================================= */
    {
        drain_all();
        robotos_platform_critical_host_reset();

        rc = robotos_core_tick();

        CHECK("TC06 tick empty returns OK",  rc == ROBOTOS_CORE_OK);
        CHECK("TC06 depth after tick == 0",
              robotos_platform_critical_host_current_depth() == 0u);
        CHECK("TC06 enter == exit",
              robotos_platform_critical_host_enter_count() ==
              robotos_platform_critical_host_exit_count());
    }

    /* =========================================================================
     * TC07: dispatch_events empty -> ERR_EMPTY; depth == 0
     * ========================================================================= */
    {
        drain_all();
        robotos_platform_critical_host_reset();

        rc = robotos_core_dispatch_events(1);

        CHECK("TC07 dispatch_events empty returns ERR_EMPTY",
              rc == ROBOTOS_CORE_ERR_EMPTY);
        CHECK("TC07 depth after dispatch == 0",
              robotos_platform_critical_host_current_depth() == 0u);
        CHECK("TC07 enter == exit",
              robotos_platform_critical_host_enter_count() ==
              robotos_platform_critical_host_exit_count());
    }

    /* =========================================================================
     * TC08: dispatch_events(max=0) -> OK; does not dispatch
     * ========================================================================= */
    {
        robotos_core_register_event_handler(EV_A, handler_count_only, NULL);
        s_handler_call_count = 0;

        const robotos_event_t ev = { .type = EV_A, .arg0 = 8 };
        robotos_core_post_event(&ev);

        rc = robotos_core_dispatch_events(0);

        CHECK("TC08 dispatch_events(0) returns OK",  rc == ROBOTOS_CORE_OK);
        CHECK("TC08 handler NOT called",              s_handler_call_count == 0u);

        drain_all(); /* clean up pending event */
    }

    /* =========================================================================
     * TC09: dispatch_events(max=2) dispatches exactly two, not three
     * ========================================================================= */
    {
        robotos_core_register_event_handler(EV_A, handler_count_only, NULL);
        s_handler_call_count = 0;

        const robotos_event_t e1 = { .type = EV_A, .arg0 = 91 };
        const robotos_event_t e2 = { .type = EV_A, .arg0 = 92 };
        const robotos_event_t e3 = { .type = EV_A, .arg0 = 93 };
        robotos_core_post_event(&e1);
        robotos_core_post_event(&e2);
        robotos_core_post_event(&e3);

        rc = robotos_core_dispatch_events(2);

        CHECK("TC09 dispatch_events(2) returns OK",  rc == ROBOTOS_CORE_OK);
        CHECK("TC09 handler called exactly 2 times", s_handler_call_count == 2u);

        robotos_core_snapshot(&snap);
        CHECK("TC09 one event still pending",        snap.pending_event_count == 1u);

        drain_all(); /* clean up remaining event */
    }

    /* =========================================================================
     * TC10: Unregister after post but before dispatch -> event becomes unhandled
     *       (handler lookup occurs at dispatch time under Phase 5F split)
     * ========================================================================= */
    {
        robotos_core_register_event_handler(EV_A, handler_count_only, NULL);

        const robotos_event_t ev = { .type = EV_A, .arg0 = 10 };
        robotos_core_post_event(&ev);

        /* Unregister BEFORE dispatch -- lookup at dispatch time finds nothing */
        robotos_core_unregister_event_handler(EV_A);

        robotos_core_snapshot(&snap);
        uint32_t unhandled_before = snap.unhandled_event_count;
        s_handler_call_count = 0;

        rc = robotos_core_dispatch_events(1);
        robotos_core_snapshot(&snap);

        CHECK("TC10 dispatch returns OK",
              rc == ROBOTOS_CORE_OK);
        CHECK("TC10 handler NOT called (lookup at dispatch time)",
              s_handler_call_count == 0u);
        CHECK("TC10 unhandled_count incremented",
              snap.unhandled_event_count == unhandled_before + 1u);
        CHECK("TC10 depth after dispatch == 0",
              robotos_platform_critical_host_current_depth() == 0u);
    }

    /* =========================================================================
     * TC11: Handler unregisters itself during callback -> no deadlock, depth == 0
     *       Handler runs outside lock; unregister enters a fresh non-nested lock.
     * ========================================================================= */
    {
        robotos_core_register_event_handler(EV_B, handler_self_unregister, NULL);
        s_handler_call_count  = 0;
        s_handler_depth_in_cb = 0xFFu;

        const robotos_event_t ev = { .type = EV_B, .arg0 = 11 };
        robotos_core_post_event(&ev);

        robotos_platform_critical_host_reset();
        rc = robotos_core_dispatch_events(1);

        CHECK("TC11 dispatch returns OK",           rc == ROBOTOS_CORE_OK);
        CHECK("TC11 handler was called",            s_handler_call_count == 1u);
        CHECK("TC11 depth inside handler == 0",     s_handler_depth_in_cb == 0u);
        CHECK("TC11 depth after dispatch == 0",
              robotos_platform_critical_host_current_depth() == 0u);
        CHECK("TC11 enter == exit (no lock leak)",
              robotos_platform_critical_host_enter_count() ==
              robotos_platform_critical_host_exit_count());
        CHECK("TC11 handler self-unregistered",
              !robotos_core_has_event_handler(EV_B));
    }

    /* =========================================================================
     * TC12: Register/replace handler then dispatch -> latest handler is used
     * ========================================================================= */
    {
        s_handler_b_called    = false;
        s_handler_b2_called   = false;
        s_handler_call_count  = 0;
        s_handler_depth_in_cb = 0xFFu;

        /* Register B first, then replace with B2 */
        robotos_core_register_event_handler(EV_C, handler_B,  NULL);
        robotos_core_register_event_handler(EV_C, handler_B2, NULL);

        const robotos_event_t ev = { .type = EV_C, .arg0 = 12 };
        robotos_core_post_event(&ev);

        rc = robotos_core_dispatch_events(1);

        CHECK("TC12 dispatch returns OK",            rc == ROBOTOS_CORE_OK);
        CHECK("TC12 handler B NOT called",           !s_handler_b_called);
        CHECK("TC12 handler B2 called (latest)",      s_handler_b2_called);
        CHECK("TC12 depth inside handler == 0",       s_handler_depth_in_cb == 0u);

        robotos_core_unregister_event_handler(EV_C);
    }

    /* =========================================================================
     * TC13: No critical depth leak after repeated post/tick loop
     * ========================================================================= */
    {
        robotos_core_register_event_handler(EV_A, handler_depth_check, NULL);
        robotos_platform_critical_host_reset();

        bool depth_always_zero = true;

        for (uint32_t i = 0u; i < 5u; i++) {
            s_handler_depth_in_cb = 0xFFu;
            const robotos_event_t ev = { .type = EV_A, .arg0 = i };
            robotos_core_post_event(&ev);
            robotos_core_tick();

            if (s_handler_depth_in_cb != 0u) {
                depth_always_zero = false;
            }
            if (robotos_platform_critical_host_current_depth() != 0u) {
                depth_always_zero = false;
            }
        }

        CHECK("TC13 depth always 0 in handler across 5 ticks",
              depth_always_zero);
        CHECK("TC13 depth after loop == 0",
              robotos_platform_critical_host_current_depth() == 0u);
        CHECK("TC13 enter == exit after loop",
              robotos_platform_critical_host_enter_count() ==
              robotos_platform_critical_host_exit_count());
    }

    /* ---------------------------------------------------------------------- */
    printf("\n=== Phase 5F Results: %d passed, %d failed ===\n",
           s_pass, s_fail);

    return (s_fail == 0) ? 0 : 1;
}
