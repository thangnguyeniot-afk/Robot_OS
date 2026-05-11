<#
.SYNOPSIS
    Phase 10B-L -- LED physical-effect command demo.

.DESCRIPTION
    Validates the Phase 10B-L `L` command on the STM32F411E-DISCO devkit.
    Sends a small bounded sequence over UART and verifies:

      - `L` triggers a physical LED toggle on the existing led0 GPIO
      - `L` returns the OK led=toggle response over the existing UART TX path
      - `L` does NOT change app state (transitions unchanged)
      - `L` does NOT increment `ignored`
      - `v` (Phase 10B-v) remains unchanged when sampled
      - `?` (Phase 9E query) format remains unchanged
      - No fault triggered (CFSR/HFSR/active all zero)

    Default sequence: `L v L ?`

      L   (state=IDLE)   expect: OK led=toggle state=IDLE      (LED phase shifts)
      v   (state=IDLE)   expect: INFO phase=10b-v ...           (unchanged from Phase 10B-v)
      L   (state=IDLE)   expect: OK led=toggle state=IDLE      (LED phase shifts again)
      ?   (state=IDLE)   expect: STATE state=IDLE transitions=0 button=0 uart=4 ignored=0

    This is a feature-validation demo. Bounded UART burst behavior was
    already characterized in Phase 9G. Phase 10B-L uses Phase 9E-style
    one-byte-per-tick pacing for transcript clarity, not stress.

    The physical LED effect is observable as a phase shift in the existing
    500 ms heartbeat blink: each `L` advances the heartbeat by one
    half-cycle. There is no clean "command flash" distinct from the
    heartbeat -- a Phase 10B-L `L` toggle interleaves with the heartbeat
    toggles in `devkit_runtime_run()`. RTT evidence confirms the toggle
    call fired (`Phase 10B-L LED command: state=...` log + the `OK
    led=toggle` UART TX response); the operator's eye is the only place
    that confirms the physical phase shift.

    The script does not modify firmware, the Phase 6O harness, or any
    devkit/core/platform source. It is a tooling wrapper that produces
    RTT + host-UART evidence for Phase 10B-L close.

.PARAMETER ComPort
    Windows COM device for the external USB-UART adapter wired to:
      board PA2 (USART2 TX) -> USB-UART RX
      board PA3 (USART2 RX) -> USB-UART TX
      board GND             -> USB-UART GND
    Required.

.PARAMETER BaudRate
    UART baud rate. Default 115200.

.PARAMETER Sequence
    Array of single characters to send. Default @('L','v','L','?').

.PARAMETER InterByteDelayMs
    Milliseconds to wait between sending a byte and reading the response.
    Default 600 ms (one byte per ~500 ms dispatch tick + margin).

.PARAMETER ReadTimeoutMs
    SerialPort.ReadTimeout value in milliseconds. Default 1200 ms.

.PARAMETER CaptureSeconds
    Total RTT capture window. Default 60 s.

.PARAMETER WaitBeforeInputSeconds
    Seconds to wait after starting RTT capture before sending UART.
    Default 15 s (board boot + IDLE baseline).

.PARAMETER OutputTranscript
    If specified, the host UART transcript is written to this path.
    Default:
    RobotOS_v1.0/devkit/logs/phase_10B_L_host_<date>.txt

.PARAMETER OutputRttLog
    RTT capture output path. Default:
    RobotOS_v1.0/devkit/logs/phase_10B_L_rtt_<date>.txt

.PARAMETER SkipCapture
    Run only the UART send/receive, without launching capture_devkit_rtt.ps1.

.PARAMETER CaptureScript
    Override the capture-script path. Default: sibling capture_devkit_rtt.ps1.

.EXAMPLE
    .\run_phase10b_l_led_command_demo.ps1 -ComPort COM5

.NOTES
    PowerShell 5.1 compatible (no ternary, no null-coalescing, no &&/||).
    Pure ASCII (encoding-safe for ParseFile / PSParser).

    POST_FLASH_AUTOSTART discipline: root cause OPEN; MITIGATED_BY_WORKFLOW
    from Phase 6O onward via capture_devkit_rtt.ps1 sidecar `reset run`.
    Manual RESET retained as fallback. Plain `west flash` alone is not
    runtime-start evidence.

    UART TX scope guard (PHASE_9EZ_CHECKPOINT.md section H) is NOT crossed
    by Phase 10B-L: single-byte command; fixed switch/snprintf; bounded
    fixed-buffer response; thread-context handler; existing LED API
    reused (no LED subsystem redesign). No parser, no shell, no command
    registry, no framing, no response queue, no heap, no ISR-context TX.
