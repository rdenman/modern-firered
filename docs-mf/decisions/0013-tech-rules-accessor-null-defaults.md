# 0013 — Rule accessor null defaults = Phase 1 baseline

- **Type:** tech
- **Status:** Accepted
- **Date:** 2026-09-12
- **Story:** S13
- **ME reference:** `IsNuzlockeActive` / direct `gSaveBlock1Ptr->tx_*` reads in `tx_randomizer_and_challenges.c` (API shape only)
- **Expansion config:** `MF_RULES_ENGINE` (`include/config/modern_firered.h`)

## Context

S13 needs a single, cheap read path for every later gameplay hook. Two failure modes must not change Phase 1 feel: compiling the rules engine out, and loading a save whose `mfRules.version` is missing or unrecognized. Meanwhile a *valid* empty struct (S12 `MfRules_ResetToEmpty`) must remain Classic-like zeros so S14 presets can define Classic vs Modern deliberately.

## Decision

- All gameplay reads go through `MfRules_GetActiveRules()` (or typed / `GetBool` / `GetValue` wrappers on top of it). Writers use `MfRules_GetSaveRules()` only.
- **Null behavior:** if `MF_RULES_ENGINE` is 0 **or** `version != MF_RULES_VERSION`, active rules are `gMfRulesPhase1Defaults` — reusable TMs, survive poison, modern Sitrus, modern types / Fairy / stats / moves / type chart on; randomizer / Nuzlocke / difficulty / challenges off (`monotype = 31`).
- A valid zeroed save is **not** null: accessors return the stored zeros (Classic-like). S14 owns filling presets on new game.
- Document the Phase 4 runtime-gate pattern (expansion config stays as capability; call site checks `MfRules_Has…()`) in [`docs-mf/RULES_ACCESSORS.md`](../RULES_ACCESSORS.md), with reusable TMs as the worked example.

## Alternatives considered

- Treat empty valid saves as Phase 1 defaults — rejected; would make Classic impossible without an explicit “off” encoding and blur S14 preset ownership.
- `#if MF_RULES_ENGINE` around every call site instead of accessor fallbacks — rejected; duplicates null policy and fights ADR 0003.
- Copy ME’s direct `gSaveBlock1Ptr->tx_*` reads — rejected; no null/version story and higher merge cost on SaveBlock1.

## Consequences

- Disabling `MF_RULES_ENGINE` keeps Phase 1 play without rewriting battle/item call sites that already use accessors.
- Pre-S12 / corrupt `mfRules` bytes are safe to ignore until S64 migrates; do not trust raw save fields in gameplay.
- S14 must apply Classic/Modern/Custom presets after reset so new games are not stuck on empty Classic zeros once Phase 4 wires behavior.
- Hot paths should prefer typed helpers over `MfRules_GetBool`’s switch.
