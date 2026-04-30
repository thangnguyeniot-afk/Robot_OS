# docs/ — Tài liệu Thiết kế Kiến trúc

Thư mục này chứa toàn bộ tài liệu thiết kế chính thức của dự án RobotOS Inspire, được viết theo **thứ tự từ bottom đến top (Layer 1 → Layer 4)**.

---

## Danh sách tài liệu

| File | Layer | Nội dung chính |
|------|-------|----------------|
| [ARCHITECTURE.md](ARCHITECTURE.md) | Tổng quan | Kiến trúc 4-layer, nguyên tắc thiết kế, dependency flow |
| [KERNEL_LAYER.md](KERNEL_LAYER.md) | Layer 1 | Zephyr RTOS — scheduler, priorities, DeviceTree, prj.conf |
| [ADAPTER_LAYER.md](ADAPTER_LAYER.md) | Layer 2 | `ro_*` API — thread, queue, pool, GPIO, PWM, I2C, SPI, log |
| [FRAMEWORK_LAYER.md](FRAMEWORK_LAYER.md) | Layer 3 | Stepper, Servo, PID, Filter, Robot SM |
| [APPLICATION_LAYER.md](APPLICATION_LAYER.md) | Layer 4 | G-code, Motion Planner, App SM, 3-lane architecture |
| [BUILD_SYSTEM.md](BUILD_SYSTEM.md) | Cross-cutting | CMake, West, Kconfig, hybrid host/firmware build |

---

## Thứ tự đọc đề xuất

Nếu bạn mới tiếp cận dự án, đọc theo thứ tự sau:

1. **ARCHITECTURE.md** — hiểu toàn bộ hệ thống trước
2. **KERNEL_LAYER.md** — nền tảng Zephyr
3. **ADAPTER_LAYER.md** — interface giữa OS và framework
4. **FRAMEWORK_LAYER.md** — các primitive robotics
5. **APPLICATION_LAYER.md** — logic ứng dụng thực tế
6. **BUILD_SYSTEM.md** — cách build và deploy

---

## Sơ đồ quan hệ giữa các tài liệu

```
ARCHITECTURE.md (tổng thể)
    ├── KERNEL_LAYER.md     ← Zephyr internals
    ├── ADAPTER_LAYER.md    ← ro_* abstraction API
    ├── FRAMEWORK_LAYER.md  ← robotics primitives
    ├── APPLICATION_LAYER.md← app logic & data flow
    └── BUILD_SYSTEM.md     ← build toolchain
```

---

## Conventions được dùng trong tài liệu

- **`ro_*` prefix** → Adapter layer (Layer 2)  
- **Không prefix** → Framework layer (Layer 3): `stepper_*`, `pid_ctrl_*`, `sm_*`
- **`app_*` prefix** → Application layer (Layer 4)
- **`CONFIG_RO_*`** → Adapter Kconfig keys
- **`CONFIG_APP_*`** → Application Kconfig keys
