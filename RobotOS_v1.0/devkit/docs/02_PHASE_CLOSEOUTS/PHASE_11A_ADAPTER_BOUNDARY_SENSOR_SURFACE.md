# Phase 11A -- Adapter Boundary & Sensor Surface Decision

**Status:** `CLOSED_DOCS_ONLY`
**Type:** Docs-only architecture-gate / boundary decision. **No source,
runtime, test, CMake, Zephyr, board, `prj.conf`, DTS overlay,
host-tool, or script change.** Phase 11A does **not** implement `T`,
does **not** start ACTIVE disarm widening, does **not** approve any
hardware purchase, and does **not** start Robot Framework or
Application work.
**Date opened/closed:** 2026-05-12 (same-day docs-only close)
**Branch:** master
**Published baseline at open:** `origin/master = 7ce8cb7`
**Prior runtime behavior phase:** Phase 10B-d (firmware `125779c`,
evidence-close `7e250dc`).
**Prior docs-only checkpoint:** Phase 10C
([`PHASE_10C_COMMAND_SET_CHECKPOINT.md`](PHASE_10C_COMMAND_SET_CHECKPOINT.md),
commit `796e114`).
**Companion docs:**
[`../01_PROGRESS/DEVKIT_PROGRESS_PHASE_11_20.md`](../01_PROGRESS/DEVKIT_PROGRESS_PHASE_11_20.md),
[`../03_SPECS/COMMAND_SET_DRAFT.md`](../03_SPECS/COMMAND_SET_DRAFT.md),
[`PHASE_10C_COMMAND_SET_CHECKPOINT.md`](PHASE_10C_COMMAND_SET_CHECKPOINT.md),
[`PHASE_9EZ_CHECKPOINT.md`](PHASE_9EZ_CHECKPOINT.md).

---

## A. Executive Summary

Phase 11A is the first Phase 11–20 architecture gate. It produces a
**docs-only decision** with three deliverables:

1. **Adapter API surface inventory** (concept-level): a per-primitive
   classification of what the Adapter / runtime substrate currently
   exposes, what is partially proven, and what is uncharacterized.
2. **Sensor surface classification** for `T`: one of `ADAPTER_PROBE`,
   `FRAMEWORK`, `APPLICATION`, `HOLD`, `PARTIAL_DECISION`. **Phase 11A
   classifies as `SENSOR_SURFACE_DECIDED_ADAPTER_PROBE`** (see §F),
   meaning `T` is a future bounded Adapter probe candidate **only** --
   not implementation approval, not hardware approval, and not a
   product-semantics promotion.
3. **Phase 11B–11E forward plan** placement: the next gate is Phase
   11B (Device / Driver Feasibility) and **no hardware purchase is
   authorized** by Phase 11A. Purchase decision belongs to Phase 11B
   after on-board / internal-resource feasibility is verified.

Phase 11A **does not** implement `T`, **does not** start ACTIVE disarm
widening, **does not** change `prj.conf`, **does not** add a DTS
overlay, **does not** enable any sensor / I²C / SPI / ADC driver, and
**does not** touch `core/`, `platform/`, `devkit/src/`, tests, CMake,
Zephyr config, or evidence logs.

---

## B. Baseline Before Phase 11A

**Confirmed repo facts (verified this audit):**

