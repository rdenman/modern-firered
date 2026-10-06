#include "global.h"
#include "event_data.h"
#include "mf_encounters.h"
#include "mf_rules.h"
#include "mf_species_map.h"
#include "pokemon.h"
#include "test/test.h"
#include "constants/flags.h"
#include "constants/map_groups.h"
#include "constants/species.h"

static void SetEncountersMode(u8 mode)
{
    struct ModernRules *save = MfRules_GetSaveRules();

    MfRules_ApplyDevDefaults(save);
    MfRules_ApplyGamemodePreset(save, MF_GAMEMODE_CUSTOM);
    save->alternateSpawns = mode & 3;
    save->rulesLocked = TRUE;
}

static void RestorePhase1Defaults(void)
{
    struct ModernRules *save = MfRules_GetSaveRules();

    save->version = 0;
}

TEST("MF: encounters vanilla never uses modern tables")
{
    SetEncountersMode(MF_ENCOUNTERS_VANILLA);
    FlagClear(FLAG_SYS_GAME_CLEAR);
    EXPECT(!MfShouldUseModernWildEncounters());

    FlagSet(FLAG_SYS_GAME_CLEAR);
    EXPECT(!MfShouldUseModernWildEncounters());

    FlagClear(FLAG_SYS_GAME_CLEAR);
    RestorePhase1Defaults();
}

TEST("MF: encounters modern always uses modern tables")
{
    SetEncountersMode(MF_ENCOUNTERS_MODERN);
    FlagClear(FLAG_SYS_GAME_CLEAR);
    EXPECT(MfShouldUseModernWildEncounters());

    FlagSet(FLAG_SYS_GAME_CLEAR);
    EXPECT(MfShouldUseModernWildEncounters());

    FlagClear(FLAG_SYS_GAME_CLEAR);
    RestorePhase1Defaults();
}

TEST("MF: encounters postgame flips only after game clear")
{
    SetEncountersMode(MF_ENCOUNTERS_POSTGAME);
    FlagClear(FLAG_SYS_GAME_CLEAR);
    EXPECT(!MfShouldUseModernWildEncounters());

    FlagSet(FLAG_SYS_GAME_CLEAR);
    EXPECT(MfShouldUseModernWildEncounters());

    FlagClear(FLAG_SYS_GAME_CLEAR);
    RestorePhase1Defaults();
}

TEST("MF: encounters active wild headers is non-NULL")
{
    SetEncountersMode(MF_ENCOUNTERS_VANILLA);
    EXPECT(MfGetActiveWildMonHeaders() != NULL);

    SetEncountersMode(MF_ENCOUNTERS_MODERN);
    EXPECT(MfGetActiveWildMonHeaders() != NULL);

    RestorePhase1Defaults();
}

#define MF_WILD_TEST_SEED 0xC0FFEEu

static void SetWildRandomizer(bool8 mapBased)
{
    struct ModernRules *save = MfRules_GetSaveRules();

    MfRules_ResetToEmpty(save);
    save->version = MF_RULES_VERSION;
    save->randomizerSeed = MF_WILD_TEST_SEED;
    save->randomizerEnabled = TRUE;
    save->randomWild = TRUE;
    save->randomStatic = TRUE;
    save->randomMapBased = mapBased;
    save->randomSimilar = FALSE;
    save->randomIncludeLegendaries = FALSE;
    save->rulesLocked = TRUE;
}

