# Phase 11E -- On-board MEMS Accelerometer Probe Evidence Closeout

**Status:** `CLOSED_WITH_HARDWARE_EVIDENCE`
**Type:** Hardware evidence-only closeout. No source, runtime, test,
CMake, Zephyr, board, `prj.conf`, DTS overlay, or script change.
Proves the Phase 11D `T`/`t` accelerometer probe command on
STM32F411E-DISCO revision D hardware.
**Date opened/closed:** 2026-05-12 (same-day hardware evidence close)
**Branch:** master
**Implementation commit tested:** `2040bfb`
(`feat: add Phase 11D accelerometer probe command`)
**Prior phase:** Phase 11D (`IMPLEMENTATION_CLOSED_HARDWARE_EVIDENCE_PENDING`
at `2040bfb`).
**Companion docs:**
[`PHASE_11D_ACCEL_PROBE_IMPLEMENTATION.md`](PHASE_11D_ACCEL_PROBE_IMPLEMENTATION.md),
[`PHASE_11C_ACCEL_PROBE_SPEC.md`](PHASE_11C_ACCEL_PROBE_SPEC.md),
[`../01_PROGRESS/DEVKIT_PROGRESS_PHASE_11_20.md`](../01_PROGRESS/DEVKIT_PROGRESS_PHASE_11_20.md),
[`../03_SPECS/COMMAND_SET_DRAFT.md`](../03_SPECS/COMMAND_SET_DRAFT.md).

---

## A. Executive Summary

Phase 11E proves the Phase 11D on-board MEMS accelerometer probe
command `T` on real hardware (STM32F411E-DISCO revision D).

**Verdict: `CLOSED_WITH_HARDWARE_EVIDENCE`.**

The **ACC success path** was observed on both `T` invocations. The
sensor read reached the hardware (I2C1 at 0x19), returned three live
`SENSOR_CHAN_ACCEL_XYZ` samples, and the firmware emitted the frozen
Phase 11C `ACC x=<v1>.<v2_6d> y=<v1>.<v2_6d> z=<v1>.<v2_6d>\r\n`
response on both `T` commands.

No ERR path, no fault, no handler error, no state transition, no
`ignored++`.

Additionally, the **`PHASE_11C_FORMAT_SIGN_EDGE`** case (val1=0,
val2<0) was exercised by X-axis reading (`x=-0.561300`) and handled
correctly by the Phase 11D formatter.

Physical sanity: Z axis read ~9.17 m/s² with board flat on bench
(close to 9.81 m/s² gravity, within expected uncalibrated range) →
`OPERATOR_PHYSICAL_SANITY_CONFIRMED`.

No source, config, runtime script, or core/platform file was
modified in Phase 11E.

---

## B. Hardware Setup

| Item | Value |
|---|---|
| Board | STM32F411E-DISCO |
| Board revision | **D** (operator-confirmed; Zephyr DT default) |
| Accelerometer | LSM303AGR (`lsm303agr_accel`); `lis2dh` driver; I2C1 / 0x19 |
| ST-LINK | USB-connected for power, debug, and OpenOCD RTT |
| UART wiring | PA2 TX → USB-UART RX; PA3 RX → USB-UART TX; GND common; 3.3V TTL; USB-UART VCC not connected |
| USB-UART adapter | CP210x (COM5) |
| Baud / format | 115200 8N1 |
| Board orientation | Flat on bench; Z axis pointing up |

---

## C. Build / Flash

### C.1 Build

Two pristine builds were run:

1. `build-phase11e/` — independent verification build:
   ```
   west build --pristine=always -d build-phase11e -b stm32f411e_disco RobotOS_v1.0/devkit
   ```
2. `build/` — default build for harness ELF path compatibility (the
   `capture_devkit_rtt.ps1` sidecar defaults to
   `build/zephyr/zephyr.elf`):
   ```
   west build --pristine=always -b stm32f411e_disco RobotOS_v1.0/devkit
   ```

Both builds: **PASS** — 161/161 steps.

```text
Memory region   Used Size   Region Size   %age Used
        FLASH:    41528 B        512 KB    7.92%
          RAM:    12352 B        128 KB    9.42%
```

Byte-identical result to Phase 11D build (same commit, same config,
same toolchain). No new warnings.

