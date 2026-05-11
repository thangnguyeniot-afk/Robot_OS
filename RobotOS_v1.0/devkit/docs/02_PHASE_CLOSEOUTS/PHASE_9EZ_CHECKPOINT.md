# Phase 9E-Z — Command Loop Checkpoint / Direction Guard

**Type:** Audit-only / checkpoint. No source, runtime, test, CMake, or Kconfig change.
**Date:** 2026-05-09
**Branch:** master
**HEAD at checkpoint:** `30bae27` (Phase 9E closeout commit)
**Scope:** Phase 9E evidence review + end-to-end capability summary + architecture
alignment audit + direction guard for the full Phase 9A–9E stream.

---

## A. Executive Summary

Phase 9E closed the first real host↔board command/response loop on RobotOS:
host UART command → core queue → app state transition/query → UART TX response.
All five commanded responses matched expectations; RTT and host transcript both
confirm correct behavior; architecture boundaries are preserved; CFSR/HFSR = 0
throughout. No core, platform, scheduler, or budget change was required.

The Phase 9A–9E stream has now proven a complete minimal embedded runtime loop
with two real hardware inputs (button + UART) and one real hardware output (UART
TX). This is a significant capability milestone. It is also the natural stopping
point for exploratory runtime growth: further work requires a product-direction
decision, not another implementation phase.

**Architecture verdict:** ON_TRACK.
**Scheduler verdict:** DEFER confirmed — no workload-driven justification for mutation.
**UART TX scope:** frozen at minimal polled response — no shell, no parser, no protocol.
**Recommended next move:** checkpoint now; await user product-direction decision.

---

## B. Phase 9E Evidence Review

### B.1 Host UART Transcript (confirmed)

File: `devkit/logs/phase_9E_uart_tx_response_host_2026-05-09.txt`

```text
SEND 'a' (0x61) -> RECV: OK state=ARMED
SEND 's' (0x73) -> RECV: OK state=ACTIVE
SEND '?' (0x3f) -> RECV: STATE state=ACTIVE transitions=2 button=0 uart=3 ignored=0
SEND 'r' (0x72) -> RECV: OK state=IDLE
SEND 'x' (0x78) -> RECV: ERR ignored byte=0x78 state=IDLE
```

All 5 responses received without timeout. `InterByteDelayMs=600`,
`ReadTimeoutMs=1200`. No response exceeded the dispatch window (one event per
500 ms tick): all responses arrived within the 600 ms pre-read sleep, confirming
the TX path adds negligible latency beyond the dispatch tick.

The `?` query correctly reports `uart=3` at query time (a, s, ? have all been
processed by the app — uart_count increments on every `on_uart_byte()` call
including queries). The final board state after all 5 commands is `IDLE`, as
designed.

### B.2 RTT Log (confirmed)

File: `devkit/logs/phase_9E_uart_tx_response_rtt_2026-05-09.txt`
Duration: 61.2 s | Size: 23,188 bytes | Harness exit: 0

| RTT Counter | Expected | Observed | Pass? |
| ----------- | -------- | -------- | ----- |
| ROBOTOS_UART rx | 5 | 5 | ✓ |
| ROBOTOS_UART ok | 5 | 5 | ✓ |
| ROBOTOS_UART full | 0 | 0 | ✓ |
| ROBOTOS_UART handled | 5 | 5 | ✓ |
| ROBOTOS_APP transitions | 3 | 3 | ✓ |
| ROBOTOS_APP uart | 5 | 5 | ✓ |
| ROBOTOS_APP ignored | 1 | 1 | ✓ |
| ROBOTOS_APP last_byte | 0x78 | 0x78 | ✓ |
| ROBOTOS_APP final state | IDLE | IDLE | ✓ |
| OBS accepted | — | 25 | ✓ |
| OBS dispatched | — | 24 | ✓ |
| OBS pending | — | 1 | ✓ |
| OBS peak | — | 2 | ✓ |
| OBS dropped | 0 | 0 | ✓ |
| OBS herr | 0 | 0 | ✓ |
| CFSR | 0x00000000 | 0x00000000 × 13 | ✓ |
| HFSR | 0x00000000 | 0x00000000 × 13 | ✓ |
| Phase 6M ok | — | 60 at ticks=120 | ✓ |

