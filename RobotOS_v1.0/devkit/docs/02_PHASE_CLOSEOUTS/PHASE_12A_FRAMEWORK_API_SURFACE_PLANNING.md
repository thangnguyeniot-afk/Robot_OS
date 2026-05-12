# Phase 12A -- Robot Framework API Surface Planning

**Status:** `CLOSED_DOCS_ONLY`
**Type:** Docs-only architecture gate / Framework API surface planning. **No
source, runtime, test, CMake, Zephyr, board, host-tool, script, `prj.conf`,
DTS overlay, evidence log, or `framework/` directory change.** Phase 12A
defines the planning boundary for the Robot Framework layer. It introduces
**no** implementation, changes **no** runtime behavior, starts **no**
Application/product work, and **does not** reopen Scheduler 7A/7B, F407,
or any other deferred gate.
**Date opened/closed:** 2026-05-12 (same-day docs-only close)
**Branch:** master
**Published baseline at open:** `origin/master = c239466`
**Prior docs-only checkpoint:** Phase 11Z (`CLOSED_DOCS_ONLY`; validated
9-command set `a/s/r/?/x/v/L/d/T`; Phase 11A–11E sensor-probe track shelved).
**Prior runtime implementation phase:** Phase 11D (firmware `2040bfb`).
**Prior hardware evidence phase:** Phase 11E (`10710b3`;
`CLOSED_WITH_HARDWARE_EVIDENCE` + `OPERATOR_PHYSICAL_SANITY_CONFIRMED`).
**Companion docs:**
[`PHASE_11Z_COMMAND_SET_CHECKPOINT.md`](PHASE_11Z_COMMAND_SET_CHECKPOINT.md),
[`PHASE_11A_ADAPTER_BOUNDARY_SENSOR_SURFACE.md`](PHASE_11A_ADAPTER_BOUNDARY_SENSOR_SURFACE.md),
[`PHASE_11E_ACCEL_PROBE_EVIDENCE.md`](PHASE_11E_ACCEL_PROBE_EVIDENCE.md),
[`PHASE_10C_COMMAND_SET_CHECKPOINT.md`](PHASE_10C_COMMAND_SET_CHECKPOINT.md),
[`../03_SPECS/COMMAND_SET_DRAFT.md`](../03_SPECS/COMMAND_SET_DRAFT.md),
[`../01_PROGRESS/DEVKIT_PROGRESS_PHASE_11_20.md`](../01_PROGRESS/DEVKIT_PROGRESS_PHASE_11_20.md).

---

## A. Executive Summary

Phase 12A is the **first Robot Framework planning gate**. It is a docs-only
architecture phase that:

1. **States the layer boundary** between the Adapter/runtime substrate
   (built in Phase 1–11) and the Robot Framework layer (not yet built).
2. **Inventories Adapter evidence available to Framework** — what the
   Framework may rely on without re-proving.
3. **Evaluates candidate Framework API domains** (state-machine, timer
   service, sensor, actuator, endstop, PID, motion primitive, fault/safety,
   observability) with rationale for why each belongs now or later.
4. **Selects a recommended first Framework slice** for Phase 12B planning.
5. **Records the remaining open gates** so they are not re-derived by the
   next agent.

Phase 12A is the bridge from Adapter evidence to Framework contract planning.
It is analogous to Phase 11A (Adapter boundary decision before the sensor-probe
track) but at the Framework layer boundary.

**Phase 12A does not implement any Framework API, does not create any
`framework/` source directory or header, does not change any devkit
command, does not promote `devkit_app_state` to Framework, does not touch
`core/` or `platform/`, and does not authorize any follow-on phase
automatically.**

Validated command set is unchanged at Phase 12A:

```text
a / s / r / ? / x / v / L / d / T
```

---

## B. Baseline Before Phase 12A

| Item | Value |
|---|---|
| `origin/master` at open | `c239466` (`docs: add Phase 11Z command-set checkpoint`) |
| Last docs-only checkpoint | Phase 11Z (`CLOSED_DOCS_ONLY`; `c239466`) |
| Last runtime implementation phase | Phase 11D (firmware `2040bfb`; `feat: add Phase 11D accelerometer probe command`) |
| Last hardware evidence phase | Phase 11E (`10710b3`; `CLOSED_WITH_HARDWARE_EVIDENCE` + `OPERATOR_PHYSICAL_SANITY_CONFIRMED`) |
| Validated command set | **`a / s / r / ? / x / v / L / d / T`** (9 commands; all hardware-evidence-backed) |
| Hardware platform | STM32F411E-DISCO **revision D** (production runtime baseline) |
| Hardware platform alternatives | F407 / custom board: `HOLD/DEFER` (unchanged) |
| Scheduler | `ROBOTOS_CORE_MAX_EVENTS_PER_TICK = 1`; `ROBOTOS_EVENT_QUEUE_CAPACITY = 16`; 7A/7B `DEFER` |
| UART TX scope | Minimal response only; all 12 scope-guard constraints intact |
| POST_FLASH_AUTOSTART | Root cause `OPEN`; `MITIGATED_BY_WORKFLOW` via Phase 6O sidecar `reset run` |
| `devkit/prj.conf` delta since Phase 9E | `CONFIG_I2C=y` + `CONFIG_SENSOR=y` (Phase 11D); no `CONFIG_SPI`, `CONFIG_ADC`, `CONFIG_CBPRINTF_FP_SUPPORT` |
| Robot Framework | **NOT BUILT** — no `framework/` directory, no Framework header, no Framework module |
| Application / product layer | **NOT BUILT** — no product vocabulary, no use-case selection |

**Adapter/devkit evidence summary at Phase 12A open:**

