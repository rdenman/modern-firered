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

bool32 MfIsHmItem(enum Item itemId);
bool32 MfIsItemProgressionProtected(enum Item itemId);
// FRLG/RSE items plus later-gen evo stones/holds used by Gen 1–3 lines.
bool32 MfIsItemRandomPoolCandidate(enum Item itemId);

// S57 ITEMS. Per-chest hash into the Gen 3 pool (ADR 0098). Active is identity unless ITEMS is on.
// Oak's ITEM_POKE_BALL grants stay pinned. Marts stay vanilla (ADR 0088).
enum Item MfItemMapExAt(enum Item itemId, u32 seed, u32 siteId);
enum Item MfItemMapEx(enum Item itemId, u32 seed);
enum Item MfItemMapActive(enum Item itemId);
// Identity: poké marts never remap (ADR 0088).
const u16 *MfMaybeRandomizeShopStock(const u16 *items);
// Bedroom PC Potion is filled before rules SAVE; rewrite that slot at lock (ADR 0092).
void MfRemapNewGamePCItems(void);

void MfScriptRemapGrantItem(void);
void MfScriptRemapHiddenItem(void);

#endif // GUARD_MF_ITEMS_H
