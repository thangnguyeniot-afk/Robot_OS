# Robot_OS — Workspace Root

> Thư mục gốc của toàn bộ dự án **RobotOS Inspire** — hệ thống điều khiển robot 4-layer chạy trên Zephyr RTOS.

---

## Cấu trúc workspace

```
Robot_OS/
├── RobotOS_v1.0/          ← Source code chính (xem README bên trong)
├── docs/                  ← Tài liệu thiết kế kiến trúc (6 file .md)
├── manifest/              ← Context & requirements của dự án
└── ROBOTOS_BOOTSTRAP.md   ← Hướng dẫn khởi động nhanh (bootstrap)
```

---

## Tài liệu quan trọng

| File / Thư mục | Mô tả |
|----------------|-------|
| [ROBOTOS_BOOTSTRAP.md](ROBOTOS_BOOTSTRAP.md) | Điểm khởi đầu — tổng quan, cách setup |
| [docs/](docs/README.md) | Tất cả tài liệu thiết kế (Architecture → Application Layer) |
| [manifest/ROBOTOS_CONTEXT.md](manifest/ROBOTOS_CONTEXT.md) | Context, yêu cầu, constraints của dự án |
| [RobotOS_v1.0/](RobotOS_v1.0/README.md) | Source code đầy đủ v1.0 |

---

## Kiến trúc tóm tắt

```
┌────────────────────────────────┐
│  Layer 4: APPLICATION          │  G-code • Motion Planner • App SM
├────────────────────────────────┤
│  Layer 3: FRAMEWORK            │  Stepper • Servo • PID • Filter
├────────────────────────────────┤
│  Layer 2: ADAPTER              │  ro_thread • ro_queue • ro_gpio …
├────────────────────────────────┤
│  Layer 1: KERNEL (Zephyr RTOS) │  k_thread • k_msgq • GPIO driver
└────────────────────────────────┘
```

Chi tiết kiến trúc: [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md)

---

## Quick Start

```bash
# Clone và khởi tạo Zephyr workspace
cd RobotOS_v1.0
west init -l .
west update

# Build firmware (STM32 Nucleo-F411RE)
west build -b nucleo_f411re .
west flash

# Hoặc build host (unit tests trên PC)
mkdir build && cd build
cmake -DHOST_BUILD=ON ..
cmake --build .
ctest --output-on-failure
```

Xem thêm: [RobotOS_v1.0/README.md](RobotOS_v1.0/README.md)

---

## Milestones

| Version | Deadline | Trạng thái |
|---------|----------|-----------|
| v0.1 Alpha | 2026-04-30 | 🔄 In progress |
| v0.2 Beta  | 2026-05-31 | ⏳ Planned |
