# Phase 11C -- On-board MEMS Accelerometer Probe Spec

**Status:** `CLOSED_DOCS_ONLY`
**Type:** Docs-only specification freeze. **No source, runtime, test,
CMake, Zephyr, board, `prj.conf`, DTS overlay, host-tool, or script
change.** Phase 11C does **not** implement `T`, does **not** add
`case 't':`, does **not** modify `prj.conf`, does **not** enable
`CONFIG_I2C` / `CONFIG_SENSOR` / `CONFIG_SPI` / `CONFIG_ADC`, does
**not** create a DTS overlay, does **not** authorize hardware
purchase, and does **not** open Phase 11D / Phase 11E.
**Date opened/closed:** 2026-05-12 (same-day docs-only close)
**Branch:** master
**Published baseline at open:** `origin/master = 2aa0435`
**Prior runtime behavior phase:** Phase 10B-d (firmware `125779c`,
evidence-close `7e250dc`).
**Prior docs-only checkpoints:** Phase 11B (`bae2436` → `90ef5cc`;
config baseline corrected at `2aa0435`); Phase 11A (`9e3daaf`); Phase
10C (`796e114`).
**Companion docs:**
[`../01_PROGRESS/DEVKIT_PROGRESS_PHASE_11_20.md`](../01_PROGRESS/DEVKIT_PROGRESS_PHASE_11_20.md),
[`../03_SPECS/COMMAND_SET_DRAFT.md`](../03_SPECS/COMMAND_SET_DRAFT.md),
[`../03_SPECS/TELEMETRY_REFERENCE.md`](../03_SPECS/TELEMETRY_REFERENCE.md),
[`PHASE_11B_DEVICE_DRIVER_FEASIBILITY.md`](PHASE_11B_DEVICE_DRIVER_FEASIBILITY.md),
[`PHASE_11A_ADAPTER_BOUNDARY_SENSOR_SURFACE.md`](PHASE_11A_ADAPTER_BOUNDARY_SENSOR_SURFACE.md),
[`PHASE_10C_COMMAND_SET_CHECKPOINT.md`](PHASE_10C_COMMAND_SET_CHECKPOINT.md),
[`PHASE_9EZ_CHECKPOINT.md`](PHASE_9EZ_CHECKPOINT.md).

---

## A. Executive Summary

Phase 11C freezes the on-board MEMS accelerometer probe specification
that future Phase 11D implementation must follow. It is the
operationalization of the Phase 11B feasibility finding
(`FEASIBILITY_CONFIRMED_ONBOARD_MEMS`) and is conditioned on the
board-revision confirmation completed after Phase 11B close (operator
confirmed revision **D**, classification `CONFIRMED_A_OR_D`).

**Three deliverables:**

1. **Target device freeze (§C).** Exact DT alias, driver, bus,
   address, channel, trigger mode, overlay/wiring assumptions, and
   board revision target are locked.
2. **Response/error/host-harness freeze (§E–§H).** Exact UART TX
   bytes Phase 11D must emit, byte-budget proof against the existing
   96-byte stack buffer, and the canonical host validation sequence
   `T T ?` are locked.
3. **Phase 11D / 11E placement (§D, §J).** Future Phase 11D config
   surface (`CONFIG_I2C=y` + `CONFIG_SENSOR=y`) and the explicit list
   of Kconfigs that must NOT be added (`CONFIG_SPI`, `CONFIG_ADC`,
   `CONFIG_CBPRINTF_FP_SUPPORT`) are stated so Phase 11D does not
   re-derive them.

Phase 11C **does not** implement, build, flash, or run anything. It
does not promote `T` from `USER_DECISION_REQUIRED`. Phase 11D
(implementation) and Phase 11E (evidence closeout) remain
`NOT_STARTED` and require explicit user approval to open.

---

## B. Baseline Before Phase 11C

