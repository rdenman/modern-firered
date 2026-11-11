#include "global.h"
#include "mf_monotype.h"
#include "mf_rules.h"
#include "mf_species_map.h"
#include "new_game.h"
#include "pokemon.h"
#include "constants/pokemon.h"
#include "constants/species.h"
#include "test/test.h"

static struct ModernRules *PrepareRules(void)
{
    struct ModernRules *save = MfRules_GetSaveRules();

    MfRules_ApplyDevDefaults(save);
    save->rulesLocked = TRUE;
    save->fairyTypes = TRUE;
    save->modernTypes = FALSE;
    return save;
}

static void RestorePhase1Defaults(void)
{
    struct ModernRules *save = MfRules_GetSaveRules();

    save->version = 0;
}

TEST("MF: monotype off allows every species")
{
    EXPECT(MfIsMonotypePartyLegalValue(SPECIES_CHARMANDER, MF_MONOTYPE_OFF));
    EXPECT(MfIsMonotypePartyLegalValue(SPECIES_PIDGEY, MF_MONOTYPE_OFF));
    EXPECT(MfIsMonotypePartyLegalValue(SPECIES_NONE, TYPE_FIRE));
}

TEST("MF: monotype dual-type matches either slot")
{
    EXPECT(MfSpeciesMatchesType(SPECIES_CHARIZARD, TYPE_FIRE));
    EXPECT(MfSpeciesMatchesType(SPECIES_CHARIZARD, TYPE_FLYING));
    EXPECT(!MfSpeciesMatchesType(SPECIES_CHARIZARD, TYPE_WATER));

    EXPECT(MfIsMonotypePartyLegalValue(SPECIES_CHARIZARD, TYPE_FIRE));
    EXPECT(MfIsMonotypePartyLegalValue(SPECIES_CHARIZARD, TYPE_FLYING));
    EXPECT(!MfIsMonotypePartyLegalValue(SPECIES_CHARIZARD, TYPE_WATER));
    EXPECT(MfIsMonotypePartyLegalValue(SPECIES_SQUIRTLE, TYPE_WATER));
    EXPECT(!MfIsMonotypePartyLegalValue(SPECIES_SQUIRTLE, TYPE_FIRE));
}

TEST("MF: monotype uses Fairy and modern-type rules")
{
    struct ModernRules *save = PrepareRules();

    save->fairyTypes = TRUE;
    EXPECT(MfSpeciesMatchesType(SPECIES_CLEFAIRY, TYPE_FAIRY));
    EXPECT(!MfSpeciesMatchesType(SPECIES_CLEFAIRY, TYPE_NORMAL));
    EXPECT(MfIsMonotypePartyLegalValue(SPECIES_CLEFAIRY, TYPE_FAIRY));
    EXPECT(!MfIsMonotypePartyLegalValue(SPECIES_CLEFAIRY, TYPE_NORMAL));

    save->fairyTypes = FALSE;
    EXPECT(!MfSpeciesMatchesType(SPECIES_CLEFAIRY, TYPE_FAIRY));
    EXPECT(MfSpeciesMatchesType(SPECIES_CLEFAIRY, TYPE_NORMAL));
    EXPECT(!MfIsMonotypePartyLegalValue(SPECIES_CLEFAIRY, TYPE_FAIRY));
    EXPECT(MfIsMonotypePartyLegalValue(SPECIES_CLEFAIRY, TYPE_NORMAL));

    save->fairyTypes = TRUE;
    save->modernTypes = FALSE;
    EXPECT(!MfSpeciesMatchesType(SPECIES_ARBOK, TYPE_DARK));
    save->modernTypes = TRUE;
    EXPECT(MfSpeciesMatchesType(SPECIES_ARBOK, TYPE_DARK));
    EXPECT(MfIsMonotypePartyLegalValue(SPECIES_ARBOK, TYPE_DARK));
    EXPECT(MfIsMonotypePartyLegalValue(SPECIES_ARBOK, TYPE_POISON));

    RestorePhase1Defaults();
}

