#include "global.h"
#include "test/test.h"
#include "test/battle.h"
#include "config/battle.h"
#include "difficulty.h"
#include "event_data.h"
#include "mf_exp.h"
#include "mf_level_cap.h"
#include "mf_rules.h"
#include "mf_species_map.h"
#include "pokemon.h"

static struct ModernRules *PrepareRules(void)
{
    struct ModernRules *save = MfRules_GetSaveRules();

    MfRules_ApplyDevDefaults(save);
    save->rulesLocked = TRUE;
    {
        u32 i;
        for (i = 0; i < NUM_BADGES; i++)
            FlagClear(FLAG_BADGE01_GET + i);
    }
    FlagClear(FLAG_SYS_GAME_CLEAR);
    SetCurrentDifficultyLevel(DIFFICULTY_NORMAL);
    return save;
}

TEST("MF: trainer battles keep the Gen 3 1.5x exp bonus")
{
    // Difficulty EXP. MULTIPLIER (S42) is a separate pool scale after this.
    EXPECT_EQ(B_TRAINER_EXP_MULTIPLIER, GEN_3);
    EXPECT(B_TRAINER_EXP_MULTIPLIER <= GEN_7);
}

TEST("MF: exp pool multiplier and hard-mode factor")
{
    // ×1, ×1.5, ×2, ×0. Hard factor is 60% after the multiplier. ×0 wins.
    EXPECT_EQ(MfResolveExpPool(100, MF_EXP_MULT_1X, FALSE), 100u);
    EXPECT_EQ(MfResolveExpPool(100, MF_EXP_MULT_15X, FALSE), 150u);
    EXPECT_EQ(MfResolveExpPool(100, MF_EXP_MULT_2X, FALSE), 200u);
    EXPECT_EQ(MfResolveExpPool(100, MF_EXP_MULT_0X, FALSE), 0u);
    EXPECT_EQ(MfResolveExpPool(100, MF_EXP_MULT_0X, TRUE), 0u);

    EXPECT_EQ(MfResolveExpPool(100, MF_EXP_MULT_1X, TRUE), 60u);
    EXPECT_EQ(MfResolveExpPool(100, MF_EXP_MULT_15X, TRUE), 90u);
    EXPECT_EQ(MfResolveExpPool(100, MF_EXP_MULT_2X, TRUE), 120u);

    // Truncation matches (n * 3) / 2 and (n * 3) / 5.
    EXPECT_EQ(MfResolveExpPool(7, MF_EXP_MULT_15X, FALSE), 10u);
    EXPECT_EQ(MfResolveExpPool(1, MF_EXP_MULT_1X, TRUE), 0u);
    // Values outside the 2-bit menu stay at ×1.
    EXPECT_EQ(MfResolveExpPool(40, 255, FALSE), 40u);
}

TEST("MF: hard exp reduction is Hard level cap or Hard difficulty")
{
    EXPECT(!MfIsHardExpReductionActive(MF_LEVEL_CAP_OFF, FALSE, FALSE, FALSE));
    EXPECT(!MfIsHardExpReductionActive(MF_LEVEL_CAP_NORMAL, FALSE, FALSE, FALSE));
    EXPECT(MfIsHardExpReductionActive(MF_LEVEL_CAP_HARD, FALSE, FALSE, FALSE));
    // Normal (bit 1) keeps the full rate. Hall of Fame lifts the cut.
    EXPECT(!MfIsHardExpReductionActive(MF_LEVEL_CAP_HARD, TRUE, FALSE, FALSE));
    EXPECT(!MfIsHardExpReductionActive(MF_LEVEL_CAP_HARD, FALSE, FALSE, TRUE));
    EXPECT(MfIsHardExpReductionActive(MF_LEVEL_CAP_OFF, FALSE, TRUE, FALSE));
    EXPECT(!MfIsHardExpReductionActive(MF_LEVEL_CAP_OFF, FALSE, TRUE, TRUE));
    EXPECT(!MfIsHardExpReductionActive(MF_LEVEL_CAP_OFF, TRUE, TRUE, FALSE));
    // Both hard signals still mean one 60% factor, not 36%.
    EXPECT(MfIsHardExpReductionActive(MF_LEVEL_CAP_HARD, FALSE, TRUE, FALSE));
    EXPECT_EQ(MfResolveExpPool(100, MF_EXP_MULT_1X, TRUE), 60u);
}

