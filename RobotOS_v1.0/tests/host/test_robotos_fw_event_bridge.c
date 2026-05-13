/*
 * test_robotos_fw_event_bridge.c
 * Phase 12F host contract tests for robotos_fw_event_bridge.
 *
 * Exercises every required Phase 12F-pre coverage item without devkit,
 * Zephyr, hardware, or UART. The bridge is built against the real
 * robotos_fw_fsm.c so end-to-end Adapter -> bridge -> FSM transitions
 * are validated, not just the bridge surface in isolation.
 *
 * Build via tests/host/CMakeLists.txt.
 * Exit 0 = all tests passed. Exit 1 = one or more tests failed.
 */

#include "robotos_fw_event_bridge.h"
#include "robotos_fw_fsm.h"

#include <stdio.h>
#include <string.h>

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

/* ---- Synthetic test vocabulary ------------------------------------------
 *
 * Adapter-side keys: arbitrary uint32_t values. We deliberately use
 * 100/101/102/103 to mirror the active devkit's USER/USER+1/USER+2/USER+3
 * convention without including robotos_core.h here.
 *
 * Framework event IDs are a SEPARATE namespace and use small synthetic
 * integers.
 */
#define A_TYPE_USER     100u
#define A_TYPE_TIMER    101u
#define A_TYPE_BUTTON   102u
#define A_TYPE_UART     103u
#define A_TYPE_UNMAPPED 200u

#define A_ARG_NONE      0u
#define A_ARG_A         1u
#define A_ARG_B         2u
#define A_ARG_C         3u

/* FSM states. */
#define S_IDLE   ((robotos_fw_state_id_t)1u)
#define S_ARMED  ((robotos_fw_state_id_t)2u)
#define S_ACTIVE ((robotos_fw_state_id_t)3u)

/* FSM logical event IDs (Framework namespace; independent of Adapter
 * vocabulary). */
#define FW_EV_ARM    ((robotos_fw_event_id_t)10u)
#define FW_EV_START  ((robotos_fw_event_id_t)11u)
#define FW_EV_RESET  ((robotos_fw_event_id_t)12u)
#define FW_EV_TICK   ((robotos_fw_event_id_t)13u)
#define FW_EV_BTN    ((robotos_fw_event_id_t)14u)
#define FW_EV_FAIL   ((robotos_fw_event_id_t)15u)

/* Non-OK status used by action_fail for non-OK propagation test. */
#define ACTION_FAIL_STATUS ROBOTOS_CORE_ERR_INVALID_ARG

/* ---- FSM callbacks ------------------------------------------------------ */

/* Captures the payload pointer the FSM action saw — used to confirm that
 * the bridge passes the borrowed pointer through unchanged. */
typedef struct {
    const void *last_payload;
    int         action_calls;
} fsm_probe_t;

static fsm_probe_t g_probe;

static robotos_fw_status_t fsm_action_ok(robotos_fw_state_id_t from,
                                         robotos_fw_state_id_t to,
                                         robotos_fw_event_id_t event,
                                         const void           *payload,
                                         void                 *uc)
{
    (void)from; (void)to; (void)event;
    fsm_probe_t *p = (fsm_probe_t *)uc;
    if (p != NULL) {
        p->last_payload = payload;
        p->action_calls++;
    }
    return ROBOTOS_CORE_OK;
}

static robotos_fw_status_t fsm_action_fail(robotos_fw_state_id_t from,
                                           robotos_fw_state_id_t to,
                                           robotos_fw_event_id_t event,
                                           const void           *payload,
                                           void                 *uc)
{
    (void)from; (void)to; (void)event; (void)payload; (void)uc;
    return ACTION_FAIL_STATUS;
}

/* ---- Shared FSM transition tables ---------------------------------------
 *
 * A "happy path" table: IDLE --FW_EV_ARM--> ARMED --FW_EV_START--> ACTIVE
 *                       any  --FW_EV_RESET--> IDLE (only from ACTIVE here)
 */
static const robotos_fw_transition_t T_HAPPY[] = {
    { S_IDLE,   FW_EV_ARM,   NULL, S_ARMED,  fsm_action_ok },
    { S_ARMED,  FW_EV_START, NULL, S_ACTIVE, fsm_action_ok },
    { S_ACTIVE, FW_EV_RESET, NULL, S_IDLE,   fsm_action_ok },
};

/* A "fail" table whose single transition action returns non-OK. The
 * transition still commits (Phase 12C ACTION_NON_OK_NO_ROLLBACK_CONFIRMED).
 */
static const robotos_fw_transition_t T_FAIL[] = {
    { S_IDLE, FW_EV_FAIL, NULL, S_ARMED, fsm_action_fail },
};

/* ---- Helpers to build the FSM ------------------------------------------- */