| Item | Value | Source |
|---|---|---|
| `origin/master` at open | `7ce8cb7` (`docs: define Phase 11-20 progress stream convention`) | git log |
| Phase 11 status before this phase | `RESERVED / NOT_STARTED` | `DEVKIT_PROGRESS_PHASE_11_20.md` §5 placeholder |
| Validated non-sensor command set | `a / s / r / ? / x / v / L / d` (8 hardware-validated) | `PHASE_10C_COMMAND_SET_CHECKPOINT.md` §3 |
| `T` status | `USER_DECISION_REQUIRED` / not implemented | `COMMAND_SET_DRAFT.md` Section B row 122 |
| ACTIVE disarm widening | `USER_DECISION_REQUIRED_ACTIVE_DISARM`; current `d` from ACTIVE = recognized no-op | `PHASE_10C_COMMAND_SET_CHECKPOINT.md` §5.1 |
| Scheduler 7A/7B | `DEFER` (no workload evidence to reopen) | `PHASE_10C_COMMAND_SET_CHECKPOINT.md` §5.3 |
| F407 / custom board | `HOLD/DEFER` (no workload evidence to reopen) | `PHASE_10C_COMMAND_SET_CHECKPOINT.md` §5.4 |
| UART TX scope | minimal response only; 12 scope-guard constraints intact | `PHASE_9EZ_CHECKPOINT.md` §H |
| POST_FLASH_AUTOSTART | root cause `OPEN`; `MITIGATED_BY_WORKFLOW` from Phase 6O onward via `capture_devkit_rtt.ps1` sidecar `reset run`; manual RESET fallback retained; plain `west flash` not runtime-start evidence | `PHASE_10C_COMMAND_SET_CHECKPOINT.md` §8 |
| `MAX_EVENTS_PER_TICK` | `1` (in `core/robotos_core.c`) | source inspection |
| Event queue capacity | `16` | source inspection |
| `CONFIG_SENSOR` / `CONFIG_I2C` / `CONFIG_SPI` / `CONFIG_ADC` in `devkit/prj.conf` | **None enabled.** `CONFIG_I2C`, `CONFIG_SPI`, `CONFIG_SENSOR`, and `CONFIG_ADC` are all absent (verified against Phase 4 bringup `43de448` and current HEAD; an earlier draft of this doc incorrectly claimed `CONFIG_I2C=y` / `CONFIG_SPI=y` were pre-existing — that has been corrected docs-only). | `devkit/prj.conf` |
| `devkit/boards/` directory | **does not exist** | filesystem |
| `devkit/*.overlay` files | **none present** | filesystem |
| F407 source / config | **none present** (grep returned 0 matches) | source inspection |
| Docs structure | `00_INDEX/`, `01_PROGRESS/`, `02_PHASE_CLOSEOUTS/`, `03_SPECS/`, `04_SCRATCH_OR_PROVENANCE/` | published at `9ff90b5`–`7ce8cb7` |
| Phase-window progress convention | Phase 1–9E/9E-Z in `DEVKIT_PROGRESS.md`; Phase 10 in `DEVKIT_PROGRESS_PHASE_10.md`; Phase 11–20 in `DEVKIT_PROGRESS_PHASE_11_20.md` | `01_PROGRESS/README.md` |

**Inference (not directly verified in this audit):**

- The Zephyr SDK installed for this repo includes the Zephyr sensor
  subsystem (generated syscalls visible under
  `devkit/build/zephyr/include/generated/syscalls/sensor.h`). Whether
  a specific sensor driver (`stm32-temp`, on-board MEMS such as
  L3GD20 / LSM303C / I3G4250D, etc.) is enabled in upstream Zephyr DT
  for `stm32f411e_disco` is **not verified by Phase 11A** -- that
  verification is explicitly **Phase 11B work**.

---

## C. Layer Map

The 4-layer architecture intent is: Application → Robot Framework →
Robot Adapter → Kernel/HW. Mapping repo reality against that intent:

| Layer | Confirmed repo realization | Status |
|---|---|---|
| **Kernel + HW** | Zephyr 3.6.0 + STM32F411E-DISCO; used unmodified | **Used as foundation.** |
| **Robot Adapter / runtime substrate** | `core/` (event queue, dispatcher, admission counter, backpressure), `platform/` (critsec, log, time, fault), plus devkit-local hardware glue (button GPIO, UART RX/TX, LED GPIO, timer producer, telemetry, fault sampling) | **Substantially built but NOT exposed as a clean public API.** There is no `robotos_adapter.h` aggregate header. Adapter capabilities are reachable only through `robotos_core.h` + `platform/robotos_platform_*.h` + devkit-local patterns. |
| **devkit / validation harness** | `devkit/src/main.c`, `devkit_runtime.c`, plus 8 probe modules; UART command vocabulary `a / s / r / ? / x / v / L / d` | **Substantially built.** This is the dominant deliverable of Phase 1–10. `devkit_app_state.c` is explicitly devkit-local (scope-guard #11 in `PHASE_9EZ_CHECKPOINT.md §H`). |
| **Robot Framework** | none | **Not built.** No stepper / servo / PID / generic state-machine / generic sensor API exists. |
| **Application / product** | none | **Not built.** No CNC / 3D printer / arm specific use case exists. |

**Confirmed interpretations (carry forward):**

- Phase 1–10 mostly delivered **Adapter / runtime substrate + devkit
  validation harness**, not Robot Framework or Application.
- `devkit_app_state` is **not** Robot Framework. Its IDLE / ARMED /
  ACTIVE FSM is a devkit-local smoke test for the event pipeline. It
  must not be promoted (scope-guard #11).
- `a / s / r / ? / x / v / L / d` are **proto-command validation
  probes**, not final product application commands. They have
  proto-vocabulary discipline (query / physical-effect / explicit
  semantic / state / negative-path) that future Framework /
  Application command surfaces may learn from, but they are not
  themselves a product spec.
- Robot Framework is **not built yet**. Choosing the first Framework
  primitive (stepper / timer-service / state-machine / sensor) is a
  separate phase (12A).
- Application / product layer is **not built yet**. No use case (CNC
  / 3D printer / arm / line tracker) has been selected.

**Inference / future recommendation (not facts):**

- A future `03_SPECS/ADAPTER_API_SURFACE.md` would make Adapter
  primitives explicit. Phase 11A inventories them conceptually (§D)
  but does not author that spec file.
- The next clean architectural step after sensor surface classification
  is Robot Framework API surface planning (Phase 12A), which will use
  the §D inventory as its dependency contract.

---

## D. Adapter API Surface Inventory (Concept Level)

Inventory of primitive classes the future Robot Framework may require
from the Adapter / runtime substrate. **Concept-level only. No
implementation file is authored by Phase 11A.**

| Primitive class | Status | Evidence / phase | Notes |
|---|---|---|---|
| Time / tick / monotonic timing | **Proven** | `platform/robotos_platform_time.h` + Zephyr `k_msleep`; 500 ms `DEVKIT_TICK_MS`; every phase since Phase 5+ | Robust. |
| Thread-context execution boundary | **Proven** | Dispatcher thread vs ISR-callback split | Phase 5F / 9A / 9B |
| Critical section / interrupt-safety boundary | **Proven** | `platform/robotos_platform_critical.h` + `platform/zephyr/critical_zephyr.c` | Phase 5D / 5E |
| ISR-safe event posting | **Proven** | `robotos_core_post_event()` called from ISR in button, UART, timer producers | Phase 5G / 6G / 6H / 9A / 9B |
| Queue / dispatch / event budget | **Proven** | `core/robotos_event_queue.c`, `MAX_EVENTS_PER_TICK=1`, `QUEUE_CAPACITY=16` | Phase 6I peak=8; Phase 9D peak=16 dropped=3 (operator-induced, within bound); Phase 9G peak=5 dropped=0 |
| GPIO input as event source | **Proven** | `devkit_button_producer.c` (EXTI + debounce, Phase 9A-A/B/C) | Robust. |
| GPIO output | **Proven** | `devkit_status_led.c` toggle; `L` command (Phase 10B-L) | OPERATOR_VISUAL_CONFIRMED. |
| UART / serial byte stream RX | **Proven** | `devkit_uart_producer.c` UART ISR → `uart_fifo_read` → event post | Phase 9B / 9G |
| UART / serial byte stream TX | **Proven** | `uart_poll_out` from thread-context handler | Phase 9E / 10B-{v,L,d} |
| RTT trace / telemetry / fault observation | **Proven** | ROBOTOS_OBS / ROBOTOS_FAULT / ROBOTOS_PROD periodic triplets via RTT; CFSR/HFSR sampling | Phase 6K / 6L / 6M / 6Z and every later phase |
| Timer-generated events | **Proven** | `devkit_timer_producer.c` (k_timer → ISR → event) | Phase 6G / 6H / 6I |
| **Driver-dependent read surface** | **Not characterized** | no `CONFIG_SENSOR`/`CONFIG_ADC`/`CONFIG_I2C`/`CONFIG_SPI` in `devkit/prj.conf`; no sensor DT overlay; no probe. I²C/SPI driver subsystems exist in upstream Zephyr but are not yet enabled for the RobotOS devkit app. | **Largest open Adapter gap.** This is the surface `T` would probe if classified as Adapter probe. |
| **I²C bus-backed IO** | **Not characterized** | none | Subset of driver-dependent read. |
| **SPI bus-backed IO** | **Not characterized** | none | Subset of driver-dependent read. |
| **ADC sampled IO** | **Not characterized** | none | Internal die temp / `Vrefint` path would use ADC. |
| **Pool / slab allocator** | **Needs decision** | event queue is fixed-capacity ring; no separate pool primitive; diagram lists "pool" under Adapter | Open question: is "pool" already realized as the fixed-capacity ring, or is it a separate primitive? Phase 11A flags but **does not decide** -- this can be settled in a later docs-only sub-phase or rolled into Phase 12A Framework API surface planning. |
| **Portability backend** | **Claimed but undemonstrated** | only `platform/zephyr/` exists; only `stm32f411e_disco` exercised | F407 / QEMU / native-host backends remain HOLD/DEFER. |
| **Scheduler / admission policy** | **Default-budget only** | `MAX_EVENTS_PER_TICK=1`; 7A/7B mutation `DEFER` | No workload evidence justifies mutation. |

**Inventory-level conclusions:**

1. **Eleven Adapter primitive classes are proven on hardware** -- a
   substantial Adapter substrate.
2. **The single largest uncharacterized primitive class is
   driver-dependent read** (and its three sub-classes: I²C, SPI,
   ADC). This is the natural place a sensor probe would land.
3. **No explicit `robotos_adapter.h` API contract exists yet.** A
   future Framework would today compile against `robotos_core.h` +
   `platform/robotos_platform_*.h` + devkit-local headers. Making
   the contract explicit is a candidate for a later docs-only spec
   phase but is **not Phase 11A scope**.
4. **Pool / slab is an open architectural question** that Phase 11A
   surfaces but does not answer.

---

## E. Hardware Input Classes Still Uncharacterized

The Adapter inventory (§D) shows the following gaps:

1. **Driver-dependent read** -- any device that requires a Zephyr
   driver subsystem (`sensor`, `adc`, `i2c`, `spi`) to read.
2. **Bus-backed sensor / device read** -- I²C / SPI bus access from
   thread-context handler, error propagation from bus driver.
3. **Error-path behavior for driver failure** -- what does the
   Adapter contract look like when a sensor returns
   `-EIO` / `-EINVAL` / `-ETIMEDOUT`? The existing devkit handlers
   only propagate GPIO/UART error variants; bus-driver error variants
   are uncharacterized.
4. **Unit / format / calibration boundary** -- if the Adapter
   returns raw counts vs. calibrated values, that is a boundary
   decision. Raw counts are clearly Adapter-class; calibration is
   Framework-class.
5. **Timeout / fault discipline** -- driver-dependent reads can
   block longer than a single tick. The Adapter contract for
   "handler may take >1 tick" is not characterized.
6. **Fixed-buffer response discipline for driver-dependent values** --
   what is the bounded byte cost of a sensor response in the existing
   96-byte stack buffer? This must be specified before any probe.

**Where each gap belongs (Phase 11A position):**

| Gap | Most natural layer | Why |
|---|---|---|
| Driver-dependent read | **Adapter** | Driver subsystem call is a Kernel/HW-adjacent primitive; matches the existing "ISR-post + thread-handle" pattern |
| Bus-backed read | **Adapter** (bus access); **Framework** (device abstraction) | Bus is platform-level; device taxonomy is robotics-level |
| Error-path behavior | **Adapter** | The Adapter must define how it surfaces driver errors; Framework can layer richer error semantics on top |
| Unit / format / calibration | **Framework** (or **Application**) | Raw values are Adapter; calibrated/typed values are Framework |
| Timeout / fault discipline | **Adapter** | Latency budget is a runtime-substrate concern |
| Fixed-buffer response | **Adapter probe constraint** | The 96-byte buffer is a current Adapter constraint; probe must respect it |

Phase 11A position: **driver-dependent read, bus access, error-path,
timeout/fault, and fixed-buffer response are Adapter-class concerns**
that a bounded probe should validate. **Unit / format / calibration is
explicitly Framework-class** and is **not** approved for invention by a
Phase 11D probe -- the probe must return Adapter-level raw values (or
direct driver-output values) without inventing calibration semantics.

---

## F. Sensor Surface Classification (`T` Decision)

### F.1 Options evaluated

| Option | Meaning | Affected boundaries | Risk if chosen |
|---|---|---|---|
| (1) Adapter-level sensor-read probe | `T` becomes a bounded Adapter probe of driver-dependent read; raw-value response; error variant; thread-context handler; fixed-buffer compliance | `prj.conf` (`CONFIG_SENSOR/I2C/SPI/ADC` Kconfig), possibly DT overlay; `devkit_app_state.c` + `devkit_uart_producer.c` (single-byte `case 't':` arm); possibly new `devkit_sensor_probe.c`. Does NOT touch `core/` or `platform/` | First Phase 10B-class command to add a non-trivial `prj.conf` change; risk of response-format invention if not carefully bounded |
| (2) Robot Framework abstraction | Sensor becomes a Framework API (`robotos_sensor_t` channels, calibration, units); `T` is the Application/test exposure of a Framework sensor object | New `framework/` dir (does not exist); Framework API design with one data point | Largest scope; commits to Framework sensor model before any other Framework primitive (stepper/servo/PID); unusual order in robotics |
| (3) Application / product feature | `T` is a product command in a chosen use case (CNC / 3D printer / arm) | Depends entirely on product; premature without product choice | Cart-before-horse; no product selected |
| (4) Hold | Keep `T` as `USER_DECISION_REQUIRED`; do not advance | none | Indefinite delay of the largest open Adapter gap |
| (5) Partial decision | Classify some sensor-related sub-aspects (e.g. raw read is Adapter; calibration is Framework) but defer the implementation decision | docs-only | Acceptable as a refinement; matches actual repo state |

### F.2 Decision

**`SENSOR_SURFACE_DECIDED_ADAPTER_PROBE`** -- with the explicit caveat
that this is a **future bounded probe classification only**, **not**
an implementation approval and **not** a hardware purchase approval.

### F.3 Rationale

1. The driver-dependent read surface is the largest uncharacterized
   Adapter primitive class (§D). Closing it requires a bounded probe.
2. Phase 1–10 has consistently used **evidence-first, bounded probes**
   to characterize Adapter primitives. Sensor read follows the same
   pattern.
3. **Option (2) Framework** is premature -- choosing sensor as the
   first Framework primitive before any other Framework primitive
   (stepper / PID / state-machine) inverts the usual order in
   robotics, where sensors typically feed control loops.
4. **Option (3) Application** is premature -- no product has been
   selected.
5. **Option (4) Hold** is defensible but stops progress on the largest
   open Adapter gap.
6. The **separation imposed by §E (raw read = Adapter; calibration =
   Framework)** keeps the probe bounded. The probe returns raw values
   or direct driver output; calibration / units / sensor identity in
   any rich-typed sense is Framework concern and is **not** approved
   by Phase 11A.

### F.4 What this classification does NOT do

- Does **not** approve implementation of `T`.
- Does **not** approve any hardware purchase.
- Does **not** authorize `CONFIG_SENSOR`, `CONFIG_I2C`, `CONFIG_SPI`,
  `CONFIG_ADC`, or any DT overlay.
- Does **not** promote `devkit_app_state` to Framework.
- Does **not** change the Phase 9E-Z scope-guard list.
- Does **not** change the 96-byte fixed-buffer constraint.
- Does **not** invent sensor units, calibration, or sensor identity
  fields.
- Does **not** authorize multi-channel sensor responses or response
  framing.

---

## G. `T` Next-Step Option Matrix

| Aspect | (1) Bounded Adapter probe | (2) Framework sensor abstraction | (3) Application sensor feature | (4) Continue hold |
|---|---|---|---|---|
| Prerequisites | Phase 11B feasibility (driver + part); Phase 11C probe spec | Framework API surface (Phase 12A); first Framework primitive list | Product choice (CNC / 3D printer / arm); Application command vocabulary spec | none |
| Affected boundaries | `prj.conf` (first time), possibly DT overlay; `devkit_app_state.c`, `devkit_uart_producer.c` | New `framework/` dir; Framework API headers; multi-phase work | Depends on product; potentially many files | none |
| Likely files if later approved | `prj.conf`, `devkit/boards/stm32f411e_disco.overlay` (new if needed), `devkit_app_state.c`, `devkit_uart_producer.c`, possibly new `devkit_sensor_probe.c` | new `framework/sensor.{h,c}`, plus all probe-class files | Product-specific Application files; potentially refactor of devkit | none |
| Risk | Response-format scope creep; first `prj.conf` change must be carefully bounded | Largest scope; Framework API designed with one data point | Premature without product choice | Indefinite delay of largest Adapter gap |
| Validation path | Phase 9E/10B-style (pristine build, sidecar `reset run`, RTT counter table, host transcript, scope-guard restatement, dedicated closeout) | Multi-phase; cannot reasonably close in one phase | Depends on product | n/a |
| Hardware requirement | Depends on §H feasibility outcome -- may be zero-purchase (internal temp / on-board MEMS) or low-cost external (~$5 I²C module) | Same as (1); plus possibly additional sensors to characterize Framework abstraction | Product-specific sensors | none |
| Phase 11B feasibility required | **Yes** | **Yes** (and Framework planning) | Depends on product | **No** |
| Requires `prj.conf` / overlay | **Yes** (Phase 11D, not 11A) | **Yes** (and Framework code) | Depends on product | **No** |
| Risk of response-buffer growth | **Bounded** -- probe spec must respect 96-byte buffer | **High** -- Framework typed responses grow naturally | **Very high** -- product commands outgrow single-byte vocabulary | **None** |
| Touches `core/` or `platform/`? | **No** | Probably **no** for the first iteration | Depends on product | **No** |
| Phase 11A recommendation | **Selected** (§F.2) | Defer until 12A or later | Defer until product chosen | Available fallback if §F.2 implementation surfaces a blocker |

---

## H. Device / Purchase Gate Placement

**No hardware purchase is authorized by Phase 11A.**

The purchase decision belongs to **Phase 11B (Device / Driver
Feasibility Gate)**, which is a separate docs / audit phase. The
sequence is strict:

1. **Phase 11B verifies existing STM32F411E-DISCO resources first**, in
   priority order:

   a. **STM32 internal die temperature via ADC.** If Zephyr's
      `stm32-temp` driver supports STM32F411 (or if a raw ADC channel
      read can be used), this is a zero-purchase, zero-wiring sensor
      probe target. Closes the ADC-sampled IO sub-class only.

   b. **On-board MEMS / peripherals on STM32F411E-DISCO.** The board
      typically carries a gyro (SPI) and accel+mag (I²C); exact parts
      depend on board revision. If Zephyr DT + driver support is
      confirmed for the part on the user's specific board revision,
      this is a zero-purchase sensor probe target that closes I²C or
      SPI + driver-dependent read in one probe.

   c. **External I²C sensor module** (e.g. BME280, BMP280, AHT10) --
      **only considered if (a) and (b) are not viable**. Low cost
      (~$2–$5), but requires purchase, wiring, and verification.

   d. **External SPI sensor** -- lower priority than (c) unless a
      specific reason exists.

2. **F407 / portability remains `HOLD/DEFER`.** Phase 15A (or
   equivalent future portability gate) is the earliest revisit. Phase
   11A does not lift this hold.

3. **Phase 11B output** is a docs-only feasibility report. If 11B
   confirms (a) or (b), Phase 11C (probe spec) follows with zero
   purchase. If 11B requires (c)/(d), the purchase decision is part
   of 11B's output -- still no hardware bought before 11B closes.

4. **Phase 11A does not name or recommend a specific part as
   "approved."** Naming a part requires Phase 11B verification of
   driver presence + DT support; Phase 11A only frames the priority
   ordering above.

---

## I. ACTIVE Disarm Widening Boundary

**ACTIVE disarm widening is NOT part of Phase 11A.** Status:

- **Current behavior preserved:** `d` from ACTIVE → recognized no-op
  (response `OK disarm no-op state=ACTIVE\r\n`). No transition, no
  `ignored++`.
- **Future option preserved:** `ACTIVE → IDLE` with
  `transitions++` and response `OK disarm state=IDLE\r\n` is the
  single-line widening that the user may authorize separately.
- **Decision gate status:** `USER_DECISION_REQUIRED_ACTIVE_DISARM`,
  unchanged from Phase 10C §5.1.
- **Validation if later approved:** Supplemental hardware run with
  sequence `d a s d ?` (expected final `transitions=3 button=0 uart=5
  ignored=0`).
- **Scheduling:** This is a separate small vocabulary housekeeping
  gate, **not** scheduled ahead of Phase 11A or Phase 11B. It is
  decoupled from the Phase 11A-E sensor track. It may be opened as
  "Phase 10B-d-A supplemental" only on **explicit user request** for
  that specific housekeeping.

Phase 11A does not modify, schedule, or recommend ACTIVE disarm
widening. The user's stated preference (avoid piecemeal phase
decisions) keeps it parked.

---

## J. Non-goals

Phase 11A is bounded by the following negative assertions. Each is a
constraint that any reader (or future agent) must respect when working
with this phase's output.

- **No `T` implementation.** No source change.
- **No ACTIVE disarm widening.** Current ACTIVE behavior preserved.
- **No sensor driver enablement.** `CONFIG_SENSOR`, `CONFIG_ADC`,
  `CONFIG_I2C`, and `CONFIG_SPI` are all absent from
  `devkit/prj.conf` (an earlier draft of this doc claimed
  `CONFIG_I2C`/`CONFIG_SPI` were pre-existing from Phase 4 bringup;
  that claim was incorrect and has been corrected docs-only).
  Phase 11A does not modify any of them and does not wire any
  sensor driver or DT node against them.
- **No DTS overlay creation.** `devkit/boards/` directory and
  `*.overlay` files remain absent.
- **No hardware purchase.** Purchase decision belongs to Phase 11B
  after on-board / internal feasibility is verified.
- **No F407 / custom-board work.** `HOLD/DEFER` preserved.
- **No Scheduler 7A/7B work.** `DEFER` preserved.
- **No Robot Framework API implementation.** Framework remains not
  built; Phase 12A (later) is its planning gate.
- **No Application / product logic.** Product not chosen.
- **No parser / shell / command registry / framing / response
  queue.** All 12 UART TX scope-guard constraints from
  `PHASE_9EZ_CHECKPOINT.md §H` preserved.
- **No UART TX scope expansion.** Single-byte command vocabulary;
  fixed 96-byte stack buffer; thread-context handler-only TX.
- **No command behavior change.** `a / s / r / ? / x / v / L / d`
  remain as closed at Phase 10C.
- **No `core/` or `platform/` change.** Adapter substrate preserved.
- **No evidence-log change.** `RobotOS_v1.0/devkit/logs/*.txt`
  preserved.
- **No `DEVKIT_PROGRESS.md` (historical master) change.** Frozen.
- **No push.** Local commit only; push is a separate user-gated
  step.

---

## K. Next Gate

Given the **`SENSOR_SURFACE_DECIDED_ADAPTER_PROBE`** classification:

**Next phase:** **Phase 11B -- Device / Driver Feasibility Gate**
(docs / audit only).

**Phase 11B objectives** (placeholder; Phase 11B is not opened by
this doc):

1. Verify STM32 internal die temperature / ADC feasibility:
   - Is `stm32-temp` driver available for STM32F411 in the installed
     Zephyr SDK?
   - If not, can a raw ADC channel read (channel 16, internal Vrefint)
     be used directly?
2. Verify on-board MEMS / peripheral feasibility on the user's
   specific STM32F411E-DISCO board revision:
   - What sensor parts are on board?
   - Are they enabled in upstream Zephyr DT for `stm32f411e_disco`?
   - If not enabled by default, what overlay would enable them?
3. If (1) and (2) are not viable, identify a small external I²C
   sensor module candidate (e.g. BME280, BMP280, AHT10) with
   confirmed Zephyr driver presence -- and record this as a purchase
   recommendation, **not a purchase authorization**.
4. Do not buy anything in Phase 11B. The purchase authorization
   happens only after Phase 11B's feasibility report is reviewed and
   the user explicitly approves a part.

**After Phase 11B closes:**

- **Phase 11C -- Sensor Probe Spec (docs-only).** Freeze response
  format (raw value, no calibration), error variant, fixed-buffer
  byte cost, harness sequence.
- **Phase 11D -- Sensor Probe Implementation (firmware).** Only if
  Phase 11B has a verified part and Phase 11C has a frozen spec.
- **Phase 11E -- Sensor Probe Evidence Closeout.** Phase 9E / 10B-style
  hardware evidence.
- **Phase 12A -- Robot Framework API Surface Planning (docs-only).**
  Opens after Phase 11E closes (or if sensor track is deliberately
  paused after 11A/11B).
- **Phase 15A -- Portability Decision Revisit.** Only after Adapter +
  Framework are stable.

**Fallback paths:**

- If Phase 11B verification surfaces a blocker (no viable internal
  resource, no viable on-board MEMS, user does not authorize external
  purchase), the sensor track converts to **Option (4) Continue
  hold**. Phase 12A Framework API surface planning then takes the
  Phase 11C-E slots, and the sensor gap is documented but left open.
- If Phase 11C spec discussion surfaces a scope-guard violation risk
  (e.g. response format cannot be bounded under 96 B), Phase 11D
  does not open. Sensor surface is re-classified at Phase 11C close.

---

## L. Phase Tag and Commit Mapping

| Phase | Type | Commit hash | Status at this doc's open |
|---|---|---|---|
| 10C | docs-only | `796e114` | published; baseline for Phase 11A |
| Migration / cleanup / convention | docs-only | `9ff90b5`, `b2f90e8`, `7ce8cb7` | published; baseline for Phase 11A |
| **11A (this doc)** | **docs-only** | this commit | `CLOSED_DOCS_ONLY` |
| 11B (next gate) | docs / audit | not opened | reserved |
| 11C | docs-only | not opened | reserved (conditional on 11A classification + 11B feasibility) |
| 11D | firmware | not opened | reserved (conditional on 11B + 11C) |
| 11E | evidence | not opened | reserved (conditional on 11D) |
| 12A | docs-only | not opened | reserved (Framework API surface planning) |
| 15A | docs-only | not opened | reserved (portability revisit) |

---

## M. What this document does not do

- Does not implement `T`.
- Does not approve any hardware purchase.
- Does not authorize `CONFIG_SENSOR`, `CONFIG_ADC`, `CONFIG_I2C`, or
  `CONFIG_SPI` enablement. None of these are currently enabled in
  `devkit/prj.conf`; Phase 11A does not modify any of them. (An
  earlier draft of this doc claimed `CONFIG_I2C`/`CONFIG_SPI` were
  pre-existing from Phase 4 bringup; that claim was incorrect and
  has been corrected docs-only.)
- Does not author a DTS overlay.
- Does not start ACTIVE disarm widening; current ACTIVE no-op
  preserved.
- Does not reopen Scheduler 7A/7B; `DEFER` preserved.
- Does not reopen F407 / custom-board; `HOLD/DEFER` preserved.
- Does not modify `core/` or `platform/`.
- Does not modify any closed phase's evidence or closeout doc.
- Does not modify `DEVKIT_PROGRESS.md` (historical master).
- Does not change UART TX scope; all 12 scope-guard constraints
  intact.
- Does not change command semantics; `a / s / r / ? / x / v / L / d`
  preserved.
- Does not change POST_FLASH_AUTOSTART status; root cause `OPEN`;
  mitigated-by-workflow preserved.
- Does not push.
