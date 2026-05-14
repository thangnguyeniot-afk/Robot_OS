# Phase 12N-pre-2 — Product / Public Protocol Decision Plan

**Status:** `CLOSED_DOCS_ONLY`
**Decision:** `PHASE_12N_PRE2_PRODUCT_PUBLIC_PROTOCOL_DECISION_PLAN_CLOSED`
**Date:** 2026-05-14
**Type:** Docs-only product/public protocol decision planning gate. Successor of Phase 12N-pre.
**Prior phase anchor:** [`PHASE_12N_PRE_PRODUCT_WORKLOAD_COMMAND_ADMISSION_PLAN.md`](PHASE_12N_PRE_PRODUCT_WORKLOAD_COMMAND_ADMISSION_PLAN.md)
**Phase 12M anchor:** [`PHASE_12M_PROBE_TRANSLATOR_HARDWARE_VALIDATION.md`](PHASE_12M_PROBE_TRANSLATOR_HARDWARE_VALIDATION.md)
**Long-lived spec produced here:** [`../03_SPECS/PRODUCT_PUBLIC_PROTOCOL_DECISION_PLAN.md`](../03_SPECS/PRODUCT_PUBLIC_PROTOCOL_DECISION_PLAN.md)

---

## A. Executive Summary

Phase 12N-pre-2 is a **docs-only planning gate** opened in response to an
explicit user decision: *"I want to design public protocol for RobotOS."*
That decision authorizes a public-protocol design discussion **at the
docs/spec level only**; it does **not** authorize implementation, UART
behavior change, firmware change, hardware validation, or product
command mapping.

Phase 12N-pre established that the existing single-byte UART command
set `a/s/r/?/x/v/L/d/T` is **devkit demo and evidence**, not a public
contract. Phase 12N-pre-2 takes the next bounded step: it audits the
full current command/evidence surface, defines what *"public protocol"*
must mean for RobotOS, evaluates six candidate strategies, and selects
a recommended direction with a draft protocol outline ready for a
future implementation planning gate.

**Recommended direction (this gate):**

- **Do not promote** the devkit single-byte UART command set to public
  protocol.
- **Design a transport-neutral public command contract first** — a
  versioned, command-categorized request/response model that is not
  tied to UART byte semantics.
- **Sketch a future UART framed-protocol v0** as the first candidate
  transport binding, defined at conceptual level only.
- **Keep `a/s/r/?/x/v/L/d/T` as devkit-evidence**, preserving Phase
  9E/10B/11D/12M evidence integrity.
- **Require another planning gate** (Phase 12N-pre-3 or equivalent)
  before any code change, host-test, or firmware work.

**Decision:** `PHASE_12N_PRE2_PRODUCT_PUBLIC_PROTOCOL_DECISION_PLAN_CLOSED`

---

## B. Baseline From Phase 12N-pre

| Item | Value |
| --- | --- |
| HEAD at Phase 12N-pre-2 open | `3cf79e4 docs: reconcile devkit guide open gate map` |
| `origin/master` at open | `3cf79e4` (synced) |
| Phase 12N-pre status | `PHASE_12N_PRE_PRODUCT_WORKLOAD_COMMAND_ADMISSION_PLAN_CLOSED` |
| Phase 12M hardware evidence | `phase_12M_rtt_2026-05-14.txt` (44,341 B); `_SEGGER_RTT=0x20000b38`; 15/15 gates PASS |
| Phase 12L adapter | `devkit/src/devkit_probe_adapter.{c,h}` runtime-admitted |
| Product command mapping | `NOT_STARTED; USER_DECISION_REQUIRED`; Phase 12N implementation `NOT AUTHORIZED` |
| Working tree | Clean on all tracked files (`.vscode/settings.json` drift unrelated) |
| User authorization for this phase | "I want to design public protocol for RobotOS" — docs/spec design only |

---

## C. Current Command / Evidence Surface Inventory

This audit re-inventories every surface that could plausibly be promoted
to, mapped through, or replaced by a future public protocol. Every
surface is classified into one of six categories defined in §H.

### C.1 UART RX command surface ([`devkit_uart_producer.c`](../../../devkit/src/devkit_uart_producer.c) lines 242–429)

