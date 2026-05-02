#Requires -Version 5.1
<#
.SYNOPSIS
    Phase 6H runtime evidence capture harness.

.DESCRIPTION
    Captures runtime counter evidence from the STM32F411E-DISCO via OpenOCD.

    Primary method  : dump_image at _SEGGER_RTT address (proven in Phase 6G).
    Fallback method : GDB batch counter read via OpenOCD GDB server.

    Evidence is saved to:
        RobotOS_v1.0\devkit\logs\phase_6H_rtt_YYYY-MM-DD.txt

    Script exits 0 on PASS, 1 on FAIL or tool/connection error.
    Never writes placeholder text. Never commits automatically.

.PARAMETER Build
    Run 'west build' before capture (pristine build).
.PARAMETER Flash
    Run 'west flash' before capture.
.PARAMETER GdbFallback
    Skip RTT dump; go directly to GDB batch counter read.
.PARAMETER WaitSeconds
    Seconds after reset before halting (default 8).
    Phase 6H: 8 events x 100ms post + 8 ticks x 500ms dispatch = ~5s total.
    8s is a conservative safe margin.

.EXAMPLE
    # Already flashed, just capture:
    .\capture_phase6h_runtime.ps1

    # Build + flash + capture:
    .\capture_phase6h_runtime.ps1 -Build -Flash

    # Force GDB fallback if RTT decode finds no Phase 6H lines:
    .\capture_phase6h_runtime.ps1 -GdbFallback
