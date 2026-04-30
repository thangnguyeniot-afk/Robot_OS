/* ============================================================================
 * ro_gpio.c — Host GPIO Stub (no-op)
 * ============================================================================
 * Layer: Adapter / Host build
 * ========================================================================= */

#include <robotos/ro_gpio.h>
#include <string.h>

#define HOST_GPIO_POOL 16

typedef struct ro_gpio {
    bool in_use;
    ro_gpio_config_t cfg;
    int  value;         /* simulated output level */
    ro_gpio_isr_t cb;
    void* cb_user;
} ro_gpio_slot_t;

static ro_gpio_slot_t s_pool[HOST_GPIO_POOL];

ro_gpio_t* ro_gpio_get(const ro_gpio_config_t* cfg) {
    if (!cfg) return NULL;
    for (int i = 0; i < HOST_GPIO_POOL; i++) {
        if (!s_pool[i].in_use) {
            memset(&s_pool[i], 0, sizeof(s_pool[i]));
            s_pool[i].in_use = true;
            s_pool[i].cfg = *cfg;
            return (ro_gpio_t*)&s_pool[i];
        }
    }
    return NULL;
}

void ro_gpio_put(ro_gpio_t* g) {
    if (g) memset(g, 0, sizeof(ro_gpio_slot_t));
}

ro_status_t ro_gpio_set_dir(ro_gpio_t* g, ro_gpio_dir_t d) {
    (void)g; (void)d; return RO_OK;
}

ro_status_t ro_gpio_set(ro_gpio_t* g, uint32_t v) {
    if (!g) return RO_EINVAL;
    ((ro_gpio_slot_t*)g)->value = (int)v;
    return RO_OK;
}

int ro_gpio_read(const ro_gpio_t* g) {
    return g ? ((const ro_gpio_slot_t*)g)->value : 0;
}

ro_status_t ro_gpio_set_interrupt(ro_gpio_t* g, ro_gpio_int_mode_t m,
                                   ro_gpio_isr_t cb, void* user) {
    if (!g) return RO_EINVAL;
    (void)m;
    ro_gpio_slot_t* s = (ro_gpio_slot_t*)g;
    s->cb = cb; s->cb_user = user;
    return RO_OK;
}

ro_status_t ro_gpio_clear_interrupt(ro_gpio_t* g) {
    if (!g) return RO_EINVAL;
    ro_gpio_slot_t* s = (ro_gpio_slot_t*)g;
    s->cb = NULL; s->cb_user = NULL;
    return RO_OK;
}
