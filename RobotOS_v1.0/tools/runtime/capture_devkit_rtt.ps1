#Requires -Version 5.1
<#
.SYNOPSIS
    RobotOS devkit RTT streaming evidence capture harness (Phase 6O).

.DESCRIPTION
    Captures RTT telemetry from a running RobotOS devkit firmware via OpenOCD
    TCP streaming. Suitable for 30-60 s captures that include ROBOTOS_OBS /
    ROBOTOS_FAULT / ROBOTOS_PROD periodic triplets without ring-buffer wraparound.

    Difference from capture_phase6h_runtime.ps1:
      - Uses OpenOCD 'rtt server start' TCP streaming instead of a one-shot
        4 KB dump_image. Prevents ring-buffer wraparound during long captures
        and preserves the full boot sequence and all periodic telemetry triplets.
      - capture_phase6h_runtime.ps1 remains the correct tool for Phase 6H
        counter-comparison validation (attempted=8 ok=8 handled=8).
        Do NOT replace it with this script.

    Lifecycle:
      1. Optionally builds (west build --pristine) and flashes (west flash).
      2. Resolves _SEGGER_RTT address from the ELF via arm-zephyr-eabi-nm.
      3. Starts OpenOCD with a sidecar .cfg that issues 'reset run',
         sets up RTT at the resolved address, and starts an RTT TCP server.
      4. Streams RTT text from TCP to OutputLog for WaitSeconds.
      5. Verifies RequirePatterns and checks CFSR/HFSR for non-zero values.
      6. Exits 0 on PASS, 1 on FAIL; raw log is preserved in both cases.
      7. Stops only the OpenOCD process started by this script.

    Does NOT modify firmware source, log formats, or telemetry cadence.

.PARAMETER OutputLog
    Path to save the raw RTT capture. If omitted, defaults to:
    <repo_root>/RobotOS_v1.0/devkit/logs/phase_rtt_YYYY-MM-DD.txt

.PARAMETER WaitSeconds
    Streaming capture window in seconds. Default: 60.
    At DEVKIT_TICK_MS=500ms: 60s yields ~12 periodic OBS/FAULT/PROD triplets.

.PARAMETER Port
    TCP port for the OpenOCD RTT server. Default: 9090.

.PARAMETER Board
    Board name for metadata and west build. Default: stm32f411e_disco.

.PARAMETER BuildFirst
    Run 'west build -b <Board> RobotOS_v1.0/devkit/ --pristine' before capture.

.PARAMETER FlashFirst
    Run 'west flash' before capture. Kills stale OpenOCD first.

.PARAMETER RequirePatterns
    Literal strings that must ALL appear in the captured log.
    Default: "ROBOTOS_OBS state=READY", "ROBOTOS_FAULT active=0",
             "ROBOTOS_PROD attempted=", "Phase 6I final:"

.PARAMETER OpenOcdExe
    Full path to openocd.exe. Auto-discovered if omitted.

.PARAMETER OpenOcdScripts
    OpenOCD scripts directory. Auto-derived from OpenOcdExe location if omitted.

.PARAMETER OpenOcdConfig
    Board-specific OpenOCD .cfg file. Default: stm32f411e_disco board cfg.
    Override for Phase 8A custom board migration.

.PARAMETER NmExe
    arm-zephyr-eabi-nm.exe path. Auto-derived from SDK if omitted.

.PARAMETER ElfPath
    Firmware ELF. Default: <repo_root>/build/zephyr/zephyr.elf

.PARAMETER RttControlBlock
    Optional explicit RTT address (e.g. "0x20000a34"). Skips nm when provided.

.PARAMETER RttSearchBase
    RAM base for rtt setup search fallback. Default: 0x20000000.

.PARAMETER RttSearchSize
    RAM range size for rtt setup search fallback. Default: 0x20000 (128 KB).
    Use 0x40000 for STM32F407 (192 KB SRAM).

.EXAMPLE
    # Already flashed - capture only:
    cd D:\Robot_OS
    .\RobotOS_v1.0\tools\runtime\capture_devkit_rtt.ps1

.EXAMPLE
    # Build + flash + capture:
    .\RobotOS_v1.0\tools\runtime\capture_devkit_rtt.ps1 -BuildFirst -FlashFirst