TEST("MF: wild table slots remap via active headers")
{
    const struct WildPokemonHeader *headers;
    const struct WildPokemon *slot = NULL;
    u32 i;
    u16 mapsec;
    enum Species src;
    enum Species dest;
    enum Species destAgain;
    struct WildPokemon table[2];
    struct WildPokemon remapped[2];

    SetWildRandomizer(TRUE);
    headers = MfGetActiveWildMonHeaders();
    EXPECT(headers != NULL);

    for (i = 0; headers[i].mapGroup != MAP_GROUP(MAP_UNDEFINED); i++)
    {
        if (headers[i].encounterTypes[0].landMonsInfo != NULL)
        {
            slot = &headers[i].encounterTypes[0].landMonsInfo->wildPokemon[0];
            if (slot->species != SPECIES_NONE)
                break;
        }
    }
    EXPECT(slot != NULL);
    EXPECT_NE(headers[i].mapGroup, MAP_GROUP(MAP_UNDEFINED));

    mapsec = 12;
    src = slot->species;
    dest = MfWildSlotSpecies(slot, mapsec);
    destAgain = MfWildEncounterSpecies(src, mapsec);
    EXPECT_EQ(dest, destAgain);
    EXPECT_EQ(dest, MfSpeciesMapActive(src, MF_RANDOM_CAT_WILD, mapsec));
    EXPECT_EQ(MfWildEncounterSpecies(src, mapsec), dest);

    table[0].minLevel = 3;
    table[0].maxLevel = 5;
    table[0].species = SPECIES_PIDGEY;
    table[1].minLevel = 4;
    table[1].maxLevel = 6;
    table[1].species = SPECIES_RATTATA;
    remapped[0] = table[0];
    remapped[1] = table[1];
    remapped[0].species = MfWildEncounterSpecies(table[0].species, mapsec);
    remapped[1].species = MfWildEncounterSpecies(table[1].species, mapsec);
    EXPECT_EQ(remapped[0].minLevel, table[0].minLevel);
    EXPECT_EQ(remapped[0].maxLevel, table[0].maxLevel);
    EXPECT_EQ(remapped[0].species, MfSpeciesMapActive(SPECIES_PIDGEY, MF_RANDOM_CAT_WILD, mapsec));
    EXPECT_EQ(remapped[1].species, MfSpeciesMapActive(SPECIES_RATTATA, MF_RANDOM_CAT_WILD, mapsec));
    EXPECT_EQ(MfWildEncounterSpecies(SPECIES_PIDGEY, mapsec), remapped[0].species);

    RestorePhase1Defaults();
}

TEST("MF: wild remap is identity when WILD POKéMON is off")
{
    SetWildRandomizer(TRUE);
    MfRules_GetSaveRules()->randomWild = FALSE;
    EXPECT_EQ(MfWildEncounterSpecies(SPECIES_PIDGEY, 3), SPECIES_PIDGEY);
    RestorePhase1Defaults();
}

TEST("MF: Unown wild slots stay Unown")
{
    SetWildRandomizer(TRUE);
    EXPECT_EQ(MfWildEncounterSpecies(SPECIES_UNOWN, 7), SPECIES_UNOWN);
    EXPECT_EQ(MfWildEncounterSpecies(SPECIES_UNOWN_C, 7), SPECIES_UNOWN_C);
    EXPECT_EQ(MfWildEncounterSpecies(SPECIES_UNOWN_QUESTION, 7), SPECIES_UNOWN_QUESTION);
    RestorePhase1Defaults();
}

TEST("MF: static gifts skip while the party is empty")
{
    enum Species dest;

    SetWildRandomizer(FALSE);
    ZeroPlayerPartyMons();
    FlagSet(FLAG_SYS_POKEMON_GET);
    EXPECT_EQ(MfStaticGiftSpecies(SPECIES_SQUIRTLE), SPECIES_SQUIRTLE);

    CreateMon(&gParties[B_TRAINER_PLAYER][0], SPECIES_PIDGEY, 5, 32, OTID_STRUCT_PRESET(0));
    gPartiesCount[B_TRAINER_PLAYER] = 1;
    dest = MfStaticGiftSpecies(SPECIES_OMANYTE);
    EXPECT_EQ(dest, MfSpeciesMapActive(SPECIES_OMANYTE, MF_RANDOM_CAT_STATIC, 0));
    EXPECT_EQ(dest, MfStaticEncounterSpecies(SPECIES_OMANYTE));

    ZeroPlayerPartyMons();
    FlagClear(FLAG_SYS_POKEMON_GET);
    RestorePhase1Defaults();
}

TEST("MF: wild remap survives save/load of the seed")
{
    struct ModernRules original;
    struct ModernRules restored;
    enum Species before;
    enum Species after;

    SetWildRandomizer(TRUE);
    before = MfWildEncounterSpecies(SPECIES_PIDGEY, 4);
    original = *MfRules_GetSaveRules();

    MfRules_ResetToEmpty(MfRules_GetSaveRules());
    *MfRules_GetSaveRules() = original;
    restored = *MfRules_GetSaveRules();
    EXPECT_EQ(restored.randomizerSeed, MF_WILD_TEST_SEED);
    after = MfWildEncounterSpecies(SPECIES_PIDGEY, 4);
    EXPECT_EQ(before, after);

    RestorePhase1Defaults();
}
