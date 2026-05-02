# RobotOS Core — Phase 4K Scheduler Producer Throttle Policy

This directory contains the portable RobotOS core module.

## Role in the Architecture

The core layer is the **Kernel/Core seed** of RobotOS. It holds no board assumptions,
no Zephyr types in its public API, and no hardware drivers. It is designed to be
portable across any host environment that can provide a C99 toolchain.

**Phase 5A change:** `robotos_core.c` no longer includes `<zephyr/logging/log.h>`
directly. All diagnostic logging is routed through `robotos_platform_log.h` — the
platform logging interface. The Zephyr backend is compiled separately for devkit
builds; host tests compile the no-op stub. The public API remains Zephyr-free.

The devkit harness (`RobotOS_v1.0/devkit/`) is the integration host. It:

- calls `robotos_core_init()` once at boot
- calls `robotos_core_tick()` every `DEVKIT_TICK_MS` (500 ms)
- owns all timing, LED, RTT, and board-specific logic
- consumes the core API through the portable public header only

## Lifecycle States

```text
UNINITIALIZED  ──── robotos_core_init() ────>  READY
                                                  │
                                              (tick loop)
                                                  │
                                              [ERROR reserved for future fault conditions]
```

| State | Value | Meaning |
| ----- | ----- | ------- |
| `ROBOTOS_CORE_STATE_UNINITIALIZED` | 0 | Power-on default. `tick()` returns `ERR_INVALID_STATE`. |
| `ROBOTOS_CORE_STATE_READY` | 1 | Core is initialized and accepting ticks. |
| `ROBOTOS_CORE_STATE_ERROR` | 2 | Reserved. Not yet triggered by any code path. |

## API Contract

### Status Codes

```c
typedef enum {
    ROBOTOS_CORE_OK                = 0,   // success
    ROBOTOS_CORE_ERR_INVALID_STATE = -1,  // wrong lifecycle state
    ROBOTOS_CORE_ERR_NULL          = -2,  // NULL pointer argument
} robotos_core_status_t;
```

### Functions

| Function | Contract |
| -------- | -------- |
| `robotos_core_version()` | Returns constant version string. Never NULL. |
| `robotos_core_init()` | Idempotent. First call: state→READY, tick_count=0, init_count=1. Repeat: init_count++, tick_count unchanged, returns OK. |
| `robotos_core_tick()` | Requires state==READY. Increments tick_count. Logs tick 1 and every 10th. Returns ERR_INVALID_STATE if not READY; warns once, then silent. |
| `robotos_core_state()` | Returns current lifecycle state enum. |
| `robotos_core_tick_count()` | Returns current tick_count. Returns 0 if not initialized. |
| `robotos_core_snapshot()` | Copies {state, tick_count, init_count, version} into caller struct. Returns ERR_NULL if out is NULL. |

### Init Idempotency Detail

```text
First call:   state = READY, tick_count = 0, init_count = 1  → LOG_INF (full init message)
Second call:  init_count = 2, tick_count unchanged           → LOG_DBG (suppressed at INF level)
```

`tick_count` is NOT reset on repeated init calls. This is intentional: a re-init
during a running tick loop must not silently discard progress.

### Tick-Before-Init Behavior

If `robotos_core_tick()` is called before `robotos_core_init()`:

- Returns `ROBOTOS_CORE_ERR_INVALID_STATE`
- Emits one `LOG_WRN` ("core tick before init — ignored")
- All subsequent invalid-state ticks are silent (no repeated warnings)
- `tick_count` is not incremented

## Snapshot

`robotos_core_snapshot_t` is a plain struct — no locking, no atomics.

**Thread-safety limitation:** This is valid only in single-threaded contexts.
When RTOS threads are introduced in a future phase, the snapshot function will
need a mutex or atomic snapshot mechanism.

## Phase 4B Scope

**Implemented:**

- Status enum (`robotos_core_status_t`)
- Lifecycle state enum (`robotos_core_state_t`)
- Snapshot struct (`robotos_core_snapshot_t`)
- `robotos_core_state()` introspection
- `robotos_core_tick_count()` introspection
- `robotos_core_snapshot()` introspection
- Init idempotency semantics
- Tick-before-init guard with one-shot warning

**Intentionally absent in Phase 4B:**

- Scheduler
- Event bus / message queue
- RTOS threads
- Peripheral drivers
- Dynamic memory allocation
- Host/unit test layer (planned for a future phase)
- Mutex / atomic protection (single-threaded assumption documented)

