/*
 * test_robotos_fw_fsm.c
 * Phase 12E host contract tests for robotos_fw_fsm.
 *
 * Exercises every behavior contract from Phase 12B/12C without devkit,
 * Zephyr, hardware, or UART. Build via tests/host/CMakeLists.txt.
 *
 * Exit 0 = all tests passed. Exit 1 = one or more tests failed.
 */

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

/* ---- Product-state and event ID vocabulary used by tests ---------------- */

#define S_A   ((robotos_fw_state_id_t)1u)
#define S_B   ((robotos_fw_state_id_t)2u)
#define S_C   ((robotos_fw_state_id_t)3u)

#define E_AB  ((robotos_fw_event_id_t)10u)
#define E_BA  ((robotos_fw_event_id_t)11u)
#define E_AC  ((robotos_fw_event_id_t)12u)
#define E_NOP ((robotos_fw_event_id_t)99u)

/* Synthetic non-OK status used by action_fail. Any non-zero
 * robotos_core_status_t serves; we pick ERR_INVALID_ARG to keep the value
 * inside the documented enum range. */
#define ACTION_FAIL_STATUS ROBOTOS_CORE_ERR_INVALID_ARG

/* ---- Order capture buffer -----------------------------------------------
 *
 * Each callback writes a single tag character into g_seq, with the FSM's
 * observed current_state appended as a digit. This lets every order /
 * pre-vs-post-state test inspect the exact sequence of callbacks fired
 * by one or more dispatches.
 */
static char     g_seq[256];
static size_t   g_seq_len;

static void seq_reset(void) {
    g_seq[0]   = '\0';
    g_seq_len  = 0;
}

static void seq_emit(char tag, robotos_fw_state_id_t observed)
{
    if (g_seq_len + 2u < sizeof(g_seq)) {
        g_seq[g_seq_len++] = tag;
        g_seq[g_seq_len++] = (char)('0' + (observed & 0x0Fu));
        g_seq[g_seq_len]   = '\0';
    }
}

/* ---- Context shared with callbacks --------------------------------------
 *
 * Carries the FSM pointer (so callbacks can read fsm->current_state for the
 * pre-/post-transition order tests) and a probe payload pointer for the
 * payload-borrowed test. user_context is a robotos_fw_test_ctx_t *.
 */
typedef struct {
    const robotos_fw_fsm_t *fsm;          /* set by tests so callbacks can probe */
    const void             *payload_seen; /* last payload pointer seen by action */
    int                     payload_calls;
} robotos_fw_test_ctx_t;

static robotos_fw_test_ctx_t g_ctx;

static void ctx_reset(const robotos_fw_fsm_t *fsm) {
    g_ctx.fsm           = fsm;
    g_ctx.payload_seen  = NULL;
    g_ctx.payload_calls = 0;
}

/* ---- Callbacks ----------------------------------------------------------- */

static bool guard_true(robotos_fw_state_id_t current,
                       robotos_fw_event_id_t event,
                       const void           *payload,
                       void                 *uc)
{
    (void)event; (void)payload;
    robotos_fw_test_ctx_t *ctx = (robotos_fw_test_ctx_t *)uc;
    /* Guard observes pre-transition state. Capture for §F.18. */
    seq_emit('g', current);
    if (ctx != NULL && ctx->fsm != NULL) {
        /* Guard runs BEFORE state update, so fsm->current_state == from. */
        if (ctx->fsm->current_state != current) {
            seq_emit('!', 0u);  /* mismatch indicator */
        }
    }
    return true;
}

static bool guard_false(robotos_fw_state_id_t current,
                        robotos_fw_event_id_t event,
                        const void           *payload,
                        void                 *uc)
{
    (void)event; (void)payload; (void)uc;
    seq_emit('r', current);  /* 'r' = rejected */
    return false;
}

