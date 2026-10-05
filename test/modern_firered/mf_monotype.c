#include "global.h"
#include "mf_monotype.h"
#include "mf_rules.h"
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
