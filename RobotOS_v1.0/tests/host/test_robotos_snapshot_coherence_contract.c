/*
 * test_robotos_snapshot_coherence_contract.c
 *
 * Phase 6J-C -- Snapshot Coherence Validation
 *   Phase 6J-D -- peak_queue_depth passive observability field validation
 *
 * Validates that robotos_core_snapshot() produces a coherent view of all
 * core state fields, and that individual getter values match the snapshot
 * at every verified point.
 *
 * Invariants tested:
 *   - Every snapshot field matches its corresponding getter at the same state
 *   - pending_event_count tracks queue depth accurately after post and dispatch
 *   - dispatched_event_count increments on successful dispatch (not on error)
 *   - dropped_event_count tracks queue-full drops accurately
 *   - handler_error_count increments on handler error (dispatched_count does not)
 *   - admission_accepted/rejected_count track admission gate outcomes
 *   - unhandled_event_count tracks events dispatched with no registered handler
 *   - backpressure_active and producer_throttle_active reflect queue state
 *   - producer_throttled_count tracks try_post_event throttle outcomes
 *   - tick_count, init_count tracked by snapshot correctly
 *   - peak_queue_depth: high-water mark, monotonically non-decreasing (6J-D)
 *
 * All validation is deterministic. No Zephyr. No hardware. No timing.
 * Exit non-zero on any failure.
 */

#include "robotos_core.h"
#include "robotos_event_queue.h"

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/* ---- Test harness ---------------------------------------------------------- */

static int s_pass = 0;
static int s_fail = 0;

#define CHECK(label, expr)                                              \
    do {                                                                \
        if (expr) {                                                     \
            printf("[PASS] %s\n", (label));                            \
            s_pass++;                                                   \
        } else {                                                        \
            printf("[FAIL] %s  (line %d)\n", (label), __LINE__);      \
            s_fail++;                                                   \
        }                                                               \
    } while (0)

/* ---- Handlers -------------------------------------------------------------- */

static robotos_core_status_t err_handler(const robotos_event_t *ev, void *ctx)
{
    (void)ev; (void)ctx;
    return ROBOTOS_CORE_ERR_INVALID_STATE;
}

/* ---- Helpers --------------------------------------------------------------- */

static const robotos_event_t EV_USER = {
    .type = ROBOTOS_EVENT_USER, .timestamp_tick = 0, .arg0 = 0x6Au, .arg1 = 0
};

static const robotos_event_t EV_NONE = {
    .type = ROBOTOS_EVENT_NONE, .timestamp_tick = 0, .arg0 = 0, .arg1 = 0
};

static void drain_queue(void)
{
    while (robotos_core_pending_event_count() > 0) {
        robotos_core_dispatch_events(ROBOTOS_EVENT_QUEUE_CAPACITY);
    }
}

static void fill_queue_to_capacity(void)
{
    for (uint32_t i = 0; i < ROBOTOS_EVENT_QUEUE_CAPACITY; i++) {
        robotos_core_post_event(&EV_USER);
    }
}

/* Verify every snapshot field matches its getter at the current moment.
 * Both calls are single-threaded -- no state change can occur between them. */
static void check_all_getters_match_snapshot(const robotos_core_snapshot_t *s)
{
    CHECK("snapshot.pending matches getter",
          s->pending_event_count == robotos_core_pending_event_count());
    CHECK("snapshot.dropped matches getter",
          s->dropped_event_count == robotos_core_dropped_event_count());
    CHECK("snapshot.dispatched matches getter",
          s->dispatched_event_count == robotos_core_dispatched_event_count());
    CHECK("snapshot.handler_error matches getter",
          s->handler_error_count == robotos_core_handler_error_count());
    CHECK("snapshot.registered_handler_count matches getter",
          s->registered_handler_count == robotos_core_registered_handler_count());
    CHECK("snapshot.unhandled matches getter",
          s->unhandled_event_count == robotos_core_unhandled_event_count());
    CHECK("snapshot.accepted matches getter",
          s->admission_accepted_count == robotos_core_admission_accepted_count());
    CHECK("snapshot.rejected matches getter",
          s->admission_rejected_count == robotos_core_admission_rejected_count());
    CHECK("snapshot.throttled matches getter",
          s->producer_throttled_count == robotos_core_producer_throttled_count());
    CHECK("snapshot.backpressure_active matches getter",
          s->backpressure_active == robotos_core_backpressure_active());
    CHECK("snapshot.throttle_active matches getter",
          s->producer_throttle_active == robotos_core_producer_throttle_active());
    CHECK("snapshot.peak_queue_depth matches getter",
          s->peak_queue_depth == robotos_core_peak_queue_depth());
}

