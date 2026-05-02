/*
 * robotos_core.h
 * RobotOS portable core — Phase 4J scheduler budget/backpressure policy.
 *
 * Phase 4I: admission gate in robotos_core_post_event().
 * NONE and reserved types (2–99) are rejected before enqueue.
 * Admission counters track accepted and rejected events.
 *
 * Phase 4J: budget/backpressure observability.
 * dispatch budget = ROBOTOS_CORE_MAX_EVENTS_PER_TICK events drained per tick.
 * backpressure_active = pending_event_count > budget OR queue is full.
 * Backpressure is observability only in Phase 4J — it does not throttle
 * producers, adjust priority, or alter scheduler fairness.
 *
 * No Zephyr or board-specific types.
 */

#ifndef ROBOTOS_CORE_H
#define ROBOTOS_CORE_H

#include <stdint.h>
#include <stdbool.h>

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
	ROBOTOS_CORE_ERR_INVALID_ARG   = -6,  /* invalid argument (e.g. NONE event type) */
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
 * Event handler function type for registered handlers.
 * Must return ROBOTOS_CORE_OK on success; non-OK is a handler error.
 * event is valid for the duration of the call only.
 * user_context is the pointer registered alongside the handler; may be NULL.
 */
typedef robotos_core_status_t (*robotos_core_event_handler_t)(
	const robotos_event_t *event,
	void *user_context
);

/*
 * Snapshot of core state at a point in time.
 * Populated by robotos_core_snapshot(). Not thread-safe.
 * All counter fields are 0 before robotos_core_init() is called.
 *
 * Counter semantics (Phase 4J):
 *   pending_event_count      events currently in queue (drain reduces this)
 *   dropped_event_count      events dropped because queue was full (ERR_FULL path)
 *   admission_accepted_count events that passed admission AND entered queue
 *   admission_rejected_count events rejected by admission gate (invalid type)
 *   dispatched_event_count   events consumed and delivered to routing handler
 *   unhandled_event_count    dispatched events with no matching registered handler
 *   handler_error_count      dispatched events where handler returned non-OK
 *   backpressure_active      true when pending > dispatch budget OR queue is full
 */
typedef struct {
	robotos_core_state_t state;
	uint32_t             tick_count;
	uint32_t             init_count;
	const char          *version;
	uint32_t             pending_event_count;      /* events in queue */
	uint32_t             dropped_event_count;      /* events dropped (queue full) */
	uint32_t             dispatched_event_count;   /* events dispatched */
	uint32_t             handler_error_count;      /* handler errors */
	uint32_t             registered_handler_count; /* currently registered handlers */
	uint32_t             unhandled_event_count;    /* dispatched with no registered handler */
	uint32_t             admission_accepted_count; /* events accepted by admission gate */
	uint32_t             admission_rejected_count; /* events rejected by admission gate */
	bool                 backpressure_active;      /* pending > budget OR queue full */
} robotos_core_snapshot_t;

/* Maximum number of simultaneously registered event handlers. */
#define ROBOTOS_CORE_MAX_EVENT_HANDLERS  8u

/*
 * Maximum events dispatched from the internal queue on each robotos_core_tick() call.
 * Phase 4G stub value: 1.
 */
#define ROBOTOS_CORE_MAX_EVENTS_PER_TICK 1u

/* Return the core version string. Never NULL. */
const char *robotos_core_version(void);

/*
 * Initialize the RobotOS core. Idempotent — safe to call multiple times.
 *
 * First call: state → READY, tick_count=0, init_count=1, queue+dispatcher+handler table init.
 * Subsequent calls: init_count++; tick_count, queue, dispatcher, and handler table NOT reset.
 *
 * Returns ROBOTOS_CORE_OK on success.
 */
robotos_core_status_t robotos_core_init(void);

/*
 * Advance the core by one tick. Requires state == READY.
 * Increments tick_count, then dispatches up to ROBOTOS_CORE_MAX_EVENTS_PER_TICK events.
 * Empty queue is normal — returns OK. Handler errors propagate.
 */
robotos_core_status_t robotos_core_tick(void);

/* Return the current core lifecycle state. */
robotos_core_state_t robotos_core_state(void);

/* Return the current tick count. Returns 0 if not initialized. */
uint32_t robotos_core_tick_count(void);

/* Copy a snapshot of core state. Returns ROBOTOS_CORE_ERR_NULL if out is NULL. */
robotos_core_status_t robotos_core_snapshot(robotos_core_snapshot_t *out);

