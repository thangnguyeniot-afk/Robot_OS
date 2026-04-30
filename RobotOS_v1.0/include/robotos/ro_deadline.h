/* ============================================================================
 * ro_deadline.h — RobotOS Deadline Monitoring
 * ============================================================================
 * Layer: Adapter (public header — NO <zephyr/*> includes)
 *
 * Programming technique: Inline deadline bracketing.
 *
 * Usage:
 *   ro_deadline_begin(APP_DL_PLANNER_TICK);
 *   // ... time-critical work ...
 *   ro_deadline_end(APP_DL_PLANNER_TICK);
 *
 *   // Later, check:
 *   uint32_t misses = ro_deadline_miss_count(APP_DL_PLANNER_TICK);
 *
 * Implementation:
 *   - Each deadline ID maps to a static slot in a table (max 16 IDs)
 *   - begin() records ro_time_us() as start timestamp
 *   - end() computes elapsed, compares to budget, increments miss counter
 *   - All operations are O(1), ISR-safe (atomic counter increment)
 *
 * Deadline IDs are uint16_t constants defined by each layer:
 *   - Adapter reserves 0x0000–0x00FF
 *   - App uses 0x0100–0x01FF (APP_DL_* in app_deadlines.h)
 * ========================================================================= */

#ifndef ROBOTOS_RO_DEADLINE_H
#define ROBOTOS_RO_DEADLINE_H

#include <robotos/ro_status.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ---- Maximum registered deadline IDs ------------------------------------- */

#define RO_DEADLINE_MAX_IDS  16

/* ---- Registration -------------------------------------------------------- */

/**
 * Register a deadline ID with its time budget.
 *
 * @param id         Unique deadline ID (uint16_t constant)
 * @param name       Human-readable name (for reporting/debug)
 * @param budget_us  Maximum allowed execution time in microseconds
 *
 * Must be called before any begin/end pair for this ID.
 * Returns: RO_OK, RO_ENOMEM if table full, RO_EINVAL if id already registered.
 */
ro_status_t ro_deadline_register(uint16_t id,
                                  const char* name,
                                  uint32_t budget_us);

/* ---- Bracket ------------------------------------------------------------ */

/**
 * Mark the start of a deadline-monitored section.
 * Records current timestamp for the given ID.
 */
void ro_deadline_begin(uint16_t id);

/**
 * Mark the end of a deadline-monitored section.
 * Computes elapsed time and increments miss counter if > budget.
 */
void ro_deadline_end(uint16_t id);

/* ---- Inspection ---------------------------------------------------------- */

/** Return cumulative miss count for a deadline ID. */
uint32_t ro_deadline_miss_count(uint16_t id);

/** Reset miss counter for a deadline ID to zero. */
void ro_deadline_miss_reset(uint16_t id);

/** Get the name string for a registered deadline ID. NULL if not registered. */
const char* ro_deadline_get_name(uint16_t id);

/** Get the budget (µs) for a registered deadline ID. 0 if not registered. */
uint32_t ro_deadline_get_budget(uint16_t id);

#ifdef __cplusplus
}
#endif

#endif /* ROBOTOS_RO_DEADLINE_H */
