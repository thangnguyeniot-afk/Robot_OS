<#
.SYNOPSIS
    Phase 9D — Repeatable RobotOS workload demo runner.

.DESCRIPTION
    Orchestrates one deterministic run of the Phase 9C application state-machine
    demo on the STM32F411E-DISCO devkit. Performs:

      1. Hardware/wiring reminder.
      2. Phase 6O RTT capture launched as a background PowerShell job.
      3. Boot settle delay.
      4. Scripted UART send: 'a' -> 's' -> 'r' (3 bytes, individually written
         to the configured COM port, no newline).
      5. Operator prompt to press the user button N times.
      6. Capture completion + verification-checklist printout.

    The script does not modify firmware, the Phase 6O harness, or any
    devkit/core/platform source. It is a tooling wrapper that produces a
    repeatable evidence log under devkit/logs/.

.PARAMETER ComPort
    Windows COM device for the external USB-UART adapter wired to the
    STM32F411E-DISCO PA3 (USART2 RX) and GND. Required.

.PARAMETER BaudRate
    UART baud rate. Default 115200 (matches stm32f411e_disco.dts &usart2 current-speed).

.PARAMETER OutputLog
    RTT capture output path. Default
    "RobotOS_v1.0/devkit/logs/phase_9D_workload_demo_<yyyy-MM-dd>.txt".

.PARAMETER CaptureSeconds
    Total RTT capture window. Default 90. Must accommodate boot settle (~10 s),
    UART send (~3 s), button procedure (>= 5 s for 3 presses with spacing),
    and a margin for periodic ROBOTOS_APP emissions.

.PARAMETER WaitBeforeInputSeconds
    Delay after starting capture before sending UART. Default 12 s. Lets the
    board boot and emit at least two periodic ROBOTOS_OBS / ROBOTOS_APP lines
    so the IDLE baseline is visible in the evidence.

.PARAMETER ButtonPressCount
    Number of physical button presses requested from the operator. Default 3.

.PARAMETER InterButtonDelaySeconds
    Recommended spacing between presses (printed to operator only; the script
    does not enforce timing). Default 1.2 s.

.PARAMETER UartSequence
    UART payload as an array of single-character strings. Default @('a','s','r').
    Each element is written individually to keep ROBOTOS_UART counters easy to
    cross-check against the byte stream.

.PARAMETER SkipCapture
    Run only the UART send + operator prompt, without launching capture_devkit_rtt.ps1.
    Useful when an operator wants to start the harness manually in a separate window.

.PARAMETER CaptureScript
    Override the capture-script path. Default resolves the sibling
    capture_devkit_rtt.ps1 in tools/runtime/.

.EXAMPLE
    .\run_phase9d_demo.ps1 -ComPort COM5

    Runs the canonical Phase 9D demo: 90 s capture, 12 s boot wait, scripted
    UART 'a'/'s'/'r' send, prompt for 3 button presses, evidence saved to
    RobotOS_v1.0/devkit/logs/phase_9D_workload_demo_<today>.txt.

.NOTES
    PowerShell 5.1 compatible (avoids ternary, null-coalescing, and pipeline-
    chain operators). Tested on Windows 11 with CP210x USB-UART adapter.

    The script does NOT verify or parse the captured log; it leaves the
    PASS/FAIL decision to the Phase 6O harness exit code and the human
    verification checklist printed at the end. This is deliberate: keep the
    script narrow, leave evidence interpretation to humans + DEVKIT_PROGRESS.md.
#>

[CmdletBinding()]
param(
    [Parameter(Mandatory=$true)]
    [string]   $ComPort,

    [int]      $BaudRate                = 115200,

    [string]   $OutputLog               = "",

    [int]      $CaptureSeconds          = 90,

    [int]      $WaitBeforeInputSeconds  = 12,

    [int]      $ButtonPressCount        = 3,

    [double]   $InterButtonDelaySeconds = 1.2,

    [string[]] $UartSequence            = @('a','s','r'),

    [switch]   $SkipCapture,

    [string]   $CaptureScript           = ""
)

$ErrorActionPreference = 'Stop'

# Resolve repo root from this script's location: <repo>\RobotOS_v1.0\tools\runtime\.
$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$repoRoot  = Resolve-Path (Join-Path $scriptDir "..\..\..") | Select-Object -ExpandProperty Path

if (-not $OutputLog) {
    $today     = Get-Date -Format "yyyy-MM-dd"
    $OutputLog = "RobotOS_v1.0/devkit/logs/phase_9D_workload_demo_$today.txt"
}

if (-not $CaptureScript) {
    $CaptureScript = Join-Path $scriptDir "capture_devkit_rtt.ps1"
}