| Evidence class | Proven | Key phases |
|---|---|---|
| Time / tick / monotonic sleep | Yes | `platform/robotos_platform_time.h`; `robotos_platform_uptime_ms()`; `robotos_platform_sleep_ms()`; Phase 5B+ |
| Thread-context vs ISR boundary | Yes | Dispatcher thread + ISR-callback split; Phase 5F / 9A / 9B |
| Critical section / interrupt safety | Yes | `platform/robotos_platform_critical.h`; Phase 5D / 5E |
| ISR-safe event posting | Yes | `robotos_core_post_event()` from button/UART/timer ISRs; Phase 5G / 9A / 9B |
| Queue / dispatch / event budget | Yes | `core/robotos_event_queue.c`; `MAX_EVENTS_PER_TICK=1`; `QUEUE_CAPACITY=16`; Phase 9G `peak=5 dropped=0`; Phase 11E `peak=2 dropped=0 herr=0` |
| GPIO input event source | Yes | `devkit_button_producer.c` (EXTI + debounce); Phase 9A–C |
| GPIO output | Yes | `devkit_status_led_toggle()`; `L` command; Phase 10B-L `OPERATOR_VISUAL_CONFIRMED` |
| UART RX / TX minimal response | Yes | `devkit_uart_producer.c` ISR→event→handler→`uart_poll_out`; Phase 9B / 9E |
| RTT telemetry / fault observation | Yes | `ROBOTOS_OBS` / `ROBOTOS_FAULT` / `ROBOTOS_PROD` triplets; CFSR/HFSR sampling; Phase 6K–6M / 6Z |
| Timer-generated events | Yes | `devkit_timer_producer.c` (k_timer → ISR → event post); heartbeat 500 ms; Phase 6G–6I |
| Driver-dependent sensor read (I2C/`struct sensor_value`) | Yes | `T` command; `lsm303agr_accel` / `lis2dh` driver; I2C1/0x19; Phase 11D / 11E |

**Current Adapter limitations visible to Framework design:**

- Dispatch budget is fixed at `MAX_EVENTS_PER_TICK=1`; not profiled under
  real robot workloads; multiple simultaneous Framework services may require
  scheduler reassessment (Scheduler 7A/7B; currently DEFER).
- Only one sensor type (I2C accelerometer) characterized; SPI/ADC sub-classes
  remain uncharacterized.
- No explicit `robotos_adapter.h` aggregate header exists; Framework would
  today depend on `robotos_core.h` + `platform/robotos_platform_*.h` + devkit
  patterns; making this contract explicit is a future docs-only sub-phase.
- Pool / slab allocator: flagged as open architectural question at Phase 11A §D;
  the fixed-capacity event ring may be the only pool primitive; not resolved here.
- Portability backend: only `platform/zephyr/` exists; only `stm32f411e_disco`
  exercised; F407 / QEMU / native-host backends remain HOLD/DEFER.
- `robotos_event_type_t`: Framework event types would use the `ROBOTOS_EVENT_USER`
  base (`= 100`); no Framework type range has been reserved yet.

**Remaining open gates (all preserved unchanged at Phase 12A):**

1. ACTIVE disarm widening — `USER_DECISION_REQUIRED_ACTIVE_DISARM`
2. Scheduler 7A/7B — `DEFER`
3. F407 / custom board — `HOLD/DEFER`
4. POST_FLASH_AUTOSTART root cause — `OPEN` / `MITIGATED_BY_WORKFLOW`
5. Robot Framework API planning — `IN_PROGRESS` (this phase)
6. Application / product layer — `NOT_STARTED`

---

## C. Layer Boundary Statement

### C.1 Layer map

| Layer | Confirmed repo realization | Status |
|---|---|---|
| **Kernel / HW** | Zephyr 3.6.0 + STM32F411E-DISCO; used unmodified as substrate | **Used as foundation.** |
| **Robot Adapter / runtime substrate** | `core/` (event queue, dispatcher, admission, backpressure, retry advisory); `platform/` (critical section, time, fault, log); plus devkit-local hardware glue: button GPIO, UART RX/TX, LED GPIO, timer producer, observability, fault sampling | **Substantially built.** No explicit `robotos_adapter.h` aggregate header yet. Adapter capabilities are reachable through `robotos_core.h` + `platform/robotos_platform_*.h` + devkit-local patterns. |
| **devkit / validation harness** | `devkit/src/` modules; UART command vocabulary `a/s/r/?/x/v/L/d/T`; devkit runtime loop; Phase 1–11 deliverable | **Substantially built.** This is the dominant deliverable of Phase 1–11. |
| **Robot Framework** | None | **Not built.** No stepper/servo/PID/state-machine/sensor/actuator Framework API exists. No `framework/` directory. |
| **Application / product** | None | **Not built.** No CNC/3D-printer/arm/mobile use case. No product vocabulary. |

### C.2 Boundary assertions (mandatory carry-forward)

These assertions are restated from Phase 11A §C and Phase 11Z §F and must be
preserved by any future agent or phase:

- **`devkit_app_state` is NOT Robot Framework.** Its `IDLE / ARMED / ACTIVE`
  FSM is a devkit-local smoke test for the event pipeline. Scope-guard #11 in
  `PHASE_9EZ_CHECKPOINT.md §H` prohibits its promotion to Framework. Phase 12A
  does not lift this prohibition.

- **`a / s / r / ? / x / v / L / d / T` are devkit validation / proto-command
  probes, not final product commands.** They have disciplined vocabulary
  (query / physical-effect / explicit-semantic / state / negative-path / probe)
  that future Framework / Application command surfaces may learn from, but
  they are not themselves a product specification. Phase 12A does not change
  their semantics.

- **`T` is an Adapter probe evidence command.** Its hardware evidence (Phase
  11E) proves the driver-dependent read Adapter primitive. It does not
  constitute a Framework sensor API. Promoting `T` to a Framework sensor
  command requires a separate Framework sensor spec phase.

- **Framework should consume Adapter/runtime primitives through explicit
  contracts, not by copying devkit harness behavior blindly.** The devkit
  patterns (`devkit_timer_producer.c`, `devkit_button_producer.c`, etc.) are
  instructive templates, not normative Framework contracts. A future Framework
  implementation phase must specify the exact Adapter API surface it consumes,
  not simply replicate devkit module code.

