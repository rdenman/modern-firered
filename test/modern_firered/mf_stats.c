#include "global.h"
#include "mf_rules.h"
#include "mf_stats.h"
#include "pokemon.h"
#include "test/test.h"

static void SetModernStats(bool8 modernStats)
{
    struct ModernRules *save = MfRules_GetSaveRules();

    MfRules_ApplyDevDefaults(save);
    MfRules_ApplyGamemodePreset(save, MF_GAMEMODE_CUSTOM);
    save->modernStats = modernStats;
    save->rulesLocked = TRUE;
}

static void RestorePhase1Defaults(void)
{
    struct ModernRules *save = MfRules_GetSaveRules();

    save->version = 0;
}

TEST("MF: modern stats on uses Gen-latest base stats")
{
    SetModernStats(TRUE);

    EXPECT_EQ(GetSpeciesBaseSpAttack(SPECIES_BUTTERFREE), 90u);
    EXPECT_EQ(GetSpeciesBaseAttack(SPECIES_ARBOK), 95u);
    EXPECT_EQ(GetSpeciesBaseAttack(SPECIES_BEEDRILL), 90u);
    EXPECT_EQ(GetSpeciesBaseSpeed(SPECIES_PIDGEOT), 101u);
    EXPECT_EQ(GetSpeciesBaseDefense(SPECIES_PIKACHU), 40u);
    EXPECT_EQ(GetSpeciesBaseSpDefense(SPECIES_PIKACHU), 50u);
    EXPECT_EQ(GetSpeciesBaseAttack(SPECIES_FARFETCHD), 90u);
    EXPECT_EQ(GetSpeciesBaseSpDefense(SPECIES_ALAKAZAM), 95u);
    EXPECT_EQ(GetSpeciesBaseAttack(SPECIES_GOLEM), 120u);
    EXPECT_EQ(GetSpeciesBaseSpAttack(SPECIES_SWELLOW), 75u);
    EXPECT_EQ(GetSpeciesBaseHP(SPECIES_LUNATONE), 90u);
    // Unchanged species still match species_info.
    EXPECT_EQ(GetSpeciesBaseHP(SPECIES_CHARIZARD), 78u);
    EXPECT_EQ(GetSpeciesBaseAttack(SPECIES_CHARIZARD), 84u);

    RestorePhase1Defaults();
}

TEST("MF: modern stats off uses Gen-3 classic base stats")
{
    SetModernStats(FALSE);

    EXPECT_EQ(GetSpeciesBaseSpAttack(SPECIES_BUTTERFREE), 80u);
    EXPECT_EQ(GetSpeciesBaseAttack(SPECIES_ARBOK), 85u);
    EXPECT_EQ(GetSpeciesBaseAttack(SPECIES_BEEDRILL), 80u);
    EXPECT_EQ(GetSpeciesBaseSpeed(SPECIES_PIDGEOT), 91u);
    EXPECT_EQ(GetSpeciesBaseDefense(SPECIES_PIKACHU), 30u);
    EXPECT_EQ(GetSpeciesBaseSpDefense(SPECIES_PIKACHU), 40u);
    EXPECT_EQ(GetSpeciesBaseAttack(SPECIES_FARFETCHD), 65u);
    EXPECT_EQ(GetSpeciesBaseSpDefense(SPECIES_ALAKAZAM), 85u);
    EXPECT_EQ(GetSpeciesBaseAttack(SPECIES_GOLEM), 110u);
    EXPECT_EQ(GetSpeciesBaseSpAttack(SPECIES_SWELLOW), 50u);
    EXPECT_EQ(GetSpeciesBaseHP(SPECIES_LUNATONE), 70u);
    // Unchanged species identical in both modes.
    EXPECT_EQ(GetSpeciesBaseHP(SPECIES_CHARIZARD), 78u);
    EXPECT_EQ(GetSpeciesBaseAttack(SPECIES_CHARIZARD), 84u);

    RestorePhase1Defaults();
}

