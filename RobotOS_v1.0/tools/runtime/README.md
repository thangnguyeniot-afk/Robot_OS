# RobotOS Runtime Evidence Harness

Canonical runtime evidence capture workflow for RobotOS phases on STM32F411E-DISCO.

---

## Scripts

| Script | Purpose | Phase |
|--------|---------|-------|
| `capture_devkit_rtt.ps1` | **Streaming RTT capture** — 30-60 s TCP streaming via OpenOCD RTT server. Required for ROBOTOS_OBS / ROBOTOS_FAULT / ROBOTOS_PROD triplet validation. | Phase 6O+ |
| `capture_phase6h_runtime.ps1` | **Snapshot RTT / GDB counter read** — 4 KB `dump_image` snapshot for Phase 6H counter comparison (attempted=8 ok=8 handled=8 ...). | Phase 6H specific |
| `phase6h_read.gdb` | GDB batch script used as automatic fallback by `capture_phase6h_runtime.ps1`. | Phase 6H specific |
| `phase6z_required_patterns.txt` | Human-readable reference for the default `capture_devkit_rtt.ps1` required patterns. Not read at runtime. | Reference only |

**Which script to use:**

- For **Phase 6K / 6L / 6M / 6Z-style validation** (ROBOTOS_OBS + ROBOTOS_FAULT + ROBOTOS_PROD):
  use `capture_devkit_rtt.ps1`. The 4 KB dump_image approach in the Phase 6H script
  wraps the RTT ring buffer in under 5 seconds of the Phase 6K/6L/6M log density.
- For **Phase 6H counter comparison** (attempted=8 ok=8 handled=8):
  use `capture_phase6h_runtime.ps1`. This script has hardcoded Phase 6H pass criteria
  and should not be used for other validation scenarios.

---

## Phase 6O — Streaming RTT Capture (`capture_devkit_rtt.ps1`)

### Method

OpenOCD is started with a sidecar `.cfg` file that issues:
```
init
reset run
rtt setup <_SEGGER_RTT address> 64 "SEGGER RTT"
rtt start
rtt server start 9090 0
```

The `reset run` command starts the firmware cleanly post-flash, mitigating the
known `POST_FLASH_AUTOSTART` unreliable-auto-start issue on STM32F411E-DISCO
(Phase 3B Open Issue in `devkit/docs/01_PROGRESS/DEVKIT_PROGRESS.md` — root cause remains
OPEN; this harness is workflow mitigation, not a firmware/runner/hardware fix).
Plain `west flash` alone remains insufficient as runtime-start evidence; this
harness or manual RESET is required.
The RTT TCP server on port 9090 streams live RTT output to a PowerShell TcpClient.

`_SEGGER_RTT` address is resolved dynamically from the ELF via
`arm-zephyr-eabi-nm` — it changes with every build and cannot be hardcoded.

Proven by Phase 6Z: 60 s capture, 16,560 bytes, ROBOTOS_OBS/FAULT/PROD baseline +
12 periodic triplets, CFSR=0 HFSR=0 throughout.

### Quick start (from repo root)

```powershell
# Already flashed — capture only with default 60 s window:
.\RobotOS_v1.0\tools\runtime\capture_devkit_rtt.ps1

# Build + flash + capture (full workflow, board must be connected):
.\RobotOS_v1.0\tools\runtime\capture_devkit_rtt.ps1 -BuildFirst -FlashFirst

# Quick 30 s smoke check with a custom log path:
.\RobotOS_v1.0\tools\runtime\capture_devkit_rtt.ps1 `
    -OutputLog RobotOS_v1.0\devkit\logs\phase_6O_harness_smoke_2026-05-08.txt `
    -WaitSeconds 30

# Phase 8A custom STM32F407 board (override config + search range):
.\RobotOS_v1.0\tools\runtime\capture_devkit_rtt.ps1 `
    -OpenOcdConfig D:\my_project\openocd_f407.cfg `
    -RttSearchBase 0x20000000 -RttSearchSize 0x40000 `
    -RequirePatterns @("ROBOTOS_OBS state=READY", "ROBOTOS_FAULT active=0")

