# Phase 12M — Probe Translator Runtime Adapter Hardware Validation

**Status:** `CLOSED_WITH_HARDWARE_EVIDENCE`
**Decision:** `PHASE_12M_PROBE_TRANSLATOR_HARDWARE_VALIDATION_PASS`
**Date:** 2026-05-14
**Branch / baseline:** `master` at `origin/master = 306db22` (Phase 12M-pre)
**Prior phase anchor:** [`PHASE_12M_PRE_PROBE_TRANSLATOR_HARDWARE_VALIDATION_PLAN.md`](PHASE_12M_PRE_PROBE_TRANSLATOR_HARDWARE_VALIDATION_PLAN.md)
**Implementation contract:** [`../03_SPECS/PROBE_TRANSLATOR_HARDWARE_VALIDATION_PLAN.md`](../03_SPECS/PROBE_TRANSLATOR_HARDWARE_VALIDATION_PLAN.md)
**RTT evidence:** `RobotOS_v1.0/devkit/logs/phase_12M_rtt_2026-05-14.txt`
**Host UART transcript:** `RobotOS_v1.0/devkit/logs/phase_12M_uart_2026-05-14.txt`
**Build log:** `RobotOS_v1.0/devkit/logs/phase_12M_build_2026-05-14.txt`
**Flash log:** `RobotOS_v1.0/devkit/logs/phase_12M_flash_2026-05-14.txt`

---

## A. Executive Summary

Phase 12M proved the Phase 12L `devkit_probe_adapter` runtime wiring on
the physical STM32F411E-DISCO board (`stm32f411e_disco` rev D). The probe
translator FSM transitioned through IDLE→READY→ACTIVE→IDLE in direct
response to UART commands `a`, `s`, `r`, with all intermediate states
captured in RTT evidence. No source change was made. No UART public
behavior was changed. No product command mapping was opened.

`run_phase12m_probe_demo.ps1 -ComPort COM5 -InterByteDelayMs 6000`
exited with all 9 required patterns FOUND. CFSR=0x00000000,
HFSR=0x00000000 (25 occurrences each). All UART TX responses matched
Phase 9E/12L baseline exactly.

---

## B. What Phase 12M Proves vs. Does Not Prove

### B.1 Proven

| Claim | Evidence |
| --- | --- |
| `devkit_probe_adapter_init()` succeeds on hardware | `ROBOTOS_PROBE init ok` at t=0 in RTT log |
| Baseline probe snapshot emitted at boot | `state=1 trans=0 events=0 no_trans=0 mapped=0 unmapped=0` at t=0 |
| UART `'a'` drives probe FSM IDLE→READY | `state=2 trans=1 events=1 no_trans=0 mapped=1 unmapped=0` at t≈14.5s |
| UART `'s'` drives probe FSM READY→ACTIVE | `state=3 trans=2 events=2 no_trans=0 mapped=2 unmapped=0` at t≈19.5s |
| UART `'r'` drives probe FSM ACTIVE→IDLE | `state=1 trans=3 events=3 no_trans=0 mapped=3 unmapped=0` at t≈24.5s |
| Existing app state IDLE→ARMED transition preserved | `Phase 9C app transition: IDLE->ARMED src=UART byte=0x61` in RTT |
| Existing app state ARMED→ACTIVE transition preserved | `Phase 9C app transition: ARMED->ACTIVE src=UART byte=0x73` in RTT |
| Existing app state ACTIVE→IDLE transition preserved | `Phase 9C app transition: ACTIVE->IDLE src=UART byte=0x72` in RTT |
| UART TX responses unchanged | `OK state=ARMED`, `OK state=ACTIVE`, `OK state=IDLE`, `STATE state=IDLE transitions=3 ...` in host transcript |
| No hardfault / crash / reset loop | CFSR=0x00000000, HFSR=0x00000000 (25 occurrences); no crash line in RTT |
| No probe dispatch error | `ROBOTOS_PROBE dispatch err` absent from RTT log |
| No dropped events / handler errors | `dropped=0 herr=0 throttled=0 rejected=0 unhandled=0` (OBS final) |