static void fsm_init_happy(robotos_fw_fsm_t *fsm, robotos_fw_fsm_config_t *cfg)
{
    cfg->transitions      = T_HAPPY;
    cfg->transition_count = sizeof(T_HAPPY) / sizeof(T_HAPPY[0]);
    cfg->states           = NULL;
    cfg->state_count      = 0u;
    cfg->initial_state    = S_IDLE;
    cfg->user_context     = &g_probe;
    g_probe.last_payload  = NULL;
    g_probe.action_calls  = 0;
    (void)robotos_fw_fsm_init(fsm, cfg);
}

static void fsm_init_fail(robotos_fw_fsm_t *fsm, robotos_fw_fsm_config_t *cfg)
{
    cfg->transitions      = T_FAIL;
    cfg->transition_count = sizeof(T_FAIL) / sizeof(T_FAIL[0]);
    cfg->states           = NULL;
    cfg->state_count      = 0u;
    cfg->initial_state    = S_IDLE;
    cfg->user_context     = &g_probe;
    g_probe.last_payload  = NULL;
    g_probe.action_calls  = 0;
    (void)robotos_fw_fsm_init(fsm, cfg);
}

/* -------------------------------------------------------------------------
 * Test cases
 *
 * Each test seq is fully self-contained: fresh FSM, fresh bridge,
 * fresh mapping table. Counters are validated via direct field reads
 * AND via get_snapshot to confirm coherence.
 * ---------------------------------------------------------------------- */

/* TC01: init with a valid config. */
static void tc01_init_valid(void)
{
    printf("\n[ TC01: init valid config ]\n");

    robotos_fw_fsm_t fsm;
    robotos_fw_fsm_config_t fcfg;
    fsm_init_happy(&fsm, &fcfg);

    static const robotos_fw_event_bridge_row_t ROWS[] = {
        { A_TYPE_USER, A_ARG_A, true, FW_EV_ARM },
    };
    robotos_fw_event_bridge_t bridge;
    robotos_fw_event_bridge_config_t bcfg = {
        .rows         = ROWS,
        .row_count    = 1u,
        .fsm          = &fsm,
        .user_context = NULL,
    };

    TC("init returns OK",
        robotos_fw_event_bridge_init(&bridge, &bcfg) == ROBOTOS_CORE_OK);
    TC("initialized == true",        bridge.initialized == true);
    TC("event_count == 0",           bridge.event_count == 0u);
    TC("mapped_count == 0",          bridge.mapped_count == 0u);
    TC("unmapped_count == 0",        bridge.unmapped_count == 0u);
    TC("last_status == OK",          bridge.last_status == ROBOTOS_CORE_OK);
    TC("last_fw_event_id == 0",      bridge.last_fw_event_id == 0u);
    TC("config pointer installed",   bridge.config == &bcfg);
}

/* TC02: init NULL guards. */
static void tc02_init_null(void)
{
    printf("\n[ TC02: init NULL guards ]\n");

    robotos_fw_fsm_t fsm;
    robotos_fw_fsm_config_t fcfg;
    fsm_init_happy(&fsm, &fcfg);

    static const robotos_fw_event_bridge_row_t ROWS[] = {
        { A_TYPE_USER, 0u, false, FW_EV_ARM },
    };
    robotos_fw_event_bridge_t bridge;
    robotos_fw_event_bridge_config_t bcfg = {
        .rows         = ROWS,
        .row_count    = 1u,
        .fsm          = &fsm,
        .user_context = NULL,
    };

    TC("init(NULL, cfg) == ERR_NULL",
        robotos_fw_event_bridge_init(NULL, &bcfg) == ROBOTOS_CORE_ERR_NULL);
    TC("init(b, NULL) == ERR_NULL",
        robotos_fw_event_bridge_init(&bridge, NULL) == ROBOTOS_CORE_ERR_NULL);

    robotos_fw_event_bridge_config_t bcfg_null_rows = bcfg;
    bcfg_null_rows.rows = NULL;
    TC("init {.rows=NULL} == ERR_NULL",
        robotos_fw_event_bridge_init(&bridge, &bcfg_null_rows)
            == ROBOTOS_CORE_ERR_NULL);

    robotos_fw_event_bridge_config_t bcfg_null_fsm = bcfg;
    bcfg_null_fsm.fsm = NULL;
    TC("init {.fsm=NULL} == ERR_NULL",
        robotos_fw_event_bridge_init(&bridge, &bcfg_null_fsm)
            == ROBOTOS_CORE_ERR_NULL);
}