| Item | Value | Source |
|---|---|---|
| `origin/master` at open | `2aa0435` (`docs: correct Phase 11 sensor config baseline`) | `git rev-parse origin/master` |
| Phase 11A | `CLOSED_DOCS_ONLY` | `DEVKIT_PROGRESS_PHASE_11_20.md` `<a id="phase-11a">` |
| Phase 11B | `CLOSED_DOCS_ONLY` | `DEVKIT_PROGRESS_PHASE_11_20.md` `<a id="phase-11b">` |
| Phase 11B result | `FEASIBILITY_CONFIRMED_ONBOARD_MEMS` | [`PHASE_11B_DEVICE_DRIVER_FEASIBILITY.md`](PHASE_11B_DEVICE_DRIVER_FEASIBILITY.md) §J |
| Phase 11B purchase | `NO_PURCHASE_NEEDED_FOR_NEXT_STEP` | [`PHASE_11B_DEVICE_DRIVER_FEASIBILITY.md`](PHASE_11B_DEVICE_DRIVER_FEASIBILITY.md) §F |
| Phase 11 config correction | `CONFIG_I2C`, `CONFIG_SPI`, `CONFIG_SENSOR`, `CONFIG_ADC` all absent from `devkit/prj.conf` (verified against Phase 4 bringup `43de448` and current HEAD) | corrected at `2aa0435` |
| Board revision | **D** | operator visual confirmation after Phase 11B push |
| Board revision classification | `CONFIRMED_A_OR_D` | operator report; consistent with Zephyr `revision.cmake` `DEFAULT_REVISION D` |
| `T` status | `USER_DECISION_REQUIRED` / not implemented | [`COMMAND_SET_DRAFT.md`](../03_SPECS/COMMAND_SET_DRAFT.md) Section B `T` row |
| `T` source presence | **none** -- no `case 't':` in any source file | grep `core/` / `platform/` / `devkit/src/` |
| Phase 11D | `NOT_STARTED` | this doc opens 11C only |
| Phase 11E | `NOT_STARTED` | this doc opens 11C only |
| ACTIVE disarm widening | `USER_DECISION_REQUIRED_ACTIVE_DISARM` / not started | [`PHASE_10C_COMMAND_SET_CHECKPOINT.md`](PHASE_10C_COMMAND_SET_CHECKPOINT.md) §5.1 |
| Scheduler 7A/7B | `DEFER` | [`PHASE_10C_COMMAND_SET_CHECKPOINT.md`](PHASE_10C_COMMAND_SET_CHECKPOINT.md) §5.3 |
| F407 / custom board | `HOLD/DEFER` | [`PHASE_10C_COMMAND_SET_CHECKPOINT.md`](PHASE_10C_COMMAND_SET_CHECKPOINT.md) §5.4 |
| UART TX scope | minimal response only; 12 scope-guard constraints intact | [`PHASE_9EZ_CHECKPOINT.md`](PHASE_9EZ_CHECKPOINT.md) §H |
| POST_FLASH_AUTOSTART | root cause `OPEN`; `MITIGATED_BY_WORKFLOW` from Phase 6O onward | unchanged |
| Last runtime behavior phase | Phase 10B-d (firmware `125779c`, evidence `7e250dc`) | unchanged |
| Validated non-sensor command set | `a / s / r / ? / x / v / L / d` (8 hardware-validated) | [`PHASE_10C_COMMAND_SET_CHECKPOINT.md`](PHASE_10C_COMMAND_SET_CHECKPOINT.md) §3 |

---

## C. Target Device Freeze

The following items are **frozen** for Phase 11D. Phase 11D may not
deviate from any row below without re-opening Phase 11C.

| Field | Frozen value |
|---|---|
| **Target command byte** | `T` (uppercase 0x54). Phase 11D **may** also accept `t` (0x74) for case-insensitivity, mirroring the existing `v`/`V` and `L`/`l` precedent; Phase 11C does **not** mandate lowercase acceptance, leaves it to Phase 11D's discretion within the existing case-insensitive convention. |
| **Command role** | Adapter-level driver-dependent read probe. **NOT** Framework. **NOT** Application. No calibration, no units conversion, no sensor identity beyond `ACC` line prefix. |
| **Board target (build command)** | `stm32f411e_disco` (no `@<rev>` suffix; revision D is Zephyr default per `zephyr/boards/arm/stm32f411e_disco/revision.cmake`) |
| **Board revision** | **D** (confirmed by operator post-Phase 11B) |
| **DT alias** | `accel0` |
| **DTS node label** | `lsm303agr_accel` |
| **Compatible strings** | `st,lis2dh`, `st,lsm303agr-accel` |
| **Zephyr driver** | `lis2dh` (path `zephyr/drivers/sensor/lis2dh/`, self-contained, no `hal_st` dependency) |
| **Bus** | I2C1 |
| **I2C address** | 0x19 |
| **I2C pins** | SCL = PB6, SDA = PB9 |
| **I2C speed** | 400 kHz (`I2C_BITRATE_FAST` in board DTS, unchanged) |
| **Read channel** | `SENSOR_CHAN_ACCEL_XYZ` — 3-axis vector returned as three `struct sensor_value` entries |
| **Read API** | `sensor_sample_fetch(dev)` followed by `sensor_channel_get(dev, SENSOR_CHAN_ACCEL_XYZ, accel[3])` |
| **Trigger mode** | **Polling only.** `CONFIG_LIS2DH_TRIGGER_NONE` (Zephyr Kconfig `choice LIS2DH_TRIGGER_MODE` first option, default when no override). Phase 11D **must not** select `LIS2DH_TRIGGER_GLOBAL_THREAD` or `LIS2DH_TRIGGER_OWN_THREAD`. |
| **Interrupt GPIOs (PE4 / PE5)** | **Reserved by DT but not used.** Phase 11D does not request the IRQ GPIOs, does not configure them, and does not route them to the event queue. |
| **Numeric domain** | Raw integer fixed-point via `struct sensor_value { int32_t val1; int32_t val2; }`. **No floating point.** No `sensor_value_to_double()`. |
| **DTS overlay** | **None expected.** Sensor node is already `status = "okay"` in upstream `stm32f411e_disco.dts` for revision D. If Phase 11D pristine build reveals the node is not enabled (unexpected), Phase 11D **must stop and report**, not silently add an overlay. |
| **External wiring** | **None.** Sensor is on-board. |
| **Hardware purchase** | **None.** Reaffirms Phase 11B `NO_PURCHASE_NEEDED_FOR_NEXT_STEP`. |
| **Source files that may be touched by Phase 11D** | `devkit/src/devkit_app_state.c`, `devkit/src/devkit_uart_producer.c`, `devkit/prj.conf`. Optionally a new `devkit_accel_probe.{c,h}` only if encapsulation is needed. See §J for the full implementation boundary. |
| **Source files that must NOT be touched by Phase 11D** | `core/`, `platform/`, scheduler constants, queue capacity macros, board DTS, board defconfig, B-revision overlay, `CMakeLists.txt` top-level, evidence logs, `DEVKIT_PROGRESS.md` historical master. |