# Use explicit RTT address (skip nm — useful when ELF not available):
.\RobotOS_v1.0\tools\runtime\capture_devkit_rtt.ps1 `
    -RttControlBlock "0x20000a34"
```

### Parameters

| Parameter | Type | Default | Description |
|---|---|---|---|
| `-OutputLog` | string | `devkit/logs/phase_rtt_<date>.txt` | Log output path; parent dir created automatically |
| `-WaitSeconds` | int | 60 | Capture window; at 500ms tick: 60s = 12 periodic triplets after burst settles |
| `-Port` | int | 9090 | OpenOCD RTT TCP server port |
| `-Board` | string | stm32f411e_disco | Board name for build metadata |
| `-BuildFirst` | switch | off | Run `west build --pristine` before capture |
| `-FlashFirst` | switch | off | Run `west flash` before capture; kills stale OpenOCD first |
| `-RequirePatterns` | string[] | see below | Literal strings all of which must appear in the captured log |
| `-OpenOcdExe` | string | auto | openocd.exe path; auto-discovered from PATH then WinGet xpack location |
| `-OpenOcdScripts` | string | auto | OpenOCD scripts dir; derived from OpenOcdExe location |
| `-OpenOcdConfig` | string | board DTS cfg | Board openocd.cfg path |
| `-NmExe` | string | auto from SDK | arm-zephyr-eabi-nm.exe path |
| `-ElfPath` | string | `build/zephyr/zephyr.elf` | Firmware ELF for _SEGGER_RTT address resolution |
| `-RttControlBlock` | string | auto via nm | Explicit RTT address; skips nm when provided |
| `-RttSearchBase` | string | 0x20000000 | RAM base for RTT fallback search when nm fails |
| `-RttSearchSize` | string | 0x20000 | RAM range size for fallback search (0x20000=128KB, 0x40000=256KB for F407) |

### Default required patterns

```text
ROBOTOS_OBS state=READY
ROBOTOS_FAULT active=0
ROBOTOS_PROD attempted=
Phase 6I final:
```

All are substring-matched (not regex). CFSR and HFSR values are additionally
checked: any non-zero value in a `cfsr=0x...` or `hfsr=0x...` field causes a
hard FAIL regardless of RequirePatterns.

### Expected output on PASS

```text
======================================================================
RobotOS Devkit RTT Streaming Capture (Phase 6O)
Date        : 2026-05-08 HH:MM:SS
Board       : stm32f411e_disco
WaitSeconds : 60
Port        : 9090
OutputLog   : RobotOS_v1.0\devkit\logs\phase_rtt_2026-05-08.txt
...
======================================================================
--- Pattern Verification ---
[OK]    FOUND    : ROBOTOS_OBS state=READY
[OK]    FOUND    : ROBOTOS_FAULT active=0
[OK]    FOUND    : ROBOTOS_PROD attempted=
[OK]    FOUND    : Phase 6I final:
[OK]    CFSR     : all 0x00000000 (13 occurrences checked)
[OK]    HFSR     : all 0x00000000 (13 occurrences checked)
======================================================================
Summary
  Output log  : RobotOS_v1.0\devkit\logs\phase_rtt_2026-05-08.txt
  File size   : ~16000 bytes
  Duration    : 60.0 s
  Patterns    : 4 required
======================================================================
[OK]    PASS -- all required patterns verified; CFSR/HFSR zero
```

### Troubleshooting

