<#
.SYNOPSIS
    Phase 9E — Minimal UART TX response demo runner.

.DESCRIPTION
    Validates the Phase 9E host-command/state-response loop on the
    STM32F411E-DISCO devkit. Performs:

      1. Hardware/wiring reminder.
      2. Phase 6O RTT capture launched as a background PowerShell job.
      3. Boot-settle delay.
      4. Opens the COM port (full-duplex: TX to board and RX from board).
      5. Sends each byte in the command sequence, waits briefly, reads
         the board's UART TX response line, and prints the transcript.
      6. Capture completion + verification-checklist printout.
      7. Optionally saves the host UART transcript.

    The script does not modify firmware, Phase 6O harness, or any
    devkit/core/platform source. It is a tooling wrapper that produces
    RTT + host-UART evidence for Phase 9E close.

.PARAMETER ComPort
    Windows COM device for the external USB-UART adapter wired to:
      board PA2 (USART2 TX) -> USB-UART RX
      board PA3 (USART2 RX) -> USB-UART TX
      board GND             -> USB-UART GND
    Required.

.PARAMETER BaudRate
    UART baud rate. Default 115200.

.PARAMETER Sequence
    Array of single characters to send. Each element produces one board
    response. Default @('a','s','?','r','x') exercises:
      a   IDLE -> ARMED       expect: OK state=ARMED
      s   ARMED -> ACTIVE     expect: OK state=ACTIVE
      ?   query               expect: STATE state=ACTIVE transitions=2 ...
      r   ACTIVE -> IDLE      expect: OK state=IDLE
      x   ignored             expect: ERR ignored byte=0x78 state=IDLE

.PARAMETER InterByteDelayMs
    Milliseconds to wait between sending a byte and reading the response.
    Default 600 ms: board must dispatch the event (one per 500 ms tick)
    and then emit the TX response before we try to read. Increase if you
    observe missed responses.

.PARAMETER ReadTimeoutMs
    SerialPort.ReadTimeout value in milliseconds. Default 1200 ms.

.PARAMETER CaptureSeconds
    Total RTT capture window. Default 60 s.

.PARAMETER WaitBeforeInputSeconds
    Seconds to wait after starting RTT capture before sending UART.
    Default 15 s (board boot + IDLE baseline).

.PARAMETER OutputTranscript
    If specified, the host UART transcript is written to this path.
    Default: RobotOS_v1.0/devkit/logs/phase_9E_uart_tx_response_host_<date>.txt

.PARAMETER OutputRttLog
    RTT capture output path. Default:
    RobotOS_v1.0/devkit/logs/phase_9E_uart_tx_response_rtt_<date>.txt

.PARAMETER SkipCapture
    Run only the UART send/receive, without launching capture_devkit_rtt.ps1.

.PARAMETER CaptureScript
    Override the capture-script path. Default: sibling capture_devkit_rtt.ps1.

.EXAMPLE
    .\run_phase9e_uart_response_demo.ps1 -ComPort COM5

    Canonical Phase 9E demo: 60 s RTT capture, 15 s boot wait, sequence
    a/s/?/r/x, full transcript printed and saved.

