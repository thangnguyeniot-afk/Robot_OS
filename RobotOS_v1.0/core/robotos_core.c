/*
 * robotos_core.c
 * RobotOS portable core — Phase 5A: platform logging boundary.
 *
 * Logging is routed through robotos_platform_log.h — a portable interface.
 * The Zephyr backend (platform/zephyr/robotos_platform_log_zephyr.c) is
 * compiled for devkit builds. The host no-op stub is compiled for host tests.
 *
 * robotos_core.c no longer includes <zephyr/logging/log.h> directly.
 * The public API (robotos_core.h) remains free of Zephyr types.
 *
 * Single-threaded assumption: no mutex or atomic operations applied.
 * Revisit when RTOS threads are introduced.
 */

#include "robotos_core.h"
#include "robotos_event_queue.h"
#include "robotos_event_dispatcher.h"
#include "robotos_platform_log.h"

#include <stdbool.h>
#include <stddef.h>

/* ---- Internal logging helpers ---------------------------------------------
 * Thin wrappers that fix the module label to "robotos_core".
 * All log output goes through robotos_platform_logf() — no Zephyr dependency.
 */
#define CORE_LOG_INF(fmt, ...) \
    robotos_platform_logf(ROBOTOS_LOG_LEVEL_INFO,  "robotos_core", fmt, ##__VA_ARGS__)
#define CORE_LOG_WRN(fmt, ...) \
    robotos_platform_logf(ROBOTOS_LOG_LEVEL_WARN,  "robotos_core", fmt, ##__VA_ARGS__)
#define CORE_LOG_DBG(fmt, ...) \
    robotos_platform_logf(ROBOTOS_LOG_LEVEL_DEBUG, "robotos_core", fmt, ##__VA_ARGS__)
/* -------------------------------------------------------------------------- */

#define CORE_VERSION           "4B-contract"
#define CORE_TICK_LOG_INTERVAL  10

/* ---- Handler table -------------------------------------------------------- */

typedef struct {
	robotos_event_type_t         type;
	robotos_core_event_handler_t handler;
	void                        *user_context;
	bool                         in_use;
} s_handler_entry_t;

static s_handler_entry_t s_handler_table[ROBOTOS_CORE_MAX_EVENT_HANDLERS];
static uint32_t          s_handler_count;
static uint32_t          s_unhandled_event_count;

/* -------------------------------------------------------------------------- */

static robotos_core_state_t      core_state     = ROBOTOS_CORE_STATE_UNINITIALIZED;
static uint32_t                  core_tick_count;
static uint32_t                  core_init_count;
static bool                      core_tick_warn_emitted;
static robotos_event_queue_t     s_core_event_queue;
static robotos_event_dispatcher_t s_core_dispatcher;

/*
 * Routing handler: looks up registered handler by event type.
 * Unregistered types: unhandled_event_count++, returns OK.
 */
static robotos_core_status_t s_core_routing_handler(const robotos_event_t *event,
                                                      void *ctx)
{
	(void)ctx;
	if (event == NULL) {
		return ROBOTOS_CORE_ERR_NULL;
	}
	for (uint32_t i = 0; i < ROBOTOS_CORE_MAX_EVENT_HANDLERS; i++) {
		if (s_handler_table[i].in_use &&
		    s_handler_table[i].type == event->type) {
			return s_handler_table[i].handler(event,
			                                  s_handler_table[i].user_context);
		}
	}
	/* No registered handler for this event type */
	s_unhandled_event_count++;
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

	/* Initialize handler table only on first init */
	for (uint32_t i = 0; i < ROBOTOS_CORE_MAX_EVENT_HANDLERS; i++) {
		s_handler_table[i].in_use       = false;
		s_handler_table[i].handler      = NULL;
		s_handler_table[i].user_context = NULL;
	}
	s_handler_count        = 0;
	s_unhandled_event_count = 0;

	robotos_event_queue_init(&s_core_event_queue);
	robotos_event_dispatcher_init(&s_core_dispatcher, &s_core_event_queue,
	                              s_core_routing_handler, NULL);

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

	/* Tick policy: dispatch up to ROBOTOS_CORE_MAX_EVENTS_PER_TICK events. */
	robotos_core_status_t dret = robotos_event_dispatcher_dispatch_all(
		&s_core_dispatcher, ROBOTOS_CORE_MAX_EVENTS_PER_TICK);

	if (dret == ROBOTOS_CORE_ERR_EMPTY) {
		return ROBOTOS_CORE_OK;
	}
	return dret;
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

	out->state                    = core_state;
	out->tick_count               = core_tick_count;
	out->init_count               = core_init_count;
	out->version                  = CORE_VERSION;
	out->pending_event_count      = robotos_event_queue_count(&s_core_event_queue);
	out->dropped_event_count      = robotos_event_queue_dropped_count(&s_core_event_queue);
	out->dispatched_event_count   = robotos_event_dispatcher_dispatched_count(&s_core_dispatcher);
	out->handler_error_count      = robotos_event_dispatcher_handler_error_count(&s_core_dispatcher);
	out->registered_handler_count = s_handler_count;
	out->unhandled_event_count    = s_unhandled_event_count;

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

/* ---- Handler registration API -------------------------------------------- */

robotos_core_status_t robotos_core_register_event_handler(
	robotos_event_type_t          type,
	robotos_core_event_handler_t  handler,
	void                         *user_context)
{
	if (handler == NULL) {
		return ROBOTOS_CORE_ERR_NULL;
	}
	if (core_state != ROBOTOS_CORE_STATE_READY) {
		return ROBOTOS_CORE_ERR_INVALID_STATE;
	}
	if (type == ROBOTOS_EVENT_NONE) {
		return ROBOTOS_CORE_ERR_INVALID_ARG;
	}

	/* Check for replacement of existing handler for same type */
	for (uint32_t i = 0; i < ROBOTOS_CORE_MAX_EVENT_HANDLERS; i++) {
		if (s_handler_table[i].in_use && s_handler_table[i].type == type) {
			s_handler_table[i].handler      = handler;
			s_handler_table[i].user_context = user_context;
			return ROBOTOS_CORE_OK; /* replacement: count unchanged */
		}
	}

	/* New registration */
	if (s_handler_count >= ROBOTOS_CORE_MAX_EVENT_HANDLERS) {
		return ROBOTOS_CORE_ERR_FULL;
	}

	for (uint32_t i = 0; i < ROBOTOS_CORE_MAX_EVENT_HANDLERS; i++) {
		if (!s_handler_table[i].in_use) {
			s_handler_table[i].type         = type;
			s_handler_table[i].handler      = handler;
			s_handler_table[i].user_context = user_context;
			s_handler_table[i].in_use       = true;
			s_handler_count++;
			return ROBOTOS_CORE_OK;
		}
	}

	return ROBOTOS_CORE_ERR_FULL; /* defensive: should not reach here */
}

robotos_core_status_t robotos_core_unregister_event_handler(robotos_event_type_t type)
{
	if (core_state != ROBOTOS_CORE_STATE_READY) {
		return ROBOTOS_CORE_ERR_INVALID_STATE;
	}
	if (type == ROBOTOS_EVENT_NONE) {
		return ROBOTOS_CORE_ERR_INVALID_ARG;
	}

	for (uint32_t i = 0; i < ROBOTOS_CORE_MAX_EVENT_HANDLERS; i++) {
		if (s_handler_table[i].in_use && s_handler_table[i].type == type) {
			s_handler_table[i].in_use       = false;
			s_handler_table[i].handler      = NULL;
			s_handler_table[i].user_context = NULL;
			s_handler_count--;
			return ROBOTOS_CORE_OK;
		}
	}

	return ROBOTOS_CORE_ERR_EMPTY; /* handler not found */
}

bool robotos_core_has_event_handler(robotos_event_type_t type)
{
	if (core_state != ROBOTOS_CORE_STATE_READY) {
		return false;
	}
	for (uint32_t i = 0; i < ROBOTOS_CORE_MAX_EVENT_HANDLERS; i++) {
		if (s_handler_table[i].in_use && s_handler_table[i].type == type) {
			return true;
		}
	}
	return false;
}

uint32_t robotos_core_registered_handler_count(void)
{
	return s_handler_count;
}

uint32_t robotos_core_unhandled_event_count(void)
{
	return s_unhandled_event_count;
}
