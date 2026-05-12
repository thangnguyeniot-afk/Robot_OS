# PHASE_11B_DEVICE_DRIVER_FEASIBILITY.md

**Phase:** 11B — Device / Driver Feasibility Gate
**Status:** `CLOSED_DOCS_ONLY`
**Type:** Docs-only feasibility audit / purchase gate. No source, runtime,
test, CMake, Zephyr, board, `prj.conf`, DTS overlay, host-tool, or script
change.
**Date opened/closed:** 2026-05-12
**Published baseline at open:** `origin/master = bae2436`
**Closeout doc:** this file.
**Phase log entry:**
[`../01_PROGRESS/DEVKIT_PROGRESS_PHASE_11_20.md`](../01_PROGRESS/DEVKIT_PROGRESS_PHASE_11_20.md)
`<a id="phase-11b"></a>`

---

## A. Executive Summary

Phase 11B is the Device / Driver Feasibility Gate. Its sole purpose is to
determine—before any purchase or `prj.conf`/source change—which candidate
sensor path is safest and most feasible for the future Phase 11C/11D bounded
Adapter probe.

**Phase 11B findings:**

1. **The on-board LSM303AGR accelerometer (sensor node `lsm303agr_accel`,
   Zephyr driver `lis2dh`) is the recommended candidate.** It is already
   defined with `status = "okay"` in the upstream Zephyr board DTS, the
   `lis2dh` driver is present locally and is self-contained (no external HAL
   module dependency), `CONFIG_I2C=y` is already in `devkit/prj.conf`, and
   no hardware purchase is needed. The only `prj.conf` addition required in
   Phase 11D is `CONFIG_SENSOR=y`.

2. **The STM32 internal die-temperature / ADC path is viable in principle
   but more complex.** It requires `CONFIG_ADC=y` (not currently present),
   enabling the `adc1` node (disabled by default in the SoC DTSI), and
   adding a compatible string to the `die_temp` DTS node. It requires more
   DTS/overlay work than the on-board MEMS path.

3. **The on-board LSM303AGR magnetometer (`lis2mdl`) is blocked.** The
   `lis2mdl` driver requires `ZEPHYR_HAL_ST_MODULE`/`HAS_STMEMSC`, and
   `hal_st` is not present in the workspace (`modules/hal` contains only
   `cmsis`, `espressif`, `nordic`, `stm32`).

4. **No hardware purchase is needed for the next step.** External sensor
   candidates (I2C / SPI modules) are deprioritized while on-board resources
   are sufficient.

5. **Phase 11B corrects the Phase 11A priority ordering.** Phase 11A listed
   die-temperature as priority (a) and on-board MEMS as priority (b). The
   audit inverts this: the on-board MEMS accelerometer path is simpler and
   fully characterized locally; die temp is viable but requires more
   infrastructure changes.

**Phase 11B does not:**
- implement `T`
- change `prj.conf`, any overlay, any source file, or any runtime behavior
- approve any hardware purchase
- open Phase 11C (that requires explicit user authorization)

---

## B. Baseline Before Phase 11B

| Item | Value | Source |
|---|---|---|
| Published baseline | `origin/master = bae2436` | `git rev-parse origin/master` |
| Phase 11A | `CLOSED_DOCS_ONLY` | `DEVKIT_PROGRESS_PHASE_11_20.md` `<a id="phase-11a">` |
| Sensor surface classification | `SENSOR_SURFACE_DECIDED_ADAPTER_PROBE` | `PHASE_11A_ADAPTER_BOUNDARY_SENSOR_SURFACE.md` §F |
| `T` status | `USER_DECISION_REQUIRED` / not implemented | `COMMAND_SET_DRAFT.md` Section B `T` row |
| `CONFIG_I2C` in `devkit/prj.conf` | `y` (pre-existing from Phase 4 bringup, `43de448`) | `devkit/prj.conf` line 19 |
| `CONFIG_SPI` in `devkit/prj.conf` | `y` (pre-existing from Phase 4 bringup, `43de448`) | `devkit/prj.conf` line 20 |
| `CONFIG_SENSOR` in `devkit/prj.conf` | **not present** | `devkit/prj.conf` grep |
| `CONFIG_ADC` in `devkit/prj.conf` | **not present** | `devkit/prj.conf` grep |
| DTS overlay in `devkit/` | **none present** (confirmed by grep) | filesystem audit |
| `devkit/boards/` directory | **does not exist** | filesystem audit |
| Existing `T` implementation | **none** — no `case 't':` in any source file | `core/` / `platform/` / `devkit/src/` grep |
| ACTIVE disarm widening | `USER_DECISION_REQUIRED_ACTIVE_DISARM` / not started | `PHASE_10C_COMMAND_SET_CHECKPOINT.md` §5.1 |
| Scheduler 7A/7B | `DEFER` | `PHASE_10C_COMMAND_SET_CHECKPOINT.md` §5.3 |
| F407 / custom board | `HOLD/DEFER` | `PHASE_10C_COMMAND_SET_CHECKPOINT.md` §5.4 |
| UART TX scope | minimal response only; all 12 scope-guard constraints intact | `PHASE_9EZ_CHECKPOINT.md` §H |
| POST_FLASH_AUTOSTART | root cause `OPEN`; `MITIGATED_BY_WORKFLOW` from Phase 6O onward | `PHASE_10C_COMMAND_SET_CHECKPOINT.md` §8 |

