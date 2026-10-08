#include "global.h"
#include "battle.h"
#include "data.h"
#include "event_data.h"
#include "fpmath.h"
#include "mf_monotype.h"
#include "mf_party_random.h"
#include "mf_random.h"
#include "mf_rules.h"
#include "mf_species_map.h"
#include "mf_theme_groups.h"
#include "mf_types.h"
#include "overworld.h"
#include "pokemon.h"
#include "random.h"
#include "constants/abilities.h"
#include "constants/moves.h"
#include "constants/pokemon.h"
#include "constants/species.h"
#include "constants/trainers.h"

#if MF_RANDOMIZER

// Oak balls: 0 Bulbasaur, 1 Squirtle, 2 Charmander (FR VAR_STARTER_MON).
static const enum Species sOakVanilla[MF_OAK_STARTER_COUNT] = {
    SPECIES_BULBASAUR,
    SPECIES_SQUIRTLE,
    SPECIES_CHARMANDER,
};

// Vanilla rival takes the type-advantage ball; alt is the leftover.
static const u8 sOakRivalSlot[MF_OAK_STARTER_COUNT] = { 2, 0, 1 };
static const u8 sOakAltSlot[MF_OAK_STARTER_COUNT] = { 1, 2, 0 };

static bool8 TrainerRandomExcluded(void)
{
    return (gBattleTypeFlags & (BATTLE_TYPE_FRONTIER
                              | BATTLE_TYPE_EREADER_TRAINER
                              | BATTLE_TYPE_TRAINER_HILL)) != 0;
}

u16 MfTrainerRandomKey(const void *trainer, u32 size)
{
    if (trainer == NULL || size == 0)
        return 0;
    return (u16)Crc32B((const u8 *)trainer, size);
}

#define MF_RANDOM_INPUT_TRAINER_THEME 0x5448u
#define MF_RANDOM_INPUT_THEME_GROUP   0x5447u

static EWRAM_DATA u16 sPartyTrainerNum = 0;

void MfBeginTrainerParty(u16 trainerNum)
{
    sPartyTrainerNum = trainerNum;
}

void MfEndTrainerParty(void)
{
    sPartyTrainerNum = 0;
}

static bool8 ThemeTypeIsEnabled(enum Type type)
{
    if (type == TYPE_NONE || type == TYPE_MYSTERY || type == TYPE_STELLAR)
        return FALSE;
    if (type == TYPE_FAIRY && !MfRules_GetActiveRules()->fairyTypes)
        return FALSE;
    return TRUE;
}

static bool8 ThemeTypeHasPoolSpecies(enum Type type, bool8 includeLegendaries)
{
    u16 i;

    for (i = 0; i < MfSpeciesMap_GetPoolCount(); i++)
    {
        enum Species species = MfSpeciesMap_GetPoolSpecies(i);

        if (!includeLegendaries && MfSpeciesMap_IsLegendary(species))
            continue;
        if (MfSpeciesMatchesType(species, type))
            return TRUE;
    }
    return FALSE;
}

static enum Type HashThemeType(u16 locationKey, u32 inputId)
{
    const struct ModernRules *rules = MfRules_GetActiveRules();
    bool8 legs = rules->randomIncludeLegendaries || rules->randomChaos;
    enum Type pool[NUMBER_OF_MON_TYPES];
    u8 count = 0;
    u8 type;

    for (type = 0; type < NUMBER_OF_MON_TYPES; type++)
    {
        if (!ThemeTypeIsEnabled(type))
            continue;
        if (!ThemeTypeHasPoolSpecies(type, legs))
            continue;
        pool[count++] = type;
    }
    if (count == 0)
        return TYPE_NORMAL;

    return pool[MfRandom_Modulo(rules->randomizerSeed,
                                MF_RANDOM_CAT_TRAINER,
                                inputId,
                                locationKey,
                                count)];
}

enum Type MfTrainerThemeTypeFor(u16 trainerKey, u16 trainerNum)
{
    const struct ModernRules *rules;
    u16 locationKey;
    u8 group;

    rules = MfRules_GetActiveRules();
    if (!rules->randomThemedTrainers || !MfSpeciesMap_CategoryRemaps(MF_RANDOM_CAT_TRAINER))
        return TYPE_NONE;

    group = MfTrainerThemeGroup(trainerNum);
    if (group != MF_THEME_GROUP_NONE)
        return HashThemeType(group, MF_RANDOM_INPUT_THEME_GROUP);

    locationKey = MfRandom_LocationKey(MF_RANDOM_CAT_TRAINER,
                                       gMapHeader.regionMapSectionId,
                                       rules->randomMapBased);
    locationKey ^= trainerKey;
    return HashThemeType(locationKey, MF_RANDOM_INPUT_TRAINER_THEME);
}

enum Type MfTrainerThemeType(u16 trainerKey)
{
    return MfTrainerThemeTypeFor(trainerKey, sPartyTrainerNum);
}