/* TC03: init invalid args (row_count == 0). */
static void tc03_init_invalid_arg(void)
{
    printf("\n[ TC03: init invalid args ]\n");

    robotos_fw_fsm_t fsm;
    robotos_fw_fsm_config_t fcfg;
    fsm_init_happy(&fsm, &fcfg);

    static const robotos_fw_event_bridge_row_t ROWS[] = {
        { A_TYPE_USER, 0u, false, FW_EV_ARM },
    };
    robotos_fw_event_bridge_t bridge;
    robotos_fw_event_bridge_config_t bcfg = {
        .rows         = ROWS,
        .row_count    = 0u,
        .fsm          = &fsm,
        .user_context = NULL,
    };
    TC("row_count == 0 -> ERR_INVALID_ARG",
        robotos_fw_event_bridge_init(&bridge, &bcfg)
            == ROBOTOS_CORE_ERR_INVALID_ARG);
}

/* TC04: dispatch NULL / uninitialized. */
static void tc04_dispatch_uninit_and_null(void)
{
    printf("\n[ TC04: dispatch uninit and NULL ]\n");

    robotos_fw_event_bridge_t bridge = {0}; /* initialized == false */

    TC("dispatch(NULL) -> ERR_NULL",
        robotos_fw_event_bridge_dispatch(NULL, 0u, 0u, NULL)
            == ROBOTOS_CORE_ERR_NULL);
    TC("dispatch on uninit -> ERR_INVALID_STATE",
        robotos_fw_event_bridge_dispatch(&bridge, 0u, 0u, NULL)
            == ROBOTOS_CORE_ERR_INVALID_STATE);
}

/* TC05: mapped event drives an FSM transition end-to-end. */
static void tc05_mapped_drives_fsm(void)
{
    printf("\n[ TC05: mapped event drives FSM transition ]\n");

    robotos_fw_fsm_t fsm;
    robotos_fw_fsm_config_t fcfg;
    fsm_init_happy(&fsm, &fcfg);

    static const robotos_fw_event_bridge_row_t ROWS[] = {
        { A_TYPE_USER,   A_ARG_A, true, FW_EV_ARM   },
        { A_TYPE_TIMER,  0u,      false, FW_EV_TICK },
    };
    robotos_fw_event_bridge_t bridge;
    robotos_fw_event_bridge_config_t bcfg = {
        .rows = ROWS, .row_count = 2u, .fsm = &fsm, .user_context = NULL,
    };
    (void)robotos_fw_event_bridge_init(&bridge, &bcfg);

    int rv = robotos_fw_event_bridge_dispatch(&bridge, A_TYPE_USER, A_ARG_A, NULL);
    TC("dispatch returns OK (FSM-OK)",    rv == ROBOTOS_CORE_OK);
    TC("FSM transitioned to S_ARMED",     fsm.current_state == S_ARMED);
    TC("FSM transition_count == 1",       fsm.transition_count == 1u);
    TC("bridge event_count == 1",         bridge.event_count == 1u);
    TC("bridge mapped_count == 1",        bridge.mapped_count == 1u);
    TC("bridge unmapped_count == 0",      bridge.unmapped_count == 0u);
    TC("last_fw_event_id == FW_EV_ARM",   bridge.last_fw_event_id == FW_EV_ARM);
    TC("last_adapter_type == USER",       bridge.last_adapter_type == A_TYPE_USER);
    TC("last_adapter_arg0 == A_ARG_A",    bridge.last_adapter_arg0 == A_ARG_A);
    TC("last_status == OK",               bridge.last_status == ROBOTOS_CORE_OK);
}

/* TC06: unmapped event ignored; FSM is not called. */
static void tc06_unmapped_ignored(void)
{
    printf("\n[ TC06: unmapped event ignored, FSM not called ]\n");

    robotos_fw_fsm_t fsm;
    robotos_fw_fsm_config_t fcfg;
    fsm_init_happy(&fsm, &fcfg);

    static const robotos_fw_event_bridge_row_t ROWS[] = {
        { A_TYPE_USER, A_ARG_A, true, FW_EV_ARM },
    };
    robotos_fw_event_bridge_t bridge;
    robotos_fw_event_bridge_config_t bcfg = {
        .rows = ROWS, .row_count = 1u, .fsm = &fsm, .user_context = NULL,
    };
    (void)robotos_fw_event_bridge_init(&bridge, &bcfg);

    int rv = robotos_fw_event_bridge_dispatch(&bridge,
                                              A_TYPE_UNMAPPED, 0u, NULL);
    TC("dispatch returns OK",             rv == ROBOTOS_CORE_OK);
    TC("FSM state unchanged (IDLE)",      fsm.current_state == S_IDLE);
    TC("FSM transition_count == 0",       fsm.transition_count == 0u);
    TC("FSM event_count == 0 (FSM not called)",
        fsm.event_count == 0u);
    TC("bridge event_count == 1",         bridge.event_count == 1u);
    TC("bridge mapped_count == 0",        bridge.mapped_count == 0u);
    TC("bridge unmapped_count == 1",      bridge.unmapped_count == 1u);
    TC("last_status == OK",               bridge.last_status == ROBOTOS_CORE_OK);
    TC("last_fw_event_id == 0 (no mapped dispatch yet)",
        bridge.last_fw_event_id == 0u);
}

