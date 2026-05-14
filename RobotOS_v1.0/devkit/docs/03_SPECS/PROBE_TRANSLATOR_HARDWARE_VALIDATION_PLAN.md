# RobotOS — Probe Translator Hardware Validation Plan

**Status:** `VALIDATED_AT_12M (HARDWARE EVIDENCE)`
**Spec type:** Long-lived hardware validation contract for Phase 12M.
**Revision:** Phase 12M (2026-05-14, `CLOSED_WITH_HARDWARE_EVIDENCE`;
`PHASE_12M_PROBE_TRANSLATOR_HARDWARE_VALIDATION_PASS`) —
hardware validation landed via `run_phase12m_probe_demo.ps1 -ComPort COM5
-InterByteDelayMs 6000 -CaptureSeconds 120`; Phase 12L firmware flashed
to `stm32f411e_disco` rev D; all 9 RTT patterns FOUND; CFSR/HFSR all 0
(25×); UART TX responses byte-matched; 15/15 validation gates PASS;
`_SEGGER_RTT=0x20000b38`; RTT log `devkit/logs/phase_12M_rtt_2026-05-14.txt`
(44,341 bytes).
**Previous revision:** Phase 12M-pre (2026-05-14, `CLOSED_DOCS_ONLY`).
**Planning closeout anchor:**
[`../02_PHASE_CLOSEOUTS/PHASE_12M_PRE_PROBE_TRANSLATOR_HARDWARE_VALIDATION_PLAN.md`](../02_PHASE_CLOSEOUTS/PHASE_12M_PRE_PROBE_TRANSLATOR_HARDWARE_VALIDATION_PLAN.md)
**Implementation closeout anchor:**
[`../02_PHASE_CLOSEOUTS/PHASE_12M_PROBE_TRANSLATOR_HARDWARE_VALIDATION.md`](../02_PHASE_CLOSEOUTS/PHASE_12M_PROBE_TRANSLATOR_HARDWARE_VALIDATION.md)
**Prior runtime-admission spec:**
[`PROBE_TRANSLATOR_RUNTIME_ADMISSION_PLAN.md`](PROBE_TRANSLATOR_RUNTIME_ADMISSION_PLAN.md)

---

## 1. Status / Scope

**What this doc is:**
The execution-ready hardware validation contract for Phase 12M.
All flash commands, RTT commands, UART sequences, expected log
patterns, PASS/FAIL criteria, evidence paths, and new-script
specification are locked here.

**What this doc is not:**
- A UART command expansion spec. `a/s/r/?/x/v/L/d/T` frozen at Phase 12M.
- A product command mapping spec.
- A FAULT source spec.
- A framework/probe_translator source change spec.

---

## 2. Phase 12M Approved File Set

### 2.1 New files (allowed and required at Phase 12M)

| Path | Purpose |
| --- | --- |
| `RobotOS_v1.0/tools/runtime/run_phase12m_probe_demo.ps1` | New demo script (Phase 11D template) |
| `RobotOS_v1.0/devkit/logs/phase_12M_rtt_2026-05-14.txt` | RTT evidence log |
| `RobotOS_v1.0/devkit/logs/phase_12M_uart_2026-05-14.txt` | UART transcript evidence log |
| `RobotOS_v1.0/devkit/logs/phase_12M_flash_2026-05-14.txt` | (optional) Flash transcript |
| `RobotOS_v1.0/devkit/docs/02_PHASE_CLOSEOUTS/PHASE_12M_PROBE_TRANSLATOR_HARDWARE_VALIDATION.md` | Phase 12M closeout |

### 2.2 Modified at Phase 12M

| Path | Change |
| --- | --- |
| `RobotOS_v1.0/devkit/logs/INDEX.md` | Add Phase 12M RTT row |
| `RobotOS_v1.0/devkit/docs/03_SPECS/PROBE_TRANSLATOR_HARDWARE_VALIDATION_PLAN.md` | Status upgrade → `VALIDATED_AT_12M (HARDWARE EVIDENCE)` |
| `CURRENT_STATE.md` | Phase 12M as latest closed |
| `RobotOS_v1.0/devkit/docs/01_PROGRESS/DEVKIT_PROGRESS_PHASE_11_20.md` | Phase 12M row + section |
| `RobotOS_v1.0/devkit/docs/00_INDEX/README.md` | Phase 12M closeout link |