#>
param(
    [switch]$Build,
    [switch]$Flash,
    [switch]$GdbFallback,
    [int]$WaitSeconds = 8
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

# ---------------------------------------------------------------------------
# Canonical paths (derived from D:\Robot_OS\build\zephyr\runners.yaml)
# ---------------------------------------------------------------------------
$RepoRoot       = 'D:\Robot_OS'
$BuildElf       = "$RepoRoot\build\zephyr\zephyr.elf"
$DevkitSrc      = "$RepoRoot\RobotOS_v1.0\devkit"
$LogDir         = "$RepoRoot\RobotOS_v1.0\devkit\logs"
$TempDir        = "$env:TEMP\RobotOS_runtime"
$RttBinTemp     = "$TempDir\phase6h_rtt_buf.bin"
$GdbTempScript  = "$TempDir\phase6h_read_runtime.gdb"
$LogDate        = Get-Date -Format 'yyyy-MM-dd'
$LogFile        = "$LogDir\phase_6H_rtt_$LogDate.txt"

$OpenOcdExe     = 'C:\Users\ttpro\AppData\Local\Microsoft\WinGet\Packages\xpack-dev-tools.openocd-xpack_Microsoft.Winget.Source_8wekyb3d8bbwe\xpack-openocd-0.12.0-7\bin\openocd.exe'
$OpenOcdScripts = 'C:\Users\ttpro\AppData\Local\Microsoft\WinGet\Packages\xpack-dev-tools.openocd-xpack_Microsoft.Winget.Source_8wekyb3d8bbwe\xpack-openocd-0.12.0-7\openocd\scripts'
$BoardCfg       = "$RepoRoot\zephyr\boards\arm\stm32f411e_disco\support\openocd.cfg"
$GdbExe         = 'C:\zephyr-sdk-0.17.0\arm-zephyr-eabi\bin\arm-zephyr-eabi-gdb.exe'
$NmExe          = 'C:\zephyr-sdk-0.17.0\arm-zephyr-eabi\bin\arm-zephyr-eabi-nm.exe'

# ---------------------------------------------------------------------------
# Phase 6H pass criteria
# Order must match GDB script print order exactly
# ---------------------------------------------------------------------------
$CounterNames    = @('attempted','ok','full','invalid','other','handled','unexpected','final_logged')
$CounterExpected = @(         8,   8,    0,       0,     0,       8,          0,             1    )

$RequiredRttLines = @(
    'Phase 6H timer event handled seq=1',
    'Phase 6H timer event handled seq=4',
    'Phase 6H timer event handled seq=8',
    'Phase 6H final summary',
    'attempted=8 ok=8',
    'handled=8'
)

# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------
function Write-Status { param([string]$Msg) Write-Host "[INFO]  $Msg" }
function Write-Ok     { param([string]$Msg) Write-Host "[OK]    $Msg" -ForegroundColor Green }
function Write-Fail   { param([string]$Msg) Write-Host "[FAIL]  $Msg" -ForegroundColor Red }
function Write-Warn   { param([string]$Msg) Write-Host "[WARN]  $Msg" -ForegroundColor Yellow }
function Write-Sep    { Write-Host ('=' * 60) }

function Assert-Tool {
    param([string]$Path, [string]$Label)
    if (-not (Test-Path $Path)) {
        Write-Fail "$Label not found: $Path"
        exit 1
    }
    Write-Ok "$Label found"
}

function Stop-StaleOpenOcd {
    $procs = Get-Process -Name 'openocd' -ErrorAction SilentlyContinue
    if ($procs) {
        Write-Status "Stopping $($procs.Count) stale openocd process(es)..."
        $procs | Stop-Process -Force
        Start-Sleep -Milliseconds 800
    }
}

# ---------------------------------------------------------------------------
# 1. Announce
# ---------------------------------------------------------------------------
Write-Sep
Write-Status "Phase 6H Runtime Evidence Capture"
Write-Status "Date       : $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')"
Write-Status "Wait       : ${WaitSeconds}s after reset"
Write-Status "Log target : $LogFile"
Write-Sep

# ---------------------------------------------------------------------------
# 2. Verify tools
# ---------------------------------------------------------------------------
Write-Status '--- Tool verification ---'
Assert-Tool $OpenOcdExe     'OpenOCD exe'
Assert-Tool $OpenOcdScripts 'OpenOCD scripts'
Assert-Tool $BoardCfg       'Board openocd.cfg'
Assert-Tool $GdbExe         'arm-zephyr-eabi-gdb'
Assert-Tool $NmExe          'arm-zephyr-eabi-nm'

# Ensure temp dir exists
if (-not (Test-Path $TempDir)) {
    New-Item -ItemType Directory -Path $TempDir | Out-Null
}

# ---------------------------------------------------------------------------
# 3. Optional build
# ---------------------------------------------------------------------------
if ($Build) {
    Write-Status ''
    Write-Status '--- Build (west build --pristine) ---'
    Push-Location $RepoRoot
    try {
        & west build -b stm32f411e_disco $DevkitSrc --pristine
        if ($LASTEXITCODE -ne 0) {
            Write-Fail "west build failed (exit $LASTEXITCODE)"
            exit 1
        }
        Write-Ok 'west build passed'
    } finally {
        Pop-Location
    }
}

# ---------------------------------------------------------------------------
# 4. Verify ELF exists
# ---------------------------------------------------------------------------
Assert-Tool $BuildElf 'ELF file'

# ---------------------------------------------------------------------------
# 5. Optional flash
# ---------------------------------------------------------------------------
if ($Flash) {
    Write-Status ''
    Write-Status '--- Flash (west flash) ---'
    Stop-StaleOpenOcd
    Push-Location $RepoRoot
    try {
        & west flash
        if ($LASTEXITCODE -ne 0) {
            Write-Fail "west flash failed (exit $LASTEXITCODE)"
            exit 1
        }
        Write-Ok 'west flash passed'
        Start-Sleep -Seconds 1
    } finally {
        Pop-Location
    }
}

# ---------------------------------------------------------------------------
# 6. Resolve _SEGGER_RTT address dynamically from ELF via nm
#    Address shifts each build -- must not be hardcoded
# ---------------------------------------------------------------------------
Write-Status ''
Write-Status '--- Resolving _SEGGER_RTT symbol ---'

$nmLines  = & $NmExe $BuildElf 2>$null
$rttMatch = $nmLines | Select-String '_SEGGER_RTT' | Select-Object -First 1
if (-not $rttMatch) {
    Write-Fail '_SEGGER_RTT not found in ELF symbol table'
    Write-Status 'nm output sample (SEGGER/RTT matches):'
    $nmLines | Select-String -Pattern 'SEGGER|segger|rtt|RTT' | Select-Object -First 10 | ForEach-Object {
        Write-Status "  $_"
    }
    exit 1
}
# nm format: "20000abc D _SEGGER_RTT"  -- first field is hex address
$rttHex     = $rttMatch.ToString().Trim().Split(' ')[0]
$rttAddress = "0x$rttHex"
Write-Ok "_SEGGER_RTT = $rttAddress"

# ---------------------------------------------------------------------------
# 7. PRIMARY: OpenOCD dump_image of RTT ring buffer
#    Uses "after <ms>" -- OpenOCD Tcl built-in that blocks while MCU runs
# ---------------------------------------------------------------------------
$captureOk     = $false
$capturedText  = ''
$captureMethod = 'RTT-dump'

if (-not $GdbFallback) {
    Write-Status ''
    Write-Status '--- PRIMARY: OpenOCD RTT dump ---'
    Write-Status "addr=$rttAddress  size=4096 bytes  wait=${WaitSeconds}s"
    Stop-StaleOpenOcd
    if (Test-Path $RttBinTemp) { Remove-Item $RttBinTemp -Force }

    $waitMs = $WaitSeconds * 1000

    & $OpenOcdExe `
        -s $OpenOcdScripts `
        -f $BoardCfg `
        -c 'init' `
        -c 'reset run' `
        -c "after $waitMs" `
        -c 'halt' `
        -c "dump_image `"$RttBinTemp`" $rttAddress 4096" `
        -c 'reset run' `
        -c 'shutdown'

    if ($LASTEXITCODE -ne 0) {
        Write-Warn "OpenOCD exit $LASTEXITCODE -- checking for dump file anyway"
    }

    if (Test-Path $RttBinTemp) {
        $dumpSize = (Get-Item $RttBinTemp).Length
        Write-Status "Dump file size: $dumpSize bytes"
        if ($dumpSize -gt 0) {
            $rawBytes     = [System.IO.File]::ReadAllBytes($RttBinTemp)
            $capturedText = [System.Text.Encoding]::ASCII.GetString($rawBytes) `
                            -replace '[^\x09\x0A\x0D\x20-\x7E]', ''
            Remove-Item $RttBinTemp -Force -ErrorAction SilentlyContinue
            if ($capturedText -match 'Phase 6H') {
                Write-Ok 'RTT decode OK -- Phase 6H lines present'
                $captureOk = $true
            } else {
                Write-Warn 'RTT decoded but no "Phase 6H" lines found'
                Write-Status "Text sample (first 300 chars): $($capturedText.Substring(0, [Math]::Min(300, $capturedText.Length)))"
                Write-Warn 'Falling through to GDB fallback'
            }
        } else {
            Write-Warn 'Dump file is 0 bytes -- falling through to GDB fallback'
        }
    } else {
        Write-Warn 'No dump file created -- falling through to GDB fallback'
    }
}

