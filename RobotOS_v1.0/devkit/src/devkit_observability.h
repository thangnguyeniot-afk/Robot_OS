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

/*
 * Phase 6L: capture and log Cortex-M fault diagnostic registers.
 *
 * Reads CFSR (0xE000ED28) and HFSR (0xE000ED2C) directly from the
 * ARMv7-M System Control Block and emits one ROBOTOS_FAULT log entry.
 *
 * Stable line shape:
 *   ROBOTOS_FAULT active=0|1 cfsr=0x........ hfsr=0x........ context=<NAME>
 *
 * "active" = (cfsr != 0) || (hfsr != 0). "context" is a coarse string
 * hint -- "none" when active=0, "fault" otherwise. Bit-level decoding
 * (UFSR/BFSR/MMFSR sub-fields, BFAR/MMFAR) is intentionally out of scope
 * for this phase and reserved for a future fault-decoding phase.
 *
 * Passive read-only: does NOT clear fault status (CFSR/HFSR are
 * write-1-to-clear; reading is non-destructive), does NOT panic,
 * does NOT trigger recovery, does NOT influence scheduling.
 *
 * Cortex-M only. Uses direct SCB memory access; no CMSIS header
 * dependency. Thread-context only (snapshot critical section is
 * unaffected; this helper holds no lock).
 */
void devkit_observability_log_fault(void);

#endif /* DEVKIT_OBSERVABILITY_H */