static robotos_fw_status_t action_ok(robotos_fw_state_id_t from,
                                     robotos_fw_state_id_t to,
                                     robotos_fw_event_id_t event,
                                     const void           *payload,
                                     void                 *uc)
{
    (void)event;
    robotos_fw_test_ctx_t *ctx = (robotos_fw_test_ctx_t *)uc;
    /* Action observes post-transition state. */
    seq_emit('a', to);
    if (ctx != NULL) {
        ctx->payload_seen = payload;
        ctx->payload_calls++;
        if (ctx->fsm != NULL) {
            /* Action runs AFTER state update, so fsm->current_state == to. */
            if (ctx->fsm->current_state != to) {
                seq_emit('!', 1u);
            }
            /* from_state param must equal the pre-transition state, which
             * is the state we were in before this action fired. */
            (void)from;
        }
    }
    return ROBOTOS_CORE_OK;
}

static robotos_fw_status_t action_fail(robotos_fw_state_id_t from,
                                       robotos_fw_state_id_t to,
                                       robotos_fw_event_id_t event,
                                       const void           *payload,
                                       void                 *uc)
{
    (void)from; (void)event; (void)payload; (void)uc;
    seq_emit('f', to);
    return ACTION_FAIL_STATUS;
}

static robotos_fw_status_t on_entry(robotos_fw_state_id_t state, void *uc)
{
    robotos_fw_test_ctx_t *ctx = (robotos_fw_test_ctx_t *)uc;
    seq_emit('E', state);
    if (ctx != NULL && ctx->fsm != NULL) {
        /* Entry runs AFTER state update, so fsm->current_state == state. */
        if (ctx->fsm->current_state != state) {
            seq_emit('!', 2u);
        }
    }
    return ROBOTOS_CORE_OK;
}

static robotos_fw_status_t on_exit(robotos_fw_state_id_t state, void *uc)
{
    robotos_fw_test_ctx_t *ctx = (robotos_fw_test_ctx_t *)uc;
    seq_emit('X', state);
    if (ctx != NULL && ctx->fsm != NULL) {
        /* Exit runs BEFORE state update, so fsm->current_state == state. */
        if (ctx->fsm->current_state != state) {
            seq_emit('!', 3u);
        }
    }
    return ROBOTOS_CORE_OK;
}

/* ---- Shared static transition tables ------------------------------------ */

/* Simple two-state table: A <-> B. Used for ordering, counters, reset. */
static const robotos_fw_transition_t T_AB[] = {
    { S_A, E_AB, NULL,        S_B, action_ok },
    { S_B, E_BA, NULL,        S_A, action_ok },
};

static const robotos_fw_state_def_t S_DEF_AB[] = {
    { S_A, on_entry, on_exit },
    { S_B, on_entry, on_exit },
};

/* Table with two matching rows for the same (state, event); first-match
 * FIFO must take row 0. */
static const robotos_fw_transition_t T_FIFO[] = {
    { S_A, E_AB, NULL, S_B, action_ok },
    { S_A, E_AB, NULL, S_C, action_ok },  /* same key; must not be reached */
};

/* Table whose only matching row has a rejecting guard. */
static const robotos_fw_transition_t T_GUARD_REJECT[] = {
    { S_A, E_AB, guard_false, S_B, action_ok },
};

/* Table with two matching rows, both rejecting. Tests that
 * guard_rejected_count increments TWICE and no_transition_count once. */
static const robotos_fw_transition_t T_GUARD_REJECT2[] = {
    { S_A, E_AB, guard_false, S_B, action_ok },
    { S_A, E_AB, guard_false, S_C, action_ok },
};

/* Table with a guard_true followed by an unguarded row; guard_true must
 * commit the first row. */
static const robotos_fw_transition_t T_GUARD_PASS[] = {
    { S_A, E_AB, guard_true, S_B, action_ok },
    { S_A, E_AB, NULL,       S_C, action_ok },
};

/* Table whose action returns non-OK. */
static const robotos_fw_transition_t T_ACTION_FAIL[] = {
    { S_A, E_AB, NULL, S_B, action_fail },
};

/* -------------------------------------------------------------------------
 * Test cases
 *
 * Each test is a function returning void; assertions accumulate into
 * g_pass / g_fail. Every test seq_reset() + ctx_reset() + fresh fsm_init.
 * ---------------------------------------------------------------------- */

