/* servo.h — Servo (PWM) API (Framework Layer) */
#ifndef ROBOTOS_SERVO_H
#define ROBOTOS_SERVO_H

#include <robotos/ro_status.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct servo servo_t;

typedef struct {
    uint32_t min_pulse_us;  /* Default 1000 µs */
    uint32_t max_pulse_us;  /* Default 2000 µs */
    uint32_t period_us;     /* Default 20000 µs (50 Hz) */
} servo_config_t;

servo_t*    servo_get(const char* dt_label);
void        servo_put(servo_t* servo);
ro_status_t servo_configure(servo_t* servo, const servo_config_t* cfg);
ro_status_t servo_set_angle(servo_t* servo, float angle);       /* 0.0–1.0 */
ro_status_t servo_set_pulse_us(servo_t* servo, uint32_t us);
ro_status_t servo_disable(servo_t* servo);

#ifdef __cplusplus
}
#endif
#endif /* ROBOTOS_SERVO_H */
