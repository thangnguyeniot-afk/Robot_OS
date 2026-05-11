# Phase 9G — Bounded UART Burst Characterization (Close)

**Type:** Evidence/tooling phase. Host-side script + docs only. No firmware,
core, platform, test, CMake, Zephyr, board, or runtime-behavior change.
**Status:** `CLOSED_WITH_HARDWARE_EVIDENCE`
**Date opened:** 2026-05-11
**Date closed:** 2026-05-11 (same-day hardware run on STM32F411E-DISCO)
**Branch:** master
**Prior runtime checkpoint:** Phase 9E-Z (`30bae27`). Runtime baseline still
ends at Phase 9E (`587dab7`); Phase 9G adds characterization evidence only,
no runtime change.
**Companion docs:**
[`DEVKIT_PROGRESS_PHASE_10.md`](../01_PROGRESS/DEVKIT_PROGRESS_PHASE_10.md) Phase 9G entry,
[`PHASE_9EZ_CHECKPOINT.md`](PHASE_9EZ_CHECKPOINT.md) section J Option C
(the original deferred Phase 9G candidate),
[`COMMAND_SET_DRAFT.md`](../03_SPECS/COMMAND_SET_DRAFT.md) section 5 (Phase 9G named as a
direction-independent supporting phase).

---

## A. Purpose

Phase 9G characterizes **bounded UART input burst** behavior on the proven
Phase 9E command/response loop. It is the smallest evidence-only phase that
closes the highest-priority outstanding unknown from `PHASE_9EZ_CHECKPOINT.md
§E`: *high-rate UART input not stress-tested*.

Phase 9G is **not** a firmware change, scheduler change, queue-capacity
change, command-set expansion, parser/shell/protocol addition, or F407 work.
It runs the existing Phase 9E command vocabulary (`a`, `s`, `?`, `r`, `x`)
through a tighter inter-byte window so that several UART RX events are
queued before the dispatcher drains them.

---

## B. Scope classification

Phase 9G is closed with hardware evidence. Two commits land its content:

- `e9a1d62` — `tools: add Phase 9G UART burst characterization harness`
  (host-side script + docs; phase opened as `OPEN_HARDWARE_VALIDATION_PENDING`).
- (this commit) — `docs: close Phase 9G UART burst characterization with
  hardware evidence` (evidence ingestion; phase flipped to
  `CLOSED_WITH_HARDWARE_EVIDENCE`).

Hardware run was executed on STM32F411E-DISCO via ST-LINK V2 (firmware
V2J47S0) + Silicon Labs CP210x USB-UART adapter on COM5, using the existing
Phase 9E firmware build (`build/zephyr/zephyr.elf`, ELF modtime 2026-05-09
matching commit `587dab7`). Flashing used `west flash` with the runner's
own `reset run`; runtime-start observation used the Phase 6O
`capture_devkit_rtt.ps1` sidecar `reset run` discipline.

---

## C. Burst scenario

The harness `RobotOS_v1.0/tools/runtime/run_phase9g_uart_burst_demo.ps1`
implements the canonical scenario:

| Knob | Default | Why |
|---|---|---|
| Sequence | `a s ? r x` (5 single bytes) | Reuses the Phase 9E vocabulary so transitions, query, and negative-path probe all exercise during the burst. |
| `BurstSpacingMs` | 30 ms between bytes | Total burst width ~120 ms (4 inter-byte gaps × 30 ms), well below the 500 ms dispatch tick. Forces >=2 UART events to be queued at some point. |
| `ReadWindowMs` | 5000 ms | One response per tick * 5 bytes = ~2.5 s; 5 s provides margin without being stress-to-failure. |
| `ReadTimeoutMs` | 6000 ms (per-read cap) | Inner per-`ReadLine` timeout; outer window still controls when the harness stops. |
| `CaptureSeconds` | 60 s | Same as Phase 9E; long enough to capture boot baseline + burst + drain + post-drain telemetry. |
| `WaitBeforeInputSeconds` | 15 s | Same as Phase 9E; board boot + IDLE baseline. |
| RTT harness | `capture_devkit_rtt.ps1` (sidecar `reset run`) | Phase 6O mitigation discipline; manual RESET retained as fallback; plain `west flash` alone is not runtime-start evidence. |

