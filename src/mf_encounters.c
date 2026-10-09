#include "global.h"
#include "event_data.h"
#include "overworld.h"
#include "pokemon.h"
#include "mf_encounters.h"
#include "mf_random.h"
#include "mf_rules.h"
#include "mf_species_map.h"
#include "constants/flags.h"
#include "constants/species.h"

#if defined(FIRERED)
#include "data/mf_modern_wild_encounters.h"
#endif

bool32 MfShouldUseModernWildEncounters(void)
{
    u8 mode = MfRules_GetAlternateSpawns();

    if (mode == MF_ENCOUNTERS_MODERN)
        return TRUE;
    if (mode == MF_ENCOUNTERS_POSTGAME)
        return FlagGet(FLAG_SYS_GAME_CLEAR);
    return FALSE;
}

const struct WildPokemonHeader *MfGetActiveWildMonHeaders(void)
{
#if defined(FIRERED)
    if (MfShouldUseModernWildEncounters())
        return gMfModernWildMonHeaders;
#endif
    return gWildMonHeaders;
}

static bool8 MfIsUnownSpecies(enum Species species)
{
    return species == SPECIES_UNOWN
        || (species >= SPECIES_UNOWN_B && species <= SPECIES_UNOWN_QUESTION);
}

enum Species MfWildEncounterSpecies(enum Species species, u16 mapsec)
{
    if (species == SPECIES_NONE || species == SPECIES_EGG || MfIsUnownSpecies(species))
        return species;
    return MfSpeciesMapActive(species, MF_RANDOM_CAT_WILD, mapsec);
}

enum Species MfWildEncounterSpeciesHere(enum Species species)
{
    return MfWildEncounterSpecies(species, gMapHeader.regionMapSectionId);
}

enum Species MfWildBattleSpecies(enum Species species)
{
    const struct ModernRules *rules;
    u16 locationKey;
    u16 salt;
    bool8 similar;
    bool8 includeLegendaries;

    if (species == SPECIES_NONE || species == SPECIES_EGG || MfIsUnownSpecies(species))
        return species;
    if (!MfSpeciesMap_CategoryRemaps(MF_RANDOM_CAT_WILD))
        return species;

    rules = MfRules_GetActiveRules();
    locationKey = MfRandom_LocationKey(MF_RANDOM_CAT_WILD,
                                       gMapHeader.regionMapSectionId,
                                       rules->randomMapBased);
    salt = MfRandom_TakeChaosSalt();
    if (salt != 0)
        locationKey ^= salt;
    similar = rules->randomSimilar && !rules->randomChaos;
    includeLegendaries = rules->randomIncludeLegendaries || rules->randomChaos;
    return MfSpeciesMapEx(species,
                          rules->randomizerSeed,
                          MF_RANDOM_CAT_WILD,
                          locationKey,
                          similar,
                          includeLegendaries);
}

enum Species MfWildSlotSpecies(const struct WildPokemon *slot, u16 mapsec)
{
    if (slot == NULL)
        return SPECIES_NONE;
    return MfWildEncounterSpecies(slot->species, mapsec);
}

u16 MfWildHeaderMapsec(const struct WildPokemonHeader *header)
{
    if (header == NULL)
        return 0;
    return Overworld_GetMapHeaderByGroupAndId(header->mapGroup, header->mapNum)->regionMapSectionId;
}

enum Species MfStaticEncounterSpecies(enum Species species)
{
    if (species == SPECIES_NONE || species == SPECIES_EGG)
        return species;
    return MfSpeciesMapActive(species, MF_RANDOM_CAT_STATIC, 0);
}

enum Species MfStaticGiftSpecies(enum Species species)
{
    // FR Oak sets FLAG_SYS_POKEMON_GET *before* givemon. Skip while the
    // party is empty so Oak's starter is S53's remap, not STATIC POKéMON.
    if (gPartiesCount[B_TRAINER_PLAYER] == 0)
        return species;
    return MfStaticEncounterSpecies(species);
}
