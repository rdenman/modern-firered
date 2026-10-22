#include "global.h"
#include "event_data.h"
#include "item.h"
#include "mf_items.h"
#include "mf_random.h"
#include "mf_tm_random.h"
#include "mf_rules.h"
#include "money.h"
#include "move.h"
#include "string_util.h"
#include "constants/characters.h"
#include "constants/flags.h"
#include "constants/hold_effects.h"
#include "constants/item_effects.h"
#include "constants/items.h"
#include "constants/maps.h"
#include "constants/vars.h"

#define MF_ITEM_POOL_MAX 1024

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

bool32 MfIsHmItem(enum Item itemId)
{
    return itemId >= ITEM_HM01 && itemId <= ITEM_HM08;
}

static bool32 MfIsFossilItem(enum Item itemId)
{
    return itemId >= ITEM_HELIX_FOSSIL && itemId <= ITEM_FOSSILIZED_DINO;
}

bool32 MfIsItemProgressionProtected(enum Item itemId)
{
    if (itemId == ITEM_NONE || itemId >= ITEMS_COUNT)
        return TRUE;
    if (MfIsHmItem(itemId))
        return TRUE;
    // I_KEY_FOSSILS is GEN_LATEST, so these sit in the Items pocket with
    // importance 0. They are still unique revive grants (ADR 0099).
    if (MfIsFossilItem(itemId))
        return TRUE;
    if (GetItemPocket(itemId) == POCKET_KEY_ITEMS)
        return TRUE;
    if (GetItemImportance(itemId) != 0 && !MfIsTmItem(itemId))
        return TRUE;
    return FALSE;
}

static bool32 ItemInRange(enum Item itemId, enum Item lo, enum Item hi)
{
    return itemId >= lo && itemId <= hi;
}

static bool8 ItemIsDummy(enum Item itemId)
{
    const u8 *name = GetItemName(itemId);

    return name == NULL
        || name == gQuestionMarksItemName
        || name[0] == EOS
        || name[0] == CHAR_QUESTION_MARK;
}

static bool32 IsClassicBall(enum Item itemId)
{
    switch (itemId)
    {
    case ITEM_POKE_BALL:
    case ITEM_GREAT_BALL:
    case ITEM_ULTRA_BALL:
    case ITEM_MASTER_BALL:
    case ITEM_PREMIER_BALL:
    case ITEM_NET_BALL:
    case ITEM_NEST_BALL:
    case ITEM_DIVE_BALL:
    case ITEM_REPEAT_BALL:
    case ITEM_TIMER_BALL:
    case ITEM_LUXURY_BALL:
    case ITEM_SAFARI_BALL:
        return TRUE;
    default:
        return FALSE;
    }
}

static bool32 IsCrossGenEvoItem(enum Item itemId)
{
    switch (itemId)
    {
    case ITEM_ICE_STONE:
    case ITEM_SHINY_STONE:
    case ITEM_DUSK_STONE:
    case ITEM_DAWN_STONE:
    case ITEM_PROTECTOR:
    case ITEM_ELECTIRIZER:
    case ITEM_MAGMARIZER:
    case ITEM_DUBIOUS_DISC:
    case ITEM_REAPER_CLOTH:
    case ITEM_PRISM_SCALE:
    case ITEM_OVAL_STONE:
    case ITEM_RAZOR_CLAW:
    case ITEM_RAZOR_FANG:
    case ITEM_LINKING_CORD:
    case ITEM_BLACK_AUGURITE:
        return TRUE;
    default:
        return FALSE;
    }
}