OBS invariant: `accepted(25) − dispatched(24) = pending(1)` ✓

Counter cross-check: `PROD ok(20) + UART ok(5) = accepted(25)` ✓

Full dispatch chain confirmed for each command (RTT timestamps):

| Firmware time | tick | Byte | App outcome | TX response confirmed |
| ------------- | ---- | ---- | ----------- | --------------------- |
| 00:00:14.504 | 30 | 0x61 `a` | IDLE→ARMED | len=16 |
| 00:00:15.505 | 31 | 0x73 `s` | ARMED→ACTIVE | len=17 |
| 00:00:16.507 | 33 | 0x3f `?` | query | len=60 |
| 00:00:17.512 | 35 | 0x72 `r` | ACTIVE→IDLE | len=15 |
| 00:00:18.513 | 37 | 0x78 `x` | ignored | len=34 |

### B.3 Architecture Boundary (confirmed)

- `core/` — zero diff from Phase 9D/9Z baseline
- `platform/` — zero diff
- `tests/` — zero diff
- `CMakeLists.txt` — zero diff (no new compiled source)
- `prj.conf` — zero diff (`CONFIG_SERIAL=y` already present from Phase 9B)
- Scheduler constants — unchanged (`ROBOTOS_CORE_MAX_EVENTS_PER_TICK = 1`,
  `ROBOTOS_EVENT_QUEUE_CAPACITY = 16`)
- `uart_poll_out()` called only from handler (thread context, never from ISR) — confirmed
- RTT is canonical logging backend — `CONFIG_LOG_BACKEND_RTT=y`,
  `CONFIG_UART_CONSOLE=n` confirmed preserved

---

## C. First Failure Root Cause and Recovery Lesson

**Root cause:** `BLOCKED_FLASH` — `west flash` returned `Error: open failed` on
the first attempt. The ST-LINK/V2 was not detected by OpenOCD (board not yet
connected or USB not re-enumerated after a prior session). The board continued
running the previous Phase 9D firmware image.

**Consequence chain:**
1. Board was running old Phase 9D firmware (no Phase 9E TX response code).
2. RTT harness connected to a fresh ELF (`_SEGGER_RTT` at `0x20000a5c`), but the
   old firmware had `_SEGGER_RTT` at a different address — RTT reported "No
   control block found."
3. All 5 UART commands timed out on the host: firmware never entered the Phase 9E
   handler, so no TX response was emitted.
4. Host transcript: all `(timeout - no response)`.

**Recovery:**
1. Kill stale OpenOCD processes.
2. Confirm COM port (CP2102 had been briefly undetected; plugging into a
   mainboard port restored it).
3. Pristine build (`west build --pristine`) to ensure clean ELF.
4. `west flash` — ST-LINK detected, 49,152 bytes written, `reset run` confirmed.
5. RTT precheck (20 s): all patterns found, CFSR/HFSR zero, 8,931 bytes.
6. Full demo: PASS in one shot.

**Lesson recorded:** The `POST_FLASH_AUTOSTART` issue documented in
`tools/runtime/README.md` is the known failure mode. The `capture_devkit_rtt.ps1`
script issues `reset run` to start the firmware cleanly after flash; this works
when the flash actually succeeded. The primary failure guard is to confirm flash
exit status before running RTT or UART demo.

---

## D. End-to-End Capability Now Proven

The Phase 9A–9E stream has validated a complete minimal embedded command/response
runtime on the STM32F411E-DISCO:

