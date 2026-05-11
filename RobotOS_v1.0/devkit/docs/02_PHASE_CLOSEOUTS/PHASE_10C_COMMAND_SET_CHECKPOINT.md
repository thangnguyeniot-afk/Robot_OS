# Phase 10C -- Command-Set Checkpoint

**Status:** `CLOSED_DOCS_ONLY`
**Type:** Docs-only checkpoint / design-state consolidation. **No
source, runtime, test, CMake, Zephyr, board, host-tool, script, or
prj.conf change.** Phase 10C does **not** open `T`, does **not**
authorize ACTIVE disarm widening, and does **not** modify any closed
phase's evidence.
**Date opened/closed:** 2026-05-11 (same-day docs-only close)
**Branch:** master
**Published baseline at open:** `origin/master = 7e250dc`
**Prior runtime behavior phase:** Phase 10B-d (firmware `125779c`,
evidence-close `7e250dc`).
**Prior docs-only checkpoint:** Phase 10A (planning) / Phase 9E-Z
(direction guard).
**Companion docs:**
[`COMMAND_SET_DRAFT.md`](../03_SPECS/COMMAND_SET_DRAFT.md),
[`PHASE_10A_CLOSE.md`](PHASE_10A_CLOSE.md),
[`PHASE_9EZ_CHECKPOINT.md`](PHASE_9EZ_CHECKPOINT.md),
[`PHASE_10B_V_CLOSE.md`](PHASE_10B_V_CLOSE.md),
[`PHASE_10B_L_CLOSE.md`](PHASE_10B_L_CLOSE.md),
[`PHASE_10B_D_CLOSE.md`](PHASE_10B_D_CLOSE.md).

---

## 1. Purpose

Phase 10C is a docs-only checkpoint that **snapshots the validated
non-sensor command group** after Phase 10B-d closes, and **prevents
blind opening of `T`** or other large-surface candidates.

Phase 10C exists because:

- The non-sensor command group `a / s / r / ? / x / v / L / d` is now
  hardware-validated end-to-end with the same Phase 9E/10B discipline
  (pristine build, sidecar `reset run` flash, RTT counter table, host
  transcript table, scope-guard restatement, dedicated closeout).
- The remaining Section B row (`T`) is the **largest** untouched
  surface in `COMMAND_SET_DRAFT.md`: it requires sensor part choice,
  driver / `prj.conf` change, response format, and error variant --
  none decided.
- The remaining decision **`USER_DECISION_REQUIRED_ACTIVE_DISARM`**
  (whether `d` should transition ACTIVE -> IDLE) is also untouched
  and silently inheriting "no-op" behavior pending explicit approval.
- Opening either without a checkpoint would re-derive the same
  scope guards every time. Phase 10C records them once.

Phase 10C is **not** an implementation phase, **not** a behavioral
specification, **not** an authorization for `T`, **not** a redefinition
of any Phase 9E/10B-{v,L,d} closed contract, and **not** a re-opening
of Scheduler 7A/7B or F407.

---

## 2. Baseline at checkpoint

| Item | Value |
|---|---|
| `origin/master` at open | `7e250dc` (`docs: close Phase 10B-d disarm command with hardware evidence`) |
| Last runtime behavior phase | Phase 10B-d (firmware `125779c`) |
| Last runtime behavior status | `CLOSED_WITH_HARDWARE_EVIDENCE` |
| Last docs-only checkpoint | Phase 10A (`CLOSED_DOCS_ONLY`) |
| Last earlier runtime checkpoint | Phase 9E-Z (`30bae27`) |
| Hardware platform | STM32F411E-DISCO (production runtime baseline) |
| Hardware platform alternatives | F407 / custom board: `HOLD/DEFER` (no change) |
| Scheduler | `ROBOTOS_CORE_MAX_EVENTS_PER_TICK=1`; `ROBOTOS_EVENT_QUEUE_CAPACITY=16`; 7A/7B `DEFER` |
| UART TX scope | Minimal response only (12 scope-guard constraints intact) |
| POST_FLASH_AUTOSTART | Root cause `OPEN`; `MITIGATED_BY_WORKFLOW` from Phase 6O onward |
| Validated non-sensor command set | `a / s / r / ? / x / v / L / d` |
| Remaining `USER_DECISION_REQUIRED` command | `T` (sensor read) |
| Remaining `USER_DECISION_REQUIRED` semantic | `USER_DECISION_REQUIRED_ACTIVE_DISARM` (`d` from ACTIVE) |

