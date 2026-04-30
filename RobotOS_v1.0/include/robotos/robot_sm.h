/* robot_sm.h — Robot State Machine (Framework Layer)
 *
 * Generic table-driven state machine used by the Framework layer.
 * The Application layer has its own app_sm (app_sm.h) built on top
 * of the adapter primitives.
 *
 * Naming: sm_* (no ro_ prefix — Framework convention)
 */
#ifndef ROBOTOS_ROBOT_SM_H
#define ROBOTOS_ROBOT_SM_H

#include <robotos/ro_status.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── States & Commands ──────────────────────────────────────── */
typedef enum {
    ROBOT_STATE_IDLE            = 0,
    ROBOT_STATE_HOMING          = 1,
    ROBOT_STATE_RUN             = 2,
    ROBOT_STATE_FAULT           = 3,
    ROBOT_STATE_COUNT
} robot_state_t;

typedef enum {
    ROBOT_CMD_HOME              = 0,
    ROBOT_CMD_START             = 1,
    ROBOT_CMD_STOP              = 2,
    ROBOT_CMD_EMERGENCY_STOP    = 3,
    ROBOT_CMD_FAULT             = 4,
    ROBOT_CMD_HOMING_COMPLETE   = 5,
    ROBOT_CMD_COUNT
} robot_cmd_t;

/* ── Transition callback ────────────────────────────────────── */
typedef void (*sm_transition_cb_t)(robot_state_t from,
                                    robot_state_t to,
                                    robot_cmd_t   cmd,
                                    void*         user_data);

/* ── Opaque SM handle ───────────────────────────────────────── */
typedef struct sm sm_t;

/* ── API ────────────────────────────────────────────────────── */
sm_t*       sm_create(void);
void        sm_destroy(sm_t* sm);

/* Dispatch a command — returns RO_OK on valid transition,
 * RO_EINVAL if the transition is not defined. */
ro_status_t sm_dispatch(sm_t* sm, robot_cmd_t cmd);

robot_state_t sm_get_state(const sm_t* sm);
const char*   sm_state_name(robot_state_t state);

/* Fault helpers */
ro_status_t sm_set_fault_reason(sm_t* sm, int32_t reason);
int32_t     sm_get_fault_reason(const sm_t* sm);
ro_status_t sm_clear_fault(sm_t* sm);

/* Optional transition observer */
ro_status_t sm_set_transition_cb(sm_t* sm, sm_transition_cb_t cb, void* user_data);

#ifdef __cplusplus
}
#endif
#endif /* ROBOTOS_ROBOT_SM_H */
