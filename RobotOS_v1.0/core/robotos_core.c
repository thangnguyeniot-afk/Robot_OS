/*
 * robotos_core.c
 * RobotOS portable core — Phase 4G: scheduler tick policy stub.
 *
 * Zephyr logging is used in firmware builds. The public API (robotos_core.h)
 * remains free of Zephyr types.
 *
 * When ROBOTOS_CORE_HOST_TEST is defined (host contract tests), all log
 * output is silenced. No Zephyr headers are included in that mode.
 *
 * Single-threaded assumption: no mutex or atomic operations applied.
 * Revisit when RTOS threads are introduced.
 */

#include "robotos_core.h"
#include "robotos_event_queue.h"
#include "robotos_event_dispatcher.h"

#include <stdbool.h>
#include <stddef.h>

/* ---- Internal logging shim ------------------------------------------------
 * In firmware mode: use Zephyr LOG_MODULE_REGISTER + LOG_* macros.
 * In host test mode: silence all log output (no Zephyr dependency).
 */
#ifdef ROBOTOS_CORE_HOST_TEST
#define CORE_LOG_INF(...) ((void)0)
#define CORE_LOG_WRN(...) ((void)0)
#define CORE_LOG_DBG(...) ((void)0)
#else
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(robotos_core, LOG_LEVEL_INF);
#define CORE_LOG_INF(...) LOG_INF(__VA_ARGS__)
#define CORE_LOG_WRN(...) LOG_WRN(__VA_ARGS__)
#define CORE_LOG_DBG(...) LOG_DBG(__VA_ARGS__)
#endif
/* -------------------------------------------------------------------------- */

#define CORE_VERSION           "4B-contract"
#define CORE_TICK_LOG_INTERVAL  10

static robotos_core_state_t      core_state     = ROBOTOS_CORE_STATE_UNINITIALIZED;
static uint32_t                  core_tick_count;
static uint32_t                  core_init_count;
static bool                      core_tick_warn_emitted;
static robotos_event_queue_t     s_core_event_queue;
static robotos_event_dispatcher_t s_core_dispatcher;

/* Default no-op handler used by the internal dispatcher. */
static robotos_core_status_t s_core_default_handler(const robotos_event_t *event,
                                                      void *ctx)
{
	(void)event;
	(void)ctx;
	return ROBOTOS_CORE_OK;
}

const char *robotos_core_version(void)
{
	return CORE_VERSION;
}

robotos_core_status_t robotos_core_init(void)
{
	if (core_state == ROBOTOS_CORE_STATE_READY) {
		core_init_count++;
		CORE_LOG_DBG("core already initialized (init_count=%u)", core_init_count);
		return ROBOTOS_CORE_OK;
	}

	core_tick_count        = 0;
	core_init_count        = 1;
	core_tick_warn_emitted = false;
	core_state             = ROBOTOS_CORE_STATE_READY;

	robotos_event_queue_init(&s_core_event_queue);
	robotos_event_dispatcher_init(&s_core_dispatcher, &s_core_event_queue,
	                              s_core_default_handler, NULL);

	CORE_LOG_INF("RobotOS core init — version=%s state=READY", CORE_VERSION);
	CORE_LOG_INF("event queue initialized capacity=%u",
	             ROBOTOS_EVENT_QUEUE_CAPACITY);
	CORE_LOG_INF("event dispatcher initialized");

	return ROBOTOS_CORE_OK;
}

robotos_core_status_t robotos_core_tick(void)
{
	if (core_state != ROBOTOS_CORE_STATE_READY) {
		if (!core_tick_warn_emitted) {
			CORE_LOG_WRN("core tick before init — ignored");
			core_tick_warn_emitted = true;
		}
		return ROBOTOS_CORE_ERR_INVALID_STATE;
	}

	core_tick_count++;

	if (core_tick_count == 1 ||
	    (core_tick_count % CORE_TICK_LOG_INTERVAL) == 0) {
		CORE_LOG_INF("core tick count=%u", core_tick_count);
	}

	/* Tick policy: dispatch up to ROBOTOS_CORE_MAX_EVENTS_PER_TICK events.
	 * Empty queue is normal — normalised to OK. */
	robotos_core_status_t dret = robotos_event_dispatcher_dispatch_all(
		&s_core_dispatcher, ROBOTOS_CORE_MAX_EVENTS_PER_TICK);

	if (dret == ROBOTOS_CORE_ERR_EMPTY) {
		return ROBOTOS_CORE_OK;
	}
	return dret; /* ROBOTOS_CORE_OK or handler's non-OK status */
}

robotos_core_state_t robotos_core_state(void)
{
	return core_state;
}

uint32_t robotos_core_tick_count(void)
{
	return core_tick_count;
}

robotos_core_status_t robotos_core_snapshot(robotos_core_snapshot_t *out)
{
	if (out == NULL) {
		return ROBOTOS_CORE_ERR_NULL;
	}

	out->state                  = core_state;
	out->tick_count             = core_tick_count;
	out->init_count             = core_init_count;
	out->version                = CORE_VERSION;
	out->pending_event_count    = robotos_event_queue_count(&s_core_event_queue);
	out->dropped_event_count    = robotos_event_queue_dropped_count(&s_core_event_queue);
	out->dispatched_event_count = robotos_event_dispatcher_dispatched_count(&s_core_dispatcher);
	out->handler_error_count    = robotos_event_dispatcher_handler_error_count(&s_core_dispatcher);

	return ROBOTOS_CORE_OK;
}

robotos_core_status_t robotos_core_post_event(const robotos_event_t *event)
{
	if (event == NULL) {
		return ROBOTOS_CORE_ERR_NULL;
	}
	if (core_state != ROBOTOS_CORE_STATE_READY) {
		return ROBOTOS_CORE_ERR_INVALID_STATE;
	}
	return robotos_event_queue_push(&s_core_event_queue, event);
}

robotos_core_status_t robotos_core_dispatch_events(uint32_t max_events)
{
	if (core_state != ROBOTOS_CORE_STATE_READY) {
		return ROBOTOS_CORE_ERR_INVALID_STATE;
	}
	if (max_events == 0u) {
		return ROBOTOS_CORE_OK;
	}
	return robotos_event_dispatcher_dispatch_all(&s_core_dispatcher, max_events);
}

uint32_t robotos_core_pending_event_count(void)
{
	return robotos_event_queue_count(&s_core_event_queue);
}

uint32_t robotos_core_dropped_event_count(void)
{
	return robotos_event_queue_dropped_count(&s_core_event_queue);
}

uint32_t robotos_core_dispatched_event_count(void)
{
	return robotos_event_dispatcher_dispatched_count(&s_core_dispatcher);
}

uint32_t robotos_core_handler_error_count(void)
{
	return robotos_event_dispatcher_handler_error_count(&s_core_dispatcher);
}
