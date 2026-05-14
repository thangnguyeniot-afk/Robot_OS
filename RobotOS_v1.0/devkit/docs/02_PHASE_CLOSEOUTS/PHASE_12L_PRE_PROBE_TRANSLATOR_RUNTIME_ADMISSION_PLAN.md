# Phase 12L-pre — Probe Translator Runtime Admission Plan

**Status:** `CLOSED_DOCS_ONLY`
**Decision:** `PHASE_12L_PRE_PROBE_TRANSLATOR_RUNTIME_ADMISSION_PLAN_CLOSED`
**Date:** 2026-05-14
**Type:** Docs-only runtime admission planning gate. No source, CMake, Kconfig,
`prj.conf`, DTS, overlay, test, or log change. No implementation.
**Prior phase anchor:** [`PHASE_12KZ_BUILD_ADMISSION_CHECKPOINT.md`](PHASE_12KZ_BUILD_ADMISSION_CHECKPOINT.md)
**Phase 12K anchor:** [`PHASE_12K_PROBE_TRANSLATOR_ZEPHYR_BUILD_ONLY_ADMISSION.md`](PHASE_12K_PROBE_TRANSLATOR_ZEPHYR_BUILD_ONLY_ADMISSION.md)
**Implementation contract (produced here):** [`../03_SPECS/PROBE_TRANSLATOR_RUNTIME_ADMISSION_PLAN.md`](../03_SPECS/PROBE_TRANSLATOR_RUNTIME_ADMISSION_PLAN.md)

---

## A. Executive Summary

Phase 12L-pre is a docs-only planning gate. It audits the devkit runtime
boundary and decides the safest next runtime-wiring contract for
`probe_translator`. No implementation is performed here.

**What Phase 12L-pre does:**

- Audits the devkit runtime call paths, UART event flow, and
  `devkit_app_state` integration points.
- Evaluates candidate entry points for Phase 12L runtime wiring.
- Selects and records the recommended Phase 12L implementation
  contract.
- Defines the allowed / forbidden / conditional file set for Phase 12L.
- Produces a long-lived implementation spec:
  `PROBE_TRANSLATOR_RUNTIME_ADMISSION_PLAN.md`.

**What Phase 12L-pre does not do:**

- Does not implement any runtime wiring.
- Does not add UART commands, responses, or behavior.
- Does not modify any source, header, CMake, Kconfig, `prj.conf`,
  DTS, overlay, test, or log file.
- Does not wire `probe_translator` into `devkit_app_state` or
  `devkit_runtime`.
- Does not claim hardware validation.

**Decision:** `PHASE_12L_PRE_PROBE_TRANSLATOR_RUNTIME_ADMISSION_PLAN_CLOSED`

---

## B. Baseline From Phase 12K-Z

| Item | Value |
| --- | --- |
| HEAD at Phase 12L-pre open | `58e532f docs: add Phase 12K-Z build admission checkpoint` |
| `origin/master` | `58e532f` (synced) |
| Phase 12K build-admission baseline | `a4b49ad` (Zephyr build PASS; host 23/23 PASS) |
| `probe_translator` status | Build-admitted; NOT runtime-wired; NOT called by devkit |
| CMake state | `devkit/CMakeLists.txt` includes framework + `probe_translator.c` sources and include paths |
| `app/probe_translator/CMakeLists.txt` | Not created |
| `app/probe_translator/Kconfig` | Not created |
| Working tree | Clean on all tracked files |

---

## C. Runtime Boundary Audit

### C.1 Devkit runtime call path (confirmed from source)

```
devkit/src/main.c
    → devkit_runtime_init()
        → devkit_app_state_init()
        → devkit_timer_producer_init()
        → devkit_button_producer_init()
        → devkit_uart_producer_init()    [registers UART handler for USER+3]
        → devkit_observability_log_snapshot()
        → [...]
    → devkit_runtime_run()   [tick loop]
        → devkit_status_led_toggle()
        → robotos_core_tick()            [dispatches queued events]
        → devkit_timer_producer_on_tick()
        → devkit_observability_log_*() every N ticks
```

### C.2 UART event path (confirmed from source)

