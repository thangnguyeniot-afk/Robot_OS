/*
 * test_robotos_platform_critical_contract.c
 * Phase 5D — Platform Critical Section / ISR Lock Boundary Contract Tests
 *
 * Validates the robotos_platform_critical API using the host stub:
 *   - enter/exit counters track calls correctly
 *   - token.opaque changes monotonically
 *   - nesting depth tracked correctly, max_depth observed
 *   - extra exit at depth=0 does not underflow
 *   - reset clears all state
 *   - public header compiles without Zephyr or board types
 *
 * Core event APIs are NOT tested here — they remain single-threaded in Phase 5D.
 * This critical boundary is NOT wired into core queue/post_event yet.
 *
 * No Zephyr. No hardware. Fresh static state.
 */

#include "robotos_platform_critical.h"
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

int main(void)
{
    printf("=== Phase 5D: Platform Critical Section Contract Tests ===\n\n");

    robotos_platform_critical_token_t tok, tok2;

    /* =========================================================================
     * TC01: Initial counts are zero after explicit reset
     * ========================================================================= */

    robotos_platform_critical_host_reset();
    CHECK("TC01 initial enter_count == 0",
          robotos_platform_critical_host_enter_count() == 0u);
    CHECK("TC01 initial exit_count == 0",
          robotos_platform_critical_host_exit_count() == 0u);
    CHECK("TC01 initial current_depth == 0",
          robotos_platform_critical_host_current_depth() == 0u);
    CHECK("TC01 initial max_depth == 0",
          robotos_platform_critical_host_max_depth() == 0u);

    /* =========================================================================
     * TC02: enter() increments enter_count and current_depth
     * ========================================================================= */

    tok = robotos_platform_critical_enter();
    CHECK("TC02 enter_count == 1",
          robotos_platform_critical_host_enter_count() == 1u);
    CHECK("TC02 current_depth == 1",
          robotos_platform_critical_host_current_depth() == 1u);
    CHECK("TC02 max_depth == 1",
          robotos_platform_critical_host_max_depth() == 1u);

    /* =========================================================================
     * TC03: token.opaque is nonzero (stub assigns monotonically increasing id)
     * ========================================================================= */

    CHECK("TC03 token.opaque is nonzero", tok.opaque != 0u);

    /* =========================================================================
     * TC04: exit() increments exit_count and decrements current_depth
     * ========================================================================= */

    robotos_platform_critical_exit(tok);
    CHECK("TC04 exit_count == 1",
          robotos_platform_critical_host_exit_count() == 1u);
    CHECK("TC04 current_depth == 0 after exit",
          robotos_platform_critical_host_current_depth() == 0u);
    CHECK("TC04 max_depth still 1",
          robotos_platform_critical_host_max_depth() == 1u);

    /* =========================================================================
     * TC05: Nested enter twice -> depth=2, max_depth=2
     * ========================================================================= */

    tok  = robotos_platform_critical_enter();
    tok2 = robotos_platform_critical_enter();
    CHECK("TC05 nested depth == 2",
          robotos_platform_critical_host_current_depth() == 2u);
    CHECK("TC05 max_depth == 2",
          robotos_platform_critical_host_max_depth() == 2u);
    CHECK("TC05 enter_count == 3 (one from TC02, two from TC05)",
          robotos_platform_critical_host_enter_count() == 3u);

    /* =========================================================================
     * TC06: token.opaque changes monotonically (tok2 > tok)
     * ========================================================================= */

    CHECK("TC06 token opaque values are distinct (tok2 > tok)",
          tok2.opaque > tok.opaque);

    /* =========================================================================
     * TC07: Nested exit restores depth to 0
     * ========================================================================= */

    robotos_platform_critical_exit(tok2);
    CHECK("TC07 depth == 1 after first exit",
          robotos_platform_critical_host_current_depth() == 1u);
    robotos_platform_critical_exit(tok);
    CHECK("TC07 depth == 0 after second exit",
          robotos_platform_critical_host_current_depth() == 0u);
    CHECK("TC07 exit_count == 3",
          robotos_platform_critical_host_exit_count() == 3u);

    /* =========================================================================
     * TC08: Extra exit at depth=0 does not underflow current_depth
     * ========================================================================= */

    robotos_platform_critical_token_t dummy = { .opaque = 0u };
    robotos_platform_critical_exit(dummy);
    CHECK("TC08 extra exit at depth=0 does not underflow (depth stays 0)",
          robotos_platform_critical_host_current_depth() == 0u);
    CHECK("TC08 exit_count incremented for extra exit",
          robotos_platform_critical_host_exit_count() == 4u);

    /* =========================================================================
     * TC09: Repeated reset clears all counters
     * ========================================================================= */

    robotos_platform_critical_host_reset();
    CHECK("TC09 reset: enter_count == 0",
          robotos_platform_critical_host_enter_count() == 0u);
    CHECK("TC09 reset: exit_count == 0",
          robotos_platform_critical_host_exit_count() == 0u);
    CHECK("TC09 reset: current_depth == 0",
          robotos_platform_critical_host_current_depth() == 0u);
    CHECK("TC09 reset: max_depth == 0",
          robotos_platform_critical_host_max_depth() == 0u);

    /* =========================================================================
     * TC10: Enter/exit sequence 10 times leaves depth=0 and counts equal
     * ========================================================================= */

    for (unsigned int i = 0u; i < 10u; i++) {
        robotos_platform_critical_token_t t = robotos_platform_critical_enter();
        robotos_platform_critical_exit(t);
    }
    CHECK("TC10 enter_count == 10",
          robotos_platform_critical_host_enter_count() == 10u);
    CHECK("TC10 exit_count == 10",
          robotos_platform_critical_host_exit_count() == 10u);
    CHECK("TC10 current_depth == 0",
          robotos_platform_critical_host_current_depth() == 0u);
    CHECK("TC10 max_depth == 1 (non-nested)",
          robotos_platform_critical_host_max_depth() == 1u);

    /* =========================================================================
     * TC11: Sanity — token type size uses uintptr_t (no Zephyr types needed)
     * ========================================================================= */

    CHECK("TC11 token struct size == sizeof(uintptr_t)",
          sizeof(robotos_platform_critical_token_t) == sizeof(uintptr_t));

    /* =========================================================================
     * TC12: Max depth tracks deepest nesting
     * ========================================================================= */

    robotos_platform_critical_host_reset();
    robotos_platform_critical_token_t t1 = robotos_platform_critical_enter();
    robotos_platform_critical_token_t t2 = robotos_platform_critical_enter();
    robotos_platform_critical_token_t t3 = robotos_platform_critical_enter();
    CHECK("TC12 depth == 3 after three nested enters",
          robotos_platform_critical_host_current_depth() == 3u);
    CHECK("TC12 max_depth == 3",
          robotos_platform_critical_host_max_depth() == 3u);
    robotos_platform_critical_exit(t3);
    robotos_platform_critical_exit(t2);
    robotos_platform_critical_exit(t1);
    CHECK("TC12 depth == 0 after all exits",
          robotos_platform_critical_host_current_depth() == 0u);
    CHECK("TC12 max_depth remains 3 after exits",
          robotos_platform_critical_host_max_depth() == 3u);

    /* =========================================================================
     * Final summary
     * ========================================================================= */

    printf("\n--- Critical Section Contract: %d passed, %d failed ---\n",
           s_pass, s_fail);
    return (s_fail == 0) ? 0 : 1;
}
