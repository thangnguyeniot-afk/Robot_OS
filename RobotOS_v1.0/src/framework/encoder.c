/* encoder.c — Rotary / Linear Encoder Implementation (Framework Layer)
 *
 * Uses GPIO interrupt on channel-A to increment an atomic counter.
 * Channel-B determines direction (quadrature decode).
 */
#include <robotos/encoder.h>
#include <robotos/ro_gpio.h>
#include <robotos/ro_time.h>
#include <robotos/ro_log.h>
#include <string.h>
#include <stdatomic.h>

#define ENCODER_MAX_INSTANCES  4

struct encoder {
    const char*       label;
    atomic_int        count;
    uint32_t          ticks_per_rev;
    ro_gpio_config_t  ch_a;
    ro_gpio_config_t  ch_b;

    /* Velocity estimation */
    int32_t           last_count;
    uint64_t          last_time_us;

    bool              in_use;
};

static struct encoder s_pool[ENCODER_MAX_INSTANCES];

/* ISR callback for channel-A edge */
static void encoder_isr(void* arg)
{
    struct encoder* e = (struct encoder*)arg;
    bool b_level = false;
    ro_gpio_read(&e->ch_b, &b_level);
    if (b_level) {
        atomic_fetch_add(&e->count, 1);
    } else {
        atomic_fetch_sub(&e->count, 1);
    }
}

/* ── Public API ─────────────────────────────────────────────── */

encoder_t* encoder_get(const char* dt_label)
{
    for (int i = 0; i < ENCODER_MAX_INSTANCES; i++) {
        if (!s_pool[i].in_use) {
            memset(&s_pool[i], 0, sizeof(struct encoder));
            s_pool[i].label         = dt_label;
            s_pool[i].in_use        = true;
            s_pool[i].ticks_per_rev = 400; /* Default — overridden */
            atomic_store(&s_pool[i].count, 0);

            /* Register ISR on ch_a (rising edge) */
            s_pool[i].ch_a.dt_label = dt_label;
            ro_gpio_get(&s_pool[i].ch_a, RO_GPIO_INPUT);
            ro_gpio_set_isr(&s_pool[i].ch_a, encoder_isr, &s_pool[i]);

            s_pool[i].ch_b.dt_label = dt_label;
            ro_gpio_get(&s_pool[i].ch_b, RO_GPIO_INPUT);

            return (encoder_t*)&s_pool[i];
        }
    }
    return NULL;
}

void encoder_put(encoder_t* enc)
{
    struct encoder* e = (struct encoder*)enc;
    if (!e) return;
    e->in_use = false;
}

int32_t encoder_get_count(const encoder_t* enc)
{
    const struct encoder* e = (const struct encoder*)enc;
    return e ? atomic_load(&e->count) : 0;
}

ro_status_t encoder_reset(encoder_t* enc)
{
    struct encoder* e = (struct encoder*)enc;
    if (!e) return RO_EINVAL;
    atomic_store(&e->count, 0);
    e->last_count   = 0;
    e->last_time_us = ro_time_us();
    return RO_OK;
}

uint32_t encoder_get_ticks_per_rev(const encoder_t* enc)
{
    const struct encoder* e = (const struct encoder*)enc;
    return e ? e->ticks_per_rev : 0;
}

float encoder_get_velocity(const encoder_t* enc, uint32_t window_ms)
{
    const struct encoder* e = (const struct encoder*)enc;
    if (!e || window_ms == 0) return 0.0f;

    /* Cast away const for velocity bookkeeping (acceptable — state is mutable) */
    struct encoder* me = (struct encoder*)e;

    uint64_t now = ro_time_us();
    int32_t  cur = atomic_load(&me->count);
    int32_t  delta_ticks = cur - me->last_count;
    uint64_t delta_us    = now - me->last_time_us;

    if (delta_us < (uint64_t)window_ms * 1000U) {
        /* Not enough time elapsed — return previous estimate */
        if (delta_us == 0) return 0.0f;
    }

    me->last_count   = cur;
    me->last_time_us = now;

    float dt_s = (float)delta_us / 1000000.0f;
    return (dt_s > 0.0f) ? ((float)delta_ticks / dt_s) : 0.0f;
}
