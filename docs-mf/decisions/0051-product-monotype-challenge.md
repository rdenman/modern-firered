# 0051 — Monotype: either type, catch refuse, deterministic Oak remap

- **Type:** product
- **Status:** Accepted
- **Date:** 2026-10-04
- **Story:** S48
- **ME reference:** `IsOneTypeChallengeActive` / `GiveMonToPlayer` / `OneTypeChallengeCaptureBlocked` / `PickRandomStarterForOneTypeChallenge` in `src/tx_randomizer_and_challenges.c`, `src/pokemon.c`, `src/battle_setup.c`, `src/starter_choose.c`
- **Expansion config:** —

## Context

ME stores ONE TYPE ONLY as a type id with `31` = Off (already in `ModernRules.monotype`). Dual-types, Fairy-off retypes, and modern-type overlays all have to agree with `GetSpeciesType`. ME also rewrites Birch's three starters to random legal species and, on a catch of a wrong type, both blocks the ball and would dump the mon to the PC inside `GiveMonToPlayer`. FireRed's Oak lab is three named Poké Balls, not Birch's bag, and S51's randomizer is not available yet.

## Decision

1. **Legality:** a species is legal if `monotype == 31` or either `GetSpeciesType` slot matches. Fairy-off Clefairy is Normal, not Fairy; modern-on Arbok is legal as Poison or Dark.
2. **Catch:** refuse the ball with a clear message (`MfIsMonotypeCaptureBlocked` / `BALL_THROW_UNABLE_MONOTYPE` / Safari selection script). Do not let the catch succeed and silently PC-dump.
3. **Party adds:** `GiveCapturedMonToPlayer` / `GiveScriptedMonToPlayer` still send an illegal mon to the PC if the player already has a Pokémon (ME safety net, gifts/eggs). An empty party is allowed so Oak cannot softlock if remap finds no candidate.
4. **PC:** withdraw, place into an empty party slot, and shift-into-party refuse with the same message. Deposit is unrestricted.
5. **Oak starter:** ME has no per-type Oak table. Birch’s three balls call `PickRandomStarterForOneTypeChallenge`: copy `sRandomSpeciesEvo0`, `ShuffleListU16` with seed `(slot+13)*12289` mixed with the low 16 bits of the player OT ID (`RandomSeeded(..., TRUE)`), take the first species whose `GetTypeBySpecies` slot 1 or 2 matches, skipping names already chosen. We copy that list (same order) into `src/data/mf_monotype_oak_starters.h` and the same shuffle. Trio is always resolved slot 0 then 1 then 2 so inspect order cannot change the set (ME fills lazily on first look). Remapped balls use a generic confirm line. Legendaries are not in evo-0; Magikarp / Pichu / Unown / gen-4 babies are.
6. **Nuzlocke composition:** an off-type wild battle does not consume the area encounter (ME `!OneTypeChallengeCaptureBlocked`) and hides the first-encounter icon.

## Alternatives considered

- Curated “sensible starter” rows — rejected; ME’s pool is the full evo-0 list and the trio is OT-seeded, not a fixed mapping.
- Dex-order scan of unevolved species — rejected; that is not ME’s shuffle, and Pikachu is evo-1 (Pichu is the evo-0).
- Send illegal catches to the PC instead of refusing the ball — rejected; the story asks for a clear catch message, and a “caught” mon the player cannot use is confusing.
- Hide Fairy on the Challenges page when Fairy types are off — left as-is (S24); legality follows `GetSpeciesType`, so that combo may have few or no legal species.

## Consequences

- Upstream FR Oak lab script and PC / ball-throw / `Give*MonToPlayer` sites take one-line `mf_monotype` hooks.
- S51 starter randomization should compose with `MfResolveMonotypeStarterSpecies` rather than bypassing it.
- Debug “Give Pokémon” of an off-type with a non-empty party goes to the PC.
