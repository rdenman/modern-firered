#include "global.h"
#include "event_data.h"
#include "item.h"
#include "mf_exp_all.h"
#include "mf_rules.h"
#include "string_util.h"
#include "text.h"
#include "constants/characters.h"
#include "constants/items.h"
#include "constants/vars.h"

static const u8 sExpAllDescBase[] = _(
    "Shares battle Exp.\n"
    "with the whole party.");

static const u8 sText_ExpAllOn[] = _(
    "The Exp. All has been turned on.{PAUSE_UNTIL_PRESS}");

static const u8 sText_ExpAllOff[] = _(
    "The Exp. All has been turned off.{PAUSE_UNTIL_PRESS}");

static const u8 sText_On[] = _("On");
static const u8 sText_Off[] = _("Off");

static u8 sExpAllDescBuffer[64];

bool32 MfExpAllIsEnabled(void)
{
    if (!MfRules_HasExpAll())
        return FALSE;
    return VarGet(VAR_EXP_ALL_ON) != 0;
}

void MfExpAllSetEnabled(bool32 enable)
{
    VarSet(VAR_EXP_ALL_ON, enable ? 1 : 0);
}

bool32 MfExpAllToggle(void)
{
    MfExpAllSetEnabled(VarGet(VAR_EXP_ALL_ON) == 0);
    return MfExpAllIsEnabled();
}

void MfExpAllGrantOnNewGame(void)
{
    if (!MfRules_HasExpAll())
        return;
    if (!CheckBagHasItem(ITEM_EXP_ALL, 1))
        AddBagItem(ITEM_EXP_ALL, 1);
    VarSet(VAR_EXP_ALL_ON, 1);
}

const u8 *MfExpAllGetDescription(void)
{
    u8 *end;

    StringCopy(sExpAllDescBuffer, sExpAllDescBase);
    end = sExpAllDescBuffer;
    while (*end != EOS)
        end++;
    *end++ = CHAR_SPACE;
    *end++ = CHAR_LEFT_PAREN;
    end = StringCopy(end, MfExpAllIsEnabled() ? sText_On : sText_Off);
    *end++ = CHAR_RIGHT_PAREN;
    *end = EOS;
    return sExpAllDescBuffer;
}

const u8 *MfExpAllGetToggleMessage(bool32 nowOn)
{
    return nowOn ? sText_ExpAllOn : sText_ExpAllOff;
}
