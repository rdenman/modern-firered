# Yellow Mode — Research & Planning Notes

Research notes for an optional **Pokémon Yellow–style mode** in Modern FireRed. This is the input for a future planning session that turns it into a story backlog, like [`STORIES.md`](./STORIES.md). Nothing here is implemented yet.

> **Scope warning:** [`PROJECT.md`](./PROJECT.md) and [`AGENTS.md`](./AGENTS.md) currently forbid story edits, new map content and followers. Yellow mode is mostly story and map edits, so the first planning step is to formally widen scope with a decision doc in `docs-mf/decisions/`, and update `PROJECT.md` / `AGENTS.md`.

---

## 1. Goal

A start-of-game option that makes the run follow the Yellow pattern:

- Start with **Pikachu**; the rival takes **Eevee**.
- All three Kanto starters (**Bulbasaur, Charmander, Squirtle**) can be obtained during the game.
- **Jessie & James** (plus Meowth) are recurring Team Rocket fights.
- Everything else (rules engine, QoL, randomizer, Nuzlocke) keeps working.

Optional, lower priority: Yellow gym leader / trainer teams, Yellow wild tables, Pikachu refusing to evolve. Explicitly out unless separately approved: a following Pikachu, the Surfing Pikachu minigame, Pikachu's Beach, printer stuff.

---

## 2. Overall difficulty

| Piece | Difficulty | Main cost |
|---|---|---|
| Option flag in rules menu | Easy | One save bit + menu entry |
| Pikachu starter / rival Eevee | Easy–Medium | Lab script + one new set of rival teams |
| Eevee evolution branching | Medium | Track win/loss of early fights; 3× late rival teams |
| Gift starters (3 NPCs) | Medium | New NPCs/scripts on existing maps |
| Jessie & James | Hard | New art, trainer class, 4 cutscenes, dialogue |
| Yellow gym/trainer/wild data | Medium, tedious | Lots of data entry; testing |

Every Yellow change needs two code paths (on/off), so testing roughly doubles.

---

## 3. Yellow reference facts

