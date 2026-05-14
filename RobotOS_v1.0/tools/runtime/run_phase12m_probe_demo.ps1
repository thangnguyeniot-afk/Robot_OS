<#
.SYNOPSIS
    Phase 12M -- Probe Translator Runtime Adapter hardware validation demo.

.DESCRIPTION
    Validates the Phase 12L devkit_probe_adapter runtime wiring on the
    STM32F411E-DISCO board. Sends the minimum validation sequence
    a -> s -> r -> ? over UART and verifies the UART TX responses and
    RTT probe adapter snapshots.

    Expected probe adapter RTT progression (from PROBE_TRANSLATOR_HARDWARE_VALIDATION_PLAN.md):
      Boot:        ROBOTOS_PROBE init ok
                   ROBOTOS_PROBE state=1 trans=0 events=0 no_trans=0 mapped=0 unmapped=0
      After 'a':   ROBOTOS_PROBE state=2 trans=1 events=1 no_trans=0 mapped=1 unmapped=0
      After 's':   ROBOTOS_PROBE state=3 trans=2 events=2 no_trans=0 mapped=2 unmapped=0
      After 'r':   ROBOTOS_PROBE state=1 trans=3 events=3 no_trans=0 mapped=3 unmapped=0

    Expected UART TX responses (unchanged from Phase 9E/12L):
      'a' -> OK state=ARMED
      's' -> OK state=ACTIVE
      'r' -> OK state=IDLE
      '?' -> STATE state=IDLE transitions=3 ...

    Boundaries (Phase 12M):
      - No source, CMake, config change.
      - No UART command surface change.
      - No product command mapping.
      - Hardware evidence only.

    POST_FLASH_AUTOSTART: root cause OPEN; MITIGATED_BY_WORKFLOW from
    Phase 6O onward via capture_devkit_rtt.ps1 sidecar 'reset run'.
    Manual RESET is allowed as fallback only.

.PARAMETER ComPort
    Windows COM device for USB-UART adapter wired to:
      board PA2 (USART2 TX) -> USB-UART RX
      board PA3 (USART2 RX) -> USB-UART TX
      board GND             -> USB-UART GND
    Required.

.PARAMETER BaudRate
    UART baud rate. Default 115200.

.PARAMETER Sequence
    Array of single characters to send. Default @('a','s','r','?').

.PARAMETER InterByteDelayMs
    Milliseconds to wait between sending a byte and reading the response.
    Default 600 ms (one dispatch tick + margin).

.PARAMETER ReadTimeoutMs
    SerialPort.ReadTimeout in milliseconds. Default 1200 ms.

.PARAMETER CaptureSeconds
    Total RTT capture window. Default 90 s.

.PARAMETER WaitBeforeInputSeconds
    Seconds to wait after RTT capture start before sending UART.
    Default 15 s (board boot + IDLE baseline).

.PARAMETER OutputTranscript
    Host UART transcript output path.
    Default: RobotOS_v1.0/devkit/logs/phase_12M_uart_<date>.txt

.PARAMETER OutputRttLog
    RTT capture output path.
    Default: RobotOS_v1.0/devkit/logs/phase_12M_rtt_<date>.txt

.PARAMETER SkipCapture
    Run only UART send/receive without launching capture_devkit_rtt.ps1.

.PARAMETER CaptureScript
    Override the capture-script path. Default: sibling capture_devkit_rtt.ps1.

.PARAMETER ElfPath
    Path to the Phase 12M ELF for _SEGGER_RTT address resolution.
    Default: build-phase12m/zephyr/zephyr.elf (relative to repo root).
    Use this to override the default build/zephyr/zephyr.elf path.

.PARAMETER RttControlBlock
    Explicit RTT block address (e.g. "0x20000b38"). Skips nm when provided.
    Phase 12M confirmed address: 0x20000b38 (shifted from Phase 11E 0x20000ad0).

.EXAMPLE
    .\run_phase12m_probe_demo.ps1 -ComPort COM5

.NOTES
    PowerShell 5.1 compatible (no ternary, no null-coalescing, no &&/||).
    Pure ASCII.
    Follows Phase 11D/Phase 9E script conventions.
#>

[CmdletBinding()]
param(
    [Parameter(Mandatory=$true)]
    [string]   $ComPort,

    [int]      $BaudRate               = 115200,

    [string[]] $Sequence               = @('a','s','r','?'),

    [int]      $InterByteDelayMs       = 600,

    [int]      $ReadTimeoutMs          = 1200,

    [int]      $CaptureSeconds         = 90,

    [int]      $WaitBeforeInputSeconds = 15,

    [string]   $OutputTranscript       = "",

    [string]   $OutputRttLog           = "",

    [switch]   $SkipCapture,

    [string]   $CaptureScript          = "",

    [string]   $ElfPath                = "build-phase12m/zephyr/zephyr.elf",

    [string]   $RttControlBlock        = "0x20000b38"
)

$ErrorActionPreference = 'Stop'

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$repoRoot  = Resolve-Path (Join-Path $scriptDir "..\..\..") | Select-Object -ExpandProperty Path
$today     = Get-Date -Format "yyyy-MM-dd"