TEST("MF: live exp scale follows rules, badges not required")
{
    struct ModernRules *save = PrepareRules();

    EXPECT_EQ(MfScaleCalculatedExp(100), 100u);
    EXPECT(!MfIsExpMultiplierZero());

    save->expMultiplier = MF_EXP_MULT_15X;
    EXPECT_EQ(MfScaleCalculatedExp(100), 150u);
    save->expMultiplier = MF_EXP_MULT_2X;
    EXPECT_EQ(MfScaleCalculatedExp(200), 400u);
    save->expMultiplier = MF_EXP_MULT_0X;
    EXPECT(MfIsExpMultiplierZero());
    EXPECT_EQ(MfScaleCalculatedExp(100), 0u);

    save->expMultiplier = MF_EXP_MULT_1X;
    save->levelCap = MF_LEVEL_CAP_HARD;
    save->hardExp = FALSE;
    EXPECT_EQ(MfScaleCalculatedExp(100), 60u);

    save->hardExp = TRUE;
    EXPECT_EQ(MfScaleCalculatedExp(100), 100u);

    save->hardExp = FALSE;
    FlagSet(FLAG_SYS_GAME_CLEAR);
    EXPECT_EQ(MfScaleCalculatedExp(100), 100u);
    FlagClear(FLAG_SYS_GAME_CLEAR);

    save->levelCap = MF_LEVEL_CAP_OFF;
    SetCurrentDifficultyLevel(DIFFICULTY_HARD);
    EXPECT_EQ(MfScaleCalculatedExp(100), 60u);

    save->hardExp = TRUE;
    EXPECT_EQ(MfScaleCalculatedExp(100), 100u);

    SetCurrentDifficultyLevel(DIFFICULTY_NORMAL);
}

TEST("MF: scaled exp gate follows the save bit")
{
    struct ModernRules *save = PrepareRules();

    EXPECT(!save->scaledExp);
    EXPECT(!MfIsScaledExpActive());

    save->scaledExp = TRUE;
    EXPECT(MfIsScaledExpActive());

    save->scaledExp = FALSE;
    EXPECT(!MfIsScaledExpActive());
}

static void SetBattleExpRules(u8 multiplier, u8 levelCap, bool8 hardExp, bool8 scaledExp)
{
    struct ModernRules *save = PrepareRules();

    save->expMultiplier = multiplier;
    save->levelCap = levelCap;
    save->hardExp = hardExp;
    save->scaledExp = scaledExp;
}

// SINGLE_BATTLE_TEST is recorded-link and skips EXP. AI trainer battles keep BATTLE_TYPE_TRAINER.
AI_SINGLE_BATTLE_TEST("MF: trainer exp is 1.5x the Gen7+ pool", s32 exp)
{
    u32 config = 0;

    PARAMETRIZE { config = GEN_3; }
    PARAMETRIZE { config = GEN_8; }

    GIVEN {
        WITH_CONFIG(B_TRAINER_EXP_MULTIPLIER, config);
        SetBattleExpRules(MF_EXP_MULT_1X, MF_LEVEL_CAP_OFF, FALSE, FALSE);
        PLAYER(SPECIES_WOBBUFFET) { Level(20); Speed(100); }
        OPPONENT(SPECIES_CATERPIE) { Level(10); HP(1); Speed(1); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_SCRATCH); }
    } SCENE {
        MESSAGE("Wobbuffet used Scratch!");
        MESSAGE("The opposing Caterpie fainted!");
        EXPERIENCE_BAR(player, captureGainedExp: &results[i].exp);
    } FINALLY {
        EXPECT_MUL_EQ(results[1].exp, Q_4_12(1.5), results[0].exp);
        SetBattleExpRules(MF_EXP_MULT_1X, MF_LEVEL_CAP_OFF, FALSE, FALSE);
    }
}

