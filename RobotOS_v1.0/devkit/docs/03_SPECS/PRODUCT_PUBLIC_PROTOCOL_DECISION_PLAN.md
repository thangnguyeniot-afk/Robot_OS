# RobotOS — Product / Public Protocol Decision Plan

**Status:** `DRAFT / NON-FINAL — DESIGN ONLY`
**Spec type:** Long-lived design spec. Defines the recommended direction
for a RobotOS public protocol (provisional name: **RPP — RobotOS Public
Protocol**), at conceptual level only. Pairs with the devkit-evidence
boundary spec
[`PRODUCT_WORKLOAD_COMMAND_ADMISSION_PLAN.md`](PRODUCT_WORKLOAD_COMMAND_ADMISSION_PLAN.md).
**Revision:** Phase 12N-pre-2 (2026-05-14, `CLOSED_DOCS_ONLY`;
`PHASE_12N_PRE2_PRODUCT_PUBLIC_PROTOCOL_DECISION_PLAN_CLOSED`)
**Planning closeout anchor:**
[`../02_PHASE_CLOSEOUTS/PHASE_12N_PRE2_PRODUCT_PUBLIC_PROTOCOL_DECISION_PLAN.md`](../02_PHASE_CLOSEOUTS/PHASE_12N_PRE2_PRODUCT_PUBLIC_PROTOCOL_DECISION_PLAN.md)

---

## 1. Scope and Status

**What this spec is:**
A design spec that defines the recommended direction for a RobotOS
public protocol. The protocol is named provisionally **RPP — RobotOS
Public Protocol**. The spec defines:

- The transport-neutral command model.
- The versioning scheme.
- The request / response frame at conceptual level.
- The status / error vocabulary.
- The command categories.
- The required minimal command set for a future `RPP/1.0`.
- The relationship to the existing devkit-evidence UART commands.
- The forbidden / disallowed shapes.
- The future implementation gate path.

**What this spec is not:**

- Not an implementation contract. No code is authorized.
- Not a transport binding. UART framed v0 is named as a candidate; the
  binding itself requires a successor planning gate (Phase 12N-pre-3).
- Not a UART command-set change.
- Not a UART TX response format change.

**Status semantics:**

- `DRAFT / NON-FINAL — DESIGN ONLY` while Phase 12N-pre-2 is the latest
  protocol-related planning gate. The spec describes a *target* design,
  not an authorized implementation.
- Would advance to `BINDING_PLANNED_AT_12N_PRE_3 / IMPLEMENTED_AT_12N`
  only if a future phase ratifies the binding and ships parser /
  dispatcher with hardware evidence.

---

## 2. Relationship to the Devkit-Evidence Boundary

This spec is layered on top of
[`PRODUCT_WORKLOAD_COMMAND_ADMISSION_PLAN.md`](PRODUCT_WORKLOAD_COMMAND_ADMISSION_PLAN.md)
and does not weaken any of its boundary rules.

| Boundary rule (from `PRODUCT_WORKLOAD_COMMAND_ADMISSION_PLAN.md`) | Status under this spec |
| --- | --- |
| Single-byte UART command set is frozen as devkit-evidence | **UNCHANGED** |
| UART TX response formats are frozen at Phase 9E/10B/11D shape | **UNCHANGED** |
| Internal evidence (`ROBOTOS_PROBE` snapshot) stays RTT-only | **UNCHANGED** |
| No automatic promotion from devkit-evidence to product status | **UNCHANGED** |
| Product mapping requires a successor planning gate | **CONSISTENT** — this spec is produced by the successor planning gate (Phase 12N-pre-2) for the *protocol shape*; a further gate (Phase 12N-pre-3, transport binding) is still required before any code change |

The devkit-evidence boundary spec defines **what is not** a public
protocol. This spec defines **what could become** a public protocol
under a controlled future implementation path.

---

## 3. Definition of "Public Protocol" for RobotOS

A public protocol in RobotOS is the externally-stable command and
response contract that a deployed RobotOS product exposes. It must
satisfy all of the following.

