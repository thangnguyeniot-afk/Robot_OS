/*
 * devkit_status_led.c
 * Owns led0 GPIO alias binding for the devkit status LED.
 * No app or core logic — init and toggle only.
 */

#include "devkit_status_led.h"

#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(devkit_status_led, LOG_LEVEL_DBG);

#define LED0_NODE DT_ALIAS(led0)

#if !DT_NODE_HAS_STATUS(LED0_NODE, okay)
#error "led0 alias not found in DTS — check board DTS or overlay"
#endif

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

int devkit_status_led_init(void)
{
	int ret;

	if (!gpio_is_ready_dt(&led)) {
		LOG_ERR("LED GPIO device not ready");
		return -ENODEV;
	}

	ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		LOG_ERR("Failed to configure LED GPIO: %d", ret);
	}

	return ret;
}

int devkit_status_led_toggle(void)
{
	return gpio_pin_toggle_dt(&led);
}
