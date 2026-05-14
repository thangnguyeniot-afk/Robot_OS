# Phase 12M-pre — Probe Translator Hardware Validation Plan

**Status:** `CLOSED_DOCS_ONLY`
**Decision:** `PHASE_12M_PRE_PROBE_TRANSLATOR_HARDWARE_VALIDATION_PLAN_CLOSED`
**Date:** 2026-05-14
**Type:** Docs-only hardware validation planning gate.
**Prior phase anchor:** [`PHASE_12LZ_RUNTIME_ADMISSION_CHECKPOINT.md`](PHASE_12LZ_RUNTIME_ADMISSION_CHECKPOINT.md)
**Phase 12L anchor:** [`PHASE_12L_PROBE_TRANSLATOR_RUNTIME_ADMISSION_ADAPTER.md`](PHASE_12L_PROBE_TRANSLATOR_RUNTIME_ADMISSION_ADAPTER.md)
**Implementation contract (produced here):** [`../03_SPECS/PROBE_TRANSLATOR_HARDWARE_VALIDATION_PLAN.md`](../03_SPECS/PROBE_TRANSLATOR_HARDWARE_VALIDATION_PLAN.md)

---

## A. Executive Summary

Phase 12M-pre is a **docs-only planning gate**. It audits the repo
hardware validation convention, defines the exact expected RTT/log
patterns for the Phase 12L runtime adapter on real hardware, and
produces the execution-ready Phase 12M hardware validation contract.

**What Phase 12M-pre does:**

- Confirms Phase 12L-Z guard is in place.
- Audits existing hardware validation tooling and conventions.
- Defines the exact flash procedure, RTT capture procedure, UART
  command sequence, expected log patterns, pass/fail criteria,
  evidence file paths, and rollback conditions for Phase 12M.
- Produces a long-lived implementation spec:
  `PROBE_TRANSLATOR_HARDWARE_VALIDATION_PLAN.md`.

**What Phase 12M-pre does not do:**

- Does not flash the board.
- Does not capture RTT.
- Does not run J-Link, OpenOCD, or any hardware session.
- Does not create new PowerShell scripts (Phase 12M creates the script).
- Does not modify any source, header, CMake, Kconfig, `prj.conf`,
  DTS, overlay, test, tool, or log file.
- Does not claim hardware validation.

**Decision:** `PHASE_12M_PRE_PROBE_TRANSLATOR_HARDWARE_VALIDATION_PLAN_CLOSED`

---

## B. Baseline From Phase 12L-Z

| Item | Value |
| --- | --- |
| HEAD at Phase 12M-pre open | `4b66487 docs: add Phase 12L-Z runtime admission checkpoint` |
| `origin/master` | `4b66487` (synced) |
| Phase 12L implementation baseline | `31968ad feat: add devkit probe translator runtime adapter` |
| Phase 12L-Z guard confirmed | `PHASE_12L_Z_RUNTIME_ADMISSION_CHECKPOINT_CLOSED` |
| Phase 12L build transcript | `RobotOS_v1.0/devkit/logs/phase_12L_build_2026-05-14.txt` |
| Phase 12L Zephyr image | FLASH 43,384 B (8.27%); RAM 12,480 B (9.52%) |
| Host regression | 23/23 PASS |
| Hardware behavior proven | **NONE** — Phase 12L is build-depth only |

---

## C. Hardware Validation Objective

Prove that the Phase 12L `devkit_probe_adapter` runtime wiring
behaves correctly on the physical `stm32f411e_disco` board:

1. `devkit_probe_adapter_init()` succeeds at boot and emits the expected
   `ROBOTOS_PROBE init ok` log line.
2. The periodic baseline snapshot is emitted with expected field values
   (state=1=IDLE, counters at zero before any command).
3. UART command 'a' (arm) triggers `devkit_app_state` IDLE→ARMED
   transition AND drives the probe_translator to READY (state=2,
   trans=1, mapped=1).
4. UART command 's' (start) triggers ARMED→ACTIVE transition AND
   drives probe_translator to ACTIVE (state=3, trans=2, mapped=2).
