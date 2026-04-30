/* robot_sm.c — Robot State Machine Implementation (Framework Layer)
 *
 * Table-driven finite state machine.
 * Transition table:
 *   IDLE   + HOME             → HOMING
 *   HOMING + HOMING_COMPLETE  → IDLE   (ready)
 *   IDLE   + START            → RUN
 *   RUN    + STOP             → IDLE
 *   *      + EMERGENCY_STOP   → FAULT
 *   *      + FAULT            → FAULT
 *   FAULT  + (cleared via sm_clear_fault) → IDLE
 */
#include <robotos/robot_sm.h>
#include <robotos/ro_log.h>
#include <stdlib.h>
#include <string.h>

/* ── Transition table ───────────────────────────────────────── */
#define INVALID  ((robot_state_t)-1)

/*   [current_state][command] → next_state                      */
static const robot_state_t s_table[ROBOT_STATE_COUNT][ROBOT_CMD_COUNT] = {
    /* IDLE */
    [ROBOT_STATE_IDLE] = {
        [ROBOT_CMD_HOME]            = ROBOT_STATE_HOMING,
        [ROBOT_CMD_START]           = ROBOT_STATE_RUN,
        [ROBOT_CMD_STOP]            = INVALID,
        [ROBOT_CMD_EMERGENCY_STOP]  = ROBOT_STATE_FAULT,
        [ROBOT_CMD_FAULT]           = ROBOT_STATE_FAULT,
        [ROBOT_CMD_HOMING_COMPLETE] = INVALID,
    },
    /* HOMING */
    [ROBOT_STATE_HOMING] = {
        [ROBOT_CMD_HOME]            = INVALID,
        [ROBOT_CMD_START]           = INVALID,
        [ROBOT_CMD_STOP]            = ROBOT_STATE_IDLE,
        [ROBOT_CMD_EMERGENCY_STOP]  = ROBOT_STATE_FAULT,
        [ROBOT_CMD_FAULT]           = ROBOT_STATE_FAULT,
        [ROBOT_CMD_HOMING_COMPLETE] = ROBOT_STATE_IDLE,
    },
    /* RUN */
    [ROBOT_STATE_RUN] = {
        [ROBOT_CMD_HOME]            = INVALID,
        [ROBOT_CMD_START]           = INVALID,
        [ROBOT_CMD_STOP]            = ROBOT_STATE_IDLE,
        [ROBOT_CMD_EMERGENCY_STOP]  = ROBOT_STATE_FAULT,
        [ROBOT_CMD_FAULT]           = ROBOT_STATE_FAULT,
        [ROBOT_CMD_HOMING_COMPLETE] = INVALID,
    },
    /* FAULT */
    [ROBOT_STATE_FAULT] = {
        [ROBOT_CMD_HOME]            = INVALID,
        [ROBOT_CMD_START]           = INVALID,
        [ROBOT_CMD_STOP]            = INVALID,
        [ROBOT_CMD_EMERGENCY_STOP]  = ROBOT_STATE_FAULT,
        [ROBOT_CMD_FAULT]           = ROBOT_STATE_FAULT,
        [ROBOT_CMD_HOMING_COMPLETE] = INVALID,
    },
};

/* ── Internal struct ────────────────────────────────────────── */
struct sm {
    robot_state_t       state;
    int32_t             fault_reason;
    sm_transition_cb_t  cb;
    void*               cb_arg;
};

/* State name look-up */
static const char* s_names[ROBOT_STATE_COUNT] = {
    "IDLE", "HOMING", "RUN", "FAULT"
};

/* ── API ────────────────────────────────────────────────────── */

sm_t* sm_create(void)
{
    sm_t* sm = (sm_t*)calloc(1, sizeof(sm_t));
    if (sm) {
        sm->state = ROBOT_STATE_IDLE;
    }
    return sm;
}

void sm_destroy(sm_t* sm)
{
    free(sm);
}

ro_status_t sm_dispatch(sm_t* sm, robot_cmd_t cmd)
{
    if (!sm) return RO_EINVAL;
    if ((int)cmd < 0 || cmd >= ROBOT_CMD_COUNT) return RO_EINVAL;

    robot_state_t next = s_table[sm->state][cmd];
    if ((int)next == (int)INVALID) {
        RO_LOG_WARN("SM: invalid cmd %d in state %s",
                     (int)cmd, s_names[sm->state]);
        return RO_EINVAL;
    }

    robot_state_t prev = sm->state;
    sm->state = next;

    RO_LOG_INFO("SM: %s --%d--> %s", s_names[prev], (int)cmd, s_names[next]);

    if (sm->cb) {
        sm->cb(prev, next, cmd, sm->cb_arg);
    }
    return RO_OK;
}

robot_state_t sm_get_state(const sm_t* sm)
{
    return sm ? sm->state : ROBOT_STATE_FAULT;
}

const char* sm_state_name(robot_state_t state)
{
    if ((int)state < 0 || state >= ROBOT_STATE_COUNT) return "?";
    return s_names[state];
}

ro_status_t sm_set_fault_reason(sm_t* sm, int32_t reason)
{
    if (!sm) return RO_EINVAL;
    sm->fault_reason = reason;
    return RO_OK;
}

int32_t sm_get_fault_reason(const sm_t* sm)
{
    return sm ? sm->fault_reason : 0;
}

ro_status_t sm_clear_fault(sm_t* sm)
{
    if (!sm) return RO_EINVAL;
    if (sm->state != ROBOT_STATE_FAULT) return RO_EINVAL;
    sm->fault_reason = 0;
    robot_state_t prev = sm->state;
    sm->state = ROBOT_STATE_IDLE;
    if (sm->cb) {
        sm->cb(prev, ROBOT_STATE_IDLE, ROBOT_CMD_STOP /* pseudo */, sm->cb_arg);
    }
    return RO_OK;
}

ro_status_t sm_set_transition_cb(sm_t* sm, sm_transition_cb_t cb, void* user_data)
{
    if (!sm) return RO_EINVAL;
    sm->cb     = cb;
    sm->cb_arg = user_data;
    return RO_OK;
}
