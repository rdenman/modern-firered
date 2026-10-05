#ifndef GUARD_MF_MONOTYPE_H
#define GUARD_MF_MONOTYPE_H

// S48 — Challenges ONE TYPE ONLY (ME IsOneTypeChallengeActive / GiveMonToPlayer).
// Off sentinel is monotype == 31. Dual-types are legal if either slot matches.
// Typings go through GetSpeciesType (Fairy / modern-types). See ADR 0051.

#include "gba/types.h"
#include "constants/species.h"

#define MF_MONOTYPE_OFF 31

bool8 MfSpeciesMatchesType(enum Species species, u8 type);
bool8 MfIsMonotypePartyLegalValue(enum Species species, u8 monotype);
bool8 MfIsMonotypePartyLegal(enum Species species);

void MfFillMonotypeOakStarters(u8 monotype, enum Species trio[3]);
enum Species MfFindNthMonotypeStarter(u8 monotype, u8 slot);
enum Species MfResolveMonotypeStarterSpecies(enum Species chosen, u8 slot);

// Oak lab special: rewrites VAR_TEMP_2 (PLAYER_STARTER_SPECIES). TRUE if changed.
u16 MfResolveOakStarterForMonotype(void);

bool8 MfIsMonotypeCaptureBlocked(void);

const u8 *MfGetMonotypeBlockedMessage(void);

#endif // GUARD_MF_MONOTYPE_H
