#ifndef GUARD_MF_BRANDING_H
#define GUARD_MF_BRANDING_H

#include "gba/types.h"

// S67 — player-facing version and credits. See ADR 0081.

u8 *MfBranding_AppendDottedVersion(u8 *dest, u32 major, u32 minor, u32 patch);
u8 *MfBranding_AppendReleaseVersion(u8 *dest);
u8 *MfBranding_AppendExpansionVersion(u8 *dest);
void MfBranding_WriteShortVersion(u8 *dest); // "v0.1.0"
void MfBranding_WriteCredits(u8 *dest);

#endif // GUARD_MF_BRANDING_H
