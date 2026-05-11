# RobotOS — Current State Snapshot

> **Purpose:** Canonical startup snapshot for agents (Claude, Copilot, GPT, GLM).
> Detailed phase history remains in `RobotOS_v1.0/devkit/docs/DEVKIT_PROGRESS.md`.
> Update this file only at phase close, from validated evidence only.
> Do not record assumptions as fact. If unknown, mark **Pending / Unknown**.

---

## Last Closed Phase

### Phase 10B-d — Explicit Disarm Command `d` (hardware evidence)

- **Date:** 2026-05-11
- **Type:** Third Phase 10B-class implementation; smallest remaining behavioral surface from `COMMAND_SET_DRAFT.md` Section B. Single-byte app-state command added to the proven Phase 9E/10B-v/10B-L UART RX/TX path. Devkit-local source changes only. No new driver, no `prj.conf` change, no physical effect, no state-machine redesign. Provides explicit user-vocabulary "disarm" distinct from `r` (`r` is preserved zero-diff and remains the canonical reset path).
- **Close status:** `CLOSED_WITH_HARDWARE_EVIDENCE`
- **Implementation commit:** `125779c` (`feat: add Phase 10B-d disarm command`)
- **Closeout doc:** `RobotOS_v1.0/devkit/docs/PHASE_10B_D_CLOSE.md`
- **Phase log entry:** `RobotOS_v1.0/devkit/docs/DEVKIT_PROGRESS_PHASE_10.md` `<a id="phase-10b-d"></a>`
- **Companion command spec update:** `RobotOS_v1.0/devkit/docs/COMMAND_SET_DRAFT.md` (the `d` row was promoted from Section B DRAFT to Section A IMPLEMENTED).
- **Evidence:** `RobotOS_v1.0/devkit/logs/phase_10B_d_rtt_2026-05-11.txt` (22623 B, 60.4 s) + `RobotOS_v1.0/devkit/logs/phase_10B_d_host_2026-05-11.txt` (401 B host transcript).
- **Source change:** `devkit/src/devkit_app_state.c` (+13 lines, `case 'd'` recognition — ARMED→IDLE via existing `transition()`; otherwise no-op LOG_INF), `devkit/src/devkit_app_state.h` (+3 lines, doc-only), and `devkit/src/devkit_uart_producer.c` (+14 lines, `case 'd'` TX response — `OK disarm state=IDLE\r\n` when changed, `OK disarm no-op state=<S>\r\n` otherwise; no new include); plus new host harness `tools/runtime/run_phase10b_d_disarm_demo.ps1`. `core/`, `platform/`, `devkit_runtime.{c,h}`, `devkit_status_led.{h,c}`, `prj.conf`, `CMakeLists.txt`, `boards/`, and `tests/` are zero-diff.
- **Response format:** `OK disarm state=IDLE\r\n` (22 B) on ARMED → IDLE transition; `OK disarm no-op state=IDLE\r\n` (28 B) or `OK disarm no-op state=ACTIVE\r\n` (30 B) on the recognized-no-op paths. All variants fit the existing 96-byte stack buffer.
- **Approved state semantics:** ARMED → IDLE with transition (`transitions++`); IDLE recognized no-op (NOT ignored — distinct from `r`); ACTIVE recognized no-op for now (`USER_DECISION_REQUIRED_ACTIVE_DISARM`, no safety semantics invented).
- **Build delta:** FLASH 36628 B → 36780 B (+152 B); RAM 12224 B unchanged.
- **Verdict:** PASS. Sequence `d a d ?` on COM5 (CP210x USB-UART @ 115200 8N1) into a Phase 6O 60-second RTT capture (sidecar `reset run`; manual RESET not required); 4/4 host responses byte-exact in send order. RTT final counters at ticks=120: ROBOTOS_UART `rx=ok=handled=4 last=0x3f`; ROBOTOS_APP `state=IDLE transitions=2 button=0 uart=4 ignored=0`; ROBOTOS_OBS `accepted=64 dispatched=63 pending=1 peak=2 dropped=0 herr=0 throttled=0 rejected=0 unhandled=0`; Phase 6M producer healthy `attempted=ok=60`; CFSR/HFSR `0x00000000` (13× each). Invariants `accepted - dispatched = pending`, `PROD ok + UART ok = accepted`, and `?` response format identity all hold. `d` from IDLE did NOT increment `ignored`; `d` from ARMED added exactly one transition.
- **ACTIVE disarm:** `USER_DECISION_REQUIRED_ACTIVE_DISARM`. Default validation sequence avoids ACTIVE. Future approval is a one-line widening of the existing guard plus a supplemental run with `d a s d ?`; out of scope for Phase 10B-d.
- **Other Phase 10B candidates (`T`):** remains `USER_DECISION_REQUIRED` in `COMMAND_SET_DRAFT.md` Section B; **NOT implemented**.
- **Next gate:** Non-sensor command group complete and hardware-validated end-to-end (`a / s / r / ? / x / v / L / d`). User to decide between (a) Phase 10B-`T` (sensor read, requires sensor part choice + driver), (b) widen `d` to cover ACTIVE → IDLE (supplemental validation only), (c) a Phase 10C command-set checkpoint (equivalent of Phase 10A for the post-10B-d vocabulary), or (d) continued hold. Phase 10B-d itself authorizes none of these.

