#include "global.h"
#include "mf_move_ability.h"
#include "mf_rules.h"
#include "move.h"
#include "pokemon.h"
#include "test/test.h"
#include "constants/abilities.h"
#include "constants/moves.h"
#include "constants/pokemon.h"
#include "constants/species.h"

#define MF_MA_SEED_A 0x51A55E51u
#define MF_MA_SEED_B 0xA11AB1E5u
#define MF_MA_SEEDS  24

static const u16 sLowLevelSpecies[] = {
    SPECIES_BULBASAUR,
    SPECIES_CHARMANDER,
    SPECIES_SQUIRTLE,
    SPECIES_PIDGEY,
    SPECIES_RATTATA,
    SPECIES_WEEDLE,
    SPECIES_MAGIKARP,
    SPECIES_ABRA,
    SPECIES_ZUBAT,
    SPECIES_GEODUDE,
};

static void SetMoveAbilityRandomizer(u32 seed, bool8 moves, bool8 abilities)
{
    struct ModernRules *save = MfRules_GetSaveRules();

    MfRules_ResetToEmpty(save);
    save->version = MF_RULES_VERSION;
    save->randomizerSeed = seed;
    save->randomizerEnabled = TRUE;
    save->randomMoves = moves;
    save->randomAbilities = abilities;
    save->modernMoves = TRUE;
    save->rulesLocked = TRUE;
}

static void RestorePhase1Defaults(void)
{
    struct ModernRules *save = MfRules_GetSaveRules();

    save->version = 0;
}

TEST("MF: move remap is deterministic and identity when MOVES is off")
{
    enum Move a;
    enum Move b;
    enum Move vanilla;

    SetMoveAbilityRandomizer(MF_MA_SEED_A, TRUE, FALSE);
    a = MfMapMove(MF_MA_SEED_A, SPECIES_MAGIKARP, MOVE_SPLASH);
    b = MfMapMove(MF_MA_SEED_A, SPECIES_MAGIKARP, MOVE_SPLASH);
    EXPECT_EQ(a, b);
    EXPECT_NE(a, MOVE_NONE);
    EXPECT_NE(a, MOVE_STRUGGLE);
    EXPECT_NE(a, LEVEL_UP_MOVE_END);

    RestorePhase1Defaults();
    SetMoveAbilityRandomizer(MF_MA_SEED_A, FALSE, FALSE);
    vanilla = gSpeciesInfo[SPECIES_MAGIKARP].levelUpLearnset[0].move;
    EXPECT_EQ(GetSpeciesLevelUpLearnset(SPECIES_MAGIKARP)[0].move, vanilla);

    RestorePhase1Defaults();
}

TEST("MF: randomized learnsets keep a damaging move at low level")
{
    u32 seed;
    u32 s;

    for (seed = 0; seed < MF_MA_SEEDS; seed++)
    {
        u32 runSeed = MF_MA_SEED_A + seed * 0x9E3779B9u;

        SetMoveAbilityRandomizer(runSeed, TRUE, FALSE);
        for (s = 0; s < ARRAY_COUNT(sLowLevelSpecies); s++)
        {
            const struct LevelUpMove *learnset = GetSpeciesLevelUpLearnset(sLowLevelSpecies[s]);

            EXPECT(MfLearnsetHasDamagingMoveByLevel(learnset, MF_RANDOM_LOW_MOVE_LEVEL));
        }
        RestorePhase1Defaults();
    }
}

TEST("MF: different seeds remap the same learnset move differently")
{
    bool8 differ = FALSE;
    static const u16 sMoves[] = { MOVE_TACKLE, MOVE_SPLASH, MOVE_GROWL, MOVE_SCRATCH, MOVE_EMBER };
    u32 i;

    for (i = 0; i < ARRAY_COUNT(sMoves); i++)
    {
        if (MfMapMove(MF_MA_SEED_A, SPECIES_BULBASAUR, sMoves[i])
         != MfMapMove(MF_MA_SEED_B, SPECIES_BULBASAUR, sMoves[i]))
            differ = TRUE;
    }
    EXPECT(differ);

    RestorePhase1Defaults();
}

TEST("MF: ability remap is deterministic; form-locked stay pinned")
{
    enum Ability a;
    enum Ability b;
    enum Ability pidgey0;
    enum Ability pidgey0b;

    SetMoveAbilityRandomizer(MF_MA_SEED_A, FALSE, TRUE);

    a = GetSpeciesAbility(SPECIES_BULBASAUR, 0);
    b = MfMapAbility(MF_MA_SEED_A, SPECIES_BULBASAUR, 0);
    EXPECT_EQ(a, b);
    EXPECT_NE(a, ABILITY_NONE);
    EXPECT_NE(a, ABILITY_WONDER_GUARD);
    EXPECT_NE(a, ABILITY_FORECAST);

    EXPECT_EQ(GetSpeciesAbility(SPECIES_SHEDINJA, 0), ABILITY_WONDER_GUARD);
    EXPECT_EQ(MfMapAbility(MF_MA_SEED_A, SPECIES_SHEDINJA, 0), ABILITY_WONDER_GUARD);
    EXPECT_EQ(GetSpeciesAbility(SPECIES_CASTFORM, 0), ABILITY_FORECAST);

    pidgey0 = GetSpeciesAbility(SPECIES_PIDGEY, 0);
    pidgey0b = MfMapAbility(MF_MA_SEED_B, SPECIES_PIDGEY, 0);
    EXPECT_NE(pidgey0, ABILITY_WONDER_GUARD);
    EXPECT_NE(pidgey0b, ABILITY_WONDER_GUARD);

    RestorePhase1Defaults();
    EXPECT_EQ(GetSpeciesAbility(SPECIES_BULBASAUR, 0), gSpeciesInfo[SPECIES_BULBASAUR].abilities[0]);
}

TEST("MF: empty ability slots stay empty under ABILITIES")
{
    u8 slot;

    SetMoveAbilityRandomizer(MF_MA_SEED_A, FALSE, TRUE);
    for (slot = 0; slot < NUM_ABILITY_SLOTS; slot++)
    {
        if (gSpeciesInfo[SPECIES_UNOWN].abilities[slot] == ABILITY_NONE)
            EXPECT_EQ(GetSpeciesAbility(SPECIES_UNOWN, slot), ABILITY_NONE);
    }
    RestorePhase1Defaults();
}

TEST("MF: GetSpeciesLevelUpLearnset returns the remapped table")
{
    const struct LevelUpMove *learnset;
    const struct LevelUpMove *vanilla;
    bool8 differed = FALSE;
    u32 i;

    SetMoveAbilityRandomizer(MF_MA_SEED_A, TRUE, FALSE);
    learnset = GetSpeciesLevelUpLearnset(SPECIES_CHARMANDER);
    vanilla = gSpeciesInfo[SPECIES_CHARMANDER].levelUpLearnset;
    for (i = 0; learnset[i].move != LEVEL_UP_MOVE_END && vanilla[i].move != LEVEL_UP_MOVE_END; i++)
    {
        if (learnset[i].move != vanilla[i].move)
            differed = TRUE;
        EXPECT_EQ(learnset[i].level, vanilla[i].level);
    }
    EXPECT(differed);
    EXPECT(learnset[0].move != MOVE_NONE);

    RestorePhase1Defaults();
}