### B.2 NOT Proven (out of Phase 12M scope)

| Claim | Status |
| --- | --- |
| All possible probe_translator input combinations | NOT PROVEN (approved sequence only) |
| FAULT adapter event sourcing from hardware signals | NOT STARTED |
| Non-NULL action / on_entry / on_exit callbacks | NOT STARTED |
| Button-driven probe transitions on hardware | NOT VALIDATED (UART-only session) |
| UART TX response for probe_translator snapshot | NOT STARTED; USER_DECISION_REQUIRED |
| Product command mapping | NOT STARTED; USER_DECISION_REQUIRED |
| F407 / custom board | HOLD/DEFER |
| Multi-product coordination | NOT STARTED |

---

## C. Hardware / Tooling Setup

| Item | Value |
| --- | --- |
| Board | STM32F411E-DISCO rev D |
| Programmer | ST-LINK/V2-A (on-board, USB connector CN1) |
| UART adapter | CP210x USB-UART on COM5 |
| UART wiring | PA3 RX ← USB-UART TX; PA2 TX → USB-UART RX; GND shared |
| Baud | 115200 8N1 |
| RTT method | Phase 6O `capture_devkit_rtt.ps1`, TCP streaming, sidecar `reset run` |
| OpenOCD | xPack Open On-Chip Debugger 0.12.0+dev-02228-ge5888bda3-dirty (2025-10-04) |
| Manual RESET | Not required |

---

## D. Build / Flash Evidence

| Item | Result |
| --- | --- |
| Build command | `py -m west build --pristine=always -d build-phase12m -b stm32f411e_disco RobotOS_v1.0/devkit` |
| Build exit | **0** (165/165 targets) |
| FLASH | 43,384 B (8.27%) — identical to Phase 12L baseline |
| RAM | 12,480 B (9.52%) — identical to Phase 12L baseline |
| Build log | `RobotOS_v1.0/devkit/logs/phase_12M_build_2026-05-14.txt` |
| Flash command | `py -m west flash -d build-phase12m` |
| Flash result | PASS — wrote 49,152 bytes in 1.551 s (ST-LINK / OpenOCD) |
| Flash log | `RobotOS_v1.0/devkit/logs/phase_12M_flash_2026-05-14.txt` |

---

## E. RTT Address Confirmation

| Item | Value |
| --- | --- |
| Command | `arm-zephyr-eabi-nm build-phase12m/zephyr/zephyr.elf \| Select-String "_SEGGER_RTT"` |
| Phase 12M address | **`0x20000b38`** |
| Phase 11E reference | `0x20000ad0` |
| Delta | +0x68 (+104 bytes) — consistent with Phase 12L RAM growth (+128 B) |
| Method used | `-RttControlBlock 0x20000b38` passed to `capture_devkit_rtt.ps1` |

---

## F. UART Demo Sequence

| Item | Value |
| --- | --- |
| Script | `run_phase12m_probe_demo.ps1 -ComPort COM5 -InterByteDelayMs 6000 -CaptureSeconds 120` |
| Sequence | `a s r ?` |
| Spacing | 6000 ms (6 s) — chosen to exceed the ~4.5 s observability period so intermediate probe states are captured in RTT |
| Boot settle | 15 s |
| Session duration | 120 s |

**Note on 600 ms vs 6000 ms:** A first run at 600 ms spacing executed all
3 commands in ~1.8 s, which is shorter than the ~4.5 s observability
period. The intermediate `state=2` and `state=3` snapshots were not
captured (timing artifact, not a firmware regression). The final
`state=1 trans=3 events=3 no_trans=0 mapped=3 unmapped=0` snapshot at
t=14.5s in that run proved that 3 complete transitions occurred (IDLE→READY,
READY→ACTIVE, ACTIVE→IDLE) with no unmapped events. The 6000 ms re-run
captured all intermediate states explicitly.

