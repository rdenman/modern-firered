#include "global.h"
#include "event_data.h"
#include "item.h"
#include "mf_encounters.h"
#include "mf_items.h"
#include "mf_party_random.h"
#include "mf_random.h"
#include "mf_rules.h"
#include "mf_species_map.h"
#include "constants/flags.h"
#include "constants/items.h"
#include "constants/species.h"
#include "test/test.h"

#define MF_ITEM_SEED_A 0x4954454Du
#define MF_ITEM_SEED_B 0x53484F50u

static void SetItemRandomizer(u32 seed, bool8 itemsOn)
{
    struct ModernRules *save = MfRules_GetSaveRules();

    MfRules_ResetToEmpty(save);
    save->version = MF_RULES_VERSION;
    save->randomizerSeed = seed;
    save->randomizerEnabled = TRUE;
    save->randomItems = itemsOn;
    save->randomWild = TRUE;
    save->randomTrainer = TRUE;
    save->randomChaos = FALSE;
    save->rulesLocked = TRUE;
    FlagSet(FLAG_SYS_POKEDEX_GET);
    MfRandom_ResetChaosSalt();
}

static void RestorePhase1Defaults(void)
{
    struct ModernRules *save = MfRules_GetSaveRules();

    save->version = 0;
    FlagClear(FLAG_SYS_POKEDEX_GET);
    MfRandom_ResetChaosSalt();
}

TEST("MF: item remap is identity when ITEMS is off")
{
    SetItemRandomizer(MF_ITEM_SEED_A, FALSE);
    EXPECT_EQ(MfItemMapActive(ITEM_POTION), ITEM_POTION);
    EXPECT_EQ(MfItemMapActive(ITEM_NUGGET), ITEM_NUGGET);
    RestorePhase1Defaults();
}

TEST("MF: item remap is deterministic and changes with seed")
{
    enum Item a;
    enum Item b;

    SetItemRandomizer(MF_ITEM_SEED_A, TRUE);
    a = MfItemMapEx(ITEM_POTION, MF_ITEM_SEED_A);
    EXPECT_EQ(a, MfItemMapEx(ITEM_POTION, MF_ITEM_SEED_A));
    EXPECT_EQ(a, MfItemMapActive(ITEM_POTION));
    b = MfItemMapEx(ITEM_POTION, MF_ITEM_SEED_B);
    EXPECT_NE(a, b);
    RestorePhase1Defaults();
}

TEST("MF: key items and HMs stay identity under item random")
{
    u32 seed;

    for (seed = 0; seed < 32; seed++)
    {
        EXPECT_EQ(MfItemMapEx(ITEM_BICYCLE, MF_ITEM_SEED_A + seed), ITEM_BICYCLE);
        EXPECT_EQ(MfItemMapEx(ITEM_TOWN_MAP, MF_ITEM_SEED_A + seed), ITEM_TOWN_MAP);
        EXPECT_EQ(MfItemMapEx(ITEM_SS_TICKET, MF_ITEM_SEED_A + seed), ITEM_SS_TICKET);
        EXPECT_EQ(MfItemMapEx(ITEM_HM_CUT, MF_ITEM_SEED_A + seed), ITEM_HM_CUT);
        EXPECT_EQ(MfItemMapEx(ITEM_HM_SURF, MF_ITEM_SEED_A + seed), ITEM_HM_SURF);
        EXPECT_EQ(MfItemMapEx(ITEM_HM_STRENGTH, MF_ITEM_SEED_A + seed), ITEM_HM_STRENGTH);
        EXPECT_EQ(MfItemMapEx(ITEM_POKE_BALL, MF_ITEM_SEED_A + seed), ITEM_POKE_BALL);
        EXPECT(MfIsItemProgressionProtected(ITEM_BICYCLE));
        EXPECT(MfIsItemProgressionProtected(ITEM_HM_FLASH));
        EXPECT(MfIsItemProgressionProtected(ITEM_POKE_BALL));
        EXPECT(!MfIsItemProgressionProtected(ITEM_POTION));
        EXPECT(!MfIsItemProgressionProtected(ITEM_TM01));
    }
}

