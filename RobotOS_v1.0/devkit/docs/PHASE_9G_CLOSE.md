# Phase 9G — Bounded UART Burst Characterization (Close)

**Type:** Evidence/tooling phase. Host-side script + docs only at this
commit. No firmware, core, platform, test, CMake, Zephyr, board, or runtime
script change.
**Status:** `OPEN_HARDWARE_VALIDATION_PENDING`
**Date opened:** 2026-05-11
**Branch:** master
**Prior runtime checkpoint:** Phase 9E-Z (`30bae27`). Runtime baseline still
ends at Phase 9E (`587dab7`).
**Companion docs:**
[`DEVKIT_PROGRESS_PHASE_10.md`](DEVKIT_PROGRESS_PHASE_10.md) Phase 9G entry,
[`PHASE_9EZ_CHECKPOINT.md`](PHASE_9EZ_CHECKPOINT.md) section J Option C
(the original deferred Phase 9G candidate),
[`COMMAND_SET_DRAFT.md`](COMMAND_SET_DRAFT.md) section 5 (Phase 9G named as a
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

This commit lands **tooling and docs only** under
`OPEN_HARDWARE_VALIDATION_PENDING`. Hardware evidence is **not** captured at
this commit because no STM32F411E-DISCO board is available to the authoring
environment. An operator with a wired board can run the harness in a
follow-up session and amend this document with the captured evidence.

The phase is **not** marked `CLOSED` until hardware evidence is captured per
the validation checklist in section F.

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

## E. Findings (record only — do NOT "fix" in this phase)

This section is intentionally empty pending hardware run. The harness
itself enumerates the four classes of finding to record (see the
"Phase 9G findings" subsection of its verification checklist):

1. If `peak` > Phase 9E `peak=2`, record as positive evidence for a
   future scheduler discussion. Not a regression; not a trigger to mutate
   `ROBOTOS_CORE_MAX_EVENTS_PER_TICK`.
2. If `dropped > 0`, record as Phase 9G finding for the scheduler /
   queue-capacity decision gate. Phase 9G **does not** change budget,
   capacity, admission, throttle, retry, or dispatch policy.
3. If response ordering does not match send ordering, record as Phase 9G
   finding for the UART decision gate. Phase 9G **does not** patch
   `uart_poll_out()`, the handler, or the TX path.
4. If `RECV_COUNT < BURST_COUNT`, record as Phase 9G finding. Phase 9G
   **does not** introduce a retry/ACK protocol, response queue, or
   parser/framing change.

A future amendment to this document should add a "Findings" table
under this section with `metric | expected | observed | classification`
rows, plus the RTT log path under section H.

---

## F. Hardware validation checklist (when board is available)

Operator steps:

1. Wire STM32F411E-DISCO USART2 (PA2 TX / PA3 RX / GND) to a USB-UART
   adapter. Confirm CP210x (or equivalent) at known COM port.
2. From repo root, capture/flash the current Phase 9E firmware using the
   `capture_devkit_rtt.ps1`-discipline. Manual RESET as fallback if the
   harness `reset run` fails; plain `west flash` alone is not runtime-start
   evidence.
3. Run:
   ```powershell
   .\RobotOS_v1.0\tools\runtime\run_phase9g_uart_burst_demo.ps1 -ComPort COMn
   ```
4. Save the host transcript and RTT log paths reported by the harness.
5. Verify the host-side checklist printed by the harness (RECV_COUNT,
   ordering, response-arrival cadence).
6. Verify the RTT-side checklist printed by the harness (ROBOTOS_UART,
   ROBOTOS_APP, ROBOTOS_OBS, CFSR/HFSR, Phase 6M producer).
7. Amend section E ("Findings") of this document with a row per metric.
8. Amend the "Date closed" line of this document and flip status from
   `OPEN_HARDWARE_VALIDATION_PENDING` to `CLOSED`.
9. Add a log-index row to `RobotOS_v1.0/devkit/logs/INDEX.md`.
10. Advance `CURRENT_STATE.md` "Last Closed Phase" to Phase 9G.

Until steps 4–10 are completed, Phase 9G remains
`OPEN_HARDWARE_VALIDATION_PENDING`.

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

## H. Evidence artifacts (populated by future hardware run)

- Host transcript: `RobotOS_v1.0/devkit/logs/phase_9G_uart_burst_host_<date>.txt`
  *(pending)*
- RTT log: `RobotOS_v1.0/devkit/logs/phase_9G_uart_burst_rtt_<date>.txt`
  *(pending)*
- Log index row in `RobotOS_v1.0/devkit/logs/INDEX.md` *(pending)*

This document will be amended with concrete paths and findings after the
hardware run completes.

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

Once the operator completes the hardware checklist in section F:

- **If predictions in section D match observed behavior:** Phase 9G can be
  closed; safe to proceed to a Phase 10B candidate selection from
  `COMMAND_SET_DRAFT.md` section B (or to remain on hold). No scheduler
  reopening is justified.
- **If `peak > 2` but still `< queue capacity` and `dropped == 0`:** record
  as informative evidence; Scheduler 7A / 7B-1 stay DEFER. Phase 10B is
  safe to consider.
- **If `dropped > 0`:** Phase 9G remains open as a finding for the
  scheduler decision gate (Phase 7B-1 reopening candidate). Phase 10B
  should be held until the scheduler question is addressed.
- **If response ordering or count is wrong:** Phase 9G surfaces a UART
  finding for the UART decision gate. Phase 10B (especially Phase 10B-`L`
  / -`T` with physical effects) must be held until the UART path is
  reviewed; Phase 10B-`v` may still be safe.

Phase 9G does not authorize any of the above by itself.

---

## K. Cross-references

- Phase 9E-Z direction guard: [`PHASE_9EZ_CHECKPOINT.md`](PHASE_9EZ_CHECKPOINT.md)
- Phase history (Phase 9E and earlier): [`DEVKIT_PROGRESS.md`](DEVKIT_PROGRESS.md)
- Phase 10+ log: [`DEVKIT_PROGRESS_PHASE_10.md`](DEVKIT_PROGRESS_PHASE_10.md)
- Phase 10A planning: [`PHASE_10A_CLOSE.md`](PHASE_10A_CLOSE.md)
- DRAFT command vocabulary: [`COMMAND_SET_DRAFT.md`](COMMAND_SET_DRAFT.md)
- Live state snapshot: [`../../../CURRENT_STATE.md`](../../../CURRENT_STATE.md)
- Telemetry reference: [`TELEMETRY_REFERENCE.md`](TELEMETRY_REFERENCE.md)
- Runtime tools: [`../../tools/runtime/README.md`](../../tools/runtime/README.md)
