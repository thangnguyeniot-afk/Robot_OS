# RobotOS — Product / Workload Command Admission Plan

**Status:** `DRAFT / NON-FINAL — TAXONOMY ONLY`
**Spec type:** Long-lived boundary spec. Defines the devkit-evidence vs.
product/workload command boundary. Locks the rule that the existing
single-byte UART command set is **devkit demo and evidence**, not a
product contract.
**Revision:** Phase 12N-pre (2026-05-14, `CLOSED_DOCS_ONLY`;
`PHASE_12N_PRE_PRODUCT_WORKLOAD_COMMAND_ADMISSION_PLAN_CLOSED`)
**Planning closeout anchor:**
[`../02_PHASE_CLOSEOUTS/PHASE_12N_PRE_PRODUCT_WORKLOAD_COMMAND_ADMISSION_PLAN.md`](../02_PHASE_CLOSEOUTS/PHASE_12N_PRE_PRODUCT_WORKLOAD_COMMAND_ADMISSION_PLAN.md)

---

## 1. Scope and Status

**What this doc is:**
A boundary spec. Defines categories of command/input surfaces in
RobotOS, classifies each existing surface, and specifies what would be
required before any surface can be promoted from devkit-evidence to
product/workload status.

**What this doc is not:**
- A product command implementation contract (no Phase 12N
  implementation is authorized here).
- A new protocol design.
- A UART command set change.
- A response format change.

**Status semantics:**
- `DRAFT / NON-FINAL — TAXONOMY ONLY` until a successor planning gate
  (Phase 12N-pre-2 or equivalent) authorizes implementation.
- Status would advance to `IMPLEMENTED_AT_12N` only if a future phase
  ships product command behavior with hardware evidence.

---

## 2. Command / Input Surface Categories

RobotOS organizes command/input surfaces into four categories. Every
existing and future surface must fit into exactly one category.

### 2.1 Internal evidence

A surface used only by devkit firmware/RTT logs to prove that internal
state machines, adapters, or framework modules behave correctly.

| Property | Rule |
| --- | --- |
| Visibility | RTT logs only; not exposed on UART TX |
| Stability guarantee | None across phases |
| Compatibility | None; may change with any phase |
| Examples | `ROBOTOS_PROBE state=...` log line; `Phase 9C app transition: ...` log line |

### 2.2 Devkit demo / evidence command

A single-byte UART command (or button event) whose purpose is to
exercise a specific runtime surface during a devkit phase. May produce a
fixed UART TX response designed for shape-match in hardware evidence
logs.

| Property | Rule |
| --- | --- |
| Visibility | UART RX + fixed UART TX response |
| Stability guarantee | Frozen across the current Phase 12 chain (Phase 10C/11Z lock) |
| Compatibility | Devkit only; not a product contract |
| Documentation | `02_PHASE_CLOSEOUTS/PHASE_10C_COMMAND_SET_CHECKPOINT.md`; `02_PHASE_CLOSEOUTS/PHASE_11Z_COMMAND_SET_CHECKPOINT.md` |
| Examples | `a`, `s`, `r`, `?`, `v`, `L`, `d`, `T`, `x` (default fallthrough) |

### 2.3 Product / public command (NOT OPENED at any phase ≤ Phase 12N-pre)

A command surface intended for production use, with versioning, error
framing, response framing, security context, parser tests, and
compatibility guarantees across product variants.

| Property | Rule |
| --- | --- |
| Visibility | Product UART (or other authorized channel) |
| Stability guarantee | Versioned; product-lifetime |
| Compatibility | Maintained across product variants |
| **Existence at Phase 12N-pre** | **NONE — not opened** |

### 2.4 Future protocol surface

A future channel separate from current UART (e.g., USB CDC ACM, multi-byte
framed protocol, etc.).

| Property | Rule |
| --- | --- |
| Status at Phase 12N-pre | `NOT_STARTED` |
| Implementation gate | Requires a dedicated planning chain (Phase 12P-pre or later) |

---

## 3. Current Command Surface Classification (Phase 12N-pre locked)

### 3.1 UART command set (frozen at Phase 10C; validated at Phase 11Z)