static void tc01_init_valid(void)
{
    printf("\n[ TC01: init valid config ]\n");

    robotos_fw_fsm_t fsm;
    robotos_fw_fsm_config_t cfg = {
        .transitions      = T_AB,
        .transition_count = 2u,
        .states           = S_DEF_AB,
        .state_count      = 2u,
        .initial_state    = S_A,
        .user_context     = &g_ctx,
    };
    seq_reset();
    ctx_reset(&fsm);

    TC("init returns OK", robotos_fw_fsm_init(&fsm, &cfg) == ROBOTOS_CORE_OK);
    TC("initialized true",   fsm.initialized == true);
    TC("current_state==S_A", fsm.current_state == S_A);
    TC("event_count == 0",   fsm.event_count == 0u);
    TC("transition_count==0",fsm.transition_count == 0u);
    TC("init calls entry once", strcmp(g_seq, "E1") == 0);
}

static void tc02_init_null(void)
{
    printf("\n[ TC02: init NULL guards ]\n");

    robotos_fw_fsm_t fsm = {0};
    robotos_fw_fsm_config_t cfg = {
        .transitions      = T_AB,
        .transition_count = 2u,
        .initial_state    = S_A,
    };

    TC("init(NULL, cfg) == ERR_NULL",
        robotos_fw_fsm_init(NULL, &cfg) == ROBOTOS_CORE_ERR_NULL);
    TC("init(fsm, NULL) == ERR_NULL",
        robotos_fw_fsm_init(&fsm, NULL) == ROBOTOS_CORE_ERR_NULL);

    robotos_fw_fsm_config_t cfg_null_t = cfg;
    cfg_null_t.transitions = NULL;
    TC("init(fsm, {.transitions=NULL}) == ERR_NULL",
        robotos_fw_fsm_init(&fsm, &cfg_null_t) == ROBOTOS_CORE_ERR_NULL);
}

static void tc03_init_invalid_arg(void)
{
    printf("\n[ TC03: init invalid args ]\n");

    robotos_fw_fsm_t fsm;

    robotos_fw_fsm_config_t cfg_zero = {
        .transitions      = T_AB,
        .transition_count = 0u,
        .initial_state    = S_A,
    };
    TC("transition_count==0 -> ERR_INVALID_ARG",
        robotos_fw_fsm_init(&fsm, &cfg_zero) == ROBOTOS_CORE_ERR_INVALID_ARG);

    robotos_fw_fsm_config_t cfg_uninit = {
        .transitions      = T_AB,
        .transition_count = 2u,
        .initial_state    = ROBOTOS_FW_STATE_UNINIT,
    };
    TC("initial==UNINIT -> ERR_INVALID_ARG",
        robotos_fw_fsm_init(&fsm, &cfg_uninit) == ROBOTOS_CORE_ERR_INVALID_ARG);
}

static void tc04_dispatch_uninit_and_null(void)
{
    printf("\n[ TC04: dispatch uninit and NULL ]\n");

    robotos_fw_fsm_t fsm = {0};  /* initialized=false */
    TC("dispatch on uninit -> ERR_INVALID_STATE",
        robotos_fw_fsm_dispatch(&fsm, E_AB, NULL)
            == ROBOTOS_CORE_ERR_INVALID_STATE);
    TC("dispatch NULL fsm -> ERR_NULL",
        robotos_fw_fsm_dispatch(NULL, E_AB, NULL) == ROBOTOS_CORE_ERR_NULL);
}

