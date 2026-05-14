# Phase 12N-pre — Product / Workload Command Admission Plan

**Status:** `CLOSED_DOCS_ONLY`
**Decision:** `PHASE_12N_PRE_PRODUCT_WORKLOAD_COMMAND_ADMISSION_PLAN_CLOSED`
**Date:** 2026-05-14
**Type:** Docs-only product/workload command admission planning gate.
**Prior phase anchor:** [`PHASE_12MZ_HARDWARE_VALIDATION_CHECKPOINT.md`](PHASE_12MZ_HARDWARE_VALIDATION_CHECKPOINT.md)
**Phase 12M anchor:** [`PHASE_12M_PROBE_TRANSLATOR_HARDWARE_VALIDATION.md`](PHASE_12M_PROBE_TRANSLATOR_HARDWARE_VALIDATION.md)
**Implementation contract (produced here):** [`../03_SPECS/PRODUCT_WORKLOAD_COMMAND_ADMISSION_PLAN.md`](../03_SPECS/PRODUCT_WORKLOAD_COMMAND_ADMISSION_PLAN.md)

---

## A. Executive Summary

Phase 12N-pre is a **docs-only planning gate** that answers a single
strategic question: *what is the safest next step for product / workload
commands now that probe_translator is hardware-validated?*

After auditing the current command surface, **the recommended path is
HOLD or a docs-only command taxonomy spec — not a UART-command product
mapping or new protocol implementation.** The existing UART command set
(`a/s/r/?/x/v/L/d/T`) is **devkit demo and evidence**, not a product
contract; promoting it directly to product semantics would create a
permanent compatibility burden and bypass the planning discipline that
made Phases 12K–12M safe.

**What Phase 12N-pre does:**

- Inventories the current command/input surface.
- Classifies each surface as devkit-evidence vs. product candidate.
- Evaluates 5 candidate strategies for Phase 12N.
- Selects HOLD + taxonomy spec as the recommended path.
- Defines what any future Phase 12N implementation would have to satisfy
  before it could be authorized.
- Produces a long-lived implementation spec
  `PRODUCT_WORKLOAD_COMMAND_ADMISSION_PLAN.md`.

**What Phase 12N-pre does not do:**

- Does not implement any product command mapping.
- Does not change UART, scheduler, or runtime behavior.
- Does not modify any source, header, CMake, Kconfig, `prj.conf`,
  DTS, overlay, test, tool, or log file.
- Does not run flash, RTT, or any hardware session.

**Decision:** `PHASE_12N_PRE_PRODUCT_WORKLOAD_COMMAND_ADMISSION_PLAN_CLOSED`

---

## B. Baseline From Phase 12M-Z

| Item | Value |
| --- | --- |
| HEAD at Phase 12N-pre open | `8c86aec docs: add Phase 12M-Z hardware validation checkpoint` |
| `origin/master` | `8c86aec` (synced) |
| Phase 12M-Z guard confirmed | `PHASE_12M_Z_HARDWARE_VALIDATION_CHECKPOINT_CLOSED` |
| Phase 12M hardware evidence | `phase_12M_rtt_2026-05-14.txt`; `phase_12M_uart_2026-05-14.txt` |
| Phase 12L adapter | `devkit/src/devkit_probe_adapter.{c,h}` present |
| Working tree | Clean on all tracked files |

---

## C. Current Command / Input Surface Inventory

### C.1 UART command surface (`devkit_uart_producer.c`)

All commands handled by `devkit_uart_emit_tx_response()` switch (line 229+).
Single-byte commands; bounded fixed-buffer responses; no parser, no
framing beyond `\r\n`; no command registry; thread-context only.