/* ---- main ------------------------------------------------------------------ */

int main(void)
{
    printf("=== Phase 6J-C/D: Snapshot Coherence + Peak Depth Validation ===\n\n");

    robotos_core_status_t   rc;
    robotos_core_snapshot_t snap;

    /* ---- Section 1: Initial state after init -------------------------------- */
    printf("[ Initial state after init ]\n");

    rc = robotos_core_init();
    CHECK("TC01: init returns OK", rc == ROBOTOS_CORE_OK);

    rc = robotos_core_snapshot(&snap);
    CHECK("TC02: snapshot returns OK", rc == ROBOTOS_CORE_OK);
    CHECK("TC03: snap.state == READY", snap.state == ROBOTOS_CORE_STATE_READY);
    CHECK("TC04: snap.pending == 0", snap.pending_event_count == 0u);
    CHECK("TC05: snap.dispatched == 0", snap.dispatched_event_count == 0u);
    CHECK("TC06: snap.dropped == 0", snap.dropped_event_count == 0u);
    CHECK("TC07: snap.accepted == 0", snap.admission_accepted_count == 0u);
    CHECK("TC08: snap.rejected == 0", snap.admission_rejected_count == 0u);
    CHECK("TC09: snap.unhandled == 0", snap.unhandled_event_count == 0u);
    CHECK("TC10: snap.handler_error == 0", snap.handler_error_count == 0u);
    CHECK("TC11: snap.registered_handler_count == 0", snap.registered_handler_count == 0u);
    CHECK("TC12: snap.throttled == 0", snap.producer_throttled_count == 0u);
    CHECK("TC13: snap.backpressure_active == false", !snap.backpressure_active);
    CHECK("TC14: snap.throttle_active == false", !snap.producer_throttle_active);
    CHECK("TC15: snap.version != NULL", snap.version != NULL);
    CHECK("TC16: snap.tick_count == 0", snap.tick_count == 0u);
    CHECK("TC17: snap.init_count == 1", snap.init_count == 1u);
    CHECK("TC18: snap.peak_queue_depth == 0 (no posts yet)", snap.peak_queue_depth == 0u);

    /* Getters must match on clean state */
    check_all_getters_match_snapshot(&snap);

    /* ---- Section 2: Post N events -- pending / accepted tracking -------------- */
    printf("\n[ Post events: pending and accepted tracking ]\n");

    uint32_t accepted_before = robotos_core_admission_accepted_count();

    /* Post 4 valid events (budget=1, so pending > budget immediately) */
    for (uint32_t i = 0; i < 4u; i++) {
        robotos_core_post_event(&EV_USER);
    }

    rc = robotos_core_snapshot(&snap);
    CHECK("TC30: snapshot after 4 posts returns OK", rc == ROBOTOS_CORE_OK);
    CHECK("TC31: snap.pending == 4", snap.pending_event_count == 4u);
    CHECK("TC32: snap.accepted increased by 4",
          snap.admission_accepted_count == accepted_before + 4u);
    CHECK("TC33: snap.dropped still 0", snap.dropped_event_count == 0u);
    CHECK("TC34: snap.backpressure_active == true (4 > budget=1)",
          snap.backpressure_active);
    CHECK("TC35: snap.throttle_active == true (pending > budget, queue not full)",
          snap.producer_throttle_active);
    CHECK("TC36: snap.peak_queue_depth == 4", snap.peak_queue_depth == 4u);

    check_all_getters_match_snapshot(&snap);

    /* Post a rejected event -- must not affect pending or accepted */
    uint32_t rejected_before = robotos_core_admission_rejected_count();
    robotos_core_post_event(&EV_NONE);
    rc = robotos_core_snapshot(&snap);
    CHECK("TC37: snap.rejected +1 after NONE post",
          snap.admission_rejected_count == rejected_before + 1u);
    CHECK("TC38: snap.pending unchanged after rejected post",
          snap.pending_event_count == 4u);
    CHECK("TC39: snap.peak unchanged after rejected post",
          snap.peak_queue_depth == 4u);

    /* ---- Section 3: Budget-limited dispatch (one event per call) ------------ */
    printf("\n[ Budget-limited dispatch: one event per tick ]\n");

    uint32_t dispatched_before = robotos_core_dispatched_event_count();

    rc = robotos_core_dispatch_events(1u);
    CHECK("TC40: dispatch_events(1) returns OK", rc == ROBOTOS_CORE_OK);

    rc = robotos_core_snapshot(&snap);
    CHECK("TC41: snap.pending == 3 (drained 1 of 4)", snap.pending_event_count == 3u);
    CHECK("TC42: snap.dispatched +1", snap.dispatched_event_count == dispatched_before + 1u);
    CHECK("TC43: snap.backpressure still active (3 > 1)", snap.backpressure_active);
    CHECK("TC44: snap.peak unchanged at 4 (drain does not reduce peak)",
          snap.peak_queue_depth == 4u);

    check_all_getters_match_snapshot(&snap);

    /* ---- Section 4: Drain all events ---------------------------------------- */
    printf("\n[ Drain all events ]\n");

    dispatched_before = robotos_core_dispatched_event_count();
    drain_queue();

    rc = robotos_core_snapshot(&snap);
    CHECK("TC45: snap.pending == 0 after drain", snap.pending_event_count == 0u);
    CHECK("TC46: snap.dispatched +3 (remaining 3 events drained)",
          snap.dispatched_event_count == dispatched_before + 3u);
    CHECK("TC47: snap.backpressure_active == false after drain", !snap.backpressure_active);
    CHECK("TC48: snap.throttle_active == false after drain", !snap.producer_throttle_active);
    CHECK("TC49: snap.peak_queue_depth still 4 (monotonically non-decreasing)",
          snap.peak_queue_depth == 4u);

    check_all_getters_match_snapshot(&snap);

    /* ---- Section 5: Queue overflow -- dropped count -------------------------- */
    printf("\n[ Queue overflow: dropped count ]\n");

    fill_queue_to_capacity();
    uint32_t dropped_before = robotos_core_dropped_event_count();

    rc = robotos_core_post_event(&EV_USER);
    CHECK("TC50: post to full queue returns ERR_FULL", rc == ROBOTOS_CORE_ERR_FULL);

    rc = robotos_core_snapshot(&snap);
    CHECK("TC51: snap.dropped +1 after overflow",
          snap.dropped_event_count == dropped_before + 1u);
    CHECK("TC52: snap.pending == CAPACITY (queue still full)",
          snap.pending_event_count == ROBOTOS_EVENT_QUEUE_CAPACITY);
    CHECK("TC53: snap.backpressure_active == true (queue full)", snap.backpressure_active);
    CHECK("TC54: snap.throttle_active == false when queue full "
          "(throttle not active when full)",
          !snap.producer_throttle_active);
    CHECK("TC55: snap.peak_queue_depth == CAPACITY",
          snap.peak_queue_depth == ROBOTOS_EVENT_QUEUE_CAPACITY);

    check_all_getters_match_snapshot(&snap);

    /* ---- Section 6: Throttle via try_post_event ----------------------------- */
    printf("\n[ Producer throttle: try_post_event under backpressure ]\n");

    /* Drain to 2 events (pending 2 > budget 1, queue not full -> throttle active) */
    while (robotos_core_pending_event_count() > 2u) {
        robotos_core_dispatch_events(1u);
    }

    uint32_t throttled_before = robotos_core_producer_throttled_count();

    rc = robotos_core_try_post_event(&EV_USER);
    CHECK("TC56: try_post_event returns ERR_THROTTLED (pending 2 > budget 1)",
          rc == ROBOTOS_CORE_ERR_THROTTLED);

    rc = robotos_core_snapshot(&snap);
    CHECK("TC57: snap.throttled +1 after ERR_THROTTLED",
          snap.producer_throttled_count == throttled_before + 1u);
    CHECK("TC58: snap.throttle_active == true", snap.producer_throttle_active);
    CHECK("TC59: snap.backpressure_active == true", snap.backpressure_active);
    CHECK("TC60: snap.pending unchanged (throttled event not enqueued)",
          snap.pending_event_count == 2u);

    check_all_getters_match_snapshot(&snap);

    /* ---- Section 7: Backpressure clears after full drain -------------------- */
    printf("\n[ Backpressure clears after drain ]\n");

    drain_queue();
    rc = robotos_core_snapshot(&snap);
    CHECK("TC61: snap.backpressure_active == false after drain", !snap.backpressure_active);
    CHECK("TC62: snap.throttle_active == false after drain", !snap.producer_throttle_active);
    CHECK("TC63: snap.pending == 0", snap.pending_event_count == 0u);

    check_all_getters_match_snapshot(&snap);

    /* ---- Section 8: Handler error counter ----------------------------------- */
    printf("\n[ Handler error counter ]\n");

    robotos_core_register_event_handler(ROBOTOS_EVENT_USER, err_handler, NULL);
    robotos_core_post_event(&EV_USER);

    uint32_t herr_before      = robotos_core_handler_error_count();
    uint32_t dispatched_base  = robotos_core_dispatched_event_count();
    uint32_t pending_before   = robotos_core_pending_event_count();

    rc = robotos_core_dispatch_events(1u);
    CHECK("TC64: dispatch with error handler returns handler error",
          rc == ROBOTOS_CORE_ERR_INVALID_STATE);

    rc = robotos_core_snapshot(&snap);
    CHECK("TC65: snap.handler_error_count +1",
          snap.handler_error_count == herr_before + 1u);
    CHECK("TC66: snap.dispatched_count unchanged (not incremented on error)",
          snap.dispatched_event_count == dispatched_base);
    CHECK("TC67: snap.pending -1 (event consumed from queue despite error)",
          snap.pending_event_count == pending_before - 1u ||
          (pending_before == 0u && snap.pending_event_count == 0u));

    check_all_getters_match_snapshot(&snap);

    robotos_core_unregister_event_handler(ROBOTOS_EVENT_USER);
    drain_queue();

    /* ---- Section 9: Dispatched count vs unhandled count -------------------- */
    printf("\n[ Dispatched count vs unhandled event count ]\n");

    /* No handlers registered. Post 3 events -- all unhandled on dispatch. */
    uint32_t unhandled_base  = robotos_core_unhandled_event_count();
    uint32_t dispatched_base2 = robotos_core_dispatched_event_count();

    for (uint32_t i = 0; i < 3u; i++) {
        robotos_core_post_event(&EV_USER);
    }
    robotos_core_dispatch_events(3u);

    rc = robotos_core_snapshot(&snap);
    CHECK("TC68: snap.unhandled +3 (no registered handler)",
          snap.unhandled_event_count == unhandled_base + 3u);
    CHECK("TC69: snap.dispatched +3 (unhandled events still count as dispatched)",
          snap.dispatched_event_count == dispatched_base2 + 3u);

    check_all_getters_match_snapshot(&snap);

    /* ---- Section 10: Tick count and init count ------------------------------ */
    printf("\n[ Tick count and init count tracking ]\n");

    uint32_t tick_before = robotos_core_tick_count();
    robotos_core_tick();
    robotos_core_tick();
    robotos_core_tick();

    rc = robotos_core_snapshot(&snap);
    CHECK("TC70: snap.tick_count == tick_before + 3",
          snap.tick_count == tick_before + 3u);
    CHECK("TC71: snap.tick_count matches robotos_core_tick_count()",
          snap.tick_count == robotos_core_tick_count());

    uint32_t init_before = snap.init_count;
    robotos_core_init(); /* repeated init: init_count++, tick_count preserved */
    rc = robotos_core_snapshot(&snap);
    CHECK("TC72: snap.init_count == init_before + 1 after repeated init",
          snap.init_count == init_before + 1u);
    CHECK("TC73: snap.tick_count preserved after repeated init",
          snap.tick_count == tick_before + 3u);

    /* ---- Section 11: Peak queue depth high-water mark semantics (6J-D) ------ */
    printf("\n[ Peak queue depth high-water mark (Phase 6J-D) ]\n");

    /* Drain any residue first */
    drain_queue();

    uint32_t peak_before = robotos_core_peak_queue_depth();

    /* Post 3 events: peak should advance to peak_before+3 if > current peak */
    robotos_core_post_event(&EV_USER);
    robotos_core_post_event(&EV_USER);
    robotos_core_post_event(&EV_USER);

    rc = robotos_core_snapshot(&snap);
    CHECK("TC74: snap.peak_queue_depth >= 3 after posting 3 events",
          snap.peak_queue_depth >= 3u);
    CHECK("TC75: snap.peak_queue_depth >= previous peak (monotonically non-decreasing)",
          snap.peak_queue_depth >= peak_before);
    CHECK("TC76: snap.peak_queue_depth matches getter",
          snap.peak_queue_depth == robotos_core_peak_queue_depth());

    /* Drain and post fewer events: peak must NOT decrease */
    drain_queue();
    uint32_t peak_after_drain = robotos_core_peak_queue_depth();
    CHECK("TC77: peak unchanged after drain (drain does not reset peak)",
          peak_after_drain == snap.peak_queue_depth);

    robotos_core_post_event(&EV_USER); /* 1 event < current peak */
    rc = robotos_core_snapshot(&snap);
    CHECK("TC78: peak does not decrease when fewer events posted",
          snap.peak_queue_depth >= peak_after_drain);
    CHECK("TC79: peak getter matches snapshot after partial post",
          snap.peak_queue_depth == robotos_core_peak_queue_depth());

    /* Fill queue to capacity: peak must advance to CAPACITY */
    drain_queue();
    fill_queue_to_capacity();
    rc = robotos_core_snapshot(&snap);
    CHECK("TC80: snap.peak_queue_depth == CAPACITY after filling queue",
          snap.peak_queue_depth == ROBOTOS_EVENT_QUEUE_CAPACITY);
    CHECK("TC81: peak getter == CAPACITY",
          robotos_core_peak_queue_depth() == ROBOTOS_EVENT_QUEUE_CAPACITY);

    /* Drain and confirm peak stays at CAPACITY */
    drain_queue();
    rc = robotos_core_snapshot(&snap);
    CHECK("TC82: peak still CAPACITY after full drain (monotonic guarantee)",
          snap.peak_queue_depth == ROBOTOS_EVENT_QUEUE_CAPACITY);

    /* ---- Section 12: Final global getter coherence check ------------------- */
    printf("\n[ Final global getter coherence ]\n");

    drain_queue();
    rc = robotos_core_snapshot(&snap);
    CHECK("TC83: final snapshot returns OK", rc == ROBOTOS_CORE_OK);
    check_all_getters_match_snapshot(&snap);

    /* ---- Summary ------------------------------------------------------------- */
    printf("\n=== Phase 6J-C/D Results: %d passed, %d failed ===\n", s_pass, s_fail);
    return (s_fail > 0) ? 1 : 0;
}

