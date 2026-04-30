# RobotOS — Final Canonical Decision Memo

> **Document type:** Architecture Synchronization — Pre-Implementation Decision Record  
> **Version:** 1.0  
> **Date:** 2026-04-01  
> **Status:** ACTIVE — supersedes conflicting text in all three input docs  
> **Authority:** This memo overrides any conflicting text in ARCHITECTURE.md, ADAPTER_LAYER.md, and APPLICATION_LAYER.md until the docs are patched to reflect these decisions.

---

## Scope

This memo resolves API contract conflicts, naming violations, and ownership ambiguities identified in the April 2026 consistency audit across:

- `docs/ARCHITECTURE.md`
- `docs/ADAPTER_LAYER.md`
- `docs/APPLICATION_LAYER.md`

It covers nine disputed areas. Each decision is marked with one of three statuses:

| Status | Meaning |
|---|---|
| **LOCKED** | Final. No change without a Decision Update (see DOC_SYNC_RULES.md §6). |
| **LOCKED WITH OWNER CONFIRMATION** | Technically decided and recommended. Requires architecture owner sign-off before permanently frozen. |
| **LOCKED WITH FOLLOW-UP** | Direction decided. One sub-point deferred for author confirmation before patch is written. |

---

## Decisions

---

### DEC-01 — `ro_queue_create` Contract

**Status: LOCKED**

#### Final Decision

```c
// Canonical 4-parameter signature.
// buf == NULL, buf_size == 0  → backing carved from RO_GLOBAL_IPC_SLAB (Framework/system use).
// buf != NULL                 → caller-owned static buffer (Application use; bypasses slab).
//   buf_size MUST equal item_size * capacity; returns NULL on mismatch.
// Always returns an opaque pointer. NEVER returns void. NEVER uses out-parameter.
// Returns NULL on any allocation failure.
// <!-- STATUS: LOCKED DEC-01 -->
ro_queue_t* ro_queue_create(size_t item_size,
                             size_t capacity,
                             void*  buf,
                             size_t buf_size);
```

**Forbidden patterns (apply to all docs and code):**

```c
// ❌ WRONG — opaque type must never be stack- or static-instantiated:
static ro_queue_t g_q_cmd;

// ❌ WRONG — out-parameter init (5-arg pattern):
ro_queue_create(&g_q_cmd, sizeof(gcode_cmd_t), COUNT, buf, sizeof(buf));

// ✅ CORRECT — slab-backed (Framework use):
ro_queue_t* fw_q = ro_queue_create(sizeof(fw_msg_t), 8, NULL, 0);

// ✅ CORRECT — app-owned buffer (Application use):
static uint8_t g_cmd_buf[sizeof(gcode_cmd_t) * CONFIG_APP_CMD_QUEUE_DEPTH];
ro_queue_t* cmd_q = ro_queue_create(sizeof(gcode_cmd_t),
                                     CONFIG_APP_CMD_QUEUE_DEPTH,
                                     g_cmd_buf, sizeof(g_cmd_buf));
```

#### Rationale

APPLICATION_LAYER.md correctly identified that `cmd_q` + `seg_q` alone consume 3584 bytes of a 4096-byte slab, exhausting it before Framework queues can allocate. The 4-parameter extension is the minimal change that satisfies both use cases without a separate function and without breaking the opaque-type contract.

#### Alternatives Rejected

| Alternative | Reason rejected |
|---|---|
| Keep 2-param slab-only | Application exhausts slab; no Framework queue budget remains |
| 5-param init-by-out-parameter | Requires `static ro_queue_t` — concrete instantiation of opaque type; compile failure |
| Separate `ro_queue_create_static()` | Two factories for same type; API surface doubles; discovery problem |
| Expand slab default to 8 KB | Wastes RAM on 20–64 KB MCUs; does not scale generically |

**Migration:** All existing 2-param call sites → `ro_queue_create(item_size, cap, NULL, 0)` — mechanical, no semantic change.

**Impacted files:** ADAPTER_LAYER.md §7 (canonical), ARCHITECTURE.md §IPC API, APPLICATION_LAYER.md §IPC Graph.

---

### DEC-02 — `ro_timer_sync` Return Type

**Status: LOCKED WITH OWNER CONFIRMATION**

#### Final Decision (Recommended)

