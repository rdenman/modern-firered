# 0015 — Rule immutability & mid-run lock semantics

- **Type:** product
- **Status:** Accepted
- **Date:** 2026-09-12
- **Story:** S15
- **ME reference:** “All selections are permanent” (`tx_rac_menu.c`); `LOCK DIFFICULTY` / `tx_Features_LimitDifficulty` (RAC Difficulty page + Options+ gates)
- **Expansion config:** —

## Context

ME’s start-of-run menu warns that selections are permanent, then only offers a read-only viewer mid-run. Separately, **LOCK DIFFICULTY** gates Options+ Easy/Normal/Hard (and Hard → Battle Style Set) until the Hall of Fame — not the RAC Difficulty page fields themselves. We need a save-backed lock bit, clear mid-run edit rules for Phase 3 writers, and a non-release debug bypass (S17 inspector).

## Decision

1. **`rulesLocked`** is set by `MfRules_CommitAndLock()` when the run commits. Until Phase 3 lands, `MfRules_InitNewGame()` calls commit immediately (skip-menu permanence). **S19** must stop committing in init so the menu can edit; **S26 SAVE** calls `MfRules_CommitAndLock()`.

2. **Writers** use `MfRules_TrySetBool` / `MfRules_TrySetValue` (never raw save mutation from menus). Gameplay still **reads** via accessors regardless of lock.

3. **While unlocked** (pre-commit): every edit class is writable.

4. **While locked** (mid-run):
   - **Core** (Gamemode / Features / Randomizer / Nuzlocke / Challenges, including `pokeCenterLimit`): immutable.
   - **Difficulty page** (party limit, level cap, EXP mult, item bans, EVs/IVs scaling, catch rate, escape rope/Dig, hard EXP, less escapes): editable **iff** `lockDifficulty == FALSE`.
   - **Meta** (`rulesLocked`, `lockDifficulty`): never editable mid-run via TrySet — only commit / debug override. Players cannot turn LOCK DIFFICULTY off after commit.

5. **Debug override:** `MfRules_DebugSetUnlockOverride(TRUE)` (session RAM, non-persistent) makes all edit classes writable. Compiles to a no-op that returns FALSE under `NDEBUG` (`make release`). Overworld debug → **Modern FireRed…** → **Unlock rules (dbg)** toggles it (S17 expands this).

6. **ME Options+ difficulty:** our Difficulty-page exception mirrors ME’s “change difficulty whenever” *intent* against our rules Difficulty knobs. When Phase 10 adds Options+ battle difficulty, `lockDifficulty` should also gate that surface (same flag).

## Alternatives considered

- Lock all ModernRules fields forever; leave `lockDifficulty` unused until Options+ — closer to ME’s RAC permanence, but then S15’s “which rules may change mid-run” has no in-engine answer until Phase 10.
- Allow Challenges mid-run — rejected; ME treats those as permanent run identity.
- Persist debug unlock in the save — rejected; would let a release-adjacent path smuggle edits across load.

## Consequences

- Phase 3 menus must call `MfRules_CanEdit` / TrySet and grey rows when denied.
- S26 viewer is read-only; optional mid-run Difficulty editor (if we expose one) must check `MfRules_CanEdit(MF_RULE_EDIT_DIFFICULTY)`.
- S19 removes `MfRules_CommitAndLock()` from `InitNewGame`; S26 owns commit.
- Unit tests cover lock transitions, difficulty exception, and `#ifdef NDEBUG` debug path (`make check TESTS='MF: rules'`).