TEST("MF: modern stats GetSpeciesBaseStat matches per-stat getters")
{
    u32 i;
    static const enum Species sSamples[] = {
        SPECIES_BUTTERFREE,
        SPECIES_ARBOK,
        SPECIES_PIKACHU,
        SPECIES_CHARIZARD,
        SPECIES_SWELLOW,
    };

    SetModernStats(FALSE);
    for (i = 0; i < ARRAY_COUNT(sSamples); i++)
    {
        enum Species sp = sSamples[i];
        EXPECT_EQ(GetSpeciesBaseStat(sp, STAT_HP), GetSpeciesBaseHP(sp));
        EXPECT_EQ(GetSpeciesBaseStat(sp, STAT_ATK), GetSpeciesBaseAttack(sp));
        EXPECT_EQ(GetSpeciesBaseStat(sp, STAT_DEF), GetSpeciesBaseDefense(sp));
        EXPECT_EQ(GetSpeciesBaseStat(sp, STAT_SPEED), GetSpeciesBaseSpeed(sp));
        EXPECT_EQ(GetSpeciesBaseStat(sp, STAT_SPATK), GetSpeciesBaseSpAttack(sp));
        EXPECT_EQ(GetSpeciesBaseStat(sp, STAT_SPDEF), GetSpeciesBaseSpDefense(sp));
    }

    SetModernStats(TRUE);
    for (i = 0; i < ARRAY_COUNT(sSamples); i++)
    {
        enum Species sp = sSamples[i];
        EXPECT_EQ(GetSpeciesBaseStat(sp, STAT_HP), GetSpeciesBaseHP(sp));
        EXPECT_EQ(GetSpeciesBaseStat(sp, STAT_ATK), GetSpeciesBaseAttack(sp));
        EXPECT_EQ(GetSpeciesBaseStat(sp, STAT_DEF), GetSpeciesBaseDefense(sp));
        EXPECT_EQ(GetSpeciesBaseStat(sp, STAT_SPEED), GetSpeciesBaseSpeed(sp));
        EXPECT_EQ(GetSpeciesBaseStat(sp, STAT_SPATK), GetSpeciesBaseSpAttack(sp));
        EXPECT_EQ(GetSpeciesBaseStat(sp, STAT_SPDEF), GetSpeciesBaseSpDefense(sp));
    }

    RestorePhase1Defaults();
}

TEST("MF: modern stats raise BST for officially buffed species")
{
    u32 classicTotal;
    u32 modernTotal;

    SetModernStats(FALSE);
    classicTotal = GetSpeciesBaseStatTotal(SPECIES_BUTTERFREE);
    SetModernStats(TRUE);
    modernTotal = GetSpeciesBaseStatTotal(SPECIES_BUTTERFREE);

    EXPECT(modernTotal > classicTotal);
    EXPECT_EQ(modernTotal - classicTotal, 10u); // SpAtk 80 → 90

    RestorePhase1Defaults();
}

static void SetBstRules(bool8 modernStats, u8 equalizer)
{
    struct ModernRules *save = MfRules_GetSaveRules();

    MfRules_ApplyDevDefaults(save);
    MfRules_ApplyGamemodePreset(save, MF_GAMEMODE_CUSTOM);
    save->modernStats = modernStats;
    save->baseStatEqualizer = equalizer;
    save->rulesLocked = TRUE;
}

static u32 SumStats(const u32 *stats, u32 count)
{
    u32 i, total = 0;

    for (i = 0; i < count; i++)
        total += stats[i];
    return total;
}

TEST("MF: BST equalizer target map is Off/100/255/500")
{
    EXPECT_EQ(MfGetBstEqualizerTarget(0), 0u);
    EXPECT_EQ(MfGetBstEqualizerTarget(1), 100u);
    EXPECT_EQ(MfGetBstEqualizerTarget(2), 255u);
    EXPECT_EQ(MfGetBstEqualizerTarget(3), 500u);
    EXPECT_EQ(MfGetBstEqualizerTarget(4), 0u);
}

