/*
 * test_robotos_core_contract.c
 * Phase 4C host contract tests for robotos_core.
 *
 * Tests the Phase 4B contract semantics without Zephyr or hardware.
 * Compile robotos_core.c with -DROBOTS_CORE_HOST_TEST=1 to suppress Zephyr logging.
 *
 * Build and run:
 *   cmake -S RobotOS_v1.0/tests/host -B build-host-core
 *   cmake --build build-host-core
 *   ./build-host-core/robotos_core_contract_test
 *
 * Exit code 0 = all tests passed.
 * Exit code 1 = one or more tests failed.
 */

#include "robotos_core.h"

#include <stdio.h>
#include <string.h>
#include <stdint.h>

/* ---- Minimal test harness ------------------------------------------------ */

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

/* -------------------------------------------------------------------------- */

int main(void)
{
    printf("=== robotos_core contract tests (Phase 4C) ===\n\n");

    robotos_core_snapshot_t snap;
    robotos_core_status_t   ret;

    /* ---- Pre-init state -------------------------------------------------- */
    printf("[ Pre-init ]\n");

    TC("TC01: version() != NULL",
        robotos_core_version() != NULL);

    TC("TC02: version() == \"4B-contract\"",
        strcmp(robotos_core_version(), "4B-contract") == 0);

    TC("TC03: initial state == UNINITIALIZED",
        robotos_core_state() == ROBOTOS_CORE_STATE_UNINITIALIZED);

    TC("TC04: initial tick_count() == 0",
        robotos_core_tick_count() == 0);

    TC("TC05: snapshot(NULL) returns ERR_NULL",
        robotos_core_snapshot(NULL) == ROBOTOS_CORE_ERR_NULL);

    TC("TC06: tick() before init returns ERR_INVALID_STATE",
        robotos_core_tick() == ROBOTOS_CORE_ERR_INVALID_STATE);

    TC("TC07: tick_count() unchanged after invalid tick",
        robotos_core_tick_count() == 0);

    ret = robotos_core_snapshot(&snap);
    TC("TC08: snapshot() before init returns OK",
        ret == ROBOTOS_CORE_OK);
    TC("TC09: snapshot.state == UNINITIALIZED before init",
        snap.state == ROBOTOS_CORE_STATE_UNINITIALIZED);
    TC("TC10: snapshot.tick_count == 0 before init",
        snap.tick_count == 0);
    TC("TC11: snapshot.init_count == 0 before init",
        snap.init_count == 0);
    TC("TC12: snapshot.version == current version string",
        snap.version != NULL && strcmp(snap.version, "4B-contract") == 0);

    /* ---- First init ----------------------------------------------------- */
    printf("\n[ First init ]\n");

    TC("TC13: first init() returns OK",
        robotos_core_init() == ROBOTOS_CORE_OK);
    TC("TC14: state == READY after init",
        robotos_core_state() == ROBOTOS_CORE_STATE_READY);
    TC("TC15: tick_count() == 0 after first init",
        robotos_core_tick_count() == 0);

    ret = robotos_core_snapshot(&snap);
    TC("TC16: snapshot.init_count == 1 after first init",
        ret == ROBOTOS_CORE_OK && snap.init_count == 1);
    TC("TC17: snapshot.state == READY after first init",
        snap.state == ROBOTOS_CORE_STATE_READY);

    /* ---- Tick after init ------------------------------------------------ */
    printf("\n[ Ticks after init ]\n");

    TC("TC18: first tick() returns OK",
        robotos_core_tick() == ROBOTOS_CORE_OK);
    TC("TC19: tick_count() == 1 after first tick",
        robotos_core_tick_count() == 1);

    TC("TC20: second tick() returns OK",
        robotos_core_tick() == ROBOTOS_CORE_OK);
    TC("TC21: tick_count() == 2 after second tick",
        robotos_core_tick_count() == 2);

    /* Advance to tick 10 to test monotonic increment */
    for (int i = 0; i < 8; i++) {
        robotos_core_tick();
    }
    TC("TC22: tick_count() == 10 after 10 total ticks",
        robotos_core_tick_count() == 10);

    /* ---- Second init idempotency ---------------------------------------- */
    printf("\n[ Second init (idempotency) ]\n");

    TC("TC23: second init() returns OK",
        robotos_core_init() == ROBOTOS_CORE_OK);
    TC("TC24: state still READY after second init",
        robotos_core_state() == ROBOTOS_CORE_STATE_READY);
    TC("TC25: tick_count() unchanged after second init (== 10)",
        robotos_core_tick_count() == 10);

    ret = robotos_core_snapshot(&snap);
    TC("TC26: snapshot.init_count == 2 after second init",
        ret == ROBOTOS_CORE_OK && snap.init_count == 2);
    TC("TC27: snapshot.tick_count unchanged == 10",
        snap.tick_count == 10);

    /* ---- Continue ticking after re-init --------------------------------- */
    printf("\n[ Ticks after re-init ]\n");

    TC("TC28: tick() after second init returns OK",
        robotos_core_tick() == ROBOTOS_CORE_OK);
    TC("TC29: tick_count() == 11 — monotonic after re-init",
        robotos_core_tick_count() == 11);

    /* Five more ticks — verify monotonic */
    uint32_t prev = robotos_core_tick_count();
    int monotonic_ok = 1;
    for (int i = 0; i < 5; i++) {
        robotos_core_tick();
        uint32_t cur = robotos_core_tick_count();
        if (cur != prev + 1) {
            monotonic_ok = 0;
        }
        prev = cur;
    }
    TC("TC30: tick_count() increments monotonically over 5 ticks",
        monotonic_ok);

    /* ---- Final snapshot ------------------------------------------------- */
    printf("\n[ Final snapshot ]\n");

    ret = robotos_core_snapshot(&snap);
    TC("TC31: final snapshot() returns OK",
        ret == ROBOTOS_CORE_OK);
    TC("TC32: final snapshot.state == READY",
        snap.state == ROBOTOS_CORE_STATE_READY);
    TC("TC33: final snapshot.tick_count == 16",
        snap.tick_count == 16);
    TC("TC34: final snapshot.init_count == 2",
        snap.init_count == 2);
    TC("TC35: final snapshot.version == \"4B-contract\"",
        snap.version != NULL && strcmp(snap.version, "4B-contract") == 0);

    /* ---- Summary -------------------------------------------------------- */
    printf("\n=== Results: %d passed, %d failed ===\n", g_pass, g_fail);

    return (g_fail > 0) ? 1 : 0;
}
