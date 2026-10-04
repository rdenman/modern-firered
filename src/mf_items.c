#include "global.h"
#include "event_data.h"
#include "item.h"
#include "mf_items.h"
#include "mf_rules.h"
#include "money.h"
#include "constants/flags.h"
#include "constants/item_effects.h"
#include "constants/items.h"

// Gen 3 Sitrus: flat 30 HP heal from the bag / battle item path.
static const u8 sItemEffect_SitrusBerryClassic[7] = {
    [4] = ITEM4_HEAL_HP,
    [6] = 30,
};

static const u8 sDesc_SitrusBerryClassic[] = _(
    "A held item that\n"
    "restores 30 HP in\n"
    "battle.");

bool32 MfIsTmItem(enum Item itemId)
{
    return itemId >= ITEM_TM01 && itemId <= ITEM_TM100;
}

bool32 MfIsNatureMintItem(enum Item itemId)
{
    return itemId >= ITEM_LONELY_MINT && itemId <= ITEM_SERIOUS_MINT;
}

// ME: On → Pretty Petal after 4th badge; Off → postgame only.
// FR: Celadon Dept Store 2F after Rainbow Badge (BADGE04) when the rule is on.
bool32 MfAreNatureMintsBuyable(void)
{
    if (MfRules_HasMints())
        return FlagGet(FLAG_BADGE04_GET);
    return FlagGet(FLAG_SYS_GAME_CLEAR);
}

const u8 *MfGetClassicSitrusItemEffect(void)
{
    return sItemEffect_SitrusBerryClassic;
}

const u8 *MfGetClassicSitrusDescription(void)
{
    return sDesc_SitrusBerryClassic;
}

// ME gText_BattleRules_NoItems_Player.
static const u8 sText_NoPlayerBattleItems[] = _(
    "Competitive rules!\n"
    "No items in battle!{PAUSE_UNTIL_PRESS}");

const u8 *MfGetNoPlayerBattleItemsMessage(void)
{
    return sText_NoPlayerBattleItems;
}

bool32 MfIsBattlePokeBall(enum Item itemId)
{
    return GetItemBattleUsage(itemId) == EFFECT_ITEM_THROW_BALL;
}

bool32 MfIsPlayerBattleItemAllowed(enum Item itemId)
{
    if (!MfRules_HasNoItemPlayer())
        return TRUE;
    return MfIsBattlePokeBall(itemId);
}

bool32 MfAreTrainerBattleItemsAllowed(void)
{
    return !MfRules_HasNoItemTrainer();
}

bool32 MfIsBattlerBattleItemAllowed(bool32 isPlayerSide, enum Item itemId)
{
    if (isPlayerSide)
        return MfIsPlayerBattleItemAllowed(itemId);
    return MfAreTrainerBattleItemsAllowed();
}

static u32 GetShopPriceMultiplier(u8 mode)
{
    switch (mode)
    {
    case MF_SHOP_PRICE_X5:
        return 5;
    case MF_SHOP_PRICE_X10:
        return 10;
    case MF_SHOP_PRICE_X50:
        return 50;
    case MF_SHOP_PRICE_OFF:
    default:
        return 1;
    }
}

u32 MfScaleShopPrice(u32 price, u8 mode)
{
    u32 mult = GetShopPriceMultiplier(mode);

    if (price == 0 || mult == 1)
        return price;
    if (price > MAX_MONEY / mult)
        return MAX_MONEY;
    return price * mult;
}

u32 MfApplyShopPriceRule(u32 price)
{
    return MfScaleShopPrice(price, MfRules_GetExpensiveShops());
}

bool8 MfIsPokecenterHealingBlocked(void)
{
    return MfRules_GetPokeCenterLimit() != 0;
}

u16 MfIsPokecenterChallengeActivated(void)
{
    return MfIsPokecenterHealingBlocked();
}

bool8 MfShouldHealOnPcDeposit(void)
{
    if (MfIsPokecenterHealingBlocked() || MfRules_HasNoPcHeal())
        return FALSE;
    return TRUE;
}
