#include "global.h"
#include "test/test.h"

// S08: lock FireRed species-range policy (Gen 1–3 families + cross-gen evos).
// See docs-mf/SPECIES_BASELINE.md and ADR 0008.
// P_* overrides apply only when !TESTING (FireRed ROM). TESTELF keeps full
// expansion species data; assert MF_SPECIES_* stamps there.

TEST("MF: species family range policy is Gen 1-3")
{
    EXPECT_EQ(MF_SPECIES_FAMILIES_MAX_GEN, 3);
#if !TESTING
    EXPECT(P_GEN_1_POKEMON);
    EXPECT(P_GEN_2_POKEMON);
    EXPECT(P_GEN_3_POKEMON);
    EXPECT(!P_GEN_4_POKEMON);
    EXPECT(!P_GEN_5_POKEMON);
    EXPECT(!P_GEN_6_POKEMON);
    EXPECT(!P_GEN_7_POKEMON);
    EXPECT(!P_GEN_8_POKEMON);
    EXPECT(!P_GEN_9_POKEMON);
#endif
}

TEST("MF: cross-gen evolutions and regional forms stay available")
{
    EXPECT(MF_SPECIES_CROSS_EVOS);
    EXPECT(MF_SPECIES_REGIONAL_FORMS);
#if !TESTING
    EXPECT(P_CROSS_GENERATION_EVOS);
    EXPECT(P_GEN_2_CROSS_EVOS);
    EXPECT(P_GEN_4_CROSS_EVOS);
    EXPECT(P_GEN_6_CROSS_EVOS);
    EXPECT(P_GEN_9_CROSS_EVOS);
    EXPECT(P_NEW_EVOS_IN_REGIONAL_DEX);
    EXPECT(P_REGIONAL_FORMS);
    EXPECT(P_ALOLAN_FORMS);
#endif
}

TEST("MF: gimmick form policy for Kanto ROM")
{
    EXPECT(MF_SPECIES_MEGA_EVOLUTIONS);
    EXPECT(MF_SPECIES_PRIMAL_REVERSIONS);
    EXPECT(!MF_SPECIES_ULTRA_BURST_FORMS);
    EXPECT(!MF_SPECIES_GIGANTAMAX_FORMS);
    EXPECT(!MF_SPECIES_TERA_FORMS);
    EXPECT(!MF_SPECIES_FUSION_FORMS);
    EXPECT(!MF_SPECIES_PIKACHU_EXTRA_FORMS);
#if !TESTING
    EXPECT(P_MEGA_EVOLUTIONS);
    EXPECT(P_PRIMAL_REVERSIONS);
    EXPECT(!P_ULTRA_BURST_FORMS);
    EXPECT(!P_GIGANTAMAX_FORMS);
    EXPECT(!P_TERA_FORMS);
    EXPECT(!P_FUSION_FORMS);
    EXPECT(!P_PIKACHU_EXTRA_FORMS);
    EXPECT(!P_GEN_9_MEGA_EVOLUTIONS);
#endif
}

TEST("MF: modern learnsets / move data configs remain GEN_LATEST")
{
    EXPECT(P_LVL_UP_LEARNSETS == GEN_LATEST);
    EXPECT(B_UPDATED_MOVE_DATA >= GEN_LATEST);
    EXPECT(P_UPDATED_ABILITIES >= GEN_LATEST);
}

TEST("MF: Kanto-line cross-evo species keep real dex data")
{
    // Cross-evos are on in both FR ROM and TESTELF (test.h forces them too).
    EXPECT(gSpeciesInfo[SPECIES_MAGNEZONE].baseHP > 0);
    EXPECT(gSpeciesInfo[SPECIES_SYLVEON].baseHP > 0);
    EXPECT(gSpeciesInfo[SPECIES_ANNIHILAPE].baseHP > 0);
    EXPECT(gSpeciesInfo[SPECIES_CROBAT].baseHP > 0);
    EXPECT_EQ(GetSpeciesType(SPECIES_MAGNEZONE, 0), TYPE_ELECTRIC);
    EXPECT_EQ(GetSpeciesType(SPECIES_MAGNEZONE, 1), TYPE_STEEL);
    EXPECT_EQ(GetSpeciesType(SPECIES_SYLVEON, 0), TYPE_FAIRY);
    EXPECT_EQ(GetSpeciesType(SPECIES_ANNIHILAPE, 0), TYPE_FIGHTING);
    EXPECT_EQ(GetSpeciesType(SPECIES_ANNIHILAPE, 1), TYPE_GHOST);
}