5. UART command 'r' (reset) triggers ACTIVE→IDLE transition AND
   resets probe_translator to IDLE (state=1, trans=3, mapped=3).
6. Existing UART TX responses (`OK state=ARMED`, `OK state=ACTIVE`,
   `OK state=IDLE`) are **unchanged**.
7. No crash, hardfault, assert, or scheduler regression appears.
8. CFSR=0x00000000 and HFSR=0x00000000 throughout.

---

## D. Tooling / Environment Assumptions

### D.1 Board

| Item | Value |
| --- | --- |
| Board | STM32F411E-DISCO (rev D) |
| MCU | STM32F411VET6 |
| On-board programmer | ST-LINK/V2-A (SWD, USB connector CN1) |
| UART RX | PA3 (USART2 RX) — via external USB-UART adapter |
| UART TX | PA2 (USART2 TX) — via external USB-UART adapter |
| Baud rate | 115200 8N1 |
| COM port | COM5 (CP210x USB-UART adapter — verify before Phase 12M run) |

### D.2 Firmware

| Item | Value |
| --- | --- |
| Firmware source | `RobotOS_v1.0/devkit` at commit `31968ad` (Phase 12L) |
| Build command | `py -m west build --pristine=always -d build-phase12m -b stm32f411e_disco RobotOS_v1.0/devkit` |
| Build directory | `build-phase12m` (new, pristine) |
| Expected FLASH | ~43,384 B (Phase 12L baseline; small variation possible if rebuilt) |
| Zephyr version | v3.6.0 |
| SDK | Zephyr SDK 0.17.0; arm-zephyr-eabi-gcc 12.2.0 |

### D.3 Flash method (per repo convention)

```powershell
py -m west flash -d build-phase12m
```

West calls ST-LINK via the Zephyr OpenOCD runner. Standard single
command; no J-Link configuration required. Probe: ST-LINK/V2-A
on-board.

### D.4 RTT capture method (per repo convention)

```powershell
.\RobotOS_v1.0\tools\runtime\capture_devkit_rtt.ps1 `
    -OutputLog RobotOS_v1.0/devkit/logs/phase_12M_rtt_2026-05-14.txt `
    -WaitSeconds 90 `
    -RequirePatterns @(...)
```

Invoked via the per-phase demo script (see §G). RTT block address
`_SEGGER_RTT` must be confirmed for the Phase 12M firmware image
before capture. In Phase 11E this was `0x20000ad0`; Phase 12L adds
~128 B RAM, so the address may shift. Get the address from the Phase
12M build map:

```powershell
arm-zephyr-eabi-nm build-phase12m/zephyr/zephyr.elf | `
    Select-String "_SEGGER_RTT"
```

### D.5 POST_FLASH_AUTOSTART (per repo convention)

Root cause is `OPEN`; `MITIGATED_BY_WORKFLOW` from Phase 6O onward.
The Phase 6O `capture_devkit_rtt.ps1` sidecar issues `reset run`
after attaching OpenOCD, which starts the firmware reliably. The
demo script must use this sidecar. Manual RESET is a fallback only;
plain `west flash` alone is not sufficient runtime-start evidence.

### D.6 UART wiring

```
board PA3 (USART2 RX) <-- USB-UART TX
board PA2 (USART2 TX) --> USB-UART RX
board GND             --- USB-UART GND
```

Both RX and TX must be wired to observe UART TX responses for
pass/fail verification.

---

## E. Existing Script / Procedure Reuse Audit

### E.1 Found tools (repo-confirmed at Phase 12M-pre)

| Script | Path | Purpose |
| --- | --- | --- |
| `capture_devkit_rtt.ps1` | `RobotOS_v1.0/tools/runtime/` | Phase 6O RTT capture harness; sidecar `reset run`; `-RequirePatterns` list |
| `run_phase9d_demo.ps1` | `RobotOS_v1.0/tools/runtime/` | Multi-source UART+button workload demo (closest analog; sequence `a s r`) |
| `run_phase9e_uart_response_demo.ps1` | `RobotOS_v1.0/tools/runtime/` | UART TX response demo (sequence `a s ? r x`) |
| `run_phase11d_accel_probe_demo.ps1` | `RobotOS_v1.0/tools/runtime/` | Most recent UART demo script; template for Phase 12M |