---

## D. Future Phase 11D Kconfig / Config Freeze

### D.1 Required Phase 11D `prj.conf` additions

Phase 11D **must add** exactly the following two lines to
`RobotOS_v1.0/devkit/prj.conf`:

```text
CONFIG_I2C=y
CONFIG_SENSOR=y
```

Rationale:

- `CONFIG_I2C=y` — neither board defconfig nor current
  `devkit/prj.conf` enables I2C. The `lis2dh` driver's Kconfig
  `select I2C if $(dt_compat_on_bus,...)` only selects I2C when the
  driver itself is being built; the I2C subsystem still has to be
  enabled at app level. Verified by Phase 11B preflight build of the
  Zephyr `lis2dh` sample (which carries `CONFIG_I2C=y` in its own
  `prj.conf`).
- `CONFIG_SENSOR=y` — `zephyr/drivers/sensor/Kconfig` wraps every
  sensor driver under `menuconfig SENSOR` with an `if SENSOR` block.
  Without `CONFIG_SENSOR=y`, `lis2dh` is dropped from the build.

### D.2 Forbidden Phase 11D `prj.conf` additions

Phase 11D **must not add** any of the following unless a future
approved spec change reopens that surface:

| Kconfig | Why forbidden |
|---|---|
| `CONFIG_SPI` | SPI bus is not needed for the I2C-attached LSM303AGR accelerometer. Adding it would pull in driver code, complicate the binary, and exceed Phase 11C scope. The on-board SPI gyro path is `NOT_CHARACTERIZED` (Phase 11B §C row 6, Phase 11B preflight §C). |
| `CONFIG_ADC` | ADC is the die-temperature path (Phase 11B §C row 1, `FEASIBLE_AFTER_DRIVER_VERIFICATION`). Phase 11C does not target die temperature; adding ADC widens surface without benefit. |
| `CONFIG_CBPRINTF_FP_SUPPORT` | The Zephyr `lis2dh` sample uses `printf("%f")` via `sensor_value_to_double()` and therefore needs FP printf support. Phase 11C **explicitly forbids** floating point in the Phase 11D response path (see §E). Adding FP support would inflate FLASH/RAM with no caller. |
| `CONFIG_LIS2DH_*` runtime range/ODR overrides | Phase 11D uses the driver defaults (`LIS2DH_ACCEL_RANGE_RUNTIME` and `LIS2DH_ODR_RUNTIME`). No range override, no ODR override, no high-pass filter, no temperature option, no block-data-update tweak. |

### D.3 Optional Phase 11D `prj.conf` addition (Phase 11D must verify)

| Kconfig | When to add | When to omit |
|---|---|---|
| `CONFIG_LIS2DH_TRIGGER_NONE=y` | If a Phase 11D pristine build's generated `.config` does **not** set `CONFIG_LIS2DH_TRIGGER_NONE=y` by default (i.e. a different trigger choice was selected upstream). | If pristine build already sets `CONFIG_LIS2DH_TRIGGER_NONE=y` by default (expected, since it is the first option in the `choice LIS2DH_TRIGGER_MODE` block). |

Phase 11D must inspect `<build>/zephyr/.config` for
`CONFIG_LIS2DH_TRIGGER_NONE=y` and add the explicit line only if
absent. Avoid unnecessary Kconfig noise.

### D.4 No DTS overlay

Phase 11D must not create a DTS overlay. The board DTS already
enables `i2c1` with the correct pinctrl and the `lsm303agr_accel`
node with `status = "okay"`. If Phase 11D's pristine build reveals a
missing node (unexpected), Phase 11D **must stop and report**, not
silently add an overlay.