| Symptom | Cause | Fix |
|---|---|---|
| `OpenOCD not found` | openocd.exe not on PATH and not at xpack WinGet location | Install xpack-openocd via WinGet, or pass `-OpenOcdExe <path>` |
| `Board .cfg not found` | Default board cfg path points to a non-existent file | Pass `-OpenOcdConfig <path>` |
| `OpenOCD exited immediately` | ST-LINK not detected, wrong cfg, or port conflict | Check USB cable; review OpenOCD stderr printed by script |
| `TCP connect failed after 3 attempts` | OpenOCD started but RTT server not up yet, or board not running | Increase `Start-Sleep` to 5 s by editing script; or press physical RESET |
| `Capture produced no usable text` | Firmware not running (halted post-flash) | Run with `-FlashFirst`; or press physical RESET manually and rerun |
| `MISSING: ROBOTOS_OBS state=READY` | Boot banner present but OBS never emitted | Check DEVKIT_OBSERVABILITY_LOG_INTERVAL_TICKS; increase -WaitSeconds |
| `NON-ZERO CFSR` | Hardware fault occurred | Do NOT close phase; inspect raw log and RTT for fault context; report to team |
| `Port 9090 already in use` | Another OpenOCD is already running on that port | Use `-Port 9091` or stop the other OpenOCD manually |
| `_SEGGER_RTT not found in ELF` | Stale ELF or nm not found | Run with `-BuildFirst`, or pass `-RttControlBlock <addr>` |

---

## Why this exists

Agent-driven runtime evidence capture has failed repeatedly due to:
- Placeholder RTT text written by agents without real device reads
- Mixed PowerShell/Bash syntax in agent-generated commands
- Invalid OpenOCD invocations (missing `-f target/stm32f4x.cfg`, wrong flags)
- Handwritten GDB scripts with invalid syntax (`print (x/*xw *)symbol`)
- GUI-oriented runbooks that depend on SEGGER RTT Viewer being installed

This harness fixes all of these by providing a single repeatable PowerShell script
that agents (GLM, Copilot, Claude) must run instead of writing OpenOCD/GDB commands by hand.

---

## Files

| File | Purpose |
|---|---|
| `capture_phase6h_runtime.ps1` | Phase 6H runtime evidence capture (primary) |
| `phase6h_read.gdb` | GDB batch script used as automatic fallback |
| `README.md` | This file |

---

## Evidence methods (in priority order)

### Method A — OpenOCD RTT dump (primary)

Proven working in Phase 6G (`_SEGGER_RTT` dump at `0x20000a1c`, 4KB buffer, 3.5s wait).

How it works:
1. `arm-zephyr-eabi-nm` resolves `_SEGGER_RTT` address from current ELF (shifts each build)
2. OpenOCD resets the board and runs for `WaitSeconds` (default 8s)
3. OpenOCD halts and dumps 4096 bytes from the RTT ring buffer to a temp file
4. PowerShell decodes binary to ASCII, strips non-printable bytes
5. Script searches for required log lines and verifies

This is the **preferred method**. It does not require SEGGER J-Link software.

### Method B — GDB batch counter read (automatic fallback)

Used automatically when Method A produces no Phase 6H lines.

How it works:
1. OpenOCD starts as a GDB server (port 3333)
2. `arm-zephyr-eabi-gdb --batch` connects and runs `phase6h_read.gdb`
3. GDB runs `monitor after 8000` — OpenOCD's Tcl delays 8s while MCU runs, then halts
4. GDB prints volatile counter values directly from MCU RAM
5. GDB reads CFSR/HFSR fault registers
6. Script parses GDB output and verifies counters match expected values

### NOT used: SEGGER RTT Viewer (GUI)

SEGGER RTT Viewer requires J-Link software installation and manual interaction.
It is not the primary path for this repo. Use Method A or B above.

---

## Quick start

```powershell
# Already flashed — just capture:
cd D:\Robot_OS
.\RobotOS_v1.0\tools\runtime\capture_phase6h_runtime.ps1

# Build + flash + capture (clean workflow):
.\RobotOS_v1.0\tools\runtime\capture_phase6h_runtime.ps1 -Build -Flash

# Force GDB fallback (skip RTT dump):
.\RobotOS_v1.0\tools\runtime\capture_phase6h_runtime.ps1 -GdbFallback

# Longer wait if events not appearing (e.g. slower tick dispatch):
.\RobotOS_v1.0\tools\runtime\capture_phase6h_runtime.ps1 -WaitSeconds 12
```

