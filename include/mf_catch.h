#ifndef GUARD_MF_CATCH_H
#define GUARD_MF_CATCH_H

// S45 — CATCH RATE, LESS ESCAPES, ESC. ROPE / DIG. See ADR 0048.

#include "gba/types.h"

#define MF_CATCH_RATE_DEFAULT 0
#define MF_CATCH_RATE_HALF    1
#define MF_CATCH_RATE_2X      2
#define MF_CATCH_RATE_3X      3

#define MF_CATCH_RATE_FLOOR 3
#define MF_CATCH_RATE_CAP   255

// ME uses Random() & 512, which is 0 or 512 and never lets a u8 speedVar win.
// Mask 511 is the intended harder roll; u32 chance vars make extra attempts help.
#define MF_LESS_ESCAPES_ROLL_MASK 0x1FF

// Menu 0 = 1x, 1 = 0.5x (floor 3), 2 = 2x (cap 255), 3 = 3x (cap 255).
s32 MfScaleCatchRate(s32 catchRate, u8 mode);
s32 MfApplyCatchRateRule(s32 catchRate);

// LESS ESCAPES always rolls, even when the player is faster.
u32 MfComputeFleeChanceVar(u32 playerSpeed, u32 foeSpeed, u32 runTries);
bool32 MfLessEscapesFleeSucceeds(u32 playerSpeed, u32 foeSpeed, u32 runTries, u32 roll);

// ESC. ROPE / DIG: Yes (0) allowed, No (1) banned.
bool8 MfAreEscapeRopeAndDigAllowed(void);
const u8 *MfGetNoEscapeRopeDigMessage(void);

#endif // GUARD_MF_CATCH_H
