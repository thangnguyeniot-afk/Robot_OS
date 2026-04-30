# src/framework/ — Robot Framework Layer

> Robotics domain semantics layer — các component phần cứng-independent:
> motor control, signal processing, state machines.
> Chỉ phụ thuộc vào Adapter layer (`ro_*` APIs). Không bao giờ include `<zephyr/*>`.

---

## 1. Bài Toán Cần Giải

Adapter layer cung cấp thread, queue, GPIO, PWM… nhưng **không biết gì về robotics**.
Application cần điều khiển stepper motor, đọc encoder, chạy PID loop, quản lý trạng thái robot…
nhưng không nên tự implement từng thứ từ đầu.

Framework layer giải quyết khoảng cách này:
> *"Làm thế nào để move một stepper motor? Chạy PID? Quản lý IDLE → HOMING → RUN → FAULT?"*

---

## 2. Vị Trí Trong Stack

```
Application  →  dùng Framework API (stepper_move_to, pid_ctrl_update, sm_dispatch…)
     ▼
┌─────────────────────────────────────────────────────────────┐
│              ROBOT FRAMEWORK LAYER  (thư mục này)           │
│  Semantics: biết "stepper", "servo", "PID" là gì            │
│  Portable: chỉ gọi ro_gpio / ro_pwm / ro_timer / ro_mutex   │
│  Không biết k_thread, k_msgq, GPIO_DT_SPEC                  │
└─────────────────────────────────────────────────────────────┘
     ▼
Adapter  →  ro_gpio, ro_pwm, ro_timer, ro_mutex, ro_pool…
     ▼
Zephyr / Host
```

---

## 3. Danh Sách Modules

| File | Component | Điểm nổi bật |
|------|-----------|-------------|
| `stepper.c` + `drivers/stepper_drv.c` | Stepper Motor | Trapezoidal profile + Bresenham DDA |
| `servo.c` | Servo PWM | Angle → pulse width → `ro_pwm_set_duty` |
| `dcmotor.c` | DC Motor | PWM + DIR pin, soft start/stop |
| `encoder.c` | Encoder | GPIO interrupt counting, velocity estimation |
| `endstop.c` + `drivers/endstop_drv.c` | Endstop | Debounce + blocking wait |
| `sensor.c` | Generic Sensor | ADC + scale + offset calibration |
| `pid.c` | PID Controller | Anti-windup, output clamping, derivative |
| `filter.c` | Signal Filters | EMA, Moving Average, Notch (IIR biquad) |
| `limiter.c` | Rate/Value Limiter | Clamp + slew rate |
| `robot_sm.c` | Robot State Machine | Table-driven FSM |

---

## 4. Giải Pháp Implement — Từng Module

### 4.1 `stepper.c` — Trapezoidal Velocity Profile
**Bài toán:** Di chuyển stepper motor đến vị trí target với gia tốc/giảm tốc smooth.

**Ý tưởng:** *Trapezoidal motion profile* — chia một move thành 3 phase:
```
Speed
  │        ___________
  │       /           \
  │      /             \
  │─────/───────────────\─────► Steps
        Accel  Cruise  Decel
```

**Giải pháp:**
```c
compute_profile(total_steps):
  accel_dist = v_max² / (2 * accel)
  decel_dist = v_max² / (2 * decel)
  if accel + decel > total: dùng triangle profile (không đạt v_max)

pulse_tick() [timer callback — ISR context]:
  if steps_done < accel_steps: ACCELERATING (tăng speed)
  else if steps_done < decel_start: CRUISING
  else: DECELERATING (giảm speed)
  → GPIO pulse step pin HIGH / LOW (width = 2µs)
  → update atomic position counter
  → nếu done: stop timer, gọi done_cb
```

Internal pool: `s_pool[STEPPER_MAX_INSTANCES=4]` — static, no heap.

