#include "global.h"
#include "mf_mirror.h"
#include "mf_rules.h"
#include "pokemon.h"
#include "test/test.h"
#include "constants/battle.h"
#include "constants/pokemon.h"
#include "constants/species.h"

static void RestorePhase1Defaults(void)
{
    struct ModernRules *save = MfRules_GetSaveRules();

    save->version = 0;
}

static void ZeroParty(struct Pokemon *party)
{
    u32 i;

    for (i = 0; i < PARTY_SIZE; i++)
        ZeroMonData(&party[i]);
}

static void MakeMon(struct Pokemon *mon, enum Species species, u8 level)
{
    CreateMon(mon, species, level, 0, OTID_STRUCT_PLAYER_ID);
}

TEST("MF: mirror eligible battles are trainer or double, not link")
{
    EXPECT(MfMirror_IsBattleEligible(BATTLE_TYPE_TRAINER));
    EXPECT(MfMirror_IsBattleEligible(BATTLE_TYPE_DOUBLE));
    EXPECT(MfMirror_IsBattleEligible(BATTLE_TYPE_TRAINER | BATTLE_TYPE_DOUBLE));
    EXPECT(!MfMirror_IsBattleEligible(0));
    EXPECT(!MfMirror_IsBattleEligible(BATTLE_TYPE_SAFARI));
    EXPECT(!MfMirror_IsBattleEligible(BATTLE_TYPE_TRAINER | BATTLE_TYPE_LINK));
    EXPECT(!MfMirror_IsBattleEligible(BATTLE_TYPE_DOUBLE | BATTLE_TYPE_RECORDED));
}

TEST("MF: mirror restores player party unless thief")
{
    EXPECT(!MfMirror_ShouldRestorePlayerParty(FALSE, FALSE));
    EXPECT(!MfMirror_ShouldRestorePlayerParty(FALSE, TRUE));
    EXPECT(MfMirror_ShouldRestorePlayerParty(TRUE, FALSE));
    EXPECT(!MfMirror_ShouldRestorePlayerParty(TRUE, TRUE));
}

TEST("MF: mirror player party copies opponent A including empty slots")
{
    struct Pokemon opponentA[PARTY_SIZE];
    struct Pokemon opponentB[PARTY_SIZE];
    struct Pokemon player[PARTY_SIZE];

    ZeroParty(opponentA);
    ZeroParty(opponentB);
    ZeroParty(player);
    MakeMon(&opponentA[0], SPECIES_GEODUDE, 12);
    MakeMon(&opponentA[1], SPECIES_ONIX, 14);
    MakeMon(&player[0], SPECIES_CHARMANDER, 5);
    MakeMon(&player[1], SPECIES_PIDGEY, 4);
    MakeMon(&player[2], SPECIES_RATTATA, 3);

    MfMirror_BuildPlayerPartyFromOpponents(opponentA, opponentB, FALSE, player);

    EXPECT_EQ(GetMonData(&player[0], MON_DATA_SPECIES), SPECIES_GEODUDE);
    EXPECT_EQ(GetMonData(&player[1], MON_DATA_SPECIES), SPECIES_ONIX);
    EXPECT_EQ(GetMonData(&player[2], MON_DATA_SPECIES), SPECIES_NONE);
    EXPECT_EQ(GetMonData(&player[0], MON_DATA_LEVEL), 12);
    EXPECT_EQ(GetMonData(&player[1], MON_DATA_LEVEL), 14);
}

TEST("MF: two-opponent mirror puts B in the second half")
{
    struct Pokemon opponentA[PARTY_SIZE];
    struct Pokemon opponentB[PARTY_SIZE];
    struct Pokemon player[PARTY_SIZE];

    ZeroParty(opponentA);
    ZeroParty(opponentB);
    ZeroParty(player);
    MakeMon(&opponentA[0], SPECIES_ODDISH, 14);
    MakeMon(&opponentA[1], SPECIES_BELLSPROUT, 14);
    MakeMon(&opponentA[2], SPECIES_GLOOM, 16);
    MakeMon(&opponentB[0], SPECIES_VOLTORB, 14);
    MakeMon(&opponentB[1], SPECIES_MAGNEMITE, 14);
    MakeMon(&opponentB[2], SPECIES_ELECTRODE, 17);
    MakeMon(&player[5], SPECIES_SQUIRTLE, 5);

    MfMirror_BuildPlayerPartyFromOpponents(opponentA, opponentB, TRUE, player);

    EXPECT_EQ(GetMonData(&player[0], MON_DATA_SPECIES), SPECIES_ODDISH);
    EXPECT_EQ(GetMonData(&player[2], MON_DATA_SPECIES), SPECIES_GLOOM);
    EXPECT_EQ(GetMonData(&player[3], MON_DATA_SPECIES), SPECIES_VOLTORB);
    EXPECT_EQ(GetMonData(&player[5], MON_DATA_SPECIES), SPECIES_ELECTRODE);
}

TEST("MF: mirror accessors follow save bits")
{
    struct ModernRules *save = MfRules_GetSaveRules();

    MfRules_ApplyDevDefaults(save);
    save->rulesLocked = TRUE;
    save->mirror = FALSE;
    save->mirrorThief = FALSE;
    EXPECT(!MfRules_IsMirror());
    EXPECT(!MfRules_IsMirrorThief());

    save->mirror = TRUE;
    save->mirrorThief = TRUE;
    EXPECT(MfRules_IsMirror());
    EXPECT(MfRules_IsMirrorThief());

    RestorePhase1Defaults();
}