## Files

| File | Role |
| ---- | ---- |
| `robotos_core.h` | Public portable API — no Zephyr or board types |
| `robotos_core.c` | Implementation — logs through `robotos_platform_log.h`; no direct Zephyr dependency |

## Host Contract Tests (Phase 4C)

Core contract semantics are validated without Zephyr, hardware, or `west build` via
a host test binary. This is **Tier 1 validation**: the fastest feedback loop.

**Phase 5A:** Host tests compile `robotos_platform_log_host_stub.c` (no-op backend)
alongside core. `ROBOTOS_CORE_HOST_TEST` is no longer needed and has been removed from
the host test build — the platform log boundary handles backend selection instead.

### How to Run

```bash
# From repo root (requires a host C compiler; verified on Linux/WSL)
cmake -S RobotOS_v1.0/tests/host -B build-host-core
cmake --build build-host-core
ctest --test-dir build-host-core --output-on-failure
```

Expected output:

```text
100% tests passed, 0 tests failed out of 1
```

### Host Test Shim

`robotos_core.c` detects host test mode via `ROBOTOS_CORE_HOST_TEST=1` compile
definition (set by `tests/host/CMakeLists.txt`). In that mode:

- `CORE_LOG_INF/WRN/DBG` macros map to `((void)0)` — no Zephyr headers needed
- `<stdbool.h>` and `<stddef.h>` are included directly (normally provided transitively by Zephyr)
- All contract logic runs identically to firmware mode

The shim is local to `robotos_core.c`. `robotos_core.h` has no Zephyr types.

### Contract Cases Covered (35 tests)

- `version()` non-NULL and equals expected string
- Initial state `UNINITIALIZED`, tick_count `0`
- `snapshot(NULL)` returns `ERR_NULL`
- `tick()` before init returns `ERR_INVALID_STATE`, no tick_count increment
- `snapshot()` before init returns state=UNINITIALIZED, tick_count=0
- First `init()` → state=READY, tick_count=0, init_count=1
- Second `init()` → idempotent, init_count=2, tick_count **not** reset
- `tick()` after init increments monotonically
- Final `snapshot()` reports READY, correct tick_count=16, init_count=2, version

### Limitations (Phase 4C)

- Tests run in a single sequential process — no reset API needed; tests order-dependent by design
- No concurrency/thread-safety validation yet
- Logging behavior (one-shot WRN for tick-before-init) not tested; private static not exposed
- Zephyr integration still validated through devkit (Phase 3B–4B hardware evidence)
- Windows native host build blocked: broken MinGW installation on dev machine; WSL Ubuntu used as workaround

## Limitations

- **Single-threaded assumption**: No mutex or atomic operations. Safe only in single-threaded runtime. Revisit when RTOS threads are introduced.
- **Platform logging boundary**: `robotos_core.c` logs through `robotos_platform_log.h`. No direct Zephyr dependency in core.
- **No scheduler or event bus**: Phase 4B/4C scope is lifecycle contract only.

## Next Phase

Candidates for team decision:

- **Phase 4K** — Scheduler Producer Throttle Policy
- **Phase 5D** — Platform Critical Section Boundary
- **Phase 6A** — Devkit Event Smoke Integration

Do not implement without explicit assignment.

---

## Scheduler Budget / Backpressure Policy (Phase 4J)

### Dispatch Budget

Each call to `robotos_core_tick()` dispatches at most
`ROBOTOS_CORE_MAX_EVENTS_PER_TICK` events from the internal queue.
The budget is a compile-time constant (currently 1).

```c
uint32_t robotos_core_dispatch_budget_per_tick(void);
/* Returns ROBOTOS_CORE_MAX_EVENTS_PER_TICK. Does not change at runtime. */
```

### Backpressure Rule

Backpressure is active when the pending backlog exceeds the per-tick budget
OR the queue is full:

```
backpressure_active = (pending_event_count > ROBOTOS_CORE_MAX_EVENTS_PER_TICK)
                      || queue_is_full
```

```c
bool robotos_core_backpressure_active(void);
/* Safe to call before init (returns false when queue uninitialized/empty). */
```

Example with budget=1, capacity=16:

| Pending | Full? | backpressure_active |
|---------|-------|---------------------|
| 0       | no    | false               |
| 1       | no    | false (== budget)   |
| 2       | no    | true  (> budget)    |
| 16      | yes   | true                |