| Byte | Action in devkit_app_state | UART TX response | Probe dispatch (Phase 12L) | Class |
| --- | --- | --- | --- | --- |
| `'a'`/`'A'` | →ARMED (or ignored if already) | `OK state=ARMED` / `OK state=ARMED unchanged=1` | `(TYPE_CONFIG, ARG_NONE)` if accepted | devkit demo + internal probe evidence |
| `'s'`/`'S'` | →ACTIVE (or ignored if already) | `OK state=ACTIVE` / `OK state=ACTIVE unchanged=1` | `(TYPE_COMMAND, ARG_START)` if accepted | devkit demo + internal probe evidence |
| `'r'`/`'R'` | →IDLE (or ignored if already) | `OK state=IDLE` / `OK state=IDLE unchanged=1` | `(TYPE_COMMAND, ARG_RESET)` if accepted | devkit demo + internal probe evidence |
| `'d'`/`'D'` | ARMED→IDLE; else no-op | `OK disarm state=IDLE` / `OK disarm no-op state=N` | `(TYPE_COMMAND, ARG_RESET)` if ARMED→IDLE | devkit demo + internal probe evidence |
| `'?'` | none | `STATE state=N transitions=N button=N uart=N ignored=N` | none | devkit query |
| `'v'`/`'V'` | none | `INFO phase=10b-v app=devkit board=stm32f411e_disco tick_ms=500 uart=minimal` | none | devkit query |
| `'l'`/`'L'` | none | `OK led=toggle state=N` / `ERR led=toggle ret=N state=N` | none | devkit physical-effect probe |
| `'t'`/`'T'` | none | `ACC x=v.v y=v.v z=v.v` / `ERR accel read=N` | none | devkit hardware probe |
| `'x'` and any other byte | none; `ignored_count++` | `ERR ignored byte=0xNN state=N` | none | devkit fallthrough |

### C.2 Button surface (`devkit_button_producer.c` → `devkit_app_state_on_button`)

Physical EXTI → debounce → core event → handler → app-state cycle
IDLE→ARMED→ACTIVE→IDLE. Phase 12L probe dispatch wired BEHIND the cycle
(same mapping as UART path).

| Cycle position | App transition | Probe dispatch | Class |
| --- | --- | --- | --- |
| Press 1 (from IDLE) | IDLE→ARMED | `(TYPE_CONFIG, ARG_NONE)` | devkit demo + internal |
| Press 2 (from ARMED) | ARMED→ACTIVE | `(TYPE_COMMAND, ARG_START)` | devkit demo + internal |
| Press 3 (from ACTIVE) | ACTIVE→IDLE | `(TYPE_COMMAND, ARG_RESET)` | devkit demo + internal |

### C.3 Host demo scripts (`RobotOS_v1.0/tools/runtime/`)

| Script | Sequence | Purpose | Class |
| --- | --- | --- | --- |
| `run_phase9d_demo.ps1` | UART + button workload | Phase 9D composite demo | devkit demo |
| `run_phase9e_uart_response_demo.ps1` | `a s ? r x` | Phase 9E response validation | devkit demo |
| `run_phase9g_uart_burst_demo.ps1` | 5-byte burst | Phase 9G UART burst characterization | devkit demo |
| `run_phase10b_v_build_query_demo.ps1` | `v a v r ?` | Phase 10B-v `v` validation | devkit demo |
| `run_phase10b_l_led_command_demo.ps1` | `L v L ?` | Phase 10B-L LED command + visual | devkit demo |
| `run_phase10b_d_disarm_demo.ps1` | `d a d ?` | Phase 10B-d disarm validation | devkit demo |
| `run_phase11d_accel_probe_demo.ps1` | `T T ?` | Phase 11D accelerometer probe | devkit demo |
| `run_phase12m_probe_demo.ps1` | `a s r ?` | Phase 12M probe runtime validation | devkit demo |

**All scripts are demo/evidence harnesses.** None defines a product
protocol.

### C.4 Probe adapter dispatch (Phase 12L)

- Module: `devkit_probe_adapter.{c,h}`
- API: `_init`, `_dispatch(type, arg0)`, `_log_snapshot`
- Class: **internal devkit-local evidence only**. No UART TX exposure.
  Snapshot visible only via Zephyr `LOG_INF` (`ROBOTOS_PROBE state=N
  trans=N events=N no_trans=N mapped=N unmapped=N`), which is RTT-only.

### C.5 Documentation claims about command semantics

| Doc | Status |
| --- | --- |
| `03_SPECS/COMMAND_SET_DRAFT.md` | Devkit command vocabulary draft (Phase 10A) |
| `02_PHASE_CLOSEOUTS/PHASE_10C_COMMAND_SET_CHECKPOINT.md` | Locks `a/s/r/?/x/v/L/d/T` as frozen devkit set |
| `02_PHASE_CLOSEOUTS/PHASE_11Z_COMMAND_SET_CHECKPOINT.md` | Validates 9-command set after Phase 11E |
| `03_SPECS/TELEMETRY_REFERENCE.md` | Documents `ROBOTOS_*` log lines (RTT only) |

