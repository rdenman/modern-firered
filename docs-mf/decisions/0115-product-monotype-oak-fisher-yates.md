# 0115 — Monotype Oak starters use a real Fisher–Yates

- **Type:** product
- **Status:** Accepted
- **Date:** 2026-11-01
- **Story:** S48 follow-up
- **ME reference:** `PickRandomStarterForOneTypeChallenge` / `ShuffleListU16` (rejected)
- **Expansion config:** —

## Context

ADR 0051 copied ME’s Oak remap: shuffle `sRandomSpeciesEvo0` with one `ISO_RANDOMIZE1` draw from the low 16 bits of the OT ID, then reuse that same index for every swap. That is not a shuffle. Bug (and other types whose unevolved species sit early in the list) almost always rolled Caterpie / Weedle / Paras even on a fresh New Game.

## Decision

1. Keep the same unevolved pool and “first type match not already in the trio” pick, still resolved slot 0 then 1 then 2.
2. Replace the one-draw swap with Fisher–Yates. Each swap `i` uses `MfRandom_Modulo(full OT ID, MF_RANDOM_CAT_MONOTYPE_STARTER, i, slot, i+1)`.
3. Seed with the **full 32-bit OT ID**, not the randomizer seed, so ONE TYPE ONLY does not require the randomizer to be on.

## Alternatives considered

- **Keep ME’s shuffle** — rejected; it is biased enough that New Game feels stuck.
- **Curated “sensible starter” rows** — still rejected (ADR 0051); Scyther/Pinsir/Heracross stay in the pool, they just get a fair roll.
- **Key off `randomizerSeed`** — rejected; monotype-only runs would share or lack that seed.

## Consequences

- Existing monotype saves change their Oak trio the next time the balls are resolved (new files; already-chosen starters are already in the party).
- `MF_RANDOM_CAT_MONOTYPE_STARTER` is hash-domain only (`LocationKey` stays 0; species remap ignores it).
