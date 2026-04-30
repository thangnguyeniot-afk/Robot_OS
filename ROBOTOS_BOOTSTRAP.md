# RobotOS Inspire — Project Bootstrap Spec (LiteOS‑M Distribution/Profile + Middleware + Toolchain)
> ⚠️ **DEPRECATION NOTICE (2026-03-03):** Tiêu đề và triết lý "LiteOS-M kernel base" trong file này đã lỗi thời. Project đã chuyển sang **Zephyr RTOS** từ tháng 02/2026. Các nguyên tắc thiết kế (determinism, no magic threads, no heap after init, observability) vẫn còn hiệu lực. Để biết kiến trúc hiện tại, xem [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md).
> **Mục tiêu của file này:** làm “nguồn yêu cầu duy nhất” (single source of truth) để GitHub Copilot/Copilot Chat tạo **khung dự án ban đầu**.
>
> **Triết lý:** RobotOS = *LiteOS‑M (kernel base) + distribution/profile + robotics middleware độc lập + toolchain + trace + benchmark + examples*.

---

## 0) Tóm tắt ý tưởng

**RobotOS Inspire** là một **distribution/profile** dựa trên **OpenHarmony LiteOS‑M** (kernel base), cộng thêm:

- **Robotics middleware độc lập** (nhẹ, deterministic, không “giấu” luồng).
- **Toolchain/CLI + Trace + Benchmark suite** cho việc đo realtime dễ tái lập.
- **Examples** cho 2 đối tượng robot (mục tiêu v0.1):
  - 3D printer (stepper + motion planner + gcode tối thiểu)
  - Cánh tay robot gắp 3 bậc tự do (control loop + kinematics + trajectory)

**Mục tiêu v1** (sau v0.1/v0.2):
- Giúp lập trình robotics **dễ hơn RTOS thuần**.
- Có khả năng **đo/kiểm soát realtime** tốt hơn các framework nặng.
- Vẫn cho phép dev **đi xuống tầng RTOS** (task/priority/memory/IPC).

---

## 1) Nguyên tắc thiết kế “đinh”

### 1.1 Determinism-by-default
- **Không** cấp phát động (malloc/free) trong “realtime path”:
  - timer callback
  - control loop
  - publish hot path
- Dùng **memory pool / fixed block allocator**.
- Ưu tiên **zero-copy** (loaned buffer) nếu có thể.

### 1.2 Không “magic”
- Middleware **không tự tạo thread ngầm**.
- Mapping rõ ràng: callback chạy trong task nào? priority nào? stack nào?

### 1.3 Observability là tính năng sản phẩm
- Trace timeline (context switch, IRQ enter/exit, pub/sub latency, missed deadline).
- Xuất trace dễ xem (Chrome Trace JSON hoặc format tương đương).

### 1.4 Scope kỷ luật
- Kernel patch budget nhỏ: chỉ sửa khi thật cần, ưu tiên module/feature-flag.
- “RobotOS = distribution/profile” chứ **không fork** OS toàn diện.

---

## 2) Target phần cứng & kế hoạch hỗ trợ

### 2.1 MCU mục tiêu ban đầu
- **STM32F4** (ưu tiên: STM32F429 / STM32F407)
- **ESP32** (classic Xtensa)

### 2.2 Chiến lược porting
- Tách rõ:
  - `platforms/<mcu>/soc/` (clock, startup, interrupt, linker)
  - `platforms/<mcu>/board/` (pinmap, peripheral instances)
- Middleware + libs phải portable 100% (C thuần), không phụ thuộc board.

---

## 3) Repo layout (Copilot hãy tạo đúng cấu trúc này)

Tạo repository monorepo `robotos-inspire/` với cấu trúc:

```
robotos-inspire/
  README.md
  ROBOTOS_BOOTSTRAP.md                # file spec này
  LICENSE
  NOTICE
  .gitmodules                         # nếu dùng submodule
  .editorconfig
  .clang-format

  .github/
    workflows/
      ci.yml
    CODEOWNERS
    pull_request_template.md
    ISSUE_TEMPLATE/
      bug_report.md
      feature_request.md

  third_party/
    kernel_liteos_m/                  # submodule/subtree từ OpenHarmony
    device_qemu/                      # optional: submodule để CI/QEMU
    unity/                            # optional: unit test framework nhỏ

  profiles/
    robotos_mcu_minimal/
      BUILD.gn
      args.gn
      Kconfig                         # nếu dùng
      profile.md

  platforms/
    stm32f4/
      soc/
        startup/
        linker/
        hal/
      board/
        <board_name>/
      BUILD.gn
      platform.md
    esp32/
      soc/
      board/
      BUILD.gn
      platform.md

  middleware/
    ro_core/
      include/robotos/
        ro_config.h
        ro_node.h
        ro_pubsub.h
        ro_timer.h
        ro_executor.h
        ro_param.h
        ro_status.h
        ro_memory.h
      src/
        ro_node.c
        ro_pubsub.c
        ro_timer.c
        ro_executor.c
        ro_param.c
        ro_memory_pool.c
      BUILD.gn
      README.md

    ro_transport/
      include/robotos/
        ro_transport.h
        ro_transport_inproc.h
        ro_transport_uart.h
      src/
        ro_transport.c
        ro_transport_inproc.c
        ro_transport_uart.c
      BUILD.gn
      README.md

    ro_trace/
      include/robotos/
        ro_trace.h
      src/
        ro_trace.c
      BUILD.gn
      README.md

  libs/
    ro_control/
      include/robotos/
        ro_pid.h
        ro_filter.h
      src/
        ro_pid.c
        ro_filter.c
      BUILD.gn
      README.md

    ro_motion/
      include/robotos/
        ro_stepgen.h
        ro_motion_planner.h
      src/
        ro_stepgen_stub.c              # v0.1 stub, HW-specific nằm ở platforms
        ro_motion_planner.c
      BUILD.gn
      README.md

    ro_kinematics/
      include/robotos/
        ro_fk_ik_3dof.h
        ro_trajectory.h
      src/
        ro_fk_ik_3dof.c
        ro_trajectory.c
      BUILD.gn
      README.md

  bench/
    bench_sched/
      main.c
      bench_sched.md
      BUILD.gn
    bench_ipc/
      main.c
      bench_ipc.md
      BUILD.gn
    bench_mem/
      main.c
      bench_mem.md
      BUILD.gn
    bench_ctrl/
      main.c
      bench_ctrl.md
      BUILD.gn

  examples/
    printer_demo/
      main.c
      config/
      README.md
      BUILD.gn
    arm3dof_demo/
      main.c
      config/
      README.md
      BUILD.gn

  tools/
    ro_cli/
      ro.py                            # CLI entry
      commands/
        build.py
        flash.py
        monitor.py
        trace.py
        bench.py
      README.md

    trace_tools/
      trace_to_chrome.py
      README.md

    host_tests/
      CMakeLists.txt                   # test middleware trên host
      test_pubsub.c
      test_memory_pool.c
      README.md

  docs/
    00_quickstart.md
    01_architecture.md
    02_middleware_api.md
    03_realtime_rules.md
    04_trace_and_benchmark.md
    05_contributing.md
    06_kernel_patch_policy.md

  TODO.md
```

**Copilot NOTE**
- Các file `BUILD.gn` là placeholder để nối GN/Ninja (LiteOS‑M build).  
- `tools/host_tests/` dùng CMake để test logic middleware độc lập (không cần target MCU).

---

## 4) Build system strategy — "2 tầng build" (CHỐT QUAN TRỌNG)

### 4.1 Nguyên tắc "CMake cho dev + GN cho integration"

**Vấn đề**: LiteOS-M dùng GN/Ninja (learning curve cao) → sinh viên khó đóng góp.

**Giải pháp**: Dual build system:

```
CMake (Development tier):
├─ Middleware/libs development
├─ Host unit tests (PC: Linux/macOS/Windows)
├─ Fast iteration (< 10s build+test)
└─ CI: test mọi PR

GN/Ninja (Integration tier):
├─ Firmware image assembly
├─ Kernel + profile linking
├─ Cross-compile cho MCU
└─ CLI wrapper: ro build/flash
```

