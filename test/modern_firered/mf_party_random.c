#include "global.h"
#include "string.h"
#include "battle.h"
#include "data.h"
#include "mf_party_random.h"
#include "mf_monotype.h"
#include "mf_rules.h"
#include "mf_species_map.h"
#include "pokemon.h"
#include "test/test.h"
#include "constants/abilities.h"
#include "constants/battle.h"
#include "constants/items.h"
#include "constants/moves.h"
#include "constants/species.h"
#include "constants/trainers.h"

#define MF_PARTY_TEST_SEED 0xA53A53A5u
#define MF_TRAINER_KEY_A 0x1111
#define MF_TRAINER_KEY_B 0x2222

static void SetTrainerRandomizer(bool8 similar)
{
    struct ModernRules *save = MfRules_GetSaveRules();

    MfRules_ResetToEmpty(save);
    save->version = MF_RULES_VERSION;
    save->randomizerSeed = MF_PARTY_TEST_SEED;
    save->randomizerEnabled = TRUE;
    save->randomTrainer = TRUE;
    save->randomStarter = TRUE;
    save->randomMapBased = FALSE;
    save->randomSimilar = similar;
    save->randomIncludeLegendaries = FALSE;
    save->randomThemedTrainers = FALSE;
    save->fairyTypes = TRUE;
    save->rulesLocked = TRUE;
}

static void RestorePhase1Defaults(void)
{
    struct ModernRules *save = MfRules_GetSaveRules();

    save->version = 0;
}

TEST("MF: trainer remap is deterministic and keeps level")
{
    struct TrainerMon mon;
    enum Species dest;
    enum Species destAgain;
    u8 level = 12;

    SetTrainerRandomizer(TRUE);
    memset(&mon, 0, sizeof(mon));
    mon.species = SPECIES_GEODUDE;
    mon.lvl = level;
    mon.heldItem = ITEM_NONE;
    mon.moves[0] = MOVE_TACKLE;
    mon.ability = ABILITY_STURDY;
    mon.gender = TRAINER_MON_MALE;

    dest = MfTrainerEncounterSpecies(SPECIES_GEODUDE, MF_TRAINER_KEY_A);
    destAgain = MfTrainerEncounterSpecies(SPECIES_GEODUDE, MF_TRAINER_KEY_A);
    EXPECT_EQ(dest, destAgain);
    EXPECT(MfSpeciesMap_IsCandidate(dest));
    EXPECT_EQ(MfSpeciesMap_GetEvoStage(dest), MfSpeciesMap_GetEvoStage(SPECIES_GEODUDE));

    MfRandomizeTrainerMon(&mon, MF_TRAINER_KEY_A);
    EXPECT_EQ(mon.species, dest);
    EXPECT_EQ(mon.lvl, level);
    if (dest != SPECIES_GEODUDE)
    {
        u8 gender = mon.gender;

        EXPECT_EQ(mon.moves[0], MOVE_NONE);
        EXPECT_EQ(mon.ability, ABILITY_NONE);
        EXPECT_EQ(gender, TRAINER_MON_RANDOM_GENDER);
    }

    RestorePhase1Defaults();
}

TEST("MF: trainer remap is identity when TRAINER is off")
{
    struct TrainerMon mon;

    SetTrainerRandomizer(TRUE);
    MfRules_GetSaveRules()->randomTrainer = FALSE;
    EXPECT_EQ(MfTrainerEncounterSpecies(SPECIES_GEODUDE, MF_TRAINER_KEY_A), SPECIES_GEODUDE);

    memset(&mon, 0, sizeof(mon));
    mon.species = SPECIES_GEODUDE;
    mon.lvl = 14;
    mon.moves[0] = MOVE_TACKLE;
    MfRandomizeTrainerMon(&mon, MF_TRAINER_KEY_A);
    EXPECT_EQ(mon.species, SPECIES_GEODUDE);
    EXPECT_EQ(mon.lvl, 14);
    EXPECT_EQ(mon.moves[0], MOVE_TACKLE);

    RestorePhase1Defaults();
}