enum Species MfTrainerEncounterSpecies(enum Species species, u16 trainerKey)
{
    const struct ModernRules *rules;
    u16 locationKey;
    bool8 similar;
    bool8 legs;
    enum Type theme;

    if (species == SPECIES_NONE || species == SPECIES_EGG)
        return species;
    if (!MfSpeciesMap_CategoryRemaps(MF_RANDOM_CAT_TRAINER))
        return species;
    if (TrainerRandomExcluded())
        return species;

    rules = MfRules_GetActiveRules();
    locationKey = MfRandom_LocationKey(MF_RANDOM_CAT_TRAINER,
                                       gMapHeader.regionMapSectionId,
                                       rules->randomMapBased);
    locationKey ^= trainerKey;
    similar = rules->randomSimilar && !rules->randomChaos;
    legs = rules->randomIncludeLegendaries || rules->randomChaos;
    theme = MfTrainerThemeTypeFor(trainerKey, sPartyTrainerNum);
    if (theme == TYPE_NONE)
    {
        return MfSpeciesMapEx(species,
                              rules->randomizerSeed,
                              MF_RANDOM_CAT_TRAINER,
                              locationKey,
                              similar,
                              legs);
    }
    return MfSpeciesMapExForType(species,
                                 rules->randomizerSeed,
                                 MF_RANDOM_CAT_TRAINER,
                                 locationKey,
                                 similar,
                                 legs,
                                 theme);
}

void MfRandomizeTrainerMon(struct TrainerMon *mon, u16 trainerKey)
{
    enum Species dest;
    u32 i;

    if (mon == NULL)
        return;

    dest = MfTrainerEncounterSpecies(mon->species, trainerKey);
    if (dest == mon->species)
        return;

    mon->species = dest;
    mon->ability = ABILITY_NONE;
    mon->gender = TRAINER_MON_RANDOM_GENDER;
    for (i = 0; i < MAX_MON_MOVES; i++)
        mon->moves[i] = MOVE_NONE;
}

static bool8 TypeIsUsable(enum Type type)
{
    return type != TYPE_NONE && type != TYPE_MYSTERY;
}

bool8 MfSpeciesHasOffensiveTypeAdvantage(enum Species attacker, enum Species defender)
{
    const uq4_12_t (*table)[NUMBER_OF_MON_TYPES];
    u8 atkSlot;
    u8 defSlot;

    attacker = SanitizeSpeciesId(attacker);
    defender = SanitizeSpeciesId(defender);
    if (attacker == SPECIES_NONE || defender == SPECIES_NONE)
        return FALSE;

    table = MfGetTypeEffectivenessTable();
    for (atkSlot = 0; atkSlot < 2; atkSlot++)
    {
        enum Type atkType = GetSpeciesType(attacker, atkSlot);
        uq4_12_t mul = UQ_4_12(1.0);

        if (!TypeIsUsable(atkType))
            continue;
        if (atkSlot == 1 && atkType == GetSpeciesType(attacker, 0))
            continue;

        for (defSlot = 0; defSlot < 2; defSlot++)
        {
            enum Type defType = GetSpeciesType(defender, defSlot);

            if (!TypeIsUsable(defType))
                continue;
            if (defSlot == 1 && defType == GetSpeciesType(defender, 0))
                continue;
            mul = uq4_12_multiply(mul, table[atkType][defType]);
        }
        if (mul > UQ_4_12(1.0))
            return TRUE;
    }
    return FALSE;
}

static bool8 SpeciesUsed(enum Species species, const enum Species used[MF_OAK_STARTER_COUNT], u8 usedCount)
{
    u8 i;

    for (i = 0; i < usedCount; i++)
    {
        if (used[i] == species)
            return TRUE;
    }
    return FALSE;
}

static void StarterMapParams(u32 *seed, bool8 *similar, bool8 *legs)
{
    const struct ModernRules *rules = MfRules_GetActiveRules();

    *seed = rules->randomizerSeed;
    *similar = rules->randomSimilar && !rules->randomChaos;
    *legs = rules->randomIncludeLegendaries || rules->randomChaos;
}

static enum Species MapStarterSlot(enum Species src, u8 slot)
{
    u32 seed;
    bool8 similar;
    bool8 legs;

    StarterMapParams(&seed, &similar, &legs);
    return MfSpeciesMapEx(src, seed, MF_RANDOM_CAT_STATIC, slot, similar, legs);
}

static enum Species UniqueStarter(enum Species src, u8 slot, const enum Species used[MF_OAK_STARTER_COUNT], u8 usedCount)
{
    u32 seed;
    bool8 similar;
    bool8 legs;
    enum Species dest;
    u16 count;
    u16 i;
    u16 pick;
    u16 n;

    dest = MapStarterSlot(src, slot);
    if (!SpeciesUsed(dest, used, usedCount))
        return dest;

