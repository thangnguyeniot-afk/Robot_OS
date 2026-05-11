<#
.SYNOPSIS
    Phase 9G -- Bounded UART burst characterization runner.

.DESCRIPTION
    Sends a bounded burst of single-byte UART commands faster than the
    500 ms dispatch tick and collects:

      - Host UART transcript with arrival timestamps and ordering.
      - RTT capture via the Phase 6O `capture_devkit_rtt.ps1` harness.

    Phase 9E proved a single-byte host->board->host loop with one byte
    per ~600 ms (one byte per dispatch tick). Phase 9G characterizes the
    bounded burst case: 4-5 bytes within ~150 ms (well below the 500 ms
    tick), then a wait window long enough to collect all dispatched
    responses.

    This is an evidence-only phase. No firmware change. No core or
    platform change. No scheduler change. The script does not "fix"
    queue saturation or response-ordering anomalies if observed; it
    reports them as Phase 9G findings for a future scheduler or UART
    decision gate.

.PARAMETER ComPort
    Windows COM device for the external USB-UART adapter wired to:
      board PA2 (USART2 TX) -> USB-UART RX
      board PA3 (USART2 RX) -> USB-UART TX
      board GND             -> USB-UART GND
    Required.

.PARAMETER BaudRate
    UART baud rate. Default 115200.

.PARAMETER Sequence
    Bounded byte burst (single characters). Default @('a','s','?','r','x')
    matches the Phase 9E vocabulary so transitions and the negative-path
    probe both exercise during the burst:

      a   IDLE -> ARMED        expect: OK state=ARMED
      s   ARMED -> ACTIVE      expect: OK state=ACTIVE
      ?   query                expect: STATE state=<S> transitions=<N> ...
      r   ACTIVE -> IDLE       expect: OK state=IDLE
      x   ignored              expect: ERR ignored byte=0x78 state=IDLE

    The burst is intentionally bounded. This is characterization, not
    stress-to-failure.

.PARAMETER BurstSpacingMs
    Milliseconds between consecutive byte writes inside the burst.
    Default 30 ms. With 5 bytes, total burst width is ~150 ms, well
    below the 500 ms dispatch tick. This forces the core event queue
    to hold >=2 pending UART events at some point during the burst.

.PARAMETER ReadWindowMs
    Total time to read responses after the burst is sent. Default
    5000 ms. With a 500 ms tick and one event per tick, 5 responses
    require ~2.5-3 s; 5 s provides margin without becoming
    stress-to-failure.

.PARAMETER ReadTimeoutMs
    SerialPort.ReadTimeout value in milliseconds for each ReadLine
    attempt within the read window. Default 6000 ms.

.PARAMETER CaptureSeconds
    Total RTT capture window. Default 60 s.

.PARAMETER WaitBeforeInputSeconds
    Seconds to wait after starting RTT capture before sending the
    burst. Default 15 s (board boot + IDLE baseline).

.PARAMETER OutputTranscript
    If specified, the host UART transcript is written to this path.
    Default:
    RobotOS_v1.0/devkit/logs/phase_9G_uart_burst_host_<date>.txt

.PARAMETER OutputRttLog
    RTT capture output path. Default:
    RobotOS_v1.0/devkit/logs/phase_9G_uart_burst_rtt_<date>.txt

.PARAMETER SkipCapture
    Run only the UART send/receive, without launching capture_devkit_rtt.ps1.

.PARAMETER CaptureScript
    Override the capture-script path. Default: sibling capture_devkit_rtt.ps1.

.EXAMPLE
    .\run_phase9g_uart_burst_demo.ps1 -ComPort COM5

    Canonical Phase 9G burst: 60 s RTT capture, 15 s boot wait, burst
    a/s/?/r/x with 30 ms spacing, 5 s read window, transcript saved.

.NOTES
    PowerShell 5.1 compatible (no ternary, no null-coalescing, no &&/||).
    Tested authoring host: Windows 11 with CP210x USB-UART adapter.

    Hardware not present at authoring time -- script is parse-checked and
    boundary-validated. Hardware run is expected to be performed by an
    operator with the STM32F411E-DISCO + USB-UART adapter wired per the
    reminder section.

    POST_FLASH_AUTOSTART discipline (Phase 6O onward, MITIGATED_BY_WORKFLOW;
    Phase 3B root cause remains OPEN): the harness is launched via
    `capture_devkit_rtt.ps1` which issues `reset run` through a sidecar
    OpenOCD .cfg. Manual RESET retained as fallback. Plain `west flash`
    alone is not runtime-start evidence.

    UART TX scope guard (PHASE_9EZ_CHECKPOINT.md sectionH) is NOT crossed by
    Phase 9G: no parser, no shell, no framing, no registry, no response
    queue, no heap, no ISR-context TX. The script sends raw single
    bytes; the board's existing fixed switch/snprintf path handles them.
#>

