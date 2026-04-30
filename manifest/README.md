# manifest/ — Project Context & Requirements

Thư mục này chứa tài liệu **context và yêu cầu** của dự án RobotOS — tài liệu gốc được dùng để thiết kế toàn bộ hệ thống.

---

## Nội dung

| File | Mô tả |
|------|-------|
| [ROBOTOS_CONTEXT.md](ROBOTOS_CONTEXT.md) | Context đầy đủ: mục tiêu dự án, constraints, requirements, design decisions |

---

## Mục đích thư mục

`manifest/` đóng vai trò như **"source of truth"** cho các quyết định thiết kế:

- Giải thích *tại sao* chọn kiến trúc 4-layer
- Ghi lại các constraints quan trọng (embedded, no heap, ISR-safe)
- Định nghĩa các yêu cầu hard/soft realtime
- Danh sách target hardware (STM32F4, ESP32, nRF52, QEMU)
- Milestones và Definition of Done

---

## Liên kết

- Tài liệu thiết kế chi tiết: [../docs/](../docs/README.md)
- Source code: [../RobotOS_v1.0/](../RobotOS_v1.0/README.md)
