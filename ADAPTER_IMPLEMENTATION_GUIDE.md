# Robot Adapter Layer — Implementation Guide

> **Version:** v0.1-alpha  
> **Audience:** C developers implementing RobotOS  
> **Status:** Guide for implementation phase  
> **Related:** ADAPTER_LAYER.md (spec), FRAMEWORK_LAYER.md (consumer spec)

---

## 1. Purpose of the Adapter Layer

### What It Is
The Adapter layer is RobotOS's **portable RTOS/hardware abstraction**. It wraps:
- Threading and synchronization (Zephyr `k_thread`, `k_sem`, `k_msgq`, etc.)
- Timing and real-time services (monotonic clock, periodic timers)
- Hardware I/O (GPIO, PWM, I2C, SPI)
- Diagnostics (logging, tracing, deadline monitoring)

### What It Must Hide
- All `<zephyr/*>` includes must be confined to `src/adapter/zephyr/*.c`
- Public headers expose ONLY `ro_*` APIs
- No Zephyr types, macros, or enums leak into Framework layer
- Internal data structures are opaque (`typedef struct ro_sem ro_sem_t;`)

### What It Must Never Do
- ❌ Add robotics domain logic (motors, encoders, state machines)
- ❌ Create worker threads autonomously
- ❌ Allocate from heap after `robotos_init()` completes (unless explicitly allowed)
- ❌ Hide ownership or thread creation by caller
- ❌ Implement "smart" behavior beyond the spec contract

### Portability Promise
**Backend swap must require ONLY touching `src/adapter/zephyr/*.c`.**

If implementing for FreeRTOS:
- Public headers stay identical
- Zephyr-specific logic is replaced
- Framework layer is rebuilt without modification

---

## 2. Implementation Goals

### Concrete Success Criteria

**Adapter is "done" when:**

1. ✅ **All core primitives exist and compile**
   - ro_status, ro_thread, ro_time, ro_timer, ro_sem, ro_queue, ro_pool, ro_mutex
   
2. ✅ **Zero Zephyr leakage in public headers**
   - grep: no `k_`, no `<zephyr/`
   
3. ✅ **All ro_status_t contracts honored**
   - RO_OK means success
   - RO_ETIMEDOUT means timeout expired
   - RO_EAGAIN means non-blocking poll failed
   - RO_EBUSY means resource in use
   - RO_ENOMEM means allocation failed
   
4. ✅ **ISR-safe variants are bounded and non-blocking**
   - `_isr` functions never sleep, block, or allocate
   - No hidden dependencies on OS state
   
5. ✅ **Framework can build and run on top without confusion**
   - Contracts clear and testable
   - No hidden assumptions

### First Usable Baseline

Minimum viable Adapter for Framework to begin integration:

```
REQUIRED:
├── ro_status.h       (error codes)
├── ro_thread.h       (thread creation, priorities)
├── ro_time.h         (monotonic clock)
├── ro_timer.h        (periodic timer with sync)
├── ro_sem.h          (binary & counting semaphores)
├── ro_queue.h        (message queue IPC)
├── ro_pool.h         (memory pool allocation)
└── ro_mutex.h        (priority-inheriting mutex)

THEN:
├── ro_gpio.h         (digital I/O + ISR)
├── ro_pwm.h          (PWM control)
├── ro_log.h          (structured logging)
└── ro_trace.h        (event tracing)

LAST:
├── ro_i2c.h, ro_spi.h
└── ro_deadline.h
```

---

## 3. Implementation Order (CRITICAL)

Implement **strictly in this order**. Earlier modules are dependencies for later ones.

### Phase A: Core Type System (1 day)

#### 1. `ro_status.h`
**Why first:** All other modules return `ro_status_t`. Must exist before anything else.

**Deliverable:**
```c
// include/robotos/ro_status.h
typedef int32_t ro_status_t;

#define RO_OK              0
#define RO_ETIMEDOUT      -1
#define RO_EAGAIN         -2
#define RO_EBUSY          -3
#define RO_ECANCELED      -4
#define RO_EINVAL         -10
#define RO_ENOMEM         -11
#define RO_ENODEV         -12
#define RO_ENOTSUP        -13
#define RO_EOVERFLOW      -14
#define RO_EFAIL          -20
#define RO_ENOTREADY      -21
#define RO_EHWINIT        -22
#define RO_EBOOTSEQ       -23

#define RO_SUCCEEDED(s)   ((s) == RO_OK)
#define RO_FAILED(s)      ((s) != RO_OK)
#define RO_IS_FATAL(s)    ((s) <= RO_EFAIL)
#define RO_TRY(expr)      do { ro_status_t _s = (expr); if (_s != RO_OK) return _s; } while (0)
```

**Test:** None (type-only module).

---

#### 2. Internal Conventions
**Why second:** Establish patterns for all remaining modules.

**Define in `src/adapter/internal.h`:**
- How to declare opaque handles
- How to translate Zephyr errors to `ro_status_t`
- Global init order during `robotos_init()`

---

### Phase B: Core Runtime Primitives (4–5 days)

#### 3. `ro_thread.h`
**Role:** Create and manage kernel threads with priority levels.  
**Maps to:** `k_thread` (Zephyr), pthread (generic), FreeRTOS tasks (FreeRTOS)

**Required public functions:**
```c
typedef struct ro_priority { int16_t v; } ro_priority_t;
typedef struct ro_thread ro_thread_t;

// Lifecycle
ro_thread_t* ro_thread_create(const ro_thread_config_t* cfg);
void         ro_thread_destroy(ro_thread_t* t);

// Priority constants
#define RO_PRIO_RT_PULSE    ((ro_priority_t){ .v = -16 })
#define RO_PRIO_RT_CONTROL  ((ro_priority_t){ .v = -10 })
#define RO_PRIO_RT_MONITOR  ((ro_priority_t){ .v =  -2 })
#define RO_PRIO_APP         ((ro_priority_t){ .v =   0 })
```

**Context rules:**
- Can be called from thread context only, not ISR
- Thread creation happens before running; no dynamic thread injection