---

## C. Candidate Feasibility Matrix

| Candidate | Device required? | Purchase required? | Local DTS evidence | Local driver evidence | Overlay / DTS change required? | `prj.conf` impact (future Phase 11D) | Risk | Adapter evidence value | Recommendation |
|---|---|---|---|---|---|---|---|---|---|
| **1. STM32 internal die temp / ADC** | No (internal to STM32F411) | No | `die_temp` node in `stm32f411.dtsi`; uses `io-channels = <&adc1 18>`; no compatible string matching `stm32_temp` driver; `adc1` is disabled in parent DTSI | `stm32_temp` driver at `zephyr/drivers/sensor/stm32_temp/`, present locally | Yes — must enable `adc1`, add compatible string to `die_temp` node, or add DT alias; requires DTS overlay | `CONFIG_ADC=y` + `CONFIG_STM32_TEMP=y` + `CONFIG_SENSOR=y` | Medium — ADC configuration, DTS overlay work, verify adc1 availability on F411 | Moderate — proves ADC path but channel 18 (internal temp) is the most constrained ADC use case | **FEASIBLE_AFTER_DRIVER_VERIFICATION** |
| **2. On-board `lsm303agr_accel` (lis2dh driver)** | No (on-board, all board revisions) | **No** | `lsm303agr_accel` node in `stm32f411e_disco.dts`, `status = "okay"`, I2C1 SCL/SDA already configured; B-rev overlay swaps to `lsm303dlhc_accel` (same lis2dh driver) | `lis2dh` driver at `zephyr/drivers/sensor/lis2dh/`, present, **self-contained** (CMakeLists: own .c files only; no hal_st); sample at `zephyr/samples/sensor/lis2dh/` confirms `CONFIG_I2C=y + CONFIG_SENSOR=y` | **No** — sensor node already in upstream board DTS | `CONFIG_SENSOR=y` (only addition needed; `CONFIG_I2C=y` already present) | **Low** — established upstream path; sample exists; driver self-contained | **High** — proves I2C bus-backed driver-dependent read, the largest open Adapter gap | **FEASIBLE_NOW_NO_PURCHASE** ✓ RECOMMENDED |
| **3. On-board `lsm303agr_magn` (lis2mdl driver)** | No (on-board) | No | `lsm303agr_magn` node in `stm32f411e_disco.dts`, `status = "okay"`, I2C1 | `lis2mdl` driver at `zephyr/drivers/sensor/lis2mdl/`; **BLOCKED** — Kconfig requires `ZEPHYR_HAL_ST_MODULE` + `HAS_STMEMSC`; `hal_st` is not in workspace | No | `CONFIG_SENSOR=y` + `CONFIG_LIS2MDL=y` + `hal_st` module added to west.yml | **Blocked** — requires `west.yml` change + `west update` to fetch `hal_st` | High (if hal_st added) | **NOT_RECOMMENDED_NOW** |
| **4. External I2C sensor module** | Yes (external breakout board) | **Yes** | None | Many options (BME280, SHT31, etc.) locally present in Zephyr sensor drivers | Yes — DTS overlay for external sensor node | `CONFIG_SENSOR=y` + specific driver | Medium — purchase + wiring + overlay | High | **FEASIBLE_AFTER_PURCHASE** (defer until on-board options exhausted) |
| **5. External SPI sensor module** | Yes (external breakout board) | **Yes** | No SPI sensor node in board DTS | Multiple SPI sensor drivers locally present | Yes — SPI bus not configured for sensor use in board DTS | `CONFIG_SENSOR=y` + `CONFIG_SPI=y` already present + specific driver | Medium-High — SPI bus config, pin assignment, DTS overlay | Moderate (SPI bus already proven via `CONFIG_SPI=y`) | **FEASIBLE_AFTER_PURCHASE** (lower priority than I2C path) |
| **6. F407 / custom board** | No | No | N/A | N/A | N/A | N/A | Per standing rule: HOLD/DEFER | N/A | **HOLD** |