| Byte(s) | Action | UART TX response | Probe dispatch (12L) | Class | Promote-to-protocol? |
| --- | --- | --- | --- | --- | --- |
| `'a'` / `'A'` | →ARMED (else no-op) | `OK state=ARMED[ unchanged=1]` | `(TYPE_CONFIG, ARG_NONE)` if accepted | **Devkit demo / evidence** | **NO** — frozen |
| `'s'` / `'S'` | →ACTIVE (else no-op) | `OK state=ACTIVE[ unchanged=1]` | `(TYPE_COMMAND, ARG_START)` if accepted | **Devkit demo / evidence** | **NO** — frozen |
| `'r'` / `'R'` | →IDLE (else no-op) | `OK state=IDLE[ unchanged=1]` | `(TYPE_COMMAND, ARG_RESET)` if accepted | **Devkit demo / evidence** | **NO** — frozen |
| `'d'` / `'D'` | ARMED→IDLE; else no-op | `OK disarm state=IDLE` / `OK disarm no-op state=N` | `(TYPE_COMMAND, ARG_RESET)` on ARMED→IDLE | **Devkit demo / evidence** | **NO** — frozen |
| `'?'` | state query | `STATE state=N transitions=N button=N uart=N ignored=N` | none | **Devkit query** | **NO** — frozen |
| `'v'` / `'V'` | build/version query | `INFO phase=10b-v app=devkit board=stm32f411e_disco tick_ms=500 uart=minimal` | none | **Devkit query** | **NO** — frozen (carries phase-tagged identity, not product identity) |
| `'l'` / `'L'` | LED toggle | `OK led=toggle state=N` / `ERR led=toggle ret=N state=N` | none | **Devkit physical-effect probe** | **NO** — frozen |
| `'t'` / `'T'` | accelerometer probe | `ACC x=v.v y=v.v z=v.v` / `ERR accel read=N` | none | **Devkit hardware probe** | **NO** — frozen |
| any other byte (`'x'`, ...) | none; `ignored_count++` | `ERR ignored byte=0xNN state=N` | none | **Devkit fallthrough (explicit-ignore probe)** | **NO** — must remain a recognizable diagnostic |

**Source-verified:** the switch table in `devkit_uart_emit_tx_response()`
([`devkit_uart_producer.c`](../../../devkit/src/devkit_uart_producer.c)
lines 242–348) contains exactly these arms, plus the `default:` ignore
arm at line 348+. The command set has not changed since Phase 11Z.

### C.2 Button input surface ([`devkit_button_producer.c`](../../../devkit/src/devkit_button_producer.c) → `devkit_app_state_on_button()`)

Physical EXTI → debounce → core event → handler → app-state cycle
IDLE → ARMED → ACTIVE → IDLE. Phase 12L probe dispatch is wired BEHIND
the cycle (same `(type, arg0)` mapping as UART path).

| Cycle position | App transition | Probe dispatch | Class | Promote-to-protocol? |
| --- | --- | --- | --- | --- |
| Press 1 (from IDLE) | IDLE→ARMED | `(TYPE_CONFIG, ARG_NONE)` | **Devkit demo / evidence** | **NO** — physical-only input |
| Press 2 (from ARMED) | ARMED→ACTIVE | `(TYPE_COMMAND, ARG_START)` | **Devkit demo / evidence** | **NO** |
| Press 3 (from ACTIVE) | ACTIVE→IDLE | `(TYPE_COMMAND, ARG_RESET)` | **Devkit demo / evidence** | **NO** |

Button is a physical input, not a protocol channel. It is not a
transport candidate.

### C.3 RTT-only internal evidence

| Surface | Module | Visibility | Class | Promote-to-protocol? |
| --- | --- | --- | --- | --- |
| `ROBOTOS_PROBE state=… trans=… events=… …` | `devkit_probe_adapter.c` | RTT log only | **Internal evidence** | **NO** — must remain RTT-only (12N-pre Rule 4) |
| `ROBOTOS_OBS …`, `ROBOTOS_FAULT …`, `ROBOTOS_PROD …`, `ROBOTOS_BTN …`, `ROBOTOS_UART …`, `ROBOTOS_APP …` | observability / fault subsystems | RTT log only | **Internal evidence** | **NO** (diagnostic only) |
| `Phase 9C app transition: …` | `devkit_app_state.c` | RTT log only | **Internal evidence** | **NO** |

### C.4 Devkit-local APIs (not externally exposed)

| API | Module | Class | Promote-to-protocol? |
| --- | --- | --- | --- |
| `devkit_probe_adapter_{init,dispatch,log_snapshot}()` | [`devkit_probe_adapter.h`](../../../devkit/src/devkit_probe_adapter.h) | **Internal runtime API** | **NO** — internal evidence wiring |
| `devkit_app_state_*()` | [`devkit_app_state.h`](../../../devkit/src/devkit_app_state.h) | **Internal runtime API** | **NO** — devkit-local |
| `probe_translator_*()` | [`app/probe_translator/probe_translator.h`](../../../app/probe_translator/probe_translator.h) | **Internal application API** | **NO at this gate** — could become a backing service for future product verbs after a dedicated phase |

### C.5 Host demo scripts ([`tools/runtime/`](../../../tools/runtime/))

| Script | Sequence | Class | Promote-to-protocol? |
| --- | --- | --- | --- |
| `run_phase9d_demo.ps1` | UART + button workload | **Devkit demo harness** | **NO** |
| `run_phase9e_uart_response_demo.ps1` | `a s ? r x` | **Devkit demo harness** | **NO** |
| `run_phase9g_uart_burst_demo.ps1` | 5-byte burst | **Devkit demo harness** | **NO** |
| `run_phase10b_v_build_query_demo.ps1` | `v a v r ?` | **Devkit demo harness** | **NO** |
| `run_phase10b_l_led_command_demo.ps1` | `L v L ?` | **Devkit demo harness** | **NO** |
| `run_phase10b_d_disarm_demo.ps1` | `d a d ?` | **Devkit demo harness** | **NO** |
| `run_phase11d_accel_probe_demo.ps1` | `T T ?` | **Devkit demo harness** | **NO** |
| `run_phase12m_probe_demo.ps1` | `a s r ?` | **Devkit demo harness** | **NO** |
| `capture_devkit_rtt.ps1` | RTT capture | **Devkit diagnostic** | **NO** — diagnostic plumbing |
| `capture_phase6h_runtime.ps1`, `phase6h_read.gdb`, `phase6z_required_patterns.txt` | historical evidence | **Devkit diagnostic** | **NO** |