---

## G. RTT Evidence Result

| Pattern | Result | RTT timestamp |
| --- | --- | --- |
| `ROBOTOS_PROBE init ok` | **FOUND** | t=0.000 s |
| `ROBOTOS_PROBE state=1 trans=0 events=0 no_trans=0 mapped=0 unmapped=0` (baseline) | **FOUND** | t=0.001 s |
| `ROBOTOS_PROBE state=2 trans=1 events=1 no_trans=0 mapped=1 unmapped=0` (after 'a') | **FOUND** | t≈14.506 s |
| `ROBOTOS_PROBE state=3 trans=2 events=2 no_trans=0 mapped=2 unmapped=0` (after 's') | **FOUND** | t≈19.508 s |
| `ROBOTOS_PROBE state=1 trans=3 events=3 no_trans=0 mapped=3 unmapped=0` (after 'r') | **FOUND** | t≈24.511 s |
| `Phase 9C app transition: IDLE->ARMED src=UART byte=0x61` | **FOUND** | t≈12.504 s |
| `Phase 9C app transition: ARMED->ACTIVE src=UART byte=0x73` | **FOUND** | t≈18.506 s |
| `Phase 9C app transition: ACTIVE->IDLE src=UART byte=0x72` | **FOUND** | t≈24.509 s |
| `ROBOTOS_OBS state=READY` | **FOUND** | Multiple |
| `ROBOTOS_FAULT active=0` | **FOUND** | Multiple |
| `ROBOTOS_PROD attempted=` | **FOUND** | Multiple |
| `CFSR=0x00000000` | **FOUND** — 25 occurrences | Throughout |
| `HFSR=0x00000000` | **FOUND** — 25 occurrences | Throughout |
| `ROBOTOS_PROBE dispatch err` | **ABSENT** (required absent) | — |
| crash / hardfault / assert | **ABSENT** | — |

OBS final counters (t≈119s):
`dropped=0 herr=0 throttled=0 rejected=0 unhandled=0 accepted=124 dispatched=123 pending=1`

---

## H. UART Evidence Result

| Command sent | Response received | Expected | Match |
| --- | --- | --- | --- |
| `'a'` (0x61) | `OK state=ARMED` | `OK state=ARMED` | **MATCH** |
| `'s'` (0x73) | `OK state=ACTIVE` | `OK state=ACTIVE` | **MATCH** |
| `'r'` (0x72) | `OK state=IDLE` | `OK state=IDLE` | **MATCH** |
| `'?'` (0x3f) | `STATE state=IDLE transitions=3 button=0 uart=4 ignored=0` | `STATE state=IDLE transitions=3 ...` | **MATCH** |

All 4 responses byte-match the Phase 9E/12L baseline.

---

## I. Expected Pattern Cross-Check (15 gates)

| # | Gate | Result |
| --- | --- | --- |
| 1 | `west flash` exit 0 | **PASS** |
| 2 | Boot banner present | **PASS** (`RobotOS devkit starting -- board: stm32f411e_disco`) |
| 3 | `ROBOTOS_PROBE init ok` | **PASS** |
| 4 | Baseline probe snapshot | **PASS** |
| 5 | `state=2` after 'a' | **PASS** |
| 6 | `state=3` after 's' | **PASS** |
| 7 | `state=1 trans=3` after 'r' | **PASS** |
| 8 | All 3 app transition log lines | **PASS** |
| 9 | UART TX responses match Phase 9E/12L | **PASS** |
| 10 | `CFSR=0x00000000` throughout | **PASS** (25×) |
| 11 | `dropped=0 herr=0 unhandled=0 rejected=0 throttled=0` | **PASS** |
| 12 | No `ROBOTOS_PROBE dispatch err` | **PASS** |
| 13 | No crash/hardfault/reset loop | **PASS** |
| 14 | RTT log saved | **PASS** (`phase_12M_rtt_2026-05-14.txt`, 44,341 bytes) |
| 15 | UART transcript saved | **PASS** (`phase_12M_uart_2026-05-14.txt`) |

