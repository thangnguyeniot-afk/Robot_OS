# src/adapter/ — Adapter Layer

> Cầu nối duy nhất giữa Zephyr RTOS kernel và các layer bên trên (Framework, Application).
> Mọi `#include <zephyr/*>` chỉ tồn tại trong thư mục này.

---

## 1. Bài Toán Cần Giải

Framework và Application cần sử dụng các nguyên thủy OS (thread, queue, timer, memory…)
nhưng **không được phụ thuộc trực tiếp vào Zephyr**. Lý do:

- **Portability:** Nếu cần đổi kernel (FreeRTOS, bare-metal…), chỉ thay Adapter — không đụng Framework hay App.
- **Testability:** Host build (`cmake -DHOST_BUILD=ON`) chạy unit test trên PC mà không cần phần cứng.
- **Boundary enforcement:** CI kiểm tra `#include <zephyr/*>` tuyệt đối không xuất hiện trong `include/robotos/*.h` hay `app/**`.

---

## 2. Ý Tưởng Thiết Kế

```
Application / Framework
       │  gọi ro_thread_create(), ro_queue_send(), ...
       ▼
┌──────────────────────────────────┐
│         ADAPTER LAYER            │   ← src/adapter/zephyr/*.c
│  Thin translation layer          │
│  ro_xxx_t  →  k_xxx (Zephyr)    │
│  ro_xxx_t  →  pthread (Host)     │
└──────────────────────────────────┘
       │
       ▼
  Zephyr kernel / POSIX / Win32
```

**Nguyên tắc cốt lõi:**
1. **Chỉ translate — không thêm logic robotics.** Adapter không biết "stepper" hay "PID" là gì.
2. **Opaque struct:** caller chỉ thấy `ro_thread_t*`, không thấy `k_thread` bên trong.
3. **ISR-safe variants có suffix `_isr`** — tách biệt hoàn toàn calling context.
4. **No heap after init** — mọi allocation đều là static pool hoặc static array.

---

## 3. Backend

| Thư mục | Dùng khi | Runtime |
|---------|----------|---------|
| [`zephyr/`](zephyr/README.md) | `west build` (firmware) | Zephyr RTOS |
| [`host/`](host/README.md) | `cmake -DHOST_BUILD=ON` (PC) | POSIX pthreads / Win32 |

Cùng một `ro_*.h` header — hai implementation hoàn toàn độc lập.

---

## 4. Giải Pháp Implement — Từng Module

### 4.1 `ro_thread` — Thread Management
**Bài toán:** Cần tạo/destroy thread portable qua Zephyr + host.

**Giải pháp:** *Static slot pool* thay vì dynamic allocation.
```
s_slots[CONFIG_RO_MAX_THREADS]   ← array tĩnh, cố định lúc biên dịch
  bitmap in_use[]                ← theo dõi slot nào đang dùng
  slot_alloc()                   ← O(N) scan, cực nhanh với N nhỏ (8)
  ro_thread_create()             → k_thread_create() / CreateThread() / pthread_create()
```
- **Zephyr:** `k_thread_create()` + wrapper entry `zephyr_thread_entry(p1,p2,p3)`
- **Host:** `pthread_create()` (Linux/macOS) hoặc `CreateThread()` (Windows)
- Priority mapping: `ro_priority_t.v` → Zephyr negative/positive priority convention

**Điểm học được:**
- Cần một `zephyr_thread_entry` wrapper vì Zephyr entry nhận `(p1, p2, p3)` còn RobotOS entry nhận `(arg)`.
- Stack phải là `K_THREAD_STACK_DEFINE` trên Zephyr — caller cần khai báo đúng macro.

---

### 4.2 `ro_queue` — Fixed-Size Message Queue
**Bài toán:** Thread-safe, ISR-safe message passing với bounded memory.

**Giải pháp:** Wrap `k_msgq` (Zephyr) với caller-provided buffer.
```
k_msgq_init(msgq, buf, item_size, capacity)
  send()  → k_msgq_put(timeout)  → convert K_MSEC / K_FOREVER / K_NO_WAIT
  recv()  → k_msgq_get(timeout)
  isr variants → same k_msgq_put/get (Zephyr k_msgq đã ISR-safe)
```
- Buffer do **caller cấp phát** (static array → no heap).
- `k_msgq` đã implement ring buffer đúng → không cần tự viết.
- `RO_QUEUE_WAIT_FOREVER` → `K_FOREVER`, `0` → `K_NO_WAIT`, `ms` → `K_MSEC(ms)`.

**Điểm học được:**
- `q->count` field được update bằng `k_msgq_num_used_get()` sau mỗi put/get. Tuy nhiên đây là approximation — không atomic với put/get trong multi-producer case.

---

### 4.3 `ro_pool` — Fixed-Block Memory Pool
**Bài toán:** Thay thế `malloc/free` trong realtime path. Cần O(1) alloc/free, ISR-safe.

**Giải pháp:** *Bitmap allocator* trên caller-provided buffer.
```
Layout:
  buffer[0..N-1]   ← N blocks liên tiếp, mỗi block = block_size bytes
  bitmap[words]    ← 1 bit/block: 1=free, 0=used

alloc():
  1. scan bitmap words → tìm word ≠ 0 (có block rảnh)
  2. __builtin_ctz(word) → vị trí bit thấp nhất = block index
  3. clear bit → mark used
  4. return buffer + index * block_size
  → O(N/32) worst case

free():
  1. index = (ptr - buffer) / block_size
  2. set bit → mark free
  → O(1)
```

