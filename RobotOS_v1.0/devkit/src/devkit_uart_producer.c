/*
 * devkit_uart_producer.c
 * RobotOS devkit UART RX event producer (Phase 9B) — devkit-local.
 *
 * Bridges STM32F411E-DISCO usart2 RX (PA3) to the portable RobotOS core
 * event pipeline. The Zephyr UART interrupt-driven API delivers RX bytes
 * via uart_irq_callback_set(); the callback runs in ISR context and posts
 * one event per byte via robotos_core_post_event() under the Phase 5G
 * ISR-safe contract.
 *
 * Implementation notes:
 *   - Zephyr UART/devicetree/interrupt usage is contained in this file only.
 *   - core/ and platform/ remain Zephyr-free and UART-agnostic.
 *   - No software RX buffering of our own. We drain the hardware FIFO in
 *     the callback (uart_fifo_read in a tight loop) and post each byte as
 *     a single event. Backpressure (queue full) is reflected in s_full.
 *   - Counters are simple volatile uint32 in ISR; benign torn-read tolerated
 *     under the existing single-CPU model. Reads occur in thread context
 *     during periodic RTT cadence — single-tick skew is acceptable for a
 *     diagnostic counter.
 *   - One byte = one event. No line buffering, no parsing, no protocol.
 */

#include "devkit_uart_producer.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/logging/log.h>

#include "robotos_core.h"
#include "devkit_app_state.h"

LOG_MODULE_REGISTER(devkit_uart, LOG_LEVEL_INF);

/*
 * Resolve the UART device via the standard Zephyr 'zephyr,console' chosen
 * node. Confirmed in zephyr/boards/arm/stm32f411e_disco/stm32f411e_disco.dts:
 *   chosen { zephyr,console = &usart2; ... };
 *   &usart2 { current-speed = <115200>; pinctrl = TX=PA2, RX=PA3; status = okay; };
 *
 * Phase 9B uses the chosen-console UART rather than introducing a new
 * devicetree alias: it's the only enabled UART on the board today, the
 * pinctrl/baud is already correct, and we are not actually using it as a
 * console (CONFIG_UART_CONSOLE=n; LOG_BACKEND_RTT=y).
 */
#define UART_NODE DT_CHOSEN(zephyr_console)

#if !DT_NODE_HAS_STATUS(UART_NODE, okay)
#error "Phase 9B requires devicetree chosen 'zephyr,console' to be present and okay"
#endif

static const struct device *const s_uart_dev = DEVICE_DT_GET(UART_NODE);

/*
 * Volatile: written from UART ISR callback context, read from thread context.
 * Benign approximate counters under the single-CPU Cortex-M4 model.
 */
static volatile uint32_t s_rx;
static volatile uint32_t s_ok;
static volatile uint32_t s_full;
static volatile uint32_t s_invalid;
static volatile uint32_t s_other;
static volatile uint8_t  s_last_byte;

/* Thread context only (handler runs from core dispatch in tick context). */
static uint32_t s_handled;

/* Phase 9E: TX responses emitted (thread context only; never from ISR). */
static uint32_t s_tx_sent;

/*
 * UART RX ISR callback (Phase 5G ISR-safe contract).
 *
 * The Zephyr STM32 UART driver invokes this in interrupt context. Same
 * forbidden-list as the GPIO ISR callback used by the button producer:
 *   - logging (LOG_INF/etc. backend may sleep)
 *   - sleeping / yielding
 *   - dispatching events
 *   - registering/unregistering handlers
 *   - allocating
 *
 * Permitted:
 *   - O(1) event construction on stack (per byte)
 *   - robotos_core_post_event() (proven ISR-safe in Phase 5G/6G/6H/6I/9A)
 *   - simple counter updates
 *   - uart_irq_update / uart_irq_rx_ready / uart_fifo_read (Zephyr ISR-safe)
 */
