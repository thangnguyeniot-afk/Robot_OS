/* limiter.h — Rate / Acceleration / Clamp Limiters (Framework Layer)
 *
 * clamp        — inline, stateless
 * rate_limiter — first-order rate-of-change limit
 * accel_limiter— trapezoidal velocity+position envelope
 */
#ifndef ROBOTOS_LIMITER_H
#define ROBOTOS_LIMITER_H

#include <robotos/ro_status.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Inline clamp ───────────────────────────────────────────── */
static inline float limiter_clamp(float v, float lo, float hi)
{
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

/* ── Rate Limiter ───────────────────────────────────────────── */
typedef struct {
    float max_rate;      /* Max |delta| per update call */
    float prev_output;
} rate_limiter_t;

ro_status_t rate_limiter_init(rate_limiter_t* r, float max_rate, float initial);
float       rate_limiter_update(rate_limiter_t* r, float input);

/* ── Acceleration Limiter ───────────────────────────────────── */
typedef struct {
    float max_accel;
    float max_speed;
    float velocity;      /* Internal state */
    float position;      /* Internal state */
} accel_limiter_t;

ro_status_t accel_limiter_init(accel_limiter_t* a,
                                float max_accel, float max_speed);
float       accel_limiter_update(accel_limiter_t* a, float target_pos, float dt);

#ifdef __cplusplus
}
#endif
#endif /* ROBOTOS_LIMITER_H */
