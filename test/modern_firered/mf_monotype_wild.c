#include "global.h"
#include "mf_encounters.h"
#include "mf_monotype.h"
#include "mf_rules.h"
#include "mf_species_map.h"
#include "test/test.h"
#include "constants/pokemon.h"
#include "constants/species.h"

#define MF_MONO_WILD_TEST_SEED 0x5EED0B0Bu

static const u16 sWildSources[] = {
    SPECIES_PIDGEY, SPECIES_RATTATA, SPECIES_CATERPIE, SPECIES_WEEDLE,
    SPECIES_ZUBAT, SPECIES_GEODUDE, SPECIES_ODDISH, SPECIES_MAGIKARP,
    SPECIES_TENTACOOL, SPECIES_PIDGEOTTO, SPECIES_KADABRA, SPECIES_GOLBAT,
    SPECIES_RAPIDASH, SPECIES_DRAGONAIR, SPECIES_GYARADOS, SPECIES_MUK,
};

static struct ModernRules *SetMonotypeWild(u8 monotype, bool8 similar, bool8 monoWild)
{
    struct ModernRules *save = MfRules_GetSaveRules();

    MfRules_ResetToEmpty(save);
    save->version = MF_RULES_VERSION;
    save->randomizerSeed = MF_MONO_WILD_TEST_SEED;
    save->randomizerEnabled = TRUE;
    save->randomWild = TRUE;
    save->randomSimilar = similar;
    save->randomIncludeLegendaries = FALSE;
    save->monotype = monotype;
    save->randomMonotypeWild = monoWild;
    save->rulesLocked = TRUE;
    return save;
}

static void RestorePhase1Defaults(void)
{
    MfRules_GetSaveRules()->version = 0;
}

static void ExpectAllWildsMatch(u8 type)
{
    u32 i;

    for (i = 0; i < ARRAY_COUNT(sWildSources); i++)
    {
        enum Species dest = MfWildEncounterSpecies(sWildSources[i], 0);

        EXPECT(MfSpeciesMatchesType(dest, type));
        EXPECT(!MfSpeciesMap_IsLegendary(dest));
        EXPECT_EQ(MfWildBattleSpecies(sWildSources[i]), dest);
    }
}

TEST("MF: one type wilds remaps every wild to the monotype")
{
    SetMonotypeWild(TYPE_WATER, FALSE, TRUE);
    EXPECT(MfRules_IsMonotypeWild());
    ExpectAllWildsMatch(TYPE_WATER);

    SetMonotypeWild(TYPE_GHOST, FALSE, TRUE);
    ExpectAllWildsMatch(TYPE_GHOST);
    RestorePhase1Defaults();
}

TEST("MF: one type wilds with balancing keeps evo stage")
{
    u32 i;

    SetMonotypeWild(TYPE_WATER, TRUE, TRUE);
    ExpectAllWildsMatch(TYPE_WATER);
    for (i = 0; i < ARRAY_COUNT(sWildSources); i++)
    {
        enum Species src = sWildSources[i];
        enum Species dest = MfWildEncounterSpecies(src, 0);

        EXPECT_EQ(MfSpeciesMap_GetEvoStage(dest), MfSpeciesMap_GetEvoStage(src));
    }
    RestorePhase1Defaults();
}

TEST("MF: one type wilds is deterministic")
{
    u32 i;

    SetMonotypeWild(TYPE_FIRE, TRUE, TRUE);
    for (i = 0; i < ARRAY_COUNT(sWildSources); i++)
        EXPECT_EQ(MfWildEncounterSpecies(sWildSources[i], 0), MfWildEncounterSpecies(sWildSources[i], 0));
    RestorePhase1Defaults();
}

TEST("MF: one type wilds needs wild remap and monotype")
{
    struct ModernRules *save = SetMonotypeWild(TYPE_WATER, FALSE, TRUE);
    enum Species plain;

    save->monotype = MF_MONOTYPE_OFF;
    EXPECT(!MfRules_MonotypeWildEditable());
    EXPECT(!MfRules_IsMonotypeWild());
    plain = MfSpeciesMapEx(SPECIES_PIDGEY, MF_MONO_WILD_TEST_SEED, MF_RANDOM_CAT_WILD, 0, FALSE, FALSE);
    EXPECT_EQ(MfWildEncounterSpecies(SPECIES_PIDGEY, 0), plain);

    save->monotype = TYPE_WATER;
    save->randomWild = FALSE;
    EXPECT(!MfRules_MonotypeWildEditable());
    EXPECT_EQ(MfWildEncounterSpecies(SPECIES_PIDGEY, 0), SPECIES_PIDGEY);

    save->randomWild = TRUE;
    save->randomMonotypeWild = FALSE;
    EXPECT(MfRules_MonotypeWildEditable());
    EXPECT(!MfRules_IsMonotypeWild());
    EXPECT_EQ(MfWildEncounterSpecies(SPECIES_PIDGEY, 0), plain);
    RestorePhase1Defaults();
}

TEST("MF: randomizer master off clears one type wilds")
{
    struct ModernRules *save = SetMonotypeWild(TYPE_WATER, FALSE, TRUE);

    save->rulesLocked = FALSE;
    EXPECT(MfRules_TrySetBool(MF_RULE_BOOL_RANDOMIZER_ENABLED, FALSE));
    EXPECT_EQ((u32)save->randomMonotypeWild, (u32)FALSE);
    RestorePhase1Defaults();
}

TEST("MF: rules migrate v10 defaults one type wilds to off")
{
    struct ModernRules rules;

    memset(&rules, 0xFF, sizeof(rules));
    rules.version = MF_RULES_VERSION_V10;
    EXPECT_EQ((u32)MfRules_Migrate(&rules), (u32)MF_RULES_MIGRATE_UPGRADED);
    EXPECT_EQ((u32)rules.randomMonotypeWild, (u32)FALSE);
}
