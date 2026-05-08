/*
 * devkit_button_producer.c
 * RobotOS devkit user-button event producer (Phase 9A-A) — devkit-local.
 *
 * Bridges STM32F411E-DISCO user button (PA0, alias sw0) to the portable
 * RobotOS core event pipeline. ISR posts via robotos_core_post_event()
 * under the Phase 5G ISR-safe contract.
 *
 * Implementation notes:
 *   - Zephyr GPIO/devicetree/interrupt usage is contained in this file only.
 *   - core/ and platform/ remain Zephyr-free and button-agnostic.
 *   - No software debounce in 9A-A (deliberate; first proof; document only).
 *   - Counters are simple volatile uint32 in ISR; benign torn-read tolerated
 *     under the existing single-CPU model. Reads occur in thread context
 *     during periodic RTT cadence — single-tick skew is acceptable for a
 *     diagnostic counter.
 */

#include "devkit_button_producer.h"

#include <stdint.h>

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

#include "robotos_core.h"

LOG_MODULE_REGISTER(devkit_btn, LOG_LEVEL_INF);

/*
 * Resolve the user button via the standard Zephyr 'sw0' alias.
 * Confirmed in zephyr/boards/arm/stm32f411e_disco/stm32f411e_disco.dts:
 *   user_button { gpios = <&gpioa 0 GPIO_ACTIVE_HIGH>; ... };
 *   aliases { sw0 = &user_button; ... };
 */
#define BTN_NODE DT_ALIAS(sw0)

#if !DT_NODE_HAS_STATUS(BTN_NODE, okay)
#error "Phase 9A-A requires devicetree alias 'sw0' to be present and okay"
#endif

static const struct gpio_dt_spec s_button = GPIO_DT_SPEC_GET(BTN_NODE, gpios);
static struct gpio_callback     s_button_cb_data;

/*
 * Volatile: written from GPIO ISR callback context, read from thread context.
 * Benign approximate counters under the single-CPU Cortex-M4 model.
 */
static volatile uint32_t s_attempted;
static volatile uint32_t s_ok;
static volatile uint32_t s_full;
static volatile uint32_t s_invalid;
static volatile uint32_t s_other;

/* Thread context only (handler runs from core dispatch in tick context). */
static uint32_t s_handled;

/*
 * Sequence counter for events. Updated only from ISR context (Zephyr serializes
 * GPIO callbacks per device on a single CPU). Static-init to 1 so first event
 * has seq=1.
 */
static uint32_t s_seq = 1u;

/*
 * Button GPIO ISR callback (Phase 5G ISR-safe contract).
 *
 * Forbidden in this context:
 *   - logging (LOG_INF/etc. backend may sleep)
 *   - sleeping / yielding
 *   - dispatching events
 *   - registering/unregistering handlers
 *   - allocating
 *
 * Permitted:
 *   - O(1) event construction on stack
 *   - robotos_core_post_event() (proven ISR-safe in Phase 5G/6G/6H/6I/6Z)
 *   - simple counter updates
 */
static void devkit_button_isr_cb(const struct device *dev,
				  struct gpio_callback *cb,
				  uint32_t pins)
{
	(void)dev;
	(void)cb;
	(void)pins;

	robotos_event_t ev = {
		.type           = DEVKIT_BUTTON_PRODUCER_TYPE,
		.timestamp_tick = 0u,                          /* uptime not ISR-claimed */
		.arg0           = DEVKIT_BUTTON_PRODUCER_MARKER,
		.arg1           = s_seq,
	};

	s_attempted++;

	robotos_core_status_t r = robotos_core_post_event(&ev);
	switch (r) {
	case ROBOTOS_CORE_OK:
		s_ok++;
		break;
	case ROBOTOS_CORE_ERR_FULL:
		s_full++;
		break;
	case ROBOTOS_CORE_ERR_INVALID_ARG:
		s_invalid++;
		break;
	default:
		s_other++;
		break;
	}

	s_seq++;
}

