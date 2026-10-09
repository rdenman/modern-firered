#include "global.h"
#include "battle_main.h"
#include "battle_util.h"
#include "mf_random.h"
#include "mf_rules.h"
#include "mf_types.h"
#include "pokemon.h"
#include "test/test.h"
#include "constants/pokemon.h"
#include "constants/species.h"

#define MF_TR_SEED_A 0x54595045u
#define MF_TR_SEED_B 0x45464645u
#define MF_TR_SEEDS  24

static void SetTypeRandomizer(u32 seed, bool8 randomType, bool8 randomEff, bool8 fairy, bool8 improved)
{
    struct ModernRules *save = MfRules_GetSaveRules();

    MfRules_ResetToEmpty(save);
    save->version = MF_RULES_VERSION;
    save->randomizerSeed = seed;
    save->randomizerEnabled = TRUE;
    save->randomType = randomType;
    save->randomTypeEffectiveness = randomEff;
    save->fairyTypes = fairy;
    save->typeEffectiveness = improved;
    save->modernTypes = FALSE;
    save->rulesLocked = TRUE;
}

static void RestorePhase1Defaults(void)
{
    struct ModernRules *save = MfRules_GetSaveRules();

    save->version = 0;
}

static void BuildChartInverse(u32 seed, u8 *inv)
{
    enum Type ordered[NUMBER_OF_MON_TYPES];
    enum Type shuffled[NUMBER_OF_MON_TYPES];
    u8 n;
    u8 i;
    s32 s;

    n = MfFillCombatTypes(ordered, TRUE);
    memcpy(shuffled, ordered, n * sizeof(enum Type));
    for (s = (s32)n - 1; s > 0; s--)
    {
        u16 j = MfRandom_Modulo(seed, MF_RANDOM_CAT_TYPE_EFF, (u32)s, 0, (u16)(s + 1));
        enum Type tmp = shuffled[s];
        shuffled[s] = shuffled[j];
        shuffled[j] = tmp;
    }
    for (i = 0; i < NUMBER_OF_MON_TYPES; i++)
        inv[i] = i;
    for (i = 0; i < n; i++)
        inv[shuffled[i]] = ordered[i];
}

TEST("MF: species types are identity when TYPE random is off")
{
    SetTypeRandomizer(MF_TR_SEED_A, FALSE, FALSE, TRUE, FALSE);
    EXPECT_EQ(GetSpeciesType(SPECIES_CHARIZARD, 0), TYPE_FIRE);
    EXPECT_EQ(GetSpeciesType(SPECIES_CHARIZARD, 1), TYPE_FLYING);
    EXPECT_EQ(GetSpeciesType(SPECIES_CLEFAIRY, 0), TYPE_FAIRY);
    RestorePhase1Defaults();
}

TEST("MF: species type remap is deterministic and uses combat types")
{
    enum Type a0;
    enum Type a1;
    enum Type b0;
    enum Type seedB0;
    enum Type seedB1;

    SetTypeRandomizer(MF_TR_SEED_A, TRUE, FALSE, TRUE, FALSE);
    a0 = GetSpeciesType(SPECIES_CHARIZARD, 0);
    a1 = GetSpeciesType(SPECIES_CHARIZARD, 1);
    b0 = GetSpeciesType(SPECIES_CHARIZARD, 0);
    EXPECT_EQ(a0, b0);
    EXPECT_EQ(a1, GetSpeciesType(SPECIES_CHARIZARD, 1));
    EXPECT(MfTypeIsCombatType(a0, TRUE));
    EXPECT(MfTypeIsCombatType(a1, TRUE));

    SetTypeRandomizer(MF_TR_SEED_B, TRUE, FALSE, TRUE, FALSE);
    seedB0 = GetSpeciesType(SPECIES_CHARIZARD, 0);
    seedB1 = GetSpeciesType(SPECIES_CHARIZARD, 1);
    EXPECT_NE((u32)(a0 | (a1 << 8)), (u32)(seedB0 | (seedB1 << 8)));

    RestorePhase1Defaults();
}

