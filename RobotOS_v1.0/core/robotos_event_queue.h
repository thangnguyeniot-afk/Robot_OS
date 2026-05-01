/*
 * robotos_event_queue.h
 * RobotOS Core Event Queue — Phase 4D contract API.
 *
 * Fixed-capacity, single-threaded ring buffer for core events.
 * No dynamic allocation. No Zephyr or board-specific types.
 *
 * NOTE: robotos_event_type_t and robotos_event_t are defined in robotos_core.h,
 * which is included here. They are core concepts, not queue-specific.
 *
 * Limitations (Phase 4D):
 *   - Single-threaded only: no mutex, no ISR-safe access.
 *   - No dispatcher or scheduler — events are pushed/popped by caller.
 *   - Capacity is compile-time constant (ROBOTOS_EVENT_QUEUE_CAPACITY).
 *   - Dynamic resize is not supported.
 */

#ifndef ROBOTOS_EVENT_QUEUE_H
#define ROBOTOS_EVENT_QUEUE_H

/* robotos_core.h provides: robotos_core_status_t, robotos_event_type_t, robotos_event_t */
#include "robotos_core.h"

#include <stdbool.h>

/* Fixed capacity of each event queue instance. */
#define ROBOTOS_EVENT_QUEUE_CAPACITY 16u

/*
 * Ring buffer queue for core events.
 * Caller owns the struct (static or stack allocation).
 * Must be initialized via robotos_event_queue_init() before use.
 */
typedef struct {
	robotos_event_t buffer[ROBOTOS_EVENT_QUEUE_CAPACITY];
	uint32_t        head;
	uint32_t        tail;
	uint32_t        count;
	uint32_t        dropped_count;
	bool            initialized;
} robotos_event_queue_t;

/*
 * Initialize a queue.
 * Returns ROBOTOS_CORE_ERR_NULL if q is NULL.
 * Resets head, tail, count to 0; clears dropped_count and initialized flag.
 * Returns ROBOTOS_CORE_OK on success.
 */
robotos_core_status_t robotos_event_queue_init(robotos_event_queue_t *q);

/*
 * NULL-safe query functions.
 * All return a safe default (false / 0) if q is NULL.
 */
bool     robotos_event_queue_is_initialized(const robotos_event_queue_t *q);
bool     robotos_event_queue_is_empty(const robotos_event_queue_t *q);
bool     robotos_event_queue_is_full(const robotos_event_queue_t *q);
uint32_t robotos_event_queue_count(const robotos_event_queue_t *q);
uint32_t robotos_event_queue_capacity(const robotos_event_queue_t *q);
uint32_t robotos_event_queue_dropped_count(const robotos_event_queue_t *q);

/*
 * Push one event into the queue.
 * Returns ROBOTOS_CORE_ERR_NULL          if q or event is NULL.
 * Returns ROBOTOS_CORE_ERR_INVALID_STATE if queue is not initialized.
 * Returns ROBOTOS_CORE_ERR_FULL          if queue is at capacity; dropped_count++.
 * Returns ROBOTOS_CORE_OK               on success; copies event by value.
 */
robotos_core_status_t robotos_event_queue_push(robotos_event_queue_t *q,
                                                const robotos_event_t *event);

/*
 * Pop the front event (FIFO order).
 * Returns ROBOTOS_CORE_ERR_NULL          if q or out is NULL.
 * Returns ROBOTOS_CORE_ERR_INVALID_STATE if queue is not initialized.
 * Returns ROBOTOS_CORE_ERR_EMPTY         if queue is empty.
 * Returns ROBOTOS_CORE_OK               on success; copies event into *out.
 */
robotos_core_status_t robotos_event_queue_pop(robotos_event_queue_t *q,
                                               robotos_event_t *out);

/*
 * Peek at the front event without removing it.
 * Returns ROBOTOS_CORE_ERR_NULL          if q or out is NULL.
 * Returns ROBOTOS_CORE_ERR_INVALID_STATE if queue is not initialized.
 * Returns ROBOTOS_CORE_ERR_EMPTY         if queue is empty.
 * Returns ROBOTOS_CORE_OK               on success; does not modify queue state.
 */
robotos_core_status_t robotos_event_queue_peek(const robotos_event_queue_t *q,
                                                robotos_event_t *out);

/*
 * Clear all events from the queue.
 * Resets head, tail, count to 0. Preserves dropped_count (diagnostic history).
 * Returns ROBOTOS_CORE_ERR_NULL if q is NULL.
 * Returns ROBOTOS_CORE_ERR_INVALID_STATE if queue is not initialized.
 * Returns ROBOTOS_CORE_OK on success.
 */
robotos_core_status_t robotos_event_queue_clear(robotos_event_queue_t *q);

#endif /* ROBOTOS_EVENT_QUEUE_H */
