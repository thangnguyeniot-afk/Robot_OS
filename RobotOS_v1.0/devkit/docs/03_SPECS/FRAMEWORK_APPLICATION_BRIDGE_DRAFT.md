# RobotOS Framework Application Bridge — Draft Spec

**Status:** `DRAFT / NON-FINAL`. No implementation exists yet.
Function names, type shapes, and behavioral guarantees are all subject
to change until an implementation phase (Phase 12F or later) locks
them. ABI is **NOT** stable.
**Revision:** Phase 12F-pre (2026-05-12, `CLOSED_DOCS_ONLY`) — initial
draft created alongside the Phase 12F-pre planning doc.
**Next revision condition:** Phase 12F — when the bridge module
`framework/robotos_fw_event_bridge.{h,c}` is created and host-tested,
§5 names and signatures move from `DRAFT` to `LOCKED-AT-12F`.

> **No `framework/robotos_fw_event_bridge.*` file exists. No bridge
> `.c` body exists. This document describes a future API that has not
> been implemented.** Phase 12C confirmed the Application bridge concept
> (`APPLICATION_OWNED_EVENT_BRIDGE_CONFIRMED`); Phase 12F-pre is the
> first concrete planning of that bridge.

This spec is anchored to
[`../02_PHASE_CLOSEOUTS/PHASE_12F_PRE_APPLICATION_BRIDGE_PLANNING.md`](../02_PHASE_CLOSEOUTS/PHASE_12F_PRE_APPLICATION_BRIDGE_PLANNING.md)
and to the Phase 12C confirmation of the bridge pattern in
[`../02_PHASE_CLOSEOUTS/PHASE_12C_FSM_EVENT_BRIDGE_STATUS_MODEL.md`](../02_PHASE_CLOSEOUTS/PHASE_12C_FSM_EVENT_BRIDGE_STATUS_MODEL.md).

---

## 1. Status / Scope

**What this doc is:**
A long-lived spec draft for the RobotOS Framework Application Bridge —
the layer that translates Adapter-level `robotos_event_t` events to
Framework-level `robotos_fw_event_id_t` logical events and calls
`robotos_fw_fsm_dispatch()`.

**What this doc is not:**
- A final ABI. No function name or type is final until Phase 12F
  locks them.
- An implementation record. No source file maps to this spec yet.
- A product / Application command vocabulary. The bridge is
  product-neutral; product semantics live in a future Application
  layer that does not exist.
- A promotion or replacement of `devkit_app_state`. That module
  remains devkit-local and authoritative for devkit runtime behavior.

**Current decision state (Phase 12F-pre):**

| Decision | Status |
|---|---|
| Bridge pattern | `APPLICATION_OWNED_EVENT_BRIDGE_CONFIRMED` at Phase 12C |
| Bridge layer | Above the Framework FSM, below any product / Application layer |
| First implementation path | `PHASE_12F_PRE_RECOMMEND_HOST_BRIDGE_PROTOTYPE` (Phase 12F, host-only) |
| Bridge module location | `RobotOS_v1.0/framework/robotos_fw_event_bridge.{h,c}` (proposed; locks at Phase 12F) |
| Bridge instance ownership | Caller-owned (proposed; locks at Phase 12F) |
| Mapping table ownership | Caller-owned static const array (proposed; locks at Phase 12F) |
| FSM ownership | Bridge takes `robotos_fw_fsm_t *` from caller; bridge does NOT own the FSM (proposed; locks at Phase 12F) |
| Adapter key shape | `(robotos_event_type_t, optional uint32_t arg0 match)` (proposed; locks at Phase 12F) |
| Unmapped event behavior | Silent OK + `unmapped_count++`; FSM not called (proposed; locks at Phase 12F) |
| Payload pass-through | Borrowed `const void *` passed through unchanged; never stored (proposed; locks at Phase 12F) |
| Status model | Reuse `robotos_core_status_t` via `robotos_fw_status_t` alias; no new enum (proposed; locks at Phase 12F) |
| Threading | Thread context only; no ISR (proposed; locks at Phase 12F) |
| Forbidden coupling | No UART TX, no GPIO/PWM/I2C/SPI drivers, no `robotos_core_register_event_handler`, no Zephyr / devkit / legacy `ro_*` includes, no heap (proposed; locks at Phase 12F) |

