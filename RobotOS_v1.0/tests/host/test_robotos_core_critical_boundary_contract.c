/*
 * test_robotos_core_critical_boundary_contract.c
 * Phase 5E — Core Critical Boundary Contract Tests
 *
 * Validates that core state transitions are guarded with balanced enter/exit
 * critical sections, and that NO critical section is held when registered
 * event handler callbacks execute.
 *
 * Critical rule (tested in TC10):
 *   handler sees robotos_platform_critical_host_current_depth() == 0
 *   confirming no lock is held during handler invocation.
 *
 * Uses robotos_platform_critical_host_stub for enter/exit instrumentation.
 * No Zephyr. No hardware. Fresh static state.
 *
 * Phase 5E limitation (documented, not a test failure):
 *   dispatcher pop and handler execution share the same unlocked region.
 *   Full dispatcher/handler separation is a Phase 5F scope item.
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

static const robotos_event_t EV_USER = {
    .type = ROBOTOS_EVENT_USER, .timestamp_tick = 0, .arg0 = 0x5E, .arg1 = 0
};

static const robotos_event_t EV_NONE = {
    .type = ROBOTOS_EVENT_NONE, .timestamp_tick = 0, .arg0 = 0, .arg1 = 0
};

/* Depth observed inside handler callback — must be 0 */
static unsigned int s_handler_depth_observed;
static uint32_t     s_handler_call_count;

static robotos_core_status_t handler_depth_check(const robotos_event_t *event, void *ctx)
{
    (void)event; (void)ctx;
    s_handler_depth_observed = robotos_platform_critical_host_current_depth();
    s_handler_call_count++;
    return ROBOTOS_CORE_OK;
}

static robotos_core_status_t handler_ok(const robotos_event_t *event, void *ctx)
{
    (void)event; (void)ctx;
    return ROBOTOS_CORE_OK;
}

/* -------------------------------------------------------------------------- */