---

## 3. Command inventory

This table is the authoritative single-page snapshot of the closed
command set at Phase 10C. It is sourced from
[`COMMAND_SET_DRAFT.md`](../03_SPECS/COMMAND_SET_DRAFT.md) Section A, the four
Phase 9E and Phase 10B-{v,L,d} closeouts, and the hardware RTT/host
evidence listed in [`../logs/INDEX.md`](../../logs/INDEX.md). Phase 10C
does **not** modify any of these contracts.

| Cmd | Meaning | Phase | Evidence | State effect | Physical effect | Response shape | Notes / limitations |
|---|---|---|---|---|---|---|---|
| `a` (0x61) | Arm | Phase 9E (`587dab7`) | `CLOSED_WITH_HARDWARE_EVIDENCE` | IDLE -> ARMED (`transitions++`); `uart++` | None | `OK state=ARMED\r\n` | Idempotency: from ARMED, `ignored++` and `OK state=ARMED unchanged=1\r\n`. |
| `s` (0x73) | Start | Phase 9E (`587dab7`) | `CLOSED_WITH_HARDWARE_EVIDENCE` | ARMED -> ACTIVE (`transitions++`); `uart++` | None | `OK state=ACTIVE\r\n` | Idempotency: from ACTIVE, `ignored++` and `OK state=ACTIVE unchanged=1\r\n`. |
| `r` (0x72) | Reset to IDLE | Phase 9E (`587dab7`) | `CLOSED_WITH_HARDWARE_EVIDENCE` | `* -> IDLE` (`transitions++` if state changed); `uart++` | None | `OK state=IDLE\r\n` (changed) / `OK state=IDLE unchanged=1\r\n` (already IDLE) | From IDLE: `ignored++` (legacy semantic). ARMED -> IDLE not surfaced in published Phase 9E transcript; documentation gap, not a behavioral claim. |
| `?` (0x3f) | Query | Phase 9E (`587dab7`) | `CLOSED_WITH_HARDWARE_EVIDENCE` | None; `uart++` | None | `STATE state=<S> transitions=<N> button=<N> uart=<N> ignored=<N>\r\n` | No LED field. Response shape is the published baseline; extension is a Phase 10B-class concern. |
| `x` (0x78) | Negative-path probe (unrecognized byte) | Phase 9E (`587dab7`) | `CLOSED_WITH_HARDWARE_EVIDENCE` | None; `uart++`; `ignored++` | None | `ERR ignored byte=0x78 state=<S>\r\n` | Generic ignored response is reused by every byte that does not match a recognized command. |
| `v` (0x76) | Build/version/info query | Phase 10B-v (`d8346db`) | `CLOSED_WITH_HARDWARE_EVIDENCE` | None; `uart++` (no `ignored++`, no transition) | None | `INFO phase=10b-v app=devkit board=<CONFIG_BOARD> tick_ms=<DEVKIT_TICK_MS> uart=minimal\r\n` (77 B on `stm32f411e_disco` at `DEVKIT_TICK_MS=500`) | State-invariant; identical response in IDLE / ARMED / ACTIVE. Deterministic (no `__DATE__`/`__TIME__`). |
| `L` (0x4c) | LED physical-effect smoke (single GPIO toggle) | Phase 10B-L (`f1db2fa` impl + `a96ce17` visual confirmation) | `CLOSED_WITH_HARDWARE_EVIDENCE` + `OPERATOR_VISUAL_CONFIRMED` | None; `uart++` (no `ignored++`, no transition) | One `devkit_status_led_toggle()` call from thread-context UART handler. Heartbeat 500 ms blink in `devkit_runtime_run()` is unchanged; an `L` interleaves one extra toggle (phase shift by one half-cycle). | `OK led=toggle state=<S>\r\n` (26 B) on success; `ERR led=toggle ret=<N> state=<S>\r\n` on failure (not observed in published evidence). | LED state is **not** exposed in `?`. Existing `devkit_status_led_toggle()` is stateless; no new LED service, queue, or subsystem was introduced. No LED ownership conflict observed between heartbeat and `L`. |
| `d` (0x64) | Explicit disarm | Phase 10B-d (`125779c` impl + `7e250dc` evidence close) | `CLOSED_WITH_HARDWARE_EVIDENCE` | ARMED -> IDLE (`transitions++`); IDLE / ACTIVE: recognized no-op (no `ignored++`, no transition); `uart++` always | None | `OK disarm state=IDLE\r\n` (22 B) on ARMED -> IDLE; `OK disarm no-op state=<S>\r\n` (28 B IDLE / 30 B ACTIVE) on the no-op paths | `d` does **not** replace or alter `r`. `r` keeps its existing `* -> IDLE` semantics including `ignored++` on the redundant IDLE-reset edge. `d` from IDLE is recognized (not ignored) -- this is the key behavioral difference. **ACTIVE -> IDLE disarm: `USER_DECISION_REQUIRED_ACTIVE_DISARM`**; current ACTIVE behavior is recognized no-op (no safety semantics invented). |
| `T` (0x54) | Sensor read (placeholder) | **NOT IMPLEMENTED** | `USER_DECISION_REQUIRED` (Section B of `COMMAND_SET_DRAFT.md`) | n/a | n/a | n/a | Requires sensor part choice (STM32 internal die temp vs. external I2C/SPI), driver / `prj.conf` change, response format (units / precision / sensor identity), error variant. None decided. Do not open blind. |

