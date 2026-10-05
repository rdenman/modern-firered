# 0054 — Species mapping: S08 pool, evo-stage+BST, HM preserve

- **Type:** product
- **Status:** Accepted
- **Date:** 2026-10-05
- **Story:** S51
- **ME reference:** `GetSpeciesRandomSeeded` / `GetRandomSpecies` / `gSpeciesMapping` in `src/pokemon.c`
- **Expansion config:** `MF_RANDOMIZER` (compile-out only)

## Context

S51 needs a shared `(species, seed, category, location) → species` service before wild/trainer/static call sites (S52+). ME picks from precomputed evo-stage tables and identity-maps legendaries unless `LEGENDARIES` is on. It does **not** compare BST, and it does not prove Kanto HMs stay available. Our candidate ceiling is ADR 0008 (Gen 1–3 families + cross-gen evos + regional forms), which ME’s hardcoded tables do not match.

## Decision

1. **Pool** — usable, non-gimmick forms whose National Dex is 1–386, plus evolution targets of that set (Magnezone, Sylveon, Annihilape, …). Megas / primals / Gmax / Tera / Ultra Burst / Totem and non-regional alt forms are out. TESTELF still has Gen 4–9 families compiled; the mapper ignores them so tests match the FR ROM.
2. **BALANCING** — same ME evo-stage buckets (0 / middle / final / legendary) **and** a BST band on raw `gSpeciesInfo` totals (not the S49 equalizer). Band starts at ±50 and widens (±100, ±200, then stage-only) if a filter is empty.
3. **LEGENDARIES** — `isRestrictedLegendary` / `isSubLegendary` / `isMythical`. Off → identity for those species and they never appear as replacements. On → they remap among themselves when Balancing is on.
4. **Map-based** — S16 location keys only (wild/trainer). The mapper does not read the current map itself.
5. **Kanto HMs** — Cut, Surf, and Strength on the compiled teachable list must be preserved: a source that can learn one of those maps only to a dest that can learn the same move(s). Combined with mapping the whole pool, every seed keeps at least one learner of each. Chaos does not re-roll here (S57).
6. **Call sites** — `MfSpeciesMapEx` is the pure remap. `MfSpeciesMapActive` is identity unless that category’s wild/trainer/static bit is on. This story does not rewrite encounter tables.

## Alternatives considered

- **Copy ME’s static `sRandomSpeciesEvo*` tables** — rejected; they are Emerald-dex lists and skip BST / HM.
- **Permutation of the full pool** — rejected; map-based keys would need a table per mapsec, which does not fit EWRAM.
- **Use `GetSpeciesBaseStatTotal` for BST** — rejected; equalizer and classic-stat toggles would collapse or scramble the band.

## Consequences

- S52+ must call `MfSpeciesMapActive` (or `Ex` in tests) and never `Random()`.
- Changing the pool filter or hash is a randomizer-run mapping break; treat like ADR 0016 if it ever changes.
- HM preserve can shrink Balancing neighborhoods for HM users (e.g. Surf users stay water-ish); that is the intended softlock guard.