TEST("MF: monotype Oak starter is a legal evo-0 of that type")
{
    struct ModernRules *save = PrepareRules();
    enum Species resolved;

    SetTrainerId(0, gSaveBlock2Ptr->playerTrainerId);
    save->monotype = TYPE_FIRE;
    resolved = MfResolveMonotypeStarterSpecies(SPECIES_SQUIRTLE, 1);
    EXPECT_NE(resolved, SPECIES_SQUIRTLE);
    EXPECT(MfIsMonotypePartyLegalValue(resolved, TYPE_FIRE));

    save->monotype = TYPE_WATER;
    resolved = MfFindNthMonotypeStarter(TYPE_WATER, 0);
    EXPECT_NE(resolved, SPECIES_NONE);
    EXPECT(MfIsMonotypePartyLegalValue(resolved, TYPE_WATER));

    save->monotype = MF_MONOTYPE_OFF;
    EXPECT_EQ(MfResolveMonotypeStarterSpecies(SPECIES_BULBASAUR, 0), SPECIES_BULBASAUR);

    RestorePhase1Defaults();
}

TEST("MF: monotype Oak trio is three unique species")
{
    struct ModernRules *save = PrepareRules();
    enum Species a, b, c;
    enum Species a2, b2, c2;

    SetTrainerId(0, gSaveBlock2Ptr->playerTrainerId);
    save->monotype = TYPE_FIRE;
    a = MfResolveMonotypeStarterSpecies(SPECIES_BULBASAUR, 0);
    b = MfResolveMonotypeStarterSpecies(SPECIES_SQUIRTLE, 1);
    c = MfResolveMonotypeStarterSpecies(SPECIES_CHARMANDER, 2);
    EXPECT_NE(a, b);
    EXPECT_NE(a, c);
    EXPECT_NE(b, c);
    EXPECT(MfIsMonotypePartyLegalValue(a, TYPE_FIRE));
    EXPECT(MfIsMonotypePartyLegalValue(b, TYPE_FIRE));
    EXPECT(MfIsMonotypePartyLegalValue(c, TYPE_FIRE));

    save->monotype = TYPE_ELECTRIC;
    a = MfResolveMonotypeStarterSpecies(SPECIES_BULBASAUR, 0);
    b = MfResolveMonotypeStarterSpecies(SPECIES_SQUIRTLE, 1);
    c = MfResolveMonotypeStarterSpecies(SPECIES_CHARMANDER, 2);
    EXPECT_NE(a, b);
    EXPECT_NE(a, c);
    EXPECT_NE(b, c);
    EXPECT(MfIsMonotypePartyLegalValue(a, TYPE_ELECTRIC));
    EXPECT(MfIsMonotypePartyLegalValue(b, TYPE_ELECTRIC));
    EXPECT(MfIsMonotypePartyLegalValue(c, TYPE_ELECTRIC));

    a2 = MfFindNthMonotypeStarter(TYPE_ELECTRIC, 0);
    b2 = MfFindNthMonotypeStarter(TYPE_ELECTRIC, 1);
    c2 = MfFindNthMonotypeStarter(TYPE_ELECTRIC, 2);
    EXPECT_EQ(a, a2);
    EXPECT_EQ(b, b2);
    EXPECT_EQ(c, c2);

    RestorePhase1Defaults();
}

TEST("MF: monotype Oak trio changes with OT ID")
{
    struct ModernRules *save = PrepareRules();
    enum Species a0, a1, a2;
    enum Species b0, b1, b2;

    save->monotype = TYPE_BUG;
    SetTrainerId(1, gSaveBlock2Ptr->playerTrainerId);
    a0 = MfFindNthMonotypeStarter(TYPE_BUG, 0);
    a1 = MfFindNthMonotypeStarter(TYPE_BUG, 1);
    a2 = MfFindNthMonotypeStarter(TYPE_BUG, 2);

    SetTrainerId(2, gSaveBlock2Ptr->playerTrainerId);
    b0 = MfFindNthMonotypeStarter(TYPE_BUG, 0);
    b1 = MfFindNthMonotypeStarter(TYPE_BUG, 1);
    b2 = MfFindNthMonotypeStarter(TYPE_BUG, 2);

    EXPECT(a0 != b0 || a1 != b1 || a2 != b2);
    EXPECT(MfIsMonotypePartyLegalValue(a0, TYPE_BUG));
    EXPECT(MfIsMonotypePartyLegalValue(b0, TYPE_BUG));

    RestorePhase1Defaults();
}

