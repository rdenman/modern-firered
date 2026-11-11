# 0120 — Run Pokédex and the Modern Evolutions rule

- **Type:** product
- **Status:** Accepted
- **Date:** 2026-11-10
- **Story:** —
- **ME reference:** —
- **Expansion config:** `include/config/mf_species_enabled.h`

## Context
The National Dex listed every compiled species, more than 1000 entries, including later-gen evolutions and regional forms the run could never produce. On a randomized run with Regional Forms on, players saw a dex over 900 with most entries unobtainable. Classic also had no way to leave out post-Gen-3 evolutions such as Magnezone, Sylveon and Munchlax.

## Decision
- New Gamemode rule **Evolutions** (`modernEvos`, from `padding0`, rules v9). Classic sets it Off, Modern sets it On, and Custom lets the player choose. Older saves migrate On, so existing runs keep their evolutions.
- `MfSpecies_IsInRun` (`src/mf_dex.c`) decides which species exist this run:
  - Gen 1–3 species are always in.
  - Regional forms, and regional-line evolutions such as Perrserker and Sirfetch'd, follow **Regional Forms**.
  - Everything else follows **Evolutions**.
- The randomizer pool, evolutions (`GetSpeciesEvolutions`), pre-evolutions and egg species all use `MfSpecies_IsInRun`.
- The **National Dex** lists only species obtainable this run, numbered 1..N. The list is computed from:
  - Seeds: wild tables for the active encounter mode, after randomizer remaps (the whole pool under Chaos), statics, gifts, Game Corner prizes, fossils, the Dojo and starter choices still open, in-game trade offers, active roamers, and anything already caught.
  - Closure: evolutions the run can reach (level cap, Kanto items, Evo Limit) and breeding (Ditto needed for genderless or male-only species). Monotype legality applies to catches and breeding.
  - Gift Only skips the wild, static and roamer seeds.
- Caught species are always listed. Seen-only species that the run can't obtain (such as trainer Pokémon) are hidden.
- The Kanto Dex is unchanged (151 entries).
- The summary screen and dex show 3 digits unless the run dex exceeds 999 entries.

## Alternatives considered
- Keep National numbering and only hide entries. Rejected because the user wanted the count and the numbers to match the run.
- Force Regional Forms off in Classic. Rejected because Regional Forms stays an independent Features toggle.

## Consequences
- The list is cached by a CRC over the rules, caught flags, choice flags and roamers. It rebuilds when any of these change, for example after a new catch.
- Known approximations:
  - A static legendary stays listed after it's knocked out.
  - Breeding assumes a compatible partner exists.
  - Trade requests are assumed to be reachable.
- Regional-line evolutions now also need Regional Forms on. Runs with Regional Forms off get a smaller randomizer pool, so their wild remaps can reshuffle.
- Emerald and test builds are unaffected: `IS_FRLG` gates the logic.