---

## 2. Layer Boundary

```text
[ Application / product layer    ]  defines mapping table, owns FSM,
                                    owns bridge instance; chooses
                                    product vocabulary (no product
                                    layer exists yet)

[ Framework Application Bridge   ]  robotos_fw_event_bridge_* — pure
    [this spec]                     mapping engine; product-neutral

[ Robot Framework FSM            ]  robotos_fw_fsm_* — flat FSM
                                    (Phase 12B/12C/12D/12E)

[ Robot Adapter / runtime        ]  core/ + platform/ + devkit hardware
  substrate                         glue (devkit_app_state is NOT
                                    bridge or Framework)

[ Kernel / HW                    ]  Zephyr + STM32F411E-DISCO
```

**Boundary assertions:**

- The bridge module sits between the Adapter event sources and the
  Framework FSM. It is the only layer that knows about both
  `robotos_event_t` (or its host-test equivalent) and
  `robotos_fw_event_id_t`.
- The bridge does **not** call UART TX. (Phase 9EZ §H scope guards
  preserved.)
- The bridge does **not** own GPIO / sensor / actuator drivers.
- The bridge is **product-neutral**: no `IDLE`/`ARMED`/`ACTIVE` (or
  any product vocabulary) names in the bridge module itself. Product
  vocabulary lives in the Application layer or in a host-test
  synthetic vocabulary clearly marked as such.
- The bridge does **not** consume `devkit_app_state` symbols.

---

## 3. Bridge Problem

The Framework FSM is product-neutral and consumes
`robotos_fw_event_id_t` (`uint32_t`) values. Real event sources in the
active stack produce `robotos_event_t` with `type ∈ {USER, USER+1,
USER+2, USER+3}` (currently 100-103) plus `arg0`/`arg1`. A consumer of
the FSM needs:

1. A mapping from Adapter event keys → Framework event IDs.
2. A dispatch entry point that scans the mapping and calls
   `robotos_fw_fsm_dispatch()` on a hit.
3. A way to ignore unmapped Adapter events without polluting the FSM
   counters or inventing a default Framework event ID.

This document specifies that bridge as a small, caller-owned,
product-neutral module.

---

## 4. Event Translation Model

### 4.1 Adapter key

The bridge accepts a small input pair, not a full `robotos_event_t`:

- `type` — primary discriminator, sized as a `uint32_t` (to keep the
  bridge core-agnostic; production callers will pass
  `(uint32_t)event->type`). Equivalent to the `robotos_event_type_t`
  value in the active core.
- `arg0` — optional sub-discriminator, `uint32_t`. Used to match
  individual UART bytes or button sequence values.

Using a small input pair (rather than the full `robotos_event_t`) lets
the bridge be tested entirely on host without forcing the test to
build the full core event struct, and keeps the bridge module free of
any include of `robotos_core.h`. Production callers translate
`robotos_event_t` to the (type, arg0) pair at the call site.

### 4.2 Mapping table

The application declares a **static const** array of mapping rows:

```c
/* DRAFT / NON-FINAL */
typedef struct {
    uint32_t                adapter_type;   /* matches event type */
    uint32_t                adapter_arg0;   /* matches event arg0 */
    bool                    match_arg0;     /* if false, arg0 is wildcard */
    robotos_fw_event_id_t   fw_event_id;    /* Framework event to dispatch */
} robotos_fw_event_bridge_row_t;
```

- Rows are scanned **FIFO first-match** (same convention as the FSM
  transition table).