### E.2 New script required at Phase 12M

Phase 12M must create:

```
RobotOS_v1.0/tools/runtime/run_phase12m_probe_demo.ps1
```

This script must follow the Phase 11D template exactly:
- Accept `-ComPort COM5` (required)
- Accept standard optional params: `-BaudRate`, `-CaptureSeconds`,
  `-WaitBeforeInputSeconds`, `-OutputRttLog`, `-OutputTranscript`,
  `-SkipCapture`
- Launch `capture_devkit_rtt.ps1` in background with `-RequirePatterns`
  list (see §F.3)
- Send UART sequence via `System.IO.Ports.SerialPort` at 600 ms spacing
- Print a verification checklist at exit (see §H)
- Output RTT log: `RobotOS_v1.0/devkit/logs/phase_12M_rtt_<date>.txt`
- Output host transcript: `RobotOS_v1.0/devkit/logs/phase_12M_uart_<date>.txt`
- PowerShell 5.1 compatible (no ternary, no `&&/||`)

**This script is NOT created at Phase 12M-pre.** It is created by
Phase 12M implementation only.

### E.3 No reuse of existing demo scripts as-is

No existing script sends the Phase 12M UART sequence with the correct
PROBE pattern requirements. A new script is required. `run_phase9d_demo.ps1`
is the closest analog and should be used as the template.

---

## F. Expected RTT / Runtime Log Patterns

### F.1 Boot and init patterns (always present)

| Pattern | Source | Notes |
| --- | --- | --- |
| `RobotOS devkit starting -- board: stm32f411e_disco` | `devkit_runtime.c` | Board banner; confirms correct firmware |
| `Phase 9C app state init: state=IDLE` | `devkit_app_state.c` | App state init confirmation |
| `ROBOTOS_PROBE init ok` | `devkit_probe_adapter.c` | **Phase 12L-specific; must be present** |
| `ROBOTOS_APP state=IDLE transitions=0 button=0 uart=0 ignored=0 last_src=NONE last_byte=0x00` | `devkit_app_state.c` | Baseline app state snapshot |
| `ROBOTOS_PROBE state=1 trans=0 events=0 no_trans=0 mapped=0 unmapped=0` | `devkit_probe_adapter.c` | **Phase 12L-specific baseline; state=1=IDLE** |
| `ROBOTOS_OBS state=READY` | `devkit_observability.c` | Core observability baseline |
| `ROBOTOS_FAULT active=0` | `devkit_observability.c` | Fault baseline |
| `CFSR=0x00000000 HFSR=0x00000000` | `devkit_observability.c` | Fault register clear; must hold throughout |

### F.2 After UART command 'a' (0x61) — arm

| Pattern | Source | Expected values |
| --- | --- | --- |
| `Phase 9C app transition: IDLE->ARMED src=UART byte=0x61` | `devkit_app_state.c` | Confirms devkit_app_state IDLE→ARMED |
| `ROBOTOS_APP state=ARMED` | `devkit_app_state.c` | State update in next periodic snapshot |
| `OK state=ARMED` | `devkit_uart_producer.c` | UART TX response — **must be unchanged** |
| `ROBOTOS_PROBE state=2 trans=1 events=1 no_trans=0 mapped=1 unmapped=0` | `devkit_probe_adapter.c` | **state=2=READY; 1 transition; 1 mapped**; appears in next periodic snapshot |

### F.3 After UART command 's' (0x73) — start/activate

| Pattern | Source | Expected values |
| --- | --- | --- |
| `Phase 9C app transition: ARMED->ACTIVE src=UART byte=0x73` | `devkit_app_state.c` | Confirms devkit_app_state ARMED→ACTIVE |
| `ROBOTOS_APP state=ACTIVE` | `devkit_app_state.c` | State update in next periodic snapshot |
| `OK state=ACTIVE` | `devkit_uart_producer.c` | UART TX response — **must be unchanged** |
| `ROBOTOS_PROBE state=3 trans=2 events=2 no_trans=0 mapped=2 unmapped=0` | `devkit_probe_adapter.c` | **state=3=ACTIVE; 2 transitions; 2 mapped** |

