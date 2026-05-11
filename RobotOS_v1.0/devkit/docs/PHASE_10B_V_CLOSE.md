# Phase 10B-v -- Build/Version Query Command (Close)

**Type:** First real Phase 10B implementation. Single-byte command added to
the proven Phase 9E UART RX/TX path; minimal devkit-local source changes
only. No core, platform, scheduler, queue, event-type, or test change.
**Status:** `CLOSED_WITH_HARDWARE_EVIDENCE`
**Date opened/closed:** 2026-05-11 (same-day hardware run)
**Branch:** master
**Prior runtime checkpoint:** Phase 9E-Z (`30bae27`). Prior characterization:
Phase 9G (commit `21ab1f0`). Prior planning: Phase 10A (`31a498b`).
**Companion docs:**
[`DEVKIT_PROGRESS_PHASE_10.md`](DEVKIT_PROGRESS_PHASE_10.md) Phase 10B-v
entry (`<a id="phase-10b-v"></a>`),
[`COMMAND_SET_DRAFT.md`](COMMAND_SET_DRAFT.md) Section A (the `v` row was
promoted from draft Section B in this commit).

---

## A. Purpose

Phase 10B-v adds the smallest possible product command on top of the proven
Phase 9E loop: a single-byte build/version query (`v` / 0x76) that returns
a bounded `INFO ...` response line.

This is the first of the four Phase 10B-class candidates identified in
`COMMAND_SET_DRAFT.md` Section B. The other three (`d` explicit disarm,
`L` LED toggle, `T` sensor read) remain `USER_DECISION_REQUIRED` and are
not implemented in this phase.

Phase 10B-v deliberately stays within the UART TX scope guard's twelve
constraints (`PHASE_9EZ_CHECKPOINT.md` section H): single-byte command,
fixed switch/snprintf, bounded fixed-buffer response, thread-context
handler, no parser, no shell, no command registry, no framing, no response
queue, no heap, no ISR-context TX, no core/platform UART abstraction, no
Robot Framework promotion.

---

## B. Implementation summary

Three surgical changes; total +18 / -2 lines of devkit-local C.

### B.1 `RobotOS_v1.0/devkit/src/devkit_app_state.c`

Added a `case 'v':` branch in `devkit_app_state_on_uart_byte()` after the
existing `case '?':`. The branch:

- Does **not** increment `s_ignored_count`.
- Does **not** call `transition()`.
- Emits a single `LOG_INF` `Phase 10B-v build query: state=<S>` line for
  RTT trace.
- Falls through the function so `s_uart_count` (incremented at function
  entry) and `s_last_src=UART` / `s_last_byte=byte` (set at function entry)
  are recorded as for any UART byte.

This places `v` in the same recognized-but-non-transitioning class as
`?` query. The existing `default:` clause continues to increment
`s_ignored_count` for any byte not in `a / s / r / ? / v`.

### B.2 `RobotOS_v1.0/devkit/src/devkit_uart_producer.c`

Added a `case 'v':` branch in `devkit_uart_emit_tx_response()` before the
`default:` clause. The branch produces:

```text
INFO phase=10b-v app=devkit board=<CONFIG_BOARD> tick_ms=<DEVKIT_TICK_MS> uart=minimal\r\n
```

Implemented as one `snprintf(buf, sizeof(buf), ...)` into the existing
96-byte stack `buf[]`. On `stm32f411e_disco` at `DEVKIT_TICK_MS=500` the
literal response is 77 bytes (confirmed at runtime by the existing
`Phase 9E UART response sent cmd=0x76 len=77` log line) -- well within
the buffer.

Added `#include "devkit_runtime.h"` to bring `DEVKIT_TICK_MS` into scope.
No other include change.

### B.3 What was deliberately NOT done

- **No new event type.** `v` reuses `DEVKIT_UART_PRODUCER_TYPE` (USER+3).
- **No parser, no token lexer, no grammar.**
- **No command registry / dynamic dispatch.** The new branch lives in the
  same `switch(c)` body as the Phase 9E command set.
- **No multi-byte framing**, no `\r\n` payload terminator parsing on
  input.
- **No response queue, no heap, no ISR TX.** Response is synchronous in
  the existing thread-context handler.
- **No build-info framework.** `v` reuses two pre-existing devkit-local
  constants (`CONFIG_BOARD` from Kconfig, `DEVKIT_TICK_MS` from
  `devkit_runtime.h`). A `phase=10b-v` tag is hard-coded in the response
  format string as a stable closeout identifier.
- **No `__DATE__` / `__TIME__`.** Response is deterministic across
  identical builds.
- **No `?` query format change.** The Phase 9E `STATE state=…
  transitions=… button=… uart=… ignored=…` shape is unchanged. `v` is a
  separate response template.