```
External UART byte
    → Zephyr UART RX ISR (devkit_uart_producer.c)
    → robotos_core_post_event(type=USER+3, arg0=marker, arg1=byte)
    → core event queue
    → robotos_core_tick() dispatches one event/tick
    → devkit_uart_handler() [thread context]
    → devkit_app_state_on_uart_byte(byte, handler_count)
        UART byte 'a' → devkit_app_state: *→ARMED
        UART byte 's' → devkit_app_state: *→ACTIVE
        UART byte 'r' → devkit_app_state: *→IDLE
        UART byte 'd' → devkit_app_state: ARMED→IDLE (no-op from IDLE; USER_DECISION_REQUIRED from ACTIVE)
        UART byte 'v' → build query (no state change)
        UART byte 'l' → LED command (no state change)
        UART byte 't' → accelerometer probe (no state change)
        UART byte '?' → state query (no state change)
        default       → ignored_count++
```

### C.3 Button event path (confirmed from source)

```
Physical button press
    → Zephyr EXTI (devkit_button_producer.c)
    → robotos_core_post_event(type=USER+2)
    → core event queue
    → robotos_core_tick() dispatches one event/tick
    → devkit_button_handler() [thread context]
    → devkit_app_state_on_button(seq)
        cycles: IDLE→ARMED→ACTIVE→IDLE
```

### C.4 `probe_translator` public API surface (confirmed from source)

| Function | Signature | Purpose |
| --- | --- | --- |
| `probe_translator_init` | `(pt, config)` | Initialize FSM + bridge; sets state=IDLE |
| `probe_translator_dispatch_adapter_event` | `(pt, adapter_type, adapter_arg0, payload)` | Forward through bridge into FSM |
| `probe_translator_reset` | `(pt)` | Reset FSM to IDLE; clear counters |
| `probe_translator_get_snapshot` | `(pt, out)` | Copy FSM + bridge snapshot |

No Zephyr include. No `devkit_*.h` include. No UART dependency. No scheduler dependency. Boundary clean.

### C.5 Probe translator event mapping (confirmed from `probe_translator.c`)

| Dispatch `(type, arg0)` | FSM event | Transition |
| --- | --- | --- |
| `(TYPE_CONFIG, ARG_NONE)` | EVT_CONFIGURED | IDLE → READY |
| `(TYPE_COMMAND, ARG_START)` | EVT_START | READY → ACTIVE |
| `(TYPE_COMMAND, ARG_STOP)` | EVT_STOP | ACTIVE → READY |
| `(TYPE_COMMAND, ARG_RESET)` | EVT_RESET | IDLE/READY/ACTIVE → IDLE |
| `(TYPE_FAULT, ARG_ANY)` | EVT_FAULT | IDLE/READY/ACTIVE → FAULT |

No FSM row for `FAULT+FAULT`: EVT_FAULT from FAULT state is bridge-mapped
(mapped_count increments) but FSM has no matching transition row
(no_transition_count increments). State stays FAULT. Sticky FAULT is
correct behavior by design.

### C.6 devkit_app_state vs. probe_translator state vocabulary

| `devkit_app_state_t` | Analogous `probe_translator` state | Notes |
| --- | --- | --- |
| IDLE | IDLE (1u) | Both start here |
| ARMED | READY (2u) | ARMED = "configured, not yet active" |
| ACTIVE | ACTIVE (3u) | Both indicate running |
| (no equivalent) | FAULT (4u) | Not yet surfaced in devkit_app_state |

State divergence is **expected and safe**. `probe_translator` is a separate
FSM. If a UART command dispatches an event that does not match the current
probe_translator state, the FSM silently increments `no_transition_count`
and returns `ROBOTOS_CORE_OK` — no error, no crash. The two FSMs track
different semantic concepts and may diverge.

### C.7 Thread-context safety (confirmed)

All candidate call sites are thread-context only:
- `devkit_app_state_on_uart_byte()` — called from core dispatcher
  thread; never from ISR.
- `devkit_app_state_on_button()` — same.
- `devkit_runtime_init()` — called before the tick loop starts.
- `devkit_runtime_run()` — tick loop body; thread context.

