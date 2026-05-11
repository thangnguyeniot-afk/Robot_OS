# COMMAND_SET_DRAFT.md — RobotOS Devkit Product Command Vocabulary (DRAFT)

**Status:** `DRAFT` for remaining Section B rows. **Section A rows `v`
(0x76) and `L` (0x4c) are now IMPLEMENTED** per Phase 10B-v (see
[`PHASE_10B_V_CLOSE.md`](PHASE_10B_V_CLOSE.md)) and Phase 10B-L (see
[`PHASE_10B_L_CLOSE.md`](PHASE_10B_L_CLOSE.md)). **No implementation
exists for the remaining Section B DRAFT rows** (`d`, `T`). This
document is a planning input for a future Phase 10B-class implementation
phase; it is neither a specification nor a behavioral claim for
unimplemented rows.

**Authoritative cross-references:**

- Phase 10A entry: [`DEVKIT_PROGRESS_PHASE_10.md`](DEVKIT_PROGRESS_PHASE_10.md)
- Phase 9E-Z direction guard: [`PHASE_9EZ_CHECKPOINT.md`](PHASE_9EZ_CHECKPOINT.md)
- UART TX scope guard: [`PHASE_9EZ_CHECKPOINT.md §H`](PHASE_9EZ_CHECKPOINT.md)
- Phase history (Phase 9E and earlier):
  [`DEVKIT_PROGRESS.md`](DEVKIT_PROGRESS.md)
- Live state snapshot: [`../../../CURRENT_STATE.md`](../../../CURRENT_STATE.md)

---

## 1. UART TX Scope Guard (restated, unchanged)

Any command in this document must respect every constraint below. A command
that crosses any row is **not** a Phase 10B-class implementation; it is an
architectural decision that must be raised separately.

| # | Constraint | Source |
|---|---|---|
| 1 | No shell / prompt / interactive session semantics | `PHASE_9EZ_CHECKPOINT.md §H` |
| 2 | No parser framework, no token lexer, no grammar | same |
| 3 | No command registry; no dynamic handler registration | same |
| 4 | No multiline / framed protocol; no delimiter framing; no length prefix | same |
| 5 | No heap buffers; all response buffers stack-allocated; fixed size (current 96 B) | same |
| 6 | No response queue; TX is synchronous in the handler | same |
| 7 | No UART logging replacement; RTT remains the canonical log backend | same |
| 8 | No core / platform UART abstraction (`robotos_platform_uart`) | same |
| 9 | No retry / ACK protocol; no sequence number; no error recovery | same |
| 10 | No TX from ISR context; `uart_poll_out()` only from handler | same |
| 11 | No promotion of `devkit_app_state` from devkit-local to Robot Framework | same |
| 12 | Single-byte command vocabulary preserved | implied by 1–4 |

---

## 2. Section A — Hardware-proven Phase 9E commands

These are **already implemented and validated** on hardware. They are
listed here for reference only; Phase 10A does not modify them.

| Byte | Human meaning | Precondition | Side effect | Response template | Phase 9E evidence |
|---|---|---|---|---|---|
| `a` (0x61) | Arm | `IDLE` | `IDLE → ARMED`; `transitions += 1`; `uart += 1` | `OK state=ARMED\r\n` | RTT + host transcript at `587dab7` |
| `s` (0x73) | Start | `ARMED` | `ARMED → ACTIVE`; `transitions += 1`; `uart += 1` | `OK state=ACTIVE\r\n` | same |
| `?` (0x3f) | Query | any | `uart += 1` only | `STATE state=<S> transitions=<N> button=<N> uart=<N> ignored=<N>\r\n` | same |
| `r` (0x72) | Reset to IDLE | any (`ACTIVE → IDLE` proven; `ARMED → IDLE` not explicitly shown in Phase 9E transcript) | `* → IDLE`; `transitions += 1` if state changed; `uart += 1` | `OK state=IDLE\r\n` | same |
| `x` (0x78) | Negative-path probe (unrecognized byte) | any | `uart += 1`; `ignored += 1` | `ERR ignored byte=0x78 state=<S>\r\n` | same |
| `v` (0x76) | Build/version/info query | any | `uart += 1` only (no transition, no ignored) | `INFO phase=10b-v app=devkit board=<CONFIG_BOARD> tick_ms=<DEVKIT_TICK_MS> uart=minimal\r\n` (77 B on `stm32f411e_disco` at `DEVKIT_TICK_MS=500`) | **Phase 10B-v** — hardware-validated 2026-05-11; see [`PHASE_10B_V_CLOSE.md`](PHASE_10B_V_CLOSE.md) and `phase_10B_v_{host,rtt}_2026-05-11.txt`. Promoted from Section B at Phase 10B-v close. |
| `L` (0x4c) | LED physical-effect smoke (single GPIO toggle) | any | `uart += 1` only (no transition, no ignored). Side effect: one `devkit_status_led_toggle()` call from thread-context UART handler. | `OK led=toggle state=<S>\r\n` (26 B) | **Phase 10B-L** — hardware-validated 2026-05-11 (electrical/RTT evidence); visual LED PHYSICAL_OBSERVATION_AMBIGUOUS pending operator-witnessed re-run; see [`PHASE_10B_L_CLOSE.md`](PHASE_10B_L_CLOSE.md) and `phase_10B_L_{host,rtt}_2026-05-11.txt`. Promoted from Section B at Phase 10B-L close. |

