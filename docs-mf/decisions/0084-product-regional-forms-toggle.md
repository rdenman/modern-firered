# 0084 — Regional forms behind a Features toggle

- **Type:** product
- **Status:** Accepted
- **Date:** 2026-10-11
- **Story:** —
- **ME reference:** — (ME has no equivalent option)
- **Expansion config:** `P_REGIONAL_FORMS` stays `TRUE` in `mf_species_enabled.h` (ADR 0008)

## Context

Alolan / Galarian / Hisuian / Paldean variants of Gen 1–3 species are compiled into the FireRed ROM (ADR 0008) and the randomizer species pool treated them as first-class candidates (`IsUsableForm`). Vanilla wild tables still use Kantonian species, so players mainly see these forms on randomizer runs. They are a surprise in Kanto and should be opt-in, including for Classic / Modern — not Randomizer-page-only.

Compile-time `P_REGIONAL_FORMS FALSE` would strip the data and make a later toggle impossible.

Existing randomizer maps are rebuilt from seed + pool. Dropping regionals without a migrate would reshuffle every remap on Continue.

## Decision

1. **Keep the data in the ROM.** `MF_SPECIES_REGIONAL_FORMS` / `P_REGIONAL_FORMS` stay on.
2. **Features `REGIONAL FORMS` Off/On** (`regionalForms` in `ModernRules`, consumed from `paddingFeatures`). Editable without Custom, like other Features rows.
3. **New games default Off** (Classic, Modern, Custom / `MF_TX_FEATURES_REGIONAL_FORMS`).
4. **`MF_RULES_VERSION` 3.** v1 and v2 saves migrate `regionalForms = TRUE` so in-progress randomizers keep their maps. Blob size stays 48 bytes.
5. **Enforcement** is the S51 pool: regional form flags are eligible only when the toggle is on. The cached pool rebuilds if the flag changes (debug mid-run).

## Alternatives considered

- **Compile-time off** — rejected; no later opt-in, and it diverges the FR binary from TESTELF more than needed.
- **Randomizer-page-only** — rejected; this is species availability for every mode that samples the pool (wild/trainer/static/starter remap, random evo, themed trainers).
- **Default On for new games** — rejected; Kanto should not inject regionals unless the player asks.
- **Skip the version bump** (padding reads as Off) — rejected; that would reshuffle existing seeded maps.

## Consequences

- Vanilla / Modern encounter tables are unchanged; they never listed Alolan Diglett. The toggle matters when something uses `MfSpeciesMap_*`.
- ADR 0008’s “regional forms on” still means data availability, not “always in the randomizer.”
- Next padding-bit feature that needs default-TRUE must bump `MF_RULES_VERSION` the same way.
