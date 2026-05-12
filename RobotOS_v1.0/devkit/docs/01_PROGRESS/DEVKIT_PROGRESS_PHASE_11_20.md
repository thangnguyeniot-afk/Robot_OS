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
