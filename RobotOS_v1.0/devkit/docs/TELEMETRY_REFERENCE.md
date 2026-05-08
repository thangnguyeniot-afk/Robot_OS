# RobotOS Devkit Telemetry Reference

Quick reference for the three runtime diagnostic log streams emitted by the
devkit observability surface (Phase 6K/6L/6M).

**Purpose:** link each field to its canonical definition.
This document does NOT redefine semantics — it is navigation glue only.
Canonical authority for each field is marked in the "Canonical Definition" column.

For the full narrative (design decisions, implementation notes, RTT evidence)
see `RobotOS_v1.0/devkit/docs/DEVKIT_PROGRESS.md` Phases 6K, 6L, 6M, and 6Z.

---

## ROBOTOS_OBS — Runtime Snapshot

Emitted by `devkit_observability_log_snapshot()` in `devkit_observability.c`.

**Literal log shape** (from `devkit_observability.c` LOG_INF format string):

```text
ROBOTOS_OBS state=<NAME> ticks=N pending=N peak=N dropped=N dispatched=N
            herr=N throttled=N rejected=N accepted=N unhandled=N bp=0|1 th_active=0|1
```

**Cadence:** one baseline line at init completion; one periodic line every
`DEVKIT_OBSERVABILITY_LOG_INTERVAL_TICKS` (= 10) runtime ticks (~5 s at
`DEVKIT_TICK_MS = 500`). Not configurable at runtime.

**Field reference:**

| Field | Type | Meaning | Canonical Definition |
|-------|------|---------|----------------------|
| `state` | string | Core lifecycle state: `UNINIT`, `READY`, `ERROR`, `UNKNOWN` | `core/README.md` — "Lifecycle States" table |
| `ticks` | uint32 | `snap.tick_count` — total `robotos_core_tick()` calls | `core/README.md` — Phase 4B/4G |
| `pending` | uint32 | `snap.pending_event_count` — events in queue awaiting dispatch | `core/README.md` — Phase 4J counter table |
| `peak` | uint32 | `snap.peak_queue_depth` — high-water queue depth (monotonically non-decreasing) | `DEVKIT_PROGRESS.md` — Phase 6J |
| `dropped` | uint32 | `snap.dropped_event_count` — `ERR_FULL` queue-push failures (NOT admission rejections) | `core/README.md` — Phase 4J "Key distinctions" |
| `dispatched` | uint32 | `snap.dispatched_event_count` — events popped from queue and delivered | `core/README.md` — Phase 4J counter table |
| `herr` | uint32 | `snap.handler_error_count` — registered handlers that returned non-OK | `core/README.md` — Phase 4J counter table |
| `throttled` | uint32 | `snap.producer_throttled_count` — `ERR_THROTTLED` returns from `try_post_event` | `core/README.md` — Phase 4K "New API" |
| `rejected` | uint32 | `snap.admission_rejected_count` — events rejected by admission gate (invalid type) | `core/README.md` — Phase 4I/4J counter table |
| `accepted` | uint32 | `snap.admission_accepted_count` — type passed gate AND queue push succeeded | `core/README.md` — Phase 4J counter table |
| `unhandled` | uint32 | `snap.unhandled_event_count` — dispatched events with no registered handler | `core/README.md` — Phase 4J counter table |
| `bp` | 0 or 1 | `snap.backpressure_active` — true when `pending > budget` OR queue full | `core/README.md` — Phase 4J "Backpressure Rule" |
| `th_active` | 0 or 1 | `snap.producer_throttle_active` — true when `pending > budget AND queue not full` | `core/README.md` — Phase 4K "New Getters" |

**Coherence invariant** (verified by Phase 6Z at ticks=120):
`accepted - dispatched = pending`.

`dropped` increments on `ERR_FULL` (queue capacity hit).
`rejected` increments on `ERR_INVALID_ARG` (admission gate fail).
These are mutually exclusive paths — a single failed post increments exactly one.

---

## ROBOTOS_FAULT — Fault Register Snapshot

Emitted by `devkit_observability_log_fault()` in `devkit_observability.c`.

**Literal log shape** (from `devkit_observability.c` LOG_INF format string):

```text
ROBOTOS_FAULT active=0|1 cfsr=0x........ hfsr=0x........ context=<NAME>
```

