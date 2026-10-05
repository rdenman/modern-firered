#include "global.h"
#include "mf_random.h"
#include "mf_rules.h"
#include "mf_species_map.h"
#include "constants/moves.h"
#include "constants/species.h"
#include "test/test.h"

#define MF_MAP_SEED_A 0xC0FFEEu
#define MF_MAP_SEED_B 0xBADC0DEEu
#define MF_MAP_HM_SEEDS 16

static u16 AbsDiff16(u16 a, u16 b)
{
    return (a > b) ? (a - b) : (b - a);
}

TEST("MF: species map pool is S08 range with HM users")
{
    u16 i;
    u16 count;
    bool8 cut = FALSE;
    bool8 surf = FALSE;
    bool8 strength = FALSE;
    bool8 magnezone = FALSE;
    bool8 turtwig = FALSE;

    count = MfSpeciesMap_GetPoolCount();
    EXPECT_GE(count, 350);
    EXPECT_LT(count, 768);

    EXPECT(MfSpeciesMap_IsCandidate(SPECIES_BULBASAUR));
    EXPECT(MfSpeciesMap_IsCandidate(SPECIES_MEWTWO));
    EXPECT(MfSpeciesMap_IsCandidate(SPECIES_TREECKO));
    EXPECT(MfSpeciesMap_IsCandidate(SPECIES_RAYQUAZA));
    EXPECT(MfSpeciesMap_IsCandidate(SPECIES_MAGNEZONE));
    EXPECT(MfSpeciesMap_IsCandidate(SPECIES_SYLVEON));
    EXPECT(!MfSpeciesMap_IsCandidate(SPECIES_TURTWIG));
    EXPECT(!MfSpeciesMap_IsCandidate(SPECIES_EGG));
    EXPECT(!MfSpeciesMap_IsCandidate(SPECIES_NONE));

    for (i = 0; i < count; i++)
    {
        enum Species species = MfSpeciesMap_GetPoolSpecies(i);
        u8 hm = MfSpeciesMap_GetKantoHmMask(species);

        if (species == SPECIES_MAGNEZONE)
            magnezone = TRUE;
        if (species == SPECIES_TURTWIG)
            turtwig = TRUE;
        if (hm & MF_SPECIES_MAP_HM_CUT)
            cut = TRUE;
        if (hm & MF_SPECIES_MAP_HM_SURF)
            surf = TRUE;
        if (hm & MF_SPECIES_MAP_HM_STRENGTH)
            strength = TRUE;
    }

    EXPECT(magnezone);
    EXPECT(!turtwig);
    EXPECT(cut);
    EXPECT(surf);
    EXPECT(strength);
}

TEST("MF: species map evo stages match Kanto lines")
{
    EXPECT_EQ(MfSpeciesMap_GetEvoStage(SPECIES_BULBASAUR), MF_SPECIES_MAP_STAGE_0);
    EXPECT_EQ(MfSpeciesMap_GetEvoStage(SPECIES_IVYSAUR), MF_SPECIES_MAP_STAGE_1);
    EXPECT_EQ(MfSpeciesMap_GetEvoStage(SPECIES_VENUSAUR), MF_SPECIES_MAP_STAGE_2);
    EXPECT_EQ(MfSpeciesMap_GetEvoStage(SPECIES_MAGIKARP), MF_SPECIES_MAP_STAGE_0);
    EXPECT_EQ(MfSpeciesMap_GetEvoStage(SPECIES_GYARADOS), MF_SPECIES_MAP_STAGE_2);
    EXPECT_EQ(MfSpeciesMap_GetEvoStage(SPECIES_TAUROS), MF_SPECIES_MAP_STAGE_0);
    EXPECT_EQ(MfSpeciesMap_GetEvoStage(SPECIES_MEWTWO), MF_SPECIES_MAP_STAGE_LEGENDARY);
    EXPECT_EQ(MfSpeciesMap_GetEvoStage(SPECIES_ARTICUNO), MF_SPECIES_MAP_STAGE_LEGENDARY);
    EXPECT(MfSpeciesMap_IsLegendary(SPECIES_MEWTWO));
    EXPECT(!MfSpeciesMap_IsLegendary(SPECIES_PIDGEY));
}