### Counter Distinction

| Counter | Incremented by | Notes |
|---------|---------------|-------|
| `pending_event_count` | successful `post_event()` | decremented by dispatch |
| `dropped_event_count` | queue full on push | ERR_FULL path; not admission reject |
| `admission_accepted_count` | type passed gate AND queue push succeeded | |
| `admission_rejected_count` | type rejected by gate (NONE / reserved 2–99) | not queue-full |
| `dispatched_event_count` | event popped and delivered to routing handler | |
| `unhandled_event_count` | dispatched event with no registered handler | still consumed, returns OK |
| `handler_error_count` | registered handler returned non-OK | event still consumed |

**Key distinctions:**
- Queue-full (`ERR_FULL`) increments `dropped_count`, NOT `admission_rejected_count`.
- Invalid type (`ERR_INVALID_ARG`) increments `admission_rejected_count`, NOT `dropped_count`.
- A handler error increments `handler_error_count`; the event is still consumed (pending decreases).

### Backpressure Is Observability Only (Phase 4J)

In Phase 4J, `backpressure_active` is a read-only signal. It does NOT:

- throttle producers (`post_event()` behavior is unchanged)
- change event priority (strict FIFO preserved)
- alter scheduler fairness
- affect admission gate decisions
- affect queue-full error returns

Full queue still returns `ERR_FULL` + increments `dropped_count`.
Invalid events still return `ERR_INVALID_ARG` + increment `admission_rejected_count`.
Neither condition is altered by backpressure state.

### Snapshot

`robotos_core_snapshot_t` includes `bool backpressure_active` populated by
`robotos_core_snapshot()`. This field equals `robotos_core_backpressure_active()`
at the time of the call.

### Phase 4J Limitations

- **Observability only.** No producer throttle in `post_event`.
- **No dynamic budget.** Budget is a compile-time constant.
- **No priority or fairness.** Strict FIFO from queue.
- **No concurrency/ISR safety.** Single-threaded only.
- **Platform critical section added in Phase 5E** — see Phase 5E section below.
- **No timer/deadline integration.** Time-based scheduling is out of scope.

---

## Phase 4K — Scheduler Producer Throttle Policy

Phase 4K adds a producer-side throttle as a new opt-in API while preserving
all existing `robotos_core_post_event()` semantics unchanged.

Phase 6C (devkit) proves `post_event` queue-full/drop semantics.
Phase 4K adds `try_post_event` as the throttle-aware path for producers that
should self-limit before the queue fills.

### New API: `robotos_core_try_post_event()`

```c
robotos_core_status_t robotos_core_try_post_event(const robotos_event_t *event);
```

Same admission and queue semantics as `post_event`, plus a throttle check:

**Throttle rule:**
- After admission passes, if queue is **not full** and
  `pending_event_count > ROBOTOS_CORE_MAX_EVENTS_PER_TICK`:
  - Returns `ROBOTOS_CORE_ERR_THROTTLED`
  - Increments `producer_throttled_count`
  - Does **not** push into queue
  - Does **not** increment `admission_accepted_count`
  - Does **not** increment `admission_rejected_count`
  - Does **not** increment `dropped_count`

**Full queue behavior in `try_post_event`:**
- If queue **is** full, throttle does not apply — push proceeds and returns
  `ROBOTOS_CORE_ERR_FULL` + `dropped_count++` (same as `post_event`).
- This preserves the full/drop distinction proven in Phase 6C.

### Three Distinct Ingestion Outcomes

| Condition | Return | Counter incremented |
|-----------|--------|---------------------|
| Invalid type | `ERR_INVALID_ARG` | `admission_rejected_count` |
| Valid, queue full | `ERR_FULL` | `dropped_count` |
| Valid, queue not full, pending > budget (try_post only) | `ERR_THROTTLED` | `producer_throttled_count` |
| Valid, queue not full, no throttle | `OK` | `admission_accepted_count` |

### New Status Code

```c
ROBOTOS_CORE_ERR_THROTTLED = -7
```

### New Getters

```c
bool     robotos_core_producer_throttle_active(void);
uint32_t robotos_core_producer_throttled_count(void);
```

`producer_throttle_active` is true when `pending > budget AND queue NOT full`.
This is a strict subset of `backpressure_active`.

### Snapshot Extension

`robotos_core_snapshot_t` gains:
- `bool     producer_throttle_active`
- `uint32_t producer_throttled_count`

