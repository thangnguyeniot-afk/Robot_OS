# RobotOS — Current State Snapshot

> **Purpose:** Canonical startup snapshot for agents (Claude, Copilot, GPT, GLM).
> Detailed phase history remains in `RobotOS_v1.0/devkit/docs/01_PROGRESS/DEVKIT_PROGRESS.md`.
> Update this file only at phase close, from validated evidence only.
> Do not record assumptions as fact. If unknown, mark **Pending / Unknown**.

---

## Last Closed Phase

### Phase 12M-pre — Probe Translator Hardware Validation Plan

- **Date:** 2026-05-14
- **Type:** Docs-only hardware validation planning gate. **No source, CMake, runtime, Kconfig, `prj.conf`, DTS, overlay, test, tool, log, or build change. No flash / RTT / hardware run. No UART behavior changed. No product command mapping opened.**
- **Close status:** `CLOSED_DOCS_ONLY`
- **Decision result:** `PHASE_12M_PRE_PROBE_TRANSLATOR_HARDWARE_VALIDATION_PLAN_CLOSED`
- **Published baseline at open:** `origin/master = 4b66487`
- **Closeout doc:** `RobotOS_v1.0/devkit/docs/02_PHASE_CLOSEOUTS/PHASE_12M_PRE_PROBE_TRANSLATOR_HARDWARE_VALIDATION_PLAN.md`
- **New long-lived spec:** `RobotOS_v1.0/devkit/docs/03_SPECS/PROBE_TRANSLATOR_HARDWARE_VALIDATION_PLAN.md` (`DRAFT / NON-FINAL`; execution-ready contract for Phase 12M hardware validation)
- **Phase log entry:** `RobotOS_v1.0/devkit/docs/01_PROGRESS/DEVKIT_PROGRESS_PHASE_11_20.md` `<a id="phase-12m-pre"></a>`

#### Hardware validation contract locked

Phase 12M is planned to:

1. Build a fresh Phase 12L firmware image (`build-phase12m`).
2. Flash to `stm32f411e_disco` via `py -m west flash -d build-phase12m`.
3. Capture RTT via `run_phase12m_probe_demo.ps1 -ComPort COM5` (new script, Phase 11D template).
4. Send UART sequence `a s r ?` and verify responses + RTT probe snapshots.

**Expected RTT probe progression (locked):**

- Boot: `ROBOTOS_PROBE init ok`; `ROBOTOS_PROBE state=1 trans=0 events=0 no_trans=0 mapped=0 unmapped=0`
- After `'a'`: `ROBOTOS_PROBE state=2 trans=1 events=1 no_trans=0 mapped=1 unmapped=0`
- After `'s'`: `ROBOTOS_PROBE state=3 trans=2 events=2 no_trans=0 mapped=2 unmapped=0`
- After `'r'`: `ROBOTOS_PROBE state=1 trans=3 events=3 no_trans=0 mapped=3 unmapped=0`

No new UART commands. No UART TX response change. No source change at Phase 12M.

**Suggested next gate:** Phase 12M — Probe Translator Hardware Validation (explicit user authorization required; hardware session; new demo script `run_phase12m_probe_demo.ps1` must be created).

---

## Phase 12L-Z Checkpoint Reference

### Phase 12L-Z — Runtime Admission / Hardware-Validation Guard

- **Date:** 2026-05-14
- **Type:** Docs-only checkpoint / hardware-validation guard. **No source, CMake, runtime, Kconfig, `prj.conf`, DTS, overlay, test, log, or build change. No new build run. No flash / RTT / hardware run. No UART behavior changed. No product command mapping opened.**
- **Close status:** `CLOSED_DOCS_ONLY`
- **Decision result:** `PHASE_12L_Z_RUNTIME_ADMISSION_CHECKPOINT_CLOSED`
- **Published baseline at open:** `origin/master = 31968ad`
- **Checkpoint doc:** `RobotOS_v1.0/devkit/docs/02_PHASE_CLOSEOUTS/PHASE_12LZ_RUNTIME_ADMISSION_CHECKPOINT.md`
- **Phase log entry:** `RobotOS_v1.0/devkit/docs/01_PROGRESS/DEVKIT_PROGRESS_PHASE_11_20.md` `<a id="phase-12lz"></a>`

#### Phase 12L-Z hardware guard

`probe_translator` is runtime-admitted at build depth via `devkit_probe_adapter` (Phase 12L). It is **not hardware-proven**. No board flash was performed. No RTT log was captured. No J-Link/OpenOCD hardware run was performed. Build-depth runtime admission is not hardware evidence. The next implementation phase must not assume hardware behavior has been proven.

**Suggested next gate:** HOLD (default) or Phase 12M-pre — Hardware Validation Planning (docs-only; must define expected RTT log patterns and hardware test criteria before any flash/RTT session is authorized).

---

## Phase 12L Implementation Baseline

### Phase 12L — Probe Translator Runtime Admission Adapter

- **Date:** 2026-05-14
- **Type:** Runtime admission implementation. New devkit-local adapter `RobotOS_v1.0/devkit/src/devkit_probe_adapter.{c,h}` owns the static `probe_translator_t` instance and is driven by `devkit_app_state` accepted command/state transitions ('a'/'s'/'r'/'d' and button cycle). **No UART command set change. No UART TX response change (`devkit_uart_producer.*` zero-diff). No `prj.conf` / DTS / overlay / Kconfig change. No framework / probe_translator / core / platform change. No new `robotos_core_register_event_handler` call. No hardware run.**
- **Close status:** `CLOSED_WITH_BUILD_EVIDENCE`
- **Decision result:** `PHASE_12L_PROBE_TRANSLATOR_RUNTIME_ADMISSION_ADAPTER_IMPLEMENTED_VALIDATED`
- **Published baseline at open:** `origin/master = 7fd04bf`
- **Closeout doc:** `RobotOS_v1.0/devkit/docs/02_PHASE_CLOSEOUTS/PHASE_12L_PROBE_TRANSLATOR_RUNTIME_ADMISSION_ADAPTER.md`
- **Updated long-lived spec:** `RobotOS_v1.0/devkit/docs/03_SPECS/PROBE_TRANSLATOR_RUNTIME_ADMISSION_PLAN.md` → `IMPLEMENTED_AT_12L (ZEPHYR-BUILD EVIDENCE)`
- **Phase log entry:** `RobotOS_v1.0/devkit/docs/01_PROGRESS/DEVKIT_PROGRESS_PHASE_11_20.md` `<a id="phase-12l"></a>`

#### Phase 12L build evidence

- `py -m west build --pristine=always -d build-phase12l -b stm32f411e_disco RobotOS_v1.0/devkit` — exit 0
- Zephyr v3.6.0 + Zephyr SDK 0.17.0 (gcc 12.2.0); board `stm32f411e_disco` rev D
- FLASH: **43,384 B (8.27%** of 512 KB; **+1,856 B** vs Phase 12K)
- RAM: **12,480 B (9.52%** of 128 KB; **+128 B** vs Phase 12K)
- No new warnings beyond pre-existing baseline
- Transcript: `RobotOS_v1.0/devkit/logs/phase_12L_build_2026-05-14.txt` (UTF-16 LE; 39,788 bytes)

#### Phase 12L host regression rerun

- `cmake -S RobotOS_v1.0/tests/host -B build-phase12l-host && cmake --build && ctest`
- **23/23 ctest PASS** (`100% tests passed, 0 tests failed out of 23`)
- `probe_translator_mapping_contract` still PASS (no new host test required)

#### Phase 12L adapter API (locked)

```c
robotos_core_status_t devkit_probe_adapter_init(void);
robotos_core_status_t devkit_probe_adapter_dispatch(uint32_t adapter_type, uint32_t adapter_arg0);
void                  devkit_probe_adapter_log_snapshot(void);
```

Single module-static `probe_translator_t s_probe_translator`. Header is `<stdint.h>` + `robotos_core.h` only — no `probe_translator.h` or framework include in header. `.c` is the only translation unit that includes `probe_translator.h`.

#### Phase 12L command mapping implemented

- `'a'` / button IDLE→ARMED → `(TYPE_CONFIG, ARG_NONE)` → probe IDLE→READY
- `'s'` / button ARMED→ACTIVE → `(TYPE_COMMAND, ARG_START)` → probe READY→ACTIVE
- `'r'` / button ACTIVE→IDLE → `(TYPE_COMMAND, ARG_RESET)` → probe any→IDLE
- `'d'` (ARMED→IDLE only) → `(TYPE_COMMAND, ARG_RESET)` → probe any→IDLE
- `'v'`, `'l'`, `'t'`, `'?'`, default, and already-in-state no-ops: **no probe dispatch**

#### Phase 12L zero-diff confirmed

- `prj.conf`, DTS, overlay, Kconfig — zero-diff
- `devkit_uart_producer.{c,h}` — **zero-diff** (UART surface frozen)
- `devkit_app_state.h` — zero-diff (public API unchanged)
- `framework/*.{h,c}` — zero-diff
- `app/probe_translator/*` — zero-diff
- `app/probe_translator/CMakeLists.txt` and `Kconfig` — **NOT CREATED**
- `core/`, `platform/`, `src/`, `include/robotos/` — zero-diff
- `tests/host/CMakeLists.txt` — zero-diff
- `tests/host/*.c` — zero-diff

#### Phase 12L open carry-forward gates

1. ACTIVE disarm widening — `USER_DECISION_REQUIRED_ACTIVE_DISARM`
2. Scheduler 7A/7B — `DEFER`
3. F407 / custom board — `HOLD/DEFER`
4. POST_FLASH_AUTOSTART root cause — `OPEN` / `MITIGATED_BY_WORKFLOW`
5. Non-NULL action / on_entry / on_exit for FAULT — open (future app-behavior phase)
6. FAULT adapter event sourcing (hardware fault signal) — `NOT_STARTED`
7. UART TX response for probe_translator snapshot — `NOT_STARTED; USER_DECISION_REQUIRED`
8. Bridge ABI memory-layout lock — `NOT_STARTED`
9. Hardware-runnable Zephyr application with `probe_translator` — `NOT_STARTED` (runtime-admitted at 12L; not hardware-proven)
10. Product command mapping / UART expansion — `NOT_STARTED; USER_DECISION_REQUIRED`
11. `RobotOS_v1.0/examples/` — `NOT_STARTED`
12. Multi-product coordination — `NOT_STARTED`

---

## Phase 12L-pre Reference

### Phase 12L-pre — Probe Translator Runtime Admission Plan

- **Date:** 2026-05-14
- **Type:** Docs-only runtime admission planning gate. **No source, CMake, runtime, Kconfig, `prj.conf`, DTS, overlay, test, or log change. No implementation. No new build run. No hardware validation claimed.**
- **Close status:** `CLOSED_DOCS_ONLY`
- **Decision result:** `PHASE_12L_PRE_PROBE_TRANSLATOR_RUNTIME_ADMISSION_PLAN_CLOSED`
- **Published baseline at open:** `origin/master = 58e532f`
- **Closeout doc:** `RobotOS_v1.0/devkit/docs/02_PHASE_CLOSEOUTS/PHASE_12L_PRE_PROBE_TRANSLATOR_RUNTIME_ADMISSION_PLAN.md`
- **New long-lived spec:** `RobotOS_v1.0/devkit/docs/03_SPECS/PROBE_TRANSLATOR_RUNTIME_ADMISSION_PLAN.md` (`DRAFT / NON-FINAL`; execution-ready contract for Phase 12L runtime admission)
- **Phase log entry:** `RobotOS_v1.0/devkit/docs/01_PROGRESS/DEVKIT_PROGRESS_PHASE_11_20.md` `<a id="phase-12l-pre"></a>`

#### Runtime admission contract locked

Phase 12L is planned to create `devkit_probe_adapter.{c,h}` as a new devkit-local module that:

- Owns a static `probe_translator_t` instance.
- Is initialized from `devkit_runtime_init()`.
- Is driven by `devkit_app_state_on_uart_byte()` and `devkit_app_state_on_button()` at 'a'/'s'/'r'/'d' and button transitions.
- Logs periodic snapshots via `devkit_probe_adapter_log_snapshot()`.

**Command mapping (locked at Phase 12L-pre):**

- `'a'` (arm) → `(PROBE_ADAPTER_TYPE_CONFIG, PROBE_ADAPTER_ARG_NONE)` → probe: IDLE→READY
- `'s'` (activate) → `(PROBE_ADAPTER_TYPE_COMMAND, PROBE_ADAPTER_ARG_START)` → probe: READY→ACTIVE
- `'r'` / `'d'` (reset/disarm) → `(PROBE_ADAPTER_TYPE_COMMAND, PROBE_ADAPTER_ARG_RESET)` → probe: any→IDLE
- button cycles same mapping: IDLE→ARMED→ACTIVE→IDLE maps to CONFIGURED/START/RESET

No new UART commands. No UART TX response change. No scheduler change. No hardware run required.

**Next gate:** Phase 12L implementation — requires explicit user authorization.

---

## Phase 12K-Z Checkpoint Reference

### Phase 12K-Z — Probe Translator Build Admission Guard

- **Date:** 2026-05-14
- **Type:** Docs-only checkpoint / build-admission guard. **No source, CMake, runtime, Kconfig, `prj.conf`, DTS, overlay, test, or log change. No new build run. No runtime wiring added. No UART behavior changed. No scheduler behavior changed. No hardware validation claimed.**
- **Close status:** `CLOSED_DOCS_ONLY`
- **Decision result:** `PHASE_12K_Z_BUILD_ADMISSION_CHECKPOINT_CLOSED`
- **Published baseline at open:** `origin/master = a4b49ad`
- **Checkpoint doc:** `RobotOS_v1.0/devkit/docs/02_PHASE_CLOSEOUTS/PHASE_12KZ_BUILD_ADMISSION_CHECKPOINT.md`
- **Phase log entry:** `RobotOS_v1.0/devkit/docs/01_PROGRESS/DEVKIT_PROGRESS_PHASE_11_20.md` `<a id="phase-12kz"></a>`

#### Guard locked

`probe_translator` is build-admitted into the devkit Zephyr image (Phase 12K). It is **not** runtime-owned by devkit. No call path from `devkit_runtime` or `devkit_app_state` to `probe_translator` exists. No UART command was added. No hardware flash or RTT evidence is claimed. The next implementation phase must not assume runtime wiring exists.

**Suggested next gate:** HOLD (default) or Phase 12L-pre — Probe Translator Devkit Runtime Admission Plan (docs-only pre-planning gate; does not wire runtime by itself).

---

## Phase 12K Baseline Reference

### Phase 12K — Probe Translator Zephyr Build-Only Admission

- **Date:** 2026-05-14
- **Type:** Build-only Zephyr admission. Additive change to `RobotOS_v1.0/devkit/CMakeLists.txt` admits `framework/robotos_fw_fsm.c`, `framework/robotos_fw_event_bridge.c`, and `app/probe_translator/probe_translator.c` into the existing devkit Zephyr build. **No runtime wiring. No `devkit_app_state` change. No UART command added or modified (`a/s/r/?/x/v/L/d/T` unchanged). No `prj.conf` / DTS / overlay / Kconfig change. No hardware run. No flash / RTT.**
- **Close status:** `CLOSED_WITH_BUILD_EVIDENCE`
- **Decision result:** `PHASE_12K_PROBE_TRANSLATOR_ZEPHYR_BUILD_ONLY_ADMITTED_VALIDATED`
- **Published baseline at open:** `origin/master = 7a8b1f4`
- **Closeout doc:** `RobotOS_v1.0/devkit/docs/02_PHASE_CLOSEOUTS/PHASE_12K_PROBE_TRANSLATOR_ZEPHYR_BUILD_ONLY_ADMISSION.md`
- **Updated long-lived spec:** `RobotOS_v1.0/devkit/docs/03_SPECS/PROBE_TRANSLATOR_ZEPHYR_BUILD_ONLY_PLAN.md` → `BUILD_ADMITTED_AT_12K (ZEPHYR-BUILD EVIDENCE)`
- **Phase log entry:** `RobotOS_v1.0/devkit/docs/01_PROGRESS/DEVKIT_PROGRESS_PHASE_11_20.md` `<a id="phase-12k"></a>`

#### Build evidence

- `west build --pristine=always -d build-phase12k -b stm32f411e_disco RobotOS_v1.0/devkit` — exit 0
- Zephyr v3.6.0 + Zephyr SDK 0.17.0 (gcc 12.2.0); board `stm32f411e_disco` rev D
- FLASH: **41,528 B (7.92%** of 512 KB)
- RAM: **12,352 B (9.42%** of 128 KB)
- No new warnings beyond pre-existing baseline
- Transcript: `RobotOS_v1.0/devkit/logs/phase_12K_build_2026-05-14.txt` (38,292 bytes)

#### Host regression rerun

- `cmake -S RobotOS_v1.0/tests/host -B build-phase12k-host && cmake --build && ctest`
- **23/23 ctest PASS** (`100% tests passed, 0 tests failed out of 23`)
- `probe_translator_mapping_contract` still PASS with 132/132 assertions (TC01–TC24)

#### Zero-diff confirmed

- `prj.conf`, DTS, overlay, Kconfig — zero-diff
- `devkit/src/*` (including `devkit_app_state`, `main.c`, `devkit_runtime.c`, UART files) — zero-diff
- `framework/*.{h,c}` — zero-diff (compiled as-is from Phase 12J baseline)
- `app/probe_translator/*` — zero-diff (compiled as-is from Phase 12J baseline)
- `app/probe_translator/CMakeLists.txt` and `Kconfig` — **NOT CREATED**
- `core/`, `platform/`, `src/`, `include/robotos/` — zero-diff
- `tests/host/CMakeLists.txt` — zero-diff

#### Open carry-forward gates (unchanged at Phase 12K close)

1. ACTIVE disarm widening — `USER_DECISION_REQUIRED_ACTIVE_DISARM`
2. Scheduler 7A/7B — `DEFER`
3. F407 / custom board — `HOLD/DEFER`
4. POST_FLASH_AUTOSTART root cause — `OPEN` / `MITIGATED_BY_WORKFLOW`
5. Non-NULL action / on_entry / on_exit for FAULT — open (future app-behavior phase)
6. Devkit runtime integration of `probe_translator` — `NOT_STARTED`
7. Bridge ABI memory-layout lock — `NOT_STARTED`
8. Hardware-runnable Zephyr application with `probe_translator` — `NOT_STARTED` (build-admitted at 12K; not wired)
9. Product command mapping / UART expansion — `NOT_STARTED; USER_DECISION_REQUIRED`
10. `RobotOS_v1.0/examples/` — `NOT_STARTED`
11. Multi-product coordination — `NOT_STARTED`

---

## Prior Closed Phase

### Phase 12K-pre — Probe Translator Zephyr Build-Only Admission Plan (docs-only)

- **Date:** 2026-05-14
- **Type:** Docs-only implementation-planning gate. **No source, runtime, CMake, Kconfig, `prj.conf`, DTS overlay, test, Framework, devkit, command-set, or `devkit_app_state` change.** No Zephyr build run.
- **Close status:** `CLOSED_DOCS_ONLY`
- **Decision result:** `PHASE_12K_PRE_PROBE_TRANSLATOR_ZEPHYR_BUILD_ONLY_PLAN_CLOSED`
- **Published baseline at open:** `origin/master = e935dd1`
- **Closeout doc:** `RobotOS_v1.0/devkit/docs/02_PHASE_CLOSEOUTS/PHASE_12K_PRE_PROBE_TRANSLATOR_ZEPHYR_BUILD_ONLY_PLAN.md`
- **New long-lived spec:** `RobotOS_v1.0/devkit/docs/03_SPECS/PROBE_TRANSLATOR_ZEPHYR_BUILD_ONLY_PLAN.md` (`DRAFT / NON-FINAL`; execution-ready contract for Phase 12K build admission)
- **Phase log entry:** `RobotOS_v1.0/devkit/docs/01_PROGRESS/DEVKIT_PROGRESS_PHASE_11_20.md` `<a id="phase-12k-pre"></a>`

#### Implementation contract resolved

`PHASE_12K_PRE_PROBE_TRANSLATOR_ZEPHYR_BUILD_ONLY_PLAN_CLOSED`

Zephyr build topology audited. No blocking conditions found. Phase 12K has a complete, unambiguous additive CMake contract.

#### Resolved decisions (locked for Phase 12K)

| Decision | Resolution |
| --- | --- |
| CMake strategy | **Option A** — additive entry in `devkit/CMakeLists.txt` |
| New files required | `devkit/CMakeLists.txt` (additive); evidence log; closeout |
| `app/probe_translator/CMakeLists.txt` | **FORBIDDEN at Phase 12K** (not needed; deferred) |
| `app/probe_translator/Kconfig` | **FORBIDDEN** (no Kconfig dependency) |
| `prj.conf` change | **NONE** — zero-diff at Phase 12K |
| DTS / overlay change | **NONE** — zero-diff at Phase 12K |
| Platform critical section | **Already satisfied** by existing devkit build |
| Framework in devkit Zephyr build | **ABSENT** before Phase 12K |
| Devkit runtime wiring | **NONE** — build-only |
| UART command change | **NONE** — `a/s/r/?/x/v/L/d/T` frozen |
| Hardware run | **NONE** — no flash, no RTT |
| West build command | `west build --pristine=always -d build-phase12k -b stm32f411e_disco RobotOS_v1.0/devkit` |

#### Phase 12K approved future file set

**Modified existing (additive only):**

- `RobotOS_v1.0/devkit/CMakeLists.txt` — add framework + probe_translator sources and include paths

**New:**

- `RobotOS_v1.0/devkit/logs/phase_12K_build_<date>.txt` — Zephyr build evidence log
- `RobotOS_v1.0/devkit/docs/02_PHASE_CLOSEOUTS/PHASE_12K_PROBE_TRANSLATOR_ZEPHYR_BUILD_ONLY.md` — closeout

**Zero-diff held at Phase 12K:** `devkit/prj.conf`, all DTS/overlay, `devkit/src/`, `framework/*.h`, `framework/*.c`, `core/`, `platform/`, `app/probe_translator/probe_translator.{h,c}`, `tests/host/CMakeLists.txt`.

#### Open carry-forward gates (unchanged at Phase 12K-pre)

1. ACTIVE disarm widening — `USER_DECISION_REQUIRED_ACTIVE_DISARM`
2. Scheduler 7A/7B — `DEFER`
3. F407 / custom board — `HOLD/DEFER`
4. POST_FLASH_AUTOSTART root cause — `OPEN` / `MITIGATED_BY_WORKFLOW`
5. Non-NULL action / on_entry / on_exit for FAULT — open (future app-behavior phase)
6. Devkit runtime integration of `probe_translator` — `NOT_STARTED`
7. Bridge ABI memory-layout lock — `NOT_STARTED`
8. Zephyr build-only admission for `app/probe_translator/` — `NOT_STARTED → opens at Phase 12K`
9. Hardware-runnable Zephyr build of `app/probe_translator/` — `NOT_STARTED`
10. Product command mapping — `NOT_STARTED; USER_DECISION_REQUIRED`
11. `RobotOS_v1.0/examples/` — `NOT_STARTED`
12. Multi-product coordination — `NOT_STARTED`