[CmdletBinding()]
param(
    [Parameter(Mandatory=$true)]
    [string]   $ComPort,

    [int]      $BaudRate               = 115200,

    [string[]] $Sequence               = @('a','s','?','r','x'),

    [int]      $BurstSpacingMs         = 30,

    [int]      $ReadWindowMs           = 5000,

    [int]      $ReadTimeoutMs          = 6000,

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
    $OutputRttLog = "RobotOS_v1.0/devkit/logs/phase_9G_uart_burst_rtt_$today.txt"
}
if (-not $OutputTranscript) {
    $OutputTranscript = "RobotOS_v1.0/devkit/logs/phase_9G_uart_burst_host_$today.txt"
}
if (-not $CaptureScript) {
    $CaptureScript = Join-Path $scriptDir "capture_devkit_rtt.ps1"
}

$burstWidthMs = if ($Sequence.Count -le 1) { 0 } else { ($Sequence.Count - 1) * $BurstSpacingMs }

Write-Host ""
Write-Host "======================================================================"
Write-Host "  RobotOS Phase 9G Bounded UART Burst Characterization Runner"
Write-Host "  Date            : $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')"
Write-Host "  Repo            : $repoRoot"
Write-Host "  ComPort         : $ComPort @ $BaudRate 8N1"
Write-Host "  Sequence        : $($Sequence -join ' ')  (count = $($Sequence.Count))"
Write-Host "  BurstSpacingMs  : $BurstSpacingMs   (total burst width ~ $burstWidthMs ms)"
Write-Host "  ReadWindowMs    : $ReadWindowMs"
Write-Host "  CaptureSeconds  : $CaptureSeconds"
Write-Host "  RTT log         : $OutputRttLog"
Write-Host "  Transcript      : $OutputTranscript"
Write-Host "======================================================================"
Write-Host ""
Write-Host "Wiring reminder (STM32F411E-DISCO):"
Write-Host "  board PA2 (USART2_TX) -> USB-UART RX"
Write-Host "  board PA3 (USART2_RX) -> USB-UART TX"
Write-Host "  board GND             -> USB-UART GND"
Write-Host "  TTL level: 3.3V only; do NOT connect VCC if board is ST-LINK powered"
Write-Host ""
Write-Host "Phase 9G intent:"
Write-Host "  - Send the burst inside ~$burstWidthMs ms (faster than one 500 ms tick)."
Write-Host "  - Read responses for ~$ReadWindowMs ms; expect responses to arrive at"
Write-Host "    ~500 ms intervals (one per dispatch tick) until the burst is drained."
Write-Host "  - Observe queue peak, drops, ordering, transitions, ignored count."
Write-Host "  - Do NOT 'fix' anomalies; report them as Phase 9G findings."
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
                    "ROBOTOS_APP"
                ) 2>&1
        }
    } else {
        Write-Host "[STEP 1/4] -SkipCapture set; operator started capture separately."
    }

    # -------------------- 2. Boot settle --------------------
    Write-Host "[STEP 2/4] Waiting $WaitBeforeInputSeconds s for board boot + IDLE baseline"
    Start-Sleep -Seconds $WaitBeforeInputSeconds

    # -------------------- 3. Burst send + collect responses ----------
    Write-Host "[STEP 3/4] Opening $ComPort for full-duplex UART"
    $port = New-Object System.IO.Ports.SerialPort `
                $ComPort, $BaudRate, ([System.IO.Ports.Parity]::None), 8, ([System.IO.Ports.StopBits]::One)
    $port.ReadTimeout  = $ReadTimeoutMs
    $port.NewLine      = "`n"

    $port.Open()

    $transcript = @()
    $transcript += "# Phase 9G bounded UART burst characterization transcript"
    $transcript += "# Date: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')  Port: $ComPort  Baud: $BaudRate"
    $transcript += "# Sequence: $($Sequence -join ' ')   (count = $($Sequence.Count))"
    $transcript += "# BurstSpacingMs: $BurstSpacingMs   total burst width ~ $burstWidthMs ms"
    $transcript += "# ReadWindowMs: $ReadWindowMs   ReadTimeoutMs: $ReadTimeoutMs"
    $transcript += ""

    # 3a. Burst send (host wallclock timestamps for each byte)
    Write-Host ""
    Write-Host "  3a. Burst send  (rapid; spacing $BurstSpacingMs ms)"
    Write-Host "  ----------------------------------------------------------"
    $burstStart = [System.Diagnostics.Stopwatch]::StartNew()
    $sendLog = @()
    try {
        $seq_index = 0
        foreach ($char in $Sequence) {
            $seq_index++
            $byte_val = [byte][char]$char
            $hex_str  = "0x{0:x2}" -f $byte_val
            $tMs      = $burstStart.ElapsedMilliseconds

            $port.Write([string]$char)
            $line = ("  [{0}] t={1,5}ms SEND '{2}' ({3})" -f $seq_index, $tMs, $char, $hex_str)
            Write-Host $line
            $sendLog += ("SEND_BURST t={0}ms idx={1} byte='{2}' ({3})" -f $tMs, $seq_index, $char, $hex_str)

            if ($seq_index -lt $Sequence.Count) {
                Start-Sleep -Milliseconds $BurstSpacingMs
            }
        }
        $burstStart.Stop()
        Write-Host ("  burst-complete-at t={0}ms (sequence sent in {0} ms wallclock)" -f $burstStart.ElapsedMilliseconds)
    } catch {
        $port.Close()
        throw
    }

    $transcript += $sendLog
    $transcript += "BURST_COMPLETE_MS=$($burstStart.ElapsedMilliseconds)"
    $transcript += ""

    # 3b. Read window -- collect all responses arriving within ReadWindowMs
    Write-Host ""
    Write-Host "  3b. Read window  ($ReadWindowMs ms)"
    Write-Host "  ----------------------------------------------------------"

    $readClock = [System.Diagnostics.Stopwatch]::StartNew()
    $recvLog   = @()
    $recvCount = 0
    try {
        while ($readClock.ElapsedMilliseconds -lt $ReadWindowMs) {
            $remaining = $ReadWindowMs - $readClock.ElapsedMilliseconds
            if ($remaining -lt 1) { break }
            # Use the smaller of remaining-window or ReadTimeoutMs per read
            $port.ReadTimeout = [int][Math]::Max(1, [Math]::Min($ReadTimeoutMs, $remaining))

            try {
                $line = $port.ReadLine()
                $received = $line.TrimEnd("`r")
                $recvCount++
                $tMs = $readClock.ElapsedMilliseconds
                $entry = ("  [{0}] t={1,5}ms RECV: {2}" -f $recvCount, $tMs, $received)
                Write-Host $entry
                $recvLog += ("RECV t={0}ms idx={1} line='{2}'" -f $tMs, $recvCount, $received)
            } catch [System.TimeoutException] {
                # Within the read window, a timeout means no further data
                # for now. Break only if window is essentially exhausted.
                if ($readClock.ElapsedMilliseconds + 100 -ge $ReadWindowMs) { break }
                # Otherwise continue polling.
            }
        }
        $readClock.Stop()
    } finally {
        $port.Close()
    }

    Write-Host ("  read-window-complete-at t={0}ms; collected {1} response line(s)" -f $readClock.ElapsedMilliseconds, $recvCount)
    Write-Host "  ----------------------------------------------------------"
    Write-Host ""

    $transcript += $recvLog
    $transcript += "READ_WINDOW_MS=$($readClock.ElapsedMilliseconds)"
    $transcript += "RECV_COUNT=$recvCount"
    $transcript += "BURST_COUNT=$($Sequence.Count)"

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
    Write-Host "  Phase 9G Verification Checklist (operator confirms from logs)"
    Write-Host "  RTT log   : $OutputRttLog"
    Write-Host "  Transcript: $OutputTranscript"
    Write-Host "----------------------------------------------------------------------"
    Write-Host "  Host UART responses (from transcript above):"
    Write-Host "  [ ] One RECV line per byte sent (RECV_COUNT == BURST_COUNT)"
    Write-Host "  [ ] Response ordering matches send ordering"
    Write-Host "  [ ] First RECV arrives ~500 ms after burst-start (one tick)"
    Write-Host "  [ ] Subsequent RECVs arrive at ~500 ms intervals"
    Write-Host "  [ ] 'x' produces 'ERR ignored byte=0x78 state=<S>'"
    Write-Host "  RTT evidence (from RTT log):"
    Write-Host "  [ ] ROBOTOS_UART rx == BURST_COUNT, ok == BURST_COUNT"
    Write-Host "  [ ] ROBOTOS_UART full == 0   (queue did not refuse a UART event)"
    Write-Host "  [ ] ROBOTOS_UART handled == BURST_COUNT"
    Write-Host "  [ ] ROBOTOS_APP transitions matches a/s/r path"
    Write-Host "  [ ] ROBOTOS_APP ignored == 1   ('x')"
    Write-Host "  [ ] ROBOTOS_OBS peak >= 2 during the burst (>=2 UART events queued)"
    Write-Host "  [ ] ROBOTOS_OBS dropped == 0  (queue capacity 16 not exceeded)"
    Write-Host "  [ ] OBS accepted - dispatched == pending"
    Write-Host "  [ ] CFSR=0x00000000 and HFSR=0x00000000 in every ROBOTOS_FAULT line"
    Write-Host "  [ ] Phase 6M producer (attempted/ok) healthy across capture"
    Write-Host "----------------------------------------------------------------------"
    Write-Host "  Phase 9G findings (record only -- do NOT 'fix' here):"
    Write-Host "  [ ] If peak > Phase 9E peak=2, record as positive evidence for"
    Write-Host "      future scheduler discussion (not a regression)."
    Write-Host "  [ ] If dropped > 0, record as Phase 9G finding for scheduler /"
    Write-Host "      queue-capacity decision gate -- do not change budget here."
    Write-Host "  [ ] If response ordering mismatches send ordering, record as"
    Write-Host "      Phase 9G finding for UART decision gate -- do not patch TX."
    Write-Host "  [ ] If RECV_COUNT < BURST_COUNT, record as Phase 9G finding --"
    Write-Host "      do not introduce retry/ACK protocol."
    Write-Host "======================================================================"
    Write-Host ""

} finally {
    Pop-Location
}
