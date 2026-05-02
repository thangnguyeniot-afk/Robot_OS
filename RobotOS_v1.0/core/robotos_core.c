/*
 * robotos_core.c
 * RobotOS portable core — Phase 5E: critical boundary applied to queue state.
 *
 * Logging is routed through robotos_platform_log.h — a portable interface.
 * The Zephyr backend (platform/zephyr/robotos_platform_log_zephyr.c) is
 * compiled for devkit builds. The host no-op stub is compiled for host tests.
 *
 * robotos_core.c no longer includes <zephyr/logging/log.h> directly.
 * The public API (robotos_core.h) remains free of Zephyr types.
 *
 * Phase 5E critical-section application:
 *   PROTECTED (short state transitions under robotos_platform_critical_enter/exit):
 *     - post_event_internal: state check, admission, throttle, queue push, counters
 *     - robotos_core_init: state/counter/handler-table mutations
 *     - robotos_core_tick: tick_count increment and state check only
 *     - robotos_core_snapshot: all counter/state reads
 *     - all individual getters: each read locked briefly
 *     - handler table register/unregister/has/count reads and mutations
 *
 *   NOT PROTECTED (remain single-threaded):
 *     - registered handler callback execution (s_core_routing_handler invoked by dispatcher)
 *     - logging (CORE_LOG_* calls)
 *     - robotos_event_dispatcher_dispatch_all (calls handler internally)
 *     - whole robotos_core_tick loop
 *
 * This is NOT a full ISR-safe or thread-safe guarantee. Dispatcher pop and
 * handler execution are not separated in Phase 5E. See Phase 5F.
 *
 * Single-threaded devkit runtime continues to work correctly.
 */

#include "robotos_core.h"
#include "robotos_event_queue.h"
#include "robotos_event_dispatcher.h"
#include "robotos_platform_log.h"
#include "robotos_platform_critical.h"

#include <stdbool.h>
#include <stddef.h>

