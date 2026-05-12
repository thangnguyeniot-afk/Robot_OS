<#
.SYNOPSIS
    Phase 11D -- On-board MEMS accelerometer probe `T` command demo.

.DESCRIPTION
    Validates the Phase 11D `T` command on the STM32F411E-DISCO devkit.
    Sends the Phase 11C-frozen canonical sequence `T T ?` over UART and
    verifies the response shapes:

      - 'T' -> ACC x=<v1>.<v2_6d> y=<v1>.<v2_6d> z=<v1>.<v2_6d>   (success)
              OR
              ERR accel read=<errno>                                (failure)
      - '?' -> STATE state=<S> transitions=<N> button=<N> uart=<U+3> ignored=<N>

    Shape match only -- the two ACC lines need NOT be byte-identical;
    the sensor is live and minor X/Y/Z variation is normal.

    Invariants (from PHASE_11C_ACCEL_PROBE_SPEC.md sections I and H.2):

      - transitions: unchanged across the sequence
      - ignored:     unchanged across the sequence
      - uart:        +3 (two T + one ?)
      - last byte:   0x3f ('?')
      - CFSR / HFSR: 0x00000000 in every fault sample
      - dropped / herr / unhandled / rejected / throttled: 0 throughout
      - accepted - dispatched = pending = 1 (steady state)

    Phase 11D = build-validated firmware. This harness is provided in
    11D so it is ready for Phase 11E (Evidence Closeout, hardware-witnessed
    run). Phase 11D does NOT itself claim hardware evidence; the operator
    chooses whether to run this script before opening 11E.

    POST_FLASH_AUTOSTART discipline: root cause OPEN; MITIGATED_BY_WORKFLOW
    from Phase 6O onward via capture_devkit_rtt.ps1 sidecar `reset run`.
    Manual RESET retained as fallback. Plain `west flash` alone is not
    runtime-start evidence.

    UART TX scope guard (PHASE_9EZ_CHECKPOINT.md section H) is NOT crossed
    by Phase 11D: single-byte command; fixed switch/snprintf; bounded
    fixed-buffer response (96 B); thread-context handler; no new driver
    abstraction in core/platform; no parser, shell, registry, framing,
    response queue, heap, or ISR-context TX. Adapter probe only.

.PARAMETER ComPort
    Windows COM device for the external USB-UART adapter wired to:
      board PA2 (USART2 TX) -> USB-UART RX
      board PA3 (USART2 RX) -> USB-UART TX
      board GND             -> USB-UART GND
    Required.

.PARAMETER BaudRate
    UART baud rate. Default 115200.

.PARAMETER Sequence
    Array of single characters to send. Default @('T','T','?').

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
    RobotOS_v1.0/devkit/logs/phase_11D_host_<date>.txt

.PARAMETER OutputRttLog
    RTT capture output path. Default:
    RobotOS_v1.0/devkit/logs/phase_11D_rtt_<date>.txt

.PARAMETER SkipCapture
    Run only the UART send/receive, without launching capture_devkit_rtt.ps1.

.PARAMETER CaptureScript
    Override the capture-script path. Default: sibling capture_devkit_rtt.ps1.

.EXAMPLE
    .\run_phase11d_accel_probe_demo.ps1 -ComPort COM5

.NOTES
    PowerShell 5.1 compatible (no ternary, no null-coalescing, no &&/||).
    Pure ASCII (encoding-safe for ParseFile / PSParser).
#>

[CmdletBinding()]
param(
    [Parameter(Mandatory=$true)]
    [string]   $ComPort,

    [int]      $BaudRate               = 115200,

    [string[]] $Sequence               = @('T','T','?'),

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
    $OutputRttLog = "RobotOS_v1.0/devkit/logs/phase_11D_rtt_$today.txt"
}
if (-not $OutputTranscript) {
    $OutputTranscript = "RobotOS_v1.0/devkit/logs/phase_11D_host_$today.txt"
}
if (-not $CaptureScript) {
    $CaptureScript = Join-Path $scriptDir "capture_devkit_rtt.ps1"
}

Write-Host ""
Write-Host "======================================================================"
Write-Host "  RobotOS Phase 11D Accelerometer Probe Command Demo Runner"
Write-Host "  Date         : $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')"
Write-Host "  Repo         : $repoRoot"
Write-Host "  ComPort      : $ComPort @ $BaudRate 8N1"
Write-Host "  Sequence     : $($Sequence -join ' ')"
Write-Host "  RTT log      : $OutputRttLog"
Write-Host "  Transcript   : $OutputTranscript"
Write-Host "  CaptureSecs  : $CaptureSeconds"
Write-Host "======================================================================"
Write-Host ""
Write-Host "Two ACC responses need not be byte-identical (sensor is live)."
Write-Host "Shape match plus invariant checks decide PASS in Phase 11E."
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
                    "Phase 11D-T accel probe"
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
    $transcript += "# Phase 11D accelerometer probe command transcript"
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
    Write-Host "  Phase 11D Verification Checklist (Phase 11E judges PASS)"
    Write-Host "  RTT log   : $OutputRttLog"
    Write-Host "  Transcript: $OutputTranscript"
    Write-Host "----------------------------------------------------------------------"
    Write-Host "  Host UART responses (shape match; ACC values are live):"
    Write-Host "  [ ] 'T' (1st) -> ACC x=<v1>.<v2_6d> y=<v1>.<v2_6d> z=<v1>.<v2_6d>"
    Write-Host "                  (or ERR accel read=<errno> if sensor unavailable)"
    Write-Host "  [ ] 'T' (2nd) -> ACC x=<v1>.<v2_6d> y=<v1>.<v2_6d> z=<v1>.<v2_6d>"
    Write-Host "  [ ] '?'       -> STATE state=<S> transitions=<N> button=<N> uart=<U+3> ignored=<N>"
    Write-Host "  RTT evidence (from RTT log):"
    Write-Host "  [ ] Two 'Phase 11D-T accel probe: state=<S>' lines (one per T)"
    Write-Host "  [ ] Two 'Phase 9E UART response sent cmd=0x74' lines (T responses)"
    Write-Host "  [ ] ROBOTOS_UART rx=ok=handled all incremented by 3; last=0x3f"
    Write-Host "  [ ] ROBOTOS_APP transitions / ignored / button unchanged"
    Write-Host "  [ ] ROBOTOS_APP uart counter incremented by exactly 3"
    Write-Host "  [ ] ROBOTOS_OBS dropped=0 herr=0 unhandled=0 throttled=0 rejected=0"
    Write-Host "  [ ] CFSR=0x00000000 and HFSR=0x00000000 in every ROBOTOS_FAULT line"
    Write-Host "  [ ] accepted - dispatched = pending = 1 invariant holds"
    Write-Host "  [ ] PROD ok + UART ok = accepted invariant holds"
    Write-Host "  Optional physical sanity:"
    Write-Host "  [ ] Z reads near +9.8 m/s^2 (val1 ~= 9, val2 ~= 8xxxxx) when"
    Write-Host "      board is flat on bench  ->  OPERATOR_PHYSICAL_SANITY_CONFIRMED"
    Write-Host "======================================================================"
    Write-Host ""

} finally {
    Pop-Location
}