- **No change to existing `a`, `s`, `?`, `r`, `x` behavior.** Confirmed by
  hardware run: `a` → `OK state=ARMED`, `r` → `OK state=IDLE`, `?` → the
  same `STATE …` shape, transitions counter unaffected by `v`.
- **No core, platform, scheduler, queue, test, CMake, Zephyr, or
  prj.conf change.**

---

## C. Response format

```text
INFO phase=10b-v app=devkit board=stm32f411e_disco tick_ms=500 uart=minimal\r\n
```

Total length on `stm32f411e_disco` at `DEVKIT_TICK_MS=500`: **77 bytes**
(`devkit_uart` log line: `Phase 9E UART response sent cmd=0x76 len=77`).
Fits in the existing 96-byte stack buffer with margin.

Fields are stable across identical builds; only `CONFIG_BOARD` and
`DEVKIT_TICK_MS` substitute, both compile-time constants. No timestamps,
no runtime-dependent fields.

`v` is intentionally **case-insensitive** -- the existing handler upcase
maps in `devkit_uart_producer.c` and `devkit_app_state.c` already convert
`V` (0x56) to `v` (0x76) before the switch. Confirmed by code inspection;
the live hardware run used lowercase only.

---

## D. Build result

```
Command: west build --pristine -b stm32f411e_disco RobotOS_v1.0/devkit/
Result:  PASS (exit 0)
FLASH:   36416 B / 524288 B  (6.95%)   [Phase 9E baseline 30032 B; +6384 B]
RAM:     12224 B / 131072 B  (9.33%)   [Phase 9E baseline 12160 B; +64 B]
Errors:  0
Warnings:
  - 1 pre-existing in core/robotos_event_queue.c (q_valid unused)
  - 1 pre-existing CMake notice "No SOURCES given to Zephyr library: drivers__console"
```

The +6384 B FLASH delta is dominated by the additional cbprintf format
strings (`INFO phase=10b-v app=devkit board=%s tick_ms=%d uart=minimal`
plus the `Phase 10B-v build query: state=%s` LOG_INF) and the
corresponding Zephyr logging plumbing pulled in by the new format
specifiers. No new linked subsystems; no new dependencies.

---

## E. Hardware setup and commands run

- **Board:** STM32F411E-DISCO (rev D); MCU Cortex-M4 r0p1; flash 512 KiB;
  device id 0x10006431.
- **Probe:** ST-LINK V2 firmware V2J47S0, VID:PID 0483:3748.
- **USB-UART:** Silicon Labs CP210x on COM5 @ 115200 8N1.
- **Toolchain:** Zephyr SDK 0.17.0; west 1.5.0; OpenOCD xpack
  0.12.0+dev-02228-ge5888bda3-dirty.

Commands (all from repo root):

1. `west build --pristine -b stm32f411e_disco RobotOS_v1.0\devkit
   --build-dir D:\Robot_OS\build` -- exit 0; FLASH/RAM as above.
2. `west flash --build-dir D:\Robot_OS\build` -- wrote 49152 bytes from
   `zephyr.hex` in 1.538 s; exit 0; clean shutdown. Cold-flash
   examination succeeded on first attempt this session.
3. `.\RobotOS_v1.0\tools\runtime\run_phase10b_v_build_query_demo.ps1
   -ComPort COM5` -- defaults (sequence `v a v r ?`, InterByteDelayMs=600,
   ReadTimeoutMs=1200, CaptureSeconds=60, WaitBeforeInputSeconds=15).
   Harness launched `capture_devkit_rtt.ps1` (sidecar `reset run` --
   Phase 6O POST_FLASH_AUTOSTART mitigation discipline); ran 61.2 s
   stream window; 23226 bytes captured. **Manual RESET was not required
   this session.**

---

## F. Host transcript summary

Five commands sent, five responses received in order:

```
SEND 'v' (0x76) -> RECV: INFO phase=10b-v app=devkit board=stm32f411e_disco tick_ms=500 uart=minimal
SEND 'a' (0x61) -> RECV: OK state=ARMED
SEND 'v' (0x76) -> RECV: INFO phase=10b-v app=devkit board=stm32f411e_disco tick_ms=500 uart=minimal
SEND 'r' (0x72) -> RECV: OK state=IDLE
SEND '?' (0x3f) -> RECV: STATE state=IDLE transitions=2 button=0 uart=5 ignored=0
```

Key checks:

- **Both `v` responses are byte-identical** despite intervening state
  change (IDLE then ARMED). Confirms `v` is state-invariant.
- **`?` reports `transitions=2`** (not 5). The two transitions are
  `IDLE → ARMED` (via `a`) and `ARMED → IDLE` (via `r`). The two `v`
  bytes and the `?` itself were not transitions. **Confirms `v` does
  not cause an app-state transition.**