**Quy tắc**:
- Contributor **chỉ edit CMakeLists.txt**
- BUILD.gn được auto-sync (hoặc CI check)
- CLI `ro build` che giấu GN complexity

### 4.2 GN wrapper template (`build/ro_component.gni`)

Tạo template đơn giản hóa 90% syntax:

```gn
# Thay vì syntax phức tạp, module chỉ cần:
ro_static_lib("ro_core") {
  sources = [ "src/ro_node.c", "src/ro_pubsub.c" ]
  public = [ "include/robotos/ro_node.h" ]
  deps = [ ":kernel" ]
}
```

### 4.3 Sync automation

Tạo `tools/sync_build.py`:
- Parse CMakeLists.txt → generate BUILD.gn
- CI check: CMake sources == GN sources
- Prevent manual sync errors

**Docs**: `docs/BUILD_SYSTEM.md` (chi tiết workflow, troubleshooting, contributor guide)

---

## 5) Quản lý upstream kernel

### 5.1 Submodule/subtree
- `third_party/kernel_liteos_m` lấy từ OpenHarmony.
- Option: `third_party/device_qemu` để chạy QEMU/CI.

Trong `README.md`, hướng dẫn:
- `git submodule update --init --recursive`

### 5.2 Kernel patch policy — Dùng trace module + hook framework sẵn có

**Nguyên tắc CHỐT**: Không patch scheduler ở v0.1/v0.2 chỉ để trace context switch.

**Thay vào đó**:
1. **Dùng LiteOS-M trace module sẵn có**:
   - `components/trace/` với APIs: `LOS_TraceStart/Stop/RecordDump`
   - Hook framework: `utils/los_hook.*`
   - Trace events: task switch, IRQ enter/exit, syscalls

2. **Portable abstraction layer** (`ro_trace.h`):
   ```c
   #ifdef ROBOTOS_TARGET_LITEOS_M
     #define RO_TRACE_EVENT(name, arg) \
       LOS_TRACE_EASY(LOS_TRACE_MODULE_USER, name, arg)
   #else
     #define RO_TRACE_EVENT(name, arg) /* no-op on host */
   #endif
   ```
   - Middleware code không dính kernel
   - Host tests vẫn compile

3. **Chỉ patch khi thiếu critical event**:
   - Tiêu chí: benchmark overhead < 5%
   - Phải có compile-time flag
   - Phải có docs + justification

**Docs**: `docs/06_kernel_patch_policy.md` (tiêu chí patch, approval process)

---

## 6) Middleware v0.1 — API contracts (Copilot hãy tạo header + stub implementation)

### 5.1 Status codes
Tạo `middleware/ro_core/include/robotos/ro_status.h`:

- `RO_OK`
- `RO_EINVAL`
- `RO_ENOMEM`
- `RO_ESTATE`
- `RO_ETIMEOUT`
- `RO_EUNSUPPORTED`

### 5.2 Memory model (cốt lõi)
Tạo `ro_memory.h` + `ro_memory_pool.c`:

- Pool fixed-block:
  - init với buffer do user cấp
  - allocate/free O(1)
  - thống kê watermark
  - optional: thread-safe bằng spinlock/mutex

**Rule:** realtime path chỉ được dùng allocator này (không malloc).

### 5.3 Node + Executor (single-thread default)
Tạo:
- `ro_node_create(name)`
- `ro_executor_create(node, cfg)` với:
  - priority
  - stack size
  - spin strategy (poll vs event) — v0.1 có thể stub
- `ro_executor_spin(node)` chạy trong **1 task** (single-thread).

**Quan trọng:** V0.1 chọn **một** mô hình rõ ràng:
- (A) executor tự tạo task, hoặc
- (B) user tự tạo task và gọi spin trong task đó.
=> Chốt 1 cách và ghi rõ trong docs.

### 5.4 Timer
- `ro_timer_create(period_us, callback, user)`
- callback chạy trong executor task (không chạy ISR).