Generated `.config` confirms (from `build-phase11e/zephyr/.config`):

```text
CONFIG_I2C=y
CONFIG_I2C_STM32=y / I2C_STM32_V1=y
CONFIG_SENSOR=y
CONFIG_LIS2DH=y
CONFIG_LIS2DH_TRIGGER_NONE=y          (driver default; not in prj.conf)
CONFIG_LIS2DH_OPER_MODE_NORMAL=y
CONFIG_LIS2DH_ACCEL_RANGE_RUNTIME=y
CONFIG_LIS2DH_ODR_RUNTIME=y
```

Forbidden Kconfigs absent: `CONFIG_SPI` not set, `CONFIG_ADC` not
set, `CONFIG_CBPRINTF_FP_SUPPORT` not set.

### C.2 Flash

```
west flash
```

Output (from default build directory):
```
wrote 49152 bytes from file D:/Robot_OS/build/zephyr/zephyr.hex in 1.578258s (30.413 KiB/s)
```

Flash: **PASS**.

### C.3 POST_FLASH_AUTOSTART discipline

`west flash` ends with `shutdown command invoked` — firmware does not
autostart after plain flash (root cause OPEN; `MITIGATED_BY_WORKFLOW`
from Phase 6O onward). The `capture_devkit_rtt.ps1` sidecar issues
`reset run` through the OpenOCD sidecar `.cfg`, starting the
firmware before RTT streaming begins. Manual RESET not required.

The sidecar resolved RTT control block `_SEGGER_RTT at 0x20000ad0`
via `arm-zephyr-eabi-nm` from the ELF and issued the standard
`rtt setup … ; rtt start ; rtt server start 9090 0` sequence.

---

## D. Host Transcript Summary

### D.1 Harness command

```
.\RobotOS_v1.0\tools\runtime\run_phase11d_accel_probe_demo.ps1 -ComPort COM5 -CaptureSeconds 60 -WaitBeforeInputSeconds 15
```

### D.2 Sequence and responses

Canonical frozen sequence: `T T ?`.
Transcript file:
[`../../../devkit/logs/phase_11E_accel_probe_host_2026-05-12.txt`](../../../devkit/logs/phase_11E_accel_probe_host_2026-05-12.txt)
(388 bytes).

```text
SEND 'T' (0x54) -> RECV: ACC x=-0.561300 y=2.619400 z=9.167900
SEND 'T' (0x54) -> RECV: ACC x=-0.598720 y=2.507140 z=9.167900
SEND '?' (0x3f) -> RECV: STATE state=IDLE transitions=0 button=0 uart=3 ignored=0
```

### D.3 Shape validation

| Response | Frozen shape | Match |
|---|---|---|
| 1st `T` | `ACC x=<v1>.<v2_6d> y=<v1>.<v2_6d> z=<v1>.<v2_6d>\r\n` | **MATCH** |
| 2nd `T` | same | **MATCH** |
| `?` | `STATE state=<S> transitions=<N> button=<N> uart=<N> ignored=<N>\r\n` | **MATCH** |

The two ACC responses are **not** byte-identical (expected: live
sensor sampling; minor X/Y variation is normal). Z is stable
at 9.167900 both times.

### D.4 Bounded-response validation

RTT confirms `Phase 9E UART response sent cmd=0x54 len=39` for both
`T` responses. 39 bytes < 96-byte buffer limit. `?` response: 58 bytes
confirmed in RTT as well. All well within the frozen Phase 11C
worst-case budget (68 B success / 28 B error / 96 B buffer).

### D.5 PHASE_11C_FORMAT_SIGN_EDGE observed and correct

The first `T` response shows `x=-0.561300`. This is the documented
edge case where `val1 = 0` and `val2 < 0` (sub-unit negative
magnitude, Zephyr convention). The Phase 11D formatter correctly
emitted a leading `-` with absolute val1 and val2, producing
`-0.561300` — inside the frozen `<v1>.<v2_6d>` shape.
`PHASE_11C_FORMAT_SIGN_EDGE` is **confirmed working on hardware**.

---

## E. RTT Counter Summary

RTT log:
[`../../../devkit/logs/phase_11E_accel_probe_rtt_2026-05-12.txt`](../../../devkit/logs/phase_11E_accel_probe_rtt_2026-05-12.txt)
(22334 bytes, 61.4 s).

