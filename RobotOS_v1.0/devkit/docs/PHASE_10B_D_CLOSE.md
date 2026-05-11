# Phase 10B-d -- Explicit Disarm Command (Close)

**Type:** Third Phase 10B-class implementation; smallest remaining
behavioral surface. Single-byte command added to the proven Phase
9E/10B-v/10B-L UART RX/TX path; devkit-local source changes only. No
new driver, no `prj.conf` change, no physical effect. `d` adds explicit
user-vocabulary "disarm" semantics distinct from `r` (which remains the
canonical reset path).
**Status:** `CLOSED_WITH_HARDWARE_EVIDENCE`
**Date opened/closed:** 2026-05-11 (same-day implement + hardware validation)
**Branch:** master
**Prior runtime baseline:** Phase 10B-L (`a96ce17`).
**Implementation commit:** `125779c` (`feat: add Phase 10B-d disarm command`)
**Companion docs:**
[`DEVKIT_PROGRESS_PHASE_10.md`](DEVKIT_PROGRESS_PHASE_10.md) Phase 10B-d
entry (`<a id="phase-10b-d"></a>`),
[`COMMAND_SET_DRAFT.md`](COMMAND_SET_DRAFT.md) Section A (the `d` row
was promoted from draft Section B in this close).

---

## A. Purpose

Phase 10B-d adds the smallest remaining behavioral product command on
top of the proven Phase 9E/10B-v/10B-L UART loop: a single-byte
disarm command (`d` / 0x64, case-insensitive). It provides explicit
"disarm" vocabulary in the user command set distinct from `r` (reset).

Both commands can land in IDLE, but their *user intent* and *behavior
on the IDLE state* differ:

| Command | From ARMED | From ACTIVE | From IDLE |
|---|---|---|---|
| `r` (reset) | -> IDLE, `transitions++` | -> IDLE, `transitions++` | `ignored++`, no transition |
| `d` (disarm) | -> IDLE, `transitions++` | recognized no-op (`USER_DECISION_REQUIRED_ACTIVE_DISARM`) | recognized no-op (not ignored) |

`d` does **not** replace `r`. `r` keeps its existing `* -> IDLE`
behavior with all its existing edges, including the `ignored++` on
redundant IDLE reset. `T` remains `USER_DECISION_REQUIRED` in
`COMMAND_SET_DRAFT.md` Section B and is **not** opened by Phase 10B-d.

---

## B. Confirmed existing `r` behavior

