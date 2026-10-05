# 0050 — Evolution limit (Off / First / All)

- **Type:** product
- **Status:** Accepted
- **Date:** 2026-10-04
- **Story:** S47
- **ME reference:** `EvolutionBlockedByEvoLimit` in `src/pokemon.c`; Challenges `EVO LIMIT` 0/1/2
- **Expansion config:** —

## Context

S47 wires ME’s Challenges row `EVO LIMIT`: Off, first evolution only, or no evolution. Every trigger must honor it (level-up, stone, trade, bag item, scripts). ME’s helper only blocks `EVO_TYPE_1` when the setting is First; All (`== 2`) is never tested there. Copying ME’s `gSpeciesMapping` table would also fight expansion’s species list and future randomizer evo work (S55).

## Decision

1. **One hook** in `GetEvolutionTargetSpecies` via `MfIsEvolutionBlockedByLimit`. That covers battle level-up, Rare Candy, stones, Linking Cord / held evo items, trade, and `EVO_MODE_SCRIPT_TRIGGER`.
2. **First** = block if the species already has a pre-evolution (`GetSpeciesPreEvolution` ≠ none). Base/baby forms may evolve once; middle and final stages may not. No static stage table.
3. **All** = block every evolution. Implement it even though ME’s function does not; the menu text and story require it.
4. **Bag message** when the used item is an `EVO_ITEM` for a blocked species. Automatic triggers (battle, trade, friendship) skip the scene with no extra line, same as Everstone.

## Alternatives considered

- Port ME’s `gSpeciesMapping` — rejected; incomplete for expansion, merge-heavy, and All still would not work without extra code.
- Count remaining evolutions instead of pre-evolutions — rejected; a caught Ivysaur would still be allowed to become Venusaur, which is not “first evolution only.”
- Dialogue after every blocked level-up — rejected; it would spam Magikarp/Ivysaur grinding. Bag use is the path that needs a clear refusal (S69).

## Consequences

- `GetEvolutionTargetSpecies` is the composition point for S69 and later S55. Do not special-case Linking Cord.
- First treats Pichu → Pikachu as the allowed step and blocks Pikachu → Raichu, matching ME’s Pikachu = stage 1 mapping.
- `GetSpeciesPreEvolution` is a full species scan; it runs only when the rule is First.