WILD_BATTLE_TEST("MF: each exp multiplier changes exp from the same battle", s32 exp)
{
    u8 multiplier = 0;

    PARAMETRIZE { multiplier = MF_EXP_MULT_1X; }
    PARAMETRIZE { multiplier = MF_EXP_MULT_15X; }
    PARAMETRIZE { multiplier = MF_EXP_MULT_2X; }

    GIVEN {
        SetBattleExpRules(multiplier, MF_LEVEL_CAP_OFF, FALSE, FALSE);
        PLAYER(SPECIES_WOBBUFFET) { Level(20); }
        OPPONENT(SPECIES_CATERPIE) { Level(10); HP(1); }
    } WHEN {
        TURN { MOVE(player, MOVE_SCRATCH); }
    } SCENE {
        MESSAGE("Wobbuffet used Scratch!");
        MESSAGE("The wild Caterpie fainted!");
        EXPERIENCE_BAR(player, captureGainedExp: &results[i].exp);
    } FINALLY {
        EXPECT_GT(results[1].exp, results[0].exp);
        EXPECT_GT(results[2].exp, results[1].exp);
        SetBattleExpRules(MF_EXP_MULT_1X, MF_LEVEL_CAP_OFF, FALSE, FALSE);
    }
}

WILD_BATTLE_TEST("MF: exp multiplier x0 grants no experience")
{
    u32 startExp = 0;

    GIVEN {
        SetBattleExpRules(MF_EXP_MULT_0X, MF_LEVEL_CAP_OFF, FALSE, FALSE);
        PLAYER(SPECIES_WOBBUFFET) { Level(20); }
        OPPONENT(SPECIES_CATERPIE) { Level(10); HP(1); }
    } WHEN {
        TURN { MOVE(player, MOVE_SCRATCH); }
    } SCENE {
        MESSAGE("Wobbuffet used Scratch!");
        MESSAGE("The wild Caterpie fainted!");
        NOT EXPERIENCE_BAR(player);
    } THEN {
        startExp = gExperienceTables[gSpeciesInfo[SPECIES_WOBBUFFET].growthRate][20];
        EXPECT_EQ(GetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_EXP), startExp);
        SetBattleExpRules(MF_EXP_MULT_1X, MF_LEVEL_CAP_OFF, FALSE, FALSE);
    }
}

WILD_BATTLE_TEST("MF: hard exp cuts gain while the level cap is Hard", s32 exp)
{
    bool8 hardExpNormal = FALSE;

    PARAMETRIZE { hardExpNormal = FALSE; }
    PARAMETRIZE { hardExpNormal = TRUE; }

    GIVEN {
        SetBattleExpRules(MF_EXP_MULT_1X, MF_LEVEL_CAP_HARD, hardExpNormal, FALSE);
        // Under the 0-badge Hard cap (12) so the ceiling does not zero the reward.
        PLAYER(SPECIES_WOBBUFFET) { Level(10); }
        OPPONENT(SPECIES_CATERPIE) { Level(10); HP(1); }
    } WHEN {
        TURN { MOVE(player, MOVE_SCRATCH); }
    } SCENE {
        MESSAGE("Wobbuffet used Scratch!");
        MESSAGE("The wild Caterpie fainted!");
        EXPERIENCE_BAR(player, captureGainedExp: &results[i].exp);
    } FINALLY {
        EXPECT_GT(results[1].exp, results[0].exp);
        EXPECT(results[0].exp > 0);
        SetBattleExpRules(MF_EXP_MULT_1X, MF_LEVEL_CAP_OFF, FALSE, FALSE);
    }
}

