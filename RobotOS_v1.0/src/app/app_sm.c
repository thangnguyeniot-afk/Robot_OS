/* app_sm.c — Application State Machine Implementation
 *
 * Table-driven FSM with 7 states × 10 commands.
 */
#include "../../include/app/app_sm.h"
#include <robotos/ro_log.h>
#include <stdlib.h>
#include <string.h>

#define INVALID ((app_state_t)-1)

/* ── Transition table [state][cmd] → next_state ─────────────── */
static const app_state_t s_table[APP_STATE_COUNT][APP_CMD_COUNT] = {
    /* BOOT */
    [APP_STATE_BOOT] = {
        [APP_CMD_INIT_DONE]     = APP_STATE_IDLE,
        [APP_CMD_HOME]          = INVALID,
        [APP_CMD_HOMING_DONE]   = INVALID,
        [APP_CMD_RUN]           = INVALID,
        [APP_CMD_STOP]          = INVALID,
        [APP_CMD_PAUSE]         = INVALID,
        [APP_CMD_RESUME]        = INVALID,
        [APP_CMD_FAULT]         = APP_STATE_ERROR,
        [APP_CMD_FAULT_CLEARED] = INVALID,
        [APP_CMD_QUEUE_EMPTY]   = INVALID,
    },
    /* IDLE */
    [APP_STATE_IDLE] = {
        [APP_CMD_INIT_DONE]     = INVALID,
        [APP_CMD_HOME]          = APP_STATE_HOMING,
        [APP_CMD_HOMING_DONE]   = INVALID,
        [APP_CMD_RUN]           = INVALID,
        [APP_CMD_STOP]          = INVALID,
        [APP_CMD_PAUSE]         = INVALID,
        [APP_CMD_RESUME]        = INVALID,
        [APP_CMD_FAULT]         = APP_STATE_ERROR,
        [APP_CMD_FAULT_CLEARED] = INVALID,
        [APP_CMD_QUEUE_EMPTY]   = INVALID,
    },
    /* HOMING */
    [APP_STATE_HOMING] = {
        [APP_CMD_INIT_DONE]     = INVALID,
        [APP_CMD_HOME]          = INVALID,
        [APP_CMD_HOMING_DONE]   = APP_STATE_READY,
        [APP_CMD_RUN]           = INVALID,
        [APP_CMD_STOP]          = APP_STATE_IDLE,
        [APP_CMD_PAUSE]         = INVALID,
        [APP_CMD_RESUME]        = INVALID,
        [APP_CMD_FAULT]         = APP_STATE_ERROR,
        [APP_CMD_FAULT_CLEARED] = INVALID,
        [APP_CMD_QUEUE_EMPTY]   = INVALID,
    },
    /* READY */
    [APP_STATE_READY] = {
        [APP_CMD_INIT_DONE]     = INVALID,
        [APP_CMD_HOME]          = APP_STATE_HOMING,
        [APP_CMD_HOMING_DONE]   = INVALID,
        [APP_CMD_RUN]           = APP_STATE_RUNNING,
        [APP_CMD_STOP]          = APP_STATE_IDLE,
        [APP_CMD_PAUSE]         = INVALID,
        [APP_CMD_RESUME]        = INVALID,
        [APP_CMD_FAULT]         = APP_STATE_ERROR,
        [APP_CMD_FAULT_CLEARED] = INVALID,
        [APP_CMD_QUEUE_EMPTY]   = INVALID,
    },
    /* RUNNING */
    [APP_STATE_RUNNING] = {
        [APP_CMD_INIT_DONE]     = INVALID,
        [APP_CMD_HOME]          = INVALID,
        [APP_CMD_HOMING_DONE]   = INVALID,
        [APP_CMD_RUN]           = INVALID,
        [APP_CMD_STOP]          = APP_STATE_READY,
        [APP_CMD_PAUSE]         = APP_STATE_PAUSED,
        [APP_CMD_RESUME]        = INVALID,
        [APP_CMD_FAULT]         = APP_STATE_ERROR,
        [APP_CMD_FAULT_CLEARED] = INVALID,
        [APP_CMD_QUEUE_EMPTY]   = APP_STATE_READY,
    },
    /* PAUSED */
    [APP_STATE_PAUSED] = {
        [APP_CMD_INIT_DONE]     = INVALID,
        [APP_CMD_HOME]          = INVALID,
        [APP_CMD_HOMING_DONE]   = INVALID,
        [APP_CMD_RUN]           = INVALID,
        [APP_CMD_STOP]          = APP_STATE_READY,
        [APP_CMD_PAUSE]         = INVALID,
        [APP_CMD_RESUME]        = APP_STATE_RUNNING,
        [APP_CMD_FAULT]         = APP_STATE_ERROR,
        [APP_CMD_FAULT_CLEARED] = INVALID,
        [APP_CMD_QUEUE_EMPTY]   = INVALID,
    },
    /* ERROR */
    [APP_STATE_ERROR] = {
        [APP_CMD_INIT_DONE]     = INVALID,
        [APP_CMD_HOME]          = INVALID,
        [APP_CMD_HOMING_DONE]   = INVALID,
        [APP_CMD_RUN]           = INVALID,
        [APP_CMD_STOP]          = INVALID,
        [APP_CMD_PAUSE]         = INVALID,
        [APP_CMD_RESUME]        = INVALID,
        [APP_CMD_FAULT]         = APP_STATE_ERROR,
        [APP_CMD_FAULT_CLEARED] = APP_STATE_IDLE,
        [APP_CMD_QUEUE_EMPTY]   = INVALID,
    },
};

