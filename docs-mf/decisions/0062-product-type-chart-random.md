# 0062 — Species types remap last; chart is a baked axis permutation

- **Type:** product
- **Status:** Accepted
- **Date:** 2026-10-09
- **Story:** S56
- **ME reference:** `GetTypeBySpecies` (`tx_Random_Type`) and `GetTypeEffectivenessRandom` / `RandomizeTypeEffectivenessListEWRAM` (`tx_Random_TypeEffectiveness`) in `src/pokemon.c`
- **Expansion config:** `MF_RANDOMIZER` (compile-out only); runtime `randomType` / `randomTypeEffectiveness`

## Context

S56 must randomize species typings and, separately, the type chart, with display consistency through `GetSpeciesType` / `GetTypeModifier` and S28’s “one pointer, then index” hot path. ME remaps each slot with `RandomSeededModulo(type + typeNum + species)` against the 18 real types, and remaps the old list-based chart by permuting type ids (`sTypeEffectivenessList`). Expansion already uses a 2D `uq4_12` matrix; per-lookup permutation would add two extra indexes on every matchup.

## Decision

1. **Species TYPE.** After Fairy-off and modern overlays (S27/S28), each combat slot hashes into the combat-type pool via S16 (`MF_RANDOM_CAT_TYPE`, location 0). Input id is `(species << 16) | (baseType << 8) | (slot + 1)` so the two slots of a monotype can diverge. `TYPE_NONE` / `MYSTERY` / `STELLAR` are left alone. When `fairyTypes` is off, Fairy is omitted from the pool (Sylveon included) so monotype / themed parties never see Fairy.
2. **EFFECTIVENESS.** Fisher–Yates shuffle of all 18 combat types (Fairy always included, matching ME) using `MF_RANDOM_CAT_TYPE_EFF`. Bake `table[atk][def] = base[inv[atk]][inv[def]]` into an EWRAM matrix and return that pointer from `MfGetTypeEffectivenessTable`. Base is Gen VI+ or ME Improved (S28). No extra `GetTypeModifier` hook. Cache on `(seed, typeEffectiveness)`.
3. **Independence.** TYPE and EFFECTIVENESS are separate flags. Move types stay vanilla (S54). Themed trainers still go through `GetSpeciesType` (ADR 0057).

## Alternatives considered

- **Permute indexes inside `GetTypeModifier`** — rejected; S28/S56 ask for a table pointer as fast as the static chart.
- **ME’s overflow-prone `NUMBER_OF_MON_TYPES-1` fill** (Mystery-only skip on expansion’s 21 ids) — rejected; skip NONE / MYSTERY / STELLAR explicitly.
- **Exclude Fairy from the chart shuffle when Fairy is off** — rejected; S27 keeps the Fairy chart, and ME always shuffles Fairy.

## Consequences

- ~882 bytes EWRAM for the baked chart. Changing the category ids, shuffle, or type pool is a mapping break (ADR 0016).
- S62 battle hints will already see randomized types and matchups through the existing helpers.
- Species-map dump logs `RType` / `RTypeE` plus Charizard / Clefairy types and Ground×Rock.
