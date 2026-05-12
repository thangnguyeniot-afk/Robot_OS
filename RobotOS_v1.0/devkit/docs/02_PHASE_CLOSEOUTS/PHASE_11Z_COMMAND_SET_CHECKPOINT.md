# Phase 11Z -- Command-Set Checkpoint

**Status:** `CLOSED_DOCS_ONLY`
**Type:** Docs-only checkpoint / design-state consolidation. **No
source, runtime, test, CMake, Zephyr, board, host-tool, script, or
`prj.conf` change.** Phase 11Z does **not** open the ACTIVE disarm
widening, does **not** introduce any new command, and does **not**
modify any closed phase's evidence.
**Date opened/closed:** 2026-05-12 (same-day docs-only close)
**Branch:** master
**Published baseline at open:** `origin/master = 10710b3`
**Prior runtime implementation phase:** Phase 11D (firmware `2040bfb`).
**Prior hardware evidence phase:** Phase 11E (`10710b3`).
**Prior docs-only checkpoint:** Phase 10C (`CLOSED_DOCS_ONLY`) — the
post-10B-d non-sensor command-set checkpoint.
**Companion docs:**
[`COMMAND_SET_DRAFT.md`](../03_SPECS/COMMAND_SET_DRAFT.md),
[`PHASE_10C_COMMAND_SET_CHECKPOINT.md`](PHASE_10C_COMMAND_SET_CHECKPOINT.md),
[`PHASE_9EZ_CHECKPOINT.md`](PHASE_9EZ_CHECKPOINT.md),
[`PHASE_11A_ADAPTER_BOUNDARY_SENSOR_SURFACE.md`](PHASE_11A_ADAPTER_BOUNDARY_SENSOR_SURFACE.md),
[`PHASE_11B_DEVICE_DRIVER_FEASIBILITY.md`](PHASE_11B_DEVICE_DRIVER_FEASIBILITY.md),
[`PHASE_11C_ACCEL_PROBE_SPEC.md`](PHASE_11C_ACCEL_PROBE_SPEC.md),
[`PHASE_11D_ACCEL_PROBE_IMPLEMENTATION.md`](PHASE_11D_ACCEL_PROBE_IMPLEMENTATION.md),
[`PHASE_11E_ACCEL_PROBE_EVIDENCE.md`](PHASE_11E_ACCEL_PROBE_EVIDENCE.md),
[`PHASE_10B_V_CLOSE.md`](PHASE_10B_V_CLOSE.md),
[`PHASE_10B_L_CLOSE.md`](PHASE_10B_L_CLOSE.md),
[`PHASE_10B_D_CLOSE.md`](PHASE_10B_D_CLOSE.md).

---

## A. Executive Summary

Phase 11Z is a docs-only checkpoint that **snapshots the validated
command surface** after the Phase 11A–11E sensor-probe track closed
with hardware evidence, and **prevents blind opening of the next
implementation phase**.

It marks the **Phase 11A–11E sensor-probe track as complete**:
the Adapter-level on-board MEMS accelerometer probe command `T` is
fully implemented, build-validated, hardware-validated end-to-end on
STM32F411E-DISCO revision D, and physically-sanity-confirmed by
operator observation (Z ≈ 9.17 m/s² flat-bench).

Validated command set at Phase 11Z:

```text
a / s / r / ? / x / v / L / d / T
```

Nine single-byte commands, all hardware-evidence-backed, all fit the
single-byte / fixed 96-byte stack buffer / no-parser / no-registry /
no-framing / thread-context-TX pattern from
[`PHASE_9EZ_CHECKPOINT.md §H`](PHASE_9EZ_CHECKPOINT.md).

Phase 11Z introduces **no** runtime behavior, **no** source/config
change, **no** new command, and **no** authorization for the open
decisions (ACTIVE disarm widening, Scheduler 7A/7B, F407/custom
board, POST_FLASH_AUTOSTART root cause, Framework API,
Application/product layer). Each remains in its prior status.

---

## B. Baseline Before Phase 11Z

