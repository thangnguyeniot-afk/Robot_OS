# RobotOS — Documentation Synchronization Rules

> **Document type:** Governance — Documentation Consistency Policy  
> **Version:** 1.0  
> **Applies to:** All source documents in `docs/`, root-level architecture files, and decision memos  
> **Authority:** Enforced alongside `FINAL_CANONICAL_DECISION_MEMO.md`

---

## 1. Purpose and Scope

This document defines the rules that keep the RobotOS documentation set internally consistent. It covers:

- Which document owns which category of information
- How conflicts between documents are resolved
- What constraints apply to code examples in documentation
- How changes are propagated across the doc set
- The review gate that must pass before implementation begins

These rules apply to all new and edited files in `docs/`, and to any root-level `.md` file that contains API signatures, code examples, layer boundary statements, or architectural claims.

---

## 2. Source-of-Truth Assignment

Every type of information has exactly one authoritative owner. Other documents may reference but may not redefine the owned content.

| Information category | Authoritative owner | Other docs may |
|---|---|---|
| System philosophy and guiding principles | `ARCHITECTURE.md` | Quote with cross-reference |
| Layer boundaries and dependency direction | `ARCHITECTURE.md` | Reference only |
| Portability law and Zephyr-isolation rules | `ARCHITECTURE.md` | Reference only |
| Boot sequence and initialization contract | `ARCHITECTURE.md` | Reference only |
| Architectural constraints on design patterns | `ARCHITECTURE.md` | Reference only |
| Canonical public API signatures | Layer doc for that layer | Excerpt with `> Source:` attribution |
| Lifecycle preconditions and postconditions | Layer doc | Quote with attribution |
| Ownership model (handle, lifetime, buffer) | Layer doc | Reference |
| Canonical usage examples for an API | Layer doc | Link; do not duplicate |
| System-level conceptual flow examples | `ARCHITECTURE.md` | Must comply with layer doc signatures |
| Locked decisions | `FINAL_CANONICAL_DECISION_MEMO.md` | Reference by DEC-ID |

---

## 3. Conflict Resolution Rules

**Rule C-1: API shape conflict (ARCHITECTURE.md vs. layer doc)**  
Layer doc wins. ARCHITECTURE.md must be updated in the same commit or the same PR. Rationale: layer docs are specifications; ARCHITECTURE.md provides architectural context. Incorrect specifications produce incorrect implementations.

**Rule C-2: Cross-layer boundary conflict (two layer docs disagree)**  
Lower-layer doc wins. If `ADAPTER_LAYER.md` and `APPLICATION_LAYER.md` disagree on an Adapter-owned API, `ADAPTER_LAYER.md` wins. Same for Framework vs. Adapter — Framework doc is authoritative for Framework APIs.

**Rule C-3: Conceptual example vs. canonical signature**  
A system-level example in ARCHITECTURE.md that uses an API from any layer doc must exactly match that layer doc's canonical signature. An example that diverges (different parameter count, different return type, invented function name) is incorrect and must be fixed.

**Rule C-4: Decision memo overrides**  
`FINAL_CANONICAL_DECISION_MEMO.md` overrides all three input docs on points where they conflict with a locked decision. When a locked decision is applied to a doc, the relevant section receives a `<!-- STATUS: LOCKED DEC-XX -->` comment. After all patches are applied, the decision memo records this in its implementation gate checklist.

---

## 4. Example-Code Compliance Rules

Code examples in documentation — whether labeled as "canonical", "conceptual", or "illustrative" — must satisfy all of the following:

**Rule E-1: No invented functions.** Every function called in an example must exist in a canonical API spec somewhere in the doc set. If a utility function is used, it must be declared in a header, not assumed to exist.

**Rule E-2: No concrete instantiation of opaque types.** If a type is declared opaque (`typedef struct foo foo_t;`), examples must never stack-allocate or statically allocate it directly. All instantiation must use the factory function.

```c
// ❌ WRONG — opaque type concretized:
static ro_queue_t g_cmd_queue;

// ✅ CORRECT — factory return:
ro_queue_t* g_cmd_queue;  // assigned from ro_queue_create(...)
```

**Rule E-3: Parameter counts must match.** The number and type of arguments in every function call in every example must match the canonical signature. No exceptions. Discrepancies must be fixed at both sites simultaneously.

