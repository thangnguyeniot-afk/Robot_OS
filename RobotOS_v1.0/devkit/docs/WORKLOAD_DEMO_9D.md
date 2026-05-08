# Phase 9D — RobotOS Workload Demo Runbook

**Purpose.** Repeatable, audit-ready demonstration of the current RobotOS
devkit application: a three-state machine (IDLE / ARMED / ACTIVE) driven
by two real hardware event sources (user button via GPIO/EXTI, UART RX
via USART2), running on the STM32F411E-DISCO.

**Phase 9D scope.** Tooling and documentation only. No firmware change,
no telemetry redesign, no scheduler change. The demo exercises code that
shipped in:

- Phase 9A-A button producer (`2068180`)
- Phase 9A-B button debounce (`92de5e0`)
- Phase 9A-C Phase 6I startup-burst gate (`3989ff9`)
- Phase 9B UART RX producer (`85389f4`)
- Phase 9C application state machine (`286e61b`)

---

## 1. Hardware setup

| Item | Value | Notes |
| ---- | ----- | ----- |
| Board | STM32F411E-DISCO | Cortex-M4 @ 96 MHz; Zephyr 3.6.0 |
| Programmer | On-board ST-LINK/V2 (USB) | also powers the board |
| Logging | SEGGER RTT via OpenOCD | RTT is canonical; UART is **not** used as a console |
| UART | USART2 (`zephyr,console` chosen node) | PA2 TX, **PA3 RX**, 115200 8N1 |
| User button | "USER" (blue) on PA0 (sw0 alias) | not the black RESET button |

**External USB-UART adapter** (CP210x / FTDI / CH340) wired to:

```text
adapter TX  -> board PA3   (USART2 RX)
adapter GND -> board GND
adapter VCC: do NOT connect (board is ST-LINK powered)
```

The board has no on-board USB-VCP; the external adapter is required for
any UART RX traffic.

---

## 2. Build & flash (one-time per firmware change)

```powershell
$env:ZEPHYR_TOOLCHAIN_VARIANT = "zephyr"
$env:ZEPHYR_SDK_INSTALL_DIR   = "C:\zephyr-sdk-0.17.0"
west build -b stm32f411e_disco RobotOS_v1.0/devkit/ --pristine
west flash
```

Expected build size at Phase 9C baseline: FLASH ≈ 35.7 KB / RAM ≈ 12 KB.

---

## 3. Run the demo

### 3.1 Recommended — scripted runner

```powershell
.\RobotOS_v1.0\tools\runtime\run_phase9d_demo.ps1 -ComPort COM5
```

Replace `COM5` with the COM port of your USB-UART adapter
(`Get-PnpDevice -Class Ports` lists candidates).

The runner:

1. Prints the wiring reminder.
2. Launches `capture_devkit_rtt.ps1` as a background job (90 s by default).
3. Waits 12 s for boot + IDLE baseline emission.
4. Sends UART bytes `a`, `s`, `r` individually (1.5 s apart) to the COM port.
5. Prompts the operator: **"PRESS THE USER BUTTON 3 TIMES NOW"**.
6. Waits for the capture window to close.
7. Surfaces the Phase 6O harness output and prints the verification checklist.

### 3.2 Alternative — semi-scripted runbook

If `run_phase9d_demo.ps1` is unavailable or you prefer manual steps:

```powershell
# Window 1 — start RTT capture (90 s)
$log = "RobotOS_v1.0/devkit/logs/phase_9D_workload_demo_$(Get-Date -Format yyyy-MM-dd).txt"
.\RobotOS_v1.0\tools\runtime\capture_devkit_rtt.ps1 `
    -OutputLog $log -WaitSeconds 90 `
    -RequirePatterns @(
        "ROBOTOS_OBS state=READY",
        "ROBOTOS_FAULT active=0",
        "ROBOTOS_PROD attempted=",
        "ROBOTOS_BTN",
        "ROBOTOS_UART",
        "ROBOTOS_APP",
        "Phase 9C app state init",
        "Phase 9C app transition",
        "Phase 9B uart handled",
        "Phase 9A button handled",
        "src=BTN",
        "src=UART"
    )

# Window 2 — after ~12 s, send UART bytes individually
$port = New-Object System.IO.Ports.SerialPort `
            'COM5', 115200, ([System.IO.Ports.Parity]::None), 8, ([System.IO.Ports.StopBits]::One)
$port.Open()
$port.Write("a"); Start-Sleep -Milliseconds 1500
$port.Write("s"); Start-Sleep -Milliseconds 1500
$port.Write("r"); Start-Sleep -Milliseconds 500
$port.Close()

