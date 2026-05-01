/*
 * robotos_event_queue.c
 * RobotOS Core Event Queue — Phase 4D implementation.
 *
 * Fixed-capacity ring buffer. No dynamic allocation. No Zephyr dependency.
 * All operations are O(1) and deterministic.
 *
 * Single-threaded assumption: no mutex or atomic operations.
 * Revisit when RTOS threads or ISR access are required.
 */

#include "robotos_event_queue.h"

#include <stddef.h>
#include <string.h>

/* ---- Internal helpers ---------------------------------------------------- */

static bool q_valid(const robotos_event_queue_t *q)
{
	return (q != NULL) && q->initialized;
}

/* -------------------------------------------------------------------------- */

robotos_core_status_t robotos_event_queue_init(robotos_event_queue_t *q)
{
	if (q == NULL) {
		return ROBOTOS_CORE_ERR_NULL;
	}

	memset(q->buffer, 0, sizeof(q->buffer));
	q->head          = 0;
	q->tail          = 0;
	q->count         = 0;
	q->dropped_count = 0;
	q->initialized   = true;

	return ROBOTOS_CORE_OK;
}

bool robotos_event_queue_is_initialized(const robotos_event_queue_t *q)
{
	return (q != NULL) && q->initialized;
}

bool robotos_event_queue_is_empty(const robotos_event_queue_t *q)
{
	if (q == NULL) {
		return true;
	}
	return q->count == 0;
}

bool robotos_event_queue_is_full(const robotos_event_queue_t *q)
{
	if (q == NULL) {
		return false;
	}
	return q->count >= ROBOTOS_EVENT_QUEUE_CAPACITY;
}

uint32_t robotos_event_queue_count(const robotos_event_queue_t *q)
{
	return (q != NULL) ? q->count : 0u;
}

uint32_t robotos_event_queue_capacity(const robotos_event_queue_t *q)
{
	return (q != NULL) ? ROBOTOS_EVENT_QUEUE_CAPACITY : 0u;
}

uint32_t robotos_event_queue_dropped_count(const robotos_event_queue_t *q)
{
	return (q != NULL) ? q->dropped_count : 0u;
}

robotos_core_status_t robotos_event_queue_push(robotos_event_queue_t *q,
                                                const robotos_event_t *event)
{
	if (q == NULL || event == NULL) {
		return ROBOTOS_CORE_ERR_NULL;
	}
	if (!q->initialized) {
		return ROBOTOS_CORE_ERR_INVALID_STATE;
	}
	if (q->count >= ROBOTOS_EVENT_QUEUE_CAPACITY) {
		q->dropped_count++;
		return ROBOTOS_CORE_ERR_FULL;
	}

	q->buffer[q->tail] = *event;
	q->tail = (q->tail + 1u) % ROBOTOS_EVENT_QUEUE_CAPACITY;
	q->count++;

	return ROBOTOS_CORE_OK;
}

robotos_core_status_t robotos_event_queue_pop(robotos_event_queue_t *q,
                                               robotos_event_t *out)
{
	if (q == NULL || out == NULL) {
		return ROBOTOS_CORE_ERR_NULL;
	}
	if (!q->initialized) {
		return ROBOTOS_CORE_ERR_INVALID_STATE;
	}
	if (q->count == 0) {
		return ROBOTOS_CORE_ERR_EMPTY;
	}

	*out = q->buffer[q->head];
	q->head = (q->head + 1u) % ROBOTOS_EVENT_QUEUE_CAPACITY;
	q->count--;

	return ROBOTOS_CORE_OK;
}

robotos_core_status_t robotos_event_queue_peek(const robotos_event_queue_t *q,
                                                robotos_event_t *out)
{
	if (q == NULL || out == NULL) {
		return ROBOTOS_CORE_ERR_NULL;
	}
	if (!q->initialized) {
		return ROBOTOS_CORE_ERR_INVALID_STATE;
	}
	if (q->count == 0) {
		return ROBOTOS_CORE_ERR_EMPTY;
	}

	*out = q->buffer[q->head];

	return ROBOTOS_CORE_OK;
}

robotos_core_status_t robotos_event_queue_clear(robotos_event_queue_t *q)
{
	if (q == NULL) {
		return ROBOTOS_CORE_ERR_NULL;
	}
	if (!q->initialized) {
		return ROBOTOS_CORE_ERR_INVALID_STATE;
	}

	q->head  = 0;
	q->tail  = 0;
	q->count = 0;
	/* dropped_count preserved — diagnostic history */

	return ROBOTOS_CORE_OK;
}