    StarterMapParams(&seed, &similar, &legs);
    count = 0;
    for (i = 0; i < MfSpeciesMap_GetPoolCount(); i++)
    {
        dest = MfSpeciesMap_GetPoolSpecies(i);
        if (SpeciesUsed(dest, used, usedCount))
            continue;
        if (similar && MfSpeciesMap_GetEvoStage(dest) != MfSpeciesMap_GetEvoStage(src))
            continue;
        if (!legs && MfSpeciesMap_IsLegendary(dest))
            continue;
        count++;
    }
    if (count == 0)
        return MapStarterSlot(src, slot);

    pick = MfRandom_Modulo(seed, MF_RANDOM_CAT_STATIC, src, (u16)(slot + 16), count);
    n = 0;
    for (i = 0; i < MfSpeciesMap_GetPoolCount(); i++)
    {
        dest = MfSpeciesMap_GetPoolSpecies(i);
        if (SpeciesUsed(dest, used, usedCount))
            continue;
        if (similar && MfSpeciesMap_GetEvoStage(dest) != MfSpeciesMap_GetEvoStage(src))
            continue;
        if (!legs && MfSpeciesMap_IsLegendary(dest))
            continue;
        if (n == pick)
            return dest;
        n++;
    }
    return MapStarterSlot(src, slot);
}

void MfFillRandomOakStarters(enum Species trio[MF_OAK_STARTER_COUNT])
{
    u8 slot;

    trio[0] = SPECIES_NONE;
    trio[1] = SPECIES_NONE;
    trio[2] = SPECIES_NONE;

    if (!MfRules_GetActiveRules()->randomStarter)
    {
        trio[0] = sOakVanilla[0];
        trio[1] = sOakVanilla[1];
        trio[2] = sOakVanilla[2];
        return;
    }

    for (slot = 0; slot < MF_OAK_STARTER_COUNT; slot++)
        trio[slot] = UniqueStarter(sOakVanilla[slot], slot, trio, slot);
}

enum Species MfPickRivalStarterSpecies(enum Species playerSpecies,
                                       enum Species vanillaRival,
                                       enum Species altRival)
{
    if (MfSpeciesHasOffensiveTypeAdvantage(vanillaRival, playerSpecies))
        return vanillaRival;
    if (MfSpeciesHasOffensiveTypeAdvantage(altRival, playerSpecies))
        return altRival;
    return vanillaRival;
}

u16 MfResolveOakStarterForRandomizer(void)
{
    enum Species trio[MF_OAK_STARTER_COUNT];
    enum Species chosen;
    enum Species resolved;
    enum Species rival;
    u8 slot;

    if (MfRules_IsMonotypeActive())
        return FALSE;
    if (!MfRules_GetActiveRules()->randomStarter)
        return FALSE;

    slot = VarGet(VAR_TEMP_1);
    if (slot >= MF_OAK_STARTER_COUNT)
        slot = 0;

    chosen = VarGet(VAR_TEMP_2);
    MfFillRandomOakStarters(trio);
    resolved = trio[slot];
    rival = MfPickRivalStarterSpecies(resolved,
                                      trio[sOakRivalSlot[slot]],
                                      trio[sOakAltSlot[slot]]);
    VarSet(VAR_TEMP_2, resolved);
    VarSet(VAR_TEMP_3, rival);
    return resolved != chosen;
}

#else // !MF_RANDOMIZER

u16 MfTrainerRandomKey(const void *trainer, u32 size)
{
    (void)trainer;
    (void)size;
    return 0;
}

enum Species MfTrainerEncounterSpecies(enum Species species, u16 trainerKey)
{
    (void)trainerKey;
    return species;
}

enum Type MfTrainerThemeType(u16 trainerKey)
{
    (void)trainerKey;
    return TYPE_NONE;
}

enum Type MfTrainerThemeTypeFor(u16 trainerKey, u16 trainerNum)
{
    (void)trainerKey;
    (void)trainerNum;
    return TYPE_NONE;
}

void MfBeginTrainerParty(u16 trainerNum)
{
    (void)trainerNum;
}

void MfEndTrainerParty(void)
{
}

void MfRandomizeTrainerMon(struct TrainerMon *mon, u16 trainerKey)
{
    (void)mon;
    (void)trainerKey;
}

bool8 MfSpeciesHasOffensiveTypeAdvantage(enum Species attacker, enum Species defender)
{
    (void)attacker;
    (void)defender;
    return FALSE;
}

void MfFillRandomOakStarters(enum Species trio[MF_OAK_STARTER_COUNT])
{
    trio[0] = SPECIES_BULBASAUR;
    trio[1] = SPECIES_SQUIRTLE;
    trio[2] = SPECIES_CHARMANDER;
}

enum Species MfPickRivalStarterSpecies(enum Species playerSpecies,
                                       enum Species vanillaRival,
                                       enum Species altRival)
{
    (void)playerSpecies;
    (void)altRival;
    return vanillaRival;
}

u16 MfResolveOakStarterForRandomizer(void)
{
    return FALSE;
}

#endif // MF_RANDOMIZER
