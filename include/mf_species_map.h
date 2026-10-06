#ifndef GUARD_MF_SPECIES_MAP_H
#define GUARD_MF_SPECIES_MAP_H

// S51 — shared "species → randomized replacement" service.
// Deterministic via S16 (never Random()). Candidates are the S08 pool
// (Gen 1–3 families + cross-gen evos + regional forms; no gimmick forms).
// See ADR 0054.

#include "gba/types.h"
#include "mf_random.h"
#include "constants/moves.h"
#include "constants/pokemon.h"
#include "constants/species.h"

#define MF_SPECIES_MAP_STAGE_0         0
#define MF_SPECIES_MAP_STAGE_1         1
#define MF_SPECIES_MAP_STAGE_2         2
#define MF_SPECIES_MAP_STAGE_LEGENDARY 3

#define MF_SPECIES_MAP_HM_CUT      0x01
#define MF_SPECIES_MAP_HM_SURF     0x02
#define MF_SPECIES_MAP_HM_STRENGTH 0x04

#define MF_SPECIES_MAP_BST_STEP    50

// Pure remap. Does not read wild/trainer/static flags.
enum Species MfSpeciesMapEx(enum Species species,
                            u32 seed,
                            enum MfRandomCategory category,
                            u16 locationKey,
                            bool8 similar,
                            bool8 includeLegendaries);

// S74 — same remap as Ex, but dest must match themeType (either slot).
enum Species MfSpeciesMapExForType(enum Species species,
                                   u32 seed,
                                   enum MfRandomCategory category,
                                   u16 locationKey,
                                   bool8 similar,
                                   bool8 includeLegendaries,
                                   enum Type themeType);

// Gameplay: identity unless the category's remap bit is on.
enum Species MfSpeciesMapActive(enum Species species,
                                enum MfRandomCategory category,
                                u16 mapsec);

bool8 MfSpeciesMap_CategoryRemaps(enum MfRandomCategory category);

bool8 MfSpeciesMap_IsCandidate(enum Species species);
u16 MfSpeciesMap_GetPoolCount(void);
enum Species MfSpeciesMap_GetPoolSpecies(u16 index);

u8 MfSpeciesMap_GetEvoStage(enum Species species);
bool8 MfSpeciesMap_IsLegendary(enum Species species);
u16 MfSpeciesMap_GetRawBst(enum Species species);
u8 MfSpeciesMap_GetKantoHmMask(enum Species species);
bool8 MfSpeciesMap_CanLearnKantoHm(enum Species species, enum Move move);

void MfSpeciesMap_EnsurePool(void);

#endif // GUARD_MF_SPECIES_MAP_H