| Item | Value |
|---|---|
| `origin/master` at open | `10710b3` (`docs: close Phase 11E accelerometer probe with hardware evidence`) |
| Last runtime implementation phase | **Phase 11D** (firmware `2040bfb`; `feat: add Phase 11D accelerometer probe command`) |
| Last hardware evidence phase | **Phase 11E** (`10710b3`; `CLOSED_WITH_HARDWARE_EVIDENCE`) |
| Last earlier docs-only checkpoint | Phase 10C (`CLOSED_DOCS_ONLY`) |
| Last earlier runtime checkpoint | Phase 9E-Z (`30bae27`) |
| Hardware platform | STM32F411E-DISCO **revision D** (production runtime baseline) |
| Hardware platform alternatives | F407 / custom board: `HOLD/DEFER` (unchanged) |
| Scheduler | `ROBOTOS_CORE_MAX_EVENTS_PER_TICK = 1`; `ROBOTOS_EVENT_QUEUE_CAPACITY = 16`; 7A/7B `DEFER` |
| UART TX scope | Minimal response only (all 12 scope-guard constraints intact) |
| POST_FLASH_AUTOSTART | Root cause `OPEN`; `MITIGATED_BY_WORKFLOW` from Phase 6O onward |
| Validated command set | **`a / s / r / ? / x / v / L / d / T`** (9 commands) |
| Build at baseline | FLASH 41528 B / RAM 12352 B on `stm32f411e_disco` at `2040bfb` |
| `devkit/prj.conf` since Phase 11D | Adds `CONFIG_I2C=y` + `CONFIG_SENSOR=y`. Does **not** add `CONFIG_SPI`, `CONFIG_ADC`, or `CONFIG_CBPRINTF_FP_SUPPORT`. No DTS overlay. |
| Remaining `USER_DECISION_REQUIRED` semantic | `USER_DECISION_REQUIRED_ACTIVE_DISARM` (`d` from ACTIVE; unchanged) |
| Remaining open decisions | ACTIVE disarm widening; Scheduler 7A/7B; F407/custom board; POST_FLASH_AUTOSTART root cause; Robot Framework API; Application/product layer (see §G) |

---

## C. Command Inventory

This table is the authoritative single-page snapshot of the closed
command set at Phase 11Z. It is sourced from
[`COMMAND_SET_DRAFT.md`](../03_SPECS/COMMAND_SET_DRAFT.md) Section A,
the Phase 9E / 10B-{v,L,d} / 11D / 11E closeouts, and the hardware
RTT/host evidence indexed at
[`../logs/INDEX.md`](../../logs/INDEX.md). Phase 11Z does **not**
modify any of these contracts.

