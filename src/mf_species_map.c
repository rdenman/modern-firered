#include "global.h"
#include "mf_evolution.h"
#include "mf_random.h"
#include "mf_rules.h"
#include "mf_species_map.h"
#include "pokemon.h"
#include "constants/pokedex.h"
#include "constants/moves.h"
#include "constants/species.h"

#if MF_RANDOMIZER

#define MF_SPECIES_MAP_POOL_MAX 768

#define MF_MAP_FILTER_BST_50  0
#define MF_MAP_FILTER_BST_100 1
#define MF_MAP_FILTER_BST_200 2
#define MF_MAP_FILTER_STAGE   3
#define MF_MAP_FILTER_HM      4
#define MF_MAP_FILTER_ANY     5
#define MF_MAP_FILTER_TYPE    6

struct MfSpeciesMapEntry
{
    u16 species;
    u16 bst;
    u8 stage;
    u8 hmMask;
    u8 legendary;
};

static EWRAM_DATA struct MfSpeciesMapEntry sPool[MF_SPECIES_MAP_POOL_MAX] = {0};
static EWRAM_DATA u16 sPoolCount = 0;
static EWRAM_DATA bool8 sPoolReady = FALSE;

static bool8 TeachableHasMove(enum Species species, enum Move move)
{
    const u16 *learnset = gSpeciesInfo[species].teachableLearnset;
    u32 i;

    if (learnset == NULL)
        return FALSE;
    for (i = 0; learnset[i] != MOVE_UNAVAILABLE; i++)
    {
        if (learnset[i] == move)
            return TRUE;
    }
    return FALSE;
}

u16 MfSpeciesMap_GetRawBst(enum Species species)
{
    const struct SpeciesInfo *info = &gSpeciesInfo[SanitizeSpeciesId(species)];

    return (u16)(info->baseHP
               + info->baseAttack
               + info->baseDefense
               + info->baseSpeed
               + info->baseSpAttack
               + info->baseSpDefense);
}

u8 MfSpeciesMap_GetKantoHmMask(enum Species species)
{
    u8 mask = 0;

    species = SanitizeSpeciesId(species);
    if (TeachableHasMove(species, MOVE_CUT))
        mask |= MF_SPECIES_MAP_HM_CUT;
    if (TeachableHasMove(species, MOVE_SURF))
        mask |= MF_SPECIES_MAP_HM_SURF;
    if (TeachableHasMove(species, MOVE_STRENGTH))
        mask |= MF_SPECIES_MAP_HM_STRENGTH;
    return mask;
}

bool8 MfSpeciesMap_CanLearnKantoHm(enum Species species, enum Move move)
{
    if (move == MOVE_CUT)
        return (MfSpeciesMap_GetKantoHmMask(species) & MF_SPECIES_MAP_HM_CUT) != 0;
    if (move == MOVE_SURF)
        return (MfSpeciesMap_GetKantoHmMask(species) & MF_SPECIES_MAP_HM_SURF) != 0;
    if (move == MOVE_STRENGTH)
        return (MfSpeciesMap_GetKantoHmMask(species) & MF_SPECIES_MAP_HM_STRENGTH) != 0;
    return FALSE;
}

bool8 MfSpeciesMap_IsLegendary(enum Species species)
{
    const struct SpeciesInfo *info = &gSpeciesInfo[SanitizeSpeciesId(species)];

    return info->isRestrictedLegendary || info->isSubLegendary || info->isMythical;
}

static bool8 IsUsableForm(enum Species species)
{
    const struct SpeciesInfo *info;
    enum Species base;

    if (species == SPECIES_NONE || species == SPECIES_EGG)
        return FALSE;
    if (!IsSpeciesEnabled(species))
        return FALSE;

    info = &gSpeciesInfo[species];
    if (info->isMegaEvolution
     || info->isPrimalReversion
     || info->isUltraBurst
     || info->isGigantamax
     || info->isTeraForm
     || info->isTotem)
        return FALSE;

    base = GET_BASE_SPECIES_ID(species);
    if (species != base
     && !info->isAlolanForm
     && !info->isGalarianForm
     && !info->isHisuianForm
     && !info->isPaldeanForm)
        return FALSE;

    return TRUE;
}

