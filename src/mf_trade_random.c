#include "global.h"
#include "trade.h"
#include "wild_encounter.h"
#include "mf_encounters.h"
#include "mf_random.h"
#include "mf_rules.h"
#include "mf_species_map.h"
#include "mf_trade_random.h"
#include "constants/maps.h"
#include "constants/rtc.h"
#include "constants/species.h"

// Each draw remaps one species; caps keep a talk-to-NPC call cheap.
#define MF_TRADE_OFFER_ATTEMPTS   16
#define MF_TRADE_REQUEST_ATTEMPTS 64

struct MfTradeRandomCache
{
    u32 seed;
    const struct WildPokemonHeader *headers;
    u8 flags;
    u8 count;
    bool8 valid;
    u16 offered[MF_TRADE_RANDOM_MAX_TRADES];
    u16 requested[MF_TRADE_RANDOM_MAX_TRADES];
};

static EWRAM_DATA struct MfTradeRandomCache sCache = {0};

bool8 MfTradeRandom_IsActive(void)
{
#if MF_RANDOMIZER
    return MfRules_GetActiveRules()->randomTrades;
#else
    return FALSE;
#endif
}

static u32 CountListSlots(const struct WildPokemonInfo *info, u32 size)
{
    return (info != NULL && info->wildPokemon != NULL) ? size : 0;
}

static u32 CountHeaderSlots(const struct WildEncounterTypes *types)
{
    return CountListSlots(types->landMonsInfo, NUM_LAND_MONS_ENCOUNTER_SLOTS)
         + CountListSlots(types->waterMonsInfo, NUM_WATER_MONS_ENCOUNTER_SLOTS)
         + CountListSlots(types->rockSmashMonsInfo, NUM_ROCK_SMASH_MONS_ENCOUNTER_SLOTS)
         + CountListSlots(types->fishingMonsInfo, NUM_FISHING_MONS_ENCOUNTER_SLOTS);
}

static u32 CountWildSlots(const struct WildPokemonHeader *headers)
{
    u32 i;
    u32 total = 0;

    for (i = 0; headers[i].mapGroup != MAP_GROUP(MAP_UNDEFINED); i++)
        total += CountHeaderSlots(&headers[i].encounterTypes[TIME_OF_DAY_DEFAULT]);
    return total;
}

static const struct WildPokemon *TakeListSlot(const struct WildPokemonInfo *info, u32 size, u32 *index)
{
    u32 count = CountListSlots(info, size);

    if (*index < count)
        return &info->wildPokemon[*index];
    *index -= count;
    return NULL;
}

// Remapped species at a flat slot index across every active wild table.
static enum Species WildSlotSpeciesAt(const struct WildPokemonHeader *headers, u32 index)
{
    u32 i;

    for (i = 0; headers[i].mapGroup != MAP_GROUP(MAP_UNDEFINED); i++)
    {
        const struct WildEncounterTypes *types = &headers[i].encounterTypes[TIME_OF_DAY_DEFAULT];
        const struct WildPokemon *slot;

        if (index >= CountHeaderSlots(types))
        {
            index -= CountHeaderSlots(types);
            continue;
        }
        slot = TakeListSlot(types->landMonsInfo, NUM_LAND_MONS_ENCOUNTER_SLOTS, &index);
        if (slot == NULL)
            slot = TakeListSlot(types->waterMonsInfo, NUM_WATER_MONS_ENCOUNTER_SLOTS, &index);
        if (slot == NULL)
            slot = TakeListSlot(types->rockSmashMonsInfo, NUM_ROCK_SMASH_MONS_ENCOUNTER_SLOTS, &index);
        if (slot == NULL)
            slot = TakeListSlot(types->fishingMonsInfo, NUM_FISHING_MONS_ENCOUNTER_SLOTS, &index);
        return MfWildSlotSpecies(slot, MfWildHeaderMapsec(&headers[i]));
    }
    return SPECIES_NONE;
}

static bool8 IsUnownSpecies(enum Species species)
{
    return species == SPECIES_UNOWN
        || (species >= SPECIES_UNOWN_B && species <= SPECIES_UNOWN_QUESTION);
}

static bool8 IsUsed(enum Species species)
{
    u32 i;

    for (i = 0; i < sCache.count; i++)
    {
        if (sCache.offered[i] == species || sCache.requested[i] == species)
            return TRUE;
    }
    return FALSE;
}

static enum Species PickOffered(const struct ModernRules *rules, enum Species vanilla, u8 tradeId)
{
    bool8 similar = rules->randomSimilar && !rules->randomChaos;
    bool8 legs = rules->randomIncludeLegendaries || rules->randomChaos;
    u16 poolCount;
    u16 start;
    u32 attempt;
    u32 i;