- **`?` reports `ignored=0`.** Confirms `v` was not classified as
  ignored.
- **`?` reports `uart=5`.** All 5 UART events were counted as recognized
  bytes.
- **`?` reports `button=0`.** No button input mixed in; clean isolation.

---

## G. RTT counter summary (final, ticks=120)

```text
ROBOTOS_OBS  state=READY ticks=120 pending=1 peak=2 dropped=0
             dispatched=64 herr=0 throttled=0 rejected=0 accepted=65
             unhandled=0 bp=0 th_active=0
ROBOTOS_UART rx=5 ok=5 full=0 invalid=0 other=0 handled=5 last=0x3f type=USER+3
ROBOTOS_APP  state=IDLE transitions=2 button=0 uart=5 ignored=0
             last_src=UART last_byte=0x3f
ROBOTOS_FAULT active=0 cfsr=0x00000000 hfsr=0x00000000 context=none
ROBOTOS_PROD attempted=60 ok=60 throttled=0 dropped=0 invalid=0 other=0 type=USER+1
```

Invariants:

- `accepted(65) - dispatched(64) = pending(1)` -- holds.
- `PROD ok(60) + UART ok(5) = accepted(65)` -- holds.
- `transitions(2) = a-event + r-event` -- holds. `v` (2x) and `?` (1x)
  did not transition; `ignored=0`.
- `ROBOTOS_UART last=0x3f`, `ROBOTOS_APP last_byte=0x3f` -- last byte
  observed was `?`, after the second `v` (consistent with send order).
- CFSR=HFSR=0 for all 13 ROBOTOS_FAULT emissions in the 60 s window.

Per-command RTT trace (firmware time):

```text
00:14.504  Phase 10B-v build query: state=IDLE      (cmd=0x76 #1)
00:14.510  Phase 9E UART response sent cmd=0x76 len=77 state=IDLE
00:15.510  Phase 9C app transition: IDLE->ARMED src=UART byte=0x61 count=2 total=1
00:15.511  Phase 9E UART response sent cmd=0x61 len=16 state=ARMED
00:16.512  Phase 10B-v build query: state=ARMED     (cmd=0x76 #2)
00:16.519  Phase 9E UART response sent cmd=0x76 len=77 state=ARMED
00:17.519  Phase 9C app transition: ARMED->IDLE src=UART byte=0x72 count=4 total=2
00:17.519  Phase 9E UART response sent cmd=0x72 len=15 state=IDLE
00:18.520  Phase 9C app query: state=IDLE transitions=2
00:18.520  Phase 9E UART response sent cmd=0x3f len=60 state=IDLE
```

The Phase 10B-v `state=ARMED` query log line is the first evidence in
repo history of a Phase 10B feature exercising the existing 9C state
machine without modifying it.

---

## H. Validation summary

| Check | Result |
|---|---|
| Build | PASS (exit 0; FLASH 36416 B / RAM 12224 B; no errors; only pre-existing warnings) |
| Flash | PASS (49152 bytes written in 1.538 s; clean shutdown) |
| Harness pattern check | PASS (6 patterns FOUND including `Phase 10B-v build query`; CFSR/HFSR all zero, 13 occurrences) |
| 5/5 host responses | PASS (in send order) |
| `v` state-invariant | PASS (both `v` responses byte-identical despite IDLE -> ARMED) |
| `v` does not transition | PASS (transitions=2 after `v a v r ?`, matching only the `a` and `r` transitions) |
| `v` does not increment ignored | PASS (ignored=0) |
| Existing `a`/`s`/`r`/`?`/`x` unchanged | PASS (`a`, `r`, `?` all produce the same shapes as Phase 9E; `s` and `x` not exercised in this minimal demo but their switch branches are untouched) |
| accepted - dispatched = pending | PASS (65 - 64 = 1) |
| Phase 6M producer healthy | PASS (attempted=60 ok=60 at ticks=120, +5 per 10 ticks for 60 s) |
| No `dropped` / `herr` / `unhandled` / `rejected` / `throttled` | PASS (all 0) |
| No fault | PASS (active=0, CFSR=HFSR=0) |
| POST_FLASH_AUTOSTART discipline | PASS (capture via `capture_devkit_rtt.ps1` sidecar `reset run`; manual RESET not required) |
| Plain `west flash` not treated as runtime-start evidence | PASS (runtime-start observed via RTT stream and host responses, not via flash exit code) |
| Architecture preservation | PASS (`core/`, `platform/`, `tests/`, `CMakeLists.txt`, `prj.conf`, `boards/` all zero diff; `ROBOTOS_CORE_MAX_EVENTS_PER_TICK = 1` and `ROBOTOS_EVENT_QUEUE_CAPACITY = 16` unchanged) |

---

## I. Architecture preservation audit (Phase 10B-v)