| # | Property | Requirement |
| --- | --- | --- |
| 1 | Stable command vocabulary with versioning | Every command name is namespaced; every command is associated with a `RPP/<MAJOR>.<MINOR>` introduction version |
| 2 | Versioned message grammar | Each frame carries its `RPP/<ver>`; parser detects compatibility ahead of dispatch |
| 3 | Explicit request / response frame | Begin/end framing or length rule sufficient for partial-read recovery; mandatory `<id>` correlation token |
| 4 | Status / error vocabulary | Exhaustive enumerated status set; each command declares which statuses it may produce |
| 5 | Compatibility promise | Semver-flavored: MAJOR breaks; MINOR adds backward-compatible capabilities |
| 6 | Transport policy | Logical protocol is transport-neutral; per-transport binding spec defines wire format and framing |
| 7 | Observability / evidence policy | Diagnostic / RTT log lines remain disjoint from public-protocol payloads |
| 8 | Safety constraints | No ISR-context parsing; no silent fallback to devkit single-byte set; no command that mutates protocol version mid-session |
| 9 | Rollback / migration path | Every published command has a deprecation path; no command can be silently retracted |

Anything that does not satisfy items 1–9 is **not** RPP.

---

## 4. Provisional Naming and Versioning

| Item | Provisional value | Notes |
| --- | --- | --- |
| Protocol name (working) | **RPP — RobotOS Public Protocol** | Subject to user re-name at Phase 12N-pre-3 |
| Versioning | `RPP/<MAJOR>.<MINOR>` | Semver-flavored |
| Initial design version | `RPP/0.1` | **Pre-product, design-only.** Must not appear in source code |
| First product-eligible version | `RPP/1.0` | Only when a future implementation phase ships a stable command set with hardware evidence |

`RPP/0.x` exists exclusively in this spec and the Phase 12N-pre-2
closeout. It must not be encoded as a literal in any tracked source
file, test, tool, script, or log. The earliest a literal `RPP/<ver>`
may appear in code is Phase 12N-impl, and only with a `MAJOR.MINOR`
that a closed Phase 12N-pre-3 has ratified.

---

## 5. Transport-Neutral Command Model

### 5.1 Command identity

Every public command has:

| Component | Notes |
| --- | --- |
| Namespace | `identity`, `status`, `lifecycle`, `diagnostic`, `probe` (reserved), etc. |
| Verb | A short imperative name (e.g., `version`, `query`, `arm`, `start`, `stop`, `reset`, `fault_clear`) |
| Argument shape | A small structured map; shape per command |
| Producible statuses | A subset of the §6 vocabulary; declared per command |
| Introduction version | The `RPP/<MAJOR>.<MINOR>` at which the command became available |
| Capability flag | A short identifier emitted in `identity.version` response so callers can detect support |

### 5.2 Conceptual request frame

```text
RPP/<ver> REQ <id> <namespace>.<verb> [<args>]
```

| Field | Notes |
| --- | --- |
| `<ver>` | Protocol version present on every message |
| `REQ` | Direction marker; lets the parser disambiguate without context |
| `<id>` | Caller-chosen correlation token; numeric or short opaque |
| `<namespace>.<verb>` | Command name |
| `<args>` | Optional; shape per command |

### 5.3 Conceptual response frame

```text
RPP/<ver> RES <id> <status> [<payload>]
```

| Field | Notes |
| --- | --- |
| `<ver>` | Echoes the request version or signals upgrade per binding rules |
| `RES` | Direction marker |
| `<id>` | Mirrors the request `<id>` |
| `<status>` | One token from §6 |
| `<payload>` | Optional; shape per command |

### 5.4 Wire form vs. logical form

The fields in §5.2 / §5.3 are **logical**. The wire encoding
(line-delimited text, length-prefixed binary, JSON, CBOR, etc.) is a
**transport binding decision** locked in Phase 12N-pre-3.

The default candidate is **UART framed v0**, a line-delimited textual
encoding chosen to be RTT-log-friendly and operator-readable; alternate
bindings (USB CDC ACM, RTT-shell, BLE GATT, TCP) are explicitly future
work.

---

## 6. Status / Error Vocabulary

