# 0058 — Kanto gym / E4 / Champion theme groups

- **Type:** product
- **Status:** Accepted
- **Date:** 2026-10-08
- **Story:** S75
- **ME reference:** — (ME has no gym-wide themed trainers)
- **Expansion config:** `MF_RANDOMIZER` (compile-out only)

## Context

S74 hashes a type per trainer CRC. That splits a gym: Camper Liam and Brock can be different types. S75 wants one type for everyone on a gym’s maps, a separate type per Elite Four member, and one Champion type for all of Blue’s starter-variant teams. Rocket hideout Giovanni and the Fighting Dojo must not piggy-back on Viridian / Saffron gyms.

## Decision

1. **Membership is trainer-id, authored from FRLG gym / League scripts** — not Hoenn, not mapsec (Pewter Gym’s mapsec is Pewter City). Table lives in `src/mf_theme_groups.c`. Viridian gym = `TRAINER_LEADER_GIOVANNI` and the gym juniors only. Hideout/Silph use `TRAINER_BOSS_GIOVANNI` / `_2` and stay S74. Dojo black belts are omitted.
2. **Hash** — `MfRandom_Modulo(seed, MF_RANDOM_CAT_TRAINER, 0x5447, group, typeCount)`. Same type pool as S74 (Fairy gated). Map-based keys and trainer CRC are ignored inside a group so the type is stable for the run.
3. **Gameplay** — `CreateNPCTrainerParty` calls `MfBeginTrainerParty(trainerNum)` so S74’s slot remap uses the group. Unlisted trainers (routes, rivals, Rocket, Dojo) keep the S74 per-trainer hash (`0x5448` ⊕ CRC).
4. **Champion** — first and rematch Squirtle / Bulbasaur / Charmander teams share `MF_THEME_GROUP_CHAMPION`. E4 rematch ids share that member’s group.

## Alternatives considered

- **Current map only** — rejected; tests and rematches need a trainer id, and mapsec would leak gym types onto the city.
- **One type per trainer class LEADER** — rejected; would not cover gym juniors and would not split the four E4 members.

## Consequences

- Adding a gym trainer to a map script without updating `sMfThemeMembers` leaves them on a per-trainer theme.
- Changing group ids or the `0x5447` input is a mapping break (ADR 0016).