---

### Phase 10B-L — LED Physical-Effect Command `L` (hardware evidence + OPERATOR_VISUAL_CONFIRMED)

- **Date:** 2026-05-11
- **Type:** Second Phase 10B-class implementation; first physical-effect command. Single-byte command added to the proven Phase 9E/10B-v UART RX/TX path. Devkit-local source changes only. Reuses the existing `devkit_status_led_toggle()` one-shot API. No LED subsystem redesign, no new LED service, no new LED API. No core, platform, scheduler, queue, event-type, test, CMake, Zephyr, board, or `prj.conf` change.
- **Close status:** `CLOSED_WITH_HARDWARE_EVIDENCE` (electrical/RTT) + **`OPERATOR_VISUAL_CONFIRMED`** (visual LED witnessed by operator in a follow-up re-run on 2026-05-11 ~18:57 local; original autonomous-run `PHYSICAL_OBSERVATION_AMBIGUOUS` preserved historically in `PHASE_10B_L_CLOSE.md` section F; visual confirmation recorded in section P)
- **Closeout doc:** `RobotOS_v1.0/devkit/docs/PHASE_10B_L_CLOSE.md`
- **Phase log entry:** `RobotOS_v1.0/devkit/docs/DEVKIT_PROGRESS_PHASE_10.md` `<a id="phase-10b-l"></a>`
- **Companion command spec update:** `RobotOS_v1.0/devkit/docs/COMMAND_SET_DRAFT.md` (the `L` row was promoted from Section B DRAFT to Section A IMPLEMENTED).
- **Evidence (autonomous run):** `RobotOS_v1.0/devkit/logs/phase_10B_L_rtt_2026-05-11.txt` (22744 B, 60.4 s) + `RobotOS_v1.0/devkit/logs/phase_10B_L_host_2026-05-11.txt` (host transcript)
- **Evidence (operator-witnessed visual re-run):** `RobotOS_v1.0/devkit/logs/phase_10B_L_visual_rtt_2026-05-11.txt` (22744 B, 60.4 s) + `RobotOS_v1.0/devkit/logs/phase_10B_L_visual_host_2026-05-11.txt` (host transcript). Operator verdict: observed visible phase-shift in the 500 ms heartbeat blink correlated with both `L` commands. Same firmware (`f1db2fa`); same wiring; sidecar `reset run`; manual RESET not required.
- **Source change:** `devkit/src/devkit_app_state.c` (+9 lines, `case 'l'` recognition; no transition, not ignored) and `devkit/src/devkit_uart_producer.c` (+24 lines, `case 'l'` toggle + response; new `#include "devkit_status_led.h"`); plus new host harness `tools/runtime/run_phase10b_l_led_command_demo.ps1`. `devkit_status_led.{h,c}` and `devkit_runtime.c` are zero-diff.
- **Response format:** `OK led=toggle state=<S>\r\n` (26 bytes; deterministic). Error variant `ERR led=toggle ret=<N> state=<S>\r\n` (not observed this run).
- **Physical effect mechanism:** Single `devkit_status_led_toggle()` (existing API) called from thread-context UART handler. The 500 ms heartbeat blink in `devkit_runtime_run()` is unchanged and continues per-tick; an `L` between heartbeat ticks adds one extra toggle, shifting the heartbeat phase by one half-cycle. No new state, no scheduler, no service.
- **Build delta:** FLASH 36416 B → 36628 B (+212 B); RAM 12224 B unchanged.
- **Verdict:** PASS electrical/RTT + visual confirmed. Sequence `L v L ?` on COM5; both `L` responses byte-identical; `v` response unchanged (Phase 10B-v preserved); `?` reports `transitions=0 button=0 uart=4 ignored=0` confirming `L` did not transition and did not increment `ignored`. ROBOTOS_UART `rx=ok=handled=4`; ROBOTOS_OBS `peak=2 dropped=0`; CFSR/HFSR `0x00000000` (13×); Phase 6M producer healthy `attempted=ok=60` at ticks=120; `accepted(64) - dispatched(63) = pending(1)` invariant holds; heartbeat continued (139 tick-count lines). **Visual LED: `OPERATOR_VISUAL_CONFIRMED` per the follow-up operator-witnessed re-run** — operator saw the predicted heartbeat phase-shift on both `L` commands. No LED ownership conflict observed; no LED-semantics design phase warranted.
- **Other Phase 10B candidates (`d`, `T`):** remain `USER_DECISION_REQUIRED` in `COMMAND_SET_DRAFT.md` Section B; **NOT implemented**.
- **Next gate:** Visual-evidence gap closed by the operator-witnessed re-run. User to decide between (a) Phase 10B-`d` (smallest remaining behavioral surface), (b) Phase 10B-`T` (sensor read, requires sensor part choice + driver), or (c) continued hold. Phase 10B-L itself does not authorize any of these.

