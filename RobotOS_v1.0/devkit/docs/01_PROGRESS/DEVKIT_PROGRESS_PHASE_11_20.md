# DEVKIT_PROGRESS_PHASE_11_20.md -- RobotOS Devkit Phase Log, Phase 11-20

**Status:** `RESERVED / NOT_STARTED`

This file is the planned progress stream for Phase 11 through Phase 20.
**Phase 11 is not opened by this scaffold.** No implementation,
checkpoint, or evidence is recorded here yet. Adding content to this
file does not authorize any source, test, CMake, Zephyr, board,
`prj.conf`, host-tool, or script change.

---

## 1. Continuation Note

This file continues the RobotOS devkit phase log after
`DEVKIT_PROGRESS_PHASE_10.md`. Together with the historical master
`DEVKIT_PROGRESS.md`, the three progress streams form the canonical
phase history of the devkit:

| Phase range | File | Status at this scaffold's creation |
|---|---|---|
| Phase 1 through Phase 9E / Phase 9E-Z | [`DEVKIT_PROGRESS.md`](DEVKIT_PROGRESS.md) | Historical master; closed; not rewritten when later phases land. |
| Phase 10 (Phase 10A, 10B-v, 10B-L, 10B-d, 10C, ...) | [`DEVKIT_PROGRESS_PHASE_10.md`](DEVKIT_PROGRESS_PHASE_10.md) | Running log for Phase 10 stream; Phase 10C is the latest closed entry. |
| Phase 11 through Phase 20 | this file | `RESERVED / NOT_STARTED`. |

**Phase-specific closeout / checkpoint docs remain a separate artifact
class** under
[`../02_PHASE_CLOSEOUTS/`](../02_PHASE_CLOSEOUTS/). A phase with
hardware evidence, implementation closeout, checkpoint, direction
guard, architecture/design decision, important validation record, or
release/freeze implication may still publish its own
`PHASE_*_CLOSE.md` / `PHASE_*_CHECKPOINT.md` document under
`02_PHASE_CLOSEOUTS/`. The progress stream file (this file) is **not**
the place for full-form evidence; it is the running narrative index
that points at those closeout docs.

---

## 2. Phase-window convention (carried forward)

Per the docs-only convention recorded at the Phase 11-20 scaffold
opening:

- Progress files are **grouped by 10-phase windows**, not one file per
  phase, starting at Phase 11.
- Phase 11 through Phase 20 share **this** file
  (`DEVKIT_PROGRESS_PHASE_11_20.md`).
- Future windows follow the same rule:
  `DEVKIT_PROGRESS_PHASE_21_30.md`,
  `DEVKIT_PROGRESS_PHASE_31_40.md`, and so on -- created when the
  window opens, not before.
- Phase 1 through Phase 10 keep their historical naming
  (`DEVKIT_PROGRESS.md` and `DEVKIT_PROGRESS_PHASE_10.md`) without
  retroactive rename or reorganization.

This convention applies to **progress stream files only**. It does
**not** prevent or replace separate per-phase closeout / checkpoint
documents under `02_PHASE_CLOSEOUTS/` when a phase warrants one.

---

## 3. Split / Editing Rules (carried forward from Phase 10 stream)

These rules govern how this file and its sibling progress streams
evolve. They mirror `DEVKIT_PROGRESS_PHASE_10.md` §4 so the
discipline does not regress at the window boundary.

1. **History stays put.** `DEVKIT_PROGRESS.md` (Phase 1..9E/9E-Z) and
   `DEVKIT_PROGRESS_PHASE_10.md` (Phase 10) remain frozen except for
   strict link / typo fixes. Do not move, rewrite, reorder, or
   deduplicate old phase sections as part of using this file.
2. **New Phase 11-20 entries land here.** Every Phase 11 through
   Phase 20 entry (including any Phase 11A, 11B-x, 12, ..., 20Z,
   etc.) must be added to this file, not back-propagated to
   `DEVKIT_PROGRESS.md` or `DEVKIT_PROGRESS_PHASE_10.md`.
3. **Manual anchors are mandatory.** Each future phase section must
   place an explicit manual anchor (`<a id="phase-NN[-X]"></a>`) on
   the line immediately before its heading.
4. **Index links must target manual anchors.** Entries in the Phase
   Index table must link to the manual anchor id, not to a heading
   slug.
5. **Do not rely on GitHub auto-generated heading anchors.** Heading
   slugs change when titles are edited; manual anchors do not.
6. **Make non-linear edits explicit.** If a phase is duplicated,
   amended, or inserted out of phase-number order, mark the index
   entry to make the duplication or insertion obvious (e.g. `‡` or
   anchor suffixes like `-1` / `-2`), mirroring the existing
   convention in `DEVKIT_PROGRESS.md`.
7. **No silent baseline change.** Do not alter Phase 10C's recorded
   baseline (or any earlier closed phase) to claim work has
   progressed beyond what has been validated and recorded as a closed
   Phase 11+ entry below.
8. **Closeouts go to `../02_PHASE_CLOSEOUTS/`, not here.** This file
   carries the narrative index and brief per-phase summaries that
   point at closeouts. Full evidence, scope-guard restatements,
   counter tables, and verdict sections belong in the dedicated
   closeout doc.

---

## 4. Phase Index

Entries are added when a phase is formally opened. Until then, the
table contains only the reserved placeholders.