**Memory model:**
- Thread struct allocated from internal pool
- Stack allocated by caller (static allocation)
- CONFIG_RO_MAX_THREADS determines pool size

**Common mistakes:**
- ❌ Creating threads from ISR
- ❌ Using Zephyr k_priority directly instead of ro_priority_t
- ❌ Dynamic stack allocation

**Unit tests:**
1. Create/destroy success
2. Cannot exceed CONFIG_RO_MAX_THREADS
3. Priority levels map correctly
4. Thread entry function runs

**Integration checks:**
- Thread can read ro_time_us()
- Thread can create semaphore and signal

---

#### 4. `ro_time.h`
**Role:** Monotonic clock and blocking sleep (low frequency only).  
**Maps to:** `k_uptime_ticks()`, `k_sleep()`, `get_core_clock_cycles()`

**Required public functions:**
```c
uint64_t    ro_time_us(void);
void        ro_thread_sleep_ms(uint32_t ms);
uint32_t    ro_time_ms(void);
```

**Context rules:**
- `ro_time_us()` usable from thread and ISR
- `ro_thread_sleep_ms()` forbidden in ISR

**Memory model:**
- No persistent state needed (wraps kernel uptime)

**Common mistakes:**
- ❌ Using thread_sleep_ms() in loops at ≥200 Hz (must use ro_timer instead)
- ❌ Assuming clock tick = 1 µs (depends on kernel config)

**Unit tests:**
1. ro_time_us() is monotonically increasing
2. ro_time_us() returns reasonable values (reasonable = > 0 at boot → very large over time)
3. ro_thread_sleep_ms(10) blocks approximately 10 ms

---

#### 5. `ro_timer.h`
**Role:** Bounded periodic timer for real-time control loops (≥200 Hz).  
**Maps to:** `k_timer`

**Required public functions:**
```c
typedef struct ro_timer ro_timer_t;

ro_status_t ro_timer_start_periodic(ro_timer_t* t, uint32_t period_us);
void        ro_timer_stop(ro_timer_t* t);
ro_status_t ro_timer_sync(ro_timer_t* t);
```

**Context rules:**
- `ro_timer_sync()` must be called from thread at predictable loop rate
- Cannot be called from ISR

**Memory model:**
- Timer struct is caller-owned static (declared on stack or global scope)
- Internal timer ISR wakes thread on each tick

**Critical implementation detail:**
- `ro_timer_sync()` returns RO_ETIMEDOUT if tick fired more than once since last sync (overrun detection)
- Must use bounded ISR callback, not polling

**Common mistakes:**
- ❌ Calling sync() from ISR
- ❌ Using for timing < 1 kHz (use lower-level primitives)
- ❌ Ignoring overrun return code

**Unit tests:**
1. Start periodic timer at 1 kHz
2. Call sync() in loop; verify wake timing
3. Detect overrun when loop slow

---

#### 6. `ro_sem.h`
**Role:** Binary and counting semaphores for signaling and resource bounding.  
**Maps to:** `k_sem`

**Required public functions:**
```c
typedef struct ro_sem ro_sem_t;

ro_sem_t*   ro_sem_create(uint32_t initial_count, uint32_t max_count);
void        ro_sem_destroy(ro_sem_t* s);
ro_status_t ro_sem_take(ro_sem_t* s, uint32_t timeout_ms);
ro_status_t ro_sem_give(ro_sem_t* s);
ro_status_t ro_sem_give_isr(ro_sem_t* s);
```

**Context rules:**
- `ro_sem_take()` blocks; thread context only
- `ro_sem_give_isr()` non-blocking; ISR-safe
- Cannot call take() from multiple threads on same semaphore simultaneously (non-reentrant)

**Memory model:**
- Semaphore allocated from internal pool
- Pool size: CONFIG_RO_SEM_POOL_SIZE
- No heap allocation

**Critical semantics:**
- Semaphore is NOT "broadcast" style
- Only ONE waiter wakes per give()
- Framework contracts depend on this

**Common mistakes:**
- ❌ Calling give_isr() with count at max_count (returns RO_EAGAIN instead of success)
- ❌ Having multiple threads wait on same semaphore expecting all to wake
- ❌ Using take() in ISR (blocks)

**Unit tests:**
1. Binary semaphore (0, 1) blocks and wakes
2. Counting semaphore allows N waiters
3. give_isr() is non-blocking and returns immediately
4. Overflow (give with count at max) returns RO_EAGAIN
5. Timeout behavior: RO_ETIMEDOUT on expiry, RO_OK if given before timeout
6. Single-waiter semantics

---

#### 7. `ro_queue.h`
**Role:** Multi-producer/consumer message queue with configurable depth.  
**Maps to:** `k_msgq`

**Required public functions:**
```c
typedef struct ro_queue ro_queue_t;

ro_queue_t* ro_queue_create(size_t item_size, size_t capacity);
void        ro_queue_destroy(ro_queue_t* q);
ro_status_t ro_queue_send(ro_queue_t* q, const void* data, uint32_t timeout_ms);
ro_status_t ro_queue_recv(ro_queue_t* q, void* data, uint32_t timeout_ms);
ro_status_t ro_queue_send_isr(ro_queue_t* q, const void* data);
ro_status_t ro_queue_recv_isr(ro_queue_t* q, void* data);
```

**Context rules:**
- send/recv block in thread context
- send_isr/recv_isr non-blocking in ISR context
- receiver waits on recv(), sender waits on send() (both blocking variants)

**Memory model:**
- Backing buffer allocated from internal IPC slab
- Global pool: CONFIG_RO_IPC_SLAB_SIZE
- No per-queue heap