---

### Phase 10B-v — Build/Version Query Command `v` (hardware evidence)

- **Date:** 2026-05-11
- **Type:** First Phase 10B-class implementation. Single-byte command added to the proven Phase 9E UART RX/TX path; devkit-local source changes only. No core, platform, scheduler, queue, event-type, test, CMake, Zephyr, board, or `prj.conf` change.
- **Close status:** `CLOSED_WITH_HARDWARE_EVIDENCE`
- **Closeout doc:** `RobotOS_v1.0/devkit/docs/PHASE_10B_V_CLOSE.md`
- **Phase log entry:** `RobotOS_v1.0/devkit/docs/DEVKIT_PROGRESS_PHASE_10.md` `<a id="phase-10b-v"></a>`
- **Companion command spec update:** `RobotOS_v1.0/devkit/docs/COMMAND_SET_DRAFT.md` (the `v` row was promoted from Section B DRAFT to Section A IMPLEMENTED).
- **Evidence:** `RobotOS_v1.0/devkit/logs/phase_10B_v_rtt_2026-05-11.txt` (23226 B, 61.2 s) + `RobotOS_v1.0/devkit/logs/phase_10B_v_host_2026-05-11.txt` (host transcript)
- **Source change:** `devkit/src/devkit_app_state.c` (+6 lines, `case 'v'` recognition; no transition, not ignored) and `devkit/src/devkit_uart_producer.c` (+12 lines, `case 'v'` response; new `#include "devkit_runtime.h"`); plus new host harness `tools/runtime/run_phase10b_v_build_query_demo.ps1`.
- **Response format:** `INFO phase=10b-v app=devkit board=stm32f411e_disco tick_ms=500 uart=minimal\r\n` (77 bytes; deterministic across identical builds; fits within existing 96-byte stack buffer).
- **Build delta:** FLASH 30032 B → 36416 B (+6384 B; dominated by cbprintf format-string plumbing); RAM 12160 B → 12224 B (+64 B). Within `stm32f411e_disco` budget.
- **Verdict:** PASS. Sequence `v a v r ?` on COM5; both `v` responses byte-identical (state-invariant); `a → OK state=ARMED`, `r → OK state=IDLE`, `?` reports `transitions=2 button=0 uart=5 ignored=0` confirming `v` did not transition and did not increment `ignored`. ROBOTOS_UART `rx=ok=handled=5`; ROBOTOS_OBS `peak=2 dropped=0`; CFSR/HFSR `0x00000000` (13×); Phase 6M producer healthy `attempted=ok=60` at ticks=120; `accepted(65) - dispatched(64) = pending(1)` invariant holds.
- **Other Phase 10B candidates (`d`, `L`, `T`):** remain `USER_DECISION_REQUIRED` in `COMMAND_SET_DRAFT.md` Section B; **NOT implemented**.
- **Next gate:** User to decide between (a) Phase 10B-`L` (LED toggle, first physical effect), (b) Phase 10B-`d` (explicit disarm, smallest behavioral surface), (c) Phase 10B-`T` (sensor read, largest surface), or (d) continued hold. Phase 10B-v itself does not authorize any of these.

---

### Phase 9G — Bounded UART Burst Characterization (hardware evidence)

