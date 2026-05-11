# DEVKIT_PROGRESS_PHASE_10.md — RobotOS Devkit Phase Log, Phase 10+

---

## 1. Continuation Note

This file continues the RobotOS devkit phase log after `DEVKIT_PROGRESS.md`.

- `DEVKIT_PROGRESS.md` remains the **historical master** for Phase 1 through
  Phase 9E / Phase 9E-Z. It must not be rewritten, reordered, or deduplicated
  as part of this seed split.
- All **Phase 10 and later** phase entries (including any Phase 10A, 10B, 10Z,
  Phase 11+, etc.) must be added to this file (`DEVKIT_PROGRESS_PHASE_10.md`).
- **Phase 10A (docs-only product command set planning) is opened** as of this
  file's most recent commit. No Phase 10B or later implementation work has
  started. No firmware, runtime, test, CMake, Zephyr, or board-config change
  has been authorized by Phase 10A.

---

## 2. Current Baseline Before Phase 10

The following facts describe the confirmed baseline that exists **before** any
Phase 10 work begins. They are reproduced here only for orientation; the
authoritative live snapshot lives in `CURRENT_STATE.md` and the authoritative
phase history lives in `DEVKIT_PROGRESS.md`.

- Baseline is **closed at Phase 9E / Phase 9E-Z**.
- Phase 9E proved the first minimal host-commanded embedded runtime loop:
  host UART command → UART RX producer → core event queue → app state machine →
  UART TX minimal response → host received response.
- Phase 9E-Z is the post-9E checkpoint / direction guard.
- **Scheduler 7A/7B** remains `DEFER`. No scheduler admission change is
  approved as part of seeding this file.
- **STM32F407 / custom board** migration remains `HOLD/DEFER`. No board
  migration is approved as part of seeding this file.
- **UART TX** scope remains **minimal response only**. No shell, parser,
  protocol expansion, or product command-set implementation is approved as
  part of seeding this file.
- The next real decision gate **before any Phase 10B+ implementation** is
  **product command set / workload intent direction**, not blind
  implementation. Phase 10A is a **docs-only planning phase** that anchors
  that direction decision; it does not authorize firmware change. The
  underlying runtime baseline still ends at Phase 9E / 9E-Z.

---

## 3. Phase Index

Phase entries below use **manual anchors** (`<a id="..."></a>`) placed
immediately before each phase heading. Index entries link to those manual
anchors so navigation does not depend solely on GitHub auto-generated heading
anchors.

