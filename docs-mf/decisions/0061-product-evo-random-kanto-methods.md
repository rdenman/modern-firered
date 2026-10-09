# 0061 — Evolution remap + Kanto-reachable methods

- **Type:** product
- **Status:** Accepted
- **Date:** 2026-10-09
- **Story:** S55
- **ME reference:** `GetEvolutionTargetSpecies` species-swap (`TX_RANDOM_T_EVO` / `TX_RANDOM_T_EVO_METH`) in `src/pokemon.c`
- **Expansion config:** `MF_RANDOMIZER` (compile-out only)

## Context

ME remaps the *source* species before reading `gEvolutionTable` (EVO LINES) and the *target* after a method matches (EVOLUTIONS). Expansion stores per-species `struct Evolution` lists with conditions ME never had (Hoenn mapsecs, RTC, Ice Stone, Dawn Stone, move counters). Copying those tables onto Kanto mons is the softlock the story forbids. S47’s First-limit walks pre-evolutions; daycare eggs walk the chain backwards. Both must keep vanilla families.

## Decision

1. **Single hook.** `GetSpeciesEvolutions` returns a remapped EWRAM copy so the Pokédex, summary, bag stones, and `GetEvolutionTargetSpecies` agree. Location key is 0 (ME never map-bases evo).
2. **EVO LINES** — donor is `MfSpeciesMapEx(src, CAT_EVO_METH)` (Balancing / Legendaries as S51). Copy that donor’s *vanilla* methods and targets.
3. **EVOLUTIONS** — each remaining target is `MfSpeciesMapEx(target, CAT_EVO)`.
4. **Sanitize every generated row** (either flag): keep `EVO_LEVEL` / `EVO_ITEM` / `EVO_TRADE`; rewrite night→Moon Stone, day→Sun Stone; mapsec/map/region/weather/steps and other battle-only methods → Linking Cord (or keep an already-valid stone); unobtainable stones (Ice / Dawn / Dusk / Shiny / …) → Linking Cord. Extra conditions that Kanto cannot satisfy are dropped. Obtainable items are Celadon 4F (ADR 0035) plus Fire/Water/Thunder/Leaf/Moon/Sun stones.
5. **Vanilla families elsewhere.** `GetSpeciesPreEvolution`, `GetEggSpecies`, Nuzlocke dupes, and S51 stage/pool walks use `MfGetVanillaSpeciesEvolutions` so First-limit and eggs do not follow the remapped graph.

## Alternatives considered

- **ME’s GetEvolutionTargetSpecies-only swap** — rejected; dex would still show vanilla lines (Phase 9 display rule).
- **Leave Hoenn mapsec / RTC conditions** — rejected; Magneton→Magnezone and Espeon/Umbreon would be unwinnable on a FR cart.
- **Remap pre-evo / egg chains** — rejected; S47 First would let Ivysaur evolve whenever nobody maps *into* Ivysaur.

## Consequences

- Vanilla Magnezone-on-New-Mauville stays broken until a randomizer evo flag is on (ADR 0008).
- Changing the item whitelist or sanitize rules is a mapping break (same seed, new methods) — treat like ADR 0016.
- A later story can stock Ice/Dawn stones if we want those methods without rewriting them to Linking Cord.