TEST("MF: item random pool is Gen 3 plus evo exceptions")
{
    EXPECT(MfIsItemRandomPoolCandidate(ITEM_POTION));
    EXPECT(MfIsItemRandomPoolCandidate(ITEM_LEFTOVERS));
    EXPECT(MfIsItemRandomPoolCandidate(ITEM_SITRUS_BERRY));
    EXPECT(MfIsItemRandomPoolCandidate(ITEM_CHOICE_BAND));
    EXPECT(MfIsItemRandomPoolCandidate(ITEM_TM50));
    EXPECT(MfIsItemRandomPoolCandidate(ITEM_ELECTIRIZER));
    EXPECT(MfIsItemRandomPoolCandidate(ITEM_LINKING_CORD));
    EXPECT(MfIsItemRandomPoolCandidate(ITEM_SHINY_STONE));
    EXPECT(MfIsItemRandomPoolCandidate(ITEM_RAZOR_CLAW));
    EXPECT(!MfIsItemRandomPoolCandidate(ITEM_POKE_BALL)); // protected
    EXPECT(!MfIsItemRandomPoolCandidate(ITEM_HEAL_BALL));
    EXPECT(!MfIsItemRandomPoolCandidate(ITEM_ADAMANT_MINT));
    EXPECT(!MfIsItemRandomPoolCandidate(ITEM_ABILITY_PATCH));
    EXPECT(!MfIsItemRandomPoolCandidate(ITEM_LIFE_ORB));
    EXPECT(!MfIsItemRandomPoolCandidate(ITEM_FOCUS_SASH));
    EXPECT(!MfIsItemRandomPoolCandidate(ITEM_ASSAULT_VEST));
    EXPECT(!MfIsItemRandomPoolCandidate(ITEM_VENUSAURITE));
    EXPECT(!MfIsItemRandomPoolCandidate(ITEM_NORMALIUM_Z));
    EXPECT(!MfIsItemRandomPoolCandidate(ITEM_TM51));
    EXPECT(!MfIsItemRandomPoolCandidate(ITEM_OCCA_BERRY));
    EXPECT(!MfIsItemRandomPoolCandidate(ITEM_SWEET_APPLE));
    EXPECT(!MfIsItemRandomPoolCandidate(ITEM_CHOICE_SPECS));
}

TEST("MF: item remap dest is never a protected progression item")
{
    enum Item dest;
    u32 i;

    SetItemRandomizer(MF_ITEM_SEED_A, TRUE);
    for (i = 0; i < 64; i++)
    {
        dest = MfItemMapEx(ITEM_POTION + (i % 8), MF_ITEM_SEED_A + i);
        EXPECT(MfIsItemRandomPoolCandidate(dest));
        EXPECT(!MfIsItemProgressionProtected(dest));
        EXPECT_NE(dest, ITEM_HM_CUT);
        EXPECT_NE(dest, ITEM_BICYCLE);
        EXPECT_NE(dest, ITEM_POKE_BALL);
        EXPECT_NE(dest, ITEM_LIFE_ORB);
        EXPECT_NE(dest, ITEM_VENUSAURITE);
    }
    RestorePhase1Defaults();
}

TEST("MF: item remap skips until the Pokédex is received")
{
    enum Item withDex;

    SetItemRandomizer(MF_ITEM_SEED_A, TRUE);
    withDex = MfItemMapActive(ITEM_POTION);
    FlagClear(FLAG_SYS_POKEDEX_GET);
    EXPECT_EQ(MfItemMapActive(ITEM_POTION), ITEM_POTION);
    FlagSet(FLAG_SYS_POKEDEX_GET);
    EXPECT_EQ(MfItemMapActive(ITEM_POTION), withDex);
    RestorePhase1Defaults();
}

