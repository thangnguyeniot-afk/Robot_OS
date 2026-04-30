/* encoder.h — Rotary / Linear Encoder API (Framework Layer) */
#ifndef ROBOTOS_ENCODER_H
#define ROBOTOS_ENCODER_H

#include <robotos/ro_status.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct encoder encoder_t;

encoder_t*  encoder_get(const char* dt_label);
void        encoder_put(encoder_t* enc);

/* ISR-safe — returns atomically-loaded count */
int32_t     encoder_get_count(const encoder_t* enc);
ro_status_t encoder_reset(encoder_t* enc);

uint32_t    encoder_get_ticks_per_rev(const encoder_t* enc);

/* Returns velocity in ticks/s using a sliding window of window_ms */
float       encoder_get_velocity(const encoder_t* enc, uint32_t window_ms);

#ifdef __cplusplus
}
#endif
#endif /* ROBOTOS_ENCODER_H */