### 2.3 Forbidden at Phase 12M

All source, header, CMake, Kconfig, `prj.conf`, DTS, overlay, test, and
existing tool files — **zero-diff at Phase 12M**. The hardware session
validates the Phase 12L firmware as-is.

---

## 3. Environment and Flash Procedure

### 3.1 Firmware

Phase 12M uses the Phase 12L firmware (commit `31968ad`) exactly as built
at Phase 12L. Build a fresh pristine image before flashing:

```powershell
# Set session-local Zephyr env first:
$env:ZEPHYR_BASE = "d:\Robot_OS\zephyr"
$env:ZEPHYR_SDK_INSTALL_DIR = "C:\zephyr-sdk-0.17.0"
$env:PATH = "C:\zephyr-sdk-0.17.0\arm-zephyr-eabi\bin;" + $env:PATH

# Build:
py -m west build --pristine=always -d build-phase12m `
    -b stm32f411e_disco RobotOS_v1.0/devkit

# Flash:
py -m west flash -d build-phase12m
```

Expected FLASH: ~43,384 B. Confirm exit 0 from `west flash`.

### 3.2 RTT block address pre-flight

The `_SEGGER_RTT` block address varies with firmware size. Confirm for
the Phase 12M build **before** launching RTT capture:

```powershell
arm-zephyr-eabi-nm build-phase12m/zephyr/zephyr.elf | `
    Select-String "_SEGGER_RTT"
```

Record the address (example from Phase 11E: `0x20000ad0`; Phase 12M
may differ by the RAM delta of ~128 B from Phase 12L).

### 3.3 POST_FLASH_AUTOSTART

Root cause `OPEN`; `MITIGATED_BY_WORKFLOW` (Phase 6O onward). The
`capture_devkit_rtt.ps1` sidecar issues `reset run` to start firmware
after OpenOCD attaches. Use the harness as specified. Manual RESET on
the board is a fallback only.

---

## 4. New Demo Script Specification

### 4.1 Script path and template

```
RobotOS_v1.0/tools/runtime/run_phase12m_probe_demo.ps1
```

Follow `run_phase11d_accel_probe_demo.ps1` as the template (most recent
demo script; PowerShell 5.1 compatible).

### 4.2 Parameters

| Parameter | Type | Default | Notes |
| --- | --- | --- | --- |
| `-ComPort` | string | required | e.g. `COM5` |
| `-BaudRate` | int | 115200 | |
| `-Sequence` | string[] | `@('a','s','r','?')` | Phase 12M minimum sequence |
| `-InterByteDelayMs` | int | 600 | One dispatch tick (500 ms) + margin |
| `-ReadTimeoutMs` | int | 1200 | |
| `-CaptureSeconds` | int | 90 | Boot (15 s) + sequence (~3 s) + observation (72 s) |
| `-WaitBeforeInputSeconds` | int | 15 | Board boot + IDLE baseline |
| `-OutputRttLog` | string | `RobotOS_v1.0/devkit/logs/phase_12M_rtt_<date>.txt` | |
| `-OutputTranscript` | string | `RobotOS_v1.0/devkit/logs/phase_12M_uart_<date>.txt` | |
| `-SkipCapture` | switch | off | |
| `-CaptureScript` | string | sibling `capture_devkit_rtt.ps1` | |

### 4.3 Required patterns for capture harness

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

### 4.4 Verification checklist to emit at script exit

