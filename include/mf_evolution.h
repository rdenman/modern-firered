#ifndef GUARD_MF_EVOLUTION_H
#define GUARD_MF_EVOLUTION_H

// S47 — Challenges EVO LIMIT (ME EvolutionBlockedByEvoLimit).
// Off / First / All. See ADR 0050.
// S55 — EVOLUTIONS / EVO LINES remaps. See ADR 0061.

#include "gba/types.h"
#include "pokemon.h"
#include "constants/items.h"
#include "constants/species.h"

#define MF_EVO_LIMIT_OFF   0
#define MF_EVO_LIMIT_FIRST 1
#define MF_EVO_LIMIT_NONE  2

#define MF_RANDOM_EVO_FALLBACK_LEVEL 30

// Pure: Off never blocks; All always blocks; First blocks if the species
// already has a pre-evolution (one evo from the baby/base is allowed).
bool8 MfIsEvolutionBlockedByLimitValue(enum Species species, u8 evoLimit, bool8 hasPreEvolution);

bool8 MfSpeciesHasPreEvolution(enum Species species);
bool8 MfIsEvolutionBlockedByLimit(enum Species species);

// TRUE when the limit blocks this species and `item` is one of its EVO_ITEM methods.
bool8 MfIsEvolutionItemBlockedByLimit(enum Species species, enum Item item);

const u8 *MfGetEvolutionLimitMessage(void);

// Vanilla ROM table (no randomizer). Used by S47 pre-evo and daycare eggs.
const struct Evolution *MfGetVanillaSpeciesEvolutions(enum Species species);

// Gameplay: identity unless EVOLUTIONS / EVO LINES is on.
const struct Evolution *MfMaybeRandomizeEvolutions(enum Species species, const struct Evolution *src);

bool8 MfEvoItemIsKantoObtainable(enum Item item);
bool8 MfEvolutionIsKantoAchievable(const struct Evolution *evo);

#endif // GUARD_MF_EVOLUTION_H