### C.6 Existing protocol-adjacent docs

| Doc | Status | Public protocol relevance |
| --- | --- | --- |
| [`03_SPECS/COMMAND_SET_DRAFT.md`](../03_SPECS/COMMAND_SET_DRAFT.md) | Devkit command vocabulary table (Phase 10A/10C/11Z) | **Not a public protocol** despite its title — locked as devkit-evidence at Phase 12N-pre |
| [`03_SPECS/TELEMETRY_REFERENCE.md`](../03_SPECS/TELEMETRY_REFERENCE.md) | Telemetry line definitions (RTT) | **Not a public protocol** — diagnostic logs |
| [`03_SPECS/PRODUCT_WORKLOAD_COMMAND_ADMISSION_PLAN.md`](../03_SPECS/PRODUCT_WORKLOAD_COMMAND_ADMISSION_PLAN.md) | Phase 12N-pre taxonomy spec | **Boundary spec** — names the devkit-vs-product gap that this phase fills |
| [`02_PHASE_CLOSEOUTS/PHASE_10C_COMMAND_SET_CHECKPOINT.md`](PHASE_10C_COMMAND_SET_CHECKPOINT.md), [`PHASE_11Z_COMMAND_SET_CHECKPOINT.md`](PHASE_11Z_COMMAND_SET_CHECKPOINT.md) | devkit-evidence command freeze | Authoritative freeze of the devkit set; **not promoted** at any phase |

### C.7 Surface-class summary

| Class | Population at Phase 12N-pre-2 |
| --- | --- |
| Internal runtime API | `devkit_probe_adapter_*`, `devkit_app_state_*`, `probe_translator_*` |
| Devkit evidence surface (UART RX/TX) | `a/s/r/?/x/v/L/d/T` + frozen TX response formats |
| Devkit diagnostic surface | RTT `ROBOTOS_*` log lines; capture/demo scripts |
| Candidate product command surface | **NONE today.** This phase will *design* one at conceptual level only. |
| Forbidden-to-promote surface | All devkit-evidence UART byte semantics; all RTT log lines |
| Transport candidate | UART (existing, framed v0 — future); USB CDC ACM (not present in repo); RTT-shell (diagnostic only); BLE / network (not present) |

---

## D. Definition of "Public Protocol" for RobotOS

A *public protocol* in RobotOS is the **externally-stable command and
response contract** that a deployed RobotOS product exposes to its
operator, host application, or upstream system. It is distinct from any
internal API, devkit-evidence channel, or diagnostic surface.

A public protocol for RobotOS must, at minimum, define:

| # | Property | Why it matters |
| --- | --- | --- |
| 1 | **Stable command vocabulary** with versioning | Permits forward evolution without breaking deployed callers |
| 2 | **Versioned message grammar** (request/response shape, framing rules) | Lets callers detect compatibility ahead of dispatch |
| 3 | **Explicit request/response frame** (begin/end markers; correlation; length or termination) | Allows partial-read recovery, pipelining, and parser correctness on lossy transports |
| 4 | **Status / error vocabulary** (success, recoverable, fatal, security-denied, etc.) | Distinguishes recoverable from unrecoverable conditions cleanly |
| 5 | **Compatibility promise** (semver-like rules; deprecation; capability negotiation) | Lets product variants share a protocol without per-variant forks |
| 6 | **Transport policy** (which transports are authorized; binding rules; framing per transport) | Decouples logical commands from any single wire |
| 7 | **Observability / evidence policy** (what is logged on RTT vs. emitted on the protocol channel) | Keeps diagnostic vs. product channels separated by construction |
| 8 | **Safety constraints** (which commands are unsafe, gated, or forbidden in which states; ISR-context bans; idempotency) | Prevents the protocol from becoming a foot-gun |
| 9 | **Rollback / migration path** (how to retract a published command without abandoning callers) | Mandatory under any public-contract discipline |

Anything that does not satisfy items 1–9 is **not** a RobotOS public
protocol. The devkit single-byte UART set fails on items 1, 2, 3, 4, 5,
7, and 9 by construction.

---

## E. Candidate Protocol Strategies

Six strategies were evaluated for Phase 12N-pre-2.

### E.1 Option 1 — HOLD / no public protocol yet

**Shape:** Keep current devkit-evidence commands only. Do not open a
public protocol at any depth.

| Property | Assessment |
| --- | --- |
| Affected modules | None |
| Likely files | None |
| Behavior risk | **NONE** |
| Public compatibility risk | **NONE** |
| Test burden | None |
| Hardware validation burden | None |
| Rollback path | Trivial (nothing to roll back) |
| Phase 12N implementation? | **NO** |
| **Verdict** | **VIABLE** as fallback; but the user has explicitly asked to *design* a public protocol, so HOLD alone does not satisfy the request. Re-classified as the fallback if no other option is approved. |

