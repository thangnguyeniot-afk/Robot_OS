/*
 * devkit_timer_producer.h
 * RobotOS devkit periodic producer diagnostic (Phase 6M).
 *
 * Bounded thread-context producer that posts one admissible event every
 * DEVKIT_TIMER_PRODUCER_TICK_PERIOD devkit ticks, intended to demonstrate
 * realistic steady-state producer/consumer interaction in RTT logs:
 *
 *   - producer rate (1 event / 2 ticks) <= consumer rate (1 event / tick)
 *     so the queue stays at or near zero in steady state, after the
 *     Phase 6I startup burst drains.
 *   - peak_queue_depth (Phase 6J-D) advances at least once.
 *   - dispatched_event_count grows steadily.
 *   - dropped/throttled remain explainable from the Phase 6I burst window
 *     and the producer's own ERR_FULL accounting.
 *
 * Architectural shape:
 *   - Pure C. No Zephyr types, no k_*, no LOG macros. Host-testable.
 *   - Uses only the public robotos_core post API.
 *   - Lives in devkit/ alongside other devkit infrastructure; not portable
 *     core. The Zephyr-bound LOG emission for ROBOTOS_PROD is in
 *     devkit_observability.c, not here.
 *
 * Event boundaries:
 *   - Event type: ROBOTOS_EVENT_USER + 1 (admissible per Phase 4I gate).
 *   - This is intentionally distinct from the Phase 6I producer's
 *     ROBOTOS_EVENT_USER usage so the two producers can coexist without
 *     handler-routing conflict or counter pollution.
 *   - arg0 = DEVKIT_TIMER_PRODUCER_MARKER (0x6D00) for runtime tagging.
 *   - arg1 = current attempt index (uint32_t monotonic).
 *
 * Post API choice:
 *   - Uses robotos_core_post_event() (raw ingestion). Rationale: Phase 6M's
 *     purpose is to demonstrate raw producer success/drop visibility under
 *     a low-pressure cadence, not to exercise the throttle path (which is
 *     already covered by Phase 4K/6E/6I). try_post_event would be the right
 *     choice if the demonstration goal were throttle visibility instead.
 *
 * Hard non-goals (Phase 6M, deliberate):
 *   - Does NOT change scheduler / dispatch / queue / retry / admission semantics.
 *   - Does NOT introduce auto-retry, priority dispatch, or async producer state.
 *   - Does NOT use ISR context. Thread-context posting only.
 *   - Does NOT influence runtime timing beyond a single post call per cadence hit.
 *   - Does NOT expose a feedback loop from queue metrics back to producer rate.
 */

#ifndef DEVKIT_TIMER_PRODUCER_H
#define DEVKIT_TIMER_PRODUCER_H

#include <stdint.h>
#include <stdbool.h>

#include "robotos_core.h"

/* Bounded cadence: post one event every N devkit ticks. */
#define DEVKIT_TIMER_PRODUCER_TICK_PERIOD 2u

/* Event type used by this producer. Distinct from Phase 6I (USER). */
#define DEVKIT_TIMER_PRODUCER_TYPE \
	((robotos_event_type_t)(ROBOTOS_EVENT_USER + 1u))

/* arg0 marker for runtime tagging / handler validation. */
#define DEVKIT_TIMER_PRODUCER_MARKER 0x6D00u

/*
 * Producer-local counters. Distinct from core admission/drop counters so
 * the two views can be compared in RTT to verify they agree.
 */
typedef struct {
	uint32_t attempted; /* every cadence hit increments this once          */
	uint32_t ok;        /* post returned ROBOTOS_CORE_OK                   */
	uint32_t throttled; /* post returned ERR_THROTTLED (not used here -    */
	                    /*   reserved for future try_post_event variants)  */
	uint32_t dropped;   /* post returned ERR_FULL                          */
	uint32_t invalid;   /* post returned ERR_INVALID_ARG (defensive)       */
	uint32_t other;     /* any other non-OK return (defensive)             */
} devkit_timer_producer_stats_t;

/*
 * Register the producer's event handler with core. Must be called after
 * robotos_core_init(). Returns the underlying registration status code.
 *
 * After a successful init, devkit_timer_producer_should_post() and
 * devkit_timer_producer_on_tick() become active. Before init they are
 * no-ops.
 *
 * Idempotent: a second call replaces the handler registration; counters
 * are NOT reset (they reflect process lifetime).
 */
robotos_core_status_t devkit_timer_producer_init(void);

/*
 * Pure cadence predicate. Returns true when the producer should post on
 * this tick value. Returns false before init regardless of tick_count.
 *
 * Decision: init_done && (tick_count % DEVKIT_TIMER_PRODUCER_TICK_PERIOD == 0).
 *
 * Pure function over module state and the input. No side effects.
 */
bool devkit_timer_producer_should_post(uint32_t tick_count);

/*
 * Cadence-driven post. If devkit_timer_producer_should_post(tick_count)
 * is true, build one event and call robotos_core_post_event() once,
 * updating the producer's local counters according to the return code.
 *
 * Otherwise no-op.
 *
 * Never blocks. Never logs (LOG emission is in devkit_observability).
 * Single post per call -- producer cannot starve the run loop.
 *
 * Thread-context only. Not safe to call from ISR.
 */
void devkit_timer_producer_on_tick(uint32_t tick_count);

/*
 * Snapshot of producer-local counters into a caller-owned struct.
 * If out is NULL, the call is a no-op.
 */
void devkit_timer_producer_get_stats(devkit_timer_producer_stats_t *out);

#endif /* DEVKIT_TIMER_PRODUCER_H */
