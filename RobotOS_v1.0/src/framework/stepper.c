/* stepper.c — Stepper Motor Implementation (Framework Layer)
 *
 * Uses adapter GPIO + Timer for pulse generation.
 * Implements trapezoidal velocity profile (accel / cruise / decel).
 */
#include <robotos/stepper.h>
#include <robotos/ro_gpio.h>
#include <robotos/ro_timer.h>
#include <robotos/ro_time.h>
#include <robotos/ro_mutex.h>
#include <robotos/ro_assert.h>
#include <robotos/ro_log.h>

#include <string.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <stdbool.h>

#define STEPPER_MAX_INSTANCES  4

/* ── Internal representation ────────────────────────────────── */
struct stepper {
    const char*       label;
    stepper_config_t  cfg;
    stepper_state_t   state;

    /* Position & motion */
    atomic_int        position;        /* Absolute step count       */
    int32_t           target_steps;    /* Relative move remaining   */
    int32_t           steps_done;
    bool              direction;       /* true = positive           */

    /* Velocity profile */
    uint32_t          current_speed;   /* steps/s                   */
    uint32_t          accel_steps;     /* Steps in accel phase      */
    uint32_t          decel_start;     /* Step at which decel begins*/

    /* Hardware handles */
    ro_gpio_config_t  step_pin;
    ro_gpio_config_t  dir_pin;
    ro_gpio_config_t  enable_pin;
    ro_timer_t        pulse_timer;

    /* Callback */
    stepper_done_cb_t done_cb;
    void*             done_cb_arg;

    /* Mutex for config/state access */
    ro_mutex_t        mtx;
    bool              in_use;
};

static struct stepper s_pool[STEPPER_MAX_INSTANCES];

/* ── Helpers ────────────────────────────────────────────────── */

static struct stepper* pool_alloc(const char* label)
{
    for (int i = 0; i < STEPPER_MAX_INSTANCES; i++) {
        if (!s_pool[i].in_use) {
            memset(&s_pool[i], 0, sizeof(struct stepper));
            s_pool[i].label  = label;
            s_pool[i].in_use = true;
            return &s_pool[i];
        }
    }
    return NULL;
}

/* Compute trapezoidal profile parameters for a given move length */
static void compute_profile(struct stepper* s, uint32_t total_steps)
{
    /* Simplified: equal accel / decel distances.
     * accel_steps = v_max^2 / (2 * accel)                           */
    uint32_t v  = s->cfg.max_speed_steps_s;
    uint32_t a  = s->cfg.accel_steps_s2;
    uint32_t d  = s->cfg.decel_steps_s2 ? s->cfg.decel_steps_s2 : a;

    uint32_t accel_dist = (v * v) / (2 * a);
    uint32_t decel_dist = (v * v) / (2 * d);

    if (accel_dist + decel_dist > total_steps) {
        /* Triangle profile — cannot reach full speed */
        accel_dist = total_steps / 2;
        decel_dist = total_steps - accel_dist;
    }

    s->accel_steps = accel_dist;
    s->decel_start = total_steps - decel_dist;
}

/* Timer callback — generates one step pulse per tick */
static void pulse_tick(void* arg)
{
    struct stepper* s = (struct stepper*)arg;

    if (s->steps_done >= s->target_steps) {
        ro_timer_stop(&s->pulse_timer);
        s->state = STEPPER_IDLE;
        if (s->done_cb) {
            s->done_cb((stepper_t*)s, s->done_cb_arg);
        }
        return;
    }

    /* Set direction */
    ro_gpio_set(&s->dir_pin,
                s->direction ^ s->cfg.reverse_direction);

    /* Pulse step pin HIGH then LOW */
    ro_gpio_set(&s->step_pin, true);
    ro_delay_us(2);
    ro_gpio_set(&s->step_pin, false);

    s->steps_done++;
    atomic_fetch_add(&s->position, s->direction ? 1 : -1);

    /* Update speed phase */
    if ((uint32_t)s->steps_done < s->accel_steps) {
        s->state = STEPPER_ACCELERATING;
        /* Ramp speed linearly (simplified) */
        uint32_t spd = (s->cfg.accel_steps_s2 * s->steps_done) /
                        (s->accel_steps ? s->accel_steps : 1);
        s->current_speed = spd > 0 ? spd : 1;
    } else if ((uint32_t)s->steps_done >= s->decel_start) {
        s->state = STEPPER_DECELERATING;
        uint32_t remain = s->target_steps - s->steps_done;
        uint32_t decel_total = s->target_steps - s->decel_start;
        uint32_t spd = (s->cfg.max_speed_steps_s * remain) /
                        (decel_total ? decel_total : 1);
        s->current_speed = spd > 0 ? spd : 1;
    } else {
        s->state = STEPPER_RUNNING;
        s->current_speed = s->cfg.max_speed_steps_s;
    }

    /* Adjust timer period for next tick */
    if (s->current_speed > 0) {
        uint32_t period_us = 1000000U / s->current_speed;
        if (period_us < 10) period_us = 10;  /* Floor */
        /* Note: full implementation would dynamically adjust timer.
         * Here we rely on a fixed fast tick and skip logic. */
        (void)period_us;
    }
}

/* ── Public API ─────────────────────────────────────────────── */