/* ---- Internal logging helpers --------------------------------------------- */
#define CORE_LOG_INF(fmt, ...) \
    robotos_platform_logf(ROBOTOS_LOG_LEVEL_INFO,  "robotos_core", fmt, ##__VA_ARGS__)
#define CORE_LOG_WRN(fmt, ...) \
    robotos_platform_logf(ROBOTOS_LOG_LEVEL_WARN,  "robotos_core", fmt, ##__VA_ARGS__)
#define CORE_LOG_DBG(fmt, ...) \
    robotos_platform_logf(ROBOTOS_LOG_LEVEL_DEBUG, "robotos_core", fmt, ##__VA_ARGS__)
/* --------------------------------------------------------------------------- */

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
static uint32_t          s_admission_accepted_count;
static uint32_t          s_admission_rejected_count;
static uint32_t          s_producer_throttled_count;

/* --------------------------------------------------------------------------- */

static robotos_core_state_t      core_state     = ROBOTOS_CORE_STATE_UNINITIALIZED;
static uint32_t                  core_tick_count;
static uint32_t                  core_init_count;
static bool                      core_tick_warn_emitted;
static robotos_event_queue_t     s_core_event_queue;
static robotos_event_dispatcher_t s_core_dispatcher;

/*
 * Routing handler: looks up registered handler by event type.
 * Unregistered types: unhandled_event_count++, returns OK.
 * NOT called under critical section — handler callback must be lock-free.
 */
static robotos_core_status_t s_core_routing_handler(const robotos_event_t *event,
                                                      void *ctx)
{
	(void)ctx;
	if (event == NULL) {
		return ROBOTOS_CORE_ERR_NULL;
	}
	/* Handler table read without lock: Phase 5E limitation.
	 * Table mutations are locked; dispatch is not concurrent with mutation
	 * under single-threaded devkit assumption. */
	for (uint32_t i = 0; i < ROBOTOS_CORE_MAX_EVENT_HANDLERS; i++) {
		if (s_handler_table[i].in_use &&
		    s_handler_table[i].type == event->type) {
			return s_handler_table[i].handler(event,
			                                  s_handler_table[i].user_context);
		}
	}
	s_unhandled_event_count++;
	return ROBOTOS_CORE_OK;
}

/*
 * Admission policy: CORE_TICK and USER+ are accepted; NONE and reserved (2-99) rejected.
 * Pure function — safe to call under or outside lock.
 */
static bool s_event_type_admissible(robotos_event_type_t type)
{
	if (type == ROBOTOS_EVENT_NONE) {
		return false;
	}
	if (type == ROBOTOS_EVENT_CORE_TICK) {
		return true;
	}
	if ((uint32_t)type >= (uint32_t)ROBOTOS_EVENT_USER) {
		return true;
	}
	return false; /* reserved range 2-99 */
}

const char *robotos_core_version(void)
{
	return CORE_VERSION;
}

robotos_core_status_t robotos_core_init(void)
{
	robotos_platform_critical_token_t tok = robotos_platform_critical_enter();

	if (core_state == ROBOTOS_CORE_STATE_READY) {
		core_init_count++;
		uint32_t ic = core_init_count;
		robotos_platform_critical_exit(tok);
		CORE_LOG_DBG("core already initialized (init_count=%u)", ic);
		return ROBOTOS_CORE_OK;
	}

	/* First init: mutate state/counters/handler table under lock */
	core_tick_count        = 0;
	core_init_count        = 1;
	core_tick_warn_emitted = false;
	core_state             = ROBOTOS_CORE_STATE_READY;

	for (uint32_t i = 0; i < ROBOTOS_CORE_MAX_EVENT_HANDLERS; i++) {
		s_handler_table[i].in_use       = false;
		s_handler_table[i].handler      = NULL;
		s_handler_table[i].user_context = NULL;
	}
	s_handler_count         = 0;
	s_unhandled_event_count = 0;

	robotos_platform_critical_exit(tok);

	/* Queue/dispatcher init and logging outside lock */
	robotos_event_queue_init(&s_core_event_queue);
	robotos_event_dispatcher_init(&s_core_dispatcher, &s_core_event_queue,
	                              s_core_routing_handler, NULL);

	CORE_LOG_INF("RobotOS core init -- version=%s state=READY", CORE_VERSION);
	CORE_LOG_INF("event queue initialized capacity=%u",
	             ROBOTOS_EVENT_QUEUE_CAPACITY);
	CORE_LOG_INF("event dispatcher initialized");

	return ROBOTOS_CORE_OK;
}

robotos_core_status_t robotos_core_tick(void)
{
	/* Lock only around state check and tick_count increment */
	robotos_platform_critical_token_t tok = robotos_platform_critical_enter();

	if (core_state != ROBOTOS_CORE_STATE_READY) {
		bool warn_needed = !core_tick_warn_emitted;
		core_tick_warn_emitted = true;
		robotos_platform_critical_exit(tok);
		if (warn_needed) {
			CORE_LOG_WRN("core tick before init -- ignored");
		}
		return ROBOTOS_CORE_ERR_INVALID_STATE;
	}

	core_tick_count++;
	uint32_t tick = core_tick_count;
	robotos_platform_critical_exit(tok);

	/* Logging and dispatch outside lock — dispatch calls handler internally */
	if (tick == 1 ||
	    (tick % CORE_TICK_LOG_INTERVAL) == 0) {
		CORE_LOG_INF("core tick count=%u", tick);
	}

	robotos_core_status_t dret = robotos_event_dispatcher_dispatch_all(
		&s_core_dispatcher, ROBOTOS_CORE_MAX_EVENTS_PER_TICK);

	if (dret == ROBOTOS_CORE_ERR_EMPTY) {
		return ROBOTOS_CORE_OK;
	}
	return dret;
}

robotos_core_state_t robotos_core_state(void)
{
	robotos_platform_critical_token_t tok = robotos_platform_critical_enter();
	robotos_core_state_t s = core_state;
	robotos_platform_critical_exit(tok);
	return s;
}

uint32_t robotos_core_tick_count(void)
{
	robotos_platform_critical_token_t tok = robotos_platform_critical_enter();
	uint32_t v = core_tick_count;
	robotos_platform_critical_exit(tok);
	return v;
}

robotos_core_status_t robotos_core_snapshot(robotos_core_snapshot_t *out)
{
	if (out == NULL) {
		return ROBOTOS_CORE_ERR_NULL;
	}

	/* Lock around all reads for coherent snapshot */
	robotos_platform_critical_token_t tok = robotos_platform_critical_enter();

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
	out->admission_accepted_count = s_admission_accepted_count;
	out->admission_rejected_count = s_admission_rejected_count;
	out->producer_throttled_count = s_producer_throttled_count;

	/* Inline backpressure/throttle calculation to avoid nested lock */
	bool     full    = robotos_event_queue_is_full(&s_core_event_queue);
	uint32_t pending = out->pending_event_count;
	out->backpressure_active      = (pending > ROBOTOS_CORE_MAX_EVENTS_PER_TICK) || full;
	out->producer_throttle_active = !full && (pending > ROBOTOS_CORE_MAX_EVENTS_PER_TICK);

	robotos_platform_critical_exit(tok);

	return ROBOTOS_CORE_OK;
}

/*
 * Internal post helper. apply_throttle=true enables Phase 4K producer throttle.
 * Entire state decision and mutation held under a single short critical section.
 * No handler call, no logging inside lock.
 */
static robotos_core_status_t post_event_internal(const robotos_event_t *event,
                                                  bool apply_throttle)
{
	/* NULL check before lock — safe, event pointer itself does not change */
	if (event == NULL) {
		return ROBOTOS_CORE_ERR_NULL;
	}

	robotos_platform_critical_token_t tok = robotos_platform_critical_enter();

	if (core_state != ROBOTOS_CORE_STATE_READY) {
		robotos_platform_critical_exit(tok);
		return ROBOTOS_CORE_ERR_INVALID_STATE;
	}
	if (!s_event_type_admissible(event->type)) {
		s_admission_rejected_count++;
		robotos_platform_critical_exit(tok);
		return ROBOTOS_CORE_ERR_INVALID_ARG;
	}
	if (apply_throttle) {
		bool     full    = robotos_event_queue_is_full(&s_core_event_queue);
		uint32_t pending = robotos_event_queue_count(&s_core_event_queue);
		if (!full && pending > ROBOTOS_CORE_MAX_EVENTS_PER_TICK) {
			s_producer_throttled_count++;
			robotos_platform_critical_exit(tok);
			return ROBOTOS_CORE_ERR_THROTTLED;
		}
	}
	robotos_core_status_t push_ret = robotos_event_queue_push(&s_core_event_queue, event);
	if (push_ret == ROBOTOS_CORE_OK) {
		s_admission_accepted_count++;
	}
	robotos_platform_critical_exit(tok);
	return push_ret;
}

robotos_core_status_t robotos_core_post_event(const robotos_event_t *event)
{
	return post_event_internal(event, false);
}

robotos_core_status_t robotos_core_try_post_event(const robotos_event_t *event)
{
	return post_event_internal(event, true);
}

robotos_core_status_t robotos_core_dispatch_events(uint32_t max_events)
{
	/* State check under lock; dispatch outside lock (calls handler) */
	robotos_platform_critical_token_t tok = robotos_platform_critical_enter();
	bool ready = (core_state == ROBOTOS_CORE_STATE_READY);
	robotos_platform_critical_exit(tok);

	if (!ready) {
		return ROBOTOS_CORE_ERR_INVALID_STATE;
	}
	if (max_events == 0u) {
		return ROBOTOS_CORE_OK;
	}
	return robotos_event_dispatcher_dispatch_all(&s_core_dispatcher, max_events);
}

uint32_t robotos_core_pending_event_count(void)
{
	robotos_platform_critical_token_t tok = robotos_platform_critical_enter();
	uint32_t v = robotos_event_queue_count(&s_core_event_queue);
	robotos_platform_critical_exit(tok);
	return v;
}

uint32_t robotos_core_dropped_event_count(void)
{
	robotos_platform_critical_token_t tok = robotos_platform_critical_enter();
	uint32_t v = robotos_event_queue_dropped_count(&s_core_event_queue);
	robotos_platform_critical_exit(tok);
	return v;
}

uint32_t robotos_core_dispatched_event_count(void)
{
	robotos_platform_critical_token_t tok = robotos_platform_critical_enter();
	uint32_t v = robotos_event_dispatcher_dispatched_count(&s_core_dispatcher);
	robotos_platform_critical_exit(tok);
	return v;
}

uint32_t robotos_core_handler_error_count(void)
{
	robotos_platform_critical_token_t tok = robotos_platform_critical_enter();
	uint32_t v = robotos_event_dispatcher_handler_error_count(&s_core_dispatcher);
	robotos_platform_critical_exit(tok);
	return v;
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

	robotos_platform_critical_token_t tok = robotos_platform_critical_enter();

	if (core_state != ROBOTOS_CORE_STATE_READY) {
		robotos_platform_critical_exit(tok);
		return ROBOTOS_CORE_ERR_INVALID_STATE;
	}
	if (type == ROBOTOS_EVENT_NONE) {
		robotos_platform_critical_exit(tok);
		return ROBOTOS_CORE_ERR_INVALID_ARG;
	}

	/* Check for replacement of existing handler for same type */
	for (uint32_t i = 0; i < ROBOTOS_CORE_MAX_EVENT_HANDLERS; i++) {
		if (s_handler_table[i].in_use && s_handler_table[i].type == type) {
			s_handler_table[i].handler      = handler;
			s_handler_table[i].user_context = user_context;
			robotos_platform_critical_exit(tok);
			return ROBOTOS_CORE_OK;
		}
	}

	/* New registration */
	if (s_handler_count >= ROBOTOS_CORE_MAX_EVENT_HANDLERS) {
		robotos_platform_critical_exit(tok);
		return ROBOTOS_CORE_ERR_FULL;
	}

	for (uint32_t i = 0; i < ROBOTOS_CORE_MAX_EVENT_HANDLERS; i++) {
		if (!s_handler_table[i].in_use) {
			s_handler_table[i].type         = type;
			s_handler_table[i].handler      = handler;
			s_handler_table[i].user_context = user_context;
			s_handler_table[i].in_use       = true;
			s_handler_count++;
			robotos_platform_critical_exit(tok);
			return ROBOTOS_CORE_OK;
		}
	}

	robotos_platform_critical_exit(tok);
	return ROBOTOS_CORE_ERR_FULL; /* defensive */
}

robotos_core_status_t robotos_core_unregister_event_handler(robotos_event_type_t type)
{
	robotos_platform_critical_token_t tok = robotos_platform_critical_enter();

	if (core_state != ROBOTOS_CORE_STATE_READY) {
		robotos_platform_critical_exit(tok);
		return ROBOTOS_CORE_ERR_INVALID_STATE;
	}
	if (type == ROBOTOS_EVENT_NONE) {
		robotos_platform_critical_exit(tok);
		return ROBOTOS_CORE_ERR_INVALID_ARG;
	}

	for (uint32_t i = 0; i < ROBOTOS_CORE_MAX_EVENT_HANDLERS; i++) {
		if (s_handler_table[i].in_use && s_handler_table[i].type == type) {
			s_handler_table[i].in_use       = false;
			s_handler_table[i].handler      = NULL;
			s_handler_table[i].user_context = NULL;
			s_handler_count--;
			robotos_platform_critical_exit(tok);
			return ROBOTOS_CORE_OK;
		}
	}

	robotos_platform_critical_exit(tok);
	return ROBOTOS_CORE_ERR_EMPTY;
}

bool robotos_core_has_event_handler(robotos_event_type_t type)
{
	robotos_platform_critical_token_t tok = robotos_platform_critical_enter();
	if (core_state != ROBOTOS_CORE_STATE_READY) {
		robotos_platform_critical_exit(tok);
		return false;
	}
	for (uint32_t i = 0; i < ROBOTOS_CORE_MAX_EVENT_HANDLERS; i++) {
		if (s_handler_table[i].in_use && s_handler_table[i].type == type) {
			robotos_platform_critical_exit(tok);
			return true;
		}
	}
	robotos_platform_critical_exit(tok);
	return false;
}

uint32_t robotos_core_registered_handler_count(void)
{
	robotos_platform_critical_token_t tok = robotos_platform_critical_enter();
	uint32_t v = s_handler_count;
	robotos_platform_critical_exit(tok);
	return v;
}

uint32_t robotos_core_unhandled_event_count(void)
{
	robotos_platform_critical_token_t tok = robotos_platform_critical_enter();
	uint32_t v = s_unhandled_event_count;
	robotos_platform_critical_exit(tok);
	return v;
}

uint32_t robotos_core_admission_accepted_count(void)
{
	robotos_platform_critical_token_t tok = robotos_platform_critical_enter();
	uint32_t v = s_admission_accepted_count;
	robotos_platform_critical_exit(tok);
	return v;
}

uint32_t robotos_core_admission_rejected_count(void)
{
	robotos_platform_critical_token_t tok = robotos_platform_critical_enter();
	uint32_t v = s_admission_rejected_count;
	robotos_platform_critical_exit(tok);
	return v;
}

uint32_t robotos_core_dispatch_budget_per_tick(void)
{
	return ROBOTOS_CORE_MAX_EVENTS_PER_TICK;
}

bool robotos_core_backpressure_active(void)
{
	robotos_platform_critical_token_t tok = robotos_platform_critical_enter();
	uint32_t pending = robotos_event_queue_count(&s_core_event_queue);
	bool     full    = robotos_event_queue_is_full(&s_core_event_queue);
	robotos_platform_critical_exit(tok);
	return (pending > ROBOTOS_CORE_MAX_EVENTS_PER_TICK) || full;
}

bool robotos_core_producer_throttle_active(void)
{
	robotos_platform_critical_token_t tok = robotos_platform_critical_enter();
	bool     full    = robotos_event_queue_is_full(&s_core_event_queue);
	uint32_t pending = robotos_event_queue_count(&s_core_event_queue);
	robotos_platform_critical_exit(tok);
	return !full && (pending > ROBOTOS_CORE_MAX_EVENTS_PER_TICK);
}

uint32_t robotos_core_producer_throttled_count(void)
{
	robotos_platform_critical_token_t tok = robotos_platform_critical_enter();
	uint32_t v = s_producer_throttled_count;
	robotos_platform_critical_exit(tok);
	return v;
}