`probe_translator_dispatch_adapter_event()` and
`probe_translator_init()` must NOT be called from ISR context
(per Framework API contract in `robotos_fw_fsm.h:282`). All proposed
call sites are thread-context only. Contract is satisfied.

---

## D. Candidate Runtime Entry Points

### D.1 Candidate A — Direct call from `devkit_app_state_on_uart_byte`

**Shape:** Add `probe_translator_dispatch_adapter_event()` calls directly
inside `devkit_app_state.c` at the 'a', 's', 'r', 'd' case handlers. A
module-static `probe_translator_t` instance lives in `devkit_app_state.c`.

| Property | Assessment |
| --- | --- |
| Affected files | `devkit_app_state.c` (modified), no new files |
| Boundary risk | **HIGH.** `devkit_app_state.c` gains a direct dependency on `probe_translator.h`. This violates the layering intent — devkit_app_state is a devkit-internal module and probe_translator is an application-layer module. |
| UART behavior change | None — no new bytes, no new responses |
| Product coupling | Moderate — app-layer state hidden inside devkit state module |
| Rollback path | Simple: revert `devkit_app_state.c` |
| **Verdict** | **REJECTED** — boundary violation; app-layer ownership inside devkit module |

### D.2 Candidate B — New devkit-local adapter module (`devkit_probe_adapter`)

**Shape:** Create `devkit/src/devkit_probe_adapter.{c,h}`. This module
owns the `probe_translator_t` instance (static), wraps the API, and is
called from `devkit_app_state.c` and `devkit_runtime.c`. The probe_translator
itself is never included directly from `devkit_app_state.c`.

| Property | Assessment |
| --- | --- |
| Affected files | `devkit_probe_adapter.{c,h}` (new), `devkit_app_state.c` (add call), `devkit_runtime.c` (add init + log), `devkit/CMakeLists.txt` (add source) |
| Boundary risk | **LOW.** `devkit_app_state.c` only includes `devkit_probe_adapter.h` (devkit-local). The adapter contains the app-layer dependency. Layering is preserved. |
| UART behavior change | None — no new bytes, no new responses |
| Product coupling | Minimal — adapter maps devkit command signals to probe_translator adapter-key tuples |
| Validation path | Zephyr build PASS (build-level proof); existing host 23/23 unchanged; no hardware required |
| Rollback path | Revert `devkit/CMakeLists.txt` + delete `devkit_probe_adapter.{c,h}` + revert `devkit_app_state.c` + revert `devkit_runtime.c` |
| **Verdict** | **RECOMMENDED.** Cleanest boundary. Minimal scope. Fully testable at build depth. |

### D.3 Candidate C — Integration with UART TX response path

**Shape:** After dispatching to probe_translator, emit a new UART TX
response encoding the probe_translator snapshot.

| Property | Assessment |
| --- | --- |
| UART behavior change | **YES** — modifies UART TX surface |
| Risk | **HIGH** — adds to the frozen UART command/response surface |
| **Verdict** | **REJECTED** — out of scope at Phase 12L; deferred to product command mapping gate (USER_DECISION_REQUIRED) |

### D.4 Candidate D — Register a new `robotos_core` event handler

**Shape:** Register a second handler for UART event type (USER+3) that
directly calls probe_translator.

| Property | Assessment |
| --- | --- |
| Boundary risk | **HIGH** — the core supports only one handler per event type in the current design; a second registration for USER+3 would be undefined behavior or fail |
| UART behavior | No change; but event routing is unclear |
| **Verdict** | **REJECTED** — core does not support multi-handler registration at this depth |

### D.5 Candidate E — Init only (no dispatch wiring)

**Shape:** Create `devkit_probe_adapter.{c,h}`, call `init()` from
`devkit_runtime_init()`, and call `log_snapshot()` periodically.
No `dispatch_adapter_event()` calls wired.

| Property | Assessment |
| --- | --- |
| Risk | **LOWEST** |
| Proof value | **LOW** — does not prove runtime dispatch path; essentially a variant of Phase 12K build admission |
| **Verdict** | **REJECTED for Phase 12L** — does not satisfy the purpose of runtime admission; deferred init-only is a degenerate case. Init-only wiring produces no new runtime evidence. |

