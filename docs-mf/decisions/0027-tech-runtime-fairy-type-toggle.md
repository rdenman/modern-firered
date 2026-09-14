# 0027 — Runtime Fairy retypes via GetSpeciesType fallback table

- **Type:** tech
- **Status:** Accepted
- **Date:** 2026-09-13
- **Story:** S27
- **ME reference:** `GetTypeBySpecies` + `types_old` when `tx_Mode_Fairy_Types == 0` in `src/pokemon.c`
- **Expansion config:** `P_UPDATED_TYPES` (compile-time Fairy-on data); runtime `MfRules_HasFairyTypes()` / `fairyTypes`

## Context

Phase 1 baked Gen-6 Fairy typings into `gSpeciesInfo` via `P_UPDATED_TYPES >= GEN_6`. The Gamemode **FAIRY TYPE** toggle must reverse those retypes per save without forking species tables or inventing a compile-time `MF_FAIRY`. ME stores dual `types` / `types_old` on every species; expansion has no `types_old` field, and adding one would bloat `SpeciesInfo` and fight upstream merges.

## Decision

1. **Keep species_info modern** (`P_UPDATED_TYPES` unchanged). Fairy-on reads `gSpeciesInfo[].types` as today.
2. **Fallback table** in `src/mf_types.c` lists the Gen-6 Fairy retypes with their pre-Gen-6 typings (ME’s `types_old` values). When `!MfRules_HasFairyTypes()`, `MfGetSpeciesType` returns the fallback; otherwise the modern types.
3. **Single upstream hook:** `GetSpeciesType` in `src/pokemon.c` calls `MfGetSpeciesType` (one-line body). Dex, summary, battle setup, and type icons all already go through that helper — display stays consistent.
4. **Species set** matches ME’s 20 Fairy retypes, plus **Cotonee / Whimsicott** (same official Gen-6 retype class; present in Emerald TESTELF). Pure Fairy introductions (Sylveon, Alolan Ninetales, megas that were always Fairy) are **not** remapped.
5. **Fairy moves and the type chart stay.** Off only means “don’t add Fairy to Gen-6 retypes.” Chart row/column and Fairy move types remain for S28’s TYPE CHART / modern-types work (matches ME).

## Alternatives considered

- Add `types_old[2]` to `SpeciesInfo` like ME — rejected; large upstream struct diff and ROM cost for ~20 rows.
- `#if` fork every Fairy species entry — rejected; compile-time, not player-toggleable (ADR 0003).
- Remap Fairy moves to Normal / strip Fairy chart when off — rejected; ME does not; menu copy is about Pokémon typings; S28 owns chart toggles.
- Remap Sylveon / other Fairy-native species when off — rejected; no pre-Gen-6 typing; ME leaves them Fairy.

## Consequences

- Hot path: one `MfRules_HasFairyTypes()` check plus a 22-entry linear scan only when Fairy is off — cheap vs damage math.
- S28 can extend `MfGetSpeciesType` (or a sibling) for `modernTypes` without a second `GetSpeciesType` hook.
- S49 monotype must call `GetSpeciesType` (already gated) so Fairy-off legality matches battle/UI.
- Upstream merges that change Gen-6 Fairy retypes must update `sFairyTypeFallbacks` in `mf_types.c`.
