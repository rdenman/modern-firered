# 0052 — BST equalizer scales totals, after modern stats

- **Type:** product
- **Status:** Accepted
- **Date:** 2026-10-05
- **Story:** S49
- **ME reference:** `tx_Challenges_BaseStatEqualizer` in `CalculateMonStats` (`src/pokemon.c`)
- **Expansion config:** —

## Context

ME’s Challenges row is labeled `BST EQUALIZER` (Off / 100 / 255 / 500) but its code sets **every** base stat to that number (HP and the five others each become 100 / 255 / 500). That is a flat-stat challenge, not an equalized BST. S49’s scope asks to normalize each species’ **base stat total** to the chosen value while keeping the relative distribution, and to compose with S29’s modern/classic table.

`GetSpeciesBase*` already centralizes S29. Putting the equalizer in `CalculateMonStats` only would leave the dex and AI BST disagreeing with battle.

## Decision

1. **Proportional BST**, not ME’s per-stat flatten. Mode 1/2/3 scales the six S29 stats so they sum to 100 / 255 / 500 (largest remainder). Relative order is preserved. Menu copy says “scaled to a total of N”.
2. **Order:** S29 classic/modern lookup first, then equalize. Off skips the scale.
3. **One hook:** `MfGetSpeciesBaseStat` (already used by `GetSpeciesBase*` / `GetSpeciesBaseStatTotal` / `CalculateMonStats`). Dex, summary, and battle stay consistent.
4. **Shedinja:** raw HP 1 is left at 1; the other five stats share `target - 1` so Wonder Guard is not paired with a real HP stat and the displayed total still matches.
5. **u8 cap:** any scaled stat above 255 is clamped and the leftover is redistributed so the total still matches (Chansey HP at 500).
6. **Debug:** cycling Challenges **BSE** calls `MfRecalculatePartyStats()`, same as the modern-stats toggle.

## Alternatives considered

- ME’s “each base stat = N” — rejected; contradicts S49 and the row’s BST name.
- Branch only in `CalculateMonStats` — rejected; summary / dex / AI would show unscaled tables.
- Equalize before S29 — rejected; modern vs classic would be overwritten to the same totals *and* the same shape once scaled from mixed sources. Applying after S29 keeps Butterfree’s extra Sp. Atk as a larger share of the equalized total.

## Consequences

- 100-BST Magikarp and 100-BST Charizard are both frail, but Charizard still looks like Charizard.
- `CalculateMonStats` uses `u32` for the base-stat term so a pre-clamp value cannot wrap if a future target exceeds 255 on a non-HP stat.
- Randomizer balancing (S51) that reads `GetSpeciesBaseStatTotal` will see the equalized total when this challenge is on.