### 5.5 Pub/Sub (inproc transport v0.1)
- `ro_pub_create(topic, msg_size, qos, pool)`
- `ro_sub_create(topic, msg_size, qos, queue_depth, callback)`
- `ro_publish(pub, msg_ptr, len)`
- `ro_take(sub, out_buf, len)` (optional)

QoS v0.1 tối thiểu:
- `RO_QOS_BEST_EFFORT`
- `RO_QOS_RELIABLE` (stub v0.1 được, implement dần)

**Không yêu cầu IDL/codegen ở v0.1**: message là blob + size, dev tự quản struct.

### 5.6 Params
- key-value string -> int/float/bool/string
- backing store: stub (RAM). Flash persistence là v0.2.

---

## 7) Trace v0.1 — Dùng LiteOS-M trace + middleware events (Copilot hãy tạo integration)

### 7.1 Architecture

```
┌─────────────────────────────────────────┐
│  Middleware Events (ro_trace.h)         │
│  • timer_cb_enter/exit                  │
│  • pub_begin/end                        │
│  • sub_cb_begin/end                     │
│  • missed_deadline                      │
└──────────────┬──────────────────────────┘
               │ (via portable API)
               ↓
┌─────────────────────────────────────────┐
│  LiteOS-M Trace Module                  │
│  • Task switch (built-in)               │
│  • IRQ enter/exit (built-in)            │
│  • Syscalls (built-in)                  │
└──────────────┬──────────────────────────┘
   8           │
               ↓
        [Ring Buffer]
               │
               ↓ (dump via UART)
        [Host Tool: merge_traces.py]
               │
               ↓
       [Chrome Trace JSON]
```

### 7.2 Implementation

**Tạo `ro_trace.h/.c`**:
- `RO_TRACE_EVENT(name, arg0, arg1)` → map xuống `LOS_TRACE_EASY()`
- Backend: LiteOS-M trace module (không tự implement ring buffer)
- Dump: `LOS_TraceRecordDump()` qua shell/UART

**Tạo tool `tools/trace_tools/merge_traces.py`**:
- Input: kernel trace + middleware trace logs
- O9tput: Chrome Trace JSON (unified timeline)

**Tạo tool `tools/trace_tools/trace_to_chrome.py`**:
- Convert merged trace → Chrome `chrome://tracing` format
- Timeline viewer với zoom/filter

**Event coverage v0.1**:
```
From kernel (built-in):
✅ Task switch
✅ IRQ enter/exit
✅ Mutex lock/unlock

From middleware (add via ro_trace):
✅ timer_cb_enter/exit
✅ pub_begin/end
✅ sub_cb_begin/end
✅ missed_deadline
```

**Benchmark requirement**:
- Trace overhead < 5% CPU @ 1kHz event rate
- Buffer size: 4KB (tunable)
- Dump time: < 100ms

---

## 7) Benchmark suite (Copilot tạo skeleton runnable)

### 7.1 bench_sched
- đo timer jitter của periodic callback 1kHz / 2kHz
- output CSV: `min, avg, p95, p99, max`

### 7.2 bench_ipc
- publish->subscribe latency với msg sizes: 16B, 64B, 256B, 1KB
- đo drop rate khi queue_depth nhỏ

### 7.3 bench_mem
- alloc/free time, watermark
- test “no alloc in realtime path” (assert/guard)

### 7.4 bench_ctrl
- loop PID giả lập 1kHz (plant mô phỏng nhẹ)
- đo jitter + thời gian compute

---

## 8) Examples v0.1 (Copilot tạo runnable demo)

### 8.1 printer_demo
Pipeline rõ ràng:
- Task high-prio: stepgen (v0.1 stub)
- Task mid: motion planner tạo segment -> publish ring
- Task low: gcode parser (stub)

### 8.2 arm3dof_demo
- Timer 1kHz: control loop (PID)
- 200Hz: sensor update (stub)
- 50Hz: command update
- libs: `ro_fk_ik_3dof` + `ro_trajectory`

---

## 10) Tooling (Copilot tạo CLI stub)

