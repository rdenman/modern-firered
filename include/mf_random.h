#ifndef GUARD_MF_RANDOM_H
#define GUARD_MF_RANDOM_H

// Deterministic seeded mapping for the randomizer (S16). Pure helpers take an
// explicit seed; gameplay wrappers read ModernRules.randomizerSeed.
// Full species/move remapping tables land in S51+ — this is the hash primitive.
// See ADR 0016.

#include "gba/types.h"
#include "mf_rules.h"

// ME TX_RANDOM_T_* category ids (same numeric values).
enum MfRandomCategory
{
    MF_RANDOM_CAT_WILD = 0,      // TX_RANDOM_T_WILD_POKEMON
    MF_RANDOM_CAT_TRAINER = 1,   // TX_RANDOM_T_TRAINER
    MF_RANDOM_CAT_MOVES = 2,     // TX_RANDOM_T_MOVES
    MF_RANDOM_CAT_ABILITY = 3,   // TX_RANDOM_T_ABILITY
    MF_RANDOM_CAT_EVO = 4,       // TX_RANDOM_T_EVO
    MF_RANDOM_CAT_EVO_METH = 5,  // TX_RANDOM_T_EVO_METH
    MF_RANDOM_CAT_STATIC = 6,    // TX_RANDOM_T_STATIC
    MF_RANDOM_CAT_COUNT,
};

// --- Pure API (no save / Random() dependence) ---

// Mix (seed, category, inputId, locationKey) → deterministic u32.
u32 MfRandom_Hash(u32 seed, enum MfRandomCategory category, u32 inputId, u16 locationKey);

// Uniform index in [0, bound). bound == 0 or 1 → 0. Rejection sampling for bias.
u16 MfRandom_Modulo(u32 seed, enum MfRandomCategory category, u32 inputId, u16 locationKey, u16 bound);

// ME map-based: only wild/trainer fold mapsec in when mapBasedEnabled; others → 0.
u16 MfRandom_LocationKey(enum MfRandomCategory category, u16 mapsec, bool8 mapBasedEnabled);

// --- Seed lifecycle ---

// Non-zero seed from the global RNG (call once at new game / commit).
u32 MfRandom_GenerateNewSeed(void);

// If randomizerSeed is 0, assign MfRandom_GenerateNewSeed().
void MfRandom_EnsureSeed(struct ModernRules *rules);

// --- Active-rules wrappers ---

// Hash/modulo using MfRules_GetRandomizerSeed() and LocationKey from the map-based rule.
u32 MfRandom_HashActive(enum MfRandomCategory category, u32 inputId, u16 mapsec);
u16 MfRandom_ModuloActive(enum MfRandomCategory category, u32 inputId, u16 mapsec, u16 bound);

#endif // GUARD_MF_RANDOM_H