- **Date:** 2026-05-11
- **Type:** Evidence/tooling + hardware run. Host-side script only (no firmware, core, platform, test, CMake, Zephyr, board change).
- **Close status:** `CLOSED_WITH_HARDWARE_EVIDENCE`
- **Open commit:** `e9a1d62` (`tools: add Phase 9G UART burst characterization harness`)
- **Close commit:** this commit (`docs: close Phase 9G UART burst characterization with hardware evidence`)
- **Closeout doc:** `RobotOS_v1.0/devkit/docs/PHASE_9G_CLOSE.md`
- **Phase log entry:** `RobotOS_v1.0/devkit/docs/DEVKIT_PROGRESS_PHASE_10.md` `<a id="phase-9g-late"></a>` (non-linear `‡` insert per §4 rule 6 — late-9-series post-split)
- **Evidence:** `RobotOS_v1.0/devkit/logs/phase_9G_uart_burst_rtt_2026-05-11.txt` (22929 B, 60.7 s) + `RobotOS_v1.0/devkit/logs/phase_9G_uart_burst_host_2026-05-11.txt` (host transcript)
- **Verdict:** PASS. 5-byte burst (`a s ? r x` at 30 ms spacing, ~185 ms total) sent on COM5 to Phase 9E firmware; 5/5 responses arrived in send order; ROBOTOS_OBS `peak=5` (vs Phase 9E `peak=2`), `dropped=0`, `herr=0`, `unhandled=0`; ROBOTOS_UART `rx=ok=handled=5 full=0`; ROBOTOS_APP `transitions=3 uart=5 ignored=1`; Phase 6M producer healthy (`attempted=ok=60` at ticks=120); CFSR/HFSR `0x00000000` (13×); `accepted - dispatched = pending` invariant holds (65 − 64 = 1).
- **Runtime baseline:** Unchanged — still Phase 9E (`587dab7`). Phase 9G characterized the existing runtime; it added no firmware, no scheduler change, no UART TX scope change.
- **Next gate:** Phase 10A's gate remains in force — user must explicitly select one of (a) a `USER_DECISION_REQUIRED` row from `COMMAND_SET_DRAFT.md` §3, (b) Phase 9F demo polish, or (c) a continued hold. Phase 9G removes the highest-priority outstanding unknown ("high-rate UART input not stress-tested" from `PHASE_9EZ_CHECKPOINT.md §E`) but does not by itself authorize any Phase 10B opening.

---

### Phase 10A — Product Command Set Planning (docs-only)

- **Date:** 2026-05-11
- **Type:** Docs-only planning. No source, runtime, test, CMake, Zephyr, board, or script change.
- **Close status:** `CLOSED_DOCS_ONLY`
- **Closeout doc:** `RobotOS_v1.0/devkit/docs/PHASE_10A_CLOSE.md`
- **Companion doc:** `RobotOS_v1.0/devkit/docs/COMMAND_SET_DRAFT.md`
- **Phase log entry:** `RobotOS_v1.0/devkit/docs/DEVKIT_PROGRESS_PHASE_10.md` §5 (`<a id="phase-10a"></a>`)
- **Purpose:** Capture product command vocabulary and workload intent in writing before any Phase 10B-class implementation is authorized.
- **Runtime baseline:** Unchanged — still ends at **Phase 9E-Z** (audit) / **Phase 9E** (implementation, commit `587dab7`). Phase 10A does not introduce or modify any runtime behavior.
- **Next gate:** User must explicitly select one of (a) a `USER_DECISION_REQUIRED` row from `COMMAND_SET_DRAFT.md` §3 with its open notes answered, (b) a direction-independent supporting phase (Phase 9F demo polish or Phase 9G UART burst characterization), or (c) a continued hold. **No Phase 10B implementation is authorized.**

---

### Phase 9E-Z — Command Loop Checkpoint / Direction Guard (last runtime checkpoint)

- **Date:** 2026-05-09
- **Type:** Audit-only. No source, runtime, test, or Kconfig change.
- **Close status:** `CLOSED`
- **Full checkpoint:** `RobotOS_v1.0/devkit/docs/PHASE_9EZ_CHECKPOINT.md`
- **Verdict:** ON_TRACK. Phase 9A–9E stream proven complete. Scheduler DEFER confirmed. UART TX scope frozen (no shell/parser/protocol). Awaiting user product-direction decision (now captured in Phase 10A planning artifacts above).

---

### Phase 9E — UART TX Minimal Response

- **Commit:** `587dab7` (implementation); closeout docs in subsequent commit
- **Date:** 2026-05-09
- **Branch:** master
- **Type:** Devkit-local firmware + tooling. Minimal UART TX response path.
- **Close status:** `CLOSED`
- **Prior phase:** Phase 9Z checkpoint (`5b71daf`)

**Phase 9E delivered:**

First real hardware host→board→host command/response loop. Board receives UART
commands (PA3 RX), routes them through the RobotOS core event queue, dispatches
in thread context, and emits bounded UART TX responses (PA2 TX) via `uart_poll_out()`.

Validated with `run_phase9e_uart_response_demo.ps1 -ComPort COM5` (60 s):

| Command | Expected | Received |
| ------- | -------- | -------- |
| `a` | `OK state=ARMED` | ✓ |
| `s` | `OK state=ACTIVE` | ✓ |
| `?` | `STATE state=ACTIVE transitions=2 button=0 uart=3 ignored=0` | ✓ |
| `r` | `OK state=IDLE` | ✓ |
| `x` | `ERR ignored byte=0x78 state=IDLE` | ✓ |

RTT: ROBOTOS_UART rx=5 ok=5 handled=5; ROBOTOS_APP transitions=3 uart=5 ignored=1;
OBS accepted=25 dispatched=24 pending=1 peak=2 dropped=0; CFSR/HFSR=0 (13 occurrences).
Phase 6M producer healthy throughout (60 ok=60 at ticks=120).