### E.2 Option 2 — Promote existing UART single-byte commands

**Shape:** Re-classify `a/s/r/?/v/L/d/T` and the frozen TX response
formats as public protocol commands. Add documentation and treat the
existing format as the public surface.

| Property | Assessment |
| --- | --- |
| Affected modules | `devkit/src/devkit_uart_producer.{c,h}` (semantically, even with zero-diff); all 8 demo scripts; `COMMAND_SET_DRAFT.md`; every Phase 9–12 closeout |
| Likely files | None changed for behavior; many docs would need re-classification |
| Behavior risk | **MEDIUM** — present behavior keeps working, but any future cleanup is now blocked by a public contract |
| Public compatibility risk | **HIGH** — locks a single-byte, no-framing, no-versioning, no-error-vocabulary surface into permanent product compatibility |
| Test burden | Full backward-compatibility regression on every future change |
| Hardware validation burden | Permanent; every product variant must replay the Phase 9E/10B/11D/12M evidence |
| Rollback path | **VERY HARD** — once a public protocol byte is published, retracting it breaks deployed callers |
| Phase 12N implementation? | Yes, by re-labeling — but the contract is irretrievably weak |
| **Verdict** | **REJECTED.** The frozen devkit set was never designed under the items in §D; promoting it as-is would lock RobotOS into the weakest possible public surface for the entire product lifetime. This re-confirms the Phase 12N-pre Option 4 rejection. |

### E.3 Option 3 — UART framed protocol v0

**Shape:** Keep `a/s/r/?/x/v/L/d/T` as devkit-evidence (unchanged). Add
a future, separately-framed text protocol on the same UART, e.g.:
`ROBOTOS/0.1 <cmd> <args>\r\n` request → `ROBOTOS/0.1 <id> OK <result>\r\n`
or `ROBOTOS/0.1 <id> ERR <code> <msg>\r\n` response. Single physical UART;
two logical surfaces distinguished by framing.

| Property | Assessment |
| --- | --- |
| Affected modules | A new parser module under `devkit/src/` or (preferred) `framework/`; possibly an explicit RX state machine to separate framed vs. single-byte input |
| Likely files | New parser source/header; new host-test target; new demo script; the existing `devkit_uart_producer.c` would need a separate framed-input dispatch path |
| Behavior risk | **MEDIUM** — coexistence of single-byte devkit commands and multi-byte framed commands on the same UART line is non-trivial (RX disambiguation, partial-frame recovery, ISR boundary) |
| Public compatibility risk | **MEDIUM** — versioned grammar mitigates risk, but UART framing choices (markers, length, escaping) commit to a wire format |
| Test burden | Full parser test suite (host); framing edge cases; coexistence regression; hardware workload validation |
| Hardware validation burden | New `run_phase*_protocol_demo.ps1`; RTT + UART capture |
| Rollback path | **Moderate** — protocol channel can be versioned out, but the parser code lives in firmware |
| Phase 12N implementation? | **NOT at this gate.** Would require Phase 12N-pre-3 (or equivalent) and Phase 12N-impl. |
| **Verdict** | **VIABLE as a first transport binding,** *after* a transport-neutral command contract is defined (Option 4). |

### E.4 Option 4 — Transport-neutral command model first

**Shape:** Define the public protocol at conceptual level first — a
versioned command vocabulary, command categories, request/response
shape, status/error model, and safety constraints — *without* tying it
to UART byte semantics. The transport (UART framed v0, USB CDC ACM,
RTT-shell, BLE) is a separate binding.

| Property | Assessment |
| --- | --- |
| Affected modules | NONE at this gate (docs/spec only); future implementation would land in `framework/` (transport-neutral command dispatcher) and per-transport binding modules |
| Likely files | At this gate: only docs (closeout + spec). At a future implementation gate: a `framework/`-layer dispatcher and one transport binding |
| Behavior risk | **NONE at this gate** |
| Public compatibility risk | **LOWEST** — defining the model first protects RobotOS from accidentally locking transport-specific footguns into the public contract |
| Test burden | Spec review at this gate; full parser + dispatcher test suite at future gate |
| Hardware validation burden | None at this gate |
| Rollback path | Trivial at this gate (revert docs) |
| Phase 12N implementation? | **NO at this gate.** Implementation requires Phase 12N-pre-3 (transport-binding planning) + Phase 12N-impl. |
| **Verdict** | **RECOMMENDED.** Highest leverage, lowest risk, satisfies the user request to *design* a public protocol, and explicitly defers any implementation cost. Pairs naturally with Option 3 as the first authorized transport. |

### E.5 Option 5 — USB CDC / shell / future protocol transport

**Shape:** Adopt a non-UART transport (USB CDC ACM, RTT-shell, BLE GATT,
TCP) as the public-protocol channel.