**No document declares any of these commands a product/public protocol.**
The closeout language consistently calls them devkit commands.

---

## D. Product vs. Devkit-Evidence Boundary

The single most important policy decision in Phase 12N-pre is:

**The current UART command set is devkit demo and evidence. It is not
a product contract.**

### D.1 What is devkit-evidence

A devkit-evidence command:
- Was added at a specific devkit phase to exercise a specific runtime
  surface (button, UART RX, app-state, accelerometer probe, probe
  translator).
- Has a single-byte, fixed-response, no-framing protocol designed for
  RTT log validation and operator visual checks.
- Has no version negotiation, no error framing standard, no security
  context, no flow control, no multi-byte commands.
- Has no compatibility guarantee across product variants.
- Lives entirely inside `devkit_uart_producer.c` and `devkit_app_state.c`.

### D.2 What a product/workload command would require

A product command surface (if ever opened) would require:
- Stable, versioned command vocabulary.
- Defined error framing (status codes, error categories, recoverable vs.
  fatal).
- Defined response framing (start/end markers, length, checksum or CRC
  if required by application context).
- Documented compatibility rules across product variants.
- Defined security context (none vs. authenticated; defaults).
- Parser separated from devkit producer.
- A dedicated test suite covering happy path, malformed input, partial
  reads, out-of-order sequences, and recovery.
- Hardware validation under the product workload profile, not just the
  devkit demo profile.

**These are absent today.** Promoting the devkit command set to product
semantics without first defining the above would create a permanent
compatibility burden.

### D.3 Boundary rule

| Command | Devkit-evidence | Product-candidate (under separate Phase 12N or later) |
| --- | --- | --- |
| `a` / `s` / `r` / `d` | **Yes** — frozen, single-byte | **No** — would need versioned arm/start/stop/reset with status |
| `?` | **Yes** — devkit state query | **No** — product would need versioned status with schema |
| `v` | **Yes** — `phase=10b-v` build-tag query | **No** — product would need versioned ID query |
| `l` / `L` | **Yes** — LED visual check | **Maybe** — physical effect; not a control surface |
| `t` / `T` | **Yes** — accelerometer probe | **Maybe** — adapter probe; might be expanded under separate spec |
| `x` | **Yes** — explicit ignored-byte probe | **No** — would be undefined in product context |
| (probe adapter snapshot) | **Internal RTT only** | **No UART TX exposure at any phase without explicit user decision** |

---

## E. Candidate Phase 12N Strategies

### E.1 Option 1 — HOLD / no product command opened

**Shape:** Phase 12 chain is complete. Do not open product mapping. Park
at hardware-proven devkit evidence.

| Property | Assessment |
| --- | --- |
| Affected files | None |
| Behavior risk | **NONE** |
| Public interface risk | **NONE** |
| Validation burden | None |
| Rollback path | Trivial (nothing to roll back) |
| **Verdict** | **VIABLE.** Safest possible default. |

### E.2 Option 2 — Docs-only command taxonomy

**Shape:** Define command categories and naming conventions in a
long-lived spec. Document the devkit-vs-product boundary explicitly.
No firmware change.

| Property | Assessment |
| --- | --- |
| Affected files | Spec doc only (`PRODUCT_WORKLOAD_COMMAND_ADMISSION_PLAN.md`) |
| Behavior risk | **NONE** |
| Public interface risk | **NONE** |
| Validation burden | Spec review only |
| Rollback path | Trivial (revert doc) |
| **Verdict** | **RECOMMENDED.** Locks the boundary without committing to implementation. |

### E.3 Option 3 — Devkit-local product namespace (docs+test, no UART change)

**Shape:** Define a product-shaped command vocabulary in spec. Add host
unit tests for the parser-shaped vocabulary. Do NOT change UART firmware.

| Property | Assessment |
| --- | --- |
| Affected files | Spec doc + new host test |
| Behavior risk | **LOW** (host-only) |
| Public interface risk | **NONE** (no UART change) |
| Validation burden | Host test suite |
| Rollback path | Delete test + spec |
| **Verdict** | **OPTIONAL — second-stage if user wants more than docs.** Not required at Phase 12N implementation. |

### E.4 Option 4 — UART public command mapping

**Shape:** Map selected UART commands to product/workload semantics. Add
new commands or modify existing responses to expose product contract.