| Cmd | Meaning | Phase | Evidence | State effect | Physical / driver effect | Response shape | Notes / limitations |
|---|---|---|---|---|---|---|---|
| `a` (0x61) | Arm | Phase 9E (`587dab7`) | `CLOSED_WITH_HARDWARE_EVIDENCE` | IDLE -> ARMED (`transitions++`); `uart++` | None | `OK state=ARMED\r\n` | Idempotency: from ARMED, `ignored++` and `OK state=ARMED unchanged=1\r\n`. |
| `s` (0x73) | Start | Phase 9E (`587dab7`) | `CLOSED_WITH_HARDWARE_EVIDENCE` | ARMED -> ACTIVE (`transitions++`); `uart++` | None | `OK state=ACTIVE\r\n` | Idempotency: from ACTIVE, `ignored++` and `OK state=ACTIVE unchanged=1\r\n`. |
| `r` (0x72) | Reset to IDLE | Phase 9E (`587dab7`) | `CLOSED_WITH_HARDWARE_EVIDENCE` | `* -> IDLE` (`transitions++` if state changed); `uart++` | None | `OK state=IDLE\r\n` (changed) / `OK state=IDLE unchanged=1\r\n` (already IDLE) | From IDLE: `ignored++` (legacy semantic). `r` is preserved zero-diff by Phase 10B-d, 11D, 11E. |
| `?` (0x3f) | Query | Phase 9E (`587dab7`) | `CLOSED_WITH_HARDWARE_EVIDENCE` | None; `uart++` | None | `STATE state=<S> transitions=<N> button=<N> uart=<N> ignored=<N>\r\n` | **No LED field. No sensor field.** Shape is unchanged across Phase 9E, 10B-{v,L,d}, 11D, 11E. Extension is out of scope at Phase 11Z. |
| `x` (0x78) | Negative-path probe (unrecognized byte) | Phase 9E (`587dab7`) | `CLOSED_WITH_HARDWARE_EVIDENCE` | None; `uart++`; `ignored++` | None | `ERR ignored byte=0x78 state=<S>\r\n` | Generic ignored response is reused by every byte that does not match a recognized command. |
| `v` (0x76) | Build/version/info query | Phase 10B-v (`d8346db`) | `CLOSED_WITH_HARDWARE_EVIDENCE` | None; `uart++` (no `ignored++`, no transition) | None | `INFO phase=10b-v app=devkit board=<CONFIG_BOARD> tick_ms=<DEVKIT_TICK_MS> uart=minimal\r\n` (77 B on `stm32f411e_disco` at `DEVKIT_TICK_MS=500`) | State-invariant; identical response in IDLE / ARMED / ACTIVE. Deterministic (no `__DATE__`/`__TIME__`). |
| `L` (0x4c) | LED physical-effect smoke (single GPIO toggle) | Phase 10B-L (`f1db2fa` impl + `a96ce17` visual confirmation) | `CLOSED_WITH_HARDWARE_EVIDENCE` + `OPERATOR_VISUAL_CONFIRMED` | None; `uart++` (no `ignored++`, no transition) | One `devkit_status_led_toggle()` call from thread-context UART handler. Heartbeat 500 ms blink in `devkit_runtime_run()` is unchanged; an `L` interleaves one extra toggle (phase shift by one half-cycle). | `OK led=toggle state=<S>\r\n` (26 B) on success; `ERR led=toggle ret=<N> state=<S>\r\n` on failure (not observed in published evidence). | LED state is **not** exposed in `?`. No new LED API/service/queue. No LED ownership conflict observed. |
| `d` (0x64) | Explicit disarm | Phase 10B-d (`125779c` impl + `7e250dc` evidence close) | `CLOSED_WITH_HARDWARE_EVIDENCE` | ARMED -> IDLE (`transitions++`); IDLE / ACTIVE: recognized no-op (no `ignored++`, no transition); `uart++` always | None | `OK disarm state=IDLE\r\n` (22 B) on ARMED -> IDLE; `OK disarm no-op state=<S>\r\n` (28 B IDLE / 30 B ACTIVE) on the no-op paths | `d` does **not** replace or alter `r`. **ACTIVE -> IDLE disarm: `USER_DECISION_REQUIRED_ACTIVE_DISARM`** (unchanged at Phase 11Z); current ACTIVE behavior is recognized no-op (no safety semantics invented). |
| `T` (0x54) | On-board MEMS accelerometer Adapter probe | Phase 11D (`2040bfb` impl) + Phase 11E (`10710b3` evidence close) | **`CLOSED_WITH_HARDWARE_EVIDENCE`** + `OPERATOR_PHYSICAL_SANITY_CONFIRMED` | None; `uart++` (no transition, no `ignored++`, no `button++`) | I2C1 read of `lsm303agr_accel` at 0x19 via the Zephyr `lis2dh` driver (polling / `LIS2DH_TRIGGER_NONE` default); 3 `struct sensor_value` reads on `SENSOR_CHAN_ACCEL_XYZ`. No actuator change. | Success: `ACC x=<v1>.<v2_6d> y=<v1>.<v2_6d> z=<v1>.<v2_6d>\r\n` (worst case 68 B; observed 39 B at `2040bfb`). Error: `ERR accel read=<errno>\r\n` (worst case 28 B). | `val1` signed decimal; `val2` 6-digit absolute fractional; sign carried by `val1`. **`PHASE_11C_FORMAT_SIGN_EDGE`** (val1=0, val2<0) handled by emitting leading `-` with abs val2; observed on hardware (`x=-0.561300`) and correct. Sensor identity is **not** exposed (`LSM303AGR` / `LIS2DH` strings absent from response — Framework-class). Units are **not** exposed (m/s² implied by the LSM303AGR `lis2dh` driver default; not part of the response). |