### E.1 Pre-sequence baseline (ticks=0, session start)

| Counter | Value |
|---|---|
| `ROBOTOS_UART rx` | 0 |
| `ROBOTOS_UART ok` | 0 |
| `ROBOTOS_UART handled` | 0 |
| `ROBOTOS_APP uart` | 0 |
| `ROBOTOS_APP transitions` | 0 |
| `ROBOTOS_APP ignored` | 0 |
| `ROBOTOS_APP button` | 0 |
| `ROBOTOS_OBS accepted` | 0 |
| `ROBOTOS_OBS dispatched` | 0 |
| `ROBOTOS_OBS dropped` | 0 |
| `ROBOTOS_OBS herr` | 0 |

### E.2 Post-sequence state (ticks=50, at `00:00:24`, first sample after `T T ?`)

| Counter | Value | Delta from pre | Expected |
|---|---|---|---|
| `ROBOTOS_UART rx` | 3 | **+3** | +3 ✓ |
| `ROBOTOS_UART ok` | 3 | **+3** | +3 ✓ |
| `ROBOTOS_UART handled` | 3 | **+3** | +3 ✓ |
| `ROBOTOS_UART last` | `0x3f` | — | `0x3f` (`?`) ✓ |
| `ROBOTOS_APP uart` | 3 | **+3** | +3 ✓ |
| `ROBOTOS_APP state` | `IDLE` | — | unchanged ✓ |
| `ROBOTOS_APP transitions` | 0 | **0** | 0 ✓ |
| `ROBOTOS_APP ignored` | 0 | **0** | 0 ✓ |
| `ROBOTOS_APP button` | 0 | **0** | 0 ✓ |

### E.3 Final OBS/PROD state (ticks=120, at `00:00:59`)

| Counter | Value |
|---|---|
| `ROBOTOS_OBS accepted` | 63 |
| `ROBOTOS_OBS dispatched` | 62 |
| `ROBOTOS_OBS pending` | 1 |
| `ROBOTOS_OBS peak` | 2 |
| `ROBOTOS_OBS dropped` | **0** |
| `ROBOTOS_OBS herr` | **0** |
| `ROBOTOS_OBS unhandled` | **0** |
| `ROBOTOS_OBS throttled` | **0** |
| `ROBOTOS_OBS rejected` | **0** |
| `ROBOTOS_PROD attempted` | 60 |
| `ROBOTOS_PROD ok` | 60 |

### E.4 Invariant cross-checks (all PASS)

| Invariant | Expression | Value | Result |
|---|---|---|---|
| `accepted - dispatched = pending` | 63 − 62 = 1 | 1 | **PASS** |
| `PROD ok + UART ok = accepted` | 60 + 3 = 63 | 63 | **PASS** |
| `UART rx = UART handled = APP uart` | 3 = 3 = 3 | all 3 | **PASS** |
| `transitions(post) = transitions(pre)` | 0 = 0 | delta = 0 | **PASS** |
| `ignored(post) = ignored(pre)` | 0 = 0 | delta = 0 | **PASS** |
| `CFSR = 0x00000000 always` | 0 at 13 samples | — | **PASS** |
| `HFSR = 0x00000000 always` | 0 at 13 samples | — | **PASS** |

### E.5 RTT diagnostic lines (confirmed in log)

```text
[00:00:13] Phase 11D-T accel probe: state=IDLE   <- 1st T
[00:00:13] Phase 9E UART response sent cmd=0x54 len=39 state=IDLE
[00:00:14] Phase 11D-T accel probe: state=IDLE   <- 2nd T
[00:00:14] Phase 9E UART response sent cmd=0x54 len=39 state=IDLE
[00:00:15] Phase 9E UART response sent cmd=0x3f len=58 state=IDLE <- ?
```

### E.6 Peak observation

`peak=2` throughout the capture — matches the Phase 10B baseline
(three byte-paced host commands interleaved with the Phase 6M
heartbeat producer). No regression from adding the sensor read path.

---

## F. Physical Sanity

**`OPERATOR_PHYSICAL_SANITY_CONFIRMED`**

Board was placed flat on bench, Z axis approximately vertical (up).