**Cadence:** same as ROBOTOS_OBS (one baseline at init; one periodic per cadence window).
Each cadence window emits ROBOTOS_OBS, then ROBOTOS_FAULT, then ROBOTOS_PROD.

**Field reference:**

| Field | Type | Meaning | Canonical Definition |
|-------|------|---------|----------------------|
| `active` | 0 or 1 | `(cfsr != 0) || (hfsr != 0)` — true if any fault bit is set | `DEVKIT_PROGRESS.md` — Phase 6L "Log Format" |
| `cfsr` | hex uint32 | Raw value of `*(0xE000ED28)` — ARMv7-M Configurable Fault Status Register | `DEVKIT_PROGRESS.md` — Phase 6L "Log Format"; ARMv7-M ARM B3.2.10 |
| `hfsr` | hex uint32 | Raw value of `*(0xE000ED2C)` — ARMv7-M HardFault Status Register | `DEVKIT_PROGRESS.md` — Phase 6L "Log Format"; ARMv7-M ARM B3.2.10 |
| `context` | string | `"none"` when `active=0`; `"fault"` otherwise | `DEVKIT_PROGRESS.md` — Phase 6L "Log Format" |

**Normal-run expected values:** `active=0 cfsr=0x00000000 hfsr=0x00000000 context=none`.
Confirmed throughout 60 s Phase 6Z RTT capture.

**Note on `HFSR.DEBUGEVT`:** bit 1 of HFSR may be set when a debugger triggers a
halt event, even without a real crash. Phase 6Z used OpenOCD in RTT-poll mode
(non-halting) and observed `hfsr=0x00000000`. A hard-debug session may show non-zero
HFSR that does not indicate a real fault. See `DEVKIT_PROGRESS.md` Phase 6L
"Known Limitations".

**What this surface does NOT do:** no bit-level decoding of CFSR sub-fields
(UFSR/BFSR/MMFSR), no BFAR/MMFAR fault-address readout, no recovery, no reset,
no scheduler influence.

---

## ROBOTOS_PROD — Producer Diagnostic Snapshot

Emitted by `devkit_observability_log_producer_stats()` in `devkit_observability.c`.

**Literal log shape** (from `devkit_observability.c` LOG_INF format string):

```text
ROBOTOS_PROD attempted=N ok=N throttled=N dropped=N invalid=N other=N type=USER+1
```

**Cadence:** same as ROBOTOS_OBS and ROBOTOS_FAULT (one baseline at init; one periodic).

**Field reference:**

| Field | Type | Meaning | Canonical Definition |
|-------|------|---------|----------------------|
| `attempted` | uint32 | Every cadence-hit post attempt; monotonically increasing | `DEVKIT_PROGRESS.md` — Phase 6M "Log Format" |
| `ok` | uint32 | Post returned `ROBOTOS_CORE_OK` | `DEVKIT_PROGRESS.md` — Phase 6M "Log Format" |
| `throttled` | uint32 | Post returned `ERR_THROTTLED` (always 0 for `post_event`; reserved field) | `DEVKIT_PROGRESS.md` — Phase 6M "Log Format" |
| `dropped` | uint32 | Post returned `ERR_FULL` (queue full) | `DEVKIT_PROGRESS.md` — Phase 6M "Log Format" |
| `invalid` | uint32 | Post returned `ERR_INVALID_ARG` (defensive; should always be 0) | `DEVKIT_PROGRESS.md` — Phase 6M "Log Format" |
| `other` | uint32 | Any other non-OK return (defensive; should always be 0) | `DEVKIT_PROGRESS.md` — Phase 6M "Log Format" |
| `type` | literal | `USER+1` — the producer's event type (`ROBOTOS_EVENT_USER + 1 = 101`) | `DEVKIT_PROGRESS.md` — Phase 6M "Producer Policy" |

**Producer design:**
- Cadence: 1 event every `DEVKIT_TIMER_PRODUCER_TICK_PERIOD` (= 2) devkit ticks
  → ~1 post/s at `DEVKIT_TICK_MS = 500`. Growth rate: +5 per 10 ticks.
- API used: `robotos_core_post_event()` (raw), not `try_post_event`.
  `throttled` is always 0 as a result.