Notes:

- All eight implemented commands (`a / s / r / ? / x / v / L / d`)
  fit the **single-byte / fixed 96-byte stack buffer / no-parser /
  no-registry / no-framing / thread-context-TX** pattern.
- The `?` response shape is unchanged across Phase 9E, 10B-v, 10B-L,
  and 10B-d. No field was added or removed.
- All commands are case-insensitive (`A`/`a`, `S`/`s`, `R`/`r`,
  `V`/`v`, `L`/`l`, `D`/`d`) via the existing upcase normalization at
  the top of `devkit_app_state_on_uart_byte()`.

---

## 4. Important semantic distinctions

Restated here so a future reader does not need to re-derive them from
the four closeouts:

### 4.1 `r` is reset; `d` is disarm

| | `r` (reset) | `d` (disarm) |
|---|---|---|
| From IDLE | `ignored++`; response `OK state=IDLE unchanged=1\r\n` | recognized no-op (NOT ignored); response `OK disarm no-op state=IDLE\r\n` |
| From ARMED | -> IDLE; `transitions++`; response `OK state=IDLE\r\n` | -> IDLE; `transitions++`; response `OK disarm state=IDLE\r\n` |
| From ACTIVE | -> IDLE; `transitions++`; response `OK state=IDLE\r\n` | recognized no-op (`USER_DECISION_REQUIRED_ACTIVE_DISARM`); response `OK disarm no-op state=ACTIVE\r\n` |

`d` is **not** a synonym for `r`. The IDLE distinction (ignored vs.
recognized no-op) and the ACTIVE distinction (transition vs.
USER_DECISION_REQUIRED no-op) are the two behavioral differences. `r`
is preserved zero-diff by Phase 10B-d and remains the canonical reset
path for callers who want `* -> IDLE` from any state.

### 4.2 `L` is a physical-effect smoke, not a LED subsystem

- `L` calls the existing `devkit_status_led_toggle()` one-shot API.
- No new LED API, no new LED state machine, no LED service, no LED
  command queue, no `prj.conf` change.