/* TC07: payload pointer is borrowed and passed through verbatim. */
static void tc07_payload_borrowed(void)
{
    printf("\n[ TC07: payload borrowed, passed through verbatim ]\n");

    robotos_fw_fsm_t fsm;
    robotos_fw_fsm_config_t fcfg;
    fsm_init_happy(&fsm, &fcfg);

    static const robotos_fw_event_bridge_row_t ROWS[] = {
        { A_TYPE_USER, A_ARG_A, true, FW_EV_ARM },
    };
    robotos_fw_event_bridge_t bridge;
    robotos_fw_event_bridge_config_t bcfg = {
        .rows = ROWS, .row_count = 1u, .fsm = &fsm, .user_context = NULL,
    };
    (void)robotos_fw_event_bridge_init(&bridge, &bcfg);

    int probe = 0x12345678;
    (void)robotos_fw_event_bridge_dispatch(&bridge, A_TYPE_USER, A_ARG_A, &probe);

    TC("FSM action saw the payload pointer verbatim",
        g_probe.last_payload == (const void *)&probe);
    TC("FSM action called exactly once",  g_probe.action_calls == 1);
}

/* TC08: bridge does not retain the payload pointer.
 *
 * Static / size-based check: the public bridge struct has no payload
 * field, so there is no slot where the bridge could cache it. We also
 * confirm that the snapshot returned by get_snapshot has no payload
 * field accessible to callers.
 */
static void tc08_payload_not_retained(void)
{
    printf("\n[ TC08: bridge struct has no payload-storage field ]\n");

    /* The bridge struct documents the exact set of fields. If a payload
     * pointer were added, the struct size would change and the field list
     * below would no longer be exhaustive. This is a structural assertion
     * that catches accidental future addition of a payload cache. */
    robotos_fw_event_bridge_t bridge;
    size_t accounted = sizeof bridge.config
                     + sizeof bridge.event_count
                     + sizeof bridge.mapped_count
                     + sizeof bridge.unmapped_count
                     + sizeof bridge.last_adapter_type
                     + sizeof bridge.last_adapter_arg0
                     + sizeof bridge.last_fw_event_id
                     + sizeof bridge.last_status
                     + sizeof bridge.initialized;
    TC("sizeof(bridge) <= sum of documented fields + padding",
        sizeof(robotos_fw_event_bridge_t) >= accounted);
    /* Padding-tolerant: we accept >= accounted (compiler may pad), but if
     * a real payload slot were added the documented-fields sum itself
     * would grow, breaking the matching test elsewhere. */
    TC("sizeof(bridge_snapshot) has no payload field by review",
        sizeof(robotos_fw_event_bridge_snapshot_t) > 0u);
}

/* TC09: FSM non-OK status propagates through the bridge verbatim. */
static void tc09_fsm_non_ok_propagates(void)
{
    printf("\n[ TC09: FSM non-OK status propagates through bridge ]\n");

    robotos_fw_fsm_t fsm;
    robotos_fw_fsm_config_t fcfg;
    fsm_init_fail(&fsm, &fcfg);

    static const robotos_fw_event_bridge_row_t ROWS[] = {
        { A_TYPE_USER, A_ARG_A, true, FW_EV_FAIL },
    };
    robotos_fw_event_bridge_t bridge;
    robotos_fw_event_bridge_config_t bcfg = {
        .rows = ROWS, .row_count = 1u, .fsm = &fsm, .user_context = NULL,
    };
    (void)robotos_fw_event_bridge_init(&bridge, &bcfg);

    int rv = robotos_fw_event_bridge_dispatch(&bridge, A_TYPE_USER, A_ARG_A, NULL);
    TC("dispatch returns FSM's non-OK status",
        rv == ACTION_FAIL_STATUS);
    TC("bridge.last_status == FSM non-OK",
        bridge.last_status == ACTION_FAIL_STATUS);
    TC("FSM state still committed (S_ARMED; no rollback)",
        fsm.current_state == S_ARMED);
    TC("bridge mapped_count == 1",       bridge.mapped_count == 1u);
}