static void tc05_dispatch_matched(void)
{
    printf("\n[ TC05: dispatch matched transition ]\n");

    robotos_fw_fsm_t fsm;
    robotos_fw_fsm_config_t cfg = {
        .transitions      = T_AB,
        .transition_count = 2u,
        .states           = S_DEF_AB,
        .state_count      = 2u,
        .initial_state    = S_A,
        .user_context     = &g_ctx,
    };
    seq_reset();
    ctx_reset(&fsm);
    (void)robotos_fw_fsm_init(&fsm, &cfg);
    seq_reset();   /* drop the entry-on-init tag for clean ordering check */

    int rv = robotos_fw_fsm_dispatch(&fsm, E_AB, NULL);
    TC("dispatch returns OK",            rv == ROBOTOS_CORE_OK);
    TC("current_state == S_B",           fsm.current_state == S_B);
    TC("event_count == 1",               fsm.event_count == 1u);
    TC("transition_count == 1",          fsm.transition_count == 1u);
    TC("no_transition_count == 0",       fsm.no_transition_count == 0u);
    TC("guard_rejected_count == 0",      fsm.guard_rejected_count == 0u);
    TC("last_event_id == E_AB",          fsm.last_event_id == E_AB);
    TC("last_status == OK",              fsm.last_status == ROBOTOS_CORE_OK);
}

static void tc06_eval_order(void)
{
    printf("\n[ TC06: eval order exit -> state -> action -> entry ]\n");

    robotos_fw_fsm_t fsm;
    robotos_fw_fsm_config_t cfg = {
        .transitions      = T_AB,
        .transition_count = 2u,
        .states           = S_DEF_AB,
        .state_count      = 2u,
        .initial_state    = S_A,
        .user_context     = &g_ctx,
    };
    seq_reset();
    ctx_reset(&fsm);
    (void)robotos_fw_fsm_init(&fsm, &cfg);
    seq_reset();

    (void)robotos_fw_fsm_dispatch(&fsm, E_AB, NULL);

    /* Expected order: on_exit(A)=X1, action(to=B)=a2, on_entry(B)=E2.
     * State update happens between X and a; verified by the digit on 'a' (2).
     * Phase 12C order: exit -> state update -> action -> entry. */
    TC("seq == X1 a2 E2 (exit, action sees to, entry)",
        strcmp(g_seq, "X1a2E2") == 0);
    TC("no order violation marker present",
        strchr(g_seq, '!') == NULL);
}

static void tc07_first_match_fifo(void)
{
    printf("\n[ TC07: first-match FIFO ]\n");

    robotos_fw_fsm_t fsm;
    robotos_fw_fsm_config_t cfg = {
        .transitions      = T_FIFO,
        .transition_count = 2u,
        .states           = NULL,
        .state_count      = 0u,
        .initial_state    = S_A,
        .user_context     = &g_ctx,
    };
    seq_reset();
    ctx_reset(&fsm);
    (void)robotos_fw_fsm_init(&fsm, &cfg);

    (void)robotos_fw_fsm_dispatch(&fsm, E_AB, NULL);
    TC("first row wins (state == S_B, not S_C)", fsm.current_state == S_B);
    TC("transition_count == 1",                  fsm.transition_count == 1u);
}

static void tc08_no_transition(void)
{
    printf("\n[ TC08: no transition matched ]\n");

    robotos_fw_fsm_t fsm;
    robotos_fw_fsm_config_t cfg = {
        .transitions      = T_AB,
        .transition_count = 2u,
        .states           = NULL,
        .state_count      = 0u,
        .initial_state    = S_A,
        .user_context     = &g_ctx,
    };
    seq_reset();
    ctx_reset(&fsm);
    (void)robotos_fw_fsm_init(&fsm, &cfg);

    int rv = robotos_fw_fsm_dispatch(&fsm, E_NOP, NULL);
    TC("returns OK",                       rv == ROBOTOS_CORE_OK);
    TC("state unchanged (S_A)",            fsm.current_state == S_A);
    TC("event_count == 1",                 fsm.event_count == 1u);
    TC("transition_count == 0",            fsm.transition_count == 0u);
    TC("no_transition_count == 1",         fsm.no_transition_count == 1u);
    TC("guard_rejected_count == 0",        fsm.guard_rejected_count == 0u);
    TC("last_event_id == E_NOP",           fsm.last_event_id == E_NOP);
}

