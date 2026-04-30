# src/app/

Application Layer — logic ứng dụng CNC/Robot. Tầng cao nhất trong kiến trúc 4-layer.

---

## Danh sách file (8 files)

| File | Mô tả |
|------|-------|
| `gcode_parser.c` | Parse text G-code line → `gcode_cmd_t`. Hỗ trợ G0/G1/G4/G28/G90/G91/M0/M3/M5 |
| `motion_planner.c` | Pop `gcode_cmd_t` từ `cmd_q`, tính velocity profile, push `motion_seg_t` vào `seg_q` |
| `kinematics_cartesian.c` | Chuyển mm → steps, áp dụng steps/mm từ `machine_profile_t` |
| `app_sm.c` | Application State Machine: BOOT→IDLE→HOMING→RUNNING→PAUSED→ESTOP |
| `config_profiles.c` | 4 machine profiles cứng + load/save skeleton qua NVS |
| `app_glue_robotos.c` | Khởi tạo tất cả modules, tạo 3 threads (shell/planner/pulse_mgr) |
| `app_glue_zephyr.c` | Zephyr-specific: Shell command handlers, DeviceTree GPIO binding |
| `app_main.c` | Entry point: `main()` → `app_init()` → Zephyr scheduler |

---

## Thread Lifecycle (từ `app_glue_robotos.c`)

```c
void app_init(void) {
    // 1. Init hardware (GPIO, PWM)
    // 2. Init adapter objects (queues, mutex)
    // 3. Init framework (stepper, endstop)
    // 4. Init app (planner, parser, sm)
    // 5. Create threads
    ro_thread_create(&t_shell,     shell_entry,     NULL, BACKGROUND, 1024);
    ro_thread_create(&t_planner,   planner_entry,   NULL, RT_CONTROL, 2048);
    ro_thread_create(&t_pulse_mgr, pulse_mgr_entry, NULL, RT_PULSE,   1024);
}
```

---

## State Machine (`app_sm.c`)

```
BOOT ──init_ok──→ IDLE
IDLE ──home_cmd──→ HOMING ──done──→ IDLE
IDLE ──run_cmd───→ RUNNING ──pause──→ PAUSED
RUNNING ──stop───→ IDLE
RUNNING ──esm───→ ESTOP ──reset──→ IDLE
HOMING  ──esm───→ ESTOP
```

---

## Supported G-codes

| Code | Mô tả |
|------|-------|
| `G0 Xn Yn Zn` | Rapid move |
| `G1 Xn Yn Zn Fn` | Linear move với feedrate |
| `G4 Pn` | Dwell (ms) |
| `G28` | Home all axes |
| `G90` | Absolute positioning |
| `G91` | Relative positioning |
| `M0` | Stop |
| `M3 Sn` | Spindle ON (speed) |
| `M5` | Spindle OFF |

---

## Thêm G-code mới

1. Thêm case vào `gcode_parser.c` trong `parse_code()`.
2. Thêm handler vào `motion_planner.c` trong `planner_process_cmd()`.
3. Viết test trong `tests/test_gcode_parser.c`.
