# RobotOS Phase Close Report Template

> **Usage:** Copy this template for each phase close report (Copilot / Claude).
> Fill every section. Mark unknown items explicitly.
> Do not claim PASS if any required evidence is missing or uncertain.
>
> **Log policy:** Full logs should be saved as repo evidence files when useful
> (e.g., `tests/host/logs/phase_XX_host_YYYY-MM-DD.log`).
> Chat/report should summarize only decisive evidence unless debugging a failure.

---

## 1. Phase Result

<!-- Choose exactly one: -->

- [ ] `CLOSED` — all gates pass, no blockers, no uncertainty
- [ ] `CLOSED_WITH_CAUTION` — all gates pass, minor uncertainty noted
- [ ] `BLOCKED` — cannot proceed; reason documented below
- [ ] `NEEDS_CORRECTION` — evidence shows error; correction required before close
- [ ] `DRIFT` — scope or behavior diverged from approved direction

**Final verdict:** `[CHOOSE ONE]`

**Phase name:** Phase XX — [Description]
**Date:** YYYY-MM-DD
**Branch:** master (or feature branch if applicable)

---

## 2. Scope Executed

<!-- What was actually implemented / audited / run. -->
<!-- Distinguish: confirmed done vs. skipped vs. deferred. -->

- [x] ...
- [ ] ...

---

## 3. Files Changed

| File | Change Summary |
|------|---------------|
| `path/to/file.c` | [what changed] |
| `path/to/file.h` | [what changed] |

---

## 4. Behavior Changed

<!-- List only observable behavior changes. -->
<!-- If none, say: No observable behavior change. -->

- ...

---

## 5. Behavior Explicitly Not Changed

<!-- List key behaviors confirmed unchanged. -->
<!-- This is evidence, not a formality. -->

- Existing `post_event` / `try_post_event` semantics: unchanged
- Status code set: unchanged / [new code added: name = value]
- Snapshot fields: unchanged / [new field added: name]
- Platform boundary: unchanged
- Devkit runtime: unchanged
- [add more as applicable]

---

## 6. Validation Evidence

### Host Tests

```
Command: [exact command run]
Result:  [N/N suites pass] / [FAILED: summary]
Log:     [path to saved log file, or N/A]
```

### Zephyr / Firmware Build

```
Command: [exact west build command, or N/A]
Result:  PASS / FAIL / N/A
FLASH:   [X B / 524288 B (Y%)]
RAM:     [X B / 131072 B (Y%)]
Errors:  [0 / list]
```

### Runtime Evidence

```
Method:  [RTT dump / GDB counter read / not required]
Log:     [path to log file, or N/A]
CFSR:    [0x00000000 / not checked]
HFSR:    [0x00000000 / not checked]
```

### Other Evidence

```
[Any additional validation: static analysis, contract review, etc.]
```

---

## 7. State / Contract Impact

| Item | Changed? | Detail |
|------|----------|--------|
| Status codes | No / Yes | [if yes: new code, value] |
| Mutable state | No / Yes | [if yes: what field, where] |
| Public API / interface | No / Yes | [if yes: signature added/changed] |
| Snapshot fields | No / Yes | [if yes: what field] |
| Scheduler / control-flow | No / Yes | [if yes: what changed] |
| Platform boundary | No / Yes | [if yes: what changed] |
| ISR safety contract | No / Yes | [if yes: what changed] |

---

## 8. Docs Updated

| Doc | Updated? | What changed |
|-----|----------|-------------|
| `01_PROGRESS/DEVKIT_PROGRESS.md` | Yes / No | [phase section added/updated] |
| `core/README.md` | Yes / No | [phase section added] |
| `platform/README.md` | Yes / No | [phase section added] |
| `CURRENT_STATE.md` | Yes / No / See section 12 | [last closed phase updated] |

---

## 9. Commit / Branch / Remote

```
Commit:  [hash]
Message: [commit message summary]
Branch:  master
Remote:  [pushed / not pushed]
```

---

## 10. Drift / Blockers / Uncertainty

<!-- List anything unresolved, unexpected, or borderline. -->
<!-- Do not rationalize missing evidence. -->

| Item | Type | Status | Notes |
|------|------|--------|-------|
| [description] | Drift / Blocker / Uncertainty | Open / Resolved | [notes] |

---

## 11. Next Candidate Work

<!-- Recommended next phases in priority order. -->
<!-- Do not invent requirements. Use only approved candidates. -->

| Phase | Description | Priority |
|-------|-------------|----------|
| Phase 6F | Devkit Mixed Event Policy Smoke | Candidate |
| Phase 6I | Timer Producer Queue-Pressure Stress | Candidate |
| [Phase XX] | [description] | [Candidate / Blocked / Deferred] |

---

## 12. CURRENT_STATE.md Update

<!-- Must be completed at phase close. -->

- [ ] **Updated** — last closed phase, commit, validation evidence, and next candidates refreshed
- [ ] **Not updated** — reason: [explain]

If not updated, this phase close is incomplete for agent startup purposes.
