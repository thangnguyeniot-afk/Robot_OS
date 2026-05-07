/*
 * test_robotos_handler_routing_stress_contract.c
 *
 * Phase 6J-A -- Handler Routing Stress Contract Tests
 *
 * Validates multi-handler dispatch routing correctness:
 *   - Each handler receives only events of its registered type (no cross-contamination)
 *   - FIFO ordering preserved within each event type across mixed-type post sequences
 *   - Dual-handler and three-handler isolation under interleaved post/dispatch
 *   - Unhandled events counted correctly alongside handled events in the same run
 *   - Repeated post/dispatch cycles maintain correct per-handler routing
 *   - USER+ range routing boundary: far USER+ types route to their registered handler
 *   - Snapshot counters remain coherent after all routing operations
 *
 * No Zephyr. No hardware. No timing dependencies.
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

/* ---- Per-handler instrumentation ------------------------------------------ */

#define SEQ_MAX 32u

typedef struct {
    uint32_t             call_count;
    bool                 saw_wrong_type;
    robotos_event_type_t registered_type;
    uint32_t             arg0_seq[SEQ_MAX];
    uint32_t             seq_len;
} handler_state_t;

static handler_state_t s_h[3];

static void reset_handler_state(handler_state_t *h)
{
    h->call_count     = 0;
    h->saw_wrong_type = false;
    h->seq_len        = 0;
    memset(h->arg0_seq, 0, sizeof(h->arg0_seq));
    /* registered_type is intentionally NOT cleared: routing check requires it */
}

static void reset_all_handlers(void)
{
    for (int i = 0; i < 3; i++) {
        reset_handler_state(&s_h[i]);
    }
}

static robotos_core_status_t routing_handler(const robotos_event_t *ev, void *ctx)
{
    handler_state_t *h = (handler_state_t *)ctx;
    h->call_count++;
    if (ev) {
        if (ev->type != h->registered_type) {
            h->saw_wrong_type = true;
        }
        if (h->seq_len < SEQ_MAX) {
            h->arg0_seq[h->seq_len++] = ev->arg0;
        }
    }
    return ROBOTOS_CORE_OK;
}

/* ---- Helpers --------------------------------------------------------------- */

static robotos_event_t make_ev(robotos_event_type_t type, uint32_t arg0)
{
    robotos_event_t e;
    e.type           = type;
    e.timestamp_tick = 0;
    e.arg0           = arg0;
    e.arg1           = 0;
    return e;
}

static void drain_queue(void)
{
    while (robotos_core_pending_event_count() > 0) {
        robotos_core_dispatch_events(ROBOTOS_EVENT_QUEUE_CAPACITY);
    }
}

/* ---- main ------------------------------------------------------------------ */

