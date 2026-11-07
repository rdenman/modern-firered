#ifndef GUARD_MF_EXP_ALL_H
#define GUARD_MF_EXP_ALL_H

#include "global.h"

bool32 MfExpAllIsEnabled(void);
void MfExpAllSetEnabled(bool32 enable);
bool32 MfExpAllToggle(void);
void MfExpAllGrantOnNewGame(void);
const u8 *MfExpAllGetDescription(void);
const u8 *MfExpAllGetToggleMessage(bool32 nowOn);

#endif // GUARD_MF_EXP_ALL_H