static bool8 InGen13Dex(enum Species species)
{
    u16 dex = gSpeciesInfo[species].natDexNum;

    return dex >= NATIONAL_DEX_BULBASAUR && dex <= NATIONAL_DEX_DEOXYS;
}

static s16 FindPoolIndex(enum Species species)
{
    u16 i;

    for (i = 0; i < sPoolCount; i++)
    {
        if (sPool[i].species == species)
            return (s16)i;
    }
    return -1;
}

static bool8 TryAddPoolSpecies(enum Species species)
{
    if (!IsUsableForm(species))
        return FALSE;
    if (FindPoolIndex(species) >= 0)
        return FALSE;
    if (sPoolCount >= MF_SPECIES_MAP_POOL_MAX)
        return FALSE;

    sPool[sPoolCount].species = species;
    sPool[sPoolCount].bst = MfSpeciesMap_GetRawBst(species);
    sPool[sPoolCount].hmMask = MfSpeciesMap_GetKantoHmMask(species);
    sPool[sPoolCount].legendary = MfSpeciesMap_IsLegendary(species);
    sPool[sPoolCount].stage = MF_SPECIES_MAP_STAGE_0;
    sPoolCount++;
    return TRUE;
}

static void FillEvoStages(void)
{
    u16 i, j, k;
    const struct Evolution *evos;

    for (i = 0; i < sPoolCount; i++)
    {
        bool8 hasPre = FALSE;
        bool8 hasEvo = FALSE;

        if (sPool[i].legendary)
        {
            sPool[i].stage = MF_SPECIES_MAP_STAGE_LEGENDARY;
            continue;
        }

        evos = MfGetVanillaSpeciesEvolutions(sPool[i].species);
        if (evos != NULL)
        {
            for (j = 0; evos[j].method != EVOLUTIONS_END; j++)
            {
                if (FindPoolIndex(SanitizeSpeciesId(evos[j].targetSpecies)) >= 0)
                {
                    hasEvo = TRUE;
                    break;
                }
            }
        }

        for (j = 0; j < sPoolCount; j++)
        {
            evos = MfGetVanillaSpeciesEvolutions(sPool[j].species);
            if (evos == NULL)
                continue;
            for (k = 0; evos[k].method != EVOLUTIONS_END; k++)
            {
                if (SanitizeSpeciesId(evos[k].targetSpecies) == sPool[i].species)
                {
                    hasPre = TRUE;
                    break;
                }
            }
            if (hasPre)
                break;
        }

        if (!hasPre)
            sPool[i].stage = MF_SPECIES_MAP_STAGE_0;
        else if (hasEvo)
            sPool[i].stage = MF_SPECIES_MAP_STAGE_1;
        else
            sPool[i].stage = MF_SPECIES_MAP_STAGE_2;
    }
}

void MfSpeciesMap_EnsurePool(void)
{
    u16 i;
    u8 wave;
    u16 startCount;

    if (sPoolReady)
        return;

    sPoolCount = 0;
    for (i = SPECIES_BULBASAUR; i < NUM_SPECIES; i++)
    {
        if (InGen13Dex(i))
            TryAddPoolSpecies(i);
    }

    for (wave = 0; wave < 8; wave++)
    {
        u16 idx;
        bool8 added = FALSE;

        startCount = sPoolCount;
        for (idx = 0; idx < startCount; idx++)
        {
            const struct Evolution *evos = MfGetVanillaSpeciesEvolutions(sPool[idx].species);

            if (evos == NULL)
                continue;
            for (i = 0; evos[i].method != EVOLUTIONS_END; i++)
            {
                if (TryAddPoolSpecies(SanitizeSpeciesId(evos[i].targetSpecies)))
                    added = TRUE;
            }
        }
        if (!added)
            break;
    }

    FillEvoStages();
    sPoolReady = TRUE;
}

