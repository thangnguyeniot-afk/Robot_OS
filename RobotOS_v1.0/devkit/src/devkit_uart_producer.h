/*
 * devkit_uart_producer.h
 * RobotOS devkit UART RX event producer (Phase 9B).
 *
 * Second real hardware event source for RobotOS. Bridges incoming UART RX
 * bytes (STM32F411E-DISCO usart2, PA3) to the portable RobotOS core event
 * pipeline:
 *
 *   external USB-UART adapter TX -> board PA3 (usart2 RX)
 *     -> Zephyr UART RX interrupt
 *     -> uart_irq_callback (ISR context)
 *     -> for each byte read via uart_fifo_read():
 *          robotos_core_post_event()           [Phase 5G ISR-safe contract]
 *     -> core queue
 *     -> robotos_core_tick() dispatcher        [thread context, budget=1/tick]
 *     -> uart byte handler (devkit-local)
 *     -> ROBOTOS_UART telemetry / per-byte handler log
 *
 * Architectural shape (Phase 9B):
 *   - DEVKIT-LOCAL ONLY. Zephyr UART/devicetree usage is confined here.
 *     core/ and platform/ remain Zephyr-free and UART-agnostic.
 *   - No new core API. No new platform abstraction (no robotos_platform_uart).
 *   - No scheduler change. No new event type in the core enum (uses
 *     ROBOTOS_EVENT_USER + 3; see DEVKIT_UART_PRODUCER_TYPE below).
 *   - Phase 5G ISR-safe contract: callback posts via robotos_core_post_event(),
 *     does not log/sleep/dispatch/register/alloc.
 *   - No shell, no parser, no command framework. One byte → one event.
 *
 * Hard non-goals (Phase 9B, deliberate):
 *   - No core/platform mutation.
 *   - No UART platform abstraction (premature; only one workload).
 *   - No threading, dynamic allocation, or floating point.
 *   - No telemetry feedback into scheduling.
 *   - No new Zephyr task/thread.
 *   - No echo/response, no line buffering, no protocol framing.
 */

#ifndef DEVKIT_UART_PRODUCER_H
#define DEVKIT_UART_PRODUCER_H

#include <stdint.h>

#include "robotos_core.h"

/*
 * Event type used by this producer. Distinct from:
 *   - Phase 6I producer:   ROBOTOS_EVENT_USER     (= 100)  [gated off by 9A-C]
 *   - Phase 6M producer:   ROBOTOS_EVENT_USER + 1 (= 101)
 *   - Phase 9A-A button:   ROBOTOS_EVENT_USER + 2 (= 102)
 *   - Phase 9B  uart  :    ROBOTOS_EVENT_USER + 3 (= 103)  <-- this module
 *
 * Admissible per the Phase 4I admission gate (USER+ accepted).
 * No new core enum required.
 */
#define DEVKIT_UART_PRODUCER_TYPE \
	((robotos_event_type_t)(ROBOTOS_EVENT_USER + 3u))

/* arg0 marker for runtime tagging / handler validation. */
#define DEVKIT_UART_PRODUCER_MARKER 0x9B0Bu

/*
 * Producer-local counters.
 *
 *   rx       : every byte read from the UART FIFO (every ISR-context byte)
 *   ok       : robotos_core_post_event() returned OK
 *   full     : returned ERR_FULL (queue at capacity)
 *   invalid  : returned ERR_INVALID_ARG (defensive; should always be 0)
 *   other    : any other non-OK return (defensive; should always be 0)
 *   handled  : handler invocations (thread context)
 *   last_byte: most recent byte read by the ISR (low 8 bits)
 */
typedef struct {
	uint32_t rx;
	uint32_t ok;
	uint32_t full;
	uint32_t invalid;
	uint32_t other;
	uint32_t handled;
	uint32_t last_byte;
} devkit_uart_producer_stats_t;

/*
 * Initialize the UART producer:
 *   1. Verify usart2 device is ready (chosen zephyr,console node).
 *   2. Register devkit-local handler for DEVKIT_UART_PRODUCER_TYPE with core.
 *   3. Install Zephyr UART IRQ callback.
 *   4. Enable UART RX interrupt.
 *
 * Must be called after robotos_core_init() so handler registration succeeds.
 *
 * Returns ROBOTOS_CORE_OK on success.
 * Returns ROBOTOS_CORE_ERR_INVALID_STATE if UART device not ready or
 *         interrupt setup fails.
 * Returns the underlying robotos_core_register_event_handler status if
 *         handler registration fails.
 *
 * Idempotent only in the sense that the producer counters are NOT reset on
 * a second call; callers should call this exactly once.
 */
robotos_core_status_t devkit_uart_producer_init(void);

/*
 * Snapshot the producer-local counters into a caller-owned struct.
 * If out is NULL, the call is a no-op.
 *
 * Reads volatile ISR counters in thread context. Tolerates single-tick skew
 * by design (the RTT cadence sampling is at the same cadence as ROBOTOS_OBS).
 */
void devkit_uart_producer_get_stats(devkit_uart_producer_stats_t *out);

/*
 * Emit one ROBOTOS_UART log line via Zephyr LOG_INF.
 *
 * Stable single-line format (integer fields only, last_byte in hex):
 *   ROBOTOS_UART rx=N ok=N full=N invalid=N other=N handled=N last=0xNN type=USER+3
 *
 * Read-only: queries the producer's stats and logs them. Does not reset
 * counters or affect the producer. Thread-context only (Zephyr LOG_INF
 * deferred path; not safe to call from ISR).
 */
void devkit_uart_producer_log_stats(void);

#endif /* DEVKIT_UART_PRODUCER_H */
