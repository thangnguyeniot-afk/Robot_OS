# RobotOS Inspire — Project Context

> **Cập nhật lần cuối:** 2026-02-27  
> **File này:** Ngữ cảnh dự án để import vào agent/AI assistant

---

## 1️⃣ Project Identity

### Mục tiêu hiện tại
- Xây dựng **RobotOS Inspire** — một robotics-optimized system dựa trên **Zephyr RTOS** (kernel base)
- Tích hợp **robotics middleware** độc lập, nhẹ, deterministic
- Cung cấp **toolchain/CLI + trace + benchmark suite** cho realtime development
- Phát triển **examples** cho 2 robot use-case (v0.1):
  - 3D printer (stepper + motion planner + gcode)
  - Cánh tay robot gắp 3 bậc tự do (control loop + kinematics + trajectory)

### Deadline gần nhất
- **v0.1 Alpha:** 2026-04-30 — middleware + STM32F4 bringup + 1 demo chạy được (inproc, stub)
- **v0.2 Beta:** 2026-05-31 — UART transport (COBS+CRC16) + Flash params + benchmark suite runnable

> **Tổng scope cuối tháng 5:** v0.1 + v0.2 = RobotOS Prototype hoàn chỉnh đầu tiên

### Phase
- [ ] Research
- [x] Build
- [ ] Stabilize
- [ ] Delivery

**Ghi chú:** Hiện đang trong phase **Build** — thiết lập kiến trúc cơ bản và các module chính

---

## 2️⃣ Current State

### % Hoàn thành
**[15%]** ━━━━━━━━━━━━━━━━━━━━

- ✅ Bootstrap spec hoàn thành
- ✅ Build system architecture định nghĩa
- ⏳ Kernel base setup
- ⏳ Middleware framework
- ⏳ Toolchain development
- 📋 Trace system
- 📋 Benchmark suite
- 📋 Example applications

### 3 Milestone tiếp theo

1. **M1: Kernel & Build System Foundation**
   - Setup Zephyr RTOS workspace (west init/update)
   - Cấu hình build system (West + CMake)
   - Tích hợp với toolchain (arm-none-eabi-gcc)
   - Target platform đầu tiên (STM32F4 hoặc QEMU)

2. **M2: Middleware Core**
   - Thiết kế pub/sub interface (zero-copy, no malloc in hot path)
   - Memory pool allocator
   - Timer/scheduler wrapper
   - Trace hooks cơ bản

3. **M3: First Example (3D Printer Stepper)**
   - Stepper motor control
   - Motion planner đơn giản
   - GCode parser tối thiểu
   - Verify determinism & latency

### Blockers

| # | Blocker | Severity | Workaround |
|---|---------|----------|------------|
| 1 | [Cập nhật blocker nếu có] | - | - |
| 2 | [Cập nhật blocker nếu có] | - | - |

---

## 3️⃣ Resource Model

### Bạn làm gì
- **Architecture & Design:** Thiết kế tổng thể middleware, API design
- **Core Implementation:** Kernel integration, build system, middleware core
- **Code Review:** Review tất cả critical path code
- **Documentation:** Duy trì spec, context, và technical docs
- **Technical Decision:** Quyết định kỹ thuật quan trọng

### Delegate làm gì
- **[Nếu có team member]** Implementation của các module cụ thể
- **[Nếu có team member]** Testing & verification
- **[Nếu có team member]** Example application development
- **[AI Assistant]** Code generation, boilerplate, refactoring suggestions

### Weekly Load dự kiến (blocks)

**Tuần này:**
- [ ] **10 blocks** — Kernel setup & build system (40h)
- [ ] **5 blocks** — Middleware design (20h)
- [ ] **2 blocks** — Documentation (8h)
- [ ] **1 block** — Planning & review (4h)

**Ghi chú:** 1 block = ~4 giờ làm việc tập trung

---

## 4️⃣ Risk Snapshot

### 1–2 Risk chính

| Risk | Impact | Mitigation |
|------|--------|------------|
| **R1: Zephyr Device Tree learning curve** — DTS overlay + Kconfig phức tạp hơn dự kiến với board mới | 🟡 MEDIUM | - Ưu tiên board đã có Zephyr support (STM32F429)<br>- Dùng Robot Adapter Layer để che giấu Zephyr API<br>- Tham khảo Zephyr sample apps |
| **R2: Determinism verification** — Khó đảm bảo và đo lường tính deterministic của middleware | 🟡 MEDIUM | - Implement trace system sớm<br>- Setup benchmark suite từ đầu<br>- Định nghĩa rõ "determinism criteria" |

### Scope đã freeze chưa?

**❌ CHƯA FREEZE**

**Scope hiện tại cho v0.1:**
- ✅ **IN SCOPE:** Zephyr RTOS base, Robot Adapter Layer, Robot Framework (Stepper/Servo/PID), trace hooks, 2 examples (printer + arm)
- ✅ **IN SCOPE:** Build system, basic toolchain
- ⚠️ **NEEDS DECISION:** Mức độ tích hợp toolchain (custom CLI vs. standard Make)
- ❌ **OUT OF SCOPE:** Multi-core support (defer to v0.2+)
- ❌ **OUT OF SCOPE:** Network stack (defer to v0.2+)
- ❌ **OUT OF SCOPE:** File system (defer to v0.2+)

**Dự kiến freeze scope:** [Cập nhật ngày freeze]

---

## 📝 Notes & Context

### Nguyên tắc thiết kế "đinh"
1. **Determinism-by-default:** Không malloc/free trong realtime path
2. **Không "magic":** Middleware không tự tạo thread ngầm
3. **Observability là tính năng sản phẩm:** Trace timeline, missed deadline detection
4. **Scope kỷ luật:** RobotOS = distribution/profile, KHÔNG fork OS toàn diện

### Tech Stack
- **Kernel:** Zephyr RTOS
- **Build System:** West + CMake
- **Toolchain:** arm-none-eabi-gcc
- **Target HW:** STM32F4, ESP32, nRF52, QEMU (cho v0.1)
- **Languages:** C (core), Python (tooling)

### Links
- [Bootstrap Spec](../ROBOTOS_BOOTSTRAP.md)
- [Build System Design](../docs/BUILD_SYSTEM.md)

---

**🤖 Agent Instructions:**
- File này cung cấp ngữ cảnh dự án cho AI assistant
- Cập nhật file này khi có thay đổi lớn về scope, milestone, hoặc risks
- Tham khảo Bootstrap Spec để biết chi tiết kỹ thuật
- Tuân thủ nguyên tắc thiết kế khi generate code