| Phase | Title | Status | Jump |
|-------|-------|--------|------|
| 10A | Product Command Set Planning (docs-only) | CLOSED_DOCS_ONLY | [→](#phase-10a) |
| 9G ‡ | Bounded UART Burst Characterization (hardware evidence captured) | CLOSED_WITH_HARDWARE_EVIDENCE | [→](#phase-9g-late) |
| 10B-v | Build/Version Query Command `v` | CLOSED_WITH_HARDWARE_EVIDENCE | [→](#phase-10b-v) |
| 10B-L | LED Physical-Effect Command `L` | CLOSED_WITH_HARDWARE_EVIDENCE (PHYSICAL_OBSERVATION_AMBIGUOUS) | [→](#phase-10b-l) |
| 10Z | RESERVED — future checkpoint / closeout slot | NOT_STARTED | [→](#phase-10z) |

`‡` = **non-linear insert.** Phase 9G is a Phase-9-series late evidence
entry opened *after* the seed-split and *after* Phase 10A. It lives in this
file (not in `DEVKIT_PROGRESS.md`) per the user-stated preference to leave
the historical master untouched. Anchor id `phase-9g-late` makes the
non-linear placement explicit and avoids collision with any future
chronological `phase-9*` slot in the historical master.

When future phases are added:

- Insert a new manual-anchor block (`<a id="phase-XY"></a>`) immediately
  before the new phase heading.
- Add a row to the table above linking to that manual anchor.
- Keep the index in insertion / session order, matching the convention used
  in `DEVKIT_PROGRESS.md`. Phase-number order is **not** required.

---

## 4. Split / Editing Rules

These rules govern how this file and `DEVKIT_PROGRESS.md` evolve together.

1. **History stays put.** Phase 1 through Phase 9E / Phase 9E-Z history
   remains in `DEVKIT_PROGRESS.md`. Do not move, rewrite, reorder, or
   deduplicate old phase sections as part of using this file.
2. **New phases land here.** Every Phase 10+ entry must be added to
   `DEVKIT_PROGRESS_PHASE_10.md`, not appended to `DEVKIT_PROGRESS.md`.
3. **Manual anchors are mandatory.** Each future phase section must place an
   explicit manual anchor (`<a id="phase-XY"></a>`) on the line immediately
   before its heading.
4. **Index links must target manual anchors.** Entries in the Phase Index
   table must link to the manual anchor id, not to a heading slug.
5. **Do not rely on GitHub auto-generated heading anchors.** Heading slugs
   change when titles are edited; manual anchors do not.
6. **Make non-linear edits explicit.** If a phase is duplicated, amended, or
   inserted out of phase-number order, mark the index entry to make the
   duplication or insertion obvious (for example, `†` for the first
   occurrence and `‑1` / `‑2` anchor suffixes for subsequent ones, mirroring
   the convention used in `DEVKIT_PROGRESS.md`).
7. **No silent baseline change.** Do not alter the "Current Baseline Before
   Phase 10" section to claim work has progressed beyond Phase 9E / 9E-Z
   unless that progression has been validated and recorded as a closed
   Phase 10+ entry below.

---

## 5. Phase Sections

<a id="phase-10a"></a>
## Phase 10A — Product Command Set Planning

**Status:** `CLOSED_DOCS_ONLY`
**Type:** Docs-only planning. No source, runtime, test, CMake, Zephyr, board,
or script change.
**Date opened/closed:** 2026-05-11
**Branch:** master
**Prior runtime checkpoint:** Phase 9E-Z (`30bae27`); runtime baseline ends at
Phase 9E (`587dab7`).
**Companion docs:**
[`COMMAND_SET_DRAFT.md`](COMMAND_SET_DRAFT.md),
[`PHASE_10A_CLOSE.md`](PHASE_10A_CLOSE.md).

### 5.1 Purpose

Phase 10A captures **product command vocabulary and workload intent** in
writing **before** any Phase 10B real-effect implementation is authorized.

This phase is the operationalization of the Phase 9E-Z direction guard
(`PHASE_9EZ_CHECKPOINT.md` §L Q1–Q5). It does **not** add, change, or imply
any runtime behavior. It exists so that the next implementation phase, when
opened, is anchored to a written product target rather than to technically-
adjacent extension of the Phase 9E loop.

### 5.2 Baseline inherited from Phase 9E / 9E-Z

Phase 10A inherits, and does not modify, the following baseline facts:

- Phase 9E proved a closed-loop host↔board command/response on STM32F411E-DISCO
  with five hardware-validated commands (`a` / `s` / `?` / `r` / `x`).
- App state machine remains devkit-local (`devkit_app_state`) with states
  `IDLE`, `ARMED`, `ACTIVE`.
- Two real hardware event sources are proven: button EXTI producer
  (Phase 9A-A) and UART RX producer (Phase 9B). Both emit `USER+3` events.
- One real hardware output is proven: UART TX via `uart_poll_out()` from
  handler/thread context only.
- `core/` and `platform/` have been untouched through 9A–9E. Scheduler budget
  `ROBOTOS_CORE_MAX_EVENTS_PER_TICK = 1` and queue capacity
  `ROBOTOS_EVENT_QUEUE_CAPACITY = 16` are unchanged.
- POST_FLASH_AUTOSTART root cause remains OPEN; mitigated-by-workflow from
  Phase 6O onward via `capture_devkit_rtt.ps1` sidecar `reset run`; manual
  RESET retained as fallback; plain `west flash` alone is not runtime-start
  evidence.
- Scheduler 7A / 7B-1 remains DEFER. F407 / custom-board (Phase 8A) remains
  HOLD/DEFER. UART TX scope remains minimal response only.

### 5.3 Explicit non-goals of Phase 10A

Phase 10A does **not** introduce, authorize, or imply any of the following:

- A parser, tokenizer, lexer, or grammar.
- An interactive shell, prompt, or command-history facility.
- A command registry or dynamic handler registration.
- A framing protocol, length prefix, delimiter scheme, or multi-byte command
  vocabulary.
- A response queue or any buffering of pending responses.
- Heap allocation for command or response handling.
- TX from ISR context. `uart_poll_out()` continues to run only from thread
  context.
- An ACK/NACK / sequence-number / retry / error-recovery protocol.
- A core or platform UART abstraction (`robotos_platform_uart`).
- Replacement of RTT as the canonical log backend with UART logging.
- Promotion of `devkit_app_state` from devkit-local to `core/` or a Robot
  Framework layer.
- Activation of Scheduler 7A or 7B-1.
- Phase 8A / F407 / custom-board work.
- Security or authentication design.
- Fault-recovery design (`active=0` invariant inherited from Phase 9E).
- Any change to `DEVKIT_PROGRESS.md` (the historical master).

### 5.4 Command-set planning table

The table below records:

- **Section A — Hardware-proven Phase 9E commands.** Already implemented and
  validated. Listed for completeness; not subject to Phase 10A revision.
- **Section B — DRAFT candidate product commands.** Marked
  `DRAFT / USER_DECISION_REQUIRED`. None of these are implemented. Each row
  is a planning artifact only; opening a Phase 10B for any of them requires
  explicit user approval of that specific row.
- **Section C — Explicitly rejected / deferred shapes.** Documented to
  prevent scope creep in future implementation phases.

**Legend.**
- *Scope guard violation?* — does the row require crossing one of the twelve
  constraints in `PHASE_9EZ_CHECKPOINT.md §H`?
- All status columns reflect the proposed (not implemented) behavior.
- `USER_DECISION_REQUIRED` means user must explicitly approve before any
  Phase 10B opening that touches this row.

#### Section A — Hardware-proven Phase 9E commands (reference only)

| Byte | Human meaning | App-state precondition | App-state side effect | Physical side effect | UART response template | Error / ignore behavior | Requires 10B? | New driver/config? | Scope-guard violation? | Notes |
|---|---|---|---|---|---|---|---|---|---|---|
| `a` (0x61) | Arm | `IDLE` | `IDLE → ARMED`; `transitions += 1`; `uart += 1` | None | `OK state=ARMED\r\n` | From non-`IDLE`: `ERR ignored byte=0x61 state=<S>\r\n`; `ignored += 1` | No (proven Phase 9E) | No | No | Hardware-validated at `587dab7` |
| `s` (0x73) | Start (ARMED → ACTIVE) | `ARMED` | `ARMED → ACTIVE`; `transitions += 1`; `uart += 1` | None | `OK state=ACTIVE\r\n` | From non-`ARMED`: ignored, same shape as above | No (proven Phase 9E) | No | No | Hardware-validated |
| `?` (0x3f) | Query | any | No state change; `uart += 1` | None | `STATE state=<S> transitions=<N> button=<N> uart=<N> ignored=<N>\r\n` | n/a (query never errors) | No (proven Phase 9E) | No | No | Hardware-validated; current `?` already exhausts the proven query surface |
| `r` (0x72) | Reset to IDLE | any (Phase 9E B.2 evidence: `ACTIVE → IDLE`; behavior from `ARMED` not surfaced in Phase 9E transcript) | `* → IDLE`; `transitions += 1` (if state changed); `uart += 1` | None | `OK state=IDLE\r\n` | n/a | No (proven Phase 9E) | No | No | Hardware-validated from `ACTIVE`. **DECISION_NOTE:** explicit behavior from `ARMED` is not visible in published Phase 9E evidence and is not in Phase 10A scope to redefine. |
| `x` (0x78) | Negative-path probe (unrecognized byte) | any | No state change; `uart += 1`; `ignored += 1` | None | `ERR ignored byte=0x78 state=<S>\r\n` | This row *is* the error path | No (proven Phase 9E) | No | No | Hardware-validated; same shape applies to any unrecognized byte |

#### Section B — DRAFT candidate product commands (`USER_DECISION_REQUIRED`)

| Byte | Human meaning | App-state precondition | App-state side effect | Physical side effect | UART response template (proposed) | Error / ignore behavior | Requires 10B? | New driver/config? | Scope-guard violation? | Notes |
|---|---|---|---|---|---|---|---|---|---|---|
| `d` (0x64) | DRAFT — Explicit disarm (`ARMED → IDLE`) | `ARMED` (proposed) | `ARMED → IDLE`; `transitions += 1`; `uart += 1` | None | `OK state=IDLE\r\n` | From non-`ARMED`: `ERR ignored byte=0x64 state=<S>\r\n` | Yes — Phase 10B | No | No | `USER_DECISION_REQUIRED` whether to introduce `d` given `r` already handles `* → IDLE`. Pro: explicit user vocabulary. Con: redundant with `r`. |
| `v` (0x76) | DRAFT — Firmware/build identification query | any | No state change; `uart += 1` | None | `BUILD <build-info-fields>\r\n` (single line, ≤ 96 bytes; format `USER_DECISION_REQUIRED`) | n/a (query) | Yes — Phase 10B | No | No | Reuses `devkit_build_info` data already emitted to RTT. Single-byte command; no parser needed. `USER_DECISION_REQUIRED` on exact response field set. |
| `L` (0x4c) | DRAFT — Onboard LED toggle (single physical effect) | any (proposed) | No app-state change; `uart += 1` | LED state flips (current `devkit_status_led` blink behavior would need a per-command override decision) | `OK led=<on|off>\r\n` | n/a | Yes — Phase 10B | Conditional — re-use of existing `devkit_status_led.c` API; no new Zephyr driver, but the existing blink semantics must be reconciled. `USER_DECISION_REQUIRED`. | No (single-byte command; bounded fixed response; thread-context handler; no parser/registry/framing) — confirmed against the 12 constraints in `PHASE_9EZ_CHECKPOINT.md §H`. | This is the first row that produces a real physical effect from a UART command. **`USER_DECISION_REQUIRED`** before opening Phase 10B for this row. |
| `T` (0x54) | DRAFT — Onboard sensor read placeholder (e.g. core temperature) | any (proposed) | No app-state change; `uart += 1` | Sensor read (no actuator change) | `TEMP <value>\r\n` (numeric format, units, precision all `USER_DECISION_REQUIRED`) | n/a (or `ERR sensor unavailable\r\n` if driver absent) | Yes — Phase 10B | Yes — sensor driver dependency; `prj.conf` flag(s) and a Zephyr sensor API path. | Conditional — if response stays single-line and bounded, no scope-guard violation; if response grows beyond fixed-buffer size or requires queuing, **would violate** the response-queue / heap / bounded-buffer guards. | **`USER_DECISION_REQUIRED`** on sensor identity, on whether a driver exists for the chosen part, and on response format. |

#### Section C — Explicitly rejected / deferred command shapes (do not implement)

These shapes are documented as **rejected at Phase 10A** because they would
cross one or more scope-guard constraints. Opening any of them requires a
separate architectural decision, not a Phase 10B-class implementation.

| Shape | Why rejected at Phase 10A |
|---|---|
| Multi-byte tokens (e.g. `ARM\r\n`, `STATUS\r\n`, `LED ON\r\n`) | Requires a parser/tokenizer. Violates UART TX scope guard rows 1, 2, and 4 (no shell, no parser, no framing). |
| Commands with arguments (e.g. `s 0.5`, `T sensor=core`) | Requires a parser and bounded argument lexer. Same as above. |
| ACK/NACK + sequence numbers (e.g. `OK seq=42\r\n`) | Requires a sequencing protocol. Violates UART TX scope guard row 9 (no retry/ACK protocol). |
| Streaming/periodic telemetry over UART (e.g. emit `STATE …` every N ticks) | Would replace RTT as the canonical log backend. Violates UART TX scope guard row 7 and the architecture alignment that `ROBOTOS_*` lines are passive RTT-only emissions. |
| Dynamic per-session command registration | Violates UART TX scope guard row 3 (no command registry). |
| Response longer than the current fixed buffer (96 bytes) | Violates UART TX scope guard rows 5 and 6 (no heap, no response queue) without a separate architectural decision to grow the buffer. |
| Commands that read or mutate `core/` or `platform/` internals | Violates the Phase 9A–9E invariant that `core/` / `platform/` are untouched. |
| Commands that originate TX from ISR context | Violates UART TX scope guard row 10 (no TX from ISR). |

### 5.5 Unresolved user decisions (carried forward to Phase 10B opening)

These map to `PHASE_9EZ_CHECKPOINT.md §L Q1–Q5` and remain
`USER_DECISION_REQUIRED`. None are resolved by Phase 10A; Phase 10A only
captures them in writing so the next implementation phase has a concrete
input.

1. **Product application intent.** What should RobotOS actually do in the
   intended use case? (real robot control, monitoring, test fixture,
   educational tool, other.) Phase 10A does not assume an answer.
2. **First real physical-effect command.** If any. Section B's `L` and `T`
   rows are the smallest candidates; neither is approved.
3. **Disarm command introduction.** Whether to introduce `d` as an explicit
   `ARMED → IDLE` transition or rely on `r` for both `ARMED → IDLE` and
   `ACTIVE → IDLE`.
4. **Query-extension policy.** Whether to keep `?` as the single query
   command or add `v` (build info). Section B's `v` row is a candidate, not
   approved.
5. **Robot Framework promotion.** Whether `devkit_app_state` should be
   promoted from devkit-local to a `core/`-adjacent layer, and when. Phase
   10A does not promote it.
6. **Phase 8A (F407) priority.** Phase 10A does not reopen Phase 8A.
7. **Demo-tooling polish (Phase 9F).** Whether to land
   `run_phase9e_uart_response_demo.ps1` PASS/FAIL polish before any Phase 10B
   implementation. Phase 10A does not land it.

### 5.6 Implementation candidates that may become Phase 10B (when approved)

- **Phase 10B-`v` (build query)** — single-byte query, reuses existing
  `devkit_build_info` content, no driver dependency, smallest implementation
  surface among Section B.
- **Phase 10B-`L` (LED toggle)** — single-byte physical effect; requires
  decision on reconciliation with `devkit_status_led` blink semantics.
- **Phase 10B-`d` (explicit disarm)** — single-byte app-state command;
  smallest behavioral surface; useful only if user vocabulary requires
  explicit disarm.
- **Phase 10B-`T` (sensor read)** — single-byte query with physical
  measurement; largest of the four candidates because of the driver
  dependency.

Order is not determined by Phase 10A. Each Phase 10B candidate, when opened,
must reproduce the Phase 9E-style validation discipline: pristine build,
flash via `capture_devkit_rtt.ps1`, RTT counter table, host transcript table,
boundary-preservation audit, scope-guard restated, closeout document.

### 5.7 Architecture preservation audit (Phase 10A)

Phase 10A is docs-only and changed no runtime surface. The following are
preserved unchanged:

- `core/` — zero diff
- `platform/` — zero diff
- `tests/` — zero diff
- `devkit/src/` — zero diff
- `devkit/CMakeLists.txt`, `prj.conf` — zero diff
- `tools/runtime/capture_devkit_rtt.ps1`, `run_phase9e_uart_response_demo.ps1`,
  `run_phase9d_demo.ps1` — zero diff
- Scheduler constants, queue capacity, event-type contract — unchanged
- POST_FLASH_AUTOSTART discipline — unchanged; root cause remains OPEN;
  mitigated-by-workflow from Phase 6O onward; manual RESET retained as
  fallback; plain `west flash` alone is not runtime-start evidence
- UART TX scope guard — twelve constraints in `PHASE_9EZ_CHECKPOINT.md §H`
  remain intact

### 5.8 Next gate

Before any Phase 10B opening, the user must explicitly select one of:

- a row from Section B (and answer that row's `USER_DECISION_REQUIRED` notes);
- a direction-independent supporting phase (Phase 9F demo polish or Phase 9G
  UART burst characterization); or
- a continued hold (no Phase 10B opening).

Phase 10A does not authorize any of these by itself.

---

<a id="phase-9g-late"></a>
## Phase 9G — Bounded UART Burst Characterization (late-9-series, non-linear insert)

**Status:** `CLOSED_WITH_HARDWARE_EVIDENCE`
**Type:** Host-side tooling + docs + hardware evidence. No firmware,
core, platform, test, CMake, Zephyr, board, or runtime-script (other than
the new sibling harness) change.
**Date opened:** 2026-05-11
**Date closed:** 2026-05-11 (same-day hardware run)
**Branch:** master
**Authority for placement here (not in `DEVKIT_PROGRESS.md`):** user
preference to leave the historical master untouched; §4 rule 6 (non-linear
edits explicit) — flagged with `‡` in the Phase Index and the
`phase-9g-late` anchor id.
**Companion closeout:** [`PHASE_9G_CLOSE.md`](PHASE_9G_CLOSE.md).
**Evidence logs:**
[`../logs/phase_9G_uart_burst_host_2026-05-11.txt`](../logs/phase_9G_uart_burst_host_2026-05-11.txt),
[`../logs/phase_9G_uart_burst_rtt_2026-05-11.txt`](../logs/phase_9G_uart_burst_rtt_2026-05-11.txt).

### 9G.1 Purpose

Characterize bounded UART input burst behavior on the proven Phase 9E
command/response loop. Closes the highest-priority outstanding unknown
identified in `PHASE_9EZ_CHECKPOINT.md` section E: *high-rate UART input
not stress-tested*.

This phase is **evidence-only**. It does not change firmware, scheduler,
queue capacity, command vocabulary, or UART TX scope. It does not
authorize Phase 10B implementation.

### 9G.2 Approved implementation surface

- **New host tool:** `RobotOS_v1.0/tools/runtime/run_phase9g_uart_burst_demo.ps1`.
  - Sends the Phase 9E byte vocabulary (`a s ? r x`) as a bounded burst
    (default 30 ms between bytes, ~120 ms total) — faster than the 500 ms
    dispatch tick.
  - Then opens a ~5 s read window and collects all response lines with
    per-byte and per-line wallclock timestamps.
  - Launches the Phase 6O `capture_devkit_rtt.ps1` harness in a background
    PowerShell job for RTT evidence (sidecar `reset run`; manual RESET
    retained as fallback; plain `west flash` alone is not runtime-start
    evidence).
  - Prints PASS-checklist criteria (RECV_COUNT, ordering, cadence) and
    finding-discipline reminders (do NOT mutate scheduler / TX path if
    anomalies are observed).
  - Pure ASCII (encoding-safe for PowerShell `ParseFile`/`PSParser` on
    Windows code-page hosts).
- **New docs:** [`PHASE_9G_CLOSE.md`](PHASE_9G_CLOSE.md) closeout document
  (now `CLOSED_WITH_HARDWARE_EVIDENCE` after the 2026-05-11 hardware run).
- **Existing scripts:** untouched (`capture_devkit_rtt.ps1`,
  `run_phase9d_demo.ps1`, `run_phase9e_uart_response_demo.ps1`,
  `capture_phase6h_runtime.ps1`).
- **No firmware change.** Phase 9G uses the existing Phase 9E firmware
  (`587dab7`).

### 9G.3 Hardware validation results

Hardware run executed 2026-05-11 17:58 local on STM32F411E-DISCO
(ST-LINK V2J47S0 + Silicon Labs CP210x USB-UART on COM5) using the
existing Phase 9E firmware build (commit `587dab7`, ELF at
`build/zephyr/zephyr.elf`, board `stm32f411e_disco`). Flash via
`west flash`, runtime-start observation via Phase 6O
`capture_devkit_rtt.ps1` sidecar `reset run` discipline. Manual RESET was
not required this session.

**Headline result:** PASS. All harness required patterns matched;
CFSR/HFSR `0x00000000` throughout (13 occurrences); 5/5 host responses
in order; 22929-byte RTT log over 60.7 s.

| Metric | Phase 9E baseline | Phase 9G observation |
|---|---|---|
| Host burst width (wallclock) | -- (one byte per ~600 ms) | 185 ms for 5 bytes |
| Host RECV count | 5 | 5 (in order) |
| ROBOTOS_UART rx / ok / handled | 5 / 5 / 5 | 5 / 5 / 5 |
| ROBOTOS_APP transitions / uart / ignored | 3 / 5 / 1 | 3 / 5 / 1 |
| ROBOTOS_OBS peak | 2 | **5** (queue safely held the bounded burst) |
| ROBOTOS_OBS dropped / herr / unhandled / rejected / throttled | all 0 | all 0 |
| accepted - dispatched = pending | invariant holds | 65 - 64 = 1 ✓ |
| CFSR / HFSR | 0x00000000 (13×) | 0x00000000 (13×) |
| Phase 6M producer (attempted/ok at ticks=120) | 60 / 60 | 60 / 60 |

See [`PHASE_9G_CLOSE.md`](PHASE_9G_CLOSE.md) sections D, E, F, H for the
full evidence record and finding classification.

### 9G.4 What this entry does not do

- Does not change `core/`, `platform/`, `tests/`, `devkit/src/`,
  `CMakeLists.txt`, or `prj.conf`.
- Does not alter `ROBOTOS_CORE_MAX_EVENTS_PER_TICK` or
  `ROBOTOS_EVENT_QUEUE_CAPACITY`.
- Does not change Phase 9E firmware, UART TX behavior, or the
  IDLE/ARMED/ACTIVE state model.
- Does not reopen Scheduler 7A / 7B-1 (still DEFER).
- Does not reopen Phase 8A / F407 (still HOLD/DEFER).
- Does not start Phase 10B; `COMMAND_SET_DRAFT.md` section B rows remain
  `USER_DECISION_REQUIRED`.
- Does not modify `DEVKIT_PROGRESS.md` (historical master preserved).
- Does not modify `CURRENT_STATE.md` (the maintenance rule there says to
  update only at phase close from validated evidence; Phase 9G is not
  closed yet).
- Does not introduce parser, shell, command registry, framing protocol,
  response queue, ACK/retry, heap allocation, ISR-context TX, or
  core/platform UART abstraction.

### 9G.5 Next gate

Phase 9G is closed. The Phase 10A "next gate" wording remains in force:
user must explicitly select one of (a) a `USER_DECISION_REQUIRED` row
from `COMMAND_SET_DRAFT.md` section 3 with its open notes answered,
(b) a different direction-independent supporting phase (e.g. Phase 9F
demo polish), or (c) a continued hold. Phase 9G's findings do not by
themselves authorize any Phase 10B opening; they remove a blocking
unknown but do not constitute approval.

---

<a id="phase-10b-v"></a>
## Phase 10B-v — Build/Version Query Command `v`

**Status:** `CLOSED_WITH_HARDWARE_EVIDENCE`
**Type:** First Phase 10B-class implementation. Single-byte command added
to the proven Phase 9E UART RX/TX path; devkit-local source changes only.
No core, platform, scheduler, queue, event-type, test, CMake, Zephyr,
board, or prj.conf change.
**Date opened/closed:** 2026-05-11 (same-day hardware run).
**Branch:** master.
**Companion closeout:** [`PHASE_10B_V_CLOSE.md`](PHASE_10B_V_CLOSE.md).
**Evidence logs:**
[`../logs/phase_10B_v_host_2026-05-11.txt`](../logs/phase_10B_v_host_2026-05-11.txt),
[`../logs/phase_10B_v_rtt_2026-05-11.txt`](../logs/phase_10B_v_rtt_2026-05-11.txt).

### 10B-v.1 Command

| Byte | 0x76 (`v`; case-insensitive, `V` also accepted) |
|---|---|
| Meaning | Build/version/info query |
| Precondition | Any app state |
| App-state side effect | None (state, transitions, ignored, button counters all unchanged) |
| Physical side effect | None |
| Response | `INFO phase=10b-v app=devkit board=<CONFIG_BOARD> tick_ms=<DEVKIT_TICK_MS> uart=minimal\r\n` (77 bytes on `stm32f411e_disco` at `DEVKIT_TICK_MS=500`) |

### 10B-v.2 Implementation surface

Three files changed, all devkit-local:

- `devkit/src/devkit_app_state.c` — added `case 'v':` recognition in
  `devkit_app_state_on_uart_byte()`; emits a `Phase 10B-v build query:
  state=<S>` LOG_INF; no transition; not counted as ignored.
- `devkit/src/devkit_uart_producer.c` — added `case 'v':` response in
  `devkit_uart_emit_tx_response()` producing the fixed `INFO …` line
  via the existing 96-byte stack buffer; added `#include
  "devkit_runtime.h"` for `DEVKIT_TICK_MS`.
- `tools/runtime/run_phase10b_v_build_query_demo.ps1` — new demo
  harness (PowerShell 5.1 compatible, pure ASCII, Phase 6O
  `capture_devkit_rtt.ps1` discipline).

### 10B-v.3 Build + flash + hardware-run summary

| Stage | Result |
|---|---|
| `west build --pristine` (`stm32f411e_disco`) | PASS; FLASH 36416 B (6.95%; +6384 B vs Phase 9E 30032 B); RAM 12224 B (9.33%; +64 B vs 12160 B). No new warnings beyond pre-existing `q_valid` and `drivers__console`. |
| `west flash` | PASS; 49152 bytes; 1.538 s; clean shutdown. Cold examination succeeded first attempt. |
| `run_phase10b_v_build_query_demo.ps1 -ComPort COM5` | PASS; 6 required patterns FOUND (including `Phase 10B-v build query`); CFSR/HFSR all 0x00000000 (13 occurrences); 23226-byte RTT log over 61.2 s; manual RESET not required. |
| Host transcript | 5/5 responses in send order; both `v` responses byte-identical across IDLE → ARMED. |
| RTT counters | `ROBOTOS_UART rx=5 ok=5 full=0 handled=5 last=0x3f`; `ROBOTOS_APP transitions=2 button=0 uart=5 ignored=0`; `ROBOTOS_OBS peak=2 dropped=0 dispatched=64 accepted=65 herr=0`; `ROBOTOS_PROD attempted=60 ok=60`. |
| Invariants | `accepted(65) - dispatched(64) = pending(1)` ✓; `PROD ok(60) + UART ok(5) = accepted(65)` ✓; `transitions(2) = a-event + r-event` ✓; `v` did not transition; `ignored=0`. |

### 10B-v.4 What this entry does not do

- Does not change `core/`, `platform/`, `tests/`, `CMakeLists.txt`, or
  `prj.conf`.
- Does not alter `ROBOTOS_CORE_MAX_EVENTS_PER_TICK` or
  `ROBOTOS_EVENT_QUEUE_CAPACITY`.
- Does not change Phase 9E UART TX behavior for `a`, `s`, `?`, `r`, or
  `x` (their switch branches are untouched).
- Does not reopen Scheduler 7A / 7B-1 (still DEFER).
- Does not reopen Phase 8A / F407 (still HOLD/DEFER).
- Does not promote `devkit_app_state` to `core/` or Robot Framework.
- Does not introduce parser, shell, command registry, framing protocol,
  response queue, ACK/retry, heap allocation, ISR-context TX, or
  core/platform UART abstraction.
- Does not implement `d`, `L`, or `T`; those remain
  `USER_DECISION_REQUIRED` in `COMMAND_SET_DRAFT.md` Section B.
- Does not modify `DEVKIT_PROGRESS.md` (historical master preserved).

### 10B-v.5 Next gate

Phase 10B-v removes the abstraction barrier between Phase 10A planning
and Phase 10B implementation. Further Phase 10B candidates (`L`, `d`,
`T`) remain `USER_DECISION_REQUIRED`; user must approve a specific row
before another phase opens.

---

<a id="phase-10b-l"></a>
## Phase 10B-L — LED Physical-Effect Command `L`

**Status:** `CLOSED_WITH_HARDWARE_EVIDENCE` (electrical/RTT) +
`PHYSICAL_OBSERVATION_AMBIGUOUS` (visual LED not human-witnessed in this
autonomous run).
**Type:** Second Phase 10B-class implementation; **first physical-effect
command**. Single-byte command added to the proven Phase 9E/10B-v UART
RX/TX path; devkit-local source changes only. Reuses the existing
`devkit_status_led_toggle()` one-shot API (LED module unchanged). No
core, platform, scheduler, queue, event-type, test, CMake, Zephyr,
board, or `prj.conf` change.
**Date opened/closed:** 2026-05-11 (same-day hardware run).
**Branch:** master.
**Companion closeout:** [`PHASE_10B_L_CLOSE.md`](PHASE_10B_L_CLOSE.md).
**Evidence logs:**
[`../logs/phase_10B_L_host_2026-05-11.txt`](../logs/phase_10B_L_host_2026-05-11.txt),
[`../logs/phase_10B_L_rtt_2026-05-11.txt`](../logs/phase_10B_L_rtt_2026-05-11.txt).

### 10B-L.1 Command

| Byte | 0x4c (`L`; case-insensitive, `l` also accepted) |
|---|---|
| Meaning | LED physical-effect smoke (single GPIO toggle) |
| Precondition | Any app state |
| App-state side effect | None (no transition, `ignored` not incremented) |
| Physical side effect | One call to `devkit_status_led_toggle()` from thread-context UART handler. The 500 ms heartbeat blink in `devkit_runtime_run()` continues; the `L` toggle shifts its phase by one half-cycle. No new LED API, no LED state machine, no LED service. |
| Response | `OK led=toggle state=<S>\r\n` (26 bytes on `stm32f411e_disco`) |
| Error response | `ERR led=toggle ret=<N> state=<S>\r\n` (not observed in this run) |

### 10B-L.2 Implementation surface

Three files changed, all devkit-local:

- `devkit/src/devkit_app_state.c` — `case 'l':` recognition (no transition, not ignored); emits `Phase 10B-L LED command: state=<S>` LOG_INF.
- `devkit/src/devkit_uart_producer.c` — `case 'l': { ... }` block calls `devkit_status_led_toggle()` then emits `OK led=toggle state=<S>\r\n` via existing 96-byte stack buffer; added `#include "devkit_status_led.h"`.
- `tools/runtime/run_phase10b_l_led_command_demo.ps1` — new demo harness (PowerShell 5.1, pure ASCII, Phase 6O capture discipline).

**Not touched:** `devkit_status_led.{h,c}` (LED module unchanged), `devkit_runtime.c` (heartbeat unchanged), `core/`, `platform/`, `tests/`, `CMakeLists.txt`, `prj.conf`, `boards/`.

### 10B-L.3 Build + flash + hardware-run summary

| Stage | Result |
|---|---|
| `west build --pristine` (`stm32f411e_disco`) | PASS; FLASH 36628 B (6.99%; +212 B vs Phase 10B-v 36416 B); RAM 12224 B (unchanged); no new warnings. |
| `west flash` | PASS; 49152 bytes; 1.574 s; clean shutdown. |
| `run_phase10b_l_led_command_demo.ps1 -ComPort COM5` | PASS; 6 required patterns FOUND (including `Phase 10B-L LED command`); CFSR/HFSR all `0x00000000` (13 occurrences); 22744-byte RTT log over 60.4 s; manual RESET not required. |
| Host transcript | 4/4 responses in send order: `OK led=toggle state=IDLE`, `INFO phase=10b-v …`, `OK led=toggle state=IDLE` (byte-identical to first `L`), `STATE state=IDLE transitions=0 button=0 uart=4 ignored=0`. |
| RTT counters | `ROBOTOS_UART rx=4 ok=4 full=0 handled=4 last=0x3f`; `ROBOTOS_APP transitions=0 button=0 uart=4 ignored=0`; `ROBOTOS_OBS peak=2 dropped=0 dispatched=63 accepted=64`; `ROBOTOS_PROD attempted=60 ok=60`. |
| Invariants | `accepted(64) - dispatched(63) = pending(1)` ✓; `PROD(60) + UART(4) = accepted(64)` ✓; `transitions=0` (no a/s/r) ✓; `L` did not transition and did not increment `ignored`. |
| Heartbeat preservation | 139 `tick count=` lines across the 60.4 s window; no `LED toggle failed` errors; cadence uninterrupted. |
| Physical LED observation | **PHYSICAL_OBSERVATION_AMBIGUOUS** — autonomous run, no human watched the LED. RTT confirms the toggle calls fired (`Phase 10B-L LED command` ×2 + `cmd=0x4c len=26 state=IDLE` ×2; no toggle-failure log). Visible effect (single phase-shift in heartbeat) is predicted but not human-verified. |

### 10B-L.4 What this entry does not do

- Does not change `core/`, `platform/`, `tests/`, `CMakeLists.txt`, or
  `prj.conf`.
- Does not alter `ROBOTOS_CORE_MAX_EVENTS_PER_TICK` or
  `ROBOTOS_EVENT_QUEUE_CAPACITY`.
- Does not modify `devkit_status_led.{h,c}` (no new LED API).
- Does not modify `devkit_runtime.c` (heartbeat loop unchanged).
- Does not change Phase 9E behavior for `a`, `s`, `?`, `r`, `x`.
- Does not change Phase 10B-v `v` behavior (response byte-identical).
- Does not change the `?` response format (no LED field added).
- Does not introduce parser, shell, command registry, framing protocol,
  response queue, ACK/retry, heap allocation, ISR-context TX, or
  core/platform UART abstraction.
- Does not promote `devkit_app_state` to `core/` or Robot Framework.
- Does not implement `d` or `T`; those remain `USER_DECISION_REQUIRED`.
- Does not modify `DEVKIT_PROGRESS.md` (historical master preserved).
- Does not reopen Scheduler 7A / 7B-1 (still DEFER).
- Does not reopen Phase 8A / F407 (still HOLD/DEFER).

### 10B-L.5 Next gate

Phase 10B-L demonstrates the smallest safe physical-effect product
command. Further candidates (`d`, `T`) remain `USER_DECISION_REQUIRED`;
user must approve a specific row before another phase opens. An
operator-witnessed re-run is recommended if visible LED feedback is a
product requirement.

---

<a id="phase-10z"></a>
## Phase 10Z — RESERVED / NOT STARTED

Status: NOT_STARTED

Placeholder only. Use this only for a future Phase 10 checkpoint/closeout if
approved.