- A row with `match_arg0 == false` matches any `arg0` value for the
  given `adapter_type`.
- A row with `match_arg0 == true` matches only the exact `arg0`.
- The application owns the array; the bridge stores a pointer.

### 4.3 Dispatch flow

```text
bridge_dispatch(bridge, type, arg0, payload):
  event_count++
  scan rows FIFO:
    if row.adapter_type != type:        continue
    if row.match_arg0 and row.adapter_arg0 != arg0: continue
    -> robotos_fw_fsm_dispatch(fsm, row.fw_event_id, payload)
    mapped_count++
    last_fw_event_id = row.fw_event_id
    last_status = <fsm dispatch return>
    return last_status
  unmapped_count++
  last_status = ROBOTOS_CORE_OK
  return ROBOTOS_CORE_OK
```

### 4.4 No "default" Framework event ID

The bridge intentionally does **not** synthesize a default
Framework event ID for unmapped Adapter events. Unmapped events
increment `unmapped_count` and return OK; they never call
`robotos_fw_fsm_dispatch()`. This keeps unmapped Adapter traffic
from affecting FSM counters.

---

## 5. Draft API Surface (DRAFT / NON-FINAL; locks at Phase 12F)

### 5.1 Types

```c
/* DRAFT / NON-FINAL */

typedef struct {
    uint32_t                adapter_type;
    uint32_t                adapter_arg0;
    bool                    match_arg0;
    robotos_fw_event_id_t   fw_event_id;
} robotos_fw_event_bridge_row_t;

typedef struct {
    const robotos_fw_event_bridge_row_t *rows;
    uint32_t                              row_count;
    robotos_fw_fsm_t                     *fsm;          /* caller-owned */
    void                                 *user_context; /* opaque; reserved */
} robotos_fw_event_bridge_config_t;

typedef struct {
    const robotos_fw_event_bridge_config_t *config;
    uint32_t                                event_count;
    uint32_t                                mapped_count;
    uint32_t                                unmapped_count;
    uint32_t                                last_adapter_type;
    uint32_t                                last_adapter_arg0;
    robotos_fw_event_id_t                   last_fw_event_id;
    robotos_fw_status_t                     last_status;
    bool                                    initialized;
} robotos_fw_event_bridge_t;

typedef struct {
    uint32_t                event_count;
    uint32_t                mapped_count;
    uint32_t                unmapped_count;
    uint32_t                last_adapter_type;
    uint32_t                last_adapter_arg0;
    robotos_fw_event_id_t   last_fw_event_id;
    robotos_fw_status_t     last_status;
    bool                    initialized;
} robotos_fw_event_bridge_snapshot_t;
```

### 5.2 Functions (DRAFT / NON-FINAL; locks at Phase 12F)

```c
/* DRAFT / NON-FINAL */

/* Initialize a caller-owned bridge against a caller-owned config.
 * Returns ERR_NULL if any required pointer is NULL.
 * Returns ERR_INVALID_ARG if row_count == 0.
 * Returns OK on success. Thread context only. */
robotos_fw_status_t robotos_fw_event_bridge_init(
    robotos_fw_event_bridge_t              *bridge,
    const robotos_fw_event_bridge_config_t *config);

/* Dispatch an Adapter event key (type, arg0) and payload through the
 * bridge. Scans the mapping table FIFO. On the first match, calls
 * robotos_fw_fsm_dispatch(fsm, row.fw_event_id, payload) and returns
 * its status. On no match, returns OK and increments unmapped_count.
 * Thread context only; never ISR. */
robotos_fw_status_t robotos_fw_event_bridge_dispatch(
    robotos_fw_event_bridge_t *bridge,
    uint32_t                   adapter_type,
    uint32_t                   adapter_arg0,
    const void                *payload);

/* Reset all bridge counters (event_count, mapped_count,
 * unmapped_count, last_*). Does NOT reset the FSM. Returns OK.
 * Returns ERR_NULL if bridge is NULL.
 * Returns ERR_INVALID_STATE if bridge is not initialized. */
robotos_fw_status_t robotos_fw_event_bridge_reset(
    robotos_fw_event_bridge_t *bridge);

/* Copy bridge counters into a caller-supplied snapshot struct.
 * Returns ERR_NULL if bridge or out is NULL.
 * Returns ERR_INVALID_STATE if bridge is not initialized.
 * Returns OK on success. */
robotos_fw_status_t robotos_fw_event_bridge_get_snapshot(
    const robotos_fw_event_bridge_t        *bridge,
    robotos_fw_event_bridge_snapshot_t     *out);
```

