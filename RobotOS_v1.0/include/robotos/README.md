# include/robotos/

Chứa toàn bộ public header của **Adapter Layer** (prefix `ro_`) và **Framework Layer**.

---

## Adapter Headers — `ro_*.h` (15 files)

Đây là Hardware Abstraction Layer (HAL). Mọi code bên trên chỉ gọi APInày, không bao giờ gọi Zephyr trực tiếp.

| Header | Mô tả |
|--------|-------|
| `ro_status.h` | Kiểu `ro_status_t = int32_t`, error codes (RO_OK=0 … EBOOTSEQ=-23) |
| `ro_assert.h` | `RO_ASSERT(expr)`, `RO_ASSERT_MSG(expr, fmt, ...)` |
| `ro_thread.h` | `ro_thread_create/yield/sleep_ms/join` |
| `ro_time.h` | `ro_time_now_us()`, `ro_time_now_ms()` |
| `ro_timer.h` | `ro_timer_t`, one-shot & periodic software timers |
| `ro_queue.h` | `ro_queue_t`, `put/get/put_isr/get_isr` (zero-copy) |
| `ro_pool.h` | Fixed-size memory pool, `ro_pool_alloc/free` |
| `ro_mutex.h` | `ro_mutex_lock/unlock/trylock` |
| `ro_gpio.h` | `ro_gpio_set/get/configure` |
| `ro_pwm.h` | `ro_pwm_enable/disable/set_duty` |
| `ro_i2c.h` | `ro_i2c_write/read/write_then_read` |
| `ro_spi.h` | `ro_spi_transfer/write/read` |
| `ro_log.h` | `RO_LOG_INF/WRN/ERR/DBG(module, fmt, ...)` |
| `ro_trace.h` | `RO_TRACE_EVENT(id)` — debug event markers |
| `ro_deadline.h` | `ro_deadline_t`, `ro_deadline_set/is_expired` |

---

## Framework Headers (10 files)

| Header | Component | Mô tả |
|--------|-----------|-------|
| `stepper.h` | Stepper Motor | `stepper_t`, init/enable/disable/move/is_done |
| `servo.h` | Servo | `servo_t`, init/set_angle/set_pulse_us |
| `dcmotor.h` | DC Motor | `dcmotor_t`, init/forward/backward/brake/speed |
| `encoder.h` | Encoder | `encoder_t`, init/read/reset |
| `endstop.h` | Endstop Switch | `endstop_t`, init/is_triggered/wait |
| `sensor.h` | Generic Sensor | `sensor_t`, init/read/calibrate |
| `pid.h` | PID Controller | `pid_t`, init/update/reset/set_gains |
| `filter.h` | Signal Filters | `lpf_t/ema_t/median_t`, MA/EMA/Median/IIR |
| `limiter.h` | Rate/Value Limiter | `limiter_t`, clamp + rate-limit |
| `robot_sm.h` | Robot State Machine | `robot_state_t`, SM events, transitions |

---

## Kiểu dữ liệu cốt lõi

```c
// Status code
typedef int32_t ro_status_t;
#define RO_OK           0
#define RO_ETIMEDOUT   (-1)
// ... đến EBOOTSEQ (-23)

// Thread priority
typedef struct { int16_t v; } ro_priority_t;
#define RT_PULSE      ((ro_priority_t){-16})
#define RT_CONTROL    ((ro_priority_t){-10})
#define RT_MONITOR    ((ro_priority_t){-2})
#define FRAMEWORK     ((ro_priority_t){ 0})
#define APP           ((ro_priority_t){ 5})
#define BACKGROUND    ((ro_priority_t){12})
```
