/* test_app_sm.c — Unit Tests for Application State Machine */
#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "../../include/app/app_sm.h"

#define TEST(name)  static void name(void)
#define RUN(name)   do { printf("  %-40s", #name); name(); printf("PASS\n"); } while(0)

/* ── Callback tracking ──────────────────────────────────────── */
static int s_cb_count = 0;
static app_state_t s_cb_from, s_cb_to;
static app_cmd_t   s_cb_cmd;

static void test_cb(app_state_t from, app_state_t to, app_cmd_t cmd, void* arg)
{
    (void)arg;
    s_cb_count++;
    s_cb_from = from;
    s_cb_to   = to;
    s_cb_cmd  = cmd;
}

/* ── Tests ──────────────────────────────────────────────────── */

TEST(test_initial_state_is_boot)
{
    app_sm_t* sm = app_sm_create();
    assert(app_sm_get_state(sm) == APP_STATE_BOOT);
    app_sm_destroy(sm);
}

TEST(test_boot_to_idle)
{
    app_sm_t* sm = app_sm_create();
    assert(app_sm_dispatch(sm, APP_CMD_INIT_DONE) == RO_OK);
    assert(app_sm_get_state(sm) == APP_STATE_IDLE);
    app_sm_destroy(sm);
}

TEST(test_full_lifecycle)
{
    app_sm_t* sm = app_sm_create();

    /* BOOT → IDLE */
    assert(app_sm_dispatch(sm, APP_CMD_INIT_DONE) == RO_OK);
    assert(app_sm_get_state(sm) == APP_STATE_IDLE);

    /* IDLE → HOMING */
    assert(app_sm_dispatch(sm, APP_CMD_HOME) == RO_OK);
    assert(app_sm_get_state(sm) == APP_STATE_HOMING);

    /* HOMING → READY */
    assert(app_sm_dispatch(sm, APP_CMD_HOMING_DONE) == RO_OK);
    assert(app_sm_get_state(sm) == APP_STATE_READY);

    /* READY → RUNNING */
    assert(app_sm_dispatch(sm, APP_CMD_RUN) == RO_OK);
    assert(app_sm_get_state(sm) == APP_STATE_RUNNING);

    /* RUNNING → PAUSED */
    assert(app_sm_dispatch(sm, APP_CMD_PAUSE) == RO_OK);
    assert(app_sm_get_state(sm) == APP_STATE_PAUSED);

    /* PAUSED → RUNNING */
    assert(app_sm_dispatch(sm, APP_CMD_RESUME) == RO_OK);
    assert(app_sm_get_state(sm) == APP_STATE_RUNNING);

    /* RUNNING → READY (queue empty) */
    assert(app_sm_dispatch(sm, APP_CMD_QUEUE_EMPTY) == RO_OK);
    assert(app_sm_get_state(sm) == APP_STATE_READY);

    app_sm_destroy(sm);
}

TEST(test_fault_from_any_state)
{
    app_state_t states[] = {
        APP_STATE_BOOT, APP_STATE_IDLE, APP_STATE_HOMING,
        APP_STATE_READY, APP_STATE_RUNNING, APP_STATE_PAUSED
    };

    for (int i = 0; i < 6; i++) {
        app_sm_t* sm = app_sm_create();
        /* Navigate to the target state */
        switch (states[i]) {
        case APP_STATE_BOOT:    break;
        case APP_STATE_IDLE:    app_sm_dispatch(sm, APP_CMD_INIT_DONE); break;
        case APP_STATE_HOMING:
            app_sm_dispatch(sm, APP_CMD_INIT_DONE);
            app_sm_dispatch(sm, APP_CMD_HOME);
            break;
        case APP_STATE_READY:
            app_sm_dispatch(sm, APP_CMD_INIT_DONE);
            app_sm_dispatch(sm, APP_CMD_HOME);
            app_sm_dispatch(sm, APP_CMD_HOMING_DONE);
            break;
        case APP_STATE_RUNNING:
            app_sm_dispatch(sm, APP_CMD_INIT_DONE);
            app_sm_dispatch(sm, APP_CMD_HOME);
            app_sm_dispatch(sm, APP_CMD_HOMING_DONE);
            app_sm_dispatch(sm, APP_CMD_RUN);
            break;
        case APP_STATE_PAUSED:
            app_sm_dispatch(sm, APP_CMD_INIT_DONE);
            app_sm_dispatch(sm, APP_CMD_HOME);
            app_sm_dispatch(sm, APP_CMD_HOMING_DONE);
            app_sm_dispatch(sm, APP_CMD_RUN);
            app_sm_dispatch(sm, APP_CMD_PAUSE);
            break;
        default: break;
        }

        assert(app_sm_get_state(sm) == states[i]);
        assert(app_sm_dispatch(sm, APP_CMD_FAULT) == RO_OK);
        assert(app_sm_get_state(sm) == APP_STATE_ERROR);

        app_sm_destroy(sm);
    }
}

TEST(test_fault_cleared_to_idle)
{
    app_sm_t* sm = app_sm_create();
    app_sm_dispatch(sm, APP_CMD_INIT_DONE);
    app_sm_dispatch(sm, APP_CMD_FAULT);
    assert(app_sm_get_state(sm) == APP_STATE_ERROR);

    assert(app_sm_dispatch(sm, APP_CMD_FAULT_CLEARED) == RO_OK);
    assert(app_sm_get_state(sm) == APP_STATE_IDLE);

    app_sm_destroy(sm);
}

TEST(test_invalid_transition_returns_einval)
{
    app_sm_t* sm = app_sm_create();
    /* BOOT + HOME should be invalid */
    assert(app_sm_dispatch(sm, APP_CMD_HOME) == RO_EINVAL);
    assert(app_sm_get_state(sm) == APP_STATE_BOOT);
    app_sm_destroy(sm);
}

TEST(test_transition_callback)
{
    app_sm_t* sm = app_sm_create();
    s_cb_count = 0;
    app_sm_set_transition_cb(sm, test_cb, NULL);

    app_sm_dispatch(sm, APP_CMD_INIT_DONE);
    assert(s_cb_count == 1);
    assert(s_cb_from == APP_STATE_BOOT);
    assert(s_cb_to == APP_STATE_IDLE);
    assert(s_cb_cmd == APP_CMD_INIT_DONE);

    app_sm_destroy(sm);
}

TEST(test_state_name)
{
    assert(strcmp(app_sm_state_name(APP_STATE_BOOT), "BOOT") == 0);
    assert(strcmp(app_sm_state_name(APP_STATE_IDLE), "IDLE") == 0);
    assert(strcmp(app_sm_state_name(APP_STATE_RUNNING), "RUNNING") == 0);
    assert(strcmp(app_sm_state_name(APP_STATE_ERROR), "ERROR") == 0);
}

/* ── Runner ─────────────────────────────────────────────────── */

int main(void)
{
    printf("test_app_sm:\n");

    RUN(test_initial_state_is_boot);
    RUN(test_boot_to_idle);
    RUN(test_full_lifecycle);
    RUN(test_fault_from_any_state);
    RUN(test_fault_cleared_to_idle);
    RUN(test_invalid_transition_returns_einval);
    RUN(test_transition_callback);
    RUN(test_state_name);

    printf("All app_sm tests passed!\n");
    return 0;
}