- **`core/` and `platform/` boundaries are preserved.** Phase 12A makes no
  change to those directories. Any future Framework implementation that requires
  `core/` or `platform/` changes must identify and document those changes
  explicitly before implementing.

---

## D. Adapter Evidence Available to Framework

This section catalogs what the Robot Framework may rely on from existing
Adapter/runtime evidence, and what remains unavailable or unsettled. It is
the primary input to §E (candidate domain evaluation).

### D.1 Proven primitives (Framework may depend on these)

| Primitive | What is proven | API surface | Notes |
|---|---|---|---|
| **Time / tick** | Monotonic uptime in ms; blocking sleep in ms; 500 ms tick cadence under `DEVKIT_TICK_MS`; `robotos_core_tick_count()` for absolute tick index | `robotos_platform_uptime_ms()`, `robotos_platform_sleep_ms()`, `robotos_core_tick_count()` | `uptime_ms` wraps at UINT32_MAX (~49.7 days); no 64-bit counter. Phase 5B scope: sleep + uptime only; deadlines and monotonic 64-bit counters are out of scope. |
| **Thread-context execution boundary** | Clear ISR vs thread-context dispatch split; ISR posts events; thread handler consumes events; no ISR TX | ISR: `robotos_core_post_event()`; thread: `robotos_core_tick()` → handler dispatch | This boundary is the most load-bearing Adapter primitive for Framework event dispatch. |
| **Critical section** | Interrupt-safe entry/exit around shared state mutations | `platform/robotos_platform_critical.h` | Phase 5D/5E; Zephyr `irq_lock()`/`irq_unlock()` backend. |
| **ISR-safe event posting** | `robotos_core_post_event()` called safely from ISR; admission gate enforced; queue push atomic under critical section | `robotos_core_post_event()`, `robotos_core_try_post_event()` | Throttle-aware variant available (`try_post_event`); retry advisory mapping available (`robotos_core_retry_decision_for_status()`). |
| **Event queue / dispatch / budget** | Fixed-capacity ring (`QUEUE_CAPACITY=16`); dispatch budget `MAX_EVENTS_PER_TICK=1`; admission gate (NONE/reserved types rejected); peak, dropped, accepted, dispatched counters | `robotos_core_post_event()`, `robotos_core_tick()`, `robotos_core_snapshot()` | Framework event types must use `>= ROBOTOS_EVENT_USER (= 100)`. No Framework event type range reserved yet. |
| **GPIO input event source** | EXTI-backed single button with debounce; ISR → event post; button counter in telemetry | `devkit_button_producer.c` pattern | Phase 9A–C; framework GPIO input would follow this producer pattern. |
| **GPIO output** | Single LED toggle; thread-context; no ISR TX; heartbeat phase-shift observed on hardware | `devkit_status_led_toggle()` | Phase 10B-L `OPERATOR_VISUAL_CONFIRMED`. Single stateless toggle API; no LED state machine. |
| **UART RX / TX (minimal)** | Byte-stream RX via ISR → event; synchronous TX from thread-context handler; fixed 96-byte stack buffer; no parser/queue/framing | `devkit_uart_producer.c` patterns; all 12 scope guards from `PHASE_9EZ_CHECKPOINT.md §H` | Not a Framework communication channel; the 12 UART TX scope guards remain in force at Phase 12A. |
| **RTT telemetry / fault observation** | `ROBOTOS_OBS`, `ROBOTOS_FAULT`, `ROBOTOS_PROD` periodic triplets; CFSR/HFSR hardware fault register sampling; `robotos_platform_fault_report()` | `devkit_observability.c` pattern; `robotos_platform_fault.h` (`INFO/WARNING/ERROR/FATAL`) | Framework observability hooks would extend this pattern. CFSR/HFSR = 0x00000000 at every evidence phase. |
| **Timer-generated events** | Periodic `k_timer` → ISR → event post; heartbeat 500 ms; observed stable at `peak=2` under normal workload | `devkit_timer_producer.c` pattern | Phase 6G–6I; timer event provides the Framework tick hook. Framework services needing periodic execution would register against timer events. |
| **Driver-dependent sensor read** | I2C1 + `lsm303agr_accel`/`lis2dh` Zephyr driver; `sensor_sample_fetch` + `sensor_channel_get`; `struct sensor_value` integer-fractional format; error path (errno) proven; `PHASE_11C_FORMAT_SIGN_EDGE` (val1=0/val2<0) confirmed; Z ≈ 9.17 m/s² flat-bench | `devkit_uart_producer.c` case `'t':`; `devkit_app_state.c` recognition | Phase 11D implementation; Phase 11E hardware evidence. Adapter-level only: no units token, no calibration, no sensor identity in response. |

### D.2 Open Adapter questions that affect Framework design

| Question | Status | Impact on Framework |
|---|---|---|
| Explicit `robotos_adapter.h` aggregate header | Not authored; Adapter surface is implicit (scattered across `core/` + `platform/` + devkit patterns) | Framework would today include individual headers; a future Adapter spec phase could consolidate this; Phase 12A does not author it |
| `ROBOTOS_EVENT_USER` range reservation for Framework | Not reserved; Framework event types would use `>= 100`; risk of collision with Application-layer types | A Framework event type range sub-allocation plan belongs in Phase 12B |
| Pool / slab allocator | Flagged at Phase 11A §D as "open question"; the fixed-capacity event ring may be the only pool primitive | Framework objects that need dynamic or pool allocation would be blocked until this is decided; statically-allocated Framework objects avoid the question |
| Dispatch budget sufficiency | `MAX_EVENTS_PER_TICK=1` profiled only under devkit workloads (peak=5 at Phase 9G, peak=2 at Phase 11E); Framework-level multiple periodic services have not been profiled | If Framework needs >1 event-type dispatch per tick, Scheduler 7A/7B assessment is required; currently DEFER |
| SPI / ADC sensor sub-classes | Uncharacterized; only I2C/accelerometer proven at Adapter level | Framework sensor abstraction with >1 sensor type would depend on at least one additional sub-class being characterized |