- The 500 ms heartbeat blink in `devkit_runtime_run()` is unchanged
  and continues per-tick; an `L` between heartbeat ticks shifts the
  heartbeat phase by one half-cycle.
- LED state is **not** exposed in the `?` response.
- `OPERATOR_VISUAL_CONFIRMED` on 2026-05-11: operator observed visible
  phase-shift in the heartbeat blink correlated with both `L` commands.
- **No LED ownership conflict** was observed between heartbeat and
  `L`. No LED-semantics design phase is warranted at this checkpoint.

### 4.3 `v` is state-invariant

- `v` does not transition, does not increment `ignored`, and produces
  byte-identical responses in IDLE / ARMED / ACTIVE.
- `v` response is **frozen** as published baseline. Any extension
  (additional fields, version-bump format) is a future docs-only
  planning concern, not a Phase 10C scope.

### 4.4 `?` shape is unchanged

- The `?` response is the published Phase 9E format:
  `STATE state=<S> transitions=<N> button=<N> uart=<N> ignored=<N>\r\n`.
- It has no LED field, no version field, no timing field.
- Phase 10B-v, 10B-L, 10B-d all preserved this shape zero-diff.
- Extending `?` is a Phase 10B-class concern that would require
  evaluating fixed-buffer compliance against the existing 96-byte
  stack limit.

### 4.5 `x` is the canonical negative-path probe

- `x` is one specific byte that exercises the `default:` branch.
- The `default:` branch handles **every** unrecognized byte uniformly
  (`ignored++`; response `ERR ignored byte=0xNN state=<S>\r\n`).
- `x` is preserved zero-diff by Phase 10B-{v,L,d}.

---

## 5. Remaining decision gates

These are the only open decisions at the Phase 10C checkpoint. Each is
listed with the prerequisite that must be answered **before** a
follow-on phase can open. Phase 10C does **not** answer any of them.

### 5.1 ACTIVE disarm widening

| Aspect | Value |
|---|---|
| Current behavior | `d` from ACTIVE -> recognized no-op (`OK disarm no-op state=ACTIVE\r\n`); no transition; no `ignored++` |
| Proposed future behavior | `d` from ACTIVE -> IDLE with `transitions++`; response `OK disarm state=IDLE\r\n` (identical to the ARMED -> IDLE response) |
| Status | `USER_DECISION_REQUIRED_ACTIVE_DISARM` (unchanged) |
| Implementation surface if approved | One-line widening of the `if (s_state == DEVKIT_APP_STATE_ARMED)` guard in `devkit_app_state.c` to `if (s_state != DEVKIT_APP_STATE_IDLE)` |
| Validation if approved | Supplemental hardware run with sequence `d a s d ?`; expected final `transitions=2 button=0 uart=4 ignored=0` (one ARMED hop from `a`, one ARMED -> ACTIVE hop from `s` is **not** expected because... wait, sequence would be d(noop) a(IDLE->ARMED) s(ARMED->ACTIVE) d(ACTIVE->IDLE) ?(query). So `transitions=3 button=0 uart=5 ignored=0`.) |
| Required user inputs | Whether ACTIVE disarm is a desired product semantic, or whether `r` should remain the canonical path for `ACTIVE -> IDLE` |
| Risk | None observed; one-line change to a closed module; no scope-guard violation; no scheduler/queue impact. The risk is **silent-semantics**: changing behavior without an explicit product decision. |

### 5.2 `T` sensor read