**Common mistakes:**
- ❌ Calling blocking send/recv from ISR
- ❌ Assuming _isr returns same semantics as blocking (it's non-blocking, returns RO_EAGAIN)
- ❌ Queue with item_size 0

**Unit tests:**
1. Send/recv cycle
2. Timeout on empty recv
3. Timeout on full send
4. ISR variants are non-blocking
5. FIFO ordering preserved

---

#### 8. `ro_pool.h`
**Role:** Static memory pool for bounded, deterministic allocation.  
**Maps to:** `k_mem_slab`

**Required public functions:**
```c
typedef enum {
    RO_POOL_LOCK_NONE   = 0,
    RO_POOL_LOCK_ATOMIC = 1,
    RO_POOL_LOCK_MUTEX  = 2,
} ro_pool_lock_t;

typedef struct ro_pool ro_pool_t;

ro_pool_t* ro_pool_create(const ro_pool_config_t* cfg);
void*      ro_pool_alloc(ro_pool_t* pool);
void*      ro_pool_alloc_isr(ro_pool_t* pool);
void       ro_pool_free(ro_pool_t* pool, void* ptr);
void       ro_pool_free_isr(ro_pool_t* pool, void* ptr);
```

**Context rules:**
- alloc() blocks if pool exhausted (thread context)
- alloc_isr() returns NULL if exhausted (never blocks)
- free() can be called from thread or ISR

**Memory model:**
- Backing buffer is caller-provided (static)
- Pool header allocated from internal pool
- Lock mode determines thread-safety

**Lock modes:**
- `LOCK_NONE`: Single owner only (fastest)
- `LOCK_ATOMIC`: Lock-free CAS (multi-thread + ISR safe)
- `LOCK_MUTEX`: Thread-only (priority inherited)

**Common mistakes:**
- ❌ Using LOCK_MUTEX with ISR path (forbidden)
- ❌ alloc() exhaustion blocks indefinitely (caller must handle timeout or use alloc_isr)
- ❌ Block size misalignment causing waste

**Unit tests:**
1. Alloc/free success
2. Exhaustion with different lock modes
3. ISR variants never block

---

#### 9. `ro_mutex.h`
**Role:** Priority-inheriting mutual exclusion for non-RT sections.  
**Maps to:** `k_mutex`

**Required public functions:**
```c
typedef struct ro_mutex ro_mutex_t;

ro_mutex_t* ro_mutex_create(void);
void        ro_mutex_destroy(ro_mutex_t* m);
ro_status_t ro_mutex_lock(ro_mutex_t* m, uint32_t timeout_ms);
void        ro_mutex_unlock(ro_mutex_t* m);
```

**Context rules:**
- Forbidden in ISR
- Forbidden in cooperative RT threads (RO_PRIO_RT_*)
- Allowed in preemptive threads only (RO_PRIO_APP or preemptive variants)

**Memory model:**
- Mutex allocated from internal pool
- Pool size: CONFIG_RO_MUTEX_POOL_SIZE

**Common mistakes:**
- ❌ Using mutex in RT thread (causes priority inversion)
- ❌ Using mutex in ISR (assertion failure)
- ❌ Nested mutex acquisition without careful deadlock analysis

**Unit tests:**
1. Lock/unlock success
2. Cannot lock from ISR (assert)
3. Timeout behavior

---

### Phase C: Hardware Abstraction (2–3 days)

#### 10. `ro_gpio.h`
**Role:** Digital input/output and interrupt callbacks.  
**Maps to:** Zephyr GPIO driver or vendor HAL

**Required functions:**
```c
typedef struct ro_gpio ro_gpio_t;

ro_gpio_t*  ro_gpio_get(const char* dt_label);
void        ro_gpio_put(ro_gpio_t* g);
ro_status_t ro_gpio_set(ro_gpio_t* g, bool value);
bool        ro_gpio_get(const ro_gpio_t* g);
ro_status_t ro_gpio_set_callback(ro_gpio_t* g, ro_gpio_callback_t cb, void* user);
```

**Context rules:**
- get/put/set in thread context
- callback fired in ISR context, must be _isr-safe

**Common mistakes:**
- ❌ Blocking in GPIO ISR callback
- ❌ Calling ro_gpio_set() from ISR (use queued update or atomic flag)

**Unit tests:**
1. Set/get success
2. Callback fires and calls _isr API safely

---

#### 11. `ro_pwm.h`
**Role:** PWM signal generation.  
**Maps to:** Zephyr PWM driver

**Required functions:**
```c
typedef struct ro_pwm ro_pwm_t;

ro_pwm_t*   ro_pwm_get(const char* dt_label);
void        ro_pwm_put(ro_pwm_t* p);
ro_status_t ro_pwm_set_pulse_us(ro_pwm_t* p, uint32_t pulse_us);
ro_status_t ro_pwm_set_period_us(ro_pwm_t* p, uint32_t period_us);
```

**Context rules:**
- Can be called from thread or ISR (non-blocking)

**Unit tests:**
1. Set pulse and read back
2. PWM output toggles at expected frequency

---

### Phase D: Diagnostics (1 day)

#### 12. `ro_log.h`
**Role:** Structured logging with level filtering.  
**Maps to:** Zephyr logging or stdio

**Required functions:**
```c
#define RO_LOG_DEBUG(fmt, ...)
#define RO_LOG_INFO(fmt, ...)
#define RO_LOG_WARN(fmt, ...)
#define RO_LOG_ERROR(fmt, ...)
```

**Context rules:**
- Usable from thread and ISR (non-blocking on modern loggers)
- May lose messages in overload (acceptable for diagnostics)

**Unit tests:**
- Minimal (log output channels vary; verify API exists)

---

#### 13. `ro_trace.h`
**Role:** Low-overhead event tracing for timing analysis.  
**Maps to:** Zephyr tracing or stub

**Unit tests:**
- Minimal (trace is observability layer)

---

#### 14. `ro_deadline.h`
**Role:** Deadline window monitoring.  
**Maps to:** ro_time_us() with inline tracking

**Required functions:**
```c
typedef struct {
    uint64_t budget_us;
    const char* name;
    uint64_t _start_us;
} ro_deadline_t;

void ro_deadline_begin(ro_deadline_t* d);
bool ro_deadline_end(ro_deadline_t* d);
```

**Implementation:**
- Pure inline, no persistent state per-deadline
- Tracing only on miss (no overhead on success)

**Unit tests:**
- Validate miss detection on overrun

---

### Phase E: I2C/SPI (Optional for Phase 1)

Skip until Framework needs sensor reading. Same patterns as GPIO/PWM.

---

## 4. Per-Module Implementation Template

Use this template for **every** Adapter module implementation:

### Template Structure

```
## [Module Name]

### 4.1 Role
[One sentence: what does this module do?]

### 4.2 Public Header
[Key function signatures and types]

### 4.3 Zephyr Primitive It Wraps
[k_sem → ro_sem, etc.]

### 4.4 Required Public Functions
[List all functions that must exist]

### 4.5 Context Rules
[Thread / ISR / forbidden combinations]

### 4.6 Memory Model
[Static vs dynamic, pool-based, etc.]

### 4.7 Error Handling
[How ro_status_t codes map to this module]

### 4.8 Common Implementation Mistakes
[Pitfalls specific to this module]

### 4.9 Required Unit Tests
[Minimal test matrix]

### 4.10 Integration Checks
[How this module interacts with others]

###4.11 Implementation Notes
[Concrete Zephyr mapping hints]
```

---

## 5. Public Header Rules

### Hard Rules

1. **No `<zephyr/*>` anywhere**
   ```c
   // ❌ WRONG
   #include <zephyr/kernel.h>
   
   // ✅ RIGHT
   #include <robotos/ro_sem.h>
   ```

2. **Opaque structures**
   ```c
   // ✅ RIGHT
   typedef struct ro_sem ro_sem_t;  // defined in .c only
   
   // ❌ WRONG
   typedef struct {
       struct k_sem kobj;  // leaks backend
   } ro_sem_t;
   ```

3. **Only ro_* names in public API**
   ```c
   // ✅ RIGHT
   ro_status_t ro_queue_send(ro_queue_t* q, ...);
   
   // ❌ WRONG
   int k_queue_send_wrapper(struct ro_queue_kobj* q, ...);
   ```

4. **Error translation happens inside .c only**
   ```c
   // ✅ in ro_queue.c
   int zephyr_ret = k_msgq_put(...);
   if (zephyr_ret != 0) {
       return (zephyr_ret == -ENOMSG) ? RO_EAGAIN : RO_EFAIL;
   }
   return RO_OK;
   
   // ❌ NOT in header
   #define RO_QUEUE_ZEPHYR_ENOMSG -ENOMSG
   ```

---

## 6. Zephyr Mapping Rules

### Rule Set for Implementation Files

**Only `src/adapter/zephyr/*.c` may include Zephyr headers.**

Mapping Strategy:

```c
// src/adapter/zephyr/ro_sem.c
#include <zephyr/kernel.h>  // ✅ permitted here

#include <robotos/ro_sem.h>
#include "../internal.h"

struct ro_sem {
    struct k_sem kobj;  // Zephyr primitive hidden inside
};

ro_status_t ro_sem_give_isr(ro_sem_t* s) {
    k_sem_give(&s->kobj);  // Zephyr call
    return RO_OK;
}
```

**Error Translation Pattern:**

```c
// Example: Zephyr uses -ETIMEDOUT, RobotOS uses RO_ETIMEDOUT
static ro_status_t zephyr_error_to_ro(int zephyr_code) {
    switch (zephyr_code) {
        case 0:           return RO_OK;
        case -ETIMEDOUT:  return RO_ETIMEDOUT;
        case -ENOMEM:     return RO_ENOMEM;
        case -EBUSY:      return RO_EBUSY;
        default:          return RO_EFAIL;
    }
}
```

---

## 7. Function-Level Implementation Rules

### MUST
✅ **Validate inputs where contract mandates**
```c
// If NULL is invalid:
if (q == NULL) return RO_EINVAL;
```

✅ **Return correct `ro_status_t`**
```c
// Never return Zephyr codes
return RO_OK;  // ✅
return k_RESULT_OK;  // ❌
```

✅ **Preserve timeout semantics**
```c
// timeout_ms == 0 → non-blocking poll
// timeout_ms > 0 → wait with timeout
// timeout_ms == UINT32_MAX (or special constant) → infinite wait (if supported)
```

✅ **Maintain deterministic behavior on RT paths**
```c
// ro_time_us() must never block
// ro_timer_sync() must wake predictably
// _isr variants must be bounded
```

✅ **Enforce preconditions strictly**
```c
RO_ASSERT(condition, "contract violation message");  // if contract violated
return RO_EINVAL;  // or return error if recoverable
```

### MUST NOT
❌ **Block in ISR**
```c
// ❌ WRONG
ro_status_t ro_queue_send_isr(ro_queue_t* q, const void* data) {
    k_msgq_put(&q->kobj, data, K_FOREVER);  // blocks!
}

// ✅ RIGHT
ro_status_t ro_queue_send_isr(ro_queue_t* q, const void* data) {
    int ret = k_msgq_put(&q->kobj, data, K_NO_WAIT);
    return (ret == -ENOMSG) ? RO_EAGAIN : RO_OK;
}
```

❌ **Create hidden worker threads**
```c
// ❌ WRONG — user has no idea thread exists
void ro_queue_init() {
    k_thread_create(...);  // hidden thread!
}

// ✅ RIGHT — explicit in API or documented precondition
// User must call ro_queue_recv() to read; no magic background thread
```

❌ **Allocate from heap after boot**
```c
// ❌ WRONG
ro_sem_t* ro_sem_create(...) {
    ro_sem_t* s = malloc(sizeof(*s));  // heap!
}

// ✅ RIGHT
ro_sem_t* ro_sem_create(...) {
    ro_sem_t* s = ro_pool_alloc(sem_pool);  // static pool
}
```

❌ **Silently ignore contract violations**
```c
// ❌ WRONG — precondition violated, but no error
ro_status_t ro_sem_take_from_multiple_threads(...) {
    // ... works but violates contract, hard to debug
}

// ✅ RIGHT — document restriction or return error
// Contract: ro_sem_take() is NOT reentrant on same semaphore
// Documentation prevents accidental misuse
```

---

## 8. ISR Design Rules

### General ISR Contract for Adapter

All `_isr` functions must satisfy:
- **Latency:** < 10 µs (critical paths)
- **Blocking:** NEVER
- **Allocation:** NEVER
- **Default handler:** Return immediately, offload work to thread

### Per-Function ISR Rules

| Function | ISR Safe | Latency | Notes |
|----------|----------|---------|-------|
| `ro_sem_give_isr()` | ✅ Yes | < 5 µs | Wakes ONE waiter |
| `ro_queue_send_isr()` | ✅ Yes | < 5 µs | Non-blocking; RO_EAGAIN if full |
| `ro_queue_recv_isr()` | ✅ Yes | < 5 µs | Non-blocking; RO_EAGAIN if empty |
| `ro_pool_alloc_isr()` | ✅ Yes | < 5 µs | RO_ENOMEM if exhausted; never blocks |
| `ro_pool_free_isr()` | ✅ Yes | < 5 µs | Always succeeds |
| `ro_gpio_set()` | ⚠️ Check | Variable | PWM/GPIO usually non-blocking; verify |
| `ro_pwm_set_pulse_us()` | ⚠️ Check | Variable | Usually non-blocking; verify |

### ISR Implementation Pattern

```c
// Example: stepper step ISR
static void step_timer_isr(struct k_timer* timer) {
    if (--step_count == 0) {
        // Motion complete; signal thread
        ro_sem_give_isr(motion_complete_sem);  // ✅ ISR-safe
    }
    // Return; no allocation, no blocking
}

// ❌ WRONG patterns in ISR:
static void bad_isr(void) {
    ro_sem_take(sem, 1000);        // ❌ blocks
    malloc(sizeof(msg));           // ❌ allocates
    ro_thread_create(...);         // ❌ creates thread
    k_mutex_lock(m);               // ❌ blocks
}
```

---

## 9. Concurrency and Ownership Rules

### Which Primitives Are Thread-Safe

| Primitive | Multi-thread? | Multi-ISR? | Notes |
|-----------|---------------|-----------|-------|
| `ro_sem` | No | Yes (give_isr only) | Single-waiter; NOT reentrant for take |
| `ro_queue` | Yes | Yes (with _isr) | Multi-producer/consumer |
| `ro_pool` | Depends on lock_mode | Depends on lock_mode | LOCK_ATOMIC safe; LOCK_MUTEX not safe in ISR |
| `ro_mutex` | Yes | No | Priority-inherit; forbidden in ISR |
| `ro_timer` | No | N/A | Thread-only ownership |
| `ro_thread` | N/A | No | Creation thread-only |

### Semaphore Waiter Semantics

**Critical rule:** Semaphores are NOT broadcast-style.

```c
// ✅ CORRECT
ro_sem_t* s = ro_sem_create(0, 1);  // binary
// One thread calls ro_sem_take(s, timeout);

// Another thread calls ro_sem_give(s);
// ONE thread wakes; others stay blocked

// ❌ WRONG (violates RobotOS semantics)
// Multiple threads call ro_sem_take(s, timeout) on same semaphore
// Only ONE wakes per give(); others wait forever or until next give
```

### Queue Ownership

Queues are owned collectively; any thread can send or receive.

```c
// ✅ Can have multiple producers and consumers
ro_queue_t* q = ro_queue_create(sizeof(int), 10);
// Thread A calls ro_queue_send(q, ...);
// Thread B calls ro_queue_send(q, ...);
// Thread C calls ro_queue_recv(q, ...);
// Thread D calls ro_queue_recv(q, ...);
// All operations are serialized internally
```

### Pool Lock Modes

```c
// LOCK_ATOMIC (safe for ISR)
ro_pool_t* msg_pool = ro_pool_create(&(ro_pool_config_t){
    .lock_mode = RO_POOL_LOCK_ATOMIC,
});
ro_pool_alloc_isr(msg_pool);  // ✅ safe in ISR

// LOCK_MUTEX (thread-only)
ro_pool_t* big_pool = ro_pool_create(&(ro_pool_config_t){
    .lock_mode = RO_POOL_LOCK_MUTEX,
});
ro_pool_alloc_isr(big_pool);  // ❌ ASSERTION: ISR cannot use MUTEX
```

---

## 10. Module Testing Strategy

### Test Organization

```
tests/
├── adapter/
│   ├── test_ro_sem.c
│   ├── test_ro_queue.c
│   ├── test_ro_timer.c
│   ├── test_ro_pool.c
│   ├── test_ro_thread.c
│   └── test_ro_gpio.c
└── integration/
    ├── test_sem_thread_wake.c
    ├── test_queue_producer_consumer.c
    └── test_timer_sync_realtime.c
```

### Minimal Test Matrix for Core Primitives

#### `ro_sem` Tests
```
✅ create(0, 1) succeeds
✅ give() from thread, take() wakes
✅ take(timeout_ms=100) RO_ETIMEDOUT if no give
✅ give_isr() non-blocking returns immediately
✅ give() when count==max returns RO_EAGAIN
✅ give() then take() immediately returns
✅ multiple threads: only ONE wakes per give
```

#### `ro_queue` Tests
```
✅ send/recv success
✅ send(timeout_ms=100) RO_ETIMEDOUT if full
✅ recv(timeout_ms=100) RO_ETIMEDOUT if empty
✅ FIFO ordering preserved
✅ send_isr → RO_EAGAIN if full
✅ recv_isr → RO_EAGAIN if empty
✅ producer/consumer threads run concurrently
```

#### `ro_timer` Tests
```
✅ start_periodic(1000µs); sync() wakes ~1000µs later
✅ multiple syncs maintain periodic behavior
✅ sync() detects overrun if loop slow
```

#### `ro_thread` Tests
```
✅ create() succeeds
✅ thread entry function executes
✅ priority levels map correctly
✅ cannot exceed CONFIG_RO_MAX_THREADS
```

#### `ro_pool` Tests
```
✅ alloc() / free() cycle
✅ LOCK_ATOMIC mode: multi-thread safe
✅ LOCK_MUTEX mode: single-thread safe
✅ alloc_isr(LOCK_ATOMIC) succeeds
✅ alloc_isr(LOCK_MUTEX) asserts (or returns error)
✅ exhaustion: alloc returns NULL (alloc_isr) or blocks (alloc)
```

### Integration Tests (Critical)

```
test_sem_thread_wake {
    // Thread A waits on semaphore
    // ISR calls give_isr()
    // Thread A wakes
    // Verify timing, state machine
}

test_queue_producer_consumer {
    // Thread A produces messages
    // Thread B consumes
    // Verify order, completeness
}

test_timer_sync_realtime {
    // 1 kHz loop using ro_timer_sync()
    // Measure actual wake time
    // Detect jitter, overruns
}

test_gpio_isr_to_queue {
    // GPIO ISR calls ro_queue_send_isr()
    // Thread receives from queue
    // Verify no lost events
}
```

---

## 11. Integration Testing Strategy

### Scenario 1: Thread Creation + Timing

```c
test("threads_can_read_time") {
    // Create thread
    // Thread reads ro_time_us() several times
    // Verify monotonicity and reasonable values
}
```

### Scenario 2: ISR → Semaphore → Thread

```c
test("isr_to_sem_wakes_thread") {
    ro_sem_t* s = ro_sem_create(0, 1);
    
    // Start thread that calls ro_sem_take(s, 5000)
    ro_thread_t* t = ro_thread_create(...);
    
    // Simulate ISR
    simulate_isr_call(ro_sem_give_isr, s);
    
    // Verify thread woke
}
```

### Scenario 3: Periodic Timer + Control Loop

```c
test("timer_sync_1khz_deterministic") {
    ro_timer_t timer;
    ro_timer_start_periodic(&timer, 1000);  // 1 kHz
    
    for (int i = 0; i < 100; i++) {
        uint64_t t0 = ro_time_us();
        ro_timer_sync(&timer);
        uint64_t t1 = ro_time_us();
        
        uint64_t elapsed = t1 - t0;
        // Verify ~1000 µs; detect jitter
        expect(elapsed > 900 && elapsed < 1100);
    }
}
```

### Scenario 4: Queue with ISR Producer

```c
test("queue_isr_send_thread_recv") {
    ro_queue_t* q = ro_queue_create(sizeof(int), 10);
    
    // Thread listens on queue
    // Simulate ISR sending messages
    // Verify all received in order
}
```

---

## 12. System Readiness Checks for Adapter

### Adapter Is Ready When

- [ ] **All core modules compile**
  - No syntax errors
  - No missing `ro_status.h` or API typos
  - All headers have no Zephyr includes

- [ ] **Tests pass (unit + integration)**
  - ✅ All 40+ module tests pass
  - ✅ All 4+ integration scenarios pass
  - ✅ No resource leaks detected

- [ ] **ISR paths are safe**
  - All `_isr` variants tested in ISR context
  - No blocking, no allocation, latency < 10 µs verified

- [ ] **Timeout semantics verified**
  - RO_ETIMEDOUT returned correctly
  - RO_EAGAIN (non-blocking) returned correctly
  - RO_OK on success

- [ ] **Cross-layer assumptions validated**
  - Framework can `#include` only public headers
  - No Zephyr types visible to Framework
  - Contracts are clear and testable

- [ ] **Error handling is complete**
  - All return paths use `ro_status_t`
  - No silent failures
  - Preconditions enforced or documented

---

## 13. Delivery Standards

### Definition of Done (DoD) for Each Module

```
Module: [name]

Code Quality:
  ☐ Compiles with no warnings
  ☐ Code review passed
  ☐ No Zephyr leakage in headers
  
Correctness:
  ☐ All function signatures match spec
  ☐ All ro_status_t codes used correctly
  ☐ All preconditions validated or documented
  
Testing:
  ☐ Unit tests pass
  ☐ Integration tests pass
  ☐ ISR tests pass (if applicable)
  ☐ No memory leaks
  ☐ Timeout edge cases covered
  
Documentation:
  ☐ Function comments explain contracts
  ☐ ISR restrictions documented
  ☐ Timeout semantics clear
  
Performance:
  ☐ ISR latency < 10 µs (if applicable)
  ☐ No unexpected blocking paths
```

### Code Review Checklist

When reviewing Adapter code:

- [ ] **Boundary intact**
  - No `<zephyr/*>` in headers
  - All Zephyr in `.c` files only
  
- [ ] **Naming correct**
  - Only `ro_*` in public API
  - No Zephyr prefixes (like k_, z_)
  
- [ ] **Ownership clear**
  - No hidden thread creation
  - No silent allocation
  - Caller owns all resources
  
- [ ] **Error handling correct**
  - All paths return `ro_status_t`
  - No silent failures
  - Errors map to correct codes
  
- [ ] **ISR-safety verified**
  - `_isr` functions non-blocking
  - No unsafe Adapter calls in ISR
  - Latency is bounded
  
- [ ] **Timeouts work**
  - RO_ETIMEDOUT on expiry
  - RO_OK on success before timeout
  - RO_EAGAIN on non-blocking poll failure

---

## 14. Coding Standards for Adapter

### File Naming

```
include/robotos/
  ├── ro_status.h           (error codes)
  ├── ro_thread.h           (threading)
  ├── ro_time.h             (time + sleep)
  ├── ro_timer.h            (periodic timer)
  ├── ro_sem.h              (semaphore)
  ├── ro_queue.h            (message queue)
  ├── ro_pool.h             (memory pool)
  ├── ro_mutex.h            (mutex)
  ├── ro_gpio.h             (GPIO I/O)
  ├── ro_pwm.h              (PWM)
  ├── ro_log.h              (logging)
  ├── ro_trace.h            (tracing)
  ├── ro_i2c.h, ro_spi.h    (serial buses)
  └── ro_deadline.h         (deadline monitoring)

src/adapter/
  ├── internal.h            (adapter-internal facilities)
  ├── common/
  │   └── ro_status.c       (if any implementation needed, but status is typedef-only)
  └── zephyr/               (backend-specific)
      ├── ro_thread.c
      ├── ro_time.c
      ├── ro_timer.c
      ├── ro_sem.c
      ├── ro_queue.c
      ├── ro_pool.c
      ├── ro_mutex.c
      ├── ro_gpio.c
      ├── ro_pwm.c
      ├── ro_log.c
      ├── ro_trace.c
      ├── ro_i2c.c, ro_spi.c
      └── ro_deadline.c
```

### Error Propagation Style

Use `RO_TRY` macro for common error propagation:

```c
// ✅ CORRECT
static ro_status_t do_critical_setup() {
    RO_TRY(ro_timer_start_periodic(&timer, 1000));
    RO_TRY(ro_sem_create(...));
    return RO_OK;
}

// ❌ INCORRECT (verbose, error-prone)
static ro_status_t do_critical_setup() {
    ro_status_t s;
    s = ro_timer_start_periodic(&timer, 1000);
    if (s != RO_OK) return s;
    // ... repeated for every call
}
```

### Struct Opaqueness

All handle types are opaque:

```c
// ✅ in ro_sem.h
typedef struct ro_sem ro_sem_t;  // size/contents not visible

// ✅ in ro_sem.c (Zephyr backend)
struct ro_sem {
    struct k_sem kobj;
};

// ❌ NEVER in header
struct ro_sem {  // visible to all callers!
    struct k_sem kobj;
};
```

### Comment Style for Contracts

```c
// ✅ CORRECT
// ro_sem_take: Block thread until semaphore acquired or timeout expires.
//
// Context: thread only (not ISR)
// Parameters:
//   s: semaphore (non-NULL)
//   timeout_ms: 0 for non-blocking poll; > 0 for timeout;
//               UINT32_MAX (optional) for infinite wait
// Returns:
//   RO_OK: semaphore acquired
//   RO_ETIMEDOUT: timeout expired before acquisition
//   RO_EAGAIN: non-blocking poll (timeout_ms==0) returned with count==0
// Precondition: Only ONE thread may call take() on same semaphore at once
ro_status_t ro_sem_take(ro_sem_t* s, uint32_t timeout_ms);

// ❌ INCORRECT (vague)
// Take the semaphore. Returns status.
ro_status_t ro_sem_take(ro_sem_t* s, uint32_t timeout_ms);
```

---

## 15. Recommended Techniques and Reference Patterns

### Techniques SHOULD Be Used

**Opaque Handle Pattern**
```c
// ✅ Hide implementation
typedef struct ro_sem ro_sem_t;  // opaque

// User never knows internals; backend can change
```

**Wrapper Isolation**
```c
// ✅ Error translation in one place
static ro_status_t zephyr_to_ro(int zephyr_code) {
    // all translations here
}
```

**Static Pool Allocation**
```c
// ✅ No heap after init; bounded, deterministic
static uint8_t sem_pool_buf[256];
static ro_pool_t* sem_pool;
```

**Atomic Operations for ISR Coordination**
```c
// ✅ Lock-free communication ISR → thread
atomic_t fault_flags;
// ISR: atomic_or(&fault_flags, FAULT_BIT);
// Thread: if (atomic_load(&fault_flags) & FAULT_BIT) { ... }
```

**Bounded ISR Work**
```c
// ✅ ISR does minimal work
ISR() {
    ro_sem_give_isr(sem);  // < 5 µs
}
// Thread wakes and does heavy work
Thread() {
    ro_sem_take(sem, timeout);
    // ... expensive operations here
}
```

### Reference Inspirations (Study, Don't Copy)

| System | What to learn | What NOT to copy |
|--------|---|---|
| **Zephyr kernel** | Priority inheritance, ISR latency budget, determinism | Specific API names, Kconfig syntax |
| **FreeRTOS** | Queue/semaphore semantics, task priorities | Heap allocation model, tick-based timing |
| **POSIX** | Thread model, timeout contracts | System-call overhead, signal complexity |
| **Linux kernel** | Preemption model, RCU for lock-free reads | Context switching assumptions, resource richness |

---

## 16. Common Failure Modes

### Failure 1: Zephyr Leakage in Headers

```c
// ❌ WRONG
// include/robotos/ro_sem.h
#include <zephyr/kernel.h>

typedef struct {
    struct k_sem kobj;  // LEAKS ZEPHYR TYPE
} ro_sem_t;

// ✅ CORRECT
// include/robotos/ro_sem.h
typedef struct ro_sem ro_sem_t;  // OPAQUE

// src/adapter/zephyr/ro_sem.c
#include <zephyr/kernel.h>
struct ro_sem {
    struct k_sem kobj;  // HIDDEN
};
```

**Detection:** `grep -r "k_\|<zephyr" include/robotos/`  
**Prevention:** Code review; CI check for Zephyr includes in headers

---

### Failure 2: ISR Path Blocks

```c
// ❌ WRONG
ro_status_t ro_queue_send_isr(ro_queue_t* q, const void* data) {
    return ro_queue_send(q, data, K_FOREVER);  // BLOCKS!
}

// ✅ CORRECT
ro_status_t ro_queue_send_isr(ro_queue_t* q, const void* data) {
    int ret = k_msgq_put(&q->kobj, (void*)data, K_NO_WAIT);
    if (ret != 0) return RO_EAGAIN;
    return RO_OK;
}
```

**Detection:** ISR timings; test ISR with scope/logic analyzer  
**Prevention:** All `_isr` functions tested in actual ISR context

---

### Failure 3: Hidden Heap Allocation

```c
// ❌ WRONG
ro_sem_t* ro_sem_create(...) {
    ro_sem_t* s = malloc(sizeof(*s));  // HEAP!
}

// ✅ CORRECT
ro_sem_t* ro_sem_create(...) {
    ro_sem_t* s = ro_pool_alloc(sem_pool);  // STATIC POOL
}
```

**Detection:** `grep -r "malloc\|calloc\|new" src/adapter/`  
**Prevention:** Adapter never calls malloc; uses static pools only

---

### Failure 4: Violating Semaphore Semantics

```c
// ❌ WRONG USAGE (developer mistake, not Adapter bug)
// but Adapter should DOCUMENT this:
ro_sem_t* s = ro_sem_create(0, 1);
// Thread A calls ro_sem_take(s, timeout);
// Thread B calls ro_sem_take(s, timeout);   // VIOLATION!
// Only ONE thread wakes per give()

// ✅ PREVENT by documenting contract
// "Semaphores are NOT reentrant. Only ONE thread may wait at a time."
// OR provide separate count semaphore for multiple waiters
```

**Detection:** Code review; Adapter documentation  
**Prevention:** Clear contract enforcement via comments + Framework contract enforcement

---

### Failure 5: Using Mutex in RT Thread

```c
// ❌ WRONG
#define RO_PRIO_RT_CONTROL  -10
static void control_loop(void* arg) {
    // ...
    ro_mutex_lock(m, timeout);  // FORBIDDEN in RT thread!
    // ... work ...
    ro_mutex_unlock(m);
}

// ✅ CORRECT
// Mutex only in preemptive APP-tier threads
#define RO_PRIO_APP  0
static void app_thread(void* arg) {
    // ...
    ro_mutex_lock(m, timeout);  // OK
}
```

**Detection:** Assertion in ro_mutex_lock() if called from RT priority  
**Prevention:** Clear priority level documentation; warnings in code

---

### Failure 6: No Timeout on Blocking Operations

```c
// ❌ RISKY
ro_status_t s = ro_queue_recv(q, &msg, 0xFFFFFFFF);
if (s == RO_ETIMEDOUT) {  // infinite wait, can never timeout!
    // ...
}

// ✅ SAFE
ro_status_t s = ro_queue_recv(q, &msg, 5000);  // 5 second timeout
if (s == RO_ETIMEDOUT) {  // meaningful timeout
    RO_LOG_WARN("Queue recv timeout; check producer");
}
```

**Detection:** Code review; design requirements specify timeouts  
**Prevention:** Never use UINT32_MAX for "infinite"; use explicit timeout values

---

## 17. First Vertical Slice for Adapter

### Goal
Get to **first working vertical slice**: thread wakeup from ISR-safe signal

### Phase 1: Core (1 day)

```
Day 1:
  ✅ ro_status.h created
  ✅ ro_time.h implemented (ro_time_us, ro_thread_sleep_ms)
  ✅ ro_thread.h implemented (create, with static stack ownership)
  ✅ ro_sem.h implemented (create, take, give, give_isr)
  ✅ Unit tests pass: time, thread creation, semaphore wake
```

Deliverable: A program where Thread A can be woken by ISR calling `ro_sem_give_isr()`.

### Phase 2: Timer (0.5 day)

```
Day 1.5:
  ✅ ro_timer.h implemented (start_periodic, sync, stop)
  ✅ Unit tests: periodic wake, overrun detection
```

Deliverable: 1 kHz control loop using `ro_timer_sync()`.

### Phase 3: Queue (0.5 day)

```
Day 2:
  ✅ ro_queue.h implemented (send, recv, send_isr, recv_isr)
  ✅ Integration test: ISR sends → thread receives
```

Deliverable: ISR can queue messages to thread without blocking.

### Phase 4: Pool + Mutex (1 day)

```
Day 3:
  ✅ ro_pool.h (with LOCK_ATOMIC for ISR safety)
  ✅ ro_mutex.h (LOCK_MUTEX for thread-only sections)
```

Deliverable: Lightweight memory allocation for runtime objects.

### Phase 5: Validation (0.5 day)

```
Day 3.5:
  ✅ All tests pass
  ✅ No Zephyr in headers (grep verified)
  ✅ Adapter is clean and ready for Framework
```

**Total: ~3.5 days for core Adapter baseline**

---

## 18. Final Adapter Go/No-Go Gate

### Readiness Question

> **Can the Framework layer begin implementation on top of this Adapter?**

### Pass Criteria (ALL must be YES)

- [ ] **Primitive completeness**
  - All 8 core modules exist: status, thread, time, timer, sem, queue, pool, mutex
  - All public headers are present and compile
  
- [ ] **ISR correctness**
  - All `_isr` variants tested in actual ISR context
  - No blocking, no allocation confirmed
  - Latency < 10 µs verified on hardware
  
- [ ] **Determinism confidence**
  - ro_timer_sync() maintains periodic schedule
  - Overrun detection works
  - Timeout contracts honored
  
- [ ] **Boundary cleanliness**
  - Zero Zephyr includes in public headers
  - All error codes consistent
  - No hidden threads or assumptions
  
- [ ] **Test coverage**
  - Unit tests: > 90% pass rate
  - Integration tests: all scenarios pass
  - No memory leaks or undefined behavior
  
- [ ] **Documentation**
  - API contracts clear in headers
  - ISR restrictions documented
  - Timeout semantics explained

### Verdict

Choose one:

- **GO** — All criteria met; Framework can proceed immediately
- **GO WITH MINOR FIXES** — 1–2 criteria partially met; fixes << 1 day effort
- **NO-GO** — ≥ 3 criteria not met; redesign scope needed

---

## Summary: Implementation Checklist

### Pre-Implementation
- [ ] Review ADAPTER_LAYER.md spec completely
- [ ] Understand Zephyr primitives being wrapped
- [ ] Set up project structure (directories, config)

### Implementation
- [ ] Implement Phase A (ro_status, conventions)
- [ ] Implement Phase B core (thread, time, sem, queue, pool, mutex)
- [ ] Write unit tests (> 90% pass rate)
- [ ] Implement Phase C hardware (GPIO, PWM, I2C/SPI as needed)
- [ ] Write integration tests

### Validation
- [ ] All tests pass
- [ ] Zero Zephyr in headers (verify with grep)
- [ ] ISR latency verified
- [ ] Feature parity with spec
- [ ] Code review complete

### Handoff to Framework
- [ ] Adapter go/no-go gate passed
- [ ] Documentation complete
- [ ] Ready for Framework integration

---

**Document Status:** 🚧 Implementation Guide  
**Related:** [ADAPTER_LAYER.md](ADAPTER_LAYER.md) (spec), [ROBOT_OS_IMPLEMENTATION_GUIDE.md](ROBOT_OS_IMPLEMENTATION_GUIDE.md)  
**Last Updated:** 2026-04-01