**Files changed:** `devkit/src/devkit_app_state.h/.c` (+state_name API),
`devkit/src/devkit_uart_producer.c` (TX helpers), `tools/runtime/run_phase9e_uart_response_demo.ps1`.
No core/, platform/, tests/, CMakeLists.txt, or prj.conf change.

---

### Phase 9Z — Workload-Branch Checkpoint Review

- **Commit:** `8e8c801` (HEAD at checkpoint; no new source commit — docs-only)
- **Date:** 2026-05-08
- **Branch:** master
- **Type:** Audit-only / checkpoint. No firmware, CMake, Kconfig, or runtime change.
- **Close status:** `CLOSED`
- **Tag:** `v0.9d-workload-baseline`
- **Full audit:** `RobotOS_v1.0/devkit/docs/PHASE_9Z_CHECKPOINT.md`
- **Prior phase:** Phase 9D (`8e8c801`), Workload Demo Script & Runbook

**Phase 9Z verdict:** ON_TRACK_WITH_WATCHPOINTS. Core/platform boundaries intact.
All Phase 9A-A through 9D sub-phases closed with hardware RTT evidence on
STM32F411E-DISCO. Scheduler (7A/7B-1) DEFER — no workload-driven justification.
Phase 8A (F407) HOLD/DEFER. Next step: user product-direction decision.

**Phase 9D delivered:**

- `RobotOS_v1.0/tools/runtime/run_phase9d_demo.ps1` — PowerShell runner that orchestrates: wiring reminder → Phase 6O RTT capture (background job) → boot-settle wait → scripted UART `a`/`s`/`r` send to a configurable COM port → operator prompt for button presses → verification checklist printout. PowerShell 5.1 compatible; no parsing/automation overreach.
- `RobotOS_v1.0/devkit/docs/WORKLOAD_DEMO_9D.md` — canonical runbook covering hardware setup, build/flash, scripted and semi-scripted demo paths, the canonical demo scenario, full pass/fail criteria (required string patterns, deterministic counter targets, architecture invariants, fault registers), and known caveats (operator timing, line-ending, bounce, etc.).
- `RobotOS_v1.0/devkit/logs/phase_9D_workload_demo_2026-05-08.txt` — RTT capture from one execution of `run_phase9d_demo.ps1 -ComPort COM5`; 90 s window, 37930 bytes; harness exit 0 with 12 default + Phase 9D-specific patterns.
- `RobotOS_v1.0/devkit/logs/INDEX.md` — Phase 9D row.
- `RobotOS_v1.0/devkit/docs/DEVKIT_PROGRESS.md` — Phase 9D section.

**Workload demo anchor:** the runbook + runner are now the canonical entry point for demonstrating the current devkit workload (button + UART → app state machine). Future phases that depend on the Phase 9A-C/9B/9C event-pipeline contract can rerun the same demo to assert no regression.

**No Kconfig/prj.conf changes. No firmware changes.** Phase 9D source impact list:

- `core/`, `platform/`, `tests/` — untouched
- `devkit/src/` — untouched
- `devkit/CMakeLists.txt` — untouched
- `tools/runtime/capture_devkit_rtt.ps1` — untouched (the runner *invokes* it; doesn't modify it)

**Demo deviation note (this run).** The captured evidence shows a transition count of 35 (button=32, uart=3, ignored=0) rather than the canonical 6/3/3/0 target. Cause: the operator pressed the user button repeatedly during the boot-settle and post-prompt windows rather than exactly 3 times after the prompt. The runbook's "Known limitations §7 — Operator timing" caveat anticipates this. The evidence is fully explainable, all architecture invariants hold, and the system handled rapid input with `peak=16` (queue-capacity briefly reached) and `dropped=3` while keeping `herr=0`, `unhandled=0`, and CFSR/HFSR=0 throughout. A subsequent careful operator run can hit the canonical 6/3/3/0 target without any firmware change.

**Phase 9C** (prior): Application state machine, commit `286e61b`.
**Phase 9B** (prior): Devkit UART RX producer, commit `85389f4`.
**Phase 9A-C** (prior): Gate Phase 6I startup burst, commit `3989ff9`.
**Phase 9A-B** (prior): Button debounce refinement, commit `92de5e0`.
**Phase 9A-A** (prior): Button EXTI producer, commit `2068180`, CLOSED with BOUNCE_OBSERVED.

---

## Validation Evidence (Phase 9D workload demo)

Phase 9D adds **no firmware change**, so build/flash artifacts are unchanged
from Phase 9C. The evidence below is from one execution of
`run_phase9d_demo.ps1 -ComPort COM5` against the Phase 9C firmware
(commit `286e61b`).

| Gate | Result | Detail |
| ---- | ------ | ------ |
| Source/runtime mutation | NONE | No firmware, CMake, prj.conf, or Phase 6O harness change. Only added: runner script + runbook + evidence log + state-doc updates. |
| PowerShell parse | PASS | `[System.Management.Automation.PSParser]::Tokenize` clean on `run_phase9d_demo.ps1` |
| Demo runner exit | PASS | All 12 required patterns FOUND; harness exit 0 |
| Phase 9C app state init | FOUND | `state=IDLE` baseline at boot |
| ROBOTOS_OBS state=READY | FOUND | Baseline + 18 periodic emissions (ticks=0,10,…,180) |
| ROBOTOS_FAULT active=0 | FOUND | All 19 emissions; CFSR=0 HFSR=0 throughout (19 occurrences) |
| ROBOTOS_PROD attempted= | FOUND | Phase 6M producer healthy across full window |
| ROBOTOS_BTN | FOUND | Operator pressed many times; producer absorbed input cleanly |
| ROBOTOS_UART | FOUND | rx=3 ok=3 full=0 invalid=0 other=0 handled=3 last=0x72 — exact match to scripted payload `a`/`s`/`r` |
| ROBOTOS_APP | FOUND | Final at ticks=180: state=IDLE transitions=35 button=32 uart=3 ignored=0 last_src=BTN last_byte=0x72 |
| Phase 9C app transition | FOUND | 35 transition lines, both `src=BTN` and `src=UART` exercised in the same capture |
| CFSR | 0x00000000 throughout | 19 occurrences checked |
| HFSR | 0x00000000 throughout | 19 occurrences checked |
| App conservation | PASS | transitions(35) = button(32) + uart(3) − ignored(0) ✓ |
| Architecture invariants | PASS | accepted=125 dispatched=124 pending=1 ✓; peak=16 (queue capacity briefly reached during rapid button input); dropped=3 (rapid burst overflow); herr=0; unhandled=0; rejected=0; throttled=0; accepted = Phase 6M(90)+button(32)+UART(3) = 125 ✓ |
| Stress observation | NOTABLE | Operator pressed button rapidly enough to fill the 16-slot queue; system handled the saturation event safely (3 dropped events, no fault). This is additional confidence beyond what Phase 9C demonstrated (peak=4). |
| Canonical-target deviation | DOCUMENTED | Runbook target is `transitions=6 button=3 uart=3 ignored=0`; this run produced 35/32/3/0. Cause is operator timing (presses started during boot wait and continued through and after the UART send window). Runbook §7 anticipates this; firmware behavior is correct in both cases. |

Capture log: `RobotOS_v1.0/devkit/logs/phase_9D_workload_demo_2026-05-08.txt`
Runner script: `RobotOS_v1.0/tools/runtime/run_phase9d_demo.ps1`
Runbook: `RobotOS_v1.0/devkit/docs/WORKLOAD_DEMO_9D.md`

**Phase 6Z verification highlights:**

- Phase 6K: ROBOTOS_OBS baseline + 12 periodic emissions; `peak=16` reached during Phase 6I burst, then steady-state `pending=1` after ticks=40.
- Phase 6L: ROBOTOS_FAULT baseline + 12 periodic emissions; CFSR/HFSR zero throughout; `active=0`, `context=none` invariant.
- Phase 6M: ROBOTOS_PROD baseline + 12 periodic emissions; producer init banner present; `attempted` grew exactly +5 per 10 ticks (60 attempts total over 60 s); `ok=attempted`, `dropped=0` for the entire capture.
- Phase 6I final summary: `attempted=24 ok=17 full=7 handled=16` (one-event timing variance vs documented `ok=16 full=8`; architectural invariants preserved — see Phase 6Z section in `DEVKIT_PROGRESS.md`).
- Counter cross-check at ticks=120: Phase 6I `ok` (17) + Phase 6M `ok` (60) = `accepted` (77); Phase 6I handled (16) + Phase 6M handled (60) = `dispatched` (76); `accepted - dispatched = pending = 1`.

---

## Behavior Guarantees Through Phase 6Z

Phase 6Z is evidence/state-reconciliation only and changed no source.
The following invariants from Phase 6J carry forward unchanged through
Phase 6K / 6L / 6M and are confirmed by the Phase 6Z RTT capture:

- **No new status codes.** `ROBOTOS_CORE_ERR_THROTTLED = -7` remains the highest-numbered error.
- **No scheduler semantics changed.** `ROBOTOS_CORE_MAX_EVENTS_PER_TICK = 1` unchanged.
- **No auto-retry behavior.** Retry policy remains pure advisory stateless mapping.
- **No dispatch lock-boundary regression.** Handler-outside-lock invariant confirmed preserved.
- **No admission policy change.** NONE/reserved rejected; USER+ accepted: unchanged.
- **`peak_queue_depth` is passive.** Updated only after successful push, under existing lock. Monotonically non-decreasing. Not reset by repeated init. Zero behavioral impact. Phase 6Z RTT confirmed `peak=16 = ROBOTOS_EVENT_QUEUE_CAPACITY` reached during the Phase 6I burst.
- Existing `post_event` / `try_post_event` semantics unchanged.
- **Phase 6K/6L/6M observability surfaces are passive.** ROBOTOS_OBS, ROBOTOS_FAULT, and ROBOTOS_PROD logs do not feed into scheduling, admission, throttle, dispatch, or retry decisions. Verified by Phase 6Z behavior coherence check.
- **Phase 6M producer cadence is fixed at compile time.** 1 post / 2 ticks at `DEVKIT_TICK_MS=500`; producer never reads core counters. Verified by Phase 6Z `attempted` growth of exactly +5 per 10 ticks.

---

## Files Changed in Phase 6Z

| File | Change |
| ---- | ------ |
| `RobotOS_v1.0/devkit/logs/phase_6Z_rtt_2026-05-07.txt` | New -- raw RTT capture, 60 s, 16,560 bytes |
| `RobotOS_v1.0/devkit/docs/DEVKIT_PROGRESS.md` | Phase 6K/6L/6M close-status flips; Phase 6Z section appended |
| `CURRENT_STATE.md` | Last-closed phase advanced to Phase 6M (closed via Phase 6Z) |

No source code, tests, CMake, Kconfig, prj.conf, or runtime behavior
modified in Phase 6Z. See `DEVKIT_PROGRESS.md` Phase 6Z Architecture
Preservation Audit for the explicit list.

For prior-phase file history (e.g. Phase 6J/6K/6L/6M originating
commits), refer to `DEVKIT_PROGRESS.md`.

---

## Phase History Summary (completed, confirmed from DEVKIT_PROGRESS.md)

The following phases are closed and committed. This list is not exhaustive — refer
to `DEVKIT_PROGRESS.md` for the full history.

| Phase | Description | Commit |
| ----- | ----------- | ------ |
| 9E-Z | Command Loop Checkpoint / Direction Guard (audit-only; ON_TRACK; UART TX scope frozen; scheduler DEFER; full doc in PHASE_9EZ_CHECKPOINT.md) | `30bae27` |
| 9E | UART TX Minimal Response (first host↔board command/response loop; `uart_poll_out()` from thread ctx; 5-command validation a/s/?/r/x) | `587dab7` |
| 9Z | Workload-Branch Checkpoint Review (audit-only; ON_TRACK_WITH_WATCHPOINTS; tag v0.9d-workload-baseline; full audit in PHASE_9Z_CHECKPOINT.md) | `8e8c801` |
| 9D | Workload Demo Script & Runbook (run_phase9d_demo.ps1 + WORKLOAD_DEMO_9D.md; canonical scenario `a`/`s`/`r` UART + 3 button presses → IDLE/ARMED/ACTIVE; tooling/docs only) | `8e8c801` |
| 9C | Devkit Minimal Application State Machine (button + UART → IDLE/ARMED/ACTIVE; 23 transitions; first multi-source workload composition) | `286e61b` |
| 9B | Devkit UART RX Producer (second real hardware event source; USER+3; rx=7 ok=7 full=0 handled=7) | `85389f4` |
| 9A-C | Gate Phase 6I Startup Burst (devkit-local compile-time gate; default disabled; full 4→0, dropped 13→0) | `3989ff9` |
| 9A-B | Devkit Button Debounce Refinement (30 ms time-guard; full 98→4) | `92de5e0` |
| 9A-A | Devkit Button EXTI Producer (first real hardware workload) | `2068180` |
| 6O | Reusable RTT Streaming Capture Harness (tooling) | `6cb979f` |
| 6N | Documentation / Navigation Consolidation (docs-only) | `ad52de5` |
| 6Z | RTT closeout for 6K/6L/6M (docs/evidence only) | `4ec5b86` |
| 6M | Producer Realism / Timer Producer Diagnostic | `a6b253b` |
| 6L | Fault Observability Integration | `d3759a7` |
| 6K | Runtime Observability Surfacing | `11516d4` |
| 6J | Observability and Contract Stress Expansion | `8a1af69` |
| 6I | Timer Producer Queue-Pressure Stress | `e78e503` |
| 6H | ISR/Timer Producer Stress-Lite | `22edfee` |
| 6G | Timer producer smoke (ISR context) | `335ee29` |
| 6F | Devkit Mixed Event Policy Smoke | `5bca62f` |
| 6A–6E | Event pipeline smokes | preceding commits |
| 5F/5F-R | Dispatch split + runtime confirm | `b7f08fe`, `460b9f1` |
| 5E | Critical boundary applied to core queue | `ff24147` |
| 5D | Platform critical section boundary | `4187bb3` |
| 4K | Scheduler admission + throttle policy | preceding commits |
| 4L | Scheduler advisory retry decision policy | `a3f4369` |

---

## Next Candidate Phases

| Phase | Description | Status |
| ----- | ----------- | ------ |
| Phase 9A-B | Devkit button debounce refinement (30 ms time-guard, no core change) | **CLOSED** — full 98→4; debounce=54; see Phase 9A-B section in DEVKIT_PROGRESS.md |
| Phase 9A-C | Gate Phase 6I startup burst (devkit-local compile-time gate; default disabled) | **CLOSED** — full 4→0; dropped 13→0; peak=14; see Phase 9A-C section in DEVKIT_PROGRESS.md |
| Phase 9B | UART RX producer (second real hardware event source; USER+3) | **CLOSED** — rx=7 ok=7 full=0 handled=7; per-byte handler logs match payload `abc123\n`; see Phase 9B section in DEVKIT_PROGRESS.md |
| Phase 9C | Minimal application state machine (compose button + UART into IDLE/ARMED/ACTIVE) | **CLOSED** — 23 transitions; button=20 uart=3 ignored=0; peak=4; dropped=0; see Phase 9C section in DEVKIT_PROGRESS.md |
| Phase 9D | Workload demo script & runbook (`run_phase9d_demo.ps1` + `WORKLOAD_DEMO_9D.md`; tooling/docs only) | **CLOSED** — 12 default + Phase 9D patterns FOUND; queue saturation observed safely (peak=16, dropped=3, herr=0); CFSR/HFSR=0 throughout; see Phase 9D section |
| Phase 9E-Z | Command loop checkpoint / direction guard (audit-only; ON_TRACK) | **CLOSED** — PHASE_9EZ_CHECKPOINT.md; scheduler DEFER confirmed; UART TX scope frozen |
| Phase 9E | Minimal UART TX response (`uart_poll_out()`, 5-command loop `a`/`s`/`?`/`r`/`x`) | **CLOSED** — all 5 host responses correct; RTT rx=5 ok=5 handled=5; CFSR/HFSR=0; see Phase 9E section |
| Phase 9Z | Workload-branch checkpoint review (audit-only; no source change) | **CLOSED** — ON_TRACK_WITH_WATCHPOINTS; full audit in `PHASE_9Z_CHECKPOINT.md`; tag `v0.9d-workload-baseline` |
| Phase 9F | Command-response polish (richer command set, button TX echo, structured response) | Candidate — only if explicitly approved |
| Phase 8A | Custom STM32F407 board bring-up | **Candidate** — retires 25-phase portability debt; use `capture_devkit_rtt.ps1 -OpenOcdConfig <f407.cfg>`; remains HOLD/DEFER until user reopens |
| Phase 7B-1 | Dispatch Budget Test Parameterization | Candidate — only if workload evidence reveals saturation; Phase 9A-C clean baseline shows budget=1 still adequate (peak=14, dropped=0) |
| Phase 7A | Dispatch Budget Evolution Planning | DEFER — Phase 9A-A workload data shows ~112 events / 60 s sustained, no workload-driven reason for budget mutation |
| Custom STM32F407 target | Migration / validation | HOLD/DEFER — not yet exercised; Phase 8A addresses when reopened |

**Primary workload anchor:** event-driven embedded device runtime, validated with the Phase 9A-A user-button input source. Future workloads can be layered using the same producer/handler pattern (see `devkit_button_producer.c` as the reference template).

Dispatch budget remains `ROBOTOS_CORE_MAX_EVENTS_PER_TICK = 1`. Any
mutation requires explicit team decision and a workload-driven
justification beyond what Phase 6Z evidence currently supports.

---

## GLM Token Optimization Note

GLM token optimization is intentionally excluded until Level 15-17 stabilization is closed. GLM operates under its own policy and is not subject to the delta-command optimizations being applied to Claude Pro and Copilot.

---

## Operating Notes

**Token/context baseline (2026-05-03):** Closed for now; monitor only. Future agents should use `CURRENT_STATE.md` as lightweight startup signal and `PHASE_CLOSE_TEMPLATE.md` as closeout structure to reduce repeated context reconstruction.

**Instruction discipline update:** Copilot/Claude instruction update committed at `6227e17` — *do not reduce validation depth for token savings*. Keep correctness checks even when optimizing context overhead. Caveat: commit `6227e17` included 131 pre-existing lines of GLM Evidence Logging content alongside new material; no revert planned.

**Validation preservation:** Never skip build/test/hardware validation steps to save tokens. Token optimization applies to repo documentation state-tracking, not to correctness gates.

**GLM status:** Excluded from token-shortening until Level 15–17 stabilization closes. GLM operates under separate policy constraints.

---

## Maintenance Rule

- Update this file **only at phase close**, from validated evidence only.
- Do not record assumptions as fact.
- If a fact is unknown, mark it **Pending / Unknown**.
- Detailed history (logs, diffs, RTT evidence) remains in `DEVKIT_PROGRESS.md`.
- This file is the lightweight startup signal; `DEVKIT_PROGRESS.md` is the audit trail.