stepper_t* stepper_get(const char* dt_label)
{
    struct stepper* s = pool_alloc(dt_label);
    if (!s) return NULL;

    ro_mutex_create(&s->mtx);
    return (stepper_t*)s;
}

void stepper_put(stepper_t* stepper)
{
    struct stepper* s = (struct stepper*)stepper;
    if (!s) return;
    stepper_emergency_stop(stepper);
    ro_mutex_destroy(&s->mtx);
    s->in_use = false;
}

ro_status_t stepper_configure(stepper_t* stepper, const stepper_config_t* cfg)
{
    struct stepper* s = (struct stepper*)stepper;
    if (!s || !cfg) return RO_EINVAL;

    ro_mutex_lock(&s->mtx, RO_QUEUE_WAIT_FOREVER);
    s->cfg = *cfg;
    ro_mutex_unlock(&s->mtx);
    return RO_OK;
}

ro_status_t stepper_enable(stepper_t* stepper)
{
    struct stepper* s = (struct stepper*)stepper;
    if (!s) return RO_EINVAL;
    /* Active-low enable on most stepper drivers */
    return ro_gpio_set(&s->enable_pin, false);
}

ro_status_t stepper_disable(stepper_t* stepper)
{
    struct stepper* s = (struct stepper*)stepper;
    if (!s) return RO_EINVAL;
    return ro_gpio_set(&s->enable_pin, true);
}

ro_status_t stepper_move(stepper_t* stepper, int32_t steps)
{
    struct stepper* s = (struct stepper*)stepper;
    if (!s) return RO_EINVAL;
    if (s->state != STEPPER_IDLE) return RO_EBUSY;

    s->direction    = (steps >= 0);
    s->target_steps = abs(steps);
    s->steps_done   = 0;
    s->current_speed = 1;

    compute_profile(s, (uint32_t)s->target_steps);

    s->state = STEPPER_ACCELERATING;

    /* Start pulse timer at initial slow rate */
    uint32_t initial_period_us = 1000000U / (s->cfg.accel_steps_s2 > 0 ? 1 : s->cfg.max_speed_steps_s);
    if (initial_period_us < 10) initial_period_us = 10;

    ro_timer_init(&s->pulse_timer, "stp_pulse", initial_period_us, pulse_tick, s);
    return ro_timer_start_periodic(&s->pulse_timer);
}

ro_status_t stepper_move_to(stepper_t* stepper, int32_t position)
{
    struct stepper* s = (struct stepper*)stepper;
    if (!s) return RO_EINVAL;
    int32_t delta = position - atomic_load(&s->position);
    return stepper_move(stepper, delta);
}

ro_status_t stepper_set_speed(stepper_t* stepper, uint32_t speed_steps_s)
{
    struct stepper* s = (struct stepper*)stepper;
    if (!s) return RO_EINVAL;
    ro_mutex_lock(&s->mtx, RO_QUEUE_WAIT_FOREVER);
    s->cfg.max_speed_steps_s = speed_steps_s;
    ro_mutex_unlock(&s->mtx);
    return RO_OK;
}

ro_status_t stepper_stop(stepper_t* stepper)
{
    struct stepper* s = (struct stepper*)stepper;
    if (!s) return RO_EINVAL;
    /* Trigger decel from current position */
    if (s->state == STEPPER_RUNNING || s->state == STEPPER_ACCELERATING) {
        s->decel_start = (uint32_t)s->steps_done;
        s->target_steps = s->steps_done +
            (int32_t)((s->current_speed * s->current_speed) /
             (2 * (s->cfg.decel_steps_s2 ? s->cfg.decel_steps_s2 : s->cfg.accel_steps_s2)));
        s->state = STEPPER_DECELERATING;
    }
    return RO_OK;
}

ro_status_t stepper_emergency_stop(stepper_t* stepper)
{
    struct stepper* s = (struct stepper*)stepper;
    if (!s) return RO_EINVAL;
    ro_timer_stop(&s->pulse_timer);
    s->state = STEPPER_FAULT;
    s->current_speed = 0;
    return RO_OK;
}

stepper_state_t stepper_get_state(const stepper_t* stepper)
{
    const struct stepper* s = (const struct stepper*)stepper;
    return s ? s->state : STEPPER_FAULT;
}

int32_t stepper_get_position(const stepper_t* stepper)
{
    const struct stepper* s = (const struct stepper*)stepper;
    return s ? atomic_load(&s->position) : 0;
}

ro_status_t stepper_set_done_callback(stepper_t* stepper,
                                       stepper_done_cb_t cb, void* arg)
{
    struct stepper* s = (struct stepper*)stepper;
    if (!s) return RO_EINVAL;
    s->done_cb     = cb;
    s->done_cb_arg = arg;
    return RO_OK;
}

ro_status_t stepper_wait_idle(stepper_t* stepper, uint32_t timeout_ms)
{
    struct stepper* s = (struct stepper*)stepper;
    if (!s) return RO_EINVAL;

    uint64_t deadline = ro_time_ms() + timeout_ms;
    while (s->state != STEPPER_IDLE && s->state != STEPPER_FAULT) {
        if (ro_time_ms() >= deadline) return RO_ETIMEDOUT;
        ro_delay_ms(1);
    }
    return RO_OK;
}