TEST("MF: species map is deterministic for identical inputs")
{
    enum Species a = MfSpeciesMapEx(SPECIES_PIDGEY, MF_MAP_SEED_A, MF_RANDOM_CAT_WILD, 0, FALSE, FALSE);
    enum Species b = MfSpeciesMapEx(SPECIES_PIDGEY, MF_MAP_SEED_A, MF_RANDOM_CAT_WILD, 0, FALSE, FALSE);

    EXPECT_EQ(a, b);
    EXPECT(MfSpeciesMap_IsCandidate(a));
}

TEST("MF: species map changes with seed, category, input, or location")
{
    enum Species base = MfSpeciesMapEx(SPECIES_PIDGEY, MF_MAP_SEED_A, MF_RANDOM_CAT_WILD, 10, FALSE, FALSE);

    EXPECT_NE(base, MfSpeciesMapEx(SPECIES_PIDGEY, MF_MAP_SEED_B, MF_RANDOM_CAT_WILD, 10, FALSE, FALSE));
    EXPECT_NE(base, MfSpeciesMapEx(SPECIES_PIDGEY, MF_MAP_SEED_A, MF_RANDOM_CAT_TRAINER, 10, FALSE, FALSE));
    EXPECT_NE(base, MfSpeciesMapEx(SPECIES_RATTATA, MF_MAP_SEED_A, MF_RANDOM_CAT_WILD, 10, FALSE, FALSE));
    EXPECT_NE(base, MfSpeciesMapEx(SPECIES_PIDGEY, MF_MAP_SEED_A, MF_RANDOM_CAT_WILD, 11, FALSE, FALSE));
}

TEST("MF: species map balancing keeps evo stage and similar BST")
{
    u32 i;
    u16 srcBst = MfSpeciesMap_GetRawBst(SPECIES_PIDGEY);
    u8 srcStage = MfSpeciesMap_GetEvoStage(SPECIES_PIDGEY);

    EXPECT_EQ(srcStage, MF_SPECIES_MAP_STAGE_0);

    for (i = 0; i < 24; i++)
    {
        enum Species dest = MfSpeciesMapEx(SPECIES_PIDGEY,
                                           MF_MAP_SEED_A + i,
                                           MF_RANDOM_CAT_WILD,
                                           0,
                                           TRUE,
                                           FALSE);
        EXPECT_EQ(MfSpeciesMap_GetEvoStage(dest), srcStage);
        EXPECT(!MfSpeciesMap_IsLegendary(dest));
        EXPECT_LE(AbsDiff16(MfSpeciesMap_GetRawBst(dest), srcBst), 200);
        EXPECT_NE(dest, SPECIES_GYARADOS);
        EXPECT_NE(dest, SPECIES_DRAGONITE);
    }

    EXPECT_EQ(MfSpeciesMap_GetEvoStage(MfSpeciesMapEx(SPECIES_IVYSAUR, MF_MAP_SEED_A, MF_RANDOM_CAT_STATIC, 0, TRUE, FALSE)),
              MF_SPECIES_MAP_STAGE_1);
    EXPECT_EQ(MfSpeciesMap_GetEvoStage(MfSpeciesMapEx(SPECIES_VENUSAUR, MF_MAP_SEED_A, MF_RANDOM_CAT_STATIC, 0, TRUE, FALSE)),
              MF_SPECIES_MAP_STAGE_2);
}