**Rule E-4: Prefix rules apply to examples.** Examples must use the same prefix (or absence of prefix) as the canonical declaration. Adapter functions use `ro_`. Framework functions do not. If an example calls `ro_pid_update(...)`, it is wrong — canonical is `pid_update(...)`.

**Rule E-5: Examples must compile in principle.** An example need not be complete compilation unit, but every statement in it must be syntactically and semantically correct given the declared types and canonical signatures. If an example would produce a compiler error when placed in a valid translation unit, it is a documentation defect.

---

## 5. Change Propagation Checklist

When any of the following changes are made, the associated propagation steps are mandatory.

### 5.1 Adapter API change

A change to any `ro_*` API signature in `ADAPTER_LAYER.md` requires:

- [ ] Update `ADAPTER_LAYER.md` canonical signature block
- [ ] Update `<!-- STATUS: ... -->` marker in `ADAPTER_LAYER.md`
- [ ] Search `ARCHITECTURE.md` for every occurrence of the changed function name; update all
- [ ] Search `APPLICATION_LAYER.md` for every occurrence; update all
- [ ] Check all source files in `include/robotos/` and `src/adapter/` for implementation impact
- [ ] If the change touches a locked decision: open a Decision Update (see §6)

### 5.2 Framework API change

A change to any `*` (non-`ro_`-prefixed) Framework API in `FRAMEWORK_LAYER.md` (if present) or `include/robotos/` headers requires:

- [ ] Update the canonical header comment in the affected `.h` file
- [ ] Search `ARCHITECTURE.md` for occurrences; update
- [ ] Search `APPLICATION_LAYER.md` for occurrences; update

### 5.3 Boundary rule change

Any change to a layer boundary statement, include restriction, dependency rule, or portability constraint in any doc requires:

- [ ] **Must go through a Decision Update** (see §6) before the doc is changed
- [ ] `ARCHITECTURE.md` is the primary site of the change; layer docs are updated to align
- [ ] Confirm: does the change require a CI rule update? If yes, update `ARCHITECTURE.md §CI Enforcement Rules`

---

## 6. API Lock and Decision Update Protocol

### Marking a section as locked

When a section is finalized by a decision memo, add the following comment immediately before or after the canonical signature block:

```
<!-- STATUS: LOCKED DEC-XX -->
```

Where `XX` is the decision ID from `FINAL_CANONICAL_DECISION_MEMO.md`. This comment survives doc reformats and is checked by the pre-implementation review.

### Decision Update format

If a locked decision must be changed, a formal Decision Update must be added to `FINAL_CANONICAL_DECISION_MEMO.md` before the doc is patched. Format:

```markdown
### UPDATE to DEC-XX — [Short description]

**Date:** YYYY-MM-DD  
**Reason:** [One-paragraph justification; must address why the locked decision no longer applies]  
**New decision:** [Full replacement text; or add as a sub-point if scope is narrow]  
**Supersedes:** [Quote the specific text overridden]  
**Approver:** [Architecture owner sign-off]
```

No doc patch that changes a locked decision is valid without this entry.

---

## 7. Sync Audit Rules

### Pre-implementation gate

Before implementation begins on any API covered by a locked decision:

1. Every `<!-- STATUS: LOCKED DEC-XX -->` marker must be present in all impacted docs.
2. Every function signature in ARCHITECTURE.md conceptual examples must match the canonical layer doc form.
3. No `static <opaque_type> <name>;` pattern appears anywhere in documentation examples.
4. No invented function names (not declared in any `include/` header or layer doc) appear in examples.
5. No cross-include boundary violations appear in example code (e.g., `<zephyr/*>` in `app_glue_robotos.c` examples).

### Post-edit signature diff pass

After patching a layer doc, run a function-name grep across all three docs for every function named in the change. Confirm each occurrence either matches the canonical form or is a history/rationale comment clearly marked as deprecated.

```
grep -rn "ro_queue_create\|ro_timer_sync\|ro_timer_start_periodic\|ro_deadline" docs/
```

### Two-person rule

Any patch to a locked section requires confirmation by a second reviewer before merge. Single-author edits to locked sections are not permitted.

---

## 8. Status Labels

Used in inline `<!-- STATUS: ... -->` comments throughout the doc set.

