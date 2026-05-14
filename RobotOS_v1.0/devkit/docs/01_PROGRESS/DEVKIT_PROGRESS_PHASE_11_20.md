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
| 12F | Framework Application Bridge Host Prototype | CLOSED_WITH_HOST_TEST_EVIDENCE | [->](#phase-12f) |
| 12G-pre | Devkit Integration Mode Decision (docs-only) | CLOSED_DOCS_ONLY | [->](#phase-12g-pre) |
| 12G | Separate Application Mode / Application Boundary Planning (docs-only) | CLOSED_DOCS_ONLY | [->](#phase-12g) |
| 12H-pre | First Application Candidate / Product Harness Selection (docs-only) | CLOSED_DOCS_ONLY | [->](#phase-12h-pre) |
| 12H | Probe Translator App Skeleton Planning (docs-only, Variant 1) | CLOSED_DOCS_ONLY | [->](#phase-12h) |
| 12I-pre | Probe Translator Host Prototype Implementation Plan (docs-only) | CLOSED_DOCS_ONLY | [->](#phase-12i-pre) |
| 12I | Probe Translator Host Prototype | CLOSED_WITH_HOST_TEST_EVIDENCE | [->](#phase-12i) |
| 12J-pre | Probe Translator FAULT Block Plan (docs-only) | CLOSED_DOCS_ONLY | [->](#phase-12j-pre) |
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
Phase 12F **remains `NOT_STARTED`** at the close of Phase 12F-pre.

---

<a id="phase-12f"></a>
## Phase 12F -- Framework Application Bridge Host Prototype

**Status:** `CLOSED_WITH_HOST_TEST_EVIDENCE`
**Decision:** `PHASE_12F_APPLICATION_BRIDGE_HOST_PROTOTYPE_CLOSED`
**Closeout:** [`../02_PHASE_CLOSEOUTS/PHASE_12F_APPLICATION_BRIDGE_HOST_PROTOTYPE.md`](../02_PHASE_CLOSEOUTS/PHASE_12F_APPLICATION_BRIDGE_HOST_PROTOTYPE.md)
**Spec:** [`../03_SPECS/FRAMEWORK_APPLICATION_BRIDGE_DRAFT.md`](../03_SPECS/FRAMEWORK_APPLICATION_BRIDGE_DRAFT.md) (§5 now `LOCKED-AT-12F`)
**Date:** 2026-05-13

### 12F.1 Outcome

Phase 12F implemented the Framework Application Event Bridge as a
host-only prototype along the path Phase 12F-pre recommended. The
bridge is a product-neutral mapping engine that translates Adapter
event keys `(uint32_t adapter_type, uint32_t adapter_arg0)` into
`robotos_fw_event_id_t` logical events and forwards them to a
caller-owned `robotos_fw_fsm_t` via `robotos_fw_fsm_dispatch()`.

### 12F.2 New / modified surfaces

- **New (3 source files):**
  - `RobotOS_v1.0/framework/robotos_fw_event_bridge.h` -- header.
  - `RobotOS_v1.0/framework/robotos_fw_event_bridge.c` -- body.
  - `RobotOS_v1.0/tests/host/test_robotos_fw_event_bridge.c` -- host
    contract test.
- **Modified (additive only):**
  - `RobotOS_v1.0/tests/host/CMakeLists.txt` -- one new
    `add_executable` + `add_test` block for
    `robotos_fw_event_bridge_contract_test`. No existing target
    altered.
- **Zero-diff held:**
  - `RobotOS_v1.0/framework/robotos_fw_fsm.h` (Phase 12D
    LOCKED-AT-12D),
  - `RobotOS_v1.0/framework/robotos_fw_fsm.c` (Phase 12E),
  - `RobotOS_v1.0/framework/README.md`,
  - `core/`, `platform/`, `devkit/src/`, `devkit/CMakeLists.txt`,
  - root `RobotOS_v1.0/CMakeLists.txt` (Architecture B frozen),
  - all `prj.conf`, board DTS, overlay files.

### 12F.3 Public API (LOCKED-AT-12F for names + signatures)

```c
robotos_fw_status_t robotos_fw_event_bridge_init(
    robotos_fw_event_bridge_t              *bridge,
    const robotos_fw_event_bridge_config_t *config);
robotos_fw_status_t robotos_fw_event_bridge_dispatch(
    robotos_fw_event_bridge_t *bridge,
    uint32_t                   adapter_type,
    uint32_t                   adapter_arg0,
    const void                *payload);
robotos_fw_status_t robotos_fw_event_bridge_reset(
    robotos_fw_event_bridge_t *bridge);
robotos_fw_status_t robotos_fw_event_bridge_get_snapshot(
    const robotos_fw_event_bridge_t    *bridge,
    robotos_fw_event_bridge_snapshot_t *out);
```

Memory layout and behavioral guarantees beyond these signatures are
still `DRAFT / EXPERIMENTAL`.

### 12F.4 Behavior locked at Phase 12F

- **Mapping:** FIFO first-match. Row order is the only precedence
  rule -- a wildcard row (`match_arg0 == false`) shadows any
  later exact-match row for the same type.
- **Wildcard:** `match_arg0 == false` matches any `arg0` value for a
  given `adapter_type`.
- **Unmapped events:** silent OK + `unmapped_count++`; FSM is **not**
  called.
- **Payload:** borrowed `const void *` forwarded verbatim to
  `robotos_fw_fsm_dispatch()`; bridge struct has no payload field.
- **Status mapping:** reuses `robotos_core_status_t`. FSM non-OK
  return propagates verbatim. No new enum.
- **Reset:** zeroes bridge counters only; FSM state preserved.
- **Re-init:** idempotent; FSM not touched.
- **Threading:** thread context only. No critical section taken by
  the bridge. No ISR support in Phase 12F.
- **Counter invariant:**
  `event_count == mapped_count + unmapped_count`.

### 12F.5 Host evidence

- **Environment:** WSL Ubuntu / gcc 13.3.0 / Unix Makefiles
  (Phase 12E precedent). MSYS2 MinGW64 not used.
- **Bridge direct run:** 17 cases / 103 assertions PASS, 0 FAIL.
- **Full host regression:** 22/22 PASS (Phase 12E baseline 21/21 +
  one new bridge ctest target).
- **Tracked log:**
  [`../../tests/host/logs/phase_12F_host_2026-05-13.log`](../../tests/host/logs/phase_12F_host_2026-05-13.log).

### 12F.6 Coverage of Phase 12F-pre required behaviors

| # | Required behavior | Verdict |
|---|---|---|
| 1-5, 7, 9-15 | Init validity, init NULL/invalid, mapped dispatch, unmapped ignored, payload borrowed, non-OK propagation, FIFO determinism, exact arg0, wildcard arg0, FIFO-beats-specificity, multi-mapping, bridge reset, snapshot, re-init | `ASSERTED_BY_TEST` (14 of 18 host-asserted) |
| 6 | Bridge does not retain payload pointer | `ASSERTED_BY_TEST` (structural; TC08) |
| 8, 16, 17, 18 | No UART/hardware/core registration; no `devkit_app_state` modification; no command semantic change; no heap | `REVIEW_VALIDATED` (no UART/hardware/registration/`devkit_app_state`/`malloc` symbol in any bridge file) |

See the Phase 12F closeout §E for the full 18-item matrix.

### 12F.7 Scope guards held

- No devkit integration.
- No UART command added; `a/s/r/?/x/v/L/d/T` unchanged.
- No hardware run.
- No `devkit_app_state` change. Scope-guard #11 re-affirmed.
- No legacy Architecture B file changed.
- No root or devkit CMake change.
- No `core/` / `platform/` mutation.
- No Zephyr / `prj.conf` / board change.
- No heap allocation in bridge.
- No Zephyr / devkit / legacy `ro_*` include in bridge.
- ACTIVE disarm `USER_DECISION_REQUIRED_ACTIVE_DISARM` preserved.
- Scheduler 7A/7B `DEFER` preserved.
- F407 / custom board `HOLD/DEFER` preserved.
- POST_FLASH_AUTOSTART `OPEN/MITIGATED_BY_WORKFLOW` preserved.
- Application / product layer `NOT_STARTED` preserved.

### 12F.8 Open items deferred from Phase 12F-pre §11

- Open #4 (arg1 / hash matching) -- deferred to a later phase if a
  use case appears.
- Open #5 (fan-out single bridge -> multiple FSMs) -- deferred to
  Phase 12G or later.
- Open #7 (unmapped-event diagnostic hook) -- deferred to Phase 12G
  or later.

All other open decisions from Phase 12F-pre §11 are resolved at
Phase 12F. See the bridge spec §11 for the full resolution table.

### 12F.9 What remains NOT_STARTED

1. Devkit integration of the Framework FSM + bridge --
   `NOT_STARTED`. Three modes (shadow / replacement / separate
   application) enumerated in Phase 12F-pre §G; each requires its
   own future planning gate.
2. Application / product layer -- `NOT_STARTED`.
3. Hardware evidence for the Framework path -- `NOT_STARTED`. Phase
   12F adds zero hardware runs. The Framework FSM + bridge have
   host-test evidence only.
4. Bridge ABI memory-layout lock -- `NOT_STARTED`. Only names and
   signatures are locked.

### 12F.10 Next gate

**Hold or open a docs-only Phase 12G-pre devkit-integration-mode
decision.** Do not open direct devkit integration without an
explicit planning gate that chooses among shadow / replacement /
separate application modes. A second viable next gate is additional
host-only bridge behavior (e.g., `arg1` matching, unmapped-event
diagnostic hook). Neither was authorized at the close of Phase 12F.

---

<a id="phase-12g-pre"></a>
## Phase 12G-pre -- Devkit Integration Mode Decision (docs-only)

**Status:** `CLOSED_DOCS_ONLY`
**Decision:** `PHASE_12G_PRE_RECOMMEND_SEPARATE_APPLICATION_MODE_PLANNING`
**Closeout:** [`../02_PHASE_CLOSEOUTS/PHASE_12G_PRE_DEVKIT_INTEGRATION_MODE_DECISION.md`](../02_PHASE_CLOSEOUTS/PHASE_12G_PRE_DEVKIT_INTEGRATION_MODE_DECISION.md)
**New long-lived spec:** [`../03_SPECS/FRAMEWORK_DEVKIT_INTEGRATION_MODE_DRAFT.md`](../03_SPECS/FRAMEWORK_DEVKIT_INTEGRATION_MODE_DRAFT.md)
**Date:** 2026-05-13

### 12G-pre.1 Purpose

Phase 12G-pre is a docs-only architecture planning gate that
selects the next direction for connecting the now-host-tested
Framework path (FSM + Application Event Bridge) to the devkit
firmware. Phase 12G-pre does not implement integration, does not
modify Framework code, does not modify devkit runtime, does not
modify `devkit_app_state`, does not change command semantics, and
does not run hardware.

### 12G-pre.2 Files added / modified

- **New (2 docs):**
  - `RobotOS_v1.0/devkit/docs/02_PHASE_CLOSEOUTS/PHASE_12G_PRE_DEVKIT_INTEGRATION_MODE_DECISION.md`
    -- decision closeout (sections A-M).
  - `RobotOS_v1.0/devkit/docs/03_SPECS/FRAMEWORK_DEVKIT_INTEGRATION_MODE_DRAFT.md`
    -- new long-lived integration-mode spec (`DRAFT / NON-FINAL`,
    sections 1-10).
- **Modified:**
  - `RobotOS_v1.0/devkit/docs/01_PROGRESS/DEVKIT_PROGRESS_PHASE_11_20.md`
    -- this entry + index row.
  - `CURRENT_STATE.md` -- Phase 12G-pre as latest closed.
  - `RobotOS_v1.0/devkit/docs/00_INDEX/README.md` -- Phase 12G-pre
    closeout link + integration-mode spec link.
  - `RobotOS_v1.0/devkit/docs/03_SPECS/FRAMEWORK_APPLICATION_BRIDGE_DRAFT.md`
    -- one-line cross-reference to the integration-mode spec.
- **Zero-diff held:**
  - All `.c` and `.h` files under `framework/`, `core/`, `platform/`,
    `devkit/src/`, `tests/`, `src/`, `include/robotos/`.
  - All `CMakeLists.txt`.
  - All `prj.conf`, board DTS, overlay, Zephyr config files.
  - All existing evidence logs.

### 12G-pre.3 Integration modes evaluated

Five candidate modes evaluated in detail in the closeout §D:

| # | Mode | Touches `devkit_app_state` | Changes command set | Hardware required | Recommendation at 12G-pre |
|---|---|---|---|---|---|
| 1 | HOLD | No | No | No | Acceptable fallback |
| 2 | SHADOW | Read-only | No | Yes | Defer until after Mode 4 decision |
| 3 | REPLACEMENT | Yes (rewrite) | Yes | Yes | Not recommended; structurally forbidden until ACTIVE disarm + Application planning resolved |
| 4 | SEPARATE APPLICATION | No | No | Eventually | **RECOMMENDED** |
| 5 | HOST-ONLY EXTENSION | No | No | No | Lower priority than Mode 4 |

### 12G-pre.4 Decision result

**`PHASE_12G_PRE_RECOMMEND_SEPARATE_APPLICATION_MODE_PLANNING`.**

Rationale (full text in closeout §E):

- Framework should not replace `devkit_app_state`; scope-guard #11
  preserved.
- Devkit command set is validation / probe surface, not product
  vocabulary.
- Application / product layer is `NOT_STARTED`; next clean step is
  to plan where the application lives before building any
  application.
- Shadow mode is useful, but premature without an Application
  decision -- a single drift in shadow log labelling could let the
  shadow be read as authoritative.
- Replacement mode is structurally forbidden until ACTIVE disarm,
  Application planning, and a dedicated migration phase are
  resolved.
- HOLD is acceptable if user wants to stop, but inferior to Mode 4
  because Mode 4 also preserves the shelf while adding direction.
- Additional host-only work is acceptable as parallel track but
  lower-priority; without a consumer it risks shaping the bridge
  for hypothetical needs.

### 12G-pre.5 Frozen planning direction (Mode 4)

1. `devkit_app_state` remains authoritative for devkit validation.
2. Framework FSM + Application Event Bridge intended to be consumed
   by a future separate application / product harness.
3. Future application layer lives at a separate path (proposal
   anchor: `RobotOS_v1.0/app/<product>/`; exact shape locks at
   Phase 12G).
4. Future application layer owns its mapping table, FSM instance,
   bridge instance, and product vocabulary.
5. No current UART command semantics change.
6. No `?` response change.
7. No `devkit_app_state` replacement, copy, or promotion.
8. No hardware run until the future application phase produces a
   board-runnable target.
9. Build separation strategy (separate Zephyr app dir vs. shared
   base + `CONFIG_*` selectable vs. board overlay) is a Phase 12G
   decision.

### 12G-pre.6 Relationship to `devkit_app_state`

`devkit_app_state` is **unchanged** by Phase 12G-pre. Any future
mode that touches `devkit_app_state` must list, before opening:

- migration risk (which Phase 9 / 10 / 11 evidence is invalidated);
- rollback plan;
- which hardware probes must rerun;
- which of `a/s/r/?/x/v/L/d/T` change semantics;
- relationship to ACTIVE disarm `USER_DECISION_REQUIRED` and
  POST_FLASH_AUTOSTART `OPEN`.

Scope-guard #11 remains active through Phase 12G-pre.

### 12G-pre.7 Relationship to command set

`a / s / r / ? / x / v / L / d / T` remain unchanged. No
integration mode evaluated in §D automatically exposes a new UART
command. The future application layer, if it grows a command
vocabulary, defines that vocabulary in the application path -- not
in `devkit/src/`. All 12 UART TX scope-guard constraints from
Phase 9EZ §H preserved.

### 12G-pre.8 What remains NOT_STARTED

1. Integration implementation -- `NOT_STARTED`.
2. Application / product layer -- `NOT_STARTED`.
3. Phase 12G (Separate Application Mode Planning) -- `NOT_STARTED`;
   requires explicit user authorization.
4. Devkit hardware integration of Framework -- `NOT_STARTED`.
5. Bridge ABI memory-layout lock -- `NOT_STARTED`.

### 12G-pre.9 Open gates preserved unchanged

- ACTIVE disarm widening -- `USER_DECISION_REQUIRED_ACTIVE_DISARM`.
- Scheduler 7A/7B -- `DEFER`.
- F407 / custom board -- `HOLD/DEFER`.
- POST_FLASH_AUTOSTART -- `OPEN/MITIGATED_BY_WORKFLOW`.
- Application / product layer -- `NOT_STARTED`.
- Architecture A ↔ Architecture B reconciliation -- `NOT_STARTED`.
- Future devkit-integration mode -- recommended SEPARATE APPLICATION;
  shadow / replacement / hold remain fallback directions if user
  overrides the recommendation.

### 12G-pre.10 Next gate

**Hold or open Phase 12G -- Separate Application Mode / Application
Boundary Planning** (docs-only architecture planning) only on
**explicit user authorization**. The recommended scope, in-scope /
non-goal list, and exit criteria are recorded in
[`../02_PHASE_CLOSEOUTS/PHASE_12G_PRE_DEVKIT_INTEGRATION_MODE_DECISION.md`](../02_PHASE_CLOSEOUTS/PHASE_12G_PRE_DEVKIT_INTEGRATION_MODE_DECISION.md)
§L. Phase 12G **was opened** after this gate and is recorded in
the next section.

---

<a id="phase-12g"></a>
## Phase 12G -- Separate Application Mode / Application Boundary Planning (docs-only)

**Status:** `CLOSED_DOCS_ONLY`
**Decision:** `PHASE_12G_RECOMMEND_APP_PRODUCT_PATH`
**Closeout:** [`../02_PHASE_CLOSEOUTS/PHASE_12G_SEPARATE_APPLICATION_BOUNDARY_PLANNING.md`](../02_PHASE_CLOSEOUTS/PHASE_12G_SEPARATE_APPLICATION_BOUNDARY_PLANNING.md)
**New long-lived spec:** [`../03_SPECS/FRAMEWORK_APPLICATION_BOUNDARY_DRAFT.md`](../03_SPECS/FRAMEWORK_APPLICATION_BOUNDARY_DRAFT.md)
**Date:** 2026-05-13

### 12G.1 Purpose

Phase 12G is the docs-only application boundary planning gate
recommended by Phase 12G-pre. It defines where future application
code lives, what it owns, how it consumes the Framework path, and
the staged validation strategy that precedes any hardware run.
Phase 12G does not create source, does not modify Framework code,
does not modify devkit runtime, does not modify `devkit_app_state`,
does not change command semantics, does not run hardware, and does
not create the `app/` directory.

### 12G.2 Files added / modified

- **New (2 docs):**
  - `RobotOS_v1.0/devkit/docs/02_PHASE_CLOSEOUTS/PHASE_12G_SEPARATE_APPLICATION_BOUNDARY_PLANNING.md`
    -- decision closeout (sections A-P).
  - `RobotOS_v1.0/devkit/docs/03_SPECS/FRAMEWORK_APPLICATION_BOUNDARY_DRAFT.md`
    -- new long-lived application boundary spec
    (`DRAFT / NON-FINAL`, sections 1-12).
- **Modified:**
  - `RobotOS_v1.0/devkit/docs/01_PROGRESS/DEVKIT_PROGRESS_PHASE_11_20.md`
    -- this entry + index row.
  - `CURRENT_STATE.md` -- Phase 12G as latest closed.
  - `RobotOS_v1.0/devkit/docs/00_INDEX/README.md` -- Phase 12G
    closeout link + application boundary spec link.
  - `RobotOS_v1.0/devkit/docs/03_SPECS/FRAMEWORK_DEVKIT_INTEGRATION_MODE_DRAFT.md`
    -- short cross-reference to the application boundary spec.
  - `RobotOS_v1.0/devkit/docs/03_SPECS/FRAMEWORK_APPLICATION_BRIDGE_DRAFT.md`
    -- short cross-reference to the application boundary spec.
- **Zero-diff held:**
  - All `.c` and `.h` files under `framework/`, `core/`,
    `platform/`, `devkit/src/`, `tests/`, `src/`,
    `include/robotos/`.
  - All `CMakeLists.txt`.
  - All `prj.conf`, board DTS, overlay, Zephyr config files.
  - All existing evidence logs.
  - **No `app/` or `application/` directory created.**

### 12G.3 Candidate directory shapes evaluated

Five candidate shapes evaluated in detail in the closeout §D:

| # | Option | Adjacent to | Risk | Scalable | Recommendation |
|---|---|---|---|---|---|
| 1 | `RobotOS_v1.0/app/<product>/` | Framework siblings | Low | Yes | **RECOMMENDED** |
| 2 | `RobotOS_v1.0/application/<product>/` | Framework siblings | Low | Yes | Rejected (ergonomics; no repo precedent) |
| 3 | `RobotOS_v1.0/devkit/app/` | Devkit harness | High | No | Rejected (blends validation + application) |
| 4 | `RobotOS_v1.0/framework/app/` | Framework internals | High | No | Rejected (violates Framework product-neutral boundary) |
| 5 | `RobotOS_v1.0/examples/<scenario>/` | Top-level sibling | Low | Sample-only | Useful later; not the product / application path |

### 12G.4 Decision result

**`PHASE_12G_RECOMMEND_APP_PRODUCT_PATH`.** Future active
application code lives under `RobotOS_v1.0/app/<product>/`. The
directory is reserved at planning depth and **not created** by
Phase 12G. The first `<product>` placeholder is selected by a
future Phase 12H-pre.

### 12G.5 App layer responsibilities (planning-level)

**May own:** product / application state machine composition (FSM
instance + transition table + state defs + product vocabulary);
mapping table (`robotos_fw_event_bridge_row_t[]`); product event
IDs; product command vocabulary (separate channel, separate
framing); product-specific sensor / actuator policy; product-
specific build / harness integration; product-level validation
scripts and docs.

**Must not own:** core queue / dispatcher internals; platform
backend primitives; devkit validation command semantics; Framework
generic FSM / bridge algorithms; legacy Architecture B; any
`devkit_app_state` read or write.

### 12G.6 Event mapping policy

Application owns the mapping table; Framework bridge stays
product-neutral (`LOCKED-AT-12F`); product event IDs are
application-local; no new `ROBOTOS_EVENT_USER` subrange required;
multiple applications can coexist with private event ID
namespaces; mapping is host-tested before any runtime / hardware
run.

### 12G.7 Build separation strategy

No build change in Phase 12G. Future `app/<product>/` builds
attach to Architecture A only; do not touch the root legacy
CMake; are separable from the devkit validation build; prefer
host-first tests. Three future CMake options (Option A: host test
target in existing `tests/host/CMakeLists.txt`; Option B: new
`app/<product>/CMakeLists.txt`; Option C: devkit integration NOT
authorized) decided at the relevant implementation phase, not at
Phase 12G.

### 12G.8 Validation strategy (staged; Phase 12G closes Stage 1 only)

| Stage | Gate | Status |
|---|---|---|
| 1 | Docs-only application boundary plan | **CLOSED at Phase 12G** |
| 2 | First product selection (Phase 12H-pre, docs-only) | NOT_STARTED |
| 3 | Host-only application mapping prototype | NOT_STARTED |
| 4 | Host regression baseline preserved (22/22 still PASS) | NOT_STARTED |
| 5 | Optional devkit shadow (only if user explicitly authorizes Mode 2) | NOT_AUTHORIZED |
| 6 | Hardware evidence (requires explicit runtime-integration phase) | NOT_AUTHORIZED |

### 12G.9 Relationship to `devkit_app_state`, command set, legacy Arch B

- `devkit_app_state` remains authoritative for current devkit
  runtime. Separate Application Mode does **not** replace, shadow,
  copy, or promote it. Scope-guard #11 re-affirmed.
- Command set `a/s/r/?/x/v/L/d/T` unchanged. Framework is not
  exposed via UART automatically. Future application command
  vocabulary requires a separate product-command phase.
- Architecture B (`src/`, `include/robotos/`) remains frozen at
  `LEGACY_SCAFFOLD_MARKED_FROZEN_DOCS_ONLY` (Phase 12D-pre).
  Future `app/<product>/` does not reuse Architecture B and uses
  Architecture A contracts only.

### 12G.10 What remains NOT_STARTED

1. Application implementation -- `NOT_STARTED`.
2. `app/` directory creation -- `NOT_CREATED`.
3. First `<product>` placeholder name -- open for Phase 12H-pre.
4. Phase 12H-pre -- `NOT_STARTED`; requires explicit user
   authorization.
5. Devkit hardware integration of Framework -- `NOT_STARTED`.
6. Bridge ABI memory-layout lock -- `NOT_STARTED`.

### 12G.11 Open gates preserved unchanged

- ACTIVE disarm widening -- `USER_DECISION_REQUIRED_ACTIVE_DISARM`.
- Scheduler 7A/7B -- `DEFER`.
- F407 / custom board -- `HOLD/DEFER`.
- POST_FLASH_AUTOSTART -- `OPEN/MITIGATED_BY_WORKFLOW`.
- Application / product layer -- `NOT_STARTED`; recommended next
  gate is Phase 12H-pre.
- Devkit integration of Framework -- `NOT_STARTED`.

### 12G.12 Next gate

**Hold or open Phase 12H-pre -- First Application Candidate /
Product Harness Selection** (docs-only) only on **explicit user
authorization**. The recommended scope, in-scope / non-goal list,
and exit criteria are recorded in
[`../02_PHASE_CLOSEOUTS/PHASE_12G_SEPARATE_APPLICATION_BOUNDARY_PLANNING.md`](../02_PHASE_CLOSEOUTS/PHASE_12G_SEPARATE_APPLICATION_BOUNDARY_PLANNING.md)
§O. Phase 12H-pre **was opened** after this gate and is recorded in
the entry below.

---

<a id="phase-12h-pre"></a>
## Phase 12H-pre -- First Application Candidate / Product Harness Selection (docs-only)

**Status:** `CLOSED_DOCS_ONLY`
**Decision:** `PHASE_12H_PRE_RECOMMEND_PROBE_TRANSLATOR_APP`
**Closeout:** [`../02_PHASE_CLOSEOUTS/PHASE_12H_PRE_FIRST_APPLICATION_CANDIDATE_SELECTION.md`](../02_PHASE_CLOSEOUTS/PHASE_12H_PRE_FIRST_APPLICATION_CANDIDATE_SELECTION.md)
**New long-lived spec:** [`../03_SPECS/FIRST_APPLICATION_CANDIDATE_DRAFT.md`](../03_SPECS/FIRST_APPLICATION_CANDIDATE_DRAFT.md)
**Date:** 2026-05-13

### 12H-pre.1 Purpose

Phase 12H-pre is the docs-only product / application planning gate
that selects the first application candidate / product harness for
the future `RobotOS_v1.0/app/<product>/` boundary reserved by Phase
12G. It freezes the planning-level answer to "which first
`<product>` placeholder, what kind of harness, what minimal state
and event vocabulary, and what host-first validation plan?" before
any `app/` directory or application source exists.

Phase 12H-pre does not create source, does not modify Framework
code, does not modify devkit runtime, does not modify
`devkit_app_state`, does not change command semantics, does not run
hardware, does not create the `app/` directory, and does not open
Phase 12H or any implementation gate.

### 12H-pre.2 Files added / modified

- **New (2 docs):**
  - `RobotOS_v1.0/devkit/docs/02_PHASE_CLOSEOUTS/PHASE_12H_PRE_FIRST_APPLICATION_CANDIDATE_SELECTION.md`
    -- decision closeout (sections A-Q).
  - `RobotOS_v1.0/devkit/docs/03_SPECS/FIRST_APPLICATION_CANDIDATE_DRAFT.md`
    -- new long-lived first-application-candidate spec
    (`DRAFT / NON-FINAL`, sections 1-14).
- **Modified:**
  - `RobotOS_v1.0/devkit/docs/01_PROGRESS/DEVKIT_PROGRESS_PHASE_11_20.md`
    -- this entry + index row.
  - `CURRENT_STATE.md` -- Phase 12H-pre as latest closed.
  - `RobotOS_v1.0/devkit/docs/00_INDEX/README.md` -- Phase 12H-pre
    closeout link + first-application-candidate spec link.
  - `RobotOS_v1.0/devkit/docs/03_SPECS/FRAMEWORK_APPLICATION_BOUNDARY_DRAFT.md`
    -- short cross-reference to the first-application-candidate
    spec.
- **Zero-diff held:**
  - All `.c` and `.h` files under `framework/`, `core/`,
    `platform/`, `devkit/src/`, `tests/`, `src/`,
    `include/robotos/`.
  - All `CMakeLists.txt`.
  - All `prj.conf`, board DTS, overlay, Zephyr config files.
  - All existing evidence logs.
  - **No `app/` or `application/` directory created.**
  - **No `app/probe_translator/` directory created.**

### 12H-pre.3 Candidate first-app shapes evaluated

Five candidates evaluated in the closeout §D:

| # | Candidate | Product commitment | Validation strategy | Risk | Recommendation |
|---|---|---|---|---|---|
| 1 | `app/probe_translator/` | Minimal (neutral) | Host-first; clean | Low | **RECOMMENDED** |
| 2 | `app/demo/` | Vague | Host-first; weak acceptance | Medium | Rejected (scope sink; reserved for future `examples/`) |
| 3 | `app/devkit_shadow/` | Low | Cannot be host-first cleanly | High | Rejected (overrides Phase 12G-pre Mode 2 DEFER) |
| 4 | `app/<real_product>/` | High | Needs product acceptance criteria | High | Rejected (premature; pending product direction) |
| 5 | HOLD | None | None | Low short / Medium long | Acceptable fallback |

### 12H-pre.4 Decision result

**`PHASE_12H_PRE_RECOMMEND_PROBE_TRANSLATOR_APP`.** The first
`<product>` placeholder under `RobotOS_v1.0/app/<product>/` is
selected as `probe_translator`. Future first application harness
lives at `RobotOS_v1.0/app/probe_translator/`. The harness is
host-first, product-neutral, and synthetic-event-driven. The
directory is reserved at planning depth and **not created** by
Phase 12H-pre.

### 12H-pre.5 Minimal state vocabulary (planning-level)

| State | Meaning |
|---|---|
| `APP_IDLE` | Default after FSM init / `APP_EVT_RESET`. |
| `APP_READY` | After `APP_EVT_CONFIGURED`. |
| `APP_ACTIVE` | After `APP_EVT_START`. |
| `APP_FAULT` (optional) | After `APP_EVT_FAULT`. May be deferred. |

All names are application-local. Name overlap with
`DEVKIT_APP_STATE_IDLE / ARMED / ACTIVE` is coincidental at the
human-readable level only; `devkit_app_state` is **not** consumed
by the application.

### 12H-pre.6 Minimal event vocabulary (planning-level)

`APP_EVT_CONFIGURED / APP_EVT_START / APP_EVT_STOP / APP_EVT_FAULT
(optional) / APP_EVT_RESET`. All synthetic at first. None map from a
real core event, UART byte, button, or hardware producer. No new
`ROBOTOS_EVENT_USER` subrange required.

### 12H-pre.7 Minimal bridge mapping table (planning-level)

| Row | `adapter_type` | `adapter_arg0` | `match_arg0` | `fw_event_id` |
|---|---|---|---|---|
| 0 | `ADAPTER_EVT_CONFIG` | `0` | `true` | `APP_EVT_CONFIGURED` |
| 1 | `ADAPTER_EVT_CONFIG` | `1` (RESET) | `true` | `APP_EVT_RESET` |
| 2 | `ADAPTER_EVT_COMMAND` | `0` (START) | `true` | `APP_EVT_START` |
| 3 | `ADAPTER_EVT_COMMAND` | `1` (STOP) | `true` | `APP_EVT_STOP` |
| 4 | `ADAPTER_EVT_FAULT` | any | `false` (wildcard) | `APP_EVT_FAULT` |

Planning-only; no source exists. `ADAPTER_EVT_*` tags are
application-local numeric constants, not core allocations. FIFO row
order matters (Phase 12F §5.3); specific rows ahead of wildcard
rows.

### 12H-pre.8 Host mapping test plan (planning-level)

Future host test
`tests/host/test_app_probe_translator_mapping.c` (when authorized)
must cover at minimum: row-by-row mapping, wildcard precedence,
unmapped-event accounting, full `IDLE -> READY -> ACTIVE -> READY
-> IDLE` path, re-init idempotence, bridge reset policy, grep gates
forbidding `devkit_app_state.h` / `a/s/r/?/x/v/L/d/T` references /
Zephyr includes, and host regression preserved at >=22/22.

### 12H-pre.9 Build strategy (preferred)

**Option A** -- additive entry in existing
`tests/host/CMakeLists.txt` (host test target). Same WSL Ubuntu /
gcc 13.3.0 environment as Phase 12E / 12F. Option B (new
`app/probe_translator/CMakeLists.txt`) acceptable but not preferred
at first implementation. Option C (devkit integration) **not
authorized**.

### 12H-pre.10 Boundary preservation

- `devkit_app_state` remains authoritative; not modified, not
  consumed by `app/probe_translator/`. Scope-guard #11 re-affirmed.
- Command set `a/s/r/?/x/v/L/d/T` unchanged; no UART command
  added by Phase 12H-pre or by the future first application.
- Architecture B (`src/`, `include/robotos/`) frozen; future
  `app/probe_translator/` uses Architecture A contracts only.

### 12H-pre.11 What remains NOT_STARTED

1. Application implementation -- `NOT_STARTED`.
2. `app/` directory creation -- `NOT_CREATED`.
3. `app/probe_translator/` directory creation -- `NOT_CREATED`.
4. Phase 12H -- `NOT_STARTED`; requires explicit user authorization.
5. Numeric values for `APP_EVT_*` -- open for Phase 12H.
6. Devkit hardware integration of Framework -- `NOT_STARTED`.
7. Bridge ABI memory-layout lock -- `NOT_STARTED`.

### 12H-pre.12 Open gates preserved unchanged

- ACTIVE disarm widening -- `USER_DECISION_REQUIRED_ACTIVE_DISARM`.
- Scheduler 7A/7B -- `DEFER`.
- F407 / custom board -- `HOLD/DEFER`.
- POST_FLASH_AUTOSTART -- `OPEN/MITIGATED_BY_WORKFLOW`.
- Application / product layer -- `NOT_STARTED`; first `<product>`
  placeholder = `probe_translator`.
- Devkit integration of Framework -- `NOT_STARTED`.

### 12H-pre.13 Next gate

**Hold or open Phase 12H -- Probe Translator App Skeleton Planning
(Variant 1, docs-only) or Probe Translator Host Prototype (Variant
2, host-first implementation)** only on **explicit user
authorization**. Variant 1 is preferred unless the user authorizes
implementation directly. The recommended scope and exit criteria
are recorded in
[`../02_PHASE_CLOSEOUTS/PHASE_12H_PRE_FIRST_APPLICATION_CANDIDATE_SELECTION.md`](../02_PHASE_CLOSEOUTS/PHASE_12H_PRE_FIRST_APPLICATION_CANDIDATE_SELECTION.md)
§P. Phase 12H **was opened** (Variant 1, docs-only) after this gate
and is recorded in the entry below.

---

<a id="phase-12h"></a>
## Phase 12H -- Probe Translator App Skeleton Planning (docs-only, Variant 1)

**Status:** `CLOSED_DOCS_ONLY`
**Decision:** `PHASE_12H_PROBE_TRANSLATOR_SKELETON_PLANNED_DOCS_ONLY`
**Closeout:** [`../02_PHASE_CLOSEOUTS/PHASE_12H_PROBE_TRANSLATOR_SKELETON_PLANNING.md`](../02_PHASE_CLOSEOUTS/PHASE_12H_PROBE_TRANSLATOR_SKELETON_PLANNING.md)
**New long-lived spec:** [`../03_SPECS/PROBE_TRANSLATOR_APP_SKELETON_DRAFT.md`](../03_SPECS/PROBE_TRANSLATOR_APP_SKELETON_DRAFT.md)
**Date:** 2026-05-13

### 12H.1 Purpose

Phase 12H is the docs-only application-skeleton planning gate
(Variant 1) recommended by Phase 12H-pre. It locks the future
`RobotOS_v1.0/app/probe_translator/` skeleton boundary: exact
future file set, public API names and shapes, app-local state /
event / adapter-key vocabulary, transition table, bridge mapping
table, host test plan, and build strategy preference.

Phase 12H does not create source, does not modify Framework code,
does not modify devkit runtime, does not modify `devkit_app_state`,
does not change command semantics, does not run hardware, does not
create the `app/` directory, does not create
`app/probe_translator/`, and does not open Phase 12I.

### 12H.2 Files added / modified

- **New (2 docs):**
  - `RobotOS_v1.0/devkit/docs/02_PHASE_CLOSEOUTS/PHASE_12H_PROBE_TRANSLATOR_SKELETON_PLANNING.md`
    -- decision closeout (sections A-T).
  - `RobotOS_v1.0/devkit/docs/03_SPECS/PROBE_TRANSLATOR_APP_SKELETON_DRAFT.md`
    -- new long-lived skeleton spec
    (`DRAFT / NON-FINAL`, sections 1-15).
- **Modified:**
  - `RobotOS_v1.0/devkit/docs/01_PROGRESS/DEVKIT_PROGRESS_PHASE_11_20.md`
    -- this entry + index row.
  - `CURRENT_STATE.md` -- Phase 12H as latest closed.
  - `RobotOS_v1.0/devkit/docs/00_INDEX/README.md` -- Phase 12H
    closeout link + skeleton spec link.
  - `RobotOS_v1.0/devkit/docs/03_SPECS/FIRST_APPLICATION_CANDIDATE_DRAFT.md`
    -- short cross-reference to the skeleton spec; sections 2-13
    unchanged.
- **Zero-diff held:**
  - All `.c` and `.h` files under `framework/`, `core/`,
    `platform/`, `devkit/src/`, `tests/`, `src/`,
    `include/robotos/`.
  - All `CMakeLists.txt`.
  - All `prj.conf`, board DTS, overlay, Zephyr config files.
  - All existing evidence logs.
  - **No `app/` or `application/` directory created.**
  - **No `app/probe_translator/` directory created.**

### 12H.3 Future file boundary locked

- **Required at first implementation:** `app/probe_translator/
  probe_translator.h`, `probe_translator.c`, `README.md`;
  `tests/host/test_app_probe_translator_mapping.c`.
- **Optional:** `app/probe_translator/PROBE_TRANSLATOR_SPEC.md`
  only if local docs grow large.
- **Forbidden at first implementation:**
  `app/probe_translator/CMakeLists.txt`; Zephyr `prj.conf` / DTS;
  `probe_translator_devkit.*`; `probe_translator_uart.*`;
  `probe_translator_main.c`; RTT / J-Link / OpenOCD / flashing
  scripts; any Architecture B reuse.

### 12H.4 Public API names locked

| API | Purpose |
|---|---|
| `probe_translator_init(pt, config)` | Init harness + embedded FSM + bridge in one call. |
| `probe_translator_dispatch_adapter_event(pt, type, arg0, payload)` | Forward synthetic adapter tuple through bridge -> FSM. |
| `probe_translator_reset(pt)` | Reset bridge counters + FSM state. |
| `probe_translator_get_snapshot(pt, out)` | Combined FSM + bridge snapshot. |

Numeric values, struct layout, and ABI memory placement remain
open at Phase 12H (deferred to Phase 12I or later).

### 12H.5 State / event vocabulary locked (names)

| States | Required / optional |
|---|---|
| `PROBE_TRANSLATOR_STATE_IDLE / READY / ACTIVE` | Required. |
| `PROBE_TRANSLATOR_STATE_FAULT` | Optional; ship-or-defer at Phase 12I-pre. |

| Events | Required / optional |
|---|---|
| `PROBE_TRANSLATOR_EVT_CONFIGURED / START / STOP / RESET` | Required. |
| `PROBE_TRANSLATOR_EVT_FAULT` | Optional; ship-or-defer at Phase 12I-pre. |

### 12H.6 Transition table locked (names)

Rows 0-4 required (`IDLE+CONFIGURED -> READY`, `READY+START ->
ACTIVE`, `ACTIVE+STOP -> READY`, `READY+RESET -> IDLE`,
`ACTIVE+RESET -> IDLE`). Row 5 (`IDLE+RESET -> IDLE`) and rows
6-9 (FAULT block) optional. All rows use `guard = NULL` and
`action = NULL` at first implementation.

### 12H.7 Bridge mapping table locked (names)

Adapter type constants: `PROBE_ADAPTER_TYPE_CONFIG / _COMMAND /
_FAULT?`. Adapter arg0 constants: `PROBE_ADAPTER_ARG_NONE /
_START / _STOP / _RESET / _ANY?`. Rows 0-3 required (specific
match); row 4 (`FAULT` wildcard) optional. Specific rows precede
the wildcard row; wildcard uses a distinct `adapter_type` so it
does not collide.

### 12H.8 Host test plan locked

16 cases (TC01-TC16): init valid/invalid, four mapping happy paths
(CONFIG / START / STOP / RESET), optional FAULT wildcard,
unmapped event accounting, payload borrowed, snapshot counters,
reset, full transition path, three grep gates (no
`devkit_app_state.h` / no `a/s/r/?/x/v/L/d/T` / no Zephyr /
devkit / legacy `ro_*` includes), regression preserved
(>=23/23 after new target).

### 12H.9 Build strategy preferred (locked at planning depth)

**Option A** -- additive entry in existing
`tests/host/CMakeLists.txt`. Option B (new
`app/probe_translator/CMakeLists.txt`) acceptable but not
preferred at first. Option C (devkit integration) NOT
authorized. Host environment = WSL Ubuntu / gcc 13.3.0.

### 12H.10 Boundary preservation

- `devkit_app_state` remains authoritative; not modified, not
  consumed by `probe_translator/`. Scope-guard #11 re-affirmed.
- Command set `a/s/r/?/x/v/L/d/T` unchanged; no UART command
  added by Phase 12H or by the future first application.
- Architecture B (`src/`, `include/robotos/`) frozen; future
  `probe_translator/` uses Architecture A contracts only.

### 12H.11 What remains NOT_STARTED

1. Application implementation -- `NOT_STARTED`.
2. `app/` directory creation -- `NOT_CREATED`.
3. `app/probe_translator/` directory creation -- `NOT_CREATED`.
4. Phase 12I -- `NOT_STARTED`; requires explicit user
   authorization.
5. Numeric values for `PROBE_TRANSLATOR_STATE_*`,
   `PROBE_TRANSLATOR_EVT_*`, `PROBE_ADAPTER_TYPE_*`,
   `PROBE_ADAPTER_ARG_*` -- open for Phase 12I-pre.
6. FAULT block ship-or-defer -- open for Phase 12I-pre.
7. Transition row 5 ship-or-defer -- open for Phase 12I-pre.
8. Devkit hardware integration of Framework -- `NOT_STARTED`.
9. Bridge ABI memory-layout lock -- `NOT_STARTED`.

### 12H.12 Open gates preserved unchanged

- ACTIVE disarm widening -- `USER_DECISION_REQUIRED_ACTIVE_DISARM`.
- Scheduler 7A/7B -- `DEFER`.
- F407 / custom board -- `HOLD/DEFER`.
- POST_FLASH_AUTOSTART -- `OPEN/MITIGATED_BY_WORKFLOW`.
- Application / product layer -- `NOT_STARTED`; first
  `<product>` placeholder = `probe_translator`; skeleton locked.
- Devkit integration of Framework -- `NOT_STARTED`.

### 12H.13 Next gate

**Hold or open Phase 12I-pre -- Probe Translator Host Prototype
Implementation Plan (docs-only)** only on **explicit user
authorization**. Phase 12I-pre **was opened** after this gate and
is recorded in the entry below.

---

<a id="phase-12i-pre"></a>
## Phase 12I-pre -- Probe Translator Host Prototype Implementation Plan (docs-only)

**Status:** `CLOSED_DOCS_ONLY`
**Decision:** `PHASE_12I_PRE_PROBE_TRANSLATOR_HOST_PROTOTYPE_PLAN_CLOSED`
**Closeout:** [`../02_PHASE_CLOSEOUTS/PHASE_12I_PRE_PROBE_TRANSLATOR_HOST_PROTOTYPE_PLAN.md`](../02_PHASE_CLOSEOUTS/PHASE_12I_PRE_PROBE_TRANSLATOR_HOST_PROTOTYPE_PLAN.md)
**New long-lived spec:** [`../03_SPECS/PROBE_TRANSLATOR_HOST_PROTOTYPE_PLAN.md`](../03_SPECS/PROBE_TRANSLATOR_HOST_PROTOTYPE_PLAN.md)
**Date:** 2026-05-13

### 12I-pre.1 Purpose

Phase 12I-pre converts the Phase 12H skeleton plan into an
execution-ready implementation contract for Phase 12I -- Probe
Translator Host Prototype. It resolves all remaining implementation
choices that Phase 12H intentionally left open, so Phase 12I can
deliver in a single pass without churn.

Phase 12I-pre does not create source, does not modify Framework
code, does not modify devkit runtime, does not modify
`devkit_app_state`, does not change command semantics, does not run
hardware, does not create the `app/` directory, and does not open
Phase 12I.

### 12I-pre.2 Files added / modified

- **New (2 docs):**
  - `RobotOS_v1.0/devkit/docs/02_PHASE_CLOSEOUTS/PHASE_12I_PRE_PROBE_TRANSLATOR_HOST_PROTOTYPE_PLAN.md`
    -- decision closeout (sections A-R).
  - `RobotOS_v1.0/devkit/docs/03_SPECS/PROBE_TRANSLATOR_HOST_PROTOTYPE_PLAN.md`
    -- new long-lived implementation-plan spec
    (`DRAFT / NON-FINAL`, sections 1-14).
- **Modified:**
  - `RobotOS_v1.0/devkit/docs/01_PROGRESS/DEVKIT_PROGRESS_PHASE_11_20.md`
    -- this entry + index row.
  - `CURRENT_STATE.md` -- Phase 12I-pre as latest closed.
  - `RobotOS_v1.0/devkit/docs/00_INDEX/README.md` -- Phase 12I-pre
    closeout link + implementation-plan spec link.
  - `RobotOS_v1.0/devkit/docs/03_SPECS/PROBE_TRANSLATOR_APP_SKELETON_DRAFT.md`
    -- revision line + cross-reference to the new implementation-
    plan spec; sections 1-15 otherwise unchanged.
- **Zero-diff held:** all `.c`/`.h`, all `CMakeLists.txt`, all
  `prj.conf`, board DTS, overlay, Zephyr config files, all
  existing evidence logs. **No `app/` or `app/probe_translator/`
  directory created.**

### 12I-pre.3 Resolved decisions summary

| Decision | Resolution |
|---|---|
| State numeric values | `IDLE=1u`, `READY=2u`, `ACTIVE=3u` |
| Event numeric values | `CONFIGURED=1u`, `START=2u`, `STOP=3u`, `RESET=4u` |
| Adapter type numeric values | `CONFIG=1u`, `COMMAND=2u` |
| Adapter arg numeric values | `NONE=0u`, `START=1u`, `STOP=2u`, `RESET=3u` |
| `FAULT` block | **DEFERRED** |
| Transition row 5 (`IDLE+RESET→IDLE`) | **DEFERRED** |
| `PROBE_ADAPTER_ARG_ANY` | **OMITTED** from Phase 12I |
| `probe_translator_t` ownership | **Embed by value** |
| `probe_translator_snapshot_t` | **Combined struct** (`fsm` + `bridge` fields) |
| Non-NULL action rows | **None** — TC08 is `REVIEW_VALIDATED` |
| Build strategy | **Option A** (additive entry in `tests/host/CMakeLists.txt`) |
| Host test count | **15 cases** (TC01–TC15) |
| Expected regression | **23/23** PASS after Phase 12I |

### 12I-pre.4 Phase 12I approved future file set

**New app files:** `app/probe_translator/probe_translator.{c,h}` +
`README.md`. **New host test:** `tests/host/test_app_probe_translator_
mapping.c`. **Modified:** `tests/host/CMakeLists.txt` (additive
block only). **New log:** `tests/host/logs/phase_12I_host_<date>.log`.
**New closeout:** `PHASE_12I_PROBE_TRANSLATOR_HOST_PROTOTYPE.md`.
**Forbidden:** `app/probe_translator/CMakeLists.txt`; Zephyr / devkit
/ UART / hardware / Architecture B files.

### 12I-pre.5 CMake block locked

Exact `add_executable` / `target_include_directories` / `add_test`
block recorded in
[`../03_SPECS/PROBE_TRANSLATOR_HOST_PROTOTYPE_PLAN.md`](../03_SPECS/PROBE_TRANSLATOR_HOST_PROTOTYPE_PLAN.md)
§8.1. Uses new `APP_DIR` variable; compiles `probe_translator.c` +
`robotos_fw_event_bridge.c` + `robotos_fw_fsm.c` + platform critical
stub.

### 12I-pre.6 Validation and exit criteria for Phase 12I

14 gates (§K of closeout): CMake configure/build PASS; 23/23 tests
PASS; FSM + bridge + probe-translator targets each PASS; log saved;
four grep gates clean (no `devkit_app_state.h` / no `devkit_*`
calls / no UART command bytes / no Zephyr-devkit-legacy includes);
command set unchanged; `devkit_app_state` zero-diff; no hardware
run.

### 12I-pre.7 Boundary preservation

- `devkit_app_state` remains authoritative; not modified.
  Scope-guard #11 re-affirmed.
- Command set `a/s/r/?/x/v/L/d/T` unchanged.
- Architecture B frozen; future `probe_translator/` uses
  Architecture A contracts only.

### 12I-pre.8 What remains NOT_STARTED

1. Application implementation -- `NOT_STARTED`.
2. `app/` directory creation -- `NOT_CREATED`.
3. `app/probe_translator/` directory creation -- `NOT_CREATED`.
4. Phase 12I -- `NOT_STARTED`; requires explicit user authorization.
5. FAULT block -- DEFERRED to a future app-behavior phase.
6. Transition row 5 -- DEFERRED.
7. Devkit hardware integration of Framework -- `NOT_STARTED`.
8. Bridge ABI memory-layout lock -- `NOT_STARTED`.

### 12I-pre.9 Open gates preserved unchanged

- ACTIVE disarm widening -- `USER_DECISION_REQUIRED_ACTIVE_DISARM`.
- Scheduler 7A/7B -- `DEFER`.
- F407 / custom board -- `HOLD/DEFER`.
- POST_FLASH_AUTOSTART -- `OPEN/MITIGATED_BY_WORKFLOW`.
- Application / product layer -- `NOT_STARTED`.
- Devkit integration of Framework -- `NOT_STARTED`.

### 12I-pre.10 Next gate

**Hold or open Phase 12I -- Probe Translator Host Prototype
(host-first implementation)** only on **explicit user
authorization**. The implementation contract is complete in
[`../03_SPECS/PROBE_TRANSLATOR_HOST_PROTOTYPE_PLAN.md`](../03_SPECS/PROBE_TRANSLATOR_HOST_PROTOTYPE_PLAN.md).
Phase 12I **remains `NOT_STARTED`** at the close of Phase 12I-pre.
Phase 12I **was opened** after this gate and is recorded in the
entry below.

---

<a id="phase-12i"></a>
## Phase 12I -- Probe Translator Host Prototype

**Status:** `CLOSED_WITH_HOST_TEST_EVIDENCE`
**Decision:** `PHASE_12I_PROBE_TRANSLATOR_HOST_PROTOTYPE_IMPLEMENTED_VALIDATED`
**Closeout:** [`../02_PHASE_CLOSEOUTS/PHASE_12I_PROBE_TRANSLATOR_HOST_PROTOTYPE.md`](../02_PHASE_CLOSEOUTS/PHASE_12I_PROBE_TRANSLATOR_HOST_PROTOTYPE.md)
**Implementation-plan spec (updated):** [`../03_SPECS/PROBE_TRANSLATOR_HOST_PROTOTYPE_PLAN.md`](../03_SPECS/PROBE_TRANSLATOR_HOST_PROTOTYPE_PLAN.md)
**Skeleton spec (updated):** [`../03_SPECS/PROBE_TRANSLATOR_APP_SKELETON_DRAFT.md`](../03_SPECS/PROBE_TRANSLATOR_APP_SKELETON_DRAFT.md)
**Host log:** [`../../../../tests/host/logs/phase_12I_host_2026-05-13.log`](../../../../tests/host/logs/phase_12I_host_2026-05-13.log)
**Date:** 2026-05-13

### 12I.1 Purpose

Phase 12I executes the implementation contract locked at Phase 12I-pre:
it creates the first application harness (`RobotOS_v1.0/app/probe_translator/`)
on top of the locked Architecture-A Framework (`robotos_fw_fsm` +
`robotos_fw_event_bridge`) and validates it via a single new host
test target, with the full host regression preserved at 23/23.

Phase 12I is a host-only prototype. No devkit binding, no UART
command surface, no Zephyr / hardware runtime, no
`devkit_app_state` change, no `a/s/r/?/x/v/L/d/T` command-set
change, no legacy Architecture-B reuse.

### 12I.2 Files added / modified

**New (4 source/test/log + 1 closeout):**

- `RobotOS_v1.0/app/probe_translator/probe_translator.h`
- `RobotOS_v1.0/app/probe_translator/probe_translator.c`
- `RobotOS_v1.0/app/probe_translator/README.md`
- `RobotOS_v1.0/tests/host/test_app_probe_translator_mapping.c`
- `RobotOS_v1.0/tests/host/logs/phase_12I_host_2026-05-13.log`
- `RobotOS_v1.0/devkit/docs/02_PHASE_CLOSEOUTS/PHASE_12I_PROBE_TRANSLATOR_HOST_PROTOTYPE.md`

**Modified (additive only):**

- `RobotOS_v1.0/tests/host/CMakeLists.txt` -- additive
  `APP_DIR` + `add_executable` + `target_include_directories` +
  `add_test` block; nothing in the prior file was removed or
  rewritten.
- `RobotOS_v1.0/devkit/docs/03_SPECS/PROBE_TRANSLATOR_HOST_PROTOTYPE_PLAN.md`
  -- status upgrade to `IMPLEMENTED_AT_12I (HOST-TEST EVIDENCE)`;
  materialized file list; evidence cross-reference.
- `RobotOS_v1.0/devkit/docs/03_SPECS/PROBE_TRANSLATOR_APP_SKELETON_DRAFT.md`
  -- status upgrade; materialized file list; evidence
  cross-reference.
- `RobotOS_v1.0/devkit/docs/01_PROGRESS/DEVKIT_PROGRESS_PHASE_11_20.md`
  -- this entry + index row.
- `CURRENT_STATE.md` -- Phase 12I as latest closed.
- `RobotOS_v1.0/devkit/docs/00_INDEX/README.md` -- Phase 12I
  closeout link.

**Zero-diff held:** `core/`, `platform/`, `framework/`,
`devkit/src/`, `src/`, `include/robotos/`, `tests/` (other than
the new test + additive CMake block), all `prj.conf` / DTS /
overlay / Zephyr config files, all prior evidence logs.

### 12I.3 Implementation outcome

- `probe_translator_init(&pt, NULL)` initializes the embedded
  FSM (initial state `STATE_IDLE`) and event bridge in one call.
- `probe_translator_dispatch_adapter_event(&pt, type, arg0, payload)`
  forwards through the bridge into the FSM and returns the bridge
  status verbatim.
- `probe_translator_reset(&pt)` resets bridge counters first,
  then the FSM (state -> `STATE_IDLE`, counters cleared).
- `probe_translator_get_snapshot(&pt, &out)` fills the combined
  FSM + bridge snapshot in one call.
- State machine: `IDLE -(CONFIG/NONE)-> READY -(COMMAND/START)->
  ACTIVE -(COMMAND/STOP)-> READY -(COMMAND/RESET)-> IDLE`.
  Both `READY+RESET` and `ACTIVE+RESET` rows ship at Phase 12I
  (5 transition rows total).

### 12I.4 Implementation deviation from spec example

The Phase 12I-pre implementation-plan spec §6.4 showed
`robotos_fw_fsm_config_t fsm_cfg;` and
`robotos_fw_event_bridge_config_t bridge_cfg;` as stack-local
variables inside `probe_translator_init`. The Framework FSM
(Phase 12E) and bridge (Phase 12F) store the config **by
pointer** (`fsm->config = config` / `bridge->config = config`),
and their headers document that "the config object and the
arrays it references must outlive the FSM". Stack-locals do not
outlive `init`, so dispatches after `init` would dereference
dangling memory.

**Resolution:** the configs are embedded into
`probe_translator_t` as `_fsm_cfg` and `_bridge_cfg` fields.
Their lifetime now matches the harness instance, which is itself
caller-owned static. All fields remain opaque per the spec.
Public API surface is unchanged. The spec example is a
documentation bug; the long-lived implementation-plan spec is
updated at Phase 12I close to record the corrected pattern.

### 12I.5 Host test contract

- File: `RobotOS_v1.0/tests/host/test_app_probe_translator_mapping.c`.
- Cases: TC01..TC15 per `PROBE_TRANSLATOR_HOST_PROTOTYPE_PLAN.md` §7.
- TC08, TC12, TC13, TC14, TC15 are `REVIEW_VALIDATED` (no runtime
  side-effect; included as explicit PASS lines so the host log
  records gate exercise).
- Total runtime assertions: **70 / 70 PASS** (within the spec's
  ~50-70 expected range).
- Single new ctest target: `probe_translator_mapping_contract`.

### 12I.6 Validation evidence (14 gates per Phase 12I-pre §K)

| # | Gate | Result |
|---|---|---|
| 1 | CMake configure | **PASS** -- gcc 13.3.0 / cmake 3.28.3 under WSL Ubuntu; no warnings. |
| 2 | Build | **PASS** -- no warnings; `probe_translator_mapping_contract_test` linked. |
| 3 | `probe_translator_mapping_contract` test | **PASS** (70/70 assertions). |
| 4 | `robotos_fw_fsm_contract` still PASS | **PASS**. |
| 5 | `robotos_fw_event_bridge_contract` still PASS | **PASS**. |
| 6 | Full host regression | **23/23 PASS** (`100% tests passed, 0 tests failed out of 23`). |
| 7 | Host log saved | `tests/host/logs/phase_12I_host_2026-05-13.log` (84124 bytes). |
| 8 | Grep gate: no `devkit_app_state` | **PASS** (no match in `app/probe_translator/` or new test). |
| 9 | Grep gate: no `devkit_*` calls | **PASS**. |
| 10 | Grep gate: no UART command bytes | **PASS** (no a/s/r/?/x/v/L/d/T literals in app or test). |
| 11 | Grep gate: no Zephyr / legacy includes | **PASS** (no `<zephyr/...>`, `ro_*`, `include/robotos/`). |
| 12 | Command set unchanged | **PASS** -- `devkit/src/` zero-diff. |
| 13 | `devkit_app_state` zero-diff | **PASS** -- `devkit/src/devkit_app_state.{c,h}` zero-diff. |
| 14 | No hardware run | **PASS** -- no RTT / J-Link / OpenOCD / `prj.conf` / DTS / overlay change. |

### 12I.7 Boundary preservation

- **No `devkit_app_state` change.** Scope-guard #11 re-affirmed.
- **Command set `a/s/r/?/x/v/L/d/T` unchanged.** No UART surface
  in `app/probe_translator/`.
- **Framework code zero-diff.** `framework/robotos_fw_fsm.{c,h}`
  and `framework/robotos_fw_event_bridge.{c,h}` are unchanged.
- **Devkit runtime zero-diff.** `devkit/src/` and all
  devkit `CMakeLists.txt` unchanged.
- **Core / platform zero-diff.**
- **Architecture B frozen.** `src/` and `include/robotos/`
  unchanged; `app/probe_translator/` uses Architecture-A
  contracts only.
- **No hardware run.** No flashing, no debug session, no
  `prj.conf` / DTS / overlay change.
- **POST_FLASH_AUTOSTART** -- still `OPEN / MITIGATED_BY_WORKFLOW`.
- **ACTIVE disarm widening** -- still
  `USER_DECISION_REQUIRED_ACTIVE_DISARM`.
- **Scheduler 7A/7B** -- still `DEFER`.
- **F407 / custom board** -- still `HOLD/DEFER`.

### 12I.8 Open carry-forward gates (unchanged at Phase 12I close)

- ACTIVE disarm widening -- `USER_DECISION_REQUIRED_ACTIVE_DISARM`.
- Scheduler 7A/7B -- `DEFER`.
- F407 / custom board -- `HOLD/DEFER`.
- POST_FLASH_AUTOSTART -- `OPEN / MITIGATED_BY_WORKFLOW`.
- `PROBE_TRANSLATOR_STATE_FAULT` + FAULT block -- `DEFERRED` to
  a future app-behavior phase.
- Devkit integration of Framework FSM + bridge -- `NOT_STARTED`.
- Bridge ABI memory-layout lock -- `NOT_STARTED`.
- Hardware-runnable Zephyr build of `app/probe_translator/` --
  open for future runtime-integration phase.
- `RobotOS_v1.0/examples/` -- open for future docs-only phase.
- Multi-product coordination rules -- open; reachable only after
  a second app exists.

### 12I.9 Next gate

**Hold.** The first application harness is host-test-validated.
Future phases must be opened only on **explicit user
authorization**. Likely candidate gates (all `NOT_STARTED`):

- FAULT block extension (future app-behavior phase).
- Devkit integration of `probe_translator` (separate planning
  phase; touches devkit runtime and `devkit_app_state` boundary
  -- requires re-evaluation of scope-guard #11).
- Hardware-runnable Zephyr build of `probe_translator` (future
  runtime-integration phase; requires `prj.conf` / DTS /
  overlay).
- Second application harness under `app/<product>/` (triggers
  multi-product coordination planning).

None of these may open without an explicit user command.
Phase 12J-pre **was opened** after this gate and is recorded in
the entry below.

---

<a id="phase-12j-pre"></a>
## Phase 12J-pre -- Probe Translator FAULT Block Plan (docs-only)

**Status:** `CLOSED_DOCS_ONLY`
**Decision:** `PHASE_12J_PRE_PROBE_TRANSLATOR_FAULT_BLOCK_PLAN_CLOSED`
**Closeout:** [`../02_PHASE_CLOSEOUTS/PHASE_12J_PRE_PROBE_TRANSLATOR_FAULT_BLOCK_PLAN.md`](../02_PHASE_CLOSEOUTS/PHASE_12J_PRE_PROBE_TRANSLATOR_FAULT_BLOCK_PLAN.md)
**New long-lived spec:** [`../03_SPECS/PROBE_TRANSLATOR_FAULT_BLOCK_PLAN.md`](../03_SPECS/PROBE_TRANSLATOR_FAULT_BLOCK_PLAN.md)
**Date:** 2026-05-13

### 12J-pre.1 Purpose

Phase 12J-pre converts the Phase 12I-deferred FAULT block into an
execution-ready implementation contract for Phase 12J -- Probe
Translator FAULT Block Extension. It commits the numeric values
that Phase 12I held in comments only, decides ship-or-defer for
the optional transition rows (5-9) and mapping row 4, and resolves
sticky-FAULT behavior, all without writing source.

Phase 12J-pre does not create source, does not modify Framework
code, does not modify devkit runtime, does not modify
`devkit_app_state`, does not change command semantics, does not run
hardware, does not modify `app/probe_translator/probe_translator.{h,c}`,
does not modify the host test, does not modify CMake, and does not
open Phase 12J.

### 12J-pre.2 Files added / modified

- **New (2 docs):**
  - `RobotOS_v1.0/devkit/docs/02_PHASE_CLOSEOUTS/PHASE_12J_PRE_PROBE_TRANSLATOR_FAULT_BLOCK_PLAN.md`
    -- decision closeout (sections A-R).
  - `RobotOS_v1.0/devkit/docs/03_SPECS/PROBE_TRANSLATOR_FAULT_BLOCK_PLAN.md`
    -- new long-lived implementation-plan spec
    (`DRAFT / NON-FINAL`, sections 1-17).
- **Modified (doc-sync only):**
  - `RobotOS_v1.0/devkit/docs/01_PROGRESS/DEVKIT_PROGRESS_PHASE_11_20.md`
    -- this entry + index row.
  - `CURRENT_STATE.md` -- Phase 12J-pre as latest closed.
  - `RobotOS_v1.0/devkit/docs/00_INDEX/README.md` -- Phase 12J-pre
    closeout link + new spec link.
- **Zero-diff held:** all `.c`/`.h`, all `CMakeLists.txt`, all
  `prj.conf`, board DTS, overlay, Zephyr config files, all
  existing evidence logs, `app/probe_translator/` source/header/
  README.

### 12J-pre.3 Resolved decisions summary

| Decision | Resolution |
|---|---|
| `STATE_FAULT` numeric value | `4u` (formalized) |
| `EVT_FAULT` numeric value | `5u` (formalized) |
| `ADAPTER_TYPE_FAULT` numeric value | `3u` (formalized) |
| `ADAPTER_ARG_ANY` declaration | `((uint32_t)0xFFFFFFFFu)` -- doc alias only |
| Transition row 5 (`IDLE + RESET → IDLE`) | **SHIPPED** at 12J |
| Transition rows 6-9 (FAULT block) | **SHIPPED** at 12J |
| Mapping row 4 (FAULT wildcard) | **SHIPPED** at 12J |
| Sticky FAULT for CONFIG/START/STOP | **implicit** (no FSM row) |
| Bridge code change | **NONE** (uses existing Phase 12F `match_arg0=false`) |
| CMake change | **NONE** (12I block already compiles `.c`) |
| Final transition table | 10 rows |
| Final mapping table | 5 rows |
| Final state-def table | 4 entries |
| New test cases | TC16-TC24 (~30-40 assertions) on top of retained TC01-TC15 |
| Expected regression | 23/23 PASS (no new ctest target) |

### 12J-pre.4 Phase 12J approved future file set

**Modified existing files (additive only):**
`app/probe_translator/probe_translator.{c,h}` (FAULT constants,
5 new transition rows, 1 new state def, 1 new mapping row, count
bumps); `app/probe_translator/README.md` (FAULT symbols appended);
`tests/host/test_app_probe_translator_mapping.c` (TC16-TC24
appended; TC01-TC15 retained verbatim).

**New:** `tests/host/logs/phase_12J_host_<date>.log`;
`PHASE_12J_PROBE_TRANSLATOR_FAULT_BLOCK.md` closeout.

**Forbidden:** `tests/host/CMakeLists.txt` change;
`app/probe_translator/CMakeLists.txt`; any Zephyr / devkit / UART
/ hardware / Architecture B file; any `framework/` / `core/` /
`platform/` / `devkit/src/` file.

### 12J-pre.5 No CMake change required

Phase 12I's additive block in `tests/host/CMakeLists.txt` lists
`${APP_DIR}/probe_translator.c` as a target source. Phase 12J adds
content **inside** that same `.c` file. CMake automatically
rebuilds the existing target on next configure/build; no
`add_executable` / `target_include_directories` / `add_test`
change.

### 12J-pre.6 No bridge ABI impact

The Phase 12J wildcard mapping row uses an existing Phase 12F
bridge feature (`match_arg0 == false`). The bridge `.c` / `.h`
files are zero-diff at Phase 12J; the bridge contract test
(`robotos_fw_event_bridge_contract`) remains unchanged and PASS.
The bridge ABI memory-layout lock (Phase 12G §11 #9 open gate)
is **not** triggered.

### 12J-pre.7 Validation and exit criteria for Phase 12J

14 gates (§K of closeout): CMake configure/build PASS; 23/23 tests
PASS; FSM + bridge + probe-translator targets each PASS; log saved
as `phase_12J_host_<date>.log`; four grep gates clean (no
`devkit_app_state.h` / no `devkit_*` calls / no UART command bytes
/ no Zephyr-devkit-legacy includes); command set unchanged;
`devkit_app_state` zero-diff; no hardware run. Plus a Phase-12J-
specific structural check: `transition_count = 10u`,
`state_count = 4u`, `row_count = 5u` in `probe_translator_init`.

### 12J-pre.8 Boundary preservation

- `devkit_app_state` remains authoritative; not modified.
  Scope-guard #11 re-affirmed.
- Command set `a/s/r/?/x/v/L/d/T` unchanged.
- Architecture B frozen; `probe_translator/` uses Architecture A
  contracts only.
- Phase 12I host-test baseline (TC01-TC15, 70/70 assertions, 23/23
  ctest) preserved verbatim; Phase 12J adds without removing.

### 12J-pre.9 What remains NOT_STARTED

1. FAULT block implementation -- `NOT_STARTED`. Phase 12J requires
   explicit user authorization.
2. Non-NULL action callbacks for FAULT -- deferred to a future
   app-behavior phase.
3. on_entry / on_exit for FAULT (e.g., cause latching) -- deferred.
4. Multi-source FAULT cause encoding -- deferred.
5. Devkit hardware integration of Framework -- `NOT_STARTED`.
6. Bridge ABI memory-layout lock -- `NOT_STARTED` (not triggered
   by Phase 12J).
7. Hardware-runnable Zephyr build of `app/probe_translator/` --
   `NOT_STARTED`.
8. Second `app/<product>/` -- `NOT_STARTED`.
9. `RobotOS_v1.0/examples/` -- `NOT_STARTED`.

### 12J-pre.10 Open gates preserved unchanged

- ACTIVE disarm widening -- `USER_DECISION_REQUIRED_ACTIVE_DISARM`.
- Scheduler 7A/7B -- `DEFER`.
- F407 / custom board -- `HOLD/DEFER`.
- POST_FLASH_AUTOSTART -- `OPEN/MITIGATED_BY_WORKFLOW`.
- Devkit integration of Framework -- `NOT_STARTED`.

### 12J-pre.11 Next gate

**Hold or open Phase 12J -- Probe Translator FAULT Block
Extension (host-only additive implementation)** only on **explicit
user authorization**. The implementation contract is complete in
[`../03_SPECS/PROBE_TRANSLATOR_FAULT_BLOCK_PLAN.md`](../03_SPECS/PROBE_TRANSLATOR_FAULT_BLOCK_PLAN.md).
Phase 12J **remains `NOT_STARTED`** at the close of Phase 12J-pre.

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
