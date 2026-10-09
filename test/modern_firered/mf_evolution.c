#include "global.h"
#include "item.h"
#include "mf_evolution.h"
#include "mf_random.h"
#include "mf_rules.h"
#include "mf_species_map.h"
#include "pokemon.h"
#include "constants/items.h"
#include "constants/pokemon.h"
#include "constants/species.h"
#include "test/test.h"

#define MF_EVO_SEED_A 0xE5011E55u
#define MF_EVO_SEEDS  24

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

static void SetEvoRandomizer(u32 seed, bool8 targets, bool8 methods)
{
    struct ModernRules *save = MfRules_GetSaveRules();

    MfRules_ResetToEmpty(save);
    save->version = MF_RULES_VERSION;
    save->randomizerSeed = seed;
    save->randomizerEnabled = TRUE;
    save->randomEvolution = targets;
    save->randomEvolutionMethods = methods;
    save->randomSimilar = TRUE;
    save->rulesLocked = TRUE;
}

static bool8 AllEvosKantoAchievable(enum Species species)
{
    const struct Evolution *evos = GetSpeciesEvolutions(species);
    u32 i;

    if (evos == NULL)
        return TRUE;
    for (i = 0; evos[i].method != EVOLUTIONS_END; i++)
    {
        if (!MfEvolutionIsKantoAchievable(&evos[i]))
            return FALSE;
    }
    return TRUE;
}

TEST("MF: evo remap is identity when EVOLUTIONS and EVO LINES are off")
{
    const struct Evolution *vanilla;
    const struct Evolution *live;

    vanilla = MfGetVanillaSpeciesEvolutions(SPECIES_CHARMANDER);
    SetEvoRandomizer(MF_EVO_SEED_A, FALSE, FALSE);
    live = GetSpeciesEvolutions(SPECIES_CHARMANDER);
    EXPECT_EQ(live[0].method, vanilla[0].method);
    EXPECT_EQ(live[0].param, vanilla[0].param);
    EXPECT_EQ(live[0].targetSpecies, vanilla[0].targetSpecies);
    RestorePhase1Defaults();
}

TEST("MF: EVOLUTIONS remaps Charmander's target deterministically")
{
    enum Species a;
    enum Species b;

    SetEvoRandomizer(MF_EVO_SEED_A, TRUE, FALSE);
    a = GetSpeciesEvolutions(SPECIES_CHARMANDER)[0].targetSpecies;
    b = GetSpeciesEvolutions(SPECIES_CHARMANDER)[0].targetSpecies;
    EXPECT_EQ(a, b);
    EXPECT_NE(a, SPECIES_NONE);
    EXPECT(MfSpeciesMap_IsCandidate(a));
    EXPECT_EQ(MfSpeciesMap_GetEvoStage(a), MfSpeciesMap_GetEvoStage(SPECIES_CHARMELEON));
    RestorePhase1Defaults();
}

TEST("MF: randomized evo methods are Kanto-achievable across seeds")
{
    u32 seed;
    u32 i;
    static const u16 sCheck[] = {
        SPECIES_CHARMANDER,
        SPECIES_MAGIKARP,
        SPECIES_EEVEE,
        SPECIES_MAGNETON,
        SPECIES_HAUNTER,
        SPECIES_KIRLIA,
        SPECIES_PIKACHU,
        SPECIES_RATTATA,
    };

    for (seed = 0; seed < MF_EVO_SEEDS; seed++)
    {
        u32 runSeed = MF_EVO_SEED_A + seed * 0x9E3779B9u;

        SetEvoRandomizer(runSeed, TRUE, TRUE);
        for (i = 0; i < ARRAY_COUNT(sCheck); i++)
            EXPECT(AllEvosKantoAchievable(sCheck[i]));
        RestorePhase1Defaults();
    }
}

TEST("MF: Magneton location evo is rewritten when the evo randomizer is on")
{
    const struct Evolution *evos;
    u32 i;
    bool8 sawMapsec = FALSE;

    evos = MfGetVanillaSpeciesEvolutions(SPECIES_MAGNETON);
    for (i = 0; evos[i].method != EVOLUTIONS_END; i++)
    {
        if (evos[i].params != NULL && evos[i].params[0].condition == IF_IN_MAPSEC)
            sawMapsec = TRUE;
    }
    EXPECT(sawMapsec);

    SetEvoRandomizer(MF_EVO_SEED_A, TRUE, FALSE);
    EXPECT(AllEvosKantoAchievable(SPECIES_MAGNETON));
    RestorePhase1Defaults();
}

TEST("MF: EVO LINES copies a donor species table")
{
    enum Species donor;
    const struct Evolution *src;
    const struct Evolution *live;

    SetEvoRandomizer(MF_EVO_SEED_A, FALSE, TRUE);
    donor = MfSpeciesMapEx(SPECIES_CHARMANDER, MF_EVO_SEED_A, MF_RANDOM_CAT_EVO_METH, 0, TRUE, FALSE);
    src = MfGetVanillaSpeciesEvolutions(donor);
    live = GetSpeciesEvolutions(SPECIES_CHARMANDER);
    if (src == NULL || src[0].method == EVOLUTIONS_END)
    {
        EXPECT_EQ(live[0].method, EVOLUTIONS_END);
    }
    else
    {
        EXPECT(MfEvolutionIsKantoAchievable(&live[0]));
        EXPECT_NE(live[0].targetSpecies, SPECIES_NONE);
    }
    RestorePhase1Defaults();
}

TEST("MF: evo limit First still uses vanilla pre-evolutions when remaps are on")
{
    struct ModernRules *save = MfRules_GetSaveRules();

    MfRules_ResetToEmpty(save);
    save->version = MF_RULES_VERSION;
    save->randomizerSeed = MF_EVO_SEED_A;
    save->randomizerEnabled = TRUE;
    save->randomEvolution = TRUE;
    save->randomEvolutionMethods = TRUE;
    save->evoLimit = MF_EVO_LIMIT_FIRST;
    save->rulesLocked = TRUE;

    EXPECT(!MfIsEvolutionBlockedByLimit(SPECIES_BULBASAUR));
    EXPECT(MfIsEvolutionBlockedByLimit(SPECIES_IVYSAUR));
    EXPECT(MfIsEvolutionBlockedByLimit(SPECIES_HAUNTER));
    RestorePhase1Defaults();
}