# ---------------------------------------------------------------------------
# 8. FALLBACK: GDB batch counter read via OpenOCD GDB server
#    "monitor after <ms>" blocks inside OpenOCD Tcl while MCU runs
# ---------------------------------------------------------------------------
if (-not $captureOk) {
    Write-Status ''
    Write-Status '--- FALLBACK: GDB batch counter read ---'
    $captureMethod = 'GDB-counter'
    Stop-StaleOpenOcd

    $waitMs = $WaitSeconds * 1000

    $gdbScript = @"
target remote localhost:3333
monitor reset run
monitor after $waitMs
monitor halt
print (unsigned int)s_timer_attempted_count
print (unsigned int)s_timer_ok_count
print (unsigned int)s_timer_full_count
print (unsigned int)s_timer_invalid_count
print (unsigned int)s_timer_other_error_count
print (unsigned int)s_timer_handled_count
print (unsigned int)s_timer_unexpected_count
print (int)s_timer_final_logged
x/1xw 0xE000ED28
x/1xw 0xE000ED2C
monitor reset run
quit
"@
    $gdbScript | Set-Content -Path $GdbTempScript -Encoding ASCII

    # Start OpenOCD as GDB server (background)
    $ocdProc = Start-Process `
        -FilePath $OpenOcdExe `
        -ArgumentList @('-s', $OpenOcdScripts, '-f', $BoardCfg) `
        -WindowStyle Hidden `
        -PassThru
    Write-Status "OpenOCD GDB server started (pid=$($ocdProc.Id))"
    Start-Sleep -Seconds 2

    try {
        # Capture GDB stdout only; stderr flows to console for diagnostics
        $gdbLines     = & $GdbExe --batch -x $GdbTempScript $BuildElf
        $capturedText = $gdbLines -join "`n"
        Write-Ok "GDB transcript: $($gdbLines.Count) line(s)"

        if ($capturedText -match '= \d') {
            $captureOk = $true
        } else {
            Write-Warn 'GDB transcript does not contain expected print output'
            Write-Status "Transcript: $capturedText"
        }
    } finally {
        if (-not $ocdProc.HasExited) {
            $ocdProc | Stop-Process -Force
            Write-Status 'OpenOCD GDB server stopped'
        }
        Remove-Item $GdbTempScript -Force -ErrorAction SilentlyContinue
    }
}