**Điểm học được:**
- Timer callback chạy trong ISR context → không được gọi `k_mutex_lock`. Cần `ro_mutex` chỉ ở config path, không ở pulse path.
- `atomic_int position` đúng cho read từ application thread trong khi ISR đang update.
- `ro_delay_us(2)` trong ISR là nguy hiểm trên hầu hết MCU — cần kiểm tra xem có phù hợp với timer period không.

---

### 4.2 `drivers/stepper_drv.c` — Bresenham DDA Multi-Axis
**Bài toán:** Nhiều axis cần step đồng bộ để tạo đường thẳng trong không gian (G-code linear interpolation).

**Ý tưởng:** *Bresenham line algorithm* áp dụng cho motion:
```
Ví dụ: move X=100, Y=60 steps
  major_axis = X (100 steps)
  error = 0

  ISR tick N:
    error_y += 60
    if 2*error_y >= 100: step Y, error_y -= 100
    step X (always — major axis)
```

**Giải pháp:**
```c
stepper_drv_load_segment(steps[], axis_mask)  ← load target steps per axis
stepper_drv_tick()  [ISR]                     ← gọi mỗi timer tick
  for each axis:
    error[i] += steps[i]
    if 2*error[i] >= major_steps: pulse step pin, error[i] -= major_steps
  step_count++
  return active (true nếu chưa xong)
```

**Điểm học được:**
- `stepper_drv` và `stepper.c` là **hai layer khác nhau**: `stepper.c` quản lý profile + state, `stepper_drv` chỉ là pulse generator thuần.
- Application gọi `stepper.c` (high-level). Pulse engine gọi `stepper_drv.c` (low-level, ISR).

---

### 4.3 `pid.c` — Discrete PID với Anti-Windup
**Bài toán:** Điều khiển vòng kín (servo/motor/temperature) ở fixed sample rate.

**Giải pháp:** Discrete PID chuẩn với integral clamping:
```
error = setpoint - measurement
P = Kp * error
I += Ki * error * dt  [clamp to integral_min..integral_max]
D = Kd * (error - prev_error) / dt
output = P + I + D     [clamp to output_min..output_max]
```

**Anti-windup:** Clamp integral **trước** khi cộng vào output, không phải clamp output sau.
Kết quả: khi actuator bão hòa, integral không tiếp tục tích lũy → recovery nhanh hơn.

**Điểm học được:**
- Module này **không dùng bất kỳ OS primitive nào** → 100% portable, testable trên host thuần C.
- `sample_time_s` phải khớp với tần số thực tế gọi `pid_ctrl_update()` — không tự đo thời gian.
- Derivative on error (thay vì on measurement) gây "derivative kick" khi setpoint thay đổi đột ngột. Production nên dùng derivative on measurement.

---

### 4.4 `filter.c` — 3 Loại Digital Filter
**Bài toán:** Loại nhiễu tín hiệu sensor trước khi đưa vào PID hoặc kinematics.

**Giải pháp:**

**EMA (Exponential Moving Average):**
```
state = alpha * input + (1 - alpha) * state
```
- `alpha` gần 1 → theo sát input (ít lọc)
- `alpha` gần 0 → bỏ qua input (lọc nhiều)
- Memory: 1 float, O(1) mỗi sample

**Moving Average (sliding window):**
```
sum -= buf[head]   // bỏ sample cũ
buf[head] = input  // thêm sample mới
sum += input
output = sum / n
head = (head + 1) % n
```
- Memory: `n * sizeof(float)` — caller cung cấp buffer (không malloc)
- O(1) mỗi sample (rolling sum, không tính lại từ đầu)

**Notch Filter (IIR Biquad):**
```
H(z) = (b0 + b1*z⁻¹ + b2*z⁻²) / (1 + a1*z⁻¹ + a2*z⁻²)
```
Thiết kế: từ `freq_hz`, `sample_hz`, `bandwidth` → tính coefficients tự động.

**Điểm học được:**
- Notch filter dùng `sinf`, `cosf`, `sinhf`, `logf` → chỉ gọi 1 lần tại `init()`, không trong hot path → OK.
- Moving average cần caller cấp `float* buf`, `uint32_t n` → đúng với "no malloc" principle.

