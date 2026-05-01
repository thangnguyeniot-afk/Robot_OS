/*
 * robotos_event_dispatcher.c
 * RobotOS Core Event Dispatcher — Phase 4E implementation.
 *
 * Drains events from a queue to a handler in FIFO order.
 * No dynamic allocation. No Zephyr dependency. No locks.
 *
 * Single-threaded assumption: no mutex or atomic operations.
 */

#include "robotos_event_dispatcher.h"

#include <stddef.h>

robotos_core_status_t robotos_event_dispatcher_init(
	robotos_event_dispatcher_t *dispatcher,
	robotos_event_queue_t      *queue,
	robotos_event_handler_t     handler,
	void                       *user_context)
{
	if (dispatcher == NULL || queue == NULL || handler == NULL) {
		return ROBOTOS_CORE_ERR_NULL;
	}
	if (!robotos_event_queue_is_initialized(queue)) {
		return ROBOTOS_CORE_ERR_INVALID_STATE;
	}

	dispatcher->queue               = queue;
	dispatcher->handler             = handler;
	dispatcher->user_context        = user_context;
	dispatcher->dispatched_count    = 0;
	dispatcher->handler_error_count = 0;
	dispatcher->initialized         = true;

	return ROBOTOS_CORE_OK;
}

bool robotos_event_dispatcher_is_initialized(const robotos_event_dispatcher_t *dispatcher)
{
	return (dispatcher != NULL) && dispatcher->initialized;
}

uint32_t robotos_event_dispatcher_dispatched_count(const robotos_event_dispatcher_t *dispatcher)
{
	return (dispatcher != NULL) ? dispatcher->dispatched_count : 0u;
}

uint32_t robotos_event_dispatcher_handler_error_count(const robotos_event_dispatcher_t *dispatcher)
{
	return (dispatcher != NULL) ? dispatcher->handler_error_count : 0u;
}

robotos_core_status_t robotos_event_dispatcher_dispatch_one(
	robotos_event_dispatcher_t *dispatcher)
{
	if (dispatcher == NULL) {
		return ROBOTOS_CORE_ERR_NULL;
	}
	if (!dispatcher->initialized) {
		return ROBOTOS_CORE_ERR_INVALID_STATE;
	}

	robotos_event_t ev;
	robotos_core_status_t ret = robotos_event_queue_pop(dispatcher->queue, &ev);
	if (ret != ROBOTOS_CORE_OK) {
		return ret; /* ERR_EMPTY or other queue error */
	}

	robotos_core_status_t handler_ret = dispatcher->handler(&ev, dispatcher->user_context);
	if (handler_ret != ROBOTOS_CORE_OK) {
		dispatcher->handler_error_count++;
		return handler_ret;
	}

	dispatcher->dispatched_count++;
	return ROBOTOS_CORE_OK;
}

robotos_core_status_t robotos_event_dispatcher_dispatch_all(
	robotos_event_dispatcher_t *dispatcher,
	uint32_t                    max_events)
{
	if (dispatcher == NULL) {
		return ROBOTOS_CORE_ERR_NULL;
	}
	if (!dispatcher->initialized) {
		return ROBOTOS_CORE_ERR_INVALID_STATE;
	}
	if (max_events == 0u) {
		return ROBOTOS_CORE_OK;
	}

	uint32_t dispatched = 0;

	for (uint32_t i = 0; i < max_events; i++) {
		robotos_event_t ev;
		robotos_core_status_t ret = robotos_event_queue_pop(dispatcher->queue, &ev);
		if (ret == ROBOTOS_CORE_ERR_EMPTY) {
			return (dispatched > 0u) ? ROBOTOS_CORE_OK : ROBOTOS_CORE_ERR_EMPTY;
		}
		if (ret != ROBOTOS_CORE_OK) {
			return ret;
		}

		robotos_core_status_t handler_ret = dispatcher->handler(&ev, dispatcher->user_context);
		if (handler_ret != ROBOTOS_CORE_OK) {
			dispatcher->handler_error_count++;
			return handler_ret;
		}

		dispatcher->dispatched_count++;
		dispatched++;
	}

	return ROBOTOS_CORE_OK;
}