.EXAMPLE
    # Quick 30s smoke, custom log path:
    .\RobotOS_v1.0\tools\runtime\capture_devkit_rtt.ps1 `
        -OutputLog RobotOS_v1.0\devkit\logs\phase_6O_smoke_2026-05-08.txt `
        -WaitSeconds 30

.EXAMPLE
    # Phase 8A custom board - override config + search range:
    .\RobotOS_v1.0\tools\runtime\capture_devkit_rtt.ps1 `
        -OpenOcdConfig D:\my_project\openocd_f407.cfg `
        -RttSearchBase 0x20000000 -RttSearchSize 0x40000 `
        -RequirePatterns @("ROBOTOS_OBS state=READY","ROBOTOS_FAULT active=0")

.NOTES
    Source:  RobotOS_v1.0/tools/runtime/capture_devkit_rtt.ps1
    Phase:   6O - Reusable RTT Streaming Capture Harness (2026-05-08)
    Proven:  Phase 6Z streaming method (60s, 16560 bytes, PASS on STM32F411E-DISCO)
#>

[CmdletBinding()]
param(
    [string]   $OutputLog       = "",
    [int]      $WaitSeconds     = 60,
    [int]      $Port            = 9090,
    [string]   $Board           = "stm32f411e_disco",
    [switch]   $BuildFirst,
    [switch]   $FlashFirst,
    [string[]] $RequirePatterns = @(
        "ROBOTOS_OBS state=READY",
        "ROBOTOS_FAULT active=0",
        "ROBOTOS_PROD attempted=",
        "Phase 6I final:"
    ),
    [string]   $OpenOcdExe      = "",
    [string]   $OpenOcdScripts  = "",
    [string]   $OpenOcdConfig   = "",
    [string]   $NmExe           = "",
    [string]   $ElfPath         = "",
    [string]   $RttControlBlock = "",
    [string]   $RttSearchBase   = "0x20000000",
    [string]   $RttSearchSize   = "0x20000"
)

Set-StrictMode  -Version Latest
$ErrorActionPreference = 'Continue'

# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------
function Write-Info { param([string]$M) Write-Host "[INFO]  $M" }
function Write-Ok   { param([string]$M) Write-Host "[OK]    $M" -ForegroundColor Green  }
function Write-Fail { param([string]$M) Write-Host "[FAIL]  $M" -ForegroundColor Red    }
function Write-Warn { param([string]$M) Write-Host "[WARN]  $M" -ForegroundColor Yellow }
function Write-Sep  {                   Write-Host ('=' * 70)                            }

# ---------------------------------------------------------------------------
# Locate repo root (script lives at tools/runtime/ -> root is 3 levels up)
# ---------------------------------------------------------------------------
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$RepoRoot  = $null
try {
    $resolved = Resolve-Path (Join-Path $ScriptDir '..\..\..') -ErrorAction Stop
    $RepoRoot = $resolved.Path
} catch {
    $RepoRoot = (Get-Location).Path
    Write-Warn "Repo root not resolved from script path; using cwd: $RepoRoot"
}

# ---------------------------------------------------------------------------
# Resolve defaults
# ---------------------------------------------------------------------------
$LogDate = Get-Date -Format 'yyyy-MM-dd'
if (-not $OutputLog) {
    $OutputLog = Join-Path $RepoRoot "RobotOS_v1.0\devkit\logs\phase_rtt_$LogDate.txt"
}
if (-not $OpenOcdConfig) {
    $OpenOcdConfig = Join-Path $RepoRoot "zephyr\boards\arm\stm32f411e_disco\support\openocd.cfg"
}
if (-not $ElfPath) {
    $ElfPath = Join-Path $RepoRoot "build\zephyr\zephyr.elf"
}

# ---------------------------------------------------------------------------
# Discover OpenOCD executable
# ---------------------------------------------------------------------------
if (-not $OpenOcdExe) {
    $onPath = Get-Command openocd -ErrorAction SilentlyContinue
    if ($onPath) {
        $OpenOcdExe = $onPath.Source
    } else {
        $xpackGlob = Join-Path $env:LOCALAPPDATA `
            "Microsoft\WinGet\Packages\xpack-dev-tools.openocd-xpack_Microsoft.Winget.Source_8wekyb3d8bbwe\*\bin\openocd.exe"
        $hit = Get-ChildItem $xpackGlob -ErrorAction SilentlyContinue |
               Sort-Object Name -Descending | Select-Object -First 1
        if ($hit) { $OpenOcdExe = $hit.FullName }
    }
}
if (-not $OpenOcdExe -or -not (Test-Path $OpenOcdExe)) {
    Write-Fail "OpenOCD not found. Provide -OpenOcdExe or add openocd to PATH."
    Write-Info "Searched: PATH, %LOCALAPPDATA%\Microsoft\WinGet\Packages\xpack-dev-tools.openocd-xpack_*"
    exit 1
}

