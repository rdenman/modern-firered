#include "global.h"
#include "event_data.h"
#include "item.h"
#include "line_break.h"
#include "mf_rules.h"
#include "mf_tm_random.h"
#include "move.h"
#include "string_util.h"
#include "text.h"
#include "constants/flags.h"
#include "constants/items.h"
#include "constants/moves.h"
#include "test/test.h"

#define MF_TM_SEED_A 0x544D4131u
#define MF_TM_SEED_B 0x544D4232u

static void SetTmRandomizer(u32 seed, bool8 tmsOn)
{
    struct ModernRules *save = MfRules_GetSaveRules();

    MfRules_ResetToEmpty(save);
    save->version = MF_RULES_VERSION;
    save->randomizerSeed = seed;
    save->randomizerEnabled = TRUE;
    save->randomTms = tmsOn;
    save->rulesLocked = TRUE;
    FlagSet(FLAG_SYS_POKEDEX_GET);
}

static void RestorePhase1Defaults(void)
{
    struct ModernRules *save = MfRules_GetSaveRules();

    save->version = 0;
    FlagClear(FLAG_SYS_POKEDEX_GET);
}

static bool32 IsForbiddenDest(enum Move move)
{
    return move == MOVE_NONE
        || move == MOVE_STRUGGLE
        || move >= MOVES_COUNT
        || GetMoveEffect(move) == EFFECT_PLACEHOLDER
        || GetMovePP(move) == 0;
}

TEST("MF: TM teach move is identity when TMs is off")
{
    SetTmRandomizer(MF_TM_SEED_A, FALSE);
    EXPECT_EQ(MfGetTmHmTeachMove(ITEM_TM01), GetItemTMHMMoveId(ITEM_TM01));
    EXPECT_EQ(MfGetTmHmTeachMove(ITEM_TM24), GetItemTMHMMoveId(ITEM_TM24));
    EXPECT_EQ(MfGetTmHmTeachMove(ITEM_TM50), GetItemTMHMMoveId(ITEM_TM50));
    EXPECT_EQ(MfMapTutorTeachMove(MOVE_MEGA_PUNCH), MOVE_MEGA_PUNCH);
    EXPECT_EQ(MfGetTmHmTeachMove(ITEM_HM_CUT), GetItemTMHMMoveId(ITEM_HM_CUT));
    RestorePhase1Defaults();
}

TEST("MF: TM01-50 dests are unique and HMs stay vanilla")
{
    enum Move dests[MF_TM_RANDOM_SLOT_COUNT];
    u16 i;
    u16 j;

    SetTmRandomizer(MF_TM_SEED_A, TRUE);
    for (i = 0; i < MF_TM_RANDOM_TM_COUNT; i++)
        dests[i] = MfGetTmHmTeachMove(ITEM_TM01 + i);
    dests[50] = MfMapTutorTeachMove(MOVE_DOUBLE_EDGE);
    dests[51] = MfMapTutorTeachMove(MOVE_THUNDER_WAVE);
    dests[52] = MfMapTutorTeachMove(MOVE_ROCK_SLIDE);
    dests[53] = MfMapTutorTeachMove(MOVE_EXPLOSION);
    dests[54] = MfMapTutorTeachMove(MOVE_MEGA_PUNCH);
    dests[55] = MfMapTutorTeachMove(MOVE_MEGA_KICK);
    dests[56] = MfMapTutorTeachMove(MOVE_DREAM_EATER);
    dests[57] = MfMapTutorTeachMove(MOVE_SOFT_BOILED);
    dests[58] = MfMapTutorTeachMove(MOVE_SUBSTITUTE);
    dests[59] = MfMapTutorTeachMove(MOVE_SWORDS_DANCE);
    dests[60] = MfMapTutorTeachMove(MOVE_SEISMIC_TOSS);
    dests[61] = MfMapTutorTeachMove(MOVE_COUNTER);
    dests[62] = MfMapTutorTeachMove(MOVE_METRONOME);
    dests[63] = MfMapTutorTeachMove(MOVE_MIMIC);
    dests[64] = MfMapTutorTeachMove(MOVE_BODY_SLAM);
    dests[65] = MfMapTutorTeachMove(MOVE_FRENZY_PLANT);
    dests[66] = MfMapTutorTeachMove(MOVE_BLAST_BURN);
    dests[67] = MfMapTutorTeachMove(MOVE_HYDRO_CANNON);

    for (i = 0; i < MF_TM_RANDOM_SLOT_COUNT; i++)
    {
        EXPECT(!IsForbiddenDest(dests[i]));
        for (j = i + 1; j < MF_TM_RANDOM_SLOT_COUNT; j++)
            EXPECT_NE(dests[i], dests[j]);
    }

    EXPECT_EQ(MfGetTmHmTeachMove(ITEM_HM_CUT), GetItemTMHMMoveId(ITEM_HM_CUT));
    EXPECT_EQ(MfGetTmHmTeachMove(ITEM_HM_SURF), GetItemTMHMMoveId(ITEM_HM_SURF));
    EXPECT_EQ(MfGetTmHmTeachMove(ITEM_TM51), GetItemTMHMMoveId(ITEM_TM51));
    RestorePhase1Defaults();
}

