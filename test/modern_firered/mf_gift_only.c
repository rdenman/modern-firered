#include "global.h"
#include "event_data.h"
#include "mf_catch.h"
#include "mf_catch_rewards.h"
#include "mf_field_hm.h"
#include "mf_monotype.h"
#include "mf_rules.h"
#include "mf_rules_conflicts.h"
#include "constants/flags.h"
#include "constants/mf_catch_rewards.h"
#include "constants/pokemon.h"
#include "test/test.h"

static struct ModernRules *PrepareGiftOnly(bool8 giftOnly)
{
    struct ModernRules *save = MfRules_GetSaveRules();
    u16 flag;

    MfRules_ResetToEmpty(save);
    save->giftOnly = giftOnly;
    for (flag = FLAG_BADGE01_GET; flag < FLAG_BADGE01_GET + NUM_BADGES; flag++)
        FlagClear(flag);
    return save;
}

static u16 Check(u16 rewardId)
{
    gSpecialVar_0x8004 = rewardId;
    return MfCatchReward_Check();
}

TEST("MF: gift only blocks every ball throw")
{
    PrepareGiftOnly(FALSE);
    EXPECT_EQ(MfIsGiftOnlyCaptureBlocked(), FALSE);

    PrepareGiftOnly(TRUE);
    EXPECT_EQ(MfIsGiftOnlyCaptureBlocked(), TRUE);
}

TEST("MF: gift only waives catch-count rewards")
{
    PrepareGiftOnly(TRUE);
    EXPECT_EQ(MfCatchReward_GetMode(), MF_CATCH_REWARDS_WAIVED);
    EXPECT_EQ(Check(MF_CATCH_REWARD_FLASH), MF_CATCH_REWARD_MET);
    EXPECT_EQ(Check(MF_CATCH_REWARD_NATIONAL_DEX), MF_CATCH_REWARD_MET);
}

TEST("MF: gift only keeps badge catch rewards")
{
    struct ModernRules *save = PrepareGiftOnly(TRUE);

    save->catchRewards = MF_CATCH_REWARDS_BADGES;
    EXPECT_EQ(MfCatchReward_GetMode(), MF_CATCH_REWARDS_BADGES);
    EXPECT_EQ(Check(MF_CATCH_REWARD_FLASH), MF_CATCH_REWARD_NOT_MET);
}

TEST("MF: gift only lets the lead use bag HMs")
{
    PrepareGiftOnly(FALSE);
    EXPECT_EQ(MfFieldHm_OverwriteActive(), FALSE);

    PrepareGiftOnly(TRUE);
    EXPECT_EQ(MfFieldHm_OverwriteActive(), TRUE);
}

TEST("MF: gift only forbids monotype and mirror thief")
{
    struct ModernRules r;

    MfRules_ResetToEmpty(&r);
    r.giftOnly = TRUE;
    EXPECT_EQ(MfRules_FindConflict(&r), NULL);

    r.monotype = TYPE_WATER;
    EXPECT_EQ((u32)MfRules_EvaluatePair(&r, MF_RULES_CONFLICT_GIFT_ONLY_MONOTYPE), (u32)MF_RULES_CONFLICT_FORBID);

    r.monotype = MF_MONOTYPE_OFF;
    r.mirror = TRUE;
    EXPECT_EQ(MfRules_FindConflict(&r), NULL);
    r.mirrorThief = TRUE;
    EXPECT_EQ((u32)MfRules_EvaluatePair(&r, MF_RULES_CONFLICT_GIFT_ONLY_MIRROR_THIEF), (u32)MF_RULES_CONFLICT_FORBID);
}

TEST("MF: rules migrate v9 defaults gift only to off")
{
    struct ModernRules rules;

    memset(&rules, 0xFF, sizeof(rules));
    rules.version = MF_RULES_VERSION_V9;
    EXPECT_EQ((u32)MfRules_Migrate(&rules), (u32)MF_RULES_MIGRATE_UPGRADED);
    EXPECT_EQ((u32)rules.giftOnly, (u32)FALSE);
}