```text
[Host machine]                     [STM32F411E-DISCO — RobotOS]
                                   ┌─────────────────────────────┐
 UART byte 'a'  ───PA3 RX────────► ISR: uart_fifo_read()
                                     robotos_core_post_event()  (Phase 5G ISR-safe)
                                   core queue (capacity=16, budget=1/tick)
                                     robotos_core_tick()
                                     devkit_uart_handler()       (thread ctx)
                                     devkit_app_state_on_uart_byte()
                                     devkit_uart_emit_tx_response()
                                     uart_poll_out()  ──PA2 TX──►
 RECV: "OK state=ARMED\r\n" ◄────────────────────────────────────┘

 RTT (via OpenOCD TCP) ◄──SWDIO──► Phase 9E UART response sent cmd=0x61 ...
                                   ROBOTOS_UART rx=N ok=N handled=N ...
                                   ROBOTOS_APP state=ARMED transitions=1 ...
```

**What this proves:**

1. Two independent real hardware event sources (GPIO/EXTI button, UART RX IRQ)
   co-exist cleanly in the same event queue without routing conflict.
2. App-level state machine (IDLE/ARMED/ACTIVE) correctly interprets events from
   both sources and applies them as designed.
3. UART TX response is emitted from handler (thread context) via `uart_poll_out()`.
   Board sends a bounded, human-readable ASCII response for every recognized command.
4. The entire pipeline — from ISR → queue → dispatch → handler → TX — fits within
   a single 500 ms tick window with negligible latency.
5. RTT evidence and host UART transcript together close the evidence loop: the RTT
   log proves internal firmware behavior; the host transcript proves the physical
   wire path.

**Classification:** Proven minimal command/state feedback loop on real hardware.

**What this is NOT:**
- A product protocol or command shell
- Robot Framework (no Robot Framework abstraction added)
- A scheduler design
- An extensible command API

---

## E. Explicit Non-Proofs / Remaining Unknowns

The following are explicitly **not proven** by Phase 9A–9E:

| Item | Status |
| ---- | ------ |
| Line-based command framing (`\r\n` terminated) | Not designed, not tested |
| Multi-byte command parsing (e.g. `ARM\r\n`) | Not implemented |
| UART flow control (CTS/RTS) | Not implemented |
| High-rate UART input (>1 byte per 500 ms) | Not stress-tested |
| UART TX under heavy load (queue near capacity during TX) | Not tested |
| Concurrent host commands (multiple bytes in-flight) | Not tested |
| Response queueing (commands arriving faster than dispatched) | Not tested |
| Button TX echo (no UART response for button events) | Out of scope |
| Security / authentication | Not designed |
| Custom STM32F407 target portability | HOLD/DEFER (Phase 8A) |
| Robot Framework abstraction of producer/app pattern | Not promoted |
| Scheduler scalability at high event rate | Not tested |
| Real fault recovery | `active=0` throughout; no fault injection in Phase 9 |
| Product application (real actuator/sensor action) | Not implemented |

The most operationally relevant unknown is **high-rate UART input behavior**: the
one-byte-one-event model consumes one dispatch slot per 500 ms tick. A host
sending bytes faster than one per 500 ms will queue them; if the burst exceeds
queue capacity (16 events), events will be dropped and go unanswered. This is
expected behavior but is not yet characterized with UART-specific stress evidence.

---

## F. Architecture Alignment Verdict

**Verdict: ON_TRACK**

| Boundary | Expected state | Confirmed? |
| -------- | -------------- | ---------- |
| `core/` unchanged | No UART, no app-state, no Zephyr includes | ✓ |
| `platform/` unchanged | No UART abstraction added | ✓ |
| `tests/` unchanged | No test mutation | ✓ |
| UART TX devkit-local | `uart_poll_out()` in `devkit_uart_producer.c` only | ✓ |
| TX from thread context only | Handler runs from `robotos_core_tick()` | ✓ |
| No shell / parser / registry | Fixed switch/snprintf; no dynamic dispatch | ✓ |
| RTT canonical logging | `CONFIG_LOG_BACKEND_RTT=y`; `CONFIG_UART_CONSOLE=n` | ✓ |
| No telemetry-driven behavior | ROBOTOS_* lines are passive log output only | ✓ |
| Dispatch budget = 1 | `ROBOTOS_CORE_MAX_EVENTS_PER_TICK = 1` unchanged | ✓ |
| Queue capacity = 16 | `ROBOTOS_EVENT_QUEUE_CAPACITY = 16` unchanged | ✓ |
| No new event types | UART still USER+3; no Phase 9E event type | ✓ |
| No new Zephyr thread | Handler runs in main dispatch thread | ✓ |