---

## D. Existing Board Resource Audit

### D.1 Board files found

Local Zephyr workspace root: `d:/Robot_OS/` (`.west/config` → `[zephyr] base = zephyr`).
Board files inspected:

| File | Path | Notes |
|---|---|---|
| Main board DTS | `zephyr/boards/arm/stm32f411e_disco/stm32f411e_disco.dts` | Defines I2C1 with both on-board sensors; status "okay" |
| Board YAML | `zephyr/boards/arm/stm32f411e_disco/stm32f411e_disco.yaml` | Identifies board; `supported: [counter]` only |
| Defconfig | `zephyr/boards/arm/stm32f411e_disco/stm32f411e_disco_defconfig` | SOC_STM32F411XE, GPIO, SERIAL, CLOCK, PINCTRL |
| B-revision overlay | `zephyr/boards/arm/stm32f411e_disco/stm32f411e_disco_B.overlay` | Swaps lsm303agr → lsm303dlhc; accel still "st,lis2dh" |
| B-revision conf | `zephyr/boards/arm/stm32f411e_disco/stm32f411e_disco_B.conf` | Empty (no Kconfig changes for B-rev) |
| D-revision conf | `zephyr/boards/arm/stm32f411e_disco/stm32f411e_disco_D.conf` | Empty |
| SoC DTSI | `zephyr/dts/arm/st/f4/stm32f411.dtsi` | `die_temp` node (`io-channels = <&adc1 18>`); `spi5` and I2S nodes |
| SoC DTSI parent | `zephyr/dts/arm/st/f4/stm32f411Xe.dtsi` | No ADC/sensor content visible in Phase 11B grep |

### D.2 On-board sensors in board DTS

The main board DTS (`stm32f411e_disco.dts`) defines:

```
&i2c1 {
    pinctrl-0 = <&i2c1_scl_pb6 &i2c1_sda_pb9>;
    pinctrl-names = "default";
    status = "okay";
    clock-frequency = <I2C_BITRATE_FAST>;   // 400 kHz

    lsm303agr_magn: lsm303agr-magn@1e {
        compatible = "st,lis2mdl", "st,lsm303agr-magn";
        status = "okay";
        reg = <0x1e>;
        irq-gpios = <&gpioe 2 GPIO_ACTIVE_HIGH>;
    };

    lsm303agr_accel: lsm303agr-accel@19 {
        compatible = "st,lis2dh", "st,lsm303agr-accel";
        status = "okay";
        reg = <0x19>;
        irq-gpios = <&gpioe 4 GPIO_ACTIVE_HIGH>,
                    <&gpioe 5 GPIO_ACTIVE_HIGH>;
    };
};
```

Aliases: `magn0 = &lsm303agr_magn`, `accel0 = &lsm303agr_accel`.

**B-revision overlay** (`stm32f411e_disco_B.overlay`) deletes both `lsm303agr` nodes and
replaces them with `lsm303dlhc_magn` (compatible "st,lsm303dlhc-magn") and
`lsm303dlhc_accel` (compatible **"st,lis2dh"**, "st,lsm303dlhc-accel"). The accelerometer
on the B-revision board still uses compatible "st,lis2dh" — the `lis2dh` driver handles both
revisions.

### D.3 Internal die temperature

`stm32f411.dtsi` defines:
```
die_temp: dietemp {
    io-channels = <&adc1 18>;
};
```
This node has **no `compatible` property** matching the `stm32_temp` driver
(`DT_HAS_ST_STM32_TEMP_ENABLED` requires `compatible = "st,stm32-temp"` or equivalent).
The `adc1` node is `status = "disabled"` in the parent DTSI. Enabling this path requires:
1. A DTS overlay to enable `adc1` and add the correct compatible string to `die_temp`.
2. `CONFIG_ADC=y` in `prj.conf`.
3. Possibly `CONFIG_STM32_TEMP=y`.

