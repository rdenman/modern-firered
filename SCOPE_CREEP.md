# Scope creep

Ideas that are **not** in [`STORIES.md`](./STORIES.md) yet. [`PROJECT.md`](./PROJECT.md) still wins if something here conflicts with it. Promote an item into a real story before implementing it.

---

## Altering Cave (Six Island)

The cave is already reachable with Rainbow Pass. Vanilla’s default table is **Zubat only**. Nine single-species tables exist; Mystery Gift was supposed to rotate `VAR_ALTERING_CAVE_WILD_SET`:

1. Zubat (default)
2. Mareep
3. Pineco
4. Houndour
5. Teddiursa
6. Aipom
7. Shuckle
8. Stantler
9. Smeargle

FireRed and LeafGreen lists match in this tree. Levels are low for a post-Rainbow-Pass cave (vanilla event data).

**Bug if we ever rotate:** `GetCurrentMapWildMonHeaderId` in `src/wild_encounter.c` only offsets Emerald’s `MAP_ALTERING_CAVE`. Six Island (`MAP_SIX_ISLAND_ALTERING_CAVE`) always hits the first header, so setting the var today would still yield Zubat. The Pokédex area screen’s altering-cave skip is also Emerald `MAPSEC_ALTERING_CAVE`, not `MAPSEC_ALTERING_CAVE_FRLG`.

Nuzlocke (ADR 0036) is per mapsec either way: one catch, then the cave is used.

### Options we discussed

1. **Flatten into one land table (preferred if we do this)**  
   Keep Zubat common; put the other eight in uncommon/rare slots. No NPC, no Mystery Gift, Dex/DexNav can show the full list. The cave stops “altering.” Fits “data-only remap of an existing map.”

2. **Keep nine tables, cycle on cave exit**  
   Fix the Six Island header lookup, wrap 0–8 when the player leaves. No NPC. Hunting one species means leaving and coming back. Better flavor, worse completeness unless the player knows the gimmick.

3. **NPC / Celio rumor / item to advance the set**  
   Recreates Mystery Gift. Extra story. Rejected for the ticket work so Celio isn’t a leftover-events dump.

4. **Do nothing (current)**  
   Map is walkable; event species stay locked. **This is the v1 choice.**

Do not bundle this with **S73** (Celio tickets after Sapphire).

---

## Yellow mode

Parked. Full research and a draft Y01–Y13 split live in [`YELLOW.md`](./YELLOW.md). Needs a scope ADR (story/map edits) before any of it becomes a `STORIES.md` item.

---

## Template for new entries

```
## Title

- **Status:** parked
- **Why not now:**
- **If we do it:**
```
