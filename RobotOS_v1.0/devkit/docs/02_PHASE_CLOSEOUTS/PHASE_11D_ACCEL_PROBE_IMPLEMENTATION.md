# Phase 11D -- On-board MEMS Accelerometer Probe Implementation

**Status:** `IMPLEMENTATION_CLOSED_HARDWARE_EVIDENCE_PENDING`
**Type:** Devkit-local firmware + tooling implementation of the Phase
11C-frozen on-board MEMS accelerometer probe (`T` command). Source +
`prj.conf` + host harness changes only. No `core/`, no `platform/`,
no `tests/`, no scheduler/queue constants, no board DTS, no DTS
overlay, no Zephyr module, no hardware purchase. Build-validated
pristine for `stm32f411e_disco`; hardware evidence is deferred to
Phase 11E.
**Date opened/closed:** 2026-05-12 (same-day open + build-validated close)
**Branch:** master
**Published baseline at open:** `origin/master = cc101cd`
**Closing commit:** to be filled in at commit time.
**Prior runtime behavior phase:** Phase 10B-d (firmware `125779c`,
evidence-close `7e250dc`; build baseline FLASH 36780 B / RAM 12224 B).
**Prior docs-only baseline:** Phase 11C (`cc101cd`) freezes the spec
this phase implements verbatim.
**Companion docs:**
[`PHASE_11C_ACCEL_PROBE_SPEC.md`](PHASE_11C_ACCEL_PROBE_SPEC.md),
[`PHASE_11B_DEVICE_DRIVER_FEASIBILITY.md`](PHASE_11B_DEVICE_DRIVER_FEASIBILITY.md),
[`PHASE_11A_ADAPTER_BOUNDARY_SENSOR_SURFACE.md`](PHASE_11A_ADAPTER_BOUNDARY_SENSOR_SURFACE.md),
[`PHASE_10C_COMMAND_SET_CHECKPOINT.md`](PHASE_10C_COMMAND_SET_CHECKPOINT.md),
[`PHASE_9EZ_CHECKPOINT.md`](PHASE_9EZ_CHECKPOINT.md),
[`../03_SPECS/COMMAND_SET_DRAFT.md`](../03_SPECS/COMMAND_SET_DRAFT.md),
[`../01_PROGRESS/DEVKIT_PROGRESS_PHASE_11_20.md`](../01_PROGRESS/DEVKIT_PROGRESS_PHASE_11_20.md).

---

## A. Executive Summary

Phase 11D implements the Phase 11C-frozen on-board MEMS accelerometer
probe as a bounded devkit-local Adapter command `T` (case-insensitive
`T`/`t`). The implementation follows the Phase 11C contract verbatim
and does **not** widen the spec.

Three deliverables:

1. **Source** -- `case 't':` branch added to
   [`devkit_app_state.c`](../../src/devkit_app_state.c) (recognition,
   non-transitioning, not ignored) and to
   [`devkit_uart_producer.c`](../../src/devkit_uart_producer.c)
   (`sensor_sample_fetch` + `sensor_channel_get` against
   `DT_ALIAS(accel0)`, fixed 96-byte stack buffer, frozen `ACC` or
   `ERR` response line). One small sensor_value formatter helper is
   added inside `devkit_uart_producer.c`; no new translation unit.
2. **Config** -- exactly two new lines in
   [`devkit/prj.conf`](../../prj.conf): `CONFIG_I2C=y` and
   `CONFIG_SENSOR=y`. `CONFIG_LIS2DH_TRIGGER_NONE=y` was **not**
   added: pristine `.config` already sets it (driver default; first
   option of `choice LIS2DH_TRIGGER_MODE`).
3. **Tooling** -- new host harness
   [`run_phase11d_accel_probe_demo.ps1`](../../../tools/runtime/run_phase11d_accel_probe_demo.ps1)
   ready for Phase 11E.

Phase 11D does **not** claim hardware evidence. The harness is
provided so Phase 11E can run it; no operator-witnessed run is
recorded by Phase 11D.

---

## B. Baseline Before Phase 11D