### Repeated Init Behavior

`producer_throttled_count` is **not reset** by repeated `robotos_core_init()`,
consistent with `admission_accepted_count` and `admission_rejected_count`.

### Phase 4K Limitations

- **No automatic retry.** Throttled producers must handle `ERR_THROTTLED` explicitly.
- **No priority or fairness.** All producers are throttled equally.
- **No dynamic budget.** Throttle threshold is `ROBOTOS_CORE_MAX_EVENTS_PER_TICK`.
- **No producer registry.** No per-producer accounting.
- **No concurrency/ISR safety.** Single-threaded only in Phase 4K.
- **No hardware runtime required.** Policy proven by host tests only.
- **post_event unchanged.** Raw ingestion path still allows queue-full/drop for
  producers that do not need throttle protection.

---

## Phase 5G — ISR-Safe Producer Contract Audit

Phase 5G is an **audit/doc-only** phase. No source code was changed.
The objective is to inspect the actual lock boundaries established in
Phase 5D/5E/5F and publish a precise ISR-safety contract for the event
producer path.

### Audit Verdict: CLOSED_AUDIT_CONFIRMED

The event producer APIs (`post_event` and `try_post_event`) are confirmed
ISR-safe on the Zephyr/ARMv7-M backend under the conditions documented below.
No blocking, logging, handler invocation, or unbounded loop occurs inside the
critical section on the producer path.

### API Classification Table

| API | Critical section | Calls handler | Calls logging | May block/sleep | ISR-safe producer |
|-----|-----------------|---------------|---------------|-----------------|-------------------|
| `robotos_core_post_event()` | YES — one short section | NO | NO | NO | **YES (conditional)** |
| `robotos_core_try_post_event()` | YES — one short section | NO | NO | NO | **YES (conditional)** |
| `robotos_core_tick()` | YES — state+count only | YES (outside lock) | YES (outside lock) | NO | **NO** — thread-context only |
| `robotos_core_dispatch_events()` | YES — state check only | YES (outside lock) | NO | NO | **NO** — thread-context only |
| `robotos_core_register_event_handler()` | YES — table mutation | NO | NO | NO | **NO** — thread-context only; semantically wrong from ISR |
| `robotos_core_unregister_event_handler()` | YES — table mutation | NO | NO | NO | **NO** — thread-context only |
| `robotos_core_has_event_handler()` | YES — brief read | NO | NO | NO | Not claimed; thread-context only |
| `robotos_core_snapshot()` | YES — all reads | NO | NO | NO | Not claimed; thread-context only |
| `robotos_core_*_count()` getters | YES — brief read per call | NO | NO | NO | Not claimed; thread-context only |
| `robotos_platform_logf()` | NO | NO | — | Backend-specific | **NO** — explicitly not ISR-safe (Phase 5A) |
| `robotos_platform_sleep_ms()` | NO | NO | NO | YES — by design | **NO** — blocks; forbidden in ISR |
| `robotos_platform_uptime_ms()` | NO | NO | NO | NO | Not claimed (Phase 5B) |
| `robotos_platform_fault_report()` | NO | NO | — | Backend-specific | **NO** — not claimed (Phase 5C) |
| `robotos_platform_assert()` | NO | NO | — | Backend-specific | **NO** — not claimed (Phase 5C) |
| `robotos_platform_critical_enter/exit()` | — | NO | NO | NO | YES — irq_lock/irq_unlock (ARMv7-M) |

### ISR-Safe Producer Contract

The following APIs may be called from an ISR-like producer context (e.g. a
timer callback or hardware interrupt handler) **subject to all conditions below**:

- `robotos_core_post_event(const robotos_event_t *event)`
- `robotos_core_try_post_event(const robotos_event_t *event)`

#### Conditions

1. **Zephyr/ARMv7-M backend only.** The critical section uses `irq_lock()`/
   `irq_unlock()` (BASEPRI masking). Safe for all maskable interrupts on
   Cortex-M4. Not valid from non-maskable (NMI, HardFault at priority 0).
2. **Event struct must be fully initialized before the call** and accessible
   for the entire duration. The core copies the event into the queue under lock
   (`q->buffer[tail] = *event`); the caller's copy is not referenced after return.
3. **No handler is invoked during post.** The producer path never calls a
   registered handler. Handler invocation occurs only in `tick()` or
   `dispatch_events()` from thread context.
