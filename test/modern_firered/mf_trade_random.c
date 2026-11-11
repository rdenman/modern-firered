#include "global.h"
#include "mf_encounters.h"
#include "mf_rules.h"
#include "mf_species_map.h"
#include "mf_trade_random.h"
#include "trade.h"
#include "wild_encounter.h"
#include "test/test.h"
#include "constants/map_groups.h"
#include "constants/rtc.h"
#include "constants/species.h"

#define MF_TRADE_SEED_A 0x54524131u
#define MF_TRADE_SEED_B 0x54524232u
static void SetTradeRandomizer(u32 seed, bool8 tradesOn, bool8 wildOn, bool8 similar)
{
    struct ModernRules *save = MfRules_GetSaveRules();

    MfRules_ResetToEmpty(save);
    save->version = MF_RULES_VERSION;
    save->randomizerSeed = seed;
    save->randomizerEnabled = TRUE;
    save->randomTrades = tradesOn;
    save->randomWild = wildOn;
    save->randomMapBased = TRUE;
    save->randomSimilar = similar;
    save->rulesLocked = TRUE;
}

static void RestorePhase1Defaults(void)
{
    MfRules_GetSaveRules()->version = 0;
}

static bool32 ListHasSpecies(const struct WildPokemonInfo *info, u32 size, u16 mapsec, enum Species species)
{
    u32 i;

    if (info == NULL || info->wildPokemon == NULL)
        return FALSE;
    for (i = 0; i < size; i++)
    {
        if (MfWildSlotSpecies(&info->wildPokemon[i], mapsec) == species)
            return TRUE;
    }
    return FALSE;
}

static bool32 IsInActiveWildTables(enum Species species)
{
    const struct WildPokemonHeader *headers = MfGetActiveWildMonHeaders();
    u32 i;

    for (i = 0; headers[i].mapGroup != MAP_GROUP(MAP_UNDEFINED); i++)
    {
        const struct WildEncounterTypes *types = &headers[i].encounterTypes[TIME_OF_DAY_DEFAULT];
        u16 mapsec = MfWildHeaderMapsec(&headers[i]);

        if (ListHasSpecies(types->landMonsInfo, NUM_LAND_MONS_ENCOUNTER_SLOTS, mapsec, species)
         || ListHasSpecies(types->waterMonsInfo, NUM_WATER_MONS_ENCOUNTER_SLOTS, mapsec, species)
         || ListHasSpecies(types->rockSmashMonsInfo, NUM_ROCK_SMASH_MONS_ENCOUNTER_SLOTS, mapsec, species)
         || ListHasSpecies(types->fishingMonsInfo, NUM_FISHING_MONS_ENCOUNTER_SLOTS, mapsec, species))
            return TRUE;
    }
    return FALSE;
}

static u32 ResolveAll(enum Species *offered, enum Species *requested)
{
    enum Species vanillaOffered;
    enum Species vanillaRequested;
    u32 n;

    for (n = 0; GetInGameTradeVanillaSpecies(n, &vanillaOffered, &vanillaRequested); n++)
    {
        offered[n] = MfTradeRandom_OfferedSpecies(vanillaOffered, n);
        requested[n] = MfTradeRandom_RequestedSpecies(vanillaRequested, vanillaOffered, n);
    }
    return n;
}

TEST("MF: trades are vanilla when TRADES is off")
{
    enum Species vanillaOffered;
    enum Species vanillaRequested;
    u32 i;

    SetTradeRandomizer(MF_TRADE_SEED_A, FALSE, TRUE, FALSE);
    for (i = 0; GetInGameTradeVanillaSpecies(i, &vanillaOffered, &vanillaRequested); i++)
    {
        EXPECT_EQ(MfTradeRandom_OfferedSpecies(vanillaOffered, i), vanillaOffered);
        EXPECT_EQ(MfTradeRandom_RequestedSpecies(vanillaRequested, vanillaOffered, i), vanillaRequested);
    }
    RestorePhase1Defaults();
}