| Byte | Action | Class |
| --- | --- | --- |
| `'a'` / `'A'` | App-state →ARMED + (Phase 12L) probe `(TYPE_CONFIG, ARG_NONE)` | **Devkit demo / evidence** |
| `'s'` / `'S'` | App-state →ACTIVE + (Phase 12L) probe `(TYPE_COMMAND, ARG_START)` | **Devkit demo / evidence** |
| `'r'` / `'R'` | App-state →IDLE + (Phase 12L) probe `(TYPE_COMMAND, ARG_RESET)` | **Devkit demo / evidence** |
| `'d'` / `'D'` | ARMED→IDLE disarm + (Phase 12L) probe `(TYPE_COMMAND, ARG_RESET)` | **Devkit demo / evidence** |
| `'?'` | State query | **Devkit query** |
| `'v'` / `'V'` | Build/version query (`phase=10b-v`) | **Devkit query** |
| `'l'` / `'L'` | LED toggle | **Devkit physical-effect probe** |
| `'t'` / `'T'` | Accelerometer probe | **Devkit hardware probe** |
| `'x'` and any other | `ERR ignored byte=0xNN` (default fallthrough) | **Devkit demo (explicit-ignore probe)** |

**None of these is a product/public command.** All UART TX responses
are devkit demo telemetry formats, frozen at the documented checkpoints
but **not productized**.

### 3.2 Button surface

Physical EXTI → app-state IDLE→ARMED→ACTIVE→IDLE cycle + (Phase 12L)
probe dispatch on each transition.

| Property | Rule |
| --- | --- |
| Class | **Devkit demo / evidence** |
| Product status | Not a product control surface |

### 3.3 Probe adapter (`devkit_probe_adapter`)

| API | Class |
| --- | --- |
| `devkit_probe_adapter_init()` | **Internal evidence** |
| `devkit_probe_adapter_dispatch(type, arg0)` | **Internal evidence** (devkit-local dispatch only) |
| `devkit_probe_adapter_log_snapshot()` | **Internal evidence** (RTT log only) |

**The probe adapter has zero UART TX exposure.** All probe state is
visible only via the `ROBOTOS_PROBE state=...` log line in RTT.

---

## 4. Boundary Rule

**Rule 1 — Frozen devkit set.** The single-byte UART command set
`a/s/r/?/x/v/L/d/T` is frozen as devkit demo / evidence. No byte may be
added, removed, or repurposed without an explicit product-decision
phase contract.

**Rule 2 — Response format freeze.** The existing UART TX response lines
(`OK state=...`, `STATE state=...`, `ACC x=...`, `OK led=toggle ...`,
`OK disarm ...`, `ERR ignored byte=...`, `ERR accel read=...`, `INFO
phase=10b-v ...`) are frozen at the Phase 9E/10B/11D shape. No response
text or format may change without explicit phase authorization.

**Rule 3 — No automatic promotion.** A devkit demo command does NOT
become a product command by accumulating evidence or hardware validation.
Phase 12M hardware-validated `a s r ?` runtime behavior is **not** a
product mapping authorization.

**Rule 4 — Internal evidence stays internal.** The `ROBOTOS_PROBE`
snapshot and any future probe adapter log lines must remain RTT-only.
No UART TX exposure of internal evidence is authorized at Phase 12N-pre.

**Rule 5 — Product mapping requires a successor planning gate.** Any
work to open a product command surface (whether by promoting existing
commands, defining new commands, or creating a new protocol channel)
must start with a new planning phase (Phase 12N-pre-2 or equivalent)
that pre-defines:

- The exact command vocabulary.
- The exact response framing.
- The version negotiation policy.
- The error framing policy.
- The security context.
- The test plan (host parser tests + Zephyr build + hardware workload validation).
- The compatibility guarantees across product variants.
- The rollback plan.

Without that successor planning gate, no firmware change for product
command mapping is authorized.

---

## 5. Forbidden Surfaces (Phase 12N-pre lock)

Until a successor planning gate explicitly authorizes otherwise, the
following changes are forbidden:

