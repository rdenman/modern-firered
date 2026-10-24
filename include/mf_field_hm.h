#ifndef GUARD_MF_FIELD_HM_H
#define GUARD_MF_FIELD_HM_H

#include "global.h"

bool8 MfFieldHm_OverwriteActive(void);
bool32 MfFieldHm_BagHasMove(enum Move move);
bool32 MfFieldHm_MonCanUse(struct Pokemon *mon, enum Move move, u8 slotId);
u8 MfFieldHm_FindPartySlot(enum Move move);

#endif // GUARD_MF_FIELD_HM_H
