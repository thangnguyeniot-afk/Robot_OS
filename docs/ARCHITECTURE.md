# RobotOS Inspire — System Architecture Design

> **Version:** v0.1-alpha  
> **Last Updated:** 2026-03-01  
> **Status:** 🚧 Under Active Development

---

## 📋 Table of Contents

1. [Executive Summary](#executive-summary)
2. [Architecture Philosophy](#architecture-philosophy)
3. [System Overview](#system-overview)
4. [Layered Architecture](#layered-architecture)
5. [Component Architecture](#component-architecture)
6. [Error Handling & Status Codes](#error-handling--status-codes)
   - [`ro_status_t` Definition](#ro_status_t-definition)
   - [Zephyr errno Mapping](#zephyr-errno-mapping)
   - [Return Value Convention](#return-value-convention)
   - [Error Propagation Rules](#error-propagation-rules)
7. [Boot Sequence & Initialization Contract](#boot-sequence--initialization-contract)
   - [Zephyr SYS_INIT Levels](#zephyr-sys_init-levels)
   - [robotos_init() Contract](#robotos_init-contract)
   - [Init Phase Constraints](#init-phase-constraints)
8. [Memory Architecture](#memory-architecture)
9. [Threading & Execution Model](#threading--execution-model)
   - [Thread Ownership Model](#thread-ownership-model)
   - [Kconfig Profile Discipline](#kconfig-profile-discipline)
10. [Isolation Profiles (RO_PROFILE)](#isolation-profiles-ro_profile)
    - [One-sentence thesis](#one-sentence-thesis)
    - [Why the isolation ladder](#why-the-isolation-ladder)
    - [Profile definitions](#profile-definitions)
    - [Profile-to-component matrix](#profile-to-component-matrix)
    - [Gate API pattern (GUARDED)](#gate-api-pattern-guarded)
    - [CI enforcement](#ci-enforcement)
    - [Device Registry concurrency hazard](#device-registry-concurrency-hazard)
11. [Communication Architecture](#communication-architecture)
12. [Build System Architecture](#build-system-architecture)
13. [Platform Abstraction](#platform-abstraction)
14. [Observability & Tracing](#observability--tracing)
    - [Deadline Monitoring](#deadline-monitoring)
15. [Design Patterns & Conventions](#design-patterns--conventions)
16. [Security Considerations](#security-considerations)
17. [Performance Characteristics](#performance-characteristics)
18. [Future Architecture Evolution](#future-architecture-evolution)
19. [Alpha MVP Demo Model](#alpha-mvp-demo-model)

---

## Executive Summary

**RobotOS Inspire** is a **robotics-optimized system** built on **Zephyr RTOS** kernel, featuring:

- **Robot Adapter Layer** — Portable abstraction for RTOS/HW primitives (threads, timing, IPC, memory, GPIO, PWM, I2C, trace)
- **Robot Framework** — Domain-specific robotics components (Stepper, Servo, PID, State Machine, Encoders, Sensors) with tuned policies and premium consistency
- **Deterministic design** — Predictable, transparent, no hidden threads
- **Observability-first** — Built-in trace, benchmark, and deadline monitoring

### Key Characteristics

| Aspect | Design Choice |
|--------|---------------|
| **Kernel Base** | Zephyr RTOS (customized for robotics) |
| **Adapter Primitives** | Threads, timing, queues, memory pools, GPIO, PWM, I2C, SPI, trace, logging |
| **Framework Components** | Stepper, ServoPWM, DCmotorPWM, Encoder, PID, Filters, Limiter, State Machine |
| **Memory Strategy** | Fixed-block pools, zero malloc in realtime |
| **Threading Model** | Zephyr threads with priority-based scheduling |
| **Target Hardware** | STM32F4, ESP32, nRF52, QEMU (v0.1) |
| **Language** | C (core), Python (tooling) |

---

## Architecture Philosophy

### Core Principles

#### 1. **Determinism-by-Default**
```
❌ NO malloc/free in realtime paths
✅ Fixed memory pools with compile-time budgets
✅ Bounded execution time for all middleware ops
```

#### 2. **No Magic**
```
❌ NO middleware-spawned threads
❌ NO hidden callbacks in unknown contexts
✅ Explicit executor → task mapping
✅ Clear priority/stack assignment
```

> **Precise contract:** See [Thread Ownership Model](#thread-ownership-model) for the exact definition of "magic thread", who is allowed to call `ro_thread_create`, stack ownership rules, and the mandatory Thread Roster format.

#### 3. **Observability as a Product Feature**
```
✅ Trace hooks in every critical path
✅ Missed deadline detection
✅ Chrome Trace JSON export
✅ Benchmark suite for every release
```

#### 4. **Upstream Compatibility**
```
✅ Minimal kernel patches (use existing hooks)
✅ Upstream-friendly changes only
✅ Easy to sync with Zephyr updates
```

---

## System Overview

### High-Level Block Diagram

```
┌─────────────────────────────────────────────────────────────────────┐
│                      APPLICATION LAYER                              │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐              │
│  │   CNC App    │  │ 3D Printer   │  │  Arm Basic   │  ┌─────────┐ │
│  │ (Focus Demo) │  │     App      │  │     App      │  │Line Track│ │
│  └──────┬───────┘  └──────┬───────┘  └──────┬───────┘  │Robot App│ │
│         │                 │                 │           └────┬────┘ │
│         │                 │                 │                │      │
└─────────┼─────────────────┼─────────────────┼────────────────┼──────┘
          │                 │                 │                │
          └─────────────────┼─────────────────┴────────────────┘
                            │
┌───────────────────────────▼──────────────────────────────────────┐
│              ROBOT FRAMEWORK LAYER (Robotics Domain)             │
│  Semantics: Stepper motor control, servo policies, PID tuning,   │
│  robot state machine, kinematics, deterministic behaviors         │
│  ┌──────────────────────────────────────────────────────────┐   │
│  │            Device Components                             │   │
│  │  ┌──────────┐  ┌──────────┐  ┌──────────┐              │   │
│  │  │ Stepper  │  │ ServoPWM │  │DCmotorPWM│              │   │
│  │  └──────────┘  └──────────┘  └──────────┘              │   │
│  │  ┌──────────┐  ┌──────────┐  ┌──────────┐              │   │
│  │  │ Encoder  │  │ Endstop  │  │ Sensors  │              │   │
│  │  └──────────┘  └──────────┘  └──────────┘              │   │
│  └──────────────────────────────────────────────────────────┘   │
│  ┌──────────────────────────────────────────────────────────┐   │
│  │        Control & Processing Components                   │   │
│  │  ┌──────────┐  ┌──────────┐  ┌──────────┐              │   │
│  │  │   PID    │  │ Filters  │  │ Limiter  │              │   │
│  │  └──────────┘  └──────────┘  └──────────┘              │   │
│  └──────────────────────────────────────────────────────────┘   │
│  ┌──────────────────────────────────────────────────────────┐   │
│  │          Robot State Machine (Lifecycle)                 │   │
│  │        IDLE → HOMING → RUN → FAULT                       │   │
│  └──────────────────────────────────────────────────────────┘   │
└───────────────────────────┬──────────────────────────────────────┘
                            │ (depends on Adapter primitives)
                            │
┌───────────────────────────▼──────────────────────────────────────┐
│         ROBOT ADAPTER LAYER (RTOS/HW Abstraction)                │
│  Capabilities: threads, timing, IPC, memory, GPIO, PWM, I2C,     │
│  SPI, trace, logging. Portable across Zephyr + other RTOS.       │
│  ┌──────────────────────────────────────────────────────────┐   │
│  │ Threading | Timing | Queues | Pools | GPIO/PWM | I2C/SPI│   │
│  │             Logging | Tracing | Power                    │   │
│  └──────────────────────────────────────────────────────────┘   │
└───────────────────────────┬──────────────────────────────────────┘
                            │
┌───────────────────────────▼──────────────────────────────────────┐
│         ZEPHYR KERNEL - Customization (Robot Optimized)          │
│  ┌──────────────────────────────────────────────────────────┐   │
│  │  Scheduler | Threads | Timers | IPC (Queue/Mutex/Sem)    │   │
│  └──────────────────────────────────────────────────────────┘   │
│  ┌──────────────────────────────────────────────────────────┐   │
│  │  Device Tree | SYS_INIT | Logging | Tracing              │   │
│  └──────────────────────────────────────────────────────────┘   │
└───────────────────────────┬──────────────────────────────────────┘
                            │
┌───────────────────────────▼──────────────────────────────────────┐
│                    HARDWARE LAYER                                 │
│        STM32F4  |  ESP32  |  nRF52  |  QEMU (Virtual)            │
└───────────────────────────────────────────────────────────────────┘
```

### Design Layers

| Layer | Responsibility | Language | Portability |
|-------|---------------|----------|-------------|
| **Application** | Robot-specific logic (CNC, 3D Printer, Arm, Line Tracker) | C | 100% (uses only Framework + Adapter APIs; NO Zephyr headers) |
| **Robot Framework** | Stepper/Servo/PID/State Machine control, policies, tuning, kinematics | C | 100% (portable; calls only Adapter APIs; may use DT internally) |
| **Robot Adapter** | RTOS/HW primitives: threads, timing, IPC, memory, GPIO, PWM, I2C, SPI, trace, logging | C | 100% (abstraction layer; hides Zephyr) |
| **Zephyr Kernel** | Scheduling, IPC, Memory, Device Tree, Device Drivers | C | 0% (RTOS-specific) |
| **Hardware** | MCU peripherals (GPIO, Timer, UART, PWM, I2C, SPI) | C | 0% (platform-specific) |

---

## Layered Architecture

### Architectural Boundary Definition

**Robot Adapter Layer = RTOS/HW primitives (capabilities)**
- Answers: "How do I run a thread, measure time, send a message, allocate memory, read GPIO, control PWM, use I2C, trace events—in a portable way?"
- Scope: Threading, timing, IPC, memory, peripheral abstraction (GPIO, PWM, I2C, SPI), logging, tracing, power management
- Contract: Small, stable, hides ALL Zephyr internals; switchable to another RTOS underneath
- NO robotics semantics; NO device policies; NO domain knowledge

**Robot Framework = Robotics semantics (components)**
- Answers: "What does Stepper motor control mean? How does servo PID policy work? When does robot transition states? What does homing look like?"
- Scope: Device components (Stepper, Servo, Encoder, Endstop, Sensors), control algorithms (PID, Filters, Limiter), state machine, kinematics, trajectory planning
- Policy-driven: Tuning parameters, acceleration profiles, homing strategies, trapped-rotor detection, anti-windup behavior, etc.
- Contract: Uses ONLY Adapter APIs publicly; may use Device Tree + drivers internally but does NOT expose Zephyr to applications
- Portability: Write once, reuse across all boards; consistent behavior regardless of RTOS backend

### Layer Dependency Rules

```
Application (CNC, Printer, Arm, Line Tracker)
    ↓ (depends on)
Robot Framework (Stepper, Servo, PID, State Machine)
    ↓ (depends on)
Robot Adapter Layer (Threads, Timing, IPC, Memory, GPIO, PWM, I2C, SPI, Trace, Logging)
    ↓ (depends on)
Zephyr Kernel (Scheduler, IPC, Timers, Device Tree, Drivers)
    ↓ (depends on)
Hardware (STM32, ESP32, nRF52)
```

**Strict Rules:**
- ✅ Higher layers may depend on lower layers
- ❌ Lower layers MUST NOT depend on higher layers
- ✅ Application code includes: `<robotos/stepper.h>`, `<robotos/pid.h>`, `<robotos/ro_thread.h>` only—NO `<zephyr/*>` headers
- ✅ Robot Framework internal implementation may use Device Tree + Zephyr drivers, but **public headers expose ONLY Adapter APIs to callers**
- ❌ No circular dependencies allowed
- ✅ Robot Adapter Layer *implementation files* (`src/adapter/zephyr/*.c`) are the ONLY place Zephyr headers appear in the codebase. **All public headers (`include/robotos/*.h`) are Zephyr-free** — including Adapter headers such as `ro_thread.h` and `ro_queue.h`. Public headers expose only opaque types, constants, and function signatures; every Zephyr concrete type is hidden behind `typedef struct ... ro_xxx_t;` in the implementation.

### Robot Framework Layer Details

#### Purpose & Scope

**Robot Framework** provides domain-specific robotics abstractions:
- **NOT** just thin wrappers around hardware (that's the Adapter layer)
- **IS** policy-driven, tuned control components with robotics semantics
- Implements deterministic, reusable behaviors (homing, acceleration planning, PID anti-windup)
- Guarantees premium consistency: same control law across all boards & backends
- All public APIs use ONLY Robot Adapter primitives (thread-safe, no Zephyr leakage)

#### Device Components

**Components:**
- **Stepper:** Stepper motor control with acceleration/deceleration profiles, position tracking, trapezoidal motion planning
- **ServoPWM:** Servo motor control with angle mapping, PWM pulse generation, safety limits
- **DCmotorPWM:** DC motor speed control with PWM + direction, dead-band handling, soft-start
- **Encoder:** Rotary encoder position/speed reading with quadrature decoding, filtering, velocity smoothing
- **Endstop:** Limit switch handling for homing sequences, safety stops, position confirmation
- **Sensors:** General sensor abstraction (ADC, digital, I2C/SPI) with calibration, averaging, fault detection

**API Stability:** v0.1 → Unstable (breaking changes allowed)

**Header Location:** `<robotos/stepper.h>`, `<robotos/servo.h>`, etc. — **NOT** under `robotos/adapter/`

#### Control & Processing Components

**Purpose:** Control algorithms and signal processing

**Components:**
- **PID:** PID controller (proportional-integral-derivative)
- **Filters:** Signal filters (low-pass, moving average, etc.)
- **Limiter:** Value clamping and rate limiting

#### Robot State Machine

**Purpose:** Unified robot lifecycle management

**States:**
- **IDLE:** Initial state, waiting for commands
- **HOMING:** Finding home position (using endstops)
- **RUN:** Normal operation (executing commands)
- **FAULT:** Error state (safety stop, requires reset)

**Transitions:**
```
IDLE --[home_cmd]--> HOMING --[homed]--> IDLE
IDLE --[start_cmd]--> RUN --[stop_cmd]--> IDLE
ANY_STATE --[error]--> FAULT --[reset]--> IDLE
```

---

## Component Architecture

### API Layer Enforcement

#### Compile-Time & CI Rules

**Application & Framework headers:**
```c
// ✅ CORRECT
// Framework headers — NO "ro_" prefix (robotics-domain names):
#include <robotos/stepper.h>
#include <robotos/pid.h>
// Adapter headers — "ro_" prefix (RTOS-primitive names):
#include <robotos/ro_thread.h>     // Adapter API is fine for app
#include <robotos/ro_queue.h>

// ❌ FORBIDDEN in app/framework public headers
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/device.h>
```

**Robot Adapter implementation (internal only):**
```c
// ONLY in concrete adapter implementation (ro_thread.c, ro_queue.c, etc.)
#include <zephyr/kernel.h>   // ← Hidden from users
struct ro_thread {
    struct k_thread zephyr_internal;  // ← Opaque
};
```

**Robot Framework internal (may use Zephyr for DT, drivers):**
```c
// Inside stepper.c implementation:
#include <zephyr/devicetree.h>  // ← OK for internal DT access
#include <robotos/ro_thread.h>  // ← Public API returns to Adapter only
```

**CI Check (githooks / linter):**
```bash
# ─── Zephyr Include Whitelist ───────────────────────────────────────────────
# Files ALLOWED to include <zephyr/*>:
#   src/adapter/zephyr/*.c        — Adapter implementations ONLY.
#                                   ALL <zephyr/*> usage must live here.
#   src/framework/drivers/*.c     — Framework driver (DT + device_is_ready) ONLY.
#                                   Permitted: <zephyr/devicetree.h>, <zephyr/device.h>
#                                   Forbidden: <zephyr/drivers/*>, <zephyr/kernel.h>
# FILES WITH ZERO TOLERANCE (emit error on any <zephyr/*>):
#   include/robotos/*.h           — Public headers (Adapter + Framework public API)
#   src/framework/*.h             — Framework public headers
# ─────────────────────────────────────────────────────────────────────────────

# 1) Public headers: absolute zero Zephyr
for f in include/robotos/*.h src/framework/*.h; do
  if grep -q '#include <zephyr/' "$f"; then
    echo "ERROR: $f includes Zephyr. Public headers must be RTOS-agnostic."
    exit 1
  fi
done

# 2) Framework drivers: only DT + device API allowed (no driver-level headers)
for f in src/framework/drivers/*.c; do
  if grep -qE '#include <zephyr/drivers/' "$f"; then
    echo "ERROR: $f uses <zephyr/drivers/*>. Framework drivers must use Adapter APIs."
    exit 1
  fi
done

# 3) Verify Framework headers do NOT use ro_ prefix (naming convention guard)
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

#### API Usage Quick Reference

**What you need in different contexts:**

| Context | Include This | NOT This | Reason |
|---------|-------------|----------|--------|
| **3D Printer App** | `<robotos/stepper.h>`, `<robotos/ro_thread.h>` | `<zephyr/*>` | Don't need RTOS details; Framework abstracts them |
| **Stepper Framework Lib** | `<robotos/ro_thread.h>`, `<robotos/ro_time.h>` | `<zephyr/kernel.h>` | Use Adapter APIs; config via CMAKE, not code |
| **Framework internal (stepper.c)** | `<zephyr/devicetree.h>`, `<robotos/ro_gpio.h>` | `<zephyr/drivers/gpio.h>` | DT is config language; adapter hides driver details |
| **Adapter impl (ro_thread.c)** | `<zephyr/kernel.h>`, `<robotos/ro_thread.h>` | Application code | Zephyr leaks in here—and STOPS here |

**General Rule:**  
✅ **App & Framework public** → **Only Adapter + Framework headers** (0 Zephyr)  
✅ **Adapter impl** → **1 Zephyr header per file** (minimal, explicit)  
✅ **Framework internals** → **Adapter public APIs; Zephyr (DT, SysLib) private**

#### Header Naming Convention

| Layer | Header Prefix | Examples | Rationale |
|-------|--------------|---------|-----------|
| **Robot Adapter Layer** | `ro_` | `ro_thread.h`, `ro_queue.h`, `ro_gpio.h`, `ro_time.h` | Signals RTOS-primitive ownership; one `ro_` → one Zephyr concept |
| **Robot Framework Layer** | _(none)_ | `stepper.h`, `pid.h`, `servo.h`, `encoder.h` | Clean domain-semantic names; no RTOS dialect visible |

> **Rule:** If you see `#include <robotos/ro_pid.h>` anywhere — it is wrong.  
> `pid.h` is a Framework header; it does **not** wrap a single RTOS primitive.  
> Applying `ro_` to Framework headers violates the naming contract and blurs layer boundaries.

---

### Robot Adapter Layer API Design

#### Design Philosophy

**Goal:** Provide a SMALL, STABLE API that:
- Hides ALL Zephyr implementation details
- Answers: "How do I safely run code across RTOS + hardware boundaries?"
- Allows swapping Zephyr for another RTOS without breaking applications

```c
// ❌ WRONG: Direct Zephyr API usage in application
#include <zephyr/kernel.h>
struct k_thread my_thread;
k_thread_create(&my_thread, ...);

// ✅ RIGHT: Use Robot Adapter API
#include <robotos/ro_thread.h>
ro_thread_t* thread = ro_thread_create(&config);
```

#### Core Adapter APIs

**Threading API:**
```c
// ro_thread.h - Thread abstraction
typedef struct ro_thread ro_thread_t;

// ---------------------------------------------------------------------------
// ro_priority_t — opaque priority type. NEVER construct directly.
// Use RO_PRIO_* macros only. Assigning a raw int is a C compile error
// (int is not implicitly convertible to a struct compound literal).
typedef struct { int16_t v; } ro_priority_t;

// ---------------------------------------------------------------------------
// Priority presets — encode tier intent AND behavioral contract.
// Numeric values are the RobotOS default profile (see docs for retuning).
//
// ── COOPERATIVE tier  (preset.v < 0) ────────────────────────────────────────
// Scheduling: thread is NEVER preempted by another thread.
//
// CONTRACT — a cooperative thread MUST:
//   ✓ Call ro_thread_yield() at every loop iteration (or use ro_thread_sleep_ms to release CPU)
//   ✓ Keep worst-case runtime per yield bounded (document the bound in µs)
//   ✓ Use only ISR-safe IPC: ro_queue_send_isr (irq_lock path, no mutex), ro_pool_alloc_isr, atomics
//
// CONTRACT — a cooperative thread MUST NOT:
//   ✗ Acquire a mutex (priority inversion risk; coop thread never yields inside mutex wait)
//   ✗ Call any blocking Zephyr kernel API directly
//   ✗ Enter unbounded loops or spin-wait without yielding
//
// Per-preset sleep rules (override the general contract where noted):
//   RT_PULSE   — ro_thread_sleep_ms() FORBIDDEN. Timing must come from HW cycle or
//                external IRQ trigger. Sleep breaks the <2 µs jitter budget.
//   RT_CONTROL — ro_thread_sleep_ms() ALLOWED only when loop period ≥ 5 ms (≤ 200 Hz) and
//                jitter tolerance ≥ one scheduler tick (~1 ms). For any loop ≥ 200 Hz or
//                jitter budget < 1 ms, use ro_timer_sync() (hardware-timer-backed, bounded
//                by IRQ latency only). See §9 §3a "RT control loop timing — closed contract".
//   RT_MONITOR — ro_thread_sleep_ms() ALLOWED for watch interval (e.g. 10 ms heartbeat).
//
// RO_PRIO_RT_PULSE   — Software pulse/step generation.
//                      PREFER hardware PWM/timer output when available
//                      (STM32 TIM, ESP32 MCPWM, nRF52 PPI+GPIOTE). Use this preset
//                      ONLY when no HW timer output is possible on the target board.
//                      Jitter budget: <2 µs per cycle. No sleep, yield per IRQ only.
#define RO_PRIO_RT_PULSE    ((ro_priority_t){ .v = -16 })
//
// RO_PRIO_RT_CONTROL — Hard-RT control loops (PID ≥1 kHz, motion planner).
//                      Max bounded runtime per yield: ≤200 µs.
//                      Sleep allowed for loop rate; document chosen rate (e.g. 1 kHz).
#define RO_PRIO_RT_CONTROL  ((ro_priority_t){ .v = -10 })
//
// RO_PRIO_RT_MONITOR — Safety watchdog / fault detector. Short, fast, yields immediately.
//                      Max bounded runtime per yield: ≤50 µs.
//                      Sleep allowed for heartbeat interval.
#define RO_PRIO_RT_MONITOR  ((ro_priority_t){ .v =  -2 })

// ── PREEMPTIVE tier  (preset.v ≥ 0) ─────────────────────────────────────────
// Scheduling: thread MAY be preempted when a higher-priority thread is ready.
//
// CONTRACT — a preemptive thread:
//   ✓ May acquire mutexes (Zephyr priority inheritance is active)
//   ✓ May block on queues with timeout
//   ✓ May sleep for variable durations
//   ✓ Must NOT include any Zephyr headers — use Robot Adapter API only
//
#define RO_PRIO_FRAMEWORK   ((ro_priority_t){ .v =   0 })  // State machine, device manager
#define RO_PRIO_APP         ((ro_priority_t){ .v =   5 })  // GCode parser, trajectory planner
#define RO_PRIO_BACKGROUND  ((ro_priority_t){ .v =  12 })  // Logging, telemetry, statistics
// ---------------------------------------------------------------------------
//
// Compile-time enforcement:
//   cfg.priority = 0;              // ← C error: cannot assign int to struct
//   cfg.priority = RO_PRIO_APP;   // ← correct

// Stack ownership model: Model S (Strict Explicit).
// Caller ALWAYS provides a static stack buffer; adapter NEVER allocates one.
// Stack lifetime MUST exceed thread lifetime. Declare stack at same scope as
// the ro_thread_create() call. See Stack Ownership rules in Thread Ownership Model.
typedef struct {
    const char*   name;
    uint8_t*      stack;       // caller-provided static array — NEVER NULL
    uint32_t      stack_size;  // sizeof(stack); use sizeof(), not a magic number
    ro_priority_t priority;    // MUST be a RO_PRIO_* constant — compile-time enforced
    void        (*entry)(void* arg);
    void*         arg;
} ro_thread_config_t;

// Lifecycle: caller owns the returned pointer until ro_thread_destroy().
// Returns NULL on failure (stack NULL, stack too small, name conflict).
ro_thread_t*  ro_thread_create(const ro_thread_config_t* cfg);
// Abort the thread and release its stack slot back to the static pool.
void          ro_thread_destroy(ro_thread_t* thread);
void          ro_thread_sleep_ms(uint32_t ms);
void          ro_thread_yield(void);
```

**Timing API:**
```c
// ro_time.h - Time abstraction
//
// Clock contract:
//   - Source   : Zephyr system uptime (k_uptime_get / k_cycle_get_32)
//   - Monotonic: YES — never jumps backward, not affected by wall-clock sync
//   - Resolution: 1 µs for ro_time_us(), 1 ms for ro_time_ms()
//   - Overflow:
//       ro_time_us()  → uint64_t, wraps after ~584,942 years (treat as infinite)
//       ro_time_ms()  → uint32_t, wraps after ~49.7 days
//       Code that computes elapsed time MUST use subtraction with unsigned wrap:
//           uint32_t elapsed = ro_time_ms() - start_ms;  // correct even at wrap
//           if (elapsed > TIMEOUT_MS) { ... }            // never use '>' on raw values
uint64_t ro_time_us(void);
uint32_t ro_time_ms(void);
void     ro_delay_us(uint32_t us);
void     ro_delay_ms(uint32_t ms);
```

**IPC API:**
```c
// ro_queue.h - Message queue abstraction
// Opaque handle. Acquire via ro_queue_create(). NEVER declare ro_queue_t by value.
typedef struct ro_queue ro_queue_t;

// Lifecycle: caller owns the returned pointer until ro_queue_destroy().
// buf == NULL, buf_size == 0  → backing from RO_GLOBAL_IPC_SLAB (system/Framework queues).
// buf != NULL                 → caller-owned static buffer (Application queues; no slab).
// buf_size MUST equal item_size * capacity when buf != NULL; returns NULL on mismatch.
// Returns NULL on any failure. See ADAPTER_LAYER.md §Queue IPC for ownership tiers.
// <!-- STATUS: LOCKED DEC-01 -->
ro_queue_t*  ro_queue_create(size_t item_size, size_t capacity,
                              void* buf, size_t buf_size);
void         ro_queue_destroy(ro_queue_t* q);  // Caller must ensure no threads are blocked on q.
ro_status_t  ro_queue_send(ro_queue_t* q, const void* data, uint32_t timeout_ms);
ro_status_t  ro_queue_recv(ro_queue_t* q, void* data, uint32_t timeout_ms);
// ISR-safe variants (non-blocking, no timeout parameter):
ro_status_t  ro_queue_send_isr(ro_queue_t* q, const void* data);
ro_status_t  ro_queue_recv_isr(ro_queue_t* q, void* data);
```

### Robot Framework Components

#### Stepper Motor Component

**API:**
```c
// robotos/stepper.h
// ⚠️  Header naming: "stepper.h", NOT "ro_stepper.h".
//     Framework headers use NO ro_ prefix (see Header Naming Convention).
//     Consequently, the exported type is stepper_t, NOT ro_stepper_t.
typedef struct stepper stepper_t;

// Motion profile configuration — set once via stepper_configure() after stepper_get().
typedef struct {
    uint32_t max_speed_steps_s;    // Maximum speed (steps/second)
    uint32_t accel_steps_s2;       // Acceleration (steps/second²)
    uint32_t decel_steps_s2;       // Deceleration (steps/second²)
    uint16_t microsteps;           // 1, 2, 4, 8, 16, 32
    bool     reverse_direction;    // Flip direction polarity
} stepper_config_t;

// Lifecycle:
//   stepper_get(dt_label) — acquire a pre-bound handle by Device Tree label string.
//                           Returns NULL if the label is not found or hardware not ready.
//                           MUST be called at setup/init, NOT in the hot control loop.
//                           (See §10 Device Registry concurrency hazard.)
//   stepper_put(st)       — release handle back to the pool.
//                           DOES NOT stop motion. Caller MUST call stepper_stop()
//                           before stepper_put() if motion is active.
stepper_t*  stepper_get(const char* dt_label);         // RO_ISO_GUARDED
void        stepper_put(stepper_t* stepper);            // RO_ISO_GUARDED
ro_status_t stepper_configure(stepper_t* st, const stepper_config_t* cfg);
ro_status_t stepper_move(stepper_t* stepper, int32_t steps);      // RO_ISO_TRUSTED
ro_status_t stepper_move_to(stepper_t* stepper, int32_t position); // RO_ISO_TRUSTED
void        stepper_stop(stepper_t* stepper);            // RO_ISO_TRUSTED
void        stepper_emergency_stop(stepper_t* stepper);  // RO_ISO_TRUSTED — no decel ramp
int32_t     stepper_get_position(const stepper_t* stepper); // atomic read — ISR-safe
ro_status_t stepper_wait_idle(stepper_t* st, uint32_t timeout_ms);

// Error model rule (applies to ALL RobotOS APIs — Adapter + Framework):
//   - Functions that acquire a resource return a typed pointer (NULL = failure).
//   - All other operations return ro_status_t.  Never mix the two patterns.
//   - RO_OK == 0, so callers may use:  if (stepper_move(...) != RO_OK) { handle_err(); }
```

**State Diagram:**
```
         ┌────────────┐
         │    IDLE     │
         └──────┬──────┘
                │
     stepper_move()
                │
                ▼
         ┌────────────┐
    ┌────┤ ACCELERATING├────┐
    │    └──────┬──────┘    │
    │           │           │
    │           ▼           │
    │    ┌────────────┐    │
    │    │  RUNNING   │    │
    │    └──────┬──────┘    │
    │           │           │
    │           ▼           │
    │    ┌────────────┐    │
    └────► DECELERATING├────┘
         └──────┬──────┘
                │ (target reached
                │  or stop())
                ▼
         ┌────────────┐
         │    IDLE     │
         └────────────┘
```

#### ServoPWM Component

**API:**
```c
// robotos/servo.h
// Header name: "servo.h", NOT "ro_servo.h". Type: servo_t, NOT ro_servo_t.
typedef struct servo servo_t;

typedef struct {
    uint32_t min_pulse_us;   // Default 1000 µs
    uint32_t max_pulse_us;   // Default 2000 µs
    uint32_t period_us;      // Default 20000 µs (50 Hz)
} servo_config_t;

// Lifecycle: same get/put pattern as stepper — acquire once at init, hold for lifetime.
servo_t*    servo_get(const char* dt_label);          // RO_ISO_GUARDED
void        servo_put(servo_t* servo);                // RO_ISO_GUARDED
ro_status_t servo_configure(servo_t* s, const servo_config_t* cfg);
// Set angle: 0.0 = min pulse, 1.0 = max pulse. Returns RO_EINVAL outside [0.0, 1.0].
ro_status_t servo_set_angle(servo_t* servo, float angle); // normalised [0.0, 1.0]
ro_status_t servo_set_pulse_us(servo_t* servo, uint32_t pulse_us);
ro_status_t servo_disable(servo_t* servo);
```

#### PID Controller Component

**API:**
```c
// robotos/pid.h
// Header name: "pid.h" (no ro_ prefix — Framework naming rule).
// Type: pid_ctrl_t (visible struct — declare by value or embed; no factory required).
// <!-- STATUS: LOCKED DEC-07 -->
typedef struct {
    float kp;              // Proportional gain
    float ki;              // Integral gain
    float kd;              // Derivative gain
    float output_min;      // Output clamp minimum
    float output_max;      // Output clamp maximum
    float integral_min;    // Anti-windup clamp minimum  ← required, not optional
    float integral_max;    // Anti-windup clamp maximum
    float sample_time_s;   // Control loop sample period (seconds)
} pid_ctrl_config_t;

// Visible struct — caller declares by value (no factory, no heap).
typedef struct {
    pid_ctrl_config_t cfg;
    float integral;
    float prev_error;
    float prev_output;
} pid_ctrl_t;

// Lifecycle: initialise in-place with pid_ctrl_init().
ro_status_t pid_ctrl_init(pid_ctrl_t* pid, const pid_ctrl_config_t* cfg);

// Compute: runs on caller's thread. RO_ISO_TRUSTED — must not block.
// No dt_s parameter — sample period is fixed via cfg.sample_time_s at init time.
float       pid_ctrl_update(pid_ctrl_t* pid, float setpoint, float measurement);

void        pid_ctrl_reset(pid_ctrl_t* pid);    // Reset integral and derivative state.
ro_status_t pid_ctrl_set_gains(pid_ctrl_t* pid, float kp, float ki, float kd);  // RO_ISO_GUARDED
```

**Example Usage:**
```c
// Motor speed control
static pid_ctrl_t pid;
static const pid_ctrl_config_t pid_cfg = {
    .kp = 1.0f,
    .ki = 0.1f,
    .kd = 0.05f,
    .output_min = -1.0f,   // normalised to dcmotor_set_speed() range
    .output_max =  1.0f,
    .integral_min = -0.5f,
    .integral_max =  0.5f,
    .sample_time_s = 0.01f  // 100 Hz
};

pid_ctrl_init(&pid, &pid_cfg);

// Control loop (runs at 100 Hz)
while (1) {
    float speed = encoder_get_velocity(encoder, 10 /*window_ms*/);  // ticks/s
    float output = pid_ctrl_update(&pid, target_speed, speed);
    dcmotor_set_speed(motor, output);   // -1.0..+1.0 range
    ro_delay_ms(10);
}
```

#### Encoder Component

**API:**
```c
// robotos/encoder.h
// Header name: "encoder.h", NOT "ro_encoder.h". Type: encoder_t, NOT ro_encoder_t.
typedef struct encoder encoder_t;

// Lifecycle: get/put pattern consistent with stepper and servo.
encoder_t*  encoder_get(const char* dt_label);          // RO_ISO_GUARDED
void        encoder_put(encoder_t* enc);                // RO_ISO_GUARDED

int32_t     encoder_get_count(const encoder_t* enc);    // atomic read — ISR-safe
void        encoder_reset(encoder_t* enc);
uint32_t    encoder_get_ticks_per_rev(const encoder_t* enc);
// velocity: rolling average over window_ms milliseconds
float       encoder_get_velocity(const encoder_t* enc, uint32_t window_ms); // ticks/s
```

### Robot State Machine

#### API Design

```c
// robotos/robot_sm.h — Framework Robot State Machine
// Header naming: "robot_sm.h" (no ro_ prefix — Framework naming rule).
// Source of truth for type names: FRAMEWORK_LAYER.md §Robot State Machine API.
// ARCHITECTURE.md is the authority on state diagram and transition semantics below.
// <!-- STATUS: LOCKED DEC-07 -->

typedef enum {
    ROBOT_STATE_IDLE    = 0,   // Waiting for commands
    ROBOT_STATE_HOMING  = 1,   // Finding home position
    ROBOT_STATE_RUN     = 2,   // Executing motion
    ROBOT_STATE_FAULT   = 3,   // Safety stop — requires reset
} robot_state_t;

typedef enum {
    ROBOT_CMD_HOME              = 0,
    ROBOT_CMD_START             = 1,
    ROBOT_CMD_STOP              = 2,
    ROBOT_CMD_EMERGENCY_STOP    = 3,
    ROBOT_CMD_FAULT             = 4,   // Trigger fault from any state
    ROBOT_CMD_HOMING_COMPLETE   = 5,
} robot_cmd_t;

// Transition callback — invoked synchronously inside sm_dispatch().
typedef void (*sm_transition_cb_t)(robot_state_t from,
                                    robot_state_t to,
                                    robot_cmd_t   cmd,
                                    void*         user_data);

typedef struct sm sm_t;    // Opaque. Acquire via sm_create(). Never dereference directly.

sm_t*         sm_create(void);
void          sm_destroy(sm_t* sm);
ro_status_t   sm_dispatch(sm_t* sm, robot_cmd_t cmd);   // Returns RO_EINVAL for invalid transition
robot_state_t sm_get_state(const sm_t* sm);
ro_status_t   sm_set_transition_cb(sm_t* sm, sm_transition_cb_t cb, void* user_data);
ro_status_t   sm_set_fault_reason(sm_t* sm, int32_t reason);
int32_t       sm_get_fault_reason(const sm_t* sm);
ro_status_t   sm_clear_fault(sm_t* sm);
```

#### State Diagram (Detailed)

```
                 ┌─────────────────┐
                 │      IDLE       │
                 │ (Ready to work) │
                 └───────┬─────────┘
                        │
          ┌─────────────┼──────────────┐
          │ ROBOT_CMD_HOME  │ ROBOT_CMD_START
          ▼             ▼
   ┌──────────────┐  ┌─────────────────┐
   │   HOMING     │  │      RUN        │
   │ (Find home)  │  │ (Execute task)  │
   └──────┬────────┘  └────────┬────────┘
          │                    │
  HOMING_COMPLETE │    ROBOT_CMD_STOP │
          │                    │
          └────────┬────────────┘
                  │
                  ▼
          ┌─────────────────┐
          │      IDLE       │
          └────────┬─────────┘
                  │
  ROBOT_CMD_FAULT │  ◄──────────────── ROBOT_CMD_FAULT
  (from any)      │               (from any)
                  ▼
          ┌─────────────────┐
          │     FAULT       │
          │ (Safety stop)   │
          └────────┬─────────┘
                  │
  sm_clear_fault()│
                  ▼
          ┌─────────────────┐
          │      IDLE       │
          └─────────────────┘
```

---

## Error Handling & Status Codes

### `ro_status_t` Definition

`ro_status_t` is the **single return-code type** for all RobotOS operations. It is an `int32_t` alias — negative values are errors, zero is success, positive values carry extra diagnostic info.

```c
// ro_status.h — the only error type in public RobotOS API
// Rule: NEVER expose errno.h, Zephyr error codes, or POSIX codes in public headers.
//       Translate them here and nowhere else.

typedef int32_t ro_status_t;

// ── Success ─────────────────────────────────────────────────────────────────
#define RO_OK              0    // Operation succeeded

// ── Operational errors (deterministic; part of expected control flow) ───────
#define RO_ETIMEDOUT      -1    // Operation timed out (queue receive, mutex wait)
#define RO_EAGAIN         -2    // Resource temporarily unavailable; caller should retry
#define RO_EBUSY          -3    // Resource in use (device occupied, state wrong)
#define RO_ECANCELED      -4    // Operation cancelled (e.g. stop() called mid-move)

// ── Programming errors (should never happen in correct code) ────────────────
#define RO_EINVAL         -10   // Invalid argument (NULL pointer, out-of-range value)
#define RO_ENOMEM         -11   // Pool exhausted — pool budget too small
#define RO_ENODEV         -12   // Device handle is NULL or not initialised
#define RO_ENOTSUP        -13   // Operation not supported on this platform/config
#define RO_EOVERFLOW      -14   // Buffer or counter overflow

// ── Fatal / boot errors (logged; triggers FAULT state) ─────────────────────
#define RO_EFAIL          -20   // Generic unrecoverable failure (use specific codes first)
#define RO_ENOTREADY      -21   // Component not initialised; robotos_init() not called
#define RO_EHWINIT        -22   // Hardware init failed (device_is_ready() returned false)
#define RO_EBOOTSEQ       -23   // Called out-of-order relative to boot sequence
```

**Convenience macros:**
```c
// Test macros — prefer these over direct comparison
#define RO_SUCCEEDED(s)   ((s) == RO_OK)
#define RO_FAILED(s)      ((s) != RO_OK)
#define RO_IS_FATAL(s)    ((s) <= RO_EFAIL)

// Propagate pattern — most common usage:
#define RO_TRY(expr)  do { ro_status_t _s = (expr); if (_s != RO_OK) return _s; } while (0)

// Example:
static ro_status_t arm_init(void) {
    RO_TRY(stepper_move_to(shoulder, 0));
    RO_TRY(stepper_move_to(elbow, 0));
    return RO_OK;
}
```

---

### Zephyr errno Mapping

Zephyr uses negative POSIX errno codes internally (e.g., `-EINVAL`, `-ENOMEM`). The **adapter layer is the only place that translates** these into `ro_status_t`. Application and Framework code never sees raw Zephyr errnos.

```c
// Adapter-internal helper — lives in ro_status.c, NOT in public header
static inline ro_status_t ro_from_zephyr(int zephyr_err) {
    // zephyr_err is typically -(POSIX errno), e.g. k_msgq_put returns -ENOMSG
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
        default:         return RO_EFAIL;  // Unknown Zephyr error → generic fatal
    }
}
```

**Mapping table:**

| Zephyr code | Meaning in Zephyr | `ro_status_t` | When it surfaces |
|-------------|------------------|---------------|------------------|
| `0` | Success | `RO_OK` | Any successful op |
| `-ETIMEDOUT` | Timeout expired | `RO_ETIMEDOUT` | `k_msgq_get` timeout, `k_sem_take` timeout |
| `-EAGAIN` | Not ready right now | `RO_EAGAIN` | ISR queue full, non-blocking send |
| `-EBUSY`  | Device busy | `RO_EBUSY` | Peripheral in-use, mutex contention |
| `-ENOMEM` | Out of memory | `RO_ENOMEM` | Pool exhausted, stack alloc failed |
| `-EINVAL` | Bad argument | `RO_EINVAL` | NULL pointers, out-of-range config |
| `-ENODEV` | No device | `RO_ENODEV` | `DEVICE_DT_GET` node absent, device not ready |
| `-ENOTSUP` | Not supported | `RO_ENOTSUP` | Feature disabled via Kconfig |
| `-EOVERFLOW` | Overflow | `RO_EOVERFLOW` | Ring buffer full, counter wrap |
| other | Any other Zephyr error | `RO_EFAIL` | Catch-all; log the original code |

> **Log the original:** when mapping to `RO_EFAIL`, always `RO_LOG_ERROR("zephyr err=%d", zephyr_err)` before returning.

---

### Return Value Convention

Two patterns, chosen by what the function does — **never mix them**:

| Pattern | Function type | Return on success | Return on failure | Example |
|---------|--------------|-------------------|-------------------|---------|
| **Alloc/init** | Creates or acquires a resource | `non-NULL pointer` | `NULL` | `ro_queue_create()`, `stepper_get()`, `pid_ctrl_init()` |
| **Operation** | Acts on an existing handle | `RO_OK` (= 0) | negative `ro_status_t` | `stepper_move()`, `ro_queue_send()`, `servo_set_angle()` |

```c
// ✅ CORRECT — alloc returns pointer, NULL on fail
static uint8_t cmd_buf[sizeof(cmd_t) * 16];
ro_queue_t* q = ro_queue_create(sizeof(cmd_t), 16, cmd_buf, sizeof(cmd_buf));
if (q == NULL) { return RO_ENOMEM; }   // caller converts to ro_status_t

// ✅ CORRECT — operation returns ro_status_t
ro_status_t s = ro_queue_send(q, &cmd, 10);
if (s != RO_OK) { return s; }

// ❌ WRONG — mixing patterns
ro_status_t s = ro_queue_create(...);  // returns pointer, not status!
ro_queue_t* q = stepper_move(...);     // returns status, not pointer!
```

> **ISR rule:** ISR-safe variants (`ro_queue_send_isr`) are **non-blocking** and return only `RO_OK` or `RO_EAGAIN`. They never return `RO_ETIMEDOUT` (no timeout parameter).

---

### Error Propagation Rules

1. **Propagate, don't swallow.** If a callee returns an error, the caller either handles it completely (logs + takes action) or returns it unchanged. Silently ignoring an `ro_status_t` is a bug.

2. **`RO_ASSERT` vs `ro_status_t` — different audiences:**

```c
// RO_ASSERT — programming contract violations (should never happen in correct code)
// Compiled out in release; aborts/panics in debug.
// Use for: NULL deref guards, enum range checks, class invariants.
RO_ASSERT(stepper != NULL, "stepper handle is NULL — caller bug");
RO_ASSERT(cfg->stack_size > 0, "stack_size must be > 0");

// ro_status_t — runtime operational conditions (expected in correct code)
// Always present in release. Caller MUST handle.
// Use for: timeouts, resource exhaustion, hardware transients.
ro_status_t s = ro_queue_send(q, &msg, timeout_ms);
if (s == RO_ETIMEDOUT) { /* retry or drop */ }
```

3. **ISR-safe logging only:** in ISR context, `RO_LOG_*` is forbidden (UART is blocking). Set an `atomic_t fault_flags` bitmask and log it from a monitor thread.

4. **Fatal errors → FAULT state:** Any `RO_IS_FATAL(s)` result from a Framework component MUST trigger `sm_dispatch(sm, ROBOT_CMD_FAULT)`. Never continue operation after a fatal status.

```c
// Canonical fatal error handler in control loop:
ro_status_t s = stepper_move(stepper, steps);
if (RO_IS_FATAL(s)) {
    atomic_set(&fault_flags, FAULT_STEPPER_INIT);
    sm_dispatch(sm, ROBOT_CMD_FAULT);   // DEC-07: canonical SM dispatch
    return;  // Exit loop; monitor thread will handle
}
```

---

## Boot Sequence & Initialization Contract

### Zephyr SYS_INIT Levels

Zephyr boots in ordered phases. RobotOS maps its own init work to explicit levels — no component may register outside its assigned level.

```
Power-on Reset
      │
      ▼
┌─────────────────────────────────────────────────────────────┐
│  PRE_KERNEL_1  (priority 0–99)                              │
│  Zephyr: interrupt controller, clock, SysTick              │
│  RobotOS: ro_log_backend_early_init() — UART ring buffer    │
│  Constraint: NO kernel services (no semaphore, no thread)   │
└─────────────────────────┬───────────────────────────────────┘
                          │
                          ▼
┌─────────────────────────────────────────────────────────────┐
│  PRE_KERNEL_2  (priority 0–99)                              │
│  Zephyr: kernel data structs, heap, device driver early     │
│  RobotOS: ro_pool_sys_init() — static pool table setup      │
│  Constraint: NO threads; pools/queues may be created        │
└─────────────────────────┬───────────────────────────────────┘
                          │
                          ▼
┌─────────────────────────────────────────────────────────────┐
│  POST_KERNEL  (priority 0–99)                               │
│  Zephyr: SPI, I2C, GPIO, PWM driver init via DT             │
│  RobotOS: Framework driver bind (stepper_drv_bind, etc.)    │
│           ro_trace_init() — tracing backend                 │
│  Constraint: threads allowed; malloc STILL forbidden        │
└─────────────────────────┬───────────────────────────────────┘
                          │
                          ▼
┌─────────────────────────────────────────────────────────────┐
│  APPLICATION  (priority 0–99)                               │
│  Zephyr: final subsystems, shell, networking                │
│  RobotOS: robotos_init() — creates system threads           │
│           User main() entry                                 │
│  Constraint: all kernel services available                  │
└─────────────────────────┬───────────────────────────────────┘
                          │
                          ▼
             main() / app_init() — user creates app threads
```

**RobotOS SYS_INIT registration table:**

| Module | Level | Priority | What it does | Fails → |
|--------|-------|----------|-------------|----------|
| `ro_log_backend` | `PRE_KERNEL_1` | 0 | Init UART ring buffer for early logs | Logs silently lost; non-fatal |
| `ro_pool_sys` | `PRE_KERNEL_2` | 0 | Set up static pool descriptor table | `RO_EHWINIT`; boot halts |
| `ro_stepper_drv` | `POST_KERNEL` | 10 | Bind DT stepper nodes to handle table | `RO_EHWINIT` per device; stored in `boot_status` |
| `ro_servo_drv` | `POST_KERNEL` | 10 | Bind DT servo/PWM nodes | Same as stepper |
| `ro_encoder_drv` | `POST_KERNEL` | 11 | Bind encoder counter devices | Same |
| `ro_trace` | `POST_KERNEL` | 20 | Init trace ring buffer, Zephyr tracing backend | Non-fatal; trace silently dropped |
| `robotos_init` | `APPLICATION` | 0 | Create `log_flush` + `state_machine` threads | `RO_EHWINIT`; boot halts |

> **Priority within a level:** lower number = earlier. Use gaps (0, 10, 11, 20) to allow future insertion without renumbering.

---

### `robotos_init()` Contract

`robotos_init()` is the **single application-visible init entry point**. It is called by Zephyr at `APPLICATION` level before `main()`.

```c
// ro_init.h — public declaration
//
// CONTRACT:
//   CALLED BY: Zephyr SYS_INIT(robotos_init, APPLICATION, 0) — NOT by user code.
//   IDEMPOTENT: No. Must be called exactly once. Second call returns RO_EBOOTSEQ.
//   THREAD-SAFE: Not applicable — called before user threads exist.
//   ON FAILURE: Stores status in ro_boot_status; dispatches ROBOT_CMD_FAULT to SM.
//               Boot continues so that fault telemetry can be sent.
//
ro_status_t robotos_init(void);

// Read-only after boot — inspect from app or monitor thread
typedef struct {
    ro_status_t overall;              // RO_OK if all modules init'd
    ro_status_t module[RO_MOD_MAX];   // Per-module status (index = ro_module_id_t)
    uint32_t    init_time_ms;         // Time from reset to robotos_init() complete
} ro_boot_status_t;

const ro_boot_status_t* ro_get_boot_status(void);
```

**What `robotos_init()` does (in order):**
1. Check all `POST_KERNEL` driver bindings succeeded — aggregate into `boot_status`
2. Create `log_flush` thread (`RO_PRIO_BACKGROUND`)
3. Create `state_machine` thread (`RO_PRIO_FRAMEWORK`), pass it `boot_status`
4. If `overall != RO_OK`: transition state machine to `FAULT` immediately
5. Return `RO_OK` (boot continues regardless; fault is handled by state machine)

**What `robotos_init()` must NOT do:**
- Call `malloc` / `k_malloc`
- Create application-domain threads (only well-known system threads)
- Block for more than 50 ms total
- Access uninitialised device handles

---

### Init Phase Constraints

Each component `_init()` function (called via SYS_INIT) must satisfy this contract:

```c
// Component init contract (enforced by code review + CI assert macros)
//
// ✅ ALLOWED at any init phase:
//   - Reading compile-time constants
//   - Populating static structs
//   - ro_pool_alloc() after PRE_KERNEL_2
//   - Zephyr device_is_ready() after POST_KERNEL
//   - ro_log_*() after PRE_KERNEL_1 (ring buffer only)
//
// ❌ FORBIDDEN at PRE_KERNEL_1 or PRE_KERNEL_2:
//   - ro_thread_create()       — kernel not ready
//   - ro_queue_create()        — kernel not ready
//   - k_mutex_lock()           — kernel not ready
//   - Any blocking call        — no scheduler
//
// ❌ FORBIDDEN at ALL init phases:
//   - malloc / free            — violates no-malloc rule unconditionally
//   - ro_thread_create()       — only robotos_init() and app may create threads
//   - Busy-waiting loops       — wastes boot budget; use SYS_INIT ordering instead
//
// IDEMPOTENCE:
//   Component _init() may be called once. Must check guard and return RO_OK
//   if already initialised (for testability), or assert if called twice.
```

**Boot status check pattern (app startup):**
```c
// main.c — first thing to do after robotos_init()
void main(void) {
    const ro_boot_status_t* bs = ro_get_boot_status();
    if (RO_FAILED(bs->overall)) {
        // State machine is already in FAULT.
        // Log details and wait for external reset / recovery command.
        RO_LOG_ERROR("Boot failed: overall=%d, stepper=%d, servo=%d",
                     bs->overall,
                     bs->module[RO_MOD_STEPPER],
                     bs->module[RO_MOD_SERVO]);
        return;  // State machine thread handles fault telemetry
    }

    // All hardware ready — proceed with app thread creation
    app_create_threads();
}
```

---

## Memory Architecture

### Memory Regions (Zephyr-based)

```
┌─────────────────────────────────────────┐ 0x20000000 (SRAM start)
│           Zephyr Kernel BSS/Data              │
├─────────────────────────────────────────┤
│         Robot Framework Static Data          │
├─────────────────────────────────────────┤
│                                         │
│      Application Memory Pools           │
│  ┌────────────────────────────────┐     │
│  │ Pool A: Device Command Buffers  │     │
│  │ (64 blocks × 128 bytes)        │     │
│  └────────────────────────────────┘     │
│  ┌────────────────────────────────┐     │
│  │ Pool B: Control State            │     │
│  │ (16 blocks × 64 bytes)          │     │
│  └────────────────────────────────┘     │
│                                         │
├─────────────────────────────────────────┤
│         Thread Stacks (Zephyr)          │
│  ┌────────────────────────────────┐     │
│  │ Thread 1: Main Loop (4KB)      │     │
│  ├────────────────────────────────┤     │
│  │ Thread 2: Control Loop (2KB)   │     │
│  ├────────────────────────────────┤     │
│  │ Thread 3: Comm Handler (2KB)   │     │
│  └────────────────────────────────┘     │
├─────────────────────────────────────────┤
│         Kernel Heap (reserved by Zephyr) │
│  ⚠️  RobotOS NEVER uses heap.            │
│      No malloc/free at any phase.        │
│      Pool region above replaces it.      │
└─────────────────────────────────────────┘ 0x2001FFFF (SRAM end)
```

### Memory Pool Design

#### Fixed-Block Allocator

```c
// Pool configuration (compile-time)
typedef enum {
    RO_POOL_LOCK_NONE    = 0,  // Single-owner, no locking — fastest, ISR-safe
    RO_POOL_LOCK_ATOMIC  = 1,  // Lock-free free-list via atomic CAS — ISR-safe
    RO_POOL_LOCK_MUTEX   = 2,  // Zephyr mutex with priority inheritance — NOT ISR-safe
} ro_pool_lock_t;

typedef struct {
    void*          buffer;      // Pre-allocated backing store (static array, NOT heap)
    size_t         block_size;  // Fixed block size (bytes)
    size_t         num_blocks;  // Total blocks
    ro_pool_lock_t lock_mode;   // See RT-safety contract below
} ro_pool_config_t;

// Pool API
// Lifecycle: pool handle is valid for program lifetime (no destroy needed for static pools).
ro_pool_t* ro_pool_create(const ro_pool_config_t* cfg);
void*      ro_pool_alloc(ro_pool_t* pool);              // Returns NULL if pool exhausted
void*      ro_pool_alloc_isr(ro_pool_t* pool);          // ISR-safe; only valid for LOCK_NONE / LOCK_ATOMIC
void       ro_pool_free(ro_pool_t* pool, void* ptr);
void       ro_pool_free_isr(ro_pool_t* pool, void* ptr);

// Statistics (for observability)
typedef struct {
    size_t used_blocks;
    size_t peak_used;       // High-water mark
    size_t alloc_failures;
} ro_pool_stats_t;
void ro_pool_get_stats(const ro_pool_t* pool, ro_pool_stats_t* out);
```

**RT-Safety Contract:**

| `lock_mode` | Thread-safe | ISR-safe | Priority inversion risk | When to use |
|---|---|---|---|---|
| `LOCK_NONE` | ❌ single owner | ✅ | None | One thread + ISR reads only |
| `LOCK_ATOMIC` | ✅ | ✅ | None | Multiple threads + ISR alloc/free |
| `LOCK_MUTEX` | ✅ | ❌ **forbidden in ISR** | Mitigated by Zephyr priority inheritance, but still adds jitter | Thread-only, non-RT paths |

> **Rule:** Any pool touched from an ISR context **must** use `LOCK_NONE` or `LOCK_ATOMIC`.
> Calling `ro_pool_alloc()` with `LOCK_MUTEX` from ISR is a hard error (assert in debug build).
> Use the `_isr` variants exclusively in interrupt handlers.

#### Budget Planning (Example: 3D Printer)

| Pool Name | Block Size | Count | Total | Usage |
|-----------|------------|-------|-------|-------|
| `msg_pool` | 256 B | 64 | 16 KB | Pub/Sub messages |
| `gcode_pool` | 128 B | 32 | 4 KB | GCode commands |
| `state_pool` | 64 B | 16 | 1 KB | Control state |
| **TOTAL** | | | **21 KB** | |

---

## Threading & Execution Model

### Zephyr Thread Model

**RobotOS uses Zephyr's native threading via the Robot Adapter Layer.**

#### Thread Creation Example

```c
// Application code using Robot Adapter API
#include <robotos/ro_thread.h>

// RO_PRIO_RT_CONTROL cooperative thread — 100 Hz PID loop.
// Contract: bounded runtime ≤200 µs per iteration; sleep releases CPU between cycles.
void control_loop_thread(void* arg) {
    pid_ctrl_t* pid = (pid_ctrl_t*)arg;
    while (1) {
        float output = pid_ctrl_update(pid, setpoint, measured);
        ro_thread_sleep_ms(10);  // 100 Hz; releases CPU cleanly (cooperative sleep)
    }
}

// Create thread — Model S: static stack declared at same scope as create call
static uint8_t control_loop_stack[2048];        // static: outlives the function frame

ro_thread_config_t cfg = {
    .name       = "control_loop",
    .stack      = control_loop_stack,           // caller always provides buffer
    .stack_size = sizeof(control_loop_stack),   // sizeof(), not a magic number
    .priority   = RO_PRIO_RT_CONTROL,           // cooperative, ≤200 µs/iter, no mutex
    .entry      = control_loop_thread,
    .arg        = &pid,
};
ro_thread_t* thread = ro_thread_create(&cfg);
```

**Behind the scenes (Robot Adapter implementation):**
```c
// ro_thread.c - uses Zephyr k_thread internally
#include <zephyr/kernel.h>

struct ro_thread {
    struct k_thread zephyr_thread;
    k_tid_t tid;
    // ... other fields
};

ro_thread_t* ro_thread_create(const ro_thread_config_t* cfg) {
    // Allocate and configure Zephyr thread
    // Users never see Zephyr types!
}
```

---

### Thread Ownership Model

#### The One-Line Rule

> **Every thread visible at runtime must have a single, named `ro_thread_create()` call in application-level or system-init code — never inside a library, component, or middleware.**

If you open the source and cannot find whose `ro_thread_create()` started a thread, it is a bug.

---

#### Who Creates Threads

Three entities are allowed to create threads; each has explicit rules:

| Creator | Allowed to call `ro_thread_create`? | Rule |
|---------|-------------------------------------|------|
| **Application** (e.g., `main.c`, `app_init.c`) | ✅ Yes | Primary locus for all thread creation. Must maintain a "thread roster" comment block. |
| **Framework system-init** (explicit `robotos_init()`) | ✅ Yes — only for declared well-known threads | Described in the thread roster; not user-configurable at this level. |
| **Framework component** (Stepper, PID, Servo, etc.) | ❌ Never internally | Components are **passive executors**: they run on whatever thread calls them. A component *may* expose a `ro_stepper_thread_config_t` helper so the **application** can pass it to `ro_thread_create`. |
| **Middleware library** (pub/sub, pool, queue) | ❌ Never | Libraries are pure data structures + algorithms. Thread-safe but thread-agnostic. |
| **Robot Adapter impl** (ro_thread.c, ro_gpio.c, etc.) | ❌ Never for users | Internal Zephyr glue only; never creates threads on its own. |

**Framework helper pattern (right way):**
```c
// ✅ CORRECT: framework gives you a config; YOU create the thread
ro_thread_config_t pulse_cfg = ro_stepper_pulse_thread_config(stepper, RO_PRIO_RT_PULSE);
ro_thread_t* pulse_thread = ro_thread_create(&pulse_cfg);  // App owns this

// ❌ WRONG: framework creates thread internally without informing app
ro_stepper_start(stepper);  // <-- hidden thread spawned inside? FORBIDDEN
```

---

#### Stack Ownership

**Rule: stack lifetime must match thread lifetime. Declare stack at the same scope as the `ro_thread_create` call.**

```c
// ✅ CORRECT: stack declared at same scope as create call (file-static)
static uint8_t control_stack[2048];

static void app_init(void) {
    ro_thread_config_t cfg = {
        .name       = "control",
        .stack      = control_stack,
        .stack_size = sizeof(control_stack),
        .priority   = RO_PRIO_RT_CONTROL,
        .entry      = control_loop,
        .arg        = &g_pid,
    };
    g_control_thread = ro_thread_create(&cfg);
}

// ❌ WRONG: stack on heap — forbidden
uint8_t* stack = malloc(2048);  // ← violates no-malloc rule

// ❌ WRONG: stack declared in caller's local frame — danged lifetime
void some_fn(void) {
    uint8_t local_stack[2048];  // ← destroyed when some_fn() returns!
    ro_thread_create(&(ro_thread_config_t){ .stack = local_stack, ... });
}
```

**Stack sizing convention:**
```c
// Each thread MUST name its stack size as a constant — never a magic number
#define CONTROL_THREAD_STACK_SZ   2048   // PID + motion planner
#define STATE_MACHINE_STACK_SZ    1536   // State machine + command parse
#define GCODE_PARSER_STACK_SZ     3072   // String work; larger stack needed
#define LOG_THREAD_STACK_SZ       1024   // Background, no deep calls
```

---

#### Executor ↔ Thread Mapping

In RobotOS, **each `ro_thread_t` is exactly one executor**: it is the entity that dequeues work and calls component functions. There is no separate "executor" abstraction—the thread IS the executor.

**Mapping rules:**

| Executor (Thread) | What it runs | When it runs |
|-------------------|-------------|--------------|
| `control_loop` (`RO_PRIO_RT_CONTROL`) | `pid_ctrl_update()`, step calculation, encoder read | Every tick (`ro_timer_sync(&ctrl_timer)` for ≥200 Hz; `ro_thread_sleep_ms(dt)` for ≤200 Hz only) |
| `state_machine` (`RO_PRIO_FRAMEWORK`) | State transitions, command dispatch, device manager | On `ro_queue_recv()` event from app or ISR |
| `gcode_parser` (`RO_PRIO_APP`) | GCode line parsing, trajectory push | On line-received queue event |
| `log_flush` (`RO_PRIO_BACKGROUND`) | Drain log ring buffer to UART | Periodic or threshold trigger |
| (ISR / hardware callback) | Step pulse emit, encoder count, endstop detect | Hardware interrupt — NOT a thread |

**Framework component callbacks always run on the caller's thread:**
```c
// Thread: control_loop  (RO_PRIO_RT_CONTROL)
void control_loop(void* arg) {
    while (1) {
        // pid_ctrl_update() runs on THIS thread — no hidden dispatch
        float out = pid_ctrl_update(&pid, setpoint, measured);

        // encoder_get_velocity() runs on THIS thread — no background polling thread
        float speed = encoder_get_velocity(&enc, 10 /*window_ms*/);

        // Use ro_timer_sync() — NOT ro_thread_sleep_ms — for ≥1 kHz loops.
        // ro_thread_sleep_ms(1) can drift by a full tick; ro_timer_sync() is IRQ-latency bounded.
        ro_timer_sync(&ctrl_timer);  // blocks until hardware timer fires
    }
}
```

**Nothing in `pid_ctrl_update` or `encoder_get_velocity` is allowed to:**
- Block internally on a semaphore
- Wake another thread
- Touch a queue without the caller knowing

---

#### "No Magic Thread" — Precise Contract

A **"magic thread"** is defined as:
> A thread started by any code path NOT reachable by a direct traceable call from application or system-init code.

**Violations (all forbidden):**

```c
// ❌ MAGIC: library constructor creates a background sampling thread
void encoder_init_bad(encoder_t* enc) {
    k_thread_create(&enc->bg_thread, ...);  // hidden! app didn't ask for this
}

// ❌ MAGIC: component lazy-starts a thread on first use
float encoder_get_velocity_bad(encoder_t* enc, uint32_t window_ms) {
    if (!enc->poll_thread_started) {
        ro_thread_create(...);  // hidden! never appears in app init
    }
    ...
}

// ❌ MAGIC: middleware creates timer callback that dispatches work to an internal pool
ro_pub_create() {
    // internally creates a k_work_q — a hidden executor
}
```

**Compliant (allowed):**

```c
// ✅ NOT MAGIC: component documents its required companion thread; app creates it
// In stepper.h:
//   Optional: on boards without HW step output, create a pulse thread:
//   ro_thread_config_t cfg = ro_stepper_pulse_thread_config(stepper);
//   Call ro_thread_create(&cfg) in your app init.

// ✅ NOT MAGIC: system-init function documents every thread it spawns
// robotos_init() creates: "state_machine", "log_flush"
// These are listed in the thread roster below.
```

---

#### Thread Roster Template

Every application MUST maintain a thread roster block in its main init file:

```c
/**
 * THREAD ROSTER — 3D Printer Demo (v0.1)
 * ============================================================
 * Rule: every thread created by this app is listed here.
 *       No thread may exist at runtime that is not in this table.
 *
 * Name              | Stack   | Priority          | Executor role
 * ------------------|---------|-------------------|--------------------------
 * "control_loop"    | 2048 B  | RO_PRIO_RT_CONTROL| PID + step calc (1 kHz)
 * "state_machine"   | 1536 B  | RO_PRIO_FRAMEWORK | State transitions + cmds
 * "gcode_parser"    | 3072 B  | RO_PRIO_APP       | GCode parse + traj push
 * "log_flush"       | 1024 B  | RO_PRIO_BACKGROUND| UART log drain
 * ============================================================
 * IRQ/ISR (not threads — listed for completeness):
 *   TIM2_IRQHandler  → step pulse emit (HW timer, STM32)
 *   EXTI0_IRQHandler → endstop detect → push to state_machine queue
 */
```

**Enforcement at link time (future CI goal):**
```bash
# Verify running thread count matches roster at test startup
robotos-threadcheck --elf zephyr.elf --roster app/thread_roster.yml --fail-on-unknown
```

---

### Priority Assignment Strategy

> **Compile-time enforced via `ro_priority_t`:** `cfg.priority = 5` is a C compile error.
> `cfg.priority = RO_PRIO_APP` compiles. Raw integers are structurally impossible in app code.

**Zephyr scheduling class rule** (sign determines class, not just ordering):
- `preset.v < 0` → **cooperative**: never preempted; thread *must* yield explicitly each cycle.
- `preset.v ≥ 0` → **preemptive**: scheduler may context-switch at any tick.

> The exact numeric range depends on Zephyr Kconfig (`CONFIG_NUM_COOP_PRIORITIES`,
> `CONFIG_NUM_PREEMPT_PRIORITIES`). The values below (`-16`, `-10`, `0`, `5`, `12`) are the
> **RobotOS default profile** — not universal Zephyr constants. Retune in one header; all app
> code adapts automatically because it never contains raw numbers.

| Constant | Value¹ | Class | Allowed APIs | Forbidden APIs | Typical use |
|---|---|---|---|---|---|
| `RO_PRIO_RT_PULSE` | -16 | **Coop** | `ro_thread_yield`, `ro_queue_*_isr`, `ro_pool_alloc_isr`, `ro_time_us` | Any mutex, blocking queue, `ro_thread_sleep_ms` as a spin-delay | SW pulse gen — **prefer HW PWM/timer first** |
| `RO_PRIO_RT_CONTROL` | -10 | **Coop** | `ro_thread_yield`, lock-free queues, atomic pool, `ro_time_us/ms` | Any mutex, blocking call | PID ≥1 kHz, motion controller |
| `RO_PRIO_RT_MONITOR` | -2 | **Coop** | `ro_thread_yield`, `ro_time_us`, read-only state | Mutex, blocking I/O | Fault detector, safety watchdog |
| `RO_PRIO_FRAMEWORK` | 0 | Preempt | All Robot Adapter APIs, mutex (w/ timeout), queue with timeout | Direct `<zephyr/*>` headers | State machine, device manager |
| `RO_PRIO_APP` | 5 | Preempt | All Robot Adapter APIs | Direct `<zephyr/*>` headers | GCode parser, trajectory planner |
| `RO_PRIO_BACKGROUND` | 12 | Preempt | All Robot Adapter APIs | Time-sensitive operations | Logging, telemetry, statistics |

¹ RobotOS default profile. Actual Zephyr range governed by `CONFIG_NUM_COOP/PREEMPT_PRIORITIES`.

> **`RO_PRIO_RT_PULSE` note:** Software pulse threads exist for boards without a hardware timer
> output. On STM32 (TIM), ESP32 (MCPWM/RMT), or nRF52 (PPI+GPIOTE), the stepper driver uses
> HW peripheral directly — no thread is created and this preset is unused.

#### Kconfig Profile Discipline

RobotOS ships three build-time profiles that constrain which priority presets and how many
threads are permitted. Selecting a profile validates the configuration at CMake time
(a `FATAL_ERROR` if limits are exceeded).

| Profile | Kconfig symbol | Cooperative threads cap | `CONFIG_NUM_COOP_PRIORITIES` min | `CONFIG_NUM_PREEMPT_PRIORITIES` min | Typical target |
|---------|---------------|------------------------|----------------------------------|--------------------------------------|----------------|
| `MIN` | `RO_PROFILE_MIN` | 2 | 4 | 4 | Tiny MCU, proof-of-concept |
| `STD` | `RO_PROFILE_STD` | 6 | 8 | 8 | Production robot (default) |
| `DEV` | `RO_PROFILE_DEV` | unlimited | 16 | 16 | Development / tracing enabled |

**Kconfig fragment (`prj.conf` excerpt for STD profile):**
```conf
CONFIG_RO_PROFILE_STD=y
# Auto-set by profile; do not override unless you know the impact:
# CONFIG_NUM_COOP_PRIORITIES=8
# CONFIG_NUM_PREEMPT_PRIORITIES=8
```

**CMake range check (runs at configure time):**
```cmake
# cmake/robotos_profile_check.cmake
if(CONFIG_NUM_COOP_PRIORITIES LESS 8 AND RO_PROFILE_STD)
  message(FATAL_ERROR
    "RO_PROFILE_STD requires CONFIG_NUM_COOP_PRIORITIES >= 8. "
    "Got ${CONFIG_NUM_COOP_PRIORITIES}.")
endif()
```

> **Denylist rule (STD):** Threads created at `RO_PRIO_RT_PULSE` require explicit opt-in via
> `CONFIG_RO_ALLOW_SW_PULSE_THREAD=y`. Without it the build fails with a message directing
> the developer to use a hardware timer peripheral instead.

### Concurrency Model

```
┌─────────────────────────────────────────────────────────┐
│                    Application Layer                     │
│                                                          │
│  ┌───────────────┐  ┌───────────────┐  ┌───────────────┐  │
│  │   Thread 1    │  │   Thread 2    │  │   Thread 3    │  │
│  │ RT_CONTROL    │  │  FRAMEWORK    │  │  BACKGROUND   │  │
│  │ (coop, v=-10) │  │ (prmp, v= 0) │  │ (prmp, v=+12) │  │
│  │   Control     │  │  State M/C   │  │   Logging     │  │
│  └───────┬───────┘  └───────┬───────┘  └───────┬───────┘  │
│        │                  │                  │          │
└────────┼──────────────────┼──────────────────┼──────────┘
         │                  │                  │
         ▼                  ▼                  ▼
┌─────────────────────────────────────────────────────────┐
│              Robot Framework Layer                       │
│  ┌──────────────────────────────────────────────────┐   │
│  │  Device Registry (Mutex-Protectedshort lock only)│   │
│  │  ┌──────────┐  ┌──────────┐  ┌──────────┐       │   │
│  │  │ Stepper  │  │  Servo   │  │ Encoder  │       │   │
│  │  └──────────┘  └──────────┘  └──────────┘       │   │
│  └──────────────────────────────────────────────────┘   │
└───────────────────────────┬─────────────────────────────┘
                            │
                            ▼
┌─────────────────────────────────────────────────────────┐
│              Robot Adapter Layer (Thread-Safe)           │
│  ┌──────────────────────────────────────────────────┐   │
│  │  Queue | Pool | Mutex | Atomic | Time            │   │
│  │  GPIO  | PWM  | I2C   | SPI    | UART            │   │
│  └──────────────────────────────────────────────────┘   │
└───────────────────────────┬─────────────────────────────┘
                            │
                            ▼
┌───────────────────────────────────────────────────────────┐
│                   Zephyr Kernel                           │
│  ┌────────────────────────────────────────────────────┐   │
│  │  Scheduler (Cooperative/Preemptive)                │   │
│  │  - Thread ready queue                              │   │
│  │  - Context switch                                  │   │
│  │  - Priority inheritance                            │   │
│  └────────────────────────────────────────────────────┘   │
└───────────────────────────────────────────────────────────┘
```

**Thread-Safety Rules:**

#### 1. Definitions

- **Thread-safe** (used in the diagram label "Robot Adapter Layer (Thread-Safe)") means: the API function can be called concurrently from multiple preemptive threads without external synchronization, and the internal data structure remains consistent. It does **not** imply ISR-safe unless explicitly marked.
- **ISR-safe** means: can be called from an interrupt service routine (no sleeping, no mutex acquire, uses only spinlock/atomic internally). Every ISR-safe function has an `_isr` suffix convention (e.g., `ro_queue_send_isr`).
- **Re-entrant** means: can be called recursively or from a callback invoked during the same API call. Unless stated otherwise, RobotOS APIs are **not re-entrant** — calling `ro_publish()` from inside a pub/sub callback is undefined behavior.

#### 2. Per-subsystem safety matrix

| Subsystem | Thread-safe | ISR-safe | Re-entrant | Protection mechanism | Notes |
|---|---|---|---|---|---|
| Pub/Sub topic registry | ✅ Yes | ❌ No | ❌ No | `ro_mutex_t` (priority-inheriting) | Registry is accessed only at subscribe/unsubscribe, not during publish hot-path |
| Pub/Sub publish hot-path | ✅ Yes | ❌ No | ❌ No | Per-topic spinlock (short, **bounded**) | Enqueues to all subscriber queues; spinlock held for duration of enqueue loop. **Hard cap: `CONFIG_RO_PUBSUB_MAX_SUBS_PER_TOPIC` (default 4, STD profile max 8).** Each enqueue to a `k_msgq` is O(1) and non-blocking (`_isr` path); spinlock hold time is proportional to subscriber count × ~100 ns. Exceeding the cap is a CMake `FATAL_ERROR`. |
| `ro_queue_send` / `ro_queue_recv` | ✅ Yes | ❌ No | ❌ No | Internal `k_msgq` mutex | Blocks caller if full/empty (timeout-based) |
| `ro_queue_send_isr` / `ro_queue_recv_isr` | ✅ Yes | ✅ **Yes** | ❌ No | ISR-safe Zephyr k_msgq path (uses `irq_lock` critical section, not a mutex — no priority inversion, no sleep) | Non-blocking; returns `RO_EAGAIN` if full/empty — never sleeps or blocks |
| Memory pool (`RO_POOL_LOCK_NONE`) | ❌ No (single owner) | ✅ Yes | ❌ No | None | Fastest; caller guarantees exclusive access |
| Memory pool (`RO_POOL_LOCK_ATOMIC`) | ✅ Yes | ✅ **Yes** | ❌ No | CAS atomic | Multi-thread + ISR safe; no priority inversion |
| Memory pool (`RO_POOL_LOCK_MUTEX`) | ✅ Yes | ❌ No | ❌ No | `ro_mutex_t` | **Must not be called from ISR**; asserts in debug build |
| Timer manager | ✅ Yes | ✅ **Yes** | ❌ No | Spinlock (critical section ≤ 1 µs on Cortex-M4 @ 168 MHz) | Only reads/writes the expiry timestamp and ready-flag; no allocation |
| Device Registry (stepper/servo/encoder pool) | ✅ Yes (setup only) | ❌ No | ❌ No | `ro_mutex_t` | Thread-safe only during boot/teardown; see §10 (Isolation Profiles → Device Registry concurrency hazard) for runtime rules |
| `ro_time_us()` / `ro_time_ms()` | ✅ Yes | ✅ **Yes** | ✅ Yes | Atomic read of hardware counter | Pure read; no internal state mutation |
| `ro_deadline_begin/end` | ✅ Yes | ✅ **Yes** | ❌ No | Lock-free (uses `ro_time_us()` only) | Miss counter per-name uses atomic increment |
| `ro_log()` | ✅ Yes | ❌ No | ❌ No | Ring-buffer + lock-free enqueue | ISR logging is **forbidden**; use atomic fault flags instead (see Logging API) |
| `ro_trace` (SYS_PORT_TRACING) | ✅ Yes | ✅ **Yes** | ❌ No | Zephyr tracing infrastructure | Compiles to zero if `CONFIG_TRACING=n` |

#### 3. Cooperative thread slipping rule (implicit → explicit)

Cooperative threads (`RO_PRIO_RT_PULSE`, `RO_PRIO_RT_CONTROL`, `RO_PRIO_RT_MONITOR`) are **never preempted by the scheduler**. This means:

- If a cooperative thread fails to call `ro_thread_yield()` or `ro_thread_sleep_ms()` within its loop, it monopolizes the CPU. All other cooperative threads at equal or lower priority are starved indefinitely.
- **Rule:** Every cooperative thread MUST call `ro_thread_yield()` or `ro_thread_sleep_ms()` at least once per loop iteration. This is a hard architectural contract, not a suggestion.
- **Debug enforcement:** `CONFIG_RO_COOP_WATCHDOG=y` (STD/DEV profiles) installs a hardware timer that fires if any cooperative thread runs without yielding for more than `CONFIG_RO_COOP_MAX_HOLD_US` microseconds (default: 500 µs). Violation triggers `k_oops()`.

#### 3a. RT control loop timing — closed contract

`ro_thread_sleep_ms(1)` is **NOT sufficient** for ≥ 1 kHz loops. `sleep_ms` releases the CPU until the next scheduler tick (~1 ms), but tick-boundary alignment is non-deterministic: the actual wakeup can occur up to one full tick late, introducing cumulative phase drift of several hundred microseconds per second.

**Closed contract for `RO_PRIO_RT_CONTROL` loops:**

| Method | Jitter | When to use |
|--------|--------|-------------|
| `ro_thread_sleep_ms(dt)` | ≤ one scheduler tick (~1 ms) | `dt ≥ 5 ms` (≤ 200 Hz) only |
| `ro_timer_sync(&timer)` | ≤ hardware IRQ latency (~5–20 µs on Cortex-M4) | Any loop ≥ 200 Hz — **required at ≥ 1 kHz** |

**`ro_timer_sync` — the preferred pattern for ≥ 1 kHz:**

```c
// robotos/timer.h
// Blocks the caller until the next firing of 'timer' (hardware-timer-backed periodic timer).
// Never misses a cycle: if the handler fired while the thread was computing,
// ro_timer_sync() returns immediately (accumulated = 1).
// Returns the number of missed firings since last call (0 = on time, ≥1 = overrun).
uint32_t ro_timer_sync(ro_timer_t* timer);

// Configure and start a periodic hardware timer.
// period_us:  desired period in microseconds. Returns RO_OK, or RO_EINVAL if period_us == 0.
// No ISR callback — hardware step generation uses the Framework stepper driver directly.
// <!-- STATUS: LOCKED DEC-03 -->
ro_status_t ro_timer_start_periodic(ro_timer_t* timer, uint32_t period_us);
void        ro_timer_stop(ro_timer_t* timer);
```

**Control loop at 1 kHz (correct pattern):**

```c
static ro_timer_t ctrl_timer;  // static lifetime — never on stack after init

void control_loop_init(void) {
    ro_timer_start_periodic(&ctrl_timer, 1000);  // 1000 µs = 1 kHz
}

void control_loop(void* arg) {
    while (1) {
        uint32_t overruns = ro_timer_sync(&ctrl_timer);  // sleep until next tick
        if (overruns > 0) {
            ro_log_warn("control_loop: %u overrun(s) — compute exceeded budget", overruns);
            // Policy: continue (don't panic on single overrun), but trace for tuning.
        }

        float out = pid_ctrl_update(&pid, setpoint, encoder_get_count(&enc));
        stepper_move_relative(x_axis, (int32_t)out);
        // No ro_thread_sleep_ms() here — timing is owned by ro_timer_sync().
    }
}
```

> **`ro_thread_sleep_ms()` in RT_CONTROL is now explicitly dual-use:**
> - Acceptable for coarse-rate loops (dt ≥ 5 ms).
> - **Forbidden** for any loop with a period < 5 ms or a jitter budget < one scheduler tick.
> If jitter matters, always use `ro_timer_sync()`.

#### 4. Message data: Copy semantics (explicit definition)

> **v0.1 payload protocol — two approved patterns, no others:**
>
> | Payload size | Approved pattern | Forbidden |
> |---|---|---|
> | ≤ 64 bytes | Embed value inline in message struct (copy-on-send). Safe, zero-setup. | Pointer to stack/heap |
> | > 64 bytes | Pool-allocated slot (LOCK_ATOMIC) + pointer + sequence-number in message. Receiver calls `ro_pool_free()` after processing; seqno detects stale-slot reuse. | `malloc` / `free`; bare pointer without seqno |
>
> **Using any other pattern is an architecture violation.** If neither pattern fits your use-case, file a design note before v0.2 (`ro_pub_loan/commit` zero-copy path is the planned resolution).

**What "copy semantics by default" means precisely:**

`ro_queue_send(q, &msg, timeout_ms)` copies `item_size` bytes (as declared at `ro_queue_create()`) from `msg` into the queue's internal ring buffer at the moment of the call. After `ro_queue_send()` returns, the caller's `msg` variable is no longer referenced by the queue.

```c
// ✅ SAFE: msg is stack-allocated; queue holds its own copy
cmd_t msg = { .type = CMD_MOVE, .target = 1000 };
ro_queue_send(q, &msg, 100);  // copies 'msg' bytes into queue buffer
// msg can now go out of scope — queue's copy is independent
```

**The user's responsibility — three concrete rules:**

**Rule A — Pointer-in-struct: the queue copies the pointer, NOT what it points to.**

```c
// ❌ DANGEROUS: queue copies the pointer, but heap_data may be freed
typedef struct { int16_t* data; size_t len; } buf_msg_t;
int16_t* heap_data = allocate_somewhere(len);
buf_msg_t msg = { .data = heap_data, .len = len };
ro_queue_send(q, &msg, 100);   // copies 4+4 bytes (pointer + len) — NOT the array
free(heap_data);               // ← heap_data freed; receiver dereferences a dangling pointer

// ✅ CORRECT: embed the data inline, or use a pool-allocated block
typedef struct { int16_t data[MAX_SAMPLES]; size_t len; } buf_msg_inline_t;
```

**Rule B — Sender must not modify `msg` after `ro_queue_send()` returns for ISR-safe variant.**

For `ro_queue_send_isr()`, the copy-and-return guarantee is the same. However, if the same buffer is reused across multiple ISR firings (e.g., a DMA completion ISR that always writes to a fixed struct), the sender must ensure the struct is fully written before each call.

**Rule C — `item_size` declared at `ro_queue_create()` is the authoritative copy width.**

If a sender passes a subtype that is larger than `item_size` bytes, only the first `item_size` bytes are copied — the rest is silently truncated. This is a programming error. Always use `sizeof()` consistently:

```c
// item_size at create-time MUST equal sizeof(msg type) at send-time
static uint8_t cmd_buf[sizeof(cmd_t) * 16];
ro_queue_t* q = ro_queue_create(sizeof(cmd_t), 16, cmd_buf, sizeof(cmd_buf));  // item_size = sizeof(cmd_t)
...
cmd_t msg = { ... };
ro_queue_send(q, &msg, 100);  // ✅ correct: &msg is exactly sizeof(cmd_t) bytes

extended_cmd_t big = { ... };  // sizeof(extended_cmd_t) > sizeof(cmd_t)
ro_queue_send(q, &big, 100);  // ❌ silent truncation — extra fields lost
```

**Zero-copy (future v0.2):** When `ro_pub_loan / ro_pub_commit` is implemented, ownership transfers to the queue and the sender must not write to the buffer after `ro_pub_commit()`. This is the *opposite* ownership model from copy semantics — the two must never be mixed on the same queue.

---

## Isolation Profiles (RO_PROFILE)

### One-sentence thesis

RobotOS introduces **Isolation Profiles** so each component runs at the right isolation level (Trusted / Guarded / Sandbox), reducing overhead on realtime-critical paths while enforcing safety and architecture boundaries through CI.

> **Motivation:** Performance degradation in high-frequency systems comes not only from IPC latency but from **IPC frequency**, **state double bookkeeping across services**, and **capability indirection that hides kernel objects** — all compounding at scale. Robotics control loops behave exactly like frequency-sensitive systems: even tiny per-call overhead repeated at 1 kHz becomes measurable jitter. RobotOS therefore must not enforce a single isolation style everywhere.

---

### Why the isolation ladder

| Cost driver | RobotOS symptom | Profile remedy |
|---|---|---|
| IPC frequency × per-invocation overhead | Control-loop jitter if every call crosses a boundary | Place hot-path code in TRUSTED (direct call, no crossing) |
| State double bookkeeping | Duplicated counters when realtime logic is split across "services" | Co-locate tightly coupled realtime logic in TRUSTED/GUARDED |
| Capability indirection | Extra lookup cost per adapter call | TRUSTED/GUARDED allow co-managed fast-path; SANDBOX forces full indirection |

---

### Profile definitions

Exactly **3 profiles** are defined. Do not introduce new profiles without an architecture review.

```c
// include/robotos/ro_profile.h
typedef enum {
    RO_ISO_TRUSTED  = 0,   // Realtime-critical hot path — direct calls, minimal overhead
    RO_ISO_GUARDED  = 1,   // Trusted module with policy gate — all entry via gate API
    RO_ISO_SANDBOX  = 2,   // Third-party / ported code — message/RPC boundary only (future)
} ro_iso_profile_t;

// Component-level annotation macro (placed in the public header, once per component)
// Compiles to nothing at runtime; used by CI grep checks.
#define RO_PROFILE(p)  /* RO_ISO_PROFILE: p */
```

#### `RO_ISO_TRUSTED`

- **Intended for:** Realtime-critical hot path and OS core runtime glue.
- **Call path:** Direct function call (no RPC, no queue crossing).
- **Restrictions:** Small public surface; no blocking calls; no mutex acquisition; no `stepper_get()` / `servo_get()` / any GUARDED entry point.
- **Typical components:** `ro_time_us/ms`, `ro_deadline_begin/end`, PID `step()`, control-loop hot path, ISR-safe queue/pool primitives.

#### `RO_ISO_GUARDED`

- **Intended for:** Trusted modules that still need a policy gate (input validation, lifecycle checks, access control).
- **Call path:** Direct call, but **only through the module's gate API** — callers must not bypass the gate to access internal state.
- **Restrictions:** May acquire mutex (non-ISR); may use `ro_log`; may NOT appear on the hot-path of a TRUSTED caller.
- **Typical components:** Device Registry (`stepper_get/put`), State Machine runtime, `ro_pool` (LOCK_MUTEX mode), `ro_log`, Telemetry/Tracing.

#### `RO_ISO_SANDBOX` *(future-ready)*

- **Intended for:** Third-party code, complex ported stacks, vendor modules.
- **Call path:** Message/RPC boundary only — no direct calls from TRUSTED/GUARDED.
- **Restrictions:** Strictly no fast-path; intended for address-space isolation when supported.
- **Typical components:** Ported driver containers, optional vendor modules.

---

### Profile-to-component matrix

| Component | Profile | Rationale |
|---|---|---|
| `ro_time_us()` / `ro_time_ms()` | `TRUSTED` | Pure atomic reads; zero overhead; ISR-safe |
| `ro_deadline_begin()` / `ro_deadline_end()` | `TRUSTED` | Uses `ro_time_us()` only; lock-free |
| `ro_queue_send_isr` / `ro_queue_recv_isr` | `TRUSTED` | Non-blocking; ISR-safe; no lock |
| PID `step()` (control loop) | `TRUSTED` | Hot path; must not block |
| `ro_pool_alloc` (LOCK_NONE / LOCK_ATOMIC) | `TRUSTED` | Lock-free; ISR-safe |
| Device Registry (`stepper_get/put`) | `GUARDED` | Setup-time only; mutex-protected; forbidden at runtime |
| State Machine runtime | `GUARDED` | Policy enforcement; may log; not hot-path |
| PID `configure()` / gain update | `GUARDED` | Takes mutex; not called at 1 kHz |
| `ro_log()` | `GUARDED` | Ring-buffer enqueue; not ISR-safe |
| Telemetry / Tracing | `GUARDED` | Must not spam control-loop hot path |
| `ro_pool_alloc` (LOCK_MUTEX) | `GUARDED` | Blocking; NOT ISR-safe |
| External / ported driver stacks | `SANDBOX` | Future-ready stub; compile-time placeholder |

---

### Gate API pattern (GUARDED)

Every GUARDED component exposes a **gate function** as its single public entry point. The gate:

1. Validates all input arguments (`RO_EINVAL` on NULL / out-of-range)
2. Checks lifecycle state (asserts component is initialized)
3. Enforces ISR-safety rules (asserts not in ISR context if mutex-based)
4. Optionally updates a trace counter

```c
// Example: stepper_get() is the gate for the Device Registry
// RO_PROFILE(RO_ISO_GUARDED)
stepper_t* stepper_get(const char* dt_label) {
    // Gate check 1: valid argument
    if (dt_label == NULL) { return NULL; }  // RO_EINVAL

    // Gate check 2: lifecycle — pool must be initialized
    RO_ASSERT(pool_initialized, "stepper pool not initialized");

    // Gate check 3: ISR safety — pool_mutex is NOT ISR-safe
    RO_ASSERT(!k_is_in_isr(), "stepper_get() called from ISR — use pre-acquired handle");

    ro_mutex_lock(pool_mutex, RO_WAIT_FOREVER);
    // ... slot search ...
    ro_mutex_unlock(pool_mutex);
    return handle;
}
```

---

### CI enforcement

```bash
# 1. Every public header must declare RO_PROFILE(...)
for f in include/robotos/*.h; do
  if ! grep -q 'RO_ISO_PROFILE:' "$f"; then
    echo "ERROR: $f missing RO_PROFILE() annotation"
    exit 1
  fi
done

# 2. TRUSTED callers must not call GUARDED entry points
# (stepper_get / servo_get / encoder_get are GUARDED)
for f in $(grep -rl 'RO_ISO_PROFILE: RO_ISO_TRUSTED' include/); do
  if grep -qE 'stepper_get|servo_get|encoder_get|ro_log\(' "$f"; then
    echo "ERROR: TRUSTED component $f calls a GUARDED entry point"
    exit 1
  fi
done

# 3. SANDBOX must not call TRUSTED fast-path APIs directly
# (no direct ro_time_us, ro_deadline_*, ro_queue_*_isr from SANDBOX)
for f in $(grep -rl 'RO_ISO_PROFILE: RO_ISO_SANDBOX' include/ src/); do
  if grep -qE 'ro_time_us|ro_deadline_begin|ro_queue_send_isr' "$f"; then
    echo "ERROR: SANDBOX component $f calls TRUSTED fast-path APIs directly"
    exit 1
  fi
done

# 4. Glue layer boundary: only app_glue_robotos.c and app_glue_zephyr.c may
#    include <robotos/*.h>. All other app/ sources must use app-layer headers only.
# <!-- STATUS: LOCKED DEC-06 -->
for f in $(find src/app/ -name '*.c' | grep -v 'app_glue_'); do
  if grep -qE '#include\s*[<"]robotos/' "$f"; then
    echo "ERROR: $f includes robotos/* directly — only app_glue_robotos.c is permitted (DEC-06)"
    exit 1
  fi
done
```

---

### Device Registry concurrency hazard

The `stepper_get()` / `servo_get()` / `encoder_get()` pattern uses a **static pool protected by a single mutex**. If misused as a per-iteration lock, this becomes a textbook global lock.

#### When it is safe vs dangerous

| Contention scenario | Risk | Reason |
|---|---|---|
| Boot-time init only (all `get()` in `APPLICATION SYS_INIT`) | ✅ Negligible | Single-threaded boot — zero contention |
| Thread setup phase (`get()` once per thread before entering loop) | ✅ Low | Microsecond contention window; not repeated |
| Repeated `get/put` in hot loop (1000×/s) | ❌ **Global lock** | Serializes every thread on pool_mutex |
| `get/put` used as a per-call exclusive device lock | ❌ **Global lock disguised as protocol** | Pool mutex becomes de-facto global mutex |

#### Architecture contract that prevents the hazard

```
RULE: stepper_get() / servo_get() / encoder_get() MUST NOT be called
      from the hot control loop or from any RO_ISO_TRUSTED context.

RULE: Every component MUST call get() once at setup/init, hold the
      handle for its lifetime, and call put() only at teardown.

ENFORCEMENT: CI check — any RO_ISO_TRUSTED function that calls
             stepper_get() is a build failure (see CI enforcement above).
```

#### Shared handle pattern (correct runtime sharing)

```c
// ✅ CORRECT: acquire once at init; shared handle; per-operation protection
static stepper_t* x_axis;   // acquired once, shared across threads

void motion_thread(void* _) {
    // stepper_move() acquires the per-device mutex INTERNALLY before submitting the
    // motion command.  The caller does NOT need an external lock — calling stepper_move()
    // concurrently from two threads on the SAME handle is safe (one will block briefly).
    // However, overlapping motion commands on the same axis are a LOGIC error (last write
    // wins after the first completes).  The app should serialize commands at a higher level
    // (e.g., via a command queue) to avoid unintended trajectory interference.
    stepper_move(x_axis, 400);                    // acquires per-device mutex internally
    int32_t pos = stepper_get_position(x_axis);   // atomic read — no mutex taken
}
void monitor_thread(void* _) {
    int32_t pos = stepper_get_position(x_axis);   // atomic read — no contention
    if (pos > SOFT_LIMIT) { stepper_emergency_stop(x_axis); }  // acquires per-device mutex internally
}
// Summary of stepper_t internal locking contract:
//   stepper_move / stepper_move_to / stepper_stop / stepper_emergency_stop :
//       acquire per-device mutex → submit command → release mutex
//   stepper_get_position : atomic read — no mutex
//   stepper_configure :    acquire per-device mutex (GUARDED — setup phase only)
// Two threads operating on DIFFERENT steppers never contend — per-device mutexes are independent.
```

#### RCU-lite optimization (lock-free runtime lookup)

If runtime label lookup becomes necessary:
- Phase 1 (POST_KERNEL, boot): pool_mutex locked → all devices registered → pool **frozen permanently**
- Phase 2 (runtime): pool is read-only → linear scan needs no lock (read-after-write ordering guaranteed by SYS_INIT barrier)

| Risk | Mitigated? | Mechanism |
|---|---|---|
| Pool mutex as global lock (hot path) | ✅ Yes | `get/put` forbidden hot-path; CI-enforced |
| Priority inversion on pool mutex | ✅ Yes | `ro_mutex_t` is priority-inheriting by design |
| Contention between two different devices | ✅ Yes | Per-device mutex in `stepper_t`; pool mutex for slot management only |
| Dynamic acquire/release in hot loop | ❌ Explicitly forbidden | Architecture rule + CI build failure |

---

## Communication Architecture

### Intra-Process Communication (v0.1)

```
Publisher Thread              Subscriber Thread
      │                             │  (explicitly created via ro_thread_create;
      │                             │   listed in app thread roster)
      │ 1. ro_publish(topic, msg)   │
      ▼                             │
┌──────────────┐                    │
│ Topic Lookup │                    │
│ (Hash Table) │                    │
└──────┬───────┘                    │
       │                            │
       │ 2. Enqueue to all subs     │
       ▼                            │
┌──────────────────────────┐        │
│  Subscriber Queue (Q1)   │────────►│ 3. Subscriber thread dequeues
│  (owned by sub thread)   │(mutex) │    → callback(msg)
└──────────────────────────┘        │
```

> **Thread rule:** Callback always runs on the subscriber's thread (explicitly created  
> & listed in the thread roster). There is no separate executor abstraction — the thread  
> IS the executor. "Executor dequeue" language is intentionally absent from this codebase.

#### Zero-Copy Optimization (Future v0.2)

```c
// Loaned buffer (zero-copy)
void* buf = ro_pub_loan(&pub, sizeof(msg));
memcpy(buf, &msg, sizeof(msg));
ro_pub_commit(&pub, buf);  // Transfers ownership to queue
```

### Inter-Process Communication (v0.2+)

**UART Transport (Planned)**

```
MCU 1 (Main Controller)          MCU 2 (Stepper Driver)
      │                                 │
      │ ro_publish("cmd/stepper", ...)  │
      ▼                                 │
┌──────────────┐                        │
│   UART TX    │──────────────────────► │
│   Serialize  │     (Wire Protocol)    │
└──────────────┘                        ▼
                                  ┌──────────────┐
                                  │   UART RX    │
                                  │ Deserialize  │
                                  └──────┬───────┘
                                         │
                                         │ ro_sub callback
                                         ▼
```

---

## Build System Architecture

### Zephyr Build System (West + CMake)

**RobotOS uses Zephyr's native build system:**

```
┌─────────────────────────────────────────────────────────┐
│                DEVELOPER WORKFLOW                        │
│                                                          │
│  west build -b stm32f429i_disc1 app/cnc_demo            │
│               │                                          │
│               ▼                                          │
│  ┌────────────────────────┐                             │
│  │   CMake Configuration  │                             │
│  │   - Kconfig options    │                             │
│  │   - Device tree        │                             │
│  │   - Board selection    │                             │
│  └────────┬───────────────┘                             │
│           │                                              │
│           ▼                                              │
│  ┌────────────────────────┐                             │
│  │   Build (Ninja/Make)   │                             │
│  │   - Compile sources    │                             │
│  │   - Link firmware      │                             │
│  └────────┬───────────────┘                             │
│           │                                              │
│           ▼                                              │
│  ┌────────────────────────┐                             │
│  │   Output: zephyr.elf   │                             │
│  │           zephyr.bin   │                             │
│  └────────────────────────┘                             │
└─────────────────────────────────────────────────────────┘
            │
            │ west flash
            ▼
       ┌──────────┐
       │  Target  │
       │  Board   │
       └──────────┘
```

**Key Build Commands:**
```bash
# Initialize Zephyr workspace
west init -m https://github.com/robotos/robotos-zephyr
west update

# Build for specific board
west build -b stm32f429i_disc1 app/cnc_demo

# Flash to board
west flash

# Serial monitor
west debug
```

**See [BUILD_SYSTEM.md](BUILD_SYSTEM.md) for detailed build architecture.**

### Repository Structure (Zephyr-based)

```
robotos/
├─ zephyr/                      # Zephyr RTOS (submodule)
│
├─ robotos/                     # RobotOS overlay
│  ├─ include/
│  │  └─ robotos/
│  │     ├─ ro_thread.h        # Adapter Layer APIs
│  │     ├─ ro_time.h
│  │     ├─ ro_queue.h
│  │     ├─ stepper.h          # Framework Components
│  │     ├─ servo.h
│  │     ├─ pid.h
│  │     ├─ encoder.h
│  │     └─ state_machine.h
│  │
│  ├─ src/
│  │  ├─ adapter/              # Robot Adapter Layer
│  │  │  ├─ ro_thread.c
│  │  │  ├─ ro_time.c
│  │  │  └─ ro_queue.c
│  │  │
│  │  └─ framework/            # Robot Framework
│  │     ├─ stepper.c
│  │     ├─ servo.c
│  │     ├─ pid.c
│  │     ├─ encoder.c
│  │     └─ state_machine.c
│  │
│  ├─ boards/                  # Board-specific configs
│  │  ├─ stm32f429i_disc1/
│  │  └─ esp32_devkitc/
│  │
│  ├─ dts/                     # Device Tree overlays
│  │  └─ bindings/
│  │
│  └─ CMakeLists.txt           # RobotOS library build
│
├─ app/                        # Application examples
│  ├─ cnc_demo/
│  │  ├─ src/
│  │  │  └─ main.c
│  │  ├─ prj.conf              # Kconfig settings
│  │  ├─ app.overlay           # Device tree overlay
│  │  └─ CMakeLists.txt
│  │
│  ├─ printer_demo/
│  ├─ arm3dof_demo/
│  └─ line_tracker_demo/
│
├─ tests/                      # Unit tests
│  ├─ adapter/
│  └─ framework/
│
├─ tools/
│  ├─ scripts/
│  └─ trace_viewer/
│
├─ docs/
│  ├─ ARCHITECTURE.md          # This file
│  ├─ BUILD_SYSTEM.md
│  └─ API_REFERENCE.md
│
├─ west.yml                    # West manifest
├─ CMakeLists.txt              # Root build
└─ Kconfig                     # Configuration options
```

---

## Platform Abstraction

### Zephyr Device Tree & Driver Model

**RobotOS leverages Zephyr's device tree for hardware abstraction.**

#### Device Tree Overlay Example (STM32F4)

```dts
// app/cnc_demo/app.overlay
// Define stepper motor pins

/ {
    stepper0: stepper0 {
        compatible = "robotos,stepper";
        step-gpios = <&gpioa 0 GPIO_ACTIVE_HIGH>;
        dir-gpios = <&gpioa 1 GPIO_ACTIVE_HIGH>;
        enable-gpios = <&gpioa 2 GPIO_ACTIVE_LOW>;
        steps-per-rev = <200>;
        max-speed = <2000>;
        acceleration = <1000>;
        status = "okay";
    };

    servo0: servo0 {
        compatible = "robotos,servo-pwm";
        pwms = <&pwm1 1 PWM_MSEC(20) PWM_POLARITY_NORMAL>;
        min-pulse-us = <1000>;
        max-pulse-us = <2000>;
        min-angle = <-90>;
        max-angle = <90>;
        status = "okay";
    };
};
```

#### Robot Framework Device Binding

> **Layer rule:** `DEVICE_DT_GET` and `device_is_ready` are used **only inside Robot Framework driver implementations**.
> Application code **never** touches Zephyr device handles directly — it only calls Robot Adapter API.

**Inside Robot Framework driver (internal — allowed to use Zephyr):**
```c
// robotos/src/framework/drivers/stepper.c  — NOT visible to applications
// Internal name: stepper_drv_bind() — distinct from public ro_stepper_get().
#include <zephyr/device.h>
#include <robotos/stepper.h>

static const struct device *stepper_dev = DEVICE_DT_GET(DT_NODELABEL(stepper0));

// Called once at boot by the framework to populate the stepper handle table.
static int stepper_drv_bind(ro_stepper_t *s)
{
    if (!device_is_ready(stepper_dev)) {
        return RO_ENOTREADY;   /* already-negative enum value; no unary minus needed */
    }
    s->dev = stepper_dev;
    return 0;
}
```

**Application code (only Robot Adapter API — no Zephyr headers):**
```c
// app/cnc_demo/src/main.c
#include <robotos/stepper.h>   // Framework header: "stepper.h", no ro_ prefix, no Zephyr

// stepper_get() uses the DT label string — matches the "stepper0" node in app.overlay.
// Called ONCE at init, handle held for the lifetime of the thread (not per-iteration).
stepper_t* stepper = stepper_get("stepper0");
if (stepper == NULL) {
    RO_LOG_ERROR("stepper0 not found or device not ready");
    return;
}
stepper_move_to(stepper, 1000);  // Move to position 1000
```

### Porting to New Hardware

**Steps to port RobotOS to a new board:**

1. **Check Zephyr board support:**
   - If board is already in Zephyr, just use it
   - If not, create board definition (see Zephyr docs)

2. **Create device tree overlay:**
   - Define robot-specific peripherals
   - Configure pins, PWM channels, etc.

3. **Configure prj.conf:**
   - Enable required Zephyr subsystems (GPIO, PWM, ADC, etc.)

4. **Build and flash:**
   ```bash
   west build -b <your_board> app/cnc_demo
   west flash
   ```

**Typical porting effort:** 1-2 days (if board is supported by Zephyr)

### Porting Checklist

| Component | Required | Optional | Effort |
|-----------|----------|----------|--------|
| Board DTS + overlay | ✅ | | 0.5 day |
| `prj.conf` Kconfig | ✅ | | 0.5 day |
| Zephyr board file (new HW only) | ✅ | | 2-4 days |
| UART / shell | ✅ | | 1 day |
| GPIO / PWM drivers | ✅ | | 1 day |
| Timer / counter | ✅ | | 1 day |
| SPI / I2C | | ✅ | 1-2 days each |
| Robot Framework drivers (stepper, servo…) | ✅ | | 1 day/device |
| **TOTAL (Zephyr-supported board)** | | | **~2 days** |
| **TOTAL (new board, no Zephyr support)** | | | **~1.5 weeks** |

---

## Observability & Tracing

### Trace Architecture (Zephyr Tracing)

**RobotOS uses Zephyr's built-in tracing infrastructure:**

```
Application/Framework             Trace Backend
      │                                │
      │ SYS_PORT_TRACING_FUNC()        │
      ▼                                │
┌──────────────┐                       │
│ Zephyr Trace │                       │
│   Hooks      │                       │
└──────┬───────┘                       │
       │                               │
       │ (if CONFIG_TRACING=y)         │
       ▼                               │
┌──────────────┐                       │
│  Trace CTF   │                       │
│  Backend     │                       │
└──────┬───────┘                       │
       │                               │
       │ Write to ring buffer          │
       ▼                               │
┌──────────────┐                       │
│ Trace Buffer │───────────────────────►│ Dump via shell
│ (Circular)   │    (on demand)        │ → CTF format
└──────────────┘                       │
```

**Configuration (prj.conf):**
```conf
CONFIG_TRACING=y
CONFIG_TRACING_CTF=y
CONFIG_TRACING_THREAD=y
CONFIG_TRACING_ISR=y
CONFIG_TRACING_SYSCALL=y
```

### Traced Events (v0.1)

| Event Type | Zephyr Hook | Use Case |
|------------|-------------|----------|
| **Thread Switch** | `sys_port_trace_thread_switched_in()` | Scheduler analysis |
| **Thread Create** | `sys_port_trace_thread_create()` | Thread lifecycle |
| **ISR Enter/Exit** | `sys_port_trace_isr_enter/exit()` | Interrupt profiling |
| **Syscall** | `sys_port_trace_syscall_enter/exit()` | API usage tracking |

**Custom RobotOS Events:**
```c
// Add custom trace points
SYS_PORT_TRACING_FUNC(stepper_move, const struct device *dev, int32_t steps);
SYS_PORT_TRACING_FUNC(pid_ctrl_update, pid_ctrl_t *pid, float error);
```

### Deadline Monitoring

> **Related:** `✅ Missed deadline detection` listed in [Architecture Philosophy](#architecture-philosophy).

#### Minimal API (`ro_deadline.h`)

```c
// ro_deadline.h
//
// Lightweight deadline window: caller marks begin/end of a real-time work unit.
// Under the hood: records entry timestamp; on end, compares elapsed vs budget.
// Works without a separate thread — inline cost is two ro_time_us() calls.

typedef struct {
    uint64_t    budget_us;     // allowed window (microseconds)
    const char* name;          // logged on violation; must be a string literal
    uint64_t    _start_us;     // private: set by ro_deadline_begin()
} ro_deadline_t;

// Mark start of real-time window.
static inline void ro_deadline_begin(ro_deadline_t* d) {
    d->_start_us = ro_time_us();
}

// Mark end of window. Returns true if deadline was met, false if missed.
// On miss: emits a trace event + increments a per-name counter (observable
// via ro_deadline_miss_count(name)).
bool ro_deadline_end(ro_deadline_t* d);
```

**Usage in a control thread:**
```c
#define CONTROL_DEADLINE_US  1000u   // 1 ms budget for 1 kHz loop

static ro_deadline_t ctrl_dl = {
    .budget_us = CONTROL_DEADLINE_US,
    .name      = "control_loop",
};

static void control_loop_thread(void* arg) {
    while (1) {
        ro_deadline_begin(&ctrl_dl);

        float out = pid_ctrl_update(pid, setpoint, measured);
        stepper_move(stepper, (int32_t)out);

        if (!ro_deadline_end(&ctrl_dl)) {
            // Missed deadline — trace event already emitted;
            // application may escalate to fault handler.
        }
        ro_thread_sleep_ms(1);
    }
}
```

**Trace integration:**  
On a missed deadline `ro_deadline_end()` emits:
```c
SYS_PORT_TRACING_FUNC(ro_deadline_miss, d->name, elapsed_us, d->budget_us);
```
The miss counter is exported via the shell: `ro deadline stats` prints a table of
`name | budget_us | misses | max_elapsed_us` for every registered deadline.

### Trace Viewing Workflow

**Using Zephyr Tracing:**
```bash
# 1. Enable tracing in prj.conf
CONFIG_TRACING=y
CONFIG_TRACING_CTF=y

# 2. Build and flash
west build -b stm32f429i_disc1
west flash

# 3. Connect to shell and dump trace
screen /dev/ttyUSB0 115200
uart:~$ trace dump

# 4. View with TraceCompass or babeltrace
babeltrace trace.ctf
```

**Chrome Trace JSON (Future):**
- Convert CTF to Chrome Trace format
- View in `chrome://tracing`

---

## Design Patterns & Conventions

### Coding Conventions

#### Naming

```c
// Types
typedef struct ro_node ro_node_t;
typedef enum ro_status ro_status_t;

// Functions
ro_status_t ro_node_create(const char* name);  // Verb_noun

// Constants
#define RO_MAX_TOPIC_LEN 64
#define RO_VERSION_MAJOR 0

// Private (static) functions
static void node_cleanup(ro_node_t* node);  // Internal use
```

#### Error Handling

```c
// Return-code pattern (no exceptions)
ro_status_t result = ro_publish(&pub, &msg, sizeof(msg));
if (result != RO_OK) {
    RO_LOG_ERROR("Publish failed: %d", result);
    return result;
}

// Assertion for impossible states
RO_ASSERT(node != NULL, "Node cannot be NULL");
```

#### Resource Lifecycle

```c
// RAII-like pattern (manual)
ro_node_t* node = ro_node_create("my_node");
if (node == NULL) {
    return RO_ENOMEM;
}

// ... use node ...

ro_node_destroy(node);  // Always cleanup
```

### Design Patterns

#### Factory Pattern (Node Creation)

```c
ro_node_t* ro_node_create(const char* name) {
    ro_node_t* node = ro_pool_alloc(&node_pool);
    if (node == NULL) return NULL;
    
    node->name = name;
    node->state = RO_NODE_IDLE;
    return node;
}
```

#### Observer Pattern (Pub/Sub)

```c
// Publishers notify all subscribers
ro_publish(&pub, &msg, sizeof(msg));
// → Iterates sub list, enqueues to each
```

#### Strategy Pattern (Transport)

```c
typedef struct {
    ro_status_t (*send)(const void* data, size_t len);
    ro_status_t (*recv)(void* data, size_t len);
} ro_transport_ops_t;

// Inproc implementation
static ro_transport_ops_t inproc_ops = {
    .send = inproc_send,
    .recv = inproc_recv
};
```

---

## Security Considerations

### v0.1 Scope (Minimal)

**In Scope:**
- ✅ Buffer overflow protection (bounds checking)
- ✅ Stack canaries (if supported by toolchain)
- ✅ Null pointer checks in public APIs

**Out of Scope (v0.2+):**
- ❌ Cryptography (no TLS/encryption)
- ❌ Authentication (no user accounts)
- ❌ Secure boot (future feature)

### Defensive Programming

```c
// Always validate inputs
ro_status_t ro_publish(ro_pub_t* pub, const void* msg, size_t len) {
    if (pub == NULL || msg == NULL) {
        return RO_EINVAL;
    }
    if (len > pub->max_msg_size) {
        return RO_EINVAL;  // Prevent overflow
    }
    // ... proceed ...
}
```

---

## Performance Characteristics

### Latency Budgets (Target)

| Operation | Target Latency | Measured (STM32F4 @ 168MHz) |
|-----------|----------------|------------------------------|
| `stepper_move()` | < 5 μs | TBD |
| `pid_ctrl_update()` | < 10 μs | TBD |
| Thread switch overhead (Zephyr) | < 3 μs | TBD |
| ISR latency | < 1 μs | TBD |
| Trace event write | < 2 μs | TBD |

### Memory Footprint (Estimate)

| Component | Flash | RAM |
|-----------|-------|-----|
| Zephyr kernel (minimal) | ~40 KB | ~10 KB |
| Robot Adapter Layer | ~5 KB | ~2 KB |
| Robot Framework (Stepper, Servo, PID) | ~15 KB | ~4 KB |
| Example app (CNC demo) | ~10 KB | ~8 KB (stacks + buffers) |
| **TOTAL (Minimal)** | **~70 KB** | **~24 KB** |

**Target Boards:**
- ✅ STM32F4 (512KB Flash, 128KB RAM) - Plenty of headroom
- ✅ ESP32 (4MB Flash, 520KB RAM) - Ideal
- ⚠️ STM32F103C8 (64KB Flash, 20KB RAM) - Tight, requires tuning
- ✅ nRF52840 (1MB Flash, 256KB RAM) - Excellent

---

## Future Architecture Evolution

### Roadmap

#### v0.1 (Current - Bootstrap)
- 🔄 Robot Adapter Layer (Thread, Time, Queue) — API designed, implementation in progress
- 🔄 Robot Framework components (Stepper, Servo, PID, Encoder) — API designed, implementation in progress
- 🔄 Robot State Machine (IDLE/HOMING/RUN/FAULT) — designed, implementation in progress
- 📋 CNC Demo application
- 🔄 Zephyr build system integration — architecture defined, setup in progress
- 📋 Device tree bindings for robot components

#### v0.2 (Expansion)
- 📋 **CNC Demo** — 1 demo app hoàn chỉnh (focus: Stepper + Endstop + GCode stub)
- 📋 DCmotorPWM component
- 📋 Endstop & Sensors components
- 📋 Filters (low-pass, moving average)
- 📋 Limiter component
- 📋 Benchmark suite (jitter, IPC latency, mem)

> **Quyết định (2026-02-27):** v0.2 chỉ làm 1 demo app — **CNC Demo** (CNC và 3D Printer overlap lớn về Stepper/motion, làm CNC trước để validate framework).

#### v0.3 (Multi-App & Advanced Features)
- 📋 3D Printer Demo (spin-off từ CNC Demo)
- 📋 Arm 3DOF Demo (Servo + PID + Kinematics)
- 📋 Line Tracker Demo (DCmotor + Encoder + Sensor)
- 📋 Network communication (UART inter-MCU)
- 📋 Multi-core support (ESP32 dual-core)
- 📋 Advanced motion planning
- 📋 Trajectory generation
- 📋 Kinematics library (FK/IK for arms)
- 📋 Multi-board support (ESP32, nRF52)

#### v1.0 (Production Ready)
- 📋 API stability guarantee
- 📋 Full test coverage (>80%)
- 📋 Production-ready examples
- 📋 Security hardening
- 📋 Complete documentation

### Open Questions

| Question | Status | Decision Deadline |
|----------|--------|-------------------|
| Device Tree vs. Runtime configuration? | ✅ Decided: Device Tree | 2026-02-22 |
| Message passing for inter-component comm? | 📋 Under discussion | v0.2 |
| Multi-language bindings (Python/Rust)? | 📋 Research phase | v0.3+ |
| Zephyr upstr
---

## Alpha MVP Demo Model

### Goal

Deliver a **minimal, end-to-end demo** that proves RobotOS' core architectural promises to the council:

- **Strict layering works in practice:** App → Framework → Adapter → Zephyr (no Zephyr leakage in app/framework public headers)
- **No Magic:** every runtime thread is explicitly created and listed in a **Thread Roster**
- **Determinism + Observability:** deadline monitoring + trace export show predictable behavior and measurable timing

This Alpha MVP is intentionally **small**: it is the smallest slice that touches all layers and produces evidence.

### Chosen Easiest Application: CNC Single-Axis Demo (1 Stepper + Endstop)

Rationale:
- CNC and 3D printer share the same core primitives (stepper + homing + command queue), but CNC is simpler.
- Demonstrates real-time control + state machine without requiring complex kinematics or networking.

### Minimal Scope (What the Team Must Implement)

#### 1) Robot Adapter (must be real, not stubbed)
- `ro_thread` (create/destroy/sleep/yield, `ro_priority_t` presets)
- `ro_time` (us/ms + delay)
- `ro_queue` (send/recv + ISR variants)
- `ro_pool` (fixed-block pool, ISR-safe modes)
- `ro_log` (ring-buffer + background flush thread)
- `ro_trace` (Zephyr tracing enable + dump command or UART dump)
- `ro_deadline` (begin/end + miss counter + trace event)

> Alpha constraint: **No heap after init**, only pools + static buffers.

#### 2) Robot Framework (minimal robotics semantics)
- **Stepper (v0.1 subset)**
  - DT binding at `POST_KERNEL`
  - `ro_stepper_get/put`, `move_to`, `stop`, `get_position`
  - Implementation may be:
    - **HW-timer step output (preferred on STM32F4)**, or
    - **SW pulse thread only if `CONFIG_RO_ALLOW_SW_PULSE_THREAD=y`** (guard already defined)
- **Endstop (minimal)**
  - DT binding for a single endstop GPIO
  - `ro_endstop_is_triggered()` (polled) or IRQ → queue event
- **State Machine**
  - `IDLE → HOMING → IDLE → RUN → IDLE`
  - `ANY → FAULT` on fatal status

> Alpha scope explicitly excludes: multi-axis planning, acceleration profiles beyond a simple trapezoid stub, filters/limiter, networking, multi-MCU transport.

#### 3) Application (demo logic only)
- **CNC app** with **one axis**:
  - `home` command: move negative until endstop triggers → set position 0
  - `move_to <pos>` command: move to target position
  - A very small command interface:
    - Option A (fastest): hardcoded command script sequence
    - Option B: UART shell commands (recommended for council demo)

### Thread Roster (Alpha Demo)

Minimum threads for the demo:

- `state_machine` — `RO_PRIO_FRAMEWORK`
- `control_loop` — `RO_PRIO_RT_CONTROL` (1 kHz loop; runs deadline window)
- `log_flush` — `RO_PRIO_BACKGROUND`
- Optional (only if SW pulse enabled): `step_pulse` — `RO_PRIO_RT_PULSE`

The CNC app must include a **Thread Roster block** matching these threads.

### Demo Evidence (What We Show to the Council)

#### A) Architecture proof (static)
- CI output showing:
  - **0 Zephyr includes** in `include/robotos/*.h`
  - Zephyr includes only in:
    - `src/adapter/zephyr/*.c`
    - `src/framework/drivers/*.c` (DT + device only)
- A short slide/screenshot of the **layer dependency diagram** and the **Thread Roster**.

#### B) Runtime proof (dynamic)
1. **Serial log** of:
   - boot status table (`ro_get_boot_status()`)
   - state transitions (`IDLE → HOMING → IDLE → RUN`)
2. **Deadline monitoring stats**
   - `control_loop` budget (e.g. 1000 µs) + miss count (should be 0 in nominal)
3. **Trace dump**
   - Thread switch + ISR enter/exit
   - Custom events: `stepper_move`, `ro_deadline_miss` (should be absent/0)

### Acceptance Criteria (Alpha Gate)

The MVP is accepted when all are true:

1. **Build & run** on at least one target:
   - **Preferred:** STM32F4 board listed in this doc
   - **Backup:** QEMU with a “virtual stepper” driver that logs pulses (still respects layering)
2. **No hidden threads:** runtime threads exactly match Thread Roster
3. **No heap in realtime:** no malloc/free after `robotos_init()`
4. **Homing works:** endstop-based homing sets position to 0 reliably
5. **Move command works:** `move_to` reaches target within tolerance
6. **Observability works:** deadline stats and trace dump are produced

### Alpha Deliverables (What the Team Ships)

- `app/cnc_demo/` runnable demo
- Minimal Adapter implementation + unit tests (where feasible)
- Minimal Framework (stepper + endstop + state machine) + basic tests
- A “Demo Script” document:
  - commands to build/flash
  - commands to run the homing + move sequence
  - how to export trace and show deadline stats

### Task Breakdown (Minimal Planning Buckets)

- **A1 — Adapter Primitives (core)**
  - ro_thread, ro_time, ro_queue, ro_pool
- **A2 — Observability**
  - ro_log + log_flush thread
  - ro_trace + trace dump
  - ro_deadline + stats
- **F1 — Framework Drivers**
  - DT bindings for stepper + endstop
- **F2 — Framework Semantics**
  - stepper minimal API + stop + position
  - state machine minimal transitions
- **APP — CNC Demo**
  - command interface (UART shell recommended)
  - thread roster + init + demo sequence
- **CI — Discipline**
  - Zephyr include whitelist checks
  - profile checks (`RO_PROFILE_STD` default)


eam contributions strategy? | 📋 Under discussion | v0.2 |

---

## Appendix

### Glossary

| Term | Definition |
|------|------------|
| **Robot Adapter Layer** | Abstraction layer hiding Zephyr implementation details |
| **Robot Framework** | Collection of device components (Stepper, Servo, PID, etc.) |
| **Device Tree** | Hardware description format used by Zephyr |
| **Kconfig** | Configuration system for Zephyr kernel options |
| **West** | Zephyr's meta-tool for building and flashing |
| **State Machine** | Robot lifecycle manager (IDLE→HOMING→RUN→FAULT) |
| **Prj.conf** | Project-specific Kconfig settings |

### References

- [Zephyr Project Documentation](https://docs.zephyrproject.org/)
- [Zephyr Device Tree Guide](https://docs.zephyrproject.org/latest/build/dts/index.html)
- [RobotOS Bootstrap Spec](../ROBOTOS_BOOTSTRAP.md)
- [Build System Documentation](BUILD_SYSTEM.md)
- [Kernel Layer Design](KERNEL_LAYER.md)
- [Adapter Layer Design](ADAPTER_LAYER.md)
- [Framework Layer Design](FRAMEWORK_LAYER.md)
- [Chrome Trace Format](https://docs.google.com/document/d/1CvAClvFfyA5R-PhYUmn5OOQtYMH4h6I0nSsKchNAySU/)

---

**Document Status:** 🚧 Living Document — Updated as architecture evolves

**Maintainer:** RobotOS Core Team  
**Last Review:** 2026-03-03