---

## E. Candidate Framework API Domains

Evaluation of candidate Framework API domains. For each: purpose, Adapter
dependencies, evidence support, risk, scheduling rationale, first-candidate
assessment, and non-goals.

Phase 12A evaluates these as planning candidates only. **No domain is
implemented here.**

---

### E.1 Framework State-Machine Abstraction

**Purpose:** Generic, reusable FSM with typed states, typed events, transition
table, and audit counters. Allows any Framework subsystem (or Application)
to define its state behavior without coupling to `devkit_app_state` logic.

**Adapter dependencies:** Core event dispatch (`robotos_core_post_event()`,
`robotos_core_register_event_handler()`); time/tick (for timestamp_tick field
in events); critical section (for ISR-safe state query).

**Evidence support:** Strong. `devkit_app_state.c` is a working FSM prototype
with IDLE/ARMED/ACTIVE states, event-driven transitions, and `uart`/
`transitions`/`ignored` counters. The event dispatch path is proven across
Phase 9A–11E. The FSM object model required by Framework is a generalization
of an already-validated pattern — not a new architectural primitive.

**Risk:** Moderate. The main risk is over-generalization: designing a generic
FSM API before the product state machine semantics are known can result in an
abstraction that does not fit the actual application (nested states, history
states, hierarchical FSMs). The mitigation is to scope Phase 12B to a minimal
flat FSM only, deferring hierarchy and concurrency.

**Why now:** Has the most Adapter evidence of any Framework candidate. The
`devkit_app_state` pattern is directly reusable as a design reference without
promoting it to Framework. No new hardware required. Does not require
Scheduler 7A/7B (single FSM + single event per tick fits `MAX_EVENTS_PER_TICK=1`
at current workloads). Avoids product semantics: a generic FSM API does not
commit to CNC/printer/arm vocabulary.

**Why later (nested/hierarchical FSM):** Nested states, history, concurrency
require product-defined semantics. Deferred to a later sub-phase after the
flat FSM is proven.

**Likely first implementation candidate:** **Yes — recommended first slice
(see §F).**

**Non-goals for first slice:** No hierarchical FSM; no history state; no
concurrent FSM instances in a single tick; no FSM persistence/serialization;
no GUI tooling.

---

### E.2 Timer / Service Abstraction

**Purpose:** Periodic and one-shot Framework-level service callbacks driven
by tick events. Allows Framework subsystems to schedule periodic work (sensor
polling, PID update, status broadcast) at defined tick rates without each
subsystem coupling to the raw timer-producer pattern.

**Adapter dependencies:** `devkit_timer_producer.c` event pattern; timer event
type registration (`robotos_core_register_event_handler()`); time/tick.

**Evidence support:** Good. The timer producer pattern is proven across Phase
6G–11E; periodic event delivery is reliable; Phase 11E `peak=2 dropped=0`
under `T T ?` confirms the timer events do not saturate the queue. The
`devkit_runtime_run()` tick loop is the correct service invocation model.

**Risk:** Moderate. A single `MAX_EVENTS_PER_TICK=1` budget means only one
Framework service event drains per tick. Multiple simultaneous periodic
services (sensor poll at 10 Hz, status at 1 Hz, PID at 50 Hz) would post
multiple events per tick, potentially exceeding the budget and triggering
backpressure. This makes timer/service abstraction schedule-budget-coupled:
if multiple services are registered, the Scheduler 7A/7B question cannot stay
DEFER indefinitely.

**Why now:** Timer substrate is proven; the pattern is well-understood.
Single-service timer abstraction (one periodic Framework service) fits the
current budget without reopening Scheduler.

**Why later (multi-service):** Multi-service scheduling requires either raising
`MAX_EVENTS_PER_TICK` (Scheduler 7A) or a service multiplexer that bundles
multiple service invocations into a single event (Scheduler 7B-adjacent). Both
require explicit user decision.

**Likely first implementation candidate:** Second priority — after FSM
abstraction establishes the object model convention.

**Non-goals:** Not an RTOS scheduler; not a hardware timer multiplexer; not a
real-time deadline monitor.

---

### E.3 Sensor Abstraction

**Purpose:** Typed sensor channel API hiding driver details from Framework
consumers. Provides a channel-typed read (`ACCEL_XYZ`, `TEMP`, etc.), a
format/unit contract at Framework level, and an error classification that
goes beyond raw errno.

**Adapter dependencies:** T/lis2dh driver-dependent read (Phase 11D/11E);
I2C bus access proven; `struct sensor_value` integer-fractional path proven.

**Evidence support:** Partial. Only one sensor type (I2C accelerometer) is
characterized at Adapter level. The raw-read pattern is proven; the
`struct sensor_value` format is proven. However, the Framework-level unit/
calibration ownership question from Phase 11A §E (raw = Adapter; calibrated =
Framework) is unresolved. Designing a Framework sensor abstraction from one
Adapter data point risks creating an API that does not generalize to a second
sensor type (gyroscope SPI, ADC temperature, ultrasonic distance).

**Risk:** High if scoped too broadly. A minimal "read last raw value for one
channel" API is achievable with current evidence; a full sensor channel
abstraction with calibration, units, and multi-sensor registration is premature
with one data point.

**Why now (minimal form):** The `T` command proves the Adapter-level sensor
read path. A minimal Framework sensor API could be a thin wrapper that
produces a typed `robotos_fw_sensor_value_t` (with signed integer + 6-digit
fraction) from the Adapter raw `struct sensor_value`, without adding
calibration or units.

**Why later (full form):** Full multi-sensor, multi-channel, calibrated sensor
abstraction needs at least two distinct Adapter sensor types characterized
(e.g. I2C accel + ADC temperature) to validate the abstraction is general.
Calibration ownership is a product-dependent decision.

