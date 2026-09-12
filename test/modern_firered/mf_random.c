#include "global.h"
#include "mf_random.h"
#include "mf_rules.h"
#include "test/test.h"

#define MF_RANDOM_TEST_SEED 0xC0FFEEu
#define MF_RANDOM_TEST_SEED_B 0xBADC0DEEu

TEST("MF: random hash is deterministic for identical inputs")
{
    u32 a = MfRandom_Hash(MF_RANDOM_TEST_SEED, MF_RANDOM_CAT_WILD, 25, 0);
    u32 b = MfRandom_Hash(MF_RANDOM_TEST_SEED, MF_RANDOM_CAT_WILD, 25, 0);

    EXPECT_EQ(a, b);
    EXPECT_NE(a, 0u); // extremely unlikely with this seed; guards against stub
}

TEST("MF: random hash changes with seed, category, input, or location")
{
    u32 base = MfRandom_Hash(MF_RANDOM_TEST_SEED, MF_RANDOM_CAT_WILD, 25, 10);

    EXPECT_NE(base, MfRandom_Hash(MF_RANDOM_TEST_SEED_B, MF_RANDOM_CAT_WILD, 25, 10));
    EXPECT_NE(base, MfRandom_Hash(MF_RANDOM_TEST_SEED, MF_RANDOM_CAT_TRAINER, 25, 10));
    EXPECT_NE(base, MfRandom_Hash(MF_RANDOM_TEST_SEED, MF_RANDOM_CAT_WILD, 26, 10));
    EXPECT_NE(base, MfRandom_Hash(MF_RANDOM_TEST_SEED, MF_RANDOM_CAT_WILD, 25, 11));
}

TEST("MF: random modulo stays in range and is deterministic")
{
    u16 bound = 151;
    u16 a = MfRandom_Modulo(MF_RANDOM_TEST_SEED, MF_RANDOM_CAT_STATIC, 144, 0, bound);
    u16 b = MfRandom_Modulo(MF_RANDOM_TEST_SEED, MF_RANDOM_CAT_STATIC, 144, 0, bound);

    EXPECT_EQ(a, b);
    EXPECT_LT(a, bound);
    EXPECT_EQ(MfRandom_Modulo(MF_RANDOM_TEST_SEED, MF_RANDOM_CAT_WILD, 1, 0, 0), 0);
    EXPECT_EQ(MfRandom_Modulo(MF_RANDOM_TEST_SEED, MF_RANDOM_CAT_WILD, 1, 0, 1), 0);
}

TEST("MF: random categories produce independent streams")
{
    u32 input = 42;
    u16 loc = 0;
    u32 wild = MfRandom_Hash(MF_RANDOM_TEST_SEED, MF_RANDOM_CAT_WILD, input, loc);
    u32 trainer = MfRandom_Hash(MF_RANDOM_TEST_SEED, MF_RANDOM_CAT_TRAINER, input, loc);
    u32 moves = MfRandom_Hash(MF_RANDOM_TEST_SEED, MF_RANDOM_CAT_MOVES, input, loc);
    u32 ability = MfRandom_Hash(MF_RANDOM_TEST_SEED, MF_RANDOM_CAT_ABILITY, input, loc);
    u32 evo = MfRandom_Hash(MF_RANDOM_TEST_SEED, MF_RANDOM_CAT_EVO, input, loc);
    u32 evoMeth = MfRandom_Hash(MF_RANDOM_TEST_SEED, MF_RANDOM_CAT_EVO_METH, input, loc);
    u32 statik = MfRandom_Hash(MF_RANDOM_TEST_SEED, MF_RANDOM_CAT_STATIC, input, loc);

    EXPECT_NE(wild, trainer);
    EXPECT_NE(wild, moves);
    EXPECT_NE(wild, ability);
    EXPECT_NE(wild, evo);
    EXPECT_NE(wild, evoMeth);
    EXPECT_NE(wild, statik);
    EXPECT_NE(moves, ability);
    EXPECT_NE(evo, evoMeth);
}

TEST("MF: random location key only for wild/trainer when map-based")
{
    EXPECT_EQ(MfRandom_LocationKey(MF_RANDOM_CAT_WILD, 12, TRUE), 12);
    EXPECT_EQ(MfRandom_LocationKey(MF_RANDOM_CAT_TRAINER, 12, TRUE), 12);
    EXPECT_EQ(MfRandom_LocationKey(MF_RANDOM_CAT_WILD, 12, FALSE), 0);
    EXPECT_EQ(MfRandom_LocationKey(MF_RANDOM_CAT_MOVES, 12, TRUE), 0);
    EXPECT_EQ(MfRandom_LocationKey(MF_RANDOM_CAT_ABILITY, 12, TRUE), 0);
    EXPECT_EQ(MfRandom_LocationKey(MF_RANDOM_CAT_EVO, 12, TRUE), 0);
    EXPECT_EQ(MfRandom_LocationKey(MF_RANDOM_CAT_EVO_METH, 12, TRUE), 0);
    EXPECT_EQ(MfRandom_LocationKey(MF_RANDOM_CAT_STATIC, 12, TRUE), 0);
}

