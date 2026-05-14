# Devkit Phase Architecture Guide

**Type:** Reading guide / architectural narrative.
**Status:** `DRAFT / NON-FINAL — DOCS-ONLY`.
**Scope:** Explains the meaning, layering, and process pattern of the
RobotOS devkit phase history. **Not a closeout.** **Not a phase truth
artifact.** Does not change any phase status and does not authorize any
implementation.

Phase truth is anchored in
[`../01_PROGRESS/`](../01_PROGRESS/) and
[`../02_PHASE_CLOSEOUTS/`](../02_PHASE_CLOSEOUTS/). Live state is
anchored in [`../../../../CURRENT_STATE.md`](../../../../CURRENT_STATE.md).
**This guide must not be cited as source-of-truth.** When this guide
disagrees with a closeout or progress entry, the closeout / progress
entry wins.

---

## 0. How to use this guide

This document exists for one purpose: **help a new reader pick up the
devkit story without having to linearly read every closeout.** It is
not a replacement for any of:

| Need | Go to |
|---|---|
| Live state ("what is the latest closed phase, what is HOLD?") | [`../../../../CURRENT_STATE.md`](../../../../CURRENT_STATE.md) |
| Phase narrative index (Phase 11–20 window) | [`../01_PROGRESS/DEVKIT_PROGRESS_PHASE_11_20.md`](../01_PROGRESS/DEVKIT_PROGRESS_PHASE_11_20.md) |
| Phase narrative index (Phase 10 window) | [`../01_PROGRESS/DEVKIT_PROGRESS_PHASE_10.md`](../01_PROGRESS/DEVKIT_PROGRESS_PHASE_10.md) |
| Phase narrative index (Phase 1–9 historical master) | [`../01_PROGRESS/DEVKIT_PROGRESS.md`](../01_PROGRESS/DEVKIT_PROGRESS.md) |
| Per-phase evidence, scope-guard, verdict | [`../02_PHASE_CLOSEOUTS/`](../02_PHASE_CLOSEOUTS/) |
| Long-lived specs (command set, telemetry, framework FSM, probe translator plans) | [`../03_SPECS/`](../03_SPECS/) |
| Raw RTT / host transcripts | `../logs/` (indexed in `../logs/INDEX.md`) |
| Docs directory map | [`../00_INDEX/README.md`](../00_INDEX/README.md) |

This guide explains **meaning and flow**; the directories above carry
**evidence and truth**.

---

## 1. RobotOS devkit big picture

The RobotOS devkit is the bring-up vehicle for the RobotOS layered
runtime on a real Cortex-M target. It is not a product. It is the
controlled environment in which RobotOS layers (platform adapter,
core, framework, application) are admitted one stage at a time, with
evidence at each stage, before any product/public surface is opened.

### 1.1 Layered architecture (current, as of Phase 12M / 12N-pre)

```
+-----------------------------------------------------------+
| Future product / workload layer                           |  <-- NOT OPENED.
|  - public command contract                                |      HOLD by Phase
|  - versioning / framing / security / test contract        |      12N-pre.
|  (Phase 12N-pre-2 successor planning gate required        |
|   before any product UART/API expansion.)                 |
+-----------------------------------------------------------+
| Application layer                                          |  app/probe_translator/
|  - probe_translator (first application candidate)         |  - probe_translator.{c,h}
|  - host-test-validated FSM + adapter-event mapping        |  - README.md
|  - product-neutral; synthetic events at host depth        |
+-----------------------------------------------------------+
| Framework layer                                            |  framework/
|  - robotos_fw_fsm.{c,h}      (flat table-driven FSM)      |  - robotos_fw_fsm.{c,h}
|  - robotos_fw_event_bridge.{c,h} (Adapter -> FSM mapping) |  - robotos_fw_event_bridge.{c,h}
|  - host-test-validated; ABI not stable                    |
+-----------------------------------------------------------+
| Devkit runtime layer                                       |  RobotOS_v1.0/devkit/
|  - devkit_runtime.c          (boot / wiring)              |  - devkit/src/
|  - devkit_app_state.c        (UART/button -> state)       |
|  - devkit_uart_producer.{c,h} (UART RX/TX, frozen surface)|
|  - devkit_probe_adapter.{c,h} (Phase 12L runtime entry)   |
|  - command set a/s/r/?/x/v/L/d/T (devkit-evidence; frozen)|
+-----------------------------------------------------------+
| RobotOS core layer                                         |  RobotOS_v1.0/core/
|  - event queue, dispatcher, scheduler policies            |
|  - status / handler registration boundary                 |
+-----------------------------------------------------------+
| Platform adapter layer                                     |  RobotOS_v1.0/platform/
|  - time, assert/fault, critical section/ISR lock         |
|  - ISR-safe producer contract                             |
+-----------------------------------------------------------+
| Zephyr RTOS + STM32 HAL                                    |  Zephyr v3.6.0
|  - drivers, scheduler, UART, GPIO, timer, RTT             |  Zephyr SDK 0.17.0
|  - board: stm32f411e_disco rev D                          |
+-----------------------------------------------------------+
| Hardware: STM32F411E-DISCO board                           |  + onboard MEMS
|  - button, LED, UART pins, onboard MEMS accelerometer     |    accelerometer
+-----------------------------------------------------------+
        ^                              ^
        |                              |
   UART (frozen response forms)    RTT / SEGGER
   - 'a' -> "OK state=ARMED"       - ROBOTOS_OBS / FAULT / PROD / BTN
   - 's' -> "OK state=ACTIVE"      - ROBOTOS_PROBE (Phase 12L+)
   - 'r' -> "OK state=IDLE"        - evidence channel only;
   - '?' -> STATE telemetry          NOT a product channel
```

