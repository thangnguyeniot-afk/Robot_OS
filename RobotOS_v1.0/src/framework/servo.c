/* servo.c — Servo Implementation (Framework Layer)
 *
 * Maps angle (0.0–1.0) to pulse width via adapter PWM.
 */
#include <robotos/servo.h>
#include <robotos/ro_pwm.h>
#include <robotos/ro_mutex.h>
#include <robotos/ro_log.h>
#include <string.h>

#define SERVO_MAX_INSTANCES  4

struct servo {
    const char*     label;
    servo_config_t  cfg;
    ro_pwm_config_t pwm;
    ro_mutex_t      mtx;
    bool            in_use;
};

static struct servo s_pool[SERVO_MAX_INSTANCES];

/* ── Public API ─────────────────────────────────────────────── */

servo_t* servo_get(const char* dt_label)
{
    for (int i = 0; i < SERVO_MAX_INSTANCES; i++) {
        if (!s_pool[i].in_use) {
            memset(&s_pool[i], 0, sizeof(struct servo));
            s_pool[i].label  = dt_label;
            s_pool[i].in_use = true;

            /* Default config */
            s_pool[i].cfg.min_pulse_us = 1000;
            s_pool[i].cfg.max_pulse_us = 2000;
            s_pool[i].cfg.period_us    = 20000;

            /* PWM adapter */
            s_pool[i].pwm.dt_label = dt_label;
            s_pool[i].pwm.channel  = 0;
            s_pool[i].pwm.period_ns = s_pool[i].cfg.period_us * 1000U;

            ro_mutex_create(&s_pool[i].mtx);
            ro_pwm_get(&s_pool[i].pwm);
            return (servo_t*)&s_pool[i];
        }
    }
    return NULL;
}

void servo_put(servo_t* servo)
{
    struct servo* s = (struct servo*)servo;
    if (!s) return;
    servo_disable(servo);
    ro_pwm_put(&s->pwm);
    ro_mutex_destroy(&s->mtx);
    s->in_use = false;
}

ro_status_t servo_configure(servo_t* servo, const servo_config_t* cfg)
{
    struct servo* s = (struct servo*)servo;
    if (!s || !cfg) return RO_EINVAL;
    if (cfg->min_pulse_us >= cfg->max_pulse_us) return RO_EINVAL;

    ro_mutex_lock(&s->mtx, RO_QUEUE_WAIT_FOREVER);
    s->cfg = *cfg;
    s->pwm.period_ns = cfg->period_us * 1000U;
    ro_pwm_set_period(&s->pwm, s->pwm.period_ns);
    ro_mutex_unlock(&s->mtx);
    return RO_OK;
}

ro_status_t servo_set_angle(servo_t* servo, float angle)
{
    struct servo* s = (struct servo*)servo;
    if (!s) return RO_EINVAL;
    if (angle < 0.0f) angle = 0.0f;
    if (angle > 1.0f) angle = 1.0f;

    uint32_t range = s->cfg.max_pulse_us - s->cfg.min_pulse_us;
    uint32_t pulse_us = s->cfg.min_pulse_us + (uint32_t)(angle * (float)range);
    return servo_set_pulse_us(servo, pulse_us);
}

ro_status_t servo_set_pulse_us(servo_t* servo, uint32_t us)
{
    struct servo* s = (struct servo*)servo;
    if (!s) return RO_EINVAL;
    /* Convert µs → ns */
    return ro_pwm_set_pulse(&s->pwm, us * 1000U);
}

ro_status_t servo_disable(servo_t* servo)
{
    struct servo* s = (struct servo*)servo;
    if (!s) return RO_EINVAL;
    return ro_pwm_disable(&s->pwm);
}