---

## Phase 12J-Z Closed Reference

### Phase 12J-Z — Probe Translator App-Layer Checkpoint / Integration Direction Guard

- **Date:** 2026-05-14
- **Type:** Docs-only checkpoint / integration direction guard. **No source, runtime, test, CMake, Zephyr, board, `prj.conf`, DTS overlay, evidence log, Framework header, Framework `.c` file, devkit integration, command-set, or `devkit_app_state` change.**
- **Close status:** `CLOSED_DOCS_ONLY`
- **Decision result:** `PHASE_12J_Z_APP_LAYER_CHECKPOINT_CLOSED`
- **Published baseline at open:** `origin/master = 2d9f832`
- **Checkpoint doc:** `RobotOS_v1.0/devkit/docs/02_PHASE_CLOSEOUTS/PHASE_12JZ_CHECKPOINT.md`
- **Phase log entry:** `RobotOS_v1.0/devkit/docs/01_PROGRESS/DEVKIT_PROGRESS_PHASE_11_20.md` `<a id="phase-12jz"></a>`

#### App-layer behavior baseline locked at `2d9f832`

`APP_LAYER_BEHAVIOR_COMPLETE_AT_HOST_DEPTH` — `probe_translator` has 4 state
defs, 10 transition rows (FAULT block + IDLE self-loop), 5 mapping rows
(wildcard FAULT row), TC01–TC24 (132/132 assertions PASS), 23/23 ctest PASS.
No devkit/Zephyr/UART/hardware integration.

#### Recommended next gate

**HOLD** (default) or:
**`Phase 12K-pre — Probe Translator Zephyr Build-Only Admission Plan`**
(docs-only planning; does not wire devkit runtime, UART, or hardware).

#### Open carry-forward gates (unchanged at Phase 12J-Z)

1. ACTIVE disarm widening — `USER_DECISION_REQUIRED_ACTIVE_DISARM`
2. Scheduler 7A/7B — `DEFER`
3. F407 / custom board — `HOLD/DEFER`
4. POST_FLASH_AUTOSTART root cause — `OPEN` / `MITIGATED_BY_WORKFLOW`
5. Non-NULL action / on_entry / on_exit for FAULT — open (future app-behavior phase)
6. Devkit integration of `probe_translator` — `NOT_STARTED`
7. Bridge ABI memory-layout lock — `NOT_STARTED`
8. Zephyr build-only admission for `app/probe_translator/` — `NOT_STARTED`
9. Hardware-runnable Zephyr build of `app/probe_translator/` — `NOT_STARTED`
10. `RobotOS_v1.0/examples/` — `NOT_STARTED`
11. Multi-product coordination rules — `NOT_STARTED`

---

## Phase 12J Closed Reference

### Phase 12J — Probe Translator FAULT Block Implementation

- **Date:** 2026-05-14
- **Type:** Host-only FAULT block extension to `probe_translator`. Additive
  changes to `app/probe_translator/probe_translator.{h,c}`,
  `app/probe_translator/README.md`, and
  `tests/host/test_app_probe_translator_mapping.c` only.
  **No devkit runtime change. No UART command added or modified
  (`a/s/r/?/x/v/L/d/T` unchanged). No `devkit_app_state` change. No
  Framework code change. No CMakeLists.txt change. No Zephyr / `prj.conf` /
  DTS / overlay / hardware change. No Architecture B reuse.**
- **Close status:** `CLOSED_WITH_HOST_TEST_EVIDENCE`
- **Decision result:** `PHASE_12J_FAULT_BLOCK_IMPLEMENTED_VALIDATED`
- **Published baseline at open:** `origin/master = 06936c0`
- **Closeout doc:** `RobotOS_v1.0/devkit/docs/02_PHASE_CLOSEOUTS/PHASE_12J_PROBE_TRANSLATOR_FAULT_BLOCK.md`
- **Updated long-lived spec:** `RobotOS_v1.0/devkit/docs/03_SPECS/PROBE_TRANSLATOR_FAULT_BLOCK_PLAN.md` upgraded to `IMPLEMENTED_AT_12J (HOST-TEST EVIDENCE)`.
- **Phase log entry:** `RobotOS_v1.0/devkit/docs/01_PROGRESS/DEVKIT_PROGRESS_PHASE_11_20.md` `<a id="phase-12j"></a>`

#### Host test evidence

- ctest summary: **23/23 PASS** (`100% tests passed, 0 tests failed out of 23`); WSL Ubuntu / gcc 13.3.0.
- `probe_translator_mapping_contract` test: **PASS**, **132/132** in-binary assertions (TC01–TC24; 70 Phase 12I retained + 62 Phase 12J new).
- Prior 22 targets unchanged in source and unchanged in pass state.
- Host log: `RobotOS_v1.0/tests/host/logs/phase_12J_host_2026-05-14.log`.

#### FAULT block implemented

| Symbol | Value | Change |
| --- | --- | --- |
| `PROBE_TRANSLATOR_STATE_FAULT` | `4u` | declared (was RESERVED comment) |
| `PROBE_TRANSLATOR_EVT_FAULT` | `5u` | declared (was RESERVED comment) |
| `PROBE_ADAPTER_TYPE_FAULT` | `3u` | declared (was RESERVED comment) |
| `PROBE_ADAPTER_ARG_ANY` | `0xFFFFFFFFu` | declared (was OMITTED comment) |
| `transition_count` | `5u → 10u` | +5 rows (IDLE+RESET self-loop; 3×X+FAULT→FAULT; FAULT+RESET→IDLE) |
| `state_count` | `3u → 4u` | +1 STATE_FAULT def |
| `row_count` | `4u → 5u` | +1 wildcard FAULT mapping row |

#### Open carry-forward gates (unchanged at Phase 12J close)

1. ACTIVE disarm widening — `USER_DECISION_REQUIRED_ACTIVE_DISARM`
2. Scheduler 7A/7B — `DEFER`
3. F407 / custom board — `HOLD/DEFER`
4. POST_FLASH_AUTOSTART root cause — `OPEN` / `MITIGATED_BY_WORKFLOW`
5. Non-NULL action / on_entry / on_exit for FAULT — open for future app-behavior phase
6. Devkit integration of `probe_translator` — `NOT_STARTED`
7. Bridge ABI memory-layout lock — `NOT_STARTED`
8. Hardware-runnable Zephyr build of `app/probe_translator/` — open for future runtime-integration phase
9. `RobotOS_v1.0/examples/` — open for future docs-only phase
10. Multi-product coordination rules — open; reachable only after a second app exists

---

## Phase 12J-pre Reference

### Phase 12J-pre — Probe Translator FAULT Block Plan (docs-only)

- **Date:** 2026-05-13
- **Type:** Docs-only implementation-planning gate. **No source, runtime, test, CMake, Zephyr, board, `prj.conf`, DTS overlay, evidence log, Framework header, Framework `.c` file, devkit integration, command-set, or `devkit_app_state` change.** No file under `RobotOS_v1.0/app/probe_translator/`, `framework/`, `tests/`, `core/`, `platform/`, `devkit/src/`, `src/`, `include/robotos/` modified. **`app/probe_translator/` remains exactly as committed at Phase 12I (`b7fe5e2`).**
- **Close status:** `CLOSED_DOCS_ONLY`
- **Decision result:** `PHASE_12J_PRE_PROBE_TRANSLATOR_FAULT_BLOCK_PLAN_CLOSED`
- **Published baseline at open:** `origin/master = b7fe5e2`
- **Closeout doc:** `RobotOS_v1.0/devkit/docs/02_PHASE_CLOSEOUTS/PHASE_12J_PRE_PROBE_TRANSLATOR_FAULT_BLOCK_PLAN.md`
- **New long-lived spec:** `RobotOS_v1.0/devkit/docs/03_SPECS/PROBE_TRANSLATOR_FAULT_BLOCK_PLAN.md` (`DRAFT / NON-FINAL`; execution-ready implementation contract for Phase 12J; all numeric values, transition rows, mapping row, state-def addition, count bumps, and 9 new test cases TC16-TC24 locked).
- **Phase log entry:** `RobotOS_v1.0/devkit/docs/01_PROGRESS/DEVKIT_PROGRESS_PHASE_11_20.md` `<a id="phase-12j-pre"></a>`

#### Phase 12J-pre contract resolved

**`PHASE_12J_PRE_PROBE_TRANSLATOR_FAULT_BLOCK_PLAN_CLOSED`** — All Phase 12I-deferred FAULT decisions are resolved. Phase 12J has a complete, unambiguous additive contract.

#### Resolved decisions (locked for Phase 12J)

| Decision | Resolution |
|---|---|
| `PROBE_TRANSLATOR_STATE_FAULT` numeric value | `4u` — formalized from Phase 12I-pre §E.1 reservation |
| `PROBE_TRANSLATOR_EVT_FAULT` numeric value | `5u` — formalized |
| `PROBE_ADAPTER_TYPE_FAULT` numeric value | `3u` — formalized |
| `PROBE_ADAPTER_ARG_ANY` declaration | `0xFFFFFFFFu` documentation alias only; wildcard semantics from `match_arg0 == false` |
| Transition row 5 (`IDLE + RESET → IDLE`) | **SHIPPED** at Phase 12J (was deferred at Phase 12I) |
| Transition rows 6-9 (FAULT block) | **SHIPPED** at Phase 12J |
| Mapping row 4 (FAULT wildcard) | **SHIPPED** at Phase 12J |
| Sticky FAULT for CONFIG/START/STOP from FAULT | **implicit** (no FSM row → `no_transition_count++`; state stays) |
| Bridge code change | **NONE** — uses existing Phase 12F `match_arg0 == false` feature |
| CMake change | **NONE** — Phase 12I CMake block already compiles `probe_translator.c` |
| Final transition table size | 10 rows (5 + 5) |
| Final mapping table size | 5 rows (4 + 1) |
| Final state-def size | 4 entries (3 + 1) |
| New test cases | TC16-TC24 (9 new); TC01-TC15 retained verbatim |
| Expected ctest target count after Phase 12J | 23/23 (unchanged; no new target) |

#### Phase 12J approved future file set

**Modified existing files (additive only):**
- `RobotOS_v1.0/app/probe_translator/probe_translator.h`
- `RobotOS_v1.0/app/probe_translator/probe_translator.c`
- `RobotOS_v1.0/app/probe_translator/README.md`
- `RobotOS_v1.0/tests/host/test_app_probe_translator_mapping.c`

**New (NOT yet created):**
- `RobotOS_v1.0/tests/host/logs/phase_12J_host_<date>.log`
- `RobotOS_v1.0/devkit/docs/02_PHASE_CLOSEOUTS/PHASE_12J_PROBE_TRANSLATOR_FAULT_BLOCK.md`

**Zero-diff held at Phase 12J:** `tests/host/CMakeLists.txt`, all `framework/`, `core/`, `platform/`, `devkit/src/`, `src/`, `include/robotos/`, `prj.conf`, DTS, overlay, Zephyr config, prior evidence logs.

#### Phase 12J validation gates (14 required)

Mirror Phase 12I §K with delta: gate 3 covers extended `probe_translator_mapping_contract` test (TC01-TC24); gate 7 new log `phase_12J_host_<date>.log`; gates 1-2-4-5-6-8-9-10-11-12-13-14 unchanged. Plus a structural check on `transition_count = 10u / state_count = 4u / row_count = 5u` after Phase 12J `init`.

#### What is preserved unchanged at Phase 12J-pre

- All `.c` and `.h` files in the repo — zero-diff.
- `app/probe_translator/probe_translator.{h,c}` — zero-diff.
- All `CMakeLists.txt` — zero-diff.
- All tracked test files under `tests/` — zero-diff.
- `framework/`, `core/`, `platform/`, `devkit/src/` — zero-diff.
- `src/`, `include/robotos/` — zero-diff.
- All evidence logs — zero-diff (incl. `phase_12I_host_2026-05-13.log`).
- Phase 12I host-test evidence (23/23; 70/70 assertions) remains the latest tracked evidence.

#### Remaining decisions (preserved unchanged at Phase 12J-pre)

1. ACTIVE disarm widening — `USER_DECISION_REQUIRED_ACTIVE_DISARM`
2. Scheduler 7A/7B — `DEFER`
3. F407 / custom board — `HOLD/DEFER`
4. POST_FLASH_AUTOSTART root cause — `OPEN` / `MITIGATED_BY_WORKFLOW`
5. FAULT block implementation — `NOT_STARTED` (contract locked at Phase 12J-pre; opens only on explicit user authorization)
6. Non-NULL action callbacks / on_entry / on_exit for FAULT — open for future app-behavior phase
7. Devkit integration of `probe_translator` — `NOT_STARTED`
8. Bridge ABI memory-layout lock — `NOT_STARTED` (Phase 12J does NOT trigger this)
9. Hardware-runnable Zephyr build of `app/probe_translator/` — open for future runtime-integration phase
10. `RobotOS_v1.0/examples/` future tree — open for future docs-only phase
11. Multi-product coordination rules — open; reachable only after a second app exists

#### Next gate

**Hold.** Phase 12J — Probe Translator FAULT Block Extension may open only on **explicit user authorization** AND with the implementation contract from `PROBE_TRANSLATOR_FAULT_BLOCK_PLAN.md`. Phase 12J remains `NOT_STARTED`. If the user prefers to hold, no next phase is required — the FAULT block contract sits at planning depth indefinitely.

---

## Phase 12I Closed Reference

### Phase 12I — Probe Translator Host Prototype

- **Date:** 2026-05-13
- **Type:** Host-only first-application implementation. New
  `RobotOS_v1.0/app/probe_translator/` directory with
  `probe_translator.{h,c}` + `README.md`; new
  `tests/host/test_app_probe_translator_mapping.c`; additive block
  in `tests/host/CMakeLists.txt`; new host evidence log
  `tests/host/logs/phase_12I_host_2026-05-13.log`.
  **No devkit runtime change. No UART command added or modified
  (`a/s/r/?/x/v/L/d/T` unchanged). No `devkit_app_state` change. No
  Framework code change. No Zephyr / `prj.conf` / DTS / overlay /
  hardware change. No Architecture B reuse. No `app/probe_translator/CMakeLists.txt`.**
- **Close status:** `CLOSED_WITH_HOST_TEST_EVIDENCE`
- **Decision result:** `PHASE_12I_PROBE_TRANSLATOR_HOST_PROTOTYPE_IMPLEMENTED_VALIDATED`
- **Published baseline at open:** `origin/master = afc6777`
- **Closeout doc:** `RobotOS_v1.0/devkit/docs/02_PHASE_CLOSEOUTS/PHASE_12I_PROBE_TRANSLATOR_HOST_PROTOTYPE.md`
- **Updated long-lived spec:** `RobotOS_v1.0/devkit/docs/03_SPECS/PROBE_TRANSLATOR_HOST_PROTOTYPE_PLAN.md` upgraded to `IMPLEMENTED_AT_12I (HOST-TEST EVIDENCE)`; embedded-config deviation recorded in §6.4.
- **Updated skeleton spec:** `RobotOS_v1.0/devkit/docs/03_SPECS/PROBE_TRANSLATOR_APP_SKELETON_DRAFT.md` upgraded to `IMPLEMENTED_AT_12I (HOST-TEST EVIDENCE)`.
- **Phase log entry:** `RobotOS_v1.0/devkit/docs/01_PROGRESS/DEVKIT_PROGRESS_PHASE_11_20.md` `<a id="phase-12i"></a>`

#### Phase 12I host test evidence

- ctest summary: **23/23 PASS** (`100% tests passed, 0 tests failed out of 23`); WSL Ubuntu / gcc 13.3.0 / cmake 3.28.3.
- New ctest target `probe_translator_mapping_contract`: **PASS**, **70/70** in-binary assertions (TC01–TC15; TC08/TC12/TC13/TC14/TC15 `REVIEW_VALIDATED`).
- Prior 22 targets unchanged in source and unchanged in pass state.
- Host log: `RobotOS_v1.0/tests/host/logs/phase_12I_host_2026-05-13.log`.

#### Implementation deviation from Phase 12I-pre spec

The Phase 12I-pre implementation-plan spec §6.4 showed the FSM and
bridge config objects as **stack-local** inside `probe_translator_init`.
The Framework FSM and bridge both store the config by pointer
(`fsm->config = config`; `bridge->config = config`) and require the
config to outlive the instance. Stack-locals do not — the first
dispatch after `init` dereferences dangling memory. **Resolution:**
`_fsm_cfg` and `_bridge_cfg` are embedded into `probe_translator_t`
as opaque fields so their lifetime matches the harness. Public API is
unchanged. The long-lived spec is updated to record the corrected
pattern.

#### Validation gates (all 14 from Phase 12I-pre §K)

CMake configure / build PASS; 23/23 tests PASS; FSM, bridge, and
probe-translator targets each PASS; host log saved; four grep gates
clean (no `devkit_app_state.h` / no `devkit_*` calls / no UART command
bytes / no Zephyr-devkit-legacy includes); command set unchanged;
`devkit_app_state` zero-diff; no hardware run.

#### Open carry-forward gates (unchanged at Phase 12I close)

1. ACTIVE disarm widening — `USER_DECISION_REQUIRED_ACTIVE_DISARM`
2. Scheduler 7A/7B — `DEFER`
3. F407 / custom board — `HOLD/DEFER`
4. POST_FLASH_AUTOSTART root cause — `OPEN` / `MITIGATED_BY_WORKFLOW`
5. `PROBE_TRANSLATOR_STATE_FAULT` + FAULT block — `DEFERRED` to a future app-behavior phase
6. Devkit integration of Framework FSM + bridge — `NOT_STARTED`
7. Bridge ABI memory-layout lock — `NOT_STARTED`
8. Hardware-runnable Zephyr build of `app/probe_translator/` — open for future runtime-integration phase
9. `RobotOS_v1.0/examples/` — open for future docs-only phase
10. Multi-product coordination rules — open; reachable only after a second app exists

#### Next gate

**Hold.** The first application harness is host-test-validated.
Future phases (FAULT extension, devkit integration, hardware runtime,
second product) may open only on **explicit user authorization**.
Phase 12I-pre prior closed phase is retained below for traceability.

---

## Older Closed Phase

### Phase 12I-pre — Probe Translator Host Prototype Implementation Plan (docs-only)

- **Date:** 2026-05-13
- **Type:** Docs-only implementation-planning gate. **No source, runtime, test, CMake, Zephyr, board, `prj.conf`, DTS overlay, evidence log, Framework header, Framework `.c` file, devkit integration, command-set, or `devkit_app_state` change.** No file under `framework/`, `tests/`, `core/`, `platform/`, `devkit/src/`, `src/`, `include/robotos/` modified. **No `app/` or `application/` directory created. No `app/probe_translator/` directory created.**
- **Close status:** `CLOSED_DOCS_ONLY`
- **Decision result:** `PHASE_12I_PRE_PROBE_TRANSLATOR_HOST_PROTOTYPE_PLAN_CLOSED`
- **Published baseline at open:** `origin/master = 03edb75`
- **Closeout doc:** `RobotOS_v1.0/devkit/docs/02_PHASE_CLOSEOUTS/PHASE_12I_PRE_PROBE_TRANSLATOR_HOST_PROTOTYPE_PLAN.md`
- **New long-lived spec draft:** `RobotOS_v1.0/devkit/docs/03_SPECS/PROBE_TRANSLATOR_HOST_PROTOTYPE_PLAN.md` (execution-ready implementation contract for Phase 12I; upgraded to `IMPLEMENTED_AT_12I` at Phase 12I close).
- **Phase log entry:** `RobotOS_v1.0/devkit/docs/01_PROGRESS/DEVKIT_PROGRESS_PHASE_11_20.md` `<a id="phase-12i-pre"></a>`

#### Implementation contract resolved

**`PHASE_12I_PRE_PROBE_TRANSLATOR_HOST_PROTOTYPE_PLAN_CLOSED`** — All open questions from Phase 12H are resolved. Phase 12I has a complete, unambiguous implementation contract. Phase 12I may open only on **explicit user authorization**.

#### Resolved decisions (all locked for Phase 12I)

| Decision | Resolution |
|---|---|
| `PROBE_TRANSLATOR_STATE_IDLE/READY/ACTIVE` numeric values | `1u / 2u / 3u` |
| `PROBE_TRANSLATOR_EVT_CONFIGURED/START/STOP/RESET` numeric values | `1u / 2u / 3u / 4u` |
| `PROBE_ADAPTER_TYPE_CONFIG/COMMAND` numeric values | `1u / 2u` |
| `PROBE_ADAPTER_ARG_NONE/START/STOP/RESET` numeric values | `0u / 1u / 2u / 3u` |
| `PROBE_TRANSLATOR_STATE_FAULT` + FAULT block | **DEFERRED** — not declared at Phase 12I |
| Transition row 5 (`IDLE + RESET → IDLE`) | **DEFERRED** from Phase 12I |
| `PROBE_ADAPTER_ARG_ANY` | **OMITTED** — no wildcard row at Phase 12I |
| `probe_translator_t` ownership | **Embed FSM + bridge by value** |
| `probe_translator_snapshot_t` | **Combined struct** (`fsm` + `bridge` fields) |
| Non-NULL action rows | **None** — all rows `action=NULL`; TC08 `REVIEW_VALIDATED` |
| Build strategy | **Option A** — additive block in `tests/host/CMakeLists.txt` |
| Host test case count | **15 cases** (TC01–TC15) |
| Expected regression count | **23/23** PASS after Phase 12I |

#### Phase 12I approved future file set

**New app files (NOT created at Phase 12I-pre):**
- `RobotOS_v1.0/app/probe_translator/probe_translator.h`
- `RobotOS_v1.0/app/probe_translator/probe_translator.c`
- `RobotOS_v1.0/app/probe_translator/README.md`

**New host test (NOT created at Phase 12I-pre):**
- `RobotOS_v1.0/tests/host/test_app_probe_translator_mapping.c`

**Modified at Phase 12I (NOT yet applied):**
- `RobotOS_v1.0/tests/host/CMakeLists.txt` — additive `APP_DIR` + `add_executable` + `add_test` block only.

**New evidence log (NOT created at Phase 12I-pre):**
- `RobotOS_v1.0/tests/host/logs/phase_12I_host_<date>.log`

**Exact CMake block:** locked in `PROBE_TRANSLATOR_HOST_PROTOTYPE_PLAN.md §8.1`.

**Exact header contract:** locked in `PROBE_TRANSLATOR_HOST_PROTOTYPE_PLAN.md §5`.

**Exact source contract:** locked in `PROBE_TRANSLATOR_HOST_PROTOTYPE_PLAN.md §6`.

**Exact 15-case host test contract:** locked in `PROBE_TRANSLATOR_HOST_PROTOTYPE_PLAN.md §7`.

#### Phase 12I validation gates (14 required)

CMake configure/build PASS; all 23 tests PASS (22 prior + 1 new); FSM / bridge / probe-translator targets each PASS; host log committed; four grep gates clean (no `devkit_app_state.h` / no `devkit_*` calls / no UART command bytes / no Zephyr-devkit-legacy includes); command set unchanged; `devkit_app_state` zero-diff; no hardware run.