Notes:

- All nine implemented commands fit the **single-byte / fixed 96-byte
  stack buffer / no-parser / no-registry / no-framing /
  thread-context-TX** pattern.
- The `?` response shape is unchanged across Phase 9E, 10B-{v,L,d},
  11D, and 11E. No field was added or removed.
- All commands are case-insensitive (`A`/`a`, `S`/`s`, `R`/`r`,
  `V`/`v`, `L`/`l`, `D`/`d`, `T`/`t`) via the existing upcase
  normalization at the top of `devkit_app_state_on_uart_byte()`.

---

## D. Phase 11A–11E Sensor-Probe Track Summary

The Phase 11A–11E sensor-probe track is **complete** at Phase 11Z.
Each gate is summarized below for navigation; full evidence is in the
linked closeouts. Phase 11Z does **not** modify any of them.

| Phase | Decision / Result | Status | Source delta | Evidence |
|---|---|---|---|---|
| 11A | `SENSOR_SURFACE_DECIDED_ADAPTER_PROBE` -- classifies `T` as a future Adapter-level bounded probe candidate; not implementation approval. | `CLOSED_DOCS_ONLY` (`9e3daaf`) | zero source/config | docs-only; [`PHASE_11A_ADAPTER_BOUNDARY_SENSOR_SURFACE.md`](PHASE_11A_ADAPTER_BOUNDARY_SENSOR_SURFACE.md) |
| 11B | `FEASIBILITY_CONFIRMED_ONBOARD_MEMS` -- on-board LSM303AGR accel (`lsm303agr_accel`, `lis2dh` driver, I2C1/0x19) is the recommended target. `NO_PURCHASE_NEEDED_FOR_NEXT_STEP`. | `CLOSED_DOCS_ONLY` (`90ef5cc`); config baseline corrected at `2aa0435` | zero source/config | docs-only audit; [`PHASE_11B_DEVICE_DRIVER_FEASIBILITY.md`](PHASE_11B_DEVICE_DRIVER_FEASIBILITY.md) |
| 11C | `PHASE_11C_SPEC_FROZEN_ACCEL_LIS2DH_REV_D` -- target, response, error, host sequence, byte-budget, and Phase 11D/11E placement frozen for revision D. | `CLOSED_DOCS_ONLY` (`cc101cd`) | zero source/config | docs-only spec freeze; [`PHASE_11C_ACCEL_PROBE_SPEC.md`](PHASE_11C_ACCEL_PROBE_SPEC.md) |
| 11D | Implementation per the Phase 11C-frozen contract. Adds `CONFIG_I2C=y` + `CONFIG_SENSOR=y` to `devkit/prj.conf`; `case 't':` in `devkit_app_state.c` and `devkit_uart_producer.c`. Build PASS pristine for `stm32f411e_disco`. | `IMPLEMENTATION_CLOSED_HARDWARE_EVIDENCE_PENDING` (`2040bfb`) | +9/-1 `prj.conf`; +12 `devkit_app_state.c`; +88/-2 `devkit_uart_producer.c`; +3 `devkit_app_state.h` (doc); new `run_phase11d_accel_probe_demo.ps1`. **No `core/`, `platform/`, `CMakeLists.txt`, board DTS, or overlay change.** | [`PHASE_11D_ACCEL_PROBE_IMPLEMENTATION.md`](PHASE_11D_ACCEL_PROBE_IMPLEMENTATION.md) |
| 11E | Hardware evidence: canonical sequence `T T ?` on STM32F411E-DISCO rev D, COM5. Two ACC responses observed (`x=-0.561300 y=2.619400 z=9.167900` and `x=-0.598720 y=2.507140 z=9.167900`); `?` response shape unchanged. `PHASE_11C_FORMAT_SIGN_EDGE` confirmed correct. RTT counters/invariants all PASS; CFSR/HFSR = 0 (13× each). | **`CLOSED_WITH_HARDWARE_EVIDENCE`** + `OPERATOR_PHYSICAL_SANITY_CONFIRMED` (`10710b3`) | zero source/config; docs/logs only | [`PHASE_11E_ACCEL_PROBE_EVIDENCE.md`](PHASE_11E_ACCEL_PROBE_EVIDENCE.md); RTT log `phase_11E_accel_probe_rtt_2026-05-12.txt` (22334 B); host transcript `phase_11E_accel_probe_host_2026-05-12.txt` (388 B) |

