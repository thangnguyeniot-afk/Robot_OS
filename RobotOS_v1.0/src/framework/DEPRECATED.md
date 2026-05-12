# `src/framework/` — DEPRECATED Legacy Robot Framework Scaffold

> **NON-AUTHORITATIVE.** This directory is a **frozen legacy Robot
> Framework scaffold** from the original Zephyr devkit bring-up baseline
> (commit `43de448`, 2025-03-05). It is **not** the active path for
> Phase 12+ Robot Framework work. The active Phase 12+ Framework design
> uses a different stack and different types and will live at
> `RobotOS_v1.0/framework/` (created in Phase 12D on explicit user
> authorization; not yet created at the time this notice is written).

---

## Status

| Item | Value |
|---|---|
| Status | **Frozen / non-authoritative legacy scaffold** |
| Baseline commit | `43de448` (2025-03-05) |
| Source evolution since baseline | **Zero.** No `.c` or header here has been modified by any phase since baseline. |
| Hardware evidence | **None.** Zero Phase 1–11 evidence log references any file in this directory. |
| Compiled by Phase 1–11 hardware-validated build | **No.** The active build (`RobotOS_v1.0/devkit/CMakeLists.txt`) does not include this directory. |
| Authoritative for Phase 12+ Framework | **No.** Architecture-incompatible with the Phase 12C-confirmed FSM design. |
| Disposition record | [`RobotOS_v1.0/devkit/docs/02_PHASE_CLOSEOUTS/PHASE_12D_PRE_LEGACY_FRAMEWORK_SCAFFOLD_DISPOSITION.md`](../../devkit/docs/02_PHASE_CLOSEOUTS/PHASE_12D_PRE_LEGACY_FRAMEWORK_SCAFFOLD_DISPOSITION.md) |

---

## Why this directory is non-authoritative

This directory uses the legacy **Architecture B** type system, which is
type-incompatible with the active Phase 12 design:

| Surface | Legacy (this directory) | Active Phase 12+ |
|---|---|---|
| Status type | `ro_status_t` (`int32_t`; `RO_OK = 0`; codes down to `EBOOTSEQ = -23`) | `robotos_core_status_t` (enum from `core/robotos_core.h`) |
| Sync primitive | `ro_mutex_*` | `robotos_platform_critical_enter/exit` |
| Queue / event model | `ro_queue_*` (zero-copy, ISR-safe) | `robotos_event_queue_*` + `robotos_event_dispatcher_*` (fixed-capacity ring, admission gate) |
| Time API | `ro_time_now_us`, `ro_time_now_ms` | `robotos_platform_uptime_ms`, `robotos_platform_sleep_ms` |
| GPIO / PWM / I2C / SPI | `ro_gpio_*`, `ro_pwm_*`, `ro_i2c_*`, `ro_spi_*` | No abstraction yet; Phase 1–11 used Zephyr APIs directly inside devkit modules |
| State machine | `robot_sm_*` with `robot_state_t = {IDLE, HOMING, RUN, FAULT}` and `robot_cmd_t = {HOME, START, STOP, EMERGENCY_STOP, FAULT, HOMING_COMPLETE}` — **product-coupled** state vocabulary | Phase 12B/12C `robotos_fw_fsm_*` with **product-neutral** `uint32_t` state and event IDs; product chooses its own vocabulary |

Mixing the two requires an explicit reconciliation phase that has not been
authorized.

---

## Contents (preserved unchanged by Phase 12D-pre)

```
src/framework/
├── README.md            (original "Robot Framework Layer" intent doc)
├── DEPRECATED.md        (this notice — added by Phase 12D-pre)
├── stepper.c            (Trapezoidal velocity profile)
├── servo.c              (PWM servo)
├── dcmotor.c            (PWM + DIR pin, soft start/stop)
├── encoder.c            (GPIO interrupt counting)
├── endstop.c            (Debounced limit switch)
├── sensor.c             (ADC + scale + offset)
├── pid.c                (Discrete PID with anti-windup)
├── filter.c             (EMA / MA / Notch IIR)
├── limiter.c            (Clamp + slew rate)
├── robot_sm.c           (Table-driven IDLE/HOMING/RUN/FAULT FSM)
└── drivers/
    ├── README.md
    ├── stepper_drv.c    (Bresenham DDA multi-axis ISR)
    └── endstop_drv.c    (Debounce + blocking wait)
```

All files above remain **on disk and tracked in git** unchanged. Phase
12D-pre adds only this `DEPRECATED.md` notice.

---

## Rules

1. **Do not add new Robot Framework work in this directory.** The active
   path for Phase 12+ Robot Framework is `RobotOS_v1.0/framework/` (not yet
   created; pending Phase 12D).
2. **Do not modify the existing files in this directory.** This scaffold is
   frozen at baseline.
3. **Do not delete files from this directory.** Git history is preserved
   in place.
4. **Do not consume this directory's headers** (`include/robotos/stepper.h`,
   `include/robotos/robot_sm.h`, etc.) from Phase 12+ active code. They use
   the legacy `ro_*` HAL which is type-incompatible with the active
   `robotos_core_*` + `robotos_platform_*` stack.
5. **A future reconciliation phase may revisit this scaffold.** Such a
   phase would need to decide whether to: (a) port selected modules to the
   active Architecture A types, (b) keep both architectures in parallel,
   or (c) retire this scaffold entirely. None of those decisions is in
   scope for Phase 12D-pre.

---

## Where to look for active Framework work

| Artifact | Path |
|---|---|
| Active Framework FSM API draft spec | [`../../devkit/docs/03_SPECS/FRAMEWORK_FSM_API_DRAFT.md`](../../devkit/docs/03_SPECS/FRAMEWORK_FSM_API_DRAFT.md) |
| Phase 12A — Framework API surface planning | [`../../devkit/docs/02_PHASE_CLOSEOUTS/PHASE_12A_FRAMEWORK_API_SURFACE_PLANNING.md`](../../devkit/docs/02_PHASE_CLOSEOUTS/PHASE_12A_FRAMEWORK_API_SURFACE_PLANNING.md) |
| Phase 12B — FSM API draft | [`../../devkit/docs/02_PHASE_CLOSEOUTS/PHASE_12B_FRAMEWORK_FSM_API_DRAFT.md`](../../devkit/docs/02_PHASE_CLOSEOUTS/PHASE_12B_FRAMEWORK_FSM_API_DRAFT.md) |
| Phase 12C — Event bridge + status model confirmation | [`../../devkit/docs/02_PHASE_CLOSEOUTS/PHASE_12C_FSM_EVENT_BRIDGE_STATUS_MODEL.md`](../../devkit/docs/02_PHASE_CLOSEOUTS/PHASE_12C_FSM_EVENT_BRIDGE_STATUS_MODEL.md) |
| Phase 12D-pre — This disposition | [`../../devkit/docs/02_PHASE_CLOSEOUTS/PHASE_12D_PRE_LEGACY_FRAMEWORK_SCAFFOLD_DISPOSITION.md`](../../devkit/docs/02_PHASE_CLOSEOUTS/PHASE_12D_PRE_LEGACY_FRAMEWORK_SCAFFOLD_DISPOSITION.md) |
| Future active Framework path | `RobotOS_v1.0/framework/` (will be created in Phase 12D on explicit user authorization) |
