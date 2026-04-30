/* ============================================================================
 * ro_time.h — RobotOS Time Utilities
 * ============================================================================
 * Layer: Adapter (public header — NO <zephyr/*> includes)
 *
 * Provides monotonic time reading and busy-wait delays.
 * Time source: Zephyr k_uptime_ticks (firmware) or clock_gettime (host).
 * ========================================================================= */

#ifndef ROBOTOS_RO_TIME_H
#define ROBOTOS_RO_TIME_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Return monotonic time in microseconds since boot. */
uint64_t ro_time_us(void);

/** Return monotonic time in milliseconds since boot. */
uint32_t ro_time_ms(void);

/** Busy-wait delay for `us` microseconds (ISR-safe, CPU-blocking). */
void ro_delay_us(uint32_t us);

/** Busy-wait delay for `ms` milliseconds (ISR-safe, CPU-blocking). */
void ro_delay_ms(uint32_t ms);

#ifdef __cplusplus
}
#endif

#endif /* ROBOTOS_RO_TIME_H */