    for (attempt = 0; attempt < MF_TRADE_OFFER_ATTEMPTS; attempt++)
    {
        enum Species species = MfSpeciesMapEx(vanilla, rules->randomizerSeed, MF_RANDOM_CAT_TRADE,
                                              (tradeId + 1) | (attempt << 8), similar, legs);

        if (!IsUsed(species))
            return species;
    }

    poolCount = MfSpeciesMap_GetPoolCount();
    start = MfRandom_Modulo(rules->randomizerSeed, MF_RANDOM_CAT_TRADE, tradeId, 0x4000, poolCount);
    for (i = 0; i < poolCount; i++)
    {
        enum Species species = MfSpeciesMap_GetPoolSpecies((start + i) % poolCount);

        if ((legs || !MfSpeciesMap_IsLegendary(species)) && !IsUsed(species))
            return species;
    }
    return vanilla;
}

static bool8 IsValidRequest(enum Species species, enum Species offered)
{
    return species != SPECIES_NONE && species != offered && !IsUnownSpecies(species) && !IsUsed(species);
}

static enum Species PickRequested(const struct ModernRules *rules, const struct WildPokemonHeader *headers, u32 total,
                                  enum Species vanilla, enum Species offered, u8 tradeId)
{
    enum Species fallback = SPECIES_NONE;
    bool8 similar = rules->randomSimilar && !rules->randomChaos;
    u8 stage = MfSpeciesMap_GetEvoStage(vanilla);
    u32 attempt;
    u32 start = 0;
    u32 i;

    if (total == 0)
        return vanilla;

    for (attempt = 0; attempt < MF_TRADE_REQUEST_ATTEMPTS; attempt++)
    {
        u32 index = MfRandom_Modulo(rules->randomizerSeed, MF_RANDOM_CAT_TRADE,
                                    (attempt << 8) | tradeId, 0x8000, (u16)min(total, 0xFFFF));
        enum Species species = WildSlotSpeciesAt(headers, index);

        if (attempt == 0)
            start = index;
        if (!IsValidRequest(species, offered))
            continue;
        if (!similar || MfSpeciesMap_GetEvoStage(species) == stage)
            return species;
        if (fallback == SPECIES_NONE)
            fallback = species;
    }
    if (fallback != SPECIES_NONE)
        return fallback;

    for (i = 0; i < total; i++)
    {
        enum Species species = WildSlotSpeciesAt(headers, (start + i) % total);

        if (IsValidRequest(species, offered))
            return species;
    }
    return vanilla;
}

static u8 CacheFlags(const struct ModernRules *rules)
{
    return (rules->randomWild << 0)
         | (rules->randomMapBased << 1)
         | (rules->randomSimilar << 2)
         | (rules->randomChaos << 3)
         | (rules->randomIncludeLegendaries << 4)
         | (rules->regionalForms << 5);
}

// Resolves every trade in table order so later trades skip earlier picks.
static void EnsureCache(void)
{
    const struct ModernRules *rules = MfRules_GetActiveRules();
    const struct WildPokemonHeader *headers = MfGetActiveWildMonHeaders();
    u8 flags = CacheFlags(rules);
    enum Species vanillaOffered;
    enum Species vanillaRequested;
    u32 total;
    u8 id;

    if (sCache.valid && sCache.seed == rules->randomizerSeed && sCache.headers == headers && sCache.flags == flags)
        return;

    sCache.seed = rules->randomizerSeed;
    sCache.headers = headers;
    sCache.flags = flags;
    sCache.count = 0;
    total = CountWildSlots(headers);

    for (id = 0; id < MF_TRADE_RANDOM_MAX_TRADES; id++)
    {
        enum Species offered;

        if (!GetInGameTradeVanillaSpecies(id, &vanillaOffered, &vanillaRequested))
            break;
        offered = PickOffered(rules, vanillaOffered, id);
        sCache.requested[id] = PickRequested(rules, headers, total, vanillaRequested, offered, id);
        sCache.offered[id] = offered;
        sCache.count = id + 1;
    }
    sCache.valid = TRUE;
}

enum Species MfTradeRandom_OfferedSpecies(enum Species vanillaOffered, u8 tradeId)
{
    if (!MfTradeRandom_IsActive())
        return vanillaOffered;
    EnsureCache();
    return (tradeId < sCache.count) ? sCache.offered[tradeId] : vanillaOffered;
}

enum Species MfTradeRandom_RequestedSpecies(enum Species vanillaRequested,
                                            enum Species vanillaOffered,
                                            u8 tradeId)
{
    (void)vanillaOffered;
    if (!MfTradeRandom_IsActive())
        return vanillaRequested;
    EnsureCache();
    return (tradeId < sCache.count) ? sCache.requested[tradeId] : vanillaRequested;
}