The burst is **bounded**: 5 bytes is exactly the queue-population a single
Phase 9E vocabulary cycle produces. It is not a stress-to-failure run.

---

## D. Expected behavior (predictions, to be confirmed by hardware run)

Under the published Phase 9E baseline (`ROBOTOS_CORE_MAX_EVENTS_PER_TICK = 1`,
`ROBOTOS_EVENT_QUEUE_CAPACITY = 16`, one-byte-one-event UART model):

- **Host transcript:** 5 RECV lines in send order; first arrives ~500 ms
  after burst-start; subsequent at ~500 ms intervals. Final RECV ~2.5 s
  after burst-start.
- **ROBOTOS_UART:** `rx == 5`, `ok == 5`, `full == 0`, `handled == 5`.
- **ROBOTOS_APP:** `transitions == 3` (a, s, r), `uart == 5`, `ignored == 1`
  (x), `last_byte == 0x78`, final `state == IDLE`.
- **ROBOTOS_OBS:** `peak >= 2` at some point during the burst (>=2 UART
  events queued before dispatcher drains them); `dropped == 0` (5-byte
  burst is well below the 16-slot queue capacity); `accepted - dispatched
  = pending` invariant holds; `herr == 0`, `unhandled == 0`, `rejected == 0`,
  `throttled == 0`.
- **CFSR / HFSR:** `0x00000000` throughout (consistent with every Phase 9
  session to date).
- **Phase 6M producer:** healthy across the full 60 s capture window.

Predictions are **conservative**. The Phase 9D operator-induced session
already demonstrated the queue safely absorbing `peak=16 dropped=3 herr=0`
without firmware fault; Phase 9G's bounded 5-byte burst is much smaller.

---

## E. Findings (record only -- no fix in Phase 9G)

### E.1 Queue-pressure finding (positive characterization, no regression)

`ROBOTOS_OBS peak` rose from Phase 9E's `peak=2` to Phase 9G's `peak=5`.
This is the **designed** outcome of the 30 ms-spaced burst: 5 UART events
queued before the dispatcher (`MAX_EVENTS_PER_TICK = 1`, ~500 ms tick) had
drained the first. The queue safely held all 5, plus a concurrent Phase 6M
producer event, with `dropped=0`, `herr=0`, `unhandled=0`.

Classification: **positive characterization evidence**. Not a regression.
Not a trigger to mutate `ROBOTOS_CORE_MAX_EVENTS_PER_TICK` or
`ROBOTOS_EVENT_QUEUE_CAPACITY`. Scheduler 7A / 7B-1 remain DEFER.

### E.2 Inter-response timing observation (expected, not a finding to fix)

Inter-response wallclock intervals in firmware time:

| Response | Cmd | Firmware time | Δ vs prior |
|---|---|---|---|
| 1 | 0x61 `a` | 00:13.505 | -- |
| 2 | 0x73 `s` | 00:14.006 | 501 ms (~1 tick) |
| 3 | 0x3f `?` | 00:14.511 | 505 ms (~1 tick) |
| 4 | 0x72 `r` | 00:15.513 | 1002 ms (~2 ticks) |
| 5 | 0x78 `x` | 00:16.016 | 503 ms (~1 tick) |

The 1002 ms gap between responses 3 and 4 is consistent with the
`MAX_EVENTS_PER_TICK = 1` model: a concurrent Phase 6M producer event
consumed that tick's dispatch slot, deferring the next UART event by one
tick. `accepted=65 = PROD ok(60) + UART ok(5)` confirms the interleaving.

Classification: **expected behavior of budget=1 dispatch under mixed
event sources**. Documents the cost: with 5 UART events plus one
producer event per 2 ticks, drain of a 5-byte UART burst takes ~5-6
ticks rather than exactly 5. No fix required; no UART-path patch
permitted in this phase regardless.

### E.3 Response ordering, count, faults

All match expectations:

- Response ordering preserved: `0x61 → 0x73 → 0x3f → 0x72 → 0x78`,
  matching the burst send order exactly. The board's app state at
  query time was `ACTIVE transitions=2 button=0 uart=3 ignored=0`,
  meaning a (transition 1), s (transition 2), and ? itself (uart=3) had
  all been processed before the query response went out -- expected.