4. **No logging under lock.** `robotos_platform_logf` is never called inside
   the critical section on the producer path.
5. **No sleep/yield under lock.** The critical section is O(1), deterministic:
   NULL check → state check → admission check → optional throttle check →
   queue push (struct copy) → counter increment. No dynamic allocation.
6. **Caller must handle all error returns:**
   - `ROBOTOS_CORE_ERR_NULL` — event pointer is NULL
   - `ROBOTOS_CORE_ERR_INVALID_STATE` — core not yet initialized
   - `ROBOTOS_CORE_ERR_INVALID_ARG` — event type rejected by admission gate
   - `ROBOTOS_CORE_ERR_FULL` — queue at capacity; event dropped
   - `ROBOTOS_CORE_ERR_THROTTLED` — (try_post only) backlog > dispatch budget

#### Not ISR-safe (must not be called from ISR)

- `robotos_core_tick()` — calls handler and logging from thread context
- `robotos_core_dispatch_events()` — calls handler from thread context
- Handler register/unregister — table mutation; semantically wrong from ISR
- All getters and snapshot — no ISR claim; may introduce reordering hazard
- `robotos_platform_logf()`, `sleep_ms()`, `fault_report()`, `assert()` —
  explicitly documented as not ISR-safe in their respective platform phases

#### Thread-context only

- Tick/dispatch loop (`tick()`, `dispatch_events()`)
- Handler registration and unregistration
- Devkit runtime loop
- Snapshot and diagnostic getters
- All platform log/fault/sleep/time APIs

#### Handler callback context

Handlers are invoked from `tick()` or `dispatch_events()` in **thread context
only**, never from ISR context. Handlers must not assume they are called from an
ISR. Handler context pointer lifetime is the caller's responsibility; if an ISR
provides a stack-local context to a registered handler, the context may be
invalid by the time the handler runs in thread context.

### Lock Boundary Audit Findings

Confirmed from direct code inspection of `robotos_core.c` Phase 5F:

| Finding | Result |
|---------|--------|
| Handler called under critical section? | **NO** — Step 3 always outside lock |
| `robotos_platform_logf` called under critical section? | **NO** — all `CORE_LOG_*` outside lock |
| `sleep`/`yield` under critical section? | **NO** |
| Unbounded loop under critical section? | **NO** — handler table loop bounded by `ROBOTOS_CORE_MAX_EVENT_HANDLERS` (compile-time constant) |
| `post_event_internal` critical section O(1)? | **YES** — NULL check, state read, admission, queue push (struct copy), counter increment |
| Queue push (`robotos_event_queue_push`) safe in critical section? | **YES** — O(1), no allocation, no logging, no handler, struct copy only |
| Dispatcher counters written under lock? | **NO** — written after handler returns in Step 4 (single-threaded assumption) |
| s_unhandled_event_count updated under lock? | **YES** — Step 2, bounded table scan, safe |

### Remaining Limitations

- **No ISR producer runtime stress.** No timer ISR or EXTI ISR posting events
  has been tested on hardware. Contract is based on code audit and Zephyr
  `irq_lock/irq_unlock` semantics on ARMv7-M.
- **No latency budget measured.** Critical section duration is O(1) and
  bounded but no cycle-count measurement has been performed.
- **No multi-producer stress.** Only single concurrent ISR producer assumed.
- **No custom board validation.** STM32F407VET6 and other boards remain
  hardware-unvalidated.
- **Handler context lifetime.** If ISR-produced events reference ISR-local
  context via the registered handler, the context may be stale when the
  handler runs in thread context. Caller owns context lifetime.
- **Dispatcher counters not ISR-protected.** `dispatched_count` and
  `handler_error_count` are written from thread context (dispatch path) without
  a lock. An ISR concurrently reading these counters could observe torn reads.
  Do not read dispatch counters from ISR context.

### Next Validation Recommendation

**Phase 6G — ISR/Timer Producer Smoke:** Wire a Zephyr timer callback to call
`robotos_core_try_post_event()` and verify on hardware that the handler receives
events, no fault occurs, and tick count and dispatched count remain consistent.

---

## Phase 5F — Dispatcher Pop / Handler Split