#### What is preserved unchanged at Phase 12I-pre

- All `.c` and `.h` files in the repo — zero-diff.
- `framework/robotos_fw_fsm.{h,c}`, `framework/robotos_fw_event_bridge.{h,c}` — zero-diff.
- All `CMakeLists.txt` — zero-diff.
- All tracked test files under `tests/` — zero-diff.
- `core/`, `platform/`, `devkit/src/`, board DTS/overlays, Zephyr workspace — zero-diff.
- `src/`, `include/robotos/` — zero-diff.
- All evidence logs — zero-diff.
- **No `app/` directory created.**
- **No `app/probe_translator/` directory created.**
- Framework FSM host test (Phase 12E, 93/93), bridge host test (Phase 12F, 103/103), and full host regression (Phase 12F, 22/22) remain the latest tracked evidence.

#### Remaining decisions (all preserved unchanged at Phase 12I-pre)

1. ACTIVE disarm widening — `USER_DECISION_REQUIRED_ACTIVE_DISARM`
2. Scheduler 7A/7B — `DEFER`
3. F407 / custom board — `HOLD/DEFER`
4. POST_FLASH_AUTOSTART root cause — `OPEN` / `MITIGATED_BY_WORKFLOW`
5. Application / product layer — `NOT_STARTED`; `app/` directory `NOT_CREATED`; `app/probe_translator/` `NOT_CREATED`; implementation contract locked at Phase 12I-pre
6. `PROBE_TRANSLATOR_STATE_FAULT` + FAULT block — `DEFERRED` to a future app-behavior phase
7. Transition row 5 (`IDLE + RESET → IDLE`) — `DEFERRED`
8. Devkit integration of Framework FSM + bridge — `NOT_STARTED`
9. Bridge ABI memory-layout lock — `NOT_STARTED`
10. Whether the first application is also a hardware-runnable Zephyr application — open for future runtime-integration phase
11. `RobotOS_v1.0/examples/` future tree — open for future docs-only phase
12. Multi-product coordination rules — open; reachable only after a second app exists

#### Next gate

**Hold.** Phase 12I — Probe Translator Host Prototype may open only on **explicit user authorization** AND with the implementation contract from `PROBE_TRANSLATOR_HOST_PROTOTYPE_PLAN.md`. Phase 12I remains `NOT_STARTED`. If the user prefers to hold instead, no next phase is required — the implementation contract sits at planning depth indefinitely.

---

## Prior Closed Phase

### Phase 12H — Probe Translator App Skeleton Planning (docs-only, Variant 1)

- **Date:** 2026-05-13
- **Type:** Docs-only application-skeleton planning gate (Variant 1). **No source, runtime, test, CMake, Zephyr, board, `prj.conf`, DTS overlay, evidence log, Framework header, Framework `.c` file, devkit integration, command-set, or `devkit_app_state` change.** No file under `framework/`, `tests/`, `core/`, `platform/`, `devkit/src/`, `src/`, `include/robotos/` modified. **No `app/` or `application/` directory created. No `app/probe_translator/` directory created.**
- **Close status:** `CLOSED_DOCS_ONLY`
- **Decision result:** `PHASE_12H_PROBE_TRANSLATOR_SKELETON_PLANNED_DOCS_ONLY`
- **Published baseline at open:** `origin/master = 45ac339`
- **Closeout doc:** `RobotOS_v1.0/devkit/docs/02_PHASE_CLOSEOUTS/PHASE_12H_PROBE_TRANSLATOR_SKELETON_PLANNING.md`
- **New long-lived spec draft:** `RobotOS_v1.0/devkit/docs/03_SPECS/PROBE_TRANSLATOR_APP_SKELETON_DRAFT.md` (`DRAFT / NON-FINAL`; locks future file set, public API names, state / event / adapter-key vocabulary, transition + mapping tables, host test plan, build strategy).
- **Phase log entry:** `RobotOS_v1.0/devkit/docs/01_PROGRESS/DEVKIT_PROGRESS_PHASE_11_20.md` `<a id="phase-12h"></a>`

#### Skeleton planning locked

**`PHASE_12H_PROBE_TRANSLATOR_SKELETON_PLANNED_DOCS_ONLY`** — Future `RobotOS_v1.0/app/probe_translator/` skeleton is locked at planning depth. The directory remains **NOT_CREATED**. The next recommended gate is Phase 12I-pre (docs-only implementation plan) which resolves numeric values for the locked constant names and ship-or-defer decisions for the optional FAULT block and transition row 5; alternatively Phase 12I (host prototype implementation) may open if the user explicitly authorizes implementation directly.

#### Future file boundary locked

**Required at first implementation:**

- `RobotOS_v1.0/app/probe_translator/probe_translator.h`
- `RobotOS_v1.0/app/probe_translator/probe_translator.c`
- `RobotOS_v1.0/app/probe_translator/README.md`
- `RobotOS_v1.0/tests/host/test_app_probe_translator_mapping.c`

**Optional:** `RobotOS_v1.0/app/probe_translator/PROBE_TRANSLATOR_SPEC.md` (only if local docs grow large).

**Forbidden at first implementation:** `app/probe_translator/CMakeLists.txt`; Zephyr `prj.conf` / DTS overlay; `probe_translator_devkit.*`; `probe_translator_uart.*`; `probe_translator_main.c`; any RTT / J-Link / OpenOCD / flashing script; any Architecture B reuse.

#### Public API names locked (planning-level)

| API | Purpose |
|---|---|
| `probe_translator_init(pt, config)` | Init harness + embedded FSM + bridge in one call. |
| `probe_translator_dispatch_adapter_event(pt, type, arg0, payload)` | Forward synthetic adapter tuple through bridge → FSM. |
| `probe_translator_reset(pt)` | Reset bridge counters + FSM state. |
| `probe_translator_get_snapshot(pt, out)` | Combined FSM + bridge snapshot. |

Numeric values, struct layout, and ABI memory placement remain open (deferred to Phase 12I or later).

#### State / event vocabulary locked (names; numeric values open)

**States:** `PROBE_TRANSLATOR_STATE_IDLE / READY / ACTIVE` (required) + `PROBE_TRANSLATOR_STATE_FAULT` (optional; ship-or-defer at Phase 12I-pre).

**Events:** `PROBE_TRANSLATOR_EVT_CONFIGURED / START / STOP / RESET` (required) + `PROBE_TRANSLATOR_EVT_FAULT` (optional).

All names are application-local. Name overlap with `DEVKIT_APP_STATE_IDLE / ARMED / ACTIVE` is coincidental at the human-readable level only.

#### Transition table locked (names; values open)

Rows 0–4 required: `IDLE+CONFIGURED → READY`, `READY+START → ACTIVE`, `ACTIVE+STOP → READY`, `READY+RESET → IDLE`, `ACTIVE+RESET → IDLE`. Row 5 (`IDLE+RESET → IDLE`) and rows 6–9 (FAULT block) optional. All rows use `guard = NULL` and `action = NULL` at first implementation.

#### Bridge mapping table locked (names; values open)

Adapter types: `PROBE_ADAPTER_TYPE_CONFIG / _COMMAND / _FAULT?`. Adapter args: `PROBE_ADAPTER_ARG_NONE / _START / _STOP / _RESET / _ANY?`. Mapping rows 0–3 required (specific `match_arg0 == true`); row 4 (`TYPE_FAULT` wildcard) optional. Specific rows precede the wildcard row; wildcard uses a distinct `adapter_type` so it does not collide.

#### Host test plan locked

16 cases (TC01–TC16): init valid/invalid, four mapping happy paths (CONFIG / START / STOP / RESET), optional FAULT wildcard, unmapped event accounting, payload borrowed, snapshot counters, reset, full transition path, three grep gates (no `devkit_app_state.h` / no `a/s/r/?/x/v/L/d/T` / no Zephyr / devkit / legacy `ro_*` includes), full regression preserved (≥23/23 after new target). Future host test naming: `tests/host/test_app_probe_translator_mapping.c`.

#### Build strategy preferred

**Option A** — additive entry in existing `tests/host/CMakeLists.txt`. Option B (new leaf `app/probe_translator/CMakeLists.txt`) acceptable but not preferred. Option C (devkit integration) **not authorized**. Host environment = WSL Ubuntu / gcc 13.3.0 (Phase 12E / 12F baseline).

#### Relationship to `devkit_app_state` (scope-guard #11 preserved)

- `devkit_app_state` remains **authoritative** for the current devkit runtime. Phase 12H does **not** modify, replace, shadow, copy, or promote it.
- `probe_translator/`, when created, **must not** include `devkit_app_state.h`, must not call any `devkit_*` function, and must not read or write `devkit_app_state` snapshots.
- The application's `PROBE_TRANSLATOR_STATE_*` states are application-local; name overlap with `DEVKIT_APP_STATE_*` is human-readable only.
- No `?` response change. No `a/s/r/?/x/v/L/d/T` semantic change.
- Scope-guard #11 remains active.

#### Relationship to command set

- `a / s / r / ? / x / v / L / d / T` remain the **devkit / probe surface**.
- `probe_translator/` defines no UART command.
- Framework not exposed via UART automatically.
- All 12 UART TX scope-guard constraints from `PHASE_9EZ_CHECKPOINT.md §H` preserved.

#### Relationship to legacy Architecture B

- `src/`, `include/robotos/`, and root `RobotOS_v1.0/CMakeLists.txt` remain frozen at `LEGACY_SCAFFOLD_MARKED_FROZEN_DOCS_ONLY` (Phase 12D-pre).
- The future `probe_translator/` directory does **not** reuse `src/app/`, `include/robotos/app*`, or any other Architecture B artifact. Uses Architecture A contracts only.
- No Architecture A ↔ Architecture B reconciliation in Phase 12H.

#### What is preserved unchanged at Phase 12H

- All `.c` and `.h` files in the repo — zero-diff.
- `framework/robotos_fw_fsm.{h,c}`, `framework/robotos_fw_event_bridge.{h,c}`, `framework/README.md` — zero-diff.
- All `CMakeLists.txt` (root, `devkit/`, `tests/`, `tests/host/`) — zero-diff.
- All tracked test files under `tests/` — zero-diff.
- `core/`, `platform/`, `devkit/src/`, board DTS/overlays, Zephyr workspace — zero-diff.
- `src/`, `include/robotos/`, `include/app/` — zero-diff.
- Architecture B legacy notices — zero-diff.
- All evidence logs — zero-diff.
- All prior closeout docs — not rewritten.
- **No `app/` directory created.** `RobotOS_v1.0/app/` does not exist.
- **No `app/probe_translator/` directory created.**
- Framework FSM host test (Phase 12E, 93/93), bridge host test (Phase 12F, 103/103), and full host regression (Phase 12F, 22/22) remain the latest tracked evidence.

#### Remaining decisions (all preserved unchanged at Phase 12H)

1. ACTIVE disarm widening — `USER_DECISION_REQUIRED_ACTIVE_DISARM`
2. Scheduler 7A/7B — `DEFER`
3. F407 / custom board — `HOLD/DEFER`
4. POST_FLASH_AUTOSTART root cause — `OPEN` / `MITIGATED_BY_WORKFLOW`
5. Application / product layer — `NOT_STARTED`; first `<product>` = `probe_translator`; `app/` directory `NOT_CREATED`; `app/probe_translator/` `NOT_CREATED`; skeleton locked at Phase 12H
6. Devkit integration of Framework FSM + bridge — `NOT_STARTED`; recommended mode = SEPARATE APPLICATION (Phase 12G-pre Mode 4)
7. Architecture A ↔ Architecture B reconciliation — `NOT_STARTED`; out of scope
8. Numeric values for `PROBE_TRANSLATOR_STATE_*` and `PROBE_TRANSLATOR_EVT_*` — open for Phase 12I-pre
9. Numeric values for `PROBE_ADAPTER_TYPE_*` and `PROBE_ADAPTER_ARG_*` — open for Phase 12I-pre
10. Whether `PROBE_TRANSLATOR_STATE_FAULT` + rows 6–9 + TC07 ship in first implementation — open for Phase 12I-pre
11. Whether transition row 5 (`IDLE + RESET → IDLE`) ships — open for Phase 12I-pre
12. Whether `probe_translator_t` embeds FSM + bridge by value or references by pointer — open for Phase 12I
13. Whether `PROBE_ADAPTER_ARG_ANY` is declared in the header — open for Phase 12I
14. Whether `probe_translator_get_snapshot` returns a combined struct or two out-params — open for Phase 12I
15. Whether Phase 12I-pre (docs-only) opens first or Phase 12I (implementation) opens directly — user decision before next gate opens
16. Exact CMake wiring choice (Option A preferred; Option B acceptable later; Option C not authorized) — open for the first application implementation phase
17. Whether the first application is also a hardware-runnable Zephyr application or strictly host-only at first — open for the first application implementation phase
18. Whether to introduce `RobotOS_v1.0/examples/` for sample integrations in addition to `app/<product>/` — open for a future docs-only phase
19. Multi-product coordination rules — open; reachable only after at least two applications exist
20. Bridge ABI memory-layout lock — `NOT_STARTED`; only names + signatures are `LOCKED-AT-12F`

#### Next gate

**Hold.** Phase 12I-pre — Probe Translator Host Prototype Implementation Plan (docs-only) may open only on **explicit user authorization** AND with the recommended scope from §R of the Phase 12H closeout. Phase 12I-pre is the preferred next gate; alternatively Phase 12I (host prototype implementation) may open if the user explicitly authorizes implementation directly. Phase 12I-pre and Phase 12I both remain `NOT_STARTED`. If the user prefers to hold instead, no next phase is required — the skeleton spec sits at planning depth indefinitely. Replacement / shadow / direct devkit integration modes remain blocked by Phase 12G-pre's enumeration.

---

## Prior Closed Phase

### Phase 12H-pre — First Application Candidate / Product Harness Selection (docs-only)

- **Date:** 2026-05-13
- **Type:** Docs-only product / application planning gate. **No source, runtime, test, CMake, Zephyr, board, `prj.conf`, DTS overlay, evidence log, Framework header, Framework `.c` file, devkit integration, command-set, or `devkit_app_state` change.** No file under `framework/`, `tests/`, `core/`, `platform/`, `devkit/src/`, `src/`, `include/robotos/` modified. **No `app/` or `application/` directory created. No `app/probe_translator/` directory created.**
- **Close status:** `CLOSED_DOCS_ONLY`
- **Decision result:** `PHASE_12H_PRE_RECOMMEND_PROBE_TRANSLATOR_APP`
- **Published baseline at open:** `origin/master = 629e062`
- **Closeout doc:** `RobotOS_v1.0/devkit/docs/02_PHASE_CLOSEOUTS/PHASE_12H_PRE_FIRST_APPLICATION_CANDIDATE_SELECTION.md`
- **New long-lived spec draft:** `RobotOS_v1.0/devkit/docs/03_SPECS/FIRST_APPLICATION_CANDIDATE_DRAFT.md` (`DRAFT / NON-FINAL`; no application implementation exists; spec for the first application candidate `probe_translator`).
- **Phase log entry:** `RobotOS_v1.0/devkit/docs/01_PROGRESS/DEVKIT_PROGRESS_PHASE_11_20.md` `<a id="phase-12h-pre"></a>`

#### First application candidate selected

**`PHASE_12H_PRE_RECOMMEND_PROBE_TRANSLATOR_APP`** — The first `<product>` placeholder under `RobotOS_v1.0/app/<product>/` is selected as `probe_translator`. Future first application harness lives at `RobotOS_v1.0/app/probe_translator/`. The harness is host-first, product-neutral, and synthetic-event-driven. The directory is **reserved at planning depth and NOT created**. Phase 12H, when authorized, should either:

- (Variant 1, preferred) be docs-only — freeze the exact file list, function signatures, build wiring choice (Option A preferred), and host test case list before any source lands; or
- (Variant 2) be a host-first implementation — create `app/probe_translator/probe_translator.{c,h}`, add a new host test target under `tests/host/CMakeLists.txt` (Option A), and produce host evidence under WSL Ubuntu / gcc 13.3.0.

Phase 12H **remains `NOT_STARTED`** at the close of Phase 12H-pre.

#### Candidate first-app shapes evaluated

| # | Candidate | Product commitment | Validation strategy | Risk | Verdict |
|---|---|---|---|---|---|
| 1 | `app/probe_translator/` | Minimal (neutral) | Host-first; clean | Low | **RECOMMENDED** |
| 2 | `app/demo/` | Vague | Host-first; weak acceptance | Medium | Rejected (scope sink; reserved for future `examples/`) |
| 3 | `app/devkit_shadow/` | Low | Cannot be host-first cleanly | High | Rejected (overrides Phase 12G-pre Mode 2 DEFER) |
| 4 | `app/<real_product>/` | High | Needs product acceptance criteria | High | Rejected (premature; pending product direction) |
| 5 | HOLD | None | None | Low short / Medium long | Acceptable fallback |

Full evaluation in the closeout §D.

#### Minimal state / event / mapping (planning-level)

**States:** `APP_IDLE / APP_READY / APP_ACTIVE` + optional `APP_FAULT`. All application-local. Name overlap with `DEVKIT_APP_STATE_IDLE / ARMED / ACTIVE` is coincidental at the human-readable level only.

**Events:** `APP_EVT_CONFIGURED / APP_EVT_START / APP_EVT_STOP / APP_EVT_FAULT (optional) / APP_EVT_RESET`. All synthetic at first. None map from real core events, UART bytes, buttons, or hardware producers. No new `ROBOTOS_EVENT_USER` subrange required.

**Mapping table (planning-only; no source):**

| Row | `adapter_type` | `adapter_arg0` | `match_arg0` | `fw_event_id` |
|---|---|---|---|---|
| 0 | `ADAPTER_EVT_CONFIG` | `0` | `true` | `APP_EVT_CONFIGURED` |
| 1 | `ADAPTER_EVT_CONFIG` | `1` (RESET) | `true` | `APP_EVT_RESET` |
| 2 | `ADAPTER_EVT_COMMAND` | `0` (START) | `true` | `APP_EVT_START` |
| 3 | `ADAPTER_EVT_COMMAND` | `1` (STOP) | `true` | `APP_EVT_STOP` |
| 4 | `ADAPTER_EVT_FAULT` | any | `false` (wildcard) | `APP_EVT_FAULT` |

#### Build strategy preferred

**Option A** — additive entry inside the existing `tests/host/CMakeLists.txt` (host test target). Same WSL Ubuntu / gcc 13.3.0 environment as Phase 12E / 12F. Option B (new `app/probe_translator/CMakeLists.txt`) acceptable but not preferred. Option C (devkit integration) **not authorized**.

#### Relationship to `devkit_app_state` (scope-guard #11 preserved)

- `devkit_app_state` remains **authoritative** for the current devkit runtime. Phase 12H-pre does **not** modify, replace, shadow, copy, or promote it.
- `app/probe_translator/`, when created, **must not** include `devkit_app_state.h`, must not call any `devkit_*` function, and must not read or write `devkit_app_state` snapshots.
- The application's `APP_IDLE / APP_READY / APP_ACTIVE / APP_FAULT` states are application-local; name overlap with `DEVKIT_APP_STATE_IDLE / ARMED / ACTIVE` is human-readable only.
- No `?` response change. No `a/s/r/?/x/v/L/d/T` semantic change.
- Scope-guard #11 remains active.

#### Relationship to command set

- `a / s / r / ? / x / v / L / d / T` remain the **devkit / probe surface**. Validated through Phase 11Z.
- `app/probe_translator/` **does not** define a UART command. It has no UART surface at the first host implementation phase.
- Framework is not exposed via UART automatically (bridge has no UART surface; Phase 12F locked).
- All 12 UART TX scope-guard constraints from `PHASE_9EZ_CHECKPOINT.md §H` preserved.

#### Relationship to legacy Architecture B

- `src/`, `include/robotos/`, and root `RobotOS_v1.0/CMakeLists.txt` remain frozen at `LEGACY_SCAFFOLD_MARKED_FROZEN_DOCS_ONLY` (Phase 12D-pre).
- The future `app/probe_translator/` directory does **not** reuse `src/app/`, `include/robotos/app*`, or any other Architecture B artifact. It uses Architecture A contracts only.
- No Architecture A ↔ Architecture B reconciliation in Phase 12H-pre.

#### What is preserved unchanged at Phase 12H-pre

- All `.c` and `.h` files in the repo — zero-diff.
- `framework/robotos_fw_fsm.{h,c}`, `framework/robotos_fw_event_bridge.{h,c}`, `framework/README.md` — zero-diff.
- All `CMakeLists.txt` (root, `devkit/`, `tests/`, `tests/host/`) — zero-diff.
- All tracked test files under `tests/` — zero-diff.
- `core/`, `platform/`, `devkit/src/`, board DTS/overlays, Zephyr workspace — zero-diff.
- `src/`, `include/robotos/`, `include/app/` — zero-diff.
- Architecture B legacy notices — zero-diff.
- All evidence logs — zero-diff.
- All prior closeout docs — not rewritten.
- **No `app/` directory created.** `RobotOS_v1.0/app/` does not exist.
- **No `app/probe_translator/` directory created.**
- Framework FSM host test (Phase 12E, 93/93), bridge host test (Phase 12F, 103/103), and full host regression (Phase 12F, 22/22) remain the latest tracked evidence.

#### Remaining decisions (all preserved unchanged at Phase 12H-pre)

1. ACTIVE disarm widening — `USER_DECISION_REQUIRED_ACTIVE_DISARM`
2. Scheduler 7A/7B — `DEFER`
3. F407 / custom board — `HOLD/DEFER`
4. POST_FLASH_AUTOSTART root cause — `OPEN` / `MITIGATED_BY_WORKFLOW`
5. Application / product layer — `NOT_STARTED`; first `<product>` placeholder = `probe_translator`; `app/` directory remains `NOT_CREATED`; `app/probe_translator/` remains `NOT_CREATED`
6. Devkit integration of Framework FSM + bridge — `NOT_STARTED`; recommended mode = SEPARATE APPLICATION (Phase 12G-pre Mode 4)
7. Architecture A ↔ Architecture B reconciliation — `NOT_STARTED`; out of scope
8. Concrete numeric values for `APP_EVT_*` — open for Phase 12H
9. Whether `APP_FAULT` ships in the first implementation or is deferred — open for Phase 12H
10. Whether `probe_translator.c` and `probe_translator_mapping.c` are split into two TUs or kept as one — open for Phase 12H
11. Final names for `ADAPTER_EVT_CONFIG / _COMMAND / _FAULT` and arg0 enum constants — open for Phase 12H
12. Whether Phase 12H is docs-only (Variant 1) or implementation (Variant 2) — user decision before Phase 12H opens
13. Exact CMake wiring choice (Option A: additive entry in `tests/host/CMakeLists.txt`; Option B: new `app/probe_translator/CMakeLists.txt`; Option C: devkit integration not authorized) — open for the first application implementation phase
14. Whether the first application is also a hardware-runnable Zephyr application or strictly host-only at first — open for the first application implementation phase
15. Whether to introduce `RobotOS_v1.0/examples/` for sample integrations in addition to `app/<product>/` — open for a future docs-only phase
16. Multi-product coordination rules — open; reachable only after at least two applications exist
17. Bridge ABI memory-layout lock — `NOT_STARTED`; only names + signatures are `LOCKED-AT-12F`