Confirmed unchanged at this commit:

- `core/` -- zero diff
- `platform/` -- zero diff
- `tests/` -- zero diff
- `devkit/CMakeLists.txt`, `devkit/prj.conf` -- zero diff
- `boards/` -- zero diff (F407 / custom-board files untouched)
- Scheduler constants (`ROBOTOS_CORE_MAX_EVENTS_PER_TICK = 1`,
  `ROBOTOS_EVENT_QUEUE_CAPACITY = 16`) -- unchanged
- Event-type contract -- `v` reuses `DEVKIT_UART_PRODUCER_TYPE`
  (USER+3); no new event type
- UART TX scope guard (12 constraints, `PHASE_9EZ_CHECKPOINT.md`
  section H) -- intact
- POST_FLASH_AUTOSTART discipline -- unchanged; root cause OPEN;
  mitigated-by-workflow from Phase 6O onward; manual RESET retained as
  fallback; plain `west flash` alone is not runtime-start evidence
- RTT canonical log backend -- unchanged
- `devkit_app_state` -- still devkit-local; not promoted to Robot
  Framework

Devkit-local changes (the only files modified):

- `devkit/src/devkit_app_state.c` (+6 lines, -0)
- `devkit/src/devkit_uart_producer.c` (+12 lines, -0; one new `#include`)
- New: `tools/runtime/run_phase10b_v_build_query_demo.ps1`

---

## J. Evidence artifacts

- Host transcript: [`../logs/phase_10B_v_host_2026-05-11.txt`](../logs/phase_10B_v_host_2026-05-11.txt)
- RTT log: [`../logs/phase_10B_v_rtt_2026-05-11.txt`](../logs/phase_10B_v_rtt_2026-05-11.txt)
  -- 23226 bytes, 61.2 s
- Log index row: added to [`../logs/INDEX.md`](../logs/INDEX.md) in this
  commit.

---

## K. Other commands' status (unchanged in Phase 10B-v)

`COMMAND_SET_DRAFT.md` Section B candidates after Phase 10B-v:

| Byte | Status after Phase 10B-v |
|---|---|
| `v` (0x76) | **IMPLEMENTED -- Phase 10B-v** (this commit). Promoted to Section A. |
| `d` (0x64) | `USER_DECISION_REQUIRED` -- not implemented |
| `L` (0x4c) | `USER_DECISION_REQUIRED` -- not implemented |
| `T` (0x54) | `USER_DECISION_REQUIRED` -- not implemented |

Scheduler 7A / 7B-1: DEFER (no workload-driven justification).
Phase 8A / F407: HOLD/DEFER.
UART TX: minimal response only.

---

## L. Recommendation

- **Safe to consider Phase 10B-L (LED toggle)** as the next implementation
  step. Phase 10B-v established the pattern: one switch branch + one
  bounded response template. `L` adds a single physical effect inside the
  same handler (call into existing `devkit_status_led.c` API). User
  decision still required on LED-blink semantics reconciliation
  (per-command latch vs. layered toggle).
- **Phase 10B-d (explicit disarm)** also safe; smallest behavioral
  surface; only adds clarity over existing `r` semantics from `ARMED`.
- **Phase 10B-T (sensor read)** unchanged risk profile; depends on
  driver choice.
- **Continued hold** equally defensible. Phase 10B-v is sufficient on
  its own to demonstrate the planning-to-implementation pipeline.

Not recommended:

- **Scheduler audit / 7B-1 reopening** -- Phase 10B-v session showed
  `peak=2 dropped=0` (matching Phase 9E's spaced-byte baseline). Phase 9G
  already characterized the bounded-burst case. No workload-driven
  justification.
- **Phase 8A (F407)** -- unchanged; HOLD/DEFER.

---

## M. Cross-references

- Phase 9E-Z direction guard: [`PHASE_9EZ_CHECKPOINT.md`](PHASE_9EZ_CHECKPOINT.md)
- Phase 9G burst characterization closeout: [`PHASE_9G_CLOSE.md`](PHASE_9G_CLOSE.md)
- Phase 10A planning closeout: [`PHASE_10A_CLOSE.md`](PHASE_10A_CLOSE.md)
- DRAFT command vocabulary: [`COMMAND_SET_DRAFT.md`](COMMAND_SET_DRAFT.md)
- Phase log: [`DEVKIT_PROGRESS_PHASE_10.md`](DEVKIT_PROGRESS_PHASE_10.md)
- Live state snapshot: [`../../../CURRENT_STATE.md`](../../../CURRENT_STATE.md)
- Telemetry reference: [`TELEMETRY_REFERENCE.md`](TELEMETRY_REFERENCE.md)
- Runtime tools: [`../../tools/runtime/README.md`](../../tools/runtime/README.md)