**Things this diagram intentionally does not show:**

- F407 / custom board (HOLD/DEFER; not validated)
- Scheduler 7A/7B (DEFER; not opened)
- Product command mapping (HOLD; not authorized)
- `RobotOS_v1.0/examples/` (NOT_STARTED)
- Multi-product coordination (NOT_STARTED)
- Legacy `RobotOS_v1.0/src/` + `include/robotos/` (marked frozen /
  non-authoritative at Phase 12D-pre)

### 1.2 Boundary rule (locked at Phase 12N-pre)

Devkit-evidence does **not** auto-promote to product. Per
[`PRODUCT_WORKLOAD_COMMAND_ADMISSION_PLAN`](../03_SPECS/PRODUCT_WORKLOAD_COMMAND_ADMISSION_PLAN.md):

- The UART command set `a/s/r/?/x/v/L/d/T` is **devkit demo / evidence**, not a product/public command contract.
- The UART TX response shapes (`OK state=...`, `STATE state=... transitions=... button=... uart=... ignored=...`) are frozen at Phase 9E / 10B / 11D shape.
- The `ROBOTOS_PROBE` snapshot is **RTT-only**.
- Any product mapping requires a `Phase 12N-pre-2` successor planning gate with versioning, framing, security, and test contract first.

---

## 2. Phase families (grouping by meaning, not by number)

Phase numbers are sequential, but the **family** a phase belongs to is
what determines its architectural meaning. The families currently
present in the repo are:

| Family | Phases | Architectural meaning |
|---|---|---|
| Bring-up / runtime visibility | 3A, 3B | Runtime skeleton + devkit hardening; first proof that a RobotOS-shaped runtime boots and is observable on the target. |
| Core contract | 4A–4L | Core bootstrap, event queue contract, event dispatch, event ingestion API, scheduler policy stubs, handler registration boundary. Builds the *contract surface* the rest of the system relies on. |
| Platform adapter boundary | 5A–5G | Zephyr/platform adapter seed, time boundary, assert/fault boundary, critical section / ISR lock, dispatcher pop / handler split, ISR-safe producer audit. Locks the platform side of the boundary so core stays platform-agnostic. |
| Observability / stress / mixed policy | 6A–6Z, 6O | Event smoke tests, burst / backpressure, queue full / drop, invalid rejection, throttled producer, ISR/timer producer smoke + stress, mixed event policy, runtime observability surfacing, fault observability, producer realism, reusable RTT streaming capture harness. Builds *diagnostic confidence* in the contract surface. |
| Scheduler 7A/7B | (Phase 7) | **DEFER.** Not opened. Carried as a recurring open gate. |
| Workload loop | 9A-A, 9A-B, 9A-C, 9B, 9C, 9D, 9E, 9E-Z, 9G, 9Z | First real host↔board workload: button EXTI producer, debounce, gate Phase 6I startup burst, UART RX producer, minimal devkit application state machine, workload demo runbook, UART TX minimal response, command-loop direction guard, bounded UART burst characterization, workload-branch checkpoint. |
| Product command set buildout | 10A, 10B-v, 10B-L, 10B-d, 10C | Command-set planning, individual command admission (`v`, `L`, `d`), command-set checkpoint. Operates inside the devkit-evidence boundary. |
| Sensor probe track | 11A, 11B, 11C, 11D, 11E, 11Z | Adapter / sensor surface decision, device-driver feasibility, on-board MEMS accelerometer probe spec → implementation → hardware evidence → command-set checkpoint. **Shelved as complete** at Phase 11Z. |
| Framework planning | 12A, 12B, 12C, 12D-pre, 12D | Robot Framework API surface planning, FSM API draft, event bridge / status model confirmation, legacy scaffold disposition, FSM header stub. |
| Framework implementation (host) | 12E-pre, 12E, 12F-pre, 12F | FSM `.c` body + host test, application bridge planning + host prototype. **Host-test-validated; no devkit/hardware integration.** |
| Application boundary planning | 12G-pre, 12G, 12H-pre, 12H | Devkit integration-mode decision (selected SEPARATE APPLICATION), application boundary planning (selected `RobotOS_v1.0/app/<product>/`), first application candidate selection (selected `probe_translator`), probe translator skeleton planning. |
| Application implementation (host) | 12I-pre, 12I, 12J-pre, 12J | Probe translator host prototype + FAULT block extension. **Host-test-validated; no devkit/hardware integration at 12I/12J.** |
| Application admission chain (devkit) | 12J-Z, 12K-pre, 12K, 12K-Z, 12L-pre, 12L, 12L-Z, 12M-pre, 12M, 12M-Z, 12N-pre | The staged admission of the host-validated application into the devkit Zephyr build, then into devkit runtime wiring, then to hardware evidence, with explicit guards between each stage and an explicit HOLD before any product mapping. |
| Reserved / future | 20Z, future windows | Reserved checkpoint slot; future progress windows (21-30, 31-40) created lazily when their window opens. |

The boundaries between families are deliberate. A phase rarely
crosses family boundaries. When it does (e.g. `12K` adds source-built
files to the devkit Zephyr build, crossing the framework/devkit
boundary), the crossing is explicit, planned by a `-pre` phase, and
guarded by a `-Z` phase afterwards.