Ba lock mode:
| Mode | Cơ chế | Dùng khi |
|------|--------|----------|
| `NONE` | Không lock | Single-thread / ISR-only |
| `ATOMIC` | CAS loop trên bitmap word | ISR + thread cùng alloc |
| `MUTEX` | `k_mutex` | Multi-thread, không ISR |

**Điểm học được:**
- `ATOMIC` mode dùng CAS trên từng word bitmap — hoạt động nhưng có ABA risk nếu nhiều ISR cùng alloc. Cần kiểm tra thực tế trên target.
- Cần `__builtin_ctz` → GCC/Clang only. MSVC host build cần fallback.

---

### 4.4 `ro_log` — ISR-Safe Ring Buffer Logger
**Bài toán:** Log từ ISR context không được block, không được malloc.

**Giải pháp:** *Lock-free SPSC ring buffer* (Single Producer Single Consumer).
```
s_ring[LOG_RING_COUNT]           ← static array of log_entry_t (64 bytes each)
s_head (volatile uint32_t)       ← producer writes here
s_tail (volatile uint32_t)       ← consumer reads from here

Producer (ro_log — ISR context):
  next_head = (head + 1) % size
  if next_head == tail: drop, ro_log_drop_count++
  else: write entry[head], advance head

Consumer (ro_log_flush — background thread):
  while tail != head: print entry[tail], advance tail
```
Entry format (64 bytes fixed):
```
[ timestamp_us : 8B ][ level : 1B ][ module : 7B ][ message : 48B ]
```

**Điểm học được:**
- Lock-free SPSC chỉ đúng khi **1 producer + 1 consumer**. Nếu nhiều ISR cùng log (multi-producer), cần atomic increment cho head.
- `drop_count` là signal quan trọng: nếu > 0 trong runtime → ring_size cần tăng.

---

### 4.5 `ro_deadline` — Deadline Window Monitor
**Bài toán:** Phát hiện task bị over-budget trong realtime path, không overhead lớn.

**Giải pháp:** *Static slot table* với ID-based lookup.
```
s_table[RO_DEADLINE_MAX_IDS=16]  ← static array of deadline_slot_t

register(id, name, budget_us)    ← claim một slot
begin(id)                        ← lưu ro_time_us() vào slot._start_us
end(id)                          ← elapsed = now - start; if > budget: miss_count++
miss_count(id)                   ← đọc counter, không reset
```
- `ro_time_us()` là nguồn timestamp duy nhất → **lock-free**, không cần mutex.
- `miss_count` là `volatile uint32_t` → atomic read trên 32-bit MCU.
- Lookup là linear scan 16 phần tử → effectively O(1) với table nhỏ.

**Điểm học được:**
- `begin()` không kiểm tra nếu đã `begin()` trước đó mà chưa `end()` → nested deadline bug. Cần thêm guard.
- Nên dùng `atomic_uint` thay `volatile uint32_t` cho `miss_count` để formal correctness.

---

### 4.6 `ro_trace` — Trace Event
**Bài toán:** Export timeline events để visualize với Chrome Trace / SystemView.

**Giải pháp (current — minimal):** `printf` với timestamp.
```c
printf("[TRACE %llu] %s.%s = %u\n", ro_time_us(), module, event, value);
```
- Khi `CONFIG_TRACING=y` → hook vào Zephyr CTF/SystemView backend.
- Khi `ROBOTOS_TRACE_ENABLED` → stdout (host/debug).
- Khi cả hai đều off → no-op (zero overhead).

**Điểm học được:**
- Đây là stub — production cần ring buffer riêng + binary format (CTF) để không block ISR.

---

## 5. Pattern Chuẩn Của Một Adapter Module

```c
/* include/robotos/ro_foo.h — PUBLIC, zero Zephyr */
typedef struct ro_foo ro_foo_t;           // opaque
ro_status_t ro_foo_init(ro_foo_t* h, const ro_foo_config_t* cfg);
ro_status_t ro_foo_do_thing(ro_foo_t* h, ...);

/* src/adapter/zephyr/ro_foo.c — PRIVATE, Zephyr only */
#include <zephyr/kernel.h>
struct ro_foo {
    struct k_xxx   impl;       // internal Zephyr object
    uint32_t       metadata;   // thêm metadata nếu cần
};
```

---

## 6. CMake Integration

```cmake
if(HOST_BUILD)
    add_subdirectory(src/adapter/host)
else()
    add_subdirectory(src/adapter/zephyr)
endif()
```

---

## 7. Checklist — Khi Viết Adapter Module Mới

- [ ] Header trong `include/robotos/ro_*.h` — **không có** `#include <zephyr/*>`
- [ ] Struct là opaque (forward declare trong header, define trong `.c`)
- [ ] ISR-safe variant có suffix `_isr`
- [ ] Buffer/memory do **caller cấp** (không tự malloc)
- [ ] Có `RO_ASSERT` tại entry points cho NULL check
- [ ] Implement cả `zephyr/ro_*.c` và `host/ro_*.c`
