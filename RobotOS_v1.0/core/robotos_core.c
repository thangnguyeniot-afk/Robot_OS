/*
 * robotos_core.c
 * RobotOS portable core — Phase 5F: dispatcher pop / handler split.
 *
 * Logging is routed through robotos_platform_log.h — a portable interface.
 * The Zephyr backend (platform/zephyr/robotos_platform_log_zephyr.c) is
 * compiled for devkit builds. The host no-op stub is compiled for host tests.
 *
 * robotos_core.c no longer includes <zephyr/logging/log.h> directly.
 * The public API (robotos_core.h) remains free of Zephyr types.
 *
 * Phase 5F critical-section application:
 *   PROTECTED (short critical sections via robotos_platform_critical_enter/exit):
 *     - post_event_internal: state check, admission, throttle, queue push, counters
 *     - robotos_core_init: state/counter/handler-table mutations
 *     - robotos_core_tick: tick_count increment and state check only
 *     - robotos_core_snapshot: all counter/state reads
 *     - all individual getters: each read locked briefly
 *     - handler table register/unregister/has/count reads and mutations
 *     - s_core_dispatch_one_safe Step 1: queue pop into local copy
 *     - s_core_dispatch_one_safe Step 2: handler table lookup, copy fn+ctx to locals
 *
 *   NOT PROTECTED (always outside critical section):
 *     - registered handler callback (hard rule: depth MUST be 0 during invocation)
 *     - CORE_LOG_* calls
 *     - dispatcher counter updates (written after handler returns, no lock)
 *
 * Phase 5F dispatch split (s_core_dispatch_one_safe):
 *   Step 1: [lock] queue pop into local robotos_event_t [unlock]
 *   Step 2: [lock] handler table search; copy fn+ctx to locals;
 *           if not found: s_unhandled_event_count++ [unlock]
 *   Step 3: call handler_fn(&ev, ctx) — NO lock held (depth == 0)
 *   Step 4: update s_core_dispatcher counters directly (no lock needed,
 *           single-threaded devkit assumption; handler has already returned)
 *
 * Limitation: handler fn+ctx are copied under lock; if unregister occurs
 * between copy and invocation, the callback runs once with the stale copy.
 * Acceptable under static/no-dynamic-handler model; caller owns context lifetime.
 *
 * This is NOT a full ISR-safe guarantee. No ISR producer stress tested yet.
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
 * No-op dispatcher handler stub.
 * Registered with s_core_dispatcher at init for API validity only.
 * The core dispatch path (tick / dispatch_events) uses s_core_dispatch_one_safe()
 * directly after Phase 5F; this handler is never invoked by the core.
 */
static robotos_core_status_t s_core_noop_handler(const robotos_event_t *ev, void *ctx)
{
	(void)ev; (void)ctx;
	return ROBOTOS_CORE_OK;
}

/*
 * Phase 5F: safe single-event dispatch helper.
 *
 * Separates queue pop, handler lookup, and handler invocation into three
 * distinct regions with minimal critical-section exposure:
 *
 *   Step 1: [lock] pop one event from queue into local copy [unlock]
 *   Step 2: [lock] search handler table; copy fn+ctx to locals;
 *           if not found: s_unhandled_event_count++ [unlock]
 *   Step 3: call local handler_fn outside lock — depth MUST be 0
 *   Step 4: update dispatcher counters after handler returns (no lock)
 *
 * Critical-section invariant: handler callback NEVER runs while lock is held.
 * No logging under lock. Counter updates are direct struct field writes
 * (single-threaded devkit assumption preserved).
 *
 * If unregister occurs between Step 2 copy and Step 3 invocation, the
 * callback runs once using the stale copied pointer. Acceptable under the
 * static/no-dynamic-handler model; caller owns context lifetime.
 */
