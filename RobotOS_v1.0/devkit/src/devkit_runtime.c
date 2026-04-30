/*
 * devkit_runtime.c
 * Boot banner, periodic tick log, and status LED orchestration.
 * Behavior: identical to baseline — banner + LED toggle + tick every 500ms.
 */

#include "devkit_runtime.h"
#include "devkit_status_led.h"

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(devkit_runtime, LOG_LEVEL_DBG);

int devkit_runtime_init(void)
{
	int ret;

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

	while (1) {
		ret = devkit_status_led_toggle();
		if (ret < 0) {
			LOG_ERR("LED toggle failed: %d", ret);
		}
		LOG_INF("tick");
		k_msleep(DEVKIT_TICK_MS);
	}
}
