#include "global.h"
#include "test/test.h"
#include "config/item.h"
#include "item.h"
#include "item_use.h"
#include "constants/items.h"
#include "constants/pokemon.h"

// S10: lock Phase 1 item QoL baseline (reusable TMs, Gen5 Exp Share, Gen4+ Sitrus).
// Shared Emerald TESTELF path — same item configs as the FireRed ROM.
// TM reuse / mint use in-game: docs-mf/manual-qa-s10-item-qol.md

TEST("MF: item QoL configs — reusable TMs, held Exp Share, Gen4+ Sitrus")
{
    EXPECT(I_REUSABLE_TMS);
    EXPECT_EQ(I_EXP_SHARE_FLAG, 0);
    EXPECT(I_EXP_SHARE_ITEM < GEN_6);
    EXPECT(I_SITRUS_BERRY_HEAL >= GEN_4);
}

// S69: PLA-style bag use for trade-held evo items (Metal Coat, Electirizer, …).
TEST("MF: item evo held items usable from bag (solo trade evolutions)")
{
    EXPECT(I_USE_EVO_HELD_ITEMS_FROM_BAG);
    EXPECT_EQ(GetItemType(ITEM_METAL_COAT), ITEM_USE_PARTY_MENU);
    EXPECT_EQ(GetItemType(ITEM_KINGS_ROCK), ITEM_USE_PARTY_MENU);
    EXPECT_EQ(GetItemType(ITEM_DRAGON_SCALE), ITEM_USE_PARTY_MENU);
    EXPECT_EQ(GetItemType(ITEM_UPGRADE), ITEM_USE_PARTY_MENU);
    EXPECT_EQ(GetItemType(ITEM_ELECTIRIZER), ITEM_USE_PARTY_MENU);
    EXPECT_EQ(GetItemFieldFunc(ITEM_METAL_COAT), ItemUseOutOfBattle_EvolutionStone);
    EXPECT_EQ(GetItemFieldFunc(ITEM_LINKING_CORD), ItemUseOutOfBattle_EvolutionStone);
    EXPECT_EQ(GetItemType(ITEM_LINKING_CORD), ITEM_USE_PARTY_MENU);
    EXPECT_EQ(GetItemFieldFunc(ITEM_DAWN_STONE), ItemUseOutOfBattle_EvolutionStone);
    EXPECT_EQ(GetItemType(ITEM_RAZOR_CLAW), ITEM_USE_PARTY_MENU);
    EXPECT_EQ(GetItemFieldFunc(ITEM_RAZOR_CLAW), ItemUseOutOfBattle_EvolutionStone);
    EXPECT_EQ(GetItemType(ITEM_OVAL_STONE), ITEM_USE_PARTY_MENU);
}

TEST("MF: item sample TMs are marked important (reusable)")
{
    EXPECT(GetItemImportance(ITEM_TM01));
    EXPECT(GetItemImportance(ITEM_TM05));
    EXPECT(GetItemImportance(ITEM_TM28));
}

TEST("MF: mart prices and Escape Rope are Gen 3 vanilla")
{
    EXPECT_EQ(I_PRICE, GEN_3);
    EXPECT(I_KEY_ESCAPE_ROPE < GEN_8);
    EXPECT(I_SELL_VALUE_FRACTION < GEN_9);
    EXPECT_EQ(ITEM_SELL_FACTOR, 2);
    EXPECT_EQ(gItemsInfo[ITEM_POKE_BALL].price, 200u);
    EXPECT_EQ(gItemsInfo[ITEM_POTION].price, 300u);
    EXPECT_EQ(gItemsInfo[ITEM_ANTIDOTE].price, 100u);
    EXPECT_EQ(gItemsInfo[ITEM_PARALYZE_HEAL].price, 200u);
    EXPECT_EQ(gItemsInfo[ITEM_BURN_HEAL].price, 250u);
    EXPECT_EQ(gItemsInfo[ITEM_AWAKENING].price, 250u);
    EXPECT_EQ(gItemsInfo[ITEM_REPEL].price, 350u);
    EXPECT_EQ(gItemsInfo[ITEM_ESCAPE_ROPE].price, 550u);
    EXPECT_EQ((u8)gItemsInfo[ITEM_ESCAPE_ROPE].importance, (u8)0);
    EXPECT_EQ(GetItemPocket(ITEM_ESCAPE_ROPE), POCKET_ITEMS);
}

TEST("MF: item nature mints exist and map to natures")
{
    EXPECT_EQ(gItemsInfo[ITEM_ADAMANT_MINT].secondaryId, NATURE_ADAMANT);
    EXPECT_EQ(gItemsInfo[ITEM_MODEST_MINT].secondaryId, NATURE_MODEST);
    EXPECT_EQ(gItemsInfo[ITEM_JOLLY_MINT].secondaryId, NATURE_JOLLY);
    EXPECT_EQ(gItemsInfo[ITEM_SERIOUS_MINT].secondaryId, NATURE_SERIOUS);
    EXPECT_EQ(GetItemPocket(ITEM_ADAMANT_MINT), POCKET_ITEMS);
}