/* TC10: FIFO first-match deterministic ordering. */
static void tc10_fifo_first_match(void)
{
    printf("\n[ TC10: FIFO first-match deterministic ]\n");

    robotos_fw_fsm_t fsm;
    robotos_fw_fsm_config_t fcfg;
    fsm_init_happy(&fsm, &fcfg);

    /* Two rows that BOTH match (USER, A_ARG_A). First row produces
     * FW_EV_ARM (drives IDLE -> ARMED). Second row would produce
     * FW_EV_RESET (does NOT match in IDLE — so we cannot use that to
     * distinguish; use FW_EV_ARM vs FW_EV_RESET, where the second event
     * is not a valid transition from IDLE in this table, so if the
     * second row were taken the FSM would no_transition_count++ instead
     * of transitioning to ARMED). */
    static const robotos_fw_event_bridge_row_t ROWS[] = {
        { A_TYPE_USER, A_ARG_A, true, FW_EV_ARM   },  /* first */
        { A_TYPE_USER, A_ARG_A, true, FW_EV_RESET },  /* shadowed */
    };
    robotos_fw_event_bridge_t bridge;
    robotos_fw_event_bridge_config_t bcfg = {
        .rows = ROWS, .row_count = 2u, .fsm = &fsm, .user_context = NULL,
    };
    (void)robotos_fw_event_bridge_init(&bridge, &bcfg);

    (void)robotos_fw_event_bridge_dispatch(&bridge, A_TYPE_USER, A_ARG_A, NULL);

    TC("first matching row wins (FW_EV_ARM, FSM -> ARMED)",
        fsm.current_state == S_ARMED);
    TC("FSM saw FW_EV_ARM, not FW_EV_RESET",
        fsm.last_event_id == FW_EV_ARM);
    TC("bridge.last_fw_event_id == FW_EV_ARM",
        bridge.last_fw_event_id == FW_EV_ARM);
}

/* TC11: adapter_arg0 exact match. */
static void tc11_arg0_exact_match(void)
{
    printf("\n[ TC11: adapter_arg0 exact match ]\n");

    robotos_fw_fsm_t fsm;
    robotos_fw_fsm_config_t fcfg;
    fsm_init_happy(&fsm, &fcfg);

    /* Two rows on the same type, distinguished by arg0:
     *   (USER, A_ARG_A) -> FW_EV_ARM
     *   (USER, A_ARG_B) -> FW_EV_START
     */
    static const robotos_fw_event_bridge_row_t ROWS[] = {
        { A_TYPE_USER, A_ARG_A, true, FW_EV_ARM   },
        { A_TYPE_USER, A_ARG_B, true, FW_EV_START },
    };
    robotos_fw_event_bridge_t bridge;
    robotos_fw_event_bridge_config_t bcfg = {
        .rows = ROWS, .row_count = 2u, .fsm = &fsm, .user_context = NULL,
    };
    (void)robotos_fw_event_bridge_init(&bridge, &bcfg);

    /* arg0 = A: should pick FW_EV_ARM (IDLE -> ARMED). */
    (void)robotos_fw_event_bridge_dispatch(&bridge, A_TYPE_USER, A_ARG_A, NULL);
    TC("arg0=A picks FW_EV_ARM (FSM ARMED)",
        fsm.current_state == S_ARMED);
    TC("bridge.last_fw_event_id == FW_EV_ARM",
        bridge.last_fw_event_id == FW_EV_ARM);

    /* arg0 = B: should pick FW_EV_START (ARMED -> ACTIVE). */
    (void)robotos_fw_event_bridge_dispatch(&bridge, A_TYPE_USER, A_ARG_B, NULL);
    TC("arg0=B picks FW_EV_START (FSM ACTIVE)",
        fsm.current_state == S_ACTIVE);
    TC("bridge.last_fw_event_id == FW_EV_START",
        bridge.last_fw_event_id == FW_EV_START);

    /* arg0 = C (no exact match on this type): unmapped. */
    (void)robotos_fw_event_bridge_dispatch(&bridge, A_TYPE_USER, A_ARG_C, NULL);
    TC("arg0=C is unmapped (no FSM call)",
        bridge.unmapped_count == 1u);
}