### F.4 After UART command 'r' (0x72) — reset to IDLE

| Pattern | Source | Expected values |
| --- | --- | --- |
| `Phase 9C app transition: ACTIVE->IDLE src=UART byte=0x72` | `devkit_app_state.c` | Confirms devkit_app_state ACTIVE→IDLE |
| `ROBOTOS_APP state=IDLE` | `devkit_app_state.c` | State update in next periodic snapshot |
| `OK state=IDLE` | `devkit_uart_producer.c` | UART TX response — **must be unchanged** |
| `ROBOTOS_PROBE state=1 trans=3 events=3 no_trans=0 mapped=3 unmapped=0` | `devkit_probe_adapter.c` | **state=1=IDLE; 3 transitions; 3 mapped** |

### F.5 Notes on probe snapshot timing

The `ROBOTOS_PROBE` snapshot is emitted by the periodic observability
block at `DEVKIT_OBSERVABILITY_LOG_INTERVAL_TICKS` (every N ticks,
500 ms each). The probe dispatch happens synchronously on the command
handler thread. The snapshot reflects the state **after dispatch** when
it next fires. To reliably observe the updated snapshot:

- Allow at least one full observability tick interval (≥ 500 ms) after
  each command before expecting the updated `ROBOTOS_PROBE` snapshot.
- The baseline `ROBOTOS_PROBE state=1 trans=0...` will appear within the
  first observability tick after boot (typically within 5 s).
- Pattern matching may use partial patterns (e.g.,
  `ROBOTOS_PROBE state=2`) to avoid dependency on exact event/mapped
  count values if the sequence is executed differently.

### F.6 Invariants throughout the session

| Invariant | Expected |
| --- | --- |
| `CFSR=0x00000000` | Every `ROBOTOS_FAULT` line |
| `HFSR=0x00000000` | Every `ROBOTOS_FAULT` line |
| `accepted - dispatched = pending = 1` | `ROBOTOS_OBS` lines |
| `dropped=0 herr=0 unhandled=0 rejected=0 throttled=0` | `ROBOTOS_OBS` lines |
| No `ROBOTOS_PROBE dispatch err` | No dispatch failure log line |
| No `ROBOTOS_PROBE init failed` | Init succeeded at boot |

---

## G. Input / Command Sequence

### G.1 Minimum validation sequence

The Phase 12M demo script must implement the following sequence with
600 ms inter-command delay (one dispatch tick + margin):

| Step | Action | Expected UART TX response | Expected RTT evidence |
| --- | --- | --- | --- |
| 0 | Boot (15 s settle) | — | `ROBOTOS_PROBE init ok`; baseline snapshot `state=1` |
| 1 | Send `'a'` (0x61) | `OK state=ARMED` | App IDLE→ARMED; probe `state=2 trans=1 mapped=1` |
| 2 | Send `'s'` (0x73) | `OK state=ACTIVE` | App ARMED→ACTIVE; probe `state=3 trans=2 mapped=2` |
| 3 | Send `'r'` (0x72) | `OK state=IDLE` | App ACTIVE→IDLE; probe `state=1 trans=3 mapped=3` |
| 4 | Send `'?'` (0x3f) | `STATE state=IDLE transitions=3 ...` | App state query |
| 5 | Wait for final observability tick | — | Final `ROBOTOS_PROBE` snapshot; `CFSR=0 HFSR=0` |

Total session: 90 s recommended (15 s boot + 4 commands at 600 ms each + 70 s RTT observation window for periodic snapshots).

### G.2 Optional 'd' (disarm) extension

After step 0, optionally run: `'a'`, `'d'`, `'?'`, `'a'`, `'s'`, `'r'`, `'?'`.