---

## 3. Major phase meaning (selected)

### 3.1 Phase 3 — bring-up / runtime visibility foundation

Phase 3A (Runtime Skeleton) and 3B (Devkit Hardening) bring the
runtime up on the target and make it observable. Without Phase 3,
nothing later in the stack has a host to live on.

### 3.2 Phase 4 — core contract / event dispatch foundation

Phase 4 builds the core contract surface in stages:

- **4A–4C** — core bootstrap, contract hardening, host/core contract tests.
- **4D–4F** — event queue contract stub, dispatch stub, ingestion API.
- **4G–4L** — scheduler tick policy stub, handler policy / handler registration boundary, scheduler admission policy stub, budget/backpressure, producer throttle, advisory retry decision policy.

Phase 4 is the layer that makes the rest of RobotOS *talk to itself*.
Almost everything later either depends on or guards a contract that
Phase 4 established.

### 3.3 Phase 5 — platform adapter boundary

Phase 5 protects the boundary between RobotOS core and Zephyr/STM32.
5A seeds the adapter, 5B locks the time boundary, 5C locks
assert/fault, 5D locks critical section / ISR lock, 5E applies that
to the core queue state, 5F splits dispatcher-pop from handler, 5G
audits the ISR-safe producer contract. If any future phase widens
core's platform assumptions, Phase 5 is where it must be re-audited.

### 3.4 Phase 6 — observability, stress, mixed policy

Phase 6 takes the contract surface from Phase 4/5 and *stresses* it
in a controlled way:

- **6A–6E, 6F** — devkit event smoke, burst/backpressure, queue full / drop, invalid rejection, throttled producer, mixed event policy.
- **6G–6I, 6M** — ISR / timer producer smoke, stress-lite, timer producer queue-pressure stress, producer realism diagnostic.
- **6J–6L, 6Z, 6O** — observability and contract stress expansion, runtime observability surfacing, fault observability integration, RTT closeout, reusable RTT streaming capture harness.

After Phase 6, the team has *diagnostic confidence* in the contract
surface — not just a contract, but a contract that has been observed
under load with structured telemetry.

### 3.5 Phase 9 — first real host↔board workload loop

Phase 9 is where the runtime first becomes a *device the host can
talk to*:

- **9A-A/B/C** — button EXTI producer, debounce refinement, gating Phase 6I startup burst.
- **9B** — UART RX producer.
- **9C** — devkit minimal application state machine (`devkit_app_state`). This module is the durable owner of devkit-level command/state transitions and remains authoritative across all later phases.
- **9D** — workload demo script & runbook (see [`../03_SPECS/WORKLOAD_DEMO_9D.md`](../03_SPECS/WORKLOAD_DEMO_9D.md)).
- **9E** — UART TX minimal response. Locks the UART TX response shape that all later phases hold zero-diff (`OK state=...` / `STATE state=... transitions=... button=... uart=... ignored=...`).
- **9E-Z** — command-loop direction guard.
- **9G** — bounded UART burst characterization (see [`../02_PHASE_CLOSEOUTS/PHASE_9G_CLOSE.md`](../02_PHASE_CLOSEOUTS/PHASE_9G_CLOSE.md)).
- **9Z** — workload-branch checkpoint review.

### 3.6 Phase 12 — application/probe-translator admission chain

Phase 12 is the longest and most structurally important chain in the
repo. It is treated separately in §4.

### 3.7 Phases not opened

- **Phase 7 (Scheduler 7A/7B)** — recorded repeatedly as `DEFER` in carry-forward gates. Not opened. Not planned at the time of this guide.
- **Phase 8** — does not appear in the phase log. Reserved.

---

## 4. Phase 12 deep explanation — the application admission chain

Phase 12 takes a host-validated application (`probe_translator`) and
admits it into the devkit, one boundary at a time, never crossing
two boundaries in a single phase. The chain is the canonical example
of the RobotOS phase pattern.

### 4.1 Chain diagram

```
+-----------+   +-----------+   +-----------+   +-----------+   +-----------+
| Framework |   | App host  |   | Zephyr    |   | Devkit    |   | Hardware  |
| planning  |-->| prototype |-->| build     |-->| runtime   |-->| evidence  |
| & host    |   |           |   | admission |   | admission |   |           |
| validation|   |           |   |           |   |           |   |           |
+-----------+   +-----------+   +-----------+   +-----------+   +-----------+
   12A-12F        12H-12J          12K            12L            12M
                                                                  |
                                                                  v
                                                       +---------------------+
                                                       | Product/workload    |
                                                       | command admission   |
                                                       | (HOLD at 12N-pre)   |
                                                       +---------------------+
                                                                12N-pre

Each major step is bracketed by:
  -pre  : docs-only admission plan (no code change)
   X    : the bounded implementation phase
  -Z    : checkpoint / non-claim guard (lock the boundary; forbid inference)
```

### 4.2 Phase 12 phase-by-phase

