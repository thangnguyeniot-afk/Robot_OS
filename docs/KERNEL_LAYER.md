# RobotOS Inspire — Kernel Layer Design

> **Version:** v0.1-alpha  
> **Last Updated:** 2026-03-03  
> **Status:** 🚧 Under Active Development  
> **Layer:** Zephyr Kernel (RTOS-specific, 0% portable)

> **Scope note:** This document describes the Zephyr kernel capabilities that RobotOS relies on
> and the policies that *constrain* how they may be used. The actual implementation of those
> policies lives in the **Adapter Layer** (`src/adapter/zephyr/*.c`) and the **Framework driver
> bindings** (`src/framework/drivers/*.c`). Sections that describe Adapter or Framework
> behaviour are included here for traceability, not to imply kernel ownership.

---

## 📋 Table of Contents

1. [Layer Role & Boundary](#layer-role--boundary)
2. [Kernel Capabilities Used by RobotOS](#kernel-capabilities-used-by-robotos)
3. [Scheduler Model](#scheduler-model)
4. [SYS_INIT Lifecycle](#sys_init-lifecycle)
5. [IPC Primitives](#ipc-primitives)
6. [Memory Sub-system](#memory-sub-system)
7. [Device Tree & Driver Model](#device-tree--driver-model)
8. [Tracing & Observability Hooks](#tracing--observability-hooks)
9. [Customization Policy](#customization-policy)
10. [What Adapter Layer Exposes (Contract Summary)](#what-adapter-layer-exposes-contract-summary)

---

## Layer Role & Boundary

### Position in the Stack

```
Application Layer
      ↓
Robot Framework Layer
      ↓
Robot Adapter Layer          ← consumes Kernel APIs; hides them from above
      ↓
┌──────────────────────────────────────────────────────────────────┐
│  ZEPHYR KERNEL LAYER  (this document)                           │
│                                                                  │
│  Ownership: Zephyr Project (upstream); RobotOS adds thin        │
│  configuration only — no kernel patches, no forks.              │
└──────────────────────────────────────────────────────────────────┘
      ↓
Hardware Layer (STM32F4 / ESP32 / nRF52 / QEMU)
```

### Responsibilities

| Responsibility | Owner |
|---------------|-------|
| Thread scheduling (cooperative + preemptive) | Zephyr kernel |
| Context switch, priority inheritance | Zephyr kernel |
| IPC primitives (msgq, mutex, semaphore, condvar) | Zephyr kernel |
| Static memory pools (`k_mem_slab`) | Zephyr kernel |
| Hardware timers, SysTick, uptime clock | Zephyr kernel |
| Device Tree compile-time binding | Zephyr build system |
| SYS_INIT ordered boot phases | Zephyr kernel |
| Tracing hooks (`SYS_PORT_TRACING_*`) | Zephyr kernel |
| GPIO / PWM / I2C / SPI driver model | Zephyr driver model |

### What the Kernel Layer Does NOT Do

- ❌ No robotics semantics
- ❌ Does not know about stepper motors, PID loops, or state machines
- ❌ Does not enforce thread roster discipline (that is Adapter + CI)
- ❌ Does not translate errno codes to `ro_status_t` (that is Adapter)

### Kernel Internal Thread Assumptions

Zephyr creates its own internal threads (idle thread, system workqueue, logging thread, etc.).
RobotOS **does not own or control** these threads, but the RobotOS profile MUST account for them:

| Zephyr-internal thread | Config key / source | RobotOS policy |
|------------------------|---------------------|----------------|
| `idle` | Always lowest preemptive (Zephyr fixed) | Always present; never interferes with RT tier |
| `sysworkq` | `CONFIG_SYSTEM_WORKQUEUE_PRIORITY` | Must be ≥ `RO_PRIO_BACKGROUND` (.v=12); CI checks |
| `logging` | `CONFIG_LOG_BACKEND_UART_OUTPUT_PRIORITY` | Must be ≥ `RO_PRIO_BACKGROUND` (.v=12) |
| Shell UART thread | `CONFIG_SHELL_THREAD_PRIORITY` | Must be ≥ `RO_PRIO_APP` (.v=5) in production builds |

**CI enforcement — Thread Roster Whitelist:**
```
CI step: ro_check_thread_roster
  Source: post-build symbol scan + CONFIG_THREAD_MONITOR=y log at boot
  Method: Assert every thread-create event matches the approved roster:
    [log_flush, state_machine, <app-declared threads>]
    + Zephyr-internal threads above
  Fail condition: any thread name NOT in whitelist → CI error
```

> **Rule:** No anonymous or surprise threads. Every thread MUST be declared in the `Thread Roster`
> section of the application's `app_config.h` and listed in `cmake/thread_whitelist.cmake`.

---

## Kernel Capabilities Used by RobotOS

### Capability Map

| Zephyr Capability | Used By | RobotOS Abstraction |
|------------------|---------|---------------------|
| `k_thread` / `k_thread_create` | `ro_thread.c` | `ro_thread_t` / `ro_thread_create()` |
| `k_uptime_get()` / `k_cycle_get_32()` | `ro_time.c` | `ro_time_ms()` / `ro_time_us()` |
| `k_msgq` (message queue) | `ro_queue.c` | `ro_queue_t` / `ro_queue_send/recv()` |
| `k_mem_slab` (fixed-block pool) | `ro_pool.c` | `ro_pool_t` / `ro_pool_alloc/free()` |
| `k_mutex` (priority-inheriting) | `ro_mutex.c` | `ro_mutex_t` (preemptive paths only) |
| `k_sem` (semaphore) | `ro_sem.c` | `ro_sem_t` |
| `CONFIG_TRACING` hooks | `ro_trace.c` | `ro_trace_init()` + trace macros |
| `SYS_INIT()` macro | all `*_drv_bind.c` | Boot phase registration |
| `DEVICE_DT_GET` / `device_is_ready` | Framework drivers | Device binding (internal only) |
| `GPIO_DT_SPEC_GET` | `ro_gpio.c` / Framework drivers | GPIO abstraction |
| Zephyr PWM API | `ro_pwm.c` | PWM abstraction |
| Zephyr I2C / SPI API | `ro_i2c.c` / `ro_spi.c` | Bus abstraction |

### Include Whitelist

Only the following source files are permitted to include `<zephyr/*>` headers:

```
src/adapter/zephyr/*.c       — ALL <zephyr/*> usage lives here
src/framework/drivers/*.c    — ONLY <zephyr/devicetree.h> + <zephyr/device.h>
```

**Zero-tolerance files (any `<zephyr/*>` → CI error):**
```
include/robotos/*.h          — Public headers (Adapter + Framework API)
src/framework/*.h            — Framework public headers
app/**/*.c / app/**/*.h      — Application code
```

**CI enforcement mechanism:**
```
CI step: ro_check_zephyr_includes
  Tool   : grep -rn '#include <zephyr/' <target_paths>
  Action : Any match in zero-tolerance paths → exit 1 (build fails)
  Config : cmake/ci/check_headers.cmake — paths listed explicitly
  Run at : cmake configure + pre-commit hook
```

**Valid exceptions (none currently):**  
There are no approved exceptions. If a Framework or Application file needs a Zephyr device
handle, it MUST go through the Adapter API or a Framework driver binding (`src/framework/drivers/*.c`).

---

## Scheduler Model

### Two Scheduling Classes

Zephyr uses a **sign-based** class distinction in thread priority. RobotOS preserves this directly:

```
Priority value < 0  →  Cooperative class
                        Thread is NEVER preempted.
                        MUST yield explicitly each loop iteration.

Priority value ≥ 0  →  Preemptive class
                        Scheduler may context-switch at any tick.
                        Priority inheritance active via k_mutex.
```

### Priority Number Space

```
Zephyr priority number line (lower value = higher priority):

 -(CONFIG_NUM_COOP_PRIORITIES)          -1    0    (CONFIG_NUM_PREEMPT_PRIORITIES-1)
  │  ◄──────── cooperative ─────────── │    │ ──────────── preemptive ─────────── │

RobotOS default profile mapping:
  -16  RT_PULSE     (coop, highest)
  -10  RT_CONTROL   (coop)
   -2  RT_MONITOR   (coop, lowest coop)
    0  FRAMEWORK    (preempt, highest)
    5  APP          (preempt)
   12  BACKGROUND   (preempt, lowest)
```

> **These values are the RobotOS default profile** — governed by `CONFIG_NUM_COOP_PRIORITIES`
> and `CONFIG_NUM_PREEMPT_PRIORITIES`. They are centralised in one Adapter header and never
> appear as magic numbers in application code.

### Cooperative Thread Contract (Kernel Perspective)

When a thread runs at cooperative priority, the Zephyr kernel guarantees:
- No other thread will preempt it
- It runs until it calls `k_yield()`, `k_sleep()`, or a blocking IPC primitive
- **ISRs STILL preempt cooperative threads** — hardware interrupts are orthogonal to software priority

```
Time ──────────────────────────────────────────────────────────────────►

  RT_CONTROL thread (coop, -10):
  ████████████████████░░░░░░░░░████████████████░░░░░░░░░███████░░░░░░░
                      │        ▲               │        ▲
                      │        │               │        │
                    yield()  resumes         yield()  resumes

  FRAMEWORK thread (preempt, 0):  ← never runs while RT_CONTROL holds CPU
  ░░░░░░░░░░░░░░░░░░░░████████░░░░░░░░░░░░░░░░████████░░░░░░░░░░░░░░░░

  ISR (step pulse / endstop):      ← CAN interrupt at any time
                     ─┐  ┌─         ─┐ ┌─
  ░░░░░░░░░░░░░░░░░░░ │  │ ░░░░░░░░ │ │ ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░
                       │  │           └─┘
                       └──┘
```

**Cooperative Thread Yield Contract (hard rules — violations are architecture bugs):**

| Priority preset | Loop rate | Required yield mechanism | Rationale |
|----------------|-----------|-------------------------|-----------|
| `RO_PRIO_RT_PULSE` | N/A (step-by-step) | `ro_thread_yield()` after every step | Sleep forbidden; each step is one yield |
| `RO_PRIO_RT_CONTROL` | < 200 Hz (> 5 ms) | `ro_thread_sleep_ms(period_ms)` | OS tick jitter negligible at low rate |
| `RO_PRIO_RT_CONTROL` | ≥ 200 Hz (≤ 5 ms) | `ro_timer_sync(&timer)` | `sleep_ms` jitter (~1 ms) > 20% of period |
| `RO_PRIO_RT_MONITOR` | Heartbeat | `ro_thread_sleep_ms(interval_ms)` | Fault watchdog; low-rate polling |

> **Hard rule:** Every cooperative thread MUST call one of the above mechanisms at each loop
> iteration. A cooperative thread that executes an unbounded loop without yielding is an
> **architecture violation** — it starves all other cooperative threads indefinitely.
> CI static analysis (`ro_check_coop_yield`) will flag threads registered at `RO_PRIO_RT_*`
> whose entry function contains no call to `ro_thread_yield`, `ro_thread_sleep_ms`, or `ro_timer_sync`.

### Context Switch Overhead

> ⚠️ The numbers below are **order-of-magnitude estimates** for Cortex-M4 @ 168 MHz with
> default Zephyr config. Actual values vary significantly by board, toolchain, and Kconfig.
> **Measure on your target** before committing to a control-loop budget.

| Metric | Typical order of magnitude (Cortex-M4) | RobotOS impact |
|--------|----------------------------------------|----------------|
| Thread switch latency | low single-digit µs | Must fit within `RO_PRIO_RT_CONTROL` ≤200 µs/iter budget |
| Priority-inheritance mutex overhead | sub-µs to low single-digit µs | Only on preemptive paths; measure if mutex is on critical path |
| ISR entry latency | sub-µs | Step pulse / encoder ISR; measure with logic analyser |

---

## SYS_INIT Lifecycle

### Boot Phase Order

Zephyr boots in strictly ordered phases. RobotOS registers init work at specific levels:

```
Power-on Reset
      │
      ▼
┌─────────────────────────────────────────────────────────────┐
│  PRE_KERNEL_1  (priority 0–99)                              │
│                                                             │
│  Kernel: interrupt controller, clock, SysTick              │
│  RobotOS:                                                   │
│    ro_log_backend_early_init() — UART ring buffer setup     │
│                                                             │
│  ⚠️  NO kernel services available                           │
│       No semaphore, no thread, no queue                     │
└────────────────────────┬────────────────────────────────────┘
                         │
                         ▼
┌─────────────────────────────────────────────────────────────┐
│  PRE_KERNEL_2  (priority 0–99)                              │
│                                                             │
│  Kernel: kernel data structs, heap, early device driver     │
│  RobotOS:                                                   │
│    ro_pool_sys_init() — static pool descriptor table        │
│                                                             │
│  ⚠️  No threads yet; pools/queues may be created            │
└────────────────────────┬────────────────────────────────────┘
                         │
                         ▼
┌─────────────────────────────────────────────────────────────┐
│  POST_KERNEL  (priority 0–99)                               │
│                                                             │
│  Kernel: SPI, I2C, GPIO, PWM driver init via DT             │
│  RobotOS (Framework driver bindings — NOT Kernel code):     │
│    stepper_drv_bind_all (p80) — DT stepper → handle table   │
│    servo_drv_bind_all   (p81) — DT servo/PWM → table        │
│    encoder_drv_bind_all (p82) — DT encoder counter → table  │
│    endstop_drv_bind_all (p83) — DT endstop GPIO → table     │
│  RobotOS (Adapter):                                         │
│    ro_trace             (p60) — trace ring buffer + backend │
│                                                             │
│  ✅ Threads allowed; malloc STILL forbidden                  │
└────────────────────────┬────────────────────────────────────┘
                         │
                         ▼
┌─────────────────────────────────────────────────────────────┐
│  APPLICATION  (priority 0–99)                               │
│                                                             │
│  Kernel: shell, networking, final subsystems                │
│  RobotOS:                                                   │
│    robotos_init() (p0) — creates system threads             │
│    User main() / app_init()                                 │
│                                                             │
│  ✅ All kernel services available                            │
└────────────────────────┬────────────────────────────────────┘
                         │
                         ▼
             main() / app_init() — user creates app threads
```

### RobotOS SYS_INIT Registration Table

| Module | Level | Priority | What it does | On Failure |
|--------|-------|----------|-------------|------------|
| `ro_log_backend` | `PRE_KERNEL_1` | 0 | Init UART ring buffer for early logs | Logs silently lost; non-fatal |
| `ro_pool_sys` | `PRE_KERNEL_2` | 0 | Set up static pool descriptor table | `RO_EHWINIT`; boot halts |
| `stepper_drv_bind_all` | `POST_KERNEL` | 80 | Bind DT stepper nodes to handle table | `RO_EHWINIT` per device; stored in `boot_status` |
| `servo_drv_bind_all` | `POST_KERNEL` | 81 | Bind DT servo/PWM nodes | Same as stepper |
| `encoder_drv_bind_all` | `POST_KERNEL` | 82 | Bind encoder counter devices | Same |
| `endstop_drv_bind_all` | `POST_KERNEL` | 83 | Bind endstop GPIO nodes | Same |
| `ro_trace` | `POST_KERNEL` | 60 | Init trace ring buffer, Zephyr tracing backend | Non-fatal; trace silently dropped |
| `robotos_init` | `APPLICATION` | 0 | Create `log_flush` + `state_machine` threads | `RO_EHWINIT`; boot halts |

> **Priority within a level:** lower number = earlier. Use gaps (0, 10, 11, 20) to allow
> future insertion without renumbering.

### SYS_INIT Usage Pattern

```c
// ✅ CORRECT: register module init at appropriate level
SYS_INIT(ro_pool_sys_init, PRE_KERNEL_2, 0);
SYS_INIT(ro_stepper_drv_bind, POST_KERNEL, 10);
SYS_INIT(robotos_init, APPLICATION, 0);

// ❌ WRONG: calling ro_thread_create() from PRE_KERNEL_2
static int bad_init(void) {
    ro_thread_create(...);  // kernel not ready!
    return 0;
}
SYS_INIT(bad_init, PRE_KERNEL_2, 0);
```

### Init Phase Constraints

```c
// ✅ ALLOWED at any init phase:
//   - Reading compile-time constants
//   - Populating static structs
//   - ro_pool_alloc() after PRE_KERNEL_2
//   - Zephyr device_is_ready() after POST_KERNEL
//   - ro_log_*() after PRE_KERNEL_1 (ring buffer only)

// ❌ FORBIDDEN at PRE_KERNEL_1 or PRE_KERNEL_2:
//   - ro_thread_create()       — kernel not ready
//   - ro_queue_create()        — kernel not ready
//   - k_mutex_lock()           — kernel not ready
//   - Any blocking API         — no scheduler

// ❌ FORBIDDEN at ALL init phases:
//   - malloc / free            — unconditional; violates no-malloc rule
//   - ro_thread_create()       — only robotos_init() and app may create threads
//   - Busy-waiting loops       — wastes boot budget; use SYS_INIT ordering
```

---

## IPC Primitives

### Message Queue (`k_msgq` → `ro_queue`)

Zephyr's `k_msgq` is a fixed-size, thread-safe FIFO. RobotOS wraps it in `ro_queue_t`.

**Kernel behaviour visible to Adapter:**
- `k_msgq_put()` → blocks if full (with timeout), or returns `-ENOMEM` for ISR variant
- `k_msgq_get()` → blocks if empty (with timeout)
- `k_msgq_put_nowait()` → non-blocking, ISR-safe; returns a negative errno (e.g. `-EAGAIN` or `-ENOMSG` depending on API variant); Adapter normalises to `ro_status_t` (`RO_EAGAIN`)

**Adapter translation:**
```c
// Inside ro_queue.c — the ONLY file that calls Zephyr IPC directly
#include <zephyr/kernel.h>

ro_status_t ro_queue_send(ro_queue_t* q, const void* data, uint32_t timeout_ms) {
    int ret = k_msgq_put(&q->zephyr_msgq, data,
                         timeout_ms == 0 ? K_NO_WAIT : K_MSEC(timeout_ms));
    return ro_from_zephyr(ret);   // -EAGAIN, -ETIMEDOUT → ro_status_t
}
```

**Error model — `ro_status_t` + `RO_TRY`:**

All Adapter IPC functions return `ro_status_t` (a `typedef int32_t`). The contract:
- `RO_OK` (0) = success
- Any **negative** value = failure (`RO_ETIMEDOUT`, `RO_EAGAIN`, `RO_EINVAL`, …)
- Callers MUST check the return value — ignoring it is an architecture violation

```c
// ✅ Propagate errors with RO_TRY (defined in ro_status.h):
ro_status_t init_ipc(void) {
    RO_TRY(ro_queue_send(cmd_q, &cmd, 100));   // returns on any non-OK
    RO_TRY(ro_pool_alloc(msg_pool));
    return RO_OK;
}

// ❌ WRONG — silently discarding errors:
ro_queue_send(cmd_q, &cmd, 100);   // return value ignored
```

> **`RO_TRY(expr)`** expands to: `{ ro_status_t _s = (expr); if (_s != RO_OK) return _s; }`
> It is the **only** approved early-return pattern for chained Adapter calls.

### Mutex (`k_mutex` → `ro_mutex`)

Zephyr's `k_mutex` implements **priority inheritance** automatically. This is required for
correctness whenever a high-priority thread may wait on a resource held by a lower-priority
thread.

**RobotOS rule:** Mutexes are **only used on preemptive threads** (`RO_PRIO_FRAMEWORK` and
below). Cooperative threads must NEVER acquire a mutex — they cannot yield inside a mutex wait,
which creates deadlock risk.

### Semaphore (`k_sem` → `ro_sem`)

Used for signalling between ISR and thread contexts:
```c
// ISR signals event (no blocking)
k_sem_give(&sem);   // Always succeeds; ISR-safe

// Thread waits for event
k_sem_take(&sem, K_MSEC(timeout));  // Blocks until signal or timeout
```

---

## Memory Sub-system

### Static Memory Layout (Runtime)

```
SRAM  0x20000000
  ┌─────────────────────────────────────────┐
  │  .bss / .data (global vars)          │   ← ro_pool_t, ro_queue_t descriptors live here
  ├─────────────────────────────────────────┤
  │  Pool backing buffers                │   ← static uint8_t pool_buf[N]; at file scope
  │  (declared at file scope)            │     msg_pool_buf, stepper_pool_buf, ...
  ├─────────────────────────────────────────┤
  │  Thread stacks (static arrays)       │   ← static uint8_t ctrl_stack[2048]; etc.
  │  Per-thread, file or module scope    │     Must outlive the thread
  ├─────────────────────────────────────────┤
  │         Kernel Heap (reserved)       │
  │  ⚠️  RobotOS NEVER uses heap.        │
  │      No malloc/free at any phase.    │
  └─────────────────────────────────────────┘
SRAM  0x2001FFFF

FLASH 0x08000000
  .text (code) + .rodata (const + DT) → read-only, never written at runtime
```

**Key rule:** `ro_pool_t` backing buffers and thread stacks are **all declared as `static`
global arrays**. They are zero-initialised before `main()`. Lifetime = process lifetime.
No allocation is needed or allowed at runtime.

Zephyr's `k_mem_slab` provides a fixed-block allocator backed by a statically declared buffer.
RobotOS wraps this as `ro_pool_t`.

**Key kernel constraints:**
- Buffer must be declared with `K_MEM_SLAB_DEFINE` or equivalent compile-time macro
- Block size must be word-aligned
- `k_mem_slab_alloc()` with `K_NO_WAIT` is ISR-safe
- `k_mem_slab_alloc()` with timeout is NOT ISR-safe

**RobotOS pool lock modes map to these constraints:**

| `ro_pool_lock_t` | Kernel primitive | ISR-safe |
|----------------|-----------------|---------|
| `LOCK_NONE` | Direct pointer bump (no kernel) | ✅ |
| `LOCK_ATOMIC` | Atomic CAS on free-list head | ✅ |
| `LOCK_MUTEX` | `k_mutex` | ❌ (never in ISR) |

### No Heap After Init (Hard Rule)

```
❌ k_malloc() / malloc() — FORBIDDEN after robotos_init() returns — CI error
❌ k_free()  / free()    — FORBIDDEN in realtime paths              — CI error
✅ ro_pool_alloc()        — only legal dynamic allocation in realtime
✅ ro_pool_alloc_isr()    — ISR-safe variant (LOCK_NONE / LOCK_ATOMIC only)
```

The Zephyr heap (`CONFIG_HEAP_MEM_POOL_SIZE`) may be used **only during SYS_INIT phases**
(e.g. Zephyr subsystem setup). After `robotos_init()` returns, all allocations must come from
`ro_pool_t` instances backed by static arrays.

**Adapter API ISR-safe allocation contract:**

| Adapter API | Context | Blocking | Pool lock required |
|-------------|---------|----------|--------------------|
| `ro_pool_alloc(pool)` | Thread only | Never blocks — returns NULL if exhausted | Any `ro_pool_lock_t` |
| `ro_pool_alloc_isr(pool)` | Thread **or** ISR | Never blocks | `LOCK_NONE` or `LOCK_ATOMIC` only |
| `ro_pool_free(pool, ptr)` | Thread only | Never blocks | Any |
| `ro_pool_free_isr(pool, ptr)` | Thread **or** ISR | Never blocks | `LOCK_NONE` or `LOCK_ATOMIC` only |
| `ro_queue_send_isr(q, data)` | Thread **or** ISR | Never blocks | Internal `irq_lock` critical section |
| `ro_queue_recv_isr(q, data)` | Thread **or** ISR | Never blocks | Internal `irq_lock` critical section |

> **Invariant:** Calling `ro_pool_alloc()` (non-ISR) or `ro_mutex_lock()` from ISR context
> triggers `RO_ASSERT` in debug builds and is undefined behaviour in release builds.
> `CONFIG_ASSERT=y` must be set in all debug and CI profiles.

---

## Device Tree & Driver Model

### Role of Device Tree in RobotOS

The Device Tree (DT) is Zephyr's compile-time hardware description language. RobotOS uses it
exclusively through the **Framework Driver layer** — application and Adapter public API never
reference DT nodes.

```
Hardware description (app.overlay / board.dts)
          │ compile time
          ▼
  Generated C macros (DT_NODELABEL, DEVICE_DT_GET, ...)
          │
          ▼  consumed ONLY by:
  src/framework/drivers/*.c    (stepper_drv_bind.c, servo_drv_bind.c, ...)
          │
          ▼  produces:
  ro_stepper_t handle table    (populated at POST_KERNEL)
          │
          ▼  accessed by application via:
  stepper_get("stepper0")   // Framework public API — string DT label, no ro_ prefix
```

**String label lookup contract:**
- The `dt_label` string MUST be a **string literal** (pointer to `.rodata`); no heap, no stack-allocated string.
- The internal lookup table is a **statically-sized array** built at `POST_KERNEL` bind time.
- Lookup is **O(n)** linear scan over the pool (n ≤ `CONFIG_RO_MAX_STEPPERS`, default 8) — deterministic worst-case, no dynamic allocation, no hashing.
- If the label is not found, `stepper_get()` returns `NULL` immediately (no blocking, no retry).
- **Forward compatibility:** The v0.1-alpha MVP uses string labels for readability. A future
  revision may add compile-time numeric IDs as an alternative (O(1) lookup, zero string compare
  at runtime). The public API signature will remain `stepper_get(const char*)` until that
  transition is explicitly decided.

### Permitted DT Usage

| File type | `DEVICE_DT_GET` | `DT_NODELABEL` | `device_is_ready` | `<zephyr/device.h>` |
|-----------|:--------------:|:--------------:|:-----------------:|:--------------------:|
| `src/adapter/zephyr/*.c` | ✅ (peripheral wrap) | ✅ | ✅ | ✅ |
| `src/framework/drivers/*.c` | ✅ | ✅ | ✅ | ✅ |
| `src/framework/*.c` (non-driver) | ❌ | ❌ | ❌ | ❌ |
| `include/robotos/*.h` | ❌ | ❌ | ❌ | ❌ |
| `app/**` | ❌ | ❌ | ❌ | ❌ |

### Device Binding Pattern (Framework Driver)

```c
// src/framework/drivers/stepper_drv_bind.c
#include <zephyr/device.h>          // ← allowed (framework driver)
#include <zephyr/devicetree.h>      // ← allowed (framework driver)
#include <robotos/stepper.h>        // ← framework public header (no ro_ prefix)

static const struct device* stepper_dev =
    DEVICE_DT_GET(DT_NODELABEL(stepper0));

static int stepper_drv_bind(stepper_t* st) {
    if (!device_is_ready(stepper_dev)) {
        return RO_EHWINIT;
    }
    st->dev = stepper_dev;
    return RO_OK;
}
SYS_INIT(stepper_drv_bind_all, POST_KERNEL, 80);
```

### Device Tree Overlay Example (STM32F4 CNC Demo)

```dts
/* app/cnc_demo/app.overlay */
/ {
    stepper0: stepper0 {
        compatible = "robotos,stepper";
        step-gpios  = <&gpioa 0 GPIO_ACTIVE_HIGH>;
        dir-gpios   = <&gpioa 1 GPIO_ACTIVE_HIGH>;
        enable-gpios= <&gpioa 2 GPIO_ACTIVE_LOW>;
        steps-per-rev = <200>;
        max-speed     = <2000>;       /* steps/sec */
        acceleration  = <1000>;       /* steps/sec² */
        status = "okay";
    };

    endstop0: endstop0 {
        compatible = "robotos,endstop";
        gpios = <&gpiob 0 (GPIO_ACTIVE_LOW | GPIO_PULL_UP)>;
        status = "okay";
    };
};
```

---

## Tracing & Observability Hooks

### Zephyr Tracing Infrastructure

Zephyr provides a compile-time tracing hook system. When `CONFIG_TRACING=y`, every thread
switch, ISR entry/exit, and syscall is funnelled through `SYS_PORT_TRACING_*` macros.
RobotOS attaches its own events to this same infrastructure.

```
Thread / ISR                      Trace Backend
     │                                  │
     │ SYS_PORT_TRACING_FUNC(...)       │
     ▼                                  │
┌──────────────────┐                    │
│  Zephyr Trace    │                    │
│  Dispatch Layer  │                    │
└────────┬─────────┘                    │
         │ (CONFIG_TRACING=y)           │
         ▼                              │
┌──────────────────┐                    │
│  CTF Backend     │────────────────────►  Ring buffer → shell dump
│  (or UART dump)  │                    │  → babeltrace / TraceCompass
└──────────────────┘                    │
```

### Built-in Kernel Trace Events (auto-captured)

| Event | Zephyr Hook | RobotOS Use |
|-------|-------------|-------------|
| Thread switch in | `sys_port_trace_thread_switched_in` | Scheduler jitter analysis |
| Thread switch out | `sys_port_trace_thread_switched_out` | CPU utilisation per thread |
| Thread create | `sys_port_trace_thread_create` | Verify Thread Roster |
| ISR enter | `sys_port_trace_isr_enter` | Step pulse ISR timing |
| ISR exit | `sys_port_trace_isr_exit` | ISR overhead |
| Syscall enter/exit | `sys_port_trace_syscall_enter/exit` | API call depth |

### Custom RobotOS Trace Events

```c
/* Declared in ro_trace.h — compiled out when CONFIG_TRACING=n */
SYS_PORT_TRACING_FUNC(ro_stepper_move,   const char* name, int32_t steps);
SYS_PORT_TRACING_FUNC(ro_pid_update,     ro_pid_t* pid, float error);
SYS_PORT_TRACING_FUNC(ro_deadline_miss,  const char* name,
                                         uint64_t elapsed_us, uint64_t budget_us);
```

### RobotOS Deadline Monitor Contract (Adapter-level)

Deadline monitoring is implemented entirely in the **Adapter Layer** (`ro_deadline.c`). It uses
no dedicated thread — it is a pair of inline calls around a real-time work unit:

```
CPU time ──────────────────────────────────────────────────────────►

  ro_deadline_begin(&d)         ro_deadline_end(&d)
        │                              │
        ▼                              ▼
  ──────┬──────────────────────────────┬───────────────────────────
        │  real-time work (PID, step)  │
        └──────────────────────────────┘
             ← budget_us (e.g. 1000 µs) →

  If elapsed > budget_us:
    SYS_PORT_TRACING_FUNC(ro_deadline_miss, name, elapsed_us, budget_us)
    atomic_inc(&miss_counter[name_hash])   ← lock-free, ISR-safe
    return false
  else:
    return true
```

**Adapter contract for deadline monitoring:**

| Function | Context | Blocking | Side-effects on miss |
|----------|---------|----------|----------------------|
| `ro_deadline_begin(d)` | Thread or ISR | Never | Records `ro_time_us()` in `d->_start_us` |
| `ro_deadline_end(d)` | Thread or ISR | Never | Emits trace event + increments atomic miss counter |
| `ro_deadline_miss_count(name)` | Thread only | Never | Read-only; returns cumulative miss count |
| `ro_deadline_miss_reset(name)` | Thread only | Never | Resets counter to 0 |

**Zephyr backend binding:**  
When `CONFIG_TRACING=y`, `ro_deadline_miss` events appear in the CTF stream and are visible
in babeltrace / TraceCompass. When `CONFIG_TRACING=n`, the trace macro compiles to zero
instructions — the miss counter increment (atomic) still occurs.

**CI assertion:** Every `RO_PRIO_RT_CONTROL` thread entry function MUST contain exactly one
`ro_deadline_begin`/`ro_deadline_end` pair. The `ro_check_deadline_pairs` CI step verifies
this by static analysis of the thread roster.

---

### Trace Configuration (`prj.conf`)

```conf
# Minimal tracing (thread switch + ISR only):
CONFIG_TRACING=y
CONFIG_TRACING_CTF=y
CONFIG_TRACING_THREAD=y
CONFIG_TRACING_ISR=y

# Full tracing (adds syscall + custom RobotOS events including deadline begin/end/miss):
CONFIG_TRACING_SYSCALL=y
CONFIG_RO_TRACING_CUSTOM=y
```

---

## Customization Policy

### What RobotOS Changes in Zephyr

RobotOS follows an **upstream-first** policy:

| Change type | Policy | Rationale |
|------------|--------|-----------|
| Kconfig options (`prj.conf`) | ✅ Freely set | Standard Zephyr mechanism |
| Device Tree overlays (`app.overlay`) | ✅ Freely set | Standard Zephyr mechanism |
| New `SYS_INIT` registrations | ✅ Allowed | Standard Zephyr mechanism |
| New device bindings (`dts/bindings/*.yaml`) | ✅ Allowed | Standard Zephyr mechanism |
| Kernel source patches | ❌ Forbidden | Breaks upstream sync |
| Forking `zephyr/` submodule | ❌ Forbidden | Breaks upstream sync |
| New `k_*` primitives | ❌ Forbidden | Use existing; propose upstream if needed |

### Kconfig Profile Requirements

RobotOS requires minimum Kconfig values per profile:

| Profile | `CONFIG_NUM_COOP_PRIORITIES` min | `CONFIG_NUM_PREEMPT_PRIORITIES` min |
|---------|:---------------------------------:|:------------------------------------:|
| `RO_PROFILE_MIN` | 4 | 4 |
| `RO_PROFILE_STD` _(default)_ | 8 | 8 |
| `RO_PROFILE_DEV` | 16 | 16 |

A CMake check in `cmake/robotos_profile_check.cmake` raises `FATAL_ERROR` if the active
profile's minimums are not met.

---

## What Adapter Layer Exposes (Contract Summary)

The Kernel Layer is entirely hidden by the Adapter Layer. From every layer above Adapter,
Zephyr does not exist. The contract:

| Kernel concept | Adapter abstraction | Caller sees |
|---------------|-------------------|-------------|
| `k_thread` | `ro_thread_t` (opaque) | `ro_thread_create()` / `ro_thread_destroy()` |
| Thread priority (int) | `ro_priority_t` (struct, opaque) | `RO_PRIO_RT_CONTROL` etc. |
| `k_uptime_get()` | — | `ro_time_ms()` |
| `k_cycle_get_32()` | — | `ro_time_us()` |
| `k_msgq` | `ro_queue_t` (opaque) | `ro_queue_create/send/recv()` |
| `k_mem_slab` | `ro_pool_t` (opaque) | `ro_pool_create/alloc/free()` |
| `k_mutex` | `ro_mutex_t` (opaque) | `ro_mutex_lock/unlock()` |
| Zephyr errno (`-ETIMEDOUT`) | — | `ro_status_t` (`RO_ETIMEDOUT`) |
| `device_is_ready` / `DEVICE_DT_GET` | Framework driver internal | `stepper_get(dt_label)` |
| `SYS_PORT_TRACING_FUNC` | `ro_trace.h` macros | `RO_TRACE_*()` (compiled out if disabled) |

---

**Document Status:** 🚧 Living Document  
**Layer:** Zephyr Kernel (0% portable)  
**Related:** [ADAPTER_LAYER.md](ADAPTER_LAYER.md) | [FRAMEWORK_LAYER.md](FRAMEWORK_LAYER.md) | [ARCHITECTURE.md](ARCHITECTURE.md)  
**Last Review:** 2026-03-05
