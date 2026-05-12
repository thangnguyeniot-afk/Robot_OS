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