| Phase | Status | What it proved | What it does NOT prove |
|---|---|---|---|
| 12A | `CLOSED_DOCS_ONLY` | Framework API surface planning gate; 9 candidate domains evaluated; FSM recommended as first slice. | Any Framework code; ABI; layer ownership. |
| 12B | `CLOSED_DOCS_ONLY` | Flat table-driven FSM model; draft `robotos_fw_fsm_*` API. | Implementation; bridge contract; status model (12C). |
| 12C | `CLOSED_DOCS_ONLY` | Event bridge + status model confirmed; evaluation order corrected. | Any Framework code. |
| 12D-pre | `CLOSED_DOCS_ONLY` | Legacy `RobotOS_v1.0/src/` + `include/robotos/` marked frozen / non-authoritative; future active Framework path = `RobotOS_v1.0/framework/`. | Any new Framework code. |
| 12D | `CLOSED_HEADER_STUB_ONLY` | `framework/robotos_fw_fsm.h` header stub + `framework/README.md`. **Declarations only.** | `.c` body; CMake admission; devkit integration. |
| 12E-pre | `CLOSED_DOCS_ONLY` | Host-unit-test consumer plan; 21 runtime + 4 review cases mapped. | Any code. |
| 12E | `CLOSED_WITH_HOST_TEST_EVIDENCE` | `framework/robotos_fw_fsm.c` + `tests/host/test_robotos_fw_fsm.c`; **93/93 assertions PASS**, 21/21 host regression. | Devkit integration; hardware. |
| 12F-pre | `CLOSED_DOCS_ONLY` | Application Bridge planning; recommended host-only bridge prototype. | Any code. |
| 12F | `CLOSED_WITH_HOST_TEST_EVIDENCE` | `framework/robotos_fw_event_bridge.{c,h}` + host test; **103/103 assertions PASS**, 22/22 regression. Bridge §5 names + signatures `LOCKED-AT-12F`. | Devkit integration; hardware; ABI memory-layout stability. |
| 12G-pre | `CLOSED_DOCS_ONLY` | Devkit integration-mode decision — `SEPARATE APPLICATION MODE` selected over HOLD / SHADOW / REPLACEMENT / HOST-ONLY EXTENSION. | Any application directory or code. |
| 12G | `CLOSED_DOCS_ONLY` | Application directory path selected = `RobotOS_v1.0/app/<product>/`. | Any application code; first `<product>` identity (resolved at 12H-pre). |
| 12H-pre | `CLOSED_DOCS_ONLY` | First application candidate = `probe_translator`. | Any application code. |
| 12H | `CLOSED_DOCS_ONLY` | `probe_translator` skeleton planning (Variant 1); future file set, public API names, state/event/adapter vocabularies, 16-case host test plan locked at planning depth. `app/probe_translator/` **NOT created**. | Any code. |
| 12I-pre | `CLOSED_DOCS_ONLY` | All Phase 12H open decisions resolved: numeric IDs, FAULT block deferred, embed-by-value, 15-case test plan, Option A CMake, 14 exit gates. | Any code. |
| 12I | `CLOSED_WITH_HOST_TEST_EVIDENCE` | First application harness materialized: `app/probe_translator/probe_translator.{c,h}` + `README.md` + `tests/host/test_app_probe_translator_mapping.c`; **70/70 assertions PASS**, 23/23 host regression. | FAULT block; devkit integration; hardware. |
| 12J-pre | `CLOSED_DOCS_ONLY` | FAULT block extension plan: `STATE_FAULT=4u`, `EVT_FAULT=5u`, `ADAPTER_TYPE_FAULT=3u`, `ADAPTER_ARG_ANY=0xFFFFFFFFu`; transition rows 5–9 + mapping row 4 wildcard. | Any code. |
| 12J | `CLOSED_WITH_HOST_TEST_EVIDENCE` | FAULT block implemented; TC16–TC24 added; **132/132 assertions PASS**, 23/23 ctest. | Devkit integration; hardware. |
| **12J-Z** | `CLOSED_DOCS_ONLY` | Behavior baseline locked at `2d9f832`; app-layer complete at host depth. **Guard:** no devkit/Zephyr/UART/hardware integration claimed. | — |
| 12K-pre | `CLOSED_DOCS_ONLY` | Zephyr build-only admission plan; Option A CMake strategy; 10 dependency audit + 10 validation gates. | Any code. |
| 12K | `CLOSED_WITH_BUILD_EVIDENCE` | Additive `devkit/CMakeLists.txt` admits `framework/*.c` + `app/probe_translator/probe_translator.c`; `west build --pristine` PASS for `stm32f411e_disco` rev D; FLASH 41,528 B / 7.92%, RAM 12,352 B / 9.42%; host regression 23/23. | Runtime wiring; hardware; UART change. |
| **12K-Z** | `CLOSED_DOCS_ONLY` | Build-admission guard. **Guard:** `probe_translator` is build-admitted, not runtime-owned by devkit. No call path from `devkit_runtime` / `devkit_app_state` to `probe_translator` exists. | — |
| 12L-pre | `CLOSED_DOCS_ONLY` | Runtime admission plan; selected Candidate B — new `devkit_probe_adapter` module; command mapping locked. | Any code. |
| 12L | `CLOSED_WITH_BUILD_EVIDENCE` | `devkit/src/devkit_probe_adapter.{c,h}` owns static `probe_translator_t`; 3-function API (`_init` / `_dispatch(type,arg0)` / `_log_snapshot`); wired from `devkit_runtime_init()` and `devkit_app_state.c` at accepted `'a'/'s'/'r'/'d'` UART + button transitions. `west build --pristine` PASS; FLASH 43,384 B / 8.27% (+1,856 B); RAM 12,480 B / 9.52% (+128 B); host regression 23/23. | Hardware. |
| **12L-Z** | `CLOSED_DOCS_ONLY` | Runtime-admission / hardware-validation guard. **Guard:** runtime-admitted at build depth, **not hardware-proven**. No board flash / RTT / J-Link / hardware run was performed. | — |
| 12M-pre | `CLOSED_DOCS_ONLY` | Hardware validation plan: flash command, RTT capture, `run_phase12m_probe_demo.ps1`, expected RTT patterns (`state=1→2→3→1`; `ROBOTOS_PROBE init ok`), 15 validation gates, `_SEGGER_RTT` pre-flight. | Any hardware session. |
| 12M | `CLOSED_WITH_HARDWARE_EVIDENCE` | Phase 12L firmware flashed to STM32F411E-DISCO; RTT + UART evidence captured; probe FSM `IDLE→READY→ACTIVE→IDLE` progression proven on hardware. `_SEGGER_RTT=0x20000b38` (+0x68 vs Phase 11E). `CFSR=0x00000000 HFSR=0x00000000` (25×). UART TX byte-matched Phase 9E baseline. 15/15 validation gates PASS. RTT log: `devkit/logs/phase_12M_rtt_2026-05-14.txt` (44,341 bytes). | Product/public command mapping; F407; other boards. |
| **12M-Z** | `CLOSED_DOCS_ONLY` | Hardware-validation / product-mapping guard. **Guard:** Phase 12M proves controlled `a s r ?` sequence on STM32F411E-DISCO, **not** a product/public command mapping. Direct Phase 12N implementation **not authorized** without Phase 12N-pre planning gate. | — |
| 12N-pre | `CLOSED_DOCS_ONLY` | Product/workload command admission taxonomy. 5 candidate strategies evaluated; `Option 2 — docs-only taxonomy spec` selected; `Option 4 — UART public command mapping` and `Option 5 — new protocol surface` **REJECTED at Phase 12N depth**. Locks rule that devkit-evidence does NOT auto-promote to product. | Any product command surface. Phase 12N implementation **NOT authorized**. |

