# RobotOS Devkit RTT Evidence Index

Indexed evidence logs for hardware validation sessions on STM32F411E-DISCO
(unless noted otherwise). One row per captured RTT log file.

For host-test logs see `tests/host/logs/` — a note on duplicates appears at
the bottom of this file.

---

## RTT Logs

| Phase | Commit | Log File | Date | Capture Method | Result Summary |
|-------|--------|----------|------|----------------|----------------|
| 5D — Platform Critical Section | `4187bb3` | [phase_5D_rtt_2026-05-02.txt](phase_5D_rtt_2026-05-02.txt) | 2026-05-02 | OpenOCD RTT dump (4 KB) | PASS — critical-section smoke, no fault |
| 5E — Critical Boundary to Core Queue | `ff24147` | [phase_5E_rtt_2026-05-02.txt](phase_5E_rtt_2026-05-02.txt) | 2026-05-02 | OpenOCD RTT dump (4 KB) | PASS — no fault |
| 5F — Dispatcher Pop / Handler Split | `460b9f1` | [phase_5F_rtt_2026-05-02.txt](phase_5F_rtt_2026-05-02.txt) | 2026-05-02 | OpenOCD RTT dump (4 KB) | PASS — handler outside lock confirmed |
| 6A — Devkit Event Smoke | (early) | [phase_6A_rtt_2026-05-02.txt](phase_6A_rtt_2026-05-02.txt) | 2026-05-02 | OpenOCD RTT dump (4 KB) | PASS — single USER event handled |
| 6B — Event Burst / Backpressure | (early) | [phase_6B_rtt_2026-05-02.txt](phase_6B_rtt_2026-05-02.txt) | 2026-05-02 | OpenOCD RTT dump (4 KB) | PASS — burst of 3, backpressure asserted |
| 6C — Queue Full / Drop | (early) | [phase_6C_rtt_2026-05-02.txt](phase_6C_rtt_2026-05-02.txt) | 2026-05-02 | OpenOCD RTT dump (4 KB) | PASS — 17 events, 1 dropped (queue full) |
| 6D — Invalid / Rejection | (early) | [phase_6D_rtt_2026-05-02.txt](phase_6D_rtt_2026-05-02.txt) | 2026-05-02 | OpenOCD RTT dump (4 KB) | PASS — ERR_INVALID_ARG, admission_rejected confirmed |
| 6E — Throttled Producer | (early) | [phase_6E_rtt_2026-05-02.txt](phase_6E_rtt_2026-05-02.txt) | 2026-05-02 | OpenOCD RTT dump (4 KB) | PASS — seq=3 ERR_THROTTLED confirmed |
| 6G — ISR/Timer Producer Smoke | `335ee29` | [phase_6G_rtt_2026-05-02.txt](phase_6G_rtt_2026-05-02.txt) | 2026-05-02 | OpenOCD RTT dump (4 KB) | PASS — 2 events from ISR timer, CFSR=0 HFSR=0 |
| 6H — ISR Timer Stress-Lite | `22edfee` | [phase_6H_rtt_2026-05-03.txt](phase_6H_rtt_2026-05-03.txt) | 2026-05-03 | OpenOCD RTT dump / GDB fallback | PASS — attempted=8 ok=8 handled=8 CFSR=0 HFSR=0 |
| 6F — Mixed Event Policy | `5bca62f` | [phase_6F_rtt_2026-05-03.txt](phase_6F_rtt_2026-05-03.txt) | 2026-05-03 | OpenOCD RTT dump (4 KB) | PASS — accept/reject/drop in one run |
| 6I — Timer Queue-Pressure Stress | `e78e503` | [phase_6I_rtt_2026-05-03.txt](phase_6I_rtt_2026-05-03.txt) | 2026-05-03 | OpenOCD RTT dump (4 KB) | PASS — attempted=24 ok=16 full=8 handled=16 CFSR=0 HFSR=0 |
| 6Z — RTT Closeout (6K/6L/6M) | `4ec5b86` | [phase_6Z_rtt_2026-05-07.txt](phase_6Z_rtt_2026-05-07.txt) | 2026-05-07 | OpenOCD streaming RTT TCP server (60 s) | PASS — ROBOTOS_OBS/FAULT/PROD baseline+12 periodic; CFSR=0 HFSR=0 throughout; accepted=77 dispatched=76 pending=1 |
| 6O — Harness smoke | `6cb979f` | [phase_6O_harness_smoke_2026-05-08.txt](phase_6O_harness_smoke_2026-05-08.txt) | 2026-05-08 | `capture_devkit_rtt.ps1` Phase 6O harness (30 s) | PASS — all 4 patterns found; CFSR=0 HFSR=0 (7 occurrences); 9558 bytes; exit 0 |
| 9A-A — Button EXTI workload | `2068180` | [phase_9A_button_rtt_2026-05-08.txt](phase_9A_button_rtt_2026-05-08.txt) | 2026-05-08 | Phase 6O harness (60 s) with manual button presses | CLOSED with BOUNCE_OBSERVED — 37 button events handled; CFSR=0 HFSR=0 (13 occurrences); 21961 bytes; bounce caused "Phase 6I final:" miss — see DEVKIT_PROGRESS.md Phase 9A-A |
| 9A-B — Button debounce | `92de5e0` | [phase_9B_debounce_rtt_2026-05-08.txt](phase_9B_debounce_rtt_2026-05-08.txt) | 2026-05-08 | Phase 6O harness (90 s) with manual button presses | PASS — attempted=94 ok=36 full=4 debounce=54; conservation 36+4+54=94 ✓; full 98→4 vs Phase 9A-A; 36 handled; CFSR=0 HFSR=0 (19 occurrences); 29160 bytes; Phase 6I final: miss due to early press (non-regression) |
| 9A-C — Phase 6I burst gated off | `3989ff9` | [phase_9C_no_burst_button_rtt_2026-05-08.txt](phase_9C_no_burst_button_rtt_2026-05-08.txt) | 2026-05-08 | Phase 6O harness (60 s) with manual button presses; Phase 6I burst compiled out via DEVKIT_PHASE6I_STARTUP_BURST_ENABLED=0 | PASS — DEVKIT_DIAG phase6i_startup_burst=0 banner present; Phase 6I timer/handler/final all absent; ROBOTOS_BTN attempted=57 ok=17 full=0 debounce=40 handled=17; OBS accepted=77 dispatched=76 pending=1 peak=14 dropped=0; CFSR=0 HFSR=0 (13 occurrences); 19564 bytes |
| 9B — UART RX producer | `85389f4` | [phase_9B_uart_rtt_2026-05-08.txt](phase_9B_uart_rtt_2026-05-08.txt) | 2026-05-08 | Phase 6O harness (60 s); CP210x USB-UART adapter on COM5 → board PA3 @ 115200 8N1; payload `abc123\n` (7 bytes) sent ~12 s into capture | PASS — Phase 9B uart producer init banner present (USER+3 marker=0x9b0b); ROBOTOS_UART rx=7 ok=7 full=0 handled=7 last=0x0a; 7 per-byte handler logs match payload exactly (a/b/c/1/2/3/0x0a); OBS accepted=67 dispatched=66 pending=1 peak=8 dropped=0; CFSR=0 HFSR=0 (13 occurrences); 20627 bytes |
| 9C — App state machine | `286e61b` | [phase_9C_app_state_rtt_2026-05-08.txt](phase_9C_app_state_rtt_2026-05-08.txt) | 2026-05-08 | Phase 6O harness (75 s); manual button presses + UART `a`/`s`/`r` on COM5; first multi-source workload composition | PASS — Phase 9C app state init banner present; ROBOTOS_APP final state=ACTIVE transitions=23 button=20 uart=3 ignored=0; 23 per-transition diagnostic lines (20 src=BTN, 3 src=UART) explain full causality chain; OBS accepted=98 dispatched=97 pending=1 peak=4 dropped=0; CFSR=0 HFSR=0 (16 occurrences); 30954 bytes |
| 9D — Workload demo | `8e8c801` | [phase_9D_workload_demo_2026-05-08.txt](phase_9D_workload_demo_2026-05-08.txt) | 2026-05-08 | `run_phase9d_demo.ps1 -ComPort COM5` (90 s); scripted UART `a`/`s`/`r` + operator button presses | PASS — all 12 default patterns FOUND; ROBOTOS_APP final state=IDLE transitions=35 button=32 uart=3 ignored=0 last_byte=0x72 (operator pressed >3 times; canonical target documented as 6/3/3/0); conservation 32+3=35 ✓; OBS accepted=125 dispatched=124 pending=1 peak=16 dropped=3 (queue saturation under rapid button input observed safely); CFSR=0 HFSR=0 (19 occurrences); herr=0 unhandled=0; 37930 bytes |
| 9E — UART TX minimal response | `587dab7` | [phase_9E_uart_tx_response_rtt_2026-05-09.txt](phase_9E_uart_tx_response_rtt_2026-05-09.txt) | 2026-05-09 | `run_phase9e_uart_response_demo.ps1 -ComPort COM5` (60 s); scripted UART `a`/`s`/`?`/`r`/`x`; board PA2 TX→USB-UART RX wired; host transcript saved separately | PASS — all 6 patterns FOUND (incl. "Phase 9E UART response"); ROBOTOS_UART rx=5 ok=5 handled=5; ROBOTOS_APP final state=IDLE transitions=3 uart=5 ignored=1 last_byte=0x78; OBS accepted=25 dispatched=24 pending=1 peak=2 dropped=0 herr=0; CFSR=0 HFSR=0 (13 occurrences); 23188 bytes; host transcript: all 5 responses correct (OK/STATE/ERR) |
| 9G ‡ — Bounded UART burst characterization | `e9a1d62` (harness) + this commit (evidence) | [phase_9G_uart_burst_rtt_2026-05-11.txt](phase_9G_uart_burst_rtt_2026-05-11.txt) (RTT) / [phase_9G_uart_burst_host_2026-05-11.txt](phase_9G_uart_burst_host_2026-05-11.txt) (host transcript) | 2026-05-11 | `run_phase9g_uart_burst_demo.ps1 -ComPort COM5` (60 s capture; 5-byte burst `a s ? r x` at 30 ms spacing, ~185 ms total burst); Phase 9E firmware (`587dab7`) reflashed via `west flash`; Phase 6O harness with sidecar `reset run`; manual RESET not required | PASS — 5 required patterns FOUND; ROBOTOS_UART rx=5 ok=5 full=0 handled=5 last=0x78; ROBOTOS_APP final state=IDLE transitions=3 uart=5 ignored=1; OBS accepted=65 dispatched=64 pending=1 **peak=5** (vs Phase 9E peak=2) dropped=0 herr=0 unhandled=0 rejected=0 throttled=0; CFSR=0 HFSR=0 (13 occurrences); Phase 6M producer attempted=60 ok=60 (healthy +5/10 ticks); 22929 bytes; host: 5/5 responses in send order. `‡` non-linear-insert (post-split late-9-series) — see DEVKIT_PROGRESS_PHASE_10.md Phase 9G entry |
| 10B-v — Build/version query command | this commit | [phase_10B_v_rtt_2026-05-11.txt](phase_10B_v_rtt_2026-05-11.txt) (RTT) / [phase_10B_v_host_2026-05-11.txt](phase_10B_v_host_2026-05-11.txt) (host transcript) | 2026-05-11 | `run_phase10b_v_build_query_demo.ps1 -ComPort COM5` (60 s capture; sequence `v a v r ?` at 600 ms spacing); Phase 10B-v firmware (pristine `west build` then `west flash`, 49152 B `.hex`); Phase 6O harness with sidecar `reset run`; manual RESET not required | PASS — 6 required patterns FOUND (incl. "Phase 10B-v build query"); ROBOTOS_UART rx=5 ok=5 full=0 handled=5 last=0x3f; ROBOTOS_APP final state=IDLE transitions=2 button=0 uart=5 ignored=0 last_byte=0x3f; OBS accepted=65 dispatched=64 pending=1 peak=2 dropped=0 herr=0 unhandled=0 rejected=0 throttled=0; CFSR=0 HFSR=0 (13 occurrences); Phase 6M producer attempted=60 ok=60; 23226 bytes; host: 5/5 responses in send order, both `v` responses byte-identical (`INFO phase=10b-v app=devkit board=stm32f411e_disco tick_ms=500 uart=minimal`, len=77); `v` did not transition and did not increment `ignored`. See PHASE_10B_V_CLOSE.md |
| 10B-L — LED physical-effect command | this commit | [phase_10B_L_rtt_2026-05-11.txt](phase_10B_L_rtt_2026-05-11.txt) (RTT) / [phase_10B_L_host_2026-05-11.txt](phase_10B_L_host_2026-05-11.txt) (host transcript) | 2026-05-11 | `run_phase10b_l_led_command_demo.ps1 -ComPort COM5` (60 s capture; sequence `L v L ?` at 600 ms spacing); Phase 10B-L firmware (pristine `west build` then `west flash`, 49152 B `.hex`); Phase 6O harness with sidecar `reset run`; manual RESET not required | PASS — 6 required patterns FOUND (incl. "Phase 10B-L LED command"); ROBOTOS_UART rx=4 ok=4 full=0 handled=4 last=0x3f; ROBOTOS_APP final state=IDLE transitions=0 button=0 uart=4 ignored=0 last_byte=0x3f; OBS accepted=64 dispatched=63 pending=1 peak=2 dropped=0 herr=0 unhandled=0 rejected=0 throttled=0; CFSR=0 HFSR=0 (13 occurrences); Phase 6M producer attempted=60 ok=60; 22744 bytes; host: 4/4 responses in send order, both `L` responses byte-identical (`OK led=toggle state=IDLE`, len=26); `v` response unchanged from Phase 10B-v; `L` did not transition and did not increment `ignored`; heartbeat continued (139 tick-count lines). **PHYSICAL_OBSERVATION_AMBIGUOUS** (autonomous run, no human watched the LED). See PHASE_10B_L_CLOSE.md |