**Likely first implementation candidate:** Third priority — after FSM
abstraction and timer/service pattern are established.

**Non-goals for first slice:** No calibration; no physical unit conversion
(m/s² vs g vs raw counts is Adapter-level contract preserved as-is from
Phase 11C); no sensor fusion; no multi-sensor aggregation.

---

### E.4 Actuator Abstraction (Stepper / Servo / DC Motor)

**Purpose:** Typed actuator command API for discrete step/position/duty-cycle
control. The foundational output primitive for any motion-control use case.

**Adapter dependencies:** GPIO output (LED toggle proven); timer events
(for step-pulse generation); SPI/ADC (for encoder feedback, if closed-loop).
A stepper driver would require a dedicated step/direction GPIO pair plus a
timer ISR pattern for pulse generation — neither is characterized at Adapter
level.

**Evidence support:** Weak. GPIO output is proven only via a stateless LED
toggle (`devkit_status_led_toggle()`). No step/direction GPIO pair is wired.
No PWM timer output has been characterized. No encoder input has been
characterized. The jump from LED toggle to stepper step-pulse generation
requires multiple new Adapter characterizations (timer ISR at step rate,
GPIO write at ISR speed, direction pin).

**Risk:** High. No actuator hardware has been characterized. This domain
represents the largest hardware gap and would require new Zephyr Kconfig
(`CONFIG_PWM`, `CONFIG_COUNTER`, `CONFIG_STEPPER`), DTS pin allocation, and
possibly new hardware purchase.

**Why now:** NOT recommended as first Framework slice. The Adapter gap for
actuators is larger than for FSM, timer service, or sensor.

**Why later:** After GPIO output characterization is extended (step/dir pin
pair, timer-at-step-rate ISR), and after at least one actuator type (stepper
or servo) is driven from a Zephyr driver or GPIO-bit-bang probe at Adapter
level (a future Phase 13-class track analogous to Phase 11A–11E for sensors).

**Likely first implementation candidate:** NOT recommended for Phase 12B.
Deferred to Phase 13+ class (actuator probe track).

**Non-goals:** No PID loop in first actuator slice; no encoder feedback in
first slice; no multi-axis coordination.

---

### E.5 Endstop / Limit-Switch Abstraction

**Purpose:** GPIO input edge detection for limit switches with debounce,
named positions, and fault injection contract. The safety boundary input for
motion-control use cases.

**Adapter dependencies:** GPIO input event source (button producer pattern;
Phase 9A–C); ISR-safe event posting; debounce proven.

**Evidence support:** Moderate. The button/GPIO input is the most deeply
proven Adapter input primitive. The endstop abstraction would be a
generalization of `devkit_button_producer.c` to support named limit inputs
and distinct active-high/active-low polarity.

**Risk:** Moderate. Endstop abstraction is most useful when paired with
actuator motion; without actuator abstraction (§E.4), an endstop API is an
orphaned input event source with no consumer. The risk of building endstop
API before actuator is creating a well-designed input abstraction that
remains unused until Phase 13+.

**Why now:** GPIO input is well-proven and low-risk to abstract. No new
hardware required (any GPIO input pin can serve as a limit switch input).

**Why later:** Maximum value requires co-design with the actuator abstraction
(§E.4). Building it in isolation before §E.4 is possible but produces an
API whose consumer is unknown.

**Likely first implementation candidate:** NOT recommended as first slice.
Pair with actuator abstraction in Phase 13+.

**Non-goals:** Not a hardware watchdog; not a safety interlock in the IEC
61508 sense; not a multi-axis endstop coordinator.

---

### E.6 PID / Control-Loop Abstraction

**Purpose:** Proportional-integral-derivative control loop for closed-loop
motion control. Standard feedback primitive for any servo or velocity-
controlled system.

**Adapter dependencies:** Sensor abstraction (feedback input); actuator
abstraction (control output); timer service (periodic update at fixed rate).
All three are either unbuilt (actuator) or minimally characterized (sensor:
one type only).

**Evidence support:** Insufficient. PID requires sensor abstraction + actuator
abstraction + timer service all working together at Framework level. None of
these are yet implemented or even specified at Framework level. Designing PID
abstraction before its three dependencies exist is speculative.

**Why now:** NOT recommended.

**Why later:** Phase 16+ class. The dependency chain is: FSM abstraction
→ timer service → sensor abstraction → actuator abstraction → PID. Each step
requires its own proof. PID cannot be meaningfully specified before sensor
and actuator are Framework-level proven.

**Likely first implementation candidate:** DEFERRED indefinitely until
dependency chain is closed.

**Non-goals:** Not feedforward; not model predictive; not multi-DOF coupled
control.

---

### E.7 Motion Primitive / Trajectory Abstraction

**Purpose:** Sequences of actuator commands (moves, arcs, delays, synchronized
multi-axis) for CNC/3D-printer-style motion planning.

**Adapter dependencies:** PID or open-loop actuator; sensor feedback; trajectory
buffering (pool/slab question); product choice (G-code? custom protocol?).

**Evidence support:** Insufficient. Requires product choice before trajectory
semantics can be defined. The pool/slab architectural question (Phase 11A §D)
is a blocking sub-question for trajectory buffering.

**Why now:** NOT recommended.

**Why later:** Phase 17+ class; product-dependent; pool/slab question must be
resolved first.

**Likely first implementation candidate:** DEFERRED; requires product direction.

**Non-goals:** Not a G-code parser; not CAM output; not real-time interpolation.

---

### E.8 Fault / Safety Abstraction

**Purpose:** Framework-level fault classification, recovery policy, and safe-
state FSM. Provides a richer fault model on top of `robotos_platform_fault.h`
`(INFO/WARNING/ERROR/FATAL`), adds recovery actions (retry, reset, safe-state
transition), and classifies fault sources (sensor failure, actuator overload,
communication timeout).