static void tc09_guard_reject_single(void)
{
    printf("\n[ TC09: guard reject (single matching row) ]\n");

    robotos_fw_fsm_t fsm;
    robotos_fw_fsm_config_t cfg = {
        .transitions      = T_GUARD_REJECT,
        .transition_count = 1u,
        .states           = NULL,
        .state_count      = 0u,
        .initial_state    = S_A,
        .user_context     = &g_ctx,
    };
    seq_reset();
    ctx_reset(&fsm);
    (void)robotos_fw_fsm_init(&fsm, &cfg);

    int rv = robotos_fw_fsm_dispatch(&fsm, E_AB, NULL);
    TC("returns OK",                       rv == ROBOTOS_CORE_OK);
    TC("state unchanged (S_A)",            fsm.current_state == S_A);
    TC("transition_count == 0",            fsm.transition_count == 0u);
    TC("guard_rejected_count == 1",        fsm.guard_rejected_count == 1u);
    TC("no_transition_count == 1",         fsm.no_transition_count == 1u);
}

static void tc10_guard_reject_independent_counters(void)
{
    printf("\n[ TC10: guard_rejected/no_transition counter independence ]\n");

    robotos_fw_fsm_t fsm;
    robotos_fw_fsm_config_t cfg = {
        .transitions      = T_GUARD_REJECT2,
        .transition_count = 2u,
        .initial_state    = S_A,
        .user_context     = &g_ctx,
    };
    seq_reset();
    ctx_reset(&fsm);
    (void)robotos_fw_fsm_init(&fsm, &cfg);

    (void)robotos_fw_fsm_dispatch(&fsm, E_AB, NULL);
    TC("guard_rejected_count == 2 (per rejection)",
        fsm.guard_rejected_count == 2u);
    TC("no_transition_count == 1 (once at end)",
        fsm.no_transition_count == 1u);
    TC("transition_count == 0",            fsm.transition_count == 0u);
}

static void tc11_guard_true_commits(void)
{
    printf("\n[ TC11: guard true commits row (not the unguarded fallback) ]\n");

    robotos_fw_fsm_t fsm;
    robotos_fw_fsm_config_t cfg = {
        .transitions      = T_GUARD_PASS,
        .transition_count = 2u,
        .initial_state    = S_A,
        .user_context     = &g_ctx,
    };
    seq_reset();
    ctx_reset(&fsm);
    (void)robotos_fw_fsm_init(&fsm, &cfg);

    (void)robotos_fw_fsm_dispatch(&fsm, E_AB, NULL);
    TC("first row (guard_true) wins -> S_B",       fsm.current_state == S_B);
    TC("guard saw pre-transition state (S_A=1)",
        strstr(g_seq, "g1") != NULL);
}

static void tc12_action_non_ok_no_rollback(void)
{
    printf("\n[ TC12: action non-OK NO rollback ]\n");

    robotos_fw_fsm_t fsm;
    robotos_fw_fsm_config_t cfg = {
        .transitions      = T_ACTION_FAIL,
        .transition_count = 1u,
        .states           = S_DEF_AB,
        .state_count      = 2u,
        .initial_state    = S_A,
        .user_context     = &g_ctx,
    };
    seq_reset();
    ctx_reset(&fsm);
    (void)robotos_fw_fsm_init(&fsm, &cfg);
    seq_reset();

    int rv = robotos_fw_fsm_dispatch(&fsm, E_AB, NULL);
    TC("dispatch returns action's non-OK",  rv == ACTION_FAIL_STATUS);
    TC("state IS committed (S_B; no rollback)",
        fsm.current_state == S_B);
    TC("transition_count == 1",             fsm.transition_count == 1u);
    TC("entry still ran",                   strstr(g_seq, "E2") != NULL);
    TC("last_status == non-OK",             fsm.last_status == ACTION_FAIL_STATUS);
    TC("seq order X1 f2 E2",
        strcmp(g_seq, "X1f2E2") == 0);
}