One watchpoint that is non-blocking but should be tracked:

**W1 — TX polling blocks dispatch thread.** `uart_poll_out()` blocks the main
dispatch loop for the duration of the TX response (< 1 ms for current responses
at 115200 baud). Acceptable for low-rate demo commands. If future command rates
increase significantly or responses grow, this becomes a real constraint. The
current implementation is explicitly documented as demo-grade polling, not a
production TX path.

---

## G. Scheduler / Budget Decision

**Verdict: DEFER maintained. No workload-driven justification exists.**

Evidence from the full Phase 9A–9E stream:

| Session | Peak queue depth | Dropped | herr | Notes |
| ------- | ---------------- | ------- | ---- | ----- |
| Phase 9A-A | 14 | 13 | 0 | Bounce-induced; Phase 6I burst active |
| Phase 9A-C | 14 | 0 | 0 | Phase 6I gated off; Phase 6M + button |
| Phase 9C | 4 | 0 | 0 | Phase 6M + button(20) + UART(3) |
| Phase 9D | 16 | 3 | 0 | Operator-induced rapid button stress |
| Phase 9E | 2 | 0 | 0 | Phase 6M + 5 UART commands (low-rate) |

Phase 9E shows `peak=2` — the lowest queue pressure of any hardware session.
The Phase 9D `peak=16 / dropped=3` was entirely operator-induced (rapid manual
button pressing during boot-settle window), not steady-state workload pressure.
The system handled that saturation safely (`herr=0`, `unhandled=0`, CFSR/HFSR=0),
providing additional confidence that the queue saturation path is safe without
any scheduler change.

`ROBOTOS_CORE_MAX_EVENTS_PER_TICK = 1` is adequate for all observed workloads.
The correct re-evaluation trigger would be: sustained non-operator-induced queue
saturation in a real use-case workload (e.g. high-rate sensor polling, control
loop events, concurrent button + UART + sensor bursts). None of these have
appeared in Phase 9A–9E. Phases 7A and 7B-1 remain **DEFER**.

---

## H. UART TX Scope Guard

The following constraints **must remain frozen** for any future command-response
work unless a specific architectural decision explicitly overrides each one:

| Constraint | Reason |
| ---------- | ------ |
| No shell / prompt | No interactive session semantics; no command history |
| No parser framework | No command-table, no token lexer, no grammar |
| No command registry | No dynamic handler registration for commands |
| No multiline / framed protocol | No delimiter-based framing; no length prefix |
| No heap buffers | All response buffers on stack; fixed size (96 bytes current) |
| No response queue | No buffering of pending responses; TX is synchronous in handler |
| No UART logging replacement | RTT is and remains the canonical log backend |
| No core/platform UART abstraction | No `robotos_platform_uart` |
| No retry / ACK protocol | No acknowledgment, no sequence number, no error recovery |
| No TX from ISR context | `uart_poll_out()` only from handler (thread context) |
| No promotion to Robot Framework | `devkit_app_state` and UART response stay devkit-local |

**Safe next increments** (if user later approves, each individually):

- Add one or two additional single-byte commands (e.g. `l`/`L` = LED on,
  `0` = reset state) using the same switch/response pattern
- Improve `run_phase9e_uart_response_demo.ps1` to auto-verify expected
  substrings in responses (PASS/FAIL on transcript content)
- Add a docs-only command map for future reference
- Test line-ending behavior explicitly (`\n`, `\r\n`, `\r` variants)
- Characterize bounded burst behavior (send 3–4 bytes in rapid succession)

None of these require core/platform change. All are tooling or devkit-local.

---

## I. Risks and Watchpoints

