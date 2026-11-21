# 0125 — SHINY CHANCE "1" tier: every encounter is shiny

- **Type:** product
- **Status:** Accepted
- **Date:** 2026-11-21
- **Story:** —
- **ME reference:** — (FR-only; ME stops at 1/512)
- **Expansion config:** —

## Context
Players wanted an always-shiny run. SHINY CHANCE (ADR 0033) only went up to 1/512.

## Decision
Add a sixth SHINY CHANCE tier, `MF_SHINY_CHANCE_ALWAYS` (stored value 5, label **1**, description "Every encounter is Shiny."). The tier math (`SHINY_ODDS << tier`) would give 1/256 for tier 5, so `MfShinyOddsThresholdForChance` special-cases it to `MF_SHINY_THRESHOLD_ALWAYS` (65536). That is above every `GET_SHINY_VALUE`, the same value debug force-shiny uses.

Because every roll goes through `MfGetShinyOddsThreshold`, this covers wild, static, gift, starter, and egg Pokémon, and in-game trades (`OT_ID_PRESET`). Trainer Pokémon are not affected: `trainerMon->isShiny` overwrites them. `P_FLAG_FORCE_NO_SHINY` still wins.

The 4-bit `shinyChance` field already fits value 5, so the save layout and `MF_RULES_VERSION` don't change.

**Nuzlocke:** with Shiny Clause on, every encounter skips the one-per-area rule. That combination is allowed on purpose. Players who want area limits can turn Shiny Clause off.

## Alternatives considered
- **Separate "Always Shiny" toggle**: rejected. It is one more rule row that would just override SHINY CHANCE.
- **Block Always + Shiny Clause**: rejected. Shiny Clause is already the player's choice.

## Consequences
- Every wild battle plays the shiny sparkle animation.