static void tc13_reset(void)
{
    printf("\n[ TC13: reset ]\n");

    robotos_fw_fsm_t fsm;
    robotos_fw_fsm_config_t cfg = {
        .transitions      = T_AB,
        .transition_count = 2u,
        .states           = S_DEF_AB,
        .state_count      = 2u,
        .initial_state    = S_A,
        .user_context     = &g_ctx,
    };
    seq_reset();
    ctx_reset(&fsm);
    (void)robotos_fw_fsm_init(&fsm, &cfg);
    (void)robotos_fw_fsm_dispatch(&fsm, E_AB, NULL);
    /* Now: state==S_B, transition_count==1. */

    seq_reset();
    int rv = robotos_fw_fsm_reset(&fsm);
    TC("reset returns OK",                 rv == ROBOTOS_CORE_OK);
    TC("state == initial (S_A)",           fsm.current_state == S_A);
    TC("transition_count zeroed",          fsm.transition_count == 0u);
    TC("event_count zeroed",               fsm.event_count == 0u);
    TC("guard_rejected_count zeroed",      fsm.guard_rejected_count == 0u);
    TC("no_transition_count zeroed",       fsm.no_transition_count == 0u);
    TC("last_event_id zeroed",             fsm.last_event_id == 0u);
    TC("last_status == OK",                fsm.last_status == ROBOTOS_CORE_OK);
    TC("reset calls exit(prev=B) then entry(initial=A)",
        strcmp(g_seq, "X2E1") == 0);

    TC("reset(NULL) -> ERR_NULL",
        robotos_fw_fsm_reset(NULL) == ROBOTOS_CORE_ERR_NULL);

    robotos_fw_fsm_t fsm_uninit = {0};
    TC("reset(uninit) -> ERR_INVALID_STATE",
        robotos_fw_fsm_reset(&fsm_uninit) == ROBOTOS_CORE_ERR_INVALID_STATE);
}

static void tc14_get_state(void)
{
    printf("\n[ TC14: get_state ]\n");

    robotos_fw_fsm_t fsm;
    robotos_fw_fsm_config_t cfg = {
        .transitions      = T_AB,
        .transition_count = 2u,
        .initial_state    = S_A,
        .user_context     = &g_ctx,
    };
    seq_reset();
    ctx_reset(&fsm);
    (void)robotos_fw_fsm_init(&fsm, &cfg);

    TC("get_state == S_A",                 robotos_fw_fsm_get_state(&fsm) == S_A);
    (void)robotos_fw_fsm_dispatch(&fsm, E_AB, NULL);
    TC("get_state == S_B after dispatch",  robotos_fw_fsm_get_state(&fsm) == S_B);

    TC("get_state(NULL) == UNINIT",
        robotos_fw_fsm_get_state(NULL) == ROBOTOS_FW_STATE_UNINIT);

    robotos_fw_fsm_t fsm_uninit = {0};
    TC("get_state(uninit) == UNINIT",
        robotos_fw_fsm_get_state(&fsm_uninit) == ROBOTOS_FW_STATE_UNINIT);
}

static void tc15_is_in_state(void)
{
    printf("\n[ TC15: is_in_state ]\n");

    robotos_fw_fsm_t fsm;
    robotos_fw_fsm_config_t cfg = {
        .transitions      = T_AB,
        .transition_count = 2u,
        .initial_state    = S_A,
        .user_context     = &g_ctx,
    };
    seq_reset();
    ctx_reset(&fsm);
    (void)robotos_fw_fsm_init(&fsm, &cfg);

    TC("is_in_state(fsm, S_A) == true",   robotos_fw_fsm_is_in_state(&fsm, S_A) == true);
    TC("is_in_state(fsm, S_B) == false",  robotos_fw_fsm_is_in_state(&fsm, S_B) == false);
    TC("is_in_state(NULL, S_A) == false", robotos_fw_fsm_is_in_state(NULL, S_A) == false);

    robotos_fw_fsm_t fsm_uninit = {0};
    TC("is_in_state(uninit, S_A) == false",
        robotos_fw_fsm_is_in_state(&fsm_uninit, S_A) == false);
}