Expected for `'d'` from ARMED:
- UART TX: `OK disarm state=IDLE` (unchanged from Phase 10B-d)
- App: ARMED→IDLE transition
- Probe: `ROBOTOS_PROBE state=1 trans=N+1 events=N+1 no_trans=0 mapped=N+1`
  (RESET dispatch fires on ARMED→IDLE path)

### G.3 `-RequirePatterns` list for `capture_devkit_rtt.ps1`

Phase 12M script must pass these required patterns to the harness:

```powershell
-RequirePatterns @(
    "ROBOTOS_OBS state=READY",
    "ROBOTOS_FAULT active=0",
    "ROBOTOS_PROD attempted=",
    "ROBOTOS_UART",
    "ROBOTOS_APP",
    "ROBOTOS_PROBE init ok",
    "ROBOTOS_PROBE state=2",
    "ROBOTOS_PROBE state=3",
    "ROBOTOS_PROBE state=1 trans=3"
)
```

The last three patterns prove that the probe_translator FSM transitioned
through IDLE→READY, READY→ACTIVE, and back to IDLE via the UART sequence.

---

## H. PASS / FAIL Criteria

### H.1 PASS criteria (all required)

| # | Criterion |
| --- | --- |
| 1 | `west flash` exits 0; board programs successfully |
| 2 | Boot RTT log confirms `RobotOS devkit starting -- board: stm32f411e_disco` |
| 3 | `ROBOTOS_PROBE init ok` appears in RTT log |
| 4 | Baseline `ROBOTOS_PROBE state=1 trans=0 events=0 no_trans=0 mapped=0 unmapped=0` appears |
| 5 | After 'a': `ROBOTOS_PROBE state=2` appears with `trans=1 mapped=1` |
| 6 | After 's': `ROBOTOS_PROBE state=3` appears with `trans=2 mapped=2` |
| 7 | After 'r': `ROBOTOS_PROBE state=1` appears with `trans=3 mapped=3` |
| 8 | App transition log lines appear for IDLE→ARMED, ARMED→ACTIVE, ACTIVE→IDLE |
| 9 | UART TX responses match exactly: `OK state=ARMED`, `OK state=ACTIVE`, `OK state=IDLE` |
| 10 | `CFSR=0x00000000 HFSR=0x00000000` in every `ROBOTOS_FAULT` line |
| 11 | `dropped=0 herr=0 unhandled=0 rejected=0 throttled=0` throughout |
| 12 | No `ROBOTOS_PROBE dispatch err` in RTT log |
| 13 | No crash / hardfault / reset loop in RTT log |
| 14 | RTT log saved to `RobotOS_v1.0/devkit/logs/phase_12M_rtt_<date>.txt` |
| 15 | Host UART transcript saved to `RobotOS_v1.0/devkit/logs/phase_12M_uart_<date>.txt` |

### H.2 FAIL conditions (any one triggers FAIL)

| Condition | Signal |
| --- | --- |
| Flash fails | `west flash` exit non-zero |
| Board does not boot | No `RobotOS devkit starting` banner in RTT |
| RTT capture unavailable | `capture_devkit_rtt.ps1` fails to connect; no RTT output |
| `ROBOTOS_PROBE init failed` in log | Adapter init returned non-OK |
| `ROBOTOS_PROBE init ok` missing | Adapter was not initialized on this board |
| Baseline probe snapshot missing | Adapter log not appearing in observability block |
| Probe state does not reach 2 after 'a' | `ROBOTOS_PROBE state=2` not found |
| Probe state does not reach 3 after 's' | `ROBOTOS_PROBE state=3` not found |
| Probe state does not reset to 1 after 'r' | Final `ROBOTOS_PROBE state=1 trans=3` not found |
| App transition log lines missing | Confirms existing behavior was not preserved |
| UART TX response text changed | Any response not matching Phase 9E/10B/11D baseline |
| `CFSR` or `HFSR` non-zero | Hardfault/bus-fault event |
| `hardfault` / `assert` / `panic` in log | Runtime crash |
| Evidence file not saved | Incomplete validation |
| Manual RESET required beyond fallback | POST_FLASH_AUTOSTART mitigation failed; stop, investigate |

