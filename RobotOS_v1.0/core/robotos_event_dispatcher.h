/*
 * robotos_event_dispatcher.h
 * RobotOS Core Event Dispatcher — Phase 4E contract API.
 *
 * Lightweight, deterministic dispatcher that drains events from a queue
 * to a registered handler in FIFO order.
 *
 * Limitations (Phase 4E):
 *   - Single-threaded only: no mutex, no ISR-safe access.
 *   - No scheduler: caller decides when to dispatch.
 *   - No priority: strict FIFO from the bound queue.
 *   - Dispatcher does not own the queue or allocate memory.
 *   - No Zephyr or board-specific types.
 */

#ifndef ROBOTOS_EVENT_DISPATCHER_H
#define ROBOTOS_EVENT_DISPATCHER_H

#include "robotos_core.h"
#include "robotos_event_queue.h"

#include <stdbool.h>
#include <stdint.h>

/*
 * Event handler function type.
 *
 * Called once per dispatched event. Must return ROBOTOS_CORE_OK on success.
 * Non-OK return is treated as a handler error: handler_error_count increments
 * and dispatch stops (for dispatch_all).
 *
 * The event pointer is valid only for the duration of the call.
 * user_context is whatever was registered at init; may be NULL.
 */
typedef robotos_core_status_t (*robotos_event_handler_t)(
	const robotos_event_t *event,
	void *user_context
);

/*
 * Event dispatcher object.
 * Caller owns the struct (static or stack). Not heap-allocated.
 * Must be initialized via robotos_event_dispatcher_init() before use.
 */
typedef struct {
	robotos_event_queue_t   *queue;
	robotos_event_handler_t  handler;
	void                    *user_context;
	bool                     initialized;
	uint32_t                 dispatched_count;
	uint32_t                 handler_error_count;
} robotos_event_dispatcher_t;

/*
 * Initialize a dispatcher.
 *
 * Returns ROBOTOS_CORE_ERR_NULL          if dispatcher, queue, or handler is NULL.
 * Returns ROBOTOS_CORE_ERR_INVALID_STATE if queue is not initialized.
 * Returns ROBOTOS_CORE_OK               on success.
 *
 * Calling init again on an already-initialized dispatcher is valid:
 * resets all counters, updates queue/handler/context.
 */
robotos_core_status_t robotos_event_dispatcher_init(
	robotos_event_dispatcher_t *dispatcher,
	robotos_event_queue_t      *queue,
	robotos_event_handler_t     handler,
	void                       *user_context
);

/*
 * NULL-safe query functions.
 * Return safe defaults (false / 0) when dispatcher is NULL.
 */
bool     robotos_event_dispatcher_is_initialized(const robotos_event_dispatcher_t *dispatcher);
uint32_t robotos_event_dispatcher_dispatched_count(const robotos_event_dispatcher_t *dispatcher);
uint32_t robotos_event_dispatcher_handler_error_count(const robotos_event_dispatcher_t *dispatcher);

/*
 * Dispatch one event from the bound queue.
 *
 * Returns ROBOTOS_CORE_ERR_NULL          if dispatcher is NULL.
 * Returns ROBOTOS_CORE_ERR_INVALID_STATE if dispatcher is not initialized.
 * Returns ROBOTOS_CORE_ERR_EMPTY         if queue is empty.
 * Returns handler's non-OK status        if handler fails (handler_error_count++).
 * Returns ROBOTOS_CORE_OK               on success (dispatched_count++).
 *
 * The event is always consumed from the queue before the handler is called.
 */
robotos_core_status_t robotos_event_dispatcher_dispatch_one(
	robotos_event_dispatcher_t *dispatcher
);

/*
 * Dispatch up to max_events events from the bound queue.
 *
 * Returns ROBOTOS_CORE_ERR_NULL          if dispatcher is NULL.
 * Returns ROBOTOS_CORE_ERR_INVALID_STATE if dispatcher is not initialized.
 * Returns ROBOTOS_CORE_OK               if max_events == 0 (nothing requested).
 * Returns ROBOTOS_CORE_ERR_EMPTY         if max_events > 0 and no events were dispatched.
 * Returns ROBOTOS_CORE_OK               if at least one event was dispatched before queue emptied.
 * Returns handler's non-OK status        if handler fails (stops dispatch; handler_error_count++).
 *
 * Events are dispatched in FIFO order. Dispatch stops on handler error.
 * Events causing handler errors are already consumed from the queue.
 */
robotos_core_status_t robotos_event_dispatcher_dispatch_all(
	robotos_event_dispatcher_t *dispatcher,
	uint32_t                    max_events
);

#endif /* ROBOTOS_EVENT_DISPATCHER_H */