| Token | Class | Meaning |
| --- | --- | --- |
| `OK` | success | Command accepted and completed (or accepted for async completion if declared async) |
| `ERR_UNKNOWN_COMMAND` | recoverable | Unknown `<namespace>.<verb>` |
| `ERR_INVALID_ARGS` | recoverable | Args present but malformed or out of range |
| `ERR_STATE_NOT_ALLOWED` | recoverable | Command rejected in current state |
| `ERR_BUSY` | recoverable | Implementation is mid-operation; caller may retry |
| `ERR_HARDWARE` | recoverable or fatal per command | A backing hardware operation failed |
| `ERR_NOT_IMPLEMENTED` | recoverable | Command is reserved but not implemented in current version |
| `ERR_SECURITY_DENIED` | fatal-per-session | Caller is not authorized for this command |
| `ERR_PROTOCOL` | fatal-per-frame | Frame violated protocol grammar; parser-detected |

Every command must declare, in its per-command spec, which statuses it
may produce and the recoverability class of each. New status tokens
require a `MINOR` version bump and a `MAJOR` bump only if they reclassify
an existing token's recoverability.

---

## 7. Command Categories

| Category | Purpose | Required at `RPP/1.0`? |
| --- | --- | --- |
| `identity.*` | Build / version / board identity | **YES** — at least `identity.version` |
| `status.*` | Read-only state and telemetry queries | **YES** — at least `status.query` |
| `lifecycle.*` | System-level control (arm / start / stop / reset / fault_clear) | OPTIONAL per product variant |
| `diagnostic.*` | Diagnostic operations *intentionally separated from the devkit-evidence surface* | OPTIONAL |
| `probe.*` | Probe-translator-style verbs that surface internal evidence through the public contract | **NO at `RPP/1.0`.** Reserved for a dedicated future product-decision phase |
| (future namespaces) | TBD | Future |

---

## 8. Required Minimal Command Set for `RPP/1.0`

| Command | Purpose | Args | Response payload | Statuses |
| --- | --- | --- | --- | --- |
| `identity.version` | Return protocol version + product / variant identifier + capability flags | none | `{ rpp_ver, product_id, variant_id, capabilities[] }` (shape per binding) | `OK` only |
| `status.query` | Return current state snapshot | none | `{ state, transitions, uart_count, button_count, ignored_count, … }` (shape per product variant; **distinct from devkit `STATE state=…` line**) | `OK` only |

Both commands are read-only and stateless. Both must be available at the
earliest possible point in the session (no precondition).

---

## 9. Optional / Future Commands

| Command | Purpose | Notes |
| --- | --- | --- |
| `lifecycle.arm` | Public arm verb | Distinct from devkit `a` byte; product semantics defined per variant |
| `lifecycle.start` | Public start verb | Distinct from devkit `s` byte |
| `lifecycle.stop` | Public stop verb | Distinct from devkit `r` byte; product semantics may differ from `reset` |
| `lifecycle.reset` | Public reset verb | Distinct from devkit `r` byte |
| `lifecycle.fault_clear` | Public fault clear verb | Reserved for FAULT-block follow-on |
| `diagnostic.snapshot` | Diagnostic state snapshot | **Must not surface RTT `ROBOTOS_*` line shapes verbatim** |
| `probe.snapshot` | Reserved — product mapping for `probe_translator` | Requires a dedicated product-decision phase before reservation becomes implementation |

All commands above are **optional** per product variant and must be
declared in the variant's `identity.version` capability descriptor.

---

## 10. Safety / Disallowed Shapes

| Constraint | Rule |
| --- | --- |
| No ISR-context parsing | Forbidden at any depth |
| No public command that re-emits a frozen devkit response shape verbatim | E.g., `status.query` MUST NOT emit `STATE state=N transitions=N button=N uart=N ignored=N` |
| No public command that mutates the in-session `RPP/<ver>` value | Version is per-session and per-message; not mutable mid-session |
| No silent fallback from framed to single-byte devkit commands | Transport bindings must keep the two surfaces disjoint by construction |
| No security-relevant command without `ERR_SECURITY_DENIED` handling | Even if the initial security model is "none", every security-relevant command must reserve `ERR_SECURITY_DENIED` |
| No heap allocation in parser / dispatcher hot paths | Carries forward Phase 9E-Z guard #5 |
| No core / platform UART abstraction promotion | Carries forward Phase 9E-Z guard #8 |
| No promotion of `devkit_app_state` to Robot Framework | Carries forward Phase 9E-Z guard #11 |