#>

[CmdletBinding()]
param(
    [Parameter(Mandatory=$true)]
    [string]   $ComPort,

    [int]      $BaudRate               = 115200,

    [string[]] $Sequence               = @('L','v','L','?'),

    [int]      $InterByteDelayMs       = 600,

    [int]      $ReadTimeoutMs          = 1200,

    [int]      $CaptureSeconds         = 60,

    [int]      $WaitBeforeInputSeconds = 15,

    [string]   $OutputTranscript       = "",

    [string]   $OutputRttLog           = "",

    [switch]   $SkipCapture,

    [string]   $CaptureScript          = ""
)

$ErrorActionPreference = 'Stop'

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$repoRoot  = Resolve-Path (Join-Path $scriptDir "..\..\..") | Select-Object -ExpandProperty Path
$today     = Get-Date -Format "yyyy-MM-dd"

if (-not $OutputRttLog) {
    $OutputRttLog = "RobotOS_v1.0/devkit/logs/phase_10B_L_rtt_$today.txt"
}
if (-not $OutputTranscript) {
    $OutputTranscript = "RobotOS_v1.0/devkit/logs/phase_10B_L_host_$today.txt"
}
if (-not $CaptureScript) {
    $CaptureScript = Join-Path $scriptDir "capture_devkit_rtt.ps1"
}

Write-Host ""
Write-Host "======================================================================"
Write-Host "  RobotOS Phase 10B-L LED Physical-Effect Command Demo Runner"
Write-Host "  Date         : $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')"
Write-Host "  Repo         : $repoRoot"
Write-Host "  ComPort      : $ComPort @ $BaudRate 8N1"
Write-Host "  Sequence     : $($Sequence -join ' ')"
Write-Host "  RTT log      : $OutputRttLog"
Write-Host "  Transcript   : $OutputTranscript"
Write-Host "  CaptureSecs  : $CaptureSeconds"
Write-Host "======================================================================"
Write-Host ""
Write-Host "Watch the user LED on the STM32F411E-DISCO during the burst window."
Write-Host "Each 'L' command should cause a single phase-shift in the 500 ms"
Write-Host "heartbeat blink. RTT evidence below confirms the toggle call fired."
Write-Host ""