#### Next gate

**Hold.** Phase 12H — Probe Translator App Skeleton Planning (Variant 1, docs-only) or Probe Translator Host Prototype (Variant 2, host-first implementation) may open only on **explicit user authorization** AND with the recommended scope from §P of the Phase 12H-pre closeout. Variant 1 is preferred unless the user authorizes implementation directly. Phase 12H remains `NOT_STARTED`. If the user prefers to hold instead, no next phase is required — the first-application-candidate spec sits at planning depth indefinitely. Replacement / shadow / direct devkit integration modes remain blocked by Phase 12G-pre's enumeration.

---

## Prior Closed Phase

### Phase 12G — Separate Application Mode / Application Boundary Planning (docs-only)

- **Date:** 2026-05-13
- **Type:** Docs-only architecture planning gate. **No source, runtime, test, CMake, Zephyr, board, `prj.conf`, DTS overlay, evidence log, Framework header, Framework `.c` file, devkit integration, command-set, or `devkit_app_state` change.** No file under `framework/`, `tests/`, `core/`, `platform/`, `devkit/src/`, `src/`, `include/robotos/` modified. **No `app/` or `application/` directory created.**
- **Close status:** `CLOSED_DOCS_ONLY`
- **Decision result:** `PHASE_12G_RECOMMEND_APP_PRODUCT_PATH`
- **Published baseline at open:** `origin/master = 399e956`
- **Closeout doc:** `RobotOS_v1.0/devkit/docs/02_PHASE_CLOSEOUTS/PHASE_12G_SEPARATE_APPLICATION_BOUNDARY_PLANNING.md`
- **New long-lived spec draft:** `RobotOS_v1.0/devkit/docs/03_SPECS/FRAMEWORK_APPLICATION_BOUNDARY_DRAFT.md` (`DRAFT / NON-FINAL`; no application implementation exists; spec for the future application boundary).
- **Phase log entry:** `RobotOS_v1.0/devkit/docs/01_PROGRESS/DEVKIT_PROGRESS_PHASE_11_20.md` `<a id="phase-12g"></a>`

#### Application boundary selected

**`PHASE_12G_RECOMMEND_APP_PRODUCT_PATH`** — Future active application code lives under `RobotOS_v1.0/app/<product>/`. The directory is **reserved at planning depth and NOT created**. The first `<product>` placeholder name is selected by a future Phase 12H-pre. Phase 12G, when authorized, should:

- Reserve `RobotOS_v1.0/app/<product>/` as the future application path (sibling of `framework/`, `devkit/`, `core/`, `platform/`).
- Define application layer responsibilities (FSM instance, mapping table, product event IDs, product command vocabulary, product policy, product-level validation).
- Define application non-responsibilities (no core / platform / devkit / Framework internals; no `devkit_app_state` read or write; no legacy Architecture B).
- Define event mapping policy (application owns mapping; Framework bridge stays product-neutral and `LOCKED-AT-12F`; product event IDs are application-local).
- Define build separation strategy at planning depth (Architecture A only; separable from devkit validation build; host-first tests).
- Define a staged validation strategy (Stage 1 = docs-only boundary plan, closed at Phase 12G; Stages 2-6 require future authorization).
- **Not** create the `app/` directory. **Not** modify `devkit_app_state`. **Not** modify the devkit firmware. **Not** add or remove UART commands. **Not** modify Architecture B. **Not** run hardware. **Not** create application source itself.

#### Candidate directory shapes evaluated

| # | Option | Adjacent to | Risk | Scalable | Verdict at 12G |
|---|---|---|---|---|---|
| 1 | `RobotOS_v1.0/app/<product>/` | Framework siblings | Low | Yes | **RECOMMENDED** |
| 2 | `RobotOS_v1.0/application/<product>/` | Framework siblings | Low | Yes | Rejected (ergonomics; no repo precedent) |
| 3 | `RobotOS_v1.0/devkit/app/` | Devkit harness | High | No | Rejected (blends validation + application) |
| 4 | `RobotOS_v1.0/framework/app/` | Framework internals | High | No | Rejected (violates Framework product-neutral boundary) |
| 5 | `RobotOS_v1.0/examples/<scenario>/` | Top-level sibling | Low | Sample-only | Useful later; not the product / application path |

Full evaluation in the closeout §D.

#### App layer responsibilities (planning-level)

**May own:** product / application state machine composition (`robotos_fw_fsm_t` instance + transition table + state defs + product vocabulary); mapping table (`robotos_fw_event_bridge_row_t[]`); product event IDs; product command vocabulary (separate channel, separate framing); product-specific sensor / actuator policy; product-specific build / harness integration; product-level validation scripts and docs.

**Must not own:** core queue / dispatcher internals; platform backend primitives; devkit validation command semantics; Framework generic FSM / bridge algorithms; legacy Architecture B; any `devkit_app_state` read or write.

#### Validation strategy (staged; Phase 12G closes Stage 1 only)

| Stage | Gate | Status at Phase 12G |
|---|---|---|
| 1 | Docs-only application boundary plan | **CLOSED at Phase 12G** |
| 2 | First product selection (Phase 12H-pre, docs-only) | NOT_STARTED |
| 3 | Host-only application mapping prototype | NOT_STARTED |
| 4 | Host regression baseline preserved (22/22 still PASS) | NOT_STARTED |
| 5 | Optional devkit shadow (only if user overrides to Mode 2) | NOT_AUTHORIZED |
| 6 | Hardware evidence | NOT_AUTHORIZED |

#### Relationship to `devkit_app_state` (scope-guard #11 preserved)

- `devkit_app_state` remains **authoritative** for the current devkit runtime. It owns `IDLE/ARMED/ACTIVE`, the `?` UART response, the `a/s/r/d/t/T` byte handlers, the button cycle, and the `ROBOTOS_APP` periodic log line.
- **Separate Application Mode does not replace, shadow, copy, or promote `devkit_app_state`.** An application under `app/<product>/` may define a state machine with overlapping names — but it does so in its own `robotos_fw_fsm_t` instance, in its own namespace. The devkit firmware does not consume the application's state.
- No `?` response change. No `a/s/r/?/x/v/L/d/T` semantic change.
- Any future interaction with `devkit_app_state` (shadow runtime; replacement migration) requires a separate planning phase beyond Phase 12H.
- Scope-guard #11 remains active.

#### Relationship to command set

- `a / s / r / ? / x / v / L / d / T` remain the **devkit / probe surface**. Validated through Phase 11Z.
- **Product / application command vocabulary is not defined in Phase 12G.** If a future application grows a command interface, it does so inside `app/<product>/`, on its own channel, with its own framing.
- Framework is not exposed via UART automatically (bridge has no UART surface; Phase 12F locked).
- All 12 UART TX scope-guard constraints from `PHASE_9EZ_CHECKPOINT.md §H` preserved.

#### Relationship to legacy Architecture B

- `src/`, `include/robotos/`, and root `RobotOS_v1.0/CMakeLists.txt` remain frozen at `LEGACY_SCAFFOLD_MARKED_FROZEN_DOCS_ONLY` (Phase 12D-pre).
- The future `app/<product>/` directory does **not** reuse `src/app/`, `include/robotos/app*`, or any other Architecture B artifact. It uses Architecture A contracts only.
- No Architecture A ↔ Architecture B reconciliation in Phase 12G.

#### What is preserved unchanged at Phase 12G

- All `.c` and `.h` files in the repo — zero-diff.
- `framework/robotos_fw_fsm.{h,c}`, `framework/robotos_fw_event_bridge.{h,c}`, `framework/README.md` — zero-diff.
- All `CMakeLists.txt` (root, `devkit/`, `tests/`, `tests/host/`) — zero-diff.
- All tracked test files under `tests/` — zero-diff.
- `core/`, `platform/`, `devkit/src/`, board DTS/overlays, Zephyr workspace — zero-diff.
- `src/`, `include/robotos/`, `include/app/` — zero-diff.
- Architecture B legacy notices — zero-diff.
- All evidence logs — zero-diff.
- All prior closeout docs — not rewritten.
- **No `app/` directory created.** `RobotOS_v1.0/app/` does not exist.
- Framework FSM host test (Phase 12E, 93/93), bridge host test (Phase 12F, 103/103), and full host regression (Phase 12F, 22/22) remain the latest tracked evidence.

#### Remaining decisions (all preserved unchanged at Phase 12G)

1. ACTIVE disarm widening — `USER_DECISION_REQUIRED_ACTIVE_DISARM`
2. Scheduler 7A/7B — `DEFER`
3. F407 / custom board — `HOLD/DEFER`
4. POST_FLASH_AUTOSTART root cause — `OPEN` / `MITIGATED_BY_WORKFLOW`
5. Application / product layer — `NOT_STARTED`; boundary reserved at `RobotOS_v1.0/app/<product>/`; first `<product>` placeholder open for Phase 12H-pre
6. Devkit integration of Framework FSM + bridge — `NOT_STARTED`; recommended mode = SEPARATE APPLICATION (Phase 12G-pre Mode 4)
7. Architecture A ↔ Architecture B reconciliation — `NOT_STARTED`; out of scope
8. First `<product>` placeholder name, minimal state vocabulary, minimal initial mapping table — open for Phase 12H-pre
9. Exact CMake wiring choice (Option A: host test target in existing `tests/host/CMakeLists.txt`; Option B: new `app/<product>/CMakeLists.txt`; Option C: devkit integration not authorized) — open for the first application implementation phase
10. Whether the first application is also a hardware-runnable Zephyr application or strictly host-only at first — open for the first application implementation phase
11. Whether to introduce `RobotOS_v1.0/examples/` for sample integrations in addition to `app/<product>/` — open for a future docs-only phase
12. Multi-product coordination rules — open; reachable only after at least two applications exist
13. Bridge ABI memory-layout lock — `NOT_STARTED`; only names + signatures are `LOCKED-AT-12F`

#### Next gate

**Hold.** Phase 12H-pre — First Application Candidate / Product Harness Selection (docs-only) may open only on **explicit user authorization** AND with the recommended scope from §O of the Phase 12G closeout. Phase 12H-pre remains `NOT_STARTED`. If the user prefers to hold instead, no next phase is required — the application boundary spec sits at planning depth indefinitely. Replacement / shadow / direct devkit integration modes remain blocked by Phase 12G-pre's enumeration.

---

### Phase 12G-pre — Devkit Integration Mode Decision (docs-only)

- **Date:** 2026-05-13
- **Type:** Docs-only architecture planning gate. **No source, runtime, test, CMake, Zephyr, board, `prj.conf`, DTS overlay, evidence log, Framework header, Framework `.c` file, devkit integration, command-set, or `devkit_app_state` change.** No file under `framework/`, `tests/`, `core/`, `platform/`, `devkit/src/`, `src/`, `include/robotos/` modified.
- **Close status:** `CLOSED_DOCS_ONLY`
- **Decision result:** `PHASE_12G_PRE_RECOMMEND_SEPARATE_APPLICATION_MODE_PLANNING`
- **Published baseline at open:** `origin/master = 524cb8b`
- **Closeout doc:** `RobotOS_v1.0/devkit/docs/02_PHASE_CLOSEOUTS/PHASE_12G_PRE_DEVKIT_INTEGRATION_MODE_DECISION.md`
- **New long-lived spec draft:** `RobotOS_v1.0/devkit/docs/03_SPECS/FRAMEWORK_DEVKIT_INTEGRATION_MODE_DRAFT.md` (`DRAFT / NON-FINAL`; no integration implementation exists; spec for the future integration boundary).
- **Phase log entry:** `RobotOS_v1.0/devkit/docs/01_PROGRESS/DEVKIT_PROGRESS_PHASE_11_20.md` `<a id="phase-12g-pre"></a>`

#### Integration mode selected for next gate

**`PHASE_12G_PRE_RECOMMEND_SEPARATE_APPLICATION_MODE_PLANNING`** — Phase 12G, when authorized, should:

- Define where a future application / product harness lives (proposal anchor: `RobotOS_v1.0/app/<product>/`).
- Choose the build separation strategy (separate Zephyr app dir vs. shared base + `CONFIG_*` selectable vs. board overlay).
- Define the event-mapping policy between adapter event keys and Framework event IDs at the application layer.
- Set the validation strategy (host test first; eventual hardware run on a separate firmware target).
- **Not** modify `devkit_app_state`. **Not** modify the devkit firmware. **Not** add or remove UART commands. **Not** modify Architecture B. **Not** run hardware. **Not** create the application source itself.

#### Integration modes evaluated

| # | Mode | Touches `devkit_app_state` | Changes command set | Hardware required | App planning required | Verdict at 12G-pre |
|---|---|---|---|---|---|---|
| 1 | HOLD | No | No | No | No | Acceptable fallback |
| 2 | SHADOW | Read-only | No | Yes | No | DEFER (fallback if Mode 4 rejected) |
| 3 | REPLACEMENT | Yes (rewrite) | Yes | Yes | Yes (implicit) | NOT RECOMMENDED; structurally forbidden until ACTIVE disarm + Application planning resolved |
| 4 | SEPARATE APPLICATION | No | No | Eventually | Yes (next gate is exactly this) | **RECOMMENDED** |
| 5 | HOST-ONLY EXTENSION | No | No | No | No | Lower priority than Mode 4 |

Full evaluation in the closeout §D.

#### Relationship to `devkit_app_state` (scope-guard #11 preserved)

- `devkit_app_state` remains **authoritative** for the devkit runtime state machine. It owns `IDLE/ARMED/ACTIVE`, the `?` UART response, `a/s/r/d/t/T` byte handling, the button cycle, and the `ROBOTOS_APP` periodic log.
- Phase 12G-pre does **not** modify it and does not propose to modify it in the recommended mode.
- The recommended mode (SEPARATE APPLICATION) keeps the Framework path consumed by a **future** separate application / product harness — never by the devkit firmware. `devkit_app_state` is untouched.
- Any future mode that touches `devkit_app_state` must list: migration risk; rollback plan; hardware probes to rerun; command impact; relationship to ACTIVE disarm `USER_DECISION_REQUIRED` and POST_FLASH_AUTOSTART `OPEN`.
- Scope-guard #11 remains active.

#### Relationship to command set

- `a / s / r / ? / x / v / L / d / T` remain unchanged. Validated through Phase 11Z.
- No integration mode evaluated automatically exposes a new UART command.
- The frozen 9-command set is devkit / probe surface, not product / application vocabulary.
- A future application layer, if it grows a command vocabulary, defines that vocabulary in the application path — not in `devkit/src/`, not by reusing the devkit byte stream, and not by overriding the `?` response shape.
- All 12 UART TX scope-guard constraints from `PHASE_9EZ_CHECKPOINT.md §H` preserved.

#### What is preserved unchanged at Phase 12G-pre

- All `.c` and `.h` files in the repo — zero-diff.
- `framework/robotos_fw_fsm.{h,c}`, `framework/robotos_fw_event_bridge.{h,c}`, `framework/README.md` — zero-diff.
- All `CMakeLists.txt` (root, `devkit/`, `tests/`, `tests/host/`) — zero-diff.
- All tracked test files under `tests/` — zero-diff.
- `core/`, `platform/`, `devkit/src/`, board DTS/overlays, Zephyr workspace — zero-diff.
- `src/`, `include/robotos/`, `include/app/` — zero-diff.
- Architecture B legacy notices — zero-diff.
- All evidence logs — zero-diff.
- All prior closeout docs — not rewritten.
- Framework FSM host test (Phase 12E, 93/93) and bridge host test (Phase 12F, 103/103) and full host regression (Phase 12F, 22/22) remain the latest tracked evidence.

#### Remaining decisions (all preserved unchanged at Phase 12G-pre)

1. ACTIVE disarm widening — `USER_DECISION_REQUIRED_ACTIVE_DISARM`
2. Scheduler 7A/7B — `DEFER`
3. F407 / custom board — `HOLD/DEFER`
4. POST_FLASH_AUTOSTART root cause — `OPEN` / `MITIGATED_BY_WORKFLOW`
5. Application / product layer — `NOT_STARTED`; recommended next gate is Phase 12G — Separate Application Mode / Application Boundary Planning (docs-only)
6. Devkit integration of Framework FSM + bridge — `NOT_STARTED`; recommended mode = SEPARATE APPLICATION (Mode 4); shadow / replacement / hold / host-extension remain fallback directions if user overrides
7. Architecture A ↔ Architecture B reconciliation — `NOT_STARTED`; out of scope
8. Final application directory shape, build separation strategy, event-mapping policy at the application layer — open for Phase 12G
9. Bridge ABI memory-layout lock — `NOT_STARTED`; only names + signatures are `LOCKED-AT-12F`

#### Next gate

**Hold.** Phase 12G — Separate Application Mode / Application Boundary Planning (docs-only) may open only on **explicit user authorization** AND with the recommended scope from §F and §L of the Phase 12G-pre closeout. Phase 12G remains `NOT_STARTED`. If the user prefers a different mode, the fallback gates are: Phase 12G — Devkit Shadow Integration Spec (if SHADOW), or no next phase (if HOLD), or Phase 12G — Bridge / FSM Host Extension Planning (if HOST-ONLY EXTENSION). Replacement mode remains structurally forbidden until preconditions are cleared.

---

### Phase 12F — Framework Application Bridge Host Prototype (host-test evidence)

- **Date:** 2026-05-13
- **Type:** First Framework Application Bridge implementation phase. **Host-only prototype.** No devkit integration. No UART command. No Zephyr config change. No hardware run. No legacy Architecture B modification. No `core/`, `platform/`, `devkit/src/`, `devkit/CMakeLists.txt`, root `RobotOS_v1.0/CMakeLists.txt`, `framework/robotos_fw_fsm.h`, `framework/robotos_fw_fsm.c`, or `framework/README.md` change.
- **Close status:** `CLOSED_WITH_HOST_TEST_EVIDENCE`
- **Decision result:** `PHASE_12F_APPLICATION_BRIDGE_HOST_PROTOTYPE_CLOSED`
- **Published baseline at open:** `origin/master = 023987a`
- **Closeout doc:** `RobotOS_v1.0/devkit/docs/02_PHASE_CLOSEOUTS/PHASE_12F_APPLICATION_BRIDGE_HOST_PROTOTYPE.md`
- **Phase log entry:** `RobotOS_v1.0/devkit/docs/01_PROGRESS/DEVKIT_PROGRESS_PHASE_11_20.md` `<a id="phase-12f"></a>`
- **Tracked host log:** `RobotOS_v1.0/tests/host/logs/phase_12F_host_2026-05-13.log`

#### Framework Application Bridge implementation status

- **`HOST_TEST_VALIDATED_PROTOTYPE`.**
- First Framework bridge module: `RobotOS_v1.0/framework/robotos_fw_event_bridge.c` (body) +`RobotOS_v1.0/framework/robotos_fw_event_bridge.h` (header — zero-diff against any prior Phase; this is its first appearance).
- Public API (four functions, `LOCKED-AT-12F` for names + signatures): `robotos_fw_event_bridge_init`, `robotos_fw_event_bridge_dispatch`, `robotos_fw_event_bridge_reset`, `robotos_fw_event_bridge_get_snapshot`.
- Memory layout / ABI: still `DRAFT / EXPERIMENTAL`.

#### Host validation evidence

- **Environment:** WSL Ubuntu / gcc 13.3.0 / Unix Makefiles (Phase 12E precedent; MSYS2 MinGW64 not used).
- **Bridge contract test:** 17 cases / **103/103 assertions PASS, 0 FAIL** (`tests/host/test_robotos_fw_event_bridge.c`).
- **Full host regression:** **22/22 PASS** (was 21/21 at Phase 12E; one additive ctest target — `robotos_fw_event_bridge_contract`).
- **FSM regression:** 93/93 (unchanged from Phase 12E; FSM header + body zero-diff).
- **Log convention:** `cmake --build … --target save_test_log` wrote `tests/host/logs/host_2026-05-13.log`; phase-tagged copy at `tests/host/logs/phase_12F_host_2026-05-13.log`.

#### Bridge behavior locked at Phase 12F

- **Mapping:** FIFO first-match; row order is the only precedence rule (wildcard rows at lower indices shadow exact-match rows at higher indices).
- **Wildcard:** `match_arg0 == false` matches any `arg0` value for a given `adapter_type`.
- **Unmapped events:** silent OK + `unmapped_count++`; FSM not called.
- **Payload:** borrowed `const void *` forwarded verbatim; bridge struct has no payload field (structurally asserted by TC08).
- **Status mapping:** reuses `robotos_core_status_t`; FSM non-OK propagates verbatim; no new enum.
- **Reset:** zeroes bridge counters only; **FSM state preserved** (TC15).
- **Re-init:** idempotent; FSM not touched (TC17).
- **Threading:** thread context only; no critical section taken by the bridge; no ISR support.
- **Counter invariant:** `event_count == mapped_count + unmapped_count`.

#### Relationship to `devkit_app_state` (scope-guard #11 preserved)

- `devkit_app_state` remains **authoritative** for the devkit runtime state machine (IDLE/ARMED/ACTIVE; owns `?` response, `a/s/r/d/t/T` byte handling, button cycle semantics).
- Phase 12F host bridge prototype lives in a separate translation unit exercised **only** by a host test executable. No devkit producer file calls into the bridge in Phase 12F.
- Three future devkit-integration modes (shadow, replacement, separate application) remain `UNDECIDED`; each requires its own future planning gate. None is authorized at this commit.

#### Relationship to command set

- `a / s / r / ? / x / v / L / d / T` unchanged. Bridge added zero UART commands; bridge surface is not reachable from UART RX or TX in Phase 12F.

#### What is preserved unchanged at Phase 12F