This work is non-trivial compared to the on-board MEMS path.

### D.4 SPI sensor availability

No SPI-connected sensor node is defined in the board DTS or B-revision overlay. The
physical I3G4250D (or L3GD20) gyroscope present on the STM32F411E-DISCO board is
**not characterized** in the local Zephyr board DTS. `CONFIG_SPI=y` is present but no
sensor DT node exists. This path would require a DTS overlay.

### D.5 External sensor (I2C/SPI)

No external sensor DTS node or overlay exists. This path is conditional on purchase and
a future DTS overlay.

### D.6 Board revision note

**The user must verify their board revision before Phase 11C.** The main board DTS targets
revision D (LSM303AGR). Revision B uses `stm32f411e_disco_B.overlay` which swaps to
LSM303DLHC. Both revisions use "st,lis2dh" for the accelerometer; the `lis2dh` driver
handles both. Phase 11C should state the target board revision explicitly.

---

## E. Driver / Zephyr Support Audit

### E.1 `lis2dh` — accelerometer driver

| Item | Finding |
|---|---|
| Local driver path | `zephyr/drivers/sensor/lis2dh/` |
| CMakeLists | `zephyr_library_sources(lis2dh.c lis2dh_i2c.c lis2dh_spi.c)` — self-contained; no external module |
| Kconfig gate | `menuconfig LIS2DH`; `default y`; `depends on DT_HAS_ST_LIS2DH_ENABLED` |
| External module dependency | **None** — no `ZEPHYR_HAL_ST_MODULE` dependency |
| `CONFIG_SENSOR` required? | **Yes** — top-level `drivers/sensor/Kconfig` wraps all sensor drivers under `menuconfig SENSOR` with an `if SENSOR` gate |
| `CONFIG_I2C` required? | `select I2C` when on I2C bus (already `y` in `prj.conf`) |
| `CONFIG_ADC` required? | No |
| DTS overlay required? | **No** — sensor node already in upstream board DTS with `status = "okay"` |
| Sample | `zephyr/samples/sensor/lis2dh/` — `prj.conf`: `CONFIG_I2C=y; CONFIG_SENSOR=y` |
| Sample `depends_on` | `i2c` and `lis2dh` |
| Trigger mode for probe | Use `CONFIG_LIS2DH_TRIGGER_NONE=y` (default choice = trigger none is first in the Kconfig `choice`; simplest for a polled bounded probe) |
| Files likely touched in Phase 11D | `devkit/prj.conf` (add `CONFIG_SENSOR=y`); `devkit/src/devkit_app_state.c` + `devkit_uart_producer.c` (add `case 't':` arm); no new file required if a minimal probe is chosen |
| Available channels | `SENSOR_CHAN_ACCEL_XYZ` (3-axis integer fixed-point via `struct sensor_value`) |
| Board revision compatibility | **Both A/D and B revisions** — B-rev accel is also "st,lis2dh" per `_B.overlay` |

### E.2 `lis2mdl` — magnetometer driver

| Item | Finding |
|---|---|
| Local driver path | `zephyr/drivers/sensor/lis2mdl/` |
| Kconfig gate | `depends on DT_HAS_ST_LIS2MDL_ENABLED`; `depends on ZEPHYR_HAL_ST_MODULE`; `select HAS_STMEMSC`; `select USE_STDC_LIS2MDL` |
| External module dependency | **`hal_st` required** — NOT present in `modules/hal/` (only cmsis, espressif, nordic, stm32) |
| Status | **BLOCKED** — cannot build without adding `hal_st` to `west.yml` and running `west update` |
| Unblocking work | Add `hal_st` project to `west.yml` manifest, run `west update`; this is a scope change not authorized by Phase 11B |

### E.3 `stm32_temp` — internal die temperature driver

| Item | Finding |
|---|---|
| Local driver path | `zephyr/drivers/sensor/stm32_temp/` |
| Kconfig gate | `depends on (ADC && SOC_FAMILY_STM32)`; `depends on DT_HAS_ST_STM32_TEMP_ENABLED (or _CAL)` |
| `CONFIG_ADC` required? | **Yes** — not in `prj.conf` |
| DTS node status | `die_temp` exists in `stm32f411.dtsi` but has no `compatible` string; `adc1` is `status = "disabled"` in parent DTSI |
| Overlay required? | **Yes** — must enable `adc1` and set `compatible = "st,stm32-temp"` (or similar) on `die_temp` |
| Status | **FEASIBLE_AFTER_DRIVER_VERIFICATION** — more infrastructure needed than on-board MEMS path |

