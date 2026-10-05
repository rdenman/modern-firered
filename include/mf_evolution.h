#ifndef GUARD_MF_EVOLUTION_H
#define GUARD_MF_EVOLUTION_H

// S47 — Challenges EVO LIMIT (ME EvolutionBlockedByEvoLimit).
// Off / First / All. See ADR 0050.

#include "gba/types.h"
#include "constants/items.h"
#include "constants/species.h"

#define MF_EVO_LIMIT_OFF   0
#define MF_EVO_LIMIT_FIRST 1
#define MF_EVO_LIMIT_NONE  2

// Pure: Off never blocks; All always blocks; First blocks if the species
// already has a pre-evolution (one evo from the baby/base is allowed).
bool8 MfIsEvolutionBlockedByLimitValue(enum Species species, u8 evoLimit, bool8 hasPreEvolution);

bool8 MfSpeciesHasPreEvolution(enum Species species);
bool8 MfIsEvolutionBlockedByLimit(enum Species species);

// TRUE when the limit blocks this species and `item` is one of its EVO_ITEM methods.
bool8 MfIsEvolutionItemBlockedByLimit(enum Species species, enum Item item);

const u8 *MfGetEvolutionLimitMessage(void);

#endif // GUARD_MF_EVOLUTION_H
