#ifndef GUARD_MF_POKEVIAL_H
#define GUARD_MF_POKEVIAL_H

#include "global.h"

#define MF_POKEVIAL_MAX_CHARGES 3

enum MfPokeVialUseResult
{
    MF_POKEVIAL_USE_DISABLED,
    MF_POKEVIAL_USE_EMPTY,
    MF_POKEVIAL_USE_NOTHING,
    MF_POKEVIAL_USE_OK,
};

u8 MfPokeVialMaxChargesFromBadgeCount(u8 badges);
u8 MfPokeVialGetMaxCharges(void);
u8 MfPokeVialGetRemaining(void);
bool32 MfPokeVialPartyNeedsHeal(void);
enum MfPokeVialUseResult MfPokeVialWouldUse(void);
enum MfPokeVialUseResult MfPokeVialTryUse(void);
void MfPokeVialGrantOnNewGame(void);
u16 MfPokeVialRefill(void);
const u8 *MfPokeVialGetDescription(void);
const u8 *MfPokeVialGetUseMessage(enum MfPokeVialUseResult result);

#endif // GUARD_MF_POKEVIAL_H