.NOTES
    PowerShell 5.1 compatible (no ternary, no null-coalescing, no &&/||).
    Tested on Windows 11 with CP210x USB-UART adapter.

    ReadLine() is used to receive one response per sent byte. The board
    emits responses terminated with \r\n; SerialPort.ReadLine() reads until
    the NewLine character (\n) and trims the trailing \r from display.

    If InterByteDelayMs is too short (board hasn't dispatched the event yet),
    ReadLine() will throw TimeoutException and print "(timeout - no response)".
    Increase InterByteDelayMs or check RTT for Phase 9E dispatch evidence.
#>

[CmdletBinding()]
param(
    [Parameter(Mandatory=$true)]
    [string]   $ComPort,

    [int]      $BaudRate               = 115200,

    [string[]] $Sequence               = @('a','s','?','r','x'),

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
    $OutputRttLog = "RobotOS_v1.0/devkit/logs/phase_9E_uart_tx_response_rtt_$today.txt"
}
if (-not $OutputTranscript) {
    $OutputTranscript = "RobotOS_v1.0/devkit/logs/phase_9E_uart_tx_response_host_$today.txt"
}
if (-not $CaptureScript) {
    $CaptureScript = Join-Path $scriptDir "capture_devkit_rtt.ps1"
}

Write-Host ""
Write-Host "======================================================================"
Write-Host "  RobotOS Phase 9E UART TX Response Demo Runner"
Write-Host "  Date         : $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')"
Write-Host "  Repo         : $repoRoot"
Write-Host "  ComPort      : $ComPort @ $BaudRate 8N1"
Write-Host "  Sequence     : $($Sequence -join ' ')"
Write-Host "  RTT log      : $OutputRttLog"
Write-Host "  Transcript   : $OutputTranscript"
Write-Host "  CaptureSecs  : $CaptureSeconds"
Write-Host "======================================================================"
Write-Host ""
Write-Host "Wiring reminder (STM32F411E-DISCO):"
Write-Host "  board PA2 (USART2_TX) -> USB-UART RX"
Write-Host "  board PA3 (USART2_RX) -> USB-UART TX"
Write-Host "  board GND             -> USB-UART GND"
Write-Host "  TTL level: 3.3V only; do NOT connect VCC if board is ST-LINK powered"
Write-Host "  user button = blue 'USER' (PA0, sw0); not the black RESET"
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
                    "Phase 9E UART response"
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
    $port.NewLine      = "`n"  # board sends \r\n; ReadLine splits on \n

    $port.Open()

    $transcript = @()
    $transcript += "# Phase 9E UART TX response transcript"
    $transcript += "# Date: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')  Port: $ComPort  Baud: $BaudRate"
    $transcript += "# Sequence: $($Sequence -join ' ')"
    $transcript += "# InterByteDelayMs: $InterByteDelayMs  ReadTimeoutMs: $ReadTimeoutMs"
    $transcript += ""

    Write-Host ""
    Write-Host "  CMD -> RESPONSE (each line is one send/receive pair)"
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
                # Trim trailing \r that ReadLine may leave from \r\n
                $received = $line.TrimEnd("`r")
            } catch [System.TimeoutException] {
                $received = "(timeout - no response)"
            }

            Write-Host $received
            $entry = "SEND '$char' ($hex_str) -> RECV: $received"
            $transcript += $entry
        }
    } finally {
        $port.Close()
    }

    Write-Host "  ----------------------------------------------------------"
    Write-Host ""

    # Save transcript
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
    Write-Host "  Phase 9E Verification Checklist"
    Write-Host "  RTT log   : $OutputRttLog"
    Write-Host "  Transcript: $OutputTranscript"
    Write-Host "----------------------------------------------------------------------"
    Write-Host "  Host UART responses (from transcript above):"
    Write-Host "  [ ] 'a' -> OK state=ARMED"
    Write-Host "  [ ] 's' -> OK state=ACTIVE"
    Write-Host "  [ ] '?' -> STATE state=ACTIVE transitions=2 ..."
    Write-Host "  [ ] 'r' -> OK state=IDLE"
    Write-Host "  [ ] 'x' -> ERR ignored byte=0x78 state=IDLE"
    Write-Host "  RTT evidence (from RTT log):"
    Write-Host "  [ ] 'Phase 9E UART response sent' for each command"
    Write-Host "  [ ] Phase 9C app transition lines for a/s/r"
    Write-Host "  [ ] ROBOTOS_APP final: state=IDLE transitions=3 uart=5 ignored=1"
    Write-Host "  [ ] ROBOTOS_UART rx=5 ok=5 full=0 handled=5"
    Write-Host "  [ ] CFSR=0x00000000 / HFSR=0x00000000 in every ROBOTOS_FAULT emission"
    Write-Host "  [ ] OBS accepted-dispatched=pending invariant holds"
    Write-Host "  [ ] No 'full=' or 'dropped=' growth"
    Write-Host "  [ ] Phase 6M producer healthy throughout"
    Write-Host "======================================================================"
    Write-Host ""

} finally {
    Pop-Location
}
