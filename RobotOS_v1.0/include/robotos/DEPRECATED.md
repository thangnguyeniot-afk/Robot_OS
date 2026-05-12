# `include/robotos/` — DEPRECATED Legacy Include Namespace

> **NON-AUTHORITATIVE NAMESPACE.** This directory is the public-header
> namespace of the **frozen legacy scaffold** under `RobotOS_v1.0/src/`.
> It is **not** the include namespace for active Phase 12+ Robot Framework
> work. Do not add new headers here without an explicit, approved
> reconciliation phase.

---

## Status

| Item | Value |
|---|---|
| Status | **Frozen / non-authoritative legacy namespace** |
| Baseline commit | `43de448` (2025-03-05) |
| Source evolution since baseline | **Zero.** No header here has been modified since baseline. |
| Used by active Phase 1–11 hardware build | **No.** The devkit hardware build does not include this directory in its include path. |
| Active namespace for Phase 12+ Framework | **No.** The Phase 12B/12C Framework FSM design uses a relative include style (`#include "robotos_fw_fsm.h"`) from `RobotOS_v1.0/framework/` (not yet created). |
| Disposition record | [`RobotOS_v1.0/devkit/docs/02_PHASE_CLOSEOUTS/PHASE_12D_PRE_LEGACY_FRAMEWORK_SCAFFOLD_DISPOSITION.md`](../../devkit/docs/02_PHASE_CLOSEOUTS/PHASE_12D_PRE_LEGACY_FRAMEWORK_SCAFFOLD_DISPOSITION.md) |

---

## Why this namespace is non-authoritative

This namespace bundles two header classes from the legacy scaffold:

1. **15 HAL headers** (`ro_*.h`) — `ro_status`, `ro_thread`, `ro_time`,
   `ro_timer`, `ro_queue`, `ro_pool`, `ro_mutex`, `ro_gpio`, `ro_pwm`,
   `ro_i2c`, `ro_spi`, `ro_log`, `ro_trace`, `ro_deadline`, `ro_assert`.
2. **10 Framework headers** — `stepper.h`, `servo.h`, `dcmotor.h`,
   `encoder.h`, `endstop.h`, `sensor.h`, `pid.h`, `filter.h`, `limiter.h`,
   `robot_sm.h`.

The HAL headers use `ro_status_t` and an `ro_*` API surface that is
**type-incompatible** with the active Phase 12+ stack's
`robotos_core_status_t` and `robotos_platform_*` API. The Framework headers
sit on top of that HAL.

Adding new Phase 12+ Framework headers here would require either:

- Maintaining two parallel header namespaces (`#include <robotos/...>`
  for legacy types and `#include "robotos_fw_*.h"` for active types) —
  confusing and error-prone, or
- Reconciling the two type systems — substantial multi-phase work that has
  not been authorized.

Neither is appropriate for Phase 12+ Framework progress.

---

## Rules

1. **Do not add `robotos_fw_*.h` (Phase 12+ Framework headers) here.** The
   active path for new Framework headers is `RobotOS_v1.0/framework/`
   (created in Phase 12D on explicit user authorization).
2. **Do not modify existing headers in this directory.** This namespace is
   frozen at baseline.
3. **Do not delete headers from this directory.** Git history is preserved
   in place.
4. **Do not consume these headers from active Phase 12+ code.** Active code
   under `core/`, `platform/`, `devkit/` does not include any header from
   this namespace, and that boundary must be preserved.

---

## Active include style for Phase 12+ Framework

The Phase 12B/12C-confirmed Framework FSM uses a **relative include**
matching the Architecture A convention:

```c
#include "robotos_fw_fsm.h"        /* future Phase 12D header, in RobotOS_v1.0/framework/ */
#include "robotos_core.h"          /* active stack, in RobotOS_v1.0/core/ */
#include "robotos_platform_critical.h"   /* active stack, in RobotOS_v1.0/platform/ */
```

The angle-bracket forms `<robotos/...>` and `<robotos_framework/...>` are
**not** used for Phase 12+ Framework headers at present. A migration to
angle-bracket namespaced includes is a possible future refinement, but it
will not reuse this legacy `<robotos/...>` namespace.

---

## See also

- [`README.md`](README.md) — original (legacy) intent doc for this namespace.
- [`../README.md`](../README.md) — top-level (legacy) include directory README.
- [`../../src/README_LEGACY_SCAFFOLD.md`](../../src/README_LEGACY_SCAFFOLD.md) — sibling notice for the `src/` legacy scaffold.
- [`../../src/framework/DEPRECATED.md`](../../src/framework/DEPRECATED.md) — sibling notice for the `src/framework/` legacy Framework scaffold.
