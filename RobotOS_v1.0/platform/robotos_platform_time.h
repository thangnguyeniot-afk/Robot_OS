/*
 * robotos_platform_time.h
 * RobotOS platform time interface — Phase 5B boundary seed.
 *
 * Provides a minimal, backend-agnostic sleep and uptime API for use by
 * devkit runtime and future portable RobotOS modules.
 *
 * Rules:
 *   - devkit/runtime may call this API; core must NOT call sleep.
 *   - Core tick remains externally driven — the runtime loop calls
 *     robotos_core_tick() and then robotos_platform_sleep_ms() to pace
 *     itself. Core has no knowledge of time.
 *   - This header contains no Zephyr types, no board types, no k_* types.
 *   - Backend selected at build time by compiling the appropriate source.
 *
 * Phase 5B scope: sleep and uptime_ms only.
 * Timer abstraction, deadlines, monotonic 64-bit counters, critical
 * sections, and scheduler integration are NOT in scope here.
 *
 * Uptime note:
 *   robotos_platform_uptime_ms() returns a uint32_t millisecond counter.
 *   This wraps after approximately 49.7 days of continuous uptime.
 *   Wraparound policy is not handled in Phase 5B — document if needed.
 */

#ifndef ROBOTOS_PLATFORM_TIME_H
#define ROBOTOS_PLATFORM_TIME_H

#include <stdint.h>

/*
 * Return the system uptime in milliseconds since boot.
 *
 * Wraps at UINT32_MAX (~49.7 days). Monotonic within a single boot.
 * Thread-safety is backend-specific. Do not call from ISR in Phase 5B.
 */
uint32_t robotos_platform_uptime_ms(void);

/*
 * Sleep for duration_ms milliseconds.
 *
 * If duration_ms == 0, returns immediately (no yield, no blocking).
 * Blocking behavior is backend-specific (e.g. Zephyr k_msleep).
 * Do not call from ISR context.
 */
void robotos_platform_sleep_ms(uint32_t duration_ms);

#endif /* ROBOTOS_PLATFORM_TIME_H */