| Label | Meaning | Who may change |
|---|---|---|
| `LOCKED DEC-XX` | Finalized by decision memo DEC-XX. Requires Decision Update to change. | Architecture owner + Decision Update |
| `DRAFT` | Under active discussion. Not authoritative. | Any editor |
| `DEPRECATED` | Superseded by a LOCKED decision. Text retained for migration reference only. | Editor + note of which DEC supersedes |
| `TRANSITIONAL` | Matches old behavior; scheduled for replacement in a future patch. Must cite deadline or DEC-ID. | Editor, with target DEC citation |
| `NEEDS CONFIRMATION` | Decision direction recommended but owner sign-off pending. Do not implement. | Architecture owner |

---

## 9. Practical Review Workflow Before Implementation

The following sequence must be completed and checked off before writing any production code that implements an API touched by this doc set.

**Step 1 — Confirm decision memo is complete**

- Open `FINAL_CANONICAL_DECISION_MEMO.md`
- Verify implementation gate checklist (§ Implementation Gate) has no unchecked items
- Verify all NEEDS CONFIRMATION and LOCKED WITH FOLLOW-UP items are resolved

**Step 2 — Patch docs in canonical order**

| Order | Document | Decisions to apply |
|---|---|---|
| 1 | `ADAPTER_LAYER.md` | DEC-01, DEC-02, DEC-03, DEC-05 |
| 2 | `ARCHITECTURE.md` | DEC-02, DEC-03, DEC-06, DEC-07, DEC-09 |
| 3 | `APPLICATION_LAYER.md` | DEC-01, DEC-04, DEC-05, DEC-06 |
| 4 | Header files in `include/` | DEC-01, DEC-02, DEC-03 |
| 5 | Source files in `src/` | DEC-04, DEC-07, DEC-08 |

**Step 3 — Post-patch grep pass**

Run the signature diff pass (§7) for all function names touched in Steps 1–5. Resolve any remaining divergences.

**Step 4 — Two-person sign-off**

Both the author and a second reviewer must confirm: no locked section has been changed without a Decision Update; all STATUS markers are present; no opaque-type stack instantiation or invented functions remain in examples.

**Step 5 — Unblock implementation**

Implementation may proceed. The `<!-- STATUS: LOCKED DEC-XX -->` markers serve as implementation contract markers for engineers reading the code to trace implementation decisions back to their rationale.

---

## 10. Relationship to FINAL_CANONICAL_DECISION_MEMO.md

`FINAL_CANONICAL_DECISION_MEMO.md` and this document are complementary:

| Aspect | Covered by |
|---|---|
| What the nine API and naming decisions actually are | `FINAL_CANONICAL_DECISION_MEMO.md` |
| Rationale, alternatives rejected, philosophy notes | `FINAL_CANONICAL_DECISION_MEMO.md` |
| Which doc sources what category of information | This document |
| How conflicts are resolved systematically | This document |
| How locked decisions are changed via Decision Updates | This document §6 |
| Pre-implementation review gate | Both (memo: gate checklist; rules: workflow) |

Neither document is optional. Skipping the memo means you don't know what was decided. Skipping these rules means you don't know how to apply or update those decisions safely.

---

## 11. Quick Review Checklist

Before marking any doc PR as ready for merge, verify:

- [ ] Every function call in examples matches a canonical signature (parameter count, types, return type)
- [ ] No opaque type is stack- or statically-allocated in examples
- [ ] No function is used in an example that isn't declared in a header or layer doc
- [ ] Every Framework function uses the correct prefix (no `ro_` for Framework)
- [ ] All `<!-- STATUS: LOCKED DEC-XX -->` markers are present on changed sections
- [ ] `app_glue_robotos.c` examples contain no `<zephyr/*>` includes
- [ ] `app_glue_zephyr.c` examples contain no `<robotos/*.h>` includes
- [ ] No `ro_deadline_register()` calls appear anywhere
- [ ] No `ro_timer_init()` calls appear anywhere
- [ ] No `APP_DL_*` integer constants appear in App Core examples
- [ ] If a locked section was changed: Decision Update is present in `FINAL_CANONICAL_DECISION_MEMO.md`
- [ ] Two-person sign-off obtained on any changes to locked sections

---

*Related documents: [FINAL_CANONICAL_DECISION_MEMO.md](FINAL_CANONICAL_DECISION_MEMO.md) | [ARCHITECTURE.md](docs/ARCHITECTURE.md) | [ADAPTER_LAYER.md](docs/ADAPTER_LAYER.md) | [APPLICATION_LAYER.md](docs/APPLICATION_LAYER.md)*
