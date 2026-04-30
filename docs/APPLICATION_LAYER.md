# RobotOS Inspire — Application Layer Design

> **Version:** v0.1-alpha
> **Last Updated:** 2026-03-05
> **Status:** 🚧 Under Active Development
> **Layer:** Application (portable App Core logic + RobotOS/Zephyr integration glue)
>
> **Scope of this document:** This is the design of a **reference application** (teaching-grade CNC/3D printer demo) that showcases RobotOS properties. The general **Application Layer** role in RobotOS is to: realize domain-specific programs using Framework components, orchestrate runtime binding in a Glue layer, and maintain clear separation between portable app semantics and platform-specific integration. This reference application is one valid instance; other RobotOS applications may use different splits, thread topologies, or state machine structures while following the same architectural principles.

---

## 📋 Table of Contents

1. [Layer Role & Boundary](#layer-role--boundary)
2. [Design Philosophy](#design-philosophy)
3. [File & Header Naming Convention](#file--header-naming-convention)
4. [App Core — OS-Agnostic Portable Modules](#app-core--os-agnostic-portable-modules)
   - [motion_seg_t — The Lane B→C Data Unit](#motion_seg_t--the-lane-bc-data-unit)
   - [G-code Subset](#g-code-subset)
   - [G-code Parser (`gcode_parser`)](#g-code-parser-gcode_parser)
   - [Motion Planner (`motion_planner`)](#motion-planner-motion_planner)
   - [Kinematics (`kinematics_cartesian`)](#kinematics-kinematics_cartesian)
   - [Application State Machine (`app_sm`)](#application-state-machine-app_sm)
   - [Config Profiles (`config_profiles`)](#config-profiles-config_profiles)
5. [3-Lane Architecture](#3-lane-architecture)
   - [Overview](#overview)
   - [Lane A — Control Plane (Shell / CLI)](#lane-a--control-plane-shell--cli)
   - [Lane B — Motion Plane (Planner)](#lane-b--motion-plane-planner)
   - [Lane C — Pulse Plane (Stepper RT)](#lane-c--pulse-plane-stepper-rt)
6. [IPC Graph](#ipc-graph)
   - [cmd_q — Lane A → Lane B](#cmd_q--lane-a--lane-b)
   - [seg_q — Lane B → Lane C](#seg_q--lane-b--lane-c)
   - [evt_q_isr — ISR → Lane C Thread](#evt_q_isr--isr--lane-c-thread)
   - [Backpressure Contract](#backpressure-contract)
   - [Feed Hold (PAUSED) Mechanism](#feed-hold-paused-mechanism)
7. [Thread Roster](#thread-roster)
8. [Kconfig Sizing](#kconfig-sizing)
9. [App Glue Layer (RobotOS Build)](#app-glue-layer-robotos-build)
10. [Shell Commands & Debug UX](#shell-commands--debug-ux)
11. [Deadline Monitoring](#deadline-monitoring)
12. [Portability Story — 3 Builds](#portability-story--3-builds)
13. [Testability](#testability)
14. [Error Model](#error-model)

---

## Layer Role & Boundary

### Position in the Stack

```
┌──────────────────────────────────────────────────────────────────────────┐
│  APPLICATION LAYER  (this document)                                     │
│                                                                          │
│  Role: Demo programs that showcase RobotOS properties.                  │
│  "Teaching-grade reference app" for 3D printer / CNC (2–3 axis).        │
│                                                                          │
│  ┌─────────────────────────────┐    ┌──────────────────────────────┐    │
│  │    App Core (portable C)    │    │  App Glue (platform-specific)│    │
│  │  gcode_parser.*             │    │  app_glue_robotos.c          │    │
│  │  motion_planner.*           │    │  app_threads_robotos.c       │    │
│  │  kinematics_cartesian.*     │    │  (or _freertos.c / _zephyr.c)│    │
│  │  app_sm.*                   │    └──────────────────────────────┘    │
│  │  config_profiles.*          │                                        │
│  │  app_deadlines.h            │                                        │
│  └─────────────────────────────┘                                        │
└──────────────────────────────────────────────────────────────────────────┘
      ↓
Robot Framework Layer     — stepper, encoder, endstop, pid, sm (robot_sm.h)
      ↓
Robot Adapter Layer       — ro_thread, ro_queue, ro_timer, ro_deadline, ro_trace
      ↓
Zephyr Kernel Layer       — Adapter implementations
      ↓
Hardware Layer            — STM32F4, ESP32, nRF52, QEMU
```

### Explanation — App Core vs App Glue

**App Core** = portable domain logic with application/domain semantics (no OS/Framework headers).
- Contains: parsing, planning, kinematics, workflow semantics, machine profiles, state lifecycle
- Depends on: portable contracts only (`motion_seg_t`, `gcode_cmd_t`, standard C)
- Must NOT include `<robotos/*.h>`, `<zephyr/*.h>`, Framework headers, or any OS/platform headers
- Consequence: does not directly access Framework components (stepper, endstop, etc.) — only domain abstractions
- Intentional constraint: App Core is unaware of both kernel APIs and Framework object handles; it operates only on portable application contracts and pure domain semantics.
- Enables: host unit tests, multi-build comparison (swap glue only), and full reusability

**App Glue** = runtime orchestration and integration layer (RobotOS/Zephyr-specific).
- Contains: thread creation, IPC initialization, Framework device binding, state-machine bridging, deadline/trace setup
- Expected to include `<robotos/*.h>` and understand `ro_thread_t`, `ro_queue_t`, Framework bindings
- Responsible for materializing App Core into RobotOS/Zephyr runtime
- **Constraint:** Orchestration and integration only. Must NOT accumulate core domain policy, planning logic, or business semantics that belong in App Core or Framework. Glue is the integration seam, not a logic module.

---

### What the Application Layer Does

**App Core (domain logic):**
| Concern | Module |
|---|---|
| Parse G-code subset (G0/G1/G28/G90/G91/M3/M5/M104) | `gcode_parser.*` |
| Generate motion segments (trapezoid profile) | `motion_planner.*` |
| Cartesian kinematics (steps per mm, axis mapping) | `kinematics_cartesian.*` |
| App-level state machine (BOOT→IDLE→HOMING→READY→RUNNING→PAUSED→ERROR) | `app_sm.*` |
| Machine profile selection (printer vs CNC, axis count, steps/mm) | `config_profiles.*` |

**App Glue (integration only):**
| Concern | Owner |
|---|---|
| UART/USB CDC shell — command/response parsing, ACK | App Glue |
| Thread lifecycle, IPC wiring, queue initialization | App Glue |
| Framework device binding (stepper, endstop, encoder) | App Glue |
| State machine callback bridge (App SM → Framework SM) | App Glue |
| Deadline registration, trace hook setup, boot sequencing | App Glue |

### What the Application Layer Does NOT Do

- ❌ App Core: No direct RTOS calls (`k_thread`, `k_msgq`, `vTaskCreate`, etc.)
- ❌ App Core: No direct Framework component calls (stepper, encoder, endstop) — integration is in Glue only
- ❌ App Core: No OS or Framework headers (`<robotos/*.h>`, `<zephyr/*.h>`, etc.)
- ❌ App Core: No heap allocation — all buffers `static` or portable contracts only
- ❌ App Glue: Cannot accumulate domain policy, planning logic, or business semantics — that's App Core's job
- ❌ App Glue: Cannot redefine state machine semantics — ownership stays with App SM and Framework SM
- ❌ Any: No Zephyr Device Tree includes in App Core headers

---

## Design Philosophy

### Teaching-Grade + Real-Time

The Application Layer has a dual mandate:

```
1. Easy to read and debug     → clear lane separation, no hidden control flow
2. Real-time correct           → proper priority tiers, ro_timer_sync, deadline monitoring
```

Both goals are achievable simultaneously. The architecture demonstrates that **clear code and
deterministic code are not in conflict** — this is one of the core messages RobotOS wants to send.

### Explanation — Why the App Is Split This Way

The **App Core ↔ App Glue separation** enables:

1. **Teaching clarity:** App Core = domain logic. Glue = OS orchestration.
2. **Multi-build validation:** Same App Core runs on RobotOS / Zephyr / FreeRTOS — latency differences become measurable and attributable.
3. **Deterministic provability:** App Core has no I/O or OS primitives — just data and pure functions.
4. **Reference vehicle:** This app validates RobotOS properties (no-malloc, deadline monitoring, ISR-safe IPC, priority discipline) in a realistic context.

### OS-Agnostic App Core

All domain logic lives in `app/core/` with zero OS headers. This enables:

1. **Unit tests on host PC** — run `gcode_parser`, `motion_planner`, `kinematics_cartesian` with
   a standard C test runner (Unity / CMock), no QEMU required.
2. **3 binary builds** — swap the glue file (`app_glue_robotos.c` / `app_glue_zephyr.c` /
   `app_glue_freertos.c`) without touching any App Core file.
3. **HIL test** — run on board but substitute a "virtual stepper" that counts pulses instead of
   driving real motors.

### RobotOS Showcase Properties

The App is specifically designed to make these RobotOS strengths visible and measurable:

| Property | How It Shows |
|---|---|
| Determinism / no-heap | Static segment buffer; stress 30 s, zero heap jitter |
| Priority safety | `ro_priority_t` prevents raw int; run dual-build comparison |
| Thread roster discipline | `ro_thread_roster_dump()` + trace thread-create events |
| Deadline monitoring | `deadline stats` shell command shows miss counters live |
| ISR-safe IPC | `evt_q_isr` — step-done events from ISR via `ro_queue_send_isr` |
| DT binding | Change DT overlay → different pin/motor layout, no recompile of App Core |
| Portability | Identical `app_sm.*` + `motion_planner.*` in FreeRTOS build |

---

## File & Header Naming Convention

```
src/
  app/
    core/                      ← OS-agnostic, no RobotOS/Zephyr/FreeRTOS includes
      gcode_parser.h / .c
      motion_planner.h / .c
      kinematics_cartesian.h / .c
      app_sm.h / .c
      config_profiles.h / .c
      app_deadlines.h           ← APP_DL_* uint16_t constants only; no OS header
      motion_seg.h              ← motion_seg_t struct; no OS header
    glue/
      app_glue_robotos.c        ← RobotOS build: threads, IPC, DT binding calls
      app_glue_zephyr.c         ← Zephyr-native build (comparison target)
      app_glue_freertos.c       ← FreeRTOS build (comparison target)
    app_main.c                  ← entry: calls app_init() then hands off to OS scheduler
```

### Naming Rules

| Scope | Symbol prefix | Example |
|---|---|---|
| App Core types | `app_` or none (plain C) | `app_state_t`, `motion_seg_t`, `gcode_cmd_t` |
| App Core functions | `app_`, or module prefix | `app_sm_dispatch()`, `gcode_parse_line()` |
| Deadline objects (glue) | `g_dl_` | `g_dl_planner_tick`, `g_dl_pulse_service` |
| Kconfig macros | `CONFIG_APP_` | `CONFIG_APP_MAX_MOTION_SEGMENTS` |
| Thread handles (glue) | `g_t_` | `g_t_shell`, `g_t_planner`, `g_t_pulse_mgr` |
| Queue handles (glue) | `g_q_` | `g_q_cmd`, `g_q_seg`, `g_q_evt_isr` |

> **Rule:** App Core files MUST NOT include `<robotos/*.h>`, `<zephyr/*.h>`, or
> `<FreeRTOS.h>`. Violation is caught by CI `ro_check_app_core_includes`.

---

## App Core — OS-Agnostic Portable Modules

### Explanation — What App Core Really Is

App Core is the portable domain model:
- Data contracts: `motion_seg_t`, `gcode_cmd_t`, `app_state_t`
- Pure functions: `gcode_parse_line()`, `motion_planner_feed()`, `kin_cartesian_to_steps()`
- Workflow lifecycle and machine personality

It stays free from RTOS calls, hardware binding, and backend dependencies. This proves that application semantics belong in the application, not the kernel — and makes the app reusable across OS choices.

---

### UML — Static Module/Class View of the Application Layer

```text
+======================================================================+
|                        APPLICATION LAYER                             |
+======================================================================+

                    << App Core / Portable >>
+--------------------+         creates         +----------------------+
|    gcode_parser    |------------------------>|      gcode_cmd_t     |
+--------------------+                         +----------------------+
| +gcode_parse_line()|                         | type                 |
+---------+----------+                         | x_mm, y_mm, z_mm     |
          |                                    | f_mm_per_min, s_val  |
          | feeds                              | home_mask            |
          v                                    +----------------------+
+--------------------+
|   motion_planner   |<----------------------+
+--------------------+        uses           |
| +init()            |                       |
| +feed()            |                       |
| +get_position()    |                 kinematics_cartesian
| +reset_position()  |                       |
+---------+----------+                       |
          |                                  |
          | produces                         |
          v                                  |
+--------------------+         uses          |
|    motion_seg_t    |<----------------------+
+--------------------+      +-------------------------+
| dx_steps           |      |  kinematics_cartesian  |
| dy_steps           |      +-------------------------+
| dz_steps           |      | +kin_to_steps()        |
| axis_mask          |      +-------------------------+
| f_steps_per_s      |
| accel_steps_s2     |
+--------------------+

+--------------------+         returns         +-------------------+
|  config_profiles   |------------------------>| app_machine_cfg_t |
+--------------------+                         +-------------------+
| +config_get_prof() |                         | profile           |
+--------------------+                         | steps_per_mm[]    |
                                               | max_f_mm_per_min  |
                                               +-------------------+

+--------------------+         contains        +-------------------+
|  app_deadlines.h   |------------------------>| ro_deadline_t externs|
+--------------------+                         +-------------------+
| g_dl_planner_tick  |                         | defined in Glue   |
| g_dl_pulse_service |                         | (DEC-05 Option A) |
+--------------------+                         +-------------------+

+--------------------+          owns           +-------------------+
|       app_sm       |------------------------>|   app_state_t     |
+--------------------+                         +-------------------+
| +create()          |                         | BOOT              |
| +reset()           |                         | IDLE              |
| +dispatch()        |                         | HOMING            |
| +get_state()       |                         | READY             |
+--------------------+                         | RUNNING / PAUSED  |
                                               | ERROR             |
                                               +-------------------+


                    << App Glue / RobotOS Build >>
+---------------------------------------------------------------------+
|                        app_glue_robotos                             |
+---------------------------------------------------------------------+
| - g_q_cmd, g_q_seg, g_q_evt_isr : ro_queue_t*                      |
| - g_t_shell, g_t_planner, g_t_pulse_mgr : ro_thread_t              |
| - g_app_sm : app_sm_t*                                              |
+---------------------------------------------------------------------+
| +app_ipc_init() / +app_threads_start()                              |
| +lane_a_shell_thread() / +lane_b_planner_thread()                   |
| +lane_c_pulse_thread() / +app_sm_callback_bridge()                  |
+---------------------------------------------------------------------+
        | uses                  | uses              | uses
        v                       v                   v
+--------------+      +----------------+   +---------------------+
| gcode_parser |      | motion_planner |   |       app_sm        |
+--------------+      +----------------+   +---------------------+
        |                       |                   |
        | via Glue              |                   |
        v                       v                   v
+---------------------------------------------------------------------+
|         RobotOS Framework Layer (stepper, servo, sm_t, etc.)       |
+---------------------------------------------------------------------+
        |
        | via Adapter
        v
+---------------------------------------------------------------------+
|         RobotOS Adapter Layer (ro_thread, ro_queue, etc.)          |
+---------------------------------------------------------------------+
        |
        | implements
        v
+---------------------------------------------------------------------+
|         Zephyr Kernel (k_thread, k_msgq, GPIO, Timers, ISR)        |
+---------------------------------------------------------------------+
```

---

### `motion_seg_t` — The Lane B→C Data Unit (MVP Contract)

This is the sole data contract between the Motion Plane (Lane B) and the Pulse Plane (Lane C).
It is defined in App Core (no OS headers) and passes through `seg_q`.

**MVP scope:** This contract is sufficient for the alpha reference app (trapezoidal acceleration, Cartesian kinematics, 3-axis CNC/printer).
**Future evolution:** Richer metadata may be added (segment flags, segment type enum, junction velocity, acceleration metadata) if needed by advanced planning or machine type support. The contract will evolve by extension, not replacement.

```c
// include/app/motion_seg.h
// NO OS headers — portable across RobotOS / FreeRTOS / bare-metal

#ifndef APP_MOTION_SEG_H
#define APP_MOTION_SEG_H

#include <stdint.h>

// Axis presence flags for axis_mask field
#define APP_AXIS_X  (1u << 0)
#define APP_AXIS_Y  (1u << 1)
#define APP_AXIS_Z  (1u << 2)

typedef struct {
    int32_t  dx_steps;        // X axis displacement (signed, 0 if X not active)
    int32_t  dy_steps;        // Y axis displacement (signed, 0 if Y not active)
    int32_t  dz_steps;        // Z axis displacement (signed, 0 if Z not active)
    uint8_t  axis_mask;       // which axes are active: APP_AXIS_X | APP_AXIS_Y | APP_AXIS_Z
                              // INVARIANT: axis_mask != 0x00 (asserted by Lane C on recv)
    uint8_t  _pad[3];         // explicit padding — struct size = 24 bytes (≤ 64B inline rule)
    uint32_t f_steps_per_s;   // vector feedrate in steps/s (post-kinematics, after F-word scaling)
    uint32_t accel_steps_s2;  // 0 = constant velocity;  >0 = trapezoidal ramp magnitude
} motion_seg_t;

// Compile-time size guard — sizeof must be 24 bytes on all targets
_Static_assert(sizeof(motion_seg_t) == 24, "motion_seg_t size mismatch");

#endif  // APP_MOTION_SEG_H
```

**Design decisions locked:**

| Decision | Rationale |
|---|---|
| No `t_total_us` field | Lane C derives timing from `dx/f` — no stale-sync risk |
| `axis_mask` required | Zero-displacement axes are ambiguous without flag; assert catches uninitialized structs |
| Explicit `_pad[3]` | Portable struct layout; no surprise padding on 64-bit host unit tests |
| 24 bytes | Fits ≤64B Adapter inline-payload rule; safe to copy on `ro_queue_send` |

---

### G-code Subset

MVP supports exactly 7 commands — sufficient to demonstrate the full pipeline:

| Command | Meaning | Parameters |
|---|---|---|
| `G0 Xnnn Ynnn Znnn` | Rapid move (max feedrate) | X, Y, Z (any subset) |
| `G1 Xnnn Ynnn Znnn Fnnn` | Linear move at feedrate F | X, Y, Z, F (mm/min) |
| `G28` | Home all axes (or specified subset) | optional X Y Z |
| `G90` | Set absolute positioning mode | — |
| `G91` | Set relative positioning mode | — |
| `M3 Snnn` | Spindle ON (CNC) / extruder ON (3D) at speed S | S (0–255 PWM) |
| `M5` | Spindle OFF / extruder OFF | — |
| `M104 Snnn` | Set extruder temperature target (simulated via `pid_ctrl_t`) | S (°C) |

> **Out of scope for MVP:** G2/G3 (arc), G4 (dwell), M109 (wait-for-temp), M140/M190 (bed).
> Add behind `CONFIG_APP_ENABLE_ARC` and `CONFIG_APP_ENABLE_TEMP_WAIT` for phase-2.

### Line-by-Line ACK Protocol

The Application uses a simple **line-by-line ACK** for flow control over UART/USB CDC.
No hardware flow control (XON/XOFF) is required.

```
Host sends:        G1 X10 Y20 F3000\n
Firmware replies:  ok\n
Host sends:        G1 X0 Y0 F3000\n
Firmware replies:  ok\n
...
```

On error:
```
Firmware replies:  error: unknown command 'G99'\n
```

**ACK Semantics (critical for host interpretation):**
- `ok` = parse + enqueue success (line is now in `cmd_q`)
- `ok` does **NOT** mean motion execution has completed or even started
- Host must use separate status/telemetry channels to determine actual motion completion
- For MVP: use `status` command to poll current position and queue depth

**Rule:** Host MUST NOT send the next line until `ok\n` or `error:...\n` is received.
This keeps `cmd_q` depth deterministic — at most 1 line in transit at any time from the host,
plus up to `CONFIG_APP_CMD_QUEUE_DEPTH` already-parsed commands buffered in `cmd_q`.

---

### G-code Parser (`gcode_parser`)

```c
// include/app/gcode_parser.h  (no OS headers)

typedef enum {
    GCMD_NONE         = 0,
    GCMD_MOVE_RAPID   = 1,   // G0
    GCMD_MOVE_LINEAR  = 2,   // G1
    GCMD_HOME         = 3,   // G28
    GCMD_SET_ABS      = 4,   // G90
    GCMD_SET_REL      = 5,   // G91
    GCMD_SPINDLE_ON   = 6,   // M3
    GCMD_SPINDLE_OFF  = 7,   // M5
    GCMD_EXTRUDER_TEMP= 8,   // M104
} gcode_cmd_type_t;

typedef struct {
    gcode_cmd_type_t type;
    float x_mm;              // NAN if not present in line
    float y_mm;
    float z_mm;
    float f_mm_per_min;      // NAN if not present
    float s_value;           // NAN if not present (spindle speed / temp)
    uint8_t home_mask;       // bit0=X, bit1=Y, bit2=Z (0xFF = home all)
} gcode_cmd_t;

// Parse one null-terminated line into *out.
// Returns 0 on success, negative error code on parse failure.
// Pure function — no side effects, safe to unit-test on host.
int gcode_parse_line(const char* line, gcode_cmd_t* out);
```

**Portability note:** `gcode_parse_line` is a pure function with no global state. Unit tests
live in `tests/host/test_gcode_parser.c` and run on the build host with no QEMU.

---

### Motion Planner (`motion_planner`)

The planner converts `gcode_cmd_t` (mm-space) into a stream of `motion_seg_t` (step-space).

```c
// include/app/motion_planner.h  (no OS headers)

// Planner context — caller-allocated (static or pool).
typedef struct motion_planner motion_planner_t;

// Initialize planner with a kinematics config.
// steps_per_mm[3]: steps-per-mm for X, Y, Z respectively.
// max_f_steps_s:   machine maximum feedrate in steps/s (from config_profiles).
void motion_planner_init(motion_planner_t* p,
                         const float steps_per_mm[3],
                         uint32_t max_f_steps_s,
                         uint32_t default_accel_steps_s2);

// Feed one parsed gcode command into the planner.
// The planner may produce 0 or 1 motion_seg_t per call.
// Returns the number of segments written to *seg_out (0 or 1).
// Pure function on G90/G91/M-codes that don't produce motion.
int motion_planner_feed(motion_planner_t* p,
                        const gcode_cmd_t* cmd,
                        motion_seg_t* seg_out);

// Query current position (absolute, in steps).
void motion_planner_get_position(const motion_planner_t* p, int32_t pos[3]);

// Reset position to zero (called after homing completes).
void motion_planner_reset_position(motion_planner_t* p);
```

**Trapezoidal profile** (per segment):

```
velocity
 ^
 |   ___________
 |  /           \
 | /  cruise     \
 |/ accel   decel \
 +──────────────────► time
   |←  t_seg     →|

f_steps_per_s  = cruise velocity (post-kinematics feed scaling)
accel_steps_s2 = ramp magnitude (0 = constant velocity / rapid)
```

Lane C derives the full timing schedule from `f_steps_per_s` and `accel_steps_s2`.
The planner does NOT compute step-level timing — that is Lane C's responsibility.

---

### Kinematics (`kinematics_cartesian`)

```c
// include/app/kinematics_cartesian.h  (no OS headers)

typedef struct {
    float steps_per_mm[3];   // X, Y, Z (e.g. 80.0, 80.0, 400.0 for typical 3D printer)
    float max_f_mm_per_min;  // machine limit (e.g. 3000.0)
    float max_accel_mm_s2;   // machine limit (e.g. 500.0)
} kin_cartesian_config_t;

// Convert mm-space displacement and feedrate into step-space.
// Input:  dx_mm, dy_mm, dz_mm, f_mm_per_min
// Output: filled motion_seg_t (axis_mask set automatically)
// Returns 0 on success, -1 if displacement is zero on all axes.
int kin_cartesian_to_steps(const kin_cartesian_config_t* cfg,
                            float dx_mm, float dy_mm, float dz_mm,
                            float f_mm_per_min,
                            motion_seg_t* seg_out);
```

---

### Application State Machine (`app_sm`)

`app_sm` is a **portable FSM** (App Core, no OS/Framework dependency). It owns the
application-level state lifecycle. The Framework `sm_t` (robot_sm.h) is used in the Glue
layer to drive hardware-level reactions — the two state machines are **independent by design**
(Option A portability model).

#### States

```
BOOT → IDLE → HOMING → READY → RUNNING → PAUSED → ERROR
                 ↑                  |
                 └──────────────────┘  (re-home after ERROR clear)
```

| State | Meaning | Entry condition |
|---|---|---|
| `APP_STATE_BOOT` | System initializing (threads not yet started) | Power-on |
| `APP_STATE_IDLE` | Initialized; motors disabled or enabled but not homed | Init complete |
| `APP_STATE_HOMING` | Homing sequence in progress | `home` command received |
| `APP_STATE_READY` | Homed + `seg_q` empty + no fault | Homing complete |
| `APP_STATE_RUNNING` | Executing motion segments | `run` or `move` in READY |
| `APP_STATE_PAUSED` | Feed hold — lane B stops producing, lane C drains then stops | `pause` command |
| `APP_STATE_ERROR` | Fatal fault — operator must acknowledge | HW fault / watchdog |

**READY precondition (locked):**
```
READY = homed AND seg_q.empty AND g_fault == false
```
"Motors enabled" is NOT a state condition — it is hardware-specific and handled by the Glue layer.

**PAUSED mechanism:**
```
g_feed_hold = 1  →  Lane B skips segment production each tick
                     Lane C finishes loading current segment, then blocks on empty seg_q
```
`g_feed_hold` is a C11 `atomic_int` — portable across all RTOS targets (no OS primitive needed).

```c
// include/app/app_sm.h  (no OS headers)

typedef enum {
    APP_STATE_BOOT    = 0,
    APP_STATE_IDLE    = 1,
    APP_STATE_HOMING  = 2,
    APP_STATE_READY   = 3,
    APP_STATE_RUNNING = 4,
    APP_STATE_PAUSED  = 5,
    APP_STATE_ERROR   = 6,
} app_state_t;

typedef enum {
    APP_CMD_INIT_DONE       = 0,   // Glue: threads started
    APP_CMD_HOME            = 1,   // Shell: "home"
    APP_CMD_HOMING_DONE     = 2,   // Glue: endstops triggered + backoff complete
    APP_CMD_RUN             = 3,   // Shell: "run" / "move"
    APP_CMD_STOP            = 4,   // Shell: "stop" (graceful)
    APP_CMD_PAUSE           = 5,   // Shell: "pause" — sets g_feed_hold
    APP_CMD_RESUME          = 6,   // Shell: "resume" — clears g_feed_hold
    APP_CMD_FAULT           = 7,   // Watchdog / HW fault
    APP_CMD_FAULT_CLEARED   = 8,   // Shell: "clear_error"
    APP_CMD_QUEUE_EMPTY     = 9,   // Lane B: planner pipeline drained (cmd_q and seg_q empty)
} app_cmd_t;

typedef struct app_sm app_sm_t;

// Lifecycle
app_sm_t*   app_sm_create(void);      // returns pointer to static singleton
void        app_sm_reset(app_sm_t* sm);

// Dispatch (thread-safe: caller holds no lock — dispatching is message-passing in glue)
// Returns 0 on success, -1 if transition invalid in current state.
int         app_sm_dispatch(app_sm_t* sm, app_cmd_t cmd);

// Query
app_state_t app_sm_get_state(const app_sm_t* sm);
const char* app_sm_state_name(app_state_t state);

// Transition callback (optional — set by Glue to drive hardware on state change)
typedef void (*app_sm_cb_t)(app_sm_t* sm, app_state_t from, app_state_t to, void* user);
void        app_sm_set_callback(app_sm_t* sm, app_sm_cb_t cb, void* user);

// Feed-hold flag (C11 atomic — visible to all threads without lock)
extern _Atomic int g_feed_hold;  // 0 = run, 1 = paused
```

#### State Transition Table

| Current | Command | Next | Glue side-effect |
|---|---|---|---|
| `BOOT` | `APP_CMD_INIT_DONE` | `IDLE` | Log "system ready" |
| `IDLE` | `APP_CMD_HOME` | `HOMING` | Glue calls `stepper_move` toward endstop; sets Framework SM `CMD_HOME` |
| `HOMING` | `APP_CMD_HOMING_DONE` | `READY` | Glue resets position counters; sets Framework SM `CMD_HOMING_COMPLETE` |
| `HOMING` | `APP_CMD_FAULT` | `ERROR` | Glue calls `stepper_stop` on all axes; sets Framework SM `CMD_FAULT` |
| `READY` | `APP_CMD_RUN` | `RUNNING` | Glue clears g_feed_hold; Lane B starts segment production |
| `RUNNING` | `APP_CMD_PAUSE` | `PAUSED` | Glue sets `atomic_store(&g_feed_hold, 1)` |
| `RUNNING` | `APP_CMD_QUEUE_EMPTY` | `READY` | Glue logs "planner pipeline drained" |
| `RUNNING` | `APP_CMD_FAULT` | `ERROR` | Glue calls `stepper_stop`; sets g_feed_hold=1 |
| `PAUSED` | `APP_CMD_RESUME` | `RUNNING` | Glue clears `atomic_store(&g_feed_hold, 0)` |
| `PAUSED` | `APP_CMD_STOP` | `READY` | Glue flushes cmd_q + seg_q; resets planner |
| `ERROR` | `APP_CMD_FAULT_CLEARED` | `IDLE` | Glue calls `sm_clear_fault` on Framework SM |

> **App SM ownership (application-level workflow):** This state machine is **application-level workflow**, owned entirely by App Core and dispatched by App Core logic in `app_sm.c`. The states represent the prepare-and-execute cycle from the user program's perspective.
> 
> **Framework SM ownership (machine/hardware state):** Framework `STATE_IDLE/HOMING/RUN/FAULT` (from `robot_sm.h`) are reusable hardware/machine states managed by the Framework layer. Glue's `app_sm_cb_t` callback bridges them: when App SM transitions to HOMING, Glue invokes `sm_dispatch(sm, ROBOT_CMD_HOME)` to synchronize Framework state. 
>
> **Key distinction:** Both machines exist, but ownership is clear — App Core owns App SM semantics, Framework owns machine state semantics. Glue is the synchronization seam. `APP_STATE_READY` and `APP_STATE_PAUSED` are purely application-level (no Framework equivalent) — the Framework SM stays in `STATE_RUN` during PAUSED since feedrate is a software concern, not a hardware state.

---

### State Ownership Summary

| State | Meaning | Entry condition | Ownership |
|---|---|---|---|
| `APP_STATE_BOOT` | System initializing (threads not yet started) | Power-on | App |
| `APP_STATE_IDLE` | Initialized; motors disabled or enabled but not homed | Init complete | App |
| `APP_STATE_HOMING` | Homing sequence in progress | `home` command received | App |
| `APP_STATE_READY` | Homed + `seg_q` empty + no fault | Homing complete | App |
| `APP_STATE_RUNNING` | Executing motion segments | `run` or `move` in READY | App |
| `APP_STATE_PAUSED` | Feed hold — lane B stops producing, lane C drains then stops | `pause` command | App |
| `APP_STATE_ERROR` | Fatal fault — operator must acknowledge | HW fault / watchdog | App |

> **MVP simplification in READY precondition:** The current rule uses `seg_q.empty` as a proxy for "planner has finished producing." This is convenient for teaching but not complete: true execution idle requires verification that Lane C has actually completed the final segment (position at target, motion stopped). Future versions should use an explicit execution-idle or actuation-complete signal (e.g., an ISR deadline-miss event on the final segment, or a separate status query to Framework). The current MVP works for unidirectional motion — validation recommended before release.

---

### UML — Application Workflow State Machine (app_sm)

```text
+====================================================================+
|                     app_sm - State Machine UML                     |
+====================================================================+

 [BOOT]
    |
    | APP_CMD_INIT_DONE
    v
 [IDLE] ------------------------------------------------+
    |                                                 |
    | APP_CMD_HOME                    APP_CMD_FAULT_CLEARED
    v                                                 |
 [HOMING] -------- APP_CMD_HOMING_DONE ---------> [READY]
    |                                                   |
    | APP_CMD_FAULT                                     | APP_CMD_RUN
    v                                                   v
 [ERROR] <-------- APP_CMD_FAULT -------------- [RUNNING]
    ^                                                   |
    |                                                   | APP_CMD_PAUSE
    |                                                   v
    +-------------- APP_CMD_FAULT ------------ [PAUSED]
                                                     |
                                                     | APP_CMD_RESUME
                                                     +----> [RUNNING]
                                                     |
                                                     | APP_CMD_STOP
                                                     v
                                                   [READY]

 [RUNNING] --- APP_CMD_QUEUE_EMPTY ---> [READY]
 [RUNNING] --- APP_CMD_FAULT ---------> [ERROR]
 [HOMING]  --- APP_CMD_FAULT ---------> [ERROR]
```

---

### Config Profiles (`config_profiles`)

```c
// include/app/config_profiles.h  (no OS headers)

typedef enum {
    APP_PROFILE_CNC_2AXIS   = 0,   // X/Y + spindle (PWM)
    APP_PROFILE_CNC_3AXIS   = 1,   // X/Y/Z + spindle (PWM)
    APP_PROFILE_PRINTER_2AXIS = 2, // X/Y + extruder (fake PID temp)
    APP_PROFILE_PRINTER_3AXIS = 3, // X/Y/Z + extruder
} app_profile_t;

typedef struct {
    app_profile_t profile;
    float         steps_per_mm[3];    // X, Y, Z
    float         max_f_mm_per_min;
    float         max_accel_mm_s2;
    uint8_t       active_axis_count;  // 2 or 3
    bool          has_spindle;
    bool          has_extruder;
} app_machine_config_t;

// Returns a const pointer to a compile-time profile struct.
const app_machine_config_t* config_get_profile(app_profile_t p);
```

The active profile is selected at startup via `CONFIG_APP_DEFAULT_PROFILE` Kconfig value
(overridable at runtime by the `profile` shell command).

---

## 3-Lane Architecture

### Explanation — 3-Lane Design Rationale

The architecture splits execution into independent priority tiers (lanes) to achieve:

1. **Separation of concerns:** Each lane has a single responsibility (input, planning, realtime pulse).
2. **Priority discipline:** Lane priorities are *strictly ordered*: A < B < C in scheduler terms (lower priority = higher OS realtime priority).
3. **Backpressure as visibility:** Queues don't hide overload — developers see queue depth and measure latency.
4. **Feed hold without preemption:** Pause works by stopping segment production (Lane B), not by interrupting pulse (Lane C).
5. **Deterministic observability:** Each lane's workload is bounded and measurable.

---

### Overview

The App runs three threads. Each thread maps to exactly one priority tier:

```
┌─────────────────────────────────────────────────────────────────────────┐
│  Lane A — Control Plane                    RO_PRIO_BACKGROUND (.v=12)  │
│  t_shell                                                                │
│  UART recv → G-code parse → cmd_q push → shell response ("ok\n")       │
├─────────────────────────────────────────────────────────────────────────┤
│  Lane B — Motion Plane                     RO_PRIO_RT_CONTROL (.v=-10) │
│  t_planner     (1 kHz, ro_timer_sync)                                  │
│  cmd_q recv → kinematics → motion_planner → seg_q push                 │
├─────────────────────────────────────────────────────────────────────────┤
│  Lane C — Pulse Plane                       RO_PRIO_RT_PULSE (.v=-16)  │
│  t_pulse_mgr                                                            │
│  seg_q recv → step schedule → timer ISR pulses                         │
│                                                                         │
│  ISR (no thread tier — preempts all)                                   │
│  Toggle step pins → ro_queue_send_isr(evt_q_isr, &step_event)          │
└─────────────────────────────────────────────────────────────────────────┘
```

**Critical rule:** App code MUST use only `ro_*` APIs. No `k_thread`, `k_msgq`,
`vTaskCreate`, etc. — the Adapter layer is the only RTOS-facing seam.

---

### Lane A — Control Plane (Shell / CLI)

**Thread:** `t_shell`  **Priority:** `RO_PRIO_BACKGROUND (.v = 12)`  **Sleep:** `ro_thread_sleep_ms` or blocking UART recv

**Responsibilities:**
- Block on UART/USB CDC byte stream
- Accumulate into line buffer (max `CONFIG_APP_GCODE_LINE_MAX` = 96 bytes, stack-allocated)
- Call `gcode_parse_line()` — pure function, no heap
- Push `gcode_cmd_t` onto `cmd_q` via `ro_queue_send()` (blocking, timeout = `RO_QUEUE_WAIT_FOREVER`)
- Reply `"ok\n"` immediately after push (not after motion completes — pipeline is async)
- Handle shell meta-commands (see [Shell Commands](#shell-commands--debug-ux))
- Drive `app_sm_dispatch()` for user commands (`home`, `stop`, `pause`, `resume`, `clear_error`)

**Why BACKGROUND priority?**  
Shell/UART parsing has no hard deadline. Running it at lower priority than Lane B and C ensures
that even a burst of incoming G-code lines cannot delay the planner tick or stepper pulses.

```c
// Conceptual loop (actual code in app_glue_robotos.c)
static void lane_a_shell_thread(void* arg) {
    char line[CONFIG_APP_GCODE_LINE_MAX];
    while (1) {
        int n = uart_recv_line(line, sizeof(line));  // blocks on UART
        if (n <= 0) continue;

        // Meta-command? (home, stop, pause, status, deadline stats, ...)
        if (handle_shell_meta(line)) {
            uart_send("ok\n");
            continue;
        }

        // G-code line
        gcode_cmd_t cmd;
        if (gcode_parse_line(line, &cmd) != 0) {
            uart_send("error: parse failed\n");
            continue;
        }

        ro_queue_send(g_q_cmd, &cmd, RO_QUEUE_WAIT_FOREVER);
        uart_send("ok\n");  // ACK after enqueue, not after motion
    }
}
```

---

### Lane B — Motion Plane (Planner)

**Thread:** `t_planner`  **Priority:** `RO_PRIO_RT_CONTROL (.v = -10)`  **Loop:** `ro_timer_sync` @ 1 kHz

> **Mandatory:** Running at ≥ 200 Hz means `ro_thread_sleep_ms` is forbidden.
> Lane B MUST use `ro_timer_sync(&g_planner_tick)` per the Kernel Layer Cooperative Yield Contract.

**Responsibilities:**
- Drain `cmd_q` (non-blocking) each tick
- Call `motion_planner_feed()` to convert `gcode_cmd_t` → `motion_seg_t`
- Push `motion_seg_t` onto `seg_q`
- Handle backpressure: if `seg_q` full, apply backpressure policy (see [Backpressure Contract](#backpressure-contract))
- Check `g_feed_hold` before each segment push (skip production if paused)
- Emit `APP_CMD_QUEUE_EMPTY` to app_sm when `cmd_q` and `seg_q` are both empty (planner pipeline drained — not physical motion complete)
- Bracket the active work with `ro_deadline_begin / ro_deadline_end (&g_dl_planner_tick)`

```c
// Conceptual loop (actual code in app_glue_robotos.c)
static void lane_b_planner_thread(void* arg) {
    ro_timer_t tick;
    ro_timer_start_periodic(&tick, 1000 /*µs — 1 kHz*/);  // DEC-04: no ro_timer_init()

    while (1) {
        ro_deadline_begin(&g_dl_planner_tick);

        if (!atomic_load(&g_feed_hold)) {
            gcode_cmd_t cmd;
            // Non-blocking drain: 1 cmd/tick (MVP simplification).
            // Phase-2: drain up to N cmds/tick for burst throughput
            // without starvation risk (bounded by deadline budget).
            if (ro_queue_recv(g_q_cmd, &cmd, 0) == RO_OK) {
                motion_seg_t seg;
                if (motion_planner_feed(&g_planner, &cmd, &seg) == 1) {
                    ro_status_t s = ro_queue_send(g_q_seg, &seg,
                                                   CONFIG_APP_SEG_PUSH_TIMEOUT_TICKS);
                    if (s == RO_EAGAIN) {
                        ro_log(RO_LOG_WARN, "APP", "planner: segment buffer full");
                        // Re-queue cmd or drop depending on profile — see backpressure rule
                    }
                }
            }

            // Detect planner pipeline drain: both queues empty.
            // MVP: polling via ro_queue_count() — simple and correct at 1 kHz.
            // Phase-2: replace with an atomic counter decremented on each recv
            //          to eliminate per-tick polling overhead at high throughput.
            if (ro_queue_count(g_q_cmd) == 0 && ro_queue_count(g_q_seg) == 0) {
                // notify app_sm (via glue's sm callback, not direct call)
                app_sm_dispatch(g_app_sm, APP_CMD_QUEUE_EMPTY);
            }
        }

        ro_deadline_end(&g_dl_planner_tick);

        // DEC-02: ro_timer_sync returns uint32_t (periods elapsed since last call).
        uint32_t tick_overruns = ro_timer_sync(&tick);
        if (tick_overruns > 0) {
            ro_log(RO_LOG_WARN, "APP", "planner: %u tick overrun(s)", tick_overruns);
        }
    }
}
```

---

### Lane C — Pulse Plane (Stepper RT)

**Thread:** `t_pulse_mgr`  **Priority:** `RO_PRIO_RT_PULSE (.v = -16)`  **Yield:** `ro_thread_yield()` between segments

**ISR context** (no thread tier — preempts all):

> ISR MUST only: toggle step pins + call `ro_queue_send_isr(evt_q_isr, &event)`.
> ISR MUST NOT: call any blocking API, touch `seg_q` directly, or call Framework APIs.

**Responsibilities of `t_pulse_mgr`:**
- Block on `seg_q` via `ro_queue_recv` (wait = FOREVER or timeout)
- Assert `seg.axis_mask != 0x00` before loading
- Compute step-level schedule from `motion_seg_t` (Bresenham DDA or timer period table)
- Load schedule into step-timer ISR context (memory-mapped timer or `ro_timer_t`)
- Drain `evt_q_isr` for housekeeping (update position counters, detect stall)
- Bracket active ISR service budget with `ro_deadline_begin/end(&g_dl_pulse_service)` called
  from the step-done event handler in the thread (not from the ISR itself)
- `ro_thread_yield()` after each loaded segment (cooperative yield to FRAMEWORK tier if needed)

```
Lane C thread lifecycle per segment:

  ro_queue_recv(seg_q) ─► assert axis_mask ─► compute DDA schedule
        ─► load timer ISR ─► wait for step-done events (evt_q_isr)
        ─► update position ─► ro_thread_yield() ─► back to recv
                                  ↑
                              ISR fires: toggle pins, send step-event
```

---

## IPC Graph

### Explanation — Why IPC Is Structured as 3 Queues

- **`cmd_q`** decouples shell/UART input (Lane A) from motion planning (Lane B) → input latency doesn't block planning
- **`seg_q`** decouples planning (Lane B) from pulse execution (Lane C) → complex planning doesn't jitter pulse timing
- **`evt_q_isr`** decouples ISR (step done / stall) from Lane C thread → ISR stays minimal, thread handles stats/counters

**Backpressure is intentional and observable:**
- If Lane B can't keep up, `seg_q` fills → visible as queue depth → developer can measure and fix
- Feed hold works by stopping segment production (Lane B waits on empty `seg_q`) rather than forcibly stopping pulses
- No hidden failure modes — queue depth is always measurable and bounded by Kconfig sizing

**Pipeline drained ≠ motion complete (CRITICAL semantic distinction):**
- `CMD_QUEUE_EMPTY` (fired when both `cmd_q` and `seg_q` are empty) signals the **planner pipeline is drained**.
- It does **NOT** mean physical actuation is complete — Lane C may still be executing the final segment loaded into the stepper ISR.
- True motion completion requires additional verification: stepper position tracking, encoder feedback, or explicit end-of-segment ISR event (phase-2 deadline miss event).
- **For host protocol:** ACK semantics (ok = enqueue) combined with a separate `status` query is needed to detect true completion.

**Design outcome:** Deterministic runtime behavior + visibility without magic or implicit buffering.

---

```
 Lane A (t_shell)
     │
     │  gcode_cmd_t
     ▼
 ┌──────────────────────────────────────────────────────┐
 │  cmd_q  (RO_PRIO_BACKGROUND → RO_PRIO_RT_CONTROL)   │
 │  depth: CONFIG_APP_CMD_QUEUE_DEPTH (default 64)      │
 │  backing: app-owned static buffer                    │
 └──────────────────────────────────────────────────────┘
     │
     │  gcode_cmd_t
     ▼
 Lane B (t_planner)
     │
     │  motion_seg_t
     ▼
 ┌──────────────────────────────────────────────────────┐
 │  seg_q  (RO_PRIO_RT_CONTROL → RO_PRIO_RT_PULSE)     │
 │  depth: CONFIG_APP_MAX_MOTION_SEGMENTS (default 64)  │
 │  backing: app-owned static buffer (NOT IPC slab)     │
 └──────────────────────────────────────────────────────┘
     │
     │  motion_seg_t
     ▼
 Lane C (t_pulse_mgr)         ISR
     │                         │
     │  ◄── step_event_t ──────┘
     │  evt_q_isr (tiny, depth 8, ISR-safe)
```

### `cmd_q` — Lane A → Lane B

```c
// Declared in app_glue_robotos.c
// App-owned buffer — does NOT consume RO_GLOBAL_IPC_SLAB (DEC-08)
static uint8_t    g_cmd_q_buf[sizeof(gcode_cmd_t) * CONFIG_APP_CMD_QUEUE_DEPTH];
static ro_queue_t* g_q_cmd;  // opaque handle — never declare ro_queue_t by value (DEC-01)

// Init (in app_glue_robotos.c setup):
g_q_cmd = ro_queue_create(sizeof(gcode_cmd_t),
                           CONFIG_APP_CMD_QUEUE_DEPTH,
                           g_cmd_q_buf, sizeof(g_cmd_q_buf));
RO_ASSERT(g_q_cmd != NULL, "cmd_q alloc failed");

// Lane A sends (blocking):
ro_queue_send(g_q_cmd, &cmd, RO_QUEUE_WAIT_FOREVER);

// Lane B receives (non-blocking, per tick):
ro_queue_recv(g_q_cmd, &cmd, 0 /*timeout_ticks*/);
```

### `seg_q` — Lane B → Lane C

```c
// Declared in app_glue_robotos.c
// App-owned buffer — does NOT consume RO_GLOBAL_IPC_SLAB
static uint8_t    g_seg_q_buf[sizeof(motion_seg_t) * CONFIG_APP_MAX_MOTION_SEGMENTS];
static ro_queue_t* g_q_seg;  // opaque handle — DEC-01

// Init:
g_q_seg = ro_queue_create(sizeof(motion_seg_t),
                           CONFIG_APP_MAX_MOTION_SEGMENTS,
                           g_seg_q_buf, sizeof(g_seg_q_buf));
RO_ASSERT(g_q_seg != NULL, "seg_q alloc failed");

// Lane B sends (with backpressure timeout):
ro_status_t s = ro_queue_send(g_q_seg, &seg, CONFIG_APP_SEG_PUSH_TIMEOUT_TICKS);

// Lane C receives (blocking):
ro_queue_recv(g_q_seg, &seg, RO_QUEUE_WAIT_FOREVER);
```

> **Why app-owned buffer?** Keeps the Adapter `RO_GLOBAL_IPC_SLAB` budget intact for Framework
> use. The app knows exactly how much segment memory it needs at compile time — no reason to
> share a global pool.

### `evt_q_isr` — ISR → Lane C Thread

> **MVP gate:** Compiled in only when `CONFIG_APP_ENABLE_ISR_EVENTS=y` (default `n` for MVP).
> When `n`: Lane C updates position counters synchronously after each segment — simpler but
> loses stall detection. Set `y` for phase-2 / production builds requiring stall feedback.

```c
// Tiny queue for step-done + stall events from ISR to Lane C thread
typedef struct {
    uint8_t  axis;      // axis index (0=X, 1=Y, 2=Z)
    uint8_t  type;      // EVT_STEP_DONE, EVT_STALL_DETECTED
    uint16_t count;     // steps completed in this burst
} step_event_t;

static uint8_t    g_evt_q_buf[sizeof(step_event_t) * CONFIG_APP_EVT_QUEUE_DEPTH];
static ro_queue_t* g_q_evt_isr;  // opaque handle — DEC-01

// Init:
g_q_evt_isr = ro_queue_create(sizeof(step_event_t),
                               CONFIG_APP_EVT_QUEUE_DEPTH,
                               g_evt_q_buf, sizeof(g_evt_q_buf));
RO_ASSERT(g_q_evt_isr != NULL, "evt_q_isr alloc failed");

// ISR sends (ISR-safe, non-blocking):
ro_queue_send_isr(g_q_evt_isr, &event);  // drops if full — stall detection is
                                           // best-effort, not life-critical here

// Lane C thread drains (non-blocking, between segments):
step_event_t ev;
while (ro_queue_recv(g_q_evt_isr, &ev, 0) == RO_OK) {
    update_position_counter(ev.axis, ev.count);
}
```

### Backpressure Contract

When Lane B attempts `ro_queue_send(seg_q, ...)` and the queue is full:

| Timeout | Behavior | When to use |
|---|---|---|
| `0` (non-blocking) | Returns `RO_EAGAIN` immediately. Lane B logs `"planner: buffer full"` and retries next tick. | Streaming run at max speed |
| `CONFIG_APP_SEG_PUSH_TIMEOUT_TICKS` (default `2`) | Blocks max 2 ticks. If still full, returns `RO_EAGAIN`. Tick budget shows deadline miss — visible in shell. | Debugging / slow Lane C |

> **Rule:** Never use `RO_QUEUE_WAIT_FOREVER` on `seg_q` from Lane B — that would stall the
> 1kHz planner tick and cause a deadline miss cascade.

**Shell visibility:** When backpressure occurs, the miss counter for `g_dl_planner_tick`
increments. The operator sees `deadline stats` output reflecting it. This is a **feature** —
it demonstrates the deadline monitor working correctly.

### Feed Hold (PAUSED) Mechanism

```c
// app_sm.h — C11 atomic, visible to all threads
extern _Atomic int g_feed_hold;   // 0 = run, 1 = paused

// app_sm_dispatch(sm, APP_CMD_PAUSE):
//   → sets atomic_store(&g_feed_hold, 1)
//   → Lane B stops seg production at next tick boundary (no mid-segment split)
//   → Lane C finishes loading current segment, then blocks on empty seg_q naturally

// app_sm_dispatch(sm, APP_CMD_RESUME):
//   → sets atomic_store(&g_feed_hold, 0)
//   → Lane B resumes segment production at next tick

// Lane B check (each tick, before production):
if (atomic_load(&g_feed_hold)) {
    ro_deadline_end(&g_dl_planner_tick);
    ro_timer_sync(&g_planner_tick);
    continue;  // skip production; deadline still OK (work = 0)
}
```

No mutex needed: `_Atomic int` provides the necessary memory ordering on ARM Cortex-M4
(Zephyr) and on FreeRTOS targets with C11 compiler support.

---

### UML — Lane / IPC Runtime View

```text
+==================================================================================+
|                           APPLICATION RUNTIME VIEW                               |
+==================================================================================+

   Lane A - Control Plane                 Lane B - Motion Plane      Lane C - Pulse Plane
   Priority: BACKGROUND                   Priority: RT_CONTROL       Priority: RT_PULSE

+---------------------------+       +--------------------------+    +-------------------------+
|       t_shell             |       |       t_planner          |    |      t_pulse_mgr        |
+---------------------------+       +--------------------------+    +-------------------------+
| recv UART/USB line        |       | periodic tick (1 kHz)    |    | recv motion segment     |
| parse meta/G-code         |       | recv gcode_cmd_t         |    | compute pulse schedule  |
| ACK ok/error              |       | plan motion              |    | load timer/ISR          |
+------------+--------------+       +-------------+------------+    +-----------+-------------+
             |                                    |                             ^
             | send gcode_cmd_t                   | send motion_seg_t           |
             v                                    v                             |
    +---------------------+              +----------------------+               |
    |        cmd_q        |              |        seg_q         |               |
    +---------------------+              +----------------------+               |
    | payload: gcode_cmd_t|              | payload: motion_seg_t|               |
    | A -> B              |              | B -> C               |               |
    +----------+----------+              +----------+-----------+               |
               |                                    |                           |
               +------------------------------------+---------------------------+
                                                    |
                                                    | ISR-safe signal path
                                                    v
                                         +------------------------+
                                         |      Step Timer ISR    |
                                         +------------------------+
                                         | toggle step pins       |
                                         | send step_event_t      |
                                         +-----------+------------+
                                                     |
                                                     | ro_queue_send_isr()
                                                     v
                                           +----------------------+
                                           |      evt_q_isr       |
                                           +----------------------+
                                           | payload: step_event_t|
                                           | ISR -> Lane C thread |
                                           +----------+-----------+
                                                      |
                                                      v
                                             +-------------------+
                                             |    t_pulse_mgr    |
                                             | drain evt_q_isr   |
                                             | update counters   |
                                             +-------------------+
```

---

## Thread Roster

### Explanation — Why Thread Roster Is a First-Class Design Artifact

- **No magic threads:** Every runtime thread must be explicitly declared and reviewed.
- **Architectural artifact:** Thread count, names, priorities, stacks, and ownership are not incidental — they are part of the system design.
- **Conformance checking:** CI verifies roster at build-time (`ro_check_thread_roster` via linker symbols + trace at boot).
- **Observability:** Thread identities are documented, making it easier to interpret traces, measure latencies, and debug timing issues.
- **Scope of this roster:** This is the thread roster of the reference CNC/printer application (3 threads: shell/planner/pulse_mgr). It is not a universal template for all RobotOS applications. Future applications can use different splits, topologies, priority assignments, or thread counts — but must apply the same discipline: explicit roster, no anonymous threads, and CI verification.

---

> **Rule:** Every thread MUST be declared here. CI step `ro_check_thread_roster` verifies
> this list against trace `thread_create` events. No anonymous or undeclared threads allowed.

| Name | Handle | Priority | Stack (bytes) | Lane | Kconfig key |
|---|---|---|---|---|---|
| `t_shell` | `g_t_shell` | `RO_PRIO_BACKGROUND` (.v=12) | 1024 | A | `CONFIG_APP_SHELL_STACK_SIZE` |
| `t_planner` | `g_t_planner` | `RO_PRIO_RT_CONTROL` (.v=-10) | 2048 | B | `CONFIG_APP_PLANNER_STACK_SIZE` |
| `t_pulse_mgr` | `g_t_pulse_mgr` | `RO_PRIO_RT_PULSE` (.v=-16) | 1024 | C | `CONFIG_APP_PULSE_MGR_STACK_SIZE` |

Total App threads: **3**. Total system threads with Zephyr-internal: see KERNEL_LAYER.md
§ "Kernel Internal Thread Assumptions".

**Thread creation (Glue):**

```c
// app_glue_robotos.c — called from APPLICATION SYS_INIT (priority 20)

static RO_THREAD_STACK_DEFINE(shell_stack,   CONFIG_APP_SHELL_STACK_SIZE);
static RO_THREAD_STACK_DEFINE(planner_stack, CONFIG_APP_PLANNER_STACK_SIZE);
static RO_THREAD_STACK_DEFINE(pulse_stack,   CONFIG_APP_PULSE_MGR_STACK_SIZE);

void app_threads_start(void) {
    g_t_shell = ro_thread_create(&(ro_thread_config_t){
        .name       = "t_shell",
        .entry      = lane_a_shell_thread,
        .stack      = shell_stack,
        .stack_size = CONFIG_APP_SHELL_STACK_SIZE,
        .priority   = RO_PRIO_BACKGROUND,
    });

    g_t_planner = ro_thread_create(&(ro_thread_config_t){
        .name       = "t_planner",
        .entry      = lane_b_planner_thread,
        .stack      = planner_stack,
        .stack_size = CONFIG_APP_PLANNER_STACK_SIZE,
        .priority   = RO_PRIO_RT_CONTROL,
    });

    g_t_pulse_mgr = ro_thread_create(&(ro_thread_config_t){
        .name       = "t_pulse_mgr",
        .entry      = lane_c_pulse_thread,
        .stack      = pulse_stack,
        .stack_size = CONFIG_APP_PULSE_MGR_STACK_SIZE,
        .priority   = RO_PRIO_RT_PULSE,
    });

    RO_ASSERT(g_t_shell    != NULL, "t_shell create failed");
    RO_ASSERT(g_t_planner  != NULL, "t_planner create failed");
    RO_ASSERT(g_t_pulse_mgr!= NULL, "t_pulse_mgr create failed");
}
```

---

## Kconfig Sizing

### Explanation — Why Sizing Is Part of Architecture

- **Explicit defaults:** Queue depth, tick rate, stack size, and buffer size are compile-time constants, not hidden costs.
- **No-malloc discipline:** All buffers are static or from fixed pools — their size is bounded by Kconfig.
- **Runtime budgets:** These Kconfig values turn logical resource needs (e.g., "need ~64ms lookahead in seg_q") into measured, testable parameters.
- **Initial vs. validated:** Current Kconfig defaults are architectural *starting points* based on typical use cases. They are not production-proven numbers.
- **Later validation required:** Stack sizes should be watermarked on real hardware (via stack guard or `ro_check_stack_usage`). Queue depths should be measured under stress and variable-load testing. Timing budgets should be validated with deadline miss counters and latency stats.
- **Feature gates:** Phase-2 features like ISR events or arc commands are disabled by default, keeping MVP scope tight and deterministic.

---

All sizing is compile-time. No runtime reallocation.

```kconfig
# App Core timing
config APP_PLANNER_TICK_HZ
    int "Lane B planner tick rate in Hz"
    default 1000
    range 100 10000

# IPC depths
config APP_CMD_QUEUE_DEPTH
    int "Depth of cmd_q (parsed G-code commands)"
    default 64
    # 64 lines × sizeof(gcode_cmd_t) ≈ 64 × 32 = 2048 bytes

config APP_MAX_MOTION_SEGMENTS
    int "Depth of seg_q (motion_seg_t segments)"
    default 64
    # 64 segments × 24 bytes = 1536 bytes; 64ms lookahead @ 1kHz

config APP_EVT_QUEUE_DEPTH
    int "Depth of evt_q_isr (step-done ISR events)"
    default 8
    # Tiny; ISR drops if full (best-effort telemetry)

# Parse buffer
config APP_GCODE_LINE_MAX
    int "Maximum G-code line length in bytes (stack-allocated in Lane A)"
    default 96

# Backpressure
config APP_SEG_PUSH_TIMEOUT_TICKS
    int "Ticks Lane B waits on full seg_q before returning RO_EAGAIN"
    default 2
    # 2 ticks = 2ms at 1kHz; must be << planner deadline budget

# Stack sizes
config APP_SHELL_STACK_SIZE
    int "t_shell stack size in bytes"
    default 1024

config APP_PLANNER_STACK_SIZE
    int "t_planner stack size in bytes"
    default 2048   # motion_planner local state + kinematics floats

config APP_PULSE_MGR_STACK_SIZE
    int "t_pulse_mgr stack size in bytes"
    default 1024

# Machine profile
config APP_DEFAULT_PROFILE
    int "Default machine profile (0=CNC_2AXIS, 1=CNC_3AXIS, 2=PRINTER_2AXIS, 3=PRINTER_3AXIS)"
    default 0

# Phase-2 feature gates (disabled for MVP)
config APP_ENABLE_ISR_EVENTS
    bool "Enable evt_q_isr: step-done/stall events from ISR to Lane C thread"
    default n
    # n = MVP: Lane C updates position counters synchronously (simpler, no stall detect)
    # y = Phase-2: ISR event queue active; enables stall detection + async telemetry

config APP_ENABLE_FS
    bool "Enable filesystem support for 'run sd:/file.gcode' command"
    default n

config APP_ENABLE_ARC
    bool "Enable G2/G3 arc motion commands"
    default n

config APP_ENABLE_TEMP_WAIT
    bool "Enable M109/M190 wait-for-temperature commands"
    default n
```

**Total static memory budget (default config):**

| Buffer | Calculation | Bytes |
|---|---|---|
| `cmd_q` backing | 64 × 32 | 2 048 |
| `seg_q` backing | 64 × 24 | 1 536 |
| `evt_q_isr` backing | 8 × 4 | 32 |
| `g_cmd_q_buf[]` + `g_seg_q_buf[]` | above | — |
| `shell_stack` | 1 024 | 1 024 |
| `planner_stack` | 2 048 | 2 048 |
| `pulse_stack` | 1 024 | 1 024 |
| Line parser buf (stack in t_shell) | 96 | 96 |
| **Total** | | **~7 808 bytes** |

---

## App Glue Layer (RobotOS Build)

### Explanation — Role of the RobotOS Glue Layer

`app_glue_robotos.c` is the composition root for the reference app on RobotOS:
- **Materializes runtime:** IPC queues, threads, device bindings, priority assignment, deadline registration
- **Knows RobotOS details:** This is the ONLY place in the application layer allowed to include `<robotos/*.h>` (Framework/Adapter headers). Zephyr headers (`<zephyr/*.h>`) are NOT included here — they remain solely in the Adapter layer.
  <!-- STATUS: LOCKED DEC-06 -->
- **Orchestrates integration:** App Core logic is bound into RobotOS runtime — callbacks set up, thread creation orchestrated, telemetry hooks installed
- **Discipline:** Must stay focused on orchestration/integration — must NOT silently absorb core domain semantics or hide policy decisions in glue code
- **Portability proof:** Swapping `app_glue_robotos.c` for `app_glue_zephyr.c` or `app_glue_freertos.c` without touching App Core files demonstrates true portability

---

`app_glue_robotos.c` wires together:

1. **IPC init** — `ro_queue_create` for `cmd_q`, `seg_q`, `evt_q_isr`
2. **Thread creation** — `ro_thread_create` for 3 lanes (see Thread Roster)
3. **Framework calls** — `stepper_get`, `endstop_get`, `encoder_get` during homing
4. **SM bridge** — `app_sm_set_callback` → drives Framework `sm_dispatch` on transitions
5. **Deadline objects** — `ro_deadline_t` structs declared at Glue scope with `.budget_us` and `.name` set;
   passed by pointer to App Core helpers (App Core MUST NOT declare deadline structs directly)
   <!-- STATUS: LOCKED DEC-05 -->
6. **Trace hooks** — `ro_trace_event` on seg enqueue/dequeue, state transitions

**Portability architecture — glue swappability (secondary validation path):**

The architecture supports multiple glue implementations. The primary build for alpha is RobotOS (`app_glue_robotos.c`). Alternative glue implementations (Zephyr-native, FreeRTOS) represent comparison targets and future validation paths:

| File | RTOS API used | App Core touched? | Status |
|---|---|---|---|
| `app_glue_robotos.c` | `ro_thread_create`, `ro_queue_*`, `ro_timer_*`, `ro_deadline_*` | ❌ No | **Primary (alpha)** |
| `app_glue_zephyr.c` | `k_thread_create`, `k_msgq_*`, `k_timer_*` (no deadline) | ❌ No | Future |
| `app_glue_freertos.c` | `xTaskCreate`, `xQueueSend/Receive`, `vTaskDelayUntil` (no deadline) | ❌ No | Future |

When implemented, identical `app_sm.*`, `motion_planner.*`, `gcode_parser.*`, `kinematics_cartesian.*`
files will compile unchanged across all glue variants. This demonstrates true portability.
**MVP gate:** RobotOS build must be production-ready. Zephyr/FreeRTOS alternate glues are optional phase-2+ activities.

**SYS_INIT boot order (RobotOS build):**

```
POST_KERNEL:
    ro_trace_init       (priority 60)
    ro_log_init         (priority 61)
    stepper_drv_bind_all (priority 80–84)
    servo/encoder/endstop/sensor bind (priority 81–84)

APPLICATION:
    app_ipc_init        (priority  5)  ← create cmd_q, seg_q, evt_q_isr
    app_threads_start   (priority 10)  ← create t_shell, t_planner, t_pulse_mgr
    app_sm_dispatch(APP_CMD_INIT_DONE) (priority 15)  ← transitions BOOT → IDLE
```

---

## Shell Commands & Debug UX

Lane A handles these commands as meta-commands (before G-code parse attempt):

### Motion Commands

| Command | Effect | App SM transition |
|---|---|---|
| `home` | Start homing sequence | IDLE → HOMING |
| `move X<n> Y<n> Z<n> F<n>` | Shorthand: enqueues G1 command | (READY/RUNNING) |
| `run` | Resume segment production | PAUSED → RUNNING |
| `stop` | Graceful stop (drain, go READY) | RUNNING/PAUSED → READY |
| `pause` | Feed hold | RUNNING → PAUSED |
| `resume` | Release feed hold | PAUSED → RUNNING |
| `clear_error` | Acknowledge fault | ERROR → IDLE |

### Status & Debug Commands

| Command | Output |
|---|---|
| `status` | Current app state, position (steps + mm), feedrate, spindle on/off |
| `deadline stats` | Miss counters for `g_dl_planner_tick` and `g_dl_pulse_service` |
| `stepper stats` | Steps requested / completed / stall events per axis |
| `trace on` | Enable `ro_trace_*` event emission (Chrome Trace JSON) |
| `trace off` | Disable |
| `profile <n>` | Switch machine profile at runtime (IDLE state only) |
| `stress 30` | Spam G1 commands for 30 seconds → measure deadline miss rate |
| `heap check` | Assert zero heap usage (calls `ro_pool_stats` for all pools) |
| `roster` | Print thread roster dump via `ro_thread_roster_dump()` |
| `version` | Print build version, RobotOS version, Zephyr version, build timestamp |

### Status Output Format

```
> status
state:    RUNNING
pos:      X=12.50mm (1000 steps)  Y=8.75mm (700 steps)  Z=0.00mm (0 steps)
feedrate: 3000 mm/min
spindle:  OFF
seg_q:    23/64 segments queued
cmd_q:    5/64 commands queued
feed_hold: 0
```

---

## Deadline Monitoring

App-layer deadline objects are declared as `ro_deadline_t` structs at Glue scope
(`app_glue_robotos.c`). App Core functions that need them receive them as pointer parameters —
App Core MUST NOT declare `ro_deadline_t` structs directly (DEC-05 Option A).

```c
// include/app/app_deadlines.h — deadline object forward declarations
// App Core may #include this header and use the extern pointers to call
// ro_deadline_begin / ro_deadline_end WITHOUT depending on <robotos/*.h>.
#ifndef APP_DEADLINES_H
#define APP_DEADLINES_H

// Forward declaration only — ro_deadline_t defined in <robotos/ro_deadline.h>
// which App Core must NOT include directly.
struct ro_deadline;

// Deadline objects defined in app_glue_robotos.c; extern for App Core use.
// <!-- STATUS: LOCKED DEC-05 -->
extern struct ro_deadline  g_dl_planner_tick;    // Lane B: 1 kHz tick budget
extern struct ro_deadline  g_dl_pulse_service;   // Lane C: step-done handling budget
extern struct ro_deadline  g_dl_control_rx_parse;// Lane A: per-line parse latency (optional)

#endif  // APP_DEADLINES_H
```

**Declaration in Glue (App Glue owns instantiation):**

```c
// app_glue_robotos.c — deadline objects declared here ONLY (DEC-05)
#include <robotos/ro_deadline.h>
#include "app/app_deadlines.h"

ro_deadline_t g_dl_planner_tick = {
    .budget_us = (1000000 / CONFIG_APP_PLANNER_TICK_HZ) * 80 / 100,
    // Budget = 80% of tick period (e.g. 800 µs for 1 kHz).
    // 20% headroom for context switch + preemption by Lane C.
    .name = "planner_tick",
};

ro_deadline_t g_dl_pulse_service = {
    .budget_us = 50,   // Lane C step-done handler must complete in < 50 µs
    .name      = "pulse_service",
};
```

**Usage in Lane B loop:**

```c
ro_deadline_begin(&g_dl_planner_tick);

// ... planner work ...

ro_deadline_end(&g_dl_planner_tick);
// Internally: if (now - begin) > budget → miss_counter++; ro_trace_event(DEADLINE_MISS)

uint32_t tick_overruns = ro_timer_sync(&g_planner_tick);
if (tick_overruns > 0) {
    ro_log(RO_LOG_WARN, "APP", "planner: %u tick overrun(s)", tick_overruns);
}
```

**`deadline stats` output:**

```
> deadline stats
planner_tick        budget=800µs  misses=0   last_duration=42µs   max_duration=318µs
pulse_service       budget= 50µs  misses=0   last_duration=8µs    max_duration=22µs
control_rx_parse    budget=  5ms  misses=0   last_duration=110µs  max_duration=210µs
```

---

## Portability Story — 3 Builds

> **RobotOS build scope:** The RobotOS build (`app_glue_robotos.c`) is the **primary demonstration** for alpha and must be production-ready. Zephyr-native (`app_glue_zephyr.c`) and FreeRTOS (`app_glue_freertos.c`) builds represent a architectural seam and future-validation paths. The comparison story is an architectural *option*, not an immediate commitment — Zephyr/FreeRTOS glue implementations are phase-2+ activities and only pursued if multi-OS validation becomes a release requirement.

```
app/core/          ← compiled IDENTICALLY by all 3 builds
  gcode_parser.*
  motion_planner.*
  kinematics_cartesian.*
  app_sm.*
  config_profiles.*
  motion_seg.h
  app_deadlines.h

app/glue/
  app_glue_robotos.c    → RobotOS Demo binary       (PRIMARY)
  app_glue_zephyr.c     → Zephyr-native Demo binary (comparison/future)
  app_glue_freertos.c   → FreeRTOS Demo binary      (comparison/future)
```

**Comparison metrics to measure between the 3 binaries (when all glues implemented):**

| Metric | Where measured | Expected winner |
|---|---|---|
| Planner tick jitter (σ, µs) | `deadline stats` | RobotOS ≤ Zephyr ≪ FreeRTOS |
| Deadline miss rate @ 1kHz | `deadline stats` | RobotOS = 0; others TBD |
| Priority sign bug risk | Code review: raw int vs `ro_priority_t` | RobotOS enforces at compile time |
| Thread roster visibility | shell `roster` | Only RobotOS has it |
| Lines of glue per RTOS | wc -l app_glue_*.c | Roughly equal |
| Code size of App Core | `size` on binary | Identical (same .o files) |

---

## Testability

### Host Unit Tests (App Core — portable C)

These tests run on the build host with no QEMU and no RTOS:

| Test file | Coverage |
|---|---|
| `tests/host/test_gcode_parser.c` | All G-code types, edge cases, error lines |
| `tests/host/test_motion_planner.c` | Segment generation, position accumulation, boundary moves |
| `tests/host/test_kinematics_cartesian.c` | steps_per_mm conversion, axis_mask correctness |
| `tests/host/test_app_sm.c` | All state transitions, invalid transitions return -1 |

### HIL Tests (on-board, virtual stepper)

Run on board with `CONFIG_APP_VIRTUAL_STEPPER=y` — stepper pulses are counted in software
instead of driving real motors:

| Test | What it verifies |
|---|---|
| `hil_homing` | G28 sequence: stepper moves, endstop triggers, backoff, READY state |
| `hil_linear_move` | G1 segment: correct step count per axis, feedrate within 1% |
| `hil_stress_30s` | 30 s G-code burst at max ACK rate — see pass criteria below |
| `hil_feed_hold` | Pause mid-motion: segment drain completes, position accurate on resume |

**`hil_stress_30s` pass criteria:**
- `g_dl_planner_tick` miss count == 0 (tolerance: ≤ 1 for warm-up tick only)
- `g_dl_pulse_service` miss count == 0
- Zero heap allocations after `robotos_init()` (verified via `heap check` shell command)
- Backpressure observable when `seg_q` saturates — Lane B logs warning, no crash, no lost ACK

### Test Helper: Virtual Stepper

```c
// CONFIG_APP_VIRTUAL_STEPPER=y — compiled in test builds only
// Intercepts step pin toggle ISR; increments counters instead.
uint32_t virtual_stepper_get_count(uint8_t axis);
void     virtual_stepper_reset(void);
```

---

## Error Model

App Core functions use plain `int` return codes (no `ro_status_t` — App Core is OS-agnostic):

| Return | Meaning |
|---|---|
| `0` | Success |
| `-1` | Generic failure (parse error, invalid state, zero displacement) |
| `1` (from `motion_planner_feed`) | Segment produced in `*seg_out` |
| `0` (from `motion_planner_feed`) | No segment (pure mode command: G90/G91/M-codes) |

App Glue functions use `ro_status_t` when calling Adapter APIs and translate to App Core
conventions at the boundary:

```c
// Glue translates ro_status_t → app log + app_sm_dispatch(APP_CMD_FAULT)
#define APP_RO_CHECK(s, ctx)  \
    do {                       \
        if ((s) != RO_OK) {    \
            ro_log(RO_LOG_ERR, "APP", ctx ": error %d", (int)(s)); \
            app_sm_dispatch(g_app_sm, APP_CMD_FAULT);               \
            return;            \
        }                      \
    } while (0)
```

**Faults escalate through `app_sm_dispatch(APP_CMD_FAULT)` → `APP_STATE_ERROR`.**
The Framework SM receives `CMD_FAULT` via the `app_sm_cb_t` callback registered in Glue.
Hardware is halted (steppers stopped) before the shell prints the error message.

---

## Implementation Checklist (DoD)

> Use as a **Definition of Done** gate before closing each implementation milestone.

### App Core (portable)
- [ ] All App Core files build with **zero OS headers** — CI `ro_check_app_core_includes` passes
- [ ] `gcode_parse_line` is a pure function with no global side-effects (unit-testable on host)
- [ ] `_Static_assert(sizeof(motion_seg_t) == 24)` compiles without error on all target triples
- [ ] All 4 host unit tests pass: `test_gcode_parser`, `test_motion_planner`, `test_kinematics_cartesian`, `test_app_sm`

### Threads & IPC
- [ ] Exactly **3 application threads** created — roster matches CI `ro_check_thread_roster` whitelist
- [ ] `cmd_q` and `seg_q` use **app-owned static backing buffers** (not `RO_GLOBAL_IPC_SLAB`)
- [ ] All deadline objects (`g_dl_planner_tick`, `g_dl_pulse_service`) declared as `ro_deadline_t` in `app_glue_robotos.c` — no `ro_deadline_register()` calls (DEC-05 Option A)
- [ ] `APP_RO_CHECK` macro used at every Adapter API call site in Glue (no silent error drops)

### Real-Time
- [ ] Lane B uses `ro_timer_sync()` at 1 kHz — `ro_thread_sleep_ms` is absent from the planner loop
- [ ] Lane B brackets each tick with `ro_deadline_begin/end(&g_dl_planner_tick)` (DEC-05)
- [ ] Lane C ISR calls only `ro_queue_send_isr` — no blocking calls, no Framework API calls
- [ ] `pause/resume` driven by C11 `_Atomic int g_feed_hold` — no mutex, no RTOS primitive

### Safety & Correctness
- [ ] `seg.axis_mask != 0x00` asserted by Lane C before every segment load
- [ ] `RO_QUEUE_WAIT_FOREVER` is **never** used on `seg_q` from Lane B
- [ ] Every fatal error path reaches `app_sm_dispatch(APP_CMD_FAULT)` → `APP_STATE_ERROR`

### Feature & Integration
- [ ] G-code streaming uses ACK-per-line `ok\n` (host must wait for ACK before next line)
- [ ] `deadline stats` shell command reports live miss counters for all deadline objects
- [ ] `hil_stress_30s` HIL test meets all pass criteria (see [Testability](#testability))
- [ ] `heap check` confirms zero heap usage post-`robotos_init()`

---

**Document Status:** 🚧 Living Document
**Layer:** Application (domain programs; depends on Framework + Adapter)
**Related:** [FRAMEWORK_LAYER.md](FRAMEWORK_LAYER.md) | [ADAPTER_LAYER.md](ADAPTER_LAYER.md) | [KERNEL_LAYER.md](KERNEL_LAYER.md) | [ARCHITECTURE.md](ARCHITECTURE.md)
**Last Review:** 2026-03-05