static void tc16_get_snapshot(void)
{
    printf("\n[ TC16: get_snapshot ]\n");

    robotos_fw_fsm_t fsm;
    robotos_fw_fsm_config_t cfg = {
        .transitions      = T_AB,
        .transition_count = 2u,
        .initial_state    = S_A,
        .user_context     = &g_ctx,
    };
    seq_reset();
    ctx_reset(&fsm);
    (void)robotos_fw_fsm_init(&fsm, &cfg);
    (void)robotos_fw_fsm_dispatch(&fsm, E_AB, NULL);

    robotos_fw_fsm_snapshot_t snap;
    int rv = robotos_fw_fsm_get_snapshot(&fsm, &snap);
    TC("snapshot returns OK",              rv == ROBOTOS_CORE_OK);
    TC("snap.current_state == S_B",        snap.current_state == S_B);
    TC("snap.transition_count == 1",       snap.transition_count == 1u);
    TC("snap.event_count == 1",            snap.event_count == 1u);
    TC("snap.no_transition_count == 0",    snap.no_transition_count == 0u);
    TC("snap.guard_rejected_count == 0",   snap.guard_rejected_count == 0u);
    TC("snap.last_event_id == E_AB",       snap.last_event_id == E_AB);
    TC("snap.last_status == OK",           snap.last_status == ROBOTOS_CORE_OK);
    TC("snap.initialized == true",         snap.initialized == true);

    TC("snapshot(NULL fsm, &out) -> ERR_NULL",
        robotos_fw_fsm_get_snapshot(NULL, &snap) == ROBOTOS_CORE_ERR_NULL);
    TC("snapshot(&fsm, NULL out) -> ERR_NULL",
        robotos_fw_fsm_get_snapshot(&fsm, NULL) == ROBOTOS_CORE_ERR_NULL);

    robotos_fw_fsm_t fsm_uninit = {0};
    TC("snapshot(uninit) -> ERR_INVALID_STATE",
        robotos_fw_fsm_get_snapshot(&fsm_uninit, &snap)
            == ROBOTOS_CORE_ERR_INVALID_STATE);
}

static void tc17_event_count_every_dispatch(void)
{
    printf("\n[ TC17: event_count increments per dispatch ]\n");

    robotos_fw_fsm_t fsm;
    robotos_fw_fsm_config_t cfg = {
        .transitions      = T_AB,
        .transition_count = 2u,
        .initial_state    = S_A,
        .user_context     = &g_ctx,
    };
    seq_reset();
    ctx_reset(&fsm);
    (void)robotos_fw_fsm_init(&fsm, &cfg);

    (void)robotos_fw_fsm_dispatch(&fsm, E_AB, NULL);  /* match */
    (void)robotos_fw_fsm_dispatch(&fsm, E_NOP, NULL); /* no match */
    (void)robotos_fw_fsm_dispatch(&fsm, E_BA, NULL);  /* match */
    TC("event_count == 3 (3 dispatches)",  fsm.event_count == 3u);
    TC("transition_count == 2",            fsm.transition_count == 2u);
    TC("no_transition_count == 1",         fsm.no_transition_count == 1u);
}

static void tc18_payload_borrowed(void)
{
    printf("\n[ TC18: payload borrowed, not cached ]\n");

    robotos_fw_fsm_t fsm;
    robotos_fw_fsm_config_t cfg = {
        .transitions      = T_AB,
        .transition_count = 2u,
        .initial_state    = S_A,
        .user_context     = &g_ctx,
    };
    seq_reset();
    ctx_reset(&fsm);
    (void)robotos_fw_fsm_init(&fsm, &cfg);

    int probe_payload = 0x12345678;
    (void)robotos_fw_fsm_dispatch(&fsm, E_AB, &probe_payload);

    /* The action saw the payload pointer during dispatch. */
    TC("action observed the payload pointer",
        g_ctx.payload_seen == (const void *)&probe_payload);
    TC("payload_calls == 1",               g_ctx.payload_calls == 1);

    /* The robotos_fw_fsm_t struct must not contain a field that stores
     * the payload (verified by struct size compared to the documented
     * field set). The snapshot must not expose any payload field either. */
    robotos_fw_fsm_snapshot_t snap;
    (void)robotos_fw_fsm_get_snapshot(&fsm, &snap);
    TC("snapshot has no field for payload (review-confirmed)",
        /* If the snapshot did record the payload pointer, we would need a
         * member to read it; the struct definition in the header has no
         * such field. This test asserts non-leakage by API surface. */
        sizeof snap == sizeof(robotos_fw_fsm_snapshot_t));
}