| Axis | 1st `T` | 2nd `T` | Assessment |
|---|---|---|---|
| X | −0.561300 m/s² | −0.598720 m/s² | Small; consistent with slight tilt |
| Y | 2.619400 m/s² | 2.507140 m/s² | Small; consistent with slight tilt |
| Z | 9.167900 m/s² | 9.167900 m/s² | Near +9.81 m/s² (gravity) ✓ |

Z ≈ 9.17 m/s² is consistent with gravitational acceleration on the
vertical axis (within the expected uncalibrated range of the
LSM303AGR; slight deviation from 9.81 is normal without calibration).
Two `T` responses differ in X/Y (live sensor sampling), confirming
the sensor is reading dynamically rather than returning a static
cached value. Z is stable between the two samples.

**Calibration is explicitly not claimed.** The value is judged
physically plausible for a flat bench orientation; no numerical
tolerance is specified.

---

## G. Scope Guard Audit

Phase 11E made **zero source, config, or runtime changes**. All
audited items inherit their status from Phase 11D close.

| Guard | Status |
|---|---|
| No source files changed | **Confirmed.** `git diff --check` clean; `git status` working tree clean before and after evidence run. |
| No `prj.conf` change | **Confirmed.** |
| No DTS overlay | **Confirmed.** |
| No `core/` mutation | **Confirmed.** |
| No `platform/` mutation | **Confirmed.** |
| All 12 UART TX scope-guard constraints | **Intact** (inherited from Phase 11D) |
| Scheduler 7A/7B | **`DEFER`** |
| F407 / custom board | **`HOLD/DEFER`** |
| UART TX | **Minimal response only**; 96-byte stack buffer; `uart_poll_out`; thread context |
| POST_FLASH_AUTOSTART discipline | **Unchanged**; root cause `OPEN`; `MITIGATED_BY_WORKFLOW` via `capture_devkit_rtt.ps1` sidecar `reset run` |
| ACTIVE disarm widening | **`USER_DECISION_REQUIRED_ACTIVE_DISARM`**; not started; decoupled from Phase 11A-E |
| Phase 12+ | Not started; not referenced |

---

## H. Decision

**`CLOSED_WITH_HARDWARE_EVIDENCE`**

The `T` command is now hardware-evidence-backed. Promotion criteria:

- [x] ACC success path observed on both `T` invocations.
- [x] Both ACC responses match frozen Phase 11C shape exactly.
- [x] Both responses within 96-byte buffer (observed len=39 each).
- [x] `PHASE_11C_FORMAT_SIGN_EDGE` (val1=0, val2<0) observed and
  correctly formatted (`x=-0.561300`).
- [x] `?` response shape unchanged.
- [x] App state does not transition due to `T`.
- [x] `ignored` does not increment due to `T`.
- [x] RTT counters and invariants all PASS (see §E.4).
- [x] `CFSR = HFSR = 0x00000000` throughout (13 samples each).
- [x] `dropped = herr = 0` throughout.
- [x] `peak=2` — no regression from sensor read path.
- [x] `OPERATOR_PHYSICAL_SANITY_CONFIRMED` (Z ≈ 9.17 m/s² ≈ gravity).
- [x] No source/config changes in Phase 11E.

`T` may be promoted from Section B to Section A in
`COMMAND_SET_DRAFT.md` at this close.

The Phase 11A–11E sensor probe track is **complete** as an
Adapter-level bounded probe:

| Gate | Status |
|---|---|
| 11A — Adapter Boundary & Sensor Surface Decision | `CLOSED_DOCS_ONLY` |
| 11B — Device / Driver Feasibility Gate | `CLOSED_DOCS_ONLY` |
| 11C — Accelerometer Probe Spec | `CLOSED_DOCS_ONLY` |
| 11D — Accelerometer Probe Implementation | `IMPLEMENTATION_CLOSED_HARDWARE_EVIDENCE_PENDING` → upgraded |
| **11E — Accelerometer Probe Evidence** | **`CLOSED_WITH_HARDWARE_EVIDENCE`** |

---

## I. What this document does not do

- Does not modify any source, prj.conf, test, CMake, overlay, board DTS,
  or Zephyr workspace file.
- Does not start ACTIVE disarm widening.
- Does not reopen Scheduler 7A/7B.
- Does not reopen F407 / custom-board.
- Does not change POST_FLASH_AUTOSTART status.
- Does not authorize hardware purchase.
- Does not push.