TEST("MF: trainer keys change the replacement")
{
    enum Species a;
    enum Species b;

    SetTrainerRandomizer(FALSE);
    a = MfTrainerEncounterSpecies(SPECIES_PIDGEY, MF_TRAINER_KEY_A);
    b = MfTrainerEncounterSpecies(SPECIES_PIDGEY, MF_TRAINER_KEY_B);
    EXPECT_NE(a, SPECIES_NONE);
    EXPECT_NE(b, SPECIES_NONE);
    EXPECT_NE(a, b);

    RestorePhase1Defaults();
}

TEST("MF: trainer remap skips Frontier battles")
{
    SetTrainerRandomizer(FALSE);
    gBattleTypeFlags = BATTLE_TYPE_FRONTIER;
    EXPECT_EQ(MfTrainerEncounterSpecies(SPECIES_PIDGEY, MF_TRAINER_KEY_A), SPECIES_PIDGEY);
    gBattleTypeFlags = 0;
    RestorePhase1Defaults();
}

TEST("MF: Oak starters remap uniquely and deterministically")
{
    enum Species a[MF_OAK_STARTER_COUNT];
    enum Species b[MF_OAK_STARTER_COUNT];

    SetTrainerRandomizer(TRUE);
    MfFillRandomOakStarters(a);
    MfFillRandomOakStarters(b);
    EXPECT_EQ(a[0], b[0]);
    EXPECT_EQ(a[1], b[1]);
    EXPECT_EQ(a[2], b[2]);
    EXPECT_NE(a[0], a[1]);
    EXPECT_NE(a[0], a[2]);
    EXPECT_NE(a[1], a[2]);
    EXPECT(MfSpeciesMap_IsCandidate(a[0]));
    EXPECT_EQ(MfSpeciesMap_GetEvoStage(a[0]), MF_SPECIES_MAP_STAGE_0);
    EXPECT_EQ(MfSpeciesMap_GetEvoStage(a[1]), MF_SPECIES_MAP_STAGE_0);
    EXPECT_EQ(MfSpeciesMap_GetEvoStage(a[2]), MF_SPECIES_MAP_STAGE_0);

    RestorePhase1Defaults();
}

TEST("MF: Oak starters stay vanilla when STARTER is off")
{
    enum Species trio[MF_OAK_STARTER_COUNT];

    SetTrainerRandomizer(TRUE);
    MfRules_GetSaveRules()->randomStarter = FALSE;
    MfFillRandomOakStarters(trio);
    EXPECT_EQ(trio[0], SPECIES_BULBASAUR);
    EXPECT_EQ(trio[1], SPECIES_SQUIRTLE);
    EXPECT_EQ(trio[2], SPECIES_CHARMANDER);

    RestorePhase1Defaults();
}

TEST("MF: rival starter keeps type advantage when it still exists")
{
    EXPECT(MfSpeciesHasOffensiveTypeAdvantage(SPECIES_CHARMANDER, SPECIES_BULBASAUR));
    EXPECT(MfSpeciesHasOffensiveTypeAdvantage(SPECIES_BULBASAUR, SPECIES_SQUIRTLE));
    EXPECT(MfSpeciesHasOffensiveTypeAdvantage(SPECIES_SQUIRTLE, SPECIES_CHARMANDER));
    EXPECT(!MfSpeciesHasOffensiveTypeAdvantage(SPECIES_CHARMANDER, SPECIES_CHARMANDER));

    EXPECT_EQ(MfPickRivalStarterSpecies(SPECIES_BULBASAUR, SPECIES_CHARMANDER, SPECIES_SQUIRTLE),
              SPECIES_CHARMANDER);
    EXPECT_EQ(MfPickRivalStarterSpecies(SPECIES_STEELIX, SPECIES_CHARMANDER, SPECIES_BULBASAUR),
              SPECIES_CHARMANDER);
    EXPECT_EQ(MfPickRivalStarterSpecies(SPECIES_CHARMANDER, SPECIES_CHARMANDER, SPECIES_SQUIRTLE),
              SPECIES_SQUIRTLE);
    EXPECT_EQ(MfPickRivalStarterSpecies(SPECIES_PIDGEY, SPECIES_RATTATA, SPECIES_PIDGEY),
              SPECIES_RATTATA);
}

