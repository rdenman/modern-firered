# Agent notes — Modern FireRed

This repo is a **FireRed** hack based on [rh-hideout/pokeemerald-expansion](https://github.com/rh-hideout/pokeemerald-expansion), built with `make firered -j$(sysctl -n hw.ncpu)`. The goal is a modular, Modern Emerald–style rules/QoL layer on **stock Kanto** — not a story/map redesign.

Read [`PROJECT.md`](./PROJECT.md) for product scope. Prefer that over inventing new world content.
Read [`STORIES.md`](./STORIES.md) for the ordered implementation backlog. Work stories top to bottom; keep their status markers up to date.

## Build

Full pin table and switch hazard: [`BUILDING.md`](./BUILDING.md).

```bash
make firered -j$(sysctl -n hw.ncpu)
```

- Output: `pokefirered.gba` (open in mGBA).
- Do **not** change Makefile defaults so bare `make` builds FireRed — that causes upstream merge pain. Always pass `firered`.
- After switching between Emerald and FireRed builds in the same tree: `make clean` then rebuild.
- Needs `python3` on `PATH` (Makefile invokes it directly; pin is `.python-version` → 3.12).

## Upstream

- Upstream remote should point at `https://github.com/rh-hideout/pokeemerald-expansion.git`.
- Standalone `cawtds/pokefirered-expansion` is obsolete for new work; do not base features on it.
- When porting systems, treat [Modern Emerald](https://github.com/resetes12/pokeemerald) (`resetes12/pokeemerald`) as a **spec + reference implementation**, not a merge source. FR/expansion APIs differ; rewrite call sites.

## Implementation priorities

1. Flip/use expansion configs that already exist (phys/spec, Fairy, summary IV/EV, reusable TMs, battle speed QoL).
2. Port ME’s start-of-game **options/rules engine** (save flags + Oak-speech menu): Gamemode, Features, Randomizer, Nuzlocke, Difficulty, Challenges.
3. Wire those flags into encounters, faint handling, shops, EXP, items, etc.
4. Port ME-style **Options+ QoL** (Even Faster Joy, autorun, fast intro/battles, bag sort UX, summary EV/IV).

## Do not do (unless explicitly asked)

- Story edits, new maps/dungeons, legendary dungeon content
- Followers / music packs
- Battle Frontier ports
- Renaming Makefile default build target to FireRed

## Key ME references

| ME area | Approx. location in ME | Purpose |
|---|---|---|
| Challenge/randomizer menu | `src/tx_rac_menu.c`, `tx_rac_viewer.c` | Start-of-game UI |
| Rules logic | `src/tx_randomizer_and_challenges.c` | Nuzlocke/randomizer/challenge hooks |
| Save flags | `tx_*` fields in `SaveBlock1` / `global.h` | Persistence |
| Options+ QoL | `src/options_plus_menu.c` | Even Faster Joy, autorun, phys/spec toggle, fast battles |

Expansion already has many battle/Pokémon/QoL primitives under `include/config/` (e.g. `battle.h`, `summary_screen.h`, `item.h`). Prefer enabling/configuring those before reinventing.