| Aspect | Value |
|---|---|
| Current state | **NOT IMPLEMENTED**; remains `USER_DECISION_REQUIRED` in `COMMAND_SET_DRAFT.md` Section B |
| Open prerequisites | (1) Which sensor part: STM32 internal die temperature (no external HW), external sensor on the disco board, or new I2C/SPI part? (2) Does a Zephyr driver exist for the chosen part? (3) Response format: units (degC vs. mK vs. raw ADC counts), precision, sensor identity. (4) Error variant: `ERR sensor unavailable\r\n` vs. `ERR sensor ret=<N> state=<S>\r\n` vs. other. (5) Fixed-buffer compliance: max bytes against the existing 96-byte stack buffer. |
| Implementation surface if approved | Sensor driver dependency (Kconfig flag(s) in `prj.conf`); Zephyr sensor API path; `case 't':` arm in both `devkit_app_state.c` and `devkit_uart_producer.c`; possibly a new include. **First Phase 10B-class command to introduce a `prj.conf` change** -- this is the surface-step-up. |
| Validation discipline if approved | Same as Phase 10B-v / 10B-L / 10B-d: pristine `west build`, sidecar `reset run` flash, RTT counter table, host transcript, scope-guard restatement, dedicated closeout. |
| Required user inputs | Answers to all five open prerequisites above. Each is a separate decision; bundling them risks invention of silent semantics. |
| Risk | Largest open surface in the post-10B-d command set. Opening blind would (a) commit to a sensor part without an evidence basis, (b) introduce a `prj.conf` change without a written gate, (c) require numeric-format invention. Phase 10C explicitly does not open it. |

### 5.3 Scheduler 7A / 7B-1

| Aspect | Value |
|---|---|
| Current state | `DEFER` (unchanged from Phase 9E-Z) |
| Workload-driven justification after 9G / 10B-v / 10B-L / 10B-d | None. Phase 9G `peak=5 dropped=0`, Phase 10B-v `peak=2 dropped=0`, Phase 10B-L `peak=2 dropped=0`, Phase 10B-d `peak=2 dropped=0`. The only observed `dropped>0` was Phase 9D operator-induced `peak=16 dropped=3`, which `PHASE_9EZ_CHECKPOINT.md` §K already rules insufficient to mutate the admission budget. |
| Risk if opened now | No new evidence has changed the prior `DEFER` ruling. Opening would be re-litigation, not progress. |

### 5.4 STM32F407 / custom board

| Aspect | Value |
|---|---|
| Current state | `HOLD/DEFER` (unchanged from Phase 8A status) |
| Risk if opened now | Same as 5.3: no new workload requires the platform shift. Opening would consume the scheduling slot without justification. |

---

## 6. Scope guard restatement

The 12 UART TX scope-guard constraints from
[`PHASE_9EZ_CHECKPOINT.md §H`](PHASE_9EZ_CHECKPOINT.md) are **intact**
at Phase 10C. Restated and re-verified against the published baseline
`origin/master = 7e250dc`:

| # | Constraint | Status at Phase 10C |
|---|---|---|
| 1 | No shell / prompt / interactive session semantics | OK -- single-byte commands; no prompt; no echo |
| 2 | No parser framework, no token lexer, no grammar | OK -- bare C `switch` on a single byte for all 8 implemented commands |
| 3 | No command registry; no dynamic handler registration | OK -- statically-compiled switch-case dispatch |
| 4 | No multiline / framed protocol; no delimiter framing; no length prefix | OK -- one byte in, fixed string out, `\r\n` only |
| 5 | No heap buffers; all response buffers stack-allocated; fixed size (96 B) | OK -- max observed response is the 77-byte `v` response; `L`, `d`, `?`, `x` are all 22-30 B |
| 6 | No response queue; TX is synchronous in the handler | OK -- `snprintf` + `uart_poll_out` inline in `devkit_uart_emit_tx_response()` |
| 7 | No UART logging replacement; RTT remains the canonical log backend | OK -- RTT log baseline unchanged; UART is command/response only |
| 8 | No core / platform UART abstraction (`robotos_platform_uart`) | OK -- `s_uart_dev` stays in `devkit_uart_producer.c`; `core/` and `platform/` zero-diff vs. 9E baseline |
| 9 | No retry / ACK protocol; no sequence number; no error recovery | OK -- one shot, one response |
| 10 | No TX from ISR context; `uart_poll_out()` only from handler | OK -- TX called only from `devkit_uart_handler()` (thread context) |
| 11 | No promotion of `devkit_app_state` from devkit-local to Robot Framework | OK -- module remains devkit-local |
| 12 | Single-byte command vocabulary preserved | OK -- all 8 implemented commands are one byte |