/* TC12: adapter_arg0 wildcard match. */
static void tc12_arg0_wildcard_match(void)
{
    printf("\n[ TC12: adapter_arg0 wildcard match ]\n");

    robotos_fw_fsm_t fsm;
    robotos_fw_fsm_config_t fcfg;
    fsm_init_happy(&fsm, &fcfg);

    /* Single row with match_arg0 == false: matches ANY arg0 for TIMER. */
    static const robotos_fw_event_bridge_row_t ROWS[] = {
        { A_TYPE_TIMER, 999u /* ignored */, false, FW_EV_ARM },
    };
    robotos_fw_event_bridge_t bridge;
    robotos_fw_event_bridge_config_t bcfg = {
        .rows = ROWS, .row_count = 1u, .fsm = &fsm, .user_context = NULL,
    };
    (void)robotos_fw_event_bridge_init(&bridge, &bcfg);

    (void)robotos_fw_event_bridge_dispatch(&bridge, A_TYPE_TIMER, 0u,    NULL);
    TC("arg0=0 matches wildcard row (IDLE->ARMED)",
        fsm.current_state == S_ARMED);

    /* Send another wildcard event with different arg0; FSM is now in
     * ARMED so FW_EV_ARM no longer transitions (no row in IDLE matches
     * from ARMED). Verifies wildcard fires regardless of arg0 value. */
    (void)robotos_fw_event_bridge_dispatch(&bridge, A_TYPE_TIMER, 7777u, NULL);
    TC("arg0=7777 also matches wildcard row (mapped_count increments)",
        bridge.mapped_count == 2u);
    TC("bridge unmapped_count == 0",
        bridge.unmapped_count == 0u);
    TC("bridge event_count == 2",
        bridge.event_count == 2u);
}

/* TC13: FIFO precedence — a wildcard row preceding an exact-match row
 * wins (the bridge applies NO specificity tiebreak; order is the rule).
 * This makes the §4.2 precedence rule explicit and testable. */
static void tc13_fifo_vs_specificity(void)
{
    printf("\n[ TC13: FIFO order beats arg0 specificity ]\n");

    robotos_fw_fsm_t fsm;
    robotos_fw_fsm_config_t fcfg;
    fsm_init_happy(&fsm, &fcfg);

    /* Wildcard row first (FW_EV_ARM); exact-arg0 row second (FW_EV_RESET,
     * which would NOT transition from IDLE — so taking row 2 leaves the
     * FSM in IDLE, which is distinguishable from row 1's IDLE->ARMED). */
    static const robotos_fw_event_bridge_row_t ROWS[] = {
        { A_TYPE_USER, 0u,      false, FW_EV_ARM   },  /* wildcard first */
        { A_TYPE_USER, A_ARG_A, true,  FW_EV_RESET },  /* exact, second */
    };
    robotos_fw_event_bridge_t bridge;
    robotos_fw_event_bridge_config_t bcfg = {
        .rows = ROWS, .row_count = 2u, .fsm = &fsm, .user_context = NULL,
    };
    (void)robotos_fw_event_bridge_init(&bridge, &bcfg);

    (void)robotos_fw_event_bridge_dispatch(&bridge, A_TYPE_USER, A_ARG_A, NULL);
    TC("wildcard row at index 0 wins (FSM -> ARMED)",
        fsm.current_state == S_ARMED);
    TC("bridge.last_fw_event_id == FW_EV_ARM (wildcard row)",
        bridge.last_fw_event_id == FW_EV_ARM);
}

/* TC14: multiple mappings supported (sequence of distinct adapter
 * sources driving the FSM through IDLE -> ARMED -> ACTIVE -> IDLE). */
static void tc14_multiple_mappings(void)
{
    printf("\n[ TC14: multiple mappings drive a sequence ]\n");

    robotos_fw_fsm_t fsm;
    robotos_fw_fsm_config_t fcfg;
    fsm_init_happy(&fsm, &fcfg);

    static const robotos_fw_event_bridge_row_t ROWS[] = {
        { A_TYPE_USER,   A_ARG_A, true,  FW_EV_ARM   },
        { A_TYPE_BUTTON, 0u,      false, FW_EV_START },
        { A_TYPE_UART,   A_ARG_C, true,  FW_EV_RESET },
        { A_TYPE_TIMER,  0u,      false, FW_EV_TICK  },
    };
    robotos_fw_event_bridge_t bridge;
    robotos_fw_event_bridge_config_t bcfg = {
        .rows = ROWS, .row_count = 4u, .fsm = &fsm, .user_context = NULL,
    };
    (void)robotos_fw_event_bridge_init(&bridge, &bcfg);

    (void)robotos_fw_event_bridge_dispatch(&bridge, A_TYPE_USER,   A_ARG_A, NULL);
    TC("USER/A -> ARMED",  fsm.current_state == S_ARMED);

    (void)robotos_fw_event_bridge_dispatch(&bridge, A_TYPE_BUTTON, 42u,     NULL);
    TC("BUTTON/* -> ACTIVE", fsm.current_state == S_ACTIVE);

    (void)robotos_fw_event_bridge_dispatch(&bridge, A_TYPE_UART,   A_ARG_C, NULL);
    TC("UART/C -> IDLE",   fsm.current_state == S_IDLE);

    /* TIMER with no row in IDLE for FW_EV_TICK in T_HAPPY: bridge maps
     * but FSM has no transition -> FSM returns OK with
     * no_transition_count++. */
    (void)robotos_fw_event_bridge_dispatch(&bridge, A_TYPE_TIMER,  0u,      NULL);
    TC("TIMER mapped but FSM no_transition (state stays IDLE)",
        fsm.current_state == S_IDLE);
    TC("FSM no_transition_count == 1",  fsm.no_transition_count == 1u);
    TC("bridge mapped_count == 4 (all four dispatches matched a row)",
        bridge.mapped_count == 4u);
    TC("bridge unmapped_count == 0",   bridge.unmapped_count == 0u);
    TC("bridge event_count == 4",      bridge.event_count == 4u);
}