TEST("MF: BST equalizer rounding preserves total and relative order")
{
    u32 even[NUM_STATS] = {10, 10, 10, 10, 10, 10};
    u32 magikarp[NUM_STATS] = {20, 10, 55, 80, 15, 20}; // HP ATK DEF SPE SPA SPD
    u32 i;

    MfNormalizeStatsToBst(even, NUM_STATS, 100);
    EXPECT_EQ(SumStats(even, NUM_STATS), 100u);
    for (i = 0; i < NUM_STATS; i++)
        EXPECT(even[i] == 16u || even[i] == 17u);

    MfNormalizeStatsToBst(magikarp, NUM_STATS, 100);
    EXPECT_EQ(SumStats(magikarp, NUM_STATS), 100u);
    EXPECT(magikarp[STAT_SPEED] > magikarp[STAT_DEF]);
    EXPECT(magikarp[STAT_DEF] > magikarp[STAT_HP]);
    EXPECT(magikarp[STAT_ATK] <= magikarp[STAT_HP]);
}

TEST("MF: BST equalizer largest-remainder matches a known vector")
{
    // Charizard modern: 78/84/78/100/109/85 = 534 → 100
    u32 stats[NUM_STATS] = {78, 84, 78, 100, 109, 85};

    MfNormalizeStatsToBst(stats, NUM_STATS, 100);
    EXPECT_EQ(SumStats(stats, NUM_STATS), 100u);
    EXPECT_EQ(stats[STAT_HP], 15u);
    EXPECT_EQ(stats[STAT_ATK], 16u);
    EXPECT_EQ(stats[STAT_DEF], 14u);
    EXPECT_EQ(stats[STAT_SPEED], 19u);
    EXPECT_EQ(stats[STAT_SPATK], 20u);
    EXPECT_EQ(stats[STAT_SPDEF], 16u);
}

TEST("MF: BST equalizer 100 makes every species share BST 100")
{
    static const enum Species sSamples[] = {
        SPECIES_MAGIKARP,
        SPECIES_CHARIZARD,
        SPECIES_CHANSEY,
        SPECIES_MEWTWO,
        SPECIES_SHEDINJA,
    };
    u32 i;

    SetBstRules(TRUE, 1);
    for (i = 0; i < ARRAY_COUNT(sSamples); i++)
        EXPECT_EQ(GetSpeciesBaseStatTotal(sSamples[i]), 100u);

    EXPECT_EQ(GetSpeciesBaseHP(SPECIES_SHEDINJA), 1u);

    RestorePhase1Defaults();
}

TEST("MF: BST equalizer applies after S29 modern/classic selection")
{
    u32 classicSpa;
    u32 modernSpa;

    SetBstRules(FALSE, 1);
    EXPECT_EQ(GetSpeciesBaseStatTotal(SPECIES_BUTTERFREE), 100u);
    classicSpa = GetSpeciesBaseSpAttack(SPECIES_BUTTERFREE);

    SetBstRules(TRUE, 1);
    EXPECT_EQ(GetSpeciesBaseStatTotal(SPECIES_BUTTERFREE), 100u);
    modernSpa = GetSpeciesBaseSpAttack(SPECIES_BUTTERFREE);

    EXPECT(modernSpa != classicSpa);

    SetBstRules(TRUE, 0);
    EXPECT_EQ(GetSpeciesBaseSpAttack(SPECIES_BUTTERFREE), 90u);
    SetBstRules(FALSE, 0);
    EXPECT_EQ(GetSpeciesBaseSpAttack(SPECIES_BUTTERFREE), 80u);

    RestorePhase1Defaults();
}

TEST("MF: BST equalizer 500 clamps a stat at 255 and keeps the total")
{
    u32 chansey[NUM_STATS] = {250, 5, 5, 50, 35, 105};
    u32 i;

    MfNormalizeStatsToBst(chansey, NUM_STATS, 500);
    EXPECT_EQ(SumStats(chansey, NUM_STATS), 500u);
    for (i = 0; i < NUM_STATS; i++)
        EXPECT(chansey[i] <= 255u);

    SetBstRules(TRUE, 3);
    EXPECT_EQ(GetSpeciesBaseStatTotal(SPECIES_CHANSEY), 500u);
    EXPECT(GetSpeciesBaseHP(SPECIES_CHANSEY) <= 255u);

    RestorePhase1Defaults();
}