---

## I. Evidence File Plan

| Artifact | Path | Notes |
| --- | --- | --- |
| RTT log | `RobotOS_v1.0/devkit/logs/phase_12M_rtt_2026-05-14.txt` | Full OpenOCD RTT capture; plain text |
| Host UART transcript | `RobotOS_v1.0/devkit/logs/phase_12M_uart_2026-05-14.txt` | Send/recv transcript from demo script |
| Optional flash log | `RobotOS_v1.0/devkit/logs/phase_12M_flash_2026-05-14.txt` | `west flash` output; recommended if capturing |

All evidence files must be committed with the Phase 12M closeout. The
`devkit/logs/INDEX.md` must be updated with Phase 12M row at Phase 12M close.

---

## J. Allowed / Forbidden / Conditional Surfaces for Phase 12M

### J.1 New files (allowed and required at Phase 12M)

| Path | Purpose |
| --- | --- |
| `RobotOS_v1.0/tools/runtime/run_phase12m_probe_demo.ps1` | New demo script (Phase 11D template; see §E.2) |
| `RobotOS_v1.0/devkit/logs/phase_12M_rtt_2026-05-14.txt` | RTT evidence log |
| `RobotOS_v1.0/devkit/logs/phase_12M_uart_2026-05-14.txt` | UART transcript evidence log |
| `RobotOS_v1.0/devkit/logs/phase_12M_flash_2026-05-14.txt` | (optional) Flash transcript |
| `RobotOS_v1.0/devkit/docs/02_PHASE_CLOSEOUTS/PHASE_12M_PROBE_TRANSLATOR_HARDWARE_VALIDATION.md` | Phase 12M closeout |

### J.2 Modified at Phase 12M

| Path | Change |
| --- | --- |
| `RobotOS_v1.0/devkit/logs/INDEX.md` | Add Phase 12M row |
| `CURRENT_STATE.md` | Phase 12M as latest closed |
| `RobotOS_v1.0/devkit/docs/01_PROGRESS/DEVKIT_PROGRESS_PHASE_11_20.md` | Phase 12M row + section |
| `RobotOS_v1.0/devkit/docs/00_INDEX/README.md` | Phase 12M closeout link |
| `RobotOS_v1.0/devkit/docs/03_SPECS/PROBE_TRANSLATOR_HARDWARE_VALIDATION_PLAN.md` | Status upgrade to `VALIDATED_AT_12M` |

### J.3 Forbidden at Phase 12M

| Forbidden | Reason |
| --- | --- |
| `RobotOS_v1.0/devkit/src/*` | Zero-diff at Phase 12M. Hardware validation only. |
| `RobotOS_v1.0/devkit/CMakeLists.txt` | Zero-diff at Phase 12M. |
| `RobotOS_v1.0/devkit/prj.conf` | Zero-diff at Phase 12M. |
| Any `*.dts`, `*.dtsi`, `*.overlay` | Zero-diff at Phase 12M. |
| `RobotOS_v1.0/framework/*` | Zero-diff at Phase 12M. |
| `RobotOS_v1.0/app/probe_translator/*` | Zero-diff at Phase 12M. |
| `RobotOS_v1.0/core/*`, `RobotOS_v1.0/platform/*` | Zero-diff at Phase 12M. |
| `RobotOS_v1.0/tests/host/*` | Zero-diff at Phase 12M. |
| New UART command byte | `a/s/r/?/x/v/L/d/T` frozen. |
| New UART TX response format/text | UART TX surface frozen at Phase 12M. |
| New product command mapping | `NOT_STARTED; USER_DECISION_REQUIRED`. |
| ACTIVE disarm widening | `USER_DECISION_REQUIRED_ACTIVE_DISARM`. |
| F407 / custom board | `HOLD/DEFER`. |

---

## K. Risk / Rollback Signals