Script exits 0 on PASS, 1 on FAIL or any error.

---

## Tool paths (from `build/zephyr/runners.yaml`)

| Tool | Path |
|---|---|
| OpenOCD | `C:\Users\ttpro\AppData\Local\Microsoft\WinGet\Packages\xpack-dev-tools.openocd-xpack_Microsoft.Winget.Source_8wekyb3d8bbwe\xpack-openocd-0.12.0-7\bin\openocd.exe` |
| OpenOCD scripts | `C:\Program Files\OpenOCD\share\openocd\scripts` |
| Board cfg | `D:\Robot_OS\zephyr\boards\arm\stm32f411e_disco\support\openocd.cfg` |
| GDB | `C:\zephyr-sdk-0.17.0\arm-zephyr-eabi\bin\arm-zephyr-eabi-gdb.exe` |
| nm | `C:\zephyr-sdk-0.17.0\arm-zephyr-eabi\bin\arm-zephyr-eabi-nm.exe` |
| ELF | `D:\Robot_OS\build\zephyr\zephyr.elf` |

---

## Phase 6H pass criteria

| Counter | Expected value |
|---|---|
| `s_timer_attempted_count` | 8 |
| `s_timer_ok_count` | 8 |
| `s_timer_full_count` | 0 |
| `s_timer_invalid_count` | 0 |
| `s_timer_other_error_count` | 0 |
| `s_timer_handled_count` | 8 |
| `s_timer_unexpected_count` | 0 |
| `s_timer_final_logged` | 1 (true) |
| CFSR @ 0xE000ED28 | 0x00000000 |
| HFSR @ 0xE000ED2C | 0x00000000 |

Required log lines (RTT method):
- `Phase 6H timer event handled seq=1`
- `Phase 6H timer event handled seq=4`
- `Phase 6H timer event handled seq=8`
- `Phase 6H final summary`
- `attempted=8 ok=8`
- `handled=8`

---

## Phase 6H timing rationale

| Event | Time |
|---|---|
| 8 events posted at 100ms intervals | t=0 to t=800ms |
| 8 ticks at 500ms each (budget=1/tick) | t=500ms to t=4000ms |
| Final summary log emitted after handled==8 | ~t=4500ms |
| Default wait | 8000ms (conservative) |

---

## Evidence commit policy

After PASS:

**Stage only these files:**
```
git add RobotOS_v1.0/devkit/logs/phase_6H_rtt_YYYY-MM-DD.txt
git add RobotOS_v1.0/devkit/docs/01_PROGRESS/DEVKIT_PROGRESS.md
```

**Never stage:**
- `rtt_buf.bin` (temp file, lives in %TEMP%)
- `*.elf`, `*.hex`, `*.bin`
- `build/` directory
- `.west/`, `zephyr/`, `modules/` directories
- Any temp file generated outside `devkit/logs/`

**Commit message format:**
```
tests: Phase 6H runtime confirmed -- ISR timer stress-lite

Counter evidence: attempted=8 ok=8 handled=8 dropped=0
CFSR=0x0 HFSR=0x0
```

**Do not update DEVKIT_PROGRESS.md until script exits 0 (PASS).**
**Do not fabricate counter values. If PASS cannot be reached, report to user.**

---

## Troubleshooting

| Symptom | Action |
|---|---|
| `_SEGGER_RTT not found in ELF` | ELF is stale — run with `-Build` |
| RTT dump: no Phase 6H lines | Try `-GdbFallback`; or increase `-WaitSeconds 12` |
| GDB: `Connection refused` | OpenOCD failed to start — check ST-LINK connection |
| GDB: symbol not found | ELF does not match flashed image — run with `-Build -Flash` |
| Counter mismatch (e.g. handled=6) | Dispatch budget or tick timing issue — investigate firmware |
| CFSR/HFSR non-zero | MCU faulted — do not claim PASS; report to user |
| `west flash failed` | Reconnect ST-LINK USB; try `west flash` manually first |