---

### 4.5 `robot_sm.c` — Table-Driven State Machine
**Bài toán:** Quản lý vòng đời robot (IDLE/HOMING/RUN/FAULT) rõ ràng, không spaghetti if/else.

**Ý tưởng:** *Transition table* (lookup table) thay vì if/else chains:
```c
// [current_state][command] → next_state
static const robot_state_t s_table[ROBOT_STATE_COUNT][ROBOT_CMD_COUNT] = {
    [ROBOT_STATE_IDLE][ROBOT_CMD_HOME]  = ROBOT_STATE_HOMING,
    [ROBOT_STATE_IDLE][ROBOT_CMD_START] = ROBOT_STATE_RUN,
    ...
    [ROBOT_STATE_FAULT][ROBOT_CMD_STOP] = INVALID,  // không cho phép
};
```

**Giải pháp:**
```c
sm_dispatch(sm, cmd):
  next = s_table[sm->state][cmd]
  if next == INVALID: log warning, return RO_EINVAL
  else: sm->state = next, gọi transition_cb(old, new)
```

**Điểm học được:**
- `sm_create()` dùng `calloc` → vi phạm "no heap" nếu gọi sau init. Cần chuyển sang static pool.
- Transition table là pattern rất tốt: thêm state/event mới chỉ sửa table, không sửa logic.
- `sm_clear_fault()` cần được implement rõ ràng (hiện chưa có trong table — FAULT chỉ accept FAULT và ESTOP).

---

## 5. Phân Tách `stepper.c` vs `stepper_drv.c`

Đây là điểm thiết kế quan trọng nhất của Framework:

```
stepper.c (high-level)             stepper_drv.c (low-level)
─────────────────────              ───────────────────────────
Trapezoidal profile logic          Bresenham DDA per axis
State: IDLE/ACCEL/CRUISE/DECEL     State: active/done
Gọi từ Application thread          Gọi từ ISR timer callback
Knows về velocity, position        Chỉ biết pulse GPIO
```

Lý do tách:
- Application cần API level cao: `move_to(1000 steps, 500 steps/s)`
- Pulse ISR cần API cực nhanh: `tick()` — chỉ làm một pulse, return ngay

---

## 6. Nguyên Tắc Không Được Vi Phạm

```c
// ❌ SAI — Framework code includes Zephyr
#include <zephyr/kernel.h>    // FORBIDDEN in src/framework/*.c

// ✅ ĐÚNG — Framework chỉ dùng Adapter
#include <robotos/ro_gpio.h>
#include <robotos/ro_timer.h>
#include <robotos/ro_mutex.h>
```

```c
// ❌ SAI — Framework tự malloc
struct stepper* s = malloc(sizeof(*s));

// ✅ ĐÚNG — Static pool
static struct stepper s_pool[STEPPER_MAX_INSTANCES];
```

---

## 7. Checklist — Khi Viết Framework Module Mới

- [ ] Header trong `include/robotos/` — không có `#include <zephyr/*>` hay OS-specific
- [ ] Chỉ gọi `ro_*` APIs từ Adapter
- [ ] Static pool hoặc caller-provided buffer — không `malloc`
- [ ] ISR-path code không gọi blocking primitives (mutex, sleep…)
- [ ] Unit test chạy được trên host (`HOST_BUILD=ON`)
- [ ] `robot_sm.c`: mọi state transition đều trong lookup table
stepper_move(&x_motor, 800);   // 800 steps

pid_t vel_pid;
pid_init(&vel_pid, 1.2f, 0.05f, 0.01f);
float output = pid_update(&vel_pid, setpoint, measured, dt);
```

---

## Nguyên tắc

- Không `#include <zephyr/...>` trực tiếp.
- Không gọi `k_sleep`, `k_thread_create` — dùng `ro_time/ro_thread`.
- Các function gọi trong ISR context phải thread-safe (dùng `ro_queue_put_isr`).
