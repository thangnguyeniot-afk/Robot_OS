# RobotOS Core — Phase 4B Contract

This directory contains the portable RobotOS core module.

## Role in the Architecture

The core layer is the **Kernel/Core seed** of RobotOS. It holds no board assumptions,
no Zephyr types in its public API, and no hardware drivers. It is designed to be
portable across any host environment that can provide a C99 toolchain.

The devkit harness (`RobotOS_v1.0/devkit/`) is the integration host. It:

- calls `robotos_core_init()` once at boot
- calls `robotos_core_tick()` every `DEVKIT_TICK_MS` (500 ms)
- owns all timing, LED, RTT, and board-specific logic
- consumes the core API through the portable public header only

## Lifecycle States

```text
UNINITIALIZED  ──── robotos_core_init() ────>  READY
                                                  │
                                              (tick loop)
                                                  │
                                              [ERROR reserved for future fault conditions]
```

| State | Value | Meaning |
| ----- | ----- | ------- |
| `ROBOTOS_CORE_STATE_UNINITIALIZED` | 0 | Power-on default. `tick()` returns `ERR_INVALID_STATE`. |
| `ROBOTOS_CORE_STATE_READY` | 1 | Core is initialized and accepting ticks. |
| `ROBOTOS_CORE_STATE_ERROR` | 2 | Reserved. Not yet triggered by any code path. |

## API Contract

### Status Codes

```c
typedef enum {
    ROBOTOS_CORE_OK                = 0,   // success
    ROBOTOS_CORE_ERR_INVALID_STATE = -1,  // wrong lifecycle state
    ROBOTOS_CORE_ERR_NULL          = -2,  // NULL pointer argument
} robotos_core_status_t;
```

### Functions

| Function | Contract |
| -------- | -------- |
| `robotos_core_version()` | Returns constant version string. Never NULL. |
| `robotos_core_init()` | Idempotent. First call: state→READY, tick_count=0, init_count=1. Repeat: init_count++, tick_count unchanged, returns OK. |
| `robotos_core_tick()` | Requires state==READY. Increments tick_count. Logs tick 1 and every 10th. Returns ERR_INVALID_STATE if not READY; warns once, then silent. |
| `robotos_core_state()` | Returns current lifecycle state enum. |
| `robotos_core_tick_count()` | Returns current tick_count. Returns 0 if not initialized. |
| `robotos_core_snapshot()` | Copies {state, tick_count, init_count, version} into caller struct. Returns ERR_NULL if out is NULL. |

### Init Idempotency Detail

```text
First call:   state = READY, tick_count = 0, init_count = 1  → LOG_INF (full init message)
Second call:  init_count = 2, tick_count unchanged           → LOG_DBG (suppressed at INF level)
```

`tick_count` is NOT reset on repeated init calls. This is intentional: a re-init
during a running tick loop must not silently discard progress.

### Tick-Before-Init Behavior

If `robotos_core_tick()` is called before `robotos_core_init()`:

- Returns `ROBOTOS_CORE_ERR_INVALID_STATE`
- Emits one `LOG_WRN` ("core tick before init — ignored")
- All subsequent invalid-state ticks are silent (no repeated warnings)
- `tick_count` is not incremented

## Snapshot

`robotos_core_snapshot_t` is a plain struct — no locking, no atomics.

**Thread-safety limitation:** This is valid only in single-threaded contexts.
When RTOS threads are introduced in a future phase, the snapshot function will
need a mutex or atomic snapshot mechanism.

## Phase 4B Scope

**Implemented:**

- Status enum (`robotos_core_status_t`)
- Lifecycle state enum (`robotos_core_state_t`)
- Snapshot struct (`robotos_core_snapshot_t`)
- `robotos_core_state()` introspection
- `robotos_core_tick_count()` introspection
- `robotos_core_snapshot()` introspection
- Init idempotency semantics
- Tick-before-init guard with one-shot warning

**Intentionally absent in Phase 4B:**

- Scheduler
- Event bus / message queue
- RTOS threads
- Peripheral drivers
- Dynamic memory allocation
- Host/unit test layer (planned for a future phase)
- Mutex / atomic protection (single-threaded assumption documented)

## Files

| File | Role |
| ---- | ---- |
| `robotos_core.h` | Public portable API — no Zephyr or board types |
| `robotos_core.c` | Implementation — uses Zephyr logging internally |

## Limitations

- **Single-threaded assumption**: No mutex or atomic operations. Safe only in single-threaded runtime. Revisit when RTOS threads are introduced.
- **Zephyr logging dependency**: `robotos_core.c` uses `LOG_MODULE_REGISTER`. Not portable to bare-metal or host-test environments without a logging abstraction layer.
- **No host-test layer**: Contract semantics cannot be verified outside Zephyr without additional abstraction. Host tests are a future phase concern.
- **No scheduler or event bus**: Phase 4B scope is lifecycle contract only.

## Next Phase

Phase 4C — options for team decision:

- Host/Core Contract Tests (verify init idempotency, tick-before-init, snapshot semantics without hardware)
- Event Queue Stub (first async primitive inside `core/`)