TEST("MF: TYPE random Fairy-off never assigns Fairy")
{
    u32 seed;
    u16 species;

    for (seed = 0; seed < MF_TR_SEEDS; seed++)
    {
        u32 runSeed = MF_TR_SEED_A + seed * 0x9E3779B9u;

        SetTypeRandomizer(runSeed, TRUE, FALSE, FALSE, FALSE);
        for (species = 1; species <= 151; species++)
        {
            EXPECT_NE(GetSpeciesType(species, 0), TYPE_FAIRY);
            EXPECT_NE(GetSpeciesType(species, 1), TYPE_FAIRY);
        }
        RestorePhase1Defaults();
    }
}

TEST("MF: randomized type chart is deterministic and identity when off")
{
    const uq4_12_t (*tableA)[NUMBER_OF_MON_TYPES];
    u32 fingerA;
    u32 fingerB;
    u8 atk;
    u8 def;
    bool8 anyDiff = FALSE;

    SetTypeRandomizer(MF_TR_SEED_A, FALSE, FALSE, TRUE, FALSE);
    EXPECT(MfGetTypeEffectivenessTable() == gTypeEffectivenessTable);
    EXPECT_EQ(GetTypeModifier(TYPE_GROUND, TYPE_ROCK), UQ_4_12(2.0));

    SetTypeRandomizer(MF_TR_SEED_A, FALSE, TRUE, TRUE, FALSE);
    tableA = MfGetTypeEffectivenessTable();
    EXPECT(tableA != gTypeEffectivenessTable);
    fingerA = GetTypeModifier(TYPE_GROUND, TYPE_ROCK)
            ^ ((u32)GetTypeModifier(TYPE_WATER, TYPE_FIRE) << 16);
    EXPECT_EQ(GetTypeModifier(TYPE_GROUND, TYPE_ROCK), tableA[TYPE_GROUND][TYPE_ROCK]);
    SetTypeRandomizer(MF_TR_SEED_A, FALSE, TRUE, TRUE, FALSE);
    EXPECT(MfGetTypeEffectivenessTable() == tableA);
    EXPECT_EQ(GetTypeModifier(TYPE_GROUND, TYPE_ROCK)
            ^ ((u32)GetTypeModifier(TYPE_WATER, TYPE_FIRE) << 16), fingerA);

    for (atk = 0; atk < NUMBER_OF_MON_TYPES; atk++)
    {
        for (def = 0; def < NUMBER_OF_MON_TYPES; def++)
        {
            uq4_12_t v = tableA[atk][def];

            EXPECT(v == UQ_4_12(0.0) || v == UQ_4_12(0.5) || v == UQ_4_12(1.0) || v == UQ_4_12(2.0));
            if (v != gTypeEffectivenessTable[atk][def])
                anyDiff = TRUE;
        }
    }
    EXPECT(anyDiff);

    SetTypeRandomizer(MF_TR_SEED_B, FALSE, TRUE, TRUE, FALSE);
    fingerB = GetTypeModifier(TYPE_GROUND, TYPE_ROCK)
            ^ ((u32)GetTypeModifier(TYPE_WATER, TYPE_FIRE) << 16);
    EXPECT_NE(fingerA, fingerB);

    RestorePhase1Defaults();
}

TEST("MF: randomized chart is a type-axis permutation of the base table")
{
    const uq4_12_t (*table)[NUMBER_OF_MON_TYPES];
    u8 inv[NUMBER_OF_MON_TYPES];
    u8 atk;
    u8 def;

    SetTypeRandomizer(MF_TR_SEED_A, FALSE, TRUE, TRUE, FALSE);
    table = MfGetTypeEffectivenessTable();
    BuildChartInverse(MF_TR_SEED_A, inv);

    for (atk = 0; atk < NUMBER_OF_MON_TYPES; atk++)
    {
        for (def = 0; def < NUMBER_OF_MON_TYPES; def++)
            EXPECT_EQ(table[atk][def], gTypeEffectivenessTable[inv[atk]][inv[def]]);
    }

    RestorePhase1Defaults();
}

TEST("MF: TYPE random composes after modern / Fairy overlays")
{
    struct ModernRules *save;

    SetTypeRandomizer(MF_TR_SEED_A, FALSE, FALSE, TRUE, FALSE);
    save = MfRules_GetSaveRules();
    save->modernTypes = TRUE;
    EXPECT_EQ(GetSpeciesType(SPECIES_ARBOK, 1), TYPE_DARK);
    save->randomType = TRUE;
    EXPECT(MfTypeIsCombatType(GetSpeciesType(SPECIES_ARBOK, 1), TRUE));

    RestorePhase1Defaults();
}
