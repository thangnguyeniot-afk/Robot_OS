# RobotOS Platform Layer — Phase 5C

## Purpose

The `platform/` directory holds backend-specific implementations of
portable platform service interfaces. It is the **only** place where
Zephyr, board, or OS-specific headers are included in normal firmware builds.

Core and other portable RobotOS modules may include platform interface
headers (e.g. `robotos_platform_log.h`) but must not include backend
headers such as `<zephyr/logging/log.h>` directly.

---

## Phase 5A Scope: Logging Boundary Only

Phase 5A introduces the minimal platform logging interface. See Phase 5B for time.

### Boundary rule

| Where | May include |
|-------|-------------|
| `core/` | `platform/robotos_platform_log.h` only |
| `platform/robotos_platform_log.h` | Standard C headers only (`<stdarg.h>`) |
| `platform/zephyr/robotos_platform_log_zephyr.c` | Zephyr logging headers + platform interface |
| `tests/host/robotos_platform_log_host_stub.c` | Platform interface only — no Zephyr |

### File structure

```
platform/
├── robotos_platform_log.h              ← portable interface (Phase 5A)
├── zephyr/
│   └── robotos_platform_log_zephyr.c  ← Zephyr backend (devkit builds)
└── README.md                           ← this file

tests/host/
└── robotos_platform_log_host_stub.c   ← host no-op backend (host tests)
```

### API summary

```c
typedef enum {
    ROBOTOS_LOG_LEVEL_DEBUG = 0,
    ROBOTOS_LOG_LEVEL_INFO,
    ROBOTOS_LOG_LEVEL_WARN,
    ROBOTOS_LOG_LEVEL_ERROR,
} robotos_log_level_t;

void robotos_platform_logf(robotos_log_level_t level,
                           const char *module,
                           const char *fmt, ...);
```

---

## Phase 5B Scope: Time Boundary

Phase 5B introduces the minimal platform time interface: sleep and uptime.

### Boundary rule

| Where | May include |
|-------|-------------|
| `devkit/runtime` | `platform/robotos_platform_time.h` |
| `platform/robotos_platform_time.h` | `<stdint.h>` only |
| `platform/zephyr/robotos_platform_time_zephyr.c` | `<zephyr/kernel.h>` + platform interface |
| `tests/host/robotos_platform_time_host_stub.c` | Platform interface only — no Zephyr |
| `core/` | Must NOT include `robotos_platform_time.h` — core does not sleep |

### File structure

```
platform/
├── robotos_platform_time.h             ← portable interface (Phase 5B)
├── zephyr/
│   ├── robotos_platform_log_zephyr.c   ← log backend
│   └── robotos_platform_time_zephyr.c  ← time backend (Phase 5B)
└── README.md

tests/host/
├── robotos_platform_log_host_stub.c    ← log no-op
└── robotos_platform_time_host_stub.c   ← time fake stub (Phase 5B, not yet wired)
```

### API summary

```c
uint32_t robotos_platform_uptime_ms(void);
void     robotos_platform_sleep_ms(uint32_t duration_ms);
```

### Design rules

- Core tick remains externally driven. `robotos_core_tick()` is called by the
  devkit runtime loop. Core never calls `robotos_platform_sleep_ms`.
- `sleep_ms(0)` returns immediately in all backends.
- `uptime_ms()` wraps at `UINT32_MAX` (~49.7 days). Wraparound not handled.
- `devkit_runtime.c` uses `robotos_platform_sleep_ms(DEVKIT_TICK_MS)` instead
  of `k_msleep` directly.

---

## Phase 5C Scope: Assert/Fault Boundary

Phase 5C introduces the minimal platform fault/assert interface: severity-graded
fault reporting and a non-panicking assert.

### Boundary rule

| Where | May include |
|-------|-------------|
| `devkit/` | `platform/robotos_platform_fault.h` |
| `platform/robotos_platform_fault.h` | `<stdbool.h>` only |
| `platform/zephyr/robotos_platform_fault_zephyr.c` | `robotos_platform_log.h` + optional `<zephyr/kernel.h>` |
| `tests/host/robotos_platform_fault_host_stub.c` | Platform interface + stub header — no Zephyr |
| `core/` | Must NOT include `robotos_platform_fault.h` — core uses return codes |

### File structure

```
platform/
├── robotos_platform_fault.h            ← portable interface (Phase 5C)
├── zephyr/
│   ├── robotos_platform_log_zephyr.c   ← log backend
│   ├── robotos_platform_time_zephyr.c  ← time backend
│   └── robotos_platform_fault_zephyr.c ← fault backend (Phase 5C)
└── README.md

tests/host/
├── robotos_platform_log_host_stub.c    ← log no-op
├── robotos_platform_time_host_stub.c   ← time fake stub (not yet wired)
├── robotos_platform_fault_host_stub.h  ← test-inspection API (Phase 5C)
├── robotos_platform_fault_host_stub.c  ← fault tracking stub (Phase 5C)
└── test_robotos_platform_fault_contract.c ← 12 contract tests (Phase 5C)
```

### API summary

```c
typedef enum {
    ROBOTOS_FAULT_INFO    = 0,
    ROBOTOS_FAULT_WARNING = 1,
    ROBOTOS_FAULT_ERROR   = 2,
    ROBOTOS_FAULT_FATAL   = 3,
} robotos_fault_severity_t;

void robotos_platform_fault_report(robotos_fault_severity_t severity,
                                   const char *module,
                                   const char *message);

bool robotos_platform_assert(bool condition,
                              const char *module,
                              const char *message);

#define ROBOTOS_PLATFORM_ASSERT(cond, module, msg) ...
```

### Design rules

- Core must NOT call fault APIs. Core errors are returned as status codes.
- `ROBOTOS_PLATFORM_ASSERT(cond, ...)` reports ERROR severity and returns `false`
  when `cond` is false. It does NOT panic.
- `fault_report(FATAL, ...)` logs but does NOT panic unless
  `ROBOTOS_PLATFORM_FAULT_PANIC_ON_FATAL` is defined at build time (default OFF).
- Severity → log level: INFO→INFO, WARNING→WARN, ERROR→ERROR, FATAL→ERROR.
- `devkit_fault.c` is a devkit-layer component and uses Zephyr logging directly.
  It was not wired to the platform fault API (devkit is not portable core).

---

## Backend Selection

The backend is selected at build time by compiling the appropriate source:

| Build target | Backend source | Backend behavior |
|---|---|---|
| devkit (Zephyr) | `platform/zephyr/robotos_platform_log_zephyr.c` | Routes to Zephyr LOG_INF/WRN/DBG/ERR via 128-byte vsnprintf buffer |
| host tests | `tests/host/robotos_platform_log_host_stub.c` | No-op — no output, no Zephyr |

---

## What is Intentionally NOT Abstracted Yet

The following platform services are candidates for future phases but are
**not** implemented here:

| Service | Candidate phase |
|---|---|
| Assert / fault handler | Phase 5C — **done** |
| Critical section / ISR lock | Future |
| Memory policy | Future |
| Thread / mutex | Future |
| Monotonic 64-bit time / timers | Future |

Do not add these here without an approved phase task.

---

## Invariants

- `robotos_platform_log.h` must remain free of Zephyr, k_*, GPIO, DTS,
  and board-specific types.
- `LOG_MODULE_REGISTER` must live in the Zephyr backend, not in core.
- Host tests must compile and pass without Zephyr SDK or `ZEPHYR_BASE`.
- Legacy `RobotOS_v1.0/src/` must never be compiled or included.
