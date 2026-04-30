/*
 * devkit/src/main.c
 * RobotOS devkit bring-up — STM32F411E-DISCO
 *
 * Phase: minimal blink + LOG.
 * Validates: Zephyr toolchain, board DTS alias, UART console.
 * No RobotOS framework dependencies at this stage.
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(devkit_main, LOG_LEVEL_DBG);

/* LED alias from boards/stm32f411e_disco.overlay or board DTS */
#define LED0_NODE DT_ALIAS(led0)

#if !DT_NODE_HAS_STATUS(LED0_NODE, okay)
#error "led0 alias not found in DTS — check boards/stm32f411e_disco.overlay"
#endif

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

int main(void)
{
	int ret;

	LOG_INF("RobotOS devkit starting — board: %s", CONFIG_BOARD);

	if (!gpio_is_ready_dt(&led)) {
		LOG_ERR("LED GPIO device not ready");
		return -ENODEV;
	}

	ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		LOG_ERR("Failed to configure LED GPIO: %d", ret);
		return ret;
	}

	LOG_INF("LED blink loop starting");

	while (1) {
		ret = gpio_pin_toggle_dt(&led);
		if (ret < 0) {
			LOG_ERR("GPIO toggle failed: %d", ret);
		}
		LOG_INF("tick");
		k_msleep(500);
	}

	return 0;
}