WILD_BATTLE_TEST("MF: scaled exp off pays the same at two player levels", s32 exp)
{
    u8 level = 0;

    PARAMETRIZE { level = 10; }
    PARAMETRIZE { level = 20; }

    GIVEN {
        SetBattleExpRules(MF_EXP_MULT_1X, MF_LEVEL_CAP_OFF, FALSE, FALSE);
        PLAYER(SPECIES_WOBBUFFET) { Level(level); }
        OPPONENT(SPECIES_CATERPIE) { Level(10); HP(1); }
    } WHEN {
        TURN { MOVE(player, MOVE_SCRATCH); }
    } SCENE {
        MESSAGE("Wobbuffet used Scratch!");
        MESSAGE("The wild Caterpie fainted!");
        EXPERIENCE_BAR(player, captureGainedExp: &results[i].exp);
    } FINALLY {
        EXPECT_EQ(results[0].exp, results[1].exp);
        EXPECT(results[0].exp > 0);
        SetBattleExpRules(MF_EXP_MULT_1X, MF_LEVEL_CAP_OFF, FALSE, FALSE);
    }
}

WILD_BATTLE_TEST("MF: scaled exp on pays less to a higher-level Pokemon", s32 exp)
{
    u8 level = 0;

    PARAMETRIZE { level = 10; }
    PARAMETRIZE { level = 20; }

    GIVEN {
        SetBattleExpRules(MF_EXP_MULT_1X, MF_LEVEL_CAP_OFF, FALSE, TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Level(level); }
        OPPONENT(SPECIES_CATERPIE) { Level(10); HP(1); }
    } WHEN {
        TURN { MOVE(player, MOVE_SCRATCH); }
    } SCENE {
        MESSAGE("Wobbuffet used Scratch!");
        MESSAGE("The wild Caterpie fainted!");
        EXPERIENCE_BAR(player, captureGainedExp: &results[i].exp);
    } FINALLY {
        EXPECT_GT(results[0].exp, results[1].exp);
        SetBattleExpRules(MF_EXP_MULT_1X, MF_LEVEL_CAP_OFF, FALSE, FALSE);
    }
}

TEST("MF: randomizer-pool species keep a non-zero unscaled wild exp pool at levels 1-10")
{
    u16 i;
    u8 level;
    u16 count = MfSpeciesMap_GetPoolCount();
    u32 zeroPool = 0;
    u32 firstBad = SPECIES_NONE;

    SetBattleExpRules(MF_EXP_MULT_1X, MF_LEVEL_CAP_OFF, FALSE, FALSE);
    for (i = 0; i < count; i++)
    {
        enum Species species = MfSpeciesMap_GetPoolSpecies(i);

        for (level = 1; level <= 10; level++)
        {
            u32 pool = MfScaleCalculatedExp(gSpeciesInfo[species].expYield * level / 7);

            if (pool == 0 && firstBad == SPECIES_NONE)
                firstBad = species;
            if (pool == 0)
                zeroPool++;
        }
    }

    EXPECT_EQ(zeroPool, 0u);
    EXPECT_EQ(firstBad, SPECIES_NONE);
}

// Cmd_getexp skips HP-0 participants (IsValidForBattle). A contact KO plus
// Rocky Helmet / recoil can faint the winner in the same turn, so nobody is
// alive to receive the pool. That matches "sometimes no XP" with randomizer
// abilities/items.
WILD_BATTLE_TEST("MF: the KO'ing mon still gets exp if it faints to Rocky Helmet")
{
    u32 startExp = 0;
    u32 endExp = 0;

    GIVEN {
        ASSUME(MoveMakesContact(MOVE_SCRATCH));
        ASSUME(GetItemHoldEffect(ITEM_ROCKY_HELMET) == HOLD_EFFECT_ROCKY_HELMET);
        SetBattleExpRules(MF_EXP_MULT_1X, MF_LEVEL_CAP_OFF, FALSE, FALSE);
        PLAYER(SPECIES_WOBBUFFET) { Level(20); HP(1); MaxHP(100); }
        PLAYER(SPECIES_WYNAUT) { Level(20); }
        OPPONENT(SPECIES_CATERPIE) { Level(10); HP(1); Item(ITEM_ROCKY_HELMET); }
    } WHEN {
        TURN { MOVE(player, MOVE_SCRATCH); }
    } SCENE {
        MESSAGE("The wild Caterpie fainted!");
    } THEN {
        startExp = gExperienceTables[gSpeciesInfo[SPECIES_WOBBUFFET].growthRate][20];
        endExp = GetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_EXP);
        EXPECT_GT(endExp, startExp);
        SetBattleExpRules(MF_EXP_MULT_1X, MF_LEVEL_CAP_OFF, FALSE, FALSE);
    }
}