Read from
[`devkit_app_state.c:141-149`](../src/devkit_app_state.c#L141-L149)
before any change was made:

```c
case 'r':
    if (s_state != DEVKIT_APP_STATE_IDLE) {
        transition(DEVKIT_APP_STATE_IDLE, DEVKIT_APP_SRC_UART,
                   handler_count, byte);
    } else {
        LOG_INF("Phase 9C app: 'r' ignored (already IDLE)");
        s_ignored_count++;
    }
    break;
```

- `r` from ARMED or ACTIVE: `transition(IDLE)` -> `transitions++`;
  response `OK state=IDLE\r\n`.
- `r` from IDLE: `s_ignored_count++`, no transition; response
  `OK state=IDLE unchanged=1\r\n`.

Key: `r` from IDLE **does** increment `ignored`. `d` from IDLE
**does not**. This is the behavioral difference Phase 10B-d
introduces, and it is consistent with `v`/`L` (also recognized no-ops
that do not increment `ignored`).

`r` is preserved zero-diff by Phase 10B-d.

---

## C. ACTIVE disarm decision

`d` from ACTIVE is **classified as `USER_DECISION_REQUIRED_ACTIVE_DISARM`** for
this phase. The current implementation treats ACTIVE the same as IDLE:
recognized no-op, not ignored, response
`OK disarm no-op state=ACTIVE\r\n`. **No safety semantics were invented.**

Rationale:

- No product safety model exists for "active disarm". Silently
  promoting `d` to `ACTIVE -> IDLE` would invent semantics the
  product vocabulary has not yet defined.
- The conservative implementation (recognized no-op) keeps the door
  open without committing to either outcome.
- `r` already provides `ACTIVE -> IDLE` for callers who want that.

The default validation sequence (`d a d ?`) does **not** exercise the
ACTIVE branch. If/when a future phase approves `d` from ACTIVE, the
change is a one-line widening of the `if (s_state ==
DEVKIT_APP_STATE_ARMED)` guard plus a supplemental validation run
with sequence `d a s d ?`. That change is **out of scope** for Phase
10B-d.

---

## D. Files changed

| File | Delta |
|---|---|
| `RobotOS_v1.0/devkit/src/devkit_app_state.c` | +13 -- `case 'd':` recognition; ARMED -> IDLE via existing `transition()`; otherwise `LOG_INF("Phase 10B-d disarm no-op: state=...")` |
| `RobotOS_v1.0/devkit/src/devkit_app_state.h` | +3 -- state-machine diagram and `devkit_app_state_on_uart_byte` doc updated to list `d`/`D` |
| `RobotOS_v1.0/devkit/src/devkit_uart_producer.c` | +14 -- `case 'd':` block; `OK disarm state=IDLE\r\n` when changed, `OK disarm no-op state=<S>\r\n` otherwise. No new include. |
| `RobotOS_v1.0/tools/runtime/run_phase10b_d_disarm_demo.ps1` | new, +288 -- PowerShell 5.1 compatible, pure ASCII demo harness modelled on the Phase 10B-v / 10B-L templates |
| `RobotOS_v1.0/devkit/docs/PHASE_10B_D_CLOSE.md` | new -- this document |
| `RobotOS_v1.0/devkit/docs/DEVKIT_PROGRESS_PHASE_10.md` | Phase 10B-d entry + index row |
| `RobotOS_v1.0/devkit/docs/COMMAND_SET_DRAFT.md` | `d` promoted from Section B DRAFT to Section A IMPLEMENTED |
| `RobotOS_v1.0/devkit/logs/INDEX.md` | Phase 10B-d row |
| `CURRENT_STATE.md` | Phase 10B-d entry under Last Closed Phase |
| `RobotOS_v1.0/devkit/logs/phase_10B_d_host_2026-05-11.txt` | new evidence (401 B host transcript) |
| `RobotOS_v1.0/devkit/logs/phase_10B_d_rtt_2026-05-11.txt` | new evidence (22623 B, 60.4 s RTT capture) |

**Not touched:** `core/`, `platform/`, `tests/`, `CMakeLists.txt`,
`prj.conf`, `boards/`, `devkit/src/devkit_runtime.{c,h}`,
`devkit/src/devkit_status_led.{h,c}`, all other devkit/src files,
`DEVKIT_PROGRESS.md` (historical master).

---

## E. Implementation summary

Two surgical source changes plus the header doc-only update and one
new demo harness.

### E.1 `devkit_app_state.c`

Added `case 'd':` after `case 'l':`, before `default:`:

```c
case 'd':
    /* Phase 10B-d: explicit disarm. ARMED -> IDLE with transition.
     * IDLE: recognized no-op (not ignored -- distinct from 'r').
     * ACTIVE: recognized no-op (USER_DECISION_REQUIRED_ACTIVE_DISARM).
     * 'd' does not replace or alter 'r'. */
    if (s_state == DEVKIT_APP_STATE_ARMED) {
        transition(DEVKIT_APP_STATE_IDLE, DEVKIT_APP_SRC_UART,
                   handler_count, byte);
    } else {
        LOG_INF("Phase 10B-d disarm no-op: state=%s",
                state_name(s_state));
    }
    break;
```

The pre-existing function-entry `s_uart_count++` and the upcase
normalization at the top of the switch continue to apply uniformly,
so `d` and `D` both land here and the global UART count increments
for both ARMED-transition and IDLE/ACTIVE no-op paths. `s_ignored_count`
is **not** incremented in any `d` path.

### E.2 `devkit_uart_producer.c`

Added `case 'd':` after `case 'l': { ... }`, before `default:`:

```c
case 'd':
    /* Phase 10B-d: explicit disarm. If state changed, ARMED->IDLE
     * transition occurred; otherwise recognized no-op from IDLE or
     * ACTIVE (ACTIVE disarm is USER_DECISION_REQUIRED). Bounded
     * fixed-buffer response; no parser, no registry, no framing. */
    if (changed) {
        n = snprintf(buf, sizeof(buf), "OK disarm state=IDLE\r\n");
    } else {
        n = snprintf(buf, sizeof(buf),
                     "OK disarm no-op state=%s\r\n",
                     devkit_app_state_state_name(after->state));
    }
    break;
```

The `changed` flag is the existing before/after `transitions`-counter
delta computed at function entry. No new variables, no new buffer, no
new include. The response uses the existing fixed 96-byte stack
buffer.

### E.3 `devkit_app_state.h`

Doc-only update: added `'d'/'D' -> IDLE (from ARMED only; recognized
no-op from IDLE; ACTIVE disarm is USER_DECISION_REQUIRED)` to the
state-machine summary and to the `devkit_app_state_on_uart_byte()`
recognized-commands list. No behavioral change in the header.

### E.4 `tools/runtime/run_phase10b_d_disarm_demo.ps1`

New host harness modelled on `run_phase10b_v_build_query_demo.ps1` and
`run_phase10b_l_led_command_demo.ps1`:

- PowerShell 5.1 compatible (no ternary, no null-coalescing, no &&/||).
- Pure ASCII (encoding-safe for ParseFile / PSParser).
- Mandatory `-ComPort` parameter.
- Default sequence `d a d ?` at 600 ms inter-byte spacing.
- Launches `capture_devkit_rtt.ps1` in a background job with the Phase
  6O sidecar `reset run` discipline and 60 s capture window.
- Saves host transcript and RTT log under
  `RobotOS_v1.0/devkit/logs/phase_10B_d_*_2026-05-11.txt`.

---

## F. Response format

| Condition | Response | Bytes |
|---|---|---|
| `d` from ARMED (`changed=true`) | `OK disarm state=IDLE\r\n` | 22 |
| `d` from IDLE (`changed=false`) | `OK disarm no-op state=IDLE\r\n` | 28 |
| `d` from ACTIVE (`changed=false`; `USER_DECISION_REQUIRED`) | `OK disarm no-op state=ACTIVE\r\n` | 30 |

All variants fit well within the existing 96-byte stack buffer.

---

## G. Build delta

`west build --pristine -b stm32f411e_disco RobotOS_v1.0/devkit` --
**PASS 152/152**:

| Region | Before (10B-L `f1db2fa`) | After (10B-d `125779c`) | Delta |
|---|---|---|---|
| FLASH | 36628 B (6.99%) | 36780 B (7.02%) | +152 B |
| RAM   | 12224 B (9.33%) | 12224 B (9.33%) | 0 |

FLASH delta is dominated by the new switch arm and snprintf format
strings. Within `stm32f411e_disco` budget (512 KB FLASH / 128 KB RAM).

---

## H. Hardware setup

- **Board:** STM32F411E-DISCO.
- **Power/flash:** ST-LINK USB.
- **UART wiring:**
  - Board PA2 (USART2 TX) -> USB-UART RX
  - Board PA3 (USART2 RX) -> USB-UART TX
  - Board GND -> USB-UART GND
  - 3.3V TTL; USB-UART VCC **not** connected to board
- **Adapter:** Silicon Labs CP210x USB-UART Bridge on **COM5**.
- **Baud:** 115200 8N1.
- **Flashing discipline:** `west flash` followed by
  `capture_devkit_rtt.ps1` sidecar `reset run` (Phase 6O harness;
  POST_FLASH_AUTOSTART mitigated-by-workflow).
- **Manual RESET:** not required this session; retained as fallback.
- **Plain `west flash` alone is not runtime-start evidence** -- the
  sidecar `reset run` is the runtime-start guarantee.

---

## I. Commands run

```powershell
cd D:\Robot_OS
west flash
.\RobotOS_v1.0\tools\runtime\run_phase10b_d_disarm_demo.ps1 -ComPort COM5
```

Default arguments applied: `Sequence=@('d','a','d','?')`,
`InterByteDelayMs=600`, `ReadTimeoutMs=1200`, `CaptureSeconds=60`,
`WaitBeforeInputSeconds=15`.

---

## J. Host transcript

Saved to
[`phase_10B_d_host_2026-05-11.txt`](../logs/phase_10B_d_host_2026-05-11.txt)
(401 B):

```
SEND 'd' (0x64) -> RECV: OK disarm no-op state=IDLE
SEND 'a' (0x61) -> RECV: OK state=ARMED
SEND 'd' (0x64) -> RECV: OK disarm state=IDLE
SEND '?' (0x3f) -> RECV: STATE state=IDLE transitions=2 button=0 uart=4 ignored=0
```

**4 of 4 responses arrived in send order, byte-exact match to
specification.**

Notable verifications from the transcript:

- `d` from IDLE -> `OK disarm no-op state=IDLE` (28 B). Confirms `d`
  is recognized (not ignored) and does not transition from IDLE.
- `d` from ARMED -> `OK disarm state=IDLE` (22 B). Confirms ARMED ->
  IDLE transition, distinct from the no-op response.
- Final `?` reports `transitions=2 button=0 uart=4 ignored=0`:
  - `uart=4`: all four commands incremented the UART counter (`d`,
    `a`, `d`, `?`).
  - `transitions=2`: `a` (IDLE -> ARMED) and second `d` (ARMED ->
    IDLE).
  - `ignored=0`: confirms `d` from IDLE was **not** counted as
    ignored.
  - `?` response format identical to Phase 9E baseline (no field
    change).

---

## K. RTT counter summary

Saved to
[`phase_10B_d_rtt_2026-05-11.txt`](../logs/phase_10B_d_rtt_2026-05-11.txt)
(22623 B, 60.4 s capture, 13 OBS/FAULT/PROD periodic triplets):

### K.1 Per-event RTT lines (in send order)

| Event | RTT line |
|---|---|
| `d` from IDLE | `devkit_app: Phase 10B-d disarm no-op: state=IDLE` |
| `d` from IDLE (TX) | `devkit_uart: Phase 9E UART response sent cmd=0x64 len=28 state=IDLE` |
| `a` IDLE -> ARMED | `devkit_app: Phase 9C app transition: IDLE->ARMED src=UART byte=0x61 count=2 total=1` |
| `d` ARMED -> IDLE | `devkit_app: Phase 9C app transition: ARMED->IDLE src=UART byte=0x64 count=3 total=2` |
| `d` ARMED -> IDLE (TX) | `devkit_uart: Phase 9E UART response sent cmd=0x64 len=22 state=IDLE` |

The TX response lengths (`len=28`, `len=22`) match the host transcript
byte counts exactly.

### K.2 Final counters (capture tail at ticks=120, t=59.525s)

| Counter | Value | Invariant check |
|---|---|---|
| OBS `accepted` | 64 | -- |
| OBS `dispatched` | 63 | `accepted - dispatched = 1` matches `pending=1` |
| OBS `pending` | 1 | held throughout |
| OBS `peak` | 2 | one-byte-per-tick pacing, no burst |
| OBS `dropped` | 0 | no queue saturation |
| OBS `herr` | 0 | no handler errors |
| OBS `throttled` | 0 | -- |
| OBS `rejected` | 0 | -- |
| OBS `unhandled` | 0 | every event handled |
| PROD `attempted` | 60 | Phase 6M producer healthy at ticks=120 (5/10 per tick) |
| PROD `ok` | 60 | -- |
| UART `rx` | 4 | matches host send count |
| UART `ok` | 4 | -- |
| UART `full` | 0 | -- |
| UART `handled` | 4 | -- |
| UART `last` | `0x3f` | last byte was `?` |
| APP `state` | `IDLE` | -- |
| APP `transitions` | 2 | matches `?` response |
| APP `button` | 0 | no button presses during run |
| APP `uart` | 4 | matches UART handled |
| APP `ignored` | 0 | confirms no `d` path incremented ignored |
| APP `last_src` | `UART` | -- |
| APP `last_byte` | `0x3f` | last byte was `?` |

### K.3 Cross-check invariants

| Invariant | Result |
|---|---|
| `accepted - dispatched = pending` (64 - 63 = 1) | **OK** |
| `PROD ok(60) + UART ok(4) = accepted(64)` | **OK** |
| `UART rx = UART handled = APP uart` (4 = 4 = 4) | **OK** |
| CFSR all zero (13 occurrences) | **OK** |
| HFSR all zero (13 occurrences) | **OK** |
| No `dropped=`, `herr=`, `unhandled=` non-zero anywhere | **OK** |

---

## L. UART TX scope-guard restatement

Phase 10B-d does **not** cross any constraint from
[`PHASE_9EZ_CHECKPOINT.md §H`](PHASE_9EZ_CHECKPOINT.md). Each row
restated and re-verified against this commit:

| # | Constraint | Status |
|---|---|---|
| 1 | No shell / prompt / interactive session semantics | OK -- single-byte command, no echo, no prompt |
| 2 | No parser framework, no token lexer, no grammar | OK -- bare C `switch` on a single byte |
| 3 | No command registry; no dynamic handler registration | OK -- switch-case dispatch, statically compiled |
| 4 | No multiline / framed protocol; no delimiter framing; no length prefix | OK -- one byte in, fixed string out, `\r\n` only |
| 5 | No heap; all response buffers stack-allocated; fixed size (96 B) | OK -- max response 30 B, same `char buf[96]` |
| 6 | No response queue; TX is synchronous in the handler | OK -- `snprintf` + `uart_poll_out` inline |
| 7 | No UART logging replacement; RTT remains the canonical log backend | OK -- RTT log unchanged |
| 8 | No core / platform UART abstraction | OK -- `s_uart_dev` stays in `devkit_uart_producer.c` |
| 9 | No retry / ACK protocol; no sequence number; no error recovery | OK -- one shot, one response |
| 10 | No TX from ISR context; `uart_poll_out()` only from handler | OK -- TX still inside `devkit_uart_handler()`, thread context |
| 11 | No promotion of `devkit_app_state` from devkit-local to Robot Framework | OK -- module stays devkit-local |
| 12 | Single-byte command vocabulary preserved | OK -- `d` is one byte |

---

## M. Architecture-boundary preservation

| Surface | Status |
|---|---|
| `RobotOS_v1.0/core/` | **zero diff** vs 10B-L baseline |
| `RobotOS_v1.0/platform/` | **zero diff** |
| `RobotOS_v1.0/devkit/src/devkit_runtime.{c,h}` | **zero diff** |
| `RobotOS_v1.0/devkit/src/devkit_status_led.{h,c}` | **zero diff** |
| `RobotOS_v1.0/devkit/src/devkit_button.{c,h}` | **zero diff** |
| Scheduler constants (`ROBOTOS_CORE_MAX_EVENTS_PER_TICK=1`) | **unchanged** |
| Event queue capacity (`ROBOTOS_EVENT_QUEUE_CAPACITY=16`) | **unchanged** |
| `prj.conf`, `CMakeLists.txt`, board / DTS | **zero diff** |
| `tests/` | **zero diff** |
| `DEVKIT_PROGRESS.md` (historical master) | **zero diff** |
| Phase 9E / 9G / 10B-v / 10B-L evidence | **not rewritten** |

`r`, `v`, `L`, `?`, `x`, `a`, `s` behavior is preserved zero-diff in
both `devkit_app_state.c` and `devkit_uart_producer.c`.

---

## N. Operational reminders carried forward

- **Scheduler 7A/7B** remains `DEFER`. Phase 10B-d peak=2 dropped=0 is
  identical to Phase 9E/10B-v/10B-L; no admission-budget mutation
  justified.
- **STM32F407 / custom board** remains `HOLD/DEFER`. No board change
  in Phase 10B-d.
- **UART TX** remains minimal response only. The 12 scope-guard
  constraints in §L are intact.
- **`T` (sensor read)** remains `USER_DECISION_REQUIRED` in
  `COMMAND_SET_DRAFT.md` Section B. Phase 10B-d does **not** open it.
- **POST_FLASH_AUTOSTART** root cause remains **OPEN**;
  MITIGATED_BY_WORKFLOW from Phase 6O onward via
  `capture_devkit_rtt.ps1` sidecar `reset run`. Manual RESET was not
  required this session but is retained as fallback. Plain `west
  flash` alone is **not** runtime-start evidence.
- **ACTIVE disarm** remains `USER_DECISION_REQUIRED_ACTIVE_DISARM`
  per §C above. The current implementation treats it as a recognized
  no-op (not ignored). No safety semantics were invented.
- **RTT** remains the canonical log backend; UART is not a log
  channel.
- **`uart_poll_out()`** runs only from handler (thread context).

---

## O. Verdict

**`CLOSED_WITH_HARDWARE_EVIDENCE`**

- Host transcript: 4 of 4 responses byte-exact, in send order.
- RTT capture: 22623 B over 60.4 s; 13 OBS/FAULT/PROD periodic
  triplets; all 6 required patterns found; `accepted(64) -
  dispatched(63) = pending(1)` invariant holds; PROD ok(60) + UART
  ok(4) = accepted(64); CFSR/HFSR `0x00000000` (13× each);
  `dropped=0 herr=0 throttled=0 rejected=0 unhandled=0`.
- Build delta: FLASH +152 B, RAM 0. Within budget.
- Scope guard: all 12 constraints intact.
- Architecture boundaries: all preserved zero-diff.
- ACTIVE disarm: documented as `USER_DECISION_REQUIRED_ACTIVE_DISARM`;
  no safety semantics invented.
- POST_FLASH_AUTOSTART discipline: preserved (sidecar `reset run`,
  manual RESET fallback retained, plain `west flash` not treated as
  runtime-start evidence).

The non-sensor command group is now hardware-validated end-to-end:
`a` / `s` / `r` / `?` / `x` / `v` / `L` / `d`. **`T` is the only
remaining `USER_DECISION_REQUIRED` row in `COMMAND_SET_DRAFT.md`
Section B.**

---

## P. What this document does not do

- Does not open Phase 10B-`T`.
- Does not authorize ACTIVE -> IDLE disarm; ACTIVE behavior remains
  `USER_DECISION_REQUIRED`.
- Does not modify Phase 9E / 9G / 10B-v / 10B-L closeouts or evidence.
- Does not modify `DEVKIT_PROGRESS.md` (historical master).
- Does not change `r`, `v`, `L`, `?`, `x`, `a`, `s` behavior.
- Does not change scheduler constants, queue capacity, F407/board
  state, or POST_FLASH_AUTOSTART status.
- Does not push. Pushing the implementation + evidence stack is a
  separate user-gated step.