static void devkit_uart_isr_cb(const struct device *dev, void *user_data)
{
	(void)user_data;

	if (!uart_irq_update(dev)) {
		return;
	}

	while (uart_irq_rx_ready(dev)) {
		uint8_t buf[16];
		int n = uart_fifo_read(dev, buf, sizeof(buf));
		if (n <= 0) {
			break;
		}

		for (int i = 0; i < n; i++) {
			s_rx++;
			s_last_byte = buf[i];

			robotos_event_t ev = {
				.type           = DEVKIT_UART_PRODUCER_TYPE,
				.timestamp_tick = 0u,
				.arg0           = DEVKIT_UART_PRODUCER_MARKER,
				.arg1           = (uint32_t)buf[i],
			};

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
		}
	}
}

/* --------------------------------------------------------------------------
 * Phase 9E: Minimal UART TX response path.
 *
 * uart_poll_out() is a polled byte-at-a-time TX primitive. Acceptable for
 * small, bounded demo responses from thread context only. Characteristics:
 *   - Never called from ISR (handler runs from core dispatcher, thread ctx).
 *   - Response is a fixed stack buffer (96 bytes); no heap, no dynamic alloc.
 *   - No TX interrupt machinery, no TX FIFO management, no async subsystem.
 *   - No new Zephyr thread. Single-threaded by construction.
 *   - No shell, no parser, no command registry, no echo, no prompt.
 *
 * Response format:
 *   State command ('a'/'s'/'r'), transition occurred:
 *     OK state=<NAME>\r\n
 *   State command ('a'/'s'/'r'), already in target state (redundant):
 *     OK state=<NAME> unchanged=1\r\n
 *   Query ('?'):
 *     STATE state=<NAME> transitions=N button=N uart=N ignored=N\r\n
 *   Unknown / unrecognized byte:
 *     ERR ignored byte=0xNN state=<NAME>\r\n
 *
 * Thread safety: called only from devkit_uart_handler(), which is serialized
 * through the core dispatcher (one event per tick, ROBOTOS_CORE_MAX_EVENTS_PER_TICK=1).
 * -------------------------------------------------------------------------- */

static void devkit_uart_send_bytes(const char *buf, int len)
{
	for (int i = 0; i < len; i++) {
		uart_poll_out(s_uart_dev, (unsigned char)buf[i]);
	}
}

static void devkit_uart_emit_tx_response(uint8_t byte,
					 const devkit_app_state_snapshot_t *before,
					 const devkit_app_state_snapshot_t *after)
{
	char    buf[96];
	int     n = 0;
	uint8_t c = byte;
	bool    changed = (after->transitions != before->transitions);

	if (c >= 'A' && c <= 'Z') {
		c = (uint8_t)(c + ('a' - 'A'));
	}

	switch (c) {
	case 'a':
	case 's':
	case 'r':
		if (changed) {
			n = snprintf(buf, sizeof(buf), "OK state=%s\r\n",
				     devkit_app_state_state_name(after->state));
		} else {
			n = snprintf(buf, sizeof(buf), "OK state=%s unchanged=1\r\n",
				     devkit_app_state_state_name(after->state));
		}
		break;

	case '?':
		n = snprintf(buf, sizeof(buf),
			     "STATE state=%s transitions=%u button=%u uart=%u ignored=%u\r\n",
			     devkit_app_state_state_name(after->state),
			     (unsigned)after->transitions,
			     (unsigned)after->button_count,
			     (unsigned)after->uart_count,
			     (unsigned)after->ignored_count);
		break;

	default:
		n = snprintf(buf, sizeof(buf), "ERR ignored byte=0x%02x state=%s\r\n",
			     (unsigned)byte,
			     devkit_app_state_state_name(after->state));
		break;
	}

	if (n > 0 && n < (int)sizeof(buf)) {
		devkit_uart_send_bytes(buf, n);
		s_tx_sent++;
		LOG_INF("Phase 9E UART response sent cmd=0x%02x len=%d state=%s",
			(unsigned)byte, n,
			devkit_app_state_state_name(after->state));
	}
}

/*
 * Devkit-local handler for DEVKIT_UART_PRODUCER_TYPE.
 * Runs in thread context from the core dispatcher (one event per tick under
 * the unchanged ROBOTOS_CORE_MAX_EVENTS_PER_TICK = 1 budget).
 *
 * Validates the marker as a defensive sanity check; should never fail under
 * normal Phase 9B operation since this is the only producer of USER+3.
 */