Notes:

- All five commands fit the single-byte / fixed-buffer / no-parser /
  no-registry pattern.
- The `?` query response shape currently exhausts the proven query surface.
  Any extension (e.g. adding fields) is a Phase 10B-class concern, not
  Phase 10A scope.
- The `r` behavior from the `ARMED` precondition is **not surfaced in
  published Phase 9E evidence** (transcript only shows `ACTIVE → IDLE`).
  This is a documentation gap, not a Phase 10A redefinition; if needed it
  must be observed and recorded by a future evidence-only phase, not
  reinterpreted here.

---

## 3. Section B — DRAFT candidate product commands (`USER_DECISION_REQUIRED`)

Each row is a **planning artifact only**. No row is approved. No row is
implemented. Opening a Phase 10B-class implementation for any row requires
explicit user approval of that specific row, including answering its
`USER_DECISION_REQUIRED` notes.

| Byte | Human meaning | Precondition | Proposed side effect | Proposed response | Phase 10B? | New driver/config? | Scope-guard violation? | `USER_DECISION_REQUIRED` notes |
|---|---|---|---|---|---|---|---|---|
| `d` (0x64) | DRAFT — Explicit disarm | `ARMED` | `ARMED → IDLE`; `transitions += 1` | `OK state=IDLE\r\n` | Yes | No | No | Whether to introduce `d` given `r` already handles `* → IDLE`; whether `r` from `ARMED` is intended to remain the canonical disarm path. |
| ~~`v` (0x76)~~ | **PROMOTED to Section A** — implemented at Phase 10B-v, 2026-05-11 | — | — | `INFO phase=10b-v app=devkit board=<CONFIG_BOARD> tick_ms=<DEVKIT_TICK_MS> uart=minimal\r\n` | **DONE** | No | No | See [`PHASE_10B_V_CLOSE.md`](PHASE_10B_V_CLOSE.md). Phase tag `10b-v` is the closeout identifier; the response format may be reviewed in a future planning phase but is **frozen** as published baseline. |
| ~~`L` (0x4c)~~ | **PROMOTED to Section A** — implemented at Phase 10B-L, 2026-05-11 | — | — | `OK led=toggle state=<S>\r\n` | **DONE** | No (existing `devkit_status_led_toggle()` API reused; no new LED function) | No | See [`PHASE_10B_L_CLOSE.md`](PHASE_10B_L_CLOSE.md). Physical effect: single GPIO toggle interleaved with the existing 500 ms heartbeat. LED state is **not** exposed in `?` (existing toggle is stateless). Visual LED observation is `PHYSICAL_OBSERVATION_AMBIGUOUS` pending operator-witnessed re-run. |
| `T` (0x54) | DRAFT — Onboard sensor read placeholder (e.g. core temperature) | any | Sensor read; no actuator change | `TEMP <value>\r\n` (or `ERR sensor unavailable\r\n`); units / precision / sensor identity `USER_DECISION_REQUIRED` | Yes | Yes — sensor driver dependency; `prj.conf` flag(s); Zephyr sensor API path | Conditional — fixed-buffer compliance must be verified for the chosen format | Which sensor (STM32 internal temp? external sensor on the dev board? new I²C/SPI part?); whether a driver exists; response format. |

DRAFT rows that were considered and intentionally **omitted** from
Section B because they do not align cleanly with the existing model:

- `n` / `S` (heartbeat) — provides no information beyond the existing
  Phase 6M producer telemetry visible on RTT; no product value at this
  layer.
- `H` (halt) — implies a stop semantic that the existing IDLE/ARMED/ACTIVE
  model does not represent. Out of scope for Phase 10A.

---

## 4. Section C — Explicitly rejected / deferred command shapes

The following shapes are **rejected at Phase 10A**. Each crosses one or more
UART TX scope-guard constraints from §1. None should be implemented under a
Phase 10B-class umbrella; each requires its own architectural decision.