```c
// Returns: number of timer periods elapsed since last call.
// 0  = on time (exactly one period elapsed).
// ≥1 = overrun (N periods missed; caller must log and evaluate fault policy).
// Never returns a negative error code.
// <!-- STATUS: LOCKED WITH OWNER CONFIRMATION DEC-02 -->
uint32_t ro_timer_sync(ro_timer_t* t);
```

**Caller pattern:**

```c
uint32_t overruns = ro_timer_sync(&ctrl_timer);
if (overruns > 0) {
    ctrl_overrun_count += overruns;
    ro_log(RO_LOG_LEVEL_WARN, "control_loop: %u overrun(s)", overruns);
}
```

#### Rationale

`uint32_t` is strictly more informative than `ro_status_t`:

- `ro_status_t == RO_ETIMEDOUT` is binary (missed / not missed). `uint32_t` reveals *how many* periods were missed — a loop that missed 3 periods requires different fault policy than one that missed 1.
- `RO_ETIMEDOUT == -1`. If the current `ro_status_t` version is used and a caller writes `if (overruns > 0)`, then `-1 > 0` evaluates to `false` → overruns are silently ignored. This is the exact silent-failure bug identified in audit issue C2.
- `ro_timer_sync` is not a resource operation in the error model sense. It has no failure modes that produce negative status codes. Using `ro_status_t` here is semantically incorrect.

#### Philosophy Note

> RobotOS otherwise prefers `ro_status_t` for operation-style APIs. `ro_timer_sync` is a
> justified exception: it is an **information-returning timing primitive**, not an operation
> that can succeed or fail. It reports elapsed timer periods — an inherently quantitative
> result, not a status condition. This makes `uint32_t` the correct return type by design.
>
> However, this does change the API style philosophy slightly relative to the rest of the
> Adapter surface. **Final confirmation from the architecture owner is required before this
> decision is permanently frozen.** The recommendation remains `uint32_t`. The confirmation
> step is a governance control, not a technical dispute.

#### Alternatives Rejected

| Alternative | Reason rejected |
|---|---|
| `ro_status_t` (current ADAPTER_LAYER.md) | Binary; `-1 > 0` silent-failure bug; no missed-count information |
| `ro_status_t` + out-param `uint32_t* overruns` | Caller manages second pointer; no advantage; awkward C ABI |

**Migration:** Callers checking `if (s == RO_ETIMEDOUT)` → `if (overruns > 0)`.

**Impacted files:** ADAPTER_LAYER.md §6 (signature + loop example), ARCHITECTURE.md §Component Architecture Timing API.  
*Note: ARCHITECTURE.md §3a already uses `uint32_t` — that section is already correct and becomes the canonical reference.*

---

### DEC-03 — `ro_timer_start_periodic` Parameter Count

**Status: LOCKED**

#### Final Decision

```c
// 2-parameter. No ISR callback.
// <!-- STATUS: LOCKED DEC-03 -->
ro_status_t ro_timer_start_periodic(ro_timer_t* t, uint32_t period_us);
```

No third `handler_isr` parameter. ADAPTER_LAYER.md's 2-parameter form is already correct and requires no change.

#### Rationale

ARCHITECTURE.md §3a introduced a third `void (*handler_isr)(ro_timer_t*)` parameter not documented anywhere in ADAPTER_LAYER.md. Adding an ISR callback to `ro_timer_start_periodic` would:

1. Require the Adapter to hold a function pointer invoked from ISR — turning a timing primitive into an event dispatch mechanism, violating the "one concept per function" principle.
2. Create a hidden callback execution context invisible to the thread roster, violating the "No Magic Thread / No Hidden Callback" contract.
3. Introduce a concurrency hazard: callback fires from ISR but manages state owned by a cooperative caller thread.

The canonical pattern for hardware step generation is: Framework stepper driver uses the MCU timer peripheral directly (`stepper_drv.c`). It does not flow through `ro_timer_start_periodic`.

#### Alternatives Rejected

| Alternative | Reason rejected |
|---|---|
| 3-param with ISR callback (ARCH §3a) | Hidden callback = undeclared execution context; violates no-magic-thread contract |
| Separate `ro_timer_start_periodic_isr(t, period_us, cb)` | Same architectural violation; duplicates the API surface |

**Impacted files:** ARCHITECTURE.md §3a only (delete `handler_isr` from prototype comment; body example already correct).