# ---------------------------------------------------------------------------
# 9. Hard fail if both methods returned nothing usable
# ---------------------------------------------------------------------------
if (-not $captureOk) {
    Write-Sep
    Write-Fail 'CAPTURE FAILED -- both RTT dump and GDB fallback returned no usable data'
    Write-Fail 'Do NOT update DEVKIT_PROGRESS.md. Report failure to user.'
    Write-Fail ''
    Write-Fail 'Possible causes:'
    Write-Fail '  - Board not connected or not flashed with Phase 6H firmware'
    Write-Fail '  - ST-LINK driver issue (try: west flash manually first)'
    Write-Fail '  - ELF does not match flashed image (run with -Build -Flash)'
    Write-Fail '  - Try -WaitSeconds 12 if dispatch is slower than expected'
    exit 1
}

# ---------------------------------------------------------------------------
# 10. Parse and verify pass criteria
# ---------------------------------------------------------------------------
Write-Status ''
Write-Status '--- Pass criteria verification ---'
$allPassed = $true

if ($captureMethod -eq 'RTT-dump') {
    foreach ($line in $RequiredRttLines) {
        if ($capturedText -match [regex]::Escape($line)) {
            Write-Ok "FOUND   : $line"
        } else {
            Write-Fail "MISSING : $line"
            $allPassed = $false
        }
    }
    Write-Warn 'CFSR/HFSR not read via RTT method -- use -GdbFallback to read fault registers'

} else {
    # GDB path: parse "$N = <value>" lines in print order
    $printLines = ($capturedText -split "`n") | Where-Object { $_ -match '^\$\d+\s*=' }
    $observed   = @()
    foreach ($pl in $printLines) {
        if ($pl -match '=\s*(\d+)') {
            $observed += [int]$Matches[1]
        }
    }
    Write-Status "Parsed $($observed.Count) counter value(s) from GDB transcript"

    if ($observed.Count -ge 8) {
        for ($i = 0; $i -lt 8; $i++) {
            $name = $CounterNames[$i]
            $obs  = $observed[$i]
            $exp  = $CounterExpected[$i]
            if ($obs -eq $exp) {
                Write-Ok "${name} = ${obs}"
            } else {
                Write-Fail "${name} = ${obs}  (expected ${exp})"
                $allPassed = $false
            }
        }
    } else {
        Write-Fail "Only $($observed.Count) values parsed from GDB (need 8)"
        Write-Status 'Raw print lines from transcript:'
        $printLines | ForEach-Object { Write-Status "  $_" }
        $allPassed = $false
    }

    # Parse CFSR/HFSR from "x/1xw" output
    # GDB format: "0xe000ed28 <SCB_CFSR>:  0x00000000"
    $allLines = $capturedText -split "`n"
    $cfsrLine = $allLines | Where-Object { $_ -match '(?i)e000ed28' } | Select-Object -First 1
    $hfsrLine = $allLines | Where-Object { $_ -match '(?i)e000ed2c' } | Select-Object -First 1

    foreach ($entry in @(@{Line=$cfsrLine; Name='CFSR'}, @{Line=$hfsrLine; Name='HFSR'})) {
        $val = $null
        if ($entry.Line -and $entry.Line -match ':\s+(0x[0-9a-fA-F]+)') {
            $val = $Matches[1]
        }
        if ($val) {
            if ($val -eq '0x00000000') {
                Write-Ok "$($entry.Name) = $val"
            } else {
                Write-Fail "$($entry.Name) = $val  (expected 0x00000000 -- FAULT PRESENT)"
                $allPassed = $false
            }
        } else {
            Write-Warn "$($entry.Name) not parsed from transcript"
        }
    }
}

