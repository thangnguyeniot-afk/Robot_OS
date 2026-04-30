/* endstop_drv.c — Low-level Endstop Driver (Framework / Drivers)
 *
 * Manages a table of endstop GPIO pins with software debounce.
 * Typically registered once during board init; the higher-level
 * endstop.c (Framework) wraps this with per-instance callbacks.
 */
#include <robotos/ro_gpio.h>
#include <robotos/ro_time.h>
#include <robotos/ro_status.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#define ENDSTOP_DRV_MAX  6
#define DEBOUNCE_US      5000  /* 5 ms software debounce */

typedef void (*endstop_drv_cb_t)(uint8_t id, bool triggered, void* arg);

typedef struct {
    ro_gpio_config_t pin;
    endstop_drv_cb_t cb;
    void*            cb_arg;
    uint64_t         last_trigger_us;
    bool             last_state;
    bool             in_use;
} endstop_drv_entry_t;

static endstop_drv_entry_t s_entries[ENDSTOP_DRV_MAX];

/* ISR trampoline — performs debounce check */
static void drv_isr(void* arg)
{
    endstop_drv_entry_t* e = (endstop_drv_entry_t*)arg;
    uint64_t now = ro_time_us();
    if (now - e->last_trigger_us < DEBOUNCE_US) return;  /* Bounce */
    e->last_trigger_us = now;

    bool level = false;
    ro_gpio_read(&e->pin, &level);
    if (level == e->last_state) return;  /* No real change */
    e->last_state = level;

    if (e->cb) {
        uint8_t id = (uint8_t)(e - s_entries);
        e->cb(id, level, e->cb_arg);
    }
}

/* ── API ────────────────────────────────────────────────────── */

ro_status_t endstop_drv_register(uint8_t id,
                                  const ro_gpio_config_t* pin,
                                  endstop_drv_cb_t cb,
                                  void* arg)
{
    if (id >= ENDSTOP_DRV_MAX || !pin) return RO_EINVAL;
    endstop_drv_entry_t* e = &s_entries[id];

    e->pin             = *pin;
    e->cb              = cb;
    e->cb_arg          = arg;
    e->last_trigger_us = 0;
    e->last_state      = false;
    e->in_use          = true;

    ro_gpio_get(&e->pin, RO_GPIO_INPUT);
    ro_gpio_set_isr(&e->pin, drv_isr, e);
    return RO_OK;
}

ro_status_t endstop_drv_unregister(uint8_t id)
{
    if (id >= ENDSTOP_DRV_MAX) return RO_EINVAL;
    endstop_drv_entry_t* e = &s_entries[id];
    ro_gpio_set_isr(&e->pin, NULL, NULL);
    memset(e, 0, sizeof(*e));
    return RO_OK;
}

bool endstop_drv_read(uint8_t id)
{
    if (id >= ENDSTOP_DRV_MAX || !s_entries[id].in_use) return false;
    bool level = false;
    ro_gpio_read(&s_entries[id].pin, &level);
    return level;
}