---

### DEC-04 — `ro_timer_init` Existence

**Status: LOCKED — REMOVE**

`ro_timer_init` does not exist in the canonical API. It appeared only in APPLICATION_LAYER.md §Lane B conceptual code. It is **deprecated text** with no specification backing.

**Every occurrence replaced with:** `ro_timer_start_periodic(&tick, period_us)`

No migration path needed — it was never formally specified.

**Impacted files:** APPLICATION_LAYER.md §Lane B (delete the call; replace with canonical form).

---

### DEC-05 — `ro_deadline` Contract

**Status: LOCKED WITH FOLLOW-UP**

#### Final Decision: Struct-based API is canonical. `ro_deadline_register()` does not exist.

```c
// Canonical API — ADAPTER_LAYER.md §16 is already correct. No change to Adapter.
typedef struct {
    uint64_t    budget_us;   // set at declaration site
    const char* name;        // string literal; used for miss counter lookup and trace
    uint64_t    _start_us;   // private: written by ro_deadline_begin()
} ro_deadline_t;

// <!-- STATUS: LOCKED DEC-05 -->
static inline void ro_deadline_begin(ro_deadline_t* d);
bool               ro_deadline_end(ro_deadline_t* d);
uint32_t           ro_deadline_miss_count(const char* name);
void               ro_deadline_miss_reset(const char* name);
```

`ro_deadline_register()` — **does not exist. Remove all references from all docs.**

#### Preferred Direction: Option A

**Option A: Deadline tracking lives entirely in App Glue. App Core has no deadline awareness.**

```c
// In app_glue_robotos.c — ro_deadline_t objects declared at glue scope:
static ro_deadline_t g_dl_planner_tick = {
    .budget_us = (1000000u / CONFIG_APP_PLANNER_TICK_HZ) * 80u / 100u,
    .name      = "planner_tick",
};
static ro_deadline_t g_dl_pulse_service = {
    .budget_us = 50u,
    .name      = "pulse_service",
};

// In Lane B loop (called from Glue context):
ro_deadline_begin(&g_dl_planner_tick);
// ... work ...
ro_deadline_end(&g_dl_planner_tick);
```

**Why Option A is preferred:**

- **Cleaner App Core boundary.** App Core remains fully OS-agnostic and integration-agnostic. It has no knowledge of `ro_deadline_t`, no Adapter headers, and no deadline IDs of any kind.
- **No undocumented runtime registry.** Struct-based configuration requires no registration call and no hidden lookup table.
- **No Adapter type leakage into App Core.** `ro_deadline_t` is an Adapter type. Allowing it — even as a forward declaration — into App Core headers violates the boundary that makes App Core portable.
- **Simpler ownership model.** Deadline objects are created and destroyed in the same Glue layer that owns thread creation and IPC initialization. No cross-layer pointer sharing.

**`app_deadlines.h` disposition under Option A:**  
The file is either deleted entirely, or retained as a comments-only header for documentation purposes (no types, no integers, no includes). The `APP_DL_*` `uint16_t` integer constants are deprecated and removed.

#### Option B — Fallback Only

Option B (App Core retains symbolic deadline IDs that Glue registers at boot) remains available **only if** future tooling or reporting requirements prove that App-Core-visible symbolic references to deadline slots are necessary — for example, if a host-side analysis tool must parse App Core source to extract deadline budgets. Option B requires a runtime registry (`ro_deadline_register(id, budget)`) that does not currently exist in the Adapter spec.

> **Option B is not pursued unless a concrete blocking constraint requires it.** If that constraint arises, a Decision Update is required.

**Impacted files:** APPLICATION_LAYER.md §Deadline Monitoring, §Lane B conceptual code, §DoD checklist (remove `APP_DL_*` references and `ro_deadline_register` calls).

---

### DEC-06 — `app_glue_robotos.c` Include Boundary

**Status: LOCKED**

#### Final Decision

| Glue file | `<robotos/*.h>` | `<zephyr/*.h>` | Non-Zephyr RTOS headers |
|---|---|---|---|
| `app_glue_robotos.c` | ✅ YES | ❌ **NO** | ❌ NO |
| `app_glue_zephyr.c` | ❌ NO | ✅ YES | ❌ NO |
| `app_glue_freertos.c` | ❌ NO | ❌ NO | ✅ YES |