---

## E. Response Format Freeze (Success Path)

### E.1 Frozen success response line

```text
ACC x=<x1>.<x2_6d> y=<y1>.<y2_6d> z=<z1>.<z2_6d>\r\n
```

Where for each axis the format substitution is:

| Token | Source | Rendering rule |
|---|---|---|
| `<n1>` | `accel[i].val1` (`int32_t`) | Signed decimal, no padding, no leading zero. Sign-carrying integer part. |
| `<n2_6d>` | `accel[i].val2` (`int32_t`) | **Absolute value** of `val2`, zero-padded to 6 digits. The sign is carried by `<n1>`; do not double-print the sign. |

Notes:

- Three axes printed in fixed order: X, Y, Z.
- Single ASCII space between axis triplets, no comma.
- Line terminator is `\r\n` (CRLF), matching all existing Phase 9E /
  Phase 10B-{v,L,d} responses.
- No leading STATUS keyword (`OK`/`STATE`/`INFO`/`ERR`). `ACC` is the
  STATUS token itself, following the `STATE` and `INFO` precedent for
  pure-data responses.
- No trailing fields. No timestamp. No counter echo. No range
  identifier. No sensor-identity string. No `seq=` token.

### E.2 Byte-budget proof against the 96-byte stack buffer

The existing `devkit_uart_producer.c` stack response buffer is 96
bytes (Phase 9E baseline, unchanged through Phase 10B-{v,L,d}).
Worst-case `ACC` line length must fit, including `\r\n`.

**Worst-case `int32_t` decimal width:**

- `INT32_MIN = -2147483648` → 11 chars
- `INT32_MAX = +2147483647` → 10 chars (no leading `+`)
- Conservative upper bound: **11 chars per `val1`**.

**Worst-case `val2` absolute width:** 6 chars (`000000` … `999999`).

**Line composition (worst case):**

| Segment | Chars |
|---|---|
| `ACC ` (prefix) | 4 |
| `x=` + 11 + `.` + 6 | 20 |
| ` y=` + 11 + `.` + 6 | 21 |
| ` z=` + 11 + `.` + 6 | 21 |
| `\r\n` | 2 |
| **Total (worst case)** | **68 bytes** |

**Headroom:** 96 − 68 = **28 bytes** spare. The line fits with margin.

**Typical case** (`±2g` default range, `val1` in `±19 m/s²` magnitude
i.e. ≤3 chars including sign): `ACC x=-19.999999 y=-19.999999 z=-19.999999\r\n` = **44 bytes**.

### E.3 Why not `ACCEL`

The earlier `COMMAND_SET_DRAFT.md` Section B placeholder showed the
prefix as `ACCEL`. Phase 11C reduces the prefix to `ACC` to claim 2
extra bytes of headroom on every successful response (still fits
either way; `ACC` simply leaves more margin in case Phase 11D
discovers a larger sensor_value width). Both `ACC` and `ACCEL` would
have been acceptable; Phase 11C freezes `ACC`. The
`COMMAND_SET_DRAFT.md` Section B `T` row is updated in this same
commit to match.

### E.4 What the success response does NOT contain

- No floating-point literal anywhere on the line.
- No unit token (`m/s2`, `g`, etc.). Units are Framework-class.
- No sample timestamp.
- No sequence number.
- No status counter echo. (`?` remains the canonical state/counter
  query; `T` does not duplicate it.)
- No sensor part name (`LSM303AGR`, `LIS2DH`, `LSM303DLHC`). Sensor
  identity is Framework-class.
- No multi-line output. Phase 9E-Z scope-guard #4 (no framing) holds.

---

## F. Error Response Freeze

### F.1 Frozen error response line

```text
ERR accel read=<errno>\r\n
```

Where:

- `<errno>` is the **numeric** signed-decimal value returned by the
  Zephyr API that failed (typically `sensor_sample_fetch()` or
  `sensor_channel_get()`). Phase 11D must **not** map errno to a
  symbolic string (`EIO`, `EINVAL`, etc.) at this layer — keep it
  numeric to preserve fixed-buffer bound and avoid pulling in
  string-translation glue.

### F.2 Byte-budget proof

| Segment | Chars |
|---|---|
| `ERR accel read=` | 15 |
| `<errno>` worst case `INT32_MIN` | 11 |
| `\r\n` | 2 |
| **Total (worst case)** | **28 bytes** |

Well within the 96-byte buffer.

### F.3 Error-source mapping

Phase 11D must emit `ERR accel read=<errno>` in the following
situations:

| Error path | `<errno>` value (Phase 11D must propagate, not invent) |
|---|---|
| `DEVICE_DT_GET(...)` returns `NULL` (device not found at all) | Emit a **distinct sentinel**: `ERR accel read=-19` (chosen to match `-ENODEV` numeric on Zephyr; Phase 11D must verify the actual `-ENODEV` value at build time and use it literally). |
| `device_is_ready(dev)` returns `false` | Same sentinel as above (`-ENODEV` or equivalent). Do not invent a new error code. |
| `sensor_sample_fetch(dev)` returns `< 0` | The driver's return value, sign preserved. Special case: `-EBADMSG` returned in polled mode by `lis2dh` means "sample overrun" — Phase 11D **must** treat this as transient (re-emit nothing extra, do not enter error path; documented in Phase 11D under "expected polled-mode behavior"). |
| `sensor_channel_get(dev, SENSOR_CHAN_ACCEL_XYZ, &accel)` returns `< 0` | The driver's return value, sign preserved. |

### F.4 What the error response does NOT do

- No retry inside the handler. Single attempt; report; return.
- No back-off, no exponential timing.
- No ACK/NACK / sequence-number protocol.
- No multi-line diagnostic.
- No app-state change. `state`, `transitions`, `button` unchanged.
- No `ignored++`. `T` is a recognized command; an internal sensor
  failure is **not** an "ignored" event. `ignored` continues to count
  unknown command bytes only (the Phase 9E `x` baseline contract).
- No fault injection into `core/`. The Zephyr core, queue, scheduler,
  and `platform/` substrate must observe no behavior change from a
  sensor read failure.

---

## G. Future `T` Semantics (Phase 11D contract)

Phase 11D implementation of `T` must obey the following behavioral
contract:

| Aspect | Contract |
|---|---|
| Recognition | `T` (and optionally `t`) recognized in `devkit_app_state.c` `devkit_app_state_on_uart_byte()` switch, matching the `v`/`L`/`d` precedent. |
| App-state transition | **None.** State stays whatever it was (`IDLE` / `ARMED` / `ACTIVE`). |
| `transitions` counter | **Not incremented.** |
| `ignored` counter | **Not incremented.** (Recognized command; not a "Phase 9E `x`-style unknown byte".) |
| `button` counter | **Not incremented.** |
| `uart` counter | **Incremented by 1 per `T`**, matching every other recognized command's counter accounting. |
| Read context | **Thread/handler context** (the UART-byte event handler). Phase 9E-Z scope-guard #10 (no TX from ISR) holds. |
| Read API | `sensor_sample_fetch()` then `sensor_channel_get()` — no Zephyr work-queue, no own-thread, no triggers. |
| Response emission | Synchronous within the same handler invocation, via the existing `uart_poll_out()` path used by `v`/`L`/`d`/`a`/`s`/`r`/`?`/`x`. |
| Response buffer | Existing 96-byte stack buffer in `devkit_uart_producer.c`. **No new buffer.** No heap. No queue. |
| `?` response | **Unchanged.** No accel fields added to the `STATE …` query. Phase 9E-Z scope-guard #1 (no shell), #2 (no parser), #4 (no framing) all hold. |
| Other commands (`a / s / r / ? / x / v / L / d`) | **Untouched.** Byte branches identical to Phase 10B-d. |
| LED side-effect | **None.** `T` does not call `devkit_status_led_toggle()`. |
| Build-info side-effect | **None.** |
| Timing budget | The Phase 9E-Z command-byte handler is expected to complete within one dispatch tick (`MAX_EVENTS_PER_TICK=1`, `DEVKIT_TICK_MS=500`). An I2C polled read of three accel channels at 400 kHz is far below 500 ms; Phase 11D must verify on hardware in Phase 11E. |

---

## H. Host Harness Spec

### H.1 Frozen canonical sequence

```text
T T ?
```

Three host commands; three responses expected, in order.

Rationale:

- **Two consecutive `T`** verifies (a) the command path is
  idempotent in app-state terms, (b) the sensor read repeats cleanly,
  (c) per-command `uart` counter increment is consistent.
- **Trailing `?`** verifies app-state invariants: `transitions`
  unchanged, `ignored` unchanged, `uart` advanced by exactly 3.
- **No `x`** in the canonical sequence. The Phase 9E `x` baseline
  already characterizes the unknown-byte error path; re-running it
  here would not add Phase 11C-specific evidence. (If Phase 11E
  later wants to confirm `T` does not poison the `x` branch, a
  supplemental sequence `T x T ?` is permitted but **not** in the
  frozen canonical sequence.)

### H.2 Expected host transcript (golden)

```text
SEND 'T' (0x54) -> RECV: ACC x=<v1>.<v2_6d> y=<v1>.<v2_6d> z=<v1>.<v2_6d>
SEND 'T' (0x54) -> RECV: ACC x=<v1>.<v2_6d> y=<v1>.<v2_6d> z=<v1>.<v2_6d>
SEND '?' (0x3f) -> RECV: STATE state=<S> transitions=<N> button=<N> uart=<U+3> ignored=<N>
```