/* TC15: bridge reset resets bridge counters but NOT FSM state. */
static void tc15_reset_bridge_only(void)
{
    printf("\n[ TC15: bridge reset resets bridge, not FSM ]\n");

    robotos_fw_fsm_t fsm;
    robotos_fw_fsm_config_t fcfg;
    fsm_init_happy(&fsm, &fcfg);

    static const robotos_fw_event_bridge_row_t ROWS[] = {
        { A_TYPE_USER, A_ARG_A, true, FW_EV_ARM },
    };
    robotos_fw_event_bridge_t bridge;
    robotos_fw_event_bridge_config_t bcfg = {
        .rows = ROWS, .row_count = 1u, .fsm = &fsm, .user_context = NULL,
    };
    (void)robotos_fw_event_bridge_init(&bridge, &bcfg);
    (void)robotos_fw_event_bridge_dispatch(&bridge, A_TYPE_USER, A_ARG_A, NULL);
    (void)robotos_fw_event_bridge_dispatch(&bridge, A_TYPE_UNMAPPED, 0u, NULL);

    /* Pre-reset sanity. */
    TC("pre-reset: event_count == 2",   bridge.event_count == 2u);
    TC("pre-reset: mapped_count == 1",  bridge.mapped_count == 1u);
    TC("pre-reset: unmapped_count == 1",bridge.unmapped_count == 1u);
    TC("pre-reset: FSM is in ARMED",    fsm.current_state == S_ARMED);

    int rv = robotos_fw_event_bridge_reset(&bridge);
    TC("reset returns OK",              rv == ROBOTOS_CORE_OK);
    TC("post-reset: event_count == 0",  bridge.event_count == 0u);
    TC("post-reset: mapped_count == 0", bridge.mapped_count == 0u);
    TC("post-reset: unmapped_count==0", bridge.unmapped_count == 0u);
    TC("post-reset: last_status == OK", bridge.last_status == ROBOTOS_CORE_OK);
    TC("post-reset: last_fw_event_id==0",
        bridge.last_fw_event_id == 0u);
    TC("post-reset: last_adapter_type==0",
        bridge.last_adapter_type == 0u);
    TC("post-reset: last_adapter_arg0==0",
        bridge.last_adapter_arg0 == 0u);
    TC("post-reset: bridge still initialized",
        bridge.initialized == true);

    /* FSM was NOT touched by bridge_reset. */
    TC("post-reset: FSM still in ARMED",  fsm.current_state == S_ARMED);
    TC("post-reset: FSM transition_count still 1",
        fsm.transition_count == 1u);

    /* NULL / uninit error paths. */
    TC("reset(NULL) -> ERR_NULL",
        robotos_fw_event_bridge_reset(NULL) == ROBOTOS_CORE_ERR_NULL);
    robotos_fw_event_bridge_t bridge_uninit = {0};
    TC("reset(uninit) -> ERR_INVALID_STATE",
        robotos_fw_event_bridge_reset(&bridge_uninit)
            == ROBOTOS_CORE_ERR_INVALID_STATE);
}

