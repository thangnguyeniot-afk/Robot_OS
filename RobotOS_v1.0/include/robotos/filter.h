/* filter.h — Digital Filters (Framework Layer)
 *   EMA  – Exponential Moving Average
 *   MA   – Moving Average (sliding window)
 *   Notch – Second-order IIR biquad notch filter
 */
#ifndef ROBOTOS_FILTER_H
#define ROBOTOS_FILTER_H

#include <robotos/ro_status.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── EMA ────────────────────────────────────────────────────── */
typedef struct {
    float alpha;   /* 0 < alpha ≤ 1           */
    float state;
} ema_filter_t;

ro_status_t ema_filter_init(ema_filter_t* f, float alpha);
float       ema_filter_update(ema_filter_t* f, float input);
void        ema_filter_reset(ema_filter_t* f, float seed);

/* ── Moving Average ─────────────────────────────────────────── */
typedef struct {
    float*   buf;    /* Caller-provided buffer of size n */
    uint32_t n;
    uint32_t head;
    float    sum;
} moving_avg_t;

ro_status_t moving_avg_init(moving_avg_t* f, float* buf, uint32_t n);
float       moving_avg_update(moving_avg_t* f, float input);
void        moving_avg_reset(moving_avg_t* f);

/* ── Notch (biquad) ─────────────────────────────────────────── */
typedef struct {
    /* Coefficients */
    float b0, b1, b2;
    float a1, a2;
    /* State (Direct-Form II transposed) */
    float x1, x2;
    float y1, y2;
} notch_filter_t;

ro_status_t notch_filter_init(notch_filter_t* f,
                               float freq_hz, float sample_hz, float bandwidth);
float       notch_filter_update(notch_filter_t* f, float input);
void        notch_filter_reset(notch_filter_t* f);

#ifdef __cplusplus
}
#endif
#endif /* ROBOTOS_FILTER_H */