static robotos_core_status_t s_core_dispatch_one_safe(void)
{
	/* Step 1: pop under lock into local event copy */
	robotos_platform_critical_token_t tok = robotos_platform_critical_enter();
	robotos_event_t ev;
	robotos_core_status_t pop_ret = robotos_event_queue_pop(&s_core_event_queue, &ev);
	robotos_platform_critical_exit(tok);

	if (pop_ret != ROBOTOS_CORE_OK) {
		return pop_ret; /* ERR_EMPTY or other queue error */
	}

	/* Step 2: handler lookup under lock — copy fn+ctx to locals */
	robotos_core_event_handler_t handler_fn  = NULL;
	void                        *handler_ctx = NULL;
	bool                         found       = false;

	tok = robotos_platform_critical_enter();
	for (uint32_t i = 0; i < ROBOTOS_CORE_MAX_EVENT_HANDLERS; i++) {
		if (s_handler_table[i].in_use &&
		    s_handler_table[i].type == ev.type) {
			handler_fn  = s_handler_table[i].handler;
			handler_ctx = s_handler_table[i].user_context;
			found       = true;
			break;
		}
	}
	if (!found) {
		s_unhandled_event_count++;
	}
	robotos_platform_critical_exit(tok);

	if (!found) {
		/* Event consumed from queue; no registered handler matched.
		 * Preserve Phase 5E dispatched_count semantics: any event consumed
		 * and processed without a handler error increments dispatched_count. */
		s_core_dispatcher.dispatched_count++;
		return ROBOTOS_CORE_OK;
	}

	/* Step 3: invoke handler outside critical section — depth MUST be 0 */
	robotos_core_status_t handler_ret = handler_fn(&ev, handler_ctx);

	/* Step 4: update dispatcher counters (no lock; single-threaded assumption) */
	if (handler_ret != ROBOTOS_CORE_OK) {
		s_core_dispatcher.handler_error_count++;
		return handler_ret;
	}
	s_core_dispatcher.dispatched_count++;
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
	                              s_core_noop_handler, NULL);

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

	/* Logging outside lock */
	if (tick == 1 ||
	    (tick % CORE_TICK_LOG_INTERVAL) == 0) {
		CORE_LOG_INF("core tick count=%u", tick);
	}

	/* Phase 5F: dispatch via safe helper — pop under lock, handler outside lock */
	for (uint32_t i = 0; i < ROBOTOS_CORE_MAX_EVENTS_PER_TICK; i++) {
		robotos_core_status_t dret = s_core_dispatch_one_safe();
		if (dret == ROBOTOS_CORE_ERR_EMPTY) {
			return ROBOTOS_CORE_OK;
		}
		if (dret != ROBOTOS_CORE_OK) {
			return dret;
		}
	}
	return ROBOTOS_CORE_OK;
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
	/* State check under lock; dispatch outside lock */
	robotos_platform_critical_token_t tok = robotos_platform_critical_enter();
	bool ready = (core_state == ROBOTOS_CORE_STATE_READY);
	robotos_platform_critical_exit(tok);

	if (!ready) {
		return ROBOTOS_CORE_ERR_INVALID_STATE;
	}
	if (max_events == 0u) {
		return ROBOTOS_CORE_OK;
	}

	/* Phase 5F: per-event safe dispatch — pop under lock, handler outside lock */
	uint32_t dispatched = 0;
	for (uint32_t i = 0; i < max_events; i++) {
		robotos_core_status_t dret = s_core_dispatch_one_safe();
		if (dret == ROBOTOS_CORE_ERR_EMPTY) {
			return (dispatched > 0u) ? ROBOTOS_CORE_OK : ROBOTOS_CORE_ERR_EMPTY;
		}
		if (dret != ROBOTOS_CORE_OK) {
			return dret;
		}
		dispatched++;
	}
	return ROBOTOS_CORE_OK;
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

/* ---------------------------------------------------------------------------
 * Phase 4L: Advisory Retry Decision Policy — pure stateless mapping
 * No lock, no logging, no platform calls, no queue access, no new status code.
 * ---------------------------------------------------------------------------
 */

robotos_core_status_t robotos_core_retry_decision_for_status(
	robotos_core_status_t          status,
	robotos_core_retry_decision_t *out)
{
	if (out == NULL) {
		return ROBOTOS_CORE_ERR_NULL;
	}

	switch (status) {
	case ROBOTOS_CORE_OK:
		out->action               = ROBOTOS_CORE_RETRY_NONE;
		out->suggested_wait_ticks = 0u;
		out->producer_should_drop   = false;
		out->producer_should_report = false;
		return ROBOTOS_CORE_OK;

	case ROBOTOS_CORE_ERR_FULL:
		out->action               = ROBOTOS_CORE_RETRY_AFTER_TICK;
		out->suggested_wait_ticks = 1u;
		out->producer_should_drop   = false;
		out->producer_should_report = false;
		return ROBOTOS_CORE_OK;

	case ROBOTOS_CORE_ERR_THROTTLED:
		out->action               = ROBOTOS_CORE_RETRY_AFTER_TICK;
		out->suggested_wait_ticks = 1u;
		out->producer_should_drop   = false;
		out->producer_should_report = false;
		return ROBOTOS_CORE_OK;

	case ROBOTOS_CORE_ERR_INVALID_STATE:
		out->action               = ROBOTOS_CORE_RETRY_SOON;
		out->suggested_wait_ticks = 0u;
		out->producer_should_drop   = false;
		out->producer_should_report = false;
		return ROBOTOS_CORE_OK;

	case ROBOTOS_CORE_ERR_INVALID_ARG:
		out->action               = ROBOTOS_CORE_RETRY_NEVER;
		out->suggested_wait_ticks = 0u;
		out->producer_should_drop   = true;
		out->producer_should_report = true;
		return ROBOTOS_CORE_OK;

	case ROBOTOS_CORE_ERR_NULL:
		out->action               = ROBOTOS_CORE_RETRY_NEVER;
		out->suggested_wait_ticks = 0u;
		out->producer_should_drop   = true;
		out->producer_should_report = true;
		return ROBOTOS_CORE_OK;

	case ROBOTOS_CORE_ERR_EMPTY:
		out->action               = ROBOTOS_CORE_RETRY_NONE;
		out->suggested_wait_ticks = 0u;
		out->producer_should_drop   = false;
		out->producer_should_report = false;
		return ROBOTOS_CORE_OK;

	default:
		out->action               = ROBOTOS_CORE_RETRY_NONE;
		out->suggested_wait_ticks = 0u;
		out->producer_should_drop   = false;
		out->producer_should_report = false;
		return ROBOTOS_CORE_ERR_INVALID_ARG;
	}
}

bool robotos_core_status_is_retryable(robotos_core_status_t status)
{
	switch (status) {
	case ROBOTOS_CORE_ERR_FULL:
	case ROBOTOS_CORE_ERR_THROTTLED:
	case ROBOTOS_CORE_ERR_INVALID_STATE:
		return true;
	default:
		return false;
	}
}