# Derive scripts directory from OpenOCD exe location (xpack: bin/../openocd/scripts)
if (-not $OpenOcdScripts) {
    $binDir     = Split-Path -Parent $OpenOcdExe
    $xpackScrip = Join-Path (Split-Path -Parent $binDir) "openocd\scripts"
    $sysScrip   = "C:\Program Files\OpenOCD\share\openocd\scripts"
    if      (Test-Path $xpackScrip) { $OpenOcdScripts = $xpackScrip }
    elseif  (Test-Path $sysScrip)   { $OpenOcdScripts = $sysScrip   }
}

# Discover nm from Zephyr SDK
if (-not $NmExe) {
    $sdkBase = if ($env:ZEPHYR_SDK_INSTALL_DIR) { $env:ZEPHYR_SDK_INSTALL_DIR } else { "C:\zephyr-sdk-0.17.0" }
    $NmExe   = Join-Path $sdkBase "arm-zephyr-eabi\bin\arm-zephyr-eabi-nm.exe"
}

# ---------------------------------------------------------------------------
# Announce
# ---------------------------------------------------------------------------
Write-Sep
Write-Info "RobotOS Devkit RTT Streaming Capture (Phase 6O)"
Write-Info "Date        : $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')"
Write-Info "Board       : $Board"
Write-Info "WaitSeconds : $WaitSeconds"
Write-Info "Port        : $Port"
Write-Info "OutputLog   : $OutputLog"
Write-Info "OpenOCD     : $OpenOcdExe"
Write-Info "Board cfg   : $OpenOcdConfig"
Write-Sep

# ---------------------------------------------------------------------------
# Verify required tools
# ---------------------------------------------------------------------------
Write-Info "--- Tool verification ---"
$toolsOk = $true
foreach ($check in @(
    @{P=$OpenOcdExe;    L="OpenOCD exe"    }
    @{P=$OpenOcdConfig; L="Board .cfg"     }
)) {
    if (Test-Path $check.P) {
        Write-Ok "$($check.L): $($check.P)"
    } else {
        Write-Fail "$($check.L) not found: $($check.P)"
        $toolsOk = $false
    }
}
if ($OpenOcdScripts) {
    if (Test-Path $OpenOcdScripts) { Write-Ok "OpenOCD scripts: $OpenOcdScripts" }
    else { Write-Warn "OpenOCD scripts not found: $OpenOcdScripts (OpenOCD may fail)" }
}
if (-not $toolsOk) { exit 1 }

# ---------------------------------------------------------------------------
# Optional: build
# ---------------------------------------------------------------------------
if ($BuildFirst) {
    Write-Info ""
    Write-Info "--- west build --pristine ---"
    if (-not $env:ZEPHYR_TOOLCHAIN_VARIANT) { $env:ZEPHYR_TOOLCHAIN_VARIANT = "zephyr" }
    if (-not $env:ZEPHYR_SDK_INSTALL_DIR) {
        $sdkGuess = Split-Path -Parent (Split-Path -Parent (Split-Path -Parent $NmExe))
        $env:ZEPHYR_SDK_INSTALL_DIR = $sdkGuess
    }
    Push-Location $RepoRoot
    & west build -b $Board "RobotOS_v1.0/devkit/" --pristine
    $buildExit = $LASTEXITCODE
    Pop-Location
    if ($buildExit -ne 0) { Write-Fail "west build failed (exit $buildExit)"; exit 1 }
    Write-Ok "west build passed"
}

# Verify ELF
if (-not (Test-Path $ElfPath)) {
    Write-Fail "ELF not found: $ElfPath"
    Write-Info "Run with -BuildFirst or build manually first."
    exit 1
}
Write-Ok "ELF: $ElfPath"

