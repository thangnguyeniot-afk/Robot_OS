# RobotOS v1.0

---

## Repository Orientation

**Active runtime/evidence track** (Phase 4–7 work, hardware-validated):

- `core/` — portable RobotOS core (event queue, scheduler, dispatcher)
- `platform/` — platform abstraction layer (log, time, fault, critical section)
- `devkit/` — Zephyr bringup application (STM32F411E-DISCO)
- `tests/host/` — host-native contract test suite (20 suites, 358+ cases)

These directories contain the live implementation that runs on hardware.
See [`CURRENT_STATE.md`](../CURRENT_STATE.md) for the latest validated state and
[`devkit/docs/01_PROGRESS/DEVKIT_PROGRESS.md`](devkit/docs/01_PROGRESS/DEVKIT_PROGRESS.md) for full
phase history up to Phase 9E / 9E-Z,
[`devkit/docs/01_PROGRESS/DEVKIT_PROGRESS_PHASE_10.md`](devkit/docs/01_PROGRESS/DEVKIT_PROGRESS_PHASE_10.md)
for Phase 10, and
[`devkit/docs/01_PROGRESS/DEVKIT_PROGRESS_PHASE_11_20.md`](devkit/docs/01_PROGRESS/DEVKIT_PROGRESS_PHASE_11_20.md)
for the Phase 11–20 window (currently `RESERVED / NOT_STARTED`). Progress
files are grouped by 10-phase windows starting at Phase 11; see
[`devkit/docs/01_PROGRESS/README.md`](devkit/docs/01_PROGRESS/README.md) for
the directory policy.

**Historical reference tree** (`src/`, `include/`, `boards/`):
The 4-layer adapter/framework/application architecture described below was the
original design intent and is preserved here as historical context.
It is **not the active runtime path**. Per `platform/README.md`:
"Legacy `RobotOS_v1.0/src/` must never be compiled or included."

---

Firmware điều khiển robot đa trục chạy trên **Zephyr RTOS**, thiết kế theo kiến trúc **4-layer** với khả năng test trên PC (host build).

---

## Cấu trúc thư mục

```
RobotOS_v1.0/
│
├── CMakeLists.txt          ← Root build (Zephyr + Host dual-mode)
├── Kconfig                 ← Toàn bộ CONFIG_RO_* và CONFIG_APP_* keys
├── prj.conf                ← Zephyr project configuration
├── west.yml                ← Zephyr manifest (v3.7.0)
├── IMPLEMENTATION_GUIDE.md ← Hướng dẫn implementation chi tiết từng layer
│
├── include/                ← Public headers (tất cả layer trên dùng)
│   ├── robotos/            ← Adapter + Framework public API
│   └── app/                ← Application-specific types & interfaces
│
├── src/                    ← Source implementations
│   ├── adapter/
│   │   ├── zephyr/         ← Zephyr backend (14 file)
│   │   └── host/           ← Host/PC stubs (15 file)
│   ├── framework/          ← Motor, sensor, PID, filter, SM (10 file)
│   │   └── drivers/        ← Low-level pulse & endstop drivers
│   └── app/                ← Application logic (8 file)
│
├── tests/                  ← Unit tests (host build, ctest)
└── boards/                 ← Board-specific overlays (DeviceTree)
```

---

## Kiến trúc 4-Layer

```
Application  ← G-code parser · Motion planner · App SM · Config profiles
Framework    ← Stepper · Servo · DC Motor · Encoder · PID · Filter · Robot SM
Adapter      ← ro_thread · ro_queue · ro_gpio · ro_pwm · ro_log · ro_deadline
Kernel       ← Zephyr RTOS (k_thread · k_msgq · GPIO driver · Timer)
```

**Quy tắc:**
- Mỗi layer chỉ gọi layer ngay bên dưới
- Adapter che giấu hoàn toàn Zephyr API → swap được bằng host stubs
- Framework không dùng prefix `ro_`; Application dùng prefix `app_`

---

## Build

### Firmware (Zephyr)

```bash
# Thiết lập Zephyr workspace lần đầu
west init -l .
west update

# Build cho Nucleo-F411RE (STM32F4)
west build -b nucleo_f411re .
west flash

# Build cho QEMU (không cần phần cứng)
west build -b qemu_cortex_m3 .
west build -t run
```

### Host (Unit Tests trên PC)

```bash
mkdir build && cd build
cmake -DHOST_BUILD=ON ..
cmake --build .
ctest --output-on-failure -V
```

### Zephyr Shell Commands (sau khi flash)

```
uart:~$ ro gcode G28        # Home all axes
uart:~$ ro gcode G1 X50 Y30 F600
uart:~$ ro status           # Print current state
uart:~$ ro run              # Start running
uart:~$ ro stop             # Stop
```

---

## Cấu hình quan trọng (Kconfig)

| Key | Default | Ý nghĩa |
|-----|---------|---------|
| `CONFIG_APP_CMD_QUEUE_DEPTH` | 64 | Slots trong G-code command queue |
| `CONFIG_APP_MAX_MOTION_SEGMENTS` | 64 | Slots trong motion segment queue |
| `CONFIG_APP_PLANNER_TICK_HZ` | 1000 | Tần suất motion planner (Hz) |
| `CONFIG_APP_DEFAULT_PROFILE` | 0 | Profile mặc định (0=CNC_2AXIS) |
| `CONFIG_RO_MAX_THREADS` | 8 | Số thread tối đa |
| `CONFIG_RO_LOG_RING_SIZE` | 2048 | Ring buffer log (bytes) |

---

## Thread Roster

| Thread | Priority | Stack | Vai trò |
|--------|----------|-------|---------|
| `t_shell` | BACKGROUND (+12) | 1 kB | Nhận input, parse G-code → cmd_q |
| `t_planner` | RT_CONTROL (-10) | 2 kB | cmd_q → motion_seg_t → seg_q |
| `t_pulse_mgr` | RT_PULSE (-16) | 1 kB | seg_q → GPIO step pulses |

---

## Data Flow

```
stdin / UART
    ↓
[t_shell] → gcode_parse_line() → gcode_cmd_t
    ↓ cmd_q (64 slots)
[t_planner] → kinematics + velocity profile → motion_seg_t
    ↓ seg_q (64 slots)
[t_pulse_mgr] → stepper_drv_tick() → GPIO step pulses
```

---

## Machine Profiles

| ID | Tên | Axes | Steps/mm X/Y/Z | Feedrate max |
|----|-----|------|----------------|-------------|
| 0 | CNC 2-Axis | 2 | 80 / 80 / — | 3000 mm/min |
| 1 | CNC 3-Axis | 3 | 80 / 80 / 400 | 3000 mm/min |
| 2 | Printer 2-Axis | 2 | 80 / 80 / — | 6000 mm/min |
| 3 | Printer 3-Axis | 3 | 80 / 80 / 400 | 6000 mm/min |

---

## Tài liệu thêm

- [IMPLEMENTATION_GUIDE.md](IMPLEMENTATION_GUIDE.md) — Hướng dẫn chi tiết từng component
- [../docs/ARCHITECTURE.md](../docs/ARCHITECTURE.md) — Kiến trúc tổng thể
- [../docs/APPLICATION_LAYER.md](../docs/APPLICATION_LAYER.md) — Chi tiết Application layer
- [tests/README.md](tests/README.md) — Hướng dẫn chạy test

---

## Milestones

- **v0.1 Alpha** — 2026-04-30: 3-axis coordinated motion, G0/G1/G28, App SM, host tests
- **v0.2 Beta** — 2026-05-31: Arc interpolation, filesystem, temperature control
