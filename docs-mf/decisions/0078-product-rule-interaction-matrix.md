# 0078 — Rule interaction matrix: one forbid, otherwise warn

- **Type:** product
- **Status:** Accepted
- **Date:** 2026-10-11
- **Story:** S65
- **ME reference:** ME has no pair legality checker; `CheckConditions` only greys nested randomizer / Nuzlocke / Custom rows
- **Expansion config:** —

## Context

S65 needs a documented matrix of interacting rules and a menu policy: allow, warn, or forbid. Most listed combos (monotype + randomizer, Nuzlocke + no Centers, chaos + Nuzlocke, level cap + hard EXP, BST + modern stats) are playable by design. One combo is actually unwinnable: Fairy monotype while Fairy Type is Off (Classic, or Custom with Fairy Off) leaves zero legal species.

## Decision

1. **Forbid only Fairy monotype + Fairy Type Off.** The last menu change is reverted (`SE_FAILURE`) and the description explains it. SAVE also refuses if that state is ever present.
2. **Warn** (keep the combo, description + SAVE “anyway”) for: monotype + randomizer; monotype + Nuzlocke; Nuzlocke + no Centers; Nuzlocke + no Centers + no PC heal; chaos + Nuzlocke; Nuzlocke + EXP ×0; Evo Limit All + random evo options.
3. **Allow** (documented, no UI) for level cap + HARD MODE EXP, BST equalizer + modern stats, solo Nuzlocke, item randomizer (HM pins), and chaos vs balancing (already gated).
4. Implement as a pure checker on `struct ModernRules` (`MfRules_EvaluatePair` / `MfRules_FindConflict`). Do not migrate or add save fields.

## Alternatives considered

- **Forbid chaos + Nuzlocke** — rejected; some players want it, and ADR 0063 already keeps maps deterministic aside from encounter salt.
- **Grey Fairy on the monotype list when Fairy is Off** — rejected for v1; cycling 19 values and reverting one illegal choice matches the other pages’ live writes.
- **Warn at SAVE only** — rejected; the story asks for selection-time feedback.

## Consequences

- Classic + Fairy monotype cannot be committed. Turn Fairy Type On (Custom) or pick another type first.
- Harsh but winnable stacks stay selectable; S66 playthroughs should still try Nuzlocke + no Centers and monotype + randomizer.
- Table lives in [`docs-mf/RULE_INTERACTIONS.md`](../RULE_INTERACTIONS.md).
