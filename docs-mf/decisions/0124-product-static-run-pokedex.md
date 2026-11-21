# 0124 — Static run Pokédex with real National Dex numbers

- **Type:** product
- **Status:** Accepted
- **Date:** 2026-11-21
- **Story:** —
- **ME reference:** ME hardcodes one 432-entry dex in every mode, with its additions numbered densely after Deoxys. Not followed.
- **Supersedes:** the National Dex listing and numbering parts of [0120](0120-product-run-pokedex-and-modern-evolutions.md). `MfSpecies_IsInRun` and the Evolutions rule are unchanged.

## Context
Under 0120 the run dex listed only species the run could still obtain. It was rebuilt from wild tables after randomizer remaps, story choices and caught flags. That caused three problems:
- **The dex changed mid-run.** After you picked a starter, fossil or Dojo Pokémon, the options you passed on disappeared.
- **Dex numbers shifted.** Entries were numbered 1..N, so removing one renumbered everything after it.
- **3–5 second black screens.** The rebuild ran synchronously on the first summary or dex use after boot, and again after every new-species catch.

## Decision
- **Fixed for the whole run.** The National Dex depends only on the locked rules. Story choices, roamers and catches never change it.
- **What's listed:**
  - Gen 1–3 species (National Dex 1–386) are always listed.
  - Post-Gen-3 species need `MfSpecies_IsInRun` (Evolutions = Modern, or Regional Forms for regional lines). On top of that:
    - **Species randomizer on** (Starters, Wild, Statics, Trades, Evolutions or Evolution Methods): the whole run species set is listed, because any pool species can reach the player. Trainer-only randomization doesn't count.
    - **Otherwise:** a species is listed only if an achievable evolution (Kanto items, learnable moves, no special map or weather) reaches it from a listed species. This drops breeding-only babies (Munchlax, Happiny, Mime Jr., Bonsly, Budew, Chingling, Mantyke), because Kanto only has Lax and Sea Incense. It also drops Ursaluna (Peat Block), Kleavor (Black Augurite) and regional-line evolutions, since regional forms only reach the player through the randomizer.
- **Real National Dex numbers.** Numbers outside the run are skipped in the list, not shown as blank rows, so every row can be filled. Magmortar is #467 and Sylveon is #700. Order is plain National Dex order; there's no special ordering for regional lines.
- **Digits.** The dex and summary use 4 digits only when the highest listed number is above 999.
- **Completion:** every listed non-mythical entry. A normal (unrandomized) run can always complete it. Challenge rules (One Type Only, Gift Only, Evo Limit) and map-based randomizer seeds may leave entries unobtainable. That's accepted.
- Caught species are always listed as a safety net (debug gifts). In normal play they are already listed by construction.

## Alternatives considered
- **ME-style dense numbering.** Rejected. Real numbers match what players look up, and a future "choose generations" option only changes which species are listed, never their numbers.
- **Hiding unreachable extras from the species set itself.** Rejected. Randomized runs would lose species that are perfectly catchable there.
- **Computing exactly what a randomizer seed places.** Rejected for now. It's accurate and still static, but it brings back the expensive wild-table sweep.

## Consequences
- The listing is cached and keyed by a CRC of the rules, which only changes when a different save loads. A rebuild walks the species table once plus a few evolution passes, well under a frame. Catches and story flags no longer trigger rebuilds.
- `SpeciesToPokedexNum` is vanilla again (no dex work on the summary screen).
- Approximations:
  - Gen 1–3 species stock FRLG can't produce (for example Gligar) are listed, so their cross-gen evolutions (Gliscor) are too. That matches the stock 386 National Dex.
  - Adding a new incense or evolution item to Kanto requires updating `MfEvoItemIsKantoObtainable`, or the breeding note above.