// Living lead, guaranteed KO. Struggle is typeless; 1 HP ignores walls
// (Shuckle took 0 from Scratch at full HP). Faint + 0 EXP is the miss.
WILD_BATTLE_TEST("MF: clean KO grants exp through walls, disguise, and transform")
{
    u32 species = SPECIES_SHUCKLE;
    u32 startExp = 0;
    u32 endExp = 0;

    PARAMETRIZE { species = SPECIES_SHUCKLE; }
    PARAMETRIZE { species = SPECIES_DITTO; }
    PARAMETRIZE { species = SPECIES_SHEDINJA; }
    PARAMETRIZE { species = SPECIES_MIMIKYU; }
    PARAMETRIZE { species = SPECIES_ZOROARK; }
    PARAMETRIZE { species = SPECIES_WOBBUFFET; }

    GIVEN {
        SetBattleExpRules(MF_EXP_MULT_1X, MF_LEVEL_CAP_OFF, FALSE, FALSE);
        PLAYER(SPECIES_WOBBUFFET) { Level(20); MaxHP(200); HP(200); Speed(100); Ability(ABILITY_MOLD_BREAKER); }
        OPPONENT(species) { Level(5); MaxHP(1); HP(1); Speed(1); Ability(ABILITY_INNER_FOCUS); }
    } WHEN {
        TURN { MOVE(player, MOVE_STRUGGLE); }
    } THEN {
        startExp = gExperienceTables[gSpeciesInfo[SPECIES_WOBBUFFET].growthRate][20];
        endExp = GetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_EXP);
        EXPECT_EQ(gBattleMons[B_BATTLER_1].hp, 0);
        EXPECT_GT(endExp, startExp);
        SetBattleExpRules(MF_EXP_MULT_1X, MF_LEVEL_CAP_OFF, FALSE, FALSE);
    }
}

// Consecutive wilds paying then not paying is per-encounter, not a sticky cap.
// Wimp Out / Emergency Exit slides off without fainting — no GiveExp.
WILD_BATTLE_TEST("MF: Emergency Exit flee grants no exp")
{
    u32 startExp = 0;

    GIVEN {
        SetBattleExpRules(MF_EXP_MULT_1X, MF_LEVEL_CAP_OFF, FALSE, FALSE);
        PLAYER(SPECIES_WOBBUFFET) { Level(20); Speed(100); }
        OPPONENT(SPECIES_GOLISOPOD) { Ability(ABILITY_EMERGENCY_EXIT); MaxHP(263); HP(262); Speed(1); }
    } WHEN {
        TURN { MOVE(player, MOVE_SUPER_FANG); }
    } THEN {
        startExp = gExperienceTables[gSpeciesInfo[SPECIES_WOBBUFFET].growthRate][20];
        EXPECT_EQ(gBattleOutcome, B_OUTCOME_MON_TELEPORTED);
        EXPECT_EQ(GetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_EXP), startExp);
        SetBattleExpRules(MF_EXP_MULT_1X, MF_LEVEL_CAP_OFF, FALSE, FALSE);
    }
}

WILD_BATTLE_TEST("MF: wild Teleport grants no exp")
{
    u32 startExp = 0;

    GIVEN {
        SetBattleExpRules(MF_EXP_MULT_1X, MF_LEVEL_CAP_OFF, FALSE, FALSE);
        PLAYER(SPECIES_WOBBUFFET) { Level(20); Speed(1); }
        OPPONENT(SPECIES_ABRA) { Level(20); Speed(100); Moves(MOVE_TELEPORT); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_TELEPORT); }
    } THEN {
        startExp = gExperienceTables[gSpeciesInfo[SPECIES_WOBBUFFET].growthRate][20];
        EXPECT_EQ(GetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_EXP), startExp);
        SetBattleExpRules(MF_EXP_MULT_1X, MF_LEVEL_CAP_OFF, FALSE, FALSE);
    }
}

