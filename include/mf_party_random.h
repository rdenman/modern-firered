#ifndef GUARD_MF_PARTY_RANDOM_H
#define GUARD_MF_PARTY_RANDOM_H

// S53 — TRAINER and STARTER POKéMON. Species go through S51; never Random().
// See ADR 0056.

#include "gba/types.h"
#include "constants/species.h"

struct TrainerMon;

#define MF_OAK_STARTER_COUNT 3

u16 MfTrainerRandomKey(const void *trainer, u32 size);

enum Species MfTrainerEncounterSpecies(enum Species species, u16 trainerKey);

// Remap species in a party-file copy. Unchanged level/item. Drops authored
// moves/ability/forced gender when the species actually changes.
void MfRandomizeTrainerMon(struct TrainerMon *mon, u16 trainerKey);

bool8 MfSpeciesHasOffensiveTypeAdvantage(enum Species attacker, enum Species defender);

void MfFillRandomOakStarters(enum Species trio[MF_OAK_STARTER_COUNT]);
enum Species MfPickRivalStarterSpecies(enum Species playerSpecies,
                                       enum Species vanillaRival,
                                       enum Species altRival);

// Oak lab special: rewrites PLAYER_STARTER_SPECIES / RIVAL_STARTER_SPECIES.
// TRUE if the player's species changed (generic confirm line).
u16 MfResolveOakStarterForRandomizer(void);

#endif // GUARD_MF_PARTY_RANDOM_H
