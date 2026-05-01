/*
 * robotos_core.h
 * RobotOS portable core — Phase 4G scheduler tick policy stub.
 *
 * Phase 4G: each tick dispatches up to ROBOTOS_CORE_MAX_EVENTS_PER_TICK events.
 * This is a deterministic tick policy stub, not a scheduler.
 * No task registry, no priority, no threads, no Zephyr types.
 */

#ifndef ROBOTOS_CORE_H
#define ROBOTOS_CORE_H

#include <stdint.h>

/*
 * Status/error codes returned by core API functions.
 * ROBOTOS_CORE_OK is always zero. Error codes are negative.
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
 * Core event types.
 * Defined here because events are a core concept shared by queue and dispatcher.
 * ROBOTOS_EVENT_NONE      = no event / empty slot.
 * ROBOTOS_EVENT_CORE_TICK = one devkit tick elapsed (reserved for future use).
 * ROBOTOS_EVENT_USER      = base value for application-defined events.
 */
typedef enum {
	ROBOTOS_EVENT_NONE      = 0,
	ROBOTOS_EVENT_CORE_TICK = 1,
	ROBOTOS_EVENT_USER      = 100,
} robotos_event_type_t;

/*
 * Generic core event.
 * type            — identifies the event class.
 * timestamp_tick  — core tick_count at time of push; set by caller.
 * arg0, arg1      — optional event-specific payload.
 */
typedef struct {
	robotos_event_type_t type;
	uint32_t             timestamp_tick;
	uint32_t             arg0;
	uint32_t             arg1;
} robotos_event_t;

/*
 * Snapshot of core state at a point in time.
 * Populated by robotos_core_snapshot(). Not thread-safe.
 *
 * Event counter fields reflect the internal queue/dispatcher state.
 * All counter fields are 0 before robotos_core_init() is called.
 */
typedef struct {
	robotos_core_state_t state;
	uint32_t             tick_count;
	uint32_t             init_count;
	const char          *version;
	uint32_t             pending_event_count;    /* events currently in queue */
	uint32_t             dropped_event_count;    /* events dropped (queue full) */
	uint32_t             dispatched_event_count; /* events dispatched by dispatcher */
	uint32_t             handler_error_count;    /* handler errors during dispatch */
} robotos_core_snapshot_t;

/*
 * Maximum events dispatched from the internal queue on each robotos_core_tick() call.
 * Bounds the dispatch budget per tick. Must be > 0.
 * Phase 4G stub value: 1. Callers may drain remaining events via dispatch_events().
 */
#define ROBOTOS_CORE_MAX_EVENTS_PER_TICK 1u

/* Return the core version string. Never NULL. */
const char *robotos_core_version(void);

/*
 * Initialize the RobotOS core. Idempotent — safe to call multiple times.
 *
 * First call: state → READY, tick_count=0, init_count=1, queue+dispatcher init.
 * Subsequent calls: init_count++, NO reset of tick_count or event queue.
 *
 * Returns ROBOTOS_CORE_OK on success.
 */
robotos_core_status_t robotos_core_init(void);

/*
 * Advance the core by one tick. Requires state == READY.
 *
 * On each successful tick:
 *   1. tick_count increments.
 *   2. Up to ROBOTOS_CORE_MAX_EVENTS_PER_TICK events dispatched from internal queue.
 *   3. Empty queue is normal — returns OK (not ERR_EMPTY).
 *   4. Handler errors are returned and reflected in handler_error_count.
 *   5. No automatic CORE_TICK event is generated.
 *   6. State does not transition to ERROR on handler error (Phase 4G stub).
 *
 * Returns ROBOTOS_CORE_ERR_INVALID_STATE before init (warns once, then silent).
 * Returns ROBOTOS_CORE_OK on success.
 * Returns handler's non-OK status if dispatch encounters a handler error.
 */
robotos_core_status_t robotos_core_tick(void);

/* Return the current core lifecycle state. */
robotos_core_state_t robotos_core_state(void);

/* Return the current tick count. Returns 0 if not initialized. */
uint32_t robotos_core_tick_count(void);

/*
 * Copy a snapshot of core state into caller-provided struct.
 * Returns ROBOTOS_CORE_ERR_NULL if out is NULL.
 */
robotos_core_status_t robotos_core_snapshot(robotos_core_snapshot_t *out);

/*
 * Post an event into the core-owned event queue.
 *
 * Returns ROBOTOS_CORE_ERR_NULL          if event is NULL.
 * Returns ROBOTOS_CORE_ERR_INVALID_STATE if core is not READY.
 * Returns ROBOTOS_CORE_ERR_FULL          if queue is at capacity (dropped_count++).
 * Returns ROBOTOS_CORE_OK               on success.
 *
 * No automatic dispatch — call robotos_core_dispatch_events() to drain.
 * Single-threaded / non-ISR only. No scheduler semantics.
 */
robotos_core_status_t robotos_core_post_event(const robotos_event_t *event);

/*
 * Dispatch up to max_events events from the core-owned queue.
 *
 * Returns ROBOTOS_CORE_ERR_INVALID_STATE if core is not READY.
 * Returns ROBOTOS_CORE_OK               if max_events == 0.
 * Returns ROBOTOS_CORE_ERR_EMPTY         if max_events > 0 and no events pending.
 * Returns ROBOTOS_CORE_OK               if at least one event dispatched.
 * Returns handler's non-OK status        if handler fails.
 *
 * Single-threaded / non-ISR only. No scheduler semantics.
 */
robotos_core_status_t robotos_core_dispatch_events(uint32_t max_events);

/* Current number of events in the internal queue. Returns 0 before init. */
uint32_t robotos_core_pending_event_count(void);

/* Cumulative events dropped due to full queue. Returns 0 before init. */
uint32_t robotos_core_dropped_event_count(void);

/* Cumulative events dispatched by the internal dispatcher. Returns 0 before init. */
uint32_t robotos_core_dispatched_event_count(void);

/* Cumulative handler errors during dispatch. Returns 0 before init. */
uint32_t robotos_core_handler_error_count(void);

#endif /* ROBOTOS_CORE_H */