TEST("MF: trade remaps are stable and seed-dependent")
{
    enum Species offeredA[MF_TRADE_RANDOM_MAX_TRADES];
    enum Species requestedA[MF_TRADE_RANDOM_MAX_TRADES];
    enum Species offeredB[MF_TRADE_RANDOM_MAX_TRADES];
    enum Species requestedB[MF_TRADE_RANDOM_MAX_TRADES];
    u32 n;
    u32 i;
    u32 diffs = 0;

    SetTradeRandomizer(MF_TRADE_SEED_A, TRUE, FALSE, FALSE);
    n = ResolveAll(offeredA, requestedA);
    ResolveAll(offeredB, requestedB);
    for (i = 0; i < n; i++)
    {
        EXPECT_EQ(offeredB[i], offeredA[i]);
        EXPECT_EQ(requestedB[i], requestedA[i]);
    }

    SetTradeRandomizer(MF_TRADE_SEED_B, TRUE, FALSE, FALSE);
    ResolveAll(offeredB, requestedB);
    for (i = 0; i < n; i++)
    {
        if (offeredB[i] != offeredA[i])
            diffs++;
        if (requestedB[i] != requestedA[i])
            diffs++;
    }
    EXPECT_GT(diffs, 0);
    RestorePhase1Defaults();
}

TEST("MF: every traded species is unique across all trades")
{
    enum Species offered[MF_TRADE_RANDOM_MAX_TRADES];
    enum Species requested[MF_TRADE_RANDOM_MAX_TRADES];
    enum Species all[MF_TRADE_RANDOM_MAX_TRADES * 2];
    u32 seed;
    u32 pass;
    u32 n;
    u32 i;
    u32 j;

    for (seed = 0; seed < 4; seed++)
    {
        for (pass = 0; pass < 4; pass++)
        {
            SetTradeRandomizer(MF_TRADE_SEED_A + seed, TRUE, pass & 1, (pass >> 1) & 1);
            n = ResolveAll(offered, requested);
            for (i = 0; i < n; i++)
            {
                all[i * 2] = offered[i];
                all[i * 2 + 1] = requested[i];
            }
            for (i = 0; i < n * 2; i++)
            {
                for (j = i + 1; j < n * 2; j++)
                    EXPECT_NE(all[i], all[j]);
            }
        }
    }
    RestorePhase1Defaults();
}

TEST("MF: trade requests are always in the active wild tables")
{
    enum Species offered[MF_TRADE_RANDOM_MAX_TRADES];
    enum Species requested[MF_TRADE_RANDOM_MAX_TRADES];
    u32 pass;
    u32 n;
    u32 i;

    for (pass = 0; pass < 2; pass++)
    {
        SetTradeRandomizer(MF_TRADE_SEED_A, TRUE, pass == 1, FALSE);
        n = ResolveAll(offered, requested);
        for (i = 0; i < n; i++)
        {
            EXPECT_NE(requested[i], SPECIES_NONE);
            EXPECT(IsInActiveWildTables(requested[i]));
        }
    }
    RestorePhase1Defaults();
}

TEST("MF: trade requests match evo stage under BALANCING")
{
    enum Species offered[MF_TRADE_RANDOM_MAX_TRADES];
    enum Species requested[MF_TRADE_RANDOM_MAX_TRADES];
    enum Species vanillaOffered;
    enum Species vanillaRequested;
    u32 n;
    u32 i;

    SetTradeRandomizer(MF_TRADE_SEED_A, TRUE, TRUE, TRUE);
    n = ResolveAll(offered, requested);
    for (i = 0; i < n; i++)
    {
        GetInGameTradeVanillaSpecies(i, &vanillaOffered, &vanillaRequested);
        EXPECT_EQ(MfSpeciesMap_GetEvoStage(requested[i]), MfSpeciesMap_GetEvoStage(vanillaRequested));
    }
    RestorePhase1Defaults();
}