bool8 MfSpeciesMap_IsCandidate(enum Species species)
{
    MfSpeciesMap_EnsurePool();
    return FindPoolIndex(SanitizeSpeciesId(species)) >= 0;
}

u16 MfSpeciesMap_GetPoolCount(void)
{
    MfSpeciesMap_EnsurePool();
    return sPoolCount;
}

enum Species MfSpeciesMap_GetPoolSpecies(u16 index)
{
    MfSpeciesMap_EnsurePool();
    if (index >= sPoolCount)
        return SPECIES_NONE;
    return sPool[index].species;
}

u8 MfSpeciesMap_GetEvoStage(enum Species species)
{
    s16 idx;

    MfSpeciesMap_EnsurePool();
    idx = FindPoolIndex(SanitizeSpeciesId(species));
    if (idx < 0)
        return MF_SPECIES_MAP_STAGE_0;
    return sPool[idx].stage;
}

static bool8 EntryMatches(const struct MfSpeciesMapEntry *src,
                          const struct MfSpeciesMapEntry *dest,
                          bool8 similar,
                          bool8 includeLegendaries,
                          u8 filter)
{
    u16 delta;
    u16 bstDiff;

    if (!includeLegendaries && dest->legendary)
        return FALSE;
    if ((dest->hmMask & src->hmMask) != src->hmMask)
        return FALSE;

    if (filter == MF_MAP_FILTER_ANY)
        return TRUE;

    if (similar && filter <= MF_MAP_FILTER_STAGE && dest->stage != src->stage)
        return FALSE;

    if (similar && filter <= MF_MAP_FILTER_BST_200)
    {
        if (filter == MF_MAP_FILTER_BST_50)
            delta = MF_SPECIES_MAP_BST_STEP;
        else if (filter == MF_MAP_FILTER_BST_100)
            delta = MF_SPECIES_MAP_BST_STEP * 2;
        else
            delta = MF_SPECIES_MAP_BST_STEP * 4;

        bstDiff = (src->bst > dest->bst) ? (src->bst - dest->bst) : (dest->bst - src->bst);
        if (bstDiff > delta)
            return FALSE;
    }

    return TRUE;
}

static bool8 ThemedEntryMatches(const struct MfSpeciesMapEntry *src,
                                const struct MfSpeciesMapEntry *dest,
                                bool8 similar,
                                bool8 includeLegendaries,
                                u8 filter,
                                u8 themeType)
{
    u16 delta;
    u16 bstDiff;

    if (!includeLegendaries && dest->legendary)
        return FALSE;
    if (GetSpeciesType(dest->species, 0) != themeType
     && GetSpeciesType(dest->species, 1) != themeType)
        return FALSE;
    if (filter == MF_MAP_FILTER_TYPE)
        return TRUE;
    if ((dest->hmMask & src->hmMask) != src->hmMask)
        return FALSE;
    if (filter == MF_MAP_FILTER_ANY)
        return TRUE;
    if (similar && filter <= MF_MAP_FILTER_STAGE && dest->stage != src->stage)
        return FALSE;
    if (similar && filter <= MF_MAP_FILTER_BST_200)
    {
        if (filter == MF_MAP_FILTER_BST_50)
            delta = MF_SPECIES_MAP_BST_STEP;
        else if (filter == MF_MAP_FILTER_BST_100)
            delta = MF_SPECIES_MAP_BST_STEP * 2;
        else
            delta = MF_SPECIES_MAP_BST_STEP * 4;
        bstDiff = (src->bst > dest->bst) ? (src->bst - dest->bst) : (dest->bst - src->bst);
        if (bstDiff > delta)
            return FALSE;
    }
    return TRUE;
}

static u16 CountMatches(const struct MfSpeciesMapEntry *src,
                        bool8 similar,
                        bool8 includeLegendaries,
                        u8 filter)
{
    u16 i;
    u16 count = 0;

    for (i = 0; i < sPoolCount; i++)
    {
        if (EntryMatches(src, &sPool[i], similar, includeLegendaries, filter))
            count++;
    }
    return count;
}