The two `ACC` lines need **not** be byte-identical — the sensor is
expected to produce live samples; minor X/Y/Z variation is normal.
Phase 11E judges PASS by **shape** match plus invariant checks
(§I), not by byte-for-byte response equality between the two `T`
responses.

The `?` line's `transitions`, `button`, `ignored` values must equal
the **pre-sequence** values (no change). The `uart` value must equal
the pre-sequence value **plus 3** (two `T` + one `?`).

### H.3 Initial app state for the canonical run

The canonical sequence assumes the runtime starts from `IDLE` (the
Phase 9E baseline). Phase 11E may optionally repeat the sequence
from `ARMED` and `ACTIVE` to confirm state invariance of `T`; this
is recommended-but-not-mandatory and **does not** widen the canonical
sequence.

### H.4 Host harness script (Phase 11D scope, named here only)

Suggested filename (Phase 11D creates it; Phase 11C only names it):
`RobotOS_v1.0/tools/runtime/run_phase11d_accel_probe_demo.ps1`. Must
follow Phase 6O `capture_devkit_rtt.ps1` sidecar `reset run`
discipline; pure ASCII PowerShell 5.1; manual RESET retained as
fallback; plain `west flash` alone is **not** runtime-start evidence.

---

## I. RTT / Counter Expectations

### I.1 Counters that must move

For canonical sequence `T T ?` from `IDLE` baseline:

| Counter | Pre | Delta | Post |
|---|---|---|---|
| `ROBOTOS_UART rx` | N | **+3** | N+3 |
| `ROBOTOS_UART ok` | N | **+3** | N+3 |
| `ROBOTOS_UART handled` | N | **+3** | N+3 |
| `ROBOTOS_UART last` | any | sets to last byte | **0x3f** (`?`) |
| `ROBOTOS_APP uart` | N | **+3** | N+3 |
| `ROBOTOS_OBS accepted` | N | **+3** | N+3 |
| `ROBOTOS_OBS dispatched` | N-1 | **+3** | N+2 |

(`accepted - dispatched = pending = 1` invariant holds throughout.)

### I.2 Counters that must NOT move

| Counter | Required behavior |
|---|---|
| `ROBOTOS_APP transitions` | **Unchanged.** `T` does not transition. |
| `ROBOTOS_APP ignored` | **Unchanged.** `T` is recognized. |
| `ROBOTOS_APP button` | **Unchanged.** No button presses. |
| `ROBOTOS_UART full` | **Unchanged at 0.** No queue overflow expected from 3 byte-paced host commands. |
| `ROBOTOS_OBS dropped` | **0 throughout.** |
| `ROBOTOS_OBS herr` | **0 throughout.** Handler returns OK; sensor error path emits TX response and returns OK from handler. |
| `ROBOTOS_OBS unhandled` | **0 throughout.** |
| `ROBOTOS_OBS rejected` | **0 throughout.** |
| `ROBOTOS_OBS throttled` | **0 throughout.** |
| `ROBOTOS_PROD attempted / ok` | Continues at the existing Phase 6M cadence (1 post per 2 ticks, ~60 over a 60 s window). `T` does not affect the producer. |
| `ROBOTOS_FAULT active` | **0 throughout.** |
| `CFSR` | **`0x00000000`** at every fault sample. |
| `HFSR` | **`0x00000000`** at every fault sample. |

### I.3 `peak` expectation (not overclaimed)

`ROBOTOS_OBS peak` is expected to remain **low** (`≤ 2` per the
Phase 10B-{v,L,d} precedent for three byte-paced host commands).
Phase 11E **must record the actual peak observed** rather than
asserting a strict upper bound; if the observed peak rises above the
Phase 10B baseline, Phase 11E must flag it as a finding (likely
sensor-read latency interleaving with the heartbeat producer) and not
classify it as a regression unless `dropped` or `herr` also rise.

### I.4 Cross-phase invariants (must all hold)

- `accepted - dispatched = pending` (= 1 at steady state)
- `PROD ok + UART ok = accepted`
- `UART rx = UART handled = APP uart`
- `transitions(post) = transitions(pre)` (i.e. delta = 0 for this sequence)
- `ignored(post) = ignored(pre)` (i.e. delta = 0 for this sequence)
- `CFSR | HFSR = 0` at every fault sample

### I.5 What Phase 11E must record

Phase 11E (evidence closeout) must include, at minimum:

- Pristine `west build` log (FLASH/RAM delta vs Phase 10B-d baseline
  `125779c`).
- `west flash` log.
- Phase 6O `capture_devkit_rtt.ps1` sidecar `reset run` confirmation.
- Host transcript file under `RobotOS_v1.0/devkit/logs/`.
- RTT log file under `RobotOS_v1.0/devkit/logs/`.
- Observed final counter table (matching the I.1/I.2/I.3 schema).
- Invariant cross-check (I.4).
- Operator-witnessed sample-value sanity check (e.g. "Z reads near
  +9.8 m/s² when board is flat on bench" — optional, recommended;
  documented as `OPERATOR_PHYSICAL_SANITY_CONFIRMED` if recorded).

