# RobotOS Core — Phase 4A Bootstrap

This directory contains the portable RobotOS core module.

## Phase 4A Scope

Phase 4A is a minimal bootstrap. The core establishes the first clean system
boundary between the devkit-specific runtime harness and portable RobotOS logic.

**Implemented:**
- `robotos_core_init()` — boot-time initialization, logs version
- `robotos_core_tick()` — per-tick lifecycle advance, increments internal counter
- `robotos_core_version()` — returns phase version string

**Intentionally absent in Phase 4A:**
- Scheduler
- Event bus / message queue
- Peripheral drivers
- RTOS threads
- Dynamic memory allocation
- Board-specific logic

## Files

| File | Role |
| ---- | ---- |
| `robotos_core.h` | Public portable API — no Zephyr or board types |
| `robotos_core.c` | Implementation — uses Zephyr logging internally |

## Integration

The devkit runtime (`devkit_runtime.c`) calls:
- `robotos_core_init()` once during boot
- `robotos_core_tick()` every `DEVKIT_TICK_MS` (500 ms)

Timing is owned entirely by the devkit runtime. The core does not use
`k_msleep`, timers, or threads.

## Next Phase

Phase 4B will harden the core contract or introduce the first event queue stub,
depending on team direction after Phase 4A runtime confirmation.