The phrase in APPLICATION_LAYER.md ("This is the ONLY place allowed to include `<robotos/*.h>` **and** `<zephyr/*.h>`") is **incorrect and must be rewritten**.

`app_glue_robotos.c` uses only `<robotos/*.h>`. These are Adapter headers that already hide all Zephyr. Including `<zephyr/*.h>` directly in the RobotOS glue file would bypass the portability seam and prevent the FreeRTOS comparison build from compiling. This CI enforcement is added to ARCHITECTURE.md §CI Enforcement Rules.

<!-- STATUS: LOCKED DEC-06 -->

**Impacted files:** APPLICATION_LAYER.md §App Glue Layer (rewrite boundary statement); ARCHITECTURE.md §CI Enforcement Rules (add `app_glue_robotos.c` to zero-tolerance list).

---

### DEC-07 — Framework State Machine Naming

**Status: LOCKED**

#### Final Decision

Framework State Machine uses **no `ro_` prefix**. The canonical names, consistent with all other Framework components and with FRAMEWORK_LAYER.md and APPLICATION_LAYER.md, are:

```c
// CANONICAL — Framework SM (robot_sm.h):
// <!-- STATUS: LOCKED DEC-07 -->
typedef struct sm sm_t;
sm_t*        sm_create(const sm_callbacks_t* cb);
ro_status_t  sm_dispatch(sm_t* sm, sm_cmd_t cmd);
sm_state_t   sm_get_state(const sm_t* sm);
ro_status_t  sm_force_fault(sm_t* sm);
```

The names `ro_state_machine_t`, `ro_sm_init`, `ro_sm_transition`, `ro_sm_get_state`, `ro_robot_state_t`, `ro_state_callbacks_t` in ARCHITECTURE.md §Robot State Machine API are **deprecated text** that must be removed.

#### Rationale

ARCHITECTURE.md §Header Naming explicitly states: "If you see `#include <robotos/ro_pid.h>` anywhere — it is wrong. Applying `ro_` blurs layer boundaries." The state machine is a Framework component. The same rule applies identically. ARCHITECTURE.md §Robot State Machine API violates a naming rule established in the same document. The naming convention rule wins; the API block is wrong.

**Impacted files:** ARCHITECTURE.md §Robot State Machine (delete API block; add cross-reference to FRAMEWORK_LAYER.md §Robot State Machine API; keep state diagram and transition table).

---

### DEC-08 — Queue Ownership and RAM Budget Policy

**Status: LOCKED**

#### Final Decision

Two queue categories with distinct ownership:

| Category | Buffer source | Budget | Typical users |
|---|---|---|---|
| **System queues** | `RO_GLOBAL_IPC_SLAB` (4 KB default) | `CONFIG_RO_IPC_SLAB_SIZE` | Framework internal, `log_flush`, system command queues |
| **Application queues** | App-owned `static uint8_t buf[]` | Application static RAM | `cmd_q`, `seg_q`, `evt_q_isr` |

**Rule:** Application queues **must** supply a non-`NULL` `buf` pointer to `ro_queue_create`. This is required — not optional — to preserve slab budget for Framework use.

**Accounting is explicit:** every queue's backing buffer is declared alongside the `ro_queue_create` call as a named `static uint8_t` array. There is no hidden allocation.

<!-- STATUS: LOCKED DEC-08 -->

**Impacted files:** ADAPTER_LAYER.md §7 Queue IPC (add ownership table); ARCHITECTURE.md §Communication Architecture (add ownership table); APPLICATION_LAYER.md §IPC Graph (fix buffer declarations and call syntax).

---

### DEC-09 — Source-of-Truth Rule and Authority Assignment

**Status: LOCKED**

#### Final Decision

| Information category | Authoritative owner | Non-owning files may |
|---|---|---|
| System philosophy and principles | ARCHITECTURE.md | Quote with cross-reference |
| Layer boundaries and dependency direction | ARCHITECTURE.md | Reference only; may not override |
| Portability law and Zephyr-boundary rules | ARCHITECTURE.md | Reference only; may not override |
| Boot sequence and init contract | ARCHITECTURE.md | Reference only |
| Architectural constraints on design patterns | ARCHITECTURE.md | Reference only |
| **Canonical public API signatures** | **Layer doc for that layer** | Excerpt with `> Source:` cross-reference |
| Lifecycle preconditions and postconditions | Layer doc | Quote with cross-reference |
| Ownership model (handle, buffer, lifetime) | Layer doc | Reference |
| Canonical usage examples for an API | Layer doc | Link; do not duplicate |
| System-level conceptual flow examples | ARCHITECTURE.md | **Must comply with layer doc signatures** |