### E.4 Top-level sensor Kconfig gate

`zephyr/drivers/sensor/Kconfig` line 6: `menuconfig SENSOR` with `if SENSOR` block
wrapping all individual sensor drivers. **`CONFIG_SENSOR=y` is required for any Zephyr
sensor driver including `lis2dh`.**

### E.5 Zephyr samples

| Sample | Path | Notes |
|---|---|---|
| LIS2DH Accelerometer Monitor | `zephyr/samples/sensor/lis2dh/` | `prj.conf` needs `CONFIG_I2C=y` + `CONFIG_SENSOR=y`; `depends_on: i2c, lis2dh` |

No dedicated `stm32f411e_disco` board sample for sensors found in local workspace.
The `lis2dh` sample uses `DEVICE_DT_GET(DT_ALIAS(accel0))` which resolves to
`lsm303agr_accel` via the board DTS alias.

---

## F. Purchase Decision

**`NO_PURCHASE_NEEDED_FOR_NEXT_STEP`**

The on-board LSM303AGR accelerometer via the `lis2dh` driver is fully accessible
without any hardware purchase. External sensor modules (I2C or SPI breakout boards)
are deprioritized until the on-board MEMS path is either completed or ruled out.

No hardware purchase is authorized or recommended by Phase 11B.

---

## G. Recommended Future Path

**Phase 11C — On-board MEMS accelerometer probe spec**

Target: `lsm303agr_accel` node (board DTS alias `accel0`), driver `lis2dh`,
bus I2C1 (SCL=PB6, SDA=PB9, 400 kHz), on-board, no purchase.

This is the lowest-risk, highest-evidence-value path available without any
hardware change or purchase. It characterizes the driver-dependent read Adapter
surface (the largest open gap identified in Phase 11A) using an already-defined
board resource.

The die-temperature path (`stm32_temp` / `CONFIG_ADC`) remains viable for a
future sub-phase if the MEMS path proves problematic, but the MEMS path is
recommended first.

---

## H. Phase 11C Spec Requirements

Phase 11C (Sensor Probe Spec, docs-only) must freeze the following before any
Phase 11D code is written:

1. **Exact target.** `lsm303agr_accel` (revision A/D) or `lsm303dlhc_accel`
   (revision B) — user must confirm board revision. Driver is `lis2dh` for both.
   DT alias `accel0`. I2C address 0x19.

2. **Trigger mode.** `CONFIG_LIS2DH_TRIGGER_NONE=y` (polled probe; no interrupt
   thread; simplest for a bounded single-read command handler). No
   `LIS2DH_TRIGGER_GLOBAL_THREAD` or `LIS2DH_TRIGGER_OWN_THREAD`.

3. **Read channel.** `SENSOR_CHAN_ACCEL_XYZ` — raw X/Y/Z signed integer fixed-point
   via `struct sensor_value { int32_t val1; int32_t val2; }`. **No
   `sensor_value_to_double()` — no floating-point.**