| Item | Value | Source |
|---|---|---|
| `origin/master` at open | `cc101cd` (`docs: add Phase 11C accelerometer probe spec`) | `git rev-parse origin/master` |
| Phase 11A / 11B / 11C | `CLOSED_DOCS_ONLY` | progress + closeout docs |
| Phase 11C decision | `PHASE_11C_SPEC_FROZEN_ACCEL_LIS2DH_REV_D` | [`PHASE_11C_ACCEL_PROBE_SPEC.md`](PHASE_11C_ACCEL_PROBE_SPEC.md) §L |
| Board revision | **D** (operator-confirmed) | Phase 11C §B |
| Pre-existing Kconfig (relevant) | none of `CONFIG_I2C`, `CONFIG_SPI`, `CONFIG_SENSOR`, `CONFIG_ADC` enabled in `devkit/prj.conf` | corrected at `2aa0435` |
| Last runtime behavior phase | Phase 10B-d (`125779c`; FLASH 36780 B / RAM 12224 B) | unchanged |
| Validated non-sensor command set | `a / s / r / ? / x / v / L / d` (8 commands) | Phase 10C §3 |
| `T` status | `USER_DECISION_REQUIRED` / not implemented | [`COMMAND_SET_DRAFT.md`](../03_SPECS/COMMAND_SET_DRAFT.md) Section B |
| ACTIVE disarm widening | `USER_DECISION_REQUIRED_ACTIVE_DISARM` | Phase 10C §5.1 |
| Scheduler 7A/7B | `DEFER` | Phase 10C §5.3 |
| F407 / custom board | `HOLD/DEFER` | Phase 10C §5.4 |
| UART TX scope | minimal response only; 12 scope-guard constraints intact | Phase 9E-Z §H |
| POST_FLASH_AUTOSTART | root cause `OPEN`; `MITIGATED_BY_WORKFLOW` from Phase 6O onward | unchanged |

---

## C. Files Changed

| File | Lines | Nature |
|---|---:|---|
| `RobotOS_v1.0/devkit/prj.conf` | +9 / -1 | Append `CONFIG_I2C=y` and `CONFIG_SENSOR=y` with a leading comment block. **Nothing else.** |
| `RobotOS_v1.0/devkit/src/devkit_app_state.h` | +5 / -1 | Doc-only: add `T` to the recognized-command list and to the per-command comment block. **No new exported symbol.** |
| `RobotOS_v1.0/devkit/src/devkit_app_state.c` | +12 / 0 | Add `case 't':` branch matching the `case 'v':` shape (`LOG_INF` only; no transition, no `ignored++`, no counters touched beyond the global `s_uart_count++` already done above the switch). |
| `RobotOS_v1.0/devkit/src/devkit_uart_producer.c` | +88 / -2 | Add `#include <errno.h>`, `#include <stdlib.h>`, `#include <zephyr/drivers/sensor.h>`; define `DEVKIT_ACCEL_NODE = DT_ALIAS(accel0)` with `#error` guard for missing alias; resolve `s_accel_dev = DEVICE_DT_GET(...)` at link time; add internal `devkit_accel_format_value()` helper; add `case 't':` TX branch (`sensor_sample_fetch` + `sensor_channel_get` + frozen response composition). |
| `RobotOS_v1.0/tools/runtime/run_phase11d_accel_probe_demo.ps1` | new file | Phase 11E-ready host harness. Pure ASCII PowerShell 5.1. Default sequence `T T ?`. PowerShell parse OK. |
| `RobotOS_v1.0/devkit/docs/01_PROGRESS/DEVKIT_PROGRESS_PHASE_11_20.md` | +N | Phase 11D index row + section with manual anchor `<a id="phase-11d"></a>`. |
| `RobotOS_v1.0/devkit/docs/02_PHASE_CLOSEOUTS/PHASE_11D_ACCEL_PROBE_IMPLEMENTATION.md` | new file | This document. |
| `RobotOS_v1.0/devkit/docs/03_SPECS/COMMAND_SET_DRAFT.md` | +N / -N | Section status preamble updated to Phase 11D checkpoint; Section B `T` row updated to mark implemented-build-validated, hardware evidence pending Phase 11E. |
| `RobotOS_v1.0/devkit/docs/00_INDEX/README.md` | +1 | Add Phase 11D row to the closeout listing. |
| `CURRENT_STATE.md` (repo root) | +N | Insert Phase 11D as latest implementation/build-validated phase. |