- `RECV_COUNT == BURST_COUNT == 5`. No retry/ACK/response-queue need.
- CFSR/HFSR `0x00000000` for all 13 ROBOTOS_FAULT emissions in the
  60 s window; `active=0` throughout.

No finding requires a Phase 9G-internal fix. No scope-guard constraint
was crossed.

---

## F. Hardware validation executed

Hardware run completed 2026-05-11 17:58 local. Steps actually taken:

1. **Wiring** (pre-existing from Phase 9B/9C/9E): STM32F411E-DISCO USART2
   PA2 → CP210x USB-UART RX, PA3 → CP210x USB-UART TX, common GND.
   USB-UART enumerated as COM5 (Silicon Labs CP210x).
2. **Tool verification:** ST-LINK V2 (V2J47S0) on USB; OpenOCD xpack
   0.12.0+dev-02228-ge5888bda3-dirty resolved on PATH; west 1.5.0
   resolved on PATH. `arm-zephyr-eabi-nm` not on PATH; the capture
   harness's RAM-search fallback was not exercised because `nm`
   resolved `_SEGGER_RTT` from the Phase 9E ELF directly at `0x20000a5c`.
3. **Flash:** `west flash --build-dir D:\Robot_OS\build`. The existing
   build at `D:\Robot_OS\build` was confirmed Phase 9E-aligned before
   flashing: `BOARD=stm32f411e_disco`, `APPLICATION_SOURCE_DIR=
   D:/Robot_OS/RobotOS_v1.0/devkit`, ELF modtime 2026-05-09 matching
   Phase 9E commit `587dab7`, ELF strings include `devkit_uart_emit_tx_response`,
   `Phase 9E UART response sent cmd=...`, and `ERR ignored byte=...`.
   OpenOCD wrote 49152 bytes in 1.561 s; exit 0; clean shutdown.
4. **Burst demo execution:** `RobotOS_v1.0\tools\runtime\run_phase9g_uart_burst_demo.ps1 -ComPort COM5`
   with all defaults (sequence `a s ? r x`, BurstSpacingMs=30,
   ReadWindowMs=5000, CaptureSeconds=60, WaitBeforeInputSeconds=15).
   The harness launched `capture_devkit_rtt.ps1` in a background job
   (sidecar OpenOCD with `init; reset run; rtt setup 0x20000a5c 64 "SEGGER RTT";
   rtt start; rtt server start 9090 0`); waited 15 s for boot; sent the
   burst over 185 ms wallclock (3a phase); read responses for 5018 ms
   (3b phase); waited for the RTT capture job to complete (60.7 s total
   stream window, 22929 bytes).
5. **Harness verdict:** PASS. All 5 required patterns FOUND
   (`ROBOTOS_OBS state=READY`, `ROBOTOS_FAULT active=0`,
   `ROBOTOS_PROD attempted=`, `ROBOTOS_UART`, `ROBOTOS_APP`);
   CFSR/HFSR `0x00000000` for all 13 occurrences.
6. **Evidence files saved:**
   - Host transcript: `RobotOS_v1.0/devkit/logs/phase_9G_uart_burst_host_2026-05-11.txt`
   - RTT log: `RobotOS_v1.0/devkit/logs/phase_9G_uart_burst_rtt_2026-05-11.txt`
7. **POST_FLASH_AUTOSTART status:** As is normal for cold flash, the
   initial OpenOCD examination from the `west flash` invocation hit
   `Failed to read memory at 0xe000ed04` once, then the subsequent
   examination succeeded; firmware was loaded and shutdown was clean.
   Runtime-start observation (RTT stream + responses) was driven by the
   Phase 6O `capture_devkit_rtt.ps1` sidecar `reset run` -- the
   documented mitigation. **Manual RESET was not required this session.**
   Plain `west flash` alone was, per discipline, **not** treated as
   runtime-start evidence; the harness's RTT capture is the
   authoritative runtime-start signal.

All checklist items completed. Phase 9G status flipped from
`OPEN_HARDWARE_VALIDATION_PENDING` to `CLOSED_WITH_HARDWARE_EVIDENCE` in
this commit.

---

## G. Architecture preservation (Phase 9G tooling/docs commit)

Confirmed unchanged at this commit:

- `core/` — zero diff
- `platform/` — zero diff
- `tests/` — zero diff
- `devkit/src/` — zero diff
- `devkit/CMakeLists.txt`, `devkit/prj.conf` — zero diff
- Existing runtime tools (`capture_devkit_rtt.ps1`,
  `capture_phase6h_runtime.ps1`, `phase6h_read.gdb`,
  `phase6z_required_patterns.txt`, `run_phase9d_demo.ps1`,
  `run_phase9e_uart_response_demo.ps1`) — **zero diff**. Phase 9G adds a
  new sibling script only.
- Scheduler constants (`ROBOTOS_CORE_MAX_EVENTS_PER_TICK = 1`,
  `ROBOTOS_EVENT_QUEUE_CAPACITY = 16`) — unchanged
- Event-type contract — unchanged
- UART TX scope guard (12 constraints, `PHASE_9EZ_CHECKPOINT.md` section H)
  — intact. Phase 9G sends raw single bytes through the existing
  fixed switch/snprintf path; no parser, no shell, no registry, no framing,
  no response queue, no heap, no ISR-context TX, no core/platform UART
  abstraction.
- POST_FLASH_AUTOSTART discipline — unchanged; root cause OPEN;
  mitigated-by-workflow from Phase 6O onward; manual RESET retained as
  fallback; plain `west flash` alone is not runtime-start evidence.
- RTT canonical log backend — unchanged.

`DEVKIT_PROGRESS.md` is **not** modified by Phase 9G. The Phase 9G entry
lands in `DEVKIT_PROGRESS_PHASE_10.md` per the §4 rule-6 non-linear-insert
convention; the historical master remains untouched.

---

## H. Evidence artifacts

- Host transcript: [`../logs/phase_9G_uart_burst_host_2026-05-11.txt`](../../logs/phase_9G_uart_burst_host_2026-05-11.txt)
  -- SEND_BURST t/idx/byte rows + RECV t/idx/line rows + BURST/READ/RECV
  count totals.
- RTT log: [`../logs/phase_9G_uart_burst_rtt_2026-05-11.txt`](../../logs/phase_9G_uart_burst_rtt_2026-05-11.txt)
  -- 22929 bytes, 60.7 s stream window.
- Log index row in [`../logs/INDEX.md`](../../logs/INDEX.md) -- added in this
  commit.

### H.1 Selected RTT lines (final state, ticks=120)

```text
ROBOTOS_OBS state=READY ticks=120 pending=1 peak=5 dropped=0
            dispatched=64 herr=0 throttled=0 rejected=0 accepted=65
            unhandled=0 bp=0 th_active=0
ROBOTOS_UART rx=5 ok=5 full=0 invalid=0 other=0 handled=5 last=0x78 type=USER+3
ROBOTOS_APP  state=IDLE transitions=3 button=0 uart=5 ignored=1
             last_src=UART last_byte=0x78
ROBOTOS_FAULT active=0 cfsr=0x00000000 hfsr=0x00000000 context=none
ROBOTOS_PROD attempted=60 ok=60 throttled=0 dropped=0 invalid=0 other=0 type=USER+1
```

### H.2 Selected RTT lines (per-command response trace)

```text
00:13.505  Phase 9C app transition: IDLE->ARMED   src=UART byte=0x61
00:13.505  Phase 9E UART response sent cmd=0x61 len=16 state=ARMED
00:14.005  Phase 9C app transition: ARMED->ACTIVE src=UART byte=0x73
00:14.006  Phase 9E UART response sent cmd=0x73 len=17 state=ACTIVE
00:14.506  Phase 9C app query:                    state=ACTIVE transitions=2
00:14.511  Phase 9E UART response sent cmd=0x3f len=60 state=ACTIVE
00:15.512  Phase 9C app transition: ACTIVE->IDLE  src=UART byte=0x72
00:15.513  Phase 9E UART response sent cmd=0x72 len=15 state=IDLE
00:16.016  Phase 9E UART response sent cmd=0x78 len=34 state=IDLE
```

(For `0x78`, no app transition fires -- `ignored` increments instead and
the response is the `ERR ignored byte=0x78 state=IDLE` line. Three real
transitions in total: a/s/r. The query `?` does not transition.)

---

## I. Explicit confirmations (this commit)

