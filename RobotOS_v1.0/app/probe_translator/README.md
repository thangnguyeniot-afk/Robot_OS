# probe_translator — First application harness (host prototype)

**Status:** Phase 12I host prototype (DRAFT / host-test evidence).

`probe_translator` is the first application harness layered on top of the
Architecture-A Framework. It exercises the locked Framework FSM
(`robotos_fw_fsm`) + event bridge (`robotos_fw_event_bridge`) contract from
an application-owned static instance — no devkit binding, no UART surface,
no Zephyr / hardware runtime.

## Purpose

- Demonstrate that a bounded application harness can be built on top of the
  Architecture-A Framework without touching any devkit or legacy
  Architecture-B surface.
- Provide a host-test-validated reference for the public API shape
  (`probe_translator_init`, `_dispatch_adapter_event`, `_reset`,
  `_get_snapshot`).
- Serve as the planning anchor for future product-level applications under
  `RobotOS_v1.0/app/<product>/`.

## Host-test entry point

```
RobotOS_v1.0/tests/host/test_app_probe_translator_mapping.c
```

Built and run via the additive block in `RobotOS_v1.0/tests/host/CMakeLists.txt`
(see [PROBE_TRANSLATOR_HOST_PROTOTYPE_PLAN.md](../../devkit/docs/03_SPECS/PROBE_TRANSLATOR_HOST_PROTOTYPE_PLAN.md)
§8.1 for the exact CMake block).

```
cmake -S RobotOS_v1.0/tests/host -B build-phase12i-host
cmake --build build-phase12i-host
ctest --test-dir build-phase12i-host --output-on-failure -V
```

## State / event vocabulary (Phase 12I)

| Symbol                              | Value |
|-------------------------------------|-------|
| `PROBE_TRANSLATOR_STATE_IDLE`       | `1u`  |
| `PROBE_TRANSLATOR_STATE_READY`      | `2u`  |
| `PROBE_TRANSLATOR_STATE_ACTIVE`     | `3u`  |
| `PROBE_TRANSLATOR_EVT_CONFIGURED`   | `1u`  |
| `PROBE_TRANSLATOR_EVT_START`        | `2u`  |
| `PROBE_TRANSLATOR_EVT_STOP`         | `3u`  |
| `PROBE_TRANSLATOR_EVT_RESET`        | `4u`  |
| `PROBE_ADAPTER_TYPE_CONFIG`         | `1u`  |
| `PROBE_ADAPTER_TYPE_COMMAND`        | `2u`  |
| `PROBE_ADAPTER_ARG_NONE`            | `0u`  |
| `PROBE_ADAPTER_ARG_START`           | `1u`  |
| `PROBE_ADAPTER_ARG_STOP`            | `2u`  |
| `PROBE_ADAPTER_ARG_RESET`           | `3u`  |

`PROBE_TRANSLATOR_STATE_FAULT`, `PROBE_TRANSLATOR_EVT_FAULT`,
`PROBE_ADAPTER_TYPE_FAULT`, and `PROBE_ADAPTER_ARG_ANY` are deferred to a
future app-behavior phase.

## Non-goals (Phase 12I)

- **No devkit binding.** This module does not include `devkit_app_state.h`,
  does not call any `devkit_*` function, and does not read or write devkit
  runtime state. Scope-guard #11 active.
- **No UART surface.** The frozen `a / s / r / ? / x / v / L / d / T`
  command set is unchanged. `probe_translator` defines no new UART command.
- **No Zephyr runtime.** No `<zephyr/...>` includes, no `prj.conf`, no DTS,
  no Zephyr main entry-point at `app/probe_translator/`. There is no
  `app/probe_translator/CMakeLists.txt` at Phase 12I — the host test target
  lives in `tests/host/CMakeLists.txt` (Option A).
- **No hardware run.** No RTT, J-Link, OpenOCD, or flashing artifacts.
- **No Architecture B reuse.** `src/` and `include/robotos/` remain frozen;
  `probe_translator` uses Architecture-A contracts only (`robotos_fw_*` +
  `robotos_core_status_t`).

## Related docs

- Implementation contract (long-lived spec):
  [`PROBE_TRANSLATOR_HOST_PROTOTYPE_PLAN.md`](../../devkit/docs/03_SPECS/PROBE_TRANSLATOR_HOST_PROTOTYPE_PLAN.md)
- Skeleton draft (planning lineage):
  [`PROBE_TRANSLATOR_APP_SKELETON_DRAFT.md`](../../devkit/docs/03_SPECS/PROBE_TRANSLATOR_APP_SKELETON_DRAFT.md)
- Application boundary spec:
  [`FRAMEWORK_APPLICATION_BOUNDARY_DRAFT.md`](../../devkit/docs/03_SPECS/FRAMEWORK_APPLICATION_BOUNDARY_DRAFT.md)
- Framework FSM API:
  [`FRAMEWORK_FSM_API_DRAFT.md`](../../devkit/docs/03_SPECS/FRAMEWORK_FSM_API_DRAFT.md)
- Framework bridge spec:
  [`FRAMEWORK_APPLICATION_BRIDGE_DRAFT.md`](../../devkit/docs/03_SPECS/FRAMEWORK_APPLICATION_BRIDGE_DRAFT.md)
- Phase 12I closeout:
  [`PHASE_12I_PROBE_TRANSLATOR_HOST_PROTOTYPE.md`](../../devkit/docs/02_PHASE_CLOSEOUTS/PHASE_12I_PROBE_TRANSLATOR_HOST_PROTOTYPE.md)
