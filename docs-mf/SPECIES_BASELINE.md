# Species, moves & movepool baseline (Phase 1 / S08)

Deliberate **data availability** for the FireRed ROM — what Pokémon exist in the binary before randomizer / modern-encounter / dual-table stories consume the pool. Product rule: this is **not** a full National Dex / Gen 9 dump (`PROJECT.md`).

Policy lives in [`include/config/mf_species_enabled.h`](../include/config/mf_species_enabled.h) (ADR 0008). Upstream `species_enabled.h` stays at RHH defaults; we override after include.

## Enabled species families

| Generation | `P_GEN_x_POKEMON` | Why |
| ---------- | ----------------- | --- |
| Gen 1 | `TRUE` | Kanto dex / wild / trainers |
| Gen 2 | `TRUE` | Sevii guests + FRLG-era babies/evos; randomizer pool |
| Gen 3 | `TRUE` | Hoenn contemporaries for randomizer / later data-only encounter remaps |
| Gen 4–9 | `FALSE` | Avoid National Dex dump; saves ROM; revisit only via ADR |

`MF_SPECIES_FAMILIES_MAX_GEN` is `3`. Changing any `P_GEN_*` **changes saveblock dex flags** — needs a new save (and S64 versioning once that lands).

## Cross-generation evolutions

`P_CROSS_GENERATION_EVOS` (and per-gen cross switches) stay **`TRUE`**. Families stay Gen 1–3, but Kanto-line additions still compile:

| Cross-gen | Examples (enabled-family lines) |
| --------- | -------------------------------- |
| Gen 2 | Pichu, Crobat, Espeon, Umbreon, Slowking, Steelix, … |
| Gen 3 | Azurill, Wynaut, … |
| Gen 4 | Magnezone, Electivire, Magmortar, Rhyperior, Tangrowth, Togekiss, … |
| Gen 6 | Sylveon |
| Gen 8 | Hisuian-line evos tied to enabled families / regional forms (e.g. Kleavor) |
| Gen 9 | Annihilape, Farigiraf, Dudunsparce, … |

`P_NEW_EVOS_IN_REGIONAL_DEX` is **`TRUE`** so those evos appear in the Kanto regional dex.

**Note:** Some location-based evos still reference Hoenn mapsecs in upstream data (e.g. Magnezone / New Mauville). Item alternates (Thunder Stone, etc.) remain usable in Kanto; remap location conditions later if needed.

## Forms & gimmicks

| Switch | Value | Why |
| ------ | ----- | --- |
| Regional forms (Alolan / Galarian / Hisuian / Paldean) | `TRUE` | Data for Kanto species variants; no new maps required for the data itself |
| Mega Evolutions | `TRUE` | Iconic Kanto/Hoenn megas; battle support already in expansion |
| Primal Reversions | `TRUE` | Gen 3 legendaries are in-range |
| Ultra Burst / Gigantamax / Tera forms | `FALSE` | No FR campaign for these; ROM cost |
| Cosplay / Cap Pikachu | `FALSE` | Unused clutter |
| Gen 9 (ZA) megas | `FALSE` | Gen 9 families off |

## Moves, abilities, learnsets

These are **not** trimmed by `P_GEN_*`:

| Area | Config / data | FR baseline |
| ---- | ------------- | ----------- |
| Move definitions | `gMovesInfo` + `B_UPDATED_MOVE_*` | Full modern move table (`GEN_LATEST`, see S07 / `BATTLE_BASELINE.md`) |
| Abilities | `gAbilitiesInfo` + `B_UPDATED_ABILITY_DATA` | Full modern ability table |
| Level-up learnsets | `P_LVL_UP_LEARNSETS` → `GEN_LATEST` | Latest learnsets for **enabled** species |
| Teachables (TM/tutor) | expansion teachable tables | Present for enabled species |

Phase 4 “modern movepool” / dual tables (later stories) gate *which* learnset table is used at runtime; they do not re-decide which species exist. Randomizer species candidates (S55) must respect `MF_SPECIES_FAMILIES_MAX_GEN` / enabled families from this doc.

## ROM size (acceptance)

Measured after enabling this policy (`make firered`, 2026-09-11):

| Metric | All-gens (before) | Gen 1–3 policy (after) |
| ------ | ----------------- | ---------------------- |
| Linker ROM used | ~26.9 MiB payload (pre-policy pad measure) | **16.13 MiB** (`16912244` B, 50.40% of 32 MiB) |
| Trailing `0xFF` pad | ~6.3 MiB | **~15.87 MiB** |

File size remains padded to 32 MiB (`0x2000000`). Headroom is the useful signal for later QoL/data stories.

## TESTING note

`make check` builds with `TESTING=1`. **`mf_species_enabled.h` does not apply `P_*` overrides under `TESTING`**, so Emerald TESTELF keeps expansion’s full Gen 1–9 species set (and `test.h` still force-enables many `P_FAMILY_*` flags). The FireRed ROM (`make firered`) is where Gen 4–9 families stay out of the binary. MF tests always lock the `MF_SPECIES_*` policy stamps; they only assert matching `P_GEN_*` values when `!TESTING`.

## After an upstream merge

If `include/config/pokemon.h` or `species_enabled.h` conflicts, keep the one-line `#include "config/mf_species_enabled.h"` immediately after `species_enabled.h`. Re-read new upstream `P_GEN_*` / form switches and extend the MF override file if RHH adds knobs we care about. Re-run `make check` (`MF:` filter) and a FireRed size check.