**ABI lock condition:** these signatures become LOCKED-AT-12F when the
Phase 12F closeout records `PHASE_12F_APPLICATION_BRIDGE_HOST_PROTOTYPE_CLOSED`.

---

## 6. Payload Lifetime

- The bridge's `payload` argument is a borrowed `const void *`,
  identical in lifetime to the FSM `dispatch()` payload (Phase 12C
  `PAYLOAD_BORROWED_FOR_DISPATCH_ONLY_CONFIRMED`).
- The bridge **does not store** the payload pointer in its instance
  struct after `dispatch()` returns. The struct definition in §5.1
  has no `void *payload` field.
- The application owns all payload memory. If a durable copy of any
  payload field is needed, the application makes the copy before
  calling the bridge, or the FSM action callback copies into
  `user_context`.

---

## 7. Status Propagation

- The bridge reuses `robotos_core_status_t` through the existing
  `robotos_fw_status_t` alias (Phase 12C
  `REUSE_ROBOTOS_CORE_STATUS_T_FOR_PHASE_12D_CONFIRMED`). **No new
  status enum is introduced.**
- `robotos_fw_event_bridge_dispatch()` returns:
  - On a mapped event: whatever
    `robotos_fw_fsm_dispatch()` returned (OK on success / no
    transition; non-OK on action failure; etc.).
  - On an unmapped event: `ROBOTOS_CORE_OK`.
  - On NULL bridge: `ROBOTOS_CORE_ERR_NULL`.
  - On uninitialized bridge: `ROBOTOS_CORE_ERR_INVALID_STATE`.
- `last_status` records the most recent dispatch's status (either the
  FSM's return or `ROBOTOS_CORE_OK` for an unmapped event).

---

## 8. Relationship to `devkit_app_state`

- `devkit_app_state` (Phase 9C) remains **authoritative** for the
  devkit runtime state machine. It owns `IDLE/ARMED/ACTIVE`, the `?`
  UART response, the `a/s/r/d/t/T` byte handling, and the button
  cycle.
- The bridge does **not** include `devkit_app_state.h`. The bridge
  contains no `devkit_app_state` symbol reference. This is testable
  via grep (`grep devkit_app_state framework/robotos_fw_event_bridge.c`
  must return 0 matches in Phase 12F).
- Three future devkit integration modes are explicitly enumerated in
  the Phase 12F-pre closeout
  [`../02_PHASE_CLOSEOUTS/PHASE_12F_PRE_APPLICATION_BRIDGE_PLANNING.md`](../02_PHASE_CLOSEOUTS/PHASE_12F_PRE_APPLICATION_BRIDGE_PLANNING.md)
  §G: shadow mode, replacement mode, separate application mode. None
  is in scope for Phase 12F.
- Scope-guard #11 is preserved.

---

## 9. Relationship to Command Set

- `a / s / r / ? / x / v / L / d / T` remain the devkit/probe command
  surface. The bridge adds **no UART command** and **no UART
  exposure** of FSM state.
- The bridge is not consumed by any UART RX path or any UART TX path
  in Phase 12F.
- Future command vocabulary belongs to the Application/product
  phase, not the bridge.
