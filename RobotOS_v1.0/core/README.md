# RobotOS Core — Phase 5A Platform Logging Boundary

This directory contains the portable RobotOS core module.

## Role in the Architecture

The core layer is the **Kernel/Core seed** of RobotOS. It holds no board assumptions,
no Zephyr types in its public API, and no hardware drivers. It is designed to be
portable across any host environment that can provide a C99 toolchain.

**Phase 5A change:** `robotos_core.c` no longer includes `<zephyr/logging/log.h>`
directly. All diagnostic logging is routed through `robotos_platform_log.h` — the
platform logging interface. The Zephyr backend is compiled separately for devkit
builds; host tests compile the no-op stub. The public API remains Zephyr-free.

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
| `robotos_core.c` | Implementation — logs through `robotos_platform_log.h`; no direct Zephyr dependency |

## Host Contract Tests (Phase 4C)

Core contract semantics are validated without Zephyr, hardware, or `west build` via
a host test binary. This is **Tier 1 validation**: the fastest feedback loop.

**Phase 5A:** Host tests compile `robotos_platform_log_host_stub.c` (no-op backend)
alongside core. `ROBOTOS_CORE_HOST_TEST` is no longer needed and has been removed from
the host test build — the platform log boundary handles backend selection instead.

### How to Run

```bash
# From repo root (requires a host C compiler; verified on Linux/WSL)
cmake -S RobotOS_v1.0/tests/host -B build-host-core
cmake --build build-host-core
ctest --test-dir build-host-core --output-on-failure
```

Expected output:

```text
100% tests passed, 0 tests failed out of 1
```

### Host Test Shim

`robotos_core.c` detects host test mode via `ROBOTOS_CORE_HOST_TEST=1` compile
definition (set by `tests/host/CMakeLists.txt`). In that mode:

- `CORE_LOG_INF/WRN/DBG` macros map to `((void)0)` — no Zephyr headers needed
- `<stdbool.h>` and `<stddef.h>` are included directly (normally provided transitively by Zephyr)
- All contract logic runs identically to firmware mode

The shim is local to `robotos_core.c`. `robotos_core.h` has no Zephyr types.

### Contract Cases Covered (35 tests)

- `version()` non-NULL and equals expected string
- Initial state `UNINITIALIZED`, tick_count `0`
- `snapshot(NULL)` returns `ERR_NULL`
- `tick()` before init returns `ERR_INVALID_STATE`, no tick_count increment
- `snapshot()` before init returns state=UNINITIALIZED, tick_count=0
- First `init()` → state=READY, tick_count=0, init_count=1
- Second `init()` → idempotent, init_count=2, tick_count **not** reset
- `tick()` after init increments monotonically
- Final `snapshot()` reports READY, correct tick_count=16, init_count=2, version

### Limitations (Phase 4C)

- Tests run in a single sequential process — no reset API needed; tests order-dependent by design
- No concurrency/thread-safety validation yet
- Logging behavior (one-shot WRN for tick-before-init) not tested; private static not exposed
- Zephyr integration still validated through devkit (Phase 3B–4B hardware evidence)
- Windows native host build blocked: broken MinGW installation on dev machine; WSL Ubuntu used as workaround

## Limitations

- **Single-threaded assumption**: No mutex or atomic operations. Safe only in single-threaded runtime. Revisit when RTOS threads are introduced.
- **Platform logging boundary**: `robotos_core.c` logs through `robotos_platform_log.h`. No direct Zephyr dependency in core.
- **No scheduler or event bus**: Phase 4B/4C scope is lifecycle contract only.

## Next Phase

Phase 4D — options for team decision:

- **Event Queue Contract Stub** — first async primitive inside `core/`; Phase 4C host test framework is the foundation
- **Core Concurrency/Host Test Expansion** — add thread-safety validation before introducing RTOS threads