Tạo `tools/ro_cli/ro.py` (Python):
- `ro build --platform stm32f4 --profile robotos_mcu_minimal`
- `ro flash --platform ...` (stub: in ra hướng dẫn OpenOCD/esptool)
- `ro monitor` (stub: mở serial)
- `ro trace` (collect + convert)
- `ro bench` (chạy bench target/host)

---

## 11) Host tests (Copilot tạo unit tests chạy trên PC)

Trong `tools/host_tests/`:
- Test pub/sub logic (inproc)
- Test memory pool correctness + watermark
- Chạy bằng CMake + `ctest`

CI sẽ chạy host tests luôn.

---

## 12) CI — CMake + GN dual workflow (Copilot tạo GitHub Actions)

File `.github/workflows/ci.yml` với 3 jobs:

**Job 1: Host tests (CMake)** - Chạy mọi PR
- Matrix: ubuntu-latest, macos-latest, windows-latest
- Build: `cmake -B build && cmake --build build`
- Test: `ctest --output-on-failure`
- Fast: < 5 phút
## 13) Docs tối thiểu phải có (Copilot tạo placeholder)

- `docs/00_quickstart.md`: clone + init submodules + build host tests
- `docs/01_architecture.md`: sơ đồ layer (kernel / middleware / libs / tools)
- `docs/02_middleware_api.md`: API v0.1 + mapping xuống RTOS
- `docs/03_realtime_rules.md`: "no malloc", priority rules, ISR rules
- `docs/04_trace_and_benchmark.md`: cách chạy trace/bench + LiteOS-M trace integration
- `docs/05_contributing.md`: coding style, PR checklist, labels, "contributor chỉ cần biết CMake"
- `docs/06_kernel_patch_policy.md`: tiêu chí patch, dùng hook/trace module trước khi patch
- **`docs/BUILD_SYSTEM.md`**: ⭐ Chi tiết về CMake vs GN, workflow, troubleshooting (ĐÃ TẠO)
- Install ARM GCC toolchain
- Bu4ld: `python build.py --platform stm32f4 --profile robotos_mcu_minimal`
- Artifact: upload `robotos.elf` + size report
4.1 PR checklist (`pull_request_template.md`)
- [ ] Có test/bench tương ứng (host tests pass)
- [ ] Không malloc trong realtime path
- [ ] Có doc update nếu API thay đổi
- [ ] Có kết quả benchmark trước/sau (nếu chạm scheduling/IPC/memory)
- [ ] **Build sync check pass**: CMakeLists.txt ↔ BUILD.gn (CI auto-verif
4
---

## 12) Docs tối thiểu phải có (Copilot tạo placeholder)

- `docs/00_quickstart.md`: clone + init submodules + build host tests
- `docs/01_architecture.md`: sơ đồ layer (kernel / middleware / libs / tools)
- `docs/02_middleware_api.md`: API v0.1 + mapping xuống RTOS
- `docs/03_realtime_rules.md`: “no malloc”, priority rules, ISR rules
- `docs/04_trace_and_benchmark.md`: cách chạy trace/bench + format output
- `docs/05_contributing.md`: coding style, PR checklist, labels

---

## 13) Contribution model (Copilot tạo template)

### 13.1 PR checklist (`pull_request_template.md`)
- [ ] Có test/bench tương ứng
- [ ] Không malloc trong realtime path
- [ ] Có doc update nếu API thay đổi
- [ ] Có kết quả benchmark trước/sau (nếu chạm scheduling/IPC/memory)

### 13.2 CODEOWNERS
- middleware owners
- platform owners
- tools owners
5
---

## 14) Licensing/NOTICE (Copilot hãy chuẩn bị file và ghi chú)

- Code mới của RobotOS: chọn 1 license rõ ràng (ví dụ Apache-2.0 hoặc MIT).
- `third_party/kernel_liteos_m` giữ nguyên license upstream.
- `NOTICE` tổng hợp attribution cho `third_party/`.
6
---

## 15) README.md (Copilot hãy viết README v0.1)

README cần có:
- What/Why (pain/solution)
- Supported platforms: STM32F4, ESP32 (v0.1)
- Quickstart:
  - init submodules
  - chạy host tests
  - build profile (stub)
- Links đến docs
- Roadmap milestones (M0..M4)
7) R7) Roadmap milestones — Điều chỉnh sau phản biện