TEST("MF: shop stock remaps non-protected rows and keeps ITEM_NONE")
{
    static const u16 sStock[] = { ITEM_POTION, ITEM_ANTIDOTE, ITEM_HM_CUT, ITEM_NONE };
    const u16 *out;

    SetItemRandomizer(MF_ITEM_SEED_A, TRUE);
    out = MfMaybeRandomizeShopStock(sStock);
    EXPECT_NE(out[0], ITEM_NONE);
    EXPECT(!MfIsItemProgressionProtected(out[0]));
    EXPECT_EQ(out[2], ITEM_HM_CUT);
    EXPECT_EQ(out[3], ITEM_NONE);
    EXPECT_EQ(out[0], MfItemMapActive(ITEM_POTION));
    RestorePhase1Defaults();
}

TEST("MF: chaos salt does not change the stable wild mapping")
{
    enum Species stableA;
    enum Species stableB;
    enum Species battleA;
    enum Species battleB;

    SetItemRandomizer(MF_ITEM_SEED_A, FALSE);
    MfRules_GetSaveRules()->randomChaos = TRUE;
    MfRandom_ResetChaosSalt();

    stableA = MfWildEncounterSpecies(SPECIES_PIDGEY, 12);
    stableB = MfWildEncounterSpecies(SPECIES_PIDGEY, 12);
    EXPECT_EQ(stableA, stableB);
    EXPECT_EQ(stableA, MfSpeciesMapActive(SPECIES_PIDGEY, MF_RANDOM_CAT_WILD, 12));

    battleA = MfWildBattleSpecies(SPECIES_PIDGEY);
    battleB = MfWildBattleSpecies(SPECIES_PIDGEY);
    EXPECT_NE(battleA, battleB);

    EXPECT_EQ(MfWildEncounterSpecies(SPECIES_PIDGEY, 12), stableA);
    RestorePhase1Defaults();
}

TEST("MF: chaos salt does not change item mappings")
{
    enum Item potion;

    SetItemRandomizer(MF_ITEM_SEED_A, TRUE);
    MfRules_GetSaveRules()->randomChaos = TRUE;
    potion = MfItemMapActive(ITEM_POTION);
    MfRandom_TakeChaosSalt();
    MfRandom_TakeChaosSalt();
    EXPECT_EQ(MfItemMapActive(ITEM_POTION), potion);
    EXPECT_EQ(MfItemMapActive(ITEM_HM_CUT), ITEM_HM_CUT);
    RestorePhase1Defaults();
}

TEST("MF: trainer chaos salt is per-party and does not leak when chaos is off")
{
    enum Species a;
    enum Species b;
    enum Species stable;

    SetItemRandomizer(MF_ITEM_SEED_A, FALSE);
    MfRules_GetSaveRules()->randomTrainer = TRUE;
    MfRules_GetSaveRules()->randomChaos = TRUE;
    MfRandom_ResetChaosSalt();

    MfBeginTrainerParty(1);
    a = MfTrainerEncounterSpecies(SPECIES_PIDGEY, 7);
    EXPECT_EQ(a, MfTrainerEncounterSpecies(SPECIES_PIDGEY, 7));
    MfEndTrainerParty();

    MfBeginTrainerParty(1);
    b = MfTrainerEncounterSpecies(SPECIES_PIDGEY, 7);
    MfEndTrainerParty();
    EXPECT_NE(a, b);

    MfRules_GetSaveRules()->randomChaos = FALSE;
    MfBeginTrainerParty(1);
    stable = MfTrainerEncounterSpecies(SPECIES_PIDGEY, 7);
    EXPECT_EQ(stable, MfTrainerEncounterSpecies(SPECIES_PIDGEY, 7));
    MfEndTrainerParty();
    RestorePhase1Defaults();
}