- **No core/platform mutation.** Verified via `git show` filename list.
- **No scheduler mutation.** `ROBOTOS_CORE_MAX_EVENTS_PER_TICK = 1` and
  `ROBOTOS_EVENT_QUEUE_CAPACITY = 16` are unchanged. Phase 7A / 7B-1
  remain DEFER.
- **No F407 / custom-board work.** Phase 8A remains HOLD/DEFER.
- **No UART TX scope expansion.** UART TX remains minimal response only;
  the 12 scope-guard constraints are intact.
- **No Phase 10B implementation.** No `devkit/src/` change. No command
  vocabulary expansion. `COMMAND_SET_DRAFT.md` section B rows are still
  `USER_DECISION_REQUIRED`.
- **POST_FLASH_AUTOSTART discipline preserved.** Root cause OPEN;
  mitigated-by-workflow from Phase 6O onward via `capture_devkit_rtt.ps1`
  sidecar `reset run`. Manual RESET retained as fallback. Plain
  `west flash` alone is not runtime-start evidence.
- **Phase 9E-Z remains the last runtime checkpoint.** Phase 9G is a
  characterization phase; runtime behavior is identical to Phase 9E
  (`587dab7`).
- **`DEVKIT_PROGRESS.md` not modified.** Phase 9G entry lands in
  `DEVKIT_PROGRESS_PHASE_10.md` per §4 rule-6 non-linear-insert convention.

---

## J. Recommendation after hardware run

The hardware run observed `peak=5 > Phase 9E peak=2` AND `dropped == 0` AND
ordering preserved AND `RECV_COUNT == BURST_COUNT` AND CFSR/HFSR/active all
clean. This is the **first** clause of the Phase 9G recommendation matrix
(see commit `e9a1d62` opening text): "safe to proceed to a Phase 10B
candidate selection from `COMMAND_SET_DRAFT.md` section B (or to remain on
hold). No scheduler reopening is justified."

Concrete recommendation:

- **Scheduler 7A / 7B-1:** remains DEFER. `peak=5` is well below queue
  capacity (16), with concurrent Phase 6M producer events successfully
  interleaved. No workload-driven justification for budget mutation.
- **Phase 8A (F407):** remains HOLD/DEFER. Phase 9G did not exercise the
  F407 target.
- **UART TX path:** unchanged. Response ordering preserved; no patch needed.
- **Phase 10B-`v` (build query):** **safe to consider.** Smallest surface;
  no driver dependency; no interaction with the burst behavior characterized
  here. User decision still required per `COMMAND_SET_DRAFT.md` section 3.
- **Phase 10B-`L` (LED toggle):** **safe to consider.** First physical-effect
  command; the queue/dispatch path is now characterized under a 5-event
  burst with `dropped=0`. User decision still required per
  `COMMAND_SET_DRAFT.md` section 3 (LED-blink semantics reconciliation).
- **Phase 10B-`d` / `T`:** unchanged risk profile; user decision still
  required.
- **Continued hold:** equally defensible. Phase 9G does not pressure any
  Phase 10B opening; it only removes a blocking unknown.

Phase 9G does not authorize any of the above by itself.

---

## K. Cross-references

- Phase 9E-Z direction guard: [`PHASE_9EZ_CHECKPOINT.md`](PHASE_9EZ_CHECKPOINT.md)
- Phase history (Phase 9E and earlier): [`DEVKIT_PROGRESS.md`](../01_PROGRESS/DEVKIT_PROGRESS.md)
- Phase 10+ log: [`DEVKIT_PROGRESS_PHASE_10.md`](../01_PROGRESS/DEVKIT_PROGRESS_PHASE_10.md)
- Phase 10A planning: [`PHASE_10A_CLOSE.md`](PHASE_10A_CLOSE.md)
- DRAFT command vocabulary: [`COMMAND_SET_DRAFT.md`](../03_SPECS/COMMAND_SET_DRAFT.md)
- Live state snapshot: [`../../../CURRENT_STATE.md`](../../../../CURRENT_STATE.md)
- Telemetry reference: [`TELEMETRY_REFERENCE.md`](../03_SPECS/TELEMETRY_REFERENCE.md)
- Runtime tools: [`../../tools/runtime/README.md`](../../tools/runtime/README.md)