**Phase 1: Foundation (Week 1-8)** - Mục tiêu: 1 demo chạy trên hardware

- **Week 1-2: Infrastructure**
  - [ ] Repo structure + CMakeLists.txt root
  - [ ] BUILD.gn placeholders + `ro_component.gni` template
  - [ ] `tools/sync_build.py` script
  - [ ] CI: CMake host tests (Ubuntu/macOS/Windows)
  - [ ] `ro_trace.h` với LiteOS-M backend + host stub
8
- **Week 3-4: Core middleware (host tests)**
  - [ ] `ro_memory_pool` + tests
  - [ ] `ro_node` + `ro_executor` (stub)
  - [ ] `ro_timer` (software timer list)
  - [ ] `ro_pubsub` (inproc, single-threaded)
  - [ ] Trace integration (middleware events)

- **Week 5-6: Platform bringup (STM32F4)**
  - [ ] Boot + LiteOS-M minimal config
  - [ ] Logging qua UART
  - [ ] Integrate middleware vào target
  - [ ] 1 periodic task (1kHz timer)
  - [ ] Trace dump qua UART
99) Definition of Done cho scaffold ban đầu

Scaffold được coi là xong khi:
1) Repo có đúng cấu trúc thư mục + file placeholder
2) **CMake build working**: Host unit tests build & pass trên Ubuntu/macOS
3) **GN placeholders**: BUILD.gn files tồn tại (chưa cần build thật)
4) **Sync script**: `tools/sync_build.py --check` runs without error
5) **CI**: GitHub Actions chạy CMake tests + build sync check
6) `ro.py` chạy được (in help/commands)
7) Examples/bench có `main.c` compile được (host stub)
8) **Docs**: BUILD_SYSTEM.md + quickstart có nội dung đầy đủ 9-12)**
20) TODO (Copilot hãy tạo `TODO.md`)

Danh sách TODO (v0.1 → v0.2/v1):

**Infrastructure (Week 1-2)**:
- [ ] `build/ro_component.gni` template implementation
- [ ] `tools/sync_build.py` script (parse CMake → generate GN)
- [ ] CI: CMake matrix (Ubuntu/macOS/Windows)
- [ ] CI: Build sync verification job

**Core (Week 3-4)**:
- [ ] `ro_trace.h` với LiteOS-M `LOS_TRACE_*` backend
---

# PHẢN BIỆN & ĐIỀU CHỈNH CHIẾN LƯỢC (Ngày 24/12/2025)

## A) Build system complexity — CHỐT "2 tầng build"

**Quyết định**: CMake cho dev/CI + GN cho firmware, với sync automation.

**Lý do**: 
- GN learning curve cao → cô lập khỏi contributors
- Maintainer xử lý GN integration
- CLI wrapper che giấu complexity

**Implementation**:
- `build/ro_component.gni`: Template đơn giản hóa 90% syntax
- `tools/sync_build.py`: Auto-sync hoặc CI verify
- `docs/BUILD_SYSTEM.md`: Chi tiết workflow ⭐

**Impact**: +2 tuần setup, -4 tuần integration → net gain 2 tuần

## B) Kernel patch policy — CHỐT "dùng LiteOS-M trace module + hook framework"

**Quyết định**: Không patch scheduler ở v0.1/v0.2.

**Lý do**:
- LiteOS-M có `components/trace/` + `utils/los_hook.*` sẵn
- APIs: `LOS_TraceStart/Stop/RecordDump`
- Task switch, IRQ events đã có

**Implementation**:
- `ro_trace.h`: Portable abstraction layer
- Map xuống LiteOS-M trace khi build target
- Host tests: no-op stub
- Middleware events (pub/sub/timer) hook riêng

**Criteria để patch sau này**:
- Missing critical event (không có cách khác)
- Benchmark overhead < 5%
- Compile-time flag
- Docs + justification

## C) UART transport — CHỐT "COBS + CRC16"