/*
 * Post an event into the core-owned queue. Requires READY.
 *
 * Admission policy (Phase 4I):
 *   Accepted:  ROBOTOS_EVENT_CORE_TICK and any type >= ROBOTOS_EVENT_USER.
 *   Rejected:  ROBOTOS_EVENT_NONE and reserved range (2–99).
 *   Rejected events increment admission_rejected_count and return ERR_INVALID_ARG.
 *   Accepted events that pass queue push increment admission_accepted_count.
 *
 * Returns ROBOTOS_CORE_ERR_NULL          if event is NULL.
 * Returns ROBOTOS_CORE_ERR_INVALID_STATE if core not READY.
 * Returns ROBOTOS_CORE_ERR_INVALID_ARG   if event type is rejected.
 * Returns ROBOTOS_CORE_ERR_FULL          if queue is full.
 * Returns ROBOTOS_CORE_OK               on success.
 */
robotos_core_status_t robotos_core_post_event(const robotos_event_t *event);

/* Dispatch up to max_events events. Requires READY. ERR_EMPTY if empty and max>0. */
robotos_core_status_t robotos_core_dispatch_events(uint32_t max_events);

/* Event counter getters — all return 0 before init. */
uint32_t robotos_core_pending_event_count(void);
uint32_t robotos_core_dropped_event_count(void);
uint32_t robotos_core_dispatched_event_count(void);
uint32_t robotos_core_handler_error_count(void);

/*
 * Register a handler for a specific event type.
 *
 * Returns ROBOTOS_CORE_ERR_NULL          if handler is NULL.
 * Returns ROBOTOS_CORE_ERR_INVALID_STATE if core is not READY.
 * Returns ROBOTOS_CORE_ERR_INVALID_ARG   if type == ROBOTOS_EVENT_NONE.
 * Returns ROBOTOS_CORE_ERR_FULL          if handler table is at capacity.
 * Returns ROBOTOS_CORE_OK               on success.
 *
 * Registering the same type again replaces the existing handler/context.
 * Replacement does NOT increment registered_handler_count.
 * Single-threaded / non-ISR only. No dynamic allocation.
 */
robotos_core_status_t robotos_core_register_event_handler(
	robotos_event_type_t          type,
	robotos_core_event_handler_t  handler,
	void                         *user_context
);

/*
 * Unregister the handler for a specific event type.
 *
 * Returns ROBOTOS_CORE_ERR_INVALID_STATE if core is not READY.
 * Returns ROBOTOS_CORE_ERR_INVALID_ARG   if type == ROBOTOS_EVENT_NONE.
 * Returns ROBOTOS_CORE_ERR_EMPTY         if no handler is registered for type.
 * Returns ROBOTOS_CORE_OK               on success.
 */
robotos_core_status_t robotos_core_unregister_event_handler(robotos_event_type_t type);

/*
 * Return true if a handler is registered for the given type.
 * Returns false before init or if no handler registered. NULL-safe.
 */
bool robotos_core_has_event_handler(robotos_event_type_t type);

/* Return number of currently registered handlers. Returns 0 before init. */
uint32_t robotos_core_registered_handler_count(void);

/*
 * Return cumulative count of events dispatched with no matching registered handler.
 * These events are consumed and counted but not delivered to any user handler.
 * Returns 0 before init.
 */
uint32_t robotos_core_unhandled_event_count(void);

/*
 * Return cumulative count of events accepted by the admission gate.
 * An event is accepted when its type passes the policy check AND queue push succeeds.
 * Returns 0 before init.
 */
uint32_t robotos_core_admission_accepted_count(void);

/*
 * Return cumulative count of events rejected by the admission gate.
 * Rejected types: ROBOTOS_EVENT_NONE and reserved range (2–99).
 * Returns 0 before init.
 */
uint32_t robotos_core_admission_rejected_count(void);

/*
 * Return the dispatch budget: maximum events drained from the queue per tick.
 * Equal to ROBOTOS_CORE_MAX_EVENTS_PER_TICK. Does not change at runtime.
 * Safe to call before init.
 */
uint32_t robotos_core_dispatch_budget_per_tick(void);

/*
 * Return true when the pending event backlog exceeds the per-tick dispatch budget
 * OR the event queue is full.
 *
 * Backpressure rule (Phase 4J):
 *   backpressure_active = (pending_event_count > ROBOTOS_CORE_MAX_EVENTS_PER_TICK)
 *                         || queue_is_full
 *
 * Observability only: does NOT throttle producers, adjust priority, or change
 * scheduler fairness. Queue-full posts still return ERR_FULL; invalid events
 * still return ERR_INVALID_ARG regardless of backpressure state.
 * Safe to call before init (returns false when queue is uninitialized/empty).
 */
bool robotos_core_backpressure_active(void);

#endif /* ROBOTOS_CORE_H */