4. **Response format.** Must be frozen at Phase 11C. Candidate (to be confirmed):
   `ACCEL x=<V1>.<V2> y=<V1>.<V2> z=<V1>.<V2>\r\n` where `<V1>` is `val1` and
   `<V2>` is micro-fraction truncated or scaled for readability. Exact format and
   field widths are Phase 11C decisions — this section lists requirements only.
   The response must fit within the **96-byte stack buffer** constraint
   (scope-guard #2 from `PHASE_9EZ_CHECKPOINT.md §H`). A 3-axis response with
   sign and decimal format is well within 96 bytes.

5. **Error response.** Exact error string if `sensor_sample_fetch()` returns
   non-zero. Candidate: `ERR accel read=<errno>\r\n`. Must also fit 96 bytes.

6. **Command sequence for host harness.** Minimal validation sequence: `t` →
   expect response; repeat N times; check counters via `?`. No blocking calls,
   no polling loops.

7. **Expected RTT counters.** Phase 11E evidence must show: `rx=ok=handled`
   increments per `t` command; `state` unchanged; `transitions` unchanged;
   `ignored` unchanged; `dropped=0`; CFSR/HFSR = 0x00000000.

8. **`prj.conf` additions.** `CONFIG_SENSOR=y`. Optionally `CONFIG_LIS2DH_TRIGGER_NONE=y`
   (if not already the default for the build). No `CONFIG_ADC`. No `CONFIG_CBPRINTF_FP_SUPPORT`.

9. **Overlay.** None required. The sensor node is already in the upstream board DTS.

10. **Wiring.** None required. Sensor is on-board, I2C bus already configured.

11. **Fallback if sensor unavailable.** If `sensor_sample_fetch()` returns error
    (e.g. sensor not responding or board revision mismatch): emit error response,
    return from handler. No hang, no fault.

12. **No parser / shell / framing / response queue.** All existing UART TX
    scope-guard constraints from `PHASE_9EZ_CHECKPOINT.md §H` remain intact.

13. **No calibration, units, sensor identity, or product semantics.** Raw `val1`/`val2`
    from `sensor_channel_get()` only. Calibration and unit conversion are Framework-class
    and not approved.

14. **Fixed-buffer compliance.** Phase 11C must verify that the proposed response
    format plus all error variants fit within the 96-byte stack-buffer constraint
    before Phase 11D writes any code.

---

## I. Non-goals

Phase 11B explicitly does **not**:

- Implement `T` or any `case 't':` source change.
- Modify `devkit/prj.conf`.
- Enable `CONFIG_SENSOR`, `CONFIG_ADC`, `CONFIG_LIS2DH`, or any new Kconfig.
- Create a DTS overlay.
- Add or enable any sensor DT node.
- Approve any hardware purchase.
- Modify `core/`, `platform/`, `devkit/src/`, `tests/`, `CMakeLists.txt`,
  `boards/`, or any evidence log.
- Modify `devkit_runtime.c`, `devkit_app_state.c`, or `devkit_uart_producer.c`.
- Add `hal_st` to `west.yml` or run `west update`.
- Start ACTIVE disarm widening.
- Reopen Scheduler 7A/7B.
- Reopen F407 / custom-board work.
- Implement any Robot Framework API.
- Add any Application / product logic.
- Add parser, shell, command registry, framing, or response queue.
- Change command semantics for `a / s / r / ? / x / v / L / d`.
- Expand UART TX scope.
- Push.

---

## J. Decision Result

**`FEASIBILITY_CONFIRMED_ONBOARD_MEMS`**

The on-board LSM303AGR accelerometer (`lsm303agr_accel` node, `lis2dh` driver, I2C1)
is confirmed as the recommended target for the Phase 11C/11D bounded Adapter sensor-read
probe.

- No purchase needed.
- No DTS overlay needed.
- No hardware change needed.
- Driver is locally present and self-contained.
- Only `prj.conf` addition: `CONFIG_SENSOR=y` (Phase 11D only, not now).
- Board revision note: user must confirm A/D vs B revision before Phase 11C spec freezes
  the exact DTS alias target.

The STM32 internal die-temperature path remains open as a fallback
(`FEASIBLE_AFTER_DRIVER_VERIFICATION`) if the MEMS path encounters a blocking issue
during Phase 11D.

---

## K. Scope Guards

All 12 UART TX scope-guard constraints from
[`../02_PHASE_CLOSEOUTS/PHASE_9EZ_CHECKPOINT.md`](../02_PHASE_CLOSEOUTS/PHASE_9EZ_CHECKPOINT.md)
§H are preserved. `core/`, `platform/`, `devkit/src/`, `prj.conf`, `CMakeLists.txt`,
`boards/`, `zephyr/`, `tests/`, runtime scripts, host tools, and evidence logs are
all zero-diff at this phase. Scheduler 7A/7B remains DEFER. F407 remains HOLD/DEFER.
UART TX remains minimal response only. POST_FLASH_AUTOSTART discipline unchanged (root
cause OPEN; `MITIGATED_BY_WORKFLOW` from Phase 6O onward). No command semantics changed.

---

## L. Next Gate

**Phase 11C — Sensor Probe Spec (docs-only).**
Reserved; not opened by this document.

Phase 11C must freeze all items listed in §H above before any Phase 11D code is written.
Phase 11D (Implementation) and Phase 11E (Evidence Closeout) are conditional on Phase 11C.

No hardware purchase is authorized or needed before Phase 11C closes.