| Phase | Title | Status | Jump |
|-------|-------|--------|------|
| 11A | Adapter Boundary & Sensor Surface Decision (docs-only) | CLOSED_DOCS_ONLY | [->](#phase-11a) |
| 11B | Device / Driver Feasibility Gate (docs-only / audit) | CLOSED_DOCS_ONLY | [->](#phase-11b) |
| 11C | On-board MEMS Accelerometer Probe Spec (docs-only) | CLOSED_DOCS_ONLY | [->](#phase-11c) |
| 11D | On-board MEMS Accelerometer Probe Implementation (firmware) | IMPLEMENTATION_CLOSED_HARDWARE_EVIDENCE_PENDING | [->](#phase-11d) |
| 11E | On-board MEMS Accelerometer Probe Evidence Closeout | CLOSED_WITH_HARDWARE_EVIDENCE | [->](#phase-11e) |
| 11Z | Command-Set Checkpoint (docs-only) | CLOSED_DOCS_ONLY | [->](#phase-11z) |
| 12A | Robot Framework API Surface Planning (docs-only) | CLOSED_DOCS_ONLY | [->](#phase-12a) |
| 12B | Robot Framework FSM API Draft (docs-only) | CLOSED_DOCS_ONLY | [->](#phase-12b) |
| 12C | Framework FSM Event Bridge + Status Model Confirmation (docs-only) | CLOSED_DOCS_ONLY | [->](#phase-12c) |
| 12D-pre | Legacy Framework Scaffold Disposition (docs-only) | CLOSED_DOCS_ONLY | [->](#phase-12d-pre) |
| 12D | Framework FSM Header Stub (header-only + docs) | CLOSED_HEADER_STUB_ONLY | [->](#phase-12d) |
| 12E-pre | Framework FSM Consumer / Test Plan (docs-only) | CLOSED_DOCS_ONLY | [->](#phase-12e-pre) |
| 12E | Framework FSM Host-Test Implementation | CLOSED_WITH_HOST_TEST_EVIDENCE | [->](#phase-12e) |
| 12F-pre | Application Bridge Planning (docs-only) | CLOSED_DOCS_ONLY | [->](#phase-12f-pre) |
| 20Z | RESERVED -- future checkpoint / closeout slot | NOT_STARTED | [->](#phase-20z) |

When future phases are added:

- Insert a new manual-anchor block (`<a id="phase-NN"></a>`)
  immediately before the new phase heading.
- Add a row to the table above linking to that manual anchor.
- Keep the index in insertion / session order, matching the
  convention used in `DEVKIT_PROGRESS_PHASE_10.md` and
  `DEVKIT_PROGRESS.md`. Phase-number order is **not** required.

---

## 5. Phase Sections

<a id="phase-11a"></a>
## Phase 11A -- Adapter Boundary & Sensor Surface Decision

**Status:** `CLOSED_DOCS_ONLY`
**Type:** Docs-only architecture gate. No source, runtime, test,
CMake, Zephyr, board, `prj.conf`, DTS overlay, host-tool, or script
change.
**Date opened/closed:** 2026-05-12
**Published baseline at open:** `origin/master = 7ce8cb7`
**Closeout doc:**
[`../02_PHASE_CLOSEOUTS/PHASE_11A_ADAPTER_BOUNDARY_SENSOR_SURFACE.md`](../02_PHASE_CLOSEOUTS/PHASE_11A_ADAPTER_BOUNDARY_SENSOR_SURFACE.md).
**Companion docs:**
[`../03_SPECS/COMMAND_SET_DRAFT.md`](../03_SPECS/COMMAND_SET_DRAFT.md),
[`../02_PHASE_CLOSEOUTS/PHASE_10C_COMMAND_SET_CHECKPOINT.md`](../02_PHASE_CLOSEOUTS/PHASE_10C_COMMAND_SET_CHECKPOINT.md),
[`../02_PHASE_CLOSEOUTS/PHASE_9EZ_CHECKPOINT.md`](../02_PHASE_CLOSEOUTS/PHASE_9EZ_CHECKPOINT.md).

### 11A.1 Purpose

Phase 11A is the first Phase 11–20 architecture gate. It produces a
docs-only decision with three deliverables: (1) an Adapter API
surface inventory at concept level, (2) a sensor-surface
classification for `T`, and (3) a placement of the next gates in
the Phase 11–20 sequence. It does **not** implement `T`, does **not**
start ACTIVE disarm widening, and does **not** authorize any
hardware purchase.

### 11A.2 Adapter API surface inventory (concept-level)

Eleven Adapter primitive classes are proven on hardware (time,
thread-context boundary, critical section, ISR-safe event posting,
queue/dispatch/budget, GPIO input as event source, GPIO output,
UART RX, UART TX, RTT trace/telemetry/fault, timer-generated
events). The single largest uncharacterized primitive class is
**driver-dependent read** (and its sub-classes: I²C, SPI, ADC).
Pool/slab allocator is flagged as an open architectural question
(event queue is fixed-capacity ring; whether "pool" in the diagram
is already realized through that ring or is a separate primitive is
deferred to a later docs phase). Portability backend is claimed but
undemonstrated (only Zephyr/STM32F4 exercised). See the closeout doc
§D for the full per-primitive table.

### 11A.3 Sensor surface classification decision

**Decision: `SENSOR_SURFACE_DECIDED_ADAPTER_PROBE`.**

This classifies `T` as a future bounded Adapter probe candidate
**only**. It is **not** implementation approval, **not** a hardware
purchase approval, and **not** a Framework / Application promotion.
The probe (if later authorized) must return Adapter-level raw values
or direct driver output; unit / calibration / sensor-identity in any
rich-typed sense is explicitly **Framework-class** and not approved
by Phase 11A.

Rationale (full table in closeout doc §F): the driver-dependent read
surface is the largest open Adapter gap; sensor-as-Framework is
premature without other Framework primitives chosen;
sensor-as-Application is premature without product chosen;
sensor-as-hold defers the largest open gap indefinitely.

### 11A.4 `T` status

`T` remains **not implemented**. Its `COMMAND_SET_DRAFT.md` Section B
row is updated with the Phase 11A cross-reference. The status tag is
preserved as `USER_DECISION_REQUIRED` (refined with a Phase 11A
cross-reference to make clear that the classification has been made
but the implementation decision has not).

### 11A.5 Phase 11B is the next gate (no purchase before 11B)

Phase 11B (Device / Driver Feasibility Gate) is the next gate. **No
hardware purchase is authorized by Phase 11A.** Phase 11B will
verify existing STM32F411E-DISCO resources first, in priority order:
(a) STM32 internal die temperature via ADC; (b) on-board MEMS
peripherals if Zephyr DT / driver support is confirmed for the
user's specific board revision; (c) external I²C sensor module only
if (a) and (b) are not viable. F407 / portability remains
`HOLD/DEFER`; Phase 15A (or equivalent future gate) is the earliest
revisit.

### 11A.6 ACTIVE disarm widening remains separate

ACTIVE disarm widening (`USER_DECISION_REQUIRED_ACTIVE_DISARM`) is
**not part of Phase 11A**. Current `d` from ACTIVE = recognized
no-op is preserved. The widening is a separate small vocabulary
housekeeping gate that can be opened on explicit user request only;
it is **not** scheduled ahead of Phase 11A or Phase 11B.

### 11A.7 Scope guards intact

All 12 UART TX scope-guard constraints from
[`../02_PHASE_CLOSEOUTS/PHASE_9EZ_CHECKPOINT.md`](../02_PHASE_CLOSEOUTS/PHASE_9EZ_CHECKPOINT.md)
§H are preserved. `core/`, `platform/`, `devkit/src/`,
`prj.conf`, `CMakeLists.txt`, `boards/`, `zephyr/`, `tests/`,
runtime scripts, host tools, and evidence logs are all zero-diff at
this phase. Scheduler 7A/7B remains DEFER. F407 remains HOLD/DEFER.
UART TX remains minimal response only. POST_FLASH_AUTOSTART
discipline unchanged.

### 11A.8 Next gate

Phase 11B -- Device / Driver Feasibility Gate (docs-only / audit).
Reserved; not opened by this doc. Phase 11C (Sensor Probe Spec) and
Phase 11D-E (probe implementation + evidence closeout) are
conditional on 11B and reserved.

---

<a id="phase-11b"></a>
## Phase 11B -- Device / Driver Feasibility Gate

**Status:** `CLOSED_DOCS_ONLY`
**Type:** Docs-only feasibility audit / purchase gate. No source, runtime,
test, CMake, Zephyr, board, `prj.conf`, DTS overlay, host-tool, or script
change.
**Date opened/closed:** 2026-05-12
**Published baseline at open:** `origin/master = bae2436`
**Closeout doc:**
[`../02_PHASE_CLOSEOUTS/PHASE_11B_DEVICE_DRIVER_FEASIBILITY.md`](../02_PHASE_CLOSEOUTS/PHASE_11B_DEVICE_DRIVER_FEASIBILITY.md).

### 11B.1 Purpose

Phase 11B is the device/driver feasibility and purchase gate. It audits
the local Zephyr workspace and board files to determine which candidate
sensor path is safest and most feasible for the future Phase 11C/11D
bounded Adapter probe. It does not implement `T`, does not change any
`prj.conf` or overlay, and does not authorize any hardware purchase.

### 11B.2 Candidates audited

Six candidates were evaluated: (1) STM32 internal die temperature / ADC;
(2) on-board `lsm303agr_accel` (lis2dh driver, I2C1); (3) on-board
`lsm303agr_magn` (lis2mdl driver); (4) external I2C sensor module;
(5) external SPI sensor module; (6) F407 / custom board.

### 11B.3 Feasibility decision

**`FEASIBILITY_CONFIRMED_ONBOARD_MEMS`**

The on-board LSM303AGR accelerometer (`lsm303agr_accel` node, Zephyr driver
`lis2dh`, I2C1 at 0x19) is the recommended target. Key findings:

- The sensor node is already defined with `status = "okay"` in the upstream
  Zephyr board DTS (`zephyr/boards/arm/stm32f411e_disco/stm32f411e_disco.dts`).
- The `lis2dh` driver is locally present and self-contained (no `hal_st`
  dependency; handles both board revision A/D and B via compatible "st,lis2dh").
- No DTS overlay is needed.
- No hardware purchase is needed.
- The `prj.conf` additions required in Phase 11D are `CONFIG_I2C=y` and
  `CONFIG_SENSOR=y` (neither currently enabled in `devkit/prj.conf`; an
  earlier draft of this entry incorrectly claimed `CONFIG_I2C=y` was
  pre-existing — that has been corrected docs-only at HEAD).
- Zephyr sample `zephyr/samples/sensor/lis2dh/` confirms this path.

Phase 11A listed die temperature as priority (a) and on-board MEMS as (b).
Phase 11B **corrects this ordering**: the on-board MEMS accelerometer path
is simpler and fully characterized locally; die temperature requires
`CONFIG_ADC=y`, enabling `adc1` in DTS, and adding a compatible string to
the `die_temp` node.

The `lis2mdl` magnetometer path is blocked by the missing `hal_st` module.

### 11B.4 Purchase recommendation

**`NO_PURCHASE_NEEDED_FOR_NEXT_STEP`**

No hardware purchase is authorized or recommended. External sensor modules
are deprioritized while on-board resources are sufficient.

### 11B.5 Next gate

Phase 11C -- Sensor Probe Spec (docs-only). Must freeze: exact DTS alias
target (user to confirm board revision A/D vs B), trigger mode
(`LIS2DH_TRIGGER_NONE`), read channel (`SENSOR_CHAN_ACCEL_XYZ`), response
format (raw `val1`/`val2`, no floating point, ≤96 bytes), error response,
host harness sequence, expected RTT counters, and `prj.conf` additions
(`CONFIG_SENSOR=y`). No Phase 11D code before Phase 11C is closed.
See closeout doc §H for the full Phase 11C spec requirements list.

---

<a id="phase-11c"></a>
## Phase 11C -- On-board MEMS Accelerometer Probe Spec

**Status:** `CLOSED_DOCS_ONLY`
**Type:** Docs-only spec freeze. No source, runtime, test, CMake, Zephyr,
board, `prj.conf`, DTS overlay, host-tool, or script change.
**Date opened/closed:** 2026-05-12
**Published baseline at open:** `origin/master = 2aa0435`
**Closeout doc:**
[`../02_PHASE_CLOSEOUTS/PHASE_11C_ACCEL_PROBE_SPEC.md`](../02_PHASE_CLOSEOUTS/PHASE_11C_ACCEL_PROBE_SPEC.md).
**Companion docs:**
[`../03_SPECS/COMMAND_SET_DRAFT.md`](../03_SPECS/COMMAND_SET_DRAFT.md),
[`../02_PHASE_CLOSEOUTS/PHASE_11B_DEVICE_DRIVER_FEASIBILITY.md`](../02_PHASE_CLOSEOUTS/PHASE_11B_DEVICE_DRIVER_FEASIBILITY.md),
[`../02_PHASE_CLOSEOUTS/PHASE_11A_ADAPTER_BOUNDARY_SENSOR_SURFACE.md`](../02_PHASE_CLOSEOUTS/PHASE_11A_ADAPTER_BOUNDARY_SENSOR_SURFACE.md).

### 11C.1 Purpose

Phase 11C freezes the on-board MEMS accelerometer probe specification
before any Phase 11D code exists. It operationalizes Phase 11B's
`FEASIBILITY_CONFIRMED_ONBOARD_MEMS` finding plus the operator's
board-revision confirmation (revision D, `CONFIRMED_A_OR_D`) into a
fixed Phase 11D contract.

### 11C.2 Target device freeze

- **Board target:** `stm32f411e_disco` (no `@<rev>` suffix; revision
  D is Zephyr default per `revision.cmake`).
- **Board revision:** D.
- **DT alias:** `accel0` → `lsm303agr_accel`.
- **Driver:** `lis2dh` (self-contained, no `hal_st`).
- **Bus / address:** I2C1 / 0x19 (SCL = PB6, SDA = PB9, 400 kHz).
- **Channel:** `SENSOR_CHAN_ACCEL_XYZ` (raw `struct sensor_value`).
- **Trigger mode:** polling / `CONFIG_LIS2DH_TRIGGER_NONE` (driver
  default). Interrupt GPIOs PE4/PE5 reserved by DT but not used.
- **Overlay:** none expected.
- **Wiring:** none.
- **Hardware purchase:** none.

### 11C.3 Frozen UART responses (fit existing 96-byte stack buffer)

- **Success:** `ACC x=<v1>.<v2_6d> y=<v1>.<v2_6d> z=<v1>.<v2_6d>\r\n`
  (worst case 68 bytes; typical ~44 bytes).
- **Error:** `ERR accel read=<errno>\r\n` (worst case 28 bytes;
  errno is the **numeric** signed-decimal return value from the
  failing Zephyr API; no symbolic mapping).
- See [`PHASE_11C_ACCEL_PROBE_SPEC.md`](../02_PHASE_CLOSEOUTS/PHASE_11C_ACCEL_PROBE_SPEC.md)
  §E / §F for the full freeze rules and byte-budget proof.

### 11C.4 Frozen host harness sequence

Canonical sequence: `T T ?` (three host commands; three responses;
verifies command path, sensor read repeatability, and app-state
invariants via the trailing `?`). See
[`PHASE_11C_ACCEL_PROBE_SPEC.md`](../02_PHASE_CLOSEOUTS/PHASE_11C_ACCEL_PROBE_SPEC.md)
§H.

### 11C.5 Future Phase 11D config implication

- **Must add:** `CONFIG_I2C=y` and `CONFIG_SENSOR=y`.
- **Optional (verify against pristine `.config`):**
  `CONFIG_LIS2DH_TRIGGER_NONE=y` only if not already the default.
- **Must NOT add:** `CONFIG_SPI`, `CONFIG_ADC`,
  `CONFIG_CBPRINTF_FP_SUPPORT`, `CONFIG_LIS2DH_*` range/ODR/HP
  overrides.

### 11C.6 Scope guards intact

All 12 UART TX scope-guard constraints from
[`../02_PHASE_CLOSEOUTS/PHASE_9EZ_CHECKPOINT.md`](../02_PHASE_CLOSEOUTS/PHASE_9EZ_CHECKPOINT.md)
§H preserved. `core/`, `platform/`, `devkit/src/`, `prj.conf`,
`CMakeLists.txt`, `boards/`, `zephyr/`, `tests/`, runtime scripts,
host tools, evidence logs all zero-diff at this phase. Scheduler 7A/7B
remains DEFER. F407 remains HOLD/DEFER. UART TX remains minimal
response only. POST_FLASH_AUTOSTART discipline unchanged. ACTIVE
disarm widening remains `USER_DECISION_REQUIRED_ACTIVE_DISARM` and
decoupled from the Phase 11A-E sensor track.

### 11C.7 `T` status

`T` remains **not implemented**. Its `COMMAND_SET_DRAFT.md` Section B
row is updated with the Phase 11C cross-reference and the frozen
response/error shapes. Status tag preserved as
`USER_DECISION_REQUIRED` pending **explicit Phase 11D authorization
by the user**.

### 11C.8 Next gate

Phase 11D -- Sensor Probe Implementation (firmware). Reserved; not
opened by this doc. Phase 11D opening requires explicit user
authorization for code / `prj.conf` changes. Phase 11E (evidence
closeout) follows Phase 11D and is also conditional.

---

<a id="phase-11d"></a>
## Phase 11D -- On-board MEMS Accelerometer Probe Implementation

**Status:** `IMPLEMENTATION_CLOSED_HARDWARE_EVIDENCE_PENDING`
**Type:** Devkit-local firmware + tooling implementation of the
Phase 11C-frozen on-board MEMS accelerometer probe (`T` command).
Source + `prj.conf` + host harness changes only. No `core/`, no
`platform/`, no `tests/`, no scheduler/queue constants, no board
DTS, no DTS overlay, no Zephyr module, no hardware purchase.
Build-validated pristine for `stm32f411e_disco`. Hardware evidence
is deferred to Phase 11E.
**Date opened/closed:** 2026-05-12
**Published baseline at open:** `origin/master = cc101cd`
**Closeout doc:**
[`../02_PHASE_CLOSEOUTS/PHASE_11D_ACCEL_PROBE_IMPLEMENTATION.md`](../02_PHASE_CLOSEOUTS/PHASE_11D_ACCEL_PROBE_IMPLEMENTATION.md).
**Companion docs:**
[`../02_PHASE_CLOSEOUTS/PHASE_11C_ACCEL_PROBE_SPEC.md`](../02_PHASE_CLOSEOUTS/PHASE_11C_ACCEL_PROBE_SPEC.md),
[`../03_SPECS/COMMAND_SET_DRAFT.md`](../03_SPECS/COMMAND_SET_DRAFT.md).

### 11D.1 Purpose

Phase 11D implements the Phase 11C-frozen on-board MEMS accelerometer
probe as a bounded devkit-local Adapter command `T` (case-insensitive
`T`/`t`). It follows the Phase 11C contract verbatim and does not
widen the spec.

### 11D.2 Source / config / tooling delta

- `devkit/prj.conf`: append exactly `CONFIG_I2C=y` and `CONFIG_SENSOR=y`
  (plus a leading comment block). **No other Kconfig**.
- `devkit/src/devkit_app_state.h`: doc-only update to include `T` in
  the recognized-command list. No new exported symbol.
- `devkit/src/devkit_app_state.c`: add `case 't':` branch matching the
  `case 'v':` shape -- LOG_INF only, no transition, not ignored.
- `devkit/src/devkit_uart_producer.c`: include `<errno.h>`,
  `<stdlib.h>`, `<zephyr/drivers/sensor.h>`; resolve
  `DEVICE_DT_GET(DT_ALIAS(accel0))` at link time with a compile-time
  `#error` guard for missing alias; add internal
  `devkit_accel_format_value()` helper; add `case 't':` TX branch
  (`sensor_sample_fetch` + `sensor_channel_get` + frozen response
  composition into the existing 96-byte stack buffer).
- `tools/runtime/run_phase11d_accel_probe_demo.ps1`: new
  PowerShell 5.1 host harness, pure ASCII, default sequence `T T ?`,
  Phase 6O sidecar-`reset run` discipline; PowerShell-parse OK.

### 11D.3 Build result

```text
[161/161] Linking C executable zephyr\zephyr.elf
Memory region         Used Size  Region Size  %age Used
           FLASH:       41528 B       512 KB      7.92%
             RAM:       12352 B       128 KB      9.42%
```

Delta vs Phase 10B-d baseline `125779c` (36780 B / 12224 B):
**+4748 B FLASH / +128 B RAM** -- dominated by Zephyr sensor
subsystem + I2C STM32 driver + `lis2dh` driver. No new warnings.

### 11D.4 Generated `.config` observations

`CONFIG_I2C=y`, `CONFIG_SENSOR=y`, `CONFIG_LIS2DH=y`,
`CONFIG_LIS2DH_TRIGGER_NONE=y`, `CONFIG_LIS2DH_OPER_MODE_NORMAL=y`,
`CONFIG_LIS2DH_ACCEL_RANGE_RUNTIME=y`, `CONFIG_LIS2DH_ODR_RUNTIME=y`
all set (driver defaults). **Forbidden Kconfigs absent**:
`CONFIG_SPI` not set, `CONFIG_ADC` not set,
`CONFIG_CBPRINTF_FP_SUPPORT` not set. `CONFIG_LIS2DH_TRIGGER_NONE=y`
is the driver default; **not** explicitly added to `prj.conf` to
avoid redundant Kconfig noise.

### 11D.5 Sign rule for `struct sensor_value`

The frozen `<v1>.<v2_6d>` shape places the sign on the integer part.
For the Zephyr edge case `val1=0, val2<0` (sub-unit negative), the
implementation emits a leading `-` and prints both fields as
absolute values, e.g. `-0.500000`. This stays inside the frozen
shape and is recorded as **`PHASE_11C_FORMAT_SIGN_EDGE`** in the
closeout doc §D.5.

### 11D.6 `T` behavioral contract realized

- Recognition in `devkit_app_state.c` (`case 't':`).
- App-state, `transitions`, `ignored`, `button` counters unchanged.
- `uart` counter +1 per `T` (existing pre-switch increment).
- Read in thread/handler context (`uart_irq` -> queue -> dispatcher
  thread).
- Single attempt; no retry.
- Frozen success line `ACC x=<v1>.<v2_6d> y=<v1>.<v2_6d>
  z=<v1>.<v2_6d>\r\n`.
- Frozen error line `ERR accel read=<errno>\r\n` with numeric errno
  (no symbolic mapping).
- Other commands' branches (`v`/`L`/`d`/`a`/`s`/`r`/`?`/`x`) are
  byte-for-byte unchanged.

### 11D.7 Hardware evidence status

**`HARDWARE_EVIDENCE_PENDING_PHASE_11E`.** The board was not flashed,
RTT was not captured, and the harness was not run by Phase 11D. The
`T T ?` harness exists and is ready for Phase 11E.

### 11D.8 Scope guards intact

All 12 UART TX scope-guard constraints from
[`../02_PHASE_CLOSEOUTS/PHASE_9EZ_CHECKPOINT.md`](../02_PHASE_CLOSEOUTS/PHASE_9EZ_CHECKPOINT.md)
§H preserved. `core/`, `platform/`, `tests/`,
`devkit/CMakeLists.txt` top-level, `devkit_runtime.{c,h}`,
`devkit_status_led.{c,h}`, `devkit_button_producer.c`,
`devkit_timer_producer.c`, `devkit_observability.c`,
`devkit_build_info.c`, `devkit_fault.c`, board DTS, board defconfig,
B-revision overlay, Zephyr workspace tracked files, and all evidence
logs zero-diff. Scheduler 7A/7B remains DEFER. F407 remains
HOLD/DEFER. UART TX remains minimal response only.
POST_FLASH_AUTOSTART discipline unchanged. ACTIVE disarm widening
`USER_DECISION_REQUIRED_ACTIVE_DISARM` preserved and decoupled.

### 11D.9 Next gate

Phase 11E -- Accelerometer Probe Evidence Closeout. Hardware
evidence-only. Opened 2026-05-12. See `<a id="phase-11e"></a>` section
below.

---

<a id="phase-11e"></a>
## Phase 11E -- On-board MEMS Accelerometer Probe Evidence Closeout

**Status:** `CLOSED_WITH_HARDWARE_EVIDENCE`
**Type:** Hardware evidence-only closeout. No source, runtime, test,
CMake, Zephyr, board, `prj.conf`, DTS overlay, or script change.
Proves the Phase 11D `T`/`t` accelerometer probe on the
STM32F411E-DISCO revision D board.
**Date opened/closed:** 2026-05-12
**Implementation commit tested:** `2040bfb`
**Closeout doc:**
[`../02_PHASE_CLOSEOUTS/PHASE_11E_ACCEL_PROBE_EVIDENCE.md`](../02_PHASE_CLOSEOUTS/PHASE_11E_ACCEL_PROBE_EVIDENCE.md).
**Companion docs:**
[`../02_PHASE_CLOSEOUTS/PHASE_11D_ACCEL_PROBE_IMPLEMENTATION.md`](../02_PHASE_CLOSEOUTS/PHASE_11D_ACCEL_PROBE_IMPLEMENTATION.md),
[`../02_PHASE_CLOSEOUTS/PHASE_11C_ACCEL_PROBE_SPEC.md`](../02_PHASE_CLOSEOUTS/PHASE_11C_ACCEL_PROBE_SPEC.md).

### 11E.1 Purpose

Phase 11E provides hardware-witnessed evidence that the Phase 11D
`T` command works on real hardware. It follows the Phase 6O
sidecar `reset run` discipline via
`capture_devkit_rtt.ps1` and uses the Phase 11D host harness
`run_phase11d_accel_probe_demo.ps1` with canonical sequence `T T ?`.

### 11E.2 Hardware and build

- **Board:** STM32F411E-DISCO revision D; accel0 = lsm303agr_accel;
  lis2dh driver; I2C1/0x19; PA2 TX / PA3 RX / COM5 @ 115200 8N1.
- **Build:** pristine `west build -b stm32f411e_disco` at `2040bfb`.
  FLASH 41528 B / RAM 12352 B; 161/161 steps. Byte-identical to Phase
  11D. `.config`: `CONFIG_I2C=y`, `CONFIG_SENSOR=y`, `CONFIG_LIS2DH=y`,
  `CONFIG_LIS2DH_TRIGGER_NONE=y`; `CONFIG_SPI`, `CONFIG_ADC`,
  `CONFIG_CBPRINTF_FP_SUPPORT` absent.
- **Flash:** `west flash`; 49152 bytes written; PASS.
- **POST_FLASH_AUTOSTART:** sidecar `reset run` via
  `capture_devkit_rtt.ps1`; RTT control block `_SEGGER_RTT` at
  `0x20000ad0`; `reset run` issued before streaming.

### 11E.3 Host transcript (captured)

Transcript:
[`../../../devkit/logs/phase_11E_accel_probe_host_2026-05-12.txt`](../../../devkit/logs/phase_11E_accel_probe_host_2026-05-12.txt)
(388 bytes).

```text
SEND 'T' (0x54) -> RECV: ACC x=-0.561300 y=2.619400 z=9.167900
SEND 'T' (0x54) -> RECV: ACC x=-0.598720 y=2.507140 z=9.167900
SEND '?' (0x3f) -> RECV: STATE state=IDLE transitions=0 button=0 uart=3 ignored=0
```

Both `T` responses match the frozen shape; not byte-identical (live
sensor sampling). `?` response shape unchanged. `PHASE_11C_FORMAT_SIGN_EDGE`
observed correctly: `x=-0.561300` (val1=0, val2<0 edge case handled).

### 11E.4 RTT counters (final, ticks=120)

RTT log:
[`../../../devkit/logs/phase_11E_accel_probe_rtt_2026-05-12.txt`](../../../devkit/logs/phase_11E_accel_probe_rtt_2026-05-12.txt)
(22334 bytes, 61.4 s, Phase 6O harness).

- `ROBOTOS_UART rx=3 ok=3 handled=3 last=0x3f` (+3 from 0 baseline)
- `ROBOTOS_APP state=IDLE transitions=0 button=0 uart=3 ignored=0`
  (+3 uart; transitions/ignored/button unchanged)
- `ROBOTOS_OBS accepted=63 dispatched=62 pending=1 peak=2 dropped=0 herr=0`
- `ROBOTOS_PROD attempted=60 ok=60`
- `CFSR=0x00000000 HFSR=0x00000000` (13 occurrences each)

Invariants: `accepted−dispatched=pending` (63−62=1) ✓;
`PROD ok + UART ok = accepted` (60+3=63) ✓;
`UART rx=handled=APP uart=3` ✓.

Six required RTT patterns FOUND by Phase 6O harness (including
`Phase 11D-T accel probe`). Harness exit: **PASS**.

### 11E.5 Physical sanity

**`OPERATOR_PHYSICAL_SANITY_CONFIRMED`** — Z axis reads ~9.17 m/s²
with board flat on bench (close to 9.81 m/s² gravitational
acceleration; slight deviation is expected without calibration).
X = −0.56 m/s², Y = 2.62 m/s² (minor tilt). Values are
physically plausible. No calibration claimed.

### 11E.6 Scope guards

Zero source/config changes in Phase 11E. All 12 UART TX scope-guard
constraints from
[`../02_PHASE_CLOSEOUTS/PHASE_9EZ_CHECKPOINT.md`](../02_PHASE_CLOSEOUTS/PHASE_9EZ_CHECKPOINT.md)
§H intact. Scheduler 7A/7B remains DEFER. F407 remains HOLD/DEFER.
UART TX remains minimal response only. POST_FLASH_AUTOSTART discipline
unchanged. ACTIVE disarm widening `USER_DECISION_REQUIRED_ACTIVE_DISARM`
preserved and decoupled.

### 11E.7 Verdict

`CLOSED_WITH_HARDWARE_EVIDENCE`. `T` command is hardware-evidence-backed.
All Phase 11C §I.1/§I.2/§I.4 invariants confirmed. Phase 11A-E sensor
probe track complete.

---

<a id="phase-11z"></a>
## Phase 11Z -- Command-Set Checkpoint

**Status:** `CLOSED_DOCS_ONLY`
**Type:** Docs-only checkpoint / design-state consolidation. **No
source, runtime, test, CMake, Zephyr, board, host-tool, script, or
`prj.conf` change.** Snapshots the validated command surface after
the Phase 11A–11E sensor-probe track closed; prevents blind opening
of the next implementation phase.
**Date opened/closed:** 2026-05-12 (same-day docs-only close)
**Published baseline at open:** `origin/master = 10710b3`
**Closeout doc:**
[`../02_PHASE_CLOSEOUTS/PHASE_11Z_COMMAND_SET_CHECKPOINT.md`](../02_PHASE_CLOSEOUTS/PHASE_11Z_COMMAND_SET_CHECKPOINT.md).
**Companion docs:**
[`../02_PHASE_CLOSEOUTS/PHASE_11E_ACCEL_PROBE_EVIDENCE.md`](../02_PHASE_CLOSEOUTS/PHASE_11E_ACCEL_PROBE_EVIDENCE.md),
[`../02_PHASE_CLOSEOUTS/PHASE_10C_COMMAND_SET_CHECKPOINT.md`](../02_PHASE_CLOSEOUTS/PHASE_10C_COMMAND_SET_CHECKPOINT.md),
[`../03_SPECS/COMMAND_SET_DRAFT.md`](../03_SPECS/COMMAND_SET_DRAFT.md).

### 11Z.1 Purpose

Phase 11Z snapshots the validated command surface after Phase 11E
closed the sensor-probe track with hardware evidence. It is the
analogue of Phase 10C, taken after the Phase 11A–11E sensor-probe
addition.

Phase 11Z exists to:

- Record that the Phase 11A–11E sensor-probe track is **complete**.
- Provide a single-page authoritative inventory of the validated
  9-command set.
- Restate the 12 UART TX scope-guard constraints against the
  post-11E baseline.
- List the remaining open decisions so the next phase opens with
  written context.

Phase 11Z is **not** an implementation phase, **not** a behavioral
specification, **not** an authorization for ACTIVE disarm widening,
Framework API, Application semantics, Scheduler reopening, F407, or
any other gate.

### 11Z.2 Validated command set

The validated command set at Phase 11Z is:

```text
a / s / r / ? / x / v / L / d / T
```

Nine single-byte commands; all hardware-evidence-backed; all fit the
single-byte / fixed 96-byte stack buffer / no-parser / no-registry /
no-framing / thread-context-TX pattern. See
[`../02_PHASE_CLOSEOUTS/PHASE_11Z_COMMAND_SET_CHECKPOINT.md`](../02_PHASE_CLOSEOUTS/PHASE_11Z_COMMAND_SET_CHECKPOINT.md)
§C for the full inventory table.

### 11Z.3 Phase 11 sensor-probe track summary

| Gate | Decision / Result | Status |
|---|---|---|
| 11A | `SENSOR_SURFACE_DECIDED_ADAPTER_PROBE` | `CLOSED_DOCS_ONLY` |
| 11B | `FEASIBILITY_CONFIRMED_ONBOARD_MEMS` + `NO_PURCHASE_NEEDED_FOR_NEXT_STEP` | `CLOSED_DOCS_ONLY` |
| 11C | `PHASE_11C_SPEC_FROZEN_ACCEL_LIS2DH_REV_D` | `CLOSED_DOCS_ONLY` |
| 11D | Implementation per frozen spec; build PASS | `IMPLEMENTATION_CLOSED_HARDWARE_EVIDENCE_PENDING` |
| 11E | Two ACC responses observed; counters/invariants PASS; CFSR/HFSR=0; `OPERATOR_PHYSICAL_SANITY_CONFIRMED` (Z ≈ 9.17 m/s² flat-bench) | **`CLOSED_WITH_HARDWARE_EVIDENCE`** |

### 11Z.4 Remaining decisions

All preserved with no change at Phase 11Z:

1. ACTIVE disarm widening — **`USER_DECISION_REQUIRED_ACTIVE_DISARM`**;
   `d` from ACTIVE remains recognized no-op.
2. Scheduler 7A/7B — **`DEFER`**; no workload evidence justifies
   reopening (Phase 11E `peak=2 dropped=0 herr=0`).
3. F407 / custom board — **`HOLD/DEFER`**; no portability requirement
   surfaced.
4. POST_FLASH_AUTOSTART root cause — **`OPEN`**, `MITIGATED_BY_WORKFLOW`
   via Phase 6O sidecar `reset run`.
5. Robot Framework API planning — **`NOT_STARTED`**; possible future
   docs-only phase (analog of Phase 11A).
6. Application / product layer — **`NOT_STARTED`**; no product
   direction.

### 11Z.5 Scope guards intact

All 12 UART TX scope-guard constraints from
[`../02_PHASE_CLOSEOUTS/PHASE_9EZ_CHECKPOINT.md`](../02_PHASE_CLOSEOUTS/PHASE_9EZ_CHECKPOINT.md)
§H preserved. `core/`, `platform/`, `tests/`,
`devkit/CMakeLists.txt` top-level, `devkit_runtime.{c,h}`,
`devkit_status_led.{c,h}`, button/timer producers, observability/
build-info/fault modules, board DTS, board defconfig, B-revision
overlay, Zephyr workspace tracked files, `DEVKIT_PROGRESS.md`
historical master, and all evidence logs zero-diff. The single
intentional change since Phase 9E baseline is `devkit/prj.conf` adding
`CONFIG_I2C=y` + `CONFIG_SENSOR=y` (Phase 11D); Phase 11Z does not
modify this delta.

### 11Z.6 Verdict

`CLOSED_DOCS_ONLY`. Phase 11A–11E sensor-probe track complete and
shelved as a stable checkpoint. No firmware change, no test change,
no scope expansion, no semantics change, no purchase authorization.

### 11Z.7 Next gate

**Hold.** No phase opens automatically. Future possible phases (each
docs-only or implementation, each requiring explicit user
authorization): Phase 12A-class Framework API surface planning,
ACTIVE disarm widening, Adapter SPI/ADC probe, POST_FLASH_AUTOSTART
investigation. Phase 11Z authorizes none.

---

<a id="phase-12a"></a>
## Phase 12A -- Robot Framework API Surface Planning

**Status:** `CLOSED_DOCS_ONLY`
**Type:** Docs-only architecture gate / Framework API surface planning. **No
source, runtime, test, CMake, Zephyr, board, host-tool, script, `prj.conf`,
DTS overlay, evidence log, or `framework/` directory change.**
**Date opened/closed:** 2026-05-12 (same-day docs-only close)
**Published baseline at open:** `origin/master = c239466`
**Closeout doc:**
[`../02_PHASE_CLOSEOUTS/PHASE_12A_FRAMEWORK_API_SURFACE_PLANNING.md`](../02_PHASE_CLOSEOUTS/PHASE_12A_FRAMEWORK_API_SURFACE_PLANNING.md).
**Companion docs:**
[`../02_PHASE_CLOSEOUTS/PHASE_11Z_COMMAND_SET_CHECKPOINT.md`](../02_PHASE_CLOSEOUTS/PHASE_11Z_COMMAND_SET_CHECKPOINT.md),
[`../02_PHASE_CLOSEOUTS/PHASE_11A_ADAPTER_BOUNDARY_SENSOR_SURFACE.md`](../02_PHASE_CLOSEOUTS/PHASE_11A_ADAPTER_BOUNDARY_SENSOR_SURFACE.md),
[`../03_SPECS/COMMAND_SET_DRAFT.md`](../03_SPECS/COMMAND_SET_DRAFT.md).

### 12A.1 Purpose

Phase 12A is the first Robot Framework planning gate. It is the bridge from
Phase 1–11 Adapter/devkit evidence to Framework contract planning. It is
analogous to Phase 11A (Adapter boundary decision before the sensor-probe
track) but at the Framework layer boundary.

Phase 12A exists to:

- State the layer boundary between the Adapter/runtime substrate (built) and
  the Robot Framework layer (not yet built).
- Inventory Adapter evidence available to Framework — what the Framework may
  rely on without re-proving.
- Evaluate candidate Framework API domains.
- Select a recommended first Framework slice for Phase 12B.
- Record remaining open gates so the next phase opens with written context.

Phase 12A is **not** an implementation phase. It does **not** implement any
Framework API, create a `framework/` directory, change any devkit command,
promote `devkit_app_state`, touch `core/` or `platform/`, or authorize any
follow-on phase automatically.

### 12A.2 Layer boundary restated

| Layer | Status |
|---|---|
| Kernel / HW | Zephyr 3.6.0 + STM32F411E-DISCO; used as foundation |
| Robot Adapter / runtime substrate | `core/` + `platform/` + devkit hardware glue; **substantially built** |
| devkit / validation harness | `devkit/src/`; UART vocabulary `a/s/r/?/x/v/L/d/T`; **substantially built** |
| Robot Framework | **Not built** — no `framework/` dir, no Framework header, no API |
| Application / product | **Not built** — no product vocabulary, no use case |

Key carry-forward assertions from Phase 11A §C and Phase 11Z §F:

- `devkit_app_state` is **not** Robot Framework (scope-guard #11; not lifted).
- `a/s/r/?/x/v/L/d/T` are devkit probe commands, not product commands.
- `T` is an Adapter probe evidence command, not a Framework sensor API.
- Framework must consume Adapter primitives through explicit contracts, not by
  copying devkit patterns blindly.

### 12A.3 Adapter evidence available to Framework

All 11 Adapter primitive classes (time/tick, thread-context boundary, critical
section, ISR-safe event post, queue/dispatch/budget, GPIO input, GPIO output,
UART RX/TX, RTT telemetry, timer events, driver-dependent sensor read) are
proven. The driver-dependent read sub-class (I2C / `struct sensor_value`)
was the last open class, closed by Phase 11D/11E.

Adapter limitations visible to Framework design: fixed dispatch budget
(`MAX_EVENTS_PER_TICK=1`); no explicit `robotos_adapter.h` aggregate header;
pool/slab open question; only one sensor type characterized; portability
backend undemonstrated (only Zephyr/STM32F4).

### 12A.4 Candidate Framework API domains evaluated

Nine candidate domains evaluated in the closeout doc §E. Summary:

| Domain | Recommendation |
|---|---|
| State-machine abstraction | **Recommended first slice** (§F) |
| Timer / service abstraction | Second priority |
| Sensor abstraction | Third priority (one data point; defer full form) |
| Fault / safety abstraction | Second-tier companion to FSM |
| Framework observability hooks | Third-tier; design alongside FSM + timer |
| Actuator (stepper/servo/DC) | Deferred — no actuator hardware characterized |
| Endstop / limit-switch | Deferred — pair with actuator |
| PID / control-loop | Deferred — needs sensor + actuator + timer first |
| Motion primitive / trajectory | Deferred — product-dependent; pool/slab open |

### 12A.5 Recommended first Framework slice

**Framework State-Machine Abstraction (flat FSM only).** Rationale: most
Adapter evidence reuse (`devkit_app_state` as design reference); no new
hardware; no Scheduler change needed; no product vocabulary risk. See
closeout §F for full rationale and §G for non-final illustrative API names
(`robotos_fw_state_machine_*`).

### 12A.6 Remaining decisions

All preserved unchanged:

1. ACTIVE disarm widening — **`USER_DECISION_REQUIRED_ACTIVE_DISARM`**
2. Scheduler 7A/7B — **`DEFER`**
3. F407 / custom board — **`HOLD/DEFER`**
4. POST_FLASH_AUTOSTART root cause — **`OPEN`** / `MITIGATED_BY_WORKFLOW`
5. Application / product layer — **`NOT_STARTED`**

### 12A.7 Verdict

`CLOSED_DOCS_ONLY`. No firmware change, no test change, no scope expansion,
no semantics change, no purchase authorization. Framework planning boundary
established. Recommended next gate: Phase 12B — Robot Framework FSM API Draft
(docs-only; explicit user authorization required).

---

<a id="phase-12b"></a>
## Phase 12B -- Robot Framework FSM API Draft

**Status:** `CLOSED_DOCS_ONLY`
**Type:** Docs-only API surface draft. **No source, runtime, test, CMake,
Zephyr, board, host-tool, script, `prj.conf`, DTS overlay, evidence log,
`framework/` directory, or `.h`/`.c` Framework file change.**
**Date opened/closed:** 2026-05-12 (same-day docs-only close)
**Published baseline at open:** `origin/master = 76cb241`
**Closeout doc:**
[`../02_PHASE_CLOSEOUTS/PHASE_12B_FRAMEWORK_FSM_API_DRAFT.md`](../02_PHASE_CLOSEOUTS/PHASE_12B_FRAMEWORK_FSM_API_DRAFT.md).
**Long-lived spec draft:**
[`../03_SPECS/FRAMEWORK_FSM_API_DRAFT.md`](../03_SPECS/FRAMEWORK_FSM_API_DRAFT.md).
**Companion docs:**
[`../02_PHASE_CLOSEOUTS/PHASE_12A_FRAMEWORK_API_SURFACE_PLANNING.md`](../02_PHASE_CLOSEOUTS/PHASE_12A_FRAMEWORK_API_SURFACE_PLANNING.md),
[`../02_PHASE_CLOSEOUTS/PHASE_11Z_COMMAND_SET_CHECKPOINT.md`](../02_PHASE_CLOSEOUTS/PHASE_11Z_COMMAND_SET_CHECKPOINT.md).

### 12B.1 Purpose

Phase 12B drafts the Robot Framework flat FSM API surface at design level.
It is the first spec-only phase for the Framework layer, analogous to Phase
11C (probe spec before probe implementation).

Phase 12B:

- Defines the FSM model: flat, table-driven, no heap, static config.
- Drafts conceptual types and function signatures (DRAFT / NON-FINAL).
- Audits the devkit event type allocation (100–103) and confirms that
  Framework FSM event IDs are decoupled from `robotos_event_type_t`.
- Records open design decisions for Phase 12C confirmation.
- Creates `03_SPECS/FRAMEWORK_FSM_API_DRAFT.md` as a long-lived spec.

Phase 12B does **not** implement the FSM, create a `framework/` directory,
modify devkit source, change command semantics, or authorize Phase 12C
automatically.

### 12B.2 Decision result

**`PHASE_12B_FSM_API_DRAFTED_DOCS_ONLY`**

FSM model confirmed: flat, table-driven, static config, no heap. Illustrative
`robotos_fw_fsm_*` API surface drafted in closeout §F and spec §4 (all DRAFT /
NON-FINAL). Two open decisions flagged for Phase 12C:

1. **Event bridge pattern** — Application layer translates Adapter events
   to `robotos_fw_event_id_t`; recommended as `OPEN_RECOMMENDATION_PENDING_CONFIRMATION`.
2. **Status model** — reuse `robotos_core_status_t` (Option A) recommended;
   `OPEN_RECOMMENDATION_PENDING_CONFIRMATION`.

### 12B.3 FSM model summary

| Model axis | Decision |
|---|---|
| Flat vs hierarchical | Flat only (Phase 12B scope) |
| Table-driven vs callback-router | Table-driven: `const robotos_fw_transition_t[]` |
| Static vs dynamic config | Static compile-time; no heap |
| State / event IDs | Product-defined `uint32_t` |
| Event namespace | Separate from `robotos_event_type_t`; no sub-range needed |
| Transition evaluation | First-match FIFO; guard-skip on reject |
| Dispatch thread | Thread context only; ISR-safe state query only |

### 12B.4 Remaining decisions

All preserved unchanged:

1. ACTIVE disarm widening — **`USER_DECISION_REQUIRED_ACTIVE_DISARM`**
2. Scheduler 7A/7B — **`DEFER`**
3. F407 / custom board — **`HOLD/DEFER`**
4. POST_FLASH_AUTOSTART — **`OPEN`** / `MITIGATED_BY_WORKFLOW`
5. Application / product layer — **`NOT_STARTED`**
6. Robot Framework (implementation) — **`NOT_STARTED`**; Phase 12B drafts API only

### 12B.5 Scope guards intact

All 12 UART TX scope-guard constraints preserved. Zero source/config/evidence-
log changes. `devkit_app_state` not promoted (scope-guard #11). `core/`,
`platform/`, `devkit/src/`, board DTS, `prj.conf`, and evidence logs
zero-diff. `T` remains Adapter probe evidence, not Framework API.

### 12B.6 Verdict

`CLOSED_DOCS_ONLY`. Framework FSM API surface drafted at design level.
Long-lived spec created at `03_SPECS/FRAMEWORK_FSM_API_DRAFT.md`. No
implementation, no source change, no `framework/` dir.

### 12B.7 Next gate

Phase 12C — Framework FSM Event Bridge Spec + Status Model Confirmation
(docs-only). Entry requires explicit user authorization. Confirms the two
open decisions from §N before any header or implementation work.

---

<a id="phase-12c"></a>
## Phase 12C -- Framework FSM Event Bridge + Status Model Confirmation

**Status:** `CLOSED_DOCS_ONLY`
**Type:** Docs-only design-confirmation phase. **No source, runtime, test,
CMake, Zephyr, board, host-tool, script, `prj.conf`, DTS overlay, evidence
log, `framework/` directory, or `.h`/`.c` Framework file change.**
**Date opened/closed:** 2026-05-12 (same-day docs-only close)
**Published baseline at open:** `origin/master = cda3810`
**Closeout doc:**
[`../02_PHASE_CLOSEOUTS/PHASE_12C_FSM_EVENT_BRIDGE_STATUS_MODEL.md`](../02_PHASE_CLOSEOUTS/PHASE_12C_FSM_EVENT_BRIDGE_STATUS_MODEL.md).
**Long-lived spec updated:**
[`../03_SPECS/FRAMEWORK_FSM_API_DRAFT.md`](../03_SPECS/FRAMEWORK_FSM_API_DRAFT.md)
(§1 decision-state table, §3.2 evaluation order, §6.2/6.3, §7.2, §9, §10).
**Companion docs:**
[`../02_PHASE_CLOSEOUTS/PHASE_12B_FRAMEWORK_FSM_API_DRAFT.md`](../02_PHASE_CLOSEOUTS/PHASE_12B_FRAMEWORK_FSM_API_DRAFT.md),
[`../02_PHASE_CLOSEOUTS/PHASE_12A_FRAMEWORK_API_SURFACE_PLANNING.md`](../02_PHASE_CLOSEOUTS/PHASE_12A_FRAMEWORK_API_SURFACE_PLANNING.md).

### 12C.1 Purpose

Phase 12C confirms the four open design decisions left by Phase 12B before
any Phase 12D header stub or implementation work. It also records one
correction to the Phase 12B evaluation-order draft.

Phase 12C:

- Confirms event bridge pattern, status model, payload lifetime, and action
  non-OK return semantics.
- Updates `FRAMEWORK_FSM_API_DRAFT.md` §9 entries from `OPEN` to `CONFIRMED`.
- Corrects evaluation order: exit → state update → action → entry (was
  drafted as exit → action → state update → entry in Phase 12B §E.3).
- Clarifies counter independence: `guard_rejected_count` and
  `no_transition_count` may both increment in a single dispatch.

Phase 12C does **not** implement the FSM, create a `framework/` directory,
modify devkit source, change command semantics, or start Phase 12D.

### 12C.2 Decision result

**`PHASE_12C_EVENT_BRIDGE_STATUS_CONFIRMED_DOCS_ONLY`**

| Decision | Phase 12C status |
|---|---|
| Event bridge pattern | `APPLICATION_OWNED_EVENT_BRIDGE_CONFIRMED` |
| Status model | `REUSE_ROBOTOS_CORE_STATUS_T_FOR_PHASE_12D_CONFIRMED` |
| Payload lifetime | `PAYLOAD_BORROWED_FOR_DISPATCH_ONLY_CONFIRMED` |
| Action non-OK semantics | `ACTION_NON_OK_NO_ROLLBACK_CONFIRMED` |
| Guard return type | `GUARD_RETURNS_BOOL_ONLY_CONFIRMED` |
| Evaluation order | Corrected to exit → state update → action → entry |

### 12C.3 Status mapping (Phase 12D target)

| FSM situation | `robotos_core_status_t` | Audit counter |
|---|---|---|
| OK / transition accepted | `ROBOTOS_CORE_OK` | `transition_count++` |
| No transition matched | `ROBOTOS_CORE_OK` | `no_transition_count++` |
| Guard rejected | `ROBOTOS_CORE_OK` | `guard_rejected_count++`; scan continues |
| Action returned non-OK | action's status | `transition_count++` (committed) |
| NULL fsm/config/transitions | `ROBOTOS_CORE_ERR_NULL` | none |
| Bad config values | `ROBOTOS_CORE_ERR_INVALID_ARG` | none |
| Uninitialized FSM | `ROBOTOS_CORE_ERR_INVALID_STATE` | none |

### 12C.4 Remaining decisions

All preserved unchanged:

1. ACTIVE disarm widening — **`USER_DECISION_REQUIRED_ACTIVE_DISARM`**
2. Scheduler 7A/7B — **`DEFER`**
3. F407 / custom board — **`HOLD/DEFER`**
4. POST_FLASH_AUTOSTART — **`OPEN`** / `MITIGATED_BY_WORKFLOW`
5. Application / product layer — **`NOT_STARTED`**
6. Robot Framework implementation — **`NOT_STARTED`**; Phase 12D not started

### 12C.5 Scope guards intact

All 12 UART TX scope-guard constraints preserved. Zero source/config/evidence-
log changes. `devkit_app_state` not promoted (scope-guard #11). `core/`,
`platform/`, `devkit/src/`, board DTS, `prj.conf`, and evidence logs
zero-diff. Pre-existing `src/framework/*.c` files (from `43de448`) unmodified.
`T` remains Adapter probe evidence, not Framework API.

### 12C.6 Verdict

`CLOSED_DOCS_ONLY`. All four Phase 12B open decisions confirmed. Spec draft
updated. No implementation, no source change, no `framework/` dir, no `.h`/
`.c` file.

### 12C.7 Next gate

Phase 12D — Framework FSM Header Stub / Compile-only Skeleton. Entry
requires **explicit user authorization** (first Framework-layer source file)
and user direction on the exact Framework layer path. Non-goals: no `.c`
body, no `devkit_app_state` replacement, no UART command integration, no
hardware run, no scheduler change, no product behavior. **Hold** is also
fully acceptable.

---

<a id="phase-12d-pre"></a>
## Phase 12D-pre -- Legacy Framework Scaffold Disposition

**Status:** `CLOSED_DOCS_ONLY`
**Type:** Docs-only architecture-boundary phase. **No source, runtime, test,
CMake, Zephyr, board, host-tool, script, `prj.conf`, DTS overlay, evidence
log change. No source file deleted, moved, or renamed. No new Framework
path created.**
**Date opened/closed:** 2026-05-12 (same-day docs-only close)
**Published baseline at open:** `origin/master = c3a9384`
**Closeout doc:**
[`../02_PHASE_CLOSEOUTS/PHASE_12D_PRE_LEGACY_FRAMEWORK_SCAFFOLD_DISPOSITION.md`](../02_PHASE_CLOSEOUTS/PHASE_12D_PRE_LEGACY_FRAMEWORK_SCAFFOLD_DISPOSITION.md).
**New legacy notices placed in repo tree:**
[`../../../src/README_LEGACY_SCAFFOLD.md`](../../../src/README_LEGACY_SCAFFOLD.md),
[`../../../src/framework/DEPRECATED.md`](../../../src/framework/DEPRECATED.md),
[`../../../include/robotos/DEPRECATED.md`](../../../include/robotos/DEPRECATED.md).
**Companion docs:**
[`../02_PHASE_CLOSEOUTS/PHASE_12C_FSM_EVENT_BRIDGE_STATUS_MODEL.md`](../02_PHASE_CLOSEOUTS/PHASE_12C_FSM_EVENT_BRIDGE_STATUS_MODEL.md),
[`../02_PHASE_CLOSEOUTS/PHASE_12B_FRAMEWORK_FSM_API_DRAFT.md`](../02_PHASE_CLOSEOUTS/PHASE_12B_FRAMEWORK_FSM_API_DRAFT.md),
[`../02_PHASE_CLOSEOUTS/PHASE_12A_FRAMEWORK_API_SURFACE_PLANNING.md`](../02_PHASE_CLOSEOUTS/PHASE_12A_FRAMEWORK_API_SURFACE_PLANNING.md).

### 12D-pre.1 Purpose

Phase 12D-pre classifies the pre-existing `RobotOS_v1.0/src/` and
`RobotOS_v1.0/include/robotos/` scaffold (committed at the original
`43de448` baseline, 2025-03-05) as **frozen / non-authoritative legacy
scaffold**, so that Phase 12D may proceed on `RobotOS_v1.0/framework/`
without silent architectural drift between the two coexisting stacks in
the repo.

Phase 12D-pre exists because the Framework Layer Path Decision Audit after
Phase 12C found two architecturally distinct stacks rooted at the same
baseline:

- **Architecture A** (`core/`+`platform/`+`devkit/`) — active,
  evidence-backed, Phase 1–12 canonical.
- **Architecture B** (`src/`+`include/robotos/`+root `CMakeLists.txt`) —
  frozen since baseline, type-incompatible with A, zero hardware evidence,
  zero compile overlap.

Phase 12D-pre records the classification, places notices, and blocks
Phase 12D from opening `RobotOS_v1.0/framework/` until the boundary is
explicit.

### 12D-pre.2 Decision result

**`LEGACY_SCAFFOLD_MARKED_FROZEN_DOCS_ONLY`**

| Statement | Status |
|---|---|
| Architecture A is canonical for active RobotOS Phase 12+ work | Confirmed |
| Architecture B is legacy / frozen / non-authoritative | Confirmed |
| `src/framework/` is not the active path for Phase 12+ Robot Framework | Confirmed |
| `include/robotos/` is not the active namespace for Phase 12+ Framework FSM | Confirmed |
| Root `RobotOS_v1.0/CMakeLists.txt` belongs to the legacy build path | Confirmed (file unmodified) |
| Future active Framework path = `RobotOS_v1.0/framework/` | Recorded |
| Phase 12D may create `RobotOS_v1.0/framework/` only on explicit user authorization after Phase 12D-pre is pushed | Recorded |

### 12D-pre.3 Notices placed in legacy tree

| Path | Content |
|---|---|
| `RobotOS_v1.0/src/README_LEGACY_SCAFFOLD.md` | Whole `src/` tree is legacy/frozen scaffold; not the active stack; rules for not extending |
| `RobotOS_v1.0/src/framework/DEPRECATED.md` | This directory is the frozen legacy Robot Framework scaffold; type-incompatible with Phase 12+ design; do not add new work here |
| `RobotOS_v1.0/include/robotos/DEPRECATED.md` | This namespace bundles legacy HAL and Framework headers; do not add `robotos_fw_*.h` here |

### 12D-pre.4 What is not changed

- Zero source file under `src/`, `include/`, `core/`, `platform/`,
  `devkit/src/` modified.
- Zero file deleted, moved, or renamed.
- Zero CMakeLists.txt modified.
- Zero `prj.conf`, board DTS, overlay, host tool, runtime script, or
  evidence log modified.
- Zero new `.h` or `.c` Framework file created.
- `RobotOS_v1.0/framework/` directory **not created**.
- All 12 UART TX scope-guard constraints from `PHASE_9EZ_CHECKPOINT.md §H`
  preserved.
- `devkit_app_state` unchanged (scope-guard #11 re-affirmed).
- `T` remains Adapter probe evidence; not promoted.
- Command set `a/s/r/?/x/v/L/d/T` unchanged.
- Phase 12A/12B/12C closeouts not rewritten.

### 12D-pre.5 Remaining decisions

All preserved unchanged:

1. ACTIVE disarm widening — **`USER_DECISION_REQUIRED_ACTIVE_DISARM`**
2. Scheduler 7A/7B — **`DEFER`**
3. F407 / custom board — **`HOLD/DEFER`**
4. POST_FLASH_AUTOSTART — **`OPEN`** / `MITIGATED_BY_WORKFLOW`
5. Application / product layer — **`NOT_STARTED`**
6. Robot Framework implementation — **`NOT_STARTED`**; Phase 12D pending explicit user authorization

### 12D-pre.6 Verdict

`CLOSED_DOCS_ONLY`. Legacy scaffold classified; notices placed; no source
change; no file deletion; no path creation. Phase 12D unblocked at the
architectural-boundary level; still requires explicit user authorization
to open.

### 12D-pre.7 Next gate

**Phase 12D — Framework FSM Header Stub.** Path: `RobotOS_v1.0/framework/`.
Scope: docs + single header (`framework/README.md` + `framework/robotos_fw_fsm.h`)
plus Phase 12D closeout, spec update (§4 DRAFT → LOCKED-AT-12D), progress
entry, CURRENT_STATE, INDEX. No `.c` body, no CMake change, no devkit
integration, no hardware run, no Scheduler change, no F407, no Application/
product work, no Architecture B modification. **Requires explicit user
authorization.** Hold is fully acceptable.

---

<a id="phase-12d"></a>
## Phase 12D -- Framework FSM Header Stub

**Status:** `CLOSED_HEADER_STUB_ONLY`
**Type:** Header stub + docs. **No `.c` body, no CMake change, no devkit
integration, no Zephyr config change, no hardware run.** Two new files
under `RobotOS_v1.0/framework/`: layer README + single public header
(`robotos_fw_fsm.h`). No file under `core/`, `platform/`, `devkit/src/`,
`src/`, `include/robotos/`, `tests/`, `boards/`, or any evidence log
modified.
**Date opened/closed:** 2026-05-12 (same-day header-stub close)
**Published baseline at open:** `origin/master = 2385b0f`
**Closeout doc:**
[`../02_PHASE_CLOSEOUTS/PHASE_12D_FSM_HEADER_STUB.md`](../02_PHASE_CLOSEOUTS/PHASE_12D_FSM_HEADER_STUB.md).
**New active Framework path:**
[`../../../framework/`](../../../framework/) with
[`../../../framework/README.md`](../../../framework/README.md) and
[`../../../framework/robotos_fw_fsm.h`](../../../framework/robotos_fw_fsm.h).
**Long-lived spec updated:**
[`../03_SPECS/FRAMEWORK_FSM_API_DRAFT.md`](../03_SPECS/FRAMEWORK_FSM_API_DRAFT.md)
(§1 decision-state table extended; §4 API surface marked
`LOCKED-AT-12D`; §10 next-revision conditions updated).
**Companion docs:**
[`../02_PHASE_CLOSEOUTS/PHASE_12D_PRE_LEGACY_FRAMEWORK_SCAFFOLD_DISPOSITION.md`](../02_PHASE_CLOSEOUTS/PHASE_12D_PRE_LEGACY_FRAMEWORK_SCAFFOLD_DISPOSITION.md),
[`../02_PHASE_CLOSEOUTS/PHASE_12C_FSM_EVENT_BRIDGE_STATUS_MODEL.md`](../02_PHASE_CLOSEOUTS/PHASE_12C_FSM_EVENT_BRIDGE_STATUS_MODEL.md),
[`../02_PHASE_CLOSEOUTS/PHASE_12B_FRAMEWORK_FSM_API_DRAFT.md`](../02_PHASE_CLOSEOUTS/PHASE_12B_FRAMEWORK_FSM_API_DRAFT.md),
[`../02_PHASE_CLOSEOUTS/PHASE_12A_FRAMEWORK_API_SURFACE_PLANNING.md`](../02_PHASE_CLOSEOUTS/PHASE_12A_FRAMEWORK_API_SURFACE_PLANNING.md).

### 12D.1 Purpose

Phase 12D creates the **first active Robot Framework artifact** under
the canonical path `RobotOS_v1.0/framework/` recorded by Phase 12D-pre.
The artifact is a single public header `robotos_fw_fsm.h` declaring the
flat FSM API surface confirmed by Phase 12B/12C, plus a Framework-layer
`README.md`. The phase stops at the header surface. No `.c` body. No
CMake integration. No devkit consumer. No hardware run.

### 12D.2 Decision result

**`PHASE_12D_FSM_HEADER_STUB_CREATED`** (`CLOSED_HEADER_STUB_ONLY`).

`RobotOS_v1.0/framework/` exists as an Architecture A sibling to
`core/`, `platform/`, `devkit/`. The Phase 12B/12C-confirmed FSM API is
**LOCKED-AT-12D** at the source-of-truth header
`RobotOS_v1.0/framework/robotos_fw_fsm.h`. Function bodies are absent
**by design**; linking against the header without a future
Phase 12E (or later) implementation fails at link time, as intended.

### 12D.3 Header surface (LOCKED-AT-12D)

The header `robotos_fw_fsm.h` is C99-compatible and includes only
`<stdbool.h>`, `<stdint.h>`, and `"robotos_core.h"`. It declares:

- Identifier types: `robotos_fw_state_id_t`, `robotos_fw_event_id_t`.
- Reserved sentinel: `ROBOTOS_FW_STATE_UNINIT`.
- Status alias: `robotos_fw_status_t` (alias of `robotos_core_status_t`).
- Callback types: `robotos_fw_guard_fn_t` (returns `bool`),
  `robotos_fw_action_fn_t`, `robotos_fw_entry_exit_fn_t`.
- Static config structs: `robotos_fw_transition_t`,
  `robotos_fw_state_def_t`, `robotos_fw_fsm_config_t`.
- Instance struct: `robotos_fw_fsm_t` (caller-owned; static allocation
  expected).
- Snapshot struct: `robotos_fw_fsm_snapshot_t`.
- Six function declarations: `robotos_fw_fsm_init`,
  `robotos_fw_fsm_dispatch`, `robotos_fw_fsm_get_state`,
  `robotos_fw_fsm_reset`, `robotos_fw_fsm_is_in_state`,
  `robotos_fw_fsm_get_snapshot`.

All Phase 12C decisions are encoded in the header's file-level comment
and per-function doc comments: application-owned event bridge, separate
Framework event ID namespace, `robotos_core_status_t` reuse, payload
borrowed for dispatch only, action non-OK no rollback, guard returns
`bool` only, evaluation order **exit → state update → action → entry**,
no heap, thread-context dispatch with ISR-safe state queries, no UART
TX from FSM, no hardware driver ownership, no core event-handler
registration.

### 12D.4 Architecture A boundary preserved

Phase 12D does not touch Architecture B (`src/`, `include/robotos/`,
root `RobotOS_v1.0/CMakeLists.txt`). The three Phase 12D-pre legacy
notices (`src/README_LEGACY_SCAFFOLD.md`, `src/framework/DEPRECATED.md`,
`include/robotos/DEPRECATED.md`) are zero-diff. The new header includes
no `ro_*` legacy symbol and no header from `include/robotos/`. The new
Framework path `RobotOS_v1.0/framework/` is a sibling of `core/`,
`platform/`, `devkit/` — never under `src/`.

### 12D.5 Syntax / validation result

`SYNTAX_CHECK_NOT_RUN_TOOLCHAIN_OUTPUT_SUPPRESSED`. A scratch
translation unit including the header was prepared at a disposable
temporary path and an attempt was made with
`gcc -fsyntax-only -std=c99 -Wall -Wextra` against the MSYS2 gcc 15.1.0
toolchain present on PATH. gcc returned exit code 1 on every invocation
(including a deliberately broken control source) with 0-byte stdout and
0-byte stderr regardless of Bash, PowerShell, or `cmd.exe` redirection.
Diagnostics are unrecoverable in this sandbox; the check is not
informative and is reported as `NOT_RUN`. Header was reviewed by hand
for include-guard balance, C99 conformance, `extern "C"` balance, and
absence of forbidden includes (no Zephyr, no devkit, no `ro_*`, no app
headers). A future implementation phase MUST run a toolchain-backed
compile via the Architecture A CMake build before claiming validity.
The scratch file was removed.

### 12D.6 What is not changed

- All `.c` files in the repo — zero-diff.
- All `CMakeLists.txt` (root legacy + `devkit/CMakeLists.txt`) — zero-diff.
- `core/`, `platform/`, `devkit/src/`, `devkit/boards/`,
  `devkit/zephyr/`, `prj.conf`, DTS overlays — zero-diff.
- `src/`, `include/robotos/`, `include/app/` — zero-diff after Phase
  12D-pre.
- `tests/`, `tests/host/`, all evidence logs — zero-diff.
- Host tools and runtime scripts — zero-diff.
- All prior closeout docs — not rewritten.
- `devkit_app_state` — unchanged (scope-guard #11 re-affirmed).
- Validated command set `a / s / r / ? / x / v / L / d / T` — unchanged.
- All 12 UART TX scope-guard constraints from
  [`../02_PHASE_CLOSEOUTS/PHASE_9EZ_CHECKPOINT.md`](../02_PHASE_CLOSEOUTS/PHASE_9EZ_CHECKPOINT.md)
  §H — preserved.

### 12D.7 Remaining decisions (preserved)

1. ACTIVE disarm widening — `USER_DECISION_REQUIRED_ACTIVE_DISARM`.
2. Scheduler 7A/7B — `DEFER`.
3. F407 / custom board — `HOLD/DEFER`.
4. POST_FLASH_AUTOSTART root cause — `OPEN` / `MITIGATED_BY_WORKFLOW`.
5. Application / product layer — `NOT_STARTED`.
6. Robot Framework implementation — `NOT_STARTED`; Phase 12E pending
   explicit user authorization AND identification of a concrete
   consumer or unit test.
7. Architecture A ↔ Architecture B reconciliation — `NOT_STARTED`; out
   of scope.

### 12D.8 Next gate

**Hold.** Phase 12E (Framework FSM implementation) may open only on
**explicit user authorization** AND with a concrete consumer (devkit
integration target) or unit-test plan identified in advance. Without
both, implementation has no integration path and would drift into dead
code. Hold is the recommended posture.

---

<a id="phase-12e-pre"></a>
## Phase 12E-pre -- Framework FSM Consumer / Test Plan

**Status:** `CLOSED_DOCS_ONLY`
**Type:** Docs-only planning gate. **No source, runtime, test, CMake,
Zephyr, board, `prj.conf`, DTS overlay, evidence log, Framework header,
Framework `.c` file, devkit integration, or command-set change.** No
file under `framework/`, `tests/host/`, `core/`, `platform/`,
`devkit/src/`, `src/`, `include/robotos/` modified.
**Date opened/closed:** 2026-05-12 (same-day docs-only close)
**Published baseline at open:** `origin/master = 87e0626`
**Closeout doc:**
[`../02_PHASE_CLOSEOUTS/PHASE_12E_PRE_FSM_CONSUMER_TEST_PLAN.md`](../02_PHASE_CLOSEOUTS/PHASE_12E_PRE_FSM_CONSUMER_TEST_PLAN.md).
**Companion docs:**
[`../02_PHASE_CLOSEOUTS/PHASE_12D_FSM_HEADER_STUB.md`](../02_PHASE_CLOSEOUTS/PHASE_12D_FSM_HEADER_STUB.md),
[`../02_PHASE_CLOSEOUTS/PHASE_12C_FSM_EVENT_BRIDGE_STATUS_MODEL.md`](../02_PHASE_CLOSEOUTS/PHASE_12C_FSM_EVENT_BRIDGE_STATUS_MODEL.md),
[`../02_PHASE_CLOSEOUTS/PHASE_12B_FRAMEWORK_FSM_API_DRAFT.md`](../02_PHASE_CLOSEOUTS/PHASE_12B_FRAMEWORK_FSM_API_DRAFT.md).

### 12E-pre.1 Purpose

Phase 12E-pre selects the validation and consumer path for the first
Framework implementation phase (Phase 12E). Phase 12D closed at the
header surface (`CLOSED_HEADER_STUB_ONLY`) with the syntax check
recorded as `SYNTAX_CHECK_NOT_RUN_TOOLCHAIN_OUTPUT_SUPPRESSED` because
the local MSYS2 MinGW64 toolchain produced exit=1 with 0-byte
diagnostics on every invocation. Phase 12E-pre answers the question
"what consumer/test path lets Phase 12E close with real evidence and
without scope drift?" before any `.c` file is created.

### 12E-pre.2 Decision result

**`PHASE_12E_RECOMMEND_HOST_UNIT_TEST_CONSUMER`** (`CLOSED_DOCS_ONLY`).

Phase 12E, when authorized, should implement
`RobotOS_v1.0/framework/robotos_fw_fsm.c` and validate it through a
**single new host test target** added to the existing
`RobotOS_v1.0/tests/host/CMakeLists.txt`. Validation runs on WSL Ubuntu
or Linux (the existing host CMake explicitly documents that Windows
MinGW64 is unreliable — same finding as Phase 12D). Test log captured
via the tracked `save_test_log.cmake` convention.

### 12E-pre.3 Candidate consumer/test paths evaluated

Five options were evaluated against safety, feasibility, and evidence
value:

1. **Host unit-test consumer (RECOMMENDED).** Reuses Architecture-A
   `tests/host/` infra proven through Phase 4-6 with ~20 tracked
   contract tests and tracked test logs. Additive only (one
   `add_executable`/`add_test` block). No devkit integration. No
   command-set risk. Validates every Phase 12B/12C decision.
2. **Compile-only skeleton (REJECTED).** No behavioral validation; sets
   the wrong precedent; Option 1 includes this option's benefit for
   free.
3. **Devkit integration consumer (REJECTED).** High scope-guard #11
   risk on `devkit_app_state`; command-semantics drift risk; hardware
   evidence + implementation + integration in one phase is too much.
4. **Application bridge prototype (REJECTED).** Application/product
   layer is `NOT_STARTED`; no product chosen; scope explosion.
5. **Hold (FALLBACK ACCEPTABLE).** Leaves Phase 12D `SYNTAX_CHECK_NOT_RUN`
   unresolved; acceptable only if user defers further; not the
   strongest option.

Full evaluation table in
[`../02_PHASE_CLOSEOUTS/PHASE_12E_PRE_FSM_CONSUMER_TEST_PLAN.md`](../02_PHASE_CLOSEOUTS/PHASE_12E_PRE_FSM_CONSUMER_TEST_PLAN.md)
§C.

### 12E-pre.4 Test infrastructure finding

**`TEST_INFRA_AUDIT_NOT_NEEDED -- EXISTING_HOST_TESTS_SUFFICE`.**

`RobotOS_v1.0/tests/host/` is the canonical Architecture-A host test
build:

- 426-line standalone `tests/host/CMakeLists.txt` (tracked).
- ~20 tracked contract test targets covering core, event queue,
  dispatcher, ingestion, tick policy, handler policy, platform critical,
  platform fault, scheduler admission, queue pressure, handler routing
  stress, handler lifecycle.
- Tracked platform host stubs (critical, fault, log, time).
- Tracked test logs through Phase 6H demonstrate the convention works.
- `save_test_log.cmake` tracked log-capture target.

Phase 12E patch surface is purely additive to this existing file. No
Phase 12E-test-pre infrastructure-creation phase is needed.

The four legacy host tests under `RobotOS_v1.0/tests/` root
(`test_app_sm.c`, `test_gcode_parser.c`, `test_kinematics_cartesian.c`,
`test_motion_planner.c`) and `RobotOS_v1.0/tests/CMakeLists.txt`
consume `include/robotos/` and `src/app/`; they belong to Architecture
B and are classified frozen by extension of Phase 12D-pre. Phase 12E
must not modify them.

### 12E-pre.5 Behavior coverage required before Phase 12E can close

25 contract items grouped into 21 runtime-asserted cases (init, dispatch
matched, first-match FIFO, guard reject, no match, action non-OK no
rollback, exit/state/action/entry order, reset, get_state, is_in_state,
get_snapshot, transition_count, event_count, last_event_id, payload
not cached, guard sees pre-transition state, action/entry see
post-transition state, exit sees pre-transition state, re-init policy)
and 4 review-driven cases (no heap, no UART, no
`robotos_core_register_event_handler` call, public-symbol surface
matches LOCKED-AT-12D). Full mapping to Phase 12B/12C decisions in
[`../02_PHASE_CLOSEOUTS/PHASE_12E_PRE_FSM_CONSUMER_TEST_PLAN.md`](../02_PHASE_CLOSEOUTS/PHASE_12E_PRE_FSM_CONSUMER_TEST_PLAN.md)
§F.

### 12E-pre.6 What is not changed

- All `.c` files in the repo -- zero-diff.
- `RobotOS_v1.0/framework/robotos_fw_fsm.h` -- zero-diff.
- `RobotOS_v1.0/framework/README.md` -- zero-diff.
- All `CMakeLists.txt` -- zero-diff.
- All tracked test files under `RobotOS_v1.0/tests/` -- zero-diff.
- `core/`, `platform/`, `devkit/src/`, `devkit/boards/`,
  `devkit/zephyr/`, `prj.conf`, DTS overlays -- zero-diff.
- `src/`, `include/robotos/`, `include/app/` -- zero-diff.
- Architecture B legacy notices -- zero-diff.
- All evidence logs -- zero-diff.
- All prior closeout docs -- not rewritten.
- `devkit_app_state` -- unchanged (scope-guard #11 re-affirmed).
- Validated command set `a / s / r / ? / x / v / L / d / T` -- unchanged.
- All 12 UART TX scope-guard constraints from
  [`../02_PHASE_CLOSEOUTS/PHASE_9EZ_CHECKPOINT.md`](../02_PHASE_CLOSEOUTS/PHASE_9EZ_CHECKPOINT.md)
  §H -- preserved.

### 12E-pre.7 Remaining decisions (preserved)

1. ACTIVE disarm widening -- `USER_DECISION_REQUIRED_ACTIVE_DISARM`.
2. Scheduler 7A/7B -- `DEFER`.
3. F407 / custom board -- `HOLD/DEFER`.
4. POST_FLASH_AUTOSTART root cause -- `OPEN` / `MITIGATED_BY_WORKFLOW`.
5. Application / product layer -- `NOT_STARTED`.
6. Robot Framework implementation -- `NOT_STARTED`; Phase 12E recommended
   path = host unit test; opening Phase 12E requires explicit user
   authorization.
7. Architecture A ↔ Architecture B reconciliation -- `NOT_STARTED`;
   out of scope.

### 12E-pre.8 Next gate

**Hold.** Phase 12E (Framework FSM host-test implementation) may open
only on **explicit user authorization**. The recommended scope, file
list, test cases, and exit criteria are recorded in
[`../02_PHASE_CLOSEOUTS/PHASE_12E_PRE_FSM_CONSUMER_TEST_PLAN.md`](../02_PHASE_CLOSEOUTS/PHASE_12E_PRE_FSM_CONSUMER_TEST_PLAN.md)
§E and §J. Phase 12E **remains `NOT_STARTED`**.

---

<a id="phase-12e"></a>
## Phase 12E -- Framework FSM Host-Test Implementation

**Status:** `CLOSED_WITH_HOST_TEST_EVIDENCE`
**Type:** First Framework implementation phase. **No devkit integration.
No UART command. No Zephyr config change. No hardware run. No legacy
Architecture B modification. No `core/`, `platform/`, `devkit/src/`,
`devkit/CMakeLists.txt`, root `RobotOS_v1.0/CMakeLists.txt`,
`framework/robotos_fw_fsm.h`, or `framework/README.md` change.**
**Date opened/closed:** 2026-05-12 (same-day host-test close)
**Published baseline at open:** `origin/master = a8019b5`
**Closeout doc:**
[`../02_PHASE_CLOSEOUTS/PHASE_12E_FSM_HOST_TEST_IMPLEMENTATION.md`](../02_PHASE_CLOSEOUTS/PHASE_12E_FSM_HOST_TEST_IMPLEMENTATION.md).
**New files:**
[`../../../framework/robotos_fw_fsm.c`](../../../framework/robotos_fw_fsm.c) (first Framework `.c` body),
[`../../../tests/host/test_robotos_fw_fsm.c`](../../../tests/host/test_robotos_fw_fsm.c) (host contract test, 20 cases / 93 assertions),
[`../../../tests/host/logs/phase_12E_host_2026-05-12.log`](../../../tests/host/logs/phase_12E_host_2026-05-12.log) (tracked test log).
**Updated:**
[`../../../tests/host/CMakeLists.txt`](../../../tests/host/CMakeLists.txt) (additive: 1 `add_executable` + 1 `add_test`; existing 20 targets untouched).
[`../03_SPECS/FRAMEWORK_FSM_API_DRAFT.md`](../03_SPECS/FRAMEWORK_FSM_API_DRAFT.md) (header revision marker advanced; §1 adds `IMPLEMENTED_AT_12E` rows; entry/exit non-OK choice documented).
**Companion docs:**
[`../02_PHASE_CLOSEOUTS/PHASE_12E_PRE_FSM_CONSUMER_TEST_PLAN.md`](../02_PHASE_CLOSEOUTS/PHASE_12E_PRE_FSM_CONSUMER_TEST_PLAN.md),
[`../02_PHASE_CLOSEOUTS/PHASE_12D_FSM_HEADER_STUB.md`](../02_PHASE_CLOSEOUTS/PHASE_12D_FSM_HEADER_STUB.md),
[`../02_PHASE_CLOSEOUTS/PHASE_12C_FSM_EVENT_BRIDGE_STATUS_MODEL.md`](../02_PHASE_CLOSEOUTS/PHASE_12C_FSM_EVENT_BRIDGE_STATUS_MODEL.md).

### 12E.1 Purpose

Phase 12E implements the six functions declared in the Phase 12D-LOCKED
header `robotos_fw_fsm.h` and validates them with host unit tests only.
This is the first Framework `.c` body in the repo and the first concrete
Framework consumer (a host test). The phase deliberately stops short of
devkit integration, Application bridge, UART command exposure, and
hardware evidence.

### 12E.2 Decision result

**`PHASE_12E_FSM_HOST_TEST_IMPLEMENTATION_CLOSED`** (`CLOSED_WITH_HOST_TEST_EVIDENCE`).

All six Phase 12D-LOCKED functions implemented in
`RobotOS_v1.0/framework/robotos_fw_fsm.c`. All 25 contract items from
the Phase 12E-pre plan satisfied: 21 `ASSERTED_BY_TEST`, 4
`REVIEW_VALIDATED`, 0 `BLOCKED`. Header zero-diff (LOCKED-AT-12D held;
no contract revision required).

### 12E.3 Host build / test result

- Environment: WSL Ubuntu 24.04, gcc 13.3.0, system cmake, Unix Makefiles.
- Local MSYS2 MinGW64 deliberately not used — Phase 12D documented it as
  unreliable; Phase 12E-pre §G mandated WSL/Linux for this phase.
- Configure: PASS.
- Build (FSM target only and full suite): PASS.
- Test (FSM contract, targeted): **PASS — 93 assertions, 0 failures, 20
  test cases**.
- Full host suite regression: **PASS — 21/21 tests (the new target plus
  the 20 pre-existing Phase 4-6 contract tests)**.
- Log: `RobotOS_v1.0/tests/host/logs/phase_12E_host_2026-05-12.log`
  (160-line ctest verbose transcript, tracked).

### 12E.4 Phase 12D syntax-check finding resolved

Phase 12D recorded `SYNTAX_CHECK_NOT_RUN_TOOLCHAIN_OUTPUT_SUPPRESSED`.
Phase 12E's WSL Ubuntu / gcc 13.3.0 build compiles the header and the
new `.c` cleanly; the Phase 12D open question is closed.

### 12E.5 What is not changed

- All files under `RobotOS_v1.0/core/`, `RobotOS_v1.0/platform/`,
  `RobotOS_v1.0/devkit/src/`, `RobotOS_v1.0/devkit/boards/`,
  `RobotOS_v1.0/devkit/zephyr/` — zero-diff.
- `RobotOS_v1.0/devkit/CMakeLists.txt`, root `RobotOS_v1.0/CMakeLists.txt` —
  zero-diff.
- `RobotOS_v1.0/framework/robotos_fw_fsm.h` (LOCKED-AT-12D held) — zero-diff.
- `RobotOS_v1.0/framework/README.md` — zero-diff.
- `RobotOS_v1.0/src/`, `RobotOS_v1.0/include/robotos/`,
  `RobotOS_v1.0/include/app/` — zero-diff (Architecture B frozen).
- `RobotOS_v1.0/tests/CMakeLists.txt` (Architecture B legacy test build),
  all tracked test sources under `RobotOS_v1.0/tests/test_*.c` and
  prior `RobotOS_v1.0/tests/host/test_*.c` — zero-diff.
- `prj.conf`, DTS files, board overlays, Zephyr workspace files — zero-diff.
- Host tools, runtime scripts, devkit evidence logs — zero-diff.
- All prior closeout docs — not rewritten.
- `devkit_app_state`: devkit-local; not promoted, not replaced, not
  copied (scope-guard #11 re-affirmed).
- Validated command set `a / s / r / ? / x / v / L / d / T` — unchanged.
- All 12 UART TX scope-guard constraints from
  [`../02_PHASE_CLOSEOUTS/PHASE_9EZ_CHECKPOINT.md`](../02_PHASE_CLOSEOUTS/PHASE_9EZ_CHECKPOINT.md)
  §H — preserved.

### 12E.6 Remaining decisions (all preserved)

1. ACTIVE disarm widening -- `USER_DECISION_REQUIRED_ACTIVE_DISARM`.
2. Scheduler 7A/7B -- `DEFER`.
3. F407 / custom board -- `HOLD/DEFER`.
4. POST_FLASH_AUTOSTART root cause -- `OPEN` / `MITIGATED_BY_WORKFLOW`.
5. Application / product layer -- `NOT_STARTED`.
6. Devkit integration of Framework FSM -- `NOT_STARTED`; gated by
   scope-guard #11 and Application/product `NOT_STARTED`.
7. Architecture A ↔ Architecture B reconciliation -- `NOT_STARTED`;
   out of scope.

### 12E.7 Next gate

**Hold.** Candidates for Phase 12F (see closeout §H.2):

- **Phase 12F-pre — Application bridge planning** (recommended when
  user is ready to move toward devkit integration).
- **Phase 12F — additional FSM host behavior** (lower priority; 25-item
  coverage from Phase 12E-pre is met).
- **Hold** (acceptable indefinitely; Framework FSM core is now complete
  at the host-test surface).

Devkit integration is **not** the next phase — that would collide with
scope-guard #11 and the Application-layer-NOT_STARTED constraint.

---

<a id="phase-12f-pre"></a>
## Phase 12F-pre -- Application Bridge Planning

**Status:** `CLOSED_DOCS_ONLY`
**Type:** Docs-only planning gate. **No source, runtime, test, CMake,
Zephyr, board, `prj.conf`, DTS overlay, evidence log, Framework header,
Framework `.c` file, devkit integration, command-set, or
`devkit_app_state` change.** No file under `framework/`, `tests/`,
`core/`, `platform/`, `devkit/src/`, `src/`, `include/robotos/`
modified.
**Date opened/closed:** 2026-05-12 (same-day docs-only close)
**Published baseline at open:** `origin/master = df9bb8e`
**Closeout doc:**
[`../02_PHASE_CLOSEOUTS/PHASE_12F_PRE_APPLICATION_BRIDGE_PLANNING.md`](../02_PHASE_CLOSEOUTS/PHASE_12F_PRE_APPLICATION_BRIDGE_PLANNING.md).
**New long-lived spec draft:**
[`../03_SPECS/FRAMEWORK_APPLICATION_BRIDGE_DRAFT.md`](../03_SPECS/FRAMEWORK_APPLICATION_BRIDGE_DRAFT.md)
(`DRAFT / NON-FINAL`; no implementation exists yet).
**Companion docs:**
[`../02_PHASE_CLOSEOUTS/PHASE_12E_FSM_HOST_TEST_IMPLEMENTATION.md`](../02_PHASE_CLOSEOUTS/PHASE_12E_FSM_HOST_TEST_IMPLEMENTATION.md),
[`../02_PHASE_CLOSEOUTS/PHASE_12E_PRE_FSM_CONSUMER_TEST_PLAN.md`](../02_PHASE_CLOSEOUTS/PHASE_12E_PRE_FSM_CONSUMER_TEST_PLAN.md),
[`../02_PHASE_CLOSEOUTS/PHASE_12C_FSM_EVENT_BRIDGE_STATUS_MODEL.md`](../02_PHASE_CLOSEOUTS/PHASE_12C_FSM_EVENT_BRIDGE_STATUS_MODEL.md).

### 12F-pre.1 Purpose

Phase 12F-pre selects the next implementation gate for connecting the
host-validated Framework FSM core (closed at Phase 12E) to a real
event source. The FSM exists at `RobotOS_v1.0/framework/robotos_fw_fsm.c`
and is host-test-validated, but it has **no consumer**. Phase 12F-pre
is the docs-only gate that decides how the first consumer/bridge
should be introduced without violating scope-guard #11
(`devkit_app_state`), the frozen command set `a/s/r/?/x/v/L/d/T`, or
the `NOT_STARTED` Application/product boundary.

### 12F-pre.2 Decision result

**`PHASE_12F_PRE_RECOMMEND_HOST_BRIDGE_PROTOTYPE`** (`CLOSED_DOCS_ONLY`).

Phase 12F, when authorized, should add a host-only bridge prototype
module under `RobotOS_v1.0/framework/` plus a single new host test
target inside the existing `RobotOS_v1.0/tests/host/CMakeLists.txt`.
No devkit code. No UART command. No `devkit_app_state` mutation. No
hardware run.

### 12F-pre.3 Candidate bridge paths evaluated

Five options were evaluated against safety, feasibility, boundary
impact, and risk:

1. **Host-only bridge prototype (RECOMMENDED).** Bridge module +
   host test; product-neutral; zero devkit drift; resolves Phase 12C
   `APPLICATION_OWNED_EVENT_BRIDGE_CONFIRMED` contract concretely.
2. **Devkit shadow bridge (REJECTED at Phase 12F).** Duplicate state
   truth risk; requires devkit producer file changes; requires
   hardware evidence; touches scope-guard #11 spirit.
3. **Devkit replacement bridge (REJECTED).** Directly violates
   scope-guard #11; `?` UART response shape can't reconcile with
   product-neutral FSM; never recommended without dedicated
   migration phase.
4. **Application-layer bridge skeleton (REJECTED at Phase 12F).**
   Application/product layer is `NOT_STARTED`; opening it inside an
   implementation phase prematurely commits the repo.
5. **Hold (FALLBACK ACCEPTABLE).** Leaves bridge contract untested;
   acceptable only if user defers further.

Full evaluation table in
[`../02_PHASE_CLOSEOUTS/PHASE_12F_PRE_APPLICATION_BRIDGE_PLANNING.md`](../02_PHASE_CLOSEOUTS/PHASE_12F_PRE_APPLICATION_BRIDGE_PLANNING.md)
§D.

### 12F-pre.4 Bridge semantics frozen at planning depth

Phase 12F-pre freezes the conceptual contract for the Phase 12F
host bridge prototype (full detail in
[`../03_SPECS/FRAMEWORK_APPLICATION_BRIDGE_DRAFT.md`](../03_SPECS/FRAMEWORK_APPLICATION_BRIDGE_DRAFT.md)):

- Bridge owns the mapping table (static const array, FIFO first-match);
  caller owns the FSM instance via pointer; caller owns mapping memory.
- Adapter key shape: `(uint32_t adapter_type, uint32_t adapter_arg0)`
  with optional `arg0` wildcard.
- Unmapped events → silent OK + `unmapped_count++`; FSM not called.
- Payload borrowed for dispatch duration only; bridge struct has no
  payload field.
- Status: reuse `robotos_core_status_t` via `robotos_fw_status_t`
  alias; no new public status enum.
- Thread context only; no ISR; no critical section needed.
- Forbidden surface: no UART TX; no GPIO/PWM/I2C/SPI drivers; no
  `robotos_core_register_event_handler`; no Zephyr / devkit / legacy
  `ro_*` includes; no heap; no `devkit_app_state` reference.

### 12F-pre.5 Relationship to `devkit_app_state`

`devkit_app_state` remains **authoritative** for the devkit runtime
state machine. Phase 12F-pre does not replace, promote, copy,
shadow, or duplicate it. The Phase 12F host bridge prototype lives
entirely in a separate translation unit exercised only by a host
test executable; no devkit producer file calls into it. Three future
devkit-integration modes (shadow, replacement, separate application)
each require their own dedicated future planning phase and explicit
user authorization. Scope-guard #11 is preserved unchanged.

### 12F-pre.6 Relationship to command set

`a / s / r / ? / x / v / L / d / T` remain the devkit/probe command
surface. The bridge adds **no UART command** and **no UART
exposure** of FSM state. Future command vocabulary belongs to the
Application/product phase, not the bridge prototype. All 12 UART TX
scope-guard constraints from
[`../02_PHASE_CLOSEOUTS/PHASE_9EZ_CHECKPOINT.md`](../02_PHASE_CLOSEOUTS/PHASE_9EZ_CHECKPOINT.md)
§H remain preserved.

### 12F-pre.7 Required behavior coverage for Phase 12F

12 contract cases mapped to bridge semantics: mapped dispatch,
unmapped silent ignore, payload pass-through, FSM non-OK
propagation, no payload caching, no UART/dispatcher coupling, mapping
determinism, multi-row support, host-only synthetic events, no
`devkit_app_state` reference, command-set zero-diff. Plus 5
review-validated items (no heap; no Zephyr/devkit/legacy includes; no
payload field on bridge struct; public symbol surface match; full host
suite regression). Full mapping in
[`../02_PHASE_CLOSEOUTS/PHASE_12F_PRE_APPLICATION_BRIDGE_PLANNING.md`](../02_PHASE_CLOSEOUTS/PHASE_12F_PRE_APPLICATION_BRIDGE_PLANNING.md)
§I.

### 12F-pre.8 What is not changed

- All `.c` and `.h` files in the repo -- zero-diff.
- `RobotOS_v1.0/framework/robotos_fw_fsm.h` -- zero-diff.
- `RobotOS_v1.0/framework/robotos_fw_fsm.c` -- zero-diff.
- `RobotOS_v1.0/framework/README.md` -- zero-diff.
- All `CMakeLists.txt` -- zero-diff.
- All tracked test files -- zero-diff.
- `core/`, `platform/`, `devkit/src/`, `devkit/boards/`,
  `devkit/zephyr/`, `prj.conf`, DTS overlays -- zero-diff.
- `src/`, `include/robotos/`, `include/app/` -- zero-diff.
- Architecture B legacy notices -- zero-diff.
- All evidence logs -- zero-diff.
- All prior closeout docs -- not rewritten.
- `devkit_app_state` -- unchanged (scope-guard #11 re-affirmed).
- Validated command set `a / s / r / ? / x / v / L / d / T` -- unchanged.

### 12F-pre.9 Remaining decisions (preserved)

1. ACTIVE disarm widening -- `USER_DECISION_REQUIRED_ACTIVE_DISARM`.
2. Scheduler 7A/7B -- `DEFER`.
3. F407 / custom board -- `HOLD/DEFER`.
4. POST_FLASH_AUTOSTART root cause -- `OPEN` / `MITIGATED_BY_WORKFLOW`.
5. Application / product layer -- `NOT_STARTED`.
6. Devkit integration of Framework FSM -- `NOT_STARTED`; Phase 12F
   recommended path = host bridge prototype only; opening Phase 12F
   requires explicit user authorization.
7. Architecture A ↔ Architecture B reconciliation -- `NOT_STARTED`.
8. Future devkit-integration mode (shadow / replacement / separate
   application) -- `UNDECIDED`; each mode requires its own future
   planning phase and explicit user authorization.

### 12F-pre.10 Next gate

**Hold.** Phase 12F (Framework Application Bridge Host Prototype)
may open only on **explicit user authorization**. The recommended
scope, file list, 12-item test coverage map, and exit criteria are
recorded in
[`../02_PHASE_CLOSEOUTS/PHASE_12F_PRE_APPLICATION_BRIDGE_PLANNING.md`](../02_PHASE_CLOSEOUTS/PHASE_12F_PRE_APPLICATION_BRIDGE_PLANNING.md)
§L and the bridge contract is recorded in
[`../03_SPECS/FRAMEWORK_APPLICATION_BRIDGE_DRAFT.md`](../03_SPECS/FRAMEWORK_APPLICATION_BRIDGE_DRAFT.md).
Phase 12F **remains `NOT_STARTED`**.

---

<a id="phase-20z"></a>
## Phase 20Z -- RESERVED / NOT STARTED

**Status:** `NOT_STARTED`

Placeholder only. Use this only for a future Phase 20 checkpoint /
closeout if approved, mirroring the existing `phase-10z` reserved
slot in `DEVKIT_PROGRESS_PHASE_10.md`. If Phase 20Z is opened, it
should:

- Be `CLOSED_DOCS_ONLY` (a checkpoint, not a runtime change), unless
  the user explicitly authorizes a runtime closeout.
- Carry forward the scope-guard and operational reminders from
  earlier checkpoints (Phase 9E-Z, Phase 10A, Phase 10C).
- Trigger creation of the **next** progress stream file
  (`DEVKIT_PROGRESS_PHASE_21_30.md`) only when Phase 21 is about to
  open.

This scaffold neither opens Phase 20Z nor pre-commits to its content.

---

## 6. What this scaffold does not do

- Does not open Phase 11.
- Does not authorize any source / test / CMake / Zephyr / board /
  `prj.conf` / host-tool / script change.
- Does not modify `DEVKIT_PROGRESS.md` (historical master) or
  `DEVKIT_PROGRESS_PHASE_10.md` (Phase 10 stream).
- Does not change command semantics (`a / s / r / ? / x / v / L / d`
  remain as closed at Phase 10C).
- Does not widen ACTIVE disarm; `USER_DECISION_REQUIRED_ACTIVE_DISARM`
  is preserved.
- Does not open `T` sensor read; `USER_DECISION_REQUIRED` is
  preserved.
- Does not reopen Scheduler 7A/7B or F407 / custom-board work.
- Does not change UART TX scope; all 12 scope-guard constraints
  remain intact.
- Does not change POST_FLASH_AUTOSTART status (root cause `OPEN`;
  `MITIGATED_BY_WORKFLOW` from Phase 6O onward).
- Does not push.