```
Phase 12M Probe Adapter Hardware Validation Checklist:
-------------------------------------------------------
RTT evidence:
[ ] 'ROBOTOS_PROBE init ok' present
[ ] Baseline 'ROBOTOS_PROBE state=1 trans=0 events=0 no_trans=0 mapped=0 unmapped=0' present
[ ] After 'a': 'ROBOTOS_PROBE state=2 trans=1 events=1 no_trans=0 mapped=1 unmapped=0'
[ ] After 's': 'ROBOTOS_PROBE state=3 trans=2 events=2 no_trans=0 mapped=2 unmapped=0'
[ ] After 'r': 'ROBOTOS_PROBE state=1 trans=3 events=3 no_trans=0 mapped=3 unmapped=0'
[ ] App transition lines: IDLE->ARMED, ARMED->ACTIVE, ACTIVE->IDLE
[ ] CFSR=0x00000000 and HFSR=0x00000000 in every fault line
[ ] dropped=0 herr=0 unhandled=0 rejected=0 throttled=0 throughout
[ ] No 'ROBOTOS_PROBE dispatch err' line
[ ] No crash/hardfault/assert/reset loop

Host UART transcript:
[ ] 'a' -> 'OK state=ARMED'
[ ] 's' -> 'OK state=ACTIVE'
[ ] 'r' -> 'OK state=IDLE'
[ ] '?' -> 'STATE state=IDLE transitions=3 ...'
```

---

## 5. Expected Log Patterns (exact)

### 5.1 Boot and init

| Expected text (partial match) | Required? |
| --- | --- |
| `RobotOS devkit starting -- board: stm32f411e_disco` | required |
| `Phase 9C app state init: state=IDLE` | required |
| `ROBOTOS_PROBE init ok` | **required** |
| `ROBOTOS_APP state=IDLE transitions=0 button=0 uart=0 ignored=0` | required |
| `ROBOTOS_PROBE state=1 trans=0 events=0 no_trans=0 mapped=0 unmapped=0` | **required** |
| `ROBOTOS_OBS state=READY` | required |
| `CFSR=0x00000000` | required (every occurrence) |
| `HFSR=0x00000000` | required (every occurrence) |

### 5.2 After 'a' (arm)

| Expected text (partial match) | Required? |
| --- | --- |
| `Phase 9C app transition: IDLE->ARMED src=UART byte=0x61` | required |
| `ROBOTOS_PROBE state=2 trans=1 events=1 no_trans=0 mapped=1 unmapped=0` | **required** |

UART TX response (host transcript): `OK state=ARMED`

### 5.3 After 's' (start)

| Expected text (partial match) | Required? |
| --- | --- |
| `Phase 9C app transition: ARMED->ACTIVE src=UART byte=0x73` | required |
| `ROBOTOS_PROBE state=3 trans=2 events=2 no_trans=0 mapped=2 unmapped=0` | **required** |

UART TX response (host transcript): `OK state=ACTIVE`

### 5.4 After 'r' (reset)

| Expected text (partial match) | Required? |
| --- | --- |
| `Phase 9C app transition: ACTIVE->IDLE src=UART byte=0x72` | required |
| `ROBOTOS_PROBE state=1 trans=3 events=3 no_trans=0 mapped=3 unmapped=0` | **required** |

UART TX response (host transcript): `OK state=IDLE`

### 5.5 Probe state constant reference

| Constant | Value | Name |
| --- | --- | --- |
| `PROBE_TRANSLATOR_STATE_IDLE` | `1u` | Initial state; after reset |
| `PROBE_TRANSLATOR_STATE_READY` | `2u` | After CONFIG event (arm) |
| `PROBE_TRANSLATOR_STATE_ACTIVE` | `3u` | After START event |
| `PROBE_TRANSLATOR_STATE_FAULT` | `4u` | After FAULT event (not exercised at Phase 12M) |

---

## 6. Validation Gates for Phase 12M (15 gates)