| Property | Assessment |
| --- | --- |
| Affected modules | Major — would require new Zephyr subsystems, `prj.conf` changes, DTS/overlay changes, new framework subsystems |
| Likely files | Many (new |
| Behavior risk | **HIGH** — pulls in subsystems not currently enabled |
| Public compatibility risk | **MEDIUM** (well-defined if a standard transport is chosen) |
| Test burden | Very large |
| Hardware validation burden | New hardware paths; per-transport validation |
| Rollback path | Complex |
| Phase 12N implementation? | **NO.** Far out of scope for any near-term gate. |
| **Verdict** | **DEFERRED.** Listed only to acknowledge the candidate; not viable for Phase 12N-pre-3 or Phase 12N-impl. Future option only. |

### E.6 Option 6 — RTT/debug-only protocol

**Shape:** Define a protocol that lives only on the RTT debug channel
(e.g., a shell or command interface over RTT).

| Property | Assessment |
| --- | --- |
| Affected modules | RTT subsystem only |
| Likely files | New RTT shell binding |
| Behavior risk | LOW |
| Public compatibility risk | **N/A** — RTT is not a product-facing surface; debug-only |
| Test burden | LOW |
| Rollback path | Trivial |
| Phase 12N implementation? | Not useful — would not satisfy the *public* protocol requirement |
| **Verdict** | **REJECTED for public-protocol purposes.** RTT is a development/diagnostic surface, not a product surface. Worth noting as a diagnostic-only candidate, but not the answer to "I want to design public protocol for RobotOS." |

### E.7 Strategy summary

| Option | Verdict |
| --- | --- |
| 1 — HOLD | VIABLE fallback only |
| 2 — Promote devkit UART set | **REJECTED** |
| 3 — UART framed protocol v0 | **VIABLE** as first transport binding *after* Option 4 |
| 4 — Transport-neutral command model first | **RECOMMENDED** |
| 5 — USB CDC / other future transport | **DEFERRED** |
| 6 — RTT/debug-only | **REJECTED** for product-protocol; diagnostic only |

---

## F. Recommended Protocol Direction

### F.1 Recommendation

**Adopt Option 4 (transport-neutral command model first), with Option 3
(UART framed protocol v0) named as the first authorized transport
binding candidate.**

Phase 12N-pre-2 produces a draft public-protocol outline at conceptual
level (see §G) and a long-lived spec
[`../03_SPECS/PRODUCT_PUBLIC_PROTOCOL_DECISION_PLAN.md`](../03_SPECS/PRODUCT_PUBLIC_PROTOCOL_DECISION_PLAN.md).
No firmware change. No host-test change. No UART semantics change.

### F.2 Rationale

- The user explicitly authorized **designing** a public protocol, not
  implementing one. Option 4 maximizes design value at the minimum
  scope footprint.
- Defining commands transport-neutrally first protects RobotOS from
  permanently inheriting UART-byte limitations (no framing, no
  versioning, no correlation) into the public contract.
- A transport-neutral model also makes it natural to bind the same
  commands to additional transports later (USB CDC, RTT-shell, BLE)
  without breaking the public contract.
- The Phase 12 chain (12K → 12K-Z → 12L → 12L-Z → 12M → 12M-Z →
  12N-pre) is complete and self-consistent. Layering a parallel
  public-protocol surface on top, rather than mutating the frozen
  devkit set, preserves all hardware-evidence integrity established
  in Phase 12M.

### F.3 What Phase 12N-pre-2 produces today

- This closeout doc.
- New long-lived spec `PRODUCT_PUBLIC_PROTOCOL_DECISION_PLAN.md`.
- Doc-sync to `CURRENT_STATE.md`, progress, and index.

### F.4 What is *not* produced today

- No protocol implementation, parser, dispatcher, or runtime hook.
- No UART command set change.
- No UART TX response format change.
- No host test.
- No build run.
- No flash / RTT / hardware run.

---

## G. Draft Public Protocol Outline (conceptual; non-binding)

> **Status:** **DRAFT / NON-FINAL — CONCEPTUAL ONLY.** This outline is a
> first-pass design sketch produced at Phase 12N-pre-2. It is not a
> ratified contract. Any binding contract requires a successor planning
> gate (Phase 12N-pre-3 transport-binding planning) plus user approval.

### G.1 Provisional name and version scheme

| Item | Provisional value |
| --- | --- |
| Protocol name (working) | **RPP — RobotOS Public Protocol** |
| Versioning | `RPP/<MAJOR>.<MINOR>` (semver-flavored; MAJOR breaks compatibility; MINOR adds backward-compatible capabilities) |
| Initial design version | `RPP/0.1` — pre-product, **design-only**; no implementation status |
| First product-eligible version | `RPP/1.0` — only when a future phase ships a stable command set with hardware evidence |

`RPP/0.x` is **explicitly pre-product**. It exists as a docs-only design
target. No code declares `RPP/0.1` at this gate.

### G.2 Transport assumption

The protocol is defined transport-neutrally. The first authorized
transport binding candidate is **UART framed v0** (Option 3); other
bindings (USB CDC, RTT-shell, BLE) are explicitly future work. Every
transport binding must:

- Carry the protocol version on every message.
- Provide a framing rule sufficient to recover from partial reads.
- Preserve the request/response correlation defined in §G.3.
- Not silently fall back to single-byte devkit commands.

### G.3 Request / response framing (conceptual)

A request is conceptually:

```text
RPP/<ver> REQ <id> <namespace>.<verb> [<args>]
```

A response is conceptually:

```text
RPP/<ver> RES <id> <status> [<payload>]
```

| Field | Purpose | Notes |
| --- | --- | --- |
| `<ver>` | Protocol version | Allows compatibility detection per message |
| `REQ` / `RES` | Direction marker | Permits the parser to disambiguate without context |
| `<id>` | Caller-chosen correlation token | Numeric or short opaque; mandatory for product use; permits pipelining and late responses |
| `<namespace>.<verb>` | Command name | `status.query`, `lifecycle.arm`, `identity.version`, etc. |
| `<args>` / `<payload>` | Command arguments / response payload | Shape defined per-command; conceptually a small structured map |
| `<status>` | Status token | See §G.4 |

The on-the-wire form (line-delimited text vs. length-prefixed binary)
is a **transport-binding decision**, not a protocol-level decision. The
UART framed v0 candidate would use a line-delimited textual encoding;
other transports could choose different encodings provided they
preserve the fields above.

### G.4 Status / error vocabulary (conceptual)

| Token | Class | Meaning |
| --- | --- | --- |
| `OK` | success | Command accepted and completed (or accepted for async completion if so noted) |
| `ERR_UNKNOWN_COMMAND` | recoverable | Unknown `<namespace>.<verb>` |
| `ERR_INVALID_ARGS` | recoverable | Args present but malformed or out of range |
| `ERR_STATE_NOT_ALLOWED` | recoverable | Command rejected in current state (e.g., `lifecycle.start` from `IDLE`) |
| `ERR_BUSY` | recoverable | Implementation is mid-operation; caller may retry |
| `ERR_HARDWARE` | recoverable / fatal (per-command) | A backing hardware operation failed |
| `ERR_NOT_IMPLEMENTED` | recoverable | Command is reserved but not implemented in current version |
| `ERR_SECURITY_DENIED` | fatal-per-session | Caller is not authorized for this command |
| `ERR_PROTOCOL` | fatal-per-frame | Frame violated protocol grammar; parser-detected |

Each command must declare which status tokens it can produce and
whether each is recoverable, retryable, or fatal.

### G.5 Command categories (conceptual)

| Category | Purpose | Required at `RPP/1.0`? |
| --- | --- | --- |
| `identity.*` | Build / version / board identity | **YES** — at least `identity.version` |
| `status.*` | Read-only state and telemetry queries | **YES** — at least `status.query` |
| `lifecycle.*` | System-level control (arm / start / stop / reset / fault-clear) | OPTIONAL per product variant |
| `diagnostic.*` | Diagnostic operations *intentionally separated from product surface* — must never silently re-expose devkit-evidence channels | OPTIONAL |
| `probe.*` | Probe-translator-style verbs that surface internal evidence through the public contract | **NO at `RPP/1.0`.** Reserved for a separate future product-decision phase |

### G.6 Required minimal command set for `RPP/1.0` (conceptual)

| Command | Purpose | Notes |
| --- | --- | --- |
| `identity.version` | Returns protocol version + product / variant identifier | Mandatory |
| `status.query` | Returns current state snapshot | Mandatory; shape per product variant |

All other commands are **optional per product variant** and must be
declared in the variant's capability descriptor (`identity.version`
response payload).