| Risk | Signal | Action |
| --- | --- | --- |
| `west flash` fails | Non-zero exit; ST-LINK not detected | Check USB connection; check `west flash -d build-phase12m` command; verify ST-LINK device appears in Device Manager |
| Board boots but probe init fails | `ROBOTOS_PROBE init failed: N` in RTT | Stop; do not claim Phase 12M PASS; investigate firmware; check if `probe_translator_init` returns non-OK |
| `ROBOTOS_PROBE init ok` missing but no error | RTT log present but init log absent | Check if `devkit_probe_adapter_init()` is actually called; verify correct firmware flashed (`west flash -d build-phase12m`) |
| Probe snapshot never appears in log | `ROBOTOS_PROBE` absent after 30 s | Check if `devkit_probe_adapter_log_snapshot()` is linked in runtime tick; check `DEVKIT_OBSERVABILITY_LOG_INTERVAL_TICKS` value |
| Probe state stuck at 1 after 'a'/'s' | `ROBOTOS_PROBE state=1` persists | Verify 's' was sent after 'a' (order matters); check that probe dispatch is wired correctly in `devkit_app_state.c` |
| `no_trans_count` incrementing unexpectedly | High `no_trans` in snapshot | State divergence is expected for out-of-sequence events; verify that 'a' was sent before 's' |
| `CFSR` or `HFSR` non-zero | Any `CFSR=0x00000008` etc. | Stop immediately. Do not claim PASS. Report exact CFSR/HFSR values. Do not reflash until root cause investigated. |
| RTT capture fails to start | OpenOCD cannot connect | Check `_SEGGER_RTT` address for Phase 12M firmware (may differ from Phase 11E's `0x20000ad0`); use `nm` to find correct address |
| POST_FLASH_AUTOSTART not triggered | Board flashed but no boot log | Use manual RESET (press board RESET button once) as documented fallback; report if harness `reset run` did not auto-start |
| UART TX response text changed | 'a' → `OK arm` instead of `OK state=ARMED` | Hard stop; this is a regression; do not claim Phase 12M PASS; investigate firmware diff |

---

## L. Explicit Non-Claims

Phase 12M-pre (this planning gate) does NOT prove or establish:

| Claim | Status |
| --- | --- |
| Hardware behavior implemented | `NOT IMPLEMENTED` (docs-only phase) |
| RTT log captured | `NOT CAPTURED` |
| Board flashed | `NOT FLASHED` |
| Phase 12M hardware session run | `NOT_RUN` |
| Hardware PASS/FAIL verdict | `NOT_DETERMINED` |
| UART command surface changed | `NOT CHANGED` (frozen) |
| Product command mapping opened | `NOT STARTED; USER_DECISION_REQUIRED` |
| FAULT source admission from hardware | `NOT STARTED` |
| Non-NULL action / on_entry / on_exit | `NOT STARTED` |

Phase 12M (the implementation phase) additionally does NOT:
- Change the UART command or response surface.
- Open product command mapping.
- Change `prj.conf` / DTS / overlay.
- Change `devkit_app_state.h` public API.
- Require a new Zephyr build (uses Phase 12L firmware).

---

## M. Next-Step Recommendation

**Phase 12M-pre is `CLOSED_DOCS_ONLY`.** The hardware validation contract
is locked in `PROBE_TRANSLATOR_HARDWARE_VALIDATION_PLAN.md`.

**Next gate: Phase 12M — Probe Translator Hardware Validation**
(explicit user authorization required before hardware session starts).

**Phase 12M is execution-ready given:**
- The expected RTT patterns (§F) are locked.
- The command sequence (§G) is locked.
- The PASS/FAIL criteria (§H) are defined.
- The evidence file plan (§I) is defined.
- The new script template (§E.2) is specified.
- The `_SEGGER_RTT` address pre-check (§D.4) is the only open
  pre-flight item.

**One pre-flight item for Phase 12M operator:**
Before launching the RTT capture, confirm the `_SEGGER_RTT` address
for the Phase 12M build:
```powershell
arm-zephyr-eabi-nm build-phase12m/zephyr/zephyr.elf | `
    Select-String "_SEGGER_RTT"
```
Use the confirmed address in `capture_devkit_rtt.ps1`
(or pass it as a script parameter if the capture harness supports it).