int main(void)
{
    printf("=== Phase 6J-A: Handler Routing Stress Contract Tests ===\n\n");

    robotos_core_status_t rc;
    robotos_event_t       ev;

    s_h[0].registered_type = ROBOTOS_EVENT_USER;
    s_h[1].registered_type = (robotos_event_type_t)(ROBOTOS_EVENT_USER + 1u);
    s_h[2].registered_type = (robotos_event_type_t)(ROBOTOS_EVENT_USER + 2u);
    reset_all_handlers();

    /* ---- Init ---------------------------------------------------------------- */
    printf("[ Init ]\n");

    rc = robotos_core_init();
    CHECK("TC01: init returns OK", rc == ROBOTOS_CORE_OK);
    CHECK("TC02: state == READY", robotos_core_state() == ROBOTOS_CORE_STATE_READY);

    /* ---- Section 1: Dual-handler isolation ---------------------------------- */
    printf("\n[ Dual-handler isolation ]\n");

    rc = robotos_core_register_event_handler(s_h[0].registered_type,
                                              routing_handler, &s_h[0]);
    CHECK("TC03: register USER handler returns OK", rc == ROBOTOS_CORE_OK);
    rc = robotos_core_register_event_handler(s_h[1].registered_type,
                                              routing_handler, &s_h[1]);
    CHECK("TC04: register USER+1 handler returns OK", rc == ROBOTOS_CORE_OK);
    CHECK("TC05: registered_handler_count == 2",
          robotos_core_registered_handler_count() == 2u);

    /* Interleaved post: USER(10), USER+1(20), USER(11), USER+1(21) */
    ev = make_ev(s_h[0].registered_type, 10u); robotos_core_post_event(&ev);
    ev = make_ev(s_h[1].registered_type, 20u); robotos_core_post_event(&ev);
    ev = make_ev(s_h[0].registered_type, 11u); robotos_core_post_event(&ev);
    ev = make_ev(s_h[1].registered_type, 21u); robotos_core_post_event(&ev);

    rc = robotos_core_dispatch_events(4u);
    CHECK("TC06: dispatch_events(4) returns OK", rc == ROBOTOS_CORE_OK);
    CHECK("TC07: h0 called exactly 2 times", s_h[0].call_count == 2u);
    CHECK("TC08: h1 called exactly 2 times", s_h[1].call_count == 2u);
    CHECK("TC09: h2 not called (not yet registered)", s_h[2].call_count == 0u);
    CHECK("TC10: h0 no cross-contamination", !s_h[0].saw_wrong_type);
    CHECK("TC11: h1 no cross-contamination", !s_h[1].saw_wrong_type);
    /* FIFO within type: h0 received 10 then 11 */
    CHECK("TC12: h0 FIFO first arg0 == 10",
          s_h[0].seq_len >= 1u && s_h[0].arg0_seq[0] == 10u);
    CHECK("TC13: h0 FIFO second arg0 == 11",
          s_h[0].seq_len >= 2u && s_h[0].arg0_seq[1] == 11u);
    /* FIFO within type: h1 received 20 then 21 */
    CHECK("TC14: h1 FIFO first arg0 == 20",
          s_h[1].seq_len >= 1u && s_h[1].arg0_seq[0] == 20u);
    CHECK("TC15: h1 FIFO second arg0 == 21",
          s_h[1].seq_len >= 2u && s_h[1].arg0_seq[1] == 21u);

    /* ---- Section 2: Three-handler routing ----------------------------------- */
    printf("\n[ Three-handler routing ]\n");

    rc = robotos_core_register_event_handler(s_h[2].registered_type,
                                              routing_handler, &s_h[2]);
    CHECK("TC16: register USER+2 handler returns OK", rc == ROBOTOS_CORE_OK);
    CHECK("TC17: registered_handler_count == 3",
          robotos_core_registered_handler_count() == 3u);

    reset_all_handlers();
    /* Round-robin post: U0(1), U1(1), U2(1), U0(2), U1(2), U2(2) */
    ev = make_ev(s_h[0].registered_type, 1u); robotos_core_post_event(&ev);
    ev = make_ev(s_h[1].registered_type, 1u); robotos_core_post_event(&ev);
    ev = make_ev(s_h[2].registered_type, 1u); robotos_core_post_event(&ev);
    ev = make_ev(s_h[0].registered_type, 2u); robotos_core_post_event(&ev);
    ev = make_ev(s_h[1].registered_type, 2u); robotos_core_post_event(&ev);
    ev = make_ev(s_h[2].registered_type, 2u); robotos_core_post_event(&ev);

    rc = robotos_core_dispatch_events(6u);
    CHECK("TC18: dispatch_events(6) returns OK", rc == ROBOTOS_CORE_OK);
    CHECK("TC19: h0 called exactly 2 times", s_h[0].call_count == 2u);
    CHECK("TC20: h1 called exactly 2 times", s_h[1].call_count == 2u);
    CHECK("TC21: h2 called exactly 2 times", s_h[2].call_count == 2u);
    CHECK("TC22: h0 no cross-contamination", !s_h[0].saw_wrong_type);
    CHECK("TC23: h1 no cross-contamination", !s_h[1].saw_wrong_type);
    CHECK("TC24: h2 no cross-contamination", !s_h[2].saw_wrong_type);
    /* FIFO within each type (interleaved post preserves per-type order) */
    CHECK("TC25: h0 FIFO sequence: arg0[0]==1, arg0[1]==2",
          s_h[0].seq_len == 2u &&
          s_h[0].arg0_seq[0] == 1u && s_h[0].arg0_seq[1] == 2u);
    CHECK("TC26: h1 FIFO sequence: arg0[0]==1, arg0[1]==2",
          s_h[1].seq_len == 2u &&
          s_h[1].arg0_seq[0] == 1u && s_h[1].arg0_seq[1] == 2u);
    CHECK("TC27: h2 FIFO sequence: arg0[0]==1, arg0[1]==2",
          s_h[2].seq_len == 2u &&
          s_h[2].arg0_seq[0] == 1u && s_h[2].arg0_seq[1] == 2u);

    /* ---- Section 3: Unhandled events alongside handled ---------------------- */
    printf("\n[ Unhandled events alongside handled ]\n");

    /* Unregister h1 and h2; keep only h0 for USER */
    robotos_core_unregister_event_handler(s_h[1].registered_type);
    robotos_core_unregister_event_handler(s_h[2].registered_type);
    CHECK("TC28: registered_handler_count == 1 after two unregisters",
          robotos_core_registered_handler_count() == 1u);

    uint32_t unhandled_before  = robotos_core_unhandled_event_count();
    uint32_t dispatched_before = robotos_core_dispatched_event_count();
    reset_all_handlers();

    /* Post: USER(handled), USER+1(unhandled), CORE_TICK(unhandled) */
    ev = make_ev(ROBOTOS_EVENT_USER, 99u);          robotos_core_post_event(&ev);
    ev = make_ev(s_h[1].registered_type, 0u);       robotos_core_post_event(&ev);
    ev = make_ev(ROBOTOS_EVENT_CORE_TICK, 0u);      robotos_core_post_event(&ev);

    rc = robotos_core_dispatch_events(3u);
    CHECK("TC29: dispatch_events(3) returns OK", rc == ROBOTOS_CORE_OK);
    CHECK("TC30: h0 called exactly 1 time (USER only)", s_h[0].call_count == 1u);
    CHECK("TC31: h0 received correct arg0 == 99",
          s_h[0].seq_len == 1u && s_h[0].arg0_seq[0] == 99u);
    CHECK("TC32: h1 not called (unregistered)", s_h[1].call_count == 0u);
    CHECK("TC33: h2 not called (unregistered)", s_h[2].call_count == 0u);
    CHECK("TC34: unhandled_count +2 (USER+1 and CORE_TICK unhandled)",
          robotos_core_unhandled_event_count() == unhandled_before + 2u);
    CHECK("TC35: dispatched_count +3 (all 3 events consumed from queue)",
          robotos_core_dispatched_event_count() == dispatched_before + 3u);

    /* ---- Section 4: Repeated post/dispatch cycles --------------------------- */
    printf("\n[ Repeated post/dispatch cycles ]\n");

    /* Re-register h1 alongside the still-registered h0 */
    rc = robotos_core_register_event_handler(s_h[1].registered_type,
                                              routing_handler, &s_h[1]);
    CHECK("TC36: re-register USER+1 handler returns OK", rc == ROBOTOS_CORE_OK);

    reset_all_handlers();
    /* 5 rounds: each round posts USER(round) + USER+1(round), then dispatches 2 */
    for (uint32_t round = 0; round < 5u; round++) {
        ev = make_ev(s_h[0].registered_type, round); robotos_core_post_event(&ev);
        ev = make_ev(s_h[1].registered_type, round); robotos_core_post_event(&ev);
        robotos_core_dispatch_events(2u);
    }

    CHECK("TC37: h0 called 5 times over 5 cycles", s_h[0].call_count == 5u);
    CHECK("TC38: h1 called 5 times over 5 cycles", s_h[1].call_count == 5u);
    CHECK("TC39: h0 no cross-contamination across cycles", !s_h[0].saw_wrong_type);
    CHECK("TC40: h1 no cross-contamination across cycles", !s_h[1].saw_wrong_type);
    /* Per-type FIFO across cycles: arg0 values are round numbers 0..4 */
    CHECK("TC41: h0 FIFO across cycles (arg0 0,1,2,3,4)",
          s_h[0].seq_len == 5u &&
          s_h[0].arg0_seq[0] == 0u && s_h[0].arg0_seq[1] == 1u &&
          s_h[0].arg0_seq[2] == 2u && s_h[0].arg0_seq[3] == 3u &&
          s_h[0].arg0_seq[4] == 4u);
    CHECK("TC42: h1 FIFO across cycles (arg0 0,1,2,3,4)",
          s_h[1].seq_len == 5u &&
          s_h[1].arg0_seq[0] == 0u && s_h[1].arg0_seq[1] == 1u &&
          s_h[1].arg0_seq[2] == 2u && s_h[1].arg0_seq[3] == 3u &&
          s_h[1].arg0_seq[4] == 4u);

    /* ---- Section 5: USER+ range routing boundary ---------------------------- */
    printf("\n[ USER+ range routing boundary ]\n");

    /* Register a handler at the far end of the USER+ range */
    robotos_event_type_t far_type = (robotos_event_type_t)(ROBOTOS_EVENT_USER + 99u);
    handler_state_t h_far = { .registered_type = far_type };

    rc = robotos_core_register_event_handler(far_type, routing_handler, &h_far);
    CHECK("TC43: register USER+99 handler returns OK", rc == ROBOTOS_CORE_OK);

    ev = make_ev(far_type, 0xABu);
    robotos_core_post_event(&ev);
    reset_handler_state(&h_far);
    rc = robotos_core_dispatch_events(1u);
    CHECK("TC44: dispatch USER+99 event returns OK", rc == ROBOTOS_CORE_OK);
    CHECK("TC45: far handler called exactly once", h_far.call_count == 1u);
    CHECK("TC46: far handler received correct type (no cross-contamination)",
          !h_far.saw_wrong_type);
    CHECK("TC47: far handler received correct arg0 == 0xAB",
          h_far.seq_len >= 1u && h_far.arg0_seq[0] == 0xABu);

    /* Confirm h0 and h1 were NOT called during the far-type dispatch */
    CHECK("TC48: h0 not called during far-type dispatch",
          s_h[0].call_count == 5u); /* unchanged from end of Section 4 */
    CHECK("TC49: h1 not called during far-type dispatch",
          s_h[1].call_count == 5u); /* unchanged from end of Section 4 */

    robotos_core_unregister_event_handler(far_type);

    /* ---- Section 6: Snapshot coherence after routing stress ------------------ */
    printf("\n[ Snapshot coherence after routing stress ]\n");

    drain_queue();
    robotos_core_snapshot_t snap;
    rc = robotos_core_snapshot(&snap);
    CHECK("TC50: snapshot returns OK", rc == ROBOTOS_CORE_OK);
    CHECK("TC51: snapshot.state == READY", snap.state == ROBOTOS_CORE_STATE_READY);
    CHECK("TC52: snapshot.pending == 0 (queue drained)", snap.pending_event_count == 0u);
    CHECK("TC53: snapshot.unhandled_event_count matches getter",
          snap.unhandled_event_count == robotos_core_unhandled_event_count());
    CHECK("TC54: snapshot.dispatched_event_count matches getter",
          snap.dispatched_event_count == robotos_core_dispatched_event_count());
    CHECK("TC55: snapshot.registered_handler_count matches getter",
          snap.registered_handler_count == robotos_core_registered_handler_count());

    /* ---- Summary ------------------------------------------------------------- */
    printf("\n=== Phase 6J-A Results: %d passed, %d failed ===\n", s_pass, s_fail);
    return (s_fail > 0) ? 1 : 0;
}
