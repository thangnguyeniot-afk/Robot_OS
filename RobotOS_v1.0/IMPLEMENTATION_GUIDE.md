# RobotOS v1.0 — Implementation Guide

> **Mục đích:** Tài liệu hướng dẫn chi tiết cách code từng layer, từng component, từng feature, cùng với các kỹ thuật lập trình và cấu trúc dữ liệu được sử dụng trong dự án RobotOS Inspire.

---

## Mục lục

1. [Tổng quan kiến trúc](#1-tổng-quan-kiến-trúc)
2. [Cấu trúc thư mục](#2-cấu-trúc-thư-mục)
3. [Hệ thống Build](#3-hệ-thống-build)
4. [Layer 1: Kernel (Zephyr RTOS)](#4-layer-1-kernel-zephyr-rtos)
5. [Layer 2: Adapter](#5-layer-2-adapter)
6. [Layer 3: Framework](#6-layer-3-framework)
7. [Layer 4: Application](#7-layer-4-application)
8. [IPC & Data Flow](#8-ipc--data-flow)
9. [State Machines](#9-state-machines)
10. [Kỹ thuật lập trình nâng cao](#10-kỹ-thuật-lập-trình-nâng-cao)
11. [Cấu trúc dữ liệu](#11-cấu-trúc-dữ-liệu)
12. [Testing](#12-testing)
13. [Cách build & chạy](#13-cách-build--chạy)
14. [Tổng kết file map](#14-tổng-kết-file-map)

---

## 1. Tổng quan kiến trúc

RobotOS sử dụng kiến trúc **4 lớp (4-layer)** theo mô hình **dependency inversion**:

```
┌─────────────────────────────────┐
│  Layer 4: APPLICATION           │  ← G-code parser, Motion planner, App SM
├─────────────────────────────────┤
│  Layer 3: FRAMEWORK             │  ← Stepper, Servo, PID, Filter, Robot SM
├─────────────────────────────────┤
│  Layer 2: ADAPTER               │  ← ro_thread, ro_queue, ro_gpio, ro_log...
├─────────────────────────────────┤
│  Layer 1: KERNEL (Zephyr RTOS)  │  ← k_thread, k_msgq, gpio_pin_configure...
└─────────────────────────────────┘
```

**Nguyên tắc chính:**
- Lớp trên **chỉ gọi** lớp dưới trực tiếp (không nhảy cấp)
- Adapter che giấu hoàn toàn Zephyr API → có thể thay bằng host stubs để test trên PC
- Framework **không có prefix `ro_`** (convention: `stepper_*`, `pid_ctrl_*`, `sm_*`)
- Adapter **luôn có prefix `ro_`** (convention: `ro_thread_*`, `ro_queue_*`)

---

## 2. Cấu trúc thư mục

```
RobotOS_v1.0/
├── CMakeLists.txt              ← Root build (hybrid Zephyr / Host)
├── Kconfig                     ← Konfiguration keys
├── prj.conf                    ← Zephyr project config
├── west.yml                    ← Zephyr manifest
│
├── include/
│   ├── robotos/                ← Public headers (Adapter + Framework)
│   │   ├── ro_status.h         ← Error codes (ro_status_t)
│   │   ├── ro_assert.h         ← Assertion macro
│   │   ├── ro_thread.h         ← Thread management
│   │   ├── ro_time.h           ← Time queries
│   │   ├── ro_timer.h          ← Periodic timer
│   │   ├── ro_queue.h          ← Message queue (IPC)
│   │   ├── ro_pool.h           ← Fixed-block memory pool
│   │   ├── ro_mutex.h          ← Mutex with priority inheritance
│   │   ├── ro_gpio.h           ← GPIO abstraction
│   │   ├── ro_pwm.h            ← PWM abstraction
│   │   ├── ro_i2c.h            ← I2C bus abstraction
│   │   ├── ro_spi.h            ← SPI bus abstraction
│   │   ├── ro_log.h            ← ISR-safe logging
│   │   ├── ro_trace.h          ← Tracing primitives
│   │   ├── ro_deadline.h       ← Deadline monitoring
│   │   ├── stepper.h           ← Stepper motor API
│   │   ├── servo.h             ← Servo motor API
│   │   ├── dcmotor.h           ← DC motor API
│   │   ├── encoder.h           ← Encoder API
│   │   ├── endstop.h           ← Limit switch API
│   │   ├── sensor.h            ← Generic sensor API
│   │   ├── pid.h               ← PID controller
│   │   ├── filter.h            ← Digital filters (EMA, MA, Notch)
│   │   ├── limiter.h           ← Rate / acceleration limiters
│   │   └── robot_sm.h          ← Robot state machine
│   │
│   └── app/                    ← Application-specific headers
│       ├── motion_seg.h        ← Motion segment struct (24B)
│       ├── app_deadlines.h     ← Deadline ID definitions
│       ├── gcode_parser.h      ← G-code parser API
│       ├── motion_planner.h    ← Motion planner API
│       ├── kinematics_cartesian.h ← Cartesian kinematics
│       ├── app_sm.h            ← Application state machine
│       └── config_profiles.h   ← Machine profiles
│
├── src/
│   ├── adapter/
│   │   ├── zephyr/             ← Zephyr backend (14 files)
│   │   │   ├── ro_status.c
│   │   │   ├── ro_thread.c
│   │   │   ├── ro_time.c
│   │   │   ├── ro_timer.c
│   │   │   ├── ro_queue.c
│   │   │   ├── ro_pool.c
│   │   │   ├── ro_mutex.c
│   │   │   ├── ro_gpio.c
│   │   │   ├── ro_pwm.c
│   │   │   ├── ro_i2c.c
│   │   │   ├── ro_spi.c
│   │   │   ├── ro_log.c
│   │   │   ├── ro_trace.c
│   │   │   └── ro_deadline.c
│   │   │
│   │   └── host/               ← Host/PC stubs (15 files)
│   │       ├── ro_status.c
│   │       ├── ro_thread.c     ← Windows + POSIX threading
│   │       ├── ro_time.c       ← QueryPerformanceCounter / clock_gettime
│   │       ├── ro_timer.c
│   │       ├── ro_queue.c      ← Ring buffer + mutex
│   │       ├── ro_pool.c
│   │       ├── ro_mutex.c
│   │       ├── ro_gpio.c       ← No-op stubs
│   │       ├── ro_pwm.c
│   │       ├── ro_i2c.c
│   │       ├── ro_spi.c
│   │       ├── ro_log.c        ← fprintf(stderr)
│   │       ├── ro_trace.c
│   │       ├── ro_deadline.c
│   │       └── ro_assert.c
│   │
│   ├── framework/              ← Framework implementations
│   │   ├── stepper.c
│   │   ├── servo.c
│   │   ├── dcmotor.c
│   │   ├── encoder.c
│   │   ├── endstop.c
│   │   ├── sensor.c
│   │   ├── pid.c
│   │   ├── filter.c
│   │   ├── limiter.c
│   │   ├── robot_sm.c
│   │   └── drivers/
│   │       ├── stepper_drv.c   ← Bresenham pulse driver
│   │       └── endstop_drv.c   ← Debounced endstop driver
│   │
│   └── app/                    ← Application implementations
│       ├── gcode_parser.c
│       ├── motion_planner.c
│       ├── kinematics_cartesian.c
│       ├── app_sm.c
│       ├── config_profiles.c
│       ├── app_glue_robotos.c  ← IPC + threads + initialization
│       ├── app_glue_zephyr.c   ← Zephyr shell + main()
│       └── app_main.c          ← Host main() + stdin loop
│
└── tests/                      ← Unit tests (host build)
    ├── CMakeLists.txt
    ├── test_gcode_parser.c
    ├── test_motion_planner.c
    ├── test_kinematics_cartesian.c
    └── test_app_sm.c
```

---

## 3. Hệ thống Build

### 3.1 Hybrid Build Strategy

RobotOS sử dụng **hybrid build**: cùng 1 source tree nhưng 2 đường build:

| Chế độ | Build tool | Adapter backend | Mục đích |
|--------|-----------|----------------|----------|
| **Zephyr** | West + CMake | `src/adapter/zephyr/` | Firmware thực (STM32, ESP32, nRF52) |
| **Host** | CMake standalone | `src/adapter/host/` | Unit test trên PC (Windows/Linux) |

**Cách phát hiện:** `CMakeLists.txt` check biến `CONFIG_ZEPHYR_KERNEL`:

```cmake
if(DEFINED CONFIG_ZEPHYR_KERNEL)
    find_package(Zephyr REQUIRED)
    # ... link Zephyr adapter sources
else()
    # Host build — link host adapter stubs
endif()
```

### 3.2 Kconfig

Mọi tham số cấu hình đều đi qua Kconfig:

```kconfig
# Adapter layer
CONFIG_RO_MAX_THREADS=8          # Max concurrent threads
CONFIG_RO_LOG_RING_SIZE=2048     # Log ring buffer bytes

# Application layer
CONFIG_APP_CMD_QUEUE_DEPTH=64    # G-code command queue slots
CONFIG_APP_MAX_MOTION_SEGMENTS=64 # Motion segment queue slots
CONFIG_APP_PLANNER_TICK_HZ=1000  # Planner tick rate
CONFIG_APP_DEFAULT_PROFILE=0     # CNC_2AXIS
```

**Kỹ thuật:** Trong host build, các giá trị Kconfig được define trực tiếp bằng `#ifndef` fallback trong source code, không cần file `.config`.

---

## 4. Layer 1: Kernel (Zephyr RTOS)

### Vai trò
Layer 1 là Zephyr RTOS — cung cấp:
- **Scheduler** (cooperative + preemptive)
- **Kernel objects** (k_thread, k_timer, k_msgq, k_mutex, k_sem)
- **Device drivers** (GPIO, PWM, I2C, SPI, UART)
- **Device Tree** bindings

### Kỹ thuật quan trọng
- **Priority system:** Zephyr dùng số âm cho cooperative (realtime), số dương cho preemptive
- **ISR safety:** Các kernel API có variant `_isr` (không block)
- **Device Tree:** Hardware configuration khai báo trong `.dts`, reference bằng `DT_NODELABEL()`

**Chúng ta KHÔNG viết code Layer 1** — chỉ cấu hình qua `prj.conf` và DeviceTree.

---

## 5. Layer 2: Adapter

### 5.1 Triết lý thiết kế

Adapter layer là lớp **trừu tượng hóa (abstraction layer)** che giấu hoàn toàn Zephyr API. Mọi component ở tầng trên chỉ nhìn thấy `ro_*` API.

**Quy ước đặt tên:**
- Prefix: `ro_` (Robot OS)
- File: `ro_{component}.h` / `ro_{component}.c`
- Type: `ro_{component}_t`

### 5.2 Error Model — `ro_status.h`

```c
typedef int32_t ro_status_t;

#define RO_OK            0
#define RO_ETIMEDOUT    -1
#define RO_EAGAIN       -2
#define RO_EBUSY        -3
#define RO_ECANCELED    -4
#define RO_EINVAL      -10
#define RO_ENOMEM      -11
// ... (14 error codes total)

// Macro tiện lợi
#define RO_SUCCEEDED(s)  ((s) >= 0)
#define RO_FAILED(s)     ((s) <  0)
#define RO_IS_FATAL(s)   ((s) <= -20)

// Try macro — early return on error
#define RO_TRY(expr) do { \
    ro_status_t _rc = (expr); \
    if (RO_FAILED(_rc)) return _rc; \
} while(0)
```

**Kỹ thuật:** `RO_TRY` pattern cho phép viết clean error propagation giống Rust's `?` operator.

### 5.3 Thread Management — `ro_thread.h`

```c
typedef struct { int16_t v; } ro_priority_t;

// Priority presets (typed constants)
#define RO_PRIO_RT_PULSE    ((ro_priority_t){-16})
#define RO_PRIO_RT_CONTROL  ((ro_priority_t){-10})
#define RO_PRIO_BACKGROUND  ((ro_priority_t){ 12})

typedef struct {
    const char*   name;
    void*         stack;
    size_t        stack_size;
    ro_priority_t priority;
    void          (*entry)(void*);
    void*         arg;
} ro_thread_config_t;
```

**Kỹ thuật — Opaque Type Pattern:**
`ro_priority_t` là struct chứa 1 field thay vì typedef đơn giản. Điều này tạo **type safety** — compiler sẽ báo lỗi nếu bạn pass `int` thay vì `ro_priority_t`.

**Zephyr implementation** (`src/adapter/zephyr/ro_thread.c`):
- Static pool `THREAD_SLOT[CONFIG_RO_MAX_THREADS]` — tránh dynamic allocation
- Priority mapping: `ro_priority_t.v` < 0 → cooperative (realtime), ≥ 0 → preemptive

**Host implementation** (`src/adapter/host/ro_thread.c`):
- Windows: `_beginthreadex()` + `WaitForSingleObject()`
- POSIX: `pthread_create()` + `pthread_join()`
- Cross-platform via `#ifdef _WIN32`

### 5.4 Message Queue — `ro_queue.h`

```c
typedef struct {
    void*    buffer;       // Caller-provided static buffer
    size_t   item_size;
    uint32_t capacity;
    uint32_t head, tail, count;
    uint8_t  _impl[64];   // Backend-specific storage
} ro_queue_t;
```

**Kỹ thuật — Caller-Provided Buffer:**
Queue KHÔNG dùng malloc. Caller cung cấp buffer tĩnh:
```c
static gcode_cmd_t cmd_buf[64];
ro_queue_t cmd_q;
ro_queue_create(&cmd_q, cmd_buf, sizeof(gcode_cmd_t), 64);
```
→ **Zero heap allocation**, deterministic, embedded-friendly.

**Zephyr backend:** Wraps `k_msgq` (kernel message queue)
**Host backend:** Manual ring buffer + mutex + condition variable

### 5.5 Memory Pool — `ro_pool.h`

**Kỹ thuật — Bitmap Allocator:**
```c
typedef enum {
    RO_POOL_LOCK_NONE   = 0,  // No locking (single-thread)
    RO_POOL_LOCK_ATOMIC = 1,  // Spinlock (ISR-safe)
    RO_POOL_LOCK_MUTEX  = 2,  // Mutex (thread-safe, priority inherit)
} ro_pool_lock_t;
```

Pool sử dụng **bitmap** để track block usage:
- Mỗi bit = 1 block
- `alloc`: tìm first-fit bit = 0, set lên 1
- `free`: clear bit tương ứng
- O(N/32) scan time (32 blocks per word)

3 chế độ lock cho phép cùng 1 pool code chạy trong nhiều context khác nhau.

### 5.6 GPIO / PWM / I2C / SPI

Tất cả follow cùng pattern:
1. `ro_xxx_get()` — acquire hardware resource
2. `ro_xxx_put()` — release
3. Operation functions — `ro_gpio_set()`, `ro_pwm_set_pulse()`, etc.
4. Config struct chứa `dt_label` (DeviceTree node name)

**Host stubs:** Trả về `RO_OK` cho mọi operation — cho phép application code chạy mà không cần hardware thực.

### 5.7 Logging — `ro_log.h`

```c
typedef enum {
    RO_LOG_DEBUG = 0,
    RO_LOG_INFO  = 1,
    RO_LOG_WARN  = 2,
    RO_LOG_ERROR = 3,
} ro_log_level_t;

#define RO_LOG_INFO(fmt, ...) \
    ro_log(RO_LOG_INFO, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
```

**Zephyr:** ISR-safe ring buffer (`CONFIG_RO_LOG_RING_SIZE` bytes) + flush thread
**Host:** Direct `fprintf(stderr, ...)` with timestamp

**Kỹ thuật — ISR-Safe Ring Buffer:**
- Atomic `head`/`tail` pointers (không cần mutex trong ISR)
- Power-of-2 buffer size → modulo bằng bitwise AND
- Flush thread chạy ở low priority, drain buffer ra UART/console

### 5.8 Deadline Monitoring — `ro_deadline.h`

```c
typedef struct {
    uint32_t    budget_us;      // Max allowed execution time
    const char* name;
    uint64_t    _start_us;      // Internal
} ro_deadline_t;

static inline void ro_deadline_begin(ro_deadline_t* d) {
    d->_start_us = ro_time_us();
}
static inline void ro_deadline_end(ro_deadline_t* d) {
    uint64_t elapsed = ro_time_us() - d->_start_us;
    if (elapsed > d->budget_us) {
        // Increment miss counter
    }
}
```

**Kỹ thuật — Inline + Zero-Cost:**
`begin`/`end` là `static inline` → no function call overhead. Deadline miss tracking hoàn toàn transparent cho caller.

---

## 6. Layer 3: Framework

### 6.1 Triết lý thiết kế

Framework cung cấp **hardware-agnostic robotics primitives**:
- Motor drivers (stepper, servo, DC)
- Sensors (encoder, endstop, generic)
- Control algorithms (PID, filters, limiters)
- Robot state machine

**Quy ước:** KHÔNG dùng prefix `ro_`. Dùng tên component trực tiếp: `stepper_*`, `pid_ctrl_*`, `sm_*`.

### 6.2 Stepper Motor — `stepper.h` / `stepper.c`

**Cấu trúc:**
```c
typedef enum {
    STEPPER_IDLE = 0,
    STEPPER_ACCELERATING,
    STEPPER_RUNNING,
    STEPPER_DECELERATING,
    STEPPER_FAULT,
} stepper_state_t;

typedef struct {
    uint32_t max_speed_steps_s;
    uint32_t accel_steps_s2;
    uint32_t decel_steps_s2;
    uint16_t microsteps;
    bool     reverse_direction;
} stepper_config_t;
```

**Kỹ thuật — Trapezoidal Velocity Profile:**
```
speed
  ^
  |      ┌─────────┐
  |     /│  cruise  │\
  |    / │          │ \
  |   /  │          │  \
  |  /   │          │   \
  | / accel         decel \
  └──────────────────────── → steps
```

Implementation trong `stepper.c`:
```c
static void compute_profile(struct stepper* s, uint32_t total_steps) {
    uint32_t accel_dist = (v * v) / (2 * a);
    uint32_t decel_dist = (v * v) / (2 * d);
    if (accel_dist + decel_dist > total_steps) {
        // Triangle profile — can't reach full speed
        accel_dist = total_steps / 2;
    }
}
```

**Kỹ thuật — ISR-Safe Position:**
```c
atomic_int position;  // C11 <stdatomic.h>
```
`stepper_get_position()` dùng `atomic_load()` → safe to call from any context.

**Static Pool Pattern:**
```c
#define STEPPER_MAX_INSTANCES 4
static struct stepper s_pool[STEPPER_MAX_INSTANCES];

stepper_t* stepper_get(const char* dt_label) {
    // Find free slot in pool
    for (int i = 0; i < STEPPER_MAX_INSTANCES; i++) {
        if (!s_pool[i].in_use) { ... }
    }
}
```
→ Zero heap allocation, bounded resource usage.

### 6.3 Stepper Driver (Bresenham) — `stepper_drv.c`

**Kỹ thuật — Bresenham Line Algorithm cho Multi-Axis:**

Thuật toán Bresenham (vốn dùng để vẽ line trên pixel grid) được áp dụng cho step generation:
- Xác định **major axis** (axis có nhiều steps nhất)
- Mỗi tick: major axis luôn step, minor axes step khi accumulated error vượt threshold

```c
bool stepper_drv_tick(stepper_drv_t* drv) {
    for (uint8_t i = 0; i < drv->num_axes; i++) {
        drv->error[i] += drv->steps[i];
        if (2 * drv->error[i] >= drv->major_steps) {
            // Step this axis
            gpio_set(step_pin, HIGH);
            gpio_set(step_pin, LOW);
            drv->error[i] -= drv->major_steps;
        }
    }
}
```

→ Tạo ra **coordinated multi-axis motion** mượt mà, identical tới cách Grbl/Marlin hoạt động.

### 6.4 Servo — `servo.h` / `servo.c`

Map `angle` (0.0–1.0) → `pulse_us` (1000–2000µs) → PWM output:
```c
ro_status_t servo_set_angle(servo_t* servo, float angle) {
    uint32_t range = cfg.max_pulse_us - cfg.min_pulse_us;
    uint32_t pulse_us = cfg.min_pulse_us + (uint32_t)(angle * range);
    return ro_pwm_set_pulse(&s->pwm, pulse_us * 1000);  // µs → ns
}
```

### 6.5 PID Controller — `pid.h` / `pid.c`

**Discrete PID với Anti-Windup:**

```c
float pid_ctrl_update(pid_ctrl_t* pid, float setpoint, float measurement) {
    float error = setpoint - measurement;
    
    // P term
    float p_term = pid->cfg.kp * error;
    
    // I term with anti-windup clamping
    pid->integral += pid->cfg.ki * error * dt;
    pid->integral = clamp(pid->integral, integral_min, integral_max);
    
    // D term (on error)
    float d_term = pid->cfg.kd * (error - pid->prev_error) / dt;
    
    // Output with clamping
    float output = clamp(p_term + pid->integral + d_term, output_min, output_max);
    return output;
}
```

**Kỹ thuật — Anti-Windup:**
Integral clamping ngăn integral term tích lũy quá lớn khi actuator bão hòa. Cấu hình `integral_min/max` riêng biệt với `output_min/max`.

### 6.6 Digital Filters — `filter.h` / `filter.c`

**3 loại filter:**

| Filter | Dùng cho | Complexity |
|--------|---------|------------|
| EMA (Exponential Moving Average) | Noise smoothing nhanh | O(1) state |
| Moving Average (sliding window) | Low-pass simple | O(N) buffer |
| Notch (biquad IIR) | Reject specific frequency | O(1) state, 5 coefficients |

**EMA — Kỹ thuật:**
```c
state = alpha * input + (1 - alpha) * state;
```
Chỉ cần 2 float (alpha + state). `alpha` gần 1 → responsive, gần 0 → heavy smoothing.

**Notch Filter — Kỹ thuật:**
Biquad IIR (Infinite Impulse Response) — 2nd order:
```
y[n] = b0*x[n] + b1*x[n-1] + b2*x[n-2] - a1*y[n-1] - a2*y[n-2]
```
Dùng để loại bỏ nhiễu tại 1 tần số cụ thể (ví dụ: 50/60Hz power line).

### 6.7 Rate / Acceleration Limiters — `limiter.h` / `limiter.c`

**Rate Limiter:**
```c
float rate_limiter_update(rate_limiter_t* r, float input) {
    float delta = input - r->prev_output;
    delta = clamp(delta, -r->max_rate, r->max_rate);
    r->prev_output += delta;
    return r->prev_output;
}
```
→ Giới hạn **tốc độ thay đổi** (slew rate) của output.

**Acceleration Limiter:**
Trapezoidal velocity + position envelope — output theo kiểu "xe chạy": tăng tốc → chạy đều → giảm tốc.

### 6.8 Robot State Machine — `robot_sm.h` / `robot_sm.c`

**Kỹ thuật — Table-Driven FSM:**

```c
// Transition table: [current_state][command] → next_state
static const robot_state_t s_table[STATE_COUNT][CMD_COUNT] = {
    [IDLE]   = { [HOME]=HOMING, [START]=RUN, [E_STOP]=FAULT, ... },
    [HOMING] = { [HOMING_COMPLETE]=IDLE, [E_STOP]=FAULT, ... },
    [RUN]    = { [STOP]=IDLE, [E_STOP]=FAULT, ... },
    [FAULT]  = { ... all INVALID except FAULT→FAULT ... },
};

ro_status_t sm_dispatch(sm_t* sm, robot_cmd_t cmd) {
    robot_state_t next = s_table[sm->state][cmd];
    if (next == INVALID) return RO_EINVAL;
    sm->state = next;
    if (sm->cb) sm->cb(prev, next, cmd, sm->cb_arg);
    return RO_OK;
}
```

**Ưu điểm:**
- O(1) lookup — không cần if/else chain
- Transition table là `const` → ROM, không tốn RAM
- Dễ verify (đọc bảng = hiểu toàn bộ logic)
- Observer pattern qua callback

**Endstop Driver — Debounce:**
```c
static void drv_isr(void* arg) {
    if (now - e->last_trigger_us < DEBOUNCE_US) return;  // 5ms debounce
    e->last_trigger_us = now;
    // ... process
}
```
Software debounce trong ISR context bằng timestamp comparison.

---

## 7. Layer 4: Application

### 7.1 Tổng quan

Application layer implement **robot application logic** cụ thể:

```
stdin/UART → [G-code Parser] → cmd_q → [Motion Planner] → seg_q → [Pulse Manager] → GPIO
```

### 7.2 3-Lane Architecture

```
  Lane A (Slow)        Lane B (RT)         Lane C (RT-Critical)
  ┌──────────┐        ┌──────────┐        ┌──────────┐
  │ t_shell  │──cmd_q→│t_planner │──seg_q→│t_pulse_mgr│
  │ BACKGROUND│        │RT_CONTROL│        │ RT_PULSE  │
  │ 1kB stack│        │ 2kB stack│        │ 1kB stack │
  └──────────┘        └──────────┘        └──────────┘
```

- **Lane A (Shell):** Nhận input (UART/stdin), parse G-code, push vào `cmd_q`
- **Lane B (Planner):** Pop `cmd_q`, convert mm→steps, push segments vào `seg_q`
- **Lane C (Pulse Manager):** Pop `seg_q`, drive stepper pulses qua GPIO

### 7.3 Motion Segment — `motion_seg.h`

```c
typedef struct {
    int32_t  dx_steps;           // 4B
    int32_t  dy_steps;           // 4B
    int32_t  dz_steps;           // 4B
    uint8_t  axis_mask;          // 1B
    uint8_t  _pad[3];            // 3B (alignment)
    uint32_t f_steps_per_s;      // 4B
    uint32_t accel_steps_s2;     // 4B
} motion_seg_t;                  // = 24 bytes

_Static_assert(sizeof(motion_seg_t) == 24, "...");
```

**Kỹ thuật:**
- `_pad[3]` đảm bảo alignment cho `uint32_t` fields sau `uint8_t`
- `_Static_assert` — compile-time check, phát hiện lỗi layout sớm
- 24 bytes × 64 slots = 1536 bytes cho toàn bộ seg_q

### 7.4 G-code Parser — `gcode_parser.h` / `gcode_parser.c`

**Supported commands:**

| G-code | Type | Description |
|--------|------|-------------|
| G0 | MOVE_RAPID | Rapid positioning |
| G1 | MOVE_LINEAR | Linear interpolation |
| G28 | HOME | Home all axes |
| G90 | SET_ABS | Absolute coordinates |
| G91 | SET_REL | Relative coordinates |
| M3 | SPINDLE_ON | Spindle on + speed |
| M5 | SPINDLE_OFF | Spindle off |
| M104 | EXTRUDER_TEMP | Set temperature |

**Kỹ thuật — Streaming Parser:**
- Parse từng line (không cần buffer toàn bộ file)
- Internal state: `s_absolute` (abs/rel mode)
- Comment stripping: `;` và `(` → stop parsing params

```c
// Parse flow:
// "G1 X100.5 Y-50.3 F600"
// → letter='G', code=1 → GCODE_MOVE_LINEAR
// → scan params: X=100.5, Y=-50.3, F=600
```

### 7.5 Motion Planner — `motion_planner.h` / `motion_planner.c`

**Kỹ thuật — Coordinate Pipeline:**

```
gcode_cmd_t (mm, float) 
    → kinematics_cart_to_steps() → cart_steps_t (steps, int32)
    → motion_seg_t (steps + speed + accel)
    → push to seg_q
```

**Position tracking:**
- Machine position kept in mm (float) inside planner
- Absolute/relative mode from parser state
- Home command resets position to (0,0,0)

**Feedrate conversion:**
```c
// mm/min → steps/s
float mm_per_s = feedrate_mm_min / 60.0f;
float max_spm = max(steps_per_mm_x, steps_per_mm_y, steps_per_mm_z);
seg.f_steps_per_s = (uint32_t)(mm_per_s * max_spm);
```

### 7.6 Cartesian Kinematics — `kinematics_cartesian.c`

Simplest kinematics model — 1:1 axis mapping:
```c
out->sx = (int32_t)roundf(pos->x * steps_per_mm_x);
```

**Tại sao tách riêng?** → Dễ swap sang kinematics khác (CoreXY, Delta) mà không đụng planner.

### 7.7 Application State Machine — `app_sm.h` / `app_sm.c`

```
BOOT ──INIT_DONE──→ IDLE ──HOME──→ HOMING ──HOMING_DONE──→ READY
                                                              │
                                                          RUN │
                                                              ▼
                    READY ←──QUEUE_EMPTY── RUNNING ──PAUSE──→ PAUSED
                      │                                        │
                      │←──────────STOP─────────────────────────┘
                      │←──────────RESUME───────────────────────┘
                      
    ANY STATE ──FAULT──→ ERROR ──FAULT_CLEARED──→ IDLE
```

**7 states × 10 commands** — đầy đủ lifecycle.

**Kỹ thuật — Feed Hold:**
```c
_Atomic int g_feed_hold = 0;  // C11 atomic

// In planner thread:
if (atomic_load(&g_feed_hold)) continue;  // Skip processing
```
→ Real-time pause/resume mà không cần state machine transition.

### 7.8 Configuration Profiles — `config_profiles.c`

Pre-built profiles cho common robot configs:

```c
static const config_profile_t s_profiles[PROFILE_COUNT] = {
    [PROFILE_CNC_3AXIS] = {
        .name             = "CNC 3-Axis",
        .num_axes         = 3,
        .steps_per_mm_x   = 80.0f,
        .steps_per_mm_y   = 80.0f,
        .steps_per_mm_z   = 400.0f,
        .max_feedrate_mm_min = 3000.0f,
        .has_spindle      = true,
    },
    // ...
};
```

### 7.9 Glue Files

**`app_glue_robotos.c`** — Brain of initialization:
1. Load config profile
2. Create IPC queues (cmd_q, seg_q)
3. Create motion planner
4. Create App SM
5. Spawn 3 threads
6. Dispatch `INIT_DONE`

**`app_glue_zephyr.c`** — Zephyr-specific:
- Shell command registration (`ro gcode G1 X10`, `ro status`, `ro home`)
- Zephyr `main()` entry point

**`app_main.c`** — Host entry point:
- `main()` for desktop build
- stdin G-code loop
- Auto-transitions (BOOT→IDLE→HOMING→READY→RUNNING)

---

## 8. IPC & Data Flow

### 8.1 Queue Architecture

```
┌────────────┐  gcode_cmd_t  ┌────────────┐  motion_seg_t  ┌────────────┐
│  t_shell   │──── cmd_q ───→│ t_planner  │──── seg_q ────→│t_pulse_mgr │
│            │   (64 slots)  │            │   (64 slots)   │            │
└────────────┘               └────────────┘                └────────────┘
```

**Tại sao 2 queue?**
- **Decoupling:** Shell/parser chạy ở tốc độ input, planner xử lý realtime 1kHz, pulse manager chạy ở khác rate
- **Buffering:** 64-slot queue cho phép burst input mà không mất data
- **Priority isolation:** Mỗi lane chạy ở priority khác nhau

### 8.2 Timeout Strategy

| Queue | Send timeout | Recv timeout |
|-------|-------------|-------------|
| cmd_q | 100 ticks (from shell) | 0 (non-blocking from planner) |
| seg_q | 2 ticks (from planner) | FOREVER (pulse mgr blocks) |

### 8.3 Data Sizes

| Type | Size | Queue depth | Total RAM |
|------|------|-------------|-----------|
| `gcode_cmd_t` | ~40B | 64 | 2560B |
| `motion_seg_t` | 24B | 64 | 1536B |

---

## 9. State Machines

### 9.1 Robot SM (Framework) — 4 states

| From | Command | To |
|------|---------|-----|
| IDLE | HOME | HOMING |
| IDLE | START | RUN |
| HOMING | HOMING_COMPLETE | IDLE |
| HOMING | STOP | IDLE |
| RUN | STOP | IDLE |
| ANY | EMERGENCY_STOP | FAULT |
| FAULT | (sm_clear_fault) | IDLE |

### 9.2 App SM (Application) — 7 states

Full transition table is in `app_sm.c`. Key difference from Robot SM:
- Includes **BOOT** (pre-initialization)
- Includes **READY** (post-homing, pre-run)
- Includes **PAUSED** (feed hold state)
- **QUEUE_EMPTY** signal from planner

### 9.3 Tại sao 2 SM?

- **Robot SM** (Framework): Generic, reusable across applications. Focuses on hardware safety (fault, emergency stop).
- **App SM** (Application): Application-specific lifecycle. Includes business logic states (PAUSED, READY, BOOT).

---

## 10. Kỹ thuật lập trình nâng cao

### 10.1 Opaque Type Pattern
```c
// Header (public)
typedef struct stepper stepper_t;  // Forward declaration only
stepper_t* stepper_get(const char* label);

// Source (private) 
struct stepper {
    // Full definition hidden from users
    const char* label;
    stepper_config_t cfg;
    // ...
};
```
→ **Information hiding**, ABI stability, prevent direct field access.

### 10.2 Static Pool Allocator
```c
#define MAX_INSTANCES 4
static struct stepper s_pool[MAX_INSTANCES];

stepper_t* stepper_get(...) {
    for (i = 0; i < MAX; i++)
        if (!s_pool[i].in_use) return &s_pool[i];
    return NULL;  // Pool exhausted
}
```
→ Zero malloc, bounded memory, compiletime-known worst case.

### 10.3 C11 Atomics for ISR-Safe Access
```c
#include <stdatomic.h>
atomic_int position;

// ISR writes:
atomic_fetch_add(&position, 1);

// Any thread reads:
int32_t pos = atomic_load(&position);
```
→ Lock-free, ISR-safe, no mutex needed for single-word data.

### 10.4 Compound Literal Typed Constants
```c
#define RO_PRIO_RT_PULSE ((ro_priority_t){-16})
```
→ Type-safe constant, compiler-enforced, cannot accidentally mix with plain int.

### 10.5 Designated Initializers
```c
static const robot_state_t table[STATE_COUNT][CMD_COUNT] = {
    [ROBOT_STATE_IDLE] = {
        [ROBOT_CMD_HOME] = ROBOT_STATE_HOMING,
        ...
    },
};
```
→ Self-documenting, sparse initialization, zero for undefined entries.

### 10.6 _Static_assert
```c
_Static_assert(sizeof(motion_seg_t) == 24, "Size mismatch");
```
→ Compile-time structural verification. Catches padding/alignment bugs.

### 10.7 RO_TRY Error Propagation
```c
ro_status_t init_system(void) {
    RO_TRY(ro_queue_create(&q, ...));
    RO_TRY(ro_timer_init(&t, ...));
    RO_TRY(ro_thread_create(&cfg));
    return RO_OK;
}
```
→ Clean error propagation without nested if/else.

### 10.8 Callback / Observer Pattern
```c
typedef void (*sm_transition_cb_t)(state_t from, state_t to, cmd_t cmd, void* arg);
ro_status_t sm_set_transition_cb(sm_t* sm, sm_transition_cb_t cb, void* arg);
```
→ Decoupled notification, multiple observers possible, user_data for context.

### 10.9 Cross-Platform Conditional Compilation
```c
#ifdef _WIN32
    #include <windows.h>
    CRITICAL_SECTION cs;
#else
    #include <pthread.h>
    pthread_mutex_t mtx;
#endif
```
→ Single codebase, dual platform support without runtime overhead.

### 10.10 Inline Functions for Zero-Overhead Abstractions
```c
static inline float limiter_clamp(float v, float lo, float hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}
```
→ Compiler inlines at call site, no function call overhead.

---

## 11. Cấu trúc dữ liệu

### 11.1 Ring Buffer (Queue)
- **Đặc điểm:** Fixed-size circular buffer, FIFO
- **Dùng ở:** `ro_queue_t`, `ro_log.c` ring buffer
- **Complexity:** O(1) enqueue/dequeue
- **Kỹ thuật:** `head = (head + 1) % capacity`

### 11.2 Bitmap
- **Đặc điểm:** Compact bit-level tracking
- **Dùng ở:** `ro_pool.c` block allocation
- **Complexity:** O(N/32) first-fit scan
- **Kỹ thuật:** `word & (1 << bit)` test, `word |= (1 << bit)` set

### 11.3 Static Pool
- **Đặc điểm:** Pre-allocated array of instances
- **Dùng ở:** stepper, servo, dcmotor, encoder, endstop, sensor
- **Complexity:** O(N) find-free, O(1) return
- **Trade-off:** Bounded max instances vs. zero heap allocation

### 11.4 Lookup Table (2D Array)
- **Đặc điểm:** [state][command] → next_state
- **Dùng ở:** `robot_sm.c`, `app_sm.c`
- **Complexity:** O(1) transition lookup
- **Storage:** `const` → resides in ROM/flash

### 11.5 Opaque Struct (with _impl buffer)
```c
typedef struct {
    // Public fields
    uint8_t _impl[64];  // Backend-private storage
} ro_queue_t;
```
- **Kỹ thuật:** Embedded backend-specific data without exposing backend types
- **Dùng ở:** `ro_queue_t`, `ro_timer_t`

### 11.6 Biquad Filter State
```c
typedef struct {
    float b0, b1, b2, a1, a2;  // Coefficients
    float x1, x2, y1, y2;       // State (2 input + 2 output history)
} notch_filter_t;
```
→ 9 floats = 36 bytes tổng. Constant memory per filter instance.

---

## 12. Testing

### 12.1 Test Strategy

Sử dụng **host build** + assert-based unit tests:

| Test file | Component | Test count |
|-----------|-----------|-----------|
| `test_gcode_parser.c` | G-code parser | 12 tests |
| `test_motion_planner.c` | Motion planner | 6 tests |
| `test_kinematics_cartesian.c` | Kinematics | 6 tests |
| `test_app_sm.c` | App state machine | 8 tests |

### 12.2 Test Pattern

```c
#define TEST(name)  static void name(void)
#define RUN(name)   do { printf("  %-40s", #name); name(); printf("PASS\n"); } while(0)

TEST(test_g1_linear_with_feedrate) {
    gcode_cmd_t cmd;
    ASSERT_EQ_INT(RO_OK, gcode_parse_line("G1 X100.5 Y-50.3 F600", &cmd));
    ASSERT_EQ_INT(GCODE_MOVE_LINEAR, cmd.type);
    ASSERT_EQ_FLOAT(100.5f, cmd.x);
}

int main(void) {
    RUN(test_g1_linear_with_feedrate);
    printf("All tests passed!\n");
    return 0;
}
```

→ Lightweight, no framework dependency, easy to read/debug.

### 12.3 Coverage

Tests cover:
- **Parser:** All G-code types, parameters, comments, error cases
- **Kinematics:** Forward/inverse, negative, fractional, null safety
- **App SM:** Full lifecycle, fault from every state, callback, invalid transitions
- **Planner:** Queue empty, linear/rapid moves, position tracking, home, relative mode

---

## 13. Cách build & chạy

### 13.1 Host Build (Windows/Linux — Unit Tests)

```bash
cd RobotOS_v1.0
mkdir build && cd build
cmake -DHOST_BUILD=ON ..
cmake --build .
ctest --output-on-failure
```

### 13.2 Zephyr Build (Firmware)

```bash
# Install Zephyr SDK first (see Zephyr Getting Started Guide)
cd RobotOS_v1.0
west init -l .
west update
west build -b nucleo_f411re .
west flash
```

### 13.3 Host Interactive Mode

```bash
cd build
./app_main          # or app_main.exe on Windows
# Enter G-code:
G28                 # Home
G1 X100 Y50 F600   # Linear move
status              # Print state
exit                # Quit
```

---

## 14. Tổng kết file map

| Layer | Headers | Sources | Tests | Total |
|-------|---------|---------|-------|-------|
| Build System | — | 4 | — | 4 |
| Adapter | 15 | 29 (14 Zephyr + 15 Host) | — | 44 |
| Framework | 10 | 12 | — | 22 |
| Application | 7 | 8 | 4 + CMakeLists | 20 |
| **Total** | **32** | **53** | **5** | **90** |

---

> **Lưu ý:** Đây là bản v1.0 MVP. Các tính năng Phase-2 (arc interpolation, filesystem, temperature wait, ISR event queue) được đánh dấu `CONFIG_APP_ENABLE_*=n` trong Kconfig và sẽ được implement từng bước.
