# 0057 — Themed trainer parties: per-trainer hashed type

- **Type:** product
- **Status:** Accepted
- **Date:** 2026-10-06
- **Story:** S74
- **ME reference:** — (ME has no THEMED TRAINERS option)
- **Expansion config:** `MF_TX_RANDOM_THEMED_TRAINERS` default FALSE; compile-out still `MF_RANDOMIZER`

## Context

S53 remaps each trainer slot independently through S51, so gyms and random trainers can mix types. S74 adds an optional single-type party without using vanilla gym identities (Brock is not locked to Rock). The type must be stable for every slot of that trainer, deterministic via S16, and S64-safe for existing saves. Fairy must not appear as a theme when Fairy types are off. S75 will share gym/E4/Champion types on top of this.

## Decision

1. **Save bit** — `randomThemedTrainers` in `paddingTail` bit 1 (next to S70 `scaledExp`). Do not reorder `ModernRules` or bump `MF_RULES_VERSION`. Existing saves read as Off. Classic/Modern/Custom default Off. Master Off clears the bit with the other remaps.
2. **Menu** — Randomizer row `THEMED TRAINERS` Off/On, immediately under `TRAINER`. Editable only when TRAINER is on.
3. **Theme hash** — `MfRandom_Modulo(seed, MF_RANDOM_CAT_TRAINER, 0x5448, locationKey, typeCount)`. `locationKey` is the same S53 trainer key (mapsec when map-based, XOR trainer CRC). Pool is Normal–Dark plus Fairy when `fairyTypes` is on; skip `TYPE_NONE` / `TYPE_MYSTERY` / `TYPE_STELLAR` and any type with no S08-pool species under the current legendaries rule.
4. **Slot remap** — `MfSpeciesMapExForType` uses `GetSpeciesType` / `MfSpeciesMatchesType` (either slot, so dual-types count). Balancing widens like S51, then any stage of that type (HM dropped at that last step). Type wins over a mixed party. Mirror still copies the foe party after generation (ADR 0053). S56 type randomization will compose through `GetSpeciesType` with no extra hook.

## Alternatives considered

- **Use the trainer’s vanilla primary type** — rejected; the story forbids gym-identity themes (S75 randomizes those separately).
- **Hash type from the first party species** — rejected; two Geodude trainers would share a type, and the first slot would dominate.
- **Bump `MF_RULES_VERSION`** — rejected; a spare padding bit already reads as Off on old saves (same as ADR 0045).

## Consequences

- Changing the theme input id (`0x5448`) or the type pool is a mapping break; treat like ADR 0016.
- S75 should reuse `MfTrainerThemeType` / the same hash primitive with gym-group keys instead of per-trainer CRC.
- Player monotype (S48) is independent: a Grass-only player can still face a hashed Electric gym.