# ---------------------------------------------------------------------------
# 11. Save evidence log
# ---------------------------------------------------------------------------
Write-Status ''
Write-Status '--- Saving evidence log ---'
if (-not (Test-Path $LogDir)) {
    New-Item -ItemType Directory -Path $LogDir | Out-Null
}

$header = @"
Phase 6H -- ISR/Timer Producer Stress-Lite
RTT Boot Log -- $LogDate
Board: STM32F411E-DISCO
Capture method: $captureMethod
_SEGGER_RTT address: $rttAddress (4KB buffer)
Wait: ${WaitSeconds}s
Script: capture_phase6h_runtime.ps1

===========================================================================

"@

($header + $capturedText) | Set-Content -Path $LogFile -Encoding UTF8
Write-Ok "Evidence saved: $LogFile"

# ---------------------------------------------------------------------------
# 12. Final verdict
# ---------------------------------------------------------------------------
Write-Status ''
Write-Sep
if ($allPassed) {
    Write-Ok 'PHASE 6H PASS -- all counters verified'
    Write-Sep
    Write-Status ''
    Write-Status 'Commit instructions (run manually after reviewing log):'
    Write-Status "  git add RobotOS_v1.0/devkit/logs/phase_6H_rtt_$LogDate.txt"
    Write-Status '  git add RobotOS_v1.0/DEVKIT_PROGRESS.md   (after updating Phase 6H status)'
    Write-Status '  git commit -m "tests: Phase 6H runtime confirmed -- ISR timer stress-lite"'
    Write-Status ''
    Write-Status 'DO NOT stage: rtt_buf.bin, *.elf, *.hex, build/, .west/, zephyr/, modules/'
    exit 0
} else {
    Write-Fail 'PHASE 6H FAIL -- one or more criteria not met'
    Write-Fail "Diagnostic log: $LogFile"
    Write-Fail 'Do NOT mark Phase 6H CLOSED. Do NOT update DEVKIT_PROGRESS.md.'
    Write-Status ''
    Write-Status 'Troubleshooting:'
    Write-Status '  - Run with -Build -Flash if firmware may be stale'
    Write-Status '  - Run with -WaitSeconds 12 if events are dispatching slowly'
    Write-Status '  - Run with -GdbFallback to read counters directly'
    Write-Sep
    exit 1
}