if (-not $OutputRttLog) {
    $OutputRttLog = "RobotOS_v1.0/devkit/logs/phase_12M_rtt_$today.txt"
}
if (-not $OutputTranscript) {
    $OutputTranscript = "RobotOS_v1.0/devkit/logs/phase_12M_uart_$today.txt"
}
if (-not $CaptureScript) {
    $CaptureScript = Join-Path $scriptDir "capture_devkit_rtt.ps1"
}

Write-Host ""
Write-Host "======================================================================"
Write-Host "  RobotOS Phase 12M Probe Translator Hardware Validation Demo"
Write-Host "  Date         : $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')"
Write-Host "  Repo         : $repoRoot"
Write-Host "  ComPort      : $ComPort @ $BaudRate 8N1"
Write-Host "  Sequence     : $($Sequence -join ' ')"
Write-Host "  RTT log      : $OutputRttLog"
Write-Host "  Transcript   : $OutputTranscript"
Write-Host "  CaptureSecs  : $CaptureSeconds"
Write-Host "======================================================================"
Write-Host ""
Write-Host "Expected probe adapter RTT progression:"
Write-Host "  Boot:        ROBOTOS_PROBE init ok"
Write-Host "               ROBOTOS_PROBE state=1 trans=0 events=0 no_trans=0 mapped=0 unmapped=0"
Write-Host "  After 'a':   ROBOTOS_PROBE state=2 trans=1 events=1 no_trans=0 mapped=1 unmapped=0"
Write-Host "  After 's':   ROBOTOS_PROBE state=3 trans=2 events=2 no_trans=0 mapped=2 unmapped=0"
Write-Host "  After 'r':   ROBOTOS_PROBE state=1 trans=3 events=3 no_trans=0 mapped=3 unmapped=0"
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
        Write-Host "[INFO]     ElfPath          : $ElfPath"
        Write-Host "[INFO]     RttControlBlock  : $RttControlBlock"
        $capJob = Start-Job -ArgumentList $CaptureScript, $OutputRttLog, $CaptureSeconds, $repoRoot, $ElfPath, $RttControlBlock `
                            -ScriptBlock {
            param($script, $log, $secs, $root, $elf, $rttAddr)
            Set-Location $root
            & $script -OutputLog $log -WaitSeconds $secs `
                -ElfPath $elf `
                -RttControlBlock $rttAddr `
                -RequirePatterns @(
                    "ROBOTOS_OBS state=READY",
                    "ROBOTOS_FAULT active=0",
                    "ROBOTOS_PROD attempted=",
                    "ROBOTOS_UART",
                    "ROBOTOS_APP",
                    "ROBOTOS_PROBE init ok",
                    "ROBOTOS_PROBE state=2",
                    "ROBOTOS_PROBE state=3",
                    "ROBOTOS_PROBE state=1 trans=3"
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
    $port.ReadTimeout = $ReadTimeoutMs
    $port.NewLine     = "`n"

    $port.Open()

    $transcript = @()
    $transcript += "# Phase 12M probe translator hardware validation transcript"
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
    Write-Host "  Phase 12M Probe Adapter Hardware Validation Checklist"
    Write-Host "  RTT log   : $OutputRttLog"
    Write-Host "  Transcript: $OutputTranscript"
    Write-Host "----------------------------------------------------------------------"
    Write-Host "  RTT evidence:"
    Write-Host "  [ ] 'ROBOTOS_PROBE init ok' present"
    Write-Host "  [ ] Baseline 'ROBOTOS_PROBE state=1 trans=0 events=0 no_trans=0 mapped=0 unmapped=0'"
    Write-Host "  [ ] After 'a': 'ROBOTOS_PROBE state=2 trans=1 events=1 no_trans=0 mapped=1 unmapped=0'"
    Write-Host "  [ ] After 's': 'ROBOTOS_PROBE state=3 trans=2 events=2 no_trans=0 mapped=2 unmapped=0'"
    Write-Host "  [ ] After 'r': 'ROBOTOS_PROBE state=1 trans=3 events=3 no_trans=0 mapped=3 unmapped=0'"
    Write-Host "  [ ] App transition: IDLE->ARMED, ARMED->ACTIVE, ACTIVE->IDLE present"
    Write-Host "  [ ] CFSR=0x00000000 and HFSR=0x00000000 throughout"
    Write-Host "  [ ] dropped=0 herr=0 unhandled=0 rejected=0 throttled=0"
    Write-Host "  [ ] No 'ROBOTOS_PROBE dispatch err' line"
    Write-Host "  [ ] No crash/hardfault/assert/reset loop"
    Write-Host "  Host UART transcript:"
    Write-Host "  [ ] 'a' -> 'OK state=ARMED'"
    Write-Host "  [ ] 's' -> 'OK state=ACTIVE'"
    Write-Host "  [ ] 'r' -> 'OK state=IDLE'"
    Write-Host "  [ ] '?' -> 'STATE state=IDLE transitions=3 ...'"
    Write-Host "======================================================================"
    Write-Host ""

} finally {
    Pop-Location
}
