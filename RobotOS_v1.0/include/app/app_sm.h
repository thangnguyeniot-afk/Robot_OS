/* app_sm.h — Application State Machine (Application Layer)
 *
 * Higher-level SM that orchestrates boot → homing → run → error flow.
 * Distinct from robot_sm.h (Framework) — this one drives the
 * application lifecycle using adapter IPC primitives.
 */
#ifndef ROBOTOS_APP_SM_H
#define ROBOTOS_APP_SM_H

#include <robotos/ro_status.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Application states ─────────────────────────────────────── */
typedef enum {
    APP_STATE_BOOT    = 0,
    APP_STATE_IDLE    = 1,
    APP_STATE_HOMING  = 2,
    APP_STATE_READY   = 3,
    APP_STATE_RUNNING = 4,
    APP_STATE_PAUSED  = 5,
    APP_STATE_ERROR   = 6,
    APP_STATE_COUNT
} app_state_t;

/* ── Application commands ───────────────────────────────────── */
typedef enum {
    APP_CMD_INIT_DONE     = 0,
    APP_CMD_HOME          = 1,
    APP_CMD_HOMING_DONE   = 2,
    APP_CMD_RUN           = 3,
    APP_CMD_STOP          = 4,
    APP_CMD_PAUSE         = 5,
    APP_CMD_RESUME        = 6,
    APP_CMD_FAULT         = 7,
    APP_CMD_FAULT_CLEARED = 8,
    APP_CMD_QUEUE_EMPTY   = 9,
    APP_CMD_COUNT
} app_cmd_t;

/* Transition callback */
typedef void (*app_sm_transition_cb_t)(app_state_t from,
                                        app_state_t to,
                                        app_cmd_t   cmd,
                                        void*       user_data);

/* Opaque handle */
typedef struct app_sm app_sm_t;

/* ── API ────────────────────────────────────────────────────── */
app_sm_t*   app_sm_create(void);
void        app_sm_destroy(app_sm_t* sm);

ro_status_t app_sm_dispatch(app_sm_t* sm, app_cmd_t cmd);
app_state_t app_sm_get_state(const app_sm_t* sm);
const char* app_sm_state_name(app_state_t state);

ro_status_t app_sm_set_transition_cb(app_sm_t* sm,
                                      app_sm_transition_cb_t cb,
                                      void* user_data);

#ifdef __cplusplus
}
#endif
#endif /* ROBOTOS_APP_SM_H */
