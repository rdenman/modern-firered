#ifndef GUARD_CONSTANTS_MF_CATCH_REWARDS_H
#define GUARD_CONSTANTS_MF_CATCH_REWARDS_H

// Features page "Catch Rewards" (rules catchRewards). See ADR 0119.
#define MF_CATCH_REWARDS_CATCHES 0
#define MF_CATCH_REWARDS_BADGES  1
#define MF_CATCH_REWARDS_LEVELS  2
#define MF_CATCH_REWARDS_COUNT   3
// Not stored: MfCatchReward_GetMode result when Gift Only waives Catches (ADR 0121).
#define MF_CATCH_REWARDS_WAIVED  3

// VAR_0x8004 input to MfCatchReward_Check.
#define MF_CATCH_REWARD_FLASH        0
#define MF_CATCH_REWARD_EVERSTONE    1
#define MF_CATCH_REWARD_ITEMFINDER   2
#define MF_CATCH_REWARD_AMULET_COIN  3
#define MF_CATCH_REWARD_EXP_SHARE    4
#define MF_CATCH_REWARD_NATIONAL_DEX 5
#define MF_CATCH_REWARD_ID_COUNT     6

// MfCatchReward_Check results.
#define MF_CATCH_REWARD_USE_CATCHES 0
#define MF_CATCH_REWARD_MET         1
#define MF_CATCH_REWARD_NOT_MET     2

#endif // GUARD_CONSTANTS_MF_CATCH_REWARDS_H
