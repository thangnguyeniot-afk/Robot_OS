/*
 * robotos_core.c
 * RobotOS portable core — Phase 4A bootstrap implementation.
 *
 * Zephyr logging is used internally. The public API (robotos_core.h)
 * remains free of Zephyr types.
 */

#include "robotos_core.h"

#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(robotos_core, LOG_LEVEL_INF);

#define CORE_VERSION "4A-bootstrap"
#define CORE_TICK_LOG_INTERVAL 10

static uint32_t core_tick_count;
static bool core_initialized;

const char *robotos_core_version(void)
{
	return CORE_VERSION;
}

int robotos_core_init(void)
{
	core_tick_count = 0;
	core_initialized = true;

	LOG_INF("RobotOS core init — version=%s", CORE_VERSION);

	return 0;
}

int robotos_core_tick(void)
{
	if (!core_initialized) {
		return -1;
	}

	core_tick_count++;

	if (core_tick_count == 1 || (core_tick_count % CORE_TICK_LOG_INTERVAL) == 0) {
		LOG_INF("core tick count=%u", core_tick_count);
	}

	return 0;
}