| Forbidden change | Phase 12N-pre status |
| --- | --- |
| Adding a new UART command byte | **FORBIDDEN** |
| Removing or repurposing any current UART command byte | **FORBIDDEN** |
| Changing existing UART TX response text or format | **FORBIDDEN** |
| Exposing `ROBOTOS_PROBE` snapshot on UART TX | **FORBIDDEN** |
| Adding a separate parser layer in `devkit_uart_producer.c` | **FORBIDDEN** |
| Adding multi-byte command parsing | **FORBIDDEN** |
| Adding command framing (length, checksum, CRC) | **FORBIDDEN** |
| Adding command version negotiation | **FORBIDDEN** |
| ISR-context command parsing | **FORBIDDEN at any depth** |

---

## 6. Allowed Without Successor Planning Gate

The following are explicitly allowed without further authorization:

| Action | Notes |
| --- | --- |
| Citing this spec to reject ad-hoc command-surface changes | Required |
| Adding new RTT log lines for further internal evidence (under a separate phase contract) | Allowed only if no UART TX exposure |
| Documentation cleanup that does not change command semantics | Allowed |
| Updating `02_PHASE_CLOSEOUTS/PHASE_*` to cross-reference this spec | Allowed |

---

## 7. Future Phase 12N Authorization Path

If a future user decision opens product command work, the path is:

1. **Phase 12N-pre-2 (docs-only planning):** A successor planning gate
   that defines exactly which commands become product, what response
   framing they use, what version negotiation applies, and what tests
   and hardware validation are required. **No firmware change.**

2. **Phase 12N-spec (optional docs+host-test):** A docs+host-test phase
   that adds host parser tests for the spec-defined vocabulary. No
   firmware change.

3. **Phase 12N (firmware):** Firmware implementation of the
   spec-defined commands. Requires Phase 12N-pre-2 + Phase 12N-spec
   completion, plus an explicit user authorization separately from
   Phase 12N-pre-2 closure.

4. **Phase 12N-hardware (validation):** Hardware validation under
   product workload profile (not the devkit demo profile). Must run
   `west flash`, RTT capture, and product UART transcript with
   pass/fail criteria defined in Phase 12N-pre-2.

5. **Phase 12N-Z (checkpoint):** Locks the result; defines what is
   product-proven vs. what remains devkit-evidence.

---

## 8. Closeout Criteria for Phase 12N-pre

Phase 12N-pre is closed when:

1. This spec is committed and the boundary rule in §4 is recorded.
2. The Phase 12N-pre closeout doc is committed at
   `02_PHASE_CLOSEOUTS/PHASE_12N_PRE_PRODUCT_WORKLOAD_COMMAND_ADMISSION_PLAN.md`.
3. `CURRENT_STATE.md`, progress, and index reflect Phase 12N-pre as the
   latest closed planning gate.
4. No source / CMake / config / test / tool / log file changes.

---

## 9. Open Decisions (after Phase 12N-pre)

| # | Open question | Status |
| --- | --- | --- |
| 1 | Whether to open product command surface at all | `USER_DECISION_REQUIRED` (no default; HOLD) |
| 2 | If opened, which commands become product candidates | `USER_DECISION_REQUIRED`; subject to Phase 12N-pre-2 |
| 3 | UART TX response for probe_translator snapshot | `NOT_STARTED; USER_DECISION_REQUIRED` |
| 4 | New protocol surface (e.g., USB CDC ACM) | `NOT_STARTED` |
| 5 | Multi-product coordination | `NOT_STARTED` |
| 6 | F407 / custom board command surface | `HOLD/DEFER` |
| 7 | Bridge ABI memory-layout lock | `NOT_STARTED` |
| 8 | FAULT adapter event sourcing | `NOT_STARTED` |
| 9 | Non-NULL action / on_entry / on_exit callbacks | `NOT_STARTED` |
| 10 | Full probe_translator input matrix proof | `NOT_STARTED` |
| 11 | ACTIVE disarm widening | `USER_DECISION_REQUIRED_ACTIVE_DISARM` |
| 12 | Scheduler 7A/7B | `DEFER` |