| Property | Assessment |
| --- | --- |
| Affected files | `devkit_uart_producer.c`, possibly new parser module, tests, scripts, docs |
| Behavior risk | **HIGH** — modifies frozen UART surface |
| Public interface risk | **HIGH** — commits to permanent compatibility |
| Validation burden | Full product test suite + hardware validation under product workload |
| Rollback path | Complex (firmware revert + doc rollback) |
| **Verdict** | **REJECTED at Phase 12N depth.** Out of scope without an explicit product-decision phase before this. UART command set `a/s/r/?/x/v/L/d/T` is frozen as devkit-evidence; promoting it requires a separate user decision and dedicated planning. |

### E.5 Option 5 — New protocol surface (future)

**Shape:** Future command layer separate from current UART demo
commands. E.g., a separate UART channel, USB CDC ACM, or in-band
multi-byte framing.

| Property | Assessment |
| --- | --- |
| Affected files | Many (new producer, parser, framing, tests, scripts) |
| Behavior risk | **HIGH** |
| Public interface risk | **HIGH** (new permanent surface) |
| Validation burden | Full new-protocol test suite + hardware validation |
| Rollback path | Complex |
| **Verdict** | **REJECTED for Phase 12N.** Future option, requires its own planning chain. |

---

## F. Recommended Path

**Recommendation: Option 1 (HOLD) as default, with Option 2 (docs-only
taxonomy spec) produced by Phase 12N-pre itself.**

### F.1 Rationale

- The Phase 12 chain (12K → 12K-Z → 12L → 12L-Z → 12M → 12M-Z) is
  complete and self-consistent. The probe translator is build-admitted,
  runtime-admitted, hardware-validated, and explicitly guarded as
  *not* a product surface.
- Promoting `a/s/r/?/x/v/L/d/T` to product semantics without first
  defining versioning, error framing, response framing, security
  context, and test coverage would create a permanent compatibility
  burden that the project has carefully avoided through Phases 10A–12M.
- The user has not requested a product command contract. `USER_DECISION_REQUIRED`
  has been preserved on this topic since Phase 10C.
- Producing only the taxonomy/spec (Option 2) gives future agents a
  clear boundary without committing to firmware change.

### F.2 What Phase 12N-pre produces today

- This closeout doc.
- New long-lived spec `PRODUCT_WORKLOAD_COMMAND_ADMISSION_PLAN.md`.
- Doc-sync to `CURRENT_STATE.md`, progress, and index.

### F.3 What Phase 12N implementation would require (if ever opened)

- An explicit user decision authorizing product command surface opening.
- A successor planning phase (Phase 12N-pre-2 or equivalent) defining
  exactly which command(s) become product, what response framing they
  use, and what compatibility guarantees apply.
- A test plan covering parser, framing, malformed input, partial reads,
  out-of-order sequences, and recovery.
- A hardware validation plan covering the product workload, not just
  the devkit demo profile.
- A rollback plan in case product semantics are later regretted.

**No Phase 12N implementation is approved at Phase 12N-pre.**

---

## G. Allowed / Forbidden / Conditional File Set for Any Phase 12N Implementation

If a future user decision opens Phase 12N implementation, the file
boundary must be defined per the long-lived spec
`PRODUCT_WORKLOAD_COMMAND_ADMISSION_PLAN.md`. Until then:

### G.1 Allowed at any future Phase 12N (after explicit user decision)

| Path | Condition |
| --- | --- |
| New spec doc updates | Allowed for further planning |
| New host test file(s) under `tests/host/` | Allowed for parser/framing tests if Option 3 is chosen |
| Closeout / progress / index doc-sync | Allowed |
| New parser module (only if a separate Phase 12N-pre-2 spec authorizes it) | Conditional — requires explicit per-file approval |

### G.2 Forbidden at any future Phase 12N without explicit pre-planning

| Path | Reason |
| --- | --- |
| `RobotOS_v1.0/devkit/src/devkit_uart_producer.{c,h}` | UART TX surface frozen; product mapping not authorized |
| `RobotOS_v1.0/devkit/src/devkit_app_state.{c,h}` | App-state semantics are devkit-evidence; not a product contract |
| `RobotOS_v1.0/devkit/src/devkit_probe_adapter.{c,h}` | Adapter is internal evidence; no UART TX exposure |
| Any new UART command byte | Frozen `a/s/r/?/x/v/L/d/T` |
| Any new UART TX response format | Frozen |
| `RobotOS_v1.0/devkit/prj.conf` | Zero-diff at Phase 12N |
| DTS / overlay | Zero-diff |
| `RobotOS_v1.0/framework/*` | Zero-diff |
| `RobotOS_v1.0/app/probe_translator/*` | Zero-diff |
| `RobotOS_v1.0/core/*`, `RobotOS_v1.0/platform/*` | Zero-diff |
| New ISR-context command parsing | Forbidden at any depth |
| Hardware validation log without an authorized Phase 12N-Hardware contract | Forbidden |