# ---------------------------------------------------------------------------
# Optional: flash
# ---------------------------------------------------------------------------
if ($FlashFirst) {
    Write-Info ""
    Write-Info "--- west flash ---"
    $stale = Get-Process -Name openocd -ErrorAction SilentlyContinue
    if ($stale) {
        Write-Info "Stopping $($stale.Count) stale openocd process(es) before flash"
        $stale | Stop-Process -Force
        Start-Sleep -Milliseconds 800
    }
    Push-Location $RepoRoot
    & west flash
    $flashExit = $LASTEXITCODE
    Pop-Location
    if ($flashExit -ne 0) { Write-Fail "west flash failed (exit $flashExit)"; exit 1 }
    Write-Ok "west flash passed"
    Start-Sleep -Seconds 1
}

# ---------------------------------------------------------------------------
# Resolve _SEGGER_RTT address
# ---------------------------------------------------------------------------
Write-Info ""
Write-Info "--- Resolving RTT control block ---"

$rttSetupLine = $null

if ($RttControlBlock) {
    Write-Info "Using explicit -RttControlBlock: $RttControlBlock"
    $rttSetupLine = "rtt setup $RttControlBlock 64 `"SEGGER RTT`""
    Write-Ok "RTT setup: exact address $RttControlBlock"
} elseif (Test-Path $NmExe) {
    $nmOut    = & $NmExe $ElfPath 2>$null
    $rttMatch = $nmOut | Select-String -Pattern '\s_SEGGER_RTT$' | Select-Object -First 1
    if ($rttMatch) {
        $rttHex  = ($rttMatch.ToString().Trim() -split '\s+')[0]
        $rttAddr = "0x$rttHex"
        Write-Ok "_SEGGER_RTT at $rttAddr (from ELF via nm)"
        $rttSetupLine = "rtt setup $rttAddr 64 `"SEGGER RTT`""
    } else {
        Write-Warn "_SEGGER_RTT not found in ELF by nm; using search range"
    }
} else {
    Write-Warn "nm not found at $NmExe; using search range"
}

