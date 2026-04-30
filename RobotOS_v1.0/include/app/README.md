# include/app/

Public headers cho **Application Layer** — types và interfaces dành riêng cho tầng ứng dụng.

---

## Danh sách Headers (7 files)

| Header | Mô tả |
|--------|-------|
| `motion_seg.h` | Type `motion_seg_t` (24 bytes) — đơn vị di chuyển atomic |
| `app_deadlines.h` | Hằng deadline cho từng thread (SHELL / PLANNER / PULSE_MGR) |
| `gcode_parser.h` | `gcode_cmd_t`, `gcode_parse_line()` — parse G-code text |
| `motion_planner.h` | `motion_planner_t`, `planner_init/push_cmd/tick` |
| `kinematics_cartesian.h` | `kin_cartesian_t`, mm→steps conversion, coordinated motion |
| `app_sm.h` | `app_state_t`, `app_event_t`, Application State Machine API |
| `config_profiles.h` | `machine_profile_t`, profile management (4 built-in profiles) |

---

## Kiểu quan trọng nhất

### `motion_seg_t` (24 bytes)

```c
typedef struct {
    int32_t  dx_steps;        // Displacement trục X (steps)
    int32_t  dy_steps;        // Displacement trục Y (steps)
    int32_t  dz_steps;        // Displacement trục Z (steps)
    uint8_t  axis_mask;       // Bit mask: bit0=X, bit1=Y, bit2=Z
    uint8_t  _pad[3];         // Padding
    uint32_t f_steps_per_s;   // Feedrate (steps/s)
    uint32_t accel_steps_s2;  // Acceleration (steps/s²)
} motion_seg_t;
```

### `gcode_cmd_t`

```c
typedef struct {
    uint8_t  code;      // G-code number (e.g., 0=G0, 1=G1, 28=G28)
    float    x, y, z;  // Target coordinates (mm), NaN = not specified
    float    f;         // Feedrate (mm/min), 0 = use current
    float    e;         // Extrude amount (future use)
} gcode_cmd_t;
```

---

## Data flow

```
UART input
   ↓  gcode_parse_line()
gcode_cmd_t  →  cmd_q  (ro_queue, depth 64)
   ↓  motion_planner_tick()
kinematics_cartesian_plan()
   ↓
motion_seg_t  →  seg_q  (ro_queue, depth 64)
```