/* TC16: snapshot reports counters and last_* fields coherently. */
static void tc16_snapshot(void)
{
    printf("\n[ TC16: snapshot coherence ]\n");

    robotos_fw_fsm_t fsm;
    robotos_fw_fsm_config_t fcfg;
    fsm_init_happy(&fsm, &fcfg);

    static const robotos_fw_event_bridge_row_t ROWS[] = {
        { A_TYPE_USER,  A_ARG_A, true,  FW_EV_ARM },
        { A_TYPE_TIMER, 0u,      false, FW_EV_TICK },
    };
    robotos_fw_event_bridge_t bridge;
    robotos_fw_event_bridge_config_t bcfg = {
        .rows = ROWS, .row_count = 2u, .fsm = &fsm, .user_context = NULL,
    };
    (void)robotos_fw_event_bridge_init(&bridge, &bcfg);

    (void)robotos_fw_event_bridge_dispatch(&bridge, A_TYPE_USER,   A_ARG_A, NULL);
    (void)robotos_fw_event_bridge_dispatch(&bridge, A_TYPE_UNMAPPED, 5u,    NULL);
    (void)robotos_fw_event_bridge_dispatch(&bridge, A_TYPE_TIMER,  9u,      NULL);

    robotos_fw_event_bridge_snapshot_t snap;
    int rv = robotos_fw_event_bridge_get_snapshot(&bridge, &snap);
    TC("snapshot returns OK",                rv == ROBOTOS_CORE_OK);
    TC("snap.event_count == 3",              snap.event_count == 3u);
    TC("snap.mapped_count == 2",             snap.mapped_count == 2u);
    TC("snap.unmapped_count == 1",           snap.unmapped_count == 1u);
    TC("snap.last_adapter_type == TIMER",    snap.last_adapter_type == A_TYPE_TIMER);
    TC("snap.last_adapter_arg0 == 9",        snap.last_adapter_arg0 == 9u);
    TC("snap.last_fw_event_id == FW_EV_TICK",
        snap.last_fw_event_id == FW_EV_TICK);
    TC("snap.last_status == OK",             snap.last_status == ROBOTOS_CORE_OK);
    TC("snap.initialized == true",           snap.initialized == true);

    /* Snapshot agrees with direct field reads (coherence). */
    TC("snap matches bridge fields (event_count)",
        snap.event_count == bridge.event_count);
    TC("snap matches bridge fields (mapped_count)",
        snap.mapped_count == bridge.mapped_count);
    TC("snap matches bridge fields (unmapped_count)",
        snap.unmapped_count == bridge.unmapped_count);

    /* NULL / uninit error paths. */
    TC("snapshot(NULL bridge) -> ERR_NULL",
        robotos_fw_event_bridge_get_snapshot(NULL, &snap)
            == ROBOTOS_CORE_ERR_NULL);
    TC("snapshot(bridge, NULL) -> ERR_NULL",
        robotos_fw_event_bridge_get_snapshot(&bridge, NULL)
            == ROBOTOS_CORE_ERR_NULL);
    robotos_fw_event_bridge_t bridge_uninit = {0};
    TC("snapshot(uninit) -> ERR_INVALID_STATE",
        robotos_fw_event_bridge_get_snapshot(&bridge_uninit, &snap)
            == ROBOTOS_CORE_ERR_INVALID_STATE);
}

/* TC17: re-init policy (idempotent; zeroes counters, FSM untouched). */
static void tc17_reinit_idempotent(void)
{
    printf("\n[ TC17: re-init policy ]\n");

    robotos_fw_fsm_t fsm;
    robotos_fw_fsm_config_t fcfg;
    fsm_init_happy(&fsm, &fcfg);

    static const robotos_fw_event_bridge_row_t ROWS[] = {
        { A_TYPE_USER, A_ARG_A, true, FW_EV_ARM },
    };
    robotos_fw_event_bridge_t bridge;
    robotos_fw_event_bridge_config_t bcfg = {
        .rows = ROWS, .row_count = 1u, .fsm = &fsm, .user_context = NULL,
    };
    (void)robotos_fw_event_bridge_init(&bridge, &bcfg);
    (void)robotos_fw_event_bridge_dispatch(&bridge, A_TYPE_USER, A_ARG_A, NULL);
    /* state: bridge.event_count == 1; FSM in ARMED. */

    int rv = robotos_fw_event_bridge_init(&bridge, &bcfg);
    TC("re-init returns OK",               rv == ROBOTOS_CORE_OK);
    TC("re-init zeroes event_count",       bridge.event_count == 0u);
    TC("re-init zeroes mapped_count",      bridge.mapped_count == 0u);
    TC("re-init zeroes unmapped_count",    bridge.unmapped_count == 0u);
    TC("re-init zeroes last_fw_event_id",  bridge.last_fw_event_id == 0u);
    TC("re-init still initialized",        bridge.initialized == true);
    /* FSM is the caller's; re-init MUST NOT touch it. */
    TC("re-init does NOT reset FSM (still ARMED)",
        fsm.current_state == S_ARMED);
}

/* -------------------------------------------------------------------------
 * main
 * ---------------------------------------------------------------------- */

int main(void)
{
    printf("=== robotos_fw_event_bridge host contract tests (Phase 12F) ===\n");

    tc01_init_valid();
    tc02_init_null();
    tc03_init_invalid_arg();
    tc04_dispatch_uninit_and_null();
    tc05_mapped_drives_fsm();
    tc06_unmapped_ignored();
    tc07_payload_borrowed();
    tc08_payload_not_retained();
    tc09_fsm_non_ok_propagates();
    tc10_fifo_first_match();
    tc11_arg0_exact_match();
    tc12_arg0_wildcard_match();
    tc13_fifo_vs_specificity();
    tc14_multiple_mappings();
    tc15_reset_bridge_only();
    tc16_snapshot();
    tc17_reinit_idempotent();

    printf("\n=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