bool32 MfIsItemRandomPoolCandidate(enum Item itemId)
{
    enum HoldEffect hold;

    if (MfIsItemProgressionProtected(itemId) || ItemIsDummy(itemId))
        return FALSE;

    hold = GetItemHoldEffect(itemId);
    if (hold == HOLD_EFFECT_MEGA_STONE
     || hold == HOLD_EFFECT_Z_CRYSTAL
     || hold == HOLD_EFFECT_GEMS
     || hold == HOLD_EFFECT_PLATE
     || hold == HOLD_EFFECT_DRIVE
     || hold == HOLD_EFFECT_MEMORY
     || hold == HOLD_EFFECT_PRIMAL_ORB)
        return FALSE;

    if (IsClassicBall(itemId) || IsCrossGenEvoItem(itemId))
        return TRUE;

    if (ItemInRange(itemId, ITEM_POTION, ITEM_SACRED_ASH))
        return TRUE;
    if (itemId == ITEM_RAGE_CANDY_BAR || itemId == ITEM_LAVA_COOKIE)
        return TRUE;
    if (ItemInRange(itemId, ITEM_HP_UP, ITEM_PP_MAX))
        return TRUE;
    if (itemId == ITEM_RARE_CANDY)
        return TRUE;
    if (ItemInRange(itemId, ITEM_BLUE_FLUTE, ITEM_WHITE_FLUTE))
        return TRUE;
    if (ItemInRange(itemId, ITEM_REPEL, ITEM_MAX_REPEL))
        return TRUE;
    if (itemId == ITEM_ESCAPE_ROPE)
        return TRUE;
    if (ItemInRange(itemId, ITEM_X_ATTACK, ITEM_GUARD_SPEC))
        return TRUE;
    if (itemId == ITEM_POKE_DOLL || itemId == ITEM_FLUFFY_TAIL)
        return TRUE;
    if (itemId == ITEM_NUGGET
     || itemId == ITEM_TINY_MUSHROOM
     || itemId == ITEM_BIG_MUSHROOM
     || itemId == ITEM_PEARL
     || itemId == ITEM_BIG_PEARL
     || itemId == ITEM_STARDUST
     || itemId == ITEM_STAR_PIECE)
        return TRUE;
    if (ItemInRange(itemId, ITEM_SHOAL_SALT, ITEM_HEART_SCALE))
        return TRUE;
    if (ItemInRange(itemId, ITEM_FIRE_STONE, ITEM_MOON_STONE))
        return TRUE;
    if (itemId == ITEM_DRAGON_SCALE || itemId == ITEM_UPGRADE || itemId == ITEM_EVERSTONE)
        return TRUE;
    if (ItemInRange(itemId, ITEM_LIGHT_BALL, ITEM_SOUL_DEW))
        return TRUE;
    if (ItemInRange(itemId, ITEM_SEA_INCENSE, ITEM_PURE_INCENSE))
        return TRUE;
    if (itemId == ITEM_MACHO_BRACE || itemId == ITEM_CHOICE_BAND)
        return TRUE;
    if (ItemInRange(itemId, ITEM_SILK_SCARF, ITEM_METAL_COAT))
        return TRUE;
    if (ItemInRange(itemId, ITEM_BRIGHT_POWDER, ITEM_SHELL_BELL))
        return TRUE;
    if (ItemInRange(itemId, ITEM_CHERI_BERRY, ITEM_BELUE_BERRY))
        return TRUE;
    if (ItemInRange(itemId, ITEM_LIECHI_BERRY, ITEM_ENIGMA_BERRY))
        return TRUE;
    if (ItemInRange(itemId, ITEM_TM01, ITEM_TM50))
        return TRUE;

    return FALSE;
}

#if MF_RANDOMIZER

#define MF_ITEM_SITE_NEW_GAME_PC 0x7F000001u
#define MF_ITEM_SITE_HIDDEN_BIT  0x80000000u

// TMs occupy [0, sTmPoolCount); other dests occupy the rest (ADR 0101).
static EWRAM_DATA u16 sItemPool[MF_ITEM_POOL_MAX] = {0};
static EWRAM_DATA u16 sTmPoolCount = 0;
static EWRAM_DATA u16 sOtherPoolCount = 0;
static EWRAM_DATA bool8 sItemPoolReady = FALSE;

static void EnsureItemPool(void)
{
    enum Item item;

    if (sItemPoolReady)
        return;

    sTmPoolCount = 0;
    sOtherPoolCount = 0;
    for (item = 1; item < ITEMS_COUNT && sTmPoolCount < MF_ITEM_POOL_MAX; item++)
    {
        if (!MfIsItemRandomPoolCandidate(item) || !MfIsTmItem(item))
            continue;
        sItemPool[sTmPoolCount++] = item;
    }
    for (item = 1; item < ITEMS_COUNT && (sTmPoolCount + sOtherPoolCount) < MF_ITEM_POOL_MAX; item++)
    {
        if (!MfIsItemRandomPoolCandidate(item) || MfIsTmItem(item))
            continue;
        sItemPool[sTmPoolCount + sOtherPoolCount] = item;
        sOtherPoolCount++;
    }
    sItemPoolReady = TRUE;
}

static bool32 MfIsOaksLabPokeBallGrant(enum Item itemId)
{
    if (itemId != ITEM_POKE_BALL || gSaveBlock1Ptr == NULL)
        return FALSE;
    return gSaveBlock1Ptr->location.mapGroup == MAP_GROUP(MAP_PALLET_TOWN_PROFESSOR_OAKS_LAB)
        && gSaveBlock1Ptr->location.mapNum == MAP_NUM(MAP_PALLET_TOWN_PROFESSOR_OAKS_LAB);
}

