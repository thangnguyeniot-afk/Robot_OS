/* ============================================================================
 * ro_timer.h — RobotOS Software Timer
 * ============================================================================
 * Layer: Adapter (public header — NO <zephyr/*> includes)
 *
 * Programming technique: "Timer-sync" pattern.
 * For high-frequency periodic loops (>=200 Hz), the recommended pattern is:
 *
 *   ro_timer_t tmr;
 *   ro_timer_init(&tmr, "planner_tick");
 *   ro_timer_start_periodic(&tmr, period_us);
 *   while (running) {
 *       // ... do work ...
 *       ro_timer_sync(&tmr);   // blocks until next period
 *   }
 *
 * This avoids drift: the timer fires at absolute intervals, and sync()
 * blocks only for the remaining time in the current period.
 *
 * On Zephyr: wraps k_timer + k_timer_status_sync().
 * On host:   wraps clock_nanosleep (POSIX) or Sleep (Windows).
 * ========================================================================= */

#ifndef ROBOTOS_RO_TIMER_H
#define ROBOTOS_RO_TIMER_H

#include <robotos/ro_status.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ---- Timer handle -------------------------------------------------------- */

/**
 * ro_timer_t — Opaque timer object.
 *
 * Data structure: Contains platform-specific timer state.
 * On Zephyr: wraps struct k_timer internally.
 * On host: stores next-deadline timestamp + period.
 *
 * The _impl[] array provides opaque storage large enough for any backend.
 */
typedef struct {
    const char* name;           /* Human-readable label (for debug/trace) */
    uint32_t    period_us;      /* Period in microseconds (0 = not started)*/
    uint8_t     _impl[64];     /* Backend-specific storage (opaque)       */
} ro_timer_t;

/* ---- API ----------------------------------------------------------------- */

/**
 * Initialize a timer object.
 * Must be called before any other ro_timer_* function.
 */
ro_status_t ro_timer_init(ro_timer_t* tmr, const char* name);

/**
 * Start periodic firing at `period_us` microsecond intervals.
 * First expiry occurs one period after this call.
 */
ro_status_t ro_timer_start_periodic(ro_timer_t* tmr, uint32_t period_us);

/**
 * Stop the timer.  Safe to call on an already-stopped timer.
 */
ro_status_t ro_timer_stop(ro_timer_t* tmr);

/**
 * Block the calling thread until the next timer expiry.
 *
 * This is the core of the "timer-sync" pattern:
 *   - If the timer has already expired (work took > 1 period), returns
 *     immediately — the caller can detect overrun by checking elapsed time.
 *   - Otherwise, sleeps for the remaining time in the period.
 *
 * Returns: RO_OK on normal sync, RO_ENOTREADY if timer not started.
 */
ro_status_t ro_timer_sync(ro_timer_t* tmr);

#ifdef __cplusplus
}
#endif

#endif /* ROBOTOS_RO_TIMER_H */