TEST("MF: TM dest mapping is seed-stable and changes with seed")
{
    enum Move tmA;
    enum Move tutorA;
    enum Move tmB;
    enum Move tutorB;

    SetTmRandomizer(MF_TM_SEED_A, TRUE);
    tmA = MfGetTmHmTeachMove(ITEM_TM01);
    tutorA = MfMapTutorTeachMove(MOVE_MEGA_PUNCH);
    EXPECT_EQ(tmA, MfGetTmHmTeachMove(ITEM_TM01));
    EXPECT_EQ(tutorA, MfMapTutorTeachMove(MOVE_MEGA_PUNCH));

    SetTmRandomizer(MF_TM_SEED_B, TRUE);
    tmB = MfGetTmHmTeachMove(ITEM_TM01);
    tutorB = MfMapTutorTeachMove(MOVE_MEGA_PUNCH);
    EXPECT(tmA != tmB || tutorA != tutorB);
    RestorePhase1Defaults();
}

TEST("MF: tutor remap buffers dest name and keeps source for learn checks")
{
    enum Move dest;

    SetTmRandomizer(MF_TM_SEED_A, TRUE);
    gSpecialVar_0x8005 = MOVE_MEGA_PUNCH;
    MfScriptRemapTutorMove();
    dest = gSpecialVar_0x8005;
    EXPECT_NE(dest, MOVE_NONE);
    EXPECT_EQ(MfTutorSourceMove(), MOVE_MEGA_PUNCH);
    EXPECT_EQ(MfTutorLearnCheckMove(dest), MOVE_MEGA_PUNCH);
    EXPECT_EQ(StringCompare(gStringVar1, GetMoveName(dest)), 0);
    EXPECT_EQ(MfTmLearnCheckMove(ITEM_TM24, MfGetTmHmTeachMove(ITEM_TM24)), GetItemTMHMMoveId(ITEM_TM24));
    RestorePhase1Defaults();
}

TEST("MF: remapped TM bag description follows dest, not vanilla")
{
    u16 i;
    enum Item item;
    enum Move dest;
    enum Item destTm;
    bool32 foundRemap = FALSE;

    SetTmRandomizer(MF_TM_SEED_A, FALSE);
    EXPECT_EQ(GetItemDescription(ITEM_TM13), gItemsInfo[ITEM_TM13].description);

    SetTmRandomizer(MF_TM_SEED_A, TRUE);
    for (i = 0; i < MF_TM_RANDOM_TM_COUNT; i++)
    {
        item = ITEM_TM01 + i;
        dest = MfGetTmHmTeachMove(item);
        if (dest == GetItemTMHMMoveId(item))
            continue;

        foundRemap = TRUE;
        destTm = GetTMHMItemIdFromMoveId(dest);
        EXPECT_NE(GetItemDescription(item), gItemsInfo[item].description);
        if (destTm != ITEM_NONE)
        {
            EXPECT_EQ(GetItemDescription(item), gItemsInfo[destTm].description);
        }
        else
        {
            const u8 *desc = GetItemDescription(item);

            EXPECT_NE(desc, GetMoveDescription(dest));
            EXPECT_LE(GetStringWidth(FONT_NORMAL, desc, 0), 102);
            EXPECT_LE(CountLineBreaks((u8 *)desc), 2);
        }
    }
    EXPECT(foundRemap);
    EXPECT_EQ(GetItemDescription(ITEM_HM_CUT), gItemsInfo[ITEM_HM_CUT].description);
    RestorePhase1Defaults();
}