<!-- STATUS: LOCKED DEC-09 -->

#### Conflict Resolution

**API signature conflict (ARCHITECTURE.md vs. layer doc):**  
Layer doc wins on API shape. ARCHITECTURE.md must be updated in the same PR. Rationale: layer docs hold specifications; ARCHITECTURE.md holds architectural context and policy. Specifications require more precision and are more expensive to get wrong downstream.

**Cross-layer boundary conflict (two layer docs disagree):**  
Lower-layer doc wins unless this memo or a subsequent Decision Update explicitly overrides it.

#### Architectural Law vs. API Authority

Layer docs win on **API shape**. They do not win on **architectural law**.

> **Precise boundary:** Architectural law — boundary rules, dependency direction, portability constraints, Zephyr-isolation rules, and system philosophy — is owned by ARCHITECTURE.md and may only be changed via an explicit decision memo or Decision Update. A layer doc change that affects any of these dimensions requires a decision memo. It cannot be enacted by updating the layer doc alone.

If a proposed API change would require relaxing a portability rule, changing a dependency direction, or altering an isolation boundary, the change must go through this decision process first — and ARCHITECTURE.md is updated as part of that process, not afterward.

---

## Outstanding Confirmations and Follow-ups

| Decision | Open item | Who confirms |
|---|---|---|
| **DEC-02** | Architecture owner sign-off on `uint32_t` return type as a justified exception to `ro_status_t` operation-style pattern | Architecture owner |
| **DEC-05** | Author to confirm Option A as final; confirm `app_deadlines.h` disposition (delete or comments-only) | Application layer author |

---

## Implementation Gate

**Implementation of any API touched by DEC-01 through DEC-08 is blocked until all of the following are complete:**

- [ ] DEC-02 owner confirmation received
- [ ] DEC-05 Option A confirmed, `app_deadlines.h` disposition decided
- [ ] ADAPTER_LAYER.md patched to reflect DEC-01, DEC-02, DEC-03, DEC-05
- [ ] ARCHITECTURE.md patched to reflect DEC-02, DEC-03, DEC-06, DEC-07, DEC-09
- [ ] APPLICATION_LAYER.md patched to reflect DEC-01, DEC-04, DEC-05, DEC-06
- [ ] Final sync review completed: all function name occurrences cross-checked, no opaque-type stack instantiation, no invented functions in examples

Once all items are checked: **implementation begins on the locked canonical contracts above.**

---

## Appendix — Decision Status Summary

| ID | Topic | Status |
|---|---|---|
| DEC-01 | `ro_queue_create` — 4-param with optional buffer | LOCKED |
| DEC-02 | `ro_timer_sync` — `uint32_t` overrun count | LOCKED WITH OWNER CONFIRMATION |
| DEC-03 | `ro_timer_start_periodic` — 2-param, no ISR callback | LOCKED |
| DEC-04 | `ro_timer_init` — remove; replace with `ro_timer_start_periodic` | LOCKED |
| DEC-05 | `ro_deadline` — struct-based; Option A preferred; `app_deadlines.h` disposition | LOCKED WITH FOLLOW-UP |
| DEC-06 | `app_glue_robotos.c` — no `<zephyr/*>` | LOCKED |
| DEC-07 | Framework SM — `sm_*` naming; delete `ro_sm_*` from ARCHITECTURE.md | LOCKED |
| DEC-08 | Queue ownership — slab for Framework, app-buffer for Application | LOCKED |
| DEC-09 | Source-of-truth — layer docs own API; ARCHITECTURE.md owns architectural law | LOCKED |

---

*Related documents: [DOC_SYNC_RULES.md](DOC_SYNC_RULES.md) | [ARCHITECTURE.md](../docs/ARCHITECTURE.md) | [ADAPTER_LAYER.md](../docs/ADAPTER_LAYER.md) | [APPLICATION_LAYER.md](../docs/APPLICATION_LAYER.md)*