WILD_BATTLE_TEST("MF: nuzlocke and randomizer flags still grant exp on a clean KO")
{
    u32 startExp = 0;
    u32 endExp = 0;

    GIVEN {
        SetBattleExpRules(MF_EXP_MULT_1X, MF_LEVEL_CAP_OFF, FALSE, FALSE);
        {
            struct ModernRules *save = MfRules_GetSaveRules();
            save->nuzlocke = TRUE;
            save->randomizerEnabled = TRUE;
            save->randomWild = TRUE;
        }
        PLAYER(SPECIES_WOBBUFFET) { Level(20); MaxHP(200); HP(200); Speed(100); Ability(ABILITY_MOLD_BREAKER); }
        OPPONENT(SPECIES_CATERPIE) { Level(5); MaxHP(1); HP(1); Speed(1); Ability(ABILITY_INNER_FOCUS); }
    } WHEN {
        TURN { MOVE(player, MOVE_STRUGGLE); }
    } THEN {
        startExp = gExperienceTables[gSpeciesInfo[SPECIES_WOBBUFFET].growthRate][20];
        endExp = GetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_EXP);
        EXPECT_EQ(gBattleMons[B_BATTLER_1].hp, 0);
        EXPECT_GT(endExp, startExp);
        SetBattleExpRules(MF_EXP_MULT_1X, MF_LEVEL_CAP_OFF, FALSE, FALSE);
    }
}

// Gym cap zeros the reward and skips the "gained EXP" line, so a clean faint
// looks like no XP. 0 badges → Normal cap 14.
WILD_BATTLE_TEST("MF: a living lead at the level cap gets no exp from a clean KO")
{
    u32 startExp = 0;

    GIVEN {
        SetBattleExpRules(MF_EXP_MULT_1X, MF_LEVEL_CAP_NORMAL, FALSE, FALSE);
        PLAYER(SPECIES_WOBBUFFET) { Level(14); MaxHP(200); HP(200); Speed(100); Ability(ABILITY_MOLD_BREAKER); }
        OPPONENT(SPECIES_CATERPIE) { Level(5); MaxHP(1); HP(1); Speed(1); Ability(ABILITY_INNER_FOCUS); }
    } WHEN {
        TURN { MOVE(player, MOVE_STRUGGLE); }
    } SCENE {
        NOT EXPERIENCE_BAR(player);
    } THEN {
        startExp = gExperienceTables[gSpeciesInfo[SPECIES_WOBBUFFET].growthRate][14];
        EXPECT_EQ(gBattleMons[B_BATTLER_1].hp, 0);
        EXPECT_EQ(GetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_EXP), startExp);
        SetBattleExpRules(MF_EXP_MULT_1X, MF_LEVEL_CAP_OFF, FALSE, FALSE);
    }
}

WILD_BATTLE_TEST("MF: the KO'ing mon still gets exp if it faints to recoil")
{
    u32 startExp = 0;
    u32 endExp = 0;

    GIVEN {
        ASSUME(GetMoveRecoil(MOVE_DOUBLE_EDGE) != 0);
        SetBattleExpRules(MF_EXP_MULT_1X, MF_LEVEL_CAP_OFF, FALSE, FALSE);
        PLAYER(SPECIES_WOBBUFFET) { Level(20); HP(1); MaxHP(100); }
        PLAYER(SPECIES_WYNAUT) { Level(20); }
        OPPONENT(SPECIES_SNORLAX) { Level(20); HP(1); }
    } WHEN {
        TURN { MOVE(player, MOVE_DOUBLE_EDGE); }
    } THEN {
        startExp = gExperienceTables[gSpeciesInfo[SPECIES_WOBBUFFET].growthRate][20];
        endExp = GetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_EXP);
        EXPECT_GT(endExp, startExp);
        SetBattleExpRules(MF_EXP_MULT_1X, MF_LEVEL_CAP_OFF, FALSE, FALSE);
    }
}