| # | Gate | Command / Evidence |
| --- | --- | --- |
| 1 | Build PASS (Phase 12M build) | `py -m west build --pristine=always -d build-phase12m -b stm32f411e_disco RobotOS_v1.0/devkit` — exit 0 |
| 2 | Flash PASS | `py -m west flash -d build-phase12m` — exit 0 |
| 3 | Boot confirmed | `RobotOS devkit starting -- board: stm32f411e_disco` in RTT log |
| 4 | Probe init confirmed | `ROBOTOS_PROBE init ok` in RTT log |
| 5 | Baseline probe snapshot | `ROBOTOS_PROBE state=1 trans=0 events=0 no_trans=0 mapped=0 unmapped=0` in RTT log |
| 6 | Probe READY after 'a' | `ROBOTOS_PROBE state=2 trans=1` in RTT log |
| 7 | Probe ACTIVE after 's' | `ROBOTOS_PROBE state=3 trans=2` in RTT log |
| 8 | Probe IDLE after 'r' | `ROBOTOS_PROBE state=1 trans=3` in RTT log |
| 9 | App transitions present | All three `Phase 9C app transition` lines in RTT log |
| 10 | UART TX responses unchanged | 'a'→`OK state=ARMED`, 's'→`OK state=ACTIVE`, 'r'→`OK state=IDLE` in host transcript |
| 11 | CFSR=0 HFSR=0 | Every `ROBOTOS_FAULT` line in RTT log |
| 12 | No errors in RTT log | `dropped=0 herr=0 unhandled=0 rejected=0 throttled=0` throughout |
| 13 | No probe dispatch error | `ROBOTOS_PROBE dispatch err` absent from RTT log |
| 14 | RTT log committed | `devkit/logs/phase_12M_rtt_2026-05-14.txt` in repo |
| 15 | `devkit/logs/INDEX.md` updated | Phase 12M row added with capture result |

---

## 7. Evidence Policy

### 7.1 RTT evidence log

```
RobotOS_v1.0/devkit/logs/phase_12M_rtt_2026-05-14.txt
```

Content: full OpenOCD RTT capture (plain text; capture_devkit_rtt.ps1
output). Must be committed with Phase 12M closeout. Update
`devkit/logs/INDEX.md` with Phase 12M row.

### 7.2 Host UART transcript

```
RobotOS_v1.0/devkit/logs/phase_12M_uart_2026-05-14.txt
```

Content: per-command send/receive transcript from demo script. Must be
committed with Phase 12M closeout.

### 7.3 Phase 12M does not require host regression rerun

Phase 12L host regression was 23/23 PASS. Phase 12M makes no source
change; a host regression rerun is not required unless a source change
is discovered during the hardware session (which would be a FAIL
condition requiring investigation, not an in-session change).

---

## 8. Closeout Criteria for Phase 12M

Phase 12M closeout must report:

1. **Flash result:** `west flash` exit code.
2. **Build command used** (if rebuilt).
3. **RTT address confirmed:** `_SEGGER_RTT=0x????????`.
4. **Session summary:** pass/fail for each of the 15 validation gates.
5. **UART TX responses observed:** exact text from host transcript.
6. **Probe state progression:** IDLE→READY→ACTIVE→IDLE confirmed.
7. **App state transitions preserved:** existing behavior unchanged.
8. **CFSR/HFSR:** 0x00000000 throughout.
9. **Evidence files committed:** RTT log + UART transcript paths.
10. **No source/CMake/config change:** zero-diff on all implementation surfaces.
11. **No UART public command change:** `a/s/r/?/x/v/L/d/T` frozen.
12. **No product command mapping opened.**
13. **No hardware behavior beyond Phase 12M session claimed.**

---

## 9. Open Decisions (after Phase 12M)

| # | Open question | Status |
| --- | --- | --- |
| 1 | FAULT adapter event sourcing (hardware fault signal) | `NOT_STARTED` |
| 2 | UART TX response for probe_translator snapshot | `NOT_STARTED; USER_DECISION_REQUIRED` |
| 3 | Non-NULL action / on_entry / on_exit callbacks | `NOT_STARTED` |
| 4 | ACTIVE disarm widening | `USER_DECISION_REQUIRED_ACTIVE_DISARM` |
| 5 | Bridge ABI memory-layout lock | `NOT_STARTED` |
| 6 | Product command mapping / UART expansion | `NOT_STARTED; USER_DECISION_REQUIRED` |
| 7 | `app/probe_translator/CMakeLists.txt` for app-module isolation | `DEFERRED` |
| 8 | Scheduler 7A/7B | `DEFER` |
| 9 | F407 / custom board | `HOLD/DEFER` |
| 10 | Multi-product coordination | `NOT_STARTED` |