The Phase 11A–11E track was the **first** RobotOS phase track to:

1. Introduce a Zephyr driver dependency to `devkit/prj.conf`
   (`CONFIG_I2C=y` + `CONFIG_SENSOR=y` -- the first time the
   non-`prj.conf-zero-diff` invariant was relaxed since Phase 4
   bringup).
2. Cross the Adapter "driver-dependent read" surface class identified
   at Phase 11A as the largest open primitive class.
3. Exercise the `struct sensor_value` integer-fractional response
   path under a frozen no-floating-point textual contract.
4. Confirm a `val1=0 / val2<0` sign edge case on real hardware
   (`PHASE_11C_FORMAT_SIGN_EDGE`).

The track did **not** introduce: Framework API, Application/product
semantics, parser, shell, command registry, framing protocol,
response queue, ACK/retry, heap, ISR-context TX, scheduler change,
queue-capacity change, board DTS change, DTS overlay, or any
hardware purchase.

---

## E. Evidence Summary

Single-page citation of the evidence already published in
[`../logs/INDEX.md`](../../logs/INDEX.md) and the linked closeout
docs. Phase 11Z does **not** modify any of these logs.

| Phase | Logs | Verdict highlights |
|---|---|---|
| Phase 9E | `phase_9E_uart_tx_response_rtt_2026-05-09.txt` | PASS — `a`/`s`/`?`/`r`/`x` host loop on COM5; `rx=ok=handled=5`; `transitions=3 uart=5 ignored=1`; `peak=2 dropped=0 herr=0`; CFSR/HFSR=0 (13×) |
| Phase 9G | `phase_9G_uart_burst_{rtt,host}_2026-05-11.txt` | PASS — 5-byte burst `a s ? r x` at 30 ms spacing; `peak=5 dropped=0`; characterization only, no firmware change |
| Phase 10B-v | `phase_10B_v_{rtt,host}_2026-05-11.txt` | PASS — sequence `v a v r ?`; both `v` responses byte-identical (`INFO phase=10b-v …`, len=77); `v` state-invariant; no `ignored++` |
| Phase 10B-L | `phase_10B_L_{rtt,host}_2026-05-11.txt` + `phase_10B_L_visual_{rtt,host}_2026-05-11.txt` | PASS + `OPERATOR_VISUAL_CONFIRMED` — sequence `L v L ?`; visible heartbeat phase-shift on both `L` commands; same firmware (`f1db2fa`) |
| Phase 10B-d | `phase_10B_d_{rtt,host}_2026-05-11.txt` | PASS — sequence `d a d ?`; `d` from IDLE recognized no-op (NOT ignored); `d` from ARMED produces one transition; ACTIVE not exercised (`USER_DECISION_REQUIRED_ACTIVE_DISARM`) |
| **Phase 11E** | **`phase_11E_accel_probe_{rtt,host}_2026-05-12.txt`** | **PASS + `OPERATOR_PHYSICAL_SANITY_CONFIRMED`** — sequence `T T ?` on rev D COM5; **two ACC responses observed** (`x=-0.561300 y=2.619400 z=9.167900` then `x=-0.598720 y=2.507140 z=9.167900`); `STATE` response shape unchanged; ACC line len=39 B per `T` (cmd=0x54 in RTT); `?` line len=58 B (cmd=0x3f); RTT counters: `UART rx=ok=handled=3 last=0x3f`, `APP state=IDLE transitions=0 button=0 uart=3 ignored=0`, `OBS accepted=63 dispatched=62 pending=1 peak=2 dropped=0 herr=0 unhandled=0 throttled=0 rejected=0`, `PROD attempted=60 ok=60`; CFSR/HFSR = 0x00000000 (13× each); invariants `accepted-dispatched=pending` (63-62=1), `PROD ok + UART ok = accepted` (60+3=63), `UART rx=handled=APP uart=3` all hold; `PHASE_11C_FORMAT_SIGN_EDGE` confirmed correct on hardware; Z ≈ 9.17 m/s² flat-bench. RTT log 22334 B / 61.4 s; host transcript 388 B. |

