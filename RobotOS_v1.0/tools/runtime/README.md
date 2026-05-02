# RobotOS Runtime Evidence Harness

Canonical runtime evidence capture workflow for RobotOS phases on STM32F411E-DISCO.

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
git add RobotOS_v1.0/DEVKIT_PROGRESS.md
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