---

## 11. Compatibility Rule

- `RPP/<MAJOR>.<MINOR>` is semver-flavored.
- `MAJOR` increments are **breaking**; callers using an earlier `MAJOR`
  must be rejected with `ERR_PROTOCOL`.
- `MINOR` increments are **backward-compatible additions**; earlier
  callers continue to work; new optional commands are declared via
  `identity.version` payload capability flags.
- A product variant declares its supported `RPP/<MAJOR>.<MINOR>` range
  in the `identity.version` response.
- A `MAJOR` increment without a published deprecation path for prior
  callers is forbidden.

---

## 12. Relationship to Existing Devkit Commands

| Devkit command (byte) | Class | Public-protocol equivalent (when future product semantics open) | Mapping rule |
| --- | --- | --- | --- |
| `a` | Devkit demo | `lifecycle.arm` | **NOT auto-mapped.** New public surface; new framing |
| `s` | Devkit demo | `lifecycle.start` | NOT auto-mapped |
| `r` | Devkit demo | `lifecycle.reset` | NOT auto-mapped |
| `d` | Devkit demo | TBD lifecycle verb (disarm or reset depending on product variant) | NOT auto-mapped |
| `?` | Devkit query | `status.query` | **Different response shape.** Devkit `STATE state=…` line is preserved for devkit-evidence only |
| `v` | Devkit query | `identity.version` | **Different response shape.** Devkit `INFO phase=10b-v …` line is preserved for devkit-evidence only |
| `L` | Devkit physical-effect probe | (no direct map) | Physical-effect probe, not a public control |
| `T` | Devkit hardware probe | (no direct map) | Hardware probe, not a public control |
| `x` and other | Devkit fallthrough | (no direct map) | Diagnostic-only |

The two surfaces coexist on the same physical UART (under the Phase
12N-pre-3 default binding) but are **not** the same protocol. A future
product operator uses `RPP/<ver>` frames; a future devkit operator may
keep using single-byte commands for hardware evidence captures.

---

## 13. Forbidden Surfaces (Phase 12N-pre-2 lock)

Until a successor planning gate (Phase 12N-pre-3) explicitly authorizes
otherwise, the following changes are forbidden:

| Forbidden change | Phase 12N-pre-2 status |
| --- | --- |
| Adding a new UART command byte to `devkit_uart_producer.c` | **FORBIDDEN** |
| Removing or repurposing any current UART command byte | **FORBIDDEN** |
| Changing existing UART TX response text or format | **FORBIDDEN** |
| Exposing `ROBOTOS_PROBE` or any `ROBOTOS_*` log line on UART TX | **FORBIDDEN** |
| Declaring `RPP/<ver>` literal in any tracked source / test / tool / script / log | **FORBIDDEN** |
| Creating a parser, dispatcher, or transport binding module | **FORBIDDEN** |
| Adding command framing (length, checksum, CRC, line markers) to any current source | **FORBIDDEN** |
| Adding command version negotiation to any current source | **FORBIDDEN** |
| ISR-context command parsing | **FORBIDDEN at any depth** |

---

## 14. Allowed Without Successor Planning Gate

The following are explicitly allowed without further authorization:

| Action | Notes |
| --- | --- |
| Citing this spec to reject ad-hoc protocol or command-surface changes | Required |
| Documentation cleanup that does not change command semantics | Allowed |
| Updating `02_PHASE_CLOSEOUTS/PHASE_*` cross-references to this spec | Allowed |
| Adding new RTT-only diagnostic log lines (under a separate phase contract) | Allowed only if no UART TX exposure and no new command bytes |

---

## 15. Future Implementation Gate Path

