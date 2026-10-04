#include "global.h"
#include "caps.h"
#include "event_data.h"
#include "mf_iv_ev.h"
#include "mf_rules.h"
#include "pokemon.h"
#include "test/test.h"
#include "constants/pokemon.h"
#include "constants/species.h"

// ME sIV_Table / sEV_Table (ADR 0047).
static const u8 sExpectedTrainerIvs[MF_TRAINER_IV_EV_BADGE_STAGES] =
{
    7, 10, 13, 16, 19, 22, 25, 28, 31,
};

static const u8 sExpectedTrainerEvs[MF_TRAINER_IV_EV_BADGE_STAGES] =
{
    12, 24, 36, 48, 60, 72, 80, 100, 128,
};

static void ClearBadgeAndClearFlags(void)
{
    u32 i;

    for (i = 0; i < NUM_BADGES; i++)
        FlagClear(FLAG_BADGE01_GET + i);
    FlagClear(FLAG_SYS_GAME_CLEAR);
}

static struct ModernRules *PrepareRules(void)
{
    struct ModernRules *save = MfRules_GetSaveRules();

    MfRules_ApplyDevDefaults(save);
    save->rulesLocked = TRUE;
    return save;
}

TEST("MF: trainer IV/EV scaling math")
{
    u32 badges;

    for (badges = 0; badges < MF_TRAINER_IV_EV_BADGE_STAGES; badges++)
    {
        EXPECT_EQ(MfResolveTrainerIVs(MF_TRAINER_IVS_OFF, badges), 0);
        EXPECT_EQ(MfResolveTrainerIVs(MF_TRAINER_IVS_SCALE, badges), sExpectedTrainerIvs[badges]);
        EXPECT_EQ(MfResolveTrainerIVs(MF_TRAINER_IVS_HARD, badges), MAX_PER_STAT_IVS);

        EXPECT_EQ(MfResolveTrainerEVs(MF_TRAINER_EVS_OFF, badges), 0);
        EXPECT_EQ(MfResolveTrainerEVs(MF_TRAINER_EVS_SCALE, badges), sExpectedTrainerEvs[badges]);
        EXPECT_EQ(MfResolveTrainerEVs(MF_TRAINER_EVS_HARD, badges), MF_TRAINER_EVS_HARD_VALUE);
        EXPECT_EQ(MfResolveTrainerEVs(MF_TRAINER_EVS_EXTREME, badges), MAX_PER_STAT_EVS);
    }

    EXPECT_EQ(MfResolveTrainerIVs(MF_TRAINER_IVS_SCALE, 9), sExpectedTrainerIvs[8]);
    EXPECT_EQ(MfResolveTrainerIVs(MF_TRAINER_IVS_SCALE, 255), sExpectedTrainerIvs[8]);
    EXPECT_EQ(MfResolveTrainerEVs(MF_TRAINER_EVS_SCALE, 9), sExpectedTrainerEvs[8]);
    EXPECT_EQ(MfResolveTrainerIVs(3, 0), 0);
    EXPECT_EQ(MfResolveTrainerEVs(4, 0), 0);
}

TEST("MF: player EV cap and trainer scale follow rules")
{
    struct ModernRules *save = PrepareRules();
    u32 badges;

    ClearBadgeAndClearFlags();

    save->noEvs = FALSE;
    save->scalingIvs = MF_TRAINER_IVS_OFF;
    save->scalingEvs = MF_TRAINER_EVS_OFF;
    save->maxPartyIvs = MF_PLAYER_IVS_OFF;
    EXPECT_EQ(GetCurrentEVCap(), MAX_TOTAL_EVS);
    EXPECT(!MfArePlayerEvsDisabled());
    EXPECT(!MfShouldCapEVItems());
    EXPECT_EQ(MfGetCurrentTrainerIVs(), 0);
    EXPECT_EQ(MfGetCurrentTrainerEVs(), 0);

    save->noEvs = TRUE;
    EXPECT(MfArePlayerEvsDisabled());
    EXPECT(MfShouldCapEVItems());
    EXPECT_EQ(GetCurrentEVCap(), 0u);

    FlagSet(FLAG_SYS_GAME_CLEAR);
    EXPECT(!MfArePlayerEvsDisabled());
    EXPECT(!MfShouldCapEVItems());
    EXPECT_EQ(GetCurrentEVCap(), MAX_TOTAL_EVS);
    FlagClear(FLAG_SYS_GAME_CLEAR);

    save->noEvs = FALSE;
    save->scalingIvs = MF_TRAINER_IVS_SCALE;
    save->scalingEvs = MF_TRAINER_EVS_SCALE;
    for (badges = 0; badges < NUM_BADGES; badges++)
    {
        EXPECT_EQ(MfGetCurrentTrainerIVs(), sExpectedTrainerIvs[badges]);
        EXPECT_EQ(MfGetCurrentTrainerEVs(), sExpectedTrainerEvs[badges]);
        FlagSet(FLAG_BADGE01_GET + badges);
    }
    EXPECT_EQ(MfGetCurrentTrainerIVs(), sExpectedTrainerIvs[8]);
    EXPECT_EQ(MfGetCurrentTrainerEVs(), sExpectedTrainerEvs[8]);

    save->scalingIvs = MF_TRAINER_IVS_HARD;
    save->scalingEvs = MF_TRAINER_EVS_HARD;
    EXPECT_EQ(MfGetCurrentTrainerIVs(), MAX_PER_STAT_IVS);
    EXPECT_EQ(MfGetCurrentTrainerEVs(), MF_TRAINER_EVS_HARD_VALUE);

    save->scalingEvs = MF_TRAINER_EVS_EXTREME;
    EXPECT_EQ(MfGetCurrentTrainerEVs(), MAX_PER_STAT_EVS);

    save->version = 0;
}

TEST("MF: givemon / CreateMonFromTemplate respects PLAYER IVs Max")
{
    struct ModernRules *save = PrepareRules();
    struct Pokemon mon;
    struct PokemonTemplate tmpl = {0};
    u32 i;

    save->maxPartyIvs = MF_PLAYER_IVS_MAX;
    save->noEvs = TRUE;
    tmpl.species = SPECIES_SQUIRTLE;
    tmpl.level = 5;
    tmpl.gender = MON_GENDER_RANDOM;
    tmpl.nature = NATURE_RANDOM;
    tmpl.origin = GIFTMON_ORIGIN;
    for (i = 0; i < NUM_STATS; i++)
        tmpl.ivs[i] = USE_RANDOM_IVS;
    for (i = 0; i < MAX_MON_MOVES; i++)
        tmpl.moves[i] = MOVE_DEFAULT;

    CreateMonFromTemplate(&mon, &tmpl);

    for (i = 0; i < NUM_STATS; i++)
    {
        EXPECT_EQ(GetMonData(&mon, MON_DATA_HP_IV + i), MAX_PER_STAT_IVS);
        EXPECT_EQ(GetMonData(&mon, MON_DATA_HP_EV + i), 0);
    }

    save->version = 0;
}