### 4.3 Phase 12 source → product distance

```
host source        |  Zephyr build       |  devkit runtime     |  hardware            |  product/public
(Phase 12E/12F/    |  (Phase 12K)        |  (Phase 12L)        |  (Phase 12M)         |  (HOLD at 12N-pre)
 12I/12J)          |                     |                     |                      |
                   |                     |                     |                      |
gcc 13.3.0 on WSL  |  west build for     |  call paths from    |  flash + RTT + UART  |  versioning,
host-only tests    |  stm32f411e_disco   |  devkit_runtime and |  on STM32F411E-DISCO |  framing, security,
132/132 asserts    |  41-43 KB FLASH     |  devkit_app_state   |  15/15 gates         |  test contract
PASS               |  zero-diff prj.conf |  +1,856 B FLASH     |  CFSR/HFSR=0         |  must be defined
                   |  zero-diff DTS      |  +128 B RAM         |  byte-matched UART   |  before any UART/
                   |                     |  UART zero-diff     |  zero-diff source    |  API expansion
```

Each step is *strictly* narrower than the next, and each step has its
own evidence type. The next step cannot be inferred from the previous.

---

## 5. Evidence ladder

Each level of evidence proves something *and* does not prove
something else. Confusing the two is the most common source of
phase-truth drift.

| Level | Evidence type | What it proves | What it does NOT prove |
|---|---|---|---|
| 0 — Plan (`-pre`) | A docs-only plan with bounded contract, validation gates, file boundary. | The next phase has a complete contract. No ambiguity at the boundary. | That the next phase has been executed. |
| 1 — Host test | `ctest` PASS on Linux/WSL with gcc; in-binary assertions. | Code compiles for host; the C contract behaves as specified under host conditions. | Cross-compile to Zephyr; integration with devkit; hardware behavior. |
| 2 — Zephyr build | `west build --pristine` exit 0 for `stm32f411e_disco`; FLASH/RAM consumed. | Code cross-compiles for the target; CMake admission is correct; linker accepts. | Runtime behavior; integration with devkit_app_state; hardware behavior. |
| 3 — Runtime wiring | New devkit call path; Zephyr build still PASS; UART surface zero-diff; host regression PASS. | The new module is wired into devkit on paper. | Whether it runs correctly on hardware; whether RTT shows expected patterns. |
| 4 — Hardware flash + RTT/UART | Firmware flashed to physical STM32F411E-DISCO; RTT captures expected `state=1→2→3→1` progression; UART TX byte-matches frozen Phase 9E baseline; `CFSR=0x00000000 HFSR=0x00000000`. | Hardware behaves as specified for the controlled sequence. | Other boards; uncontrolled inputs; product/public surface; stress beyond the validated sequence. |
| 5 — Checkpoint (`-Z`) | A docs-only guard that names the *non-claim*. | The boundary is held: future phases must not infer beyond the proven level. | Any new behavior. |

**Rule:** when reasoning about what is proven, walk *up* from the
last `CLOSED_WITH_*_EVIDENCE` phase to the boundary the next `-Z`
guards. Do not walk past the guard.

---

## 6. Status taxonomy

Every phase has a status tag and (when closed) a result tag. These
strings come straight from the closeouts and progress entries — they
are not editorial. Treat them as canonical phase truth.

### 6.1 Phase status (set during the phase lifecycle)