Write-Host ""
Write-Host "======================================================================"
Write-Host "  RobotOS Phase 9D Workload Demo Runner"
Write-Host "  Date         : $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')"
Write-Host "  Repo         : $repoRoot"
Write-Host "  ComPort      : $ComPort @ $BaudRate 8N1"
Write-Host "  CaptureScript: $CaptureScript"
Write-Host "  OutputLog    : $OutputLog"
Write-Host "  CaptureSecs  : $CaptureSeconds"
Write-Host "  UART payload : $($UartSequence -join ' ')  (no newline)"
Write-Host "  Buttons      : $ButtonPressCount presses, ~$InterButtonDelaySeconds s apart"
Write-Host "======================================================================"
Write-Host ""
Write-Host "Wiring reminder (STM32F411E-DISCO):"
Write-Host "  external USB-UART TX  -> board PA3 (USART2 RX)"
Write-Host "  external USB-UART GND -> board GND"
Write-Host "  do NOT cross-wire VCC; the board is powered by ST-LINK USB"
Write-Host "  user button = blue 'USER' button (PA0, sw0); not the black RESET"
Write-Host ""

Push-Location $repoRoot
try {

    # -------------------- 1. Capture launch --------------------
    $capJob = $null
    if (-not $SkipCapture) {
        if (-not (Test-Path $CaptureScript)) {
            throw "capture_devkit_rtt.ps1 not found: $CaptureScript"
        }

        Write-Host "[STEP 1/4] Launching Phase 6O RTT capture (background, $CaptureSeconds s)"
        $capJob = Start-Job -ArgumentList $CaptureScript, $OutputLog, $CaptureSeconds, $repoRoot `
                            -ScriptBlock {
            param($script, $log, $secs, $root)
            Set-Location $root
            & $script -OutputLog $log -WaitSeconds $secs `
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
                ) 2>&1
        }
    } else {
        Write-Host "[STEP 1/4] -SkipCapture set; assuming operator started capture separately."
    }

    # -------------------- 2. Boot settle --------------------
    Write-Host "[STEP 2/4] Waiting $WaitBeforeInputSeconds s for board boot + IDLE baseline"
    Start-Sleep -Seconds $WaitBeforeInputSeconds

    # -------------------- 3. Scripted UART send --------------------
    Write-Host "[STEP 3/4] Sending UART payload: $($UartSequence -join ' ')"
    $port = New-Object System.IO.Ports.SerialPort `
                $ComPort, $BaudRate, ([System.IO.Ports.Parity]::None), 8, ([System.IO.Ports.StopBits]::One)
    $port.Open()
    try {
        for ($i = 0; $i -lt $UartSequence.Count; $i++) {
            $b = $UartSequence[$i]
            Write-Host ("  -> UART byte {0}: '{1}'" -f ($i+1), $b)
            $port.Write([string]$b)
            if ($i -lt ($UartSequence.Count - 1)) {
                Start-Sleep -Milliseconds 1500
            }
        }
        # Allow last byte to drain.
        Start-Sleep -Milliseconds 500
    } finally {
        $port.Close()
    }

    # -------------------- 4. Operator-prompted button presses --------------------
    Write-Host ""
    Write-Host "================================================================"
    Write-Host ("  >>> PRESS THE USER BUTTON {0} TIMES NOW" -f $ButtonPressCount)
    Write-Host ("  >>> Space presses ~{0:N1} s apart" -f $InterButtonDelaySeconds)
    Write-Host "  >>> Use the BLUE 'USER' button (PA0), not the black RESET"
    Write-Host "================================================================"
    Write-Host ""

    if ($capJob) {
        Write-Host "[STEP 4/4] Waiting for capture to finish..."
        $null = Wait-Job $capJob
        $capOutput = Receive-Job $capJob
        Remove-Job $capJob

        # Surface the full Phase 6O harness output verbatim.
        $capOutput | ForEach-Object { Write-Host $_ }
    } else {
        Write-Host "[STEP 4/4] Capture skipped; operator must finish capture manually."
    }

    Write-Host ""
    Write-Host "======================================================================"
    Write-Host "  Phase 9D Verification Checklist (consult $OutputLog)"
    Write-Host "----------------------------------------------------------------------"
    Write-Host "  [ ] Phase 9C app state init: state=IDLE   (boot banner)"
    Write-Host "  [ ] 3 'Phase 9B uart handled byte=...'    (a, s, r)"
    Write-Host "  [ ] 3 'Phase 9C app transition: ...src=UART'"
    Write-Host ("  [ ] {0} 'Phase 9A button handled' lines" -f $ButtonPressCount)
    Write-Host ("  [ ] {0} 'Phase 9C app transition: ...src=BTN' lines" -f $ButtonPressCount)
    Write-Host "  [ ] Final ROBOTOS_APP: state=IDLE transitions=6 button=3 uart=3 ignored=0"
    Write-Host "  [ ] CFSR / HFSR all 0x00000000"
    Write-Host "  [ ] OBS accepted-dispatched=pending invariant holds"
    Write-Host "  [ ] No 'full=' or 'dropped=' growth on ROBOTOS_BTN/UART/PROD"
    Write-Host "======================================================================"
    Write-Host ""

} finally {
    Pop-Location
}
