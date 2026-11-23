#include "global.h"
#include "test/test.h"
#include "test/battle.h"
#include "event_data.h"
#include "mf_rules.h"
#include "pokemon.h"

// End-of-turn damage that KOs the last foe sets the battle outcome inside the
// residual script (checkteamslost). DoEndTurnEffects then skipped straight to
// the trainer slides, past ENDTURN_FAINTED_MON_ACTIONS, so GiveExp never ran:
// no "gained Exp. Points" line and no exp bar.

enum
{
    RESIDUAL_POISON,
    RESIDUAL_TOXIC,
    RESIDUAL_BURN,
    RESIDUAL_LEECH_SEED,
    RESIDUAL_SANDSTORM,
};

static void ResidualRules(bool8 nuzlocke)
{
    struct ModernRules *save = MfRules_GetSaveRules();
    u32 i;

    MfRules_ApplyDevDefaults(save);
    save->rulesLocked = TRUE;
    save->nuzlocke = nuzlocke;
    for (i = 0; i < NUM_BADGES; i++)
        FlagClear(FLAG_BADGE01_GET + i);
    FlagClear(FLAG_SYS_GAME_CLEAR);
}

#define RESIDUAL_STATUS(r) ((r) == RESIDUAL_POISON ? STATUS1_POISON       \
                          : (r) == RESIDUAL_TOXIC  ? STATUS1_TOXIC_POISON \
                          : (r) == RESIDUAL_BURN   ? STATUS1_BURN         \
                          : STATUS1_NONE)

#define RESIDUAL_PARTIES(r)                                                                         \
    PLAYER(SPECIES_WOBBUFFET) { Level(5); MaxHP(500); HP(500); Speed(1);                            \
        Ability((r) == RESIDUAL_SANDSTORM ? ABILITY_SAND_STREAM : ABILITY_TELEPATHY);               \
        Moves((r) == RESIDUAL_LEECH_SEED ? MOVE_LEECH_SEED : MOVE_CELEBRATE); }                     \
    OPPONENT(SPECIES_RATTATA) { Level(5); MaxHP(100); HP(1); Speed(100);                            \
        Ability(ABILITY_RUN_AWAY); Status1(RESIDUAL_STATUS(r)); Moves(MOVE_CELEBRATE); }

#define EXPECT_RESIDUAL_KO_GAVE_EXP()                                                               \
    do {                                                                                            \
        u32 startExp = gExperienceTables[gSpeciesInfo[SPECIES_WOBBUFFET].growthRate][5];            \
        EXPECT_EQ(gBattleOutcome, B_OUTCOME_WON);                                                   \
        EXPECT_GT(GetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_EXP), startExp);              \
        ResidualRules(FALSE);                                                                       \
    } while (0)

WILD_BATTLE_TEST("MF: wild foe KO'd by end-of-turn damage still gives exp")
{
    u32 residual = RESIDUAL_POISON;
    bool8 nuzlocke = FALSE;

    PARAMETRIZE { residual = RESIDUAL_POISON;     nuzlocke = FALSE; }
    PARAMETRIZE { residual = RESIDUAL_POISON;     nuzlocke = TRUE; }
    PARAMETRIZE { residual = RESIDUAL_TOXIC;      nuzlocke = TRUE; }
    PARAMETRIZE { residual = RESIDUAL_BURN;       nuzlocke = TRUE; }
    PARAMETRIZE { residual = RESIDUAL_LEECH_SEED; nuzlocke = TRUE; }
    PARAMETRIZE { residual = RESIDUAL_SANDSTORM;  nuzlocke = TRUE; }

    GIVEN {
        ResidualRules(nuzlocke);
        RESIDUAL_PARTIES(residual);
    } WHEN {
        TURN { MOVE(player, residual == RESIDUAL_LEECH_SEED ? MOVE_LEECH_SEED : MOVE_CELEBRATE);
               MOVE(opponent, MOVE_CELEBRATE); }
    } SCENE {
        MESSAGE("The wild Rattata fainted!");
        EXPERIENCE_BAR(player);
    } THEN {
        EXPECT_RESIDUAL_KO_GAVE_EXP();
    }
}

AI_SINGLE_BATTLE_TEST("MF: trainer's last mon KO'd by end-of-turn damage still gives exp")
{
    u32 residual = RESIDUAL_POISON;

    PARAMETRIZE { residual = RESIDUAL_POISON; }
    PARAMETRIZE { residual = RESIDUAL_BURN; }
    PARAMETRIZE { residual = RESIDUAL_LEECH_SEED; }

    GIVEN {
        ResidualRules(TRUE);
        RESIDUAL_PARTIES(residual);
    } WHEN {
        TURN { MOVE(player, residual == RESIDUAL_LEECH_SEED ? MOVE_LEECH_SEED : MOVE_CELEBRATE); }
    } SCENE {
        MESSAGE("The opposing Rattata fainted!");
        EXPERIENCE_BAR(player);
    } THEN {
        EXPECT_RESIDUAL_KO_GAVE_EXP();
    }
}

// A non-final foe already got exp mid-battle; the fix must not double-pay it.
AI_SINGLE_BATTLE_TEST("MF: end-of-turn KO of a non-final trainer mon gives exp once")
{
    GIVEN {
        ResidualRules(TRUE);
        PLAYER(SPECIES_WOBBUFFET) { Level(5); MaxHP(500); HP(500); Speed(1); Moves(MOVE_CELEBRATE, MOVE_SCRATCH); }
        OPPONENT(SPECIES_RATTATA) { Level(5); MaxHP(100); HP(1); Speed(100); Status1(STATUS1_POISON); Moves(MOVE_CELEBRATE); }
        OPPONENT(SPECIES_PIDGEY) { Level(5); MaxHP(100); HP(1); Speed(100); Moves(MOVE_CELEBRATE); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); }
        TURN { MOVE(player, MOVE_SCRATCH); }
    } SCENE {
        MESSAGE("The opposing Rattata fainted!");
        EXPERIENCE_BAR(player);
        MESSAGE("The opposing Pidgey fainted!");
        EXPERIENCE_BAR(player);
    } THEN {
        EXPECT_EQ(gBattleOutcome, B_OUTCOME_WON);
        ResidualRules(FALSE);
    }
}