**Adapter dependencies:** `robotos_platform_fault_report()` (Phase 5C);
RTT telemetry / fault observability; existing CFSR/HFSR sampling. All proven.
No new hardware required.

**Evidence support:** Partial. The platform fault API is proven and stable.
The CFSR/HFSR = 0x00000000 across every evidence phase confirms the base
fault sampling works. However, the Framework-level recovery policy is
application-dependent: "safe state" for a CNC machine differs from "safe
state" for a mobile robot. An abstract fault/safety Framework layer is
achievable; a meaningful policy layer requires product direction.

**Risk:** Moderate. The Adapter fault primitive is clean and stable; wrapping
it in a Framework layer is low risk. The risk is designing recovery policy
without knowing the product safety requirements.

**Why now:** Fault abstraction is closely linked to FSM abstraction (faults
trigger state transitions to a safe state). If FSM abstraction is the first
Framework slice (§F), a minimal fault classification layer can be designed
alongside it, with recovery policy deferred to the product layer.

**Why later (recovery policy):** Recovery policy is product-semantics-dependent.
Deferred until product direction is established.

**Likely first implementation candidate:** Second-tier companion to FSM
abstraction. Does not require separate hardware. The fault severity enum from
`robotos_platform_fault.h` is the natural base for a Framework fault
classifier.

**Non-goals:** Not IEC 61508 functional safety; not hardware watchdog; not
safety-certified; not multi-channel fault voter.

---

### E.9 Framework Observability Hooks

**Purpose:** Framework-layer telemetry complementing the existing
`ROBOTOS_OBS` / `ROBOTOS_FAULT` / `ROBOTOS_PROD` RTT streams. Would add
a `ROBOTOS_FW` stream reporting Framework-level state (active FSM state,
service invocation counts, sensor read rates, fault classification counts).

**Adapter dependencies:** `devkit_observability.c` pattern; RTT log
infrastructure. Both proven.

**Evidence support:** Good for the mechanism (RTT + LOG_INF pattern is stable).
Weak for the content (no Framework objects exist yet to observe; the hook
stream would be empty until Framework modules are implemented).

**Risk:** Low mechanism risk. The risk is designing observability hooks before
the Framework objects they would observe exist, producing a stub stream with
no content.

**Why now:** Observability hook contracts should be planned early so Framework
modules are implemented with hook instrumentation from the start rather than
retrofitted. Designing the hook contract now (in Phase 12B or 12C) is lower
cost than retrofitting later.

**Why later:** Hook contract is driven by which Framework objects exist.
Until FSM + timer service are specified, the hook schema cannot be finalized.

**Likely first implementation candidate:** Third-tier — design alongside
Framework FSM and timer service in Phase 12B/12C so the hook contract is
available for those implementations to consume.

**Non-goals:** Not Prometheus metrics; not distributed tracing; not an
over-the-wire telemetry protocol.

---

## F. Recommended First Framework Slice

### F.1 Decision

**Recommended: Framework State-Machine Abstraction (flat FSM only).**

The recommended first Framework slice for Phase 12B is a docs-only
specification of a generic flat FSM API at Framework level.

### F.2 Rationale

| Criterion | State-Machine | Timer Service | Sensor | Others |
|---|---|---|---|---|
| Smallest useful API | **Best** — states + events + transition table; no driver dep | Good | Moderate (needs sensor value type + error type) | Poor (actuator/PID/motion need more infra) |
| Least hardware purchase | **None** | None | None (uses existing T/lis2dh) | Actuator/endstop need hardware |
| Best reuse of Adapter evidence | **Best** — core event dispatch is the backbone; FSM pattern implicit in `devkit_app_state.c` | Good | Partial (only one sensor type) | Poor |
| Minimal product semantics risk | **Best** — generic FSM has no domain vocabulary | Moderate (service names could carry product semantics) | Moderate (sensor identity could leak product semantics) | High |
| No scheduler/F407 requirement | **Yes** — single FSM + single event per tick fits current budget | Conditional — multi-service timer would need Scheduler | Yes | Varies |
| No new hardware | **Yes** | Yes | Yes | Actuator/endstop: No |

### F.3 Scope of recommended first slice (for Phase 12B planning)

The recommended Phase 12B planning target is:

- **A flat FSM** with a compile-time-defined state table (state enum, event
  enum, transition function table).
- **ISR-safe state query** (current state readable from any context).
- **Thread-context state transition** (triggered by core event dispatch;
  transitions happen in the handler thread, not in ISR).
- **Audit counters** analogous to `devkit_app_state` (`transition_count`,
  `event_count`, `error_count`).
- **No promotion of `devkit_app_state`** — the Framework FSM is a new,
  separate API. `devkit_app_state` remains devkit-local.
- **No implementation in Phase 12A or Phase 12B docs** — Phase 12B is the
  spec/draft phase; implementation is a later Phase 12C-class gate.

### F.4 What the first slice does NOT include

- No hierarchical FSM; no nested states; no history state.
- No concurrent FSM instances.
- No GUI tooling or code generation.
- No product state names (`HOMING`, `CUTTING`, `PRINTING`, etc.).
- No `devkit_app_state` promotion.

---

## G. Framework API Draft Boundary

Phase 12A may outline **conceptual, non-final** example API names to
communicate the surface shape. These are **not** final ABIs, **not**
implemented, and **not** authoritative contracts. They are illustrative
sketches to support Phase 12B scoping.

**Illustrative Framework FSM API names (non-final, Phase 12B will specify):**