if (-not $rttSetupLine) {
    Write-Info "Using RAM search: base=$RttSearchBase size=$RttSearchSize"
    $rttSetupLine = "rtt setup $RttSearchBase $RttSearchSize `"SEGGER RTT`""
}

# ---------------------------------------------------------------------------
# Write OpenOCD sidecar .cfg (avoids Start-Process argument-quoting issues)
# ---------------------------------------------------------------------------
$TempDir    = "$env:TEMP\RobotOS_rtt_capture"
if (-not (Test-Path $TempDir)) { New-Item -ItemType Directory -Path $TempDir | Out-Null }
$SidecarCfg = "$TempDir\rtt_stream.cfg"
$OcdOut     = "$TempDir\ocd.out"
$OcdErr     = "$TempDir\ocd.err"

$sidecarContent = "init`nreset run`n$rttSetupLine`nrtt start`nrtt server start $Port 0`n"
[System.IO.File]::WriteAllText($SidecarCfg, $sidecarContent, [System.Text.Encoding]::ASCII)
Write-Ok "Sidecar cfg: init; reset run; $rttSetupLine; rtt start; rtt server start $Port 0"

# ---------------------------------------------------------------------------
# Warn about existing OpenOCD (do NOT kill - we only clean up what we start)
# ---------------------------------------------------------------------------
$preExistingOcd = Get-Process -Name openocd -ErrorAction SilentlyContinue
if ($preExistingOcd) {
    Write-Warn "$($preExistingOcd.Count) openocd process(es) already running."
    Write-Warn "If port $Port is in use, rerun with -Port <other_port>."
    Write-Warn "This script cleans up only the OpenOCD process it starts."
}

foreach ($f in @($OcdOut, $OcdErr)) {
    if (Test-Path $f) { Remove-Item $f -Force }
}

# ---------------------------------------------------------------------------
# Start OpenOCD
# ---------------------------------------------------------------------------
Write-Info ""
Write-Info "--- Starting OpenOCD RTT server on port $Port ---"
$ocdArgList = @('-s', $OpenOcdScripts, '-f', $OpenOcdConfig, '-f', $SidecarCfg)
$ocdProc = Start-Process -FilePath $OpenOcdExe -ArgumentList $ocdArgList `
           -PassThru -WindowStyle Hidden `
           -RedirectStandardOutput $OcdOut -RedirectStandardError $OcdErr
Write-Info "OpenOCD started (pid=$($ocdProc.Id))"

Start-Sleep -Seconds 3

if ($ocdProc.HasExited) {
    Write-Fail "OpenOCD exited early (code=$($ocdProc.ExitCode))"
    Write-Info "OpenOCD stderr:"
    Get-Content $OcdErr -ErrorAction SilentlyContinue |
        ForEach-Object { Write-Info "  $_" }
    exit 1
}

# ---------------------------------------------------------------------------
# Connect to RTT TCP server (retry up to 3 times, 2s between)
# ---------------------------------------------------------------------------
Write-Info "--- Connecting to RTT TCP server (localhost:$Port) ---"

$tcpClient  = $null
$tcpStream  = $null
$fileStream = $null
$connected  = $false

for ($attempt = 1; $attempt -le 3; $attempt++) {
    if ($attempt -gt 1) { Start-Sleep -Seconds 2 }
    try {
        $tcpClient = New-Object System.Net.Sockets.TcpClient
        $ctask = $tcpClient.ConnectAsync('127.0.0.1', $Port)
        if ($ctask.Wait(3000)) {
            $connected = $true
            break
        }
    } catch {
        $null = $_
    }
    if ($tcpClient) {
        try { $tcpClient.Close() } catch { $null = $_ }
        $tcpClient = $null
    }
}

if (-not $connected) {
    Write-Fail "Cannot connect to OpenOCD RTT server on localhost:$Port (3 attempts, 3s each)"
    Write-Info "--- OpenOCD stderr (last 30 lines) ---"
    Get-Content $OcdErr -ErrorAction SilentlyContinue -Tail 30 |
        ForEach-Object { Write-Info "  $_" }
    Write-Info "Troubleshooting:"
    Write-Info "  1. Board not connected: check USB cable and ST-LINK"
    Write-Info "  2. Firmware not running: press RESET or use -FlashFirst"
    Write-Info "  3. Port $Port in use: add -Port <other_port>"
    Write-Info "  4. RTT not found: add -RttControlBlock 0x<addr>"
    if (-not $ocdProc.HasExited) { $ocdProc | Stop-Process -Force }
    Remove-Item $SidecarCfg -Force -ErrorAction SilentlyContinue
    exit 1
}

$tcpClient.ReceiveTimeout = 1500
$tcpStream = $tcpClient.GetStream()
Write-Ok "TCP connected to localhost:$Port"

# ---------------------------------------------------------------------------
# Ensure output directory exists and open file stream
# ---------------------------------------------------------------------------
$logParent = Split-Path -Parent $OutputLog
if ($logParent -and -not (Test-Path $logParent)) {
    New-Item -ItemType Directory -Path $logParent | Out-Null
}
if (Test-Path $OutputLog) { Remove-Item $OutputLog -Force }

$fileStream = [System.IO.File]::Open(
    $OutputLog,
    [System.IO.FileMode]::Create,
    [System.IO.FileAccess]::Write
)

$buf       = New-Object byte[] 8192
$startTime = Get-Date
$endTime   = $startTime.AddSeconds($WaitSeconds)
$bytes     = 0

Write-Info "Streaming for $WaitSeconds s..."

# ---------------------------------------------------------------------------
# Streaming loop
# ---------------------------------------------------------------------------
while ((Get-Date) -lt $endTime) {
    $readOk = $false
    try {
        $n = $tcpStream.Read($buf, 0, $buf.Length)
        if ($n -gt 0) {
            $fileStream.Write($buf, 0, $n)
            $fileStream.Flush()
            $bytes += $n
        }
        $readOk = $true
    } catch [System.IO.IOException] {
        $readOk = $true   # read timeout - normal, keep looping
    } catch {
        Write-Warn "Stream read error: $_"
    }
    if (-not $readOk) { break }
    if ($ocdProc.HasExited) {
        Write-Warn "OpenOCD exited during capture - capture may be incomplete"
        break
    }
}

$elapsed = [Math]::Round(((Get-Date) - $startTime).TotalSeconds, 1)
Write-Ok "Stream closed: $bytes bytes in ${elapsed}s"

# ---------------------------------------------------------------------------
# Cleanup (only our OpenOCD process)
# ---------------------------------------------------------------------------
try { $fileStream.Close()  } catch { $null = $_ }
try { $tcpStream.Close()   } catch { $null = $_ }
try { $tcpClient.Close()   } catch { $null = $_ }

if (-not $ocdProc.HasExited) {
    $ocdProc | Stop-Process -Force -ErrorAction SilentlyContinue
    Write-Info "OpenOCD (pid=$($ocdProc.Id)) stopped"
}
Remove-Item $SidecarCfg -Force -ErrorAction SilentlyContinue

# ---------------------------------------------------------------------------
# Decode captured bytes to printable ASCII
# ---------------------------------------------------------------------------
$capturedText = ""
if ((Test-Path $OutputLog) -and ($bytes -gt 0)) {
    $rawBytes     = [System.IO.File]::ReadAllBytes($OutputLog)
    $decoded      = [System.Text.Encoding]::ASCII.GetString($rawBytes)
    $capturedText = $decoded -replace '[^\x09\x0A\x0D\x20-\x7E]', ''
}

if ($bytes -lt 10 -or $capturedText.Trim().Length -lt 10) {
    $byteCount = $bytes
    Write-Fail "Capture produced no usable text ($byteCount raw bytes)"
    Write-Info "Board may not be running or firmware may need a manual RESET."
    Write-Info "Try: (1) press RESET; (2) use -FlashFirst; (3) increase -WaitSeconds"
    exit 1
}

# ---------------------------------------------------------------------------
# Pattern verification
# ---------------------------------------------------------------------------
Write-Info ""
Write-Sep
Write-Info "--- Pattern Verification ---"
$allPassed = $true

foreach ($pat in $RequirePatterns) {
    if ($capturedText.Contains($pat)) {
        Write-Ok  "FOUND    : $pat"
    } else {
        Write-Fail "MISSING  : $pat"
        $allPassed = $false
    }
}

# CFSR / HFSR - any non-zero value is a hard FAIL (stop condition per Phase 6O brief)
$cfsrMatches = [regex]::Matches($capturedText, 'cfsr=0x([0-9a-fA-F]+)')
$hfsrMatches = [regex]::Matches($capturedText, 'hfsr=0x([0-9a-fA-F]+)')

$cfsrBad = @($cfsrMatches | Where-Object { $_.Groups[1].Value -ne "00000000" })
$hfsrBad = @($hfsrMatches | Where-Object { $_.Groups[1].Value -ne "00000000" })

if ($cfsrBad.Count -gt 0) {
    foreach ($m in $cfsrBad) {
        Write-Fail "NON-ZERO CFSR: cfsr=0x$($m.Groups[1].Value)  -- hardware fault; do NOT close phase"
    }
    $allPassed = $false
} elseif ($cfsrMatches.Count -gt 0) {
    Write-Ok  "CFSR     : all 0x00000000 ($($cfsrMatches.Count) occurrences checked)"
} else {
    Write-Warn "CFSR     : no cfsr= lines found in capture"
}

if ($hfsrBad.Count -gt 0) {
    foreach ($m in $hfsrBad) {
        Write-Fail "NON-ZERO HFSR: hfsr=0x$($m.Groups[1].Value)  -- hardware fault"
    }
    $allPassed = $false
} elseif ($hfsrMatches.Count -gt 0) {
    Write-Ok  "HFSR     : all 0x00000000 ($($hfsrMatches.Count) occurrences checked)"
} else {
    Write-Warn "HFSR     : no hfsr= lines found in capture"
}

# ---------------------------------------------------------------------------
# Summary
# ---------------------------------------------------------------------------
Write-Info ""
Write-Sep
Write-Info "Summary"
$logSz = if (Test-Path $OutputLog) { (Get-Item $OutputLog).Length } else { 0 }
Write-Info "  Output log  : $OutputLog"
Write-Info "  File size   : $logSz bytes"
Write-Info "  Duration    : ${elapsed}s"
Write-Info "  Patterns    : $($RequirePatterns.Count) required"
Write-Sep

if ($allPassed) {
    Write-Ok "PASS -- all required patterns verified; CFSR/HFSR zero"
    Write-Info ""
    Write-Info "Next steps:"
    Write-Info "  git add $OutputLog"
    Write-Info "  Update RobotOS_v1.0/devkit/logs/INDEX.md with new log entry"
    Write-Info "  Update RobotOS_v1.0/devkit/docs/DEVKIT_PROGRESS.md Phase 6O"
    exit 0
} else {
    Write-Fail "FAIL -- one or more required patterns missing or CFSR/HFSR non-zero"
    Write-Info "  Raw log preserved: $OutputLog"
    Write-Info "  Do NOT mark Phase 6O CLOSED until this script exits 0."
    exit 1
}