static const char* s_names[APP_STATE_COUNT] = {
    "BOOT", "IDLE", "HOMING", "READY", "RUNNING", "PAUSED", "ERROR"
};

/* ── Internal struct ────────────────────────────────────────── */
struct app_sm {
    app_state_t            state;
    app_sm_transition_cb_t cb;
    void*                  cb_arg;
};

/* ── API ────────────────────────────────────────────────────── */

app_sm_t* app_sm_create(void)
{
    app_sm_t* sm = (app_sm_t*)calloc(1, sizeof(app_sm_t));
    if (sm) {
        sm->state = APP_STATE_BOOT;
    }
    return sm;
}

void app_sm_destroy(app_sm_t* sm)
{
    free(sm);
}

ro_status_t app_sm_dispatch(app_sm_t* sm, app_cmd_t cmd)
{
    if (!sm) return RO_EINVAL;
    if ((int)cmd < 0 || cmd >= APP_CMD_COUNT) return RO_EINVAL;

    app_state_t next = s_table[sm->state][cmd];
    if ((int)next == (int)INVALID) {
        RO_LOG_WARN("APP-SM: invalid cmd %d in state %s",
                     (int)cmd, s_names[sm->state]);
        return RO_EINVAL;
    }

    app_state_t prev = sm->state;
    sm->state = next;

    RO_LOG_INFO("APP-SM: %s --%d--> %s",
                 s_names[prev], (int)cmd, s_names[next]);

    if (sm->cb) {
        sm->cb(prev, next, cmd, sm->cb_arg);
    }
    return RO_OK;
}

app_state_t app_sm_get_state(const app_sm_t* sm)
{
    return sm ? sm->state : APP_STATE_ERROR;
}

const char* app_sm_state_name(app_state_t state)
{
    if ((int)state < 0 || state >= APP_STATE_COUNT) return "?";
    return s_names[state];
}

ro_status_t app_sm_set_transition_cb(app_sm_t* sm,
                                      app_sm_transition_cb_t cb,
                                      void* user_data)
{
    if (!sm) return RO_EINVAL;
    sm->cb     = cb;
    sm->cb_arg = user_data;
    return RO_OK;
}
