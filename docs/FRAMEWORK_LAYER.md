# RobotOS Inspire — Robot Framework Layer Design

> **Version:** v0.1-alpha  
> **Last Updated:** 2026-03-03  
> **Status:** 🚧 Under Active Development  
> **Layer:** Robot Framework (Robotics domain semantics — sits above Adapter)

---

## 📋 Table of Contents

1. [Layer Role & Boundary](#layer-role--boundary)
2. [Design Philosophy](#design-philosophy)
3. [Specification Scope Rules](#specification-scope-rules)
4. [Lifecycle Policy](#lifecycle-policy)
5. [Header Naming Convention](#header-naming-convention)
6. [Stepper Motor API (`stepper`)](#stepper-motor-api-stepper)
7. [Servo (PWM) API (`servo`)](#servo-pwm-api-servo)
8. [DC Motor (PWM) API (`dcmotor`)](#dc-motor-pwm-api-dcmotor)
9. [Encoder API (`encoder`)](#encoder-api-encoder)
10. [Endstop API (`endstop`)](#endstop-api-endstop)
11. [Sensor API (`sensor`)](#sensor-api-sensor)
12. [PID Controller API (`pid`)](#pid-controller-api-pid)
13. [Filter API (`filter`)](#filter-api-filter)
14. [Limiter API (`limiter`)](#limiter-api-limiter)
15. [Robot State Machine API (`robot_sm`)](#robot-state-machine-api-robot_sm)
16. [Callback and Execution Context Rules](#callback-and-execution-context-rules)
17. [DT-Based Device Binding](#dt-based-device-binding)
18. [Layer Boundary Rules](#layer-boundary-rules)
19. [Error Model](#error-model)

---

## Layer Role & Boundary

### Position in the Stack

```
Application Layer         — GCode parser, trajectory planner, demo programs
      ↓
┌──────────────────────────────────────────────────────────────────────────┐
│  ROBOT FRAMEWORK LAYER  (this document)                                 │
│                                                                          │
│  Role: Robotics domain semantics.                                        │
│  Answers: "What is a stepper? A PID? A state machine?                   │
│            How do I move a motor? Apply a control loop?                  │
│            Manage robot states (IDLE / HOMING / RUN / FAULT)?"          │
│                                                                          │
│  Portability: Depends only on Robot Adapter Layer.                       │
│  Never includes <zephyr/*> in public headers.                            │
│  Uses only ro_* APIs for threading, timing, IPC, GPIO, PWM.             │
└──────────────────────────────────────────────────────────────────────────┘
      ↓
Robot Adapter Layer       — ro_thread, ro_queue, ro_pool, ro_gpio, ro_pwm, ...
      ↓
Zephyr Kernel Layer       — Adapter implementations consume Zephyr
      ↓
Hardware Layer            — STM32F4, ESP32, nRF52, QEMU
```

### What the Framework Layer Does

| Capability | Module | Public Header |
|---|---|---|
| Stepper motor management | `stepper.c` + `stepper_drv.c` | `stepper.h` |
| Servo (PWM-based) control | `servo.c` | `servo.h` |
| DC motor via PWM | `dcmotor.c` | `dcmotor.h` |
| Quadrature encoder reading | `encoder.c` | `encoder.h` |
| Limit switch / endstop | `endstop.c` | `endstop.h` |
| Generic sensor interface | `sensor.c` | `sensor.h` |
| PID control algorithm | `pid.c` | `pid.h` |
| Digital filters | `filter.c` | `filter.h` |
| Output limiters | `limiter.c` | `limiter.h` |
| Robot state machine | `robot_sm.c` | `robot_sm.h` |

### What the Framework Layer Does NOT Do

- ❌ No RTOS concepts leaked to callers (`k_thread`, `k_msgq`, etc.)
- ❌ No raw I2C/SPI transactions in public headers (use Adapter or a device driver)
- ❌ No heap allocation — all objects come from `ro_pool_t` configured by the caller
- ❌ Does not create or own threads by default — the application creates every thread
  using `ro_thread_create`. The `sm_runloop_start()` helper is an **opt-in**
  convenience; if not used, the app provides its own event-loop thread.

---

## Design Philosophy

### Core Tenets

```
✅ Domain-semantic APIs — "move 200 steps" not "toggle GPIO 200 times"
✅ Policy-driven — Framework enforces acceleration profiles, safety limits, state contracts
✅ Uses ONLY Adapter APIs for RTOS operations (ro_thread, ro_queue, ro_gpio, etc.)
✅ DT-based device binding at POST_KERNEL boot phase
✅ No <zephyr/*> in public headers — swap Adapter backend, Framework stays unchanged
✅ Alloc functions return object-or-NULL; operations return ro_status_t
```

### Typical Data Flow

```
Application Layer (GCode parser / trajectory planner)
  │
  │  sm_dispatch(sm, CMD_START)   ← via Glue app_sm_cb_t callback
  ▼
Robot Framework — State Machine thread  (RO_PRIO_FRAMEWORK)
  │
  │  stepper_move(x_axis, target_steps)       ← Framework API, no ro_ prefix
  ▼
Robot Framework — stepper.c
  │  └ builds trapezoidal motion profile (accel/cruise/decel ramp)
  │  └ publishes step events via ro_queue_send_isr() ← Adapter API
  ▼
Robot Adapter Layer — ro_queue.c / ro_timer.c
  │  └ wraps Zephyr k_msgq / k_timer
  │  └ fires ISR at each step interval
  ▼
ISR context (step timer, encoder edge, endstop)
  │  ro_gpio_set(step_pin, 1) → ro_gpio_set(step_pin, 0)   ← Adapter API
  ▼
Hardware Layer — stepper driver IC (A4988 / TMC2209, etc.)

╔════════════════════════════════════════════════════════╗
║  Layer             What it owns                          ║
╠════════════════════════════════════════════════════════╣
║  Application       Mission logic, GCode, user commands    ║
║  Framework         Domain semantics (stepper, pid, sm)    ║
║  Adapter           RTOS/HW abstraction (ro_thread, etc.)  ║
║  Kernel (Zephyr)   Scheduling, IRQ, DT binding            ║
║  Hardware          GPIO, PWM, I2C, SPI, timers            ║
╚════════════════════════════════════════════════════════╝
```

---

## Specification Scope Rules

### Spec Bug vs Implementation Choice

This specification distinguishes two categories of constraints:

**Spec Bugs** = contract violations that MUST return an error code.
- Example: `stepper_put(st)` when `st` is not in IDLE/FAULT state is a **contract violation**.
- Framework MUST NOT silently accept violations; Framework returns `RO_EBUSY` to signal the violation.
- App is responsible for code that respects preconditions, but Framework helps by reporting violations.

**Implementation Choices** = behaviors that are flexible or deferred.
- Example: Whether encoder_get_velocity uses exponential moving average or rolling average.
- Specification may suggest a canonical pattern but allow implementer flexibility.
- Documentation marks these as "Recommended" or presents alternatives.

### Canonical vs Bring-Up Patterns

**Canonical patterns** = production-ready, used in all deployments.
- Interrupt-driven event handling (ISR callbacks + semaphores)
- Deadline-monitored control loops

**Bring-up patterns** = temporary, used only for initial debug/bringup, then removed.
- Polling loops (`while (!condition) { ro_thread_sleep_ms(1); }`)
- Busy-wait in homing sequences

Bring-up patterns appear in examples and documentation for quick onboarding; they are NOT intended for production code. Clear markers (e.g., "BRINGUP ONLY", "CANONICAL") indicate which is recommended.

---

## Lifecycle Policy

### Pattern 1: Pool-Based (DT-bound hardware components)

**Components:** stepper, servo, dcmotor, encoder, endstop, sensor

**Ownership:** Framework owns the static pool; Adapter/DT binds hardware to pool entries at boot.

**Justification:** Devices are hardware-fixed and discovered via Device Tree at `POST_KERNEL` SYS_INIT. Pool size is a compile-time `CONFIG_*` value.

**Lifecycle:**
```
POST_KERNEL boot:  stepper_drv_bind_all()  ← populates static pool from DT nodes
Application:       stepper_get("dt_label")  ← acquire from pool (or NULL if exhausted)
                   [use stepper: stepper_move(), stepper_stop(), etc.]
                   stepper_put(st)          ← MUST be IDLE or FAULT; returns RO_OK or RO_EBUSY
```

**Preconditions:**
- `stepper_put()` PRECONDITION: stepper MUST be in IDLE or FAULT state.
  - Returns `RO_OK` on success.
  - Returns `RO_EBUSY` if stepper is ACCELERATING, RUNNING, or DECELERATING (motion in progress).
  - Same for `servo_put()`, `encoder_put()`, `endstop_put()`, `sensor_put()`.
- `get()` returns `NULL` if pool is exhausted or DT label not found.

**Contract:**
- Pool is static; no runtime device injection.
- Max concurrent pool users = `CONFIG_RO_STEPPER_POOL_SIZE`, etc.
- Caller MUST NOT use the pointer after `put()` (use-after-free error).

### Pattern 2: Alloc-Based (configuration/algorithm objects)

**Components:** pid_ctrl_t, sm_t, ema_filter_t, moving_avg_t, notch_filter_t

**Ownership:** Caller owns the object lifetime; Framework provides creation/destruction.

**Justification:** No DT binding; created on-demand by application; app knows when to create/destroy. No fixed pool size limit conceptually, though internal pool is bounded by CONFIG_*.

**Lifecycle:**
```
Application:       cfg = { .kp = 5.0, ... };
                   pid = pid_create(&cfg)  ← alloc object (or NULL if pool exhausted)
                   [use pid: pid_update(...)]
                   pid_destroy(pid)        ← return to pool
```

**Preconditions:**
- `pid_create()` returns `NULL` if object pool exhausted.
- `pid_destroy()` requires that no other thread is currently calling `pid_update()` on the same object.
  - Caller MUST synchronize (e.g., ensure destroy happens after all uses complete).

**Contract:**
- Alloc functions return pointer-or-NULL (no error code).
- Destroy functions typically return void (caller responsible for synchronization).
- Caller MUST NOT use pointer after destroy (use-after-free error).

### Choosing Pattern When Extending Framework

When adding a new Framework component:

- Use **pool-based** if:
  - Component is bound to hardware via Device Tree.
  - Discovery/binding happens at `POST_KERNEL` SYS_INIT.
  - Use count is bounded and known at compile time.
  - Release preconditions are important (e.g., must be idle).

- Use **alloc-based** if:
  - Component is pure algorithm/control (no DT binding).
  - Configuration is application-chosen (not hardware-fixed).
  - Creation/destruction timing is dynamic.
  - Pre/post conditions involve synchronization (not state constraints).

---

## Header Naming Convention

Framework public headers use **domain-semantic names** — no `ro_` prefix.

> **Rationale:** `ro_pid.h` implies PID is an RTOS concept (like `ro_mutex.h`).  
> PID is robotics domain; its header is `pid.h`.

| Header | Layer | Naming | Rationale |
|--------|-------|--------|-----------|
| `stepper.h` | Framework | No prefix | Domain: stepper motor; types: `stepper_t`, `stepper_config_t` |
| `servo.h` | Framework | No prefix | Domain: servo motor |
| `pid.h` | Framework | No prefix | Domain: PID controller; types: `pid_ctrl_t`, `pid_ctrl_config_t` |
| `robot_sm.h` | Framework | No prefix | Domain: robot state machine |
| `encoder.h` | Framework | No prefix | Domain: encoder |
| `endstop.h` | Framework | No prefix | Domain: limit switch |
| `ro_thread.h` | Adapter | `ro_` prefix | RTOS: thread primitive |
| `ro_queue.h` | Adapter | `ro_` prefix | RTOS: message queue |

> **Rule:** No Framework type or function uses the `ro_` prefix. That includes
> `typedef`s inside Framework headers. `ro_pid_t` / `ro_pid_create` are
> **wrong**; `pid_ctrl_t` / `pid_create` are correct.

---

## Stepper Motor API (`stepper`)

### State Diagram

```
                    stepper_get("dt_label")
                          │
                          ▼
                      [ IDLE ] ◄────────────────────────────────────────┐
                          │                                             │
        stepper_move() ───┤                        stepper_stop() ─────┤
                          ▼                                             │
                  [ ACCELERATING ]                                      │
                          │                                             │
         ramp done ───────┤                                             │
                          ▼                                             │
                     [ RUNNING ]                                        │
                          │                                             │
        target near ──────┤                                             │
                          ▼                                             │
                  [ DECELERATING ]                                      │
                          │                                             │
         at target ───────┴─────────────────────────────────────────────┘

    From any state:
        fault condition → [ FAULT ]
        stepper_put(st) → state asserted to be IDLE
```

### API

```c
// stepper.h

typedef struct stepper stepper_t;

// ── Lifecycle ────────────────────────────────────────────────────────────────

// Acquire a stepper instance from the DT-bound pool.
// Returns NULL if the pool is exhausted or the DT label is invalid.
stepper_t*  stepper_get(const char* dt_label);

// Release the stepper instance back to the pool.
// PRECONDITION: stepper must be in IDLE or FAULT state (motion must not be in progress).
// Returns RO_OK on success.
// Returns RO_EBUSY if stepper is ACCELERATING, RUNNING, or DECELERATING.
ro_status_t stepper_put(stepper_t* st);

// ── Motion control ───────────────────────────────────────────────────────────

// Move `steps` microsteps (positive = forward, negative = backward).
// Internally applies trapezoidal acceleration and deceleration profile.
// Returns RO_EBUSY if the stepper is not in IDLE state.
ro_status_t stepper_move(stepper_t* st, int32_t steps);

// Move to an absolute position (microsteps from home).
ro_status_t stepper_move_to(stepper_t* st, int32_t position);

// Immediate stop (deceleration ramp applied; does NOT fault).
ro_status_t stepper_stop(stepper_t* st);

// Emergency stop — zero deceleration. Transitions to FAULT.
void        stepper_emergency_stop(stepper_t* st);

// ── State & feedback ─────────────────────────────────────────────────────────

typedef enum {
    STEPPER_STATE_IDLE        = 0,
    STEPPER_STATE_ACCELERATING,
    STEPPER_STATE_RUNNING,
    STEPPER_STATE_DECELERATING,
    STEPPER_STATE_FAULT,
} stepper_state_t;

stepper_state_t stepper_get_state(const stepper_t* st);
int32_t         stepper_get_position(const stepper_t* st);

// Block the calling thread until stepper returns to IDLE or FAULT.
// Returns RO_OK (idle reached) or RO_ETIMEDOUT.
//
// Sync contract:
//   MUST NOT spin (busy loop). Uses Adapter semaphore (ro_sem_t) with ISR wake.
//   Implementation posts semaphore from step-complete ISR (deterministic < 10 µs).
//   ISR callback MUST only call _isr Adapter variants (ro_sem_give_isr).
//
// Multi-waiter semantics (IMPORTANT):
//   The underlying Adapter semaphore wakes only ONE waiter per give().
//   If multiple threads call stepper_wait_idle() on the same stepper instance:
//     - Only ONE will unblock when motion completes
//     - Others will continue waiting until the next give (stalls indefinitely)
//   RULE: Do NOT call stepper_wait_idle() from multiple threads on the same
//         stepper instance. Each stepper is designed for single-threaded use.
//         If multi-reader access is needed, use a separate queue or event
//         broadcasting mechanism at the application layer.
ro_status_t     stepper_wait_idle(stepper_t* st, uint32_t timeout_ms);

// ── Configuration ─────────────────────────────────────────────────────────────

typedef struct {
    uint32_t max_speed_steps_s;    // Maximum speed (steps/second)
    uint32_t accel_steps_s2;       // Acceleration (steps/second²)
    uint32_t decel_steps_s2;       // Deceleration (steps/second²)
    uint16_t microsteps;           // 1, 2, 4, 8, 16, 32
    bool     reverse_direction;    // Flip direction polarity
} stepper_config_t;

ro_status_t stepper_configure(stepper_t* st, const stepper_config_t* cfg);
```

### Usage Example

```c
#include <robotos/stepper.h>
#include <robotos/ro_thread.h>

static uint8_t motion_stack[2048];

static void motion_thread(void* arg) {
    stepper_t* x_axis = stepper_get("stepper_x");
    RO_ASSERT(x_axis != NULL, "x_axis stepper not found in DT");

    stepper_config_t cfg = {
        .max_speed_steps_s = 2000,
        .accel_steps_s2    = 500,
        .decel_steps_s2    = 500,
        .microsteps        = 16,
        .reverse_direction = false,
    };
    stepper_configure(x_axis, &cfg);

    while (1) {
        stepper_move(x_axis, 400);          // 400 steps forward
        stepper_wait_idle(x_axis, 5000);    // wait up to 5 s
        stepper_move(x_axis, -400);         // 400 steps back
        stepper_wait_idle(x_axis, 5000);
    }
}
```

---

## Servo (PWM) API (`servo`)

```c
// servo.h

typedef struct servo servo_t;

// Acquire servo instance bound to DT PWM node.
// Returns NULL if dt_label not found or pool exhausted.
servo_t*    servo_get(const char* dt_label);

// Release the servo instance back to the pool.
// Returns RO_OK on success.
ro_status_t servo_put(servo_t* servo);

// Set angle: 0.0 = min (1 ms pulse), 1.0 = max (2 ms pulse)
// Returns RO_EINVAL if angle outside [0.0, 1.0]
ro_status_t servo_set_angle(servo_t* servo, float angle);

// Set pulse directly (microseconds)
ro_status_t servo_set_pulse_us(servo_t* servo, uint32_t pulse_us);

// Disable output (PWM output held low)
ro_status_t servo_disable(servo_t* servo);

typedef struct {
    uint32_t min_pulse_us;   // Default 1000 µs
    uint32_t max_pulse_us;   // Default 2000 µs
    uint32_t period_us;      // Default 20000 µs (50 Hz)
} servo_config_t;

ro_status_t servo_configure(servo_t* servo, const servo_config_t* cfg);
```

---

## DC Motor (PWM) API (`dcmotor`)

```c
// dcmotor.h

typedef struct dcmotor dcmotor_t;

dcmotor_t*  dcmotor_get(const char* dt_label);

// Release the DC motor instance back to the pool.
// Returns RO_OK on success, RO_EBUSY if motor is not stopped (speed != 0).
ro_status_t dcmotor_put(dcmotor_t* motor);

// Set speed: -1.0 = full reverse, 0.0 = stop, +1.0 = full forward
// Returns RO_EINVAL if speed outside [-1.0, 1.0]
ro_status_t dcmotor_set_speed(dcmotor_t* motor, float speed);

ro_status_t dcmotor_stop(dcmotor_t* motor);

typedef struct {
    uint32_t pwm_period_ns;
    bool     invert_dir_pin;
} dcmotor_config_t;

ro_status_t dcmotor_configure(dcmotor_t* motor, const dcmotor_config_t* cfg);
```

---

## Encoder API (`encoder`)

```c
// encoder.h

typedef struct encoder encoder_t;

encoder_t*  encoder_get(const char* dt_label);

// Release the encoder instance back to the pool.
// Returns RO_OK on success.
ro_status_t encoder_put(encoder_t* enc);

// Returns accumulated tick count (signed, wraps at int32_t limits).
// Thread-safe: atomic read from ISR-updated counter.
int32_t     encoder_get_count(const encoder_t* enc);

// Reset counter to 0.
void        encoder_reset(encoder_t* enc);

// Returns ticks per revolution (configured from DT).
uint32_t    encoder_get_ticks_per_rev(const encoder_t* enc);

// Compute velocity in ticks/second over a sliding observation window.
// 
// Contract (normative):
//   Returns estimated velocity based on tick count accumulated within
//   `window_ms` milliseconds. The returned value represents tick delta ÷ elapsed time.
//   Invalid input (window_ms == 0) returns unspecified value; caller MUST ensure
//   window_ms > 0 (typically 1–100 ms for 1–100 kHz control loops).
//
// Implementation choice: internal smoothing algorithm (exponential moving average,
//   rolling buffer average, Kalman filter, etc.) is not prescribed by contract,
//   provided the result remains a reasonable tick_delta / elapsed_time estimate.
//
// Thread-safety: reads (not updates) tick counter; ISR-safe atomic read.
//   Applications may call from any thread; ISR-safe under typical architectures.
//   Multiple concurrent readers return consistent state snapshot from their call time.
float       encoder_get_velocity(const encoder_t* enc, uint32_t window_ms);
```

---

## Endstop API (`endstop`)

```c
// endstop.h

typedef struct endstop endstop_t;

endstop_t*  endstop_get(const char* dt_label);

// Release the endstop instance back to the pool.
// Returns RO_OK on success.
ro_status_t endstop_put(endstop_t* es);

// Returns current logical level (accounting for active_high config).
bool        endstop_is_triggered(const endstop_t* es);

// Register interrupt callback (fired from ISR context).
// IMPORTANT: callback runs in ISR — MUST use _isr variants of Adapter APIs.
typedef void (*endstop_trigger_cb_t)(endstop_t* es, void* user);
ro_status_t endstop_set_callback(endstop_t* es, endstop_trigger_cb_t cb, void* user);
ro_status_t endstop_clear_callback(endstop_t* es);
```

---

## Sensor API (`sensor`)

```c
// sensor.h

typedef struct sensor sensor_t;

typedef enum {
    SENSOR_TYPE_TEMPERATURE = 0,
    SENSOR_TYPE_HUMIDITY,
    SENSOR_TYPE_PRESSURE,
    SENSOR_TYPE_DISTANCE,
    SENSOR_TYPE_CURRENT,
    SENSOR_TYPE_VOLTAGE,
} sensor_type_t;

sensor_t*   sensor_get(const char* dt_label, sensor_type_t type);

// Release the sensor instance back to the pool.
// Returns RO_OK on success.
ro_status_t sensor_put(sensor_t* s);

// Read latest sample (float, SI units: °C, %, Pa, m, A, V)
ro_status_t sensor_read(sensor_t* s, float* value_out);

// Trigger an on-demand measurement (if device supports it).
//
// Contract (normative):
//   `sensor_trigger()` initiates a conversion (if supported) but DOES NOT block
//   waiting for completion. It is an asynchronous request initiation.
//   
// Returns:
//   RO_OK          — trigger initiated; conversion in progress or queued
//   RO_ENOTSUP     — device does not support on-demand triggering (passive sensor)
//   RO_ETIMEDOUT   — (rare) previous conversion still in progress; call failed
//   RO_EFAIL       — hardware error during trigger request
//
// Expected flow:
//   sensor_trigger(s);           ← request conversion
//   [wait for conversion time]   ← application-managed delay (not blocking)
//   sensor_read(s, &value);      ← read completed result
//
// Implementation note: Many sensors (I2C, SPI) perform conversions continuously
//   in the background; trigger() may be a no-op (RO_OK returned) or enforce
//   a conversion initiation depending on device capabilities. Consult device
//   binding documentation for behavior specific to each sensor.
ro_status_t sensor_trigger(sensor_t* s);
```

---

## PID Controller API (`pid`)

### Data Structures

```c
// pid.h

typedef struct pid_ctrl pid_ctrl_t;
// No `ro_` prefix: this is a pure-robotics library type, not an RTOS concept.
// `pid_ctrl_t` is distinct from POSIX `pid_t` (process ID) and avoids
// any name collision on Zephyr MCU targets where CONFIG_POSIX_API=n.

typedef struct {
    float kp;              // Proportional gain
    float ki;              // Integral gain
    float kd;              // Derivative gain
    float output_min;      // Output clamp minimum
    float output_max;      // Output clamp maximum
    float integral_min;    // Anti-windup clamp minimum
    float integral_max;    // Anti-windup clamp maximum
    float sample_time_s;   // Control loop sample period (seconds)
} pid_ctrl_config_t;
```

### API

```c
// Lifecycle
pid_ctrl_t* pid_create(const pid_ctrl_config_t* cfg);
void        pid_destroy(pid_ctrl_t* pid);

// Compute PID output.
//   setpoint: desired value
//   measured: actual value from sensor/encoder
//   dt_s: elapsed time since last call (seconds); use cfg.sample_time_s for fixed rate
// Returns: output in [output_min, output_max]
float       pid_update(pid_ctrl_t* pid, float setpoint, float measured, float dt_s);

// Reset integral accumulator and derivative filter.
void        pid_reset(pid_ctrl_t* pid);

// Runtime gain tuning (thread-safe; takes effect on next pid_update call)
ro_status_t pid_set_gains(pid_ctrl_t* pid, float kp, float ki, float kd);
```

### Anti-Windup Contract

```
When saturation occurs (output clamped to output_min or output_max),
the integral term is also clamped to [integral_min, integral_max].
This prevents the integrator from accumulating error while the output
is already saturated — avoiding overshoot on reference change.
```

### Usage Example

```c
#include <robotos/pid.h>
#include <robotos/stepper.h>
#include <robotos/encoder.h>

// PID configuration for velocity control
pid_ctrl_config_t cfg = {
    .kp             = 5.0f,
    .ki             = 0.2f,
    .kd             = 0.05f,
    .output_min     = -2000.0f,  // steps/s
    .output_max     =  2000.0f,
    .integral_min   = -500.0f,
    .integral_max   =  500.0f,
    .sample_time_s  = 0.001f,    // 1 kHz control loop
};

pid_ctrl_t* velocity_pid = pid_create(&cfg);
RO_ASSERT(velocity_pid != NULL, "PID alloc failed");

// Constants (application-defined targets)
float setpoint_vel = 1000.0f;  // target velocity (steps/s)

// In control thread (executes at RO_PRIO_RT_CONTROL, 1 kHz)
stepper_t* x_axis = stepper_get("stepper_x");
encoder_t* x_enc = encoder_get("encoder_x");

float measured = encoder_get_velocity(x_enc, 10);  // velocity over 10 ms window
float cmd_step_rate = pid_update(velocity_pid, setpoint_vel, measured, 0.001f);
stepper_move(x_axis, (int32_t)(cmd_step_rate * 0.001f));
```

---

## Filter API (`filter`)

```c
// filter.h

// ── Exponential moving average (EMA) ─────────────────────────────────────────

typedef struct {
    float alpha;    // Smoothing factor: 0.0 = no update, 1.0 = no filter
    float state;    // Current filtered value
} ema_filter_t;

void  ema_filter_init(ema_filter_t* f, float alpha, float initial);
float ema_filter_update(ema_filter_t* f, float sample);

// ── Moving average ────────────────────────────────────────────────────────────

// N must be a power of 2 for efficient modulo (ring buffer).
typedef struct {
    float*   buf;
    uint16_t n;
    uint16_t head;
    float    sum;
} moving_avg_t;

// buf_storage: caller-provided array of size `n`
ro_status_t moving_avg_init(moving_avg_t* f, float* buf_storage, uint16_t n);
float       moving_avg_update(moving_avg_t* f, float sample);

// ── Notch filter (parametric) ─────────────────────────────────────────────────
typedef struct {
    float b0, b1, b2, a1, a2;   // Biquad coefficients
    float x1, x2, y1, y2;       // State registers
} notch_filter_t;

// f0 = notch frequency (Hz), Q = quality factor,
// fs = sample rate (Hz)
ro_status_t notch_filter_init(notch_filter_t* f, float f0, float Q, float fs);
float       notch_filter_update(notch_filter_t* f, float sample);
```

---

## Limiter API (`limiter`)

```c
// limiter.h

// ── Output clamp ──────────────────────────────────────────────────────────────

static inline float limiter_clamp(float val, float lo, float hi) {
    if (val < lo) return lo;
    if (val > hi) return hi;
    return val;
}

// ── Rate limiter (slew rate) ──────────────────────────────────────────────────

typedef struct {
    float max_rate;     // Maximum change per second (positive)
    float prev_output;  // Last output value
} rate_limiter_t;

void  rate_limiter_init(rate_limiter_t* r, float max_rate, float initial);
float rate_limiter_update(rate_limiter_t* r, float target, float dt_s);

// ── Acceleration limiter ──────────────────────────────────────────────────────

typedef struct {
    float max_accel;    // Maximum acceleration (units/s²)
    float max_speed;    // Maximum speed (units/s)
    float velocity;     // Current velocity (internal state)
    float position;     // Current position (internal state)
} accel_limiter_t;

void  accel_limiter_init(accel_limiter_t* a, float max_accel, float max_speed);
float accel_limiter_update(accel_limiter_t* a, float target_pos, float dt_s);
```

---

## Robot State Machine API (`robot_sm`)

### States

```c
// robot_sm.h

typedef enum {
    STATE_IDLE    = 0,  // Motor disabled, waiting for commands
    STATE_HOMING  = 1,  // Homing sequence in progress
    STATE_RUN     = 2,  // Normal operation — commands accepted
    STATE_FAULT   = 3,  // Faulted — must be cleared before resuming
} robot_state_t;
```

### State Transition Diagram

```
                   ┌──────────────┐
             ┌────►│              │◄────────────────────────────┐
             │     │    IDLE      │                             │
             │     │              │                             │
             │     └──────┬───────┘                             │
             │            │                               sm_clear_fault()
             │            │ sm_dispatch(CMD_HOME)               │
             │            ▼                                     │
             │     ┌──────────────┐                             │
             │     │              │  endstop triggered          │
             │     │   HOMING     ├────────────────────►  [ homing done? ] ──► RUN
             │     │              │                             │
             │     └──────┬───────┘                             │
             │            │ homing complete                     │
             │            ▼                                     │
             │     ┌──────────────┐                             │
             │     │              │                             │
             │     │    RUN       │◄────────────────────────────┘
             │     │              │
             │     └──────┬───────┘
             │            │ sm_dispatch(CMD_STOP)
             │            ▼
             │     ┬──────────────┐
             └─────┤    IDLE (2)  │
                   └──────────────┘

    ANY STATE ──► FAULT: sm_dispatch(CMD_EMERGENCY_STOP)
                         or watchdog timeout
                         or hardware fault signal

    FAULT ──► IDLE: sm_clear_fault() (only if fault condition resolved)
```

### API

```c
typedef struct sm sm_t;

typedef enum {
    CMD_HOME              = 0,
    CMD_START             = 1,
    CMD_STOP              = 2,
    CMD_EMERGENCY_STOP    = 3,
    CMD_FAULT             = 4,  // Internal: triggered by watchdog/HW
    CMD_HOMING_COMPLETE   = 5,  // Internal: fired by homing subsystem
} robot_cmd_t;
```

### Command Legality Matrix

The table below defines which commands are valid (VALID) or invalid (invalid → `RO_EBUSY`) in each state:

| Command | IDLE | HOMING | RUN | FAULT |
|---|---|---|---|---|
| `CMD_HOME` | VALID (IDLE→HOMING) | INVALID (RO_EBUSY) | INVALID (RO_EBUSY) | INVALID (RO_EBUSY) |
| `CMD_START` | INVALID (RO_EBUSY) | INVALID (RO_EBUSY) | INVALID (RO_EBUSY) | INVALID (RO_EBUSY) |
| `CMD_STOP` | VALID (no-op) | INVALID (RO_EBUSY) | VALID (RUN→IDLE) | INVALID (RO_EBUSY) |
| `CMD_EMERGENCY_STOP` | VALID (→FAULT) | VALID (→FAULT) | VALID (→FAULT) | VALID (no-op) |
| `CMD_FAULT` | VALID (→FAULT) | VALID (→FAULT) | VALID (→FAULT) | VALID (no-op) |
| `CMD_HOMING_COMPLETE` | INVALID (RO_EBUSY) | VALID (HOMING→IDLE) | INVALID (RO_EBUSY) | INVALID (RO_EBUSY) |

**Dispatch Semantics:**
- `sm_dispatch()` returns `RO_OK` if command is valid in current state (transition executed).
- `sm_dispatch()` returns `RO_EBUSY` if command is invalid in current state (no transition; state unchanged).
- **Result:** All command rejections are explicit and reportable; silent failures are prevented.

### Lifecycle

```c
// Create a state machine object (no thread created).
// Returns NULL if the internal pool is exhausted.
sm_t*       sm_create(void);
void        sm_destroy(sm_t* sm);
```

### Optional Event-Loop Helper

```c
// Start a dedicated event-loop thread for this state machine (opt-in helper).
// The thread calls sm_dispatch() for queued commands at RO_PRIO_FRAMEWORK.
// thread_cfg.stack and thread_cfg.stack_size MUST be caller-provided (static pool).
// If NOT called: the application is responsible for driving the SM via
// sm_dispatch() from its own thread (default / recommended for App layer).
ro_status_t sm_runloop_start(sm_t* sm, const ro_thread_config_t* thread_cfg);
void        sm_runloop_stop(sm_t* sm);
```

### Command Dispatch

```c
// Dispatch a command to the state machine (thread-safe, non-blocking).
// Returns RO_EBUSY if command not valid in current state.
ro_status_t sm_dispatch(sm_t* sm, robot_cmd_t cmd);
```

### State Query

```c
robot_state_t sm_get_state(const sm_t* sm);
const char*   sm_state_name(robot_state_t state);
```

### Fault Management

```c
// Record a fault reason string (stored internally, logged on transition).
void        sm_set_fault_reason(sm_t* sm, const char* reason);
const char* sm_get_fault_reason(const sm_t* sm);

// Clear fault and transition to IDLE (caller must resolve hardware issue first).
ro_status_t sm_clear_fault(sm_t* sm);
```

### State Change Callback

```c
typedef void (*sm_transition_cb_t)(sm_t* sm,
                                   robot_state_t from,
                                   robot_state_t to,
                                   void* user);

ro_status_t sm_set_transition_cb(sm_t* sm, sm_transition_cb_t cb, void* user);
```

### Fault Handling Pattern

```c
// Watchdog thread checks system health and drives SM to FAULT
// Note: temperature_too_high() and overcurrent_detected() are pseudo-code
// (application-defined; implement via ADC polling, circuit sensors, etc.).
static void watchdog_thread(void* arg) {
    sm_t* sm = (sm_t*)arg;
    while (1) {
        // Application-defined health check functions
        if (temperature_too_high() || overcurrent_detected()) {
            sm_set_fault_reason(sm, "overcurrent or overtemp");
            sm_dispatch(sm, CMD_FAULT);
        }
        ro_thread_sleep_ms(10);
    }
}
```

---

## Callback and Execution Context Rules

### sm_transition_cb_t (State Machine Transition Callback)

**Execution Context:** Application thread context only.  
Callback is invoked from the **thread that called `sm_dispatch()`**, never from ISR.

**Reentrancy:** NOT reentrant. Callback MUST NOT call `sm_dispatch()` on ANY state machine (same or different instance).  
Violation results in: deadlock, undefined behavior, stack overflow.

**Work Budget:** Callback work counts against application thread deadline budgets.  
Recommended: Complete in < 500 µs. MUST NOT block, sleep, or call `ro_thread_sleep_ms()`.

**Safe Operations:**
```c
- stepper_move(), stepper_stop(), encoder_reset()  [Framework operations]
- servo_set_angle(), dcmotor_set_speed()           [Framework operations]
- ro_log(), ro_trace()                             [Adapter diagnostics]
- atomic_store/load()                              [atomic operations]
```

**UNSAFE Operations (Risk of Deadlock / Reentrance):**
```c
- sm_dispatch()  [any state machine — DEADLOCK RISK]
- stepper_wait_idle(), ro_sem_take()  [blocking calls in ISR-derived chain]
- sensor_read(), ro_i2c_transfer()  [I2C/SPI may timeout]
- Any call that acquires ro_mutex while holding another lock
```

### endstop_trigger_cb_t (Hardware Interrupt Callback)

**Execution Context:** ISR context (hardware interrupt).  
Callback is invoked from hardware interrupt handler; caller MUST adhere to ISR execution contract.

**Reentrancy:** Fully reentrant. Multiple endstops may fire simultaneously from different interrupt sources.  
Callback must use ONLY atomic and `_isr`-variant Adapter operations.

**Work Budget:** Must execute in < 10 µs (ISR latency budget).  
MUST NOT block, sleep, allocate, or call non-_isr Adapter APIs.

**Safe Operations:**
```c
- ro_queue_send_isr()               [ISR-safe queue send]
- ro_sem_give_isr()                 [ISR-safe semaphore post]
- atomic_store(), atomic_load()     [ISR-safe atomics]
- ro_trace_event_isr()              [ISR-safe tracing]
```

**UNSAFE Operations (ISR Context Violations):**
```c
- ro_queue_send()                   [blocks; illegal in ISR]
- stepper_move(), servo_set_angle() [may call ro_mutex_lock()]
- sm_dispatch()                     [thread-level state machine]
- ro_sem_take(), ro_thread_sleep_ms(), ro_thread_create()  [blocking in ISR]
```

```c
typedef void (*endstop_trigger_cb_t)(endstop_t* es, void* user);

ro_status_t endstop_set_callback(endstop_t* es, endstop_trigger_cb_t cb, void* user);
ro_status_t endstop_clear_callback(endstop_t* es);
```

### Polling vs ISR-Driven Pattern (Canonical vs Bring-Up)

**BRING-UP ONLY (non-canonical):**
```c
// Polling example — used only for initial debug/bringup, then replaced.
// NOT suitable for production.
while (!endstop_is_triggered(es)) {
    ro_thread_sleep_ms(1);  // 1 ms latency; blocking poll; jitter ✗
}
```

**CANONICAL (production):**
```c
// Interrupt-driven with semaphore wakeup (deterministic < 10 µs latency).
// Homing semaphore: ISR signals when endstop is triggered.
static ro_sem_t* homing_sem = NULL;

static void homing_endstop_isr(endstop_t* es, void* user) {
    // ISR context; wakes waiting homing thread
    ro_sem_give_isr((ro_sem_t*)user);
}

// do_homing: move stepper toward endstop until trigger, then back off and zero
// Parameters:
//   sm: state machine (receive homing completion event)
//   x: stepper motor (X axis)
//   enc: encoder associated with X axis (to zero after homing)
//   es: endstop sensor (triggers when limit reached)
void do_homing(sm_t* sm, stepper_t* x, encoder_t* enc, endstop_t* es) {
    // Allocate semaphore from Adapter pool (binary: initial=0, max=1)
    homing_sem = ro_sem_create(0, 1);
    RO_ASSERT(homing_sem != NULL, "homing_sem alloc failed");
    
    // Register ISR callback; pass semaphore as user data
    endstop_set_callback(es, homing_endstop_isr, homing_sem);
    
    // Move toward endstop (negative direction, large step count)
    stepper_move(x, -100000);
    
    // Block until endstop ISR fires (signals semaphore)
    // Deterministic < 10 µs latency vs polling (~50 ms latency)
    ro_status_t s = ro_sem_take(homing_sem, 5000);
    if (s != RO_OK) {
        RO_LOG_ERROR("Homing timeout or endstop never triggered");
        sm_set_fault_reason(sm, "homing_timeout");
        sm_dispatch(sm, CMD_FAULT);
        goto cleanup;
    }
    
    // Endstop triggered; stop stepper and wait for idle
    stepper_stop(x);
    stepper_wait_idle(x, 1000);
    
    // Back off from endstop
    stepper_move(x, 200);
    stepper_wait_idle(x, 1000);
    
    // Zero position (encoder + stepper counters)
    encoder_reset(enc);
    
cleanup:
    // Deregister callback and release semaphore
    endstop_clear_callback(es);
    ro_sem_destroy(homing_sem);
    homing_sem = NULL;
    
    // Notify state machine
    if (s == RO_OK) {
        sm_dispatch(sm, CMD_HOMING_COMPLETE);
    }
}
```

---

## DT-Based Device Binding

### Overview

Framework device components do not create Zephyr devices — they bind to them via Device Tree labels at boot. Binding occurs in the `POST_KERNEL` phase, before any application threads start.

**Normative Contract:**
- All Framework hardware devices (stepper, servo, encoder, endstop, sensor) MUST be discovered via Device Tree labels.
- Binding MUST occur at `POST_KERNEL` SYS_INIT time (no runtime injection).
- Get/put lifecycle MUST respect pool exhaustion (return NULL / RO_EBUSY).

**Implementation Pattern (illustrative):**
The following shows a typical pattern; actual implementation may vary by driver.

```
POST_KERNEL SYS_INIT order:
    ro_trace_init  (priority 60)
    ro_log_init    (priority 61)
    stepper_drv_bind_all  (priority 80) ← binds DT nodes to stepper pool
    servo_drv_bind_all    (priority 81)
    encoder_drv_bind_all  (priority 82)
    endstop_drv_bind_all  (priority 83)
    sensor_drv_bind_all   (priority 84)

APPLICATION SYS_INIT order:
    robot_sm_init  (priority 10)  ← creates state machine object only; NO thread started
    app_init       (priority 20)  ← application creates threads + calls sm_runloop_start()
                                    (or drives sm manually via sm_dispatch)
```

### DT Node Binding Pattern

Below is an **illustrative reference pattern** for how driver implementations should bind DT nodes to Framework pools. Specific implementations may differ; what matters is the **normative outcome**: DT nodes are registered in component pools at `POST_KERNEL` time.

```c
// src/framework/drivers/stepper_drv.c  (reference pattern — may vary in implementation)
#include <zephyr/devicetree.h>   // ✅ permitted in framework/drivers/*.c
#include <zephyr/device.h>       // ✅ permitted in framework/drivers/*.c
#include <robotos/ro_gpio.h>     // ✅ Adapter API for GPIO

// Framework/drivers/*.c may use DT macros to enumerate bound nodes
#define STEPPER_DT_FOREACH(fn)   DT_FOREACH_STATUS_OKAY(robotos_stepper, fn)

static int stepper_drv_bind(const struct device* dev) {
    const ro_gpio_config_t step_cfg = {
        .dt_label    = DT_LABEL(DT_INST(0, robotos_stepper)),
        .pin         = DT_INST_GPIO_PIN(0, step_gpios),
        .active_high = true,
    };
    // Bind GPIO to stepper pool entry (Adapter API only)
    return stepper_pool_register(dev, &step_cfg);
}

SYS_INIT(stepper_drv_bind_all, POST_KERNEL, 80);
```

### Device Tree Overlay Example

```dts
/* boards/arm/custom_v1/custom_v1.overlay */

/ {
    stepper_x: stepper_x {
        compatible = "robotos,stepper";
        step-gpios = <&gpioa 0 GPIO_ACTIVE_HIGH>;
        dir-gpios  = <&gpioa 1 GPIO_ACTIVE_HIGH>;
        en-gpios   = <&gpioa 2 GPIO_ACTIVE_LOW>;
        steps-per-rev = <3200>;  /* 200 steps × 16 microsteps */
        label = "stepper_x";
    };

    endstop_x_min: endstop_x_min {
        compatible = "robotos,endstop";
        gpios = <&gpiob 3 (GPIO_ACTIVE_LOW | GPIO_PULL_UP)>;
        label = "endstop_x_min";
    };
};
```

### stepper_get() / stepper_put() Pool Contract

```
stepper_drv_bind_all()  ──► populates static stepper pool (N entries)
stepper_get("label")    ──► finds entry by label, marks as "in-use"
stepper_put(st)         ──► marks entry as "free"

Pool is static: no malloc. Max concurrent users = pool size (defined in prj.conf).
CONFIG_RO_STEPPER_POOL_SIZE=4  (default)
```

---

## Layer Boundary Rules

### Summary

| File location | May include `<zephyr/*>`? | May include Adapter `ro_*.h`? | May include Framework `*.h`? |
|---|---|---|---|
| `include/robotos/ro_*.h` (Adapter public) | ❌ Never | — | ❌ No Framework |
| `include/robotos/*.h` (Framework public) | ❌ Never | ✅ Yes | ✅ Yes |
| `src/adapter/zephyr/*.c` | ✅ Any | ✅ Yes | ❌ No Framework |
| `src/framework/*.c` (non-driver) | ❌ Never | ✅ Yes | ✅ Yes |
| `src/framework/drivers/*.c` | `<zephyr/devicetree.h>` + `<zephyr/device.h>` only | ✅ Yes | ✅ Yes |
| `app/**` | ❌ Never | ✅ Yes | ✅ Yes |

### Enforcement

```bash
# CI check: framework public headers must not include <zephyr/*>
for f in include/robotos/*.h; do
  # Skip Adapter headers (they're checked separately)
  if [[ "$(basename $f)" != ro_* ]]; then
    if grep -q '#include <zephyr/' "$f"; then
      echo "ERROR: Framework header $f must not include Zephyr"
      exit 1
    fi
  fi
done

# CI check: framework src (non-driver) must not include <zephyr/*>
for f in src/framework/*.c src/framework/*.h; do
  if grep -q '#include <zephyr/' "$f"; then
    echo "ERROR: $f must use Adapter API, not Zephyr directly"
    exit 1
  fi
done
```

---

## Error Model

Two distinct patterns — **never mix them**:

| Function type | Success | Failure | Example |
|---|---|---|---|
| Alloc / acquire | non-NULL pointer | NULL | `stepper_get()`, `pid_create()` |
| Operation | `RO_OK` (0) | negative `ro_status_t` | `stepper_move()`, `servo_set_angle()` |

### Propagation Pattern

```c
stepper_t* st = stepper_get("stepper_x");
if (st == NULL) {
    RO_LOG_ERROR("Failed to acquire stepper_x from DT");
    return RO_ENODEV;
}

ro_status_t s = stepper_configure(st, &cfg);
RO_TRY(s);  // returns the status if not RO_OK

s = stepper_move(st, 400);
if (s == RO_EBUSY) {
    RO_LOG_WARN("stepper_x busy; waiting for idle");
    stepper_wait_idle(st, 1000);
    s = stepper_move(st, 400);
    RO_TRY(s);
}
```

### Fault Escalation

```c
// When a Framework operation detects an unrecoverable error, it:
// 1. Logs the error via ro_log
// 2. Emits a trace event via ro_trace
// 3. Reports RO_EFAIL to the caller
// 4. Caller is responsible for dispatching CMD_FAULT to the state machine

static void check_and_escalate(sm_t* sm, ro_status_t s, const char* ctx) {
    if (RO_IS_FATAL(s)) {
        char reason[64];
        snprintf(reason, sizeof(reason), "%s returned %d", ctx, s);
        sm_set_fault_reason(sm, reason);
        sm_dispatch(sm, CMD_FAULT);
    }
}
```

---

**Document Status:** 🚧 Living Document  
**Layer:** Robot Framework (domain semantics; depends only on Adapter)  
**Related:** [ADAPTER_LAYER.md](ADAPTER_LAYER.md) | [KERNEL_LAYER.md](KERNEL_LAYER.md) | [ARCHITECTURE.md](ARCHITECTURE.md)  
**Last Review:** 2026-03-01
