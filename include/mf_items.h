#ifndef GUARD_MF_ITEMS_H
#define GUARD_MF_ITEMS_H

#include "global.h"
#include "item.h"

bool32 MfIsTmItem(enum Item itemId);
bool32 MfIsNatureMintItem(enum Item itemId);
bool32 MfAreNatureMintsBuyable(void);

const u8 *MfGetClassicSitrusItemEffect(void);
const u8 *MfGetClassicSitrusDescription(void);

// S43 — PLAYER ITEMS / TRAINER ITEMS. Balls stay legal so catching works.
const u8 *MfGetNoPlayerBattleItemsMessage(void);
bool32 MfIsBattlePokeBall(enum Item itemId);
bool32 MfIsPlayerBattleItemAllowed(enum Item itemId);
bool32 MfAreTrainerBattleItemsAllowed(void);
bool32 MfIsBattlerBattleItemAllowed(bool32 isPlayerSide, enum Item itemId);

// S46 — POKéCENTER / PC HEALS / ULTRA EXPENSIVE!. See ADR 0049.
#define MF_SHOP_PRICE_OFF 0
#define MF_SHOP_PRICE_X5  1
#define MF_SHOP_PRICE_X10 2
#define MF_SHOP_PRICE_X50 3

u32 MfScaleShopPrice(u32 price, u8 mode);
u32 MfApplyShopPriceRule(u32 price);
bool8 MfIsPokecenterHealingBlocked(void);
u16 MfIsPokecenterChallengeActivated(void);
bool8 MfShouldHealOnPcDeposit(void);

#endif // GUARD_MF_ITEMS_H
