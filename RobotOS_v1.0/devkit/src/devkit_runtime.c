/*
 * devkit_runtime.c
 * Boot sequence, periodic tick log, and status LED orchestration.
 * Phase 4A: RobotOS core init and tick integrated.
 */

#include "devkit_runtime.h"
#include "devkit_build_info.h"
#include "devkit_fault.h"
#include "devkit_status_led.h"
#include "robotos_core.h"

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(devkit_runtime, LOG_LEVEL_INF);

int devkit_runtime_init(void)
{
	int ret;

	devkit_fault_init();
	devkit_build_info_log();

	ret = robotos_core_init();
	if (ret < 0) {
		LOG_ERR("Core init failed: %d — continuing", ret);
	}

	LOG_INF("RobotOS devkit starting — board: %s", CONFIG_BOARD);

	ret = devkit_status_led_init();
	if (ret < 0) {
		LOG_ERR("Status LED init failed: %d", ret);
		return ret;
	}

	LOG_INF("LED blink loop starting");

	return 0;
}

void devkit_runtime_run(void)
{
	int ret;
	uint32_t tick_count = 0;

	while (1) {
		ret = devkit_status_led_toggle();
		if (ret < 0) {
			LOG_ERR("LED toggle failed: %d", ret);
		}
		LOG_INF("tick count=%u", tick_count++);

		ret = robotos_core_tick();
		if (ret < 0) {
			LOG_ERR("Core tick failed: %d", ret);
		}

		k_msleep(DEVKIT_TICK_MS);
	}
}