static u16 CountThemedMatches(const struct MfSpeciesMapEntry *src,
                              bool8 similar,
                              bool8 includeLegendaries,
                              u8 filter,
                              u8 themeType)
{
    u16 i;
    u16 count = 0;

    for (i = 0; i < sPoolCount; i++)
    {
        if (ThemedEntryMatches(src, &sPool[i], similar, includeLegendaries, filter, themeType))
            count++;
    }
    return count;
}

static enum Species NthMatch(const struct MfSpeciesMapEntry *src,
                             bool8 similar,
                             bool8 includeLegendaries,
                             u8 filter,
                             u16 n)
{
    u16 i;

    for (i = 0; i < sPoolCount; i++)
    {
        if (EntryMatches(src, &sPool[i], similar, includeLegendaries, filter))
        {
            if (n == 0)
                return sPool[i].species;
            n--;
        }
    }
    return src->species;
}

static enum Species NthThemedMatch(const struct MfSpeciesMapEntry *src,
                                   bool8 similar,
                                   bool8 includeLegendaries,
                                   u8 filter,
                                   u8 themeType,
                                   u16 n)
{
    u16 i;

    for (i = 0; i < sPoolCount; i++)
    {
        if (ThemedEntryMatches(src, &sPool[i], similar, includeLegendaries, filter, themeType))
        {
            if (n == 0)
                return sPool[i].species;
            n--;
        }
    }
    return src->species;
}

enum Species MfSpeciesMapEx(enum Species species,
                            u32 seed,
                            enum MfRandomCategory category,
                            u16 locationKey,
                            bool8 similar,
                            bool8 includeLegendaries)
{
    s16 idx;
    const struct MfSpeciesMapEntry *src;
    u8 filter;
    u8 startFilter;
    u8 endFilter;
    u16 count;
    u16 pick;

    species = SanitizeSpeciesId(species);
    if (species == SPECIES_NONE || species == SPECIES_EGG)
        return species;

    MfSpeciesMap_EnsurePool();
    idx = FindPoolIndex(species);
    if (idx < 0)
        return species;

    src = &sPool[idx];
    if (src->legendary && !includeLegendaries)
        return species;

    startFilter = similar ? MF_MAP_FILTER_BST_50 : MF_MAP_FILTER_HM;
    endFilter = MF_MAP_FILTER_ANY;
    for (filter = startFilter; filter <= endFilter; filter++)
    {
        if (!similar && filter < MF_MAP_FILTER_HM)
            continue;
        count = CountMatches(src, similar, includeLegendaries, filter);
        if (count == 0)
            continue;
        pick = MfRandom_Modulo(seed, category, species, locationKey, count);
        return NthMatch(src, similar, includeLegendaries, filter, pick);
    }
    return species;
}

enum Species MfSpeciesMapExForType(enum Species species,
                                   u32 seed,
                                   enum MfRandomCategory category,
                                   u16 locationKey,
                                   bool8 similar,
                                   bool8 includeLegendaries,
                                   enum Type themeType)
{
    s16 idx;
    const struct MfSpeciesMapEntry *src;
    u8 filter;
    u8 startFilter;
    u8 endFilter;
    u16 count;
    u16 pick;
    bool8 legs;

    if (themeType == TYPE_NONE)
        return MfSpeciesMapEx(species, seed, category, locationKey, similar, includeLegendaries);

    species = SanitizeSpeciesId(species);
    if (species == SPECIES_NONE || species == SPECIES_EGG)
        return species;

    MfSpeciesMap_EnsurePool();
    idx = FindPoolIndex(species);
    if (idx < 0)
        return species;

    src = &sPool[idx];
    startFilter = similar ? MF_MAP_FILTER_BST_50 : MF_MAP_FILTER_HM;
    endFilter = MF_MAP_FILTER_TYPE;
    for (legs = includeLegendaries; ; )
    {
        for (filter = startFilter; filter <= endFilter; filter++)
        {
            if (!similar && filter < MF_MAP_FILTER_HM)
                continue;
            count = CountThemedMatches(src, similar, legs, filter, themeType);
            if (count == 0)
                continue;
            pick = MfRandom_Modulo(seed, category, src->species, locationKey, count);
            return NthThemedMatch(src, similar, legs, filter, themeType, pick);
        }
        if (legs)
            break;
        legs = TRUE;
    }
    return src->species;
}

