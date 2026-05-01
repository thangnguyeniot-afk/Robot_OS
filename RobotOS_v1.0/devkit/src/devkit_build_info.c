/*
 * devkit_build_info.c
 * Logs board, Zephyr version, build timestamp, and runtime config at boot.
 * Uses generated version.h for KERNEL_VERSION_STRING (confirmed in build/).
 */

#include "devkit_build_info.h"
#include "devkit_runtime.h"

#include <version.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(devkit_build_info, LOG_LEVEL_INF);

void devkit_build_info_log(void)
{
	LOG_INF("RobotOS devkit build info:");
	LOG_INF("  board=" CONFIG_BOARD);
	LOG_INF("  zephyr=" KERNEL_VERSION_STRING);
	LOG_INF("  build=" __DATE__ " " __TIME__);
	LOG_INF("  tick_ms=%d", DEVKIT_TICK_MS);
	LOG_INF("  log_backend=RTT");
}
