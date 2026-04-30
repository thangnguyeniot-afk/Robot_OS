# src/adapter/zephyr/

Zephyr RTOS backend cho Adapter Layer. Được compile khi build firmware (`west build`).

---

## Danh sách file (14 files)

| File | Header | Implement |
|------|--------|-----------|
| `ro_status.c` | `ro_status.h` | Error string table, `ro_strerror()` |
| `ro_thread.c` | `ro_thread.h` | `k_thread_create`, `k_thread_join` wrapped |
| `ro_time.c` | `ro_time.h` | `k_uptime_get()` → `ro_time_now_ms/us()` |
| `ro_timer.c` | `ro_timer.h` | `k_timer_init/start/stop` wrapped |
| `ro_queue.c` | `ro_queue.h` | `k_msgq` — zero-copy put/get + ISR variants |
| `ro_pool.c` | `ro_pool.h` | `k_mem_slab` fixed-size allocator |
| `ro_mutex.c` | `ro_mutex.h` | `k_mutex` wrapped |
| `ro_gpio.c` | `ro_gpio.h` | Zephyr GPIO driver API + interrupt callbacks |
| `ro_pwm.c` | `ro_pwm.h` | Zephyr PWM driver API, set_duty → `pwm_set_dt` |
| `ro_i2c.c` | `ro_i2c.h` | `i2c_write/read/write_read_dt` wrapped |
| `ro_spi.c` | `ro_spi.h` | `spi_transceive_dt` wrapped |
| `ro_log.c` | `ro_log.h` | Zephyr `LOG_MODULE_REGISTER` + level routing |
| `ro_trace.c` | `ro_trace.h` | `SEGGER_SYSVIEW_RecordVoid` hoặc GPIO toggle |
| `ro_deadline.c` | `ro_deadline.h` | `k_uptime_get` based deadline tracking |

> **Không có `ro_assert.c`** trong `zephyr/` — assert dùng Zephyr built-in `__ASSERT`.

---

## Build chỉ file này

```bash
west build -b nucleo_f411re .
# hoặc
west build -b qemu_cortex_m3 .
```

---

## Yêu cầu Zephyr config (`prj.conf`)

```
CONFIG_GPIO=y
CONFIG_PWM=y
CONFIG_I2C=y
CONFIG_SPI=y
CONFIG_LOG=y
CONFIG_CBPRINTF_PACKAGE_LONGDOUBLE=n
```