### G.7 Optional / future commands (conceptual)

| Command | Notes |
| --- | --- |
| `lifecycle.arm` / `.start` / `.stop` / `.reset` / `.fault_clear` | Public mapping of the lifecycle vocabulary; **explicitly distinct** from devkit `a/s/r/d` bytes |
| `diagnostic.snapshot` | Diagnostic state snapshot; must not surface RTT-internal `ROBOTOS_PROBE`/`ROBOTOS_OBS` line shapes verbatim |
| `probe.snapshot` | Reserved — would be a product mapping for `probe_translator`; requires a dedicated product-decision phase before reservation becomes implementation |

### G.8 Safety / disallowed commands

| Constraint | Rule |
| --- | --- |
| No ISR-context parsing | Forbidden at all depths (carries forward Phase 9E-Z guard #10) |
| No command that re-exposes the frozen devkit UART byte responses verbatim | A public `status.query` MUST NOT emit the existing `STATE state=N transitions=N button=N uart=N ignored=N` line shape; the devkit shape is preserved for devkit-evidence only |
| No command that mutates the public `RPP/<ver>` version mid-session | Version is per-session and per-message; not mutable |
| No silent fallback from framed to single-byte devkit commands | Transports must keep the two surfaces disjoint by construction |
| No security-relevant command without `ERR_SECURITY_DENIED` handling | Even if the initial security model is "none", every security-relevant command must reserve the error class |

### G.9 Compatibility rule (conceptual)

- `RPP/<MAJOR>.<MINOR>` is semver-flavored.
- `RPP/<MAJOR>` increments are **breaking**; older callers must be
  rejected with `ERR_PROTOCOL`.
- `RPP/<MAJOR>.<MINOR>` increments are **backward-compatible additions**;
  older callers continue to work; new optional commands are declared
  via `identity.version` payload.
- A product variant declares its supported `RPP/<MAJOR>.<MINOR>` range
  in the `identity.version` response.

### G.10 Relationship to current devkit commands

| Devkit command | Public-protocol equivalent (when future product semantics open) | Mapping rule |
| --- | --- | --- |
| `a` / `s` / `r` / `d` (bytes) | `lifecycle.arm` / `.start` / `.reset` / (TBD disarm verb) | **NOT auto-mapped.** The devkit byte stays a devkit byte. A future `lifecycle.*` command is a new public surface with its own framing, args, and status |
| `?` (byte) | `status.query` | **Different response shape.** The public `status.query` returns a structured payload, not the `STATE state=N…` devkit line |
| `v` (byte) | `identity.version` | **Different response shape.** The public `identity.version` returns protocol version + product / variant identity, not the `INFO phase=10b-v …` devkit line |
| `L` (byte) | (no direct map) | Physical-effect probe; not a product control |
| `T` (byte) | (no direct map) | Hardware probe; not a product control |
| `x` and fallthrough | (no direct map) | Diagnostic-only |

The two surfaces coexist on the same physical UART under Option 3, but
are **not** the same protocol. A future operator that wants product
control must use `RPP/<ver>` frames; a future devkit operator may keep
using single-byte commands for hardware evidence captures.

---

## H. Product vs. Devkit-Evidence Boundary (carried forward)

The Phase 12N-pre boundary rule is preserved without weakening:

| Rule | Status at Phase 12N-pre-2 |
| --- | --- |
| Single-byte UART command set is frozen as devkit-evidence | **UNCHANGED** |
| UART TX response formats are frozen at Phase 9E / 10B / 11D shape | **UNCHANGED** |
| Internal evidence (`ROBOTOS_PROBE` snapshot, `ROBOTOS_*` log lines) stays RTT-only | **UNCHANGED** |
| No automatic promotion from devkit-evidence to product status | **UNCHANGED** |
| Product mapping requires a successor planning gate | **PARTIALLY ADVANCED** — Phase 12N-pre-2 is a planning gate; a *further* planning gate (Phase 12N-pre-3, transport binding) is still required before any code change |
| RPP/0.x is design-only | **NEW.** No code declares `RPP/0.x` |
| RPP/1.0 is product-eligible only after a dedicated future phase ships a stable command set with hardware evidence | **NEW** |

---

## I. Future Implementation Gate Requirements

If a future user decision authorizes any code change to advance public
protocol, the path is:

1. **Phase 12N-pre-3 — Transport Binding Planning (docs-only).** Selects
   the first transport binding (default: UART framed v0). Locks the
   wire format, framing markers, escape rules, length/termination rules,
   coexistence rules with the existing single-byte devkit commands,
   parser placement (thread-context only; no ISR), and host-test plan.
   No firmware change.

2. **Phase 12N-spec (docs + host test).** Implements the parser /
   dispatcher in host-only form under `tests/host/`. Validates
   request/response framing, command vocabulary correctness, and
   error model. No firmware change.

3. **Phase 12N-impl (firmware).** Lands the parser / dispatcher in
   `framework/` (transport-neutral core) and one transport binding
   module. Requires Phase 12N-pre-3 + Phase 12N-spec closed plus an
   **explicit user authorization separate from Phase 12N-pre-2
   closure**.

4. **Phase 12N-hardware (validation).** Flashes the firmware to
   `stm32f411e_disco` rev D, captures RTT + product-protocol UART
   transcript, validates each PASS/FAIL criterion. Pre-flight `_SEGGER_RTT`
   address check; new `run_phase12n_protocol_demo.ps1` (template:
   Phase 12M `run_phase12m_probe_demo.ps1`).

5. **Phase 12N-Z (checkpoint).** Locks the result; defines what is
   product-proven vs. what remains devkit-evidence.

### I.1 Allowed files at a future implementation phase (after explicit user decision)

| Path | Condition |
| --- | --- |
| New `framework/robotos_fw_protocol.{c,h}` (transport-neutral dispatcher) | Only if Phase 12N-pre-3 locks the API and Phase 12N-spec ships host parser tests first |
| New UART framed-v0 binding module | Only with Phase 12N-pre-3 locking framing rules |
| New `tests/host/test_*_protocol_*.c` files | Allowed under Phase 12N-spec |
| New `tools/runtime/run_phase12n_protocol_demo.ps1` | Allowed under Phase 12N-hardware |
| Closeout / progress / index doc-sync | Allowed |

### I.2 Forbidden files at any phase up to and including Phase 12N-pre-2

| Path | Reason |
| --- | --- |
| `RobotOS_v1.0/devkit/src/devkit_uart_producer.{c,h}` | Frozen UART surface; no public-protocol mapping at this gate |
| `RobotOS_v1.0/devkit/src/devkit_app_state.{c,h}` | App-state semantics are devkit-evidence; not a product contract |
| `RobotOS_v1.0/devkit/src/devkit_probe_adapter.{c,h}` | Adapter is internal evidence; no UART TX exposure |
| `RobotOS_v1.0/framework/*` | No new dispatcher at this gate |
| `RobotOS_v1.0/app/probe_translator/*` | No product mapping at this gate |
| `RobotOS_v1.0/core/*`, `RobotOS_v1.0/platform/*` | Zero-diff |
| Any new UART command byte | Frozen `a/s/r/?/x/v/L/d/T` |
| Any new UART TX response format | Frozen |
| `RobotOS_v1.0/devkit/prj.conf` | Zero-diff at all gates ≤ Phase 12N-pre-2 |
| DTS / overlay | Zero-diff |
| New ISR-context parsing | Forbidden at any depth |

### I.3 Required validation at a future implementation phase

| # | Required validation |
| --- | --- |
| 1 | Host test suite: framed-protocol parser correctness; partial reads; malformed input; recovery; coexistence with single-byte devkit commands |
| 2 | Host regression: existing 23+ tests must still PASS |
| 3 | Zephyr build PASS for `stm32f411e_disco` rev D |
| 4 | Hardware validation under product-workload profile (new RTT + product-UART transcript) |
| 5 | Backwards-compatibility check: existing devkit demo scripts (Phase 9D/9E/9G/10B/11D/12M) must still run unchanged |
| 6 | Boundary grep gates: no new ISR-context parsing; no heap; no scheduler change; no UART command-byte addition |
| 7 | Documentation: closeout doc, progress entry, INDEX entry, spec status upgrade |

---

## J. Risks / Rollback Signals

| Risk | Signal | Action |
| --- | --- | --- |
| Phase 12N-pre-2 closeout is misread as authorizing implementation | A new firmware diff appears citing this phase as authority | Hard stop; cite §F.4 and §K; close as `BLOCKED_NO_IMPLEMENTATION_AUTHORIZATION` |
| `RPP/0.x` is declared in source code | A C string `"RPP/0.1"` or similar appears in a tracked file | Hard stop; `RPP/0.x` is explicitly pre-product and design-only |
| Public protocol verbs leak into `devkit_uart_producer.c` | Any new `case` arm in the existing UART switch | Hard stop; the framed channel is a *separate* parser, not new bytes in the existing switch |
| Devkit response format mutates | `STATE state=…`, `OK state=…`, `INFO phase=10b-v …` line shape changes | Hard stop; these are devkit-evidence and frozen at Phase 11Z / 12M |
| `ROBOTOS_PROBE` line is exposed on UART | UART TX emits a string matching `ROBOTOS_PROBE state=…` | Hard stop; RTT-only per Phase 12N-pre Rule 4 |
| Phase 12N implementation is opened without Phase 12N-pre-3 | New parser / dispatcher diff lands without a transport-binding planning gate | Hard stop; close the offending phase as `BLOCKED_NO_TRANSPORT_BINDING_PLANNING` |
| Compatibility burden inflation | `RPP/<MAJOR>` is incremented without a deprecation path for prior callers | Hard stop; semver-flavored rule (§G.9) requires a deprecation path |
| Diagnostic leakage into product channel | `diagnostic.snapshot` payload contains RTT-line shapes verbatim | Stop; rework to a product-shaped payload that does not depend on RTT format |

---

## K. Explicit Non-Claims

Phase 12N-pre-2 (this planning gate) does NOT:

| Claim | Status |
| --- | --- |
| Public protocol implemented | **NOT IMPLEMENTED** |
| `RPP/0.x` declared in source code | **NOT DECLARED** |
| UART behavior changed | **NOT CHANGED** |
| UART TX response format changed | **NOT CHANGED** |
| New UART command byte added | **NOT ADDED** |
| Single-byte devkit command set promoted to public | **NOT PROMOTED** |
| Parser, dispatcher, or transport binding module created | **NOT CREATED** |
| Runtime / scheduler / framework behavior changed | **NOT CHANGED** |
| Hardware validation performed | **NOT PERFORMED** |
| Host test added or modified | **NOT TOUCHED** |
| Demo / tooling script added or modified | **NOT TOUCHED** |
| Build run | **NOT RUN** |
| Flash / RTT / UART session performed | **NOT PERFORMED** |
| Phase 12N implementation authorized | **NOT AUTHORIZED** — requires (Phase 12N-pre-3 + Phase 12N-spec) and a separate explicit user decision |
| F407 / custom board opened | **HOLD/DEFER — not opened** |
| Phase 12 chain status changed | **UNCHANGED** — Phase 12 chain remains complete with product boundary HOLD |
| Product command mapping opened in code | **NOT OPENED** |

---

## L. Next-Step Recommendation

**Default after Phase 12N-pre-2:** **HOLD** at the spec level. The
recommended direction (Option 4: transport-neutral first) is captured
in the long-lived spec and is ready for the next planning gate, but no
further action is required to keep RobotOS in a consistent state.

**If the user wants to advance one more docs-only gate:**

- Open **Phase 12N-pre-3 — Transport Binding Planning** (docs-only).
  Selects the first transport binding (default: UART framed v0). Locks
  framing markers, escape rules, length/termination, coexistence with
  single-byte devkit commands, parser placement, and host-test plan.
  No firmware change.

**If the user wants to advance toward code:**

- Phase 12N-pre-3 must close first (transport binding locked).
- Then Phase 12N-spec (host-only parser + dispatcher) under explicit
  user authorization.
- Then Phase 12N-impl (firmware) under a further explicit user
  authorization.
- Then Phase 12N-hardware (validation) and Phase 12N-Z (checkpoint).

**`USER_DECISION_REQUIRED`** for any path beyond HOLD. Phase 12N-pre-2
itself is now closed; no further action is implied by its closure.