static robotos_core_status_t devkit_uart_handler(
	const robotos_event_t *event,
	void                  *user_context)
{
	(void)user_context;

	if (event == NULL) {
		return ROBOTOS_CORE_ERR_NULL;
	}
	if (event->type != DEVKIT_UART_PRODUCER_TYPE ||
	    event->arg0 != DEVKIT_UART_PRODUCER_MARKER) {
		LOG_ERR("Phase 9B uart: unexpected event type=%d arg0=0x%x",
			(int)event->type, (unsigned)event->arg0);
		return ROBOTOS_CORE_ERR_INVALID_ARG;
	}

	s_handled++;
	uint8_t byte = (uint8_t)(event->arg1 & 0xFFu);

	/* Per-byte handler log. Cadence is bounded by physical UART input rate
	 * (manual typing or short bursts) so log volume stays small. Printable
	 * bytes are shown; control bytes are shown as hex only. */
	if (byte >= 0x20u && byte < 0x7Fu) {
		LOG_INF("Phase 9B uart handled byte=0x%02x ('%c') count=%u",
			(unsigned)byte, (char)byte, s_handled);
	} else {
		LOG_INF("Phase 9B uart handled byte=0x%02x count=%u",
			(unsigned)byte, s_handled);
	}

	/* Phase 9E: snapshot before driving app state so we can detect whether
	 * a transition occurred (distinguishes "OK state=X" from
	 * "OK state=X unchanged=1" for redundant state commands). */
	devkit_app_state_snapshot_t snap_before, snap_after;
	devkit_app_state_get_snapshot(&snap_before);

	/* Phase 9C: drive the devkit application state machine.
	 * Thread-context only; app state module is single-threaded by
	 * construction (dispatcher serializes handler invocations). */
	devkit_app_state_on_uart_byte(byte, s_handled);

	/* Phase 9E: snapshot after, then emit minimal UART TX response. */
	devkit_app_state_get_snapshot(&snap_after);
	devkit_uart_emit_tx_response(byte, &snap_before, &snap_after);

	return ROBOTOS_CORE_OK;
}

robotos_core_status_t devkit_uart_producer_init(void)
{
	if (!device_is_ready(s_uart_dev)) {
		LOG_ERR("Phase 9B: UART device not ready (chosen zephyr,console)");
		return ROBOTOS_CORE_ERR_INVALID_STATE;
	}

	/*
	 * Register the handler BEFORE enabling RX interrupt so any byte that
	 * arrives during the gap routes to the registered handler instead of
	 * incrementing unhandled_event_count.
	 */
	robotos_core_status_t hr = robotos_core_register_event_handler(
		DEVKIT_UART_PRODUCER_TYPE,
		devkit_uart_handler,
		NULL);
	if (hr != ROBOTOS_CORE_OK) {
		LOG_ERR("Phase 9B: handler register failed: %d", (int)hr);
		return hr;
	}

	int rc = uart_irq_callback_set(s_uart_dev, devkit_uart_isr_cb);
	if (rc < 0) {
		LOG_ERR("Phase 9B: uart_irq_callback_set failed: %d", rc);
		return ROBOTOS_CORE_ERR_INVALID_STATE;
	}

	uart_irq_rx_enable(s_uart_dev);

	LOG_INF("Phase 9B uart producer init: type=USER+3 marker=0x%x dev=%s",
		(unsigned)DEVKIT_UART_PRODUCER_MARKER,
		s_uart_dev->name);
	return ROBOTOS_CORE_OK;
}

void devkit_uart_producer_get_stats(devkit_uart_producer_stats_t *out)
{
	if (out == NULL) {
		return;
	}
	out->rx        = s_rx;
	out->ok        = s_ok;
	out->full      = s_full;
	out->invalid   = s_invalid;
	out->other     = s_other;
	out->handled   = s_handled;
	out->last_byte = (uint32_t)s_last_byte;
}

void devkit_uart_producer_log_stats(void)
{
	devkit_uart_producer_stats_t st;
	devkit_uart_producer_get_stats(&st);
	LOG_INF("ROBOTOS_UART rx=%u ok=%u full=%u invalid=%u other=%u "
		"handled=%u last=0x%02x type=USER+3",
		st.rx, st.ok, st.full, st.invalid, st.other,
		st.handled, (unsigned)st.last_byte);
}
