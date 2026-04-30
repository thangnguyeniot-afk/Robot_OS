/* endstop.c — Endstop / Limit Switch Implementation (Framework Layer)
 *
 * Wraps adapter GPIO with interrupt callback for trigger detection.
 */
#include <robotos/endstop.h>
#include <robotos/ro_gpio.h>
#include <robotos/ro_log.h>
#include <string.h>
#include <stdbool.h>

#define ENDSTOP_MAX_INSTANCES  6

struct endstop {
    const char*           label;
    ro_gpio_config_t      pin;
    endstop_trigger_cb_t  cb;
    void*                 cb_arg;
    bool                  in_use;
};

static struct endstop s_pool[ENDSTOP_MAX_INSTANCES];

/* ISR trampoline */
static void endstop_isr(void* arg)
{
    struct endstop* e = (struct endstop*)arg;
    if (e->cb) {
        e->cb((endstop_t*)e, e->cb_arg);
    }
}

/* ── Public API ─────────────────────────────────────────────── */

endstop_t* endstop_get(const char* dt_label)
{
    for (int i = 0; i < ENDSTOP_MAX_INSTANCES; i++) {
        if (!s_pool[i].in_use) {
            memset(&s_pool[i], 0, sizeof(struct endstop));
            s_pool[i].label  = dt_label;
            s_pool[i].in_use = true;

            s_pool[i].pin.dt_label    = dt_label;
            s_pool[i].pin.active_high = false; /* NC endstops default */
            ro_gpio_get(&s_pool[i].pin, RO_GPIO_INPUT);

            return (endstop_t*)&s_pool[i];
        }
    }
    return NULL;
}

void endstop_put(endstop_t* endstop)
{
    struct endstop* e = (struct endstop*)endstop;
    if (!e) return;
    endstop_clear_callback(endstop);
    e->in_use = false;
}

bool endstop_is_triggered(const endstop_t* endstop)
{
    const struct endstop* e = (const struct endstop*)endstop;
    if (!e) return false;
    bool level = false;
    ro_gpio_read((ro_gpio_config_t*)&e->pin, &level);
    /* active_high: triggered when pin is high; else when low */
    return e->pin.active_high ? level : !level;
}

ro_status_t endstop_set_callback(endstop_t* endstop,
                                  endstop_trigger_cb_t cb, void* user_data)
{
    struct endstop* e = (struct endstop*)endstop;
    if (!e) return RO_EINVAL;
    e->cb     = cb;
    e->cb_arg = user_data;
    return ro_gpio_set_isr(&e->pin, endstop_isr, e);
}

ro_status_t endstop_clear_callback(endstop_t* endstop)
{
    struct endstop* e = (struct endstop*)endstop;
    if (!e) return RO_EINVAL;
    e->cb     = NULL;
    e->cb_arg = NULL;
    return ro_gpio_set_isr(&e->pin, NULL, NULL);
}
