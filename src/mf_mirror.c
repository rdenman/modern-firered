#include "global.h"
#include "battle.h"
#include "debug.h"
#include "mf_mirror.h"
#include "mf_rules.h"
#include "pokemon.h"
#include "test_runner.h"
#include "constants/battle.h"
#include "constants/pokemon.h"

static EWRAM_DATA bool8 sMfMirrorSwapActive = FALSE;
static EWRAM_DATA bool8 sMfMirrorBackupValid = FALSE;
static EWRAM_DATA struct Pokemon sMfMirrorPlayerBackup[PARTY_SIZE] = {0};

bool8 MfMirror_IsBattleEligible(u32 battleTypeFlags)
{
    if (battleTypeFlags & (BATTLE_TYPE_LINK
                         | BATTLE_TYPE_LINK_IN_BATTLE
                         | BATTLE_TYPE_RECORDED
                         | BATTLE_TYPE_RECORDED_LINK))
        return FALSE;
    return (battleTypeFlags & (BATTLE_TYPE_TRAINER | BATTLE_TYPE_DOUBLE)) != 0;
}

bool8 MfMirror_ShouldRestorePlayerParty(bool8 mirror, bool8 thief)
{
    return mirror && !thief;
}

void MfMirror_BuildPlayerPartyFromOpponents(const struct Pokemon *opponentA,
                                            const struct Pokemon *opponentB,
                                            bool8 twoOpponents,
                                            struct Pokemon *outPlayer)
{
    u32 i;

    memcpy(outPlayer, opponentA, sizeof(struct Pokemon) * PARTY_SIZE);
    if (!twoOpponents || opponentB == NULL)
        return;
    for (i = 0; i < PARTY_SIZE / 2; i++)
        outPlayer[PARTY_SIZE / 2 + i] = opponentB[i];
}

static bool8 MfMirrorShouldSkipSession(void)
{
    if (gTestRunnerEnabled)
        return TRUE;
#if DEBUG_OVERWORLD_MENU
    if (gIsDebugBattle)
        return TRUE;
#endif
    return FALSE;
}

void MfMirror_OnBattleStart(u32 battleTypeFlags)
{
    sMfMirrorSwapActive = FALSE;
    sMfMirrorBackupValid = FALSE;

    if (MfMirrorShouldSkipSession())
        return;
    if (!MfRules_IsMirror())
        return;
    if (!MfMirror_IsBattleEligible(battleTypeFlags))
        return;

    if (MfMirror_ShouldRestorePlayerParty(TRUE, MfRules_IsMirrorThief()))
    {
        memcpy(sMfMirrorPlayerBackup, gParties[B_TRAINER_PLAYER], sizeof(sMfMirrorPlayerBackup));
        sMfMirrorBackupValid = TRUE;
    }

    MfMirror_BuildPlayerPartyFromOpponents(
        gParties[B_TRAINER_OPPONENT_A],
        gParties[B_TRAINER_OPPONENT_B],
        (battleTypeFlags & BATTLE_TYPE_TWO_OPPONENTS) != 0,
        gParties[B_TRAINER_PLAYER]);
    sMfMirrorSwapActive = TRUE;
}

void MfMirror_OnBattleEnd(u32 battleTypeFlags)
{
    (void)battleTypeFlags;

    if (!sMfMirrorSwapActive)
        return;
    sMfMirrorSwapActive = FALSE;

    if (!sMfMirrorBackupValid)
        return;
    if (!MfMirror_ShouldRestorePlayerParty(MfRules_IsMirror(), MfRules_IsMirrorThief()))
        return;

    memcpy(gParties[B_TRAINER_PLAYER], sMfMirrorPlayerBackup, sizeof(sMfMirrorPlayerBackup));
    sMfMirrorBackupValid = FALSE;
}