### G.3 Conditional

| Path | Condition |
| --- | --- |
| New host test under `tests/host/` for spec-defined command vocabulary | Allowed only if it tests parser-shaped logic with no Zephyr/devkit leakage |
| New demo script under `tools/runtime/` | Only if authorized by Phase 12N-Hardware contract |

---

## H. Required Validation If Product Mapping Is Later Opened

If a future phase authorizes any UART/API behavior change for product
semantics, the validation contract must include all of the following:

| # | Required validation |
| --- | --- |
| 1 | Host test suite: parser correctness, framing, partial reads, malformed input, recovery |
| 2 | Host regression: existing 23+ tests must still PASS |
| 3 | Zephyr build PASS for `stm32f411e_disco` |
| 4 | Hardware validation under product workload profile (RTT + UART transcript) |
| 5 | Backwards-compatibility check: existing devkit demo scripts (Phase 9D/9E/9G/10B/11D/12M) must still run unchanged |
| 6 | Boundary grep gates: no new ISR-context parsing; no heap; no scheduler change |
| 7 | Documentation: closeout doc, progress entry, INDEX entry, spec status upgrade |

---

## I. Risk / Rollback Signals

| Risk | Signal | Action |
| --- | --- | --- |
| Future agent treats `a/s/r/?` as product protocol | Reference to "product command" without phase contract | Cite this doc + spec; reject the change; require an explicit phase contract |
| Promoting `?` to product status query | UART TX response change to existing `STATE state=...` line | Hard stop; this would break Phase 9E/10B/11D/12M evidence; revert |
| Adding new UART command byte | New `case '...'` in `devkit_uart_producer.c` switch | Hard stop; command set is frozen at `a/s/r/?/x/v/L/d/T` |
| Mixing devkit-evidence and product semantics into a single response | Single response line changes format | Hard stop; product responses, if any, must be a separate surface |
| Phase 12N implementation opened without Phase 12N-pre-2 | New firmware diff without authorizing spec | Stop; close as `BLOCKED_NO_USER_DECISION` |

---

## J. Explicit Non-Claims

Phase 12N-pre (this planning gate) does NOT:

| Claim | Status |
| --- | --- |
| Product command mapping implemented | **NOT IMPLEMENTED** |
| UART behavior changed | **NOT CHANGED** |
| UART TX response format changed | **NOT CHANGED** |
| Runtime behavior changed | **NOT CHANGED** |
| Scheduler behavior changed | **NOT CHANGED** |
| New hardware validation performed | **NOT PERFORMED** |
| F407 / custom board opened | **HOLD/DEFER — not opened** |
| Full probe_translator matrix proven | **NOT PROVEN** |
| New protocol surface created | **NOT CREATED** |
| Phase 12N implementation authorized | **NOT AUTHORIZED** — requires explicit user decision and a successor planning gate (Phase 12N-pre-2 or equivalent) |

---

## K. Next-Step Recommendation

**Default: HOLD.** The Phase 12 chain is complete and the codebase is
stable. The user has not requested product command opening, and the
existing devkit command set is fit-for-purpose as a hardware evidence
harness.

**If the user wants forward motion without firmware change:**
- Phase 12N (Option 2 only) — adopt this docs-only taxonomy spec as
  the canonical boundary. Already produced at Phase 12N-pre.
- Optionally extend with host-only parser tests (Option 3) under a
  separate small phase.

**If the user wants product semantics:**
- Open `Phase 12N-pre-2` (a successor planning gate) to define the
  product command surface, response framing, version negotiation,
  and test contract before any firmware change.
- Do NOT open Phase 12N implementation directly.

**`USER_DECISION_REQUIRED`** for any path beyond HOLD. The recommended
default action is to take no further action until the user explicitly
authorizes either Option 2 extension (host tests) or Option 4 redesign
(product command surface).
