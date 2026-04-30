/* ============================================================================
 * ro_pwm.h — RobotOS PWM Abstraction
 * ============================================================================
 * Layer: Adapter (public header — NO <zephyr/*> includes)
 * ========================================================================= */

#ifndef ROBOTOS_RO_PWM_H
#define ROBOTOS_RO_PWM_H

#include <robotos/ro_status.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    const char* dt_label;     /* PWM controller DT label              */
    uint32_t    channel;      /* PWM channel number                   */
    uint32_t    period_ns;    /* Default period in nanoseconds        */
} ro_pwm_config_t;

typedef struct ro_pwm ro_pwm_t;

ro_pwm_t*   ro_pwm_get(const ro_pwm_config_t* cfg);
void        ro_pwm_put(ro_pwm_t* pwm);

/** Set pulse width in nanoseconds (0 = output low). */
ro_status_t ro_pwm_set_pulse(ro_pwm_t* pwm, uint32_t pulse_ns);

/** Change period (and optionally pulse) dynamically. */
ro_status_t ro_pwm_set_period(ro_pwm_t* pwm, uint32_t period_ns, uint32_t pulse_ns);

/** Disable PWM output (hold low). */
ro_status_t ro_pwm_disable(ro_pwm_t* pwm);

#ifdef __cplusplus
}
#endif

#endif /* ROBOTOS_RO_PWM_H */
