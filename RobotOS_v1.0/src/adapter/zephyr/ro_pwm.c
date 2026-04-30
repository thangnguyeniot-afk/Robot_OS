/* ============================================================================
 * ro_pwm.c — Zephyr PWM Backend
 * ============================================================================
 * Layer: Adapter / Zephyr backend
 * ========================================================================= */

#include <robotos/ro_pwm.h>
#include <robotos/ro_assert.h>

#ifndef ROBOTOS_HOST_BUILD
#include <zephyr/drivers/pwm.h>
#include <zephyr/device.h>
#endif

#include <string.h>

#define RO_PWM_POOL_SIZE 8

typedef struct ro_pwm {
    bool             in_use;
    ro_pwm_config_t  cfg;
    uint32_t         current_pulse_ns;
#ifndef ROBOTOS_HOST_BUILD
    const struct device* dev;
#endif
} ro_pwm_slot_t;

static ro_pwm_slot_t s_pwm_pool[RO_PWM_POOL_SIZE];

ro_pwm_t* ro_pwm_get(const ro_pwm_config_t* cfg)
{
    RO_ASSERT(cfg != NULL, "ro_pwm_get: NULL config");

    for (int i = 0; i < RO_PWM_POOL_SIZE; i++) {
        if (!s_pwm_pool[i].in_use) {
            ro_pwm_slot_t* slot = &s_pwm_pool[i];
            memset(slot, 0, sizeof(*slot));
            slot->in_use = true;
            slot->cfg    = *cfg;

#ifndef ROBOTOS_HOST_BUILD
            slot->dev = device_get_binding(cfg->dt_label);
            if (slot->dev == NULL) {
                slot->in_use = false;
                return NULL;
            }
#endif
            return (ro_pwm_t*)slot;
        }
    }
    return NULL;
}

void ro_pwm_put(ro_pwm_t* pwm)
{
    if (pwm == NULL) return;
    ro_pwm_slot_t* slot = (ro_pwm_slot_t*)pwm;
    memset(slot, 0, sizeof(*slot));
}

ro_status_t ro_pwm_set_pulse(ro_pwm_t* pwm, uint32_t pulse_ns)
{
    RO_ASSERT(pwm != NULL, "ro_pwm_set_pulse: NULL pwm");
    ro_pwm_slot_t* slot = (ro_pwm_slot_t*)pwm;
    slot->current_pulse_ns = pulse_ns;

#ifndef ROBOTOS_HOST_BUILD
    int ret = pwm_set(slot->dev, slot->cfg.channel,
                       slot->cfg.period_ns, pulse_ns, 0);
    return (ret == 0) ? RO_OK : RO_EFAIL;
#else
    return RO_OK;
#endif
}

ro_status_t ro_pwm_set_period(ro_pwm_t* pwm, uint32_t period_ns, uint32_t pulse_ns)
{
    RO_ASSERT(pwm != NULL, "ro_pwm_set_period: NULL pwm");
    ro_pwm_slot_t* slot = (ro_pwm_slot_t*)pwm;
    slot->cfg.period_ns    = period_ns;
    slot->current_pulse_ns = pulse_ns;

#ifndef ROBOTOS_HOST_BUILD
    int ret = pwm_set(slot->dev, slot->cfg.channel, period_ns, pulse_ns, 0);
    return (ret == 0) ? RO_OK : RO_EFAIL;
#else
    return RO_OK;
#endif
}

ro_status_t ro_pwm_disable(ro_pwm_t* pwm)
{
    return ro_pwm_set_pulse(pwm, 0);
}
