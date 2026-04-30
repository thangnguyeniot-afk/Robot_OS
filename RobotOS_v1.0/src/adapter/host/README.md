# src/adapter/host/

Host build stubs cho Adapter Layer — cho phép compile và test toàn bộ firmware trên PC (Windows / Linux / macOS) mà **không cần phần cứng**.

---

## Danh sách file (15 files)

| File | Implement |
|------|-----------|
| `ro_assert.c` | `assert()` từ `<assert.h>` (chỉ có trong host) |
| `ro_status.c` | Giống Zephyr — error string table |
| `ro_thread.c` | `pthread_create/join/yield`, `usleep` → `ro_thread_sleep_ms` |
| `ro_time.c` | `clock_gettime(CLOCK_MONOTONIC)` hoặc `QueryPerformanceCounter` (Win) |
| `ro_timer.c` | Software timer list, ticked trong `ro_time.c` callback |
| `ro_queue.c` | `pthread_mutex + pthread_cond` bounded queue |
| `ro_pool.c` | `malloc/free` wrapped (no-slab) |
| `ro_mutex.c` | `pthread_mutex_t` wrapped |
| `ro_gpio.c` | No-op stub — logs set/get calls, no real GPIO |
| `ro_pwm.c` | No-op stub — records duty cycle for assertion |
| `ro_i2c.c` | No-op stub — returns `RO_OK` by default |
| `ro_spi.c` | No-op stub — can be mocked in tests |
| `ro_log.c` | `printf` / `fprintf(stderr)` với timestamp + level |
| `ro_trace.c` | No-op |
| `ro_deadline.c` | `clock_gettime` based — fully functional |

---

## Cross-platform

- **Linux / macOS:** dùng `pthreads`, `clock_gettime(CLOCK_MONOTONIC)`.
- **Windows:** dùng `CreateThread`, `QueryPerformanceCounter`. CMake tự detect qua `if(WIN32)`.

---

## Dùng trong test

```c
// test_motion_planner.c
#include <robotos/ro_queue.h>   // → host stub
#include <app/motion_planner.h>

// Không cần hardware — chạy trên PC
int main(void) {
    ro_queue_t q;
    ro_queue_init(&q, buf, sizeof(gcode_cmd_t), 64);
    // ...
}
```

---

## Build

```bash
mkdir build && cd build
cmake -DHOST_BUILD=ON ..
cmake --build .
ctest --output-on-failure -V
```