TEST("MF: random map-based remaps wild by location, not moves")
{
    u16 route1 = MfRandom_Modulo(MF_RANDOM_TEST_SEED, MF_RANDOM_CAT_WILD, 16,
                                  MfRandom_LocationKey(MF_RANDOM_CAT_WILD, 1, TRUE), 300);
    u16 route2 = MfRandom_Modulo(MF_RANDOM_TEST_SEED, MF_RANDOM_CAT_WILD, 16,
                                  MfRandom_LocationKey(MF_RANDOM_CAT_WILD, 2, TRUE), 300);
    u16 movesA = MfRandom_Modulo(MF_RANDOM_TEST_SEED, MF_RANDOM_CAT_MOVES, 16,
                                  MfRandom_LocationKey(MF_RANDOM_CAT_MOVES, 1, TRUE), 300);
    u16 movesB = MfRandom_Modulo(MF_RANDOM_TEST_SEED, MF_RANDOM_CAT_MOVES, 16,
                                  MfRandom_LocationKey(MF_RANDOM_CAT_MOVES, 2, TRUE), 300);

    EXPECT_NE(route1, route2);
    EXPECT_EQ(movesA, movesB);
}

TEST("MF: random distribution covers most of a small bound")
{
    // 64 inputs into bound 16 should hit every bucket with high probability.
    bool8 seen[16] = {0};
    u32 i;
    u32 distinct = 0;

    for (i = 0; i < 64; i++)
    {
        u16 v = MfRandom_Modulo(MF_RANDOM_TEST_SEED, MF_RANDOM_CAT_WILD, i, 0, 16);
        EXPECT_LT(v, 16);
        seen[v] = TRUE;
    }
    for (i = 0; i < 16; i++)
    {
        if (seen[i])
            distinct++;
    }
    EXPECT_GE(distinct, 14u);
}

TEST("MF: random pack/unpack keeps seed mapping stable (reload)")
{
    struct ModernRules original;
    struct ModernRules restored;
    u8 blob[MF_RULES_SERIALIZED_SIZE];
    u16 before;
    u16 after;

    MfRules_ResetToEmpty(&original);
    original.randomizerSeed = MF_RANDOM_TEST_SEED;
    original.randomMapBased = TRUE;

    before = MfRandom_Modulo(original.randomizerSeed, MF_RANDOM_CAT_WILD, 100,
                             MfRandom_LocationKey(MF_RANDOM_CAT_WILD, 7, original.randomMapBased),
                             400);

    MfRules_Pack(&original, blob);
    memset(&restored, 0, sizeof(restored));
    MfRules_Unpack(blob, &restored);

    EXPECT_EQ(restored.randomizerSeed, MF_RANDOM_TEST_SEED);
    after = MfRandom_Modulo(restored.randomizerSeed, MF_RANDOM_CAT_WILD, 100,
                            MfRandom_LocationKey(MF_RANDOM_CAT_WILD, 7, restored.randomMapBased),
                            400);
    EXPECT_EQ(before, after);
}

TEST("MF: random EnsureSeed assigns once and is stable")
{
    struct ModernRules rules;
    u32 first;

    MfRules_ResetToEmpty(&rules);
    EXPECT_EQ(rules.randomizerSeed, 0u);

    MfRandom_EnsureSeed(&rules);
    first = rules.randomizerSeed;
    EXPECT_NE(first, 0u);

    MfRandom_EnsureSeed(&rules);
    EXPECT_EQ(rules.randomizerSeed, first);
}

TEST("MF: random active wrappers follow save seed and map-based rule")
{
    struct ModernRules *save = MfRules_GetSaveRules();
    u16 withMap;
    u16 withoutMap;

    MfRules_ResetToEmpty(save);
    save->version = MF_RULES_VERSION;
    save->randomizerSeed = MF_RANDOM_TEST_SEED;
    save->randomMapBased = TRUE;

    withMap = MfRandom_ModuloActive(MF_RANDOM_CAT_WILD, 33, 5, 250);
    EXPECT_EQ(withMap, MfRandom_Modulo(MF_RANDOM_TEST_SEED, MF_RANDOM_CAT_WILD, 33, 5, 250));

    save->randomMapBased = FALSE;
    withoutMap = MfRandom_ModuloActive(MF_RANDOM_CAT_WILD, 33, 5, 250);
    EXPECT_EQ(withoutMap, MfRandom_Modulo(MF_RANDOM_TEST_SEED, MF_RANDOM_CAT_WILD, 33, 0, 250));
    EXPECT_NE(withMap, withoutMap);
}