TEST("MF: species map excludes legendaries unless opted in")
{
    u16 i;
    u16 count = MfSpeciesMap_GetPoolCount();

    EXPECT_EQ(MfSpeciesMapEx(SPECIES_MEWTWO, MF_MAP_SEED_A, MF_RANDOM_CAT_STATIC, 0, FALSE, FALSE), SPECIES_MEWTWO);
    EXPECT_EQ(MfSpeciesMapEx(SPECIES_ARTICUNO, MF_MAP_SEED_A, MF_RANDOM_CAT_WILD, 0, TRUE, FALSE), SPECIES_ARTICUNO);
    EXPECT_NE(MfSpeciesMapEx(SPECIES_MEWTWO, MF_MAP_SEED_A, MF_RANDOM_CAT_STATIC, 0, FALSE, TRUE), SPECIES_NONE);

    for (i = 0; i < count; i++)
    {
        enum Species src = MfSpeciesMap_GetPoolSpecies(i);
        enum Species dest;

        if (MfSpeciesMap_IsLegendary(src))
            continue;
        dest = MfSpeciesMapEx(src, MF_MAP_SEED_A, MF_RANDOM_CAT_WILD, 3, FALSE, FALSE);
        EXPECT(!MfSpeciesMap_IsLegendary(dest));
        dest = MfSpeciesMapEx(src, MF_MAP_SEED_A, MF_RANDOM_CAT_WILD, 3, TRUE, FALSE);
        EXPECT(!MfSpeciesMap_IsLegendary(dest));
    }
}

TEST("MF: species map preserves Cut Surf Strength across many seeds")
{
    u32 seed;
    u16 i;
    u16 count = MfSpeciesMap_GetPoolCount();
    bool8 similar;

    for (similar = FALSE; similar <= TRUE; similar++)
    for (seed = 0; seed < MF_MAP_HM_SEEDS; seed++)
    {
        bool8 cut = FALSE;
        bool8 surf = FALSE;
        bool8 strength = FALSE;
        u32 salt = 0x10001u * (seed + 1) + (similar ? 0xABCDu : 0);

        for (i = 0; i < count; i++)
        {
            enum Species src = MfSpeciesMap_GetPoolSpecies(i);
            enum Species dest = MfSpeciesMapEx(src, salt, MF_RANDOM_CAT_WILD, 0, similar, FALSE);
            u8 srcHm = MfSpeciesMap_GetKantoHmMask(src);
            u8 destHm = MfSpeciesMap_GetKantoHmMask(dest);

            EXPECT_EQ(destHm & srcHm, srcHm);
            if (destHm & MF_SPECIES_MAP_HM_CUT)
                cut = TRUE;
            if (destHm & MF_SPECIES_MAP_HM_SURF)
                surf = TRUE;
            if (destHm & MF_SPECIES_MAP_HM_STRENGTH)
                strength = TRUE;
        }
        EXPECT(cut);
        EXPECT(surf);
        EXPECT(strength);
    }
}

TEST("MF: species map active wrappers honor flags and map-based keys")
{
    struct ModernRules *save = MfRules_GetSaveRules();
    enum Species withMap;
    enum Species withoutMap;
    enum Species off;

    MfRules_ResetToEmpty(save);
    save->version = MF_RULES_VERSION;
    save->randomizerSeed = MF_MAP_SEED_A;
    save->randomizerEnabled = TRUE;
    save->randomWild = TRUE;
    save->randomMapBased = TRUE;
    save->randomSimilar = FALSE;
    save->randomIncludeLegendaries = FALSE;

    withMap = MfSpeciesMapActive(SPECIES_PIDGEY, MF_RANDOM_CAT_WILD, 5);
    EXPECT_EQ(withMap, MfSpeciesMapEx(SPECIES_PIDGEY, MF_MAP_SEED_A, MF_RANDOM_CAT_WILD, 5, FALSE, FALSE));

    save->randomMapBased = FALSE;
    withoutMap = MfSpeciesMapActive(SPECIES_PIDGEY, MF_RANDOM_CAT_WILD, 5);
    EXPECT_EQ(withoutMap, MfSpeciesMapEx(SPECIES_PIDGEY, MF_MAP_SEED_A, MF_RANDOM_CAT_WILD, 0, FALSE, FALSE));
    EXPECT_NE(withMap, withoutMap);

    off = MfSpeciesMapActive(SPECIES_PIDGEY, MF_RANDOM_CAT_TRAINER, 5);
    EXPECT_EQ(off, SPECIES_PIDGEY);

    save->randomStatic = TRUE;
    EXPECT_NE(MfSpeciesMapActive(SPECIES_PIDGEY, MF_RANDOM_CAT_STATIC, 9), SPECIES_NONE);

    save->version = 0;
}
