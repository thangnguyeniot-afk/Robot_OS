# RobotOS Robot Framework Layer (Active Path)

> **STATUS:** **Phase 12D — header stub only. No implementation exists.**
> This directory is the **active** Robot Framework path for RobotOS
> Phase 12+ and belongs to **Architecture A**. It consumes the active
> `core/` + `platform/` contracts, **not** the legacy `ro_*` HAL.

---

## Layer identity

This directory is the canonical home of the RobotOS Robot Framework
layer as authorized by Phase 12D after Phase 12D-pre
(`LEGACY_SCAFFOLD_MARKED_FROZEN_DOCS_ONLY`).

| Item | Value |
|---|---|
| Architecture | **A** (active / evidence-backed) |
| Sibling of | `RobotOS_v1.0/core/`, `RobotOS_v1.0/platform/`, `RobotOS_v1.0/devkit/` |
| Type system | `robotos_core_status_t`, `robotos_event_t`, `robotos_platform_*` |
| Created at | **Phase 12D** (header stub) |
| Implementation status | **NOT BUILT.** No `.c` file exists. No CMake integration. |
| Hardware evidence | **None.** Phase 12D adds no `.c` body and no runtime path. |

---

## What this directory contains (Phase 12D)

```
RobotOS_v1.0/framework/
├── README.md            (this file — layer identity, Phase 12D header stub)
└── robotos_fw_fsm.h     (Phase 12D header stub; DRAFT / EXPERIMENTAL API)
```

**No `.c` file. No `CMakeLists.txt`. No `src/` subdirectory.** Phase 12D
deliberately stops at the header surface. An implementation (`.c` body,
CMake wiring, devkit integration) is a separate later phase.

---

## Distinction from legacy Architecture B

This directory is **architecturally distinct** from the frozen legacy
scaffold at:

- `RobotOS_v1.0/src/framework/` — frozen legacy Robot Framework
  scaffold (`ro_status_t`, `ro_*` HAL, IDLE/HOMING/RUN/FAULT vocabulary);
  classified `LEGACY_SCAFFOLD_MARKED_FROZEN_DOCS_ONLY` at Phase 12D-pre.
  See [`../src/framework/DEPRECATED.md`](../src/framework/DEPRECATED.md).
- `RobotOS_v1.0/include/robotos/` — frozen legacy include namespace
  bundling `ro_*` HAL and Architecture-B Framework headers.
  See [`../include/robotos/DEPRECATED.md`](../include/robotos/DEPRECATED.md).
- `RobotOS_v1.0/CMakeLists.txt` (root) — legacy build entry for
  Architecture B.

The two architectures are **type-incompatible**:

| Surface | Legacy (Architecture B) | Active (this directory, Architecture A) |
|---|---|---|
| Status type | `ro_status_t` | `robotos_core_status_t` |
| Event model | `ro_queue_t` (zero-copy, ISR-safe) | `robotos_event_t` + core dispatcher; FSM consumes logical `robotos_fw_event_id_t` |
| Sync primitive | `ro_mutex_*` | `robotos_platform_critical_enter/exit` |
| Time | `ro_time_now_*` | `robotos_platform_uptime_ms`, `robotos_platform_sleep_ms` |
| FSM | `robot_sm_*` with product-coupled `{IDLE, HOMING, RUN, FAULT}` | `robotos_fw_fsm_*` with product-neutral `uint32_t` state and event IDs |

No file under this directory may include any header from
`include/robotos/` or call any `ro_*` symbol. The boundary is enforced
by inclusion discipline.

---

## What does NOT exist yet

Phase 12D explicitly **does not** create:

- Any `.c` file under this directory.
- A `framework/src/` subdirectory.
- A `framework/CMakeLists.txt` or any CMake integration.
- Any link from `devkit/CMakeLists.txt` into this directory.
- Any include-path entry in the devkit build referencing this directory.
- Any UART command exposed through this layer.
- Any Application/product layer.
- Any replacement for `devkit_app_state` (which remains devkit-local;
  scope-guard #11 re-affirmed).
- Any new `ROBOTOS_FW` RTT telemetry stream.
- Any hardware evidence run.

A future Phase (12E or later) may add an implementation, but it must
not be opened without:

1. **Explicit user authorization**, and
2. A concrete consumer (devkit integration target) or unit-test plan
   identified in advance.

---

## Public header

The single public header in Phase 12D is:

- [`robotos_fw_fsm.h`](robotos_fw_fsm.h)

It declares (as Phase 12D **header stub / DRAFT / EXPERIMENTAL**) the
flat, table-driven, product-neutral FSM API confirmed at
Phase 12B/12C. It includes only:

- `<stdbool.h>`
- `<stdint.h>`
- `"robotos_core.h"` (relative include; resolved via the active build
  include path)

It does **not** include any Zephyr, devkit, app, or legacy `ro_*` header.

---

## How to consume this header (future, not Phase 12D)

A future implementation phase (not Phase 12D) is expected to:

1. Add a `robotos_fw_fsm.c` next to the header.
2. Add a `framework/CMakeLists.txt` building a static library.
3. Wire that library into `RobotOS_v1.0/devkit/CMakeLists.txt`
   (Architecture A build) **only**, not into the root legacy
   `CMakeLists.txt`.
4. Identify a concrete consumer (devkit integration target) or unit
   test before opening implementation.
5. Lock the API at that point (move §4 of
   [`../devkit/docs/03_SPECS/FRAMEWORK_FSM_API_DRAFT.md`](../devkit/docs/03_SPECS/FRAMEWORK_FSM_API_DRAFT.md)
   from `LOCKED-AT-12D` to a hardened state).

None of the above is in scope for Phase 12D.

---

## Spec and history

- Phase 12D header stub: this directory + the closeout doc
  [`../devkit/docs/02_PHASE_CLOSEOUTS/PHASE_12D_FSM_HEADER_STUB.md`](../devkit/docs/02_PHASE_CLOSEOUTS/PHASE_12D_FSM_HEADER_STUB.md).
- Long-lived spec (DRAFT, evolving):
  [`../devkit/docs/03_SPECS/FRAMEWORK_FSM_API_DRAFT.md`](../devkit/docs/03_SPECS/FRAMEWORK_FSM_API_DRAFT.md).
- Phase 12C event bridge + status model confirmation:
  [`../devkit/docs/02_PHASE_CLOSEOUTS/PHASE_12C_FSM_EVENT_BRIDGE_STATUS_MODEL.md`](../devkit/docs/02_PHASE_CLOSEOUTS/PHASE_12C_FSM_EVENT_BRIDGE_STATUS_MODEL.md).
- Phase 12B initial FSM draft:
  [`../devkit/docs/02_PHASE_CLOSEOUTS/PHASE_12B_FRAMEWORK_FSM_API_DRAFT.md`](../devkit/docs/02_PHASE_CLOSEOUTS/PHASE_12B_FRAMEWORK_FSM_API_DRAFT.md).
- Phase 12A surface planning:
  [`../devkit/docs/02_PHASE_CLOSEOUTS/PHASE_12A_FRAMEWORK_API_SURFACE_PLANNING.md`](../devkit/docs/02_PHASE_CLOSEOUTS/PHASE_12A_FRAMEWORK_API_SURFACE_PLANNING.md).
- Phase 12D-pre legacy disposition:
  [`../devkit/docs/02_PHASE_CLOSEOUTS/PHASE_12D_PRE_LEGACY_FRAMEWORK_SCAFFOLD_DISPOSITION.md`](../devkit/docs/02_PHASE_CLOSEOUTS/PHASE_12D_PRE_LEGACY_FRAMEWORK_SCAFFOLD_DISPOSITION.md).

---

## Stability promise

**None.** The Phase 12D header is marked `DRAFT / EXPERIMENTAL`. ABI is
not stable. Function signatures may change before an implementation is
authorized. Do not depend on this header from Application code yet.
