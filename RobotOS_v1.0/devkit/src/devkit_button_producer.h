/*
 * devkit_button_producer.h
 * RobotOS devkit user-button event producer (Phase 9A-A).
 *
 * Thin devkit-local producer that bridges a real GPIO/EXTI hardware input
 * (the STM32F411E-DISCO user button, alias sw0) to the portable RobotOS
 * core event pipeline. The first non-synthetic event source for RobotOS:
 *
 *   PA0 user button rising edge
 *     -> Zephyr GPIO ISR callback
 *     -> robotos_core_post_event()           [Phase 5G ISR-safe contract]
 *     -> core queue
 *     -> robotos_core_tick() dispatcher      [thread context, budget=1/tick]
 *     -> button handler (devkit-local)
 *     -> ROBOTOS_BTN telemetry / Phase 9A milestone log
 *
 * Architectural shape (Phase 9A-A):
 *   - DEVKIT-LOCAL ONLY. Zephyr GPIO/devicetree usage is confined here.
 *     core/ and platform/ remain Zephyr-free and button-agnostic.
 *   - No new core API. No new platform abstraction (no robotos_platform_gpio).
 *   - No scheduler change. No new event type in the core enum (uses
 *     ROBOTOS_EVENT_USER + 2; see DEVKIT_BUTTON_PRODUCER_TYPE below).
 *   - Phase 5G ISR-safe contract: ISR posts via robotos_core_post_event(),
 *     does not log/sleep/dispatch/register/alloc.
 *   - No software debounce in 9A-A. Mechanical bounce, if observed, is
 *     documented but not fixed; a future phase may add devkit-local debounce.
 *
 * Hard non-goals (Phase 9A-A, deliberate):
 *   - No core/platform mutation.
 *   - No GPIO platform abstraction (premature; only one workload).
 *   - No threading, dynamic allocation, or floating point.
 *   - No telemetry feedback into scheduling.
 *   - No new Zephyr task/thread.
 */

#ifndef DEVKIT_BUTTON_PRODUCER_H
#define DEVKIT_BUTTON_PRODUCER_H

#include <stdint.h>

#include "robotos_core.h"

/*
 * Event type used by this producer. Distinct from:
 *   - Phase 6I producer:  ROBOTOS_EVENT_USER     (= 100)
 *   - Phase 6M producer:  ROBOTOS_EVENT_USER + 1 (= 101)
 *   - Phase 9A-A button:  ROBOTOS_EVENT_USER + 2 (= 102)  <-- this module
 *
 * Admissible per the Phase 4I admission gate (USER+ accepted).
 * No new core enum required.
 */
#define DEVKIT_BUTTON_PRODUCER_TYPE \
	((robotos_event_type_t)(ROBOTOS_EVENT_USER + 2u))

/* arg0 marker for runtime tagging / handler validation. */
#define DEVKIT_BUTTON_PRODUCER_MARKER 0x9A0Au

/*
 * Producer-local counters. Distinct from core admission/drop counters
 * so the two views can be cross-checked in RTT.
 *
 *   attempted: every ISR firing increments this once
 *   ok       : robotos_core_post_event returned OK
 *   full     : returned ERR_FULL (queue at capacity)
 *   invalid  : returned ERR_INVALID_ARG (defensive; should always be 0)
 *   other    : any other non-OK return (defensive; should always be 0)
 *   handled  : button handler invocations (thread context)
 */
typedef struct {
	uint32_t attempted;
	uint32_t ok;
	uint32_t full;
	uint32_t invalid;
	uint32_t other;
	uint32_t handled;
} devkit_button_producer_stats_t;

/*
 * Initialize the button producer:
 *   1. Verify sw0 GPIO device is ready.
 *   2. Configure pin as input.
 *   3. Configure interrupt: edge-triggered, active edge.
 *   4. Register the button event handler with core for
 *      DEVKIT_BUTTON_PRODUCER_TYPE.
 *   5. Install GPIO callback and enable interrupt.
 *
 * Must be called after robotos_core_init() so handler registration succeeds.
 *
 * Returns ROBOTOS_CORE_OK on success.
 * Returns ROBOTOS_CORE_ERR_INVALID_STATE if GPIO not ready or
 *         pin/interrupt configure fails.
 * Returns the underlying robotos_core_register_event_handler status if
 *         handler registration fails.
 *
 * Idempotent only in the sense that the producer counters are NOT reset on
 * a second call; however gpio_add_callback should not be called twice with
 * the same callback struct. Callers should call this exactly once.
 */
robotos_core_status_t devkit_button_producer_init(void);

/*
 * Snapshot the producer-local counters into a caller-owned struct.
 * If out is NULL, the call is a no-op.
 *
 * Reads volatile ISR counters in thread context. The values are a near-
 * coherent snapshot; readers tolerating single-tick skew are safe (the
 * RTT cadence sampling tolerates this by design).
 */
void devkit_button_producer_get_stats(devkit_button_producer_stats_t *out);

/*
 * Emit one ROBOTOS_BTN log line via Zephyr LOG_INF.
 *
 * Stable single-line format (integer fields only):
 *   ROBOTOS_BTN attempted=N ok=N full=N invalid=N other=N handled=N type=USER+2
 *
 * Read-only: queries the producer's stats and logs them. Does not reset
 * counters or affect the producer. Thread-context only (Zephyr LOG_INF
 * deferred path; not safe to call from ISR).
 */
void devkit_button_producer_log_stats(void);

#endif /* DEVKIT_BUTTON_PRODUCER_H */
