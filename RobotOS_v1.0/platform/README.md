# RobotOS Platform Layer — Phase 5A

## Purpose

The `platform/` directory holds backend-specific implementations of
portable platform service interfaces. It is the **only** place where
Zephyr, board, or OS-specific headers are included in normal firmware builds.

Core and other portable RobotOS modules may include platform interface
headers (e.g. `robotos_platform_log.h`) but must not include backend
headers such as `<zephyr/logging/log.h>` directly.

---

## Phase 5A Scope: Logging Boundary Only

Phase 5A introduces the minimal platform logging interface. No other
platform services are abstracted yet.

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
| Time / tick source | Phase 5B |
| Assert / fault handler | Phase 5B or 5C |
| Critical section / ISR lock | Future |
| Memory policy | Future |
| Thread / mutex | Future |

Do not add these here without an approved phase task.

---

## Invariants

- `robotos_platform_log.h` must remain free of Zephyr, k_*, GPIO, DTS,
  and board-specific types.
- `LOG_MODULE_REGISTER` must live in the Zephyr backend, not in core.
- Host tests must compile and pass without Zephyr SDK or `ZEPHYR_BASE`.
- Legacy `RobotOS_v1.0/src/` must never be compiled or included.