static u32 MfItemSiteObject(void)
{
    u32 group = 0;
    u32 num = 0;

    if (gSaveBlock1Ptr != NULL)
    {
        group = (u8)gSaveBlock1Ptr->location.mapGroup;
        num = (u8)gSaveBlock1Ptr->location.mapNum;
    }
    return (group << 24) | (num << 16) | (gSpecialVar_LastTalked & 0xFFFF);
}

enum Item MfItemMapExAt(enum Item itemId, u32 seed, u32 siteId)
{
    u16 pick;
    u16 bound;
    u16 offset;

    if (MfIsItemProgressionProtected(itemId))
        return itemId;

    EnsureItemPool();
    if (MfIsTmItem(itemId))
    {
        bound = sTmPoolCount;
        offset = 0;
    }
    else
    {
        bound = sOtherPoolCount;
        offset = sTmPoolCount;
    }
    if (bound == 0)
        return itemId;

    pick = MfRandom_Modulo(seed, MF_RANDOM_CAT_ITEMS, siteId, (u16)itemId, bound);
    return sItemPool[offset + pick];
}

enum Item MfItemMapEx(enum Item itemId, u32 seed)
{
    return MfItemMapExAt(itemId, seed, 0);
}

enum Item MfItemMapActive(enum Item itemId)
{
    const struct ModernRules *rules = MfRules_GetActiveRules();

    if (!rules->randomItems)
        return itemId;
    return MfItemMapExAt(itemId, rules->randomizerSeed, 0);
}

// Bedroom PC is filled in NewGameInitData before the rules menu SAVE, so the
// starter Potion is still vanilla until lock. Rewrite that slot in place;
// skip anything else so a debug re-commit cannot double-map deposits.
void MfRemapNewGamePCItems(void)
{
    const struct ModernRules *rules;

    if (gSaveBlock1Ptr == NULL)
        return;
    rules = MfRules_GetActiveRules();
    if (!rules->randomItems)
        return;
    if (gSaveBlock1Ptr->pcItems[0].itemId != ITEM_POTION)
        return;
    if (gSaveBlock1Ptr->pcItems[0].quantity != 1)
        return;
    gSaveBlock1Ptr->pcItems[0].itemId = MfItemMapExAt(ITEM_POTION, rules->randomizerSeed, MF_ITEM_SITE_NEW_GAME_PC);
}

const u16 *MfMaybeRandomizeShopStock(const u16 *items)
{
    // ADR 0088: marts stay vanilla so potions and other staples stay buyable.
    return items;
}

void MfScriptRemapGrantItem(void)
{
    const struct ModernRules *rules = MfRules_GetActiveRules();
    enum Item itemId;

    if (!rules->randomItems)
        return;
    itemId = VarGet(VAR_0x8000);
    if (MfIsOaksLabPokeBallGrant(itemId))
        return;
    VarSet(VAR_0x8000, MfItemMapExAt(itemId, rules->randomizerSeed, MfItemSiteObject()));
}

void MfScriptRemapHiddenItem(void)
{
    const struct ModernRules *rules = MfRules_GetActiveRules();

    if (!rules->randomItems)
        return;
    gSpecialVar_0x8005 = MfItemMapExAt(gSpecialVar_0x8005,
                                       rules->randomizerSeed,
                                       MF_ITEM_SITE_HIDDEN_BIT | gSpecialVar_0x8004);
}

void MfScriptBufferGiftTmNames(void)
{
    const struct ModernRules *rules = MfRules_GetActiveRules();
    enum Item src = VarGet(VAR_0x8000);
    enum Item dest = src;

    if (rules->randomItems)
        dest = MfItemMapExAt(src, rules->randomizerSeed, MfItemSiteObject());
    MfBufferTmItemAndMoveNames(dest);
}

#else // !MF_RANDOMIZER

enum Item MfItemMapExAt(enum Item itemId, u32 seed, u32 siteId)
{
    (void)seed;
    (void)siteId;
    return itemId;
}

enum Item MfItemMapEx(enum Item itemId, u32 seed)
{
    (void)seed;
    return itemId;
}

enum Item MfItemMapActive(enum Item itemId)
{
    return itemId;
}

const u16 *MfMaybeRandomizeShopStock(const u16 *items)
{
    return items;
}

void MfScriptRemapGrantItem(void)
{
}

void MfRemapNewGamePCItems(void)
{
}

void MfScriptRemapHiddenItem(void)
{
}

void MfScriptBufferGiftTmNames(void)
{
    MfBufferTmItemAndMoveNames(VarGet(VAR_0x8000));
}

#endif // MF_RANDOMIZER

void MfBufferTmItemAndMoveNames(enum Item itemId)
{
    CopyItemName(itemId, gStringVar1);
    StringCopy(gStringVar2, GetMoveName(MfGetTmHmTeachMove(itemId)));
}