Files **not** touched (zero diff): `core/`, `platform/`,
`tests/`, `devkit/CMakeLists.txt` top-level, `devkit_runtime.{c,h}`,
`devkit_status_led.{c,h}`, `devkit_button_producer.c`,
`devkit_timer_producer.c`, `devkit_observability.c`,
`devkit_build_info.c`, `devkit_fault.c`, board DTS, board defconfig,
B-revision overlay, Zephyr workspace tracked files,
`DEVKIT_PROGRESS.md` (historical master),
`DEVKIT_PROGRESS_PHASE_10.md`, any evidence log under
`RobotOS_v1.0/devkit/logs/`.

---

## D. Implementation Summary

### D.1 `devkit_app_state.c` -- recognition

A `case 't':` branch is added (the upstream switch already
upper-to-lower normalizes by `c >= 'A' && c <= 'Z'`, so `T` is
case-insensitive without changing the switch label vocabulary). The
branch logs a `Phase 11D-T accel probe: state=<S>` line at `LOG_INF`
and falls through. No state transition, no `ignored++`, no
`transitions++`, no `button++`. The global `s_uart_count++` was
already incremented above the switch (matches every other
recognized command).

### D.2 `devkit_uart_producer.c` -- read path and response

- New includes: `<errno.h>`, `<stdlib.h>`, `<zephyr/drivers/sensor.h>`.
- New compile-time guard:
  ```c
  #define DEVKIT_ACCEL_NODE DT_ALIAS(accel0)
  #if !DT_NODE_EXISTS(DEVKIT_ACCEL_NODE)
  #error "Phase 11D requires DT alias 'accel0' (DT_ALIAS_ACCEL0_MISSING)"
  #endif
  ```
- New link-time device handle:
  ```c
  static const struct device *const s_accel_dev =
      DEVICE_DT_GET(DEVKIT_ACCEL_NODE);
  ```
- New internal helper `devkit_accel_format_value()` -- formats one
  `struct sensor_value` into the `<v1>.<v2_6d>` sub-token of the
  frozen `ACC` line. Sign rule per §D.5.
- New `case 't':` in the `devkit_uart_emit_tx_response()` switch.
  Behavior:
  1. `device_is_ready(s_accel_dev)` -> on `false`, emit
     `ERR accel read=-19\r\n` (the numeric value of `-ENODEV` on
     Zephyr; we let the `errno.h` macro `ENODEV` decide at compile
     time rather than hardcoding the integer).
  2. `sensor_sample_fetch(s_accel_dev)` -> on `< 0`, emit
     `ERR accel read=<rc>\r\n`.
  3. `sensor_channel_get(s_accel_dev, SENSOR_CHAN_ACCEL_XYZ,
     accel[3])` -> on `< 0`, emit `ERR accel read=<rc>\r\n`.
  4. Otherwise, format each axis with the helper and emit
     `ACC x=<v1>.<v2_6d> y=<v1>.<v2_6d> z=<v1>.<v2_6d>\r\n`.
  All four paths route through the existing 96-byte stack `buf` and
  the existing `devkit_uart_send_bytes()` polled-TX path. The handler
  returns `OK` to the dispatcher regardless of sensor success/failure
  (sensor error is **not** a handler error per Phase 11C §F.4 -- the
  handler did its job by emitting the `ERR` line and incrementing
  `s_tx_sent`).

### D.3 No new compilation unit

The implementation fits inside `devkit_uart_producer.c` (one helper +
one switch branch). The optional `devkit_accel_probe.{c,h}` allowed
by Phase 11C §J.1 is **not** used: the body is small enough that
extracting it would add a separate translation unit without
proportional benefit, and would force a `target_sources(...)` edit
which the existing source list does not need.

