/* dcmotor.c — DC Motor Implementation (Framework Layer)
 *
 * Uses adapter GPIO (direction) + PWM (speed) to drive an H-bridge.
 */
#include <robotos/dcmotor.h>
#include <robotos/ro_gpio.h>
#include <robotos/ro_pwm.h>
#include <robotos/ro_mutex.h>
#include <robotos/ro_log.h>
#include <string.h>
#include <math.h>

#define DCMOTOR_MAX_INSTANCES  4

struct dcmotor {
    const char*       label;
    dcmotor_config_t  cfg;
    ro_gpio_config_t  dir_pin;
    ro_pwm_config_t   pwm;
    ro_mutex_t        mtx;
    bool              in_use;
};

static struct dcmotor s_pool[DCMOTOR_MAX_INSTANCES];

/* ── Public API ─────────────────────────────────────────────── */

dcmotor_t* dcmotor_get(const char* dt_label)
{
    for (int i = 0; i < DCMOTOR_MAX_INSTANCES; i++) {
        if (!s_pool[i].in_use) {
            memset(&s_pool[i], 0, sizeof(struct dcmotor));
            s_pool[i].label  = dt_label;
            s_pool[i].in_use = true;

            s_pool[i].cfg.pwm_period_ns  = 1000000; /* 1 kHz default */
            s_pool[i].cfg.invert_dir_pin = false;

            s_pool[i].pwm.dt_label  = dt_label;
            s_pool[i].pwm.channel   = 0;
            s_pool[i].pwm.period_ns = s_pool[i].cfg.pwm_period_ns;

            ro_mutex_create(&s_pool[i].mtx);
            ro_pwm_get(&s_pool[i].pwm);
            return (dcmotor_t*)&s_pool[i];
        }
    }
    return NULL;
}

void dcmotor_put(dcmotor_t* motor)
{
    struct dcmotor* m = (struct dcmotor*)motor;
    if (!m) return;
    dcmotor_stop(motor);
    ro_pwm_put(&m->pwm);
    ro_mutex_destroy(&m->mtx);
    m->in_use = false;
}

ro_status_t dcmotor_configure(dcmotor_t* motor, const dcmotor_config_t* cfg)
{
    struct dcmotor* m = (struct dcmotor*)motor;
    if (!m || !cfg) return RO_EINVAL;

    ro_mutex_lock(&m->mtx, RO_QUEUE_WAIT_FOREVER);
    m->cfg = *cfg;
    m->pwm.period_ns = cfg->pwm_period_ns;
    ro_pwm_set_period(&m->pwm, m->pwm.period_ns);
    ro_mutex_unlock(&m->mtx);
    return RO_OK;
}

ro_status_t dcmotor_set_speed(dcmotor_t* motor, float speed)
{
    struct dcmotor* m = (struct dcmotor*)motor;
    if (!m) return RO_EINVAL;
    if (speed < -1.0f) speed = -1.0f;
    if (speed >  1.0f) speed =  1.0f;

    bool dir = (speed >= 0.0f);
    if (m->cfg.invert_dir_pin) dir = !dir;
    float magnitude = fabsf(speed);

    ro_gpio_set(&m->dir_pin, dir);

    /* duty = magnitude * period */
    uint32_t pulse_ns = (uint32_t)(magnitude * (float)m->cfg.pwm_period_ns);
    return ro_pwm_set_pulse(&m->pwm, pulse_ns);
}

ro_status_t dcmotor_stop(dcmotor_t* motor)
{
    return dcmotor_set_speed(motor, 0.0f);
}
