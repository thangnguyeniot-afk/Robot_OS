/* filter.c — Digital Filters Implementation (Framework Layer)
 *
 * EMA, Moving Average, and second-order Notch (biquad).
 */
#include <robotos/filter.h>
#include <string.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* ══════════════════════════════════════════════════════════════
 *  EMA — Exponential Moving Average
 * ══════════════════════════════════════════════════════════════ */

ro_status_t ema_filter_init(ema_filter_t* f, float alpha)
{
    if (!f)                    return RO_EINVAL;
    if (alpha <= 0.0f || alpha > 1.0f) return RO_EINVAL;
    f->alpha = alpha;
    f->state = 0.0f;
    return RO_OK;
}

float ema_filter_update(ema_filter_t* f, float input)
{
    f->state = f->alpha * input + (1.0f - f->alpha) * f->state;
    return f->state;
}

void ema_filter_reset(ema_filter_t* f, float seed)
{
    if (f) f->state = seed;
}

/* ══════════════════════════════════════════════════════════════
 *  Moving Average (sliding window)
 * ══════════════════════════════════════════════════════════════ */

ro_status_t moving_avg_init(moving_avg_t* f, float* buf, uint32_t n)
{
    if (!f || !buf || n == 0) return RO_EINVAL;
    f->buf  = buf;
    f->n    = n;
    f->head = 0;
    f->sum  = 0.0f;
    memset(buf, 0, sizeof(float) * n);
    return RO_OK;
}

float moving_avg_update(moving_avg_t* f, float input)
{
    f->sum -= f->buf[f->head];
    f->buf[f->head] = input;
    f->sum += input;
    f->head = (f->head + 1) % f->n;
    return f->sum / (float)f->n;
}

void moving_avg_reset(moving_avg_t* f)
{
    if (!f) return;
    memset(f->buf, 0, sizeof(float) * f->n);
    f->head = 0;
    f->sum  = 0.0f;
}

/* ══════════════════════════════════════════════════════════════
 *  Notch Filter — Second-order IIR biquad
 *
 *  H(z) = (b0 + b1*z^-1 + b2*z^-2) / (1 + a1*z^-1 + a2*z^-2)
 *
 *  Design for centre frequency `freq_hz`, sample rate `sample_hz`,
 *  and 3dB `bandwidth` (Hz).
 * ══════════════════════════════════════════════════════════════ */

ro_status_t notch_filter_init(notch_filter_t* f,
                               float freq_hz, float sample_hz, float bandwidth)
{
    if (!f)                          return RO_EINVAL;
    if (freq_hz <= 0.0f || sample_hz <= 0.0f || bandwidth <= 0.0f)
        return RO_EINVAL;

    float w0    = 2.0f * (float)M_PI * freq_hz / sample_hz;
    float alpha = sinf(w0) * sinhf(logf(2.0f) / 2.0f * bandwidth * w0 / sinf(w0));

    float b0 =  1.0f;
    float b1 = -2.0f * cosf(w0);
    float b2 =  1.0f;
    float a0 =  1.0f + alpha;
    float a1 = -2.0f * cosf(w0);
    float a2 =  1.0f - alpha;

    /* Normalize */
    f->b0 = b0 / a0;
    f->b1 = b1 / a0;
    f->b2 = b2 / a0;
    f->a1 = a1 / a0;
    f->a2 = a2 / a0;

    f->x1 = f->x2 = 0.0f;
    f->y1 = f->y2 = 0.0f;
    return RO_OK;
}

float notch_filter_update(notch_filter_t* f, float input)
{
    float y = f->b0 * input + f->b1 * f->x1 + f->b2 * f->x2
                             - f->a1 * f->y1 - f->a2 * f->y2;
    f->x2 = f->x1;  f->x1 = input;
    f->y2 = f->y1;  f->y1 = y;
    return y;
}

void notch_filter_reset(notch_filter_t* f)
{
    if (!f) return;
    f->x1 = f->x2 = 0.0f;
    f->y1 = f->y2 = 0.0f;
}