| ID | Risk | Severity | Status |
| -- | ---- | -------- | ------ |
| P0 | `uart_poll_out()` blocks dispatch thread | **P0** — watch at higher TX rates | MONITOR — acceptable at current demo-grade rate |
| P1 | High-rate UART input not characterized | **P1** — one-byte-one-event model; queue overflow if burst > 16 bytes faster than 500 ms/tick | KNOWN — not a current workload; document before expanding command set |
| P1 | Phase 8A (F407 portability) untested | **P1** — 25-phase portability debt | HOLD/DEFER — not reopened |
| P2 | `devkit_app_state` stays devkit-local | **P2** — promotion to Robot Framework requires explicit decision | KNOWN — intentional; documented in Phase 9Z checkpoint |
| P2 | Dispatch budget stub | **P2** — `ROBOTOS_CORE_MAX_EVENTS_PER_TICK = 1` not a final scheduler design | MONITOR — no workload evidence justifies change |
| P2 | CP2102 USB adapter reliability | **P2** — observed "Device Descriptor Request Failed" during Phase 9E recovery | KNOWN — hardware issue; resolved by using mainboard USB port |

---

## J. Next Branch Options

### Option A — Checkpoint / Pause

Hold all runtime work. Wait for user to define product direction before any
further implementation.

- **Value:** Prevents scope creep; keeps architecture clean.
- **Risk:** May feel like stalled momentum.
- **Should be next?** Yes — this is the correct default.
- **Precondition:** None. Already at natural stopping point.

### Option B — Phase 9F: Demo Script Polish

Improve `run_phase9e_uart_response_demo.ps1` to automatically verify response
content (not just timeout/no-timeout). Add a PASS/FAIL checker that asserts each
response matches the expected string. **No firmware change.**

- **Value:** Makes the demo runbook testable; useful if phases become repetitive.
- **Risk:** Very low. Tooling only.
- **Should be next?** Good candidate if user wants a tighter demo workflow.
- **Precondition:** None. Can be done independently of direction decision.

### Option C — Phase 9G: Bounded UART Burst Characterization

Send 4–5 bytes in rapid succession (faster than one per 500 ms) and observe
queue behavior, drop count, and response order. Evidence-only or minimal tooling.
**No firmware change.**

- **Value:** Characterizes the one-byte-one-event model under mild stress.
  Closes the "high-rate UART input not tested" unknown from §E.
- **Risk:** Low (observation only). Could expose queue pressure that informs
  Phase 7B-1 justification.
- **Should be next?** Good candidate if user wants to understand UART burst limits.
- **Precondition:** None. Current firmware handles this today; just needs a test.

### Option D — Phase 10A: Product Command Set Planning (docs-only)

Define the intended real command set before implementing it. What commands
should the board respond to? What actuator/sensor actions? What response format?

- **Value:** Prevents ad-hoc command accumulation; gives Robot Framework
  promotion decision a concrete target.
- **Risk:** Very low. Docs only.
- **Should be next?** Yes — recommended if direction decision is "build
  product command set."
- **Precondition:** User must have a product application in mind.

### Option E — Phase 10B: Real Actuator / Sensor Action

Add a physical output behavior to the existing UART command loop (e.g. toggle
an LED in response to `l`/`L` command, or read the onboard accelerometer and
return a value).

- **Value:** Demonstrates that RobotOS can drive real physical output from a
  host command.
- **Risk:** Low for LED (already wired); moderate for sensor (new driver dependency).
- **Should be next?** Yes if user wants to demonstrate a real robot-control loop.
- **Precondition:** User approves; specific actuator/sensor defined.

### Option F — Phase 7B-1: Scheduler Test Parameterization

Add a host-test that exercises dispatch budget as a configurable parameter.

- **Value:** Enables future scheduler evolution to be tested without hardware.
- **Risk:** Low (host test only).
- **Should be next?** Not yet — no workload evidence demands it. Should follow
  a real workload that creates sustained queue pressure.
- **Precondition:** Workload evidence showing consistent `peak > budget`.

### Option G — Phase 8A: Custom STM32F407 Bring-up

