/* ============================================================================
 * ro_gpio.c — Zephyr GPIO Backend
 * ============================================================================
 * Layer: Adapter / Zephyr backend
 * ========================================================================= */

#include <robotos/ro_gpio.h>
#include <robotos/ro_assert.h>

#ifndef ROBOTOS_HOST_BUILD
#include <zephyr/drivers/gpio.h>
#include <zephyr/device.h>
#endif

#include <string.h>

/* ---- Static GPIO pool ---------------------------------------------------- */

#define RO_GPIO_POOL_SIZE 16

typedef struct ro_gpio {
    bool            in_use;
    ro_gpio_config_t cfg;
    ro_gpio_isr_t   isr_cb;
    void*           isr_user;
#ifndef ROBOTOS_HOST_BUILD
    const struct device* dev;
    struct gpio_callback zcb;
#endif
} ro_gpio_slot_t;

static ro_gpio_slot_t s_gpio_pool[RO_GPIO_POOL_SIZE];

/* ---- ISR trampoline ----------------------------------------------------- */

#ifndef ROBOTOS_HOST_BUILD
static void gpio_isr_trampoline(const struct device* dev,
                                 struct gpio_callback* cb,
                                 uint32_t pins)
{
    (void)dev; (void)pins;
    ro_gpio_slot_t* slot = CONTAINER_OF(cb, ro_gpio_slot_t, zcb);
    if (slot->isr_cb) {
        slot->isr_cb((ro_gpio_t*)slot, slot->isr_user);
    }
}
#endif

/* ---- Lifecycle ----------------------------------------------------------- */

ro_gpio_t* ro_gpio_get(const ro_gpio_config_t* cfg)
{
    RO_ASSERT(cfg != NULL, "ro_gpio_get: NULL config");

    for (int i = 0; i < RO_GPIO_POOL_SIZE; i++) {
        if (!s_gpio_pool[i].in_use) {
            ro_gpio_slot_t* slot = &s_gpio_pool[i];
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
            return (ro_gpio_t*)slot;
        }
    }
    return NULL;  /* Pool exhausted */
}

void ro_gpio_put(ro_gpio_t* gpio)
{
    if (gpio == NULL) return;
    ro_gpio_slot_t* slot = (ro_gpio_slot_t*)gpio;
    memset(slot, 0, sizeof(*slot));
}

/* ---- Direction ----------------------------------------------------------- */

ro_status_t ro_gpio_set_dir(ro_gpio_t* gpio, ro_gpio_dir_t dir)
{
    RO_ASSERT(gpio != NULL, "ro_gpio_set_dir: NULL gpio");
    ro_gpio_slot_t* slot = (ro_gpio_slot_t*)gpio;

#ifndef ROBOTOS_HOST_BUILD
    gpio_flags_t flags = (dir == RO_GPIO_DIR_OUTPUT)
                         ? GPIO_OUTPUT_INACTIVE
                         : GPIO_INPUT;
    if (!slot->cfg.active_high) {
        flags |= GPIO_ACTIVE_LOW;
    }
    int ret = gpio_pin_configure(slot->dev, slot->cfg.pin, flags);
    return (ret == 0) ? RO_OK : RO_EFAIL;
#else
    (void)slot; (void)dir;
    return RO_OK;
#endif
}

/* ---- Read / Write -------------------------------------------------------- */

ro_status_t ro_gpio_set(ro_gpio_t* gpio, uint32_t value)
{
    RO_ASSERT(gpio != NULL, "ro_gpio_set: NULL gpio");
    ro_gpio_slot_t* slot = (ro_gpio_slot_t*)gpio;

#ifndef ROBOTOS_HOST_BUILD
    int ret = gpio_pin_set(slot->dev, slot->cfg.pin, (int)value);
    return (ret == 0) ? RO_OK : RO_EFAIL;
#else
    (void)slot; (void)value;
    return RO_OK;
#endif
}

int ro_gpio_read(const ro_gpio_t* gpio)
{
    RO_ASSERT(gpio != NULL, "ro_gpio_read: NULL gpio");
    const ro_gpio_slot_t* slot = (const ro_gpio_slot_t*)gpio;

#ifndef ROBOTOS_HOST_BUILD
    return gpio_pin_get(slot->dev, slot->cfg.pin);
#else
    (void)slot;
    return 0;
#endif
}

/* ---- Interrupt ----------------------------------------------------------- */

ro_status_t ro_gpio_set_interrupt(ro_gpio_t* gpio,
                                  ro_gpio_int_mode_t mode,
                                  ro_gpio_isr_t cb,
                                  void* user)
{
    RO_ASSERT(gpio != NULL, "ro_gpio_set_interrupt: NULL gpio");
    ro_gpio_slot_t* slot = (ro_gpio_slot_t*)gpio;
    slot->isr_cb   = cb;
    slot->isr_user = user;

#ifndef ROBOTOS_HOST_BUILD
    gpio_flags_t int_flags;
    switch (mode) {
        case RO_GPIO_INT_EDGE_RISING:  int_flags = GPIO_INT_EDGE_RISING;  break;
        case RO_GPIO_INT_EDGE_FALLING: int_flags = GPIO_INT_EDGE_FALLING; break;
        case RO_GPIO_INT_EDGE_BOTH:    int_flags = GPIO_INT_EDGE_BOTH;    break;
        case RO_GPIO_INT_LEVEL_HIGH:   int_flags = GPIO_INT_LEVEL_HIGH;   break;
        case RO_GPIO_INT_LEVEL_LOW:    int_flags = GPIO_INT_LEVEL_LOW;    break;
        default: return RO_EINVAL;
    }

    gpio_pin_interrupt_configure(slot->dev, slot->cfg.pin, int_flags);
    gpio_init_callback(&slot->zcb, gpio_isr_trampoline, BIT(slot->cfg.pin));
    gpio_add_callback(slot->dev, &slot->zcb);
#else
    (void)mode;
#endif

    return RO_OK;
}

ro_status_t ro_gpio_clear_interrupt(ro_gpio_t* gpio)
{
    RO_ASSERT(gpio != NULL, "ro_gpio_clear_interrupt: NULL gpio");
    ro_gpio_slot_t* slot = (ro_gpio_slot_t*)gpio;

#ifndef ROBOTOS_HOST_BUILD
    gpio_pin_interrupt_configure(slot->dev, slot->cfg.pin, GPIO_INT_DISABLE);
    gpio_remove_callback(slot->dev, &slot->zcb);
#endif

    slot->isr_cb   = NULL;
    slot->isr_user = NULL;
    return RO_OK;
}