---

## Notes

### Phases without a standalone RTT log

The following phases were validated via host tests only (no hardware session required
per their approved scope):

| Phase | Commit | Validation |
|-------|--------|------------|
| 4A–4L | see git log | Host-only; each phase has a corresponding `tests/host/logs/` entry |
| 5A–5C | see git log | Host-only (platform boundary definitions) |
| 5G | `e4ab853` | Audit-only (no source change, no new RTT required) |
| 6J | `8a1af69` | Host-only (contract tests for snapshot/peak); no devkit smoke needed per scope |
| 6K | `11516d4` | Build+host validated; RTT confirmed by Phase 6Z log above |
| 6L | `d3759a7` | Build+host validated; RTT confirmed by Phase 6Z log above |
| 6M | `a6b253b` | Build+host validated; RTT confirmed by Phase 6Z log above |

---

### Host log duplicate note

`tests/host/logs/host_2026-05-02.log` and `phase_6H_host_2026-05-02.log` are
byte-identical (MD5: `31FC67382B2F8590CC15D5777E15805A`, 39,868 bytes). One is
a copy of the other from a double-commit; both are preserved to avoid breaking
any references. Neither is deleted.

The three 27,755-byte files (`phase_6B_`, `phase_6C_`, `phase_6D_`) have
different MD5 hashes despite identical sizes — they are genuinely different
per-phase captures.
