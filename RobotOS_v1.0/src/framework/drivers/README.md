# src/framework/drivers/

Low-level drivers — các file ở đây chạy trong **ISR context** hoặc real-time thread, cần đặc biệt chú ý khi sửa.

---

## Files (2 files)

### `stepper_drv.c`

**Bresenham DDA pulse generator** cho stepper motor đa trục.

- Tính toán step timing bằng thuật toán Bresenham — không dùng floating point.
- Được gọi từ `t_pulse_mgr` (priority RT_PULSE = -16).
- Nhận `motion_seg_t` từ `seg_q`, phát GPIO pulses cho X/Y/Z axes.
- Trapezoidal velocity profile: Accel → Const → Decel.

```c
// API (nội bộ — không expose ra include/)
void stepper_drv_init(stepper_drv_t *drv, const stepper_drv_cfg_t *cfg);
void stepper_drv_load_seg(stepper_drv_t *drv, const motion_seg_t *seg);
bool stepper_drv_tick(stepper_drv_t *drv);   // returns true when segment done
```

**Timing budget:** `stepper_drv_tick()` phải hoàn thành < 10 µs tại 200 kHz step rate.

---

### `endstop_drv.c`

**Hardware debounce và edge detection** cho limit switches.

- Debounce bằng counter (mặc định 5 ms qua `CONFIG_ENDSTOP_DEBOUNCE_MS`).
- Callback khi triggered: `endstop_drv_callback_t fn(void *user)`.
- Thread-safe: callback được gọi từ ISR, dùng `ro_queue_put_isr` để gửi event.

```c
void endstop_drv_init(endstop_drv_t *drv, const endstop_drv_cfg_t *cfg);
bool endstop_drv_is_triggered(const endstop_drv_t *drv);   // debounced read
void endstop_drv_set_callback(endstop_drv_t *drv,
                              endstop_drv_callback_t fn, void *user);
```

---

## Lưu ý khi sửa

> **Không thêm `k_sleep` hay `ro_thread_sleep_ms` vào các file này.**
> Drivers này phải hoàn thành trong vài µs. Blocking sẽ gây missed steps.

Nếu cần giao tiếp với thread khác → dùng `ro_queue_put_isr` (non-blocking).