| Shape | Constraint(s) crossed | Why rejected |
|---|---|---|
| Multi-byte tokens (e.g. `ARM\r\n`, `STATUS\r\n`, `LED ON\r\n`) | #1, #2, #4, #12 | Requires shell/parser/framing. |
| Commands with arguments (e.g. `s 0.5`, `T sensor=core`) | #2, #4 | Requires parser + bounded argument lexer. |
| ACK/NACK + sequence numbers (e.g. `OK seq=42\r\n`) | #9 | Introduces a sequencing protocol; cascades into retry semantics. |
| Streaming/periodic telemetry over UART (e.g. emit `STATE …` every N ticks) | #7 | Replaces RTT as the canonical log backend; introduces TX cadence outside command/response. |
| Dynamic per-session command registration | #3 | Requires command registry. |
| Response longer than the current 96-byte fixed buffer | #5, #6 | Requires heap or response queue. |
| Commands that read or mutate `core/` or `platform/` internals | architectural | Breaks the 9A–9E invariant that `core/` / `platform/` are untouched. |
| Commands that originate TX from ISR context | #10 | TX must be thread-context only. |
| Authentication / authorization commands | architectural | Security design is not in any current phase scope. |
| Fault-injection or fault-recovery commands | architectural | Fault-recovery is not in any current phase scope. |

---

## 5. Implementation candidates that may become Phase 10B (when approved)

Each candidate below is a **non-binding ordering suggestion** for future
Phase 10B-class implementation. Order is not fixed by Phase 10A. Each
candidate, when opened, must follow the Phase 9E-style validation
discipline: pristine build, flash via `capture_devkit_rtt.ps1` (manual RESET
retained as fallback; plain `west flash` alone is not runtime-start
evidence), RTT counter table, host transcript table, architecture-boundary
preservation audit, scope-guard restated, and a dedicated closeout document.

1. ~~**Phase 10B-`v` (build query)**~~ — **IMPLEMENTED 2026-05-11**;
   see [`PHASE_10B_V_CLOSE.md`](PHASE_10B_V_CLOSE.md). Hardware-validated
   on STM32F411E-DISCO; 77-byte fixed response; no parser/registry; no
   driver dependency; no state-change side effect; `ignored` does not
   increment.
2. ~~**Phase 10B-`L` (LED toggle)**~~ — **IMPLEMENTED 2026-05-11**;
   see [`PHASE_10B_L_CLOSE.md`](PHASE_10B_L_CLOSE.md). Hardware-validated
   electrical/RTT evidence on STM32F411E-DISCO; 26-byte fixed response;
   existing `devkit_status_led_toggle()` reused (no new LED API); heartbeat
   semantics preserved; visual LED `PHYSICAL_OBSERVATION_AMBIGUOUS`
   pending operator-witnessed re-run.
3. **Phase 10B-`d` (explicit disarm)** — smallest remaining behavioral
   surface; single-byte app-state command; useful only if user vocabulary
   requires explicit disarm over `r`'s `* → IDLE` semantics.
4. **Phase 10B-`T` (sensor read)** — largest remaining surface; introduces
   a driver dependency and a numeric response format; requires choosing a
   sensor part.

Direction-independent supporting phases that the user may prefer to land
**before** any Phase 10B opening:

- **Phase 9F — Demo script PASS/FAIL polish.** Tooling only; no firmware
  change. Adds substring-assertion checking to
  `run_phase9e_uart_response_demo.ps1`.
- **Phase 9G — Bounded UART burst characterization.** Evidence-only;
  no firmware change. Closes the highest-priority outstanding unknown
  (high-rate UART input) and informs any future scheduler discussion.

Neither 9F nor 9G consumes the Phase 10 opening.

---

## 6. Operational reminders carried forward from Phase 9E-Z

- `core/` and `platform/` remain untouched through Phase 9A–9E. No Phase
  10B-class implementation may regress this without a written justification
  anchored in evidence.
- Scheduler 7A / 7B-1 remains DEFER. Phase 9E `peak=2 dropped=0` and
  Phase 9D operator-induced `peak=16 dropped=3 herr=0` do not justify
  budget mutation.
- Phase 8A / F407 remains HOLD/DEFER.
- POST_FLASH_AUTOSTART root cause remains OPEN; mitigated-by-workflow from
  Phase 6O onward via `capture_devkit_rtt.ps1` sidecar `reset run`; manual
  RESET retained as fallback; plain `west flash` alone is not runtime-start
  evidence.
- RTT remains the canonical log backend; UART is not a log channel.
- `uart_poll_out()` runs only from handler (thread context).

---

## 7. What this document does not do

- Does not specify behavior. Section B rows are draft, not specification.
- Does not commit to any command. Section B rows are user-approval-gated.
- Does not authorize any source/test/CMake/Zephyr/board/script change.
- Does not start Phase 10B.
- Does not change Phase 9E / 9E-Z baseline.
- Does not modify `DEVKIT_PROGRESS.md`, scheduler state, F407 state, or UART
  TX scope.
