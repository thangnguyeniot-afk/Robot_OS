/* dcmotor.h — DC Motor API (Framework Layer) */
#ifndef ROBOTOS_DCMOTOR_H
#define ROBOTOS_DCMOTOR_H

#include <robotos/ro_status.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct dcmotor dcmotor_t;

typedef struct {
    uint32_t pwm_period_ns;   /* PWM period in nanoseconds */
    bool     invert_dir_pin;  /* Invert direction logic    */
} dcmotor_config_t;

dcmotor_t*  dcmotor_get(const char* dt_label);
void        dcmotor_put(dcmotor_t* motor);
ro_status_t dcmotor_configure(dcmotor_t* motor, const dcmotor_config_t* cfg);
ro_status_t dcmotor_set_speed(dcmotor_t* motor, float speed);   /* -1.0 … +1.0 */
ro_status_t dcmotor_stop(dcmotor_t* motor);

#ifdef __cplusplus
}
#endif
#endif /* ROBOTOS_DCMOTOR_H */