/*
 * Devkit-local handler for DEVKIT_BUTTON_PRODUCER_TYPE.
 * Runs in thread context from the core dispatcher (one event per tick under
 * the unchanged ROBOTOS_CORE_MAX_EVENTS_PER_TICK = 1 budget).
 *
 * Validates the marker as a defensive sanity check; should never fail under
 * normal Phase 9A-A operation since this is the only producer of USER+2.
 */
static robotos_core_status_t devkit_button_handler(
	const robotos_event_t *event,
	void                  *user_context)
{
	(void)user_context;

	if (event == NULL) {
		return ROBOTOS_CORE_ERR_NULL;
	}
	if (event->type != DEVKIT_BUTTON_PRODUCER_TYPE ||
	    event->arg0 != DEVKIT_BUTTON_PRODUCER_MARKER) {
		LOG_ERR("Phase 9A button: unexpected event type=%d arg0=0x%x",
			(int)event->type, (unsigned)event->arg0);
		return ROBOTOS_CORE_ERR_INVALID_ARG;
	}

	s_handled++;
	uint32_t seq = (uint32_t)event->arg1;

	/* Per-press milestone log. Cadence is bounded by user button press
	 * rate (mechanical, far below tick rate) so log noise is small. */
	LOG_INF("Phase 9A button handled seq=%u count=%u", seq, s_handled);

	return ROBOTOS_CORE_OK;
}

robotos_core_status_t devkit_button_producer_init(void)
{
	if (!device_is_ready(s_button.port)) {
		LOG_ERR("Phase 9A-A: button GPIO port not ready");
		return ROBOTOS_CORE_ERR_INVALID_STATE;
	}

	int rc = gpio_pin_configure_dt(&s_button, GPIO_INPUT);
	if (rc < 0) {
		LOG_ERR("Phase 9A-A: gpio_pin_configure_dt failed: %d", rc);
		return ROBOTOS_CORE_ERR_INVALID_STATE;
	}

	rc = gpio_pin_interrupt_configure_dt(&s_button, GPIO_INT_EDGE_TO_ACTIVE);
	if (rc < 0) {
		LOG_ERR("Phase 9A-A: gpio_pin_interrupt_configure_dt failed: %d", rc);
		return ROBOTOS_CORE_ERR_INVALID_STATE;
	}

	/*
	 * Register the handler BEFORE enabling the interrupt so that any
	 * very-early press (e.g. user holding the button at boot) routes to
	 * the registered handler instead of incrementing unhandled_event_count.
	 */
	robotos_core_status_t hr = robotos_core_register_event_handler(
		DEVKIT_BUTTON_PRODUCER_TYPE,
		devkit_button_handler,
		NULL);
	if (hr != ROBOTOS_CORE_OK) {
		LOG_ERR("Phase 9A-A: handler register failed: %d", (int)hr);
		return hr;
	}

	gpio_init_callback(&s_button_cb_data, devkit_button_isr_cb, BIT(s_button.pin));

	rc = gpio_add_callback(s_button.port, &s_button_cb_data);
	if (rc < 0) {
		LOG_ERR("Phase 9A-A: gpio_add_callback failed: %d", rc);
		return ROBOTOS_CORE_ERR_INVALID_STATE;
	}

	LOG_INF("Phase 9A-A button producer init: type=USER+2 marker=0x%x pin=%s.%d",
		(unsigned)DEVKIT_BUTTON_PRODUCER_MARKER,
		s_button.port->name,
		(int)s_button.pin);
	return ROBOTOS_CORE_OK;
}

void devkit_button_producer_get_stats(devkit_button_producer_stats_t *out)
{
	if (out == NULL) {
		return;
	}
	out->attempted = s_attempted;
	out->ok        = s_ok;
	out->full      = s_full;
	out->invalid   = s_invalid;
	out->other     = s_other;
	out->handled   = s_handled;
}

void devkit_button_producer_log_stats(void)
{
	devkit_button_producer_stats_t st;
	devkit_button_producer_get_stats(&st);
	LOG_INF("ROBOTOS_BTN attempted=%u ok=%u full=%u invalid=%u other=%u "
		"handled=%u type=USER+2",
		st.attempted, st.ok, st.full, st.invalid, st.other, st.handled);
}