Phase 11Z does not run any new validation. The above table is
citation only.

---

## F. Scope Guard Restatement

The 12 UART TX scope-guard constraints from
[`PHASE_9EZ_CHECKPOINT.md §H`](PHASE_9EZ_CHECKPOINT.md) are **intact**
at Phase 11Z. Restated and re-verified against published baseline
`origin/master = 10710b3` (post Phase 11E):

| # | Constraint | Status at Phase 11Z |
|---|---|---|
| 1 | No shell / prompt / interactive session semantics | OK — single-byte commands; no prompt; no echo. `CONFIG_SHELL=n`. |
| 2 | No parser framework, no token lexer, no grammar | OK — bare C `switch` on a single byte for all 9 implemented commands |
| 3 | No command registry; no dynamic handler registration | OK — statically-compiled switch-case dispatch |
| 4 | No multiline / framed protocol; no delimiter framing; no length prefix | OK — one byte in, fixed string out, `\r\n` only |
| 5 | No heap buffers; all response buffers stack-allocated; fixed size (96 B) | OK — max observed response is the 77 B `v` response; `T` ACC observed at 39 B; worst-case `T` ACC 68 B; all within 96-byte stack `buf` |
| 6 | No response queue; TX is synchronous in the handler | OK — `snprintf` + `uart_poll_out` inline in `devkit_uart_emit_tx_response()` |
| 7 | No UART logging replacement; RTT remains the canonical log backend | OK — `CONFIG_LOG_BACKEND_RTT=y`, `CONFIG_LOG_BACKEND_UART=n` |
| 8 | No core / platform UART abstraction (`robotos_platform_uart`) | OK — `s_uart_dev` stays in `devkit_uart_producer.c`; `core/` and `platform/` zero-diff vs. 9E baseline |
| 9 | No retry / ACK protocol; no sequence number; no error recovery | OK — `T` sensor read is single-attempt with numeric errno; no retry |
| 10 | No TX from ISR context; `uart_poll_out()` only from handler | OK — TX called only from `devkit_uart_handler()` (thread context) |
| 11 | No promotion of `devkit_app_state` from devkit-local to Robot Framework | OK — module remains devkit-local; no Framework API |
| 12 | Single-byte command vocabulary preserved | OK — all 9 implemented commands are one byte (`a`, `s`, `r`, `?`, `x`, `v`, `L`, `d`, `T`) |

Architecture boundaries at Phase 11Z (relative to Phase 9E baseline):

| Surface | Status at Phase 11Z |
|---|---|
| `RobotOS_v1.0/core/` | **zero diff** since Phase 9E baseline |
| `RobotOS_v1.0/platform/` | **zero diff** since Phase 9E baseline |
| `RobotOS_v1.0/devkit/src/devkit_runtime.{c,h}` | **zero diff** |
| `RobotOS_v1.0/devkit/src/devkit_status_led.{h,c}` | **zero diff** |
| `RobotOS_v1.0/devkit/src/devkit_button_producer.c` | **zero diff** |
| `RobotOS_v1.0/devkit/src/devkit_timer_producer.c` | **zero diff** |
| `RobotOS_v1.0/devkit/src/devkit_observability.c`, `devkit_build_info.c`, `devkit_fault.c` | **zero diff** |
| Scheduler constants (`ROBOTOS_CORE_MAX_EVENTS_PER_TICK = 1`) | **unchanged** |
| Event queue capacity (`ROBOTOS_EVENT_QUEUE_CAPACITY = 16`) | **unchanged** |
| Board DTS, B-revision overlay, board defconfig, board Kconfig | **zero diff** |
| `tests/` | **zero diff** |
| `DEVKIT_PROGRESS.md` (historical master) | **zero diff** |
| Evidence logs under `RobotOS_v1.0/devkit/logs/` | **zero diff** (Phase 11Z adds nothing here) |