Flash current firmware on a custom F407 target.

- **Value:** Retires 25-phase portability debt; validates hardware portability.
- **Risk:** Medium — requires F407 hardware, OpenOCD config, and portability
  analysis of devkit-local code.
- **Should be next?** Only if user reopens it explicitly.
- **Precondition:** User has F407 hardware and reopens Phase 8A.

---

## K. Recommended Next Move

### Primary Recommendation: Checkpoint and Await Direction

Commit this Phase 9E-Z checkpoint document. Do not implement any further runtime
phase until the user has defined the product application intent.

The Phase 9A–9E stream has proven all the foundational primitives:
real hardware input (button + UART), event queue/dispatch, app state machine,
and bidirectional command/response. The next increment should be driven by
**what the system should actually do**, not by what is technically adjacent.

### Alternative 1: Phase 9F Demo Script Polish (immediate, low-cost)

If the user wants tighter validation tooling before moving to product work,
Phase 9F can polish the `run_phase9e_uart_response_demo.ps1` script to
auto-verify expected response substrings. This is a one-session tooling task
with no firmware change and no product-direction dependency.

### Alternative 2: Phase 10A Product Command Set Planning (docs-only)

If the user has a product application in mind, document the intended command
set before implementing it. This anchors the next implementation phase to a
real product need rather than exploratory extension.

---

## L. Decisions Needed From User

Before any further runtime implementation phase is opened, the following
questions need user answers:

1. **Product application intent**: What should RobotOS actually do in the
   intended use case? (real robot control, monitoring, test fixture, demo,
   educational tool, other?)

2. **Command set definition**: What commands beyond `a`/`s`/`r`/`?` are
   needed? Should they remain single-byte or expand to multi-byte strings?

3. **Robot Framework promotion**: Should the producer/handler/app-state
   pattern be promoted from devkit-local to Robot Framework? When?

4. **Phase 8A (F407) priority**: Is the custom STM32F407 board a priority,
   or is single-board (F411E-DISCO) adequate for the current direction?

5. **Demo script polish**: Is the current Phase 9E demo script `run_phase9e_uart_response_demo.ps1`
   adequate, or is automated response verification (Phase 9F) worth doing next?

---

## M. Suggested Follow-up Commit

This checkpoint document should be committed as a docs-only update:

```text
Suggested commit message:
docs: Phase 9E-Z command-loop checkpoint / direction guard
```

Files to commit:
- `RobotOS_v1.0/devkit/docs/PHASE_9EZ_CHECKPOINT.md` (this file — new)
- `RobotOS_v1.0/devkit/docs/DEVKIT_PROGRESS.md` (Phase 9E-Z index row + section)
- `CURRENT_STATE.md` (last checkpoint pointer to this document)

No source, test, runtime, CMake, Kconfig, or evidence log change in this commit.

---

## Cross-references

- Phase 9Z workload checkpoint: [`PHASE_9Z_CHECKPOINT.md`](PHASE_9Z_CHECKPOINT.md)
- Phase history (full): [`DEVKIT_PROGRESS.md`](../01_PROGRESS/DEVKIT_PROGRESS.md)
- Phase 9E RTT log: [`../logs/phase_9E_uart_tx_response_rtt_2026-05-09.txt`](../../logs/phase_9E_uart_tx_response_rtt_2026-05-09.txt)
- Phase 9E host transcript: [`../logs/phase_9E_uart_tx_response_host_2026-05-09.txt`](../../logs/phase_9E_uart_tx_response_host_2026-05-09.txt)
- Workload demo runbook: [`WORKLOAD_DEMO_9D.md`](../03_SPECS/WORKLOAD_DEMO_9D.md)
- Telemetry reference: [`TELEMETRY_REFERENCE.md`](../03_SPECS/TELEMETRY_REFERENCE.md)
- Live phase state: [`../../../CURRENT_STATE.md`](../../../../CURRENT_STATE.md)
- Runtime tools: [`../../tools/runtime/README.md`](../../tools/runtime/README.md)
