/*
 * robotos_core.h
 * RobotOS portable core — Phase 4B contract API.
 *
 * Phase 4B: explicit lifecycle states, status codes, and introspection API.
 * Scheduler, event bus, and peripherals are intentionally not implemented yet.
 * No Zephyr or board-specific types exposed in this header.
 */

#ifndef ROBOTOS_CORE_H
#define ROBOTOS_CORE_H

#include <stdint.h>

/*
 * Status/error codes returned by core API functions.
 * ROBOTOS_CORE_OK is always zero.
 * Error codes are negative.
 */
typedef enum {
	ROBOTOS_CORE_OK                = 0,
	ROBOTOS_CORE_ERR_INVALID_STATE = -1,
	ROBOTOS_CORE_ERR_NULL          = -2,
	ROBOTOS_CORE_ERR_FULL          = -3,
	ROBOTOS_CORE_ERR_EMPTY         = -4,
} robotos_core_status_t;

/*
 * Core lifecycle state.
 * Starts UNINITIALIZED on power-on.
 * Transitions to READY on first successful robotos_core_init().
 * ERROR is reserved for future fault conditions.
 */
typedef enum {
	ROBOTOS_CORE_STATE_UNINITIALIZED = 0,
	ROBOTOS_CORE_STATE_READY,
	ROBOTOS_CORE_STATE_ERROR,
} robotos_core_state_t;

/*
 * Snapshot of core state at a point in time.
 * Populated by robotos_core_snapshot().
 *
 * NOTE: No locking is applied. Valid only in single-threaded contexts.
 * Future phases may add mutex protection when RTOS threads are introduced.
 */
typedef struct {
	robotos_core_state_t state;
	uint32_t             tick_count;
	uint32_t             init_count;
	const char          *version;
} robotos_core_snapshot_t;

/* Return the core version string. Never NULL. */
const char *robotos_core_version(void);

/*
 * Initialize the RobotOS core. Idempotent — safe to call multiple times.
 *
 * First call:
 *   - transitions state to ROBOTOS_CORE_STATE_READY
 *   - resets tick_count to 0
 *   - sets init_count to 1
 *   - emits full init log message
 *
 * Subsequent calls:
 *   - increments init_count
 *   - does NOT reset tick_count
 *   - emits low-noise log only (LOG_DBG level, suppressed at default log level)
 *   - returns ROBOTOS_CORE_OK
 *
 * Returns ROBOTOS_CORE_OK on success.
 */
robotos_core_status_t robotos_core_init(void);

/*
 * Advance the RobotOS core by one tick.
 *
 * Requires state == ROBOTOS_CORE_STATE_READY.
 * Returns ROBOTOS_CORE_ERR_INVALID_STATE if called before init or in ERROR state.
 * On the first invalid-state tick call, emits a LOG_WRN. Subsequent calls are silent.
 *
 * On success: increments tick_count, logs at first tick and every 10th tick.
 * Core has no internal timer — timing is owned by the caller.
 *
 * Returns ROBOTOS_CORE_OK on success.
 */
robotos_core_status_t robotos_core_tick(void);

/* Return the current core lifecycle state. */
robotos_core_state_t robotos_core_state(void);

/*
 * Return the current tick count.
 * Returns 0 if core has not been initialized or tick has never been called.
 */
uint32_t robotos_core_tick_count(void);

/*
 * Copy a snapshot of core state into caller-provided struct.
 *
 * Returns ROBOTOS_CORE_ERR_NULL if out is NULL.
 * Not thread-safe — valid only in single-threaded contexts.
 * Returns ROBOTOS_CORE_OK on success.
 */
robotos_core_status_t robotos_core_snapshot(robotos_core_snapshot_t *out);

#endif /* ROBOTOS_CORE_H */
