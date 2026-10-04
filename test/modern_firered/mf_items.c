#include "global.h"
#include "event_data.h"
#include "item.h"
#include "mf_items.h"
#include "mf_rules.h"
#include "money.h"
#include "constants/flags.h"
#include "constants/hold_effects.h"
#include "constants/item.h"
#include "constants/item_effects.h"
#include "constants/items.h"
#include "test/test.h"

static struct ModernRules *PrepareCustomRules(void)
{
    struct ModernRules *save = MfRules_GetSaveRules();

    MfRules_ApplyDevDefaults(save);
    MfRules_ApplyGamemodePreset(save, MF_GAMEMODE_CUSTOM);
    save->rulesLocked = TRUE;
    return save;
}

static void RestorePhase1Defaults(void)
{
    struct ModernRules *save = MfRules_GetSaveRules();

    save->version = 0;
}

TEST("MF: reusable TMs gate GetItemImportance for TMs only")
{
    struct ModernRules *save = PrepareCustomRules();

    save->infiniteTms = TRUE;
    EXPECT_NE(GetItemImportance(ITEM_TM01), (u8)0);
    EXPECT_EQ(GetItemImportance(ITEM_HM_CUT), (u8)1);

    save->infiniteTms = FALSE;
    EXPECT_EQ(GetItemImportance(ITEM_TM01), (u8)0);
    EXPECT_EQ(GetItemImportance(ITEM_HM_CUT), (u8)1);

    RestorePhase1Defaults();
}

TEST("MF: modern Sitrus gate switches hold effect and bag heal")
{
    struct ModernRules *save = PrepareCustomRules();
    const u8 *effect;

    save->modernSitrus = TRUE;
    EXPECT_EQ(GetItemHoldEffect(ITEM_SITRUS_BERRY), HOLD_EFFECT_RESTORE_PCT_HP);
    EXPECT_EQ(GetItemHoldEffectParam(ITEM_SITRUS_BERRY), (u32)25);
    effect = GetItemEffect(ITEM_SITRUS_BERRY);
    EXPECT_EQ(effect[6], (u8)ITEM6_HEAL_HP_QUARTER);

    save->modernSitrus = FALSE;
    EXPECT_EQ(GetItemHoldEffect(ITEM_SITRUS_BERRY), HOLD_EFFECT_RESTORE_HP);
    EXPECT_EQ(GetItemHoldEffectParam(ITEM_SITRUS_BERRY), (u32)30);
    effect = GetItemEffect(ITEM_SITRUS_BERRY);
    EXPECT_EQ(effect[6], (u8)30);

    RestorePhase1Defaults();
}

TEST("MF: nature mint shop availability follows mints rule + badge/clear")
{
    struct ModernRules *save = PrepareCustomRules();

    FlagClear(FLAG_BADGE04_GET);
    FlagClear(FLAG_SYS_GAME_CLEAR);

    save->mints = TRUE;
    EXPECT(!MfAreNatureMintsBuyable());
    FlagSet(FLAG_BADGE04_GET);
    EXPECT(MfAreNatureMintsBuyable());
    EXPECT(IsItemShopCriteriaFulfilled(ITEM_ADAMANT_MINT));

    FlagClear(FLAG_BADGE04_GET);
    save->mints = FALSE;
    EXPECT(!MfAreNatureMintsBuyable());
    FlagSet(FLAG_SYS_GAME_CLEAR);
    EXPECT(MfAreNatureMintsBuyable());

    FlagClear(FLAG_SYS_GAME_CLEAR);
    RestorePhase1Defaults();
}

TEST("MF: typed helpers mirror gamemode bools for S31 toggles")
{
    struct ModernRules *save = PrepareCustomRules();

    save->survivePoison = FALSE;
    EXPECT_EQ((u32)MfRules_HasSurvivePoison(), (u32)FALSE);
    save->survivePoison = TRUE;
    EXPECT_EQ((u32)MfRules_HasSurvivePoison(), (u32)TRUE);

    save->synchronize = FALSE;
    EXPECT_EQ((u32)MfRules_HasSynchronize(), (u32)FALSE);
    save->sturdy = FALSE;
    EXPECT_EQ((u32)MfRules_HasSturdy(), (u32)FALSE);

    RestorePhase1Defaults();
}

TEST("MF: shop prices scale 1x / 5x / 10x / 50x and cap at MAX_MONEY")
{
    EXPECT_EQ(MfScaleShopPrice(200, MF_SHOP_PRICE_OFF), 200u);
    EXPECT_EQ(MfScaleShopPrice(200, MF_SHOP_PRICE_X5), 1000u);
    EXPECT_EQ(MfScaleShopPrice(200, MF_SHOP_PRICE_X10), 2000u);
    EXPECT_EQ(MfScaleShopPrice(200, MF_SHOP_PRICE_X50), 10000u);
    EXPECT_EQ(MfScaleShopPrice(0, MF_SHOP_PRICE_X50), 0u);
    EXPECT_EQ(MfScaleShopPrice(200, 4), 200u);
    EXPECT_EQ(MfScaleShopPrice(MAX_MONEY, MF_SHOP_PRICE_X5), (u32)MAX_MONEY);
    EXPECT_EQ(MfScaleShopPrice(200, MF_SHOP_PRICE_X5) / ITEM_SELL_FACTOR, 250u);
}

TEST("MF: live shop-price rule, PC heal, and Poké Center block")
{
    struct ModernRules *save = PrepareCustomRules();
    u32 potion = GetItemPrice(ITEM_POTION);

    save->expensiveShops = MF_SHOP_PRICE_OFF;
    save->pokeCenterLimit = 0;
    save->noPcHeal = FALSE;
    potion = GetItemPrice(ITEM_POTION);
    EXPECT_EQ(GetItemSellPrice(ITEM_POTION), potion / ITEM_SELL_FACTOR);
    EXPECT(!MfIsPokecenterHealingBlocked());
    EXPECT(MfShouldHealOnPcDeposit());
    EXPECT_EQ((u32)MfIsPokecenterChallengeActivated(), 0u);

    save->expensiveShops = MF_SHOP_PRICE_X5;
    EXPECT_EQ(GetItemPrice(ITEM_POTION), potion * 5);
    EXPECT_EQ(GetItemSellPrice(ITEM_POTION), (potion * 5) / ITEM_SELL_FACTOR);

    save->expensiveShops = MF_SHOP_PRICE_X10;
    EXPECT_EQ(GetItemPrice(ITEM_POTION), potion * 10);
    save->expensiveShops = MF_SHOP_PRICE_X50;
    EXPECT_EQ(GetItemPrice(ITEM_POTION), potion * 50);

    save->noPcHeal = TRUE;
    EXPECT(!MfShouldHealOnPcDeposit());
    save->noPcHeal = FALSE;
    save->pokeCenterLimit = 1;
    EXPECT(MfIsPokecenterHealingBlocked());
    EXPECT(!MfShouldHealOnPcDeposit());
    EXPECT_EQ((u32)MfIsPokecenterChallengeActivated(), 1u);

    RestorePhase1Defaults();
}
