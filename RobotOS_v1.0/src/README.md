# src/

Chứa toàn bộ implementation của 3 layer (Adapter, Framework, Application).

```
src/
├── adapter/
│   ├── zephyr/      ← Zephyr RTOS backend (firmware)
│   └── host/        ← PC/host stubs (unit test build)
├── framework/       ← Motor, sensor, control logic
│   └── drivers/     ← Low-level pulse/timing drivers
└── app/             ← Application logic, G-code, motion planning
```

---

## Adapter Layer (`adapter/`)

Hiện thực HAL interface khai báo trong `include/robotos/ro_*.h`.

- **`zephyr/`** — dùng Zephyr kernel API (`k_thread`, `k_msgq`, `gpio_pin_set`, …).
- **`host/`** — dùng POSIX pthread + đồng hồ hệ thống. Cho phép build và test trên PC không cần phần cứng.

Chọn backend qua CMake:
```cmake
# Firmware build (default)
# -DHOST_BUILD=OFF  →  compiles src/adapter/zephyr/

# Host build
# -DHOST_BUILD=ON   →  compiles src/adapter/host/
```

## Framework Layer (`framework/`)

Không phụ thuộc Zephyr. Chỉ gọi Adapter API (`ro_gpio`, `ro_pwm`, `ro_queue`…).

- **`framework/*.c`** — 10 components: stepper, servo, dcmotor, encoder, endstop, sensor, PID, filter, limiter, robot_sm.
- **`framework/drivers/`** — 2 low-level ISR-safe drivers: stepper_drv (Bresenham), endstop_drv (debounce).

## Application Layer (`app/`)

Logic ứng dụng cụ thể. Gồm 8 file: G-code parser, motion planner, kinematics, app state machine, config profiles, glue code, và main entry point.

---

Xem README trong mỗi subdirectory để biết chi tiết từng file.