### D.4 No retry, no symbolic errno

Phase 11C §F.4 forbids retry; the implementation makes a single
attempt and reports. The `<errno>` in the error response is the raw
signed-decimal return code from the failing API; no symbolic mapping
(`EIO`, `EINVAL`, ...) and no `strerror()`.

### D.5 Sign rule for `struct sensor_value`

Phase 11C §E.1 states val1 is "Signed decimal, no padding, no leading
zero. Sign-carrying integer part." and val2 is "Absolute value of
val2, zero-padded to 6 digits. The sign is carried by val1."

The Zephyr convention is that for sub-unit magnitudes, **only val2**
carries the sign (e.g. -0.5 is encoded `val1 = 0, val2 = -500000`).
The frozen response shape places the sign on the integer part, so the
implementation preserves sign by emitting a leading `-` when **either**
`val1 < 0` or `(val1 == 0 && val2 < 0)`. Both `val1` and `val2` are
then printed as absolute values. This stays inside the frozen
`<v1>.<v2_6d>` shape (no extra characters, no double sign) and is
recorded here as **`PHASE_11C_FORMAT_SIGN_EDGE`**. Concretely:

| sensor_value | Emitted token | Why |
|---|---|---|
| val1=+9, val2=+810000 | `9.810000` | trivial positive |
| val1=-9, val2=+810000 | `-9.810000` | val1 carries sign; abs val2 |
| val1=-9, val2=-810000 | `-9.810000` | val1 carries sign; abs val2 |
| val1=0, val2=+500000 | `0.500000` | positive |
| val1=0, val2=-500000 | `-0.500000` | **edge case**: val2 carries sign, we emit a leading `-` and abs val2 |
| val1=0, val2=0 | `0.000000` | zero |

This is **not** a spec change: the frozen text shape is unchanged;
the helper only decides where the implicit sign goes when the two
sign-positions disagree.

### D.6 Counters

`T` matches the `v`/`L` counter contract:

- `s_uart_count` (in `devkit_app_state`): **+1 per `T`** (existing
  pre-switch increment).
- `s_transitions`, `s_button_count`, `s_ignored_count`: **unchanged**.
- `s_tx_sent` (in `devkit_uart_producer`): **+1 per `T`** (existing
  post-snprintf increment; applies to both success and error paths).
- `ROBOTOS_OBS dropped / herr / unhandled / rejected / throttled`:
  expected **0 throughout** (handler returns OK; sensor error path
  does not poison the dispatcher).

Phase 11E will record the actual values.

---

## E. Config Changes

Exact `prj.conf` delta:

```diff
 # Disable CTF tracing
 CONFIG_TRACING=n
+
+# Phase 11D: on-board MEMS accelerometer probe (LSM303AGR, lis2dh driver,
+# I2C1 @ 0x19). Adapter-level probe. No floating-point printf; no SPI;
+# no ADC; no Kconfig overrides for range/ODR/HP/trigger (driver defaults
+# are used). LIS2DH_TRIGGER_NONE is the first option of choice
+# LIS2DH_TRIGGER_MODE -> selected by default; no explicit override added.
+# See PHASE_11C_ACCEL_PROBE_SPEC.md §D for the frozen config rules.
+CONFIG_I2C=y
+CONFIG_SENSOR=y
```

**Forbidden Kconfigs are absent** (per Phase 11C §D.2):

- `CONFIG_SPI` -- not added, not selected by build.
- `CONFIG_ADC` -- not added, not selected by build.
- `CONFIG_CBPRINTF_FP_SUPPORT` -- not added (response path uses
  integer-only formatting).
- `CONFIG_LIS2DH_*` range/ODR/HP overrides -- not added (driver
  defaults are used).

**Optional Kconfig not needed** (per Phase 11C §D.3):

- `CONFIG_LIS2DH_TRIGGER_NONE=y` -- already set in the pristine
  generated `.config` because it is the first option in the
  `choice LIS2DH_TRIGGER_MODE` block. Adding it explicitly to
  `prj.conf` would be redundant Kconfig noise; we omit it.