- Validated command set: `a / s / r / ? / x / v / L / d / T` (unchanged).
- `devkit_app_state`: devkit-local; not promoted, not replaced, not copied (scope-guard #11 re-affirmed).
- `T`: Adapter probe evidence (Phase 11E); not promoted.
- All 12 UART TX scope-guard constraints from `PHASE_9EZ_CHECKPOINT.md §H` intact.
- `framework/robotos_fw_fsm.h` (Phase 12D LOCKED-AT-12D), `framework/robotos_fw_fsm.c` (Phase 12E), `framework/README.md` — zero-diff.
- `core/`, `platform/`, `devkit/src/`, `devkit/CMakeLists.txt`, root `RobotOS_v1.0/CMakeLists.txt` — zero-diff.
- `tests/CMakeLists.txt`, all existing host test sources and targets — zero-diff (only `tests/host/CMakeLists.txt` modified, additively).
- `src/`, `include/robotos/`, `include/app/` — zero-diff (Architecture B frozen at Phase 12D-pre).
- All board DTS, overlays, `prj.conf`, Zephyr workspace files — zero-diff.
- All evidence logs from prior phases — zero-diff.
- All prior closeout docs — not rewritten.

#### Remaining decisions (all preserved unchanged at Phase 12F)

1. ACTIVE disarm widening — `USER_DECISION_REQUIRED_ACTIVE_DISARM`
2. Scheduler 7A/7B — `DEFER`
3. F407 / custom board — `HOLD/DEFER`
4. POST_FLASH_AUTOSTART root cause — `OPEN` / `MITIGATED_BY_WORKFLOW`
5. Application / product layer — `NOT_STARTED`
6. Devkit integration of Framework FSM + bridge — `NOT_STARTED`; future planning gate required to choose a mode
7. Architecture A ↔ Architecture B reconciliation — `NOT_STARTED`; out of scope
8. Future devkit-integration mode (shadow / replacement / separate application) — `UNDECIDED`; each mode requires its own future planning phase
9. Bridge ABI memory-layout lock — `NOT_STARTED`; only names + signatures are `LOCKED-AT-12F`

#### Next gate

**Hold or open a docs-only Phase 12G-pre devkit-integration-mode decision.** Do not open direct devkit integration without an explicit planning gate. A second viable next gate is additional host-only bridge behavior (e.g., `arg1` matching, unmapped-event diagnostic hook, fan-out). Neither is authorized at this commit. Phase 12G / 12G-pre / further bridge extension all remain `NOT_STARTED`.

---

### Phase 12F-pre — Application Bridge Planning (docs-only)

- **Date:** 2026-05-12
- **Type:** Docs-only planning gate. **No source, runtime, test, CMake, Zephyr, board, `prj.conf`, DTS overlay, evidence log, Framework header, Framework `.c` file, devkit integration, command-set, or `devkit_app_state` change.** No file under `framework/`, `tests/`, `core/`, `platform/`, `devkit/src/`, `src/`, `include/robotos/` modified.
- **Close status:** `CLOSED_DOCS_ONLY`
- **Decision result:** `PHASE_12F_PRE_RECOMMEND_HOST_BRIDGE_PROTOTYPE`
- **Published baseline at open:** `origin/master = df9bb8e`
- **Closeout doc:** `RobotOS_v1.0/devkit/docs/02_PHASE_CLOSEOUTS/PHASE_12F_PRE_APPLICATION_BRIDGE_PLANNING.md`
- **New long-lived spec draft:** `RobotOS_v1.0/devkit/docs/03_SPECS/FRAMEWORK_APPLICATION_BRIDGE_DRAFT.md` (`DRAFT / NON-FINAL`; no implementation exists yet; spec for the future bridge module).
- **Phase log entry:** `RobotOS_v1.0/devkit/docs/01_PROGRESS/DEVKIT_PROGRESS_PHASE_11_20.md` `<a id="phase-12f-pre"></a>`

#### Phase 12F recommendation

**`PHASE_12F_PRE_RECOMMEND_HOST_BRIDGE_PROTOTYPE`.** Phase 12F, when authorized, should:

- Add `RobotOS_v1.0/framework/robotos_fw_event_bridge.h` and `RobotOS_v1.0/framework/robotos_fw_event_bridge.c` — a small product-neutral mapping module that translates Adapter-style keys `(adapter_type, adapter_arg0)` into `robotos_fw_event_id_t` and calls `robotos_fw_fsm_dispatch()`.
- Add `RobotOS_v1.0/tests/host/test_robotos_fw_event_bridge.c` exercising 12 runtime cases + 5 review-validated items mapping to the Phase 12C `APPLICATION_OWNED_EVENT_BRIDGE_CONFIRMED` contract.
- Update `RobotOS_v1.0/tests/host/CMakeLists.txt` additively only (one `add_executable` + one `add_test`).
- Validate on WSL Ubuntu / gcc 13.3.0 (same toolchain as Phase 12E).
- Commit a tracked test log at `RobotOS_v1.0/tests/host/logs/phase_12F_host_<date>.log`.
- **Not** integrate with devkit. **Not** modify `devkit_app_state`. **Not** add or remove UART commands. **Not** modify Architecture B. **Not** create `framework/CMakeLists.txt` or `framework/src/`. **Not** modify `devkit/CMakeLists.txt` or root `RobotOS_v1.0/CMakeLists.txt`. **Not** create `RobotOS_v1.0/app/` or any Application-layer file.

#### Bridge contract frozen at planning depth

- **Ownership:** bridge owns mapping table (static const array, FIFO first-match); caller owns FSM instance via pointer; caller owns mapping memory.
- **Adapter key:** `(uint32_t adapter_type, uint32_t adapter_arg0)` with optional `arg0` wildcard.
- **Unmapped events:** silent OK + `unmapped_count++`; FSM not called; no default Framework event ID synthesized.
- **Payload:** borrowed `const void *` for dispatch duration only; bridge struct has no payload field.
- **Status:** reuse `robotos_core_status_t` via `robotos_fw_status_t` alias; no new public status enum.
- **Threading:** thread context only; no ISR; no critical section needed.
- **Forbidden surface:** no UART TX; no GPIO/PWM/I2C/SPI drivers; no `robotos_core_register_event_handler`; no Zephyr / devkit / legacy `ro_*` includes; no heap; no `devkit_app_state` reference.

Full spec at `RobotOS_v1.0/devkit/docs/03_SPECS/FRAMEWORK_APPLICATION_BRIDGE_DRAFT.md` (sections 1-12).

#### Candidate bridge paths evaluated

| Option | Verdict | Reason |
|---|---|---|
| 1. Host-only bridge prototype | **RECOMMENDED** | Product-neutral; reuses Architecture-A host test infra; zero devkit drift; zero `devkit_app_state` collision; zero command-set risk; resolves Phase 12C bridge contract concretely |
| 2. Devkit shadow bridge | REJECTED at Phase 12F | Duplicate state truth; modifies devkit producers; requires hardware evidence; touches scope-guard #11 spirit; deferrable to future Phase 12G-pre |
| 3. Devkit replacement bridge | REJECTED | Directly violates scope-guard #11; `?` UART response shape can't reconcile with product-neutral FSM; never recommended without dedicated migration phase |
| 4. Application-layer bridge skeleton | REJECTED at Phase 12F | Application/product layer is `NOT_STARTED`; scope explosion; needs Phase 13-pre Application planning first |
| 5. Hold | FALLBACK ACCEPTABLE | Leaves bridge contract untested; acceptable only if user defers further |

#### Relationship to `devkit_app_state` (scope-guard #11 preserved)

- `devkit_app_state` remains **authoritative** for the devkit runtime state machine (IDLE/ARMED/ACTIVE; owns `?` response, `a/s/r/d/t/T` byte handling, button cycle semantics).
- Phase 12F (host bridge prototype) does **not** replace, promote, copy, shadow, or duplicate `devkit_app_state`. The bridge lives in a separate translation unit exercised only by a host test executable. No devkit producer file calls into the bridge in Phase 12F.
- Three future devkit-integration modes are explicitly enumerated (shadow, replacement, separate application); each requires its own future planning phase and explicit user authorization. None is in scope for Phase 12F.

#### Relationship to command set

- `a / s / r / ? / x / v / L / d / T` remain devkit/probe command surface; bridge adds no UART command and no UART exposure of FSM state.
- Framework FSM is not exposed via UART automatically.
- Future command vocabulary belongs to a future Application/product phase, not the bridge prototype.

#### Required behavior coverage for future Phase 12F

12 runtime cases (mapped dispatch; unmapped silent ignore; payload pass-through; FSM non-OK propagation; no payload caching; no UART/dispatcher coupling; mapping determinism; multi-row support; host-only synthetic events; no `devkit_app_state` reference; command-set zero-diff) + 5 review-validated items (no heap; no Zephyr/devkit/legacy includes; no payload field on bridge struct; public symbol surface match; full host suite regression). Full mapping at closeout §I.

#### Bridge implementation status

- **Bridge implementation = `NOT_STARTED`.** No `framework/robotos_fw_event_bridge.{h,c}` exists.
- **Phase 12F = `NOT_STARTED`.** Recommended path is recorded. Opening Phase 12F requires explicit user authorization.

#### What is preserved unchanged at Phase 12F-pre

- Validated command set: **`a / s / r / ? / x / v / L / d / T`** (unchanged).
- `devkit_app_state`: devkit-local; not promoted, not replaced, not copied (scope-guard #11 re-affirmed).
- `T`: Adapter probe evidence (Phase 11E); not promoted.
- All 12 UART TX scope-guard constraints from `PHASE_9EZ_CHECKPOINT.md §H` intact.
- All `.c` and `.h` files in the repo — zero-diff.
- `framework/robotos_fw_fsm.h`, `framework/robotos_fw_fsm.c`, `framework/README.md` — zero-diff.
- All `CMakeLists.txt` (root, `devkit/`, `tests/`, `tests/host/`) — zero-diff.
- All tracked test files under `tests/` — zero-diff.
- `core/`, `platform/`, `devkit/src/`, board DTS/overlays, Zephyr workspace — zero-diff.
- `src/`, `include/robotos/`, `include/app/` — zero-diff.
- Architecture B legacy notices — zero-diff.
- All evidence logs — zero-diff.
- All prior closeout docs — not rewritten.

#### Remaining decisions (all preserved unchanged at Phase 12F-pre)

1. ACTIVE disarm widening — `USER_DECISION_REQUIRED_ACTIVE_DISARM`
2. Scheduler 7A/7B — `DEFER`
3. F407 / custom board — `HOLD/DEFER`
4. POST_FLASH_AUTOSTART root cause — `OPEN` / `MITIGATED_BY_WORKFLOW`
5. Application / product layer — `NOT_STARTED`
6. Devkit integration of Framework FSM — `NOT_STARTED`; Phase 12F recommended path = host bridge prototype only; opening requires explicit user authorization
7. Architecture A ↔ Architecture B reconciliation — `NOT_STARTED`; out of scope
8. Future devkit-integration mode (shadow / replacement / separate application) — `UNDECIDED`; each mode requires its own future planning phase

#### Next gate

**Hold.** Phase 12F (Framework Application Bridge Host Prototype) may open only on **explicit user authorization** AND with the recommended scope from §F/§L of the Phase 12F-pre closeout. Phase 12F remains `NOT_STARTED`.

---

### Phase 12E — Framework FSM Host-Test Implementation (host-test evidence)

- **Date:** 2026-05-12
- **Type:** First Framework implementation phase. **No devkit integration. No UART command. No Zephyr config change. No hardware run. No legacy Architecture B modification. No `core/`, `platform/`, `devkit/src/`, `devkit/CMakeLists.txt`, root `RobotOS_v1.0/CMakeLists.txt`, `framework/robotos_fw_fsm.h`, or `framework/README.md` change.**
- **Close status:** `CLOSED_WITH_HOST_TEST_EVIDENCE`
- **Decision result:** `PHASE_12E_FSM_HOST_TEST_IMPLEMENTATION_CLOSED`
- **Published baseline at open:** `origin/master = a8019b5`
- **Closeout doc:** `RobotOS_v1.0/devkit/docs/02_PHASE_CLOSEOUTS/PHASE_12E_FSM_HOST_TEST_IMPLEMENTATION.md`
- **Phase log entry:** `RobotOS_v1.0/devkit/docs/01_PROGRESS/DEVKIT_PROGRESS_PHASE_11_20.md` `<a id="phase-12e"></a>`

#### Framework implementation status

- **`HOST_TEST_VALIDATED_FSM_CORE`.**
- Active Framework path: `RobotOS_v1.0/framework/` (Architecture A; sibling of `core/`, `platform/`, `devkit/`).
- **First Framework `.c` body in repo:** `RobotOS_v1.0/framework/robotos_fw_fsm.c` (~210 lines).
- Header `RobotOS_v1.0/framework/robotos_fw_fsm.h` is LOCKED-AT-12D and zero-diff at Phase 12E.
- Implementation includes only `"robotos_fw_fsm.h"`, `"robotos_platform_critical.h"`, `<stddef.h>`. No Zephyr, no devkit, no legacy `ro_*`, no heap, no UART, no core dispatcher registration.
- Six public functions implemented: `robotos_fw_fsm_init`, `robotos_fw_fsm_dispatch`, `robotos_fw_fsm_get_state`, `robotos_fw_fsm_reset`, `robotos_fw_fsm_is_in_state`, `robotos_fw_fsm_get_snapshot`. Two private static helpers: `fw_find_state_def`, `fw_reset_counters`.
- Entry/exit non-OK behavior: **observed but not propagated** by FSM (Phase 12E design decision; documented in closeout §D.5; action return is the sole driver of `last_status`/`dispatch()` return).
- Re-init policy: **idempotent** — re-init resets state and counters; entry runs again.

#### Host test result

- **Environment:** WSL Ubuntu 24.04 LTS, gcc 13.3.0, system cmake, Unix Makefiles.
- **Local MSYS2 MinGW64 NOT used** — Phase 12D documented it as unreliable (silent exit=1 with 0-byte diagnostics); Phase 12E-pre §G mandated WSL/Linux for this phase.
- **New host test target:** `robotos_fw_fsm_contract_test` added additively to `RobotOS_v1.0/tests/host/CMakeLists.txt` (one `add_executable` + one `add_test`; no existing test target modified; existing 20 Phase 4-6 contract targets untouched).
- **Test source:** `RobotOS_v1.0/tests/host/test_robotos_fw_fsm.c` — 20 test cases, **93 assertions**.
- **Targeted ctest run:** PASS (93/93 assertions; 0 failures; 0.06 s).
- **Full host suite regression:** PASS (21/21 tests passed, 0 failures).
- **Test log committed:** `RobotOS_v1.0/tests/host/logs/phase_12E_host_2026-05-12.log` (160 lines, tracked).

#### Behavior coverage matrix

All 25 contract items from Phase 12E-pre §F satisfied:

| Verdict | Count | Items |
|---|---|---|
| `ASSERTED_BY_TEST` | 21 | init valid/invalid; dispatch matched/unmatched; first-match FIFO; guard reject (single + counter independence); guard pass; exit→state→action→entry order; action non-OK no rollback; reset; get_state; is_in_state; get_snapshot; transition_count; event_count; last_event_id; payload borrowed not cached; pre/post-transition state observations; re-init idempotent; critical-section usage |
| `REVIEW_VALIDATED` | 4 | entry/exit non-OK behavior (Phase 12E design choice documented); no heap (grep 0 matches); no UART/core register (grep 0 matches); public-symbol surface matches LOCKED-AT-12D |
| `BLOCKED` | 0 | — |

#### Phase 12D syntax-check resolution

Phase 12D's `SYNTAX_CHECK_NOT_RUN_TOOLCHAIN_OUTPUT_SUPPRESSED` is resolved. The header is now compile-validated by transitive inclusion in `robotos_fw_fsm.c` and `test_robotos_fw_fsm.c` under gcc 13.3.0.

#### What is preserved unchanged at Phase 12E

- Validated command set: **`a / s / r / ? / x / v / L / d / T`** (unchanged).
- `devkit_app_state`: devkit-local; not promoted, not replaced, not copied (scope-guard #11 re-affirmed).
- `T`: Adapter probe evidence (Phase 11E); not promoted.
- All 12 UART TX scope-guard constraints from `PHASE_9EZ_CHECKPOINT.md §H` intact.
- `framework/robotos_fw_fsm.h` — zero-diff (LOCKED-AT-12D held).
- `framework/README.md` — zero-diff.
- All `.c` files outside `framework/robotos_fw_fsm.c` and `tests/host/test_robotos_fw_fsm.c` — zero-diff.
- All `CMakeLists.txt` outside `tests/host/CMakeLists.txt` (root, `devkit/`, `tests/`) — zero-diff.
- `core/`, `platform/`, `devkit/src/`, board DTS/overlays, Zephyr workspace — zero-diff.
- `src/`, `include/robotos/`, `include/app/` — zero-diff (Architecture B).
- Architecture B legacy notices — zero-diff.
- All evidence logs outside the new Phase 12E host log — zero-diff.
- All prior closeout docs — not rewritten.

#### Remaining decisions (all preserved unchanged at Phase 12E)

1. ACTIVE disarm widening — `USER_DECISION_REQUIRED_ACTIVE_DISARM`
2. Scheduler 7A/7B — `DEFER`
3. F407 / custom board — `HOLD/DEFER`
4. POST_FLASH_AUTOSTART root cause — `OPEN` / `MITIGATED_BY_WORKFLOW`
5. Application / product layer — `NOT_STARTED`
6. Devkit integration of Framework FSM — `NOT_STARTED`; gated by scope-guard #11 and Application/product layer `NOT_STARTED`
7. Architecture A ↔ Architecture B reconciliation — `NOT_STARTED`; out of scope

#### Next gate

**Hold.** Phase 12F candidates (see closeout §H.2):
- **Phase 12F-pre — Application bridge planning** (recommended when user is ready to move toward devkit integration).
- **Phase 12F — additional FSM host behavior** (lower priority; 25-item coverage already met).
- **Hold** (acceptable indefinitely; Framework FSM core is complete at the host-test surface).

Devkit integration is **not** the next phase — that would collide with scope-guard #11 and the Application-layer-NOT_STARTED constraint.

---

### Phase 12E-pre — Framework FSM Consumer / Test Plan (docs-only)

- **Date:** 2026-05-12
- **Type:** Docs-only planning gate. **No source, runtime, test, CMake, Zephyr, board, `prj.conf`, DTS overlay, evidence log, Framework header, Framework `.c` file, devkit integration, or command-set change.** No file under `framework/`, `tests/host/`, `core/`, `platform/`, `devkit/src/`, `src/`, `include/robotos/` modified.
- **Close status:** `CLOSED_DOCS_ONLY`
- **Decision result:** `PHASE_12E_RECOMMEND_HOST_UNIT_TEST_CONSUMER`
- **Published baseline at open:** `origin/master = 87e0626`
- **Closeout doc:** `RobotOS_v1.0/devkit/docs/02_PHASE_CLOSEOUTS/PHASE_12E_PRE_FSM_CONSUMER_TEST_PLAN.md`
- **Phase log entry:** `RobotOS_v1.0/devkit/docs/01_PROGRESS/DEVKIT_PROGRESS_PHASE_11_20.md` `<a id="phase-12e-pre"></a>`

#### Phase 12E recommendation

**`PHASE_12E_RECOMMEND_HOST_UNIT_TEST_CONSUMER`.** Phase 12E, when authorized, should:

- Implement `RobotOS_v1.0/framework/robotos_fw_fsm.c` against the Phase 12D-LOCKED header surface.
- Add a single new host test target inside the existing `RobotOS_v1.0/tests/host/CMakeLists.txt` (additive only — one `add_executable` + one `add_test`).
- Add `RobotOS_v1.0/tests/host/test_robotos_fw_fsm.c` exercising 21 runtime-asserted contract cases + 4 review cases mapping to Phase 12B/12C decisions.
- Validate on WSL Ubuntu or Linux (the existing host CMake documents Windows MinGW64 as unreliable — same finding as Phase 12D `SYNTAX_CHECK_NOT_RUN_TOOLCHAIN_OUTPUT_SUPPRESSED`).
- Commit a tracked test log via the existing `save_test_log.cmake` convention at `RobotOS_v1.0/tests/host/logs/phase_12E_host_<date>.log`.
- **Not** integrate with devkit. **Not** modify `devkit_app_state`. **Not** add or remove UART commands. **Not** modify Architecture B. **Not** create `framework/CMakeLists.txt` or `framework/src/`. **Not** modify `devkit/CMakeLists.txt` or root `RobotOS_v1.0/CMakeLists.txt`.

#### Candidate consumer/test paths evaluated

| Option | Verdict | Reason |
|---|---|---|
| 1. Host unit-test consumer | **RECOMMENDED** | Reuses Architecture-A host test infra proven through Phase 4-6; additive only; resolves Phase 12D syntax-check unknown; no devkit drift |
| 2. Compile-only skeleton | REJECTED | No behavioral validation; Option 1 absorbs its benefit for free |
| 3. Devkit integration consumer | REJECTED | Scope-guard #11 collision on `devkit_app_state`; command-semantics drift risk; too much for one phase |
| 4. Application bridge prototype | REJECTED | Application/product layer is `NOT_STARTED`; no product chosen; three layers in one phase |
| 5. Hold | FALLBACK ACCEPTABLE | Leaves Phase 12D syntax-check unresolved; acceptable only if user defers further |

#### Test infrastructure finding

**`TEST_INFRA_AUDIT_NOT_NEEDED — EXISTING_HOST_TESTS_SUFFICE`.**

- `RobotOS_v1.0/tests/host/CMakeLists.txt` (426 lines, tracked, standalone — no Zephyr / west / legacy `src/` dependency) is the canonical Architecture-A host test build.
- ~20 tracked Phase 4-6 contract test targets demonstrate the convention works (core, event queue, dispatcher, ingestion, tick policy, handler policy, platform critical/fault, scheduler admission, queue pressure, handler routing stress, handler lifecycle).
- Tracked logs through Phase 6H (`phase_4K_host_*.log` ... `phase_6H_host_*.log`) prove the log-capture convention.
- Tracked platform host stubs (`robotos_platform_critical_host_stub.{c,h}`, `robotos_platform_fault_host_stub.{c,h}`, `robotos_platform_log_host_stub.c`, `robotos_platform_time_host_stub.c`) reusable for FSM test.
- No Phase 12E-test-pre infrastructure phase needed.

`RobotOS_v1.0/tests/` root (`test_app_sm.c`, `test_gcode_parser.c`, `test_kinematics_cartesian.c`, `test_motion_planner.c`, `tests/CMakeLists.txt`) is Architecture B and frozen by extension of Phase 12D-pre. Phase 12E must not modify it.

#### Phase 12E exit criteria (recorded for future use; not active until Phase 12E opens)

- `framework/robotos_fw_fsm.c` exists; compiles cleanly via the host CMake on WSL/Linux.
- `tests/host/test_robotos_fw_fsm.c` covers 21 runtime cases.
- `ctest --output-on-failure` reports PASS for `test_robotos_fw_fsm`.
- Test log committed at `tests/host/logs/phase_12E_host_<date>.log`.
- Review evidence (grep) for the 4 review cases recorded in the Phase 12E closeout.
- All Phase 12E-pre gates preserved unchanged.

#### Phase 12E status

- **Phase 12E = `NOT_STARTED`.** Recommended path is recorded. Opening Phase 12E requires explicit user authorization.

#### What is preserved unchanged at Phase 12E-pre

- Validated command set: **`a / s / r / ? / x / v / L / d / T`** (unchanged).
- `devkit_app_state`: devkit-local; not promoted, not replaced (scope-guard #11 re-affirmed).
- `T`: Adapter probe evidence (Phase 11E); not promoted.
- All 12 UART TX scope-guard constraints from `PHASE_9EZ_CHECKPOINT.md §H` intact.
- All `.c` files everywhere in the repo — zero-diff.
- `RobotOS_v1.0/framework/robotos_fw_fsm.h` — zero-diff.
- `RobotOS_v1.0/framework/README.md` — zero-diff.
- All `CMakeLists.txt` (root, `devkit/`, `tests/`, `tests/host/`) — zero-diff.
- All tracked test files under `tests/` — zero-diff.
- `core/`, `platform/`, `devkit/src/`, board DTS/overlays, Zephyr workspace — zero-diff.
- `src/`, `include/robotos/`, `include/app/` — zero-diff.
- Architecture B legacy notices — zero-diff.
- All evidence logs — zero-diff.
- All prior closeout docs — not rewritten.

#### Remaining decisions (all preserved unchanged at Phase 12E-pre)

1. ACTIVE disarm widening — `USER_DECISION_REQUIRED_ACTIVE_DISARM`
2. Scheduler 7A/7B — `DEFER`
3. F407 / custom board — `HOLD/DEFER`
4. POST_FLASH_AUTOSTART root cause — `OPEN` / `MITIGATED_BY_WORKFLOW`
5. Application / product layer — `NOT_STARTED`
6. Robot Framework implementation — `NOT_STARTED`; Phase 12E recommended path = host unit test; opening requires explicit user authorization
7. Architecture A ↔ Architecture B reconciliation — `NOT_STARTED`; out of scope

#### Next gate

**Hold.** Phase 12E (Framework FSM host-test implementation) may open only on **explicit user authorization** AND with the recommended scope from §E/§J of the Phase 12E-pre closeout doc.

---

### Phase 12D — Framework FSM Header Stub (header-only + docs)

- **Date:** 2026-05-12
- **Type:** Header stub + docs. **No `.c` body, no CMake change, no devkit integration, no Zephyr config change, no hardware run, no source modification outside the two new framework files, no command-semantic change.** Two new files at `RobotOS_v1.0/framework/`: layer README + single public header.
- **Close status:** `CLOSED_HEADER_STUB_ONLY`
- **Decision result:** `PHASE_12D_FSM_HEADER_STUB_CREATED`
- **Published baseline at open:** `origin/master = 2385b0f`
- **Closeout doc:** `RobotOS_v1.0/devkit/docs/02_PHASE_CLOSEOUTS/PHASE_12D_FSM_HEADER_STUB.md`
- **Phase log entry:** `RobotOS_v1.0/devkit/docs/01_PROGRESS/DEVKIT_PROGRESS_PHASE_11_20.md` `<a id="phase-12d"></a>`
- **New files (only):** `RobotOS_v1.0/framework/README.md`, `RobotOS_v1.0/framework/robotos_fw_fsm.h`.
- **Long-lived spec updated:** `RobotOS_v1.0/devkit/docs/03_SPECS/FRAMEWORK_FSM_API_DRAFT.md` (§1 decision-state table; §4 names move from DRAFT to `LOCKED-AT-12D`; §10 next-revision conditions updated). ABI remains **NOT** stable.

#### Active Framework path created

- **Path:** `RobotOS_v1.0/framework/` (Architecture A; sibling of `core/`, `platform/`, `devkit/`).
- **Contents (Phase 12D):**
  - `README.md` — Framework layer identity, boundary statement, distinction from frozen Architecture B, scope of Phase 12D.
  - `robotos_fw_fsm.h` — Phase 12D header stub; DRAFT / EXPERIMENTAL; declarations only.
- **What is NOT present:** no `.c` file, no `framework/CMakeLists.txt`, no `framework/src/`, no `framework/include/`, no app/ subdirectory, no devkit consumer of the header, no link from `devkit/CMakeLists.txt`, no `ROBOTOS_FW` RTT stream.

#### Header surface summary (LOCKED-AT-12D)

`robotos_fw_fsm.h` declares (declarations only — no bodies):

- Types: `robotos_fw_state_id_t` (`uint32_t`), `robotos_fw_event_id_t` (`uint32_t`, separate namespace from `robotos_event_type_t`), `robotos_fw_status_t` (alias of `robotos_core_status_t`).
- Constant: `ROBOTOS_FW_STATE_UNINIT` = `((robotos_fw_state_id_t)0u)`.
- Callbacks: `robotos_fw_guard_fn_t` (returns `bool`), `robotos_fw_action_fn_t`, `robotos_fw_entry_exit_fn_t`.
- Structs: `robotos_fw_transition_t`, `robotos_fw_state_def_t`, `robotos_fw_fsm_config_t`, `robotos_fw_fsm_snapshot_t`, `robotos_fw_fsm_t`.
- Functions: `robotos_fw_fsm_init`, `robotos_fw_fsm_dispatch`, `robotos_fw_fsm_get_state`, `robotos_fw_fsm_reset`, `robotos_fw_fsm_is_in_state`, `robotos_fw_fsm_get_snapshot`.
- Includes (only): `<stdbool.h>`, `<stdint.h>`, `"robotos_core.h"`. No Zephyr, no devkit, no legacy `ro_*`, no app headers.

#### Phase 12C decisions encoded in header

- `APPLICATION_OWNED_EVENT_BRIDGE_CONFIRMED` — FSM never calls `robotos_core_register_event_handler`.
- `REUSE_ROBOTOS_CORE_STATUS_T_FOR_PHASE_12D_CONFIRMED` — `robotos_fw_status_t` is an alias.
- `PAYLOAD_BORROWED_FOR_DISPATCH_ONLY_CONFIRMED` — `event_payload` is `const void *`, borrowed.
- `ACTION_NON_OK_NO_ROLLBACK_CONFIRMED` — state committed before action; entry runs regardless.
- `GUARD_RETURNS_BOOL_ONLY_CONFIRMED` — guard returns `bool` only.
- Evaluation order: **exit → state update → action → entry**.

#### Syntax / validation

`SYNTAX_CHECK_NOT_RUN_TOOLCHAIN_OUTPUT_SUPPRESSED`. Attempt with MSYS2 gcc 15.1.0 (`gcc -fsyntax-only -std=c99 -Wall -Wextra`) returned exit code 1 with 0-byte stdout/stderr on every invocation, including a deliberately broken control source. Diagnostics are unrecoverable in this sandbox; the check is not informative. Header was reviewed by hand. A future implementation phase MUST run a toolchain-backed compile (CMake + Ninja under the Architecture A devkit build) before claiming validity.

#### Implementation status

- Robot Framework implementation = **`HEADER_STUB_ONLY` / no `.c` body**.
- Phase 12E = `NOT_STARTED`. Requires explicit user authorization AND a concrete consumer (devkit integration target) or unit-test plan identified in advance.
- Application / product layer = `NOT_STARTED`.

#### What is preserved unchanged at Phase 12D

- Validated command set: **`a / s / r / ? / x / v / L / d / T`** (9 commands; hardware-evidence-backed).
- `devkit_app_state`: devkit-local; not promoted, not replaced, not copied (scope-guard #11 re-affirmed).
- `T`: Adapter probe evidence (Phase 11E `CLOSED_WITH_HARDWARE_EVIDENCE`); not promoted.
- All 12 UART TX scope-guard constraints from `PHASE_9EZ_CHECKPOINT.md §H` intact.
- All `.c` files everywhere in the repo — zero-diff.
- All `CMakeLists.txt` (root legacy and `devkit/`) — zero-diff.
- `core/`, `platform/`, `devkit/src/`, `tests/`, board DTS/overlays, Zephyr workspace — zero-diff.
- `src/`, `include/robotos/`, `include/app/` — zero-diff after Phase 12D-pre.
- Architecture B legacy notices (`src/README_LEGACY_SCAFFOLD.md`, `src/framework/DEPRECATED.md`, `include/robotos/DEPRECATED.md`) — zero-diff.
- All evidence logs — zero-diff.
- All prior closeout docs — not rewritten.

#### Remaining decisions (all preserved unchanged at Phase 12D)

1. ACTIVE disarm widening — `USER_DECISION_REQUIRED_ACTIVE_DISARM`
2. Scheduler 7A/7B — `DEFER`
3. F407 / custom board — `HOLD/DEFER`
4. POST_FLASH_AUTOSTART root cause — `OPEN` / `MITIGATED_BY_WORKFLOW`
5. Application / product layer — `NOT_STARTED`
6. Robot Framework implementation — `HEADER_STUB_ONLY`; Phase 12E pending explicit user authorization + concrete consumer/test
7. Architecture A ↔ Architecture B reconciliation — `NOT_STARTED`; out of scope

#### Next gate

**Hold.** Phase 12E (Framework FSM implementation) may open only on **explicit user authorization** AND with a concrete consumer or unit-test plan identified in advance. Hold is the recommended posture.

---

### Phase 12D-pre — Legacy Framework Scaffold Disposition (docs-only)

- **Date:** 2026-05-12
- **Type:** Docs-only architecture-boundary phase. No source, runtime, test, CMake, Zephyr, board, `prj.conf`, DTS overlay, evidence log, `framework/` directory, or `.h`/`.c` Framework file change. No file deleted, moved, or renamed.
- **Close status:** `CLOSED_DOCS_ONLY`
- **Decision result:** `LEGACY_SCAFFOLD_MARKED_FROZEN_DOCS_ONLY`
- **Published baseline at open:** `origin/master = c3a9384`
- **Closeout doc:** `RobotOS_v1.0/devkit/docs/02_PHASE_CLOSEOUTS/PHASE_12D_PRE_LEGACY_FRAMEWORK_SCAFFOLD_DISPOSITION.md`
- **Phase log entry:** `RobotOS_v1.0/devkit/docs/01_PROGRESS/DEVKIT_PROGRESS_PHASE_11_20.md` `<a id="phase-12d-pre"></a>`
- **New legacy notices placed in repo tree:** `RobotOS_v1.0/src/README_LEGACY_SCAFFOLD.md`, `RobotOS_v1.0/src/framework/DEPRECATED.md`, `RobotOS_v1.0/include/robotos/DEPRECATED.md`. Notices are classification documents only; zero source file under `src/` or `include/` is modified, deleted, moved, or renamed.

#### Architecture classification (canonical statement)

- **Architecture A — active / evidence-backed / canonical for Phase 12+:**
  - Top-level dirs: `RobotOS_v1.0/core/`, `RobotOS_v1.0/platform/`, `RobotOS_v1.0/devkit/`.
  - Build entry: `RobotOS_v1.0/devkit/CMakeLists.txt` (Zephyr devkit hardware build).
  - Types: `robotos_core_status_t`, `robotos_event_t`, `robotos_event_type_t`, `robotos_platform_*`.
  - Hardware-validated through Phase 11E on STM32F411E-DISCO rev D.
  - All Phase 12A/12B/12C Framework planning is built on this stack.
- **Architecture B — frozen legacy scaffold / non-authoritative:**
  - Top-level dirs: `RobotOS_v1.0/src/`, `RobotOS_v1.0/include/robotos/`, root `RobotOS_v1.0/CMakeLists.txt`.
  - Source baseline: `43de448` (2025-03-05); **zero source evolution** since baseline.
  - Contents: `src/framework/` (Robot Framework Layer scaffold: stepper, servo, dcmotor, encoder, endstop, sensor, pid, filter, limiter, robot_sm + drivers), `src/adapter/` (zephyr + host HAL backends), `src/app/` (G-code parser, motion planner, kinematics, app_sm), `include/robotos/` (15 `ro_*.h` HAL + 10 framework headers), `include/app/` (app headers).
  - Types: `ro_status_t`, `ro_queue_t`, `ro_thread_t`, `ro_gpio_*`, `ro_pwm_*`, `ro_*` HAL surface.
  - **Zero Phase 1–11 hardware evidence references any file under this tree.**
  - **Not compiled by the active devkit hardware build.**
  - **Type-incompatible with Architecture A.**

#### Confirmed disposition

- Architecture A is **canonical** for Phase 12+ work.
- Architecture B is **legacy / frozen / non-authoritative** unless reopened by a future explicit reconciliation phase (not authorized at Phase 12D-pre).
- `src/framework/` is **not** the active path for Phase 12+ Robot Framework implementation.
- `include/robotos/` is **not** the active namespace for Phase 12+ Framework FSM.
- Root `RobotOS_v1.0/CMakeLists.txt` belongs to the legacy build path; **not modified**, classified.
- **Future active Framework path: `RobotOS_v1.0/framework/`** — new top-level sibling to `core/`, `platform/`, `devkit/`. **Not created by Phase 12D-pre.** Created (if at all) by Phase 12D on explicit user authorization.

#### Phase 12D status

- **Phase 12D = NOT_STARTED.**
- Architectural boundary now explicit; Phase 12D unblocked at the boundary level.
- Phase 12D still requires **explicit user authorization** to open.
- Phase 12D scope (when/if opened): docs + single header (`framework/README.md` + `framework/robotos_fw_fsm.h`) + Phase 12D closeout + spec update (§4 DRAFT → LOCKED-AT-12D) + progress entry + CURRENT_STATE + INDEX. No `.c` body, no CMake change, no devkit integration, no hardware run, no Scheduler change, no F407, no Application/product work, no Architecture B modification.

#### What is preserved unchanged at Phase 12D-pre

- Validated command set: **`a / s / r / ? / x / v / L / d / T`** (9 commands; hardware-evidence-backed).
- `devkit_app_state`: devkit-local; not promoted (scope-guard #11 re-affirmed).
- `T`: Adapter probe evidence (Phase 11E `CLOSED_WITH_HARDWARE_EVIDENCE`); not promoted to Framework sensor API.
- All 12 UART TX scope-guard constraints from `PHASE_9EZ_CHECKPOINT.md §H` intact.
- `core/`, `platform/`, `devkit/src/`, `tests/`, board DTS, board defconfig, Zephyr workspace, `DEVKIT_PROGRESS.md` historical master, and all evidence logs zero-diff.
- All source under `src/`, `include/` — zero-diff (only new notice docs added alongside).
- `RobotOS_v1.0/CMakeLists.txt` (root, legacy) — zero-diff.
- `RobotOS_v1.0/devkit/CMakeLists.txt` (active) — zero-diff.

#### Remaining decisions (all preserved unchanged at Phase 12D-pre)

1. ACTIVE disarm widening — `USER_DECISION_REQUIRED_ACTIVE_DISARM`
2. Scheduler 7A/7B — `DEFER`
3. F407 / custom board — `HOLD/DEFER`
4. POST_FLASH_AUTOSTART root cause — `OPEN` / `MITIGATED_BY_WORKFLOW`
5. Application / product layer — `NOT_STARTED`
6. Robot Framework implementation — `NOT_STARTED`; Phase 12D pending explicit user authorization
7. Architecture A ↔ Architecture B reconciliation — `NOT_STARTED`; out of scope for Phase 12D-pre; would require a separate authorized phase

#### Next gate

**Hold.** No phase opens automatically. Phase 12D (Framework FSM Header Stub at `RobotOS_v1.0/framework/`) may open only on **explicit user authorization**.

---

### Phase 12C — Framework FSM Event Bridge + Status Model Confirmation (docs-only)

- **Date:** 2026-05-12
- **Type:** Docs-only design-confirmation phase. No source, runtime, test, CMake, Zephyr, board, `prj.conf`, DTS overlay, evidence log, `framework/` directory, or `.h`/`.c` Framework file change. Pre-existing `src/framework/*.c` (from `43de448`) unmodified.
- **Close status:** `CLOSED_DOCS_ONLY`
- **Decision result:** `PHASE_12C_EVENT_BRIDGE_STATUS_CONFIRMED_DOCS_ONLY`
- **Published baseline at open:** `origin/master = cda3810`
- **Closeout doc:** `RobotOS_v1.0/devkit/docs/02_PHASE_CLOSEOUTS/PHASE_12C_FSM_EVENT_BRIDGE_STATUS_MODEL.md`
- **Long-lived spec updated:** `RobotOS_v1.0/devkit/docs/03_SPECS/FRAMEWORK_FSM_API_DRAFT.md` (§1 decision-state, §3.2 evaluation order, §6.2/6.3, §7.2, §9 all CONFIRMED, §10 revision conditions). Spec remains `DRAFT / NON-FINAL` — function names lock in Phase 12D.
- **Phase log entry:** `RobotOS_v1.0/devkit/docs/01_PROGRESS/DEVKIT_PROGRESS_PHASE_11_20.md` `<a id="phase-12c"></a>`
- **Robot Framework status:** **NOT BUILT.** No `framework/` directory created by Phase 12C, no Framework header, no `.h`/`.c` file. Phase 12C is design-confirmation only.
- **Confirmed Phase 12B open decisions:**
  1. Event bridge pattern → `APPLICATION_OWNED_EVENT_BRIDGE_CONFIRMED` — Application layer translates Adapter events to `robotos_fw_event_id_t`; FSM does not register with `robotos_core_register_event_handler()` directly; no `ROBOTOS_EVENT_USER` sub-range needed for FSM core.
  2. Status model → `REUSE_ROBOTOS_CORE_STATUS_T_FOR_PHASE_12D_CONFIRMED` — Phase 12D reuses existing `robotos_core_status_t`; no new Framework status enum.
  3. Payload lifetime → `PAYLOAD_BORROWED_FOR_DISPATCH_ONLY_CONFIRMED` — `const void *` payload valid only for `dispatch()` duration; FSM must not cache; Application owns memory; no heap.
  4. Action non-OK semantics → `ACTION_NON_OK_NO_ROLLBACK_CONFIRMED` — state already updated before action runs; non-OK return does not roll back; entry still runs; `transition_count++` regardless.
- **Additional Phase 12C confirmations:**
  - Guard return type → `GUARD_RETURNS_BOOL_ONLY_CONFIRMED` (Phase 12B signature unchanged).
  - **Evaluation order correction:** exit → **state update** → action → entry. Phase 12B had drafted exit → action → state update → entry; Phase 12C corrects to put state update before action to match the no-rollback policy.
  - Counter independence: `guard_rejected_count` and `no_transition_count` are independent; both may increment in a single dispatch when matching rows exist but all guards reject.
- **Status mapping (Phase 12D target):** OK = `ROBOTOS_CORE_OK`; no-match/guard-rejected = `ROBOTOS_CORE_OK` (audit counters); action failure = action's status (transition committed); invalid config = `ERR_INVALID_ARG` or `ERR_NULL`; uninit = `ERR_INVALID_STATE`.
- **Validated command set (unchanged):** `a / s / r / ? / x / v / L / d / T` (9 commands; hardware-evidence-backed).
- **`devkit_app_state` status:** Devkit-local; not promoted (scope-guard #11 re-affirmed); design reference only.
- **`T` status:** Adapter probe evidence (Phase 11E `CLOSED_WITH_HARDWARE_EVIDENCE`); not promoted to Framework sensor API.
- **Last runtime implementation phase:** Phase 11D (`2040bfb`).
- **Last hardware evidence phase:** Phase 11E (`10710b3`).
- **All scope guards preserved:** Zero source/config/script changes. All 12 UART TX scope-guard constraints from `PHASE_9EZ_CHECKPOINT.md §H` intact. `core/`, `platform/`, `devkit/src/`, `tests/`, board DTS, board defconfig, Zephyr workspace, `DEVKIT_PROGRESS.md` historical master, and all evidence logs zero-diff. Pre-existing `src/framework/*.c` unmodified.
- **Remaining decisions (all preserved unchanged at Phase 12C):**
  1. ACTIVE disarm widening — `USER_DECISION_REQUIRED_ACTIVE_DISARM`
  2. Scheduler 7A/7B — `DEFER`
  3. F407 / custom board — `HOLD/DEFER`
  4. POST_FLASH_AUTOSTART root cause — `OPEN` / `MITIGATED_BY_WORKFLOW`
  5. Application / product layer — `NOT_STARTED`
  6. Robot Framework implementation — `NOT_STARTED`; Phase 12D not started
- **Next gate:** Phase 12D — Framework FSM Header Stub / Compile-only Skeleton. Entry requires **explicit user authorization** (first Framework-layer source file) and user direction on the exact Framework layer path (e.g., `RobotOS_v1.0/framework/`). Non-goals: no `.c` body, no `devkit_app_state` replacement, no UART command integration, no hardware run, no scheduler change. **Hold** is fully acceptable.

---

### Phase 12B — Robot Framework FSM API Draft (docs-only)

- **Date:** 2026-05-12
- **Type:** Docs-only API surface draft. No source, runtime, test, CMake, Zephyr, board, `prj.conf`, DTS overlay, evidence log, `framework/` directory, or `.h`/`.c` Framework file change.
- **Close status:** `CLOSED_DOCS_ONLY`
- **Decision result:** `PHASE_12B_FSM_API_DRAFTED_DOCS_ONLY`
- **Published baseline at open:** `origin/master = 76cb241`
- **Closeout doc:** `RobotOS_v1.0/devkit/docs/02_PHASE_CLOSEOUTS/PHASE_12B_FRAMEWORK_FSM_API_DRAFT.md`
- **Long-lived spec draft:** `RobotOS_v1.0/devkit/docs/03_SPECS/FRAMEWORK_FSM_API_DRAFT.md` (DRAFT / NON-FINAL; evolves through Phase 12C+)
- **Phase log entry:** `RobotOS_v1.0/devkit/docs/01_PROGRESS/DEVKIT_PROGRESS_PHASE_11_20.md` `<a id="phase-12b"></a>`
- **Robot Framework status:** **NOT BUILT.** No `framework/` directory, no Framework header, no Framework implementation. Phase 12B defines the API surface at design level only.
- **FSM model (confirmed at Phase 12B):** Flat, table-driven (`const robotos_fw_transition_t[]`), static compile-time config, no heap, product-defined `uint32_t` state/event IDs, first-match FIFO evaluation, thread-context dispatch, ISR-safe state query.
- **Draft API surface (NON-FINAL):** `robotos_fw_fsm_init`, `robotos_fw_fsm_dispatch`, `robotos_fw_fsm_get_state`, `robotos_fw_fsm_reset`, `robotos_fw_fsm_is_in_state`, `robotos_fw_fsm_get_snapshot` — all DRAFT; no header file created.
- **Event namespace:** Framework `robotos_fw_event_id_t` is decoupled from `robotos_event_type_t`; no sub-range allocation needed. Devkit uses types 100–103; Framework FSM event IDs are application-defined `uint32_t` in a separate namespace.
- **Open decisions (Phase 12C must confirm before any implementation):**
  1. Event bridge pattern — how Application layer translates Adapter events to `robotos_fw_event_id_t` calls; recommendation documented (`OPEN_RECOMMENDATION_PENDING_CONFIRMATION`).
  2. Status model — reuse `robotos_core_status_t` (Option A, recommended) vs new `robotos_fw_status_t` (`OPEN_RECOMMENDATION_PENDING_CONFIRMATION`).
  3. Event payload lifetime rules — documented; confirmation needed.
  4. Action return semantics on non-OK — documented (no rollback policy); confirmation needed.
- **`devkit_app_state` status:** Devkit-local; not promoted (scope-guard #11 re-affirmed). Used as design reference only; IDLE/ARMED/ACTIVE vocabulary does NOT appear in Framework API.
- **Validated command set (unchanged):** `a / s / r / ? / x / v / L / d / T` (9 commands; hardware-evidence-backed).
- **Last runtime implementation phase:** Phase 11D (`2040bfb`).
- **Last hardware evidence phase:** Phase 11E (`10710b3`; `CLOSED_WITH_HARDWARE_EVIDENCE`).
- **All scope guards preserved:** Zero source/config/script changes. All 12 UART TX scope-guard constraints from `PHASE_9EZ_CHECKPOINT.md §H` intact. `core/`, `platform/`, `devkit/src/`, board DTS, board defconfig, Zephyr workspace, `DEVKIT_PROGRESS.md` historical master, and all evidence logs zero-diff.
- **Remaining decisions (all preserved unchanged at Phase 12B):**
  1. ACTIVE disarm widening — `USER_DECISION_REQUIRED_ACTIVE_DISARM`
  2. Scheduler 7A/7B — `DEFER`
  3. F407 / custom board — `HOLD/DEFER`
  4. POST_FLASH_AUTOSTART root cause — `OPEN` / `MITIGATED_BY_WORKFLOW`
  5. Application / product layer — `NOT_STARTED`
  6. Robot Framework (implementation) — `NOT_STARTED`
- **Next gate:** Phase 12C — Framework FSM Event Bridge Spec + Status Model Confirmation (docs-only). Confirms open decisions before any header/implementation work. Requires explicit user authorization.

---

### Phase 12A — Robot Framework API Surface Planning (docs-only)

- **Date:** 2026-05-12
- **Type:** Docs-only architecture gate / Framework API surface planning. No source, runtime, test, CMake, Zephyr, board, `prj.conf`, DTS overlay, evidence log, or `framework/` directory change.
- **Close status:** `CLOSED_DOCS_ONLY`
- **Published baseline at open:** `origin/master = c239466`
- **Closeout doc:** `RobotOS_v1.0/devkit/docs/02_PHASE_CLOSEOUTS/PHASE_12A_FRAMEWORK_API_SURFACE_PLANNING.md`
- **Phase log entry:** `RobotOS_v1.0/devkit/docs/01_PROGRESS/DEVKIT_PROGRESS_PHASE_11_20.md` `<a id="phase-12a"></a>`
- **Robot Framework status:** **Not built.** No `framework/` directory, no Framework header, no Framework API. Phase 12A defines the planning boundary only.
- **Application / product layer:** **Not built.** No product vocabulary, no use-case selection.
- **Layer boundary confirmed:** `devkit_app_state` is NOT Robot Framework (scope-guard #11 re-affirmed). `a/s/r/?/x/v/L/d/T` are devkit probe commands, not product commands. `T` is an Adapter probe evidence command, not a Framework sensor API.
- **Adapter evidence inventory (all 11 classes proven):** time/tick, thread-context boundary, critical section, ISR-safe event post, queue/dispatch/budget, GPIO input, GPIO output, UART RX/TX, RTT telemetry, timer events, driver-dependent sensor read (I2C/`struct sensor_value`; Phase 11D/11E).
- **Candidate Framework domains evaluated (9):** state-machine abstraction, timer/service abstraction, sensor abstraction, fault/safety abstraction, Framework observability hooks, actuator (stepper/servo/DC), endstop/limit-switch, PID/control-loop, motion primitive/trajectory.
- **Recommended first Framework slice:** Framework State-Machine Abstraction (flat FSM only). Rationale: most Adapter evidence reuse; no new hardware; no Scheduler change needed; no product vocabulary risk; `devkit_app_state.c` is a validated design reference (not promoted).
- **Validated command set (unchanged):** `a / s / r / ? / x / v / L / d / T` (9 commands; all hardware-evidence-backed).
- **Last runtime implementation phase:** Phase 11D (`2040bfb`; `feat: add Phase 11D accelerometer probe command`).
- **Last hardware evidence phase:** Phase 11E (`10710b3`; `CLOSED_WITH_HARDWARE_EVIDENCE`; `OPERATOR_PHYSICAL_SANITY_CONFIRMED`).
- **Remaining decisions (all preserved unchanged at Phase 12A):**
  1. ACTIVE disarm widening — `USER_DECISION_REQUIRED_ACTIVE_DISARM`
  2. Scheduler 7A/7B — `DEFER`
  3. F407 / custom board — `HOLD/DEFER`
  4. POST_FLASH_AUTOSTART root cause — `OPEN` / `MITIGATED_BY_WORKFLOW`
  5. Application / product layer — `NOT_STARTED`
- **All scope guards preserved:** Zero source/config/script changes in Phase 12A. All 12 UART TX scope-guard constraints from `PHASE_9EZ_CHECKPOINT.md §H` intact. `core/`, `platform/`, `devkit/src/`, `tests/`, board DTS, board defconfig, Zephyr workspace tracked files, `DEVKIT_PROGRESS.md` historical master, and all evidence logs zero-diff. Phase 11D `devkit/prj.conf` delta (`CONFIG_I2C=y` + `CONFIG_SENSOR=y`) unchanged.
- **Next gate:** Phase 12B — Robot Framework FSM API Draft (docs-only). Entry requires explicit user authorization. Non-goals: no Framework implementation, no `framework/` dir, no source change, no `devkit_app_state` promotion.

---

### Phase 11Z — Command-Set Checkpoint (docs-only)

- **Date:** 2026-05-12
- **Type:** Docs-only checkpoint / design-state consolidation. No source, runtime, test, CMake, Zephyr, board, host-tool, script, or `prj.conf` change. Snapshots the validated command surface after the Phase 11A–11E sensor-probe track closed; prevents blind opening of the next implementation phase.
- **Close status:** `CLOSED_DOCS_ONLY`
- **Published baseline at open:** `origin/master = 10710b3`
- **Closeout doc:** `RobotOS_v1.0/devkit/docs/02_PHASE_CLOSEOUTS/PHASE_11Z_COMMAND_SET_CHECKPOINT.md`
- **Phase log entry:** `RobotOS_v1.0/devkit/docs/01_PROGRESS/DEVKIT_PROGRESS_PHASE_11_20.md` `<a id="phase-11z"></a>`
- **Companion doc:** `RobotOS_v1.0/devkit/docs/03_SPECS/COMMAND_SET_DRAFT.md` (status preamble updated to Phase 11Z checkpoint; Section B intro updated; no semantic change to any command row).
- **Validated command set at checkpoint:** **`a / s / r / ? / x / v / L / d / T`** (9 single-byte commands; all hardware-evidence-backed; all fit the single-byte / fixed 96-byte stack buffer / no-parser / no-registry / no-framing / thread-context-TX pattern).
- **Last runtime implementation phase:** Phase 11D (firmware `2040bfb`).
- **Last hardware evidence phase:** Phase 11E (`10710b3`; `CLOSED_WITH_HARDWARE_EVIDENCE`).
- **Phase 11A–11E sensor-probe track:** Complete. `T` accelerometer probe is fully implemented, build-validated, hardware-validated, with `OPERATOR_PHYSICAL_SANITY_CONFIRMED`.
- **Remaining decisions (all preserved unchanged at Phase 11Z):**
  1. ACTIVE disarm widening — `USER_DECISION_REQUIRED_ACTIVE_DISARM` (`d` from ACTIVE = recognized no-op).
  2. Scheduler 7A/7B — `DEFER` (no workload evidence justifies reopening; Phase 11E `peak=2 dropped=0 herr=0`).
  3. F407 / custom board — `HOLD/DEFER` (no portability requirement surfaced).
  4. POST_FLASH_AUTOSTART root cause — `OPEN`, `MITIGATED_BY_WORKFLOW` via Phase 6O sidecar `reset run`.
  5. Robot Framework API planning — `NOT_STARTED`; possible future docs-only phase (analog of Phase 11A).
  6. Application / product layer — `NOT_STARTED`; requires product direction.
- **All scope guards preserved:** Zero source/config/script changes in Phase 11Z. All 12 UART TX scope-guard constraints from `PHASE_9EZ_CHECKPOINT.md §H` intact. `core/`, `platform/`, `tests/`, `devkit/CMakeLists.txt` top-level, `devkit_runtime.{c,h}`, `devkit_status_led.{c,h}`, `devkit_button_producer.c`, `devkit_timer_producer.c`, `devkit_observability.c`, `devkit_build_info.c`, `devkit_fault.c`, board DTS, board defconfig, B-revision overlay, Zephyr workspace tracked files, `DEVKIT_PROGRESS.md` historical master, and all evidence logs zero-diff. Scheduler 7A/7B remains DEFER. F407 remains HOLD/DEFER. UART TX remains minimal response only. POST_FLASH_AUTOSTART discipline unchanged. ACTIVE disarm widening `USER_DECISION_REQUIRED_ACTIVE_DISARM` preserved and decoupled. The **single** intentional change since Phase 9E baseline remains the Phase 11D delta in `devkit/prj.conf` adding `CONFIG_I2C=y` + `CONFIG_SENSOR=y` (no SPI, no ADC, no CBPRINTF_FP_SUPPORT, no overlay); Phase 11Z does not modify this delta.
- **Verdict:** Docs-only close. No firmware change, no test change, no scope expansion, no semantics change, no purchase authorization, no new command candidate. Stable shelf before any future Framework, scheduler, F407, or housekeeping work.
- **Next gate:** **Hold.** No phase opens automatically. Future possible phases — each requires explicit user authorization: (1) Phase 12A-class docs-only Framework API surface planning (recommended next planning step if advancing into Framework territory); (2) ACTIVE disarm widening (cheap one-line + supplemental run); (3) Adapter SPI/ADC probe (only if a specific workload motivates it); (4) POST_FLASH_AUTOSTART root cause investigation (optional engineering work); (5) Scheduler 7A/7B (do not reopen without new workload evidence); (6) F407/custom board (do not reopen without explicit portability requirement); (7) Application/product layer (requires product direction). Phase 11Z authorizes none of these.

---

### Phase 11E — On-board MEMS Accelerometer Probe Evidence Closeout (hardware-validated)

- **Date:** 2026-05-12
- **Type:** Hardware evidence-only closeout. No source, runtime, test, CMake, Zephyr, board, `prj.conf`, DTS overlay, or script change.
- **Close status:** `CLOSED_WITH_HARDWARE_EVIDENCE`
- **Implementation commit tested:** `2040bfb` (Phase 11D)
- **Closeout doc:** `RobotOS_v1.0/devkit/docs/02_PHASE_CLOSEOUTS/PHASE_11E_ACCEL_PROBE_EVIDENCE.md`
- **Phase log entry:** `RobotOS_v1.0/devkit/docs/01_PROGRESS/DEVKIT_PROGRESS_PHASE_11_20.md` `<a id="phase-11e"></a>`
- **Hardware:** STM32F411E-DISCO revision D; LSM303AGR (`lsm303agr_accel`); `lis2dh` driver; I2C1/0x19; COM5 @ 115200 8N1; ST-LINK USB; PA2 TX / PA3 RX.
- **Build:** Pristine `west build -b stm32f411e_disco` at `2040bfb`; FLASH 41528 B / RAM 12352 B; 161/161 PASS; byte-identical to Phase 11D build. `CONFIG_I2C=y`, `CONFIG_SENSOR=y`, `CONFIG_LIS2DH=y`, `CONFIG_LIS2DH_TRIGGER_NONE=y`; `CONFIG_SPI`/`CONFIG_ADC`/`CONFIG_CBPRINTF_FP` absent.
- **Flash:** `west flash`; 49152 bytes; PASS. POST_FLASH_AUTOSTART: sidecar `reset run` via `capture_devkit_rtt.ps1`; `_SEGGER_RTT` at `0x20000ad0`.
- **Host transcript:** Sequence `T T ?`; transcript saved to `RobotOS_v1.0/devkit/logs/phase_11E_accel_probe_host_2026-05-12.txt` (388 B).
- **Evidence — T responses:**
  - 1st `T` → `ACC x=-0.561300 y=2.619400 z=9.167900` (len=39 B; matches frozen shape; `PHASE_11C_FORMAT_SIGN_EDGE` observed and correct)
  - 2nd `T` → `ACC x=-0.598720 y=2.507140 z=9.167900` (len=39 B; matches frozen shape; minor X/Y variation = live sensor)
  - `?` → `STATE state=IDLE transitions=0 button=0 uart=3 ignored=0` (shape unchanged)
- **RTT log:** `RobotOS_v1.0/devkit/logs/phase_11E_accel_probe_rtt_2026-05-12.txt` (22334 B; 61.4 s; Phase 6O harness). 6/6 required patterns FOUND; `CFSR=0x00000000` and `HFSR=0x00000000` (13× each).
- **Final RTT counters (ticks=120):** `ROBOTOS_UART rx=3 ok=3 handled=3 last=0x3f`; `ROBOTOS_APP state=IDLE transitions=0 button=0 uart=3 ignored=0`; `ROBOTOS_OBS accepted=63 dispatched=62 pending=1 peak=2 dropped=0 herr=0`; `ROBOTOS_PROD attempted=60 ok=60`.
- **Invariants (all PASS):** `accepted−dispatched=pending` (63−62=1) ✓; `PROD ok + UART ok = accepted` (60+3=63) ✓; `UART rx=handled=APP uart=3` ✓; `transitions/ignored delta=0` ✓; `CFSR|HFSR=0` ✓.
- **Physical sanity:** **`OPERATOR_PHYSICAL_SANITY_CONFIRMED`** — Z ≈ 9.17 m/s² flat-bench (close to 9.81 m/s² gravity; uncalibrated). No calibration claimed.
- **`T` command status:** **Hardware-evidence-backed.** Promoted to Section A in `COMMAND_SET_DRAFT.md`. Phase 11A–11E sensor probe track complete.
- **All scope guards preserved:** Zero source/config/script changes in Phase 11E. All 12 UART TX scope-guard constraints from `PHASE_9EZ_CHECKPOINT.md §H` intact. `core/`, `platform/`, `prj.conf`, `CMakeLists.txt`, board DTS, and all evidence logs (except newly added Phase 11E logs) zero-diff. Scheduler 7A/7B remains DEFER. F407 remains HOLD/DEFER. UART TX remains minimal response only. POST_FLASH_AUTOSTART discipline unchanged. ACTIVE disarm widening `USER_DECISION_REQUIRED_ACTIVE_DISARM` preserved and decoupled.
- **Verdict:** `CLOSED_WITH_HARDWARE_EVIDENCE`. The `T` accelerometer probe command is fully validated end-to-end on real hardware. ACC success path observed; frozen response shape confirmed; counter invariants confirmed; CFSR/HFSR clean; physical sanity confirmed.
- **Next gate:** User decision. Phase 11A–11E sensor track is complete. Remaining open items: ACTIVE disarm widening (`USER_DECISION_REQUIRED_ACTIVE_DISARM`); Scheduler 7A/7B (`DEFER`); F407/custom-board (`HOLD/DEFER`); POST_FLASH_AUTOSTART root cause (`OPEN`, `MITIGATED_BY_WORKFLOW`).

---

### Phase 11D — On-board MEMS Accelerometer Probe Implementation (firmware, build-validated)

- **Date:** 2026-05-12
- **Type:** Devkit-local firmware + tooling implementation of the Phase 11C-frozen on-board MEMS accelerometer probe (`T` command). Source + `prj.conf` + host harness changes only. No `core/`, no `platform/`, no `tests/`, no scheduler/queue constants, no board DTS, no DTS overlay, no Zephyr module, no hardware purchase.
- **Close status:** `IMPLEMENTATION_CLOSED_HARDWARE_EVIDENCE_PENDING` — build-validated pristine for `stm32f411e_disco`; hardware evidence deferred to Phase 11E.
- **Published baseline at open:** `origin/master = cc101cd`
- **Closeout doc:** `RobotOS_v1.0/devkit/docs/02_PHASE_CLOSEOUTS/PHASE_11D_ACCEL_PROBE_IMPLEMENTATION.md`
- **Phase log entry:** `RobotOS_v1.0/devkit/docs/01_PROGRESS/DEVKIT_PROGRESS_PHASE_11_20.md` `<a id="phase-11d"></a>`
- **Companion docs:** `COMMAND_SET_DRAFT.md` Section B intro and `T` row updated to mark `T` implemented-build-validated, hardware evidence pending Phase 11E; status preamble updated to Phase 11D checkpoint.
- **Source change:** `devkit/src/devkit_app_state.c` (+12 lines, `case 't':` recognition — LOG_INF, no transition, not ignored), `devkit/src/devkit_app_state.h` (+5/-1 lines, doc-only), and `devkit/src/devkit_uart_producer.c` (+88/-2 lines — new includes `<errno.h>`, `<stdlib.h>`, `<zephyr/drivers/sensor.h>`; new `DEVKIT_ACCEL_NODE = DT_ALIAS(accel0)` with `#error` guard; new `s_accel_dev = DEVICE_DT_GET(...)`; new internal `devkit_accel_format_value()` helper; new `case 't':` TX branch — `sensor_sample_fetch` + `sensor_channel_get` + frozen `ACC`/`ERR` response). Plus new host harness `tools/runtime/run_phase11d_accel_probe_demo.ps1` (Phase 6O sidecar-`reset-run` discipline, default sequence `T T ?`, PowerShell-parse OK).
- **Config change:** `devkit/prj.conf` (+9/-1 lines) — appended `CONFIG_I2C=y` and `CONFIG_SENSOR=y` with leading comment block. **No other Kconfig added.** `CONFIG_LIS2DH_TRIGGER_NONE=y` not added explicitly — pristine `.config` already sets it as the driver default (first option of `choice LIS2DH_TRIGGER_MODE`).
- **Generated `.config` observations:** `CONFIG_I2C=y`, `CONFIG_SENSOR=y`, `CONFIG_LIS2DH=y`, `CONFIG_LIS2DH_TRIGGER_NONE=y`, `CONFIG_LIS2DH_OPER_MODE_NORMAL=y`, `CONFIG_LIS2DH_ACCEL_RANGE_RUNTIME=y`, `CONFIG_LIS2DH_ODR_RUNTIME=y` all set (driver defaults). **Forbidden Kconfigs absent:** `CONFIG_SPI` not set, `CONFIG_ADC` not set, `CONFIG_CBPRINTF_FP_SUPPORT` not set.
- **Response/error implementation:** Success line `ACC x=<v1>.<v2_6d> y=<v1>.<v2_6d> z=<v1>.<v2_6d>\r\n` (worst case 68 B, typical ~44 B); error line `ERR accel read=<errno>\r\n` (worst case 28 B; errno is numeric signed-decimal from failing Zephyr API; no symbolic mapping; no retry). Both paths share the existing 96-byte stack buffer via the existing `uart_poll_out` path. **Sign rule** `PHASE_11C_FORMAT_SIGN_EDGE`: for the Zephyr edge case `val1=0, val2<0` (sub-unit negative), implementation emits a leading `-` and absolute val2, e.g. `-0.500000`, staying within the frozen `<v1>.<v2_6d>` shape. Documented in closeout doc §D.5.
- **Build delta vs Phase 10B-d baseline `125779c`:** FLASH 36780 B → 41528 B (+4748 B); RAM 12224 B → 12352 B (+128 B). Cost dominated by Zephyr sensor subsystem + STM32 I2C driver + `lis2dh` driver. Comfortably within `stm32f411e_disco` budget; no fault-stack adjustment needed. No new warnings.
- **`T` behavioral contract realized:** Recognition in `devkit_app_state.c`; `transitions / ignored / button` unchanged; `uart` +1 per `T`; read in thread/handler context only (no ISR TX); single attempt; no retry; no symbolic errno; handler returns OK in both success and sensor-error paths. Other commands' branches (`v / L / d / a / s / r / ? / x`) byte-for-byte unchanged.
- **Hardware evidence status:** **`HARDWARE_EVIDENCE_PENDING_PHASE_11E`.** Board not flashed; RTT not captured; harness not run by Phase 11D. The `T T ?` host harness exists, PowerShell-parses OK, and is ready for Phase 11E.
- **All scope guards preserved:** All 12 UART TX scope-guard constraints from `PHASE_9EZ_CHECKPOINT.md §H` intact. `core/`, `platform/`, `tests/`, `devkit/CMakeLists.txt` top-level, `devkit_runtime.{c,h}`, `devkit_status_led.{c,h}`, `devkit_button_producer.c`, `devkit_timer_producer.c`, `devkit_observability.c`, `devkit_build_info.c`, `devkit_fault.c`, board DTS, board defconfig, B-revision overlay, Zephyr workspace tracked files, and all evidence logs zero-diff. Scheduler 7A/7B remains DEFER. F407 remains HOLD/DEFER. UART TX remains minimal response only. POST_FLASH_AUTOSTART discipline unchanged. ACTIVE disarm widening `USER_DECISION_REQUIRED_ACTIVE_DISARM` preserved and decoupled.
- **Verdict:** Build-validated close. `T` is implemented per the Phase 11C-frozen contract; build passes pristine for `stm32f411e_disco`; generated `.config` matches the Phase 11C §D rules. **Hardware-runtime PASS / FAIL is a Phase 11E decision; Phase 11D does NOT claim runtime evidence.**
- **Next gate:** **Phase 11E — Accelerometer Probe Evidence Closeout.** Hardware evidence-only. Reserved; not opened by Phase 11D. Phase 11E opening requires **explicit user authorization**. Phase 11E must flash the Phase 11D image, run `run_phase11d_accel_probe_demo.ps1`, capture RTT + host transcript under `RobotOS_v1.0/devkit/logs/phase_11D_*_<date>.txt`, verify §I.1/§I.2 counter expectations from Phase 11C, optionally record `OPERATOR_PHYSICAL_SANITY_CONFIRMED` for ~+9.8 m/s² Z reading with board flat, and publish a Phase 11E closeout with status `CLOSED_WITH_HARDWARE_EVIDENCE`.

---

### Phase 11C — On-board MEMS Accelerometer Probe Spec (docs-only)

- **Date:** 2026-05-12
- **Type:** Docs-only specification freeze. No source, runtime, test, CMake, Zephyr, board, `prj.conf`, DTS overlay, host-tool, or script change. Freezes the on-board MEMS accelerometer probe spec for future Phase 11D implementation. Operationalizes Phase 11B's `FEASIBILITY_CONFIRMED_ONBOARD_MEMS` plus the operator-confirmed board revision (D).
- **Close status:** `CLOSED_DOCS_ONLY`
- **Published baseline at open:** `origin/master = 2aa0435`
- **Closeout doc:** `RobotOS_v1.0/devkit/docs/02_PHASE_CLOSEOUTS/PHASE_11C_ACCEL_PROBE_SPEC.md`
- **Phase log entry:** `RobotOS_v1.0/devkit/docs/01_PROGRESS/DEVKIT_PROGRESS_PHASE_11_20.md` `<a id="phase-11c"></a>`
- **Companion docs:** `COMMAND_SET_DRAFT.md` Section B intro and `T` row updated with Phase 11C cross-reference and frozen response/error shapes; status preamble updated to Phase 11C checkpoint; no command semantics changed (T not promoted).
- **Decision result:** **`PHASE_11C_SPEC_FROZEN_ACCEL_LIS2DH_REV_D`**
- **Target device freeze:** Board target `stm32f411e_disco` (no `@<rev>`; revision D Zephyr default); board revision **D** (operator-confirmed, `CONFIRMED_A_OR_D`); DT alias `accel0` → `lsm303agr_accel`; driver `lis2dh` (self-contained, no `hal_st`); bus/address I2C1 @ 0x19 (SCL=PB6, SDA=PB9, 400 kHz); channel `SENSOR_CHAN_ACCEL_XYZ` (raw `struct sensor_value`); trigger mode polling / `CONFIG_LIS2DH_TRIGGER_NONE` (driver default); interrupt GPIOs PE4/PE5 reserved but not used; overlay none; wiring none; hardware purchase none.
- **Frozen success response:** `ACC x=<v1>.<v2_6d> y=<v1>.<v2_6d> z=<v1>.<v2_6d>\r\n` (worst case 68 B, typical ~44 B; fits 96-byte stack buffer with 28 B headroom). Raw `sensor_value.val1` signed decimal, `val2` 6-digit absolute fractional (sign carried by `val1`); no floating point.
- **Frozen error response:** `ERR accel read=<errno>\r\n` (worst case 28 B). Errno is numeric signed-decimal return value from failing Zephyr API; no symbolic mapping.
- **Frozen canonical host harness sequence:** `T T ?` (three commands; two ACC responses + one STATE response; verifies command path, sensor read repeatability, and app-state invariants).
- **Expected counter behavior for `T T ?` from IDLE:** `ROBOTOS_UART rx=ok=handled` +3; `ROBOTOS_APP uart` +3; `transitions`, `ignored`, `button` unchanged; `dropped=0`; `herr=0`; CFSR/HFSR=0; `accepted - dispatched = pending = 1` invariant holds. `peak` not overclaimed; Phase 11E must record actual.
- **Future Phase 11D config implication (frozen):** **MUST add** `CONFIG_I2C=y` + `CONFIG_SENSOR=y`. Optionally `CONFIG_LIS2DH_TRIGGER_NONE=y` only if pristine `.config` does not already set it. **MUST NOT add** `CONFIG_SPI`, `CONFIG_ADC`, `CONFIG_CBPRINTF_FP_SUPPORT`, `CONFIG_LIS2DH_*` range/ODR/HP overrides. **MUST NOT** create DTS overlay.
- **`T` status:** **Not implemented.** Remains `USER_DECISION_REQUIRED` in `COMMAND_SET_DRAFT.md` Section B with Phase 11C cross-reference. Phase 11D (Implementation) and Phase 11E (Evidence Closeout) remain `NOT_STARTED` and require **explicit user authorization** to open.
- **All scope guards preserved:** `core/`, `platform/`, `devkit/src/`, `prj.conf`, `CMakeLists.txt`, `boards/`, `zephyr/`, `tests/`, runtime scripts, host tools, and evidence logs all zero-diff. All 12 UART TX scope-guard constraints from `PHASE_9EZ_CHECKPOINT.md §H` intact. Scheduler 7A/7B remains DEFER. F407 remains HOLD/DEFER. UART TX remains minimal response only. POST_FLASH_AUTOSTART discipline unchanged. ACTIVE disarm widening `USER_DECISION_REQUIRED_ACTIVE_DISARM` preserved and decoupled from Phase 11A-E sensor track.
- **Verdict:** Docs-only close. No firmware change, no test change, no `prj.conf` change, no overlay, no hardware purchase, no scope expansion, no semantics change. Spec frozen for downstream consumption.
- **Next gate:** **Phase 11D — Sensor Probe Implementation (firmware).** Reserved; not opened by Phase 11C. Phase 11D opening requires **explicit user authorization** for source / `prj.conf` changes. Phase 11E (Evidence Closeout) follows Phase 11D and is also conditional. See `PHASE_11C_ACCEL_PROBE_SPEC.md` §J for the implementation boundary (devkit-local files only; no `core/`, no `platform/`, no overlay).

---

### Phase 11B — Device / Driver Feasibility Gate (docs-only / audit)

- **Date:** 2026-05-12
- **Type:** Docs-only feasibility audit / purchase gate. No source, runtime, test, CMake, Zephyr, board, `prj.conf`, DTS overlay, host-tool, or script change. Audits local Zephyr workspace to determine the safest feasible target for the future Phase 11C/11D bounded Adapter sensor-read probe.
- **Close status:** `CLOSED_DOCS_ONLY`
- **Published baseline at open:** `origin/master = bae2436`
- **Closeout doc:** `RobotOS_v1.0/devkit/docs/02_PHASE_CLOSEOUTS/PHASE_11B_DEVICE_DRIVER_FEASIBILITY.md`
- **Phase log entry:** `RobotOS_v1.0/devkit/docs/01_PROGRESS/DEVKIT_PROGRESS_PHASE_11_20.md` `<a id="phase-11b"></a>`
- **Companion docs:** `COMMAND_SET_DRAFT.md` Section B intro and `T` row updated with Phase 11B cross-reference; status preamble updated to Phase 11B checkpoint; no command semantics changed.
- **Feasibility decision:** **`FEASIBILITY_CONFIRMED_ONBOARD_MEMS`** — the on-board LSM303AGR accelerometer (`lsm303agr_accel` node, Zephyr driver `lis2dh`, I2C1 at 0x19, PB6/PB9, 400 kHz) is confirmed as the recommended target. Sensor node already defined with `status = "okay"` in upstream Zephyr board DTS. Driver locally present and self-contained (no `hal_st` dependency; handles both board revision A/D and B via compatible "st,lis2dh"). No DTS overlay needed. No hardware purchase needed. Required `prj.conf` additions for Phase 11D: `CONFIG_I2C=y` + `CONFIG_SENSOR=y` (both currently absent from `devkit/prj.conf`; an earlier baseline claim that `CONFIG_I2C=y` was pre-existing was incorrect and has been corrected docs-only at HEAD).
- **Priority correction:** Phase 11A listed die temperature as priority (a) and on-board MEMS as (b). Phase 11B **corrects this**: on-board MEMS accelerometer is simpler (no `CONFIG_ADC`, no DTS overlay); die temperature requires enabling `adc1`, adding a compatible string to the `die_temp` DTS node, and `CONFIG_ADC=y`. Magnetometer (`lis2mdl`) blocked — `hal_st` module not in workspace.
- **Purchase decision:** **`NO_PURCHASE_NEEDED_FOR_NEXT_STEP`**. No purchase authorized or recommended.
- **Board revision note:** User must confirm board revision (A/D vs B) before Phase 11C spec freezes the exact DTS alias target. Both revisions use "st,lis2dh" compatible — `lis2dh` driver handles both.
- **`T` status:** **Not implemented.** Remains `USER_DECISION_REQUIRED` in `COMMAND_SET_DRAFT.md` Section B with Phase 11B cross-reference. Phase 11C (Probe Spec, docs-only) is the next gate; no implementation before Phase 11C is approved.
- **All scope guards preserved:** `core/`, `platform/`, `devkit/src/`, `prj.conf`, `CMakeLists.txt`, `boards/`, `zephyr/`, `tests/`, runtime scripts, host tools, and evidence logs all zero-diff. All 12 UART TX scope-guard constraints from `PHASE_9EZ_CHECKPOINT.md §H` intact. Scheduler 7A/7B remains DEFER. F407 remains HOLD/DEFER. UART TX remains minimal response only. POST_FLASH_AUTOSTART discipline unchanged. ACTIVE disarm widening `USER_DECISION_REQUIRED_ACTIVE_DISARM` preserved.
- **Verdict:** Docs-only close. No firmware change, no test change, no scope expansion, no semantics change, no purchase authorization.
- **Next gate:** **Phase 11C — Sensor Probe Spec (docs-only).** Must freeze before any Phase 11D code: exact DTS alias target (after operator confirms board revision A/D vs B), trigger mode (`LIS2DH_TRIGGER_NONE`), read channel (`SENSOR_CHAN_ACCEL_XYZ`), response format (raw `val1`/`val2`, no floating point, ≤96 bytes), error response, host harness sequence, expected RTT counters, `prj.conf` additions (`CONFIG_I2C=y` + `CONFIG_SENSOR=y`). See `PHASE_11B_DEVICE_DRIVER_FEASIBILITY.md §H` for full spec requirements.

---

### Phase 11A — Adapter Boundary & Sensor Surface Decision (docs-only)

- **Date:** 2026-05-12
- **Type:** Docs-only architecture gate / boundary decision. No source, runtime, test, CMake, Zephyr, board, `prj.conf`, DTS overlay, host-tool, or script change. First Phase 11–20 architecture gate.
- **Close status:** `CLOSED_DOCS_ONLY`
- **Published baseline at open:** `origin/master = 7ce8cb7`
- **Closeout doc:** `RobotOS_v1.0/devkit/docs/02_PHASE_CLOSEOUTS/PHASE_11A_ADAPTER_BOUNDARY_SENSOR_SURFACE.md`
- **Phase log entry:** `RobotOS_v1.0/devkit/docs/01_PROGRESS/DEVKIT_PROGRESS_PHASE_11_20.md` `<a id="phase-11a"></a>`
- **Companion docs:** `RobotOS_v1.0/devkit/docs/03_SPECS/COMMAND_SET_DRAFT.md` (Section B intro and `T` row updated with Phase 11A cross-reference; status preamble updated to Phase 11A checkpoint; no command semantics changed).
- **Adapter API surface inventory at checkpoint (concept-level):** Eleven primitive classes proven on hardware — time, thread-context boundary, critical section, ISR-safe event posting, queue/dispatch/budget, GPIO input as event source, GPIO output, UART RX, UART TX, RTT trace/telemetry/fault, timer-generated events. **Largest uncharacterized primitive class: driver-dependent read** (and its sub-classes I²C, SPI, ADC). Pool/slab is flagged as an open architectural question; portability backend is claimed but undemonstrated (only Zephyr/STM32F4 exercised).
- **Sensor surface classification decision:** **`SENSOR_SURFACE_DECIDED_ADAPTER_PROBE`** — `T` is classified as a future bounded Adapter probe candidate only. **NOT implementation approval, NOT hardware purchase approval, NOT Framework/Application promotion.** The probe (if later authorized) must return Adapter-level raw values; calibration / units / sensor identity in any rich-typed sense is explicitly Framework-class and not approved by Phase 11A.
- **`T` status:** **Not implemented.** Remains `USER_DECISION_REQUIRED` in `COMMAND_SET_DRAFT.md` Section B with Phase 11A cross-reference. Phase 11B (Device / Driver Feasibility) is the next gate; no hardware purchase authorized by Phase 11A.
- **ACTIVE disarm widening:** **`USER_DECISION_REQUIRED_ACTIVE_DISARM` preserved.** Current `d` from ACTIVE = recognized no-op preserved. NOT part of Phase 11A; remains a separate small vocabulary housekeeping gate that may be opened only on explicit user request, decoupled from the Phase 11A–11E sensor track.
- **Scope guards:** All 12 UART TX scope-guard constraints from `PHASE_9EZ_CHECKPOINT.md §H` intact. `core/`, `platform/`, `devkit/src/`, `prj.conf`, `CMakeLists.txt`, `boards/`, `zephyr/`, `tests/`, runtime scripts, host tools, evidence logs all zero-diff. Scheduler 7A/7B remains DEFER. F407 / custom board remains HOLD/DEFER. UART TX remains minimal response only. POST_FLASH_AUTOSTART discipline unchanged (root cause OPEN; mitigated-by-workflow via `capture_devkit_rtt.ps1` sidecar `reset run`; manual RESET fallback; plain `west flash` not runtime-start evidence).
- **Verdict:** Docs-only close. No firmware change, no test change, no scope expansion, no semantics change, no purchase authorization.
- **Next gate:** **Phase 11B — Device / Driver Feasibility Gate (docs-only / audit).** Verify existing STM32F411E-DISCO resources first, in priority order: (a) STM32 internal die temp / ADC; (b) on-board MEMS peripherals if Zephyr DT/driver support is confirmed; (c) external I²C sensor module only if (a) and (b) are not viable. No hardware purchased in 11B. If 11B confirms a feasible part, Phase 11C (Probe Spec, docs-only) follows; Phase 11D (Implementation) and Phase 11E (Evidence Closeout) are conditional. F407 / portability remains HOLD/DEFER until Phase 15A or equivalent future portability gate.

---

### Phase 10C — Command-Set Checkpoint (docs-only)

- **Date:** 2026-05-11
- **Type:** Docs-only checkpoint / design-state consolidation. No source, runtime, test, CMake, Zephyr, board, host-tool, script, or `prj.conf` change. Snapshots the validated non-sensor command group after Phase 10B-d closes; prevents blind opening of `T` or ACTIVE disarm widening.
- **Close status:** `CLOSED_DOCS_ONLY`
- **Published baseline at open:** `origin/master = 7e250dc`
- **Closeout doc:** `RobotOS_v1.0/devkit/docs/02_PHASE_CLOSEOUTS/PHASE_10C_COMMAND_SET_CHECKPOINT.md`
- **Phase log entry:** `RobotOS_v1.0/devkit/docs/01_PROGRESS/DEVKIT_PROGRESS_PHASE_10.md` `<a id="phase-10c"></a>`
- **Companion docs:** `RobotOS_v1.0/devkit/docs/03_SPECS/COMMAND_SET_DRAFT.md` (header status updated; Section B intro clarified to track both the `T` row and the ACTIVE disarm widening as `USER_DECISION_REQUIRED`; no command semantics changed).
- **Validated non-sensor command set at checkpoint:** `a / s / r / ? / x / v / L / d` — all eight hardware-validated; all fit the single-byte / fixed 96-byte stack-buffer / no-parser / no-registry / no-framing / thread-context-TX pattern.
- **Runtime behavior baseline:** Unchanged — last runtime behavior phase is Phase 10B-d (firmware `125779c`, evidence-close `7e250dc`). Phase 10C adds no new runtime behavior and runs no new validation.
- **Remaining `USER_DECISION_REQUIRED`:** (1) `T` sensor read — largest open surface (sensor part, driver / `prj.conf` change, response format, error variant, fixed-buffer compliance — five open prerequisites); (2) ACTIVE disarm widening — `USER_DECISION_REQUIRED_ACTIVE_DISARM`; current behavior is recognized no-op (no transition, no `ignored++`).
- **Scope guards:** All 12 UART TX scope-guard constraints from `PHASE_9EZ_CHECKPOINT.md §H` intact. `core/`, `platform/`, `devkit_runtime.{c,h}`, `devkit_status_led.{h,c}`, `devkit_button.{c,h}`, `prj.conf`, `CMakeLists.txt`, `boards/`, `zephyr/`, `tests/`, and `DEVKIT_PROGRESS.md` all zero-diff at this checkpoint. Scheduler 7A/7B remains DEFER. F407 / custom board remains HOLD/DEFER. UART TX remains minimal response only. POST_FLASH_AUTOSTART discipline unchanged (root cause OPEN; mitigated-by-workflow via `capture_devkit_rtt.ps1` sidecar `reset run`; manual RESET fallback; plain `west flash` not runtime-start evidence).
- **Verdict:** Docs-only close. No firmware change, no test change, no scope expansion, no semantics change.
- **Next gate:** (a) hold, (b) decide ACTIVE disarm widening (cheap, one-line + supplemental run), (c) decide `T` prerequisites (expensive; five open questions — do not open blind), (d) do not reopen Scheduler 7A/7B or F407. Phase 10C itself authorizes none of these.

---

### Phase 10B-d — Explicit Disarm Command `d` (hardware evidence)

- **Date:** 2026-05-11
- **Type:** Third Phase 10B-class implementation; smallest remaining behavioral surface from `COMMAND_SET_DRAFT.md` Section B. Single-byte app-state command added to the proven Phase 9E/10B-v/10B-L UART RX/TX path. Devkit-local source changes only. No new driver, no `prj.conf` change, no physical effect, no state-machine redesign. Provides explicit user-vocabulary "disarm" distinct from `r` (`r` is preserved zero-diff and remains the canonical reset path).
- **Close status:** `CLOSED_WITH_HARDWARE_EVIDENCE`
- **Implementation commit:** `125779c` (`feat: add Phase 10B-d disarm command`)
- **Closeout doc:** `RobotOS_v1.0/devkit/docs/02_PHASE_CLOSEOUTS/PHASE_10B_D_CLOSE.md`
- **Phase log entry:** `RobotOS_v1.0/devkit/docs/01_PROGRESS/DEVKIT_PROGRESS_PHASE_10.md` `<a id="phase-10b-d"></a>`
- **Companion command spec update:** `RobotOS_v1.0/devkit/docs/03_SPECS/COMMAND_SET_DRAFT.md` (the `d` row was promoted from Section B DRAFT to Section A IMPLEMENTED).
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
- **Closeout doc:** `RobotOS_v1.0/devkit/docs/02_PHASE_CLOSEOUTS/PHASE_10B_L_CLOSE.md`
- **Phase log entry:** `RobotOS_v1.0/devkit/docs/01_PROGRESS/DEVKIT_PROGRESS_PHASE_10.md` `<a id="phase-10b-l"></a>`
- **Companion command spec update:** `RobotOS_v1.0/devkit/docs/03_SPECS/COMMAND_SET_DRAFT.md` (the `L` row was promoted from Section B DRAFT to Section A IMPLEMENTED).
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
- **Closeout doc:** `RobotOS_v1.0/devkit/docs/02_PHASE_CLOSEOUTS/PHASE_10B_V_CLOSE.md`
- **Phase log entry:** `RobotOS_v1.0/devkit/docs/01_PROGRESS/DEVKIT_PROGRESS_PHASE_10.md` `<a id="phase-10b-v"></a>`
- **Companion command spec update:** `RobotOS_v1.0/devkit/docs/03_SPECS/COMMAND_SET_DRAFT.md` (the `v` row was promoted from Section B DRAFT to Section A IMPLEMENTED).
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
- **Closeout doc:** `RobotOS_v1.0/devkit/docs/02_PHASE_CLOSEOUTS/PHASE_9G_CLOSE.md`
- **Phase log entry:** `RobotOS_v1.0/devkit/docs/01_PROGRESS/DEVKIT_PROGRESS_PHASE_10.md` `<a id="phase-9g-late"></a>` (non-linear `‡` insert per §4 rule 6 — late-9-series post-split)
- **Evidence:** `RobotOS_v1.0/devkit/logs/phase_9G_uart_burst_rtt_2026-05-11.txt` (22929 B, 60.7 s) + `RobotOS_v1.0/devkit/logs/phase_9G_uart_burst_host_2026-05-11.txt` (host transcript)
- **Verdict:** PASS. 5-byte burst (`a s ? r x` at 30 ms spacing, ~185 ms total) sent on COM5 to Phase 9E firmware; 5/5 responses arrived in send order; ROBOTOS_OBS `peak=5` (vs Phase 9E `peak=2`), `dropped=0`, `herr=0`, `unhandled=0`; ROBOTOS_UART `rx=ok=handled=5 full=0`; ROBOTOS_APP `transitions=3 uart=5 ignored=1`; Phase 6M producer healthy (`attempted=ok=60` at ticks=120); CFSR/HFSR `0x00000000` (13×); `accepted - dispatched = pending` invariant holds (65 − 64 = 1).
- **Runtime baseline:** Unchanged — still Phase 9E (`587dab7`). Phase 9G characterized the existing runtime; it added no firmware, no scheduler change, no UART TX scope change.
- **Next gate:** Phase 10A's gate remains in force — user must explicitly select one of (a) a `USER_DECISION_REQUIRED` row from `COMMAND_SET_DRAFT.md` §3, (b) Phase 9F demo polish, or (c) a continued hold. Phase 9G removes the highest-priority outstanding unknown ("high-rate UART input not stress-tested" from `PHASE_9EZ_CHECKPOINT.md §E`) but does not by itself authorize any Phase 10B opening.

---

### Phase 10A — Product Command Set Planning (docs-only)

- **Date:** 2026-05-11
- **Type:** Docs-only planning. No source, runtime, test, CMake, Zephyr, board, or script change.
- **Close status:** `CLOSED_DOCS_ONLY`
- **Closeout doc:** `RobotOS_v1.0/devkit/docs/02_PHASE_CLOSEOUTS/PHASE_10A_CLOSE.md`
- **Companion doc:** `RobotOS_v1.0/devkit/docs/03_SPECS/COMMAND_SET_DRAFT.md`
- **Phase log entry:** `RobotOS_v1.0/devkit/docs/01_PROGRESS/DEVKIT_PROGRESS_PHASE_10.md` §5 (`<a id="phase-10a"></a>`)
- **Purpose:** Capture product command vocabulary and workload intent in writing before any Phase 10B-class implementation is authorized.
- **Runtime baseline:** Unchanged — still ends at **Phase 9E-Z** (audit) / **Phase 9E** (implementation, commit `587dab7`). Phase 10A does not introduce or modify any runtime behavior.
- **Next gate:** User must explicitly select one of (a) a `USER_DECISION_REQUIRED` row from `COMMAND_SET_DRAFT.md` §3 with its open notes answered, (b) a direction-independent supporting phase (Phase 9F demo polish or Phase 9G UART burst characterization), or (c) a continued hold. **No Phase 10B implementation is authorized.**

---

### Phase 9E-Z — Command Loop Checkpoint / Direction Guard (last runtime checkpoint)

- **Date:** 2026-05-09
- **Type:** Audit-only. No source, runtime, test, or Kconfig change.
- **Close status:** `CLOSED`
- **Full checkpoint:** `RobotOS_v1.0/devkit/docs/02_PHASE_CLOSEOUTS/PHASE_9EZ_CHECKPOINT.md`
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
- **Full audit:** `RobotOS_v1.0/devkit/docs/02_PHASE_CLOSEOUTS/PHASE_9Z_CHECKPOINT.md`
- **Prior phase:** Phase 9D (`8e8c801`), Workload Demo Script & Runbook

**Phase 9Z verdict:** ON_TRACK_WITH_WATCHPOINTS. Core/platform boundaries intact.
All Phase 9A-A through 9D sub-phases closed with hardware RTT evidence on
STM32F411E-DISCO. Scheduler (7A/7B-1) DEFER — no workload-driven justification.
Phase 8A (F407) HOLD/DEFER. Next step: user product-direction decision.

**Phase 9D delivered:**

- `RobotOS_v1.0/tools/runtime/run_phase9d_demo.ps1` — PowerShell runner that orchestrates: wiring reminder → Phase 6O RTT capture (background job) → boot-settle wait → scripted UART `a`/`s`/`r` send to a configurable COM port → operator prompt for button presses → verification checklist printout. PowerShell 5.1 compatible; no parsing/automation overreach.
- `RobotOS_v1.0/devkit/docs/03_SPECS/WORKLOAD_DEMO_9D.md` — canonical runbook covering hardware setup, build/flash, scripted and semi-scripted demo paths, the canonical demo scenario, full pass/fail criteria (required string patterns, deterministic counter targets, architecture invariants, fault registers), and known caveats (operator timing, line-ending, bounce, etc.).
- `RobotOS_v1.0/devkit/logs/phase_9D_workload_demo_2026-05-08.txt` — RTT capture from one execution of `run_phase9d_demo.ps1 -ComPort COM5`; 90 s window, 37930 bytes; harness exit 0 with 12 default + Phase 9D-specific patterns.
- `RobotOS_v1.0/devkit/logs/INDEX.md` — Phase 9D row.
- `RobotOS_v1.0/devkit/docs/01_PROGRESS/DEVKIT_PROGRESS.md` — Phase 9D section.

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
Runbook: `RobotOS_v1.0/devkit/docs/03_SPECS/WORKLOAD_DEMO_9D.md`

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
| `RobotOS_v1.0/devkit/docs/01_PROGRESS/DEVKIT_PROGRESS.md` | Phase 6K/6L/6M close-status flips; Phase 6Z section appended |
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