| # | Phase | Type | Outputs |
| --- | --- | --- | --- |
| 1 | **Phase 12N-pre-3** | Docs-only — Transport Binding Planning | Locks first transport binding (default: UART framed v0). Defines wire format, framing markers, escape rules, length/termination rules, coexistence rules, parser placement (thread-context only), host-test plan |
| 2 | **Phase 12N-spec** | Docs + host test | Implements parser + dispatcher in host-only form under `tests/host/`. Validates request/response framing, command vocabulary, error model |
| 3 | **Phase 12N-impl** | Firmware | Lands parser + dispatcher in `framework/` (transport-neutral core) + one transport binding module. Requires explicit user authorization separate from Phase 12N-pre-2 closure |
| 4 | **Phase 12N-hardware** | Hardware validation | Flashes firmware to `stm32f411e_disco` rev D; captures RTT + product-protocol UART transcript; new `run_phase12n_protocol_demo.ps1` script (template: Phase 12M demo script) |
| 5 | **Phase 12N-Z** | Docs-only checkpoint | Locks the result; defines what is product-proven vs. what remains devkit-evidence |

No step in this path is authorized at Phase 12N-pre-2 close. Step 1
itself requires user authorization.

---

## 16. Required Validation At a Future Implementation Phase

| # | Required validation |
| --- | --- |
| 1 | Host test suite: framed-protocol parser correctness; partial reads; malformed input; recovery; coexistence with single-byte devkit commands |
| 2 | Host regression: existing 23+ tests must still PASS |
| 3 | Zephyr build PASS for `stm32f411e_disco` rev D |
| 4 | Hardware validation under product-workload profile (new RTT + product-protocol UART transcript) |
| 5 | Backwards-compatibility check: existing devkit demo scripts (Phase 9D/9E/9G/10B/11D/12M) must still run unchanged |
| 6 | Boundary grep gates: no new ISR-context parsing; no heap; no scheduler change; no UART command-byte addition |
| 7 | Documentation: closeout doc, progress entry, INDEX entry, spec status upgrade |

---

## 17. Closeout Criteria for Phase 12N-pre-2

Phase 12N-pre-2 is closed when:

1. This spec is committed and the design direction in §3 / §4 / §5 is
   recorded.
2. The Phase 12N-pre-2 closeout doc is committed at
   `02_PHASE_CLOSEOUTS/PHASE_12N_PRE2_PRODUCT_PUBLIC_PROTOCOL_DECISION_PLAN.md`.
3. `CURRENT_STATE.md`, progress, and index reflect Phase 12N-pre-2 as
   the latest closed planning gate.
4. No source / CMake / config / test / tool / log file changes.

---

## 18. Open Decisions (after Phase 12N-pre-2)

| # | Open question | Status |
| --- | --- | --- |
| 1 | Final protocol name (RPP vs. alternative) | `USER_DECISION_REQUIRED at Phase 12N-pre-3` |
| 2 | First transport binding wire format (UART framed text vs. length-prefixed) | `USER_DECISION_REQUIRED at Phase 12N-pre-3` |
| 3 | UART framed v0 coexistence rule with single-byte devkit commands | `USER_DECISION_REQUIRED at Phase 12N-pre-3` |
| 4 | Initial security model (none vs. authenticated session) | `USER_DECISION_REQUIRED at Phase 12N-pre-3` |
| 5 | Whether `lifecycle.*` is required at `RPP/1.0` or optional | `USER_DECISION_REQUIRED at Phase 12N-pre-3` |
| 6 | Whether `diagnostic.*` is shipped at `RPP/1.0` or deferred | `USER_DECISION_REQUIRED at Phase 12N-pre-3` |
| 7 | Whether `probe.*` is ever shipped on the public channel | `USER_DECISION_REQUIRED; default DEFER` |
| 8 | Other transport bindings (USB CDC ACM, RTT-shell, BLE) | `NOT_STARTED` |
| 9 | UART TX response for `probe_translator` snapshot (carry-forward) | `NOT_STARTED; USER_DECISION_REQUIRED` |
| 10 | Multi-product coordination | `NOT_STARTED` |
| 11 | F407 / custom-board command surface | `HOLD/DEFER` |
| 12 | Bridge ABI memory-layout lock | `NOT_STARTED` |
| 13 | FAULT adapter event sourcing | `NOT_STARTED` |
| 14 | Non-NULL action / on_entry / on_exit callbacks | `NOT_STARTED` |
| 15 | Full `probe_translator` input matrix proof | `NOT_STARTED` |
| 16 | ACTIVE disarm widening | `USER_DECISION_REQUIRED_ACTIVE_DISARM` |
| 17 | Scheduler 7A/7B | `DEFER` |
| 18 | Button-path hardware validation | `NOT_STARTED` |
