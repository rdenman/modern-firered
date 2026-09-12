# 0008 — Species families Gen 1–3 + cross-gen Kanto evolutions

- **Type:** product
- **Status:** Accepted
- **Date:** 2026-09-11
- **Story:** S08
- **ME reference:** Full modern dex in ME is *not* copied; ME is UX/rules inspiration only
- **Expansion config:** `P_GEN_*_POKEMON`, `P_CROSS_GENERATION_EVOS`, form switches in `species_enabled.h` (overridden via `mf_species_enabled.h`)

## Context

Expansion defaults every generation family and most gimmick forms to `TRUE`. `PROJECT.md` says a full National Dex / Gen 9 dump is out of scope unless deliberately decided later. S08 requires picking an enabled range (ROM size, build time, randomizer candidate pool) and documenting cross-gen Kanto-line behavior before Phases 4 and 9 depend on it.

## Decision

- **Families:** enable Gen **1–3** only; Gen **4–9** families stay off.
- **Cross-generation evolutions:** keep **on** through Gen 9 so Magnezone, Sylveon, Annihilape, and other additions to *enabled* lines still exist.
- **Regional forms:** on (Alolan/Galarian/Hisuian/Paldean variants of in-range species).
- **Megas + Primals:** on; **Ultra Burst / Gigantamax / Tera / Fusion forms / Cap–Cosplay Pikachu / ZA megas:** off.
- **Implementation:** do not edit upstream defaults in `species_enabled.h`. Override in `include/config/mf_species_enabled.h`, hooked with one include line after `species_enabled.h` in `pokemon.h`.
- **Reference:** `docs-mf/SPECIES_BASELINE.md`.

## Alternatives considered

- Keep Gen 1–9 all on — rejected; contradicts PROJECT “no National Dex dump,” burns ROM, and implies a product we are not shipping (wild/national completion across gens).
- Gen 1 + cross-evos only — rejected; Sevii/Johto guests and a usable randomizer pool want Gen 2–3 families.
- Gen 1–5 or 1–6 — rejected for v1; larger dump without Kanto geography to place them. Can revisit with a new ADR if encounter remaps need a specific later family.
- Edit `species_enabled.h` in place — rejected; maximizes `RHH/master` merge noise versus a one-line hook + MF override file.

## Consequences

- Randomizer / modern-encounter stories (S32, S55, …) treat Gen 1–3 + cross-evos / regional forms as the candidate ceiling unless this ADR is revised.
- Save dex flag layout changes vs an all-gens ROM — new saves required when flipping this policy; call out in S12/S64 if those stories assume expansion defaults.
- FireRed ROM payload dropped to ~16.1 MiB used (~15.9 MiB free pad) vs ~25.7 MiB used with all gens on.
- Some expansion tests need Gen 4–9 species data in Emerald TESTELF. **`mf_species_enabled.h` therefore applies `P_*` overrides only when `!TESTING`**; CI/`make check` keeps the full expansion dex. The FireRed ROM is the product surface for the Gen 1–3 ceiling.
- Location-based cross-evos that reference Hoenn mapsecs remain a known upstream quirk; item evolutions still work in Kanto.
