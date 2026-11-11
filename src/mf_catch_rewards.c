#include "global.h"
#include "event_data.h"
#include "mf_catch_rewards.h"
#include "mf_rules.h"
#include "pokemon.h"
#include "string_util.h"
#include "constants/flags.h"
#include "constants/mf_catch_rewards.h"

// Oak's Aide rewards (and Oak's National Dex check) in catch order. ADR 0119.
static const u8 sRequiredBadges[MF_CATCH_REWARD_ID_COUNT] =
{
    [MF_CATCH_REWARD_FLASH]        = 3,
    [MF_CATCH_REWARD_EVERSTONE]    = 4,
    [MF_CATCH_REWARD_ITEMFINDER]   = 5,
    [MF_CATCH_REWARD_AMULET_COIN]  = 6,
    [MF_CATCH_REWARD_EXP_SHARE]    = 7,
    [MF_CATCH_REWARD_NATIONAL_DEX] = 8,
};

static const u8 sRequiredLevels[MF_CATCH_REWARD_ID_COUNT] =
{
    [MF_CATCH_REWARD_FLASH]        = 20,
    [MF_CATCH_REWARD_EVERSTONE]    = 30,
    [MF_CATCH_REWARD_ITEMFINDER]   = 40,
    [MF_CATCH_REWARD_AMULET_COIN]  = 50,
    [MF_CATCH_REWARD_EXP_SHARE]    = 60,
    [MF_CATCH_REWARD_NATIONAL_DEX] = 60,
};

static u8 MfCatchReward_CountBadges(void)
{
    u8 count = 0;
    u16 flag;

    for (flag = FLAG_BADGE01_GET; flag < FLAG_BADGE01_GET + NUM_BADGES; flag++)
    {
        if (FlagGet(flag))
            count++;
    }
    return count;
}

static u8 MfCatchReward_HighestPartyLevel(void)
{
    u8 best = 0;
    u32 i;

    for (i = 0; i < PARTY_SIZE; i++)
    {
        struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][i];
        u8 level;

        if (GetMonData(mon, MON_DATA_SPECIES) == SPECIES_NONE || GetMonData(mon, MON_DATA_IS_EGG))
            continue;
        level = GetMonData(mon, MON_DATA_LEVEL);
        if (level > best)
            best = level;
    }
    return best;
}

u16 MfCatchReward_GetMode(void)
{
    u8 mode = MfRules_GetCatchRewards();

    if (mode == MF_CATCH_REWARDS_CATCHES && MfRules_IsGiftOnly())
        return MF_CATCH_REWARDS_WAIVED;
    return mode;
}

// In: VAR_0x8004 = MF_CATCH_REWARD_*. Buffers STR_VAR_1 = required and
// STR_VAR_3 = current (badges or highest party level) in non-catch modes.
u16 MfCatchReward_Check(void)
{
    u8 mode = MfCatchReward_GetMode();
    u16 id = gSpecialVar_0x8004;
    u8 required, current;

    if (id >= MF_CATCH_REWARD_ID_COUNT)
        return MF_CATCH_REWARD_USE_CATCHES;
    if (mode == MF_CATCH_REWARDS_WAIVED)
        return MF_CATCH_REWARD_MET;
    if (mode == MF_CATCH_REWARDS_CATCHES)
        return MF_CATCH_REWARD_USE_CATCHES;

    if (mode == MF_CATCH_REWARDS_BADGES)
    {
        required = sRequiredBadges[id];
        current = MfCatchReward_CountBadges();
    }
    else
    {
        required = sRequiredLevels[id];
        current = MfCatchReward_HighestPartyLevel();
    }

    ConvertIntToDecimalStringN(gStringVar1, required, STR_CONV_MODE_LEFT_ALIGN, 3);
    ConvertIntToDecimalStringN(gStringVar3, current, STR_CONV_MODE_LEFT_ALIGN, 3);
    return current >= required ? MF_CATCH_REWARD_MET : MF_CATCH_REWARD_NOT_MET;
}
