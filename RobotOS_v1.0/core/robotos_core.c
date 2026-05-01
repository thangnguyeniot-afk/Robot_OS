/*
 * robotos_core.c
 * RobotOS portable core — Phase 4B contract implementation.
 *
 * Zephyr logging is used internally. The public API (robotos_core.h)
 * remains free of Zephyr types.
 *
 * Single-threaded assumption: no mutex or atomic operations applied.
 * This will need revisiting when RTOS threads are introduced.
 */

#include "robotos_core.h"

#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(robotos_core, LOG_LEVEL_INF);

#define CORE_VERSION           "4B-contract"
#define CORE_TICK_LOG_INTERVAL  10

static robotos_core_state_t core_state     = ROBOTOS_CORE_STATE_UNINITIALIZED;
static uint32_t             core_tick_count;
static uint32_t             core_init_count;
static bool                 core_tick_warn_emitted;

const char *robotos_core_version(void)
{
	return CORE_VERSION;
}

robotos_core_status_t robotos_core_init(void)
{
	if (core_state == ROBOTOS_CORE_STATE_READY) {
		core_init_count++;
		LOG_DBG("core already initialized (init_count=%u)", core_init_count);
		return ROBOTOS_CORE_OK;
	}

	core_tick_count        = 0;
	core_init_count        = 1;
	core_tick_warn_emitted = false;
	core_state             = ROBOTOS_CORE_STATE_READY;

	LOG_INF("RobotOS core init — version=%s state=READY", CORE_VERSION);

	return ROBOTOS_CORE_OK;
}

robotos_core_status_t robotos_core_tick(void)
{
	if (core_state != ROBOTOS_CORE_STATE_READY) {
		if (!core_tick_warn_emitted) {
			LOG_WRN("core tick before init — ignored");
			core_tick_warn_emitted = true;
		}
		return ROBOTOS_CORE_ERR_INVALID_STATE;
	}

	core_tick_count++;

	if (core_tick_count == 1 ||
	    (core_tick_count % CORE_TICK_LOG_INTERVAL) == 0) {
		LOG_INF("core tick count=%u", core_tick_count);
	}

	return ROBOTOS_CORE_OK;
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

	out->state      = core_state;
	out->tick_count = core_tick_count;
	out->init_count = core_init_count;
	out->version    = CORE_VERSION;

	return ROBOTOS_CORE_OK;
}