**Quyết định**: v0.2 dùng COBS framing + CRC16-CCITT.

**Lý do**:
- COBS: self-sync với 0x00 delimiter
- Overhead thấp (~0.4%)
- Mature, dùng trong SLIP/PPP

**Frame format** (optimized 8 bytes header):
```c
struct {
  uint16_t sync;      // 'RO'
  uint8_t  flags;     // [version:3][msg_type:5]
  uint8_t  topic_id;
  uint16_t seq;
  uint16_t len;
  // payload[len]
  // crc16
} __packed;
```

**Testing**:
- Fuzzing: random corruption → must resync
- Throughput: > 90% theoretical
- Latency: p99 < 10ms @ 115200 bps

---

# KẾT THÚC SPEC v1.1 (Updated với phản biện)
> Hãy dùng file này làm "single source of truth" để tạo khung dự án RobotOS Inspire v0.1.
>
> **Thay đổi chính**:
> - Build system: CMake (dev) + GN (integration) với sync automation
> - Trace: Dùng LiteOS-M module sẵn có, không patch kernel sớm
> - UART: COBS + CRC16 với spec rõ ràng
> - Docs: BUILD_SYSTEM.md chi tiết (đã tạo)
> - Roadmap: Điều chỉnh timeline theo phản biện
- [ ] Pub/sub với zero-copy loaned buffer (optional v0.1)

**Platform (Week 5-6)**:
- [ ] STM32F4 boot sequence + LiteOS-M config
- [ ] UART trace dump implementation
- [ ] GN firmware build working (không chỉ placeholder)

**Transport (Week 9-10, v0.2)**:
- [ ] COBS + CRC16 implementation
- [ ] UART transport với DMA
- [ ] Fuzzing test suite
- [ ] Latency benchmark: p95/p99 < target

**Future (v0.3+)**:
- [ ] Reliable QoS (seq/ack/retry)
- [ ] Stepgen bằng hardware timer (STM32F4)
- [ ] ESP32 port validation
- [ ] QEMU CI integration
  - [ ] `printer_demo` (stepper stub)
  - [ ] Docs hoàn chỉnh (BUILD_SYSTEM.md đã có)
  - [ ] Blog post: "Why RobotOS" + comparison benchmark
  - [ ] YouTube tutorial: walkthrough + demo

**Phase 3: Community & Ecosystem (3-6 tháng sau v0.1)**

- [ ] ESP32 port
- [ ] Sensor libs (IMU, encoder)
- [ ] PlatformIO plugin
- [ ] University partnerships
- [ ] API v1 stabilizatio
- **M2**: UART transport + trace export + bench suite runnable
- **M3**: 2 demos (printer, arm3dof) có benchmark
- **M4**: ổn định API v1 + tài liệu hoá + contribution workflow cho sinh viên

---

## 17) Coding rules (Copilot hãy áp dụng ngay)

- C language, portable, ưu tiên C99.
- Không dùng dynamic allocation trong realtime code (enforce bằng macro/guard).
- Mọi module phải có `README.md` mô tả:
  - mục tiêu
  - API surface
  - realtime constraints
  - benchmark liên quan

---

## 18) Definition of Done cho scaffold ban đầu

Scaffold được coi là xong khi:
1) Repo có đúng cấu trúc thư mục + file placeholder
2) Host unit tests build & pass (CMake)
3) `ro.py` chạy được (in help/commands)
4) Examples/bench có `main.c` compile được (host hoặc stub)
5) Docs tối thiểu có nội dung khung

---

## 19) TODO (Copilot hãy tạo `TODO.md`)

Danh sách TODO (v0.1 → v0.2/v1):
- [ ] Kết nối GN build với LiteOS‑M thật
- [ ] Implement reliable QoS
- [ ] Implement UART transport thật
- [ ] Hook trace vào kernel context switch (nếu cần)
- [ ] Stepgen bằng hardware timer (STM32F4)
- [ ] ESP32 port validation + CI QEMU (nếu dùng)

---

# KẾT THÚC SPEC
> Hãy dùng file này làm “single source of truth” để tạo khung dự án RobotOS Inspire v0.1.