bool8 MfSpeciesMap_CategoryRemaps(enum MfRandomCategory category)
{
    const struct ModernRules *rules = MfRules_GetActiveRules();

    switch (category)
    {
    case MF_RANDOM_CAT_WILD:
        return rules->randomWild;
    case MF_RANDOM_CAT_TRAINER:
        return rules->randomTrainer;
    case MF_RANDOM_CAT_STATIC:
        return rules->randomStatic;
    default:
        return FALSE;
    }
}

enum Species MfSpeciesMapActive(enum Species species,
                                enum MfRandomCategory category,
                                u16 mapsec)
{
    const struct ModernRules *rules;
    u16 locationKey;
    bool8 similar;
    bool8 includeLegendaries;

    if (!MfSpeciesMap_CategoryRemaps(category))
        return species;

    rules = MfRules_GetActiveRules();
    locationKey = MfRandom_LocationKey(category, mapsec, rules->randomMapBased);
    similar = rules->randomSimilar && !rules->randomChaos;
    includeLegendaries = rules->randomIncludeLegendaries || rules->randomChaos;
    return MfSpeciesMapEx(species,
                          rules->randomizerSeed,
                          category,
                          locationKey,
                          similar,
                          includeLegendaries);
}

#else // !MF_RANDOMIZER

enum Species MfSpeciesMapEx(enum Species species,
                            u32 seed,
                            enum MfRandomCategory category,
                            u16 locationKey,
                            bool8 similar,
                            bool8 includeLegendaries)
{
    (void)seed;
    (void)category;
    (void)locationKey;
    (void)similar;
    (void)includeLegendaries;
    return species;
}

enum Species MfSpeciesMapExForType(enum Species species,
                                   u32 seed,
                                   enum MfRandomCategory category,
                                   u16 locationKey,
                                   bool8 similar,
                                   bool8 includeLegendaries,
                                   enum Type themeType)
{
    (void)seed;
    (void)category;
    (void)locationKey;
    (void)similar;
    (void)includeLegendaries;
    (void)themeType;
    return species;
}

enum Species MfSpeciesMapActive(enum Species species,
                                enum MfRandomCategory category,
                                u16 mapsec)
{
    (void)category;
    (void)mapsec;
    return species;
}

bool8 MfSpeciesMap_CategoryRemaps(enum MfRandomCategory category)
{
    (void)category;
    return FALSE;
}

bool8 MfSpeciesMap_IsCandidate(enum Species species)
{
    (void)species;
    return FALSE;
}

u16 MfSpeciesMap_GetPoolCount(void)
{
    return 0;
}

enum Species MfSpeciesMap_GetPoolSpecies(u16 index)
{
    (void)index;
    return SPECIES_NONE;
}

u8 MfSpeciesMap_GetEvoStage(enum Species species)
{
    (void)species;
    return MF_SPECIES_MAP_STAGE_0;
}

bool8 MfSpeciesMap_IsLegendary(enum Species species)
{
    const struct SpeciesInfo *info = &gSpeciesInfo[SanitizeSpeciesId(species)];

    return info->isRestrictedLegendary || info->isSubLegendary || info->isMythical;
}

u16 MfSpeciesMap_GetRawBst(enum Species species)
{
    const struct SpeciesInfo *info = &gSpeciesInfo[SanitizeSpeciesId(species)];

    return (u16)(info->baseHP
               + info->baseAttack
               + info->baseDefense
               + info->baseSpeed
               + info->baseSpAttack
               + info->baseSpDefense);
}

u8 MfSpeciesMap_GetKantoHmMask(enum Species species)
{
    (void)species;
    return 0;
}

bool8 MfSpeciesMap_CanLearnKantoHm(enum Species species, enum Move move)
{
    (void)species;
    (void)move;
    return FALSE;
}

void MfSpeciesMap_EnsurePool(void)
{
}

#endif // MF_RANDOMIZER
