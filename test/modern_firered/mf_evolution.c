#include "global.h"
#include "item.h"
#include "mf_evolution.h"
#include "mf_rules.h"
#include "pokemon.h"
#include "constants/items.h"
#include "constants/species.h"
#include "test/test.h"

static struct ModernRules *PrepareRules(void)
{
    struct ModernRules *save = MfRules_GetSaveRules();

    MfRules_ApplyDevDefaults(save);
    save->rulesLocked = TRUE;
    return save;
}

static void RestorePhase1Defaults(void)
{
    struct ModernRules *save = MfRules_GetSaveRules();

    save->version = 0;
}

TEST("MF: evo limit Off never blocks; All always blocks; First blocks pre-evolved")
{
    EXPECT(!MfIsEvolutionBlockedByLimitValue(SPECIES_BULBASAUR, MF_EVO_LIMIT_OFF, FALSE));
    EXPECT(!MfIsEvolutionBlockedByLimitValue(SPECIES_IVYSAUR, MF_EVO_LIMIT_OFF, TRUE));

    EXPECT(MfIsEvolutionBlockedByLimitValue(SPECIES_BULBASAUR, MF_EVO_LIMIT_NONE, FALSE));
    EXPECT(MfIsEvolutionBlockedByLimitValue(SPECIES_IVYSAUR, MF_EVO_LIMIT_NONE, TRUE));
    EXPECT(MfIsEvolutionBlockedByLimitValue(SPECIES_MAGIKARP, MF_EVO_LIMIT_NONE, FALSE));

    EXPECT(!MfIsEvolutionBlockedByLimitValue(SPECIES_BULBASAUR, MF_EVO_LIMIT_FIRST, FALSE));
    EXPECT(MfIsEvolutionBlockedByLimitValue(SPECIES_IVYSAUR, MF_EVO_LIMIT_FIRST, TRUE));
    EXPECT(!MfIsEvolutionBlockedByLimitValue(SPECIES_EEVEE, MF_EVO_LIMIT_FIRST, FALSE));
    EXPECT(MfIsEvolutionBlockedByLimitValue(SPECIES_HAUNTER, MF_EVO_LIMIT_FIRST, TRUE));
}

TEST("MF: pre-evolution walk matches Kanto stages")
{
    EXPECT(!MfSpeciesHasPreEvolution(SPECIES_BULBASAUR));
    EXPECT(MfSpeciesHasPreEvolution(SPECIES_IVYSAUR));
    EXPECT(MfSpeciesHasPreEvolution(SPECIES_VENUSAUR));
    EXPECT(!MfSpeciesHasPreEvolution(SPECIES_PICHU));
    EXPECT(MfSpeciesHasPreEvolution(SPECIES_PIKACHU));
    EXPECT(!MfSpeciesHasPreEvolution(SPECIES_MAGIKARP));
    EXPECT(MfSpeciesHasPreEvolution(SPECIES_GYARADOS));
    EXPECT(!MfSpeciesHasPreEvolution(SPECIES_EEVEE));
    EXPECT(!MfSpeciesHasPreEvolution(SPECIES_GASTLY));
    EXPECT(MfSpeciesHasPreEvolution(SPECIES_HAUNTER));
    EXPECT(!MfSpeciesHasPreEvolution(SPECIES_ONIX));
    EXPECT(!MfSpeciesHasPreEvolution(SPECIES_SCYTHER));
}

TEST("MF: live evo-limit rule blocks First and All")
{
    struct ModernRules *save = PrepareRules();

    save->evoLimit = MF_EVO_LIMIT_OFF;
    EXPECT(!MfIsEvolutionBlockedByLimit(SPECIES_BULBASAUR));
    EXPECT(!MfIsEvolutionBlockedByLimit(SPECIES_IVYSAUR));

    save->evoLimit = MF_EVO_LIMIT_FIRST;
    EXPECT(!MfIsEvolutionBlockedByLimit(SPECIES_BULBASAUR));
    EXPECT(MfIsEvolutionBlockedByLimit(SPECIES_IVYSAUR));
    EXPECT(!MfIsEvolutionBlockedByLimit(SPECIES_EEVEE));
    EXPECT(MfIsEvolutionBlockedByLimit(SPECIES_HAUNTER));

    save->evoLimit = MF_EVO_LIMIT_NONE;
    EXPECT(MfIsEvolutionBlockedByLimit(SPECIES_BULBASAUR));
    EXPECT(MfIsEvolutionBlockedByLimit(SPECIES_MAGIKARP));

    RestorePhase1Defaults();
}

TEST("MF: bag evo items refuse only when the item matches a blocked evo")
{
    struct ModernRules *save = PrepareRules();

    save->evoLimit = MF_EVO_LIMIT_FIRST;
    EXPECT(MfIsEvolutionItemBlockedByLimit(SPECIES_HAUNTER, ITEM_LINKING_CORD));
    EXPECT(!MfIsEvolutionItemBlockedByLimit(SPECIES_GASTLY, ITEM_LINKING_CORD));
    EXPECT(!MfIsEvolutionItemBlockedByLimit(SPECIES_CHARMANDER, ITEM_THUNDER_STONE));
    EXPECT(MfIsEvolutionItemBlockedByLimit(SPECIES_PIKACHU, ITEM_THUNDER_STONE));

    save->evoLimit = MF_EVO_LIMIT_NONE;
    EXPECT(MfIsEvolutionItemBlockedByLimit(SPECIES_PIKACHU, ITEM_THUNDER_STONE));
    EXPECT(!MfIsEvolutionItemBlockedByLimit(SPECIES_CHARMANDER, ITEM_THUNDER_STONE));

    save->evoLimit = MF_EVO_LIMIT_OFF;
    EXPECT(!MfIsEvolutionItemBlockedByLimit(SPECIES_HAUNTER, ITEM_LINKING_CORD));

    RestorePhase1Defaults();
}
