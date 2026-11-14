# 0123 — One Type Wilds: wild Pokémon match the One Type Only type

- **Type:** product
- **Status:** Accepted
- **Date:** 2026-11-12
- **Story:** —
- **ME reference:** — (FR-only; ME has no monotype wild remap)
- **Expansion config:** —

## Context
With One Type Only (ADR 0051), most wild encounters can't join the team, so random wilds are mostly noise. Players asked for a mode where every wild Pokémon is the chosen type, while still respecting the randomizer's Balancing and Legendaries settings.

## Decision
New Randomizer-page row **One Type Wilds** (Off/On, default Off), directly under **Wild Pokémon**. It is editable only when the randomizer is on, **Wild Pokémon** is on, and **One Type Only** is set. Otherwise it greys out with "Needs Wild Pokémon and the One Type Only challenge!". The bit is `randomMonotypeWild`, taken from the last `paddingTail` bit, with `MF_RULES_VERSION` 11 and old saves defaulting to Off. Turning the randomizer master off clears it, and the runtime gate (`MfRules_IsMonotypeWild`) also requires Wild + monotype, the same pattern Themed Trainers uses.

When active, the wild remap (`MfSpeciesMapActive` for the wild category, plus `MfWildBattleSpecies`) calls `MfSpeciesMapExMonotype` and tries these steps in order:

1. **Balancing on:** same evo stage, then the same BST bands and HM-user filters the normal remap uses, restricted to the type.
2. **Any stage** of the type. Balancing is relaxed only when the type has no species at that stage.
3. **Legendaries** of the type, only if the type has no non-legendary species at all.

With Balancing off, step 1 is skipped. The Legendaries and Chaos settings apply as usual. Types come from `GetSpeciesType`, so random **Type** and Fairy toggles are respected, which matches the party-legality check.

The Pokédex area screen, DexNav, the Nuzlocke catchable dex, and randomized trade requests all read the same remap, so they stay consistent. The trade cache now keys on this flag too.

## Alternatives considered
- **Work without Wild Pokémon (only swap off-type vanilla wilds)**: rejected. That would be a second remap path, and the Themed Trainers precedent already requires the parent remap to be on.
- **Reuse `MfSpeciesMapExForType` as is**: rejected. It adds legendaries before it relaxes the stage and returns the off-type source when nothing matches. Both would break "every wild is on-type, no surprise legendaries".
- **Hide the row unless monotype is on**: rejected. The menu greys out gated rows instead of hiding them.

## Consequences
- Unown stay Unown (Tanoby puzzle, ADR 0055), so they are still off-type unless the run is Psychic.
- Static encounters, gifts and trainers are unaffected. Use Static Pokémon / Themed Trainers for those.
- `paddingTail` is now fully used. The next tail field needs a different padding source or a larger blob.
- Tests: `test/modern_firered/mf_monotype_wild.c`.
