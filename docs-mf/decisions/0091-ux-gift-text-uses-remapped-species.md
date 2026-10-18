# 0091 — Gift nickname/obtain text uses the remapped species

- **Type:** ux
- **Status:** Accepted
- **Date:** 2026-10-17
- **Story:** — (follow-up to S52)
- **ME reference:** —
- **Expansion config:** `MF_RANDOMIZER` (runtime `randomStatic`)

## Context

S52 remaps `givemon` through `MfStaticGiftSpecies`, so the Celadon rooftop ball can be Vulpix. FireRed gift scripts still buffered the vanilla species (`SPECIES_EEVEE`) into `gText_NicknameThisPokemon` (`this {STR_VAR_1}`), so the prompt named Eevee.

## Decision

1. Player `givemon` / `ScriptGiveMonParameterized` writes the species actually given to `VAR_TEMP_TRANSFERRED_SPECIES` (`VAR_TEMP_1`).
2. `Common_EventScript_AskNicknameGiftMon` buffers that var before the Yes/No, so every FR gift that already uses the helper names the remapped mon.
3. Obtain fanfares that hardcoded EEVEE / LAPRAS (and Magikarp / fossil receive lines that re-buffer after `givemon`) read `VAR_TEMP_1` instead.
4. Gift NPC chatter peeks the same remap via `MfScript_StaticGiftSpecies` (`VAR_0x8004` in, remapped id out) and `{STR_VAR_1}`. `givemon` still uses the vanilla species so it is not mapped twice. Covers Silph, Magikarp salesman, fossils, Dojo pics/confirm, Game Corner confirm.
5. Game Corner Pokémon prize **list** is a `dynmultistack` filled by `MfScript_BufferGameCornerPrizeMenu` (FireRed Abra/Clefairy/Dratini/Scyther/Porygon slots + coin prices). Slot index is unchanged so `givemon` still uses the vanilla species. World NPCs that mention Magikarp/Eevee/Porygon in passing stay vanilla. The unused ROM `MULTI_GAME_CORNER_POKEMON_PRIZES` table is left alone (LeafGreen / merge).

## Alternatives considered

- **Rewrite every `bufferspeciesname SPECIES_*` by hand only** — rejected; Game Corner / Dojo already buffer `VAR_TEMP_1` after `givemon`, so one C write plus the shared nickname helper covers them.
- **Buffer from the last party slot in script** — rejected; PC gifts are not in the party; the transfer var already exists for that.

## Consequences

- Upstream `script_pokemon_util.c` (`VarSet` after gift remap). FR `*_Frlg` obtain/NPC strings that interpolated a hardcoded gift species now use `{STR_VAR_1}`.
- Dojo `showmonpic` shows the remapped species. Silph still says “good swimmer” even when the remap is not Water.
- Game Corner menu is slightly left-shifted (`left=6`) so 12-letter names fit.
- Oak lab `PLAYER_STARTER_NUM` cannot be `VAR_TEMP_1` — `givemon` overwrites it and Gary never walks. Slot lives in `VAR_TEMP_5`; `VAR_STARTER_MON` is copied before `givemon`.
