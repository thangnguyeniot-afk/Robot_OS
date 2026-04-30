/* limiter.c — Rate / Acceleration Limiters Implementation (Framework Layer) */
#include <robotos/limiter.h>
#include <math.h>

/* ── Rate Limiter ───────────────────────────────────────────── */

ro_status_t rate_limiter_init(rate_limiter_t* r, float max_rate, float initial)
{
    if (!r || max_rate <= 0.0f) return RO_EINVAL;
    r->max_rate    = max_rate;
    r->prev_output = initial;
    return RO_OK;
}

float rate_limiter_update(rate_limiter_t* r, float input)
{
    float delta = input - r->prev_output;
    if (delta >  r->max_rate) delta =  r->max_rate;
    if (delta < -r->max_rate) delta = -r->max_rate;
    r->prev_output += delta;
    return r->prev_output;
}

/* ── Acceleration Limiter ───────────────────────────────────── */

ro_status_t accel_limiter_init(accel_limiter_t* a,
                                float max_accel, float max_speed)
{
    if (!a || max_accel <= 0.0f || max_speed <= 0.0f) return RO_EINVAL;
    a->max_accel = max_accel;
    a->max_speed = max_speed;
    a->velocity  = 0.0f;
    a->position  = 0.0f;
    return RO_OK;
}

float accel_limiter_update(accel_limiter_t* a, float target_pos, float dt)
{
    if (!a || dt <= 0.0f) return a ? a->position : 0.0f;

    float error = target_pos - a->position;

    /* Desired velocity to close the gap */
    float v_desired = error / dt;

    /* Clamp velocity to max_speed */
    v_desired = limiter_clamp(v_desired, -a->max_speed, a->max_speed);

    /* Clamp acceleration */
    float dv = v_desired - a->velocity;
    float max_dv = a->max_accel * dt;
    dv = limiter_clamp(dv, -max_dv, max_dv);

    a->velocity += dv;
    a->position += a->velocity * dt;
    return a->position;
}