---

## F. Response / Error Implementation

### F.1 Success response

```text
ACC x=<v1>.<v2_6d> y=<v1>.<v2_6d> z=<v1>.<v2_6d>\r\n
```

- Composed by `snprintf` into the same 96-byte stack `buf` used by
  every other `devkit_uart_emit_tx_response()` branch.
- Worst case 68 B; typical ~44 B; the existing `n > 0 && n < (int)sizeof(buf)`
  guard catches any future overflow.
- `<v1>` is signed decimal; `<v2_6d>` is 6-digit absolute fractional.
- Sign rule: see §D.5 (`PHASE_11C_FORMAT_SIGN_EDGE`).

### F.2 Error response

```text
ERR accel read=<errno>\r\n
```

- Worst case 28 B (15 chars prefix + 11 chars `INT32_MIN` + CRLF).
- `<errno>` is the **numeric** signed-decimal return value from the
  failing Zephyr API. Three error paths feed it:
  1. `device_is_ready(...)` false -> `-ENODEV` (`errno.h` numeric;
     resolved at build time, no hardcoded integer).
  2. `sensor_sample_fetch()` returns negative -> propagated verbatim.
  3. `sensor_channel_get()` returns negative -> propagated verbatim.
- No symbolic mapping. No retry.

### F.3 Handler return value

The handler returns `ROBOTOS_CORE_OK` in **all** outcomes (success
**and** sensor failure). Phase 11C §F.4 forbids `herr++` on sensor
read failure; the handler did its job by emitting the `ERR` line.

---

## G. Build Result

### G.1 Command

```
west build --pristine=always -d build-phase11d -b stm32f411e_disco RobotOS_v1.0/devkit
```

### G.2 Outcome

```
[161/161] Linking C executable zephyr\zephyr.elf
Memory region         Used Size  Region Size  %age Used
           FLASH:       41528 B       512 KB      7.92%
             RAM:       12352 B       128 KB      9.42%
        IDT_LIST:          0 GB        32 KB      0.00%
```

### G.3 FLASH / RAM delta vs Phase 10B-d baseline (`125779c`)

| Region | Phase 10B-d | Phase 11D | Delta |
|---|---:|---:|---:|
| FLASH | 36780 B | 41528 B | **+4748 B** |
| RAM | 12224 B | 12352 B | **+128 B** |
| FLASH headroom (512 KB) | 92.97% free | 92.08% free | -0.89% |
| RAM headroom (128 KB) | 90.67% free | 90.58% free | -0.09% |

Cost is dominated by the Zephyr sensor subsystem (`SENSOR=y`),
`drivers__i2c` (`i2c_ll_stm32_v1.c` + `i2c_ll_stm32.c`), and
`drivers__sensor__lis2dh` (`lis2dh.c` + `lis2dh_i2c.c` +
`lis2dh_spi.c` -- the SPI variant is compiled but only its I2C path
is active at runtime). Comfortably within the
`stm32f411e_disco` budget; no fault-handler / fault-stack
adjustment needed.

### G.4 Warnings

No **new** warnings introduced by Phase 11D. The two known
pre-existing warnings (`q_valid` unused in core and
`drivers__console` STDIN_USB-related) are unchanged.

### G.5 Built artifacts

- `build-phase11d/zephyr/zephyr.elf` -- main image
- `build-phase11d/zephyr/zephyr.hex`, `zephyr.bin` -- flash images
- `build-phase11d/zephyr/.config` -- generated Kconfig (inspected in §H)

---

## H. Generated `.config` Observations

Direct grep of `build-phase11d/zephyr/.config`:

```text
CONFIG_I2C=y                          # added by Phase 11D prj.conf
CONFIG_I2C_INIT_PRIORITY=50
CONFIG_I2C_LOG_LEVEL=3
CONFIG_I2C_LOG_LEVEL_DEFAULT=y
CONFIG_I2C_NXP_TRANSFER_TIMEOUT=0
CONFIG_I2C_STM32=y
CONFIG_I2C_STM32_INTERRUPT=y
CONFIG_I2C_STM32_V1=y
CONFIG_LIS2DH=y                       # selected by SENSOR + DT
CONFIG_LIS2DH_ACCEL_RANGE_RUNTIME=y   # driver default
CONFIG_LIS2DH_ODR_RUNTIME=y           # driver default
CONFIG_LIS2DH_OPER_MODE_NORMAL=y      # driver default
CONFIG_LIS2DH_TRIGGER_NONE=y          # driver default (first option)
CONFIG_SENSOR=y                       # added by Phase 11D prj.conf
CONFIG_SENSOR_INIT_PRIORITY=90
CONFIG_SENSOR_LOG_LEVEL=3
CONFIG_SENSOR_LOG_LEVEL_DEFAULT=y
```

Forbidden Kconfigs (per Phase 11C §D.2):

```text
# grep -E "^CONFIG_SPI=" build-phase11d/zephyr/.config            -> (no match)
# grep -E "^CONFIG_ADC=" build-phase11d/zephyr/.config            -> (no match)
# grep -E "^CONFIG_CBPRINTF_FP" build-phase11d/zephyr/.config     -> (no match)
```

All Phase 11C §D constraints satisfied.

`CONFIG_LIS2DH_TRIGGER_NONE=y` is set automatically as the first
option of the `choice LIS2DH_TRIGGER_MODE` block -- explicit
addition in `prj.conf` is unnecessary (§D.3 second column).

---

## I. Scope Guard Audit

All 12 UART TX scope-guard constraints from
[`PHASE_9EZ_CHECKPOINT.md`](PHASE_9EZ_CHECKPOINT.md) §H remain
intact after Phase 11D:

| # | Guard | Status after Phase 11D |
|---|---|---|
| 1 | No shell / prompt / interactive session | **Intact.** `CONFIG_SHELL=n`. |
| 2 | No parser framework, no token lexer, no grammar | **Intact.** `T` is one byte through the existing switch. |
| 3 | No command registry; no dynamic handler registration | **Intact.** Static switch only. |
| 4 | No multiline / framed protocol; no delimiter framing; no length prefix | **Intact.** One `\r\n`-terminated line per response. |
| 5 | No heap buffers; all response buffers stack-allocated; fixed size (96 B) | **Intact.** Same `buf[96]`. Per-axis helper writes into 24-byte stack scratch. |
| 6 | No response queue; TX synchronous in the handler | **Intact.** `uart_poll_out()` polled in-line. |
| 7 | No UART logging replacement; RTT remains canonical | **Intact.** `CONFIG_LOG_BACKEND_RTT=y`, `CONFIG_LOG_BACKEND_UART=n`. |
| 8 | No core/platform UART abstraction | **Intact.** UART use still confined to `devkit_uart_producer.c`. |
| 9 | No retry/ACK protocol | **Intact.** Single sensor read attempt; no resend. |
| 10 | No TX from ISR context | **Intact.** Handler runs in dispatcher thread context. |
| 11 | No promotion of `devkit_app_state` from devkit-local | **Intact.** No new exported symbol. |
| 12 | Single-byte command vocabulary preserved | **Intact.** `T` is one byte. |

Other invariants:

- **`core/` zero diff.** No file under `core/` touched.
- **`platform/` zero diff.** No file under `platform/` touched.
- **`tests/` zero diff.** No file under `tests/` touched.
- **Scheduler constants** (`ROBOTOS_CORE_MAX_EVENTS_PER_TICK = 1`,
  `ROBOTOS_EVENT_QUEUE_CAPACITY = 16`): unchanged.
- **Board DTS / board defconfig / B-revision overlay**: unchanged.
- **DTS overlay**: none created (`accel0` alias is already
  `status = "okay"` in upstream DTS for revision D).
- **No new translation unit added to `target_sources(...)`**.
- **No new Zephyr thread, no new K_WORK, no new timer, no new
  ISR.**
- **`v / L / d / a / s / r / ? / x` branches**: byte-for-byte
  unchanged (verified by reading the surrounding switch).
- **POST_FLASH_AUTOSTART discipline**: unchanged.

---