---

## E. Recommended Phase 12L Contract

**Decision: Candidate B — New devkit-local adapter module.**

### E.1 New module: `devkit_probe_adapter`

Create `RobotOS_v1.0/devkit/src/devkit_probe_adapter.h` and
`RobotOS_v1.0/devkit/src/devkit_probe_adapter.c`.

The adapter:
- Declares a module-static `probe_translator_t` instance.
- Exposes three functions:

```c
/* Initialize the adapter; must be called once from devkit_runtime_init()
 * after devkit_app_state_init(). Returns ROBOTOS_CORE_OK on success. */
robotos_core_status_t devkit_probe_adapter_init(void);

/* Dispatch an adapter event to the probe translator.
 * adapter_type and adapter_arg0 are PROBE_ADAPTER_* constants from
 * probe_translator.h. Returns probe_translator_dispatch_adapter_event status. */
robotos_core_status_t devkit_probe_adapter_dispatch(
    uint32_t adapter_type, uint32_t adapter_arg0);

/* Emit one log line with the probe translator FSM + bridge snapshot. */
void devkit_probe_adapter_log_snapshot(void);
```

### E.2 Command mapping (Phase 12L-pre locked)

This mapping translates `devkit_app_state` command signals to probe_translator
adapter-key tuples. It must be implemented exactly as specified here.

| UART byte | `devkit_app_state` action | `devkit_probe_adapter_dispatch` call |
| --- | --- | --- |
| `'a'` / `'A'` | → ARMED | `(PROBE_ADAPTER_TYPE_CONFIG, PROBE_ADAPTER_ARG_NONE)` |
| `'s'` / `'S'` | → ACTIVE | `(PROBE_ADAPTER_TYPE_COMMAND, PROBE_ADAPTER_ARG_START)` |
| `'r'` / `'R'` | → IDLE | `(PROBE_ADAPTER_TYPE_COMMAND, PROBE_ADAPTER_ARG_RESET)` |
| `'d'` / `'D'` | ARMED→IDLE (or no-op) | `(PROBE_ADAPTER_TYPE_COMMAND, PROBE_ADAPTER_ARG_RESET)` |
| button | cycles IDLE→ARMED→ACTIVE→IDLE | same as 'a' / 's' / 'r' per cycle position |

**State divergence is intentional and handled:** If `probe_translator`
receives a dispatch for an event that does not match its current state
(e.g., `ARG_START` while in IDLE), the FSM increments
`no_transition_count` and returns `ROBOTOS_CORE_OK`. No error is raised.
The two FSMs track separate semantic concepts and may diverge.

**FAULT adapter type** (`TYPE_FAULT`): Not wired at Phase 12L. Deferred
until a FAULT signal source is defined. The probe_translator mapping table
already supports it (wildcard row) but no hardware fault signal is
admitted at this phase.

**Dispatch return value:** On non-OK return from
`devkit_probe_adapter_dispatch()`, the adapter logs the error and continues.
It does NOT propagate non-OK to the caller (`devkit_app_state_on_uart_byte`).
Rationale: `devkit_app_state` must not fail a UART command due to probe_translator
state. The probe_translator is additive; it does not own the UART command flow.

### E.3 Integration points

**`devkit_runtime.c` (modified):**
- After `devkit_app_state_init()`: add `devkit_probe_adapter_init()`.
  Log error and continue on failure (mirror the existing producer-init
  error-log-and-continue pattern).
- In the periodic observability block: add `devkit_probe_adapter_log_snapshot()`.
- `#include "devkit_probe_adapter.h"` added at the top.