**All 15 gates: PASS.**

---

## J. Boundary / Non-Regression Confirmation

| Surface | Status |
| --- | --- |
| `RobotOS_v1.0/devkit/CMakeLists.txt` | Zero-diff |
| `RobotOS_v1.0/devkit/prj.conf` | Zero-diff |
| DTS / overlay | Zero-diff |
| `RobotOS_v1.0/devkit/src/devkit_app_state.{c,h}` | Zero-diff |
| `RobotOS_v1.0/devkit/src/devkit_runtime.c` | Zero-diff |
| `RobotOS_v1.0/devkit/src/devkit_probe_adapter.{c,h}` | Zero-diff (hardware validates as-is) |
| `RobotOS_v1.0/devkit/src/devkit_uart_producer.{c,h}` | Zero-diff |
| `RobotOS_v1.0/framework/*.{h,c}` | Zero-diff |
| `RobotOS_v1.0/app/probe_translator/probe_translator.{h,c}` | Zero-diff |
| `RobotOS_v1.0/core/*`, `RobotOS_v1.0/platform/*` | Zero-diff |
| `RobotOS_v1.0/tests/host/*` | Zero-diff |
| UART command set `a/s/r/?/x/v/L/d/T` | **FROZEN — unchanged** |
| UART TX response text/format | **UNCHANGED** (byte-matched Phase 9E baseline) |
| Product command mapping | **NOT OPENED** |
| Scheduler behavior | **UNCHANGED** |
| F407 / custom board | **HOLD/DEFER — not opened** |

---

## K. PASS / FAIL Decision

**PHASE 12M: PASS**
**Decision token:** `PHASE_12M_PROBE_TRANSLATOR_HARDWARE_VALIDATION_PASS`

All 15 validation gates PASS. RTT evidence confirms the complete probe
translator FSM progression IDLE→READY→ACTIVE→IDLE on real hardware,
driven by UART commands `a s r` via the Phase 12L runtime adapter. Existing
app-state and UART TX behavior are unchanged.

---

## L. Known Limitations

1. **Observability timing:** The probe adapter snapshot fires only at the
   periodic observability interval (~4.5 s). At 600 ms command spacing,
   intermediate states are not captured. This is a log-capture timing
   artifact, not a firmware regression. The 6000 ms spacing run captures
   all states explicitly.

2. **UART-only session:** Phase 12M exercised only the UART command path.
   Button-driven probe transitions (`devkit_app_state_on_button()`) are
   validated at build depth (host regression 23/23 PASS) but not
   hardware-validated in this session.

3. **Single transition sequence only:** Only the `a s r` happy path was
   validated. Out-of-order dispatches (e.g., `s` without `a`) and FAULT
   adapter events were not exercised on hardware.

4. **No visual hardware observation:** No LED blink phase-shift or physical
   measurement was included — this is a log-pattern-only evidence session.

---

## M. Next-Step Recommendation

**User** — decide next phase priority.

**Open at PASS:**
- Hardware validation is complete at the approved depth. Phase 12M-Z
  (checkpoint) is a natural next docs-only gate.
- FAULT source wiring (hardware fault signal → FAULT adapter event)
  requires a separate phase and `USER_DECISION_REQUIRED`.
- UART TX response for probe_translator snapshot requires
  `USER_DECISION_REQUIRED`.
- Product command mapping requires `USER_DECISION_REQUIRED`.
- Button-path hardware validation (optional) would require a new
  hardware session.

**HOLD** is a safe default — all hardware evidence is captured; the
probe translator is now hardware-validated at the approved sequence depth.