Phase 5F refactors `robotos_core.c` to explicitly split the dispatch path
into three distinct regions so that no critical section is ever held when a
registered handler executes, and the separation is structural (not relying on
the dispatcher's internal behavior).

### Dispatch Path (s_core_dispatch_one_safe)

```
Step 1: [lock] queue pop into local ev   [unlock]
Step 2: [lock] handler table lookup →    [unlock]
             copy fn + ctx to locals
Step 3: invoke handler_fn(&ev, ctx)      // depth MUST be 0 here
Step 4: update dispatcher counters       // no lock needed (same TU)
```

### What Changed

| Item | Phase 5E | Phase 5F |
|------|----------|----------|
| `s_core_routing_handler` | present (registered with dispatcher) | **removed** |
| `s_core_noop_handler` | absent | **added** (init-only, never dispatched) |
| `s_core_dispatch_one_safe` | absent | **added** (core dispatch primitive) |
| `tick()` dispatch path | called `dispatcher_dispatch_all` | loops `s_core_dispatch_one_safe` |
| `dispatch_events()` dispatch path | called `dispatcher_dispatch_all` | loops `s_core_dispatch_one_safe` |
| Dispatcher counters | updated by dispatcher | updated directly by `s_core_dispatch_one_safe` |

### What is Protected

| API | Protected state |
|-----|----------------|
| `post_event_internal()` | state check, admission, throttle check, queue push, all counters |
| `robotos_core_init()` | core_state, core_tick_count, core_init_count, handler table clear |
| `robotos_core_tick()` | state only and tick_count increment only |
| `robotos_core_dispatch_events()` | state check only |
| `robotos_core_snapshot()` | all counter/state reads as one coherent snapshot |
| All individual getters | brief lock around each read |
| Handler registration/unregistration/has/count | table search and mutation |
| `s_core_dispatch_one_safe` Step 1 | queue pop |
| `s_core_dispatch_one_safe` Step 2 | handler table lookup, copy fn+ctx to locals |

### What is NOT Protected

| Item | Reason |
|------|--------|
| Registered handler callback (Step 3) | Hard rule — no lock held during invocation |
| `robotos_platform_logf` / `CORE_LOG_*` | Must run outside lock |
| Dispatcher counter update (Step 4) | Same translation unit, single-threaded devkit assumption |

### Critical Rule

**No critical section is held while a registered event handler callback executes.**

Proven by Phase 5F host test (TC01, TC02, TC13):
- `tick()` path: handler sees `robotos_platform_critical_host_current_depth() == 0`
- `dispatch_events()` path: same
- 5× stress loop: `enter_count == exit_count` at all points

### Known Limitation

If a handler is **unregistered** between Step 2 (local copy) and Step 3
(invocation), the handler will execute once with the stale copy. This is
acceptable under the current single-threaded devkit assumption. A production
multi-threaded build would need a reference-count or epoch-based invalidation
mechanism.

### Semantics Unchanged

All existing public API return values, counters, and behavior are identical
to Phase 5E. `dispatched_count` is incremented for every event consumed from
the queue regardless of whether a handler was registered (matching pre-5F
semantics).

---

## Phase 5E — Apply Critical Boundary to Core Queue State

Phase 5E wires `robotos_platform_critical_enter`/`exit` into selected short
core state transitions. This is the first step toward a concurrent-safe core;
it does NOT claim full thread-safety or ISR-safe posting.

### What is Protected

Short state transitions held under a single critical section per operation:

| API | Protected state |
|-----|----------------|
| `post_event_internal()` | state check, admission, throttle check, queue push, all counters |
| `robotos_core_init()` | core_state, core_tick_count, core_init_count, handler table clear |
| `robotos_core_tick()` | state only and tick_count increment only |
| `robotos_core_dispatch_events()` | state check only |
| `robotos_core_snapshot()` | all counter/state reads as one coherent snapshot |
| All individual getters | brief lock around each read |
| Handler registration/unregistration/has/count | table search and mutation |

### What is NOT Protected

| Item | Reason |
|------|--------|
| Registered handler callback | Must run outside lock — Phase 5E hard rule |
| `robotos_platform_logf` / `CORE_LOG_*` | Must run outside lock |
| `robotos_event_dispatcher_dispatch_all` | Calls handler internally; cannot lock whole dispatch |
| Dispatcher counters (dispatched_count, handler_error_count) | Updated by dispatcher outside lock — Phase 5F scope |
| Routing handler's handler table read | Lock not held during dispatch — Phase 5F scope |

### Critical Rule

**No critical section is held while a registered event handler callback executes.**

Proven by Phase 5E host test TC10: handler observes
`robotos_platform_critical_host_current_depth() == 0` during invocation.

### Current Limitation

This is NOT a full thread-safe or ISR-safe guarantee. The dispatcher pops
from the queue and calls the handler in the same unlocked region. A producer
posting concurrently with a tick dispatch could still observe a race on queue
state and dispatcher counters. Phase 5F (Dispatcher Pop/Handler Split) is
the appropriate follow-on to address this.

### Semantics Unchanged

All existing public API return values, counters, and behavior are identical
to pre-5E. Phase 5E is a structural improvement only.

---

## Phase 4L — Scheduler Retry/Backoff Policy Stub

Phase 4L adds a scheduler retry/backoff policy stub as an opt-in API.
This is a **host-only stub phase** — retry loop is tight (no actual backoff delay)
and policy is proven by host tests alone. No devkit runtime is required.

### New API: Retry Policy Configuration

\`\`\`c
robotos_core_status_t robotos_core_set_retry_policy(
    bool    retry_enabled,
    bool    retry_on_full_enabled,
    uint32_t max_retry_attempts,
    uint32_t backoff_delay_ms
);
\`\`\`

Configures retry behavior for \`robotos_core_post_event_with_retry()\`:

| Parameter | Meaning | Default |
|-----------|---------|----------|
| \`retry_enabled\` | If true, retry on ERR_THROTTLED. If false, \`retry_on_full_enabled\` is also ignored. | \`false\` |
| \`retry_on_full_enabled\` | If true AND \`\retry_enabled\` is true, also retry on ERR_FULL (queue-full). Must be enabled explicitly. | \`false\` |
| \`max_retry_attempts\` | Maximum retry attempts per event. 0 = immediate one-shot attempt (no retry). | \`ROBOTOS_CORE_DEFAULT_RETRY_ATTEMPTS\` (3) |
| \`backoff_delay_ms\` | Backoff delay between attempts in milliseconds. Stub: not actually honored. | \`ROBOTOS_CORE_DEFAULT_BACKOFF_DELAY_MS\` (10) |

### New API: Post with Retry

\`\`\`c
robotos_core_status_t robotos_core_post_event_with_retry(const robotos_event_t *event);
\`\`\`

Behavior (Phase 4L stub):

1. Attempts \`robotos_core_post_event(event)\` once.
2. If retry policy is not enabled, returns result immediately.
3. If result is ERR_THROTTLED (or ERR_FULL if \`retry_on_full_enabled\`):
   - And retry attempts remain: increment \`retry_attempt_count\` and retry immediately.
   - Stub: no actual backoff delay (tight retry loop).
4. If max retry attempts exhausted:
   - Increment \`retry_exhausted_count\`
   - Return \`ERR_RETRY_EXHAUSTED\`
5. If retry eventually succeeds:
   - Increment \`retry_success_count\`
   - Return \`OK\`

### New Status Code

\`\`\`c
ROBOTOS_CORE_ERR_RETRY_EXHAUSTED = -8
\`\`\`

### New Counters

| Counter | Incremented by | Meaning |
|---------|---------------|---------|
| \`retry_attempt_count\` | Every retry loop iteration, regardless of outcome |
| \`retry_success_count\` | Retries that eventually returned OK (not immediate one-shot) |
| \`retry_exhausted_count\` | Retries that exhausted allowed attempts (returned ERR_RETRY_EXHAUSTED) |

### Counter Reset Behavior

Retry counters are **not reset** by repeated \`robotos_core_init()\`, consistent with
\`producer_throttled_count\`, \`admission_accepted_count\` and \`admission_rejected_count\`.

### Snapshot Extension

\`robotos_core_snapshot_t\` gains:
- \`uint32_t retry_attempt_count\`
- \`uint32_t retry_success_count\`
- \`uint32_t retry_exhausted_count\`

### Phase 4L Limitations

- **Stub implementation only.** No actual backoff delay is honored. All retries happen
  immediately without yielding.
- **No retry on ERR_FULL by default.** \`retry_on_full_enabled\` must be explicitly
  enabled to retry on queue-full conditions.
- **No exponential backoff.** Linear delay only (stub: no delay at all).
- **No priority or fairness.** All retries are handled equally.
- **No per-producer accounting.** No producer registry or per-producer counters.
- **No concurrency/ISR safety.** Single-threaded only in Phase 4L.
- **No hardware runtime required.** Policy proven by host tests only.
- **post_event/try_post_event unchanged.** Raw ingestion paths bypass retry entirely.

---