Architecture boundaries also unchanged at Phase 10C:

| Surface | Status vs. Phase 9E baseline |
|---|---|
| `RobotOS_v1.0/core/` | **zero diff** |
| `RobotOS_v1.0/platform/` | **zero diff** |
| `RobotOS_v1.0/devkit/src/devkit_runtime.{c,h}` | **zero diff** |
| `RobotOS_v1.0/devkit/src/devkit_status_led.{h,c}` | **zero diff** |
| `RobotOS_v1.0/devkit/src/devkit_button.{c,h}` | **zero diff** |
| Scheduler constants (`ROBOTOS_CORE_MAX_EVENTS_PER_TICK=1`) | **unchanged** |
| Event queue capacity (`ROBOTOS_EVENT_QUEUE_CAPACITY=16`) | **unchanged** |
| `prj.conf`, `CMakeLists.txt`, `boards/`, `zephyr/` | **zero diff** |
| `tests/` | **zero diff** |
| `DEVKIT_PROGRESS.md` (historical master) | **zero diff** |

---

## 7. Validation / evidence summary

Phase 10C does not run any new validation. The table below is a
single-page citation of the evidence already published in
[`../logs/INDEX.md`](../../logs/INDEX.md) and the linked closeout docs.
Phase 10C does **not** modify any of these logs.

| Phase | Status | Evidence locations | Key counters |
|---|---|---|---|
| Phase 9E -- UART TX minimal response | `CLOSED_WITH_HARDWARE_EVIDENCE` | `phase_9E_uart_tx_response_rtt_2026-05-09.txt`; commit `587dab7` | `rx=5 ok=5 handled=5`; `transitions=3 uart=5 ignored=1`; `peak=2 dropped=0`; CFSR/HFSR `0x00000000` |
| Phase 9G -- Bounded UART burst characterization | `CLOSED_WITH_HARDWARE_EVIDENCE` | `phase_9G_uart_burst_{rtt,host}_2026-05-11.txt`; harness commit `e9a1d62` | 5-byte burst at 30 ms spacing; `rx=5 ok=5 handled=5 full=0`; `transitions=3 uart=5 ignored=1`; **`peak=5`** (vs. Phase 9E `peak=2`) `dropped=0 herr=0`; CFSR/HFSR `0x00000000` |
| Phase 10B-v -- Build/version query | `CLOSED_WITH_HARDWARE_EVIDENCE` | `phase_10B_v_{rtt,host}_2026-05-11.txt`; firmware `d8346db` | `rx=5 ok=5 handled=5`; `transitions=2 button=0 uart=5 ignored=0`; both `v` responses byte-identical (77 B); `peak=2 dropped=0`; CFSR/HFSR `0x00000000` |
| Phase 10B-L -- LED physical-effect (autonomous) | `CLOSED_WITH_HARDWARE_EVIDENCE` | `phase_10B_L_{rtt,host}_2026-05-11.txt`; firmware `f1db2fa` | `rx=4 ok=4 handled=4`; `transitions=0 button=0 uart=4 ignored=0`; both `L` responses byte-identical (26 B); `peak=2 dropped=0`; CFSR/HFSR `0x00000000`; 139 heartbeat tick-count lines |
| Phase 10B-L visual re-run -- operator-witnessed LED check | `OPERATOR_VISUAL_CONFIRMED` | `phase_10B_L_visual_{rtt,host}_2026-05-11.txt`; same firmware `f1db2fa`; close commit `a96ce17` | Electrical evidence byte-identical to autonomous run (same firmware, deterministic command path); operator observed visible phase-shift in the 500 ms heartbeat blink correlated with both `L` commands |
| Phase 10B-d -- Explicit disarm | `CLOSED_WITH_HARDWARE_EVIDENCE` | `phase_10B_d_{rtt,host}_2026-05-11.txt`; firmware `125779c`; evidence close `7e250dc` | `rx=4 ok=4 handled=4 last=0x3f`; `transitions=2 button=0 uart=4 ignored=0`; `d` from IDLE no-op (NOT ignored); `d` from ARMED transitioned to IDLE; `peak=2 dropped=0 herr=0`; CFSR/HFSR `0x00000000`; PROD `attempted=ok=60`; `accepted(64) - dispatched(63) = pending(1)` |

