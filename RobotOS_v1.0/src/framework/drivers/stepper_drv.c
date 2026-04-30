/* stepper_drv.c — Low-level Stepper Pulse Driver (Framework / Drivers)
 *
 * Bresenham-style step generation suitable for ISR-driven operation.
 * Used by the Application layer's pulse manager to output precisely-
 * timed step pulses for multi-axis coordinated motion.
 *
 * This driver is decoupled from the high-level stepper.c (which owns
 * the trapezoidal profile); stepper_drv operates at the individual
 * pulse level and is meant to be called from a hardware timer ISR.
 */
#include <robotos/ro_gpio.h>
#include <robotos/ro_status.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

/* ── Public types ───────────────────────────────────────────── */

#define STEPPER_DRV_MAX_AXES  3

typedef struct {
    ro_gpio_config_t step_pin;
    ro_gpio_config_t dir_pin;
} stepper_drv_axis_t;

typedef struct {
    stepper_drv_axis_t axes[STEPPER_DRV_MAX_AXES];
    uint8_t            num_axes;

    /* Bresenham state (per axis) */
    int32_t  steps[STEPPER_DRV_MAX_AXES];      /* Total steps for segment */
    int32_t  error[STEPPER_DRV_MAX_AXES];       /* Accumulated error      */
    int32_t  major_steps;                        /* Longest axis count     */
    int32_t  step_count;                         /* Steps executed         */
    bool     dir[STEPPER_DRV_MAX_AXES];          /* Direction per axis     */
    bool     active;
} stepper_drv_t;

/* ── API ────────────────────────────────────────────────────── */

ro_status_t stepper_drv_init(stepper_drv_t* drv, uint8_t num_axes)
{
    if (!drv || num_axes == 0 || num_axes > STEPPER_DRV_MAX_AXES)
        return RO_EINVAL;

    memset(drv, 0, sizeof(*drv));
    drv->num_axes = num_axes;
    return RO_OK;
}

/* Load a new segment: dx/dy/dz steps.  Returns RO_EBUSY if previous
 * segment is still executing. */
ro_status_t stepper_drv_load_segment(stepper_drv_t* drv,
                                      const int32_t steps[],
                                      uint8_t axis_mask)
{
    if (!drv || drv->active) return RO_EBUSY;

    drv->major_steps = 0;
    for (uint8_t i = 0; i < drv->num_axes; i++) {
        if (axis_mask & (1u << i)) {
            drv->steps[i] = abs(steps[i]);
            drv->dir[i]   = (steps[i] >= 0);
            ro_gpio_set(&drv->axes[i].dir_pin, drv->dir[i]);
        } else {
            drv->steps[i] = 0;
            drv->dir[i]   = true;
        }
        if (drv->steps[i] > drv->major_steps)
            drv->major_steps = drv->steps[i];
        drv->error[i] = 0;
    }
    drv->step_count = 0;
    drv->active     = true;
    return RO_OK;
}

/* Call this from the pulse timer ISR.
 * Returns true while the segment is still in progress. */
bool stepper_drv_tick(stepper_drv_t* drv)
{
    if (!drv || !drv->active) return false;

    if (drv->step_count >= drv->major_steps) {
        drv->active = false;
        return false;
    }

    /* Bresenham per axis */
    for (uint8_t i = 0; i < drv->num_axes; i++) {
        drv->error[i] += drv->steps[i];
        if (2 * drv->error[i] >= drv->major_steps) {
            /* Step this axis */
            ro_gpio_set(&drv->axes[i].step_pin, true);
            ro_gpio_set(&drv->axes[i].step_pin, false);
            drv->error[i] -= drv->major_steps;
        }
    }
    drv->step_count++;
    return true;
}

bool stepper_drv_is_active(const stepper_drv_t* drv)
{
    return drv ? drv->active : false;
}