Facts from memory of Gen 1 Yellow. **Verify each one on [Bulbapedia](https://bulbapedia.bulbagarden.net/wiki/Pok%C3%A9mon_Yellow_Version) before implementing.**

### Opening
- Oak stops the player in the Route 1 grass, catches a wild Pikachu, and gives it to the player (Lv 5).
- The rival grabs the Poké Ball on the table, which turns out to be Eevee (Lv 5).
- The first rival battle is in the lab (Pikachu vs Eevee).

### Rival Eevee evolution
- Eevee's final form depends on the results of the **lab battle** and the **first Route 22 battle**. It ends up as Jolteon, Flareon or Vaporeon. **Look up the exact win/loss → evolution table; don't trust memory here.**
- Eevee evolves partway through the game. Check at which rival battle it first appears evolved; likely around the S.S. Anne or Pokémon Tower fight.
- The rival's later teams differ per evolution, and the rest of his team changes with it (Yellow has 3 rival team variants, keyed by evolution, not by player starter).

### Gift starters
| Pokémon | Where | Condition (verify) |
|---|---|---|
| Bulbasaur | Cerulean City house (girl, "Melanie") | Pikachu must be friendly enough |
| Charmander | Route 24 (trainer, "Damian") | Talk to him; he gives it away |
| Squirtle | Vermilion City (Officer Jenny) | After beating Lt. Surge |

Since FireRed has no Pikachu friendship mechanic tied to the story, Bulbasaur's condition needs a decision: use regular friendship, a badge gate, or drop the condition.

### Jessie & James fights (verify locations and teams)
- **Mt. Moon** (B2F area)
- **Rocket Hideout** (near Giovanni)
- **Pokémon Tower** (top floor, before Mr. Fuji)
- **Silph Co.** (a mid/upper floor)
- Teams scale roughly Ekans / Koffing / Meowth → Arbok / Weezing / Meowth.
- In Yellow they're one "ROCKET" trainer showing both of them on one sprite. They recite the motto before each fight.

### Other Yellow changes (optional tier)
- Gym leaders use different teams (anime-inspired: e.g. Surge with Raichu, Sabrina with Abra/Kadabra/Alakazam, Blaine with Ninetales/Rapidash/Arcanine).
- Many trainer and wild tables change; some species are unavailable in the wild.
- Pikachu refuses the Thunder Stone (can't evolve).

---

## 4. Codebase map (where the work would land)

### Starter & rival
- **Lab script:** `data/maps/PalletTown_ProfessorOaksLab_Frlg/scripts.inc`
  - Starter choice sets `PLAYER_STARTER_SPECIES` / `RIVAL_STARTER_SPECIES`, then `givemon PLAYER_STARTER_SPECIES, 5`.
  - Sets `VAR_STARTER_MON` (0 = Charmander, 1 = Bulbasaur, 2 = Squirtle per the branch on lines ~299–301).
- **`VAR_STARTER_MON` users in FR maps** (all branch rival battles by starter):
  - `PalletTown_ProfessorOaksLab_Frlg`, `Route22_Frlg`, `CeruleanCity_Frlg`, `SSAnne_2F_Corridor_Frlg`, `PokemonTower_2F_Frlg`, `SilphCo_7F_Frlg`, `PokemonLeague_ChampionsRoom_Frlg`
  - C code: `src/battle_setup.c` (~line 1012), `src/field_specials.c`, `src/credits.c`
  - Defined in `include/constants/vars_frlg.h`
  - (Emerald maps like `Route103`, `RustboroCity` also use it; ignore them.)
- **Rival trainer data:** `src/data/trainers_frlg.party` has 21 rival entries, 7 battles × 3 starters:
  - `TRAINER_RIVAL_OAKS_LAB_*`, `ROUTE22_EARLY_*`, `CERULEAN_*`, `SS_ANNE_*`, `POKEMON_TOWER_*`, `SILPH_*`, `ROUTE22_LATE_*`
  - Plus `TRAINER_CHAMPION_FIRST_*` and `TRAINER_CHAMPION_REMATCH_*` (3 each).
  - Yellow mode needs a new set keyed by Eevee evolution instead: lab + Route 22 early as Eevee, then 3 variants (Jolteon/Flareon/Vaporeon) for each later fight.
- **Idea:** reuse `VAR_STARTER_MON` with new values (e.g. 3 = Eevee/undetermined, 4/5/6 = Jolteon/Flareon/Vaporeon paths) so existing branch points only need one extra case each. Needs a check that nothing else assumes 0–2.

### Rules engine (option toggle)
- `include/mf_rules.h` → `struct ModernRules` in SaveBlock3.
  - Free bits exist (e.g. `padding0:5` at 0x02). Adding a field means following the additive-only rule and `MF_RULES_VERSION` / migration notes there.
- Menu: the Gamemode page (ME `tx_Mode_*` equivalent). See `docs-mf/RULES_ACCESSORS.md` and ADRs 0012–0016.
- The flag must be committed before `NewGameInitData()` touches anything starter-related (same ordering issue noted in `STORIES.md` S19).

### Gift NPCs & Jessie/James maps (all exist)
- Gift starters: `CeruleanCity_House1..5_Frlg` (pick one), `Route24_Frlg`, `VermilionCity_Frlg`
- Jessie & James: `MtMoon_B2F_Frlg`, `RocketHideout_B4F_Frlg`, `PokemonTower_7F_Frlg`, `SilphCo_*F_Frlg`
- NPCs can be hidden/shown per mode with event flags on the object events (standard decomp pattern).

### Battles
- `asm/macros/event.inc` has `trainerbattle_single`, `trainerbattle_double`, and `trainerbattle_two_trainers` (two separate opponents). That gives two options for Jessie & James:
  1. **One trainer, combined sprite** (Yellow-accurate, simplest scripting; needs a merged 64×64 sprite).
  2. **Two trainers** via `trainerbattle_two_trainers` (separate sprites; more "modern" double battle).
- Existing Rocket trainers to look at for patterns: `TRAINER_TEAM_ROCKET_ADMIN*`, `TRAINER_BOSS_GIOVANNI*` in `trainers_frlg.party`.

### Overworld config
- `include/config/overworld.h`: `OW_POKEMON_OBJECT_EVENTS TRUE` (Pokémon can be overworld NPCs, so a Meowth NPC is free), `OW_FOLLOWERS_ENABLED FALSE` (followers off; keep it that way unless scope changes).

### Interactions with existing rules (need decisions)
- **Randomizer (S53, `randomStarter`):** Does Yellow mode randomize Pikachu/Eevee, or does Yellow win? Do gift starters get randomized as statics?
- **Nuzlocke (ADR 0036):** Gifts and the starter are excluded from area locking today; confirm gift starters stay excluded.
- **Monotype/type challenges:** Pikachu must be legal under the chosen type, or Yellow mode and monotype are mutually exclusive.
- **Level caps:** If gym teams change, caps in `src/caps.c` may need Yellow values.

---

## 5. Sprites & art

### What's needed
| Asset | Format | Notes |
|---|---|---|
| Jessie & James battle sprite(s) | 64×64, 4bpp, ≤16 colors (index 0 = transparent) | One combined or two separate |
| Jessie & James overworld sprites | 16×32 frames, walk cycle in 4 directions | Needs its own palette |
| Meowth overworld | — | Already available via `OBJ_EVENT_GFX_SPECIES(MEOWTH)` |
| Trainer class name / music | — | New "ROCKET" class or reuse Team Rocket class |

Battle sprite files go in `graphics/trainers/front_pics/` + `graphics/trainers/palettes/`, registered in `src/data/graphics/trainers.h`, `include/graphics.h`, and the front pic tables in `src/data/trainer_graphics/`. Expansion has an upstream guide: [how_to_trainer_class.md](https://github.com/rh-hideout/pokeemerald-expansion/blob/master/docs/how_to_trainer_class.md).

### Community sources (fan-made; credit the artists)
- **Best match:** [Looking for Trainer & Overworld Sprites of Jesse & James](https://www.pokecommunity.com/threads/looking-for-trainer-overworld-sprites-of-jesse-james.404892/) (PokéCommunity, 2018). User "kalarie" posted `Overworld Sprites.zip` and `Trainer Sprites.zip`. **Not yet downloaded or inspected.** May need a login.
- [Fire Red Overworld Sprite Resource](https://www.pokecommunity.com/threads/fire-red-overworld-sprite-resource.407124/) has an "Anime Overworld Sprites" section (credited to kalarie). Free with credit; some need custom palettes.
- [Accurate FRLG-style NPC Megapack](https://eeveeexpo.com/resources/823/) (Eevee Expo) collects many FRLG-style sprites; formatted for RPG Maker, so it needs repacking.
- [Accurate FireRed Overworld Sprite Resource](https://www.pokecommunity.com/threads/the-accurate-firered-overworld-sprite-resource.361337/) is generic trainer classes, useful for style reference.

### Licensing
These are fan works of Nintendo/Game Freak designs. There's no formal license; the community norm is "free with credit." Add credits to the project's credits/README when used.

### AI-generated fallback
Possible for battle-sprite placeholders (generate → downscale → quantize to 16 colors → hand cleanup in Aseprite). Overworld walk cycles are poor candidates. Prefer the community sprites.

---

## 6. Open decisions for the planning session

1. **Scope:** Approve widening `PROJECT.md` to include Yellow mode story/map edits? (Decision doc.)
2. **Toggle location:** Gamemode page entry vs. separate "Version: FireRed / Yellow" choice before the rules menu?
3. **Opening:** Replicate the Route 1 Oak-catches-Pikachu cutscene, or keep the lab and swap the balls on the table? (Swapping is far cheaper.)
4. **Eevee evolution:** Replicate Yellow's win/loss branching, or pick one evolution (or randomize it)?
5. **Bulbasaur condition:** Friendship gate, badge gate, or none?
6. **Jessie & James format:** One combined trainer (Yellow-accurate) or two-trainer battle?
7. **Which Jessie & James fights:** All four locations, or start with Mt. Moon only as a vertical slice?
8. **Optional tier:** Include Yellow gym teams / trainer teams / wild tables? Pikachu Thunder Stone refusal?
9. **Rules interactions:** Randomizer, Nuzlocke, monotype, level caps (see §4).
10. **Champion/rematch & Sevii:** Rival's champion team and any postgame rival references must handle the Eevee paths.

---

## 7. Suggested story order (draft)

A starting point for the backlog; refine during planning.

1. **Y01 — Scope decision & toggle:** ADR + `PROJECT.md`/`AGENTS.md` update; add Yellow-mode bit to `ModernRules`, menu entry, accessor, debug display.
2. **Y02 — Pikachu starter / rival Eevee:** Lab script branch; Pikachu given at Lv 5; rival takes Eevee; lab battle uses new trainer entry.
3. **Y03 — Rival team set (Eevee paths):** New rival entries for all 7 battles + champion/rematch; extend `VAR_STARTER_MON` branching in every FR map/C file listed in §4.
4. **Y04 — Eevee evolution branching:** Record lab and Route 22 results; choose the path; later fights pick the matching team.
5. **Y05 — Gift Charmander (Route 24).**
6. **Y06 — Gift Squirtle (Vermilion, post-Surge).**
7. **Y07 — Gift Bulbasaur (Cerulean, condition per decision).**
8. **Y08 — Jessie & James art integration:** Import/fix sprites, new trainer pic(s), overworld graphics + palette, trainer class.
9. **Y09 — Jessie & James: Mt. Moon** (vertical slice: cutscene, motto, battle, exit).
10. **Y10 — Jessie & James: Rocket Hideout, Pokémon Tower, Silph Co.**
11. **Y11 (optional) — Yellow gym leader teams + level cap values.**
12. **Y12 (optional) — Yellow trainer / wild tables; Pikachu Thunder Stone refusal.**
13. **Y13 — Rules interaction pass:** randomizer, Nuzlocke, monotype, tests.

Each story follows the normal bar from `STORIES.md`: clean `make firered` build, in-game check with the mode **on and off**, and unit tests where logic exists (e.g. evolution path selection).
