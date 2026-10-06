# 0055 — Wild tables remap at read/create; statics skip Oak

- **Type:** tech
- **Status:** Accepted
- **Date:** 2026-10-05
- **Story:** S52
- **ME reference:** `CreateWildMon` / `CreateScriptedWildMon` / `ScriptGiveMon` in ME `wild_encounter.c` + `script_pokemon_util.c`; `tx_Random_WildPokemon` / `tx_Random_Static`
- **Expansion config:** `MF_RANDOMIZER` (compile-out only)

## Context

S52 must apply S51’s mapper to wilds (on top of S32’s vanilla/modern header choice) and to statics (scripted legendaries, fossils, gifts). ME remaps only inside `CreateWildMon` / gift helpers, so the Pokédex area screen still lists vanilla species. Phase 9 requires display consistency. Starters are S53. ME skips gift remap until `FLAG_SYS_POKEMON_GET`; FireRed sets that flag before Oak’s `givemon`, so we skip while the party is empty instead. Unown chambers are a Kanto puzzle; remapping them would break Ruins of Alph.

## Decision

1. **Wild species** — `MfWildEncounterSpecies` / `MfWildSlotSpecies` wrap `MfSpeciesMapActive(..., WILD, mapsec)` on the species taken from `MfGetActiveWildMonHeaders()` (S32). `CreateWildMon` remaps with the current mapsec so grass/surf/fish/rock/outbreak/DexNav battles stay randomized. Pokédex area, DexNav icons/names, match-call names, and ambient cries remap at read time. DexNav still *generates* from the original slot so `CreateWildMon` is not applied twice.
2. **Unown** — identity. Letter forms are not remapped.
3. **Static** — `MfStaticEncounterSpecies` for `setwildbattle` / `CreateEnemyEventMon`. Player `givemon` / `ScriptGiveMon` use `MfStaticGiftSpecies`, which is identity while the party is empty (FR Oak sets `FLAG_SYS_POKEMON_GET` *before* `givemon`, so ME’s flag check would remap the starter). Eggs are not remapped. In-game NPC trades stay vanilla, matching ME.
4. **No RAM copies** of wild tables. Mapping is a pure function of seed + slot species + mapsec, so revisits and save/load are stable without rewriting `.rodata`.

## Alternatives considered

- **ME-only `CreateWildMon` hook** — rejected; Pokédex area / DexNav would show stock species.
- **Remap every `.species` read and also `CreateWildMon`** — rejected; DexNav would double-map.
- **EWRAM copies of remapped headers** — rejected; too large and unnecessary given a deterministic mapper.

## Consequences

- Upstream call sites: `wild_encounter.c`, `script_pokemon_util.c`, `pokemon.c` (`CreateEnemyEventMon`), `pokedex_area_screen.c`, `dexnav.c`, `match_call.c`.
- Chaos (S57) still goes through `MfSpeciesMapActive`; do not call `Random()` here.
- DexNav search-level stats stay keyed on the *original* slot species.
