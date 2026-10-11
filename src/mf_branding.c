#include "global.h"
#include "config/modern_firered.h"
#include "constants/characters.h"
#include "constants/expansion.h"
#include "mf_branding.h"
#include "string_util.h"

static const u8 sCreditsBased[] = _("Based off RHH expansion ");
static const u8 sCreditsInspired[] = _(".\nInspired by Modern Emerald.");

u8 *MfBranding_AppendDottedVersion(u8 *dest, u32 major, u32 minor, u32 patch)
{
    if (dest == NULL)
        return dest;

    dest = ConvertIntToDecimalStringN(dest, major, STR_CONV_MODE_LEFT_ALIGN, 2);
    *dest++ = CHAR_PERIOD;
    dest = ConvertIntToDecimalStringN(dest, minor, STR_CONV_MODE_LEFT_ALIGN, 2);
    *dest++ = CHAR_PERIOD;
    dest = ConvertIntToDecimalStringN(dest, patch, STR_CONV_MODE_LEFT_ALIGN, 2);
    *dest = EOS;
    return dest;
}

u8 *MfBranding_AppendReleaseVersion(u8 *dest)
{
    return MfBranding_AppendDottedVersion(dest,
        MF_RELEASE_VERSION_MAJOR,
        MF_RELEASE_VERSION_MINOR,
        MF_RELEASE_VERSION_PATCH);
}

u8 *MfBranding_AppendExpansionVersion(u8 *dest)
{
    return MfBranding_AppendDottedVersion(dest,
        EXPANSION_VERSION_MAJOR,
        EXPANSION_VERSION_MINOR,
        EXPANSION_VERSION_PATCH);
}

void MfBranding_WriteShortVersion(u8 *dest)
{
    if (dest == NULL)
        return;

    *dest++ = CHAR_v;
    MfBranding_AppendReleaseVersion(dest);
}

void MfBranding_WriteCredits(u8 *dest)
{
    if (dest == NULL)
        return;

    dest = StringCopy(dest, sCreditsBased);
    dest = MfBranding_AppendExpansionVersion(dest);
    StringCopy(dest, sCreditsInspired);
}