| Status | Meaning |
|---|---|
| `RESERVED / NOT_STARTED` | Scaffold exists; phase not opened; no implementation authority. |
| `NOT_STARTED` | Recognized as a future phase, no execution. |
| `OPEN` | Phase opened; work in progress; no closeout yet. |
| `CLOSED_DOCS_ONLY` | Closed with docs-only deliverables (planning gate, taxonomy spec, checkpoint, direction guard). |
| `CLOSED_HEADER_STUB_ONLY` | Closed with header declarations only (no `.c` body, no CMake admission). |
| `CLOSED_WITH_HOST_TEST_EVIDENCE` | Closed with host (`ctest`) evidence; no Zephyr build / hardware. |
| `CLOSED_WITH_BUILD_EVIDENCE` | Closed with Zephyr `west build` evidence; no hardware run. |
| `CLOSED_WITH_HARDWARE_EVIDENCE` | Closed with flash + RTT/UART evidence on real hardware. |
| `IMPLEMENTATION_CLOSED_HARDWARE_EVIDENCE_PENDING` | Implementation phase closed but the corresponding hardware-evidence phase has not yet been opened (used historically for the Phase 11D → 11E pair). |

### 6.2 Carry-forward gate status (recorded in the open-gates list)

| Status | Meaning |
|---|---|
| `HOLD` | Intentionally not opened. **Not a failure.** A safe default until explicit decision. |
| `DEFER` | Known future work; explicitly out of current scope. |
| `HOLD/DEFER` | Either disposition is acceptable; not currently scoped. |
| `OPEN` | Known issue; not yet resolved. |
| `OPEN / MITIGATED_BY_WORKFLOW` | Known issue; mitigated by operator workflow (e.g. POST_FLASH_AUTOSTART). |
| `NOT_STARTED` | Recognized as work; not in scope. |
| `NOT_STARTED; USER_DECISION_REQUIRED` | Recognized as work; awaiting explicit user direction. |
| `USER_DECISION_REQUIRED_*` | A specific named decision the user must make before this gate moves. |
| `NOT AUTHORIZED` | Implementation is explicitly forbidden until a successor planning gate opens. (Used at 12N for product mapping.) |

### 6.3 Phase suffixes

| Suffix | Meaning |
|---|---|
| `-pre` | A docs-only **admission planning** phase that *precedes* an implementation phase. Locks the contract, validation gates, and file boundary. `-pre` never changes source. |
| `-Z` | A docs-only **checkpoint / non-claim guard** that *follows* an implementation phase. Names what was proven and what was NOT proven. Locks the boundary. `-Z` never changes source. |
| `Z` (standalone, e.g. `9Z`, `11Z`, `20Z`) | A checkpoint phase or a reserved slot. Standalone `Z` is contextual — see the closeout. |

### 6.4 What HOLD is, and is not

`HOLD` is the **safe default**. It is recorded at every checkpoint
where a new boundary could be crossed but has not been. Examples
currently live in the repo:

- Product command mapping — `HOLD` at Phase 12N-pre.
- F407 / custom board — `HOLD/DEFER`.
- Phase 12N implementation — **not authorized**.

`HOLD` is *not* a missing item on a checklist. It is a deliberate
recorded decision that the current evidence does not justify
crossing the next boundary.

---

## 7. Process pattern — `-pre` / implementation / `-Z`

The recurring pattern across Phase 11 and Phase 12 is:

```
                  +----------------------------+
                  |  -pre  (docs-only)         |
                  |  - audit current surface   |
                  |  - lock contract           |
                  |  - lock file boundary      |
                  |  - lock validation gates   |
                  |  - lock evidence policy    |
                  |  - lock zero-diff surfaces |
                  +----------------------------+
                              |
                              v
                  +----------------------------+
                  |  X  (bounded implementation)|
                  |  - only the changes the    |
                  |    -pre authorized         |
                  |  - validation gates run    |
                  |  - evidence captured       |
                  |  - zero-diff held on the   |
                  |    forbidden surfaces      |
                  +----------------------------+
                              |
                              v
                  +----------------------------+
                  |  -Z  (docs-only checkpoint) |
                  |  - lock baseline           |
                  |  - state explicit non-claim|
                  |  - forbid inference past   |
                  |    the proven boundary     |
                  |  - recommend HOLD or next  |
                  |    -pre as default         |
                  +----------------------------+
```

Phase 12K-pre → 12K → 12K-Z → 12L-pre → 12L → 12L-Z → 12M-pre → 12M
→ 12M-Z → 12N-pre is the canonical example. Read those ten
closeouts in order and the pattern is unambiguous.

---

## 8. Module boundary map

Module responsibilities, distilled from the closeouts. **These are
the boundaries that must not move silently.**

