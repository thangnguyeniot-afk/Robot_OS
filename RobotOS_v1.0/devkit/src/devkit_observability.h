/*
 * devkit_observability.h
 * RobotOS devkit observability surface (Phase 6K).
 *
 * Provides a single one-line log of the robotos_core_snapshot() for
 * periodic visibility on hardware/devkit RTT logs.
 *
 * Passive: reads core state via the existing snapshot API and emits
 * a stable, grep-friendly LOG_INF line. Does NOT participate in
 * scheduling, dispatch, admission, throttle, or retry decisions.
 *
 * Thread-context only. Not safe to call from ISR (snapshot uses
 * platform critical section internally; the log macro itself is the
 * Zephyr LOG_INF deferred path which is thread-safe but we keep the
 * helper devkit-thread-only by convention).
 */

#ifndef DEVKIT_OBSERVABILITY_H
#define DEVKIT_OBSERVABILITY_H

#include <stdint.h>

/*
 * Log cadence: emit one ROBOTOS_OBS line every N runtime ticks.
 * At DEVKIT_TICK_MS = 500, N = 10 means once every ~5 seconds.
 * Bounded log rate by design; not configurable at runtime.
 */
#define DEVKIT_OBSERVABILITY_LOG_INTERVAL_TICKS 10u

/*
 * Capture a core snapshot and emit one ROBOTOS_OBS log entry.
 *
 * Stable line shape (single line, integer/boolean fields only):
 *   ROBOTOS_OBS state=<NAME> ticks=N pending=N peak=N dropped=N
 *               dispatched=N herr=N throttled=N rejected=N accepted=N
 *               unhandled=N bp=0|1 th_active=0|1
 *
 * If the snapshot API fails, a single error line is emitted instead
 * and the function returns; no fault is reported, no retry is performed.
 */
void devkit_observability_log_snapshot(void);

#endif /* DEVKIT_OBSERVABILITY_H */
