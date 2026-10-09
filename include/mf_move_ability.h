#ifndef GUARD_MF_MOVE_ABILITY_H
#define GUARD_MF_MOVE_ABILITY_H

// S54 — deterministic MOVES / ABILITIES remaps (ME GetRandomMove /
// GetAbilityBySpecies species-swap). See ADR 0060.

#include "gba/types.h"
#include "pokemon.h"

#define MF_RANDOM_LOW_MOVE_LEVEL 5

enum Move MfMapMove(u32 seed, enum Species species, enum Move move);
enum Ability MfMapAbility(u32 seed, enum Species species, u8 slot);

// Gameplay: identity unless randomMoves / randomAbilities is on.
const struct LevelUpMove *MfMaybeRandomizeLevelUpLearnset(enum Species species, const struct LevelUpMove *src);
enum Ability MfGetSpeciesAbility(enum Species species, u8 slot);

bool8 MfLearnsetHasDamagingMoveByLevel(const struct LevelUpMove *learnset, u8 maxLevel);

#endif // GUARD_MF_MOVE_ABILITY_H
