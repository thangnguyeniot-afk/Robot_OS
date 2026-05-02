/*
 * devkit_runtime.c
 * Boot sequence, periodic tick log, and status LED orchestration.
 * Phase 5B: sleep path routed through platform time boundary.
 *           Direct k_msleep dependency removed from runtime.
 * Phase 6A: smoke event handler registered at init; one USER event posted
 *           to exercise the full core event pipeline on first tick.
 */

#include "devkit_runtime.h"
#include "devkit_build_info.h"
#include "devkit_fault.h"
#include "devkit_status_led.h"
#include "robotos_core.h"
#include "robotos_platform_time.h"

#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(devkit_runtime, LOG_LEVEL_INF);

/* --------------------------------------------------------------------------
 * Phase 6A: smoke event integration
 * -------------------------------------------------------------------------- */

static uint32_t s_smoke_handled_count;

static const robotos_event_t s_smoke_event = {
	.type           = ROBOTOS_EVENT_USER,
	.timestamp_tick = 0u,
	.arg0           = 0x6Au,
	.arg1           = 1u,
};

static robotos_core_status_t devkit_smoke_event_handler(
	const robotos_event_t *event, void *user_context)
{
	(void)user_context;
	if (event == NULL) {
		return ROBOTOS_CORE_ERR_NULL;
	}
	if (event->type != ROBOTOS_EVENT_USER) {
		return ROBOTOS_CORE_ERR_INVALID_ARG;
	}
	s_smoke_handled_count++;
	LOG_INF("smoke event handled type=USER arg0=0x%x arg1=%u",
		(unsigned)event->arg0, (unsigned)event->arg1);
	return ROBOTOS_CORE_OK;
}

/* -------------------------------------------------------------------------- */

int devkit_runtime_init(void)
{
	int ret;
	robotos_core_status_t core_ret;

	devkit_fault_init();
	devkit_build_info_log();

	core_ret = robotos_core_init();
	if (core_ret != ROBOTOS_CORE_OK) {
		LOG_ERR("Core init failed: %d — continuing", (int)core_ret);
	}

	/* Phase 6A: register smoke handler then post one controlled event */
	core_ret = robotos_core_register_event_handler(ROBOTOS_EVENT_USER,
						       devkit_smoke_event_handler, NULL);
	if (core_ret != ROBOTOS_CORE_OK) {
		LOG_ERR("smoke handler registration failed: %d", (int)core_ret);
	}

	core_ret = robotos_core_post_event(&s_smoke_event);
	if (core_ret != ROBOTOS_CORE_OK) {
		LOG_ERR("smoke event post failed: %d", (int)core_ret);
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
	robotos_core_status_t core_ret;
	uint32_t tick_count = 0;

	while (1) {
		ret = devkit_status_led_toggle();
		if (ret < 0) {
			LOG_ERR("LED toggle failed: %d", ret);
		}
		LOG_INF("tick count=%u", tick_count++);

		core_ret = robotos_core_tick();
		if (core_ret != ROBOTOS_CORE_OK) {
			LOG_ERR("Core tick failed: %d", (int)core_ret);
		}

		robotos_platform_sleep_ms(DEVKIT_TICK_MS);
	}
}
