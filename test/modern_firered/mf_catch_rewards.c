#include "global.h"
#include "event_data.h"
#include "mf_catch_rewards.h"
#include "mf_rules.h"
#include "pokemon.h"
#include "string_util.h"
#include "constants/flags.h"
#include "constants/species.h"
#include "test/test.h"

static void PrepareCatchRewards(u8 mode)
{
    struct ModernRules *save = MfRules_GetSaveRules();
    u16 flag;

    MfRules_ResetToEmpty(save);
    save->catchRewards = mode;
    for (flag = FLAG_BADGE01_GET; flag < FLAG_BADGE01_GET + NUM_BADGES; flag++)
        FlagClear(flag);
    ZeroPlayerPartyMons();
}

static void GiveBadges(u8 count)
{
    u16 i;

    for (i = 0; i < count; i++)
        FlagSet(FLAG_BADGE01_GET + i);
}

static void AddPartyMon(u8 level)
{
    struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][gPartiesCount[B_TRAINER_PLAYER]];

    CreateMon(mon, SPECIES_BULBASAUR, level, 32, OTID_STRUCT_PRESET(0));
    gPartiesCount[B_TRAINER_PLAYER]++;
}

static u16 Check(u16 rewardId)
{
    gSpecialVar_0x8004 = rewardId;
    return MfCatchReward_Check();
}

TEST("MF: catch rewards default to the vanilla catch-count flow")
{
    PrepareCatchRewards(MF_CATCH_REWARDS_CATCHES);
    GiveBadges(8);
    AddPartyMon(100);
    EXPECT_EQ(Check(MF_CATCH_REWARD_FLASH), MF_CATCH_REWARD_USE_CATCHES);
    EXPECT_EQ(Check(MF_CATCH_REWARD_NATIONAL_DEX), MF_CATCH_REWARD_USE_CATCHES);
}

TEST("MF: badge catch rewards need 3-7 badges, National Dex 8")
{
    PrepareCatchRewards(MF_CATCH_REWARDS_BADGES);
    GiveBadges(2);
    EXPECT_EQ(Check(MF_CATCH_REWARD_FLASH), MF_CATCH_REWARD_NOT_MET);

    GiveBadges(3);
    EXPECT_EQ(Check(MF_CATCH_REWARD_FLASH), MF_CATCH_REWARD_MET);
    EXPECT_EQ(Check(MF_CATCH_REWARD_EVERSTONE), MF_CATCH_REWARD_NOT_MET);

    GiveBadges(7);
    EXPECT_EQ(Check(MF_CATCH_REWARD_EXP_SHARE), MF_CATCH_REWARD_MET);
    EXPECT_EQ(Check(MF_CATCH_REWARD_NATIONAL_DEX), MF_CATCH_REWARD_NOT_MET);

    GiveBadges(8);
    EXPECT_EQ(Check(MF_CATCH_REWARD_NATIONAL_DEX), MF_CATCH_REWARD_MET);
}

TEST("MF: level catch rewards use the highest party level")
{
    PrepareCatchRewards(MF_CATCH_REWARDS_LEVELS);
    AddPartyMon(12);
    AddPartyMon(19);
    EXPECT_EQ(Check(MF_CATCH_REWARD_FLASH), MF_CATCH_REWARD_NOT_MET);
    EXPECT_EQ(StringCompare(gStringVar1, COMPOUND_STRING("20")), 0);
    EXPECT_EQ(StringCompare(gStringVar3, COMPOUND_STRING("19")), 0);

    AddPartyMon(40);
    EXPECT_EQ(Check(MF_CATCH_REWARD_FLASH), MF_CATCH_REWARD_MET);
    EXPECT_EQ(Check(MF_CATCH_REWARD_ITEMFINDER), MF_CATCH_REWARD_MET);
    EXPECT_EQ(Check(MF_CATCH_REWARD_AMULET_COIN), MF_CATCH_REWARD_NOT_MET);
    EXPECT_EQ(Check(MF_CATCH_REWARD_NATIONAL_DEX), MF_CATCH_REWARD_NOT_MET);
}

TEST("MF: rules migrate v7 defaults catch rewards to catches")
{
    struct ModernRules rules;

    memset(&rules, 0xFF, sizeof(rules));
    rules.version = MF_RULES_VERSION_V7;
    EXPECT_EQ((u32)MfRules_Migrate(&rules), (u32)MF_RULES_MIGRATE_UPGRADED);
    EXPECT_EQ((u32)rules.catchRewards, (u32)MF_CATCH_REWARDS_CATCHES);
}