## J. Hardware Evidence Status

**`HARDWARE_EVIDENCE_PENDING_PHASE_11E`.**

Phase 11D is a **build-validated** close only. The board was **not**
flashed, RTT was **not** captured, the harness was **not** run, and
no operator-witnessed `OPERATOR_PHYSICAL_SANITY_CONFIRMED` reading
was recorded by this phase.

The `T T ?` host harness exists
([`run_phase11d_accel_probe_demo.ps1`](../../../tools/runtime/run_phase11d_accel_probe_demo.ps1))
and PowerShell-parses cleanly. Phase 11E (Evidence Closeout) is the
phase that runs it, records the counter table, and declares the
runtime PASS / FAIL verdict against the Phase 11C §I expectations.

Nothing in Phase 11D constitutes runtime validation of:

- I2C bus actually clocking PB6/PB9 at 400 kHz.
- LSM303AGR ACK at 0x19.
- WHO_AM_I register read on `sensor_sample_fetch()` first invocation
  (`lis2dh` driver probes WHO_AM_I in its `lis2dh_init()`).
- Three live `struct sensor_value` reads on `SENSOR_CHAN_ACCEL_XYZ`.
- `accepted - dispatched = pending = 1` invariant under the new `T`
  load.
- `CFSR/HFSR = 0` across an accel-read burst.

All of the above are the Phase 11E acceptance criteria.

---

## K. Next Gate

**Phase 11E -- Accelerometer Probe Evidence Closeout.** Hardware
evidence-only phase. Reserved; not opened by this document.

Phase 11E, when authorized by the user, must:

1. Flash the Phase 11D image (this commit's resulting `.hex`).
2. Run
   [`run_phase11d_accel_probe_demo.ps1`](../../../tools/runtime/run_phase11d_accel_probe_demo.ps1)
   with operator-supplied `-ComPort`. Use Phase 6O sidecar `reset
   run` discipline (the harness already does this when
   `-SkipCapture` is not set).
3. Capture host transcript and RTT log under
   `RobotOS_v1.0/devkit/logs/phase_11D_*_<date>.txt` (the harness
   default).
4. Verify the §I.1 / §I.2 expectations from Phase 11C against the
   captured RTT log.
5. Optionally record `OPERATOR_PHYSICAL_SANITY_CONFIRMED` if the Z
   axis reads near +9.8 m/s² with the board flat on the bench.
6. Publish a Phase 11E closeout doc here under `02_PHASE_CLOSEOUTS/`
   with status `CLOSED_WITH_HARDWARE_EVIDENCE`.

Phase 11E **does not** widen scope, change response format, or
modify any source.

Other open user decisions remain unchanged and decoupled from the
Phase 11A-E sensor track:

- ACTIVE disarm widening (`USER_DECISION_REQUIRED_ACTIVE_DISARM`).
- Scheduler 7A/7B (`DEFER`).
- F407 / custom board (`HOLD/DEFER`).
- POST_FLASH_AUTOSTART root cause (`OPEN`, `MITIGATED_BY_WORKFLOW`).

---

## L. What this document does not do

- Does not declare hardware evidence for `T`.
- Does not modify `core/`, `platform/`, `tests/`, scheduler constants,
  queue capacity, board DTS, board defconfig, B-revision overlay,
  Zephyr workspace tracked files, or any evidence log.
- Does not enable `CONFIG_SPI`, `CONFIG_ADC`, or
  `CONFIG_CBPRINTF_FP_SUPPORT`.
- Does not create or modify any DTS overlay.
- Does not authorize hardware purchase.
- Does not open Phase 11E (evidence closeout). Phase 11E opening is
  user-authorized.
- Does not start ACTIVE disarm widening.
- Does not change command semantics for
  `a / s / r / ? / x / v / L / d`.
- Does not promote `T` past `IMPLEMENTED_BUILD_VALIDATED_HARDWARE_EVIDENCE_PENDING`.
- Does not reopen Scheduler 7A/7B.
- Does not reopen F407 / custom-board.
- Does not change POST_FLASH_AUTOSTART status.
- Does not push.