Cross-phase invariants holding at Phase 10C:

- `accepted - dispatched = pending` invariant: **OK** across 9E, 9G,
  10B-v, 10B-L, 10B-L visual, 10B-d.
- `PROD ok + UART ok = accepted` invariant: **OK** at every evidence
  capture.
- `peak <= QUEUE_CAPACITY=16`: **OK** (max observed `peak=16` from
  Phase 9D, which was operator-induced and within bound).
- `dropped` only ever non-zero in Phase 9D (operator-induced
  saturation); all 9E / 9G / 10B-* runs hold `dropped=0`.
- CFSR/HFSR `0x00000000` at every fault sample in every published
  capture.

---

## 8. POST_FLASH_AUTOSTART discipline restatement

Carried forward unchanged from Phase 9E-Z, Phase 10B-{v,L,d}:

- Root cause: **OPEN / unfixed**.
- Mitigation: **MITIGATED_BY_WORKFLOW** from Phase 6O onward via
  `capture_devkit_rtt.ps1` sidecar `reset run`.
- Manual RESET remains the fallback runtime-start path.
- **Plain `west flash` alone is not runtime-start evidence.** Every
  Phase 10B-* run treats the sidecar `reset run` as the runtime-start
  guarantee.

Phase 10C does not authorize a `west flash`-only workflow, does not
authorize bypassing the sidecar, and does not close the underlying
POST_FLASH_AUTOSTART root-cause investigation. That investigation
remains a separate future phase if and when it opens.

---

## 9. Next gate recommendation

In priority order, the recommended next actions at the Phase 10C
checkpoint are:

1. **Hold.** The current vocabulary is internally consistent,
   hardware-validated, self-contained, and aligned with all scope
   guards. There is no externally-imposed deadline forcing the next
   opening.
2. **Decide ACTIVE disarm widening (cheap).** Answer
   `USER_DECISION_REQUIRED_ACTIVE_DISARM`. If approved, the
   implementation is a one-line guard widening plus a single
   supplemental validation run (`d a s d ?`); if denied, record the
   denial here and lock the current ACTIVE no-op semantic. Either way,
   the gate closes cheaply.
3. **Decide `T` prerequisites (expensive).** Answer the five open
   prerequisites in §5.2 (sensor part, driver presence, response
   format, error variant, fixed-buffer compliance). Each is a
   separate decision; bundling risks silent invention. **Do not open
   `T` before these are decided.**
4. **Do not reopen Scheduler 7A/7B.** No new workload evidence justifies
   it. Re-opening at Phase 10C without new evidence would be
   re-litigation.
5. **Do not reopen F407 / custom board.** Same reasoning. No new
   workload requires the platform shift.

---

## 10. What this document does not do

- Does not specify or change any command behavior.
- Does not authorize any source, test, CMake, Zephyr, board,
  host-tool, script, or `prj.conf` change.
- Does not start Phase 10B-`T` or any other implementation phase.
- Does not widen ACTIVE disarm; ACTIVE remains
  `USER_DECISION_REQUIRED_ACTIVE_DISARM`.
- Does not modify any Phase 9E / 9G / 10B-v / 10B-L / 10B-d
  closeout, evidence log, or commit.
- Does not modify `DEVKIT_PROGRESS.md` (historical master).
- Does not reopen Scheduler 7A/7B-1 or change admission constants.
- Does not reopen STM32F407 / custom-board migration.
- Does not change UART TX scope; all 12 scope-guard constraints
  remain intact.
- Does not change POST_FLASH_AUTOSTART status.
- Does not push.
