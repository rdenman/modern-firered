#include "global.h"
#include "event_data.h"
#include "overworld.h"
#include "pokemon.h"
#include "string_util.h"
#include "text.h"
#include "mf_encounters.h"
#include "mf_random.h"
#include "mf_rules.h"
#include "mf_species_map.h"
#include "constants/characters.h"
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

u16 MfScript_StaticGiftSpecies(void)
{
    return MfStaticGiftSpecies(gSpecialVar_0x8004);
}

// FireRed Celadon prize clerk (not LeafGreen's Pinsir/Dratini swap).
static const u16 sGameCornerPrizeSpecies[MF_GAME_CORNER_PRIZE_COUNT] = {
    SPECIES_ABRA,
    SPECIES_CLEFAIRY,
    SPECIES_DRATINI,
    SPECIES_SCYTHER,
    SPECIES_PORYGON,
};

static const u16 sGameCornerPrizeCoins[MF_GAME_CORNER_PRIZE_COUNT] = {
    180,
    500,
    2800,
    5500,
    9999,
};

EWRAM_DATA u8 gMfGameCornerPrizeLabels[MF_GAME_CORNER_PRIZE_COUNT][MF_GAME_CORNER_PRIZE_LABEL_SIZE] = {0};

// Keep in sync with dynmultipush strides in Celadon Game Corner FR script.
STATIC_ASSERT(MF_GAME_CORNER_PRIZE_LABEL_SIZE == 48, MfGameCornerPrizeLabelStride);

static u8 *AppendPrizeCoins(u8 *s, u16 coins)
{
    if (coins >= 1000)
    {
        s = ConvertIntToDecimalStringN(s, coins / 1000, STR_CONV_MODE_LEFT_ALIGN, 1);
        *s++ = CHAR_COMMA;
        s = ConvertIntToDecimalStringN(s, coins % 1000, STR_CONV_MODE_LEADING_ZEROS, 3);
    }
    else
    {
        s = ConvertIntToDecimalStringN(s, coins, STR_CONV_MODE_LEFT_ALIGN, 3);
    }
    return StringCopy(s, COMPOUND_STRING(" COINS"));
}

void MfScript_BufferGameCornerPrizeMenu(void)
{
    u32 i;

    for (i = 0; i < MF_GAME_CORNER_PRIZE_COUNT; i++)
    {
        u8 *s = StringCopy(gMfGameCornerPrizeLabels[i],
                           GetSpeciesName(MfStaticGiftSpecies(sGameCornerPrizeSpecies[i])));

        *s++ = EXT_CTRL_CODE_BEGIN;
        *s++ = EXT_CTRL_CODE_CLEAR_TO;
        *s++ = 80;
        *s++ = EXT_CTRL_CODE_BEGIN;
        *s++ = EXT_CTRL_CODE_FONT;
        *s++ = FONT_SMALL;
        *s++ = CHAR_SPACE;
        AppendPrizeCoins(s, sGameCornerPrizeCoins[i]);
    }
}