# Window 3 (or your physical hand) — press USER button 3 times, ~1.2 s apart
```

---

## 4. Demo scenario (canonical)

| Step | Input | Source | Expected app transition |
| ---- | ----- | ------ | ----------------------- |
| 1 | UART byte `'a'` (0x61) | host PowerShell | IDLE → ARMED |
| 2 | UART byte `'s'` (0x73) | host PowerShell | ARMED → ACTIVE |
| 3 | UART byte `'r'` (0x72) | host PowerShell | ACTIVE → IDLE |
| 4 | Button press #1 | physical USER button | IDLE → ARMED |
| 5 | Button press #2 | physical USER button | ARMED → ACTIVE |
| 6 | Button press #3 | physical USER button | ACTIVE → IDLE |

**Order matters.** If the button is pressed during the UART send window
or vice-versa, the transition sequence will reflect the actual event
order; the runner enforces UART-first by sending all 3 bytes before
prompting the operator.

---

## 5. Pass / fail criteria

The capture passes when **all** of the following hold in the log file
specified by `-OutputLog`:

### 5.1 Required string patterns (Phase 6O harness)

The script's `-RequirePatterns` covers these; the harness exits 0 only
when each substring appears at least once:

- `ROBOTOS_OBS state=READY`
- `ROBOTOS_FAULT active=0`
- `ROBOTOS_PROD attempted=`
- `ROBOTOS_BTN`
- `ROBOTOS_UART`
- `ROBOTOS_APP`
- `Phase 9C app state init`
- `Phase 9C app transition`
- `Phase 9B uart handled`
- `Phase 9A button handled`
- `src=BTN`
- `src=UART`

### 5.2 Final `ROBOTOS_APP` line (deterministic targets)

After all 6 transitions complete, the next periodic snapshot should be:

```text
ROBOTOS_APP state=IDLE transitions=6 button=3 uart=3 ignored=0 last_src=BTN last_byte=0x72
```

**`last_byte=0x72`** is the LF-free 'r' byte from step 3; subsequent
button presses do not overwrite `last_byte` (button has no byte payload).

If `ignored=1` appears, a stray byte (typically LF) was sent — see §7.

### 5.3 Per-source counter consistency

| Source | Final counter | Expected | Notes |
| ------ | ------------- | -------- | ----- |
| ROBOTOS_UART | rx | 3 | bytes read from UART FIFO |
| ROBOTOS_UART | ok | 3 | all posts succeeded |
| ROBOTOS_UART | full / invalid / other | 0 / 0 / 0 | clean |
| ROBOTOS_UART | handled | 3 | thread-context handler invocations |
| ROBOTOS_BTN | ok | 3 | accepted button events |
| ROBOTOS_BTN | handled | 3 | thread-context handler invocations |
| ROBOTOS_BTN | debounce | ≥ 0 | bounce filtering may add to `attempted` but not to `ok` |
| ROBOTOS_BTN | full / invalid / other | 0 / 0 / 0 | clean (no contention with synthetic burst — Phase 6I gate is off) |

### 5.4 Architecture invariants

- `OBS accepted − OBS dispatched = OBS pending` (single tick may have a 1-event lag)
- `OBS accepted = ROBOTOS_PROD ok + ROBOTOS_BTN ok + ROBOTOS_UART ok`
- `OBS herr = 0`, `OBS unhandled = 0`, `OBS rejected = 0`, `OBS throttled = 0`
- `OBS dropped = 0` under controlled input
- `OBS peak ≤ 8` (combined-source workload should be well below capacity=16)

### 5.5 Fault registers

- CFSR = `0x00000000` in every `ROBOTOS_FAULT` emission
- HFSR = `0x00000000` in every `ROBOTOS_FAULT` emission

### 5.6 Phase 6I gate (preserved from 9A-C)

- Boot banner shows `DEVKIT_DIAG phase6i_startup_burst=0`
- `Phase 6I timer producer started`, `Phase 6I event handled`, `Phase 6I final:` are **absent**

---

## 6. Capture deliverable

The Phase 6O harness writes the full RTT log to:

```text
RobotOS_v1.0/devkit/logs/phase_9D_workload_demo_<yyyy-MM-dd>.txt
```

This file is the canonical Phase 9D evidence and is committed alongside
the runbook.

---

## 7. Known limitations and caveats

- **External USB-UART adapter required.** The DISCO board has no on-board USB-VCP. Without the adapter, UART input cannot reach PA3 and the demo cannot be run.
- **UART line-ending.** This runbook deliberately writes single bytes with no terminator. If a serial-terminal app sends `asr\n` (with LF), the `\n` (0x0a) increments `ignored`, leaving the deterministic check at `transitions=6 ignored=1` instead of `0`. `last_byte` will then be `0x0a` instead of `0x72`. Both states are explainable; only the no-newline path matches §5.2 verbatim.
- **Button bounce.** The Phase 9A-B 30 ms time-guard filters mechanical bounce; only debounced presses become app events. If a press is held > 30 ms or pressed extremely rapidly, `ROBOTOS_BTN debounce` will be > 0 while `ok` remains 3.
- **Operator timing.** The runner prompts for button presses *after* the UART send completes; if the operator is slow to start (> ~30 s), the capture window may close before the third press. The default 90 s window leaves comfortable margin.
- **No UART TX response.** Phase 9D does not introduce a TX path. A `'?'` query is logged to RTT only, never echoed on the wire.
- **Workload is devkit-local.** This is a demo, not a Robot Framework application abstraction. Future application phases (9E, 10+) may promote this pattern — that is out of scope here.
- **STM32F407 / custom board.** Phase 8A is HOLD/DEFER; the demo runs only on the F411E-DISCO target.
- **Scheduler.** Phase 7A/7B-1 remain DEFER. Combined-source `peak` from the demo is typically ≤ 4, well below `ROBOTOS_EVENT_QUEUE_CAPACITY = 16`.

---

## 8. Cross-references

- Capture harness: [`tools/runtime/capture_devkit_rtt.ps1`](../../tools/runtime/capture_devkit_rtt.ps1) — Phase 6O.
- Telemetry field reference: [`devkit/docs/TELEMETRY_REFERENCE.md`](TELEMETRY_REFERENCE.md).
- Phase history (full audit trail): [`devkit/docs/DEVKIT_PROGRESS.md`](DEVKIT_PROGRESS.md) Phase 9A-A through 9D.
- Per-phase RTT logs: [`devkit/logs/INDEX.md`](../logs/INDEX.md).
- Lightweight startup snapshot: [`CURRENT_STATE.md`](../../../CURRENT_STATE.md).