The **one** intentional change since Phase 9E baseline is in
`devkit/prj.conf`: Phase 11D added `CONFIG_I2C=y` and
`CONFIG_SENSOR=y` to enable the `lis2dh` driver and Zephyr sensor
subsystem. **No** `CONFIG_SPI`, `CONFIG_ADC`, or
`CONFIG_CBPRINTF_FP_SUPPORT` are enabled. **No** Kconfig overrides
for `LIS2DH_*` range/ODR/HP. **No** DTS overlay. Phase 11Z does not
modify this delta further.

Promotion guards (no Framework / no Application) restated:

| Guard | Status |
|---|---|
| No Robot Framework API | OK — no Framework header, no Framework module, no Framework promotion of `devkit_app_state` |
| No Application / product semantics | OK — `T` is Adapter-level only; no calibration; no units token; no sensor-identity string; no product vocabulary |
| No F407 / custom-board work | OK — `HOLD/DEFER` (unchanged) |
| No reopening of Scheduler 7A/7B | OK — `DEFER` (unchanged) |
| No ACTIVE disarm widening | OK — `USER_DECISION_REQUIRED_ACTIVE_DISARM` preserved |
| No new command candidate | OK — no row added to `COMMAND_SET_DRAFT.md` Section B |
| POST_FLASH_AUTOSTART discipline | Unchanged — root cause `OPEN`; `MITIGATED_BY_WORKFLOW` via `capture_devkit_rtt.ps1` sidecar `reset run` |

---

## G. Remaining Decisions / Future Gates

These are the open decisions at the Phase 11Z checkpoint. Each is
listed with its prerequisite. Phase 11Z does **not** answer any of
them. None should be opened without explicit user direction.

### G.1 ACTIVE disarm widening

| Aspect | Value |
|---|---|
| Current behavior | `d` from ACTIVE -> recognized no-op (`OK disarm no-op state=ACTIVE\r\n`); no transition; no `ignored++` |
| Proposed future behavior | `d` from ACTIVE -> IDLE with `transitions++`; response `OK disarm state=IDLE\r\n` (identical to ARMED -> IDLE) |
| Status | **`USER_DECISION_REQUIRED_ACTIVE_DISARM`** (unchanged at Phase 11Z) |
| Implementation surface if approved | One-line widening of the `if (s_state == DEVKIT_APP_STATE_ARMED)` guard in `devkit_app_state.c` to `if (s_state != DEVKIT_APP_STATE_IDLE)` |
| Validation if approved | Supplemental hardware run with sequence `d a s d ?`; expected final `transitions=3 button=0 uart=4 ignored=0` |
| Required user inputs | Whether ACTIVE disarm is a desired product semantic, or whether `r` should remain the canonical path for `ACTIVE -> IDLE` |
| Risk | None observed; one-line change to a closed module; no scope-guard violation; no scheduler/queue impact. The risk is **silent-semantics**: changing behavior without an explicit product decision. |

### G.2 Scheduler 7A / 7B-1

| Aspect | Value |
|---|---|
| Current state | `DEFER` (unchanged since Phase 9E-Z) |
| Workload evidence after Phase 11E | `peak=2` on the `T T ?` sequence; no `dropped`, no `herr`. The sensor read path did **not** push observed peak above the Phase 10B baseline. |
| Risk if opened now | No new evidence has changed the prior `DEFER` ruling. Opening would be re-litigation, not progress. |

### G.3 STM32F407 / custom board

| Aspect | Value |
|---|---|
| Current state | `HOLD/DEFER` (unchanged from Phase 8A status) |
| Risk if opened now | No new workload requires the platform shift. Opening would consume the scheduling slot without justification. |

### G.4 POST_FLASH_AUTOSTART root cause

| Aspect | Value |
|---|---|
| Current state | Root cause `OPEN`; `MITIGATED_BY_WORKFLOW` from Phase 6O onward via `capture_devkit_rtt.ps1` sidecar `reset run`. Manual RESET retained as fallback. Plain `west flash` alone is **not** runtime-start evidence. |
| Open since | Phase 6 series; was non-blocking for Phase 9A–11E |
| Future option | Optional engineering investigation (likely OpenOCD/STM32 ST-LINK reset-after-flash interaction); not blocking any current or proposed gate |
| Risk if not investigated | Operational: every flash session must continue using the sidecar `reset run` discipline |

