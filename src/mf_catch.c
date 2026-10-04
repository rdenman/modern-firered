#include "global.h"
#include "mf_catch.h"
#include "mf_rules.h"

static const u8 sText_NoEscapeRopeDig[] = _(
    "Escape Rope and Dig can't\n"
    "be used to exit dungeons.{PAUSE_UNTIL_PRESS}");

s32 MfScaleCatchRate(s32 catchRate, u8 mode)
{
    switch (mode)
    {
    case MF_CATCH_RATE_HALF:
        catchRate /= 2;
        if (catchRate < MF_CATCH_RATE_FLOOR)
            catchRate = MF_CATCH_RATE_FLOOR;
        break;
    case MF_CATCH_RATE_2X:
        catchRate *= 2;
        if (catchRate > MF_CATCH_RATE_CAP)
            catchRate = MF_CATCH_RATE_CAP;
        break;
    case MF_CATCH_RATE_3X:
        catchRate *= 3;
        if (catchRate > MF_CATCH_RATE_CAP)
            catchRate = MF_CATCH_RATE_CAP;
        break;
    case MF_CATCH_RATE_DEFAULT:
    default:
        break;
    }

    return catchRate;
}

s32 MfApplyCatchRateRule(s32 catchRate)
{
    return MfScaleCatchRate(catchRate, MfRules_GetCatchRate());
}

u32 MfComputeFleeChanceVar(u32 playerSpeed, u32 foeSpeed, u32 runTries)
{
    if (foeSpeed == 0)
        foeSpeed = 1;
    return (playerSpeed * 128) / foeSpeed + runTries * 30;
}

bool32 MfLessEscapesFleeSucceeds(u32 playerSpeed, u32 foeSpeed, u32 runTries, u32 roll)
{
    return MfComputeFleeChanceVar(playerSpeed, foeSpeed, runTries) > (roll & MF_LESS_ESCAPES_ROLL_MASK);
}

bool8 MfAreEscapeRopeAndDigAllowed(void)
{
    return !MfRules_BansEscapeRopeDig();
}

const u8 *MfGetNoEscapeRopeDigMessage(void)
{
    return sText_NoEscapeRopeDig;
}
