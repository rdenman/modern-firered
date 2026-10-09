#ifndef GUARD_MF_TYPES_H
#define GUARD_MF_TYPES_H

#include "global.h"
#include "constants/pokemon.h"
#include "fpmath.h"

// Runtime type helpers (S27–S28, S56). GetSpeciesType / GetTypeModifier route
// here so Fairy, modern typings, TYPE CHART, and randomizer remaps stay out of
// upstream species_info / chart data.

enum Type MfGetSpeciesType(enum Species species, u8 slot);

// Combat types used by TYPE / EFFECTIVENESS randomization (skip NONE / MYSTERY /
// STELLAR). Pass includeFairy=FALSE to omit Fairy from the TYPE species pool.
bool8 MfTypeIsCombatType(enum Type type, bool8 includeFairy);
u8 MfFillCombatTypes(enum Type *out, bool8 includeFairy);

// Active type-effectiveness matrix (Gen VI+, ME Improved, or S56 baked shuffle).
// Hot path: pick a table pointer, then a single 2D index — no per-matchup branching.
const uq4_12_t (*MfGetTypeEffectivenessTable(void))[NUMBER_OF_MON_TYPES];

#endif // GUARD_MF_TYPES_H