```c
/* --- ILLUSTRATIVE ONLY. NON-FINAL. NOT IMPLEMENTED. --- */

/* Opaque FSM object (static allocation; no heap) */
typedef struct robotos_fw_state_machine robotos_fw_state_machine_t;

/* Initialize FSM with compile-time state/event table */
robotos_core_status_t robotos_fw_sm_init(
    robotos_fw_state_machine_t *sm,
    /* state table params TBD in Phase 12B */
);

/* Post an event to the FSM (ISR-safe; routes through core event queue) */
robotos_core_status_t robotos_fw_sm_post_event(
    robotos_fw_state_machine_t *sm,
    uint32_t event_type
);

/* Query current state (ISR-safe read) */
uint32_t robotos_fw_sm_state(const robotos_fw_state_machine_t *sm);

/* Audit counters */
uint32_t robotos_fw_sm_transition_count(const robotos_fw_state_machine_t *sm);
uint32_t robotos_fw_sm_event_count(const robotos_fw_state_machine_t *sm);
uint32_t robotos_fw_sm_error_count(const robotos_fw_state_machine_t *sm);
```

**Illustrative future Framework API name prefixes (non-final; Phase 12B+):**

- `robotos_fw_timer_service_*` — timer/service registration and management
- `robotos_fw_sensor_*` — sensor channel read, last value, error
- `robotos_fw_fault_*` — fault classification, recovery policy
- `robotos_fw_observer_*` — Framework-level observability hooks

**Constraints on all future Framework APIs:**

- No Zephyr-specific types in Framework headers (portability requirement).
- No `devkit_*` types in Framework headers (layer boundary).
- Static allocation only in first slices; heap/pool only after pool/slab
  architectural question is resolved.
- Event types must use `>= ROBOTOS_EVENT_USER` (= 100); a sub-range
  allocation plan is Phase 12B scope.
- No implicit promotion of `devkit_app_state`, `devkit_uart_producer`,
  or any devkit module to Framework status.

**Phase 12A explicitly does not:**

- Create any `framework/` directory.
- Create any `.h` or `.c` Framework source file.
- Claim any API listed above is final or implemented.

---

## H. Interaction With Existing Command Set

- **The current UART command set (`a/s/r/?/x/v/L/d/T`) remains the
  devkit/probe surface.** Phase 12A does not add, remove, or redefine any
  command.

- **Framework APIs are not automatically exposed through new UART commands.**
  If a future phase wants to expose a Framework operation (e.g. query FSM
  state, trigger a service) through the UART interface, that requires a
  separate explicit decision phase (analogous to Phase 10B-{v,L,d}) that
  evaluates the new command against the 12 UART TX scope guards from
  `PHASE_9EZ_CHECKPOINT.md §H`.

- **Future command surface must be decided separately.** The product UART
  vocabulary (if any) is an Application/product-layer concern that requires
  product direction before any command row is added to
  `COMMAND_SET_DRAFT.md` Section A.

- **`T` is not promoted to a Framework sensor API by Phase 12A.** `T` is
  `CLOSED_WITH_HARDWARE_EVIDENCE` at Phase 11E as an Adapter probe command.
  A Framework sensor API is a separate abstraction that Phase 12B+ may plan.
  The two are related (Framework sensor API would consume the same Adapter
  I2C primitive that `T` exercises) but are not the same artifact.

- **All 12 UART TX scope guards from `PHASE_9EZ_CHECKPOINT.md §H` remain
  in force at Phase 12A.** No parser, no shell, no registry, no framing,
  no heap response buffer, no response queue, no ISR TX, no UART abstraction
  promotion, no ACK/retry added by Phase 12A.

---

## I. Remaining Gates

All gates are preserved with no change at Phase 12A:

### I.1 ACTIVE disarm widening

| Aspect | Value |
|---|---|
| Status | **`USER_DECISION_REQUIRED_ACTIVE_DISARM`** (unchanged) |
| Current behavior | `d` from ACTIVE → recognized no-op (`OK disarm no-op state=ACTIVE\r\n`); no transition; no `ignored++` |
| Future option | `d` from ACTIVE → IDLE with `transitions++`; one-line guard widening in `devkit_app_state.c` |
| Phase 12A position | Not started; not authorized; fully decoupled from Framework planning |

### I.2 Scheduler 7A / 7B

| Aspect | Value |
|---|---|
| Status | **`DEFER`** (unchanged from Phase 9E-Z) |
| Phase 12A position | No new workload evidence changes this. Opening Scheduler without a concrete Framework multi-service workload to profile against would be re-litigation. Do not open. |
| Future trigger | When Framework timer service or sensor polling creates a profiled multi-service scenario with observed `dropped > 0` or `backpressure_active = 1` under steady-state Framework load |

### I.3 F407 / custom board

| Aspect | Value |
|---|---|
| Status | **`HOLD/DEFER`** (unchanged from Phase 8A) |
| Phase 12A position | No portability requirement surfaced. Framework API design should be written to avoid `platform/zephyr/`-specific types (a property of good layer discipline, not a portability gate). Do not reopen. |

### I.4 POST_FLASH_AUTOSTART root cause

| Aspect | Value |
|---|---|
| Status | Root cause **`OPEN`**; **`MITIGATED_BY_WORKFLOW`** via Phase 6O sidecar `reset run` |
| Phase 12A position | Non-blocking. Investigation remains optional. Every flash session must continue using the sidecar `reset run` discipline. Plain `west flash` alone is not runtime-start evidence. |

### I.5 Application / product layer

| Aspect | Value |
|---|---|
| Status | **`NOT_STARTED`** (unchanged) |
| Phase 12A position | No product vocabulary has been chosen. Framework API planning (this phase) is the prerequisite for Application planning, not a substitute for it. Application planning requires product direction. |

---

## J. Phase 12B Recommendation

### J.1 Recommended next gate

**Phase 12B — Robot Framework FSM API Draft (docs-only).**

A docs-only phase that specifies the flat FSM API surface in enough detail
to enable a subsequent implementation phase (Phase 12C-class). Pattern mirrors
Phase 11C (spec-freeze before implementation).

### J.2 Entry criteria for Phase 12B

- Phase 12A is `CLOSED_DOCS_ONLY` (this document).
- Explicit user authorization to open Phase 12B.
- No blocking open decision that must be resolved before FSM API can be
  specified (ACTIVE disarm widening, Scheduler, F407 are all decoupled).