**`devkit_app_state.c` (modified):**
- `#include "devkit_probe_adapter.h"` added.
- In `devkit_app_state_on_uart_byte()`:
  - After 'a' transition: add `devkit_probe_adapter_dispatch(PROBE_ADAPTER_TYPE_CONFIG, PROBE_ADAPTER_ARG_NONE)`.
  - After 's' transition: add `devkit_probe_adapter_dispatch(PROBE_ADAPTER_TYPE_COMMAND, PROBE_ADAPTER_ARG_START)`.
  - After 'r' transition: add `devkit_probe_adapter_dispatch(PROBE_ADAPTER_TYPE_COMMAND, PROBE_ADAPTER_ARG_RESET)`.
  - After 'd' transition (ARMED→IDLE case): add `devkit_probe_adapter_dispatch(PROBE_ADAPTER_TYPE_COMMAND, PROBE_ADAPTER_ARG_RESET)`.
- In `devkit_app_state_on_button()`:
  - After IDLE→ARMED: add `devkit_probe_adapter_dispatch(PROBE_ADAPTER_TYPE_CONFIG, PROBE_ADAPTER_ARG_NONE)`.
  - After ARMED→ACTIVE: add `devkit_probe_adapter_dispatch(PROBE_ADAPTER_TYPE_COMMAND, PROBE_ADAPTER_ARG_START)`.
  - After ACTIVE→IDLE: add `devkit_probe_adapter_dispatch(PROBE_ADAPTER_TYPE_COMMAND, PROBE_ADAPTER_ARG_RESET)`.

**`devkit/CMakeLists.txt` (modified):**
- Add `src/devkit_probe_adapter.c` to the existing devkit `target_sources` block.

### E.4 UART behavior locked (unchanged)

The command set `a/s/r/?/x/v/L/d/T` is **frozen**. Phase 12L adds no new
UART bytes. Phase 12L adds no new UART TX responses. Phase 12L does not
modify `devkit_uart_producer.c`.

### E.5 No scheduler change

Phase 12L does not add Zephyr tasks, timers, threads, `k_sleep`, or
`k_sem_*` calls. All probe_translator dispatch calls are synchronous,
in thread context, and return quickly.

### E.6 No UART TX response change

`devkit_uart_producer.c` is zero-diff at Phase 12L. The probe_translator
snapshot is observable only via Zephyr RTT log (`LOG_INF`) from
`devkit_probe_adapter_log_snapshot()`. No new UART TX encoding is
introduced. Product command mapping (which could add UART TX) remains
`NOT_STARTED; USER_DECISION_REQUIRED`.

---

## F. Allowed / Forbidden / Conditional File Set for Phase 12L

### F.1 New files (allowed and required)

| Path | Purpose |
| --- | --- |
| `RobotOS_v1.0/devkit/src/devkit_probe_adapter.h` | Adapter public API (3 functions) |
| `RobotOS_v1.0/devkit/src/devkit_probe_adapter.c` | Adapter implementation; owns static `probe_translator_t` |
| `RobotOS_v1.0/devkit/logs/phase_12L_build_<YYYY-MM-DD>.txt` | Committed Zephyr build transcript |
| `RobotOS_v1.0/devkit/docs/02_PHASE_CLOSEOUTS/PHASE_12L_PROBE_TRANSLATOR_RUNTIME_ADMISSION.md` | Phase 12L closeout |

### F.2 Modified files (additive or targeted changes only)

| Path | Change |
| --- | --- |
| `RobotOS_v1.0/devkit/CMakeLists.txt` | Add `src/devkit_probe_adapter.c` to `target_sources` |
| `RobotOS_v1.0/devkit/src/devkit_runtime.c` | Add `#include`, `devkit_probe_adapter_init()`, `devkit_probe_adapter_log_snapshot()` |
| `RobotOS_v1.0/devkit/src/devkit_app_state.c` | Add `#include`, `devkit_probe_adapter_dispatch()` calls at 'a'/'s'/'r'/'d' and button transitions |
| `CURRENT_STATE.md` | Phase 12L as latest closed |
| `RobotOS_v1.0/devkit/docs/01_PROGRESS/DEVKIT_PROGRESS_PHASE_11_20.md` | Phase 12L index row + section |
| `RobotOS_v1.0/devkit/docs/00_INDEX/README.md` | Phase 12L closeout link |

### F.3 Forbidden at Phase 12L