Push-Location $repoRoot
try {

    # -------------------- 1. RTT capture launch --------------------
    $capJob = $null
    if (-not $SkipCapture) {
        if (-not (Test-Path $CaptureScript)) {
            throw "capture_devkit_rtt.ps1 not found: $CaptureScript"
        }

        Write-Host "[STEP 1/4] Launching Phase 6O RTT capture (background, $CaptureSeconds s)"
        $capJob = Start-Job -ArgumentList $CaptureScript, $OutputRttLog, $CaptureSeconds, $repoRoot `
                            -ScriptBlock {
            param($script, $log, $secs, $root)
            Set-Location $root
            & $script -OutputLog $log -WaitSeconds $secs `
                -RequirePatterns @(
                    "ROBOTOS_OBS state=READY",
                    "ROBOTOS_FAULT active=0",
                    "ROBOTOS_PROD attempted=",
                    "ROBOTOS_UART",
                    "ROBOTOS_APP",
                    "Phase 10B-L LED command"
                ) 2>&1
        }
    } else {
        Write-Host "[STEP 1/4] -SkipCapture set; operator started capture separately."
    }

    # -------------------- 2. Boot settle --------------------
    Write-Host "[STEP 2/4] Waiting $WaitBeforeInputSeconds s for board boot + IDLE baseline"
    Start-Sleep -Seconds $WaitBeforeInputSeconds

    # -------------------- 3. UART send + receive --------------------
    Write-Host "[STEP 3/4] Opening $ComPort for full-duplex UART"
    $port = New-Object System.IO.Ports.SerialPort `
                $ComPort, $BaudRate, ([System.IO.Ports.Parity]::None), 8, ([System.IO.Ports.StopBits]::One)
    $port.ReadTimeout  = $ReadTimeoutMs
    $port.NewLine      = "`n"

    $port.Open()

    $transcript = @()
    $transcript += "# Phase 10B-L LED physical-effect command transcript"
    $transcript += "# Date: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')  Port: $ComPort  Baud: $BaudRate"
    $transcript += "# Sequence: $($Sequence -join ' ')"
    $transcript += "# InterByteDelayMs: $InterByteDelayMs  ReadTimeoutMs: $ReadTimeoutMs"
    $transcript += ""

    Write-Host ""
    Write-Host "  CMD -> RESPONSE"
    Write-Host "  ----------------------------------------------------------"

    try {
        $seq_index = 0
        foreach ($char in $Sequence) {
            $seq_index++
            $byte_val = [byte][char]$char
            $hex_str  = "0x{0:x2}" -f $byte_val

            Write-Host -NoNewline ("  [{0}] SEND: '{1}' ({2})   -> " -f $seq_index, $char, $hex_str)

            $port.Write([string]$char)
            Start-Sleep -Milliseconds $InterByteDelayMs

            $received = ""
            try {
                $line = $port.ReadLine()
                $received = $line.TrimEnd("`r")
            } catch [System.TimeoutException] {
                $received = "(timeout - no response)"
            }

            Write-Host $received
            $transcript += ("SEND '{0}' ({1}) -> RECV: {2}" -f $char, $hex_str, $received)
        }
    } finally {
        $port.Close()
    }

    Write-Host "  ----------------------------------------------------------"
    Write-Host ""

    $transcript | Out-File -FilePath $OutputTranscript -Encoding utf8
    Write-Host "[INFO] Host UART transcript saved: $OutputTranscript"

    # -------------------- 4. Wait for RTT capture --------------------
    if ($capJob) {
        Write-Host "[STEP 4/4] Waiting for RTT capture to finish..."
        $null = Wait-Job $capJob
        $capOutput = Receive-Job $capJob
        Remove-Job $capJob

        $capOutput | ForEach-Object { Write-Host $_ }
    } else {
        Write-Host "[STEP 4/4] Capture skipped; operator must finish manually."
    }

    # -------------------- Verification checklist --------------------
    Write-Host ""
    Write-Host "======================================================================"
    Write-Host "  Phase 10B-L Verification Checklist"
    Write-Host "  RTT log   : $OutputRttLog"
    Write-Host "  Transcript: $OutputTranscript"
    Write-Host "----------------------------------------------------------------------"
    Write-Host "  Host UART responses (from transcript above):"
    Write-Host "  [ ] 'L' (state=IDLE) -> OK led=toggle state=IDLE"
    Write-Host "  [ ] 'v' (state=IDLE) -> INFO phase=10b-v app=devkit board=stm32f411e_disco tick_ms=500 uart=minimal"
    Write-Host "  [ ] 'L' (state=IDLE) -> OK led=toggle state=IDLE  (identical to first 'L')"
    Write-Host "  [ ] '?' (state=IDLE) -> STATE state=IDLE transitions=0 button=0 uart=4 ignored=0"
    Write-Host "  Physical LED observation:"
    Write-Host "  [ ] Operator observed two phase-shifts in the heartbeat blink during the burst window"
    Write-Host "  [ ] If LED observation was ambiguous, classify PHYSICAL_OBSERVATION_AMBIGUOUS"
    Write-Host "  RTT evidence (from RTT log):"
    Write-Host "  [ ] Two 'Phase 10B-L LED command: state=IDLE' lines"
    Write-Host "  [ ] Two 'Phase 9E UART response sent cmd=0x4c' lines (the L responses)"
    Write-Host "  [ ] One 'Phase 9E UART response sent cmd=0x76' line (the v response)"
    Write-Host "  [ ] ROBOTOS_UART rx=4 ok=4 handled=4 last=0x3f (last byte was '?')"
    Write-Host "  [ ] ROBOTOS_APP final state=IDLE transitions=0 uart=4 ignored=0"
    Write-Host "  [ ] CFSR=0x00000000 and HFSR=0x00000000 in every ROBOTOS_FAULT line"
    Write-Host "  [ ] OBS accepted - dispatched == pending invariant holds"
    Write-Host "  [ ] No 'dropped=' growth; no herr; no unhandled"
    Write-Host "  [ ] Heartbeat 'tick count=N' messages continue uninterrupted across the L commands"
    Write-Host "  [ ] Phase 6M producer (attempted/ok) healthy across capture"
    Write-Host "======================================================================"
    Write-Host ""

} finally {
    Pop-Location
}