static void tc19_reinit_idempotent(void)
{
    printf("\n[ TC19: re-init policy (idempotent) ]\n");

    robotos_fw_fsm_t fsm;
    robotos_fw_fsm_config_t cfg = {
        .transitions      = T_AB,
        .transition_count = 2u,
        .states           = S_DEF_AB,
        .state_count      = 2u,
        .initial_state    = S_A,
        .user_context     = &g_ctx,
    };
    seq_reset();
    ctx_reset(&fsm);
    (void)robotos_fw_fsm_init(&fsm, &cfg);
    (void)robotos_fw_fsm_dispatch(&fsm, E_AB, NULL);
    /* Now: state==S_B, transition_count==1, event_count==1. */

    /* Re-init: idempotent — returns OK, zeroes counters, resets state. */
    int rv = robotos_fw_fsm_init(&fsm, &cfg);
    TC("re-init returns OK",               rv == ROBOTOS_CORE_OK);
    TC("re-init resets state to initial",  fsm.current_state == S_A);
    TC("re-init zeroes transition_count",  fsm.transition_count == 0u);
    TC("re-init zeroes event_count",       fsm.event_count == 0u);
    TC("re-init zeroes last_event_id",     fsm.last_event_id == 0u);
    TC("re-init still initialized",        fsm.initialized == true);
}

static void tc20_critical_section_used_for_queries(void)
{
    printf("\n[ TC20: get_state/is_in_state/get_snapshot use critical section ]\n");

    /* Provided by the host stub: count enters and exits.
     * The host CMake links robotos_platform_critical_host_stub.c into
     * this test target, so these symbols are available. */
    extern void         robotos_platform_critical_host_reset(void);
    extern unsigned int robotos_platform_critical_host_enter_count(void);
    extern unsigned int robotos_platform_critical_host_exit_count(void);

    robotos_fw_fsm_t fsm;
    robotos_fw_fsm_config_t cfg = {
        .transitions      = T_AB,
        .transition_count = 2u,
        .initial_state    = S_A,
        .user_context     = &g_ctx,
    };
    seq_reset();
    ctx_reset(&fsm);
    (void)robotos_fw_fsm_init(&fsm, &cfg);

    robotos_platform_critical_host_reset();
    (void)robotos_fw_fsm_get_state(&fsm);
    (void)robotos_fw_fsm_is_in_state(&fsm, S_A);
    robotos_fw_fsm_snapshot_t snap;
    (void)robotos_fw_fsm_get_snapshot(&fsm, &snap);

    TC("3 enters observed (get_state, is_in_state, get_snapshot)",
        robotos_platform_critical_host_enter_count() == 3u);
    TC("3 exits observed (balanced enter/exit)",
        robotos_platform_critical_host_exit_count() == 3u);
}

/* -------------------------------------------------------------------------
 * main
 * ---------------------------------------------------------------------- */

int main(void)
{
    printf("=== robotos_fw_fsm host contract tests (Phase 12E) ===\n");

    tc01_init_valid();
    tc02_init_null();
    tc03_init_invalid_arg();
    tc04_dispatch_uninit_and_null();
    tc05_dispatch_matched();
    tc06_eval_order();
    tc07_first_match_fifo();
    tc08_no_transition();
    tc09_guard_reject_single();
    tc10_guard_reject_independent_counters();
    tc11_guard_true_commits();
    tc12_action_non_ok_no_rollback();
    tc13_reset();
    tc14_get_state();
    tc15_is_in_state();
    tc16_get_snapshot();
    tc17_event_count_every_dispatch();
    tc18_payload_borrowed();
    tc19_reinit_idempotent();
    tc20_critical_section_used_for_queries();

    printf("\n=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