### J.3 Exit criteria for Phase 12B

- **Draft API header content** for `robotos_fw_state_machine_*` specified in
  prose (not as a `.h` file; source files are Phase 12C scope).
- **Event type sub-range allocation plan** documented (Framework event types
  are `ROBOTOS_EVENT_USER` + offset; range boundary set).
- **ISR-safety contract** for state query and event post specified.
- **State table representation** decided (compile-time static table preferred
  over runtime registration for first slice; decision documented with rationale).
- **`devkit_app_state` non-promotion** explicitly confirmed: the Framework FSM
  API is a new artifact; `devkit_app_state` is not renamed, moved, or aliased.
- **Optional:** Start of `03_SPECS/FRAMEWORK_API_SURFACE_DRAFT.md` as a
  long-lived spec that will evolve across Phase 12B/12C/12D.
- Phase 12B is `CLOSED_DOCS_ONLY`.

### J.4 Files likely touched by Phase 12B

| File | Change type |
|---|---|
| `02_PHASE_CLOSEOUTS/PHASE_12B_FRAMEWORK_FSM_API_DRAFT.md` | New |
| `03_SPECS/FRAMEWORK_API_SURFACE_DRAFT.md` | New (optional long-lived spec; create in Phase 12B if useful as a living spec) |
| `01_PROGRESS/DEVKIT_PROGRESS_PHASE_11_20.md` | Add Phase 12B index row + section |
| `CURRENT_STATE.md` | Update latest phase |
| `00_INDEX/README.md` | Add Phase 12B closeout link |

### J.5 Validation path for Phase 12B

- `git diff --check`: clean whitespace.
- `git status --short`: changed files are docs-only (`.md` only).
- No source/test/CMake/Zephyr/board/`prj.conf`/script changes.
- No evidence logs changed.
- No `framework/` directory created.
- No `.h` or `.c` Framework file created.
- `devkit_app_state` unchanged.
- All 12 UART TX scope guards intact.
- Phase 12B is `CLOSED_DOCS_ONLY`.

### J.6 Explicit non-goals for Phase 12B

- Does not implement the FSM API (that is Phase 12C scope).
- Does not create a `framework/` source directory or any `.h`/`.c` file.
- Does not modify `devkit_app_state` or any devkit source.
- Does not widen the UART command vocabulary.
- Does not start Application/product semantics.
- Does not reopen Scheduler 7A/7B.
- Does not reopen F407.
- Does not start ACTIVE disarm widening.

---

## K. Scope Guard Restatement

All 12 UART TX scope-guard constraints from
[`PHASE_9EZ_CHECKPOINT.md §H`](PHASE_9EZ_CHECKPOINT.md) are **intact**
at Phase 12A. Phase 12A makes zero source/config/script changes; the
restatement is a continuity record only.

| # | Constraint | Status at Phase 12A |
|---|---|---|
| 1 | No shell / prompt / interactive session semantics | OK — unchanged from Phase 11Z |
| 2 | No parser framework, no token lexer, no grammar | OK — unchanged |
| 3 | No command registry; no dynamic handler registration | OK — unchanged |
| 4 | No multiline / framed protocol; no delimiter framing; no length prefix | OK — unchanged |
| 5 | No heap buffers; all response buffers stack-allocated; fixed 96 B | OK — unchanged |
| 6 | No response queue; TX is synchronous in the handler | OK — unchanged |
| 7 | No UART logging replacement; RTT remains canonical log backend | OK — unchanged |
| 8 | No core / platform UART abstraction | OK — unchanged |
| 9 | No retry / ACK protocol; no sequence number | OK — unchanged |
| 10 | No TX from ISR context | OK — unchanged |
| 11 | No promotion of `devkit_app_state` to Robot Framework | OK — **no Framework exists; this guard is re-affirmed** |
| 12 | Single-byte command vocabulary preserved | OK — unchanged (`a/s/r/?/x/v/L/d/T`) |

Architecture boundary status at Phase 12A:

| Surface | Status |
|---|---|
| `RobotOS_v1.0/core/` | **zero diff** from Phase 9E baseline |
| `RobotOS_v1.0/platform/` | **zero diff** from Phase 9E baseline |
| `RobotOS_v1.0/devkit/src/` | **zero diff** from Phase 11E/11Z baseline |
| `devkit/prj.conf` | Unchanged from Phase 11D delta (`CONFIG_I2C=y` + `CONFIG_SENSOR=y`); Phase 12A does not modify |
| Board DTS, overlay, defconfig | **zero diff** |
| `tests/` | **zero diff** |
| Evidence logs | **zero diff** |
| `DEVKIT_PROGRESS.md` historical master | **zero diff** |

---

## L. What this document does not do

- Does not implement any Framework API.
- Does not create a `framework/` source directory.
- Does not create any `.h` or `.c` Framework file.
- Does not modify `devkit_app_state` or any devkit source.
- Does not modify `core/` or `platform/`.
- Does not change any UART command semantics (`a/s/r/?/x/v/L/d/T` unchanged).
- Does not add any new UART command or command candidate.
- Does not promote `T` to a Framework sensor API.
- Does not promote `devkit_app_state` to Framework.
- Does not widen ACTIVE disarm; `USER_DECISION_REQUIRED_ACTIVE_DISARM` preserved.
- Does not reopen Scheduler 7A/7B; `DEFER` preserved.
- Does not reopen F407 / custom board; `HOLD/DEFER` preserved.
- Does not change POST_FLASH_AUTOSTART status; `OPEN` / `MITIGATED_BY_WORKFLOW` preserved.
- Does not start Application / product semantics.
- Does not authorize any hardware purchase.
- Does not modify any closed phase's evidence or closeout doc.
- Does not modify `DEVKIT_PROGRESS.md` historical master.
- Does not modify any evidence log under `RobotOS_v1.0/devkit/logs/`.
- Does not push.
