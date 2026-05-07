/*
 * devkit_timer_producer.c
 * RobotOS devkit periodic producer diagnostic (Phase 6M) -- pure C.
 *
 * No Zephyr dependency: only stdint, stdbool, and the public RobotOS
 * core API are used here. LOG emission for ROBOTOS_PROD lives in
 * devkit_observability.c so this file stays host-testable.
 */

#include "devkit_timer_producer.h"

#include "robotos_core.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Module-local counters. Reset only at process start (static-init = 0). */
static uint32_t s_attempted;
static uint32_t s_ok;
static uint32_t s_throttled;
static uint32_t s_dropped;
static uint32_t s_invalid;
static uint32_t s_other;
static bool     s_init_done;

/*
 * Producer's own handler. Registered for DEVKIT_TIMER_PRODUCER_TYPE only.
 * Validates the marker as a defensive sanity check; should never fail
 * under normal Phase 6M operation since the only producer of this type
 * is this module.
 */
static robotos_core_status_t producer_handler(const robotos_event_t *ev,
					       void                  *ctx)
{
	(void)ctx;
	if (ev == NULL) {
		return ROBOTOS_CORE_ERR_NULL;
	}
	if (ev->arg0 != DEVKIT_TIMER_PRODUCER_MARKER) {
		return ROBOTOS_CORE_ERR_INVALID_ARG;
	}
	return ROBOTOS_CORE_OK;
}

robotos_core_status_t devkit_timer_producer_init(void)
{
	robotos_core_status_t ret = robotos_core_register_event_handler(
		DEVKIT_TIMER_PRODUCER_TYPE, producer_handler, NULL);
	if (ret == ROBOTOS_CORE_OK) {
		s_init_done = true;
	}
	return ret;
}

bool devkit_timer_producer_should_post(uint32_t tick_count)
{
	if (!s_init_done) {
		return false;
	}
	return (tick_count % DEVKIT_TIMER_PRODUCER_TICK_PERIOD) == 0u;
}

void devkit_timer_producer_on_tick(uint32_t tick_count)
{
	if (!devkit_timer_producer_should_post(tick_count)) {
		return;
	}

	robotos_event_t ev = {
		.type           = DEVKIT_TIMER_PRODUCER_TYPE,
		.timestamp_tick = tick_count,
		.arg0           = DEVKIT_TIMER_PRODUCER_MARKER,
		.arg1           = s_attempted,
	};

	s_attempted++;
	robotos_core_status_t ret = robotos_core_post_event(&ev);

	switch (ret) {
	case ROBOTOS_CORE_OK:
		s_ok++;
		break;
	case ROBOTOS_CORE_ERR_THROTTLED:
		/* Not expected under post_event (no throttle); reserved slot
		 * so the counter shape is stable across API choice. */
		s_throttled++;
		break;
	case ROBOTOS_CORE_ERR_FULL:
		s_dropped++;
		break;
	case ROBOTOS_CORE_ERR_INVALID_ARG:
		s_invalid++;
		break;
	default:
		s_other++;
		break;
	}
}

void devkit_timer_producer_get_stats(devkit_timer_producer_stats_t *out)
{
	if (out == NULL) {
		return;
	}
	out->attempted = s_attempted;
	out->ok        = s_ok;
	out->throttled = s_throttled;
	out->dropped   = s_dropped;
	out->invalid   = s_invalid;
	out->other     = s_other;
}