| Module | Owner of | NOT responsible for |
|---|---|---|
| `RobotOS_v1.0/devkit/src/devkit_runtime.c` | Devkit boot, devkit-level wiring, periodic devkit-level logging. | Application-level FSM state; product/public command interpretation. |
| `RobotOS_v1.0/devkit/src/devkit_app_state.c` | Devkit-level state machine driven by UART byte + button events. **Authoritative** for devkit-level command/state transitions. Frozen scope-guard #11. | Application-layer FSM; framework FSM internals; product command mapping. |
| `RobotOS_v1.0/devkit/src/devkit_uart_producer.{c,h}` | UART RX byte producer; UART TX response emission with **frozen Phase 9E response shape**. Zero-diff in 12K/12L/12M. | Command-set expansion; product/public command framing. |
| `RobotOS_v1.0/devkit/src/devkit_probe_adapter.{c,h}` (Phase 12L) | The **single** runtime entry point from devkit into `probe_translator`. Owns the static `probe_translator_t`. 3-function API: `_init`, `_dispatch(type,arg0)`, `_log_snapshot`. Header is `<stdint.h>` + `robotos_core.h` only — no `probe_translator.h` in header. | Owning the `probe_translator` semantics (that lives in the app layer); UART surface; FSM table content. |
| `RobotOS_v1.0/app/probe_translator/probe_translator.{c,h}` | Application-layer FSM + adapter-event mapping. Public API `probe_translator_init / _dispatch_adapter_event / _reset / _get_snapshot`. State / event / adapter-key vocabularies locked at 12H/12I. FAULT block at 12J. | Devkit wiring; UART; hardware; product command contract. |
| `RobotOS_v1.0/framework/robotos_fw_fsm.{c,h}` | Flat table-driven FSM contract for the Framework layer. §4 names `LOCKED-AT-12D`. Host-test-validated. ABI not stable. | Application semantics; devkit wiring; bridge mapping (lives in `robotos_fw_event_bridge`). |
| `RobotOS_v1.0/framework/robotos_fw_event_bridge.{c,h}` | Adapter event `(type, arg0)` → Framework event ID mapping. §5 names + signatures `LOCKED-AT-12F`. FIFO first-match, optional `arg0` wildcard. Caller-owned bridge/FSM/mapping. | Heap; UART; devkit/legacy includes; ABI memory-layout stability. |
| `RobotOS_v1.0/core/` | Core event queue, dispatcher, scheduler policies, status, handler registration. Contract surface for the rest of RobotOS. | Platform specifics; application semantics; framework FSM. |
| `RobotOS_v1.0/platform/` | Time, assert/fault, critical section / ISR lock, ISR-safe producer contract. The platform-side of the core/platform boundary. | Core contract definition; application semantics. |
| `RobotOS_v1.0/src/` + `include/robotos/` | **Legacy / frozen / non-authoritative.** Marked at Phase 12D-pre. | Anything. Do not extend; do not depend on for new work. |
| `RobotOS_v1.0/tools/runtime/` | Devkit runtime scripts (RTT capture, demo scripts). | Build authority; phase truth. |
| `RobotOS_v1.0/devkit/logs/` + `RobotOS_v1.0/tests/host/logs/` | **Canonical evidence.** Build transcripts, host transcripts, RTT captures. Cited by closeouts via relative path. | Phase truth (phase truth is in `01_PROGRESS/` + `02_PHASE_CLOSEOUTS/`). |

**Boundaries that must not be crossed accidentally:**

- `devkit/src/*` must not include `framework/*.h` directly except via the dedicated adapter (`devkit_probe_adapter`).
- `framework/*.h` must not include `app/probe_translator/*.h`.
- `app/probe_translator/*.h` must not include devkit / Zephyr / legacy headers.
- `core/` must not include platform-specific or Zephyr-specific headers.
- UART TX response shapes must not change without a phase that explicitly proposes the change against the frozen Phase 9E baseline.

---

## 9. Open / HOLD / DEFER / pending map (as of 2026-05-14)

This table is a snapshot. Always cross-check against
[`../../../../CURRENT_STATE.md`](../../../../CURRENT_STATE.md) before
acting.

| Item | Status | Source |
|---|---|---|
| Devkit baseline (latest hardware-proven) | `CLOSED_WITH_HARDWARE_EVIDENCE` at Phase 12M for the controlled `a s r ?` sequence on STM32F411E-DISCO. | `PHASE_12M_PROBE_TRANSLATOR_HARDWARE_VALIDATION.md` |
| Phase 12N-pre product command admission | `CLOSED_DOCS_ONLY` (taxonomy spec only). | `PHASE_12N_PRE_PRODUCT_WORKLOAD_COMMAND_ADMISSION_PLAN.md` |
| Phase 12N implementation | **NOT AUTHORIZED.** Requires Phase 12N-pre-2 successor planning gate. | Phase 12N-pre boundary rule. |
| Product command mapping / UART expansion | `HOLD` / `NOT_STARTED; USER_DECISION_REQUIRED`. | Recurring open gate, every Phase 12 closeout. |
| ACTIVE disarm widening | `USER_DECISION_REQUIRED_ACTIVE_DISARM`. | Open gate, every Phase 12 closeout. |
| Scheduler 7A/7B | `DEFER`. | Open gate, every Phase 12 closeout. |
| F407 / custom board | `HOLD/DEFER`. | Open gate, every Phase 12 closeout. |
| POST_FLASH_AUTOSTART root cause | `OPEN` / `MITIGATED_BY_WORKFLOW`. | Open gate, every Phase 12 closeout. |
| Non-NULL action / `on_entry` / `on_exit` for FAULT | open (future app-behavior phase). | Open gate, 12K/12L closeouts. |
| FAULT adapter event sourcing (hardware fault signal) | `NOT_STARTED`. | Open gate, 12L/12M closeouts. |
| UART TX response for `probe_translator` snapshot | `NOT_STARTED; USER_DECISION_REQUIRED`. | Open gate, 12L/12M closeouts. |
| Bridge ABI memory-layout lock | `NOT_STARTED`. | Open gate, 12L/12M closeouts. |
| `RobotOS_v1.0/examples/` | `NOT_STARTED`. | Open gate, every Phase 12 closeout. |
| Multi-product coordination | `NOT_STARTED`. | Open gate, every Phase 12 closeout. |
| Full `probe_translator` matrix beyond `a s r ?` | Not proven on hardware. Only the controlled `a s r ?` sequence is hardware-validated. | Phase 12M closeout. |
| Production hardware | Not validated. Devkit board is `stm32f411e_disco` rev D only. | Phase 12M closeout. |
| `app/probe_translator/CMakeLists.txt` and `Kconfig` | **NOT CREATED.** Forbidden at 12K. Not created at 12L. | Phase 12K-pre / 12K / 12L closeouts. |
| Legacy `RobotOS_v1.0/src/` + `include/robotos/` | Marked frozen / non-authoritative at Phase 12D-pre. | `PHASE_12D_PRE_LEGACY_FRAMEWORK_SCAFFOLD_DISPOSITION.md` |