TEST("MF: themed trainer party shares one type")
{
    enum Species dests[3];
    enum Type theme;
    u8 i;

    SetTrainerRandomizer(TRUE);
    MfRules_GetSaveRules()->randomThemedTrainers = TRUE;

    theme = MfTrainerThemeType(MF_TRAINER_KEY_A);
    EXPECT_NE(theme, TYPE_NONE);
    EXPECT_NE(theme, TYPE_MYSTERY);

    dests[0] = MfTrainerEncounterSpecies(SPECIES_GEODUDE, MF_TRAINER_KEY_A);
    dests[1] = MfTrainerEncounterSpecies(SPECIES_PIDGEY, MF_TRAINER_KEY_A);
    dests[2] = MfTrainerEncounterSpecies(SPECIES_ABRA, MF_TRAINER_KEY_A);
    EXPECT_EQ(theme, MfTrainerThemeType(MF_TRAINER_KEY_A));
    for (i = 0; i < 3; i++)
    {
        EXPECT(MfSpeciesMap_IsCandidate(dests[i]));
        EXPECT(MfSpeciesMatchesType(dests[i], theme));
    }

    RestorePhase1Defaults();
}

TEST("MF: themed dual-type counts either slot")
{
    enum Species dest;

    SetTrainerRandomizer(FALSE);
    dest = MfSpeciesMapExForType(SPECIES_MAGIKARP,
                                 MF_PARTY_TEST_SEED,
                                 MF_RANDOM_CAT_TRAINER,
                                 MF_TRAINER_KEY_A,
                                 FALSE,
                                 FALSE,
                                 TYPE_FLYING);
    EXPECT(MfSpeciesMatchesType(dest, TYPE_FLYING));
    EXPECT(GetSpeciesType(dest, 0) == TYPE_FLYING || GetSpeciesType(dest, 1) == TYPE_FLYING);

    RestorePhase1Defaults();
}

TEST("MF: themed balancing keeps evo stage when a typed match exists")
{
    enum Species dest;

    SetTrainerRandomizer(TRUE);
    dest = MfSpeciesMapExForType(SPECIES_GEODUDE,
                                 MF_PARTY_TEST_SEED,
                                 MF_RANDOM_CAT_TRAINER,
                                 MF_TRAINER_KEY_A,
                                 TRUE,
                                 FALSE,
                                 TYPE_WATER);
    EXPECT(MfSpeciesMatchesType(dest, TYPE_WATER));
    EXPECT_EQ(MfSpeciesMap_GetEvoStage(dest), MfSpeciesMap_GetEvoStage(SPECIES_GEODUDE));

    RestorePhase1Defaults();
}

TEST("MF: themed trainers never pick Fairy when Fairy types are off")
{
    u16 key;
    enum Type theme;

    SetTrainerRandomizer(FALSE);
    MfRules_GetSaveRules()->randomThemedTrainers = TRUE;
    MfRules_GetSaveRules()->fairyTypes = FALSE;

    for (key = 0; key < 64; key++)
    {
        theme = MfTrainerThemeType(key);
        EXPECT_NE(theme, TYPE_FAIRY);
        EXPECT(MfSpeciesMatchesType(MfTrainerEncounterSpecies(SPECIES_CLEFAIRY, key), theme));
        EXPECT_NE(GetSpeciesType(SPECIES_CLEFAIRY, 0), TYPE_FAIRY);
    }

    RestorePhase1Defaults();
}

TEST("MF: trainer remap is identity to S53 when THEMED is off")
{
    enum Species mixed;
    enum Species themed;

    SetTrainerRandomizer(TRUE);
    mixed = MfTrainerEncounterSpecies(SPECIES_GEODUDE, MF_TRAINER_KEY_A);
    EXPECT_EQ(MfTrainerThemeType(MF_TRAINER_KEY_A), TYPE_NONE);

    MfRules_GetSaveRules()->randomThemedTrainers = TRUE;
    themed = MfTrainerEncounterSpecies(SPECIES_GEODUDE, MF_TRAINER_KEY_A);
    EXPECT_NE(MfTrainerThemeType(MF_TRAINER_KEY_A), TYPE_NONE);
    EXPECT(MfSpeciesMatchesType(themed, MfTrainerThemeType(MF_TRAINER_KEY_A)));

    MfRules_GetSaveRules()->randomThemedTrainers = FALSE;
    EXPECT_EQ(MfTrainerEncounterSpecies(SPECIES_GEODUDE, MF_TRAINER_KEY_A), mixed);

    RestorePhase1Defaults();
}
