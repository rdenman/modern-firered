# 0060 — Level-up remap + form-locked ability ban; TMs stay vanilla

- **Type:** product
- **Status:** Accepted
- **Date:** 2026-10-08
- **Story:** S54
- **ME reference:** `GetRandomMove` / `GetAbilityBySpecies` in `src/pokemon.c`; `sRandomValidMoves`; `TX_RANDOM_T_MOVES` / `TX_RANDOM_T_ABILITY`
- **Expansion config:** `MF_RANDOMIZER` (compile-out only)

## Context

ME remaps each `(species, originalMove)` through a near-full move whitelist (`RandomSeededModulo(move + species, …)`), and remaps abilities by substituting another species then reading that dest’s ability 0/1. It only forces a damaging starter move before `FLAG_SYS_POKEMON_GET`. TM/HM checks remap the *species* onto another learnset, which can strip Cut/Surf/Strength from the original HM user and fight S51’s Kanto HM guarantee. Expansion also has form-locked abilities (Wonder Guard, Forecast, Disguise, …) that crash or brick mons that are not their intended species.

## Decision

1. **Level-up only.** `MfGetSpeciesLevelUpLearnset` keeps classic/modern tables, then remaps each move with S16 (`MF_RANDOM_CAT_MOVES`, location key 0). Levels stay put so summary, dex, `GiveBoxMonInitialMoveset`, and level-up all share one table (EWRAM last-species cache). Teachable TM/tutor and egg lists are **not** remapped — Kanto HMs stay on the species that already had them.
2. **Damaging at low level.** After remap, every species must have a non-status move at level ≤ 5 (or in the first positive-level slot if nothing is that early). Magikarp cannot keep only Splash for Route 1. Pool skips `MOVE_NONE` / Struggle / Sketch / placeholder / 0 PP.
3. **Abilities.** `GetSpeciesAbility` is the single hook. Empty slots stay empty. Form-locked abilities are omitted from the S08 assignment pool and **kept** on species that already have them (Shedinja Wonder Guard, Castform Forecast). Other slots hash into the pool (`MF_RANDOM_CAT_ABILITY`). ME’s “always use dest slot 1” is not copied.

## Alternatives considered

- **ME TM species-swap** — rejected; can drop HMs and desync S51.
- **Remap every TM into a random move** — rejected; the TM item still teaches its real move, so the list would lie.
- **Full ability shuffle including Wonder Guard** — rejected; 1 HP + Wonder Guard on a normal type, or Shedinja without it, is an unwinnable/crash case.

## Consequences

- Trainer authored moves still clear on S53 remap, then refill from the randomized level-up set.
- Egg/TM pages stay classic-vs-modern (S30). A later story can randomize TMs without touching HMs.
- Changing the move/ability pool or hash is a randomizer mapping break (same seed, new sets) — treat like ADR 0016.
