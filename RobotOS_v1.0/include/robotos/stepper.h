/* ============================================================================
 * stepper.h — Stepper Motor API (Framework Layer)
 * ============================================================================
 * Layer: Framework (NO ro_ prefix — domain-semantic naming)
 * Depends on: Adapter APIs (ro_gpio, ro_timer, ro_queue) via .c file only
 * Public header: NO <zephyr/*> includes
 * ========================================================================= */

#ifndef ROBOTOS_STEPPER_H
#define ROBOTOS_STEPPER_H

#include <robotos/ro_status.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ---- Opaque handle ------------------------------------------------------- */

typedef struct stepper stepper_t;

/* ---- State --------------------------------------------------------------- */

typedef enum {
    STEPPER_STATE_IDLE         = 0,
    STEPPER_STATE_ACCELERATING = 1,
    STEPPER_STATE_RUNNING      = 2,
    STEPPER_STATE_DECELERATING = 3,
    STEPPER_STATE_FAULT        = 4,
} stepper_state_t;

/* ---- Configuration ------------------------------------------------------- */

typedef struct {
    uint32_t max_speed_steps_s;   /* Maximum speed (steps/second)       */
    uint32_t accel_steps_s2;      /* Acceleration (steps/second²)       */
    uint32_t decel_steps_s2;      /* Deceleration (steps/second²)       */
    uint16_t microsteps;          /* 1, 2, 4, 8, 16, 32                */
    bool     reverse_direction;   /* Flip direction polarity            */
} stepper_config_t;

/* ---- Lifecycle ----------------------------------------------------------- */

stepper_t*  stepper_get(const char* dt_label);
void        stepper_put(stepper_t* st);
ro_status_t stepper_configure(stepper_t* st, const stepper_config_t* cfg);

/* ---- Motion control ------------------------------------------------------ */

ro_status_t stepper_move(stepper_t* st, int32_t steps);
ro_status_t stepper_move_to(stepper_t* st, int32_t position);
ro_status_t stepper_stop(stepper_t* st);
void        stepper_emergency_stop(stepper_t* st);

/* ---- State & feedback ---------------------------------------------------- */

stepper_state_t stepper_get_state(const stepper_t* st);
int32_t         stepper_get_position(const stepper_t* st);
ro_status_t     stepper_wait_idle(stepper_t* st, uint32_t timeout_ms);

#ifdef __cplusplus
}
#endif

#endif /* ROBOTOS_STEPPER_H */