| Forbidden path | Reason |
| --- | --- |
| `RobotOS_v1.0/devkit/prj.conf` | No new Kconfig dependency. Zero-diff. |
| Any `*.dts`, `*.dtsi`, `*.overlay` | No hardware dependency. Zero-diff. |
| `RobotOS_v1.0/devkit/src/devkit_uart_producer.{c,h}` | UART TX surface frozen; no new responses. |
| `RobotOS_v1.0/framework/*.{h,c}` | Framework source used as-is. Zero-diff. |
| `RobotOS_v1.0/app/probe_translator/probe_translator.{h,c}` | Application source used as-is. Zero-diff. |
| `RobotOS_v1.0/app/probe_translator/CMakeLists.txt` | Not created at Phase 12L (not needed). |
| `RobotOS_v1.0/app/probe_translator/Kconfig` | No Kconfig flag needed. Not created. |
| `RobotOS_v1.0/core/*` | Zero-diff. |
| `RobotOS_v1.0/platform/*` | Zero-diff. |
| Any new event type registration in `robotos_core` | No new `robotos_core_register_event_handler()` call. |
| Any UART TX response for probe_translator snapshot | Deferred; product command mapping gate. |
| Any hardware log (RTT, J-Link, flash) | Out of Phase 12L scope. |

### F.4 Conditional (only if validation requires)

| Path | Condition |
| --- | --- |
| `RobotOS_v1.0/devkit/src/devkit_app_state.h` | Zero-diff preferred; modify only if function signature changes are needed (not expected). |
| `RobotOS_v1.0/devkit/src/devkit_runtime.h` | Zero-diff preferred; modify only if new API needs to be declared in the header (not expected). |

---

## G. Validation Plan for Phase 12L

| # | Gate | Command / Evidence |
| --- | --- | --- |
| 1 | Zephyr build PASS | `py -m west build --pristine=always -d build-phase12l -b stm32f411e_disco RobotOS_v1.0/devkit` — exit 0; FLASH/RAM delta reported |
| 2 | Host regression PASS | `cmake -S RobotOS_v1.0/tests/host -B build-phase12l-host && cmake --build build-phase12l-host && ctest --test-dir build-phase12l-host` — **23/23 PASS** (all prior host tests unchanged; no new host test target required since probe_translator logic is already validated at Phase 12I/12J) |
| 3 | CMake diff scope | `git diff -- devkit/CMakeLists.txt` — only `src/devkit_probe_adapter.c` added |
| 4 | `prj.conf` zero-diff | `git diff -- devkit/prj.conf` — empty |
| 5 | DTS/overlay zero-diff | `git diff -- "*.overlay" "*.dts" "*.dtsi"` — empty |
| 6 | `framework/` zero-diff | `git diff -- framework/` — empty |
| 7 | `app/probe_translator/` zero-diff | `git diff -- app/probe_translator/` — empty |
| 8 | `devkit_uart_producer.*` zero-diff | `git diff -- devkit/src/devkit_uart_producer.*` — empty |
| 9 | No `robotos_core_register_event_handler` call in new files | `grep -r "register_event_handler" devkit/src/devkit_probe_adapter.*` — no match |
| 10 | No `#include <zephyr/>` in `devkit_probe_adapter.h` | `grep "#include.*zephyr" devkit/src/devkit_probe_adapter.h` — no match (Zephyr log only in `.c`) |
| 11 | No heap in `devkit_probe_adapter.c` | `grep "malloc\|free\|calloc" devkit/src/devkit_probe_adapter.c` — no match |
| 12 | No ISR-context call | No `k_sleep`, `k_sem_give`, ISR-context dispatch in `devkit_probe_adapter.c` |
| 13 | `git diff --check` PASS | EXIT:0 |

**Note:** No new host test file is required for Phase 12L because
`probe_translator` dispatch/reset/snapshot behavior is already fully
validated at Phase 12I/12J (132/132 assertions, TC01–TC24). The
`devkit_probe_adapter` wrapper is a thin Zephyr-logger-coupled module;
its devkit-local `LOG_INF` calls cannot be usefully exercised in a
host test without mocking Zephyr logging. Zephyr build evidence + host
regression unchanged is sufficient.

---

## H. Risk / Rollback Signals