- All 12 UART TX scope-guard constraints from
  [`../02_PHASE_CLOSEOUTS/PHASE_9EZ_CHECKPOINT.md`](../02_PHASE_CLOSEOUTS/PHASE_9EZ_CHECKPOINT.md)
  §H remain preserved.

---

## 10. Candidate Future Implementations

The bridge is a small mapping engine; production callers will use it
in one of several patterns (none of which are Phase 12F scope):

| Future pattern | Owner of bridge instance | Owner of FSM instance | Mapping source | Status |
|---|---|---|---|---|
| Host unit-test prototype (Phase 12F) | Test executable | Test executable | Static const array in test source | RECOMMENDED |
| Devkit shadow integration (future Phase 12G-pre etc.) | New `devkit/src/devkit_fw_shadow.c` | Same file | Static const array in same file; rows reflect `robotos_event_type_t` 102/103 + UART byte values | NOT_AUTHORIZED (planning gate required) |
| Devkit replacement integration | `devkit/src/devkit_app_state.c` rewrite | Same file | Static const array | NOT_RECOMMENDED ever without dedicated migration phase |
| Separate Application layer | `RobotOS_v1.0/app/<product>/<file>.c` | Same file | Static const array | NOT_AUTHORIZED (Application planning required first) |

All four patterns reuse the **same bridge module** declared in §5;
none of them require a separate bridge implementation. The bridge is
designed to be product-neutral so it can serve all four call sites.

---

## 11. Open Decisions

Items intentionally left open for Phase 12F (or later) to resolve:

| # | Open question | Latest at |
|---|---|---|
| 1 | Whether the bridge module lives at `framework/robotos_fw_event_bridge.{h,c}` or at a different path | Phase 12F |
| 2 | Whether the public surface in §5 is exactly four functions or fewer / more | Phase 12F (Phase 12F-pre proposes four) |
| 3 | Whether `robotos_fw_event_bridge_t` exposes its fields publicly (like the FSM struct does) or keeps them opaque | Phase 12F (Phase 12F-pre proposes public, consistent with FSM) |
| 4 | Whether `arg0` is sufficient as a sub-discriminator, or whether `arg1` (or a hash function pointer) is also needed | Phase 12F |
| 5 | Whether a bridge can hold a list of FSMs (fan-out) | Phase 12G or later (no use case yet; Phase 12F-pre proposes single FSM) |
| 6 | Whether the bridge's `event_count` should be the count of *attempted* dispatches or *mapped* dispatches | Phase 12F (Phase 12F-pre proposes attempted; mapped is recorded separately as `mapped_count`) |
| 7 | Whether the bridge should expose a hook to log unmapped events for diagnostic purposes | Phase 12G or later |
| 8 | Whether the bridge needs critical-section protection on its counters (similar to the FSM's ISR-safe getters) | Phase 12F (Phase 12F-pre proposes thread-context only: no ISR access, no critical section needed) |

---

## 12. Next Revision Conditions

This spec is revised when:

1. **Phase 12F opens (host bridge prototype):** Header
   `robotos_fw_event_bridge.h` and source `robotos_fw_event_bridge.c`
   are created. §5 names move from `DRAFT` to `LOCKED-AT-12F`. The
   eight open decisions in §11 are resolved or explicitly deferred
   per item. A host-test target validates the bridge contract.
2. **Phase 12G or later (devkit shadow or other integration):** Spec
   gains a new section describing the integration mode. §10 marks the
   chosen pattern as `IMPLEMENTED_AT_12G`.
3. **Phase 13 or later (Application/product layer):** Spec gains a
   §13 referring to product vocabulary; bridge stays product-neutral.

This spec is **NOT** revised to:

- Record devkit implementation details (devkit docs are authoritative
  for devkit; this spec is authoritative for the bridge layer).
- Track Phase-by-Phase test results (closeouts are authoritative).
- Pick a product vocabulary (that belongs to a future Application
  phase).
- Promote `devkit_app_state` (scope-guard #11 enforced).
- Change command semantics (frozen).