- arg0 = `0x6D00` (marker for runtime tagging).
- Distinct from Phase 6I producer (`ROBOTOS_EVENT_USER` / `0x6900`) — no
  handler-routing conflict.

**Coherence cross-check** (verified by Phase 6Z at ticks=120):
Phase 6I `ok` (17) + Phase 6M `ok` (60) = `ROBOTOS_OBS accepted` (77).

---

## ROBOTOS_BTN — User Button Producer Snapshot (Phase 9A-A)

Emitted by `devkit_button_producer_log_stats()` in `devkit_button_producer.c`.

**Literal log shape** (from `devkit_button_producer.c` LOG_INF format string):

```text
ROBOTOS_BTN attempted=N ok=N full=N invalid=N other=N handled=N type=USER+2
```

**Cadence:** same as ROBOTOS_OBS / FAULT / PROD (one baseline at init; one
periodic per cadence window). Each cadence window now emits a 4-line block:
ROBOTOS_OBS, ROBOTOS_FAULT, ROBOTOS_PROD, ROBOTOS_BTN.

**Field reference:**

| Field | Type | Meaning | Canonical Definition |
|-------|------|---------|----------------------|
| `attempted` | uint32 | Every GPIO ISR firing increments this once (includes mechanical bounce) | `DEVKIT_PROGRESS.md` — Phase 9A-A "Counter / Behavior Analysis" |
| `ok` | uint32 | `robotos_core_post_event()` returned `OK` | `DEVKIT_PROGRESS.md` — Phase 9A-A |
| `full` | uint32 | Returned `ERR_FULL` (queue at capacity, common during bounce) | `DEVKIT_PROGRESS.md` — Phase 9A-A "BOUNCE_OBSERVED" |
| `invalid` | uint32 | Returned `ERR_INVALID_ARG` (defensive; should always be 0) | `DEVKIT_PROGRESS.md` — Phase 9A-A |
| `other` | uint32 | Any other non-OK return (defensive; should always be 0) | `DEVKIT_PROGRESS.md` — Phase 9A-A |
| `handled` | uint32 | Button handler invocations (thread context) | `DEVKIT_PROGRESS.md` — Phase 9A-A |
| `type` | literal | `USER+2` — the producer's event type (`ROBOTOS_EVENT_USER + 2 = 102`) | `DEVKIT_PROGRESS.md` — Phase 9A-A "Chosen Event Type / Marker" |

**Producer design:**

- ISR source: STM32F411E-DISCO user button (PA0, alias `sw0`), edge-to-active.
- API used: `robotos_core_post_event()` (raw, ISR-safe per Phase 5G).
- arg0 = `0x9A0A` (Phase 9A-A marker).
- arg1 = monotonic ISR-firing sequence (advances even on ERR_FULL).
- No software debounce; mechanical bounce produces multiple ISR firings per
  physical press. `attempted - ok = full` is the bounce-rejection count.
- Distinct from Phase 6I (USER) and Phase 6M (USER+1) — three producers
  coexist without routing conflict.

**Per-press handler log** (separate from periodic ROBOTOS_BTN line):

```text
Phase 9A button handled seq=N count=M
```

Emitted once per accepted-and-dispatched button event. Useful for ordering
verification (`seq` may have gaps reflecting ERR_FULL entries; `count` is
strictly monotonic).

---

## Telemetry Stack Summary

```
ROBOTOS_OBS   --- core snapshot getter (robotos_core_snapshot)
                  canonical counter semantics: core/README.md Phase 4J
ROBOTOS_FAULT --- direct SCB register read (0xE000ED28, 0xE000ED2C)
                  Cortex-M only, devkit-local, passive read-only
ROBOTOS_PROD  --- devkit_timer_producer_get_stats()
                  producer module: devkit_timer_producer.c (pure C, host-testable)
ROBOTOS_BTN   --- devkit_button_producer_get_stats()  (Phase 9A-A)
                  producer module: devkit_button_producer.c (Zephyr GPIO/EXTI; devkit-local)
```

All three streams are **passive read-only**. They do not feed back into
scheduling, admission, throttle, dispatch, or retry decisions. This is
structural, not behavioral — the helpers hold no mutable state of their own.

Phase 6Z RTT evidence log:
[devkit/logs/phase_6Z_rtt_2026-05-07.txt](../logs/phase_6Z_rtt_2026-05-07.txt)