Phase 11E does **not** need to byte-match the two `ACC` lines or
assert a specific X/Y/Z range — the sensor is live.

---

## J. Phase 11D Implementation Boundary

### J.1 Files Phase 11D **may** touch (devkit-local only)

| File | Allowed delta |
|---|---|
| `RobotOS_v1.0/devkit/prj.conf` | Append `CONFIG_I2C=y` and `CONFIG_SENSOR=y`. Optionally `CONFIG_LIS2DH_TRIGGER_NONE=y` only if §D.3 verification shows it is needed. **No other Kconfig.** |
| `RobotOS_v1.0/devkit/src/devkit_app_state.c` | Add `case 'T':` (and optionally `case 't':`) branch that recognizes `T` as a non-transitioning command (LOG_INF, no state change, not ignored). Pattern: copy the structure of the existing `case 'v':` branch. |
| `RobotOS_v1.0/devkit/src/devkit_uart_producer.c` | Add `case 'T':` (and optionally `case 't':`) TX response branch that calls `sensor_sample_fetch()` + `sensor_channel_get()` and emits the §E.1 response line on success or §F.1 line on error, using the existing 96-byte stack buffer. Add `#include <zephyr/drivers/sensor.h>` and `#include <zephyr/device.h>` if not already present transitively. |
| `RobotOS_v1.0/devkit/src/devkit_app_state.h` | Doc-only update of the recognized-command list / state-machine diagram comment block to include `T`. **No new exported function.** |
| `RobotOS_v1.0/tools/runtime/run_phase11d_accel_probe_demo.ps1` | New file. PowerShell 5.1, pure ASCII, Phase 6O sidecar-`reset-run` discipline. |
| Optional: `RobotOS_v1.0/devkit/src/devkit_accel_probe.{c,h}` | New devkit-local helper if the body of the `case 'T':` branch is large enough to warrant encapsulation. **Optional.** Encapsulation does not change the contract. |

### J.2 Files Phase 11D **must not** touch

- `RobotOS_v1.0/core/` — any file. Phase 9A–11C invariant.
- `RobotOS_v1.0/platform/` — any file.
- `RobotOS_v1.0/tests/` — any file.
- `RobotOS_v1.0/devkit/CMakeLists.txt` top-level — Zephyr sensor
  subsystem is pulled by `CONFIG_SENSOR=y` automatically; no source
  list edit needed unless §J.1's optional helper file is added (in
  which case its `.c` file goes into `target_sources(...)` — that is
  the only allowed `CMakeLists.txt` change).
- `RobotOS_v1.0/devkit/src/devkit_runtime.{c,h}` — heartbeat
  unchanged.
- `RobotOS_v1.0/devkit/src/devkit_status_led.{c,h}` — LED module
  untouched.
- `RobotOS_v1.0/devkit/src/devkit_button_producer.c` — button
  producer untouched.
- `RobotOS_v1.0/devkit/src/devkit_timer_producer.c` — timer producer
  untouched.
- `RobotOS_v1.0/devkit/src/devkit_observability.c`,
  `devkit_build_info.c`, `devkit_fault.c` — observability untouched.
- Scheduler constants (`ROBOTOS_CORE_MAX_EVENTS_PER_TICK = 1`).
- Event queue capacity (`ROBOTOS_EVENT_QUEUE_CAPACITY = 16`).
- Board DTS, B-revision overlay, board defconfig, board Kconfig.
- Zephyr workspace tracked files.
- `DEVKIT_PROGRESS.md` historical master.
- Any existing evidence log under `RobotOS_v1.0/devkit/logs/`.

### J.3 Phase 11D scope-guard restatement (still intact)

All 12 UART TX scope-guard constraints from
[`PHASE_9EZ_CHECKPOINT.md`](PHASE_9EZ_CHECKPOINT.md) §H must remain
intact after Phase 11D closes:

1. No shell / prompt / interactive session
2. No parser framework, no token lexer, no grammar
3. No command registry; no dynamic handler registration
4. No multiline / framed protocol; no delimiter framing; no length prefix
5. No heap buffers; all response buffers stack-allocated; fixed size (96 B)
6. No response queue; TX synchronous in the handler
7. No UART logging replacement; RTT remains canonical
8. No core/platform UART abstraction
9. No retry/ACK protocol
10. No TX from ISR context
11. No promotion of `devkit_app_state` from devkit-local
12. Single-byte command vocabulary preserved (`T` is single-byte)

---

## K. Non-goals

Phase 11C explicitly does **not**:

- Implement `T`. No `case 't':` or `case 'T':` added to any source.
- Modify `RobotOS_v1.0/devkit/prj.conf`.
- Enable `CONFIG_I2C`, `CONFIG_SENSOR`, `CONFIG_SPI`, `CONFIG_ADC`, or
  `CONFIG_CBPRINTF_FP_SUPPORT`.
- Create or modify any DTS overlay.
- Authorize or execute a hardware purchase.
- Run any build, flash, or runtime evidence capture.
- Open Phase 11D (implementation) or Phase 11E (evidence closeout).
- Start ACTIVE disarm widening; `USER_DECISION_REQUIRED_ACTIVE_DISARM`
  is preserved and remains decoupled from the Phase 11A-E sensor
  track.
- Promote `T` from `USER_DECISION_REQUIRED` in
  [`COMMAND_SET_DRAFT.md`](../03_SPECS/COMMAND_SET_DRAFT.md) Section
  B. The `T` row is updated to reference Phase 11C as its frozen
  spec, but the row stays in Section B.
- Introduce a Robot Framework sensor abstraction. Calibration, units,
  sensor identity, multi-sensor fusion remain Framework-class and are
  **not** approved by Phase 11C.
- Introduce Application / product semantics. `T` is a probe of the
  Adapter surface only.
- Reopen Scheduler 7A/7B. `DEFER` preserved.
- Reopen F407 / custom-board. `HOLD/DEFER` preserved.
- Expand UART TX scope. Minimal response only; 12 scope-guard
  constraints intact.
- Modify any closed phase's evidence or closeout doc.
- Modify `DEVKIT_PROGRESS.md` (historical master).
- Change command semantics for `a / s / r / ? / x / v / L / d`.
- Change the POST_FLASH_AUTOSTART discipline. Root cause `OPEN`;
  `MITIGATED_BY_WORKFLOW` from Phase 6O onward via
  `capture_devkit_rtt.ps1` sidecar `reset run`.
- Push.

---

## L. Decision Result

**`PHASE_11C_SPEC_FROZEN_ACCEL_LIS2DH_REV_D`**

All required items (target device §C, future config §D, success
response §E, error response §F, future semantics §G, host harness
§H, RTT expectations §I, implementation boundary §J) are frozen.
No blocker remains for Phase 11D opening **once user explicitly
approves code/config changes**.

Phase 11C does not by itself authorize Phase 11D. Phase 11D opening
requires:

1. Explicit user authorization (this doc does not constitute that
   authorization).
2. A new task that opens Phase 11D under the boundary defined in §J.
3. Phase 11D's closing closeout will be Phase 11E (evidence-only,
   hardware-validated).

---

## M. Next Gate

**Phase 11D — Sensor Probe Implementation (firmware).** Reserved; not
opened by this document.

Phase 11D, when authorized by the user, must:

- Add `CONFIG_I2C=y` and `CONFIG_SENSOR=y` (and optionally
  `CONFIG_LIS2DH_TRIGGER_NONE=y` only per §D.3) to
  `devkit/prj.conf` — **two or three lines, nothing else**.
- Add `case 'T':` (and optionally `case 't':`) branches to
  `devkit_app_state.c` and `devkit_uart_producer.c` per the §C / §E /
  §F / §G contract.
- Add the host harness script per §H.4.
- Build pristine (`west build --pristine -b stm32f411e_disco`),
  observe FLASH/RAM delta against Phase 10B-d baseline `125779c`
  (FLASH 36780 B / RAM 12224 B), and verify no new warnings beyond
  the established `q_valid` and `drivers__console` pre-existing
  warnings.

Phase 11E (evidence closeout) follows Phase 11D and must record the
counter table per §I and the host transcript per §H.2.

---

## N. What this document does not do

- Does not implement `T`.
- Does not add `case 't':` or `case 'T':` to any source file.
- Does not modify `RobotOS_v1.0/devkit/prj.conf`.
- Does not enable `CONFIG_I2C`, `CONFIG_SENSOR`, `CONFIG_SPI`,
  `CONFIG_ADC`, or `CONFIG_CBPRINTF_FP_SUPPORT`.
- Does not create or modify any DTS overlay.
- Does not authorize hardware purchase.
- Does not run any build, flash, or evidence capture.
- Does not modify `core/`, `platform/`, `tests/`, runtime scripts
  beyond naming them in §H.4 / §J.1, or any evidence log.
- Does not modify any closed phase's evidence or closeout doc.
- Does not modify `DEVKIT_PROGRESS.md` historical master.
- Does not change UART TX scope; all 12 scope-guard constraints
  intact.
- Does not change command semantics for
  `a / s / r / ? / x / v / L / d`.
- Does not promote `T` from `USER_DECISION_REQUIRED`.
- Does not start ACTIVE disarm widening.
- Does not reopen Scheduler 7A/7B.
- Does not reopen F407 / custom-board.
- Does not change POST_FLASH_AUTOSTART status.
- Does not push.