TEST("MF: monotype Oak Bug trio is not pinned to Caterpie Weedle Paras")
{
    struct ModernRules *save = PrepareRules();
    u32 ot;
    u32 classic = 0;
    bool8 sawBeyondEarlyBugs = FALSE;

    save->monotype = TYPE_BUG;
    for (ot = 0; ot < 48; ot++)
    {
        enum Species a, b, c;
        u32 mask;

        SetTrainerId(ot, gSaveBlock2Ptr->playerTrainerId);
        a = MfFindNthMonotypeStarter(TYPE_BUG, 0);
        b = MfFindNthMonotypeStarter(TYPE_BUG, 1);
        c = MfFindNthMonotypeStarter(TYPE_BUG, 2);
        mask = 0;
        if (a == SPECIES_CATERPIE || b == SPECIES_CATERPIE || c == SPECIES_CATERPIE)
            mask |= 1;
        if (a == SPECIES_WEEDLE || b == SPECIES_WEEDLE || c == SPECIES_WEEDLE)
            mask |= 2;
        if (a == SPECIES_PARAS || b == SPECIES_PARAS || c == SPECIES_PARAS)
            mask |= 4;
        if (mask == 7)
            classic++;
        if (a != SPECIES_CATERPIE && a != SPECIES_WEEDLE && a != SPECIES_PARAS && a != SPECIES_VENONAT)
            sawBeyondEarlyBugs = TRUE;
        if (b != SPECIES_CATERPIE && b != SPECIES_WEEDLE && b != SPECIES_PARAS && b != SPECIES_VENONAT)
            sawBeyondEarlyBugs = TRUE;
        if (c != SPECIES_CATERPIE && c != SPECIES_WEEDLE && c != SPECIES_PARAS && c != SPECIES_VENONAT)
            sawBeyondEarlyBugs = TRUE;
    }

    EXPECT_LT(classic, 3);
    EXPECT(sawBeyondEarlyBugs);

    RestorePhase1Defaults();
}

TEST("MF: monotype Oak trio uses full pool when randomized without balancing")
{
    struct ModernRules *save = PrepareRules();
    u32 ot;
    bool8 sawEvolved = FALSE;

    save->monotype = TYPE_WATER;
    save->randomizerEnabled = TRUE;
    save->randomStarter = TRUE;
    save->randomSimilar = FALSE;
    save->randomChaos = FALSE;
    save->randomIncludeLegendaries = FALSE;
    for (ot = 0; ot < 32; ot++)
    {
        u8 slot;

        SetTrainerId(ot, gSaveBlock2Ptr->playerTrainerId);
        for (slot = 0; slot < 3; slot++)
        {
            enum Species s = MfFindNthMonotypeStarter(TYPE_WATER, slot);

            EXPECT(MfIsMonotypePartyLegalValue(s, TYPE_WATER));
            EXPECT(!MfSpeciesMap_IsLegendary(s));
            if (MfSpeciesMap_GetEvoStage(s) != MF_SPECIES_MAP_STAGE_0)
                sawEvolved = TRUE;
        }
    }
    EXPECT(sawEvolved);

    save->randomSimilar = TRUE;
    for (ot = 0; ot < 8; ot++)
    {
        SetTrainerId(ot, gSaveBlock2Ptr->playerTrainerId);
        EXPECT_EQ(MfSpeciesMap_GetEvoStage(MfFindNthMonotypeStarter(TYPE_WATER, 0)), MF_SPECIES_MAP_STAGE_0);
    }

    RestorePhase1Defaults();
}

TEST("MF: live monotype rule matches accessor")
{
    struct ModernRules *save = PrepareRules();

    save->monotype = MF_MONOTYPE_OFF;
    EXPECT(MfIsMonotypePartyLegal(SPECIES_PIDGEY));

    save->monotype = TYPE_GRASS;
    EXPECT(MfIsMonotypePartyLegal(SPECIES_BULBASAUR));
    EXPECT(!MfIsMonotypePartyLegal(SPECIES_CHARMANDER));

    RestorePhase1Defaults();
}