| Risk | Signal | Rollback |
| --- | --- | --- |
| `probe_translator.h` include path broken | Compile error: cannot find `probe_translator.h` | Check `devkit_probe_adapter.c` includes — `probe_translator.h` is reachable via `../app/probe_translator` already in `target_include_directories` |
| `devkit_probe_adapter_init` returns non-OK | `LOG_ERR` at boot; runtime continues (error-log-and-continue pattern) | Inspect snapshot at next observability tick; check init return code |
| FLASH budget exceeded | `west build` reports FLASH > 90% | Report delta to user; decide if acceptable or reduce observability verbosity |
| `devkit_app_state.c` includes `probe_translator.h` directly | Boundary violation — devkit module importing app-layer header | Stop; refactor to route through `devkit_probe_adapter.h` |
| State divergence causes unexpected probe_translator FAULT | `probe_translator_get_snapshot().fsm.current_state == FAULT` in periodic log | Report; do not roll back (FAULT is valid state); investigate input source |
| `no_transition_count` unexpectedly high | Snapshot log shows high `no_transition_count` | Investigate command dispatch order; may indicate UART commands arrive in unexpected order |
| `devkit_app_state_on_uart_byte` modified incorrectly | Command set behavior regression (UART response format change) | `git diff` check; host regression must still be 23/23 |
| `prj.conf` or DTS accidentally modified | `git diff -- devkit/prj.conf` non-empty | Revert immediately |

---

## I. Explicit Non-Claims

Phase 12L-pre (this planning gate) does NOT prove or establish:

| Claim | Status |
| --- | --- |
| Runtime behavior implemented | `NOT IMPLEMENTED` (docs-only phase) |
| Hardware behavior proven | `NOT PROVEN` |
| UART command/response behavior changed | `NOT CHANGED` |
| Shell or protocol behavior changed | `NOT CHANGED` |
| Scheduler behavior changed | `NOT CHANGED` |
| FAULT adapter event sourced from real hardware signals | `NOT STARTED` |
| Non-NULL action / on_entry / on_exit callbacks | `NOT STARTED` |
| Product command mapping / UART expansion | `NOT STARTED; USER_DECISION_REQUIRED` |
| F407 / custom board work | `HOLD/DEFER` |
| Bridge ABI memory-layout stability | `NOT LOCKED` |
| `app/probe_translator/CMakeLists.txt` or `Kconfig` created | `NOT CREATED` |

Phase 12L (the implementation phase) additionally does NOT:
- Flash to hardware.
- Produce RTT hardware evidence.
- Change the UART command surface.
- Add new UART TX responses.
- Wire the FAULT adapter type to any hardware signal.

---

## J. Next-Step Recommendation

Phase 12L-pre is `CLOSED_DOCS_ONLY`. The implementation contract is
locked in `PROBE_TRANSLATOR_RUNTIME_ADMISSION_PLAN.md`.

**Next gate:** Phase 12L — Probe Translator Runtime Admission
(explicit user authorization required before implementation starts).

Phase 12L is implementation-ready given:
- The command mapping (§E.2) is locked.
- The file boundary (§F) is defined.
- The validation plan (§G) specifies all required gates.
- No blocking open decisions remain at the implementation level.

**User decision required before Phase 12L opens:** Confirm that the
command mapping in §E.2 (`a` → CONFIG, `s` → START, `r`/`d` → RESET)
is acceptable. If a different mapping is desired, report it before
Phase 12L opens.

**Gates that remain HOLD / NOT_STARTED after Phase 12L:**
- ACTIVE disarm widening — `USER_DECISION_REQUIRED_ACTIVE_DISARM`
- Scheduler 7A/7B — `DEFER`
- F407 / custom board — `HOLD/DEFER`
- UART TX response for probe_translator snapshot — `NOT_STARTED; USER_DECISION_REQUIRED`
- FAULT source admission (hardware signal for FAULT adapter type) — `NOT_STARTED`
- Product command mapping / UART expansion — `NOT_STARTED; USER_DECISION_REQUIRED`
- Hardware-runnable Zephyr application with probe_translator — `NOT_STARTED`
