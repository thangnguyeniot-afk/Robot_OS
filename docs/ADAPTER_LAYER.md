# RobotOS Inspire — Robot Adapter Layer Design

> **Version:** v0.1-alpha  
> **Last Updated:** 2026-03-03  
> **Status:** 🚧 Under Active Development  
> **Layer:** Robot Adapter (100% portable — hides all Zephyr from above)

---

## 📋 Table of Contents

1. [Layer Role & Boundary](#layer-role--boundary)
2. [Design Philosophy](#design-philosophy)
3. [Header Naming Convention](#header-naming-convention)
4. [Threading API (`ro_thread`)](#threading-api-ro_thread)
5. [Timing API (`ro_time`)](#timing-api-ro_time)
6. [Hardware Timer API (`ro_timer`)](#hardware-timer-api-ro_timer)
7. [Queue IPC API (`ro_queue`)](#queue-ipc-api-ro_queue)
8. [Memory Pool API (`ro_pool`)](#memory-pool-api-ro_pool)
9. [Mutex API (`ro_mutex`)](#mutex-api-ro_mutex)
10. [Semaphore API (`ro_sem`)](#semaphore-api-ro_sem)
11. [GPIO API (`ro_gpio`)](#gpio-api-ro_gpio)
12. [PWM API (`ro_pwm`)](#pwm-api-ro_pwm)
13. [I2C / SPI API (`ro_i2c`, `ro_spi`)](#i2c--spi-api-ro_i2c-ro_spi)
14. [Logging API (`ro_log`)](#logging-api-ro_log)
15. [Tracing API (`ro_trace`)](#tracing-api-ro_trace)
16. [Deadline Monitoring API (`ro_deadline`)](#deadline-monitoring-api-ro_deadline)
17. [Error & Status Codes (`ro_status`)](#error--status-codes-ro_status)
18. [CI Enforcement Rules](#ci-enforcement-rules)
19. [Porting Guide](#porting-guide)

---

## Layer Role & Boundary

### Position in the Stack

```
Application Layer          — uses Adapter APIs directly (ro_thread, ro_queue, ...)
      ↓
Robot Framework Layer      — uses Adapter APIs for all RTOS operations
      ↓
┌──────────────────────────────────────────────────────────────────┐
│  ROBOT ADAPTER LAYER  (this document)                           │
│                                                                  │
│  Role: Portable RTOS/HW abstraction.                            │
│  Answers: "How do I run threads, measure time, send messages,   │
│           allocate memory, read GPIO, control PWM, use I2C,     │
│           trace events — in a backend-agnostic way?"            │
│                                                                  │
│  Portability: 100%.  Swap Zephyr for FreeRTOS or bare-metal     │
│  by replacing src/adapter/zephyr/*.c without touching any       │
│  Framework or Application code.                                  │
└──────────────────────────────────────────────────────────────────┘
      ↓
Zephyr Kernel Layer        — Adapter impl files consume Zephyr; no one else does
```

### What the Adapter Layer Does

| Capability | Module | Public Header |
|-----------|--------|--------------|
| Threads, priorities, stack | `ro_thread.c` | `ro_thread.h` |
| Monotonic clock, delays | `ro_time.c` | `ro_time.h` |
| Fixed-size message queues | `ro_queue.c` | `ro_queue.h` |
| Fixed-block memory pools | `ro_pool.c` | `ro_pool.h` |
| Priority-inheriting mutex | `ro_mutex.c` | `ro_mutex.h` |
| Binary/counting semaphore | `ro_sem.c` | `ro_sem.h` |
| GPIO read/write/interrupt | `ro_gpio.c` | `ro_gpio.h` |
| PWM channel control | `ro_pwm.c` | `ro_pwm.h` |
| I2C bus transactions | `ro_i2c.c` | `ro_i2c.h` |
| SPI bus transactions | `ro_spi.c` | `ro_spi.h` |
| Structured logging | `ro_log.c` | `ro_log.h` |
| Trace event emission | `ro_trace.c` | `ro_trace.h` |
| Deadline window checking | `ro_deadline.c` | `ro_deadline.h` |
| Error/status codes | — | `ro_status.h` |

### What the Adapter Layer Does NOT Do

- ❌ No robotics semantics (no "stepper", no "PID", no "homing")
- ❌ No device policies (acceleration profiles, anti-windup, etc.)
- ❌ Does not create threads on its own (only provides `ro_thread_create`)
- ❌ Does not expose `<zephyr/*>` in any public header

---

## Design Philosophy

### Core Tenets

```
✅ Small, stable, minimal surface area
✅ Hides ALL Zephyr internals from callers
✅ One ro_* function → one RTOS concept
✅ Zero heap after robotos_init()
✅ ISR-safe variants where needed (_isr suffix)
✅ Portable: replacing src/adapter/zephyr/*.c = full backend swap
```

### Usage Contract

```c
// ❌ WRONG: direct Zephyr usage in application or framework
#include <zephyr/kernel.h>
struct k_thread my_thread;
k_thread_create(&my_thread, stack, stack_size, entry, arg, NULL, NULL, -10, 0, K_NO_WAIT);

// ✅ RIGHT: use Robot Adapter API
#include <robotos/ro_thread.h>
static uint8_t my_stack[2048];
ro_thread_config_t cfg = {
    .name       = "my_thread",
    .stack      = my_stack,
    .stack_size = sizeof(my_stack),
    .priority   = RO_PRIO_RT_CONTROL,
    .entry      = my_entry,
    .arg        = NULL,
};
ro_thread_t* t = ro_thread_create(&cfg);
```

---

## Header Naming Convention

All Adapter public headers use the `ro_` prefix. This signals:
- The header wraps exactly one RTOS/HW primitive
- It is part of the Adapter layer (not Framework)
- The corresponding implementation lives in `src/adapter/zephyr/ro_*.c`

| Header | Layer | Prefix | Rationale |
|--------|-------|--------|-----------|
| `ro_thread.h` | Adapter | `ro_` | Wraps `k_thread` (one RTOS concept) |
| `ro_time.h` | Adapter | `ro_` | Wraps `k_uptime_get` / cycle counter |
| `ro_queue.h` | Adapter | `ro_` | Wraps `k_msgq` |
| `ro_pool.h` | Adapter | `ro_` | Wraps `k_mem_slab` |
| `ro_mutex.h` | Adapter | `ro_` | Wraps `k_mutex` |
| `ro_sem.h` | Adapter | `ro_` | Wraps `k_sem` (binary + counting) |
| `ro_gpio.h` | Adapter | `ro_` | Wraps Zephyr GPIO API |
| `ro_pwm.h` | Adapter | `ro_` | Wraps Zephyr PWM API |
| `ro_i2c.h` | Adapter | `ro_` | Wraps Zephyr I2C API |
| `ro_spi.h` | Adapter | `ro_` | Wraps Zephyr SPI API |
| `ro_log.h` | Adapter | `ro_` | Wraps Zephyr LOG subsystem |
| `ro_trace.h` | Adapter | `ro_` | Wraps `SYS_PORT_TRACING_*` |
| `ro_deadline.h` | Adapter | `ro_` | Uses `ro_time_us()` internally |
| `ro_status.h` | Adapter | `ro_` | Defines `ro_status_t` + error codes |
| `stepper.h` | Framework | _(none)_ | Domain-semantic name; NOT an RTOS concept |
| `pid.h` | Framework | _(none)_ | Domain-semantic name |
| `servo.h` | Framework | _(none)_ | Domain-semantic name |

> **Rule:** `#include <robotos/ro_pid.h>` is always wrong.  
> `pid.h` is a Framework header. Applying `ro_` blurs layer boundaries.

---

## Threading API (`ro_thread`)

### Priority System

```c
// ro_thread.h

// ---------------------------------------------------------------------------
// ro_priority_t — opaque. NEVER construct directly.
// Use RO_PRIO_* macros only. Assigning a raw int is a C compile error
// (int is not implicitly convertible to a struct compound literal).
typedef struct { int16_t v; } ro_priority_t;

// ── COOPERATIVE tier (preset.v < 0) ────────────────────────────────────────
// Thread is NEVER preempted. MUST call ro_thread_yield() each loop iteration.
//
// CONTRACT — MUST:
//   ✓ Call ro_thread_yield() or ro_thread_sleep_ms() each iteration
//   ✓ Keep worst-case runtime per yield bounded (document the bound in µs)
//   ✓ Use only ISR-safe IPC: ro_queue_send_isr, ro_pool_alloc_isr, atomics
//
// CONTRACT — MUST NOT:
//   ✗ Acquire a mutex
//   ✗ Call any blocking Zephyr kernel API directly
//   ✗ Enter unbounded loops without yielding

// Software pulse/step generation (prefer HW timer when available).
// Jitter budget: <2 µs/cycle.  ro_thread_sleep_ms() FORBIDDEN.
// Requires CONFIG_RO_ALLOW_SW_PULSE_THREAD=y in STD profile.
#define RO_PRIO_RT_PULSE    ((ro_priority_t){ .v = -16 })

// Hard-RT control loops (PID ≥1 kHz, motion planner).
// Bounded runtime ≤200 µs/iter. ro_thread_sleep_ms() ALLOWED for loop rate.
#define RO_PRIO_RT_CONTROL  ((ro_priority_t){ .v = -10 })

// Safety watchdog / fault detector. Bounded runtime ≤50 µs/iter.
// ro_thread_sleep_ms() ALLOWED for heartbeat interval.
#define RO_PRIO_RT_MONITOR  ((ro_priority_t){ .v =  -2 })

// ── PREEMPTIVE tier (preset.v ≥ 0) ─────────────────────────────────────────
// Thread MAY be preempted. May acquire mutexes. May block on queues.
//
// CONTRACT — MUST NOT:
//   ✗ Include any <zephyr/*> headers — use Robot Adapter API only

#define RO_PRIO_FRAMEWORK   ((ro_priority_t){ .v =   0 })  // State machine, device manager
#define RO_PRIO_APP         ((ro_priority_t){ .v =   5 })  // GCode parser, trajectory planner
#define RO_PRIO_BACKGROUND  ((ro_priority_t){ .v =  12 })  // Logging, telemetry, statistics
// ---------------------------------------------------------------------------
```

### `ro_thread_config_t` — Model S (Strict Explicit)

```c
// Stack ownership model: Model S (Strict Explicit).
// Caller ALWAYS provides a static stack buffer; adapter NEVER allocates one.
// Stack lifetime MUST exceed thread lifetime.
// Declare stack at the same scope as the ro_thread_create() call.
typedef struct {
    const char*   name;
    uint8_t*      stack;       // caller-provided static array — NEVER NULL
    uint32_t      stack_size;  // sizeof(stack); use sizeof(), not a magic number
    ro_priority_t priority;    // MUST be a RO_PRIO_* constant — compile-time enforced
    void        (*entry)(void* arg);
    void*         arg;
} ro_thread_config_t;
```

### Thread API

```c
// Lifecycle: caller owns the returned pointer until ro_thread_destroy().
// Returns NULL on failure (stack NULL, stack too small, name conflict,
// or thread slot pool exhausted).
//
// Thread slot pool contract:
//   The Adapter maintains a static pool of CONFIG_RO_MAX_THREADS slots
//   (compile-time constant; default 8, set per profile in prj.conf).
//   ro_thread_create() occupies one slot; ro_thread_destroy() releases it.
//   Attempting to create more threads than CONFIG_RO_MAX_THREADS → returns NULL.
//   CI roster check (ro_check_thread_roster) verifies that the sum of all
//   threads declared by the application never exceeds CONFIG_RO_MAX_THREADS.
ro_thread_t*  ro_thread_create(const ro_thread_config_t* cfg);

// Abort the thread and release its stack slot back to the static pool.
void          ro_thread_destroy(ro_thread_t* thread);

// Suspend calling thread for at least `ms` milliseconds.
// FORBIDDEN on RO_PRIO_RT_PULSE threads.
void          ro_thread_sleep_ms(uint32_t ms);

// Yield execution to other threads at same or higher priority.
// Required on cooperative threads at every loop iteration.
void          ro_thread_yield(void);
```

### Thread Creation Example

```c
#include <robotos/ro_thread.h>

// Stack declared at file scope — lifetime matches program lifetime.
static uint8_t control_loop_stack[2048];

void app_create_threads(void) {
    ro_thread_config_t cfg = {
        .name       = "control_loop",
        .stack      = control_loop_stack,
        .stack_size = sizeof(control_loop_stack),
        .priority   = RO_PRIO_RT_CONTROL,
        .entry      = control_loop_thread,
        .arg        = &g_pid,
    };
    g_control_thread = ro_thread_create(&cfg);
    RO_ASSERT(g_control_thread != NULL, "Failed to create control_loop thread");
}
```

### Stack Ownership Rules

```c
// ✅ CORRECT — static stack, same scope as create call
static uint8_t ctrl_stack[2048];
static void app_init(void) {
    ro_thread_config_t cfg = { .stack = ctrl_stack, .stack_size = sizeof(ctrl_stack), ... };
    g_ctrl = ro_thread_create(&cfg);
}

// ❌ WRONG — heap: violates no-malloc rule
uint8_t* stack = malloc(2048);

// ❌ WRONG — local frame: stack destroyed when function returns!
void some_fn(void) {
    uint8_t local_stack[2048];
    ro_thread_create(&(ro_thread_config_t){ .stack = local_stack, ... });
}
```

### Stack Size Convention

```c
// Each thread MUST name its stack size — never a magic number.
#define CONTROL_THREAD_STACK_SZ   2048   // PID + motion planner
#define STATE_MACHINE_STACK_SZ    1536   // State machine + command parse
#define GCODE_PARSER_STACK_SZ     3072   // String work; larger stack needed
#define LOG_THREAD_STACK_SZ       1024   // Background, no deep calls
```

### Priority Table

| Constant | Value | Class | Allowed APIs | Forbidden APIs | Use |
|---|---|---|---|---|---|
| `RO_PRIO_RT_PULSE` | -16 | Coop | `ro_thread_yield`, `*_isr` variants, `ro_time_us` | mutex, blocking queue, `ro_thread_sleep_ms` | SW pulse gen |
| `RO_PRIO_RT_CONTROL` | -10 | Coop | `ro_thread_yield`, `ro_timer_sync()` (>200 Hz loops), `ro_thread_sleep_ms()` (≤200 Hz only), lock-free queues, `ro_time_us/ms` | mutex, blocking call | PID ≥1kHz |
| `RO_PRIO_RT_MONITOR` | -2 | Coop | `ro_thread_yield`, `ro_time_us`, read-only state | mutex, blocking I/O | Fault watchdog |
| `RO_PRIO_FRAMEWORK` | 0 | Preempt | All Adapter APIs, mutex (w/ timeout) | `<zephyr/*>` | State machine |
| `RO_PRIO_APP` | 5 | Preempt | All Adapter APIs | `<zephyr/*>` | GCode parser |
| `RO_PRIO_BACKGROUND` | 12 | Preempt | All Adapter APIs | Time-sensitive ops | Logging |

---

## Timing API (`ro_time`)

```c
// ro_time.h
//
// Clock contract:
//   Source   : Zephyr system uptime (k_uptime_get / k_cycle_get_32)
//   Monotonic: YES — never jumps backward, not affected by wall-clock sync
//   Resolution:
//       ro_time_us() → uint64_t, 1 µs resolution, wraps after ~584,942 years
//       ro_time_ms() → uint32_t, 1 ms resolution, wraps after ~49.7 days
//
// Elapsed time MUST use unsigned subtraction (correct even at wrap):
//   uint32_t elapsed = ro_time_ms() - start_ms;  // ✅ always correct
//   if (elapsed > TIMEOUT_MS) { ... }

uint64_t ro_time_us(void);   // microseconds since boot
uint32_t ro_time_ms(void);   // milliseconds since boot (wraps at ~49.7 days)
void     ro_delay_us(uint32_t us);
void     ro_delay_ms(uint32_t ms);
```

---

## Hardware Timer API (`ro_timer`)

```c
// ro_timer.h
//
// Hardware-backed periodic timer for high-rate cooperative control loops.
// Backed by a Zephyr k_timer; wraps k_timer_start / k_timer_status_sync.
//
// WHY ro_timer vs ro_thread_sleep_ms:
//   ro_thread_sleep_ms(1)  — software sleep; jitter ±1 OS tick (1 ms typical)
//   ro_timer_sync()        — hardware-timer wake; jitter ≤2 µs on Cortex-M4
//
// RULE: loops running at ≥200 Hz MUST use ro_timer_sync().
//       loops running  <200 Hz MAY use ro_thread_sleep_ms().

typedef struct ro_timer ro_timer_t;             // Opaque. Declare as static.

// Configure and start a periodic timer.
// period_us: desired tick interval in microseconds.
// Returns RO_OK, or RO_EINVAL if period_us == 0.
// No ISR callback — hardware step generation uses the Framework stepper driver directly.
// <!-- STATUS: LOCKED DEC-03 -->
ro_status_t ro_timer_start_periodic(ro_timer_t* t, uint32_t period_us);

// Stop a running timer.  Safe to call even if timer was never started.
void        ro_timer_stop(ro_timer_t* t);

// Block the calling cooperative thread until the next timer tick.
// If the tick has already fired since the last call, returns immediately.
//
// Returns: number of timer periods elapsed since the last call.
//   0  — woke on time (exactly one period elapsed); deadline healthy.
//   ≥1 — overrun: N periods were missed. Caller MUST increment an overrun
//         counter and log. A loop that missed 3 periods requires different
//         fault policy than one that missed 1 — the count is the information.
//
// Return type rationale (DEC-02): ro_timer_sync is an information-returning
// timing primitive (how many periods elapsed), not an operation that can
// succeed or fail. uint32_t is the correct return type by design.
// Architecture owner confirmation pending — see FINAL_CANONICAL_DECISION_MEMO.md §DEC-02.
// <!-- STATUS: LOCKED WITH OWNER CONFIRMATION DEC-02 -->
uint32_t    ro_timer_sync(ro_timer_t* t);
```

### Canonical ≥200 Hz Loop Pattern

```c
#include <robotos/ro_timer.h>
#include <robotos/ro_thread.h>

static ro_timer_t ctrl_timer;           // file-scope: static lifetime
static uint32_t   ctrl_overruns = 0;

static void control_loop_1khz(void* arg) {
    // Start 1 kHz periodic tick
    ro_timer_start_periodic(&ctrl_timer, 1000 /*µs*/);

    while (1) {
        uint32_t overruns = ro_timer_sync(&ctrl_timer);
        if (overruns > 0) {
            ctrl_overruns += overruns;   // missed N periods — log or fault
        }

        // --- real-time work (must complete in < 1000 µs) ---
        float out = pid_ctrl_update(pid, setpoint, measured);
        stepper_move(stepper, (int32_t)out);  // Framework API
    }
}
```

### Sleep vs Timer Decision Table

| Loop rate | Mechanism | Rationale |
|-----------|-----------|----------|
| < 200 Hz (> 5 ms period) | `ro_thread_sleep_ms()` | OS tick jitter negligible at low rates |
| ≥ 200 Hz (≤ 5 ms period) | `ro_timer_sync()` | OS tick (1 ms) introduces >20% jitter |
| RT_PULSE threads (SW step gen) | Hardware step-timer ISR only | No sleep allowed; cooperative yield each step |

---

## Queue IPC API (`ro_queue`)

```c
// ro_queue.h
// Opaque handle. Acquire via ro_queue_create(). NEVER declare ro_queue_t by value.
typedef struct ro_queue ro_queue_t;

// ─── Backing memory — two ownership tiers (DEC-08) ───────────────────────────
//
//   SYSTEM queues (Framework-internal, log-flush, SM command queues):
//     buf == NULL, buf_size == 0
//     Backing carved from RO_GLOBAL_IPC_SLAB (static k_mem_slab, compile-time bounded).
//     Total slab budget: CONFIG_RO_IPC_SLAB_SIZE bytes across all slab-backed queues.
//
//   APPLICATION queues (cmd_q, seg_q, evt_q_isr — depth known at compile time):
//     buf != NULL, buf_size == item_size * capacity
//     Backing provided by caller (static uint8_t array — no slab consumed).
//     buf_size MUST equal item_size * capacity exactly; returns NULL on mismatch.
//
// RULE: Application code MUST supply a non-NULL buf to preserve slab budget
//       for Framework use. No per-queue heap allocation is performed in either tier.
// <!-- STATUS: LOCKED DEC-08 -->
#define CONFIG_RO_IPC_SLAB_SIZE   4096   // bytes; override in prj.conf

// Factory function — the ONLY way to create a queue.
// Returns an opaque handle. Returns NULL on any failure.
// NEVER instantiate ro_queue_t on the stack or as a static variable.
// <!-- STATUS: LOCKED DEC-01 -->
ro_queue_t*  ro_queue_create(size_t item_size,
                              size_t capacity,
                              void*  buf,
                              size_t buf_size);

// Caller must ensure no threads are blocked on q before calling destroy.
void         ro_queue_destroy(ro_queue_t* q);

// Thread-blocking send/recv (with timeout).
// timeout_ms == 0 → return immediately (non-blocking).
ro_status_t  ro_queue_send(ro_queue_t* q, const void* data, uint32_t timeout_ms);
ro_status_t  ro_queue_recv(ro_queue_t* q, void* data, uint32_t timeout_ms);

// ISR-safe variants — non-blocking, no timeout.
// Returns RO_OK or RO_EAGAIN (queue full/empty).
// NEVER returns RO_ETIMEDOUT.
ro_status_t  ro_queue_send_isr(ro_queue_t* q, const void* data);
ro_status_t  ro_queue_recv_isr(ro_queue_t* q, void* data);
```

### Usage Examples

```c
// ── System queue (Framework-internal, backed by slab) ─────────────────────
ro_queue_t* sys_q = ro_queue_create(sizeof(sys_msg_t), 16, NULL, 0);
RO_ASSERT(sys_q != NULL, "sys_q slab allocation failed");

// ── Application queue (caller-owned static buffer, no slab consumed) ──────
static uint8_t cmd_buf[sizeof(cmd_t) * 16];   // static lifetime — never on stack
ro_queue_t* cmd_q = ro_queue_create(sizeof(cmd_t), 16, cmd_buf, sizeof(cmd_buf));
RO_ASSERT(cmd_q != NULL, "cmd_q allocation failed");

// Producer (normal thread context)
cmd_t cmd = { .type = CMD_MOVE, .target = 1000 };
ro_status_t s = ro_queue_send(cmd_q, &cmd, 100 /*ms timeout*/);
if (s == RO_ETIMEDOUT) { /* queue full — drop or retry */ }

// Producer (ISR context — must use _isr variant, no timeout)
void endstop_isr(void) {
    cmd_t ev = { .type = CMD_HOME_TRIGGERED };
    ro_queue_send_isr(cmd_q, &ev);  // returns RO_EAGAIN if full, never blocks
}

// Consumer (state machine thread)
cmd_t received;
ro_status_t s = ro_queue_recv(cmd_q, &received, RO_WAIT_FOREVER);
```

---

## Memory Pool API (`ro_pool`)

```c
// ro_pool.h
typedef struct ro_pool ro_pool_t;

typedef enum {
    RO_POOL_LOCK_NONE   = 0,  // Single owner — fastest, ISR-safe
    RO_POOL_LOCK_ATOMIC = 1,  // Lock-free CAS — multi-thread + ISR safe
    RO_POOL_LOCK_MUTEX  = 2,  // Priority-inheriting mutex — NOT ISR-safe
} ro_pool_lock_t;

typedef struct {
    void*          buffer;      // Static backing array — NOT heap
    size_t         block_size;  // Fixed block size in bytes
    size_t         num_blocks;  // Number of blocks
    ro_pool_lock_t lock_mode;
} ro_pool_config_t;

// Pool handle is valid for program lifetime (no destroy for static pools).
ro_pool_t* ro_pool_create(const ro_pool_config_t* cfg);

void*      ro_pool_alloc(ro_pool_t* pool);            // Returns NULL if exhausted
void*      ro_pool_alloc_isr(ro_pool_t* pool);        // ISR-safe; LOCK_NONE/ATOMIC only
void       ro_pool_free(ro_pool_t* pool, void* ptr);
void       ro_pool_free_isr(ro_pool_t* pool, void* ptr);

typedef struct {
    size_t used_blocks;
    size_t peak_used;      // High-water mark
    size_t alloc_failures;
} ro_pool_stats_t;
void ro_pool_get_stats(const ro_pool_t* pool, ro_pool_stats_t* out);
```

### RT-Safety Contract

| `lock_mode` | Thread-safe | ISR-safe | Priority inversion | When to use |
|---|---|---|---|---|
| `LOCK_NONE` | ❌ single owner | ✅ | None | One thread + ISR reads only |
| `LOCK_ATOMIC` | ✅ | ✅ | None | Multiple threads + ISR |
| `LOCK_MUTEX` | ✅ | ❌ **forbidden in ISR** | Mitigated by inheritance | Thread-only, non-RT |

> **Rule:** Any pool touched from ISR MUST use `LOCK_NONE` or `LOCK_ATOMIC`.
> Calling `ro_pool_alloc()` (LOCK_MUTEX) from ISR → assert in debug build.

### Pool Declaration Pattern

```c
// Declare backing buffer at file scope (static lifetime)
static uint8_t msg_pool_buf[64 * 256];  // 64 blocks × 256 bytes = 16 KB

static ro_pool_t* msg_pool;

static void pools_init(void) {
    ro_pool_config_t cfg = {
        .buffer     = msg_pool_buf,
        .block_size = 256,
        .num_blocks = 64,
        .lock_mode  = RO_POOL_LOCK_ATOMIC,  // multi-thread + ISR safe
    };
    msg_pool = ro_pool_create(&cfg);
    RO_ASSERT(msg_pool != NULL, "msg_pool creation failed");
}
```

---

## Mutex API (`ro_mutex`)

```c
// ro_mutex.h
// Priority-inheriting mutex — for PREEMPTIVE threads only.
// NEVER acquire a mutex from a cooperative thread (RO_PRIO_RT_*).
typedef struct ro_mutex ro_mutex_t;

ro_mutex_t*  ro_mutex_create(void);
void         ro_mutex_destroy(ro_mutex_t* m);
ro_status_t  ro_mutex_lock(ro_mutex_t* m, uint32_t timeout_ms);
void         ro_mutex_unlock(ro_mutex_t* m);
```

---

## Semaphore API (`ro_sem`)

### Overview

A **semaphore** is a lightweight synchronization primitive distinct from a mutex:

- **Mutex** = mutual exclusion lock (ownership-based)
- **Semaphore** = counting resource (state-based)

Semaphores in RobotOS are used for:
1. **Simple signaling** — ISR notifies a waiting thread (e.g., step interrupt → motion complete)
2. **Bounded blocking waits** — thread blocks until a condition fires, with timeout
3. **ISR-to-thread wakeup** — ISR calls `ro_sem_give_isr()` (non-blocking); thread calls `ro_sem_take()` (blocking)

### Design

```c
// ro_sem.h

typedef struct ro_sem ro_sem_t;

// Lifecycle: caller owns the semaphore object; no hidden heap allocation
// initial_count: starting semaphore count (typically 0 for blocking-wait pattern)
// max_count: maximum value the semaphore can reach (prevents confusion on overflow)
ro_sem_t*   ro_sem_create(uint32_t initial_count, uint32_t max_count);
void        ro_sem_destroy(ro_sem_t* s);

// Take (decrement): block if count <= 0
// timeout_ms = 0 → non-blocking poll (return RO_EAGAIN if count == 0)
// timeout_ms > 0 → block with timeout (return RO_OK or RO_ETIMEDOUT)
// Returns:
//   RO_OK         — semaphore acquired (count decremented)
//   RO_ETIMEDOUT  — timeout expired before acquisition
//   RO_EAGAIN     — timeout_ms == 0 and count was 0 (non-blocking poll failed)
ro_status_t ro_sem_take(ro_sem_t* s, uint32_t timeout_ms);

// Give (increment): post the semaphore
// Increments count and wakes ONE waiting thread (if any are blocked).
// IMPORTANT: If multiple threads are waiting, only ONE is woken.
//            Others must call ro_sem_take() again after being woken by a
//            subsequent give (no spurious wakes, but also no broadcast).
ro_status_t ro_sem_give(ro_sem_t* s);

// Give-ISR: ISR-safe variant (non-blocking, no timeout)
// May be called from ISR context. Never blocks or sleeps.
// Returns:
//   RO_OK     — semaphore posted (count incremented)
//   RO_EAGAIN — semaphore already at max_count (overflow prevented)
ro_status_t ro_sem_give_isr(ro_sem_t* s);
```

### ISR / Thread Coordination Pattern (Canonical)

```c
#include <robotos/ro_sem.h>
#include <robotos/ro_gpio.h>

// Semaphore for ISR-to-thread signaling
static ro_sem_t* motion_complete_sem;

// ISR callback (hardware step timer interrupt)
static void step_complete_isr(void) {
    // Called at high frequency (e.g., 10 kHz); MUST be fast (<1 µs).
    // Decrement step counter; if motion done, signal the thread.
    if (--steps_remaining == 0) {
        ro_sem_give_isr(motion_complete_sem);  // wake stepper_wait_idle
    }
}

// Application thread (RO_PRIO_APP or RO_PRIO_RT_CONTROL)
static void motion_thread(void* arg) {
    motion_complete_sem = ro_sem_create(0, 1);  // binary semaphore
    RO_ASSERT(motion_complete_sem != NULL, "sem alloc failed");

    while (1) {
        // Start motion; ISR will count down and post semaphore
        stepper_move(stepper, 400);

        // Block until step counter reaches 0 (step-complete ISR posts)
        ro_status_t s = ro_sem_take(motion_complete_sem, 5000 /*ms*/);
        if (s == RO_ETIMEDOUT) {
            RO_LOG_ERROR("Motion timeout — stalled?");
            sm_dispatch(sm, CMD_FAULT);
        } else if (RO_SUCCEEDED(s)) {
            // Motion complete; continue
        }
    }
}
```

### Polling Pattern (Bring-up only — NOT recommended)

```c
// ❌ This is NOT the canonical RobotOS pattern.
// Use ISR callback + semaphore wakeup instead.
// Polling is documented here only as a DEBUG fallback.

static void polling_fallback(stepper_t* st) {
    while (stepper_get_state(st) != STEPPER_STATE_IDLE) {
        ro_thread_sleep_ms(1);  // busy-wait; wastes CPU
    }
}
```

### Semaphore vs Queue vs Mutex Decision Table

| Use case | Primitive | Reason |
|----------|-----------|--------|
| Simple signaling (ISR → thread) | **Semaphore** | Lightweight; no data payload; ISR-safe post |
| Message passing (data queued) | Queue | Payload-bearing; multiple readers possible |
| Mutual exclusion (resource lock) | Mutex | Ownership-based; prevents re-entry |
| Counting resource (thread pool) | Semaphore | Max_count enforces bounded resource |

### Contract Summary

| Aspect | Guarantee |
|--------|-----------|
| **ISR context** | `ro_sem_give_isr()` is safe; returns immediately; no block-able operations |
| **Multi-waiter** | Only ONE waiter is woken per `give()`; others must retry their `take()` |
| **Timeout** | `RO_ETIMEDOUT` if timeout_ms elapses; `RO_OK` if acquired before timeout |
| **Overflow** | If count reaches max_count, `give_isr()` returns `RO_EAGAIN`; count unchanged |
| **Heap** | Semaphore objects allocated from static Adapter pool; no heap after `robotos_init()` |
| **Reentrancy** | Not reentrant: caller MUST NOT call `ro_sem_take()` on same semaphore from multiple threads simultaneously (use separate instances if needed) |

---

## GPIO API (`ro_gpio`)

```c
// ro_gpio.h
typedef struct ro_gpio ro_gpio_t;

typedef struct {
    const char* dt_label;          // Device Tree label string — MUST be a string literal
    uint32_t    pin;               // Pin index
    bool        active_high;       // true = active high, false = active low
} ro_gpio_config_t;
```

> **DT label lookup contract (Adapter GPIO / PWM / I2C / SPI):**  
> The `dt_label` string MUST be a `const char*` string literal (`.rodata`).  
> Lookup is **O(1) table-indexed**: at `POST_KERNEL` bind time each DT node is assigned a
> compile-time slot index; `ro_gpio_get` / `ro_pwm_get` / `ro_i2c_get` resolve the label
> **once** at get-time via a fixed-size lookup table — no `strcmp` loop, no malloc, no
> runtime string parsing.  
> If the label is not found, the function returns `NULL` immediately.

```c

typedef enum {
    RO_GPIO_INPUT  = 0,
    RO_GPIO_OUTPUT = 1,
} ro_gpio_dir_t;

ro_gpio_t*  ro_gpio_get(const ro_gpio_config_t* cfg, ro_gpio_dir_t dir);
void        ro_gpio_put(ro_gpio_t* gpio);
ro_status_t ro_gpio_set(ro_gpio_t* gpio, bool level);   // OUTPUT only
bool        ro_gpio_read(ro_gpio_t* gpio);              // INPUT only

// Interrupt-driven input
typedef void (*ro_gpio_isr_t)(ro_gpio_t* gpio, void* user);
ro_status_t ro_gpio_set_isr(ro_gpio_t* gpio, ro_gpio_isr_t cb, void* user);
ro_status_t ro_gpio_clear_isr(ro_gpio_t* gpio);
```

---

## PWM API (`ro_pwm`)

```c
// ro_pwm.h
typedef struct ro_pwm ro_pwm_t;

typedef struct {
    const char* dt_label;       // DT PWM controller label
    uint32_t    channel;        // PWM channel index
    uint32_t    period_ns;      // Default period in nanoseconds
} ro_pwm_config_t;

ro_pwm_t*   ro_pwm_get(const ro_pwm_config_t* cfg);
void        ro_pwm_put(ro_pwm_t* pwm);
ro_status_t ro_pwm_set_pulse(ro_pwm_t* pwm, uint32_t pulse_ns);
ro_status_t ro_pwm_set_period(ro_pwm_t* pwm, uint32_t period_ns);
ro_status_t ro_pwm_disable(ro_pwm_t* pwm);
```

---

## I2C / SPI API (`ro_i2c`, `ro_spi`)

```c
// ro_i2c.h
typedef struct ro_i2c_bus ro_i2c_bus_t;

ro_i2c_bus_t* ro_i2c_get(const char* dt_label);
void          ro_i2c_put(ro_i2c_bus_t* bus);
ro_status_t   ro_i2c_write(ro_i2c_bus_t* bus, uint8_t addr,
                           const uint8_t* buf, size_t len);
ro_status_t   ro_i2c_read(ro_i2c_bus_t* bus, uint8_t addr,
                          uint8_t* buf, size_t len);
ro_status_t   ro_i2c_write_read(ro_i2c_bus_t* bus, uint8_t addr,
                                const uint8_t* wbuf, size_t wlen,
                                uint8_t* rbuf, size_t rlen);

// ro_spi.h (similar pattern — omitted for brevity)
```

---

## Logging API (`ro_log`)

```c
// ro_log.h
//
// Ring-buffer backed logger. Log entries are written lock-free to a ring buffer;
// a background log_flush thread (RO_PRIO_BACKGROUND) drains to UART.
// ISR context: logging is FORBIDDEN (UART is blocking). Use atomic fault flags instead.
//
// Ring buffer contract:
//   Size    : CONFIG_RO_LOG_RING_SIZE bytes (compile-time; default 2048).
//             Set in prj.conf; must be a power of 2.
//   Overflow: DROP_OLDEST policy — when the ring is full, the oldest un-flushed
//             entry is silently overwritten and ro_log_drop_count incremented.
//             Alternative DROP_NEWEST is available via
//             CONFIG_RO_LOG_OVERFLOW=DROP_NEWEST but is NOT the default.
//   Thread-safety: lock-free atomic ring-head update; safe from any thread.
//   ISR:     FORBIDDEN — ro_log() may call ro_time_us() which is ISR-safe, but
//            the ring-head CAS may spin; use atomic fault flags from ISR instead.
#define CONFIG_RO_LOG_RING_SIZE   2048   // bytes; override in prj.conf (power of 2)

extern uint32_t ro_log_drop_count;       // incremented on DROP_OLDEST overwrite

// Severity levels
typedef enum {
    RO_LOG_LEVEL_DEBUG = 0,
    RO_LOG_LEVEL_INFO,
    RO_LOG_LEVEL_WARN,
    RO_LOG_LEVEL_ERROR,
} ro_log_level_t;

// Convenience macros — compile-time level stripping possible
#define RO_LOG_DEBUG(fmt, ...)  ro_log(RO_LOG_LEVEL_DEBUG, fmt, ##__VA_ARGS__)
#define RO_LOG_INFO(fmt, ...)   ro_log(RO_LOG_LEVEL_INFO,  fmt, ##__VA_ARGS__)
#define RO_LOG_WARN(fmt, ...)   ro_log(RO_LOG_LEVEL_WARN,  fmt, ##__VA_ARGS__)
#define RO_LOG_ERROR(fmt, ...)  ro_log(RO_LOG_LEVEL_ERROR, fmt, ##__VA_ARGS__)

void ro_log(ro_log_level_t level, const char* fmt, ...);
```

### ISR Logging Workaround

```c
// ❌ FORBIDDEN: ro_log inside ISR (UART is blocking)
void endstop_isr(void) {
    RO_LOG_INFO("endstop triggered");  // WRONG — may sleep
}

// ✅ CORRECT: set atomic flag; log from monitor thread
static atomic_t fault_flags;

void endstop_isr(void) {
    atomic_or(&fault_flags, FAULT_FLAG_ENDSTOP);
}

void monitor_thread(void* arg) {
    while (1) {
        uint32_t f = atomic_clear(&fault_flags);
        if (f & FAULT_FLAG_ENDSTOP) {
            RO_LOG_INFO("endstop triggered");
        }
        ro_thread_sleep_ms(10);
    }
}
```

---

## Tracing API (`ro_trace`)

```c
// ro_trace.h
//
// Wraps Zephyr's SYS_PORT_TRACING infrastructure.
// All macros compile to zero instructions when CONFIG_TRACING=n.

// Emit a custom trace event (compiled out if CONFIG_TRACING=n)
#define RO_TRACE_EVENT(category, ...)  \
    SYS_PORT_TRACING_FUNC(category, ##__VA_ARGS__)

// Pre-defined RobotOS trace points:
//
//   RO_TRACE_EVENT(stepper_move, name, steps)
//   RO_TRACE_EVENT(pid_ctrl_update, pid_ptr, error)
//   RO_TRACE_EVENT(ro_deadline_miss, name, elapsed_us, budget_us)
//   RO_TRACE_EVENT(ro_state_transition, from, to)

// Initialise tracing backend (called by SYS_INIT at POST_KERNEL).
// Must be called before any RO_TRACE_EVENT macros.
ro_status_t ro_trace_init(void);
```

---

## Deadline Monitoring API (`ro_deadline`)

```c
// ro_deadline.h
//
// Lightweight deadline window: caller marks begin/end of a real-time work unit.
// Inline cost: two ro_time_us() calls (≤1 µs on Cortex-M4 @ 168 MHz).
// No separate thread required.

typedef struct {
    uint64_t    budget_us;   // Allowed window (microseconds)
    const char* name;        // Logged on violation — MUST be a string literal
    uint64_t    _start_us;   // Private: set by ro_deadline_begin()
} ro_deadline_t;

// Mark start of real-time window.
static inline void ro_deadline_begin(ro_deadline_t* d) {
    d->_start_us = ro_time_us();
}

// Mark end of window.
// Returns true  → deadline met.
// Returns false → deadline missed; emits trace event + increments miss counter.
bool ro_deadline_end(ro_deadline_t* d);

// Read miss counter for a named deadline (thread-safe, read-only).
uint32_t ro_deadline_miss_count(const char* name);

// Reset miss counter.
void ro_deadline_miss_reset(const char* name);
```

### Trace Emission on Miss

```c
// Inside ro_deadline_end() implementation:
if (elapsed_us > d->budget_us) {
    SYS_PORT_TRACING_FUNC(ro_deadline_miss, d->name, elapsed_us, d->budget_us);
    // Increment per-name counter (lock-free atomic)
    return false;
}
return true;
```

### Usage in Control Thread

```c
// Example: PID control loop with deadline monitoring
static ro_deadline_t pid_loop_deadline = {
    .budget_us = 1000,                    // 1 ms = 1 kHz loop
    .name      = "pid_control_1khz"
};

static void pid_control_thread(void* arg) {
    while (1) {
        ro_deadline_begin(&pid_loop_deadline);
        
        // Perform 1 kHz work (~< 1000 µs)
        float measured = encoder_get_count(enc);     // ISR-safe; returns int32_t tick count
        float output = pid_ctrl_update(pid, setpoint, measured);  // no dt param
        motor_set_speed(motor, output);
        
        // Check deadline
        if (!ro_deadline_end(&pid_loop_deadline)) {
            RO_LOG_WARN("pid_deadline missed: overrun detected");
        }
    }
}
```


```c
#define CONTROL_DEADLINE_US  1000u   // 1 ms budget for 1 kHz loop

static ro_deadline_t ctrl_dl = {
    .budget_us = CONTROL_DEADLINE_US,
    .name      = "control_loop",
};

static ro_timer_t ctrl_timer;           // hardware-backed periodic timer
static uint32_t   ctrl_overruns = 0;

static void control_loop_thread(void* arg) {
    ro_timer_start_periodic(&ctrl_timer, 1000 /*µs — 1 kHz*/);
    while (1) {
        // Block until next hardware tick (≥200 Hz MUST use ro_timer_sync).
        uint32_t timer_overruns = ro_timer_sync(&ctrl_timer);
        if (timer_overruns > 0) {
            ctrl_overruns += timer_overruns;   // overrun — count for observability
        }

        ro_deadline_begin(&ctrl_dl);

        float out = pid_ctrl_update(pid, setpoint, measured);  // Framework API
        stepper_move(stepper, (int32_t)out);  // Framework API — no ro_ prefix

        if (!ro_deadline_end(&ctrl_dl)) {
            // Deadline missed — trace already emitted.
            // Escalate to fault handler if miss_count exceeds tolerance.
            if (ro_deadline_miss_count("control_loop") > MAX_TOLERATED_MISSES) {
                sm_dispatch(sm, ROBOT_CMD_FAULT);   // DEC-07: canonical SM dispatch
            }
        }
    }
}
```

### Shell Observability

```
uart:~$ ro deadline stats

Name            | Budget (µs) | Misses | Max elapsed (µs)
----------------|-------------|--------|------------------
control_loop    | 1000        | 0      | 347
state_machine   | 5000        | 0      | 1203
```

---

## Error & Status Codes (`ro_status`)

```c
// ro_status.h — the ONLY error type in public RobotOS API.
// NEVER expose errno.h, Zephyr codes, or POSIX codes in public headers.
// Translate them in the adapter layer and nowhere else.

typedef int32_t ro_status_t;

// ── Success ─────────────────────────────────────────────────────────────────
#define RO_OK              0

// ── Operational errors (expected in correct code) ───────────────────────────
#define RO_ETIMEDOUT      -1    // Timeout expired
#define RO_EAGAIN         -2    // Temporarily unavailable; retry
#define RO_EBUSY          -3    // Resource in use
#define RO_ECANCELED      -4    // Operation cancelled

// ── Programming errors (should never occur in correct code) ─────────────────
#define RO_EINVAL         -10   // Invalid argument (NULL, out-of-range)
#define RO_ENOMEM         -11   // Pool exhausted
#define RO_ENODEV         -12   // Device handle NULL or uninitialised
#define RO_ENOTSUP        -13   // Not supported on this platform
#define RO_EOVERFLOW      -14   // Buffer or counter overflow

// ── Fatal / boot errors (trigger FAULT state) ───────────────────────────────
#define RO_EFAIL          -20   // Generic unrecoverable failure
#define RO_ENOTREADY      -21   // Component not initialised
#define RO_EHWINIT        -22   // Hardware init failed (device_is_ready() == false)
#define RO_EBOOTSEQ       -23   // Called out-of-order relative to boot sequence

// ── Test macros ─────────────────────────────────────────────────────────────
#define RO_SUCCEEDED(s)   ((s) == RO_OK)
#define RO_FAILED(s)      ((s) != RO_OK)
#define RO_IS_FATAL(s)    ((s) <= RO_EFAIL)

// ── Propagate pattern ───────────────────────────────────────────────────────
#define RO_TRY(expr)  do { ro_status_t _s = (expr); if (_s != RO_OK) return _s; } while (0)
```

### Return Value Convention

Two patterns — **never mix them**:

| Pattern | When to use | Success | Failure | Example |
|---------|------------|---------|---------|---------|
| **Alloc/init** | Creates or acquires a resource | non-NULL pointer | NULL | `ro_queue_create()`, `ro_pool_create()` |
| **Operation** | Acts on an existing handle | `RO_OK` (0) | negative `ro_status_t` | `ro_queue_send()`, `ro_pwm_set_pulse()` |

```c
// ✅ CORRECT
static uint8_t cmd_buf[sizeof(cmd_t) * 16];
ro_queue_t* q = ro_queue_create(sizeof(cmd_t), 16, cmd_buf, sizeof(cmd_buf));
if (q == NULL) { return RO_ENOMEM; }

ro_status_t s = ro_queue_send(q, &cmd, 100);
if (s != RO_OK) { return s; }

// ❌ WRONG — pattern mismatch
ro_status_t s = ro_queue_create(...);   // returns pointer, not status!
```

### Zephyr errno Translation (Adapter-internal only)

```c
// ro_status.c — NOT in public header
static inline ro_status_t ro_from_zephyr(int zephyr_err) {
    switch (zephyr_err) {
        case 0:          return RO_OK;
        case -ETIMEDOUT: return RO_ETIMEDOUT;
        case -EAGAIN:    return RO_EAGAIN;
        case -EBUSY:     return RO_EBUSY;
        case -ENOMEM:    return RO_ENOMEM;
        case -EINVAL:    return RO_EINVAL;
        case -ENODEV:    return RO_ENODEV;
        case -ENOTSUP:   return RO_ENOTSUP;
        case -EOVERFLOW: return RO_EOVERFLOW;
        default:
            RO_LOG_ERROR("Unknown zephyr err=%d", zephyr_err);
            return RO_EFAIL;
    }
}
```

---

## CI Enforcement Rules

The following checks run on every commit (pre-push hook + CI pipeline):

```bash
# ─── Zephyr Include Whitelist ───────────────────────────────────────────────
# Source files ALLOWED to include <zephyr/*>:
#   src/adapter/zephyr/*.c     — Any <zephyr/*> (adapter implementations)
#   src/framework/drivers/*.c  — ONLY <zephyr/devicetree.h> + <zephyr/device.h>
#
# Zero-tolerance (emit error on any <zephyr/*>):
#   include/robotos/*.h        — Public headers
#   src/framework/*.h          — Framework public headers
#   app/**                     — Application code
# ─────────────────────────────────────────────────────────────────────────────

# 1) Public headers: absolute zero Zephyr
for f in include/robotos/*.h src/framework/*.h; do
  if grep -q '#include <zephyr/' "$f"; then
    echo "ERROR: $f includes Zephyr. Public headers must be RTOS-agnostic."
    exit 1
  fi
done

# 2) Framework drivers: DT + device API only (no driver-level or kernel headers)
for f in src/framework/drivers/*.c; do
  if grep -qE '#include <zephyr/(drivers|kernel)' "$f"; then
    echo "ERROR: $f uses <zephyr/drivers/*> or <zephyr/kernel.h>. Use Adapter APIs."
    exit 1
  fi
done

# 3) Application: zero Zephyr
for f in $(find app/ -name '*.c' -o -name '*.h'); do
  if grep -q '#include <zephyr/' "$f"; then
    echo "ERROR: $f includes Zephyr. Application must use Robot Adapter/Framework APIs."
    exit 1
  fi
done

# 4) Framework headers must NOT use ro_ prefix (naming convention)
for f in include/robotos/*.h; do
  basename=$(basename "$f")
  if [[ "$basename" != ro_* ]]; then
    if grep -q '#include <zephyr/' "$f"; then
      echo "ERROR: Framework header $f must not include Zephyr."
      exit 1
    fi
  fi
done
```

---

## Porting Guide

### How to Port Adapter to a Different RTOS

The Adapter Layer is architecturally separated so that a full RTOS swap requires only:

1. Replace `src/adapter/zephyr/` with `src/adapter/<new_rtos>/`
2. Implement each `ro_*.c` file against the new RTOS primitives
3. Map the new RTOS error codes to `ro_status_t` in the new `ro_status.c`
4. Ensure the new RTOS supports priority-based scheduling with at least 6 coop + 3 preempt levels

**What you do NOT need to change:**
- Any header in `include/robotos/`
- Any file in `src/framework/`
- Any file in `app/`

### Porting Effort Estimate

| Module | Effort | Notes |
|--------|--------|-------|
| `ro_thread.c` | 0.5 day | Core; must map priority model |
| `ro_time.c` | 0.25 day | Just wrap uptime + cycle counter |
| `ro_queue.c` | 0.5 day | Wrap FIFO primitive |
| `ro_pool.c` | 0.5 day | Wrap slab allocator |
| `ro_mutex.c` | 0.25 day | Priority-inheriting mutex required |
| `ro_gpio.c` | 0.5–1 day | HW-specific |
| `ro_pwm.c` | 0.5–1 day | HW-specific |
| `ro_i2c.c` / `ro_spi.c` | 0.5–1 day each | HW-specific |
| `ro_log.c` | 0.5 day | Ring buffer is portable |
| `ro_trace.c` | 0.25 day | Stub if new RTOS has no tracing |
| `ro_deadline.c` | 0.1 day | Uses `ro_time_us()` — auto-works |
| **TOTAL** | **~5–7 days** | To FreeRTOS or bare-metal |

---

**Document Status:** 🚧 Living Document  
**Layer:** Robot Adapter (100% portable)  
**Related:** [KERNEL_LAYER.md](KERNEL_LAYER.md) | [FRAMEWORK_LAYER.md](FRAMEWORK_LAYER.md) | [ARCHITECTURE.md](ARCHITECTURE.md)  
**Last Review:** 2026-03-01