---

## 10. Reader navigation

| Question | Where to look |
|---|---|
| "What is the live current state?" | [`../../../../CURRENT_STATE.md`](../../../../CURRENT_STATE.md). |
| "What phases exist in window 11-20?" | [`../01_PROGRESS/DEVKIT_PROGRESS_PHASE_11_20.md`](../01_PROGRESS/DEVKIT_PROGRESS_PHASE_11_20.md), §4 Phase Index. |
| "What does Phase 12K (or any other phase) actually prove?" | The closeout under [`../02_PHASE_CLOSEOUTS/`](../02_PHASE_CLOSEOUTS/) — scope-guard, evidence section, verdict. |
| "What is the contract for FSM / bridge / probe_translator / probe adapter / hardware validation / product admission?" | The corresponding long-lived spec under [`../03_SPECS/`](../03_SPECS/). |
| "What did the hardware actually print?" | `../logs/INDEX.md` and the raw RTT / build transcripts under `../logs/` and `../../tests/host/logs/`. |
| "How is the docs directory structured?" | [`../00_INDEX/README.md`](../00_INDEX/README.md). |
| "What is this guide?" | This file — architectural reading guide only, not a phase truth artifact. |

---

## 11. Agent guidance (Claude, Copilot, GPT, GLM)

Use these rules when assisting on RobotOS devkit work. They mirror
the `-pre` / `-Z` discipline above.

1. **Do not infer runtime from build.** A `CLOSED_WITH_BUILD_EVIDENCE` phase proves CMake/cross-compile only. It does not prove that the new code is wired into devkit or that it runs correctly.
2. **Do not infer hardware from runtime build.** A devkit-runtime phase (e.g. Phase 12L) is build-depth runtime admission. It does **not** imply hardware behavior. The next `-Z` guard restates this explicitly; do not erase it.
3. **Do not infer product contract from devkit evidence.** The UART command set `a/s/r/?/x/v/L/d/T` and the UART TX response shapes are **devkit-evidence**, locked at Phase 9E / 10B / 11D and confirmed devkit-evidence at Phase 12N-pre. They are not a product/public command contract.
4. **Always check `CURRENT_STATE.md` before acting.** This guide is a narrative; live state lives there.
5. **Preserve non-claims.** When a `-Z` checkpoint says "this is NOT proven", do not silently rewrite that as "proven" because a later phase proved a different thing.
6. **Prefer `-pre` / implementation / `-Z` for any boundary-crossing change.** If a planned change crosses a module boundary (devkit ↔ framework, framework ↔ app, runtime ↔ hardware, devkit-evidence ↔ product), it requires a `-pre` planning gate first, not an inline implementation patch.
7. **Do not mark deferred/pending work as completed.** `HOLD`, `DEFER`, `NOT_STARTED`, and `USER_DECISION_REQUIRED` are recorded decisions, not bugs to be quietly closed. Move them only through an explicit phase.
8. **Do not stage `.vscode/settings.json` or unrelated untracked build/log directories** during docs-only commits. The repo state is intentionally noisy with `build-*` directories that are local artifacts.
9. **For GLM specifically:** this guide is `AUDIT ONLY` context. GLM must not edit, extend, or commit changes to this guide without an explicit `PATCH ALLOWED` instruction. See `.claude/CLAUDE.md` GLM section.

---

## 12. What this guide is not

- **Not a closeout.** No phase status is changed by this file.
- **Not phase truth.** When this guide disagrees with a closeout under `../02_PHASE_CLOSEOUTS/` or a progress entry under `../01_PROGRESS/`, the closeout / progress entry wins.
- **Not a spec.** Long-lived specs live under `../03_SPECS/`.
- **Not an evidence log.** Logs live under `../logs/` and `../../tests/host/logs/`.
- **Not exhaustive.** It deliberately groups by family and omits phase-detail noise. Use the closeouts for full evidence.
- **Not an authorization.** Nothing in this guide authorizes opening a new phase, modifying source, modifying CMake, or running hardware.

---

## 13. Provenance

- **Created:** 2026-05-14.
- **Baseline at creation:** `origin/master = 76ec2f6` (Phase 12N-pre closed).
- **Source of phase truth used:** `../00_INDEX/README.md` (per-phase summaries), `../01_PROGRESS/DEVKIT_PROGRESS_PHASE_11_20.md` (§4 Phase Index and per-phase narratives), `../01_PROGRESS/DEVKIT_PROGRESS.md` (Phase 3/4/5/6/9 family headings), `../../../../CURRENT_STATE.md` (live state for Phase 12K through 12N-pre).
- **Inference vs. fact:** Phase families in §2 are a grouping convention introduced by this guide; the individual phase facts (status, result tags, evidence types, file paths) are reproduced verbatim from the closeouts and progress entries.
- **Known limits of this guide:** Diagrams in §1.1, §4.1, §4.3, and §7 are illustrative ASCII; they are not exhaustive component diagrams and do not replace the closeouts' file-level evidence.