### G.5 Robot Framework API planning

| Aspect | Value |
|---|---|
| Current state | **`NOT_STARTED`** |
| Why not now | The Adapter surface is the only one currently exercised. Phase 11A §B inventories eleven Adapter primitive classes; only the "driver-dependent read" sub-class has been crossed (by Phase 11D/11E `T`). Other open Adapter sub-classes (SPI, ADC, additional MEMS) have not been characterized. Pool/slab is flagged as an open architectural question. Portability backend is claimed but undemonstrated (only Zephyr/STM32F4 exercised). |
| Future option (docs-only) | A Phase 12-class docs-only planning gate could inventory candidate Framework primitives (units, sensor identity, calibration, fault classification, time semantics, multi-sensor fusion) and choose **one** narrow surface to specify first. **No implementation, no `core/`/`platform/` change.** Pattern would mirror Phase 11A → 11B → 11C planning sequence. |
| Risk if opened now without explicit direction | Framework choice has long-running impact on every later module; opening without a written direction risks invented requirements. |

### G.6 Application / product layer

| Aspect | Value |
|---|---|
| Current state | **`NOT_STARTED`** |
| Why not now | No product vocabulary has been chosen. `T` is explicitly **not** a product command; it is an Adapter probe. Promoting any command to Application semantics requires a product decision (what is the product, what is the operator-facing vocabulary, what is calibration ownership). |
| Future option | A docs-only Application planning phase analogous to Phase 11A. Not scheduled. |
| Risk if opened now without explicit direction | Same as G.5: invented requirements. |

---

## H. Next Recommendation

**Hold.** Phase 11Z is the closure of the Phase 11A–11E sensor-probe
track. No further phase should open autonomously.

Future possible next phases, in priority order **only when
explicitly authorized by the user**:

1. **Phase 12A-class docs-only Framework API surface planning**
   (analog of Phase 11A) — inventories candidate Framework
   primitives and chooses one narrow surface to specify first. No
   implementation. **Recommended next planning step** if the user
   wants the project to advance into Framework territory.
2. **ACTIVE disarm widening** — small vocabulary housekeeping; one
   source line + one supplemental hardware run with `d a s d ?`;
   fully decoupled from the Phase 11A–11E sensor track and from
   Framework/Application planning. Cheap; user-driven only.
3. **Adapter SPI / ADC probe** (analogue of Phase 11A–11E for the
   other open Adapter sub-classes) — only if a specific workload
   motivates it. No current workload requires it.
4. **POST_FLASH_AUTOSTART root cause investigation** — optional
   engineering work; not blocking.
5. **Scheduler 7A/7B** — `DEFER`; do not reopen without new
   workload evidence.
6. **F407 / custom board** — `HOLD/DEFER`; do not reopen without
   explicit portability requirement.
7. **Application / product layer** — `NOT_STARTED`; requires
   product direction.

Phase 11Z itself **authorizes none** of the above. The user must
explicitly select one (or none) before any successor phase opens.

---

## I. What this document does not do

- Does not modify any source, `prj.conf`, test, CMake, overlay, board
  DTS, Zephyr workspace file, runtime script, host tool, or evidence
  log.
- Does not modify any closed phase's evidence or closeout doc.
- Does not modify `DEVKIT_PROGRESS.md` historical master.
- Does not implement any new command.
- Does not change command semantics for `a / s / r / ? / x / v / L / d / T`.
- Does not promote `T` (already `CLOSED_WITH_HARDWARE_EVIDENCE` at
  Phase 11E; Phase 11Z only records that fact in the inventory).
- Does not authorize ACTIVE disarm widening.
- Does not reopen Scheduler 7A/7B.
- Does not reopen F407 / custom-board.
- Does not change POST_FLASH_AUTOSTART status.
- Does not start Framework API planning.
- Does not start Application / product semantics.
- Does not authorize any hardware purchase.
- Does not push.