int main(void)
{
    printf("=== Phase 5E: Core Critical Boundary Contract Tests ===\n\n");

    robotos_core_status_t rc;
    robotos_core_snapshot_t snap;
    unsigned int e0, x0;

    /* =========================================================================
     * TC01: Reset host critical counters; pre-init depth must be 0
     * ========================================================================= */

    robotos_platform_critical_host_reset();
    CHECK("TC01 initial enter_count == 0",
          robotos_platform_critical_host_enter_count() == 0u);
    CHECK("TC01 initial exit_count == 0",
          robotos_platform_critical_host_exit_count() == 0u);
    CHECK("TC01 initial current_depth == 0",
          robotos_platform_critical_host_current_depth() == 0u);

    /* =========================================================================
     * TC02: snapshot(NULL) returns ERR_NULL; depth still 0 (null check before lock)
     * ========================================================================= */

    robotos_platform_critical_host_reset();
    rc = robotos_core_snapshot(NULL);
    CHECK("TC02 snapshot(NULL) returns ERR_NULL", rc == ROBOTOS_CORE_ERR_NULL);
    CHECK("TC02 depth 0 after snapshot(NULL) (null checked before lock)",
          robotos_platform_critical_host_current_depth() == 0u);

    /* =========================================================================
     * TC03: robotos_core_init() — enter/exit balanced, depth 0 after
     * ========================================================================= */

    robotos_platform_critical_host_reset();
    e0 = robotos_platform_critical_host_enter_count();
    x0 = robotos_platform_critical_host_exit_count();
    rc = robotos_core_init();
    CHECK("TC03 init returns OK", rc == ROBOTOS_CORE_OK);
    CHECK("TC03 enter_count increased after init",
          robotos_platform_critical_host_enter_count() > e0);
    CHECK("TC03 enter == exit (balanced) after init",
          robotos_platform_critical_host_enter_count() == robotos_platform_critical_host_exit_count());
    CHECK("TC03 depth 0 after init",
          robotos_platform_critical_host_current_depth() == 0u);
    (void)x0;

    /* =========================================================================
     * TC04: post_event valid — enter/exit balanced, depth 0 after
     * ========================================================================= */

    robotos_platform_critical_host_reset();
    rc = robotos_core_post_event(&EV_USER);
    CHECK("TC04 post_event returns OK", rc == ROBOTOS_CORE_OK);
    CHECK("TC04 enter_count increased",
          robotos_platform_critical_host_enter_count() > 0u);
    CHECK("TC04 enter == exit (balanced) after post_event",
          robotos_platform_critical_host_enter_count() == robotos_platform_critical_host_exit_count());
    CHECK("TC04 depth 0 after post_event",
          robotos_platform_critical_host_current_depth() == 0u);

    /* Drain the posted event */
    robotos_core_dispatch_events(1u);

    /* =========================================================================
     * TC05: try_post_event valid — enter/exit balanced, depth 0 after
     * ========================================================================= */

    robotos_platform_critical_host_reset();
    rc = robotos_core_try_post_event(&EV_USER);
    CHECK("TC05 try_post_event returns OK", rc == ROBOTOS_CORE_OK);
    CHECK("TC05 enter == exit (balanced) after try_post_event",
          robotos_platform_critical_host_enter_count() == robotos_platform_critical_host_exit_count());
    CHECK("TC05 depth 0 after try_post_event",
          robotos_platform_critical_host_current_depth() == 0u);

    robotos_core_dispatch_events(1u);

    /* =========================================================================
     * TC06: post_event invalid (NONE) — enter/exit balanced, depth 0 after
     * ========================================================================= */

    robotos_platform_critical_host_reset();
    rc = robotos_core_post_event(&EV_NONE);
    CHECK("TC06 post_event NONE returns ERR_INVALID_ARG",
          rc == ROBOTOS_CORE_ERR_INVALID_ARG);
    CHECK("TC06 enter == exit (balanced) after invalid post",
          robotos_platform_critical_host_enter_count() == robotos_platform_critical_host_exit_count());
    CHECK("TC06 depth 0 after invalid post",
          robotos_platform_critical_host_current_depth() == 0u);

    /* =========================================================================
     * TC07: try_post_event throttled — enter/exit balanced, depth 0 after
     *       Create pending=2 > budget=1 without throttle first (using post_event),
     *       then call try_post_event which should return ERR_THROTTLED.
     * ========================================================================= */

    /* Fill to pending=2 using raw post_event (no throttle) */
    robotos_core_post_event(&EV_USER);
    robotos_core_post_event(&EV_USER);

    robotos_platform_critical_host_reset();
    rc = robotos_core_try_post_event(&EV_USER);
    CHECK("TC07 try_post_event throttled returns ERR_THROTTLED",
          rc == ROBOTOS_CORE_ERR_THROTTLED);
    CHECK("TC07 enter == exit (balanced) after throttle",
          robotos_platform_critical_host_enter_count() == robotos_platform_critical_host_exit_count());
    CHECK("TC07 depth 0 after throttle",
          robotos_platform_critical_host_current_depth() == 0u);

    /* Drain backlog */
    robotos_core_dispatch_events(ROBOTOS_EVENT_QUEUE_CAPACITY);

    /* =========================================================================
     * TC08: snapshot — enter/exit balanced, depth 0 after, returns correct state
     * ========================================================================= */

    robotos_platform_critical_host_reset();
    rc = robotos_core_snapshot(&snap);
    CHECK("TC08 snapshot returns OK", rc == ROBOTOS_CORE_OK);
    CHECK("TC08 enter == exit (balanced) after snapshot",
          robotos_platform_critical_host_enter_count() == robotos_platform_critical_host_exit_count());
    CHECK("TC08 depth 0 after snapshot",
          robotos_platform_critical_host_current_depth() == 0u);
    CHECK("TC08 snapshot state == READY",
          snap.state == ROBOTOS_CORE_STATE_READY);

    /* =========================================================================
     * TC09: handler registration/unregistration — enter/exit balanced
     * ========================================================================= */

    robotos_platform_critical_host_reset();
    rc = robotos_core_register_event_handler(ROBOTOS_EVENT_USER, handler_ok, NULL);
    CHECK("TC09 register_handler returns OK", rc == ROBOTOS_CORE_OK);
    CHECK("TC09 enter == exit (balanced) after register",
          robotos_platform_critical_host_enter_count() == robotos_platform_critical_host_exit_count());
    CHECK("TC09 depth 0 after register",
          robotos_platform_critical_host_current_depth() == 0u);

    robotos_platform_critical_host_reset();
    rc = robotos_core_unregister_event_handler(ROBOTOS_EVENT_USER);
    CHECK("TC09b unregister_handler returns OK", rc == ROBOTOS_CORE_OK);
    CHECK("TC09b enter == exit (balanced) after unregister",
          robotos_platform_critical_host_enter_count() == robotos_platform_critical_host_exit_count());
    CHECK("TC09b depth 0 after unregister",
          robotos_platform_critical_host_current_depth() == 0u);

    /* =========================================================================
     * TC10: CRITICAL — handler callback sees depth == 0 (no lock held)
     *       Register depth-check handler, post event, tick dispatches it.
     *       Handler records current_depth; we verify it is 0.
     * ========================================================================= */

    s_handler_depth_observed = 999u; /* sentinel */
    s_handler_call_count     = 0u;

    rc = robotos_core_register_event_handler(ROBOTOS_EVENT_USER,
                                              handler_depth_check, NULL);
    CHECK("TC10 register depth-check handler", rc == ROBOTOS_CORE_OK);

    robotos_core_post_event(&EV_USER);

    robotos_platform_critical_host_reset();
    rc = robotos_core_tick();
    CHECK("TC10 tick returns OK", rc == ROBOTOS_CORE_OK);
    CHECK("TC10 handler was called",
          s_handler_call_count == 1u);
    CHECK("TC10 CRITICAL: depth == 0 inside handler (no lock held during callback)",
          s_handler_depth_observed == 0u);
    CHECK("TC10 enter == exit (balanced) after tick",
          robotos_platform_critical_host_enter_count() == robotos_platform_critical_host_exit_count());
    CHECK("TC10 depth 0 after tick",
          robotos_platform_critical_host_current_depth() == 0u);

    robotos_core_unregister_event_handler(ROBOTOS_EVENT_USER);

    /* =========================================================================
     * TC11: dispatch_events — handler sees depth == 0
     * ========================================================================= */

    s_handler_depth_observed = 999u;
    s_handler_call_count     = 0u;

    rc = robotos_core_register_event_handler(ROBOTOS_EVENT_USER,
                                              handler_depth_check, NULL);
    CHECK("TC11 register depth-check handler", rc == ROBOTOS_CORE_OK);

    robotos_core_post_event(&EV_USER);
    robotos_platform_critical_host_reset();
    rc = robotos_core_dispatch_events(1u);
    CHECK("TC11 dispatch_events returns OK", rc == ROBOTOS_CORE_OK);
    CHECK("TC11 handler was called in dispatch_events",
          s_handler_call_count == 1u);
    CHECK("TC11 CRITICAL: depth == 0 inside handler during dispatch_events",
          s_handler_depth_observed == 0u);
    CHECK("TC11 depth 0 after dispatch_events",
          robotos_platform_critical_host_current_depth() == 0u);

    robotos_core_unregister_event_handler(ROBOTOS_EVENT_USER);

    /* =========================================================================
     * TC12: Stress — loop 5 post/tick cycles; depth always 0 at end
     * ========================================================================= */

    robotos_platform_critical_host_reset();
    for (uint32_t i = 0u; i < 5u; i++) {
        robotos_core_post_event(&EV_USER);
        robotos_core_tick();
    }
    CHECK("TC12 depth 0 after 5x post+tick loop",
          robotos_platform_critical_host_current_depth() == 0u);
    CHECK("TC12 enter == exit (balanced) after stress loop",
          robotos_platform_critical_host_enter_count() == robotos_platform_critical_host_exit_count());

    /* =========================================================================
     * TC13: Repeated init — enter/exit balanced, does not reset event counters
     * ========================================================================= */

    uint32_t accepted_before = robotos_core_admission_accepted_count();
    robotos_platform_critical_host_reset();
    rc = robotos_core_init();
    CHECK("TC13 repeated init returns OK", rc == ROBOTOS_CORE_OK);
    CHECK("TC13 enter == exit (balanced) after repeated init",
          robotos_platform_critical_host_enter_count() == robotos_platform_critical_host_exit_count());
    CHECK("TC13 depth 0 after repeated init",
          robotos_platform_critical_host_current_depth() == 0u);
    CHECK("TC13 admission_accepted not reset by repeated init",
          robotos_core_admission_accepted_count() == accepted_before);

    /* =========================================================================
     * Final summary
     * ========================================================================= */

    printf("\n--- Core Critical Boundary Contract: %d passed, %d failed ---\n",
           s_pass, s_fail);
    return (s_fail == 0) ? 0 : 1;
}
