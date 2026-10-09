#include "global.h"
#include "mf_evolution.h"
#include "mf_random.h"
#include "mf_rules.h"
#include "mf_species_map.h"
#include "pokemon.h"
#include "constants/items.h"
#include "constants/pokemon.h"
#include "constants/rtc.h"

static const u8 sText_EvolutionLimit[] = _(
    "The evolution limit won't let\n"
    "this Pokémon evolve.{PAUSE_UNTIL_PRESS}");

bool8 MfIsEvolutionBlockedByLimitValue(enum Species species, u8 evoLimit, bool8 hasPreEvolution)
{
    (void)species;

    if (evoLimit == MF_EVO_LIMIT_NONE)
        return TRUE;
    if (evoLimit == MF_EVO_LIMIT_FIRST)
        return hasPreEvolution;
    return FALSE;
}

const struct Evolution *MfGetVanillaSpeciesEvolutions(enum Species species)
{
    const struct Evolution *evolutions = gSpeciesInfo[SanitizeSpeciesId(species)].evolutions;

    if (evolutions == NULL)
        return gSpeciesInfo[SPECIES_NONE].evolutions;
    return evolutions;
}

bool8 MfSpeciesHasPreEvolution(enum Species species)
{
    species = GET_BASE_SPECIES_ID(SanitizeSpeciesId(species));
    if (species == SPECIES_NONE)
        return FALSE;
    return GetSpeciesPreEvolution(species) != SPECIES_NONE;
}

bool8 MfIsEvolutionBlockedByLimit(enum Species species)
{
    u8 limit = MfRules_GetEvoLimit();

    if (limit == MF_EVO_LIMIT_OFF)
        return FALSE;
    if (limit == MF_EVO_LIMIT_NONE)
        return TRUE;
    return MfIsEvolutionBlockedByLimitValue(species, limit, MfSpeciesHasPreEvolution(species));
}

bool8 MfIsEvolutionItemBlockedByLimit(enum Species species, enum Item item)
{
    const struct Evolution *evolutions;
    u32 i;

    if (!MfIsEvolutionBlockedByLimit(species))
        return FALSE;

    evolutions = GetSpeciesEvolutions(species);
    if (evolutions == NULL)
        return FALSE;

    for (i = 0; evolutions[i].method != EVOLUTIONS_END; i++)
    {
        if (evolutions[i].method == EVO_ITEM && evolutions[i].param == item)
            return TRUE;
    }
    return FALSE;
}

const u8 *MfGetEvolutionLimitMessage(void)
{
    return sText_EvolutionLimit;
}

bool8 MfEvoItemIsKantoObtainable(enum Item item)
{
    switch (item)
    {
    case ITEM_FIRE_STONE:
    case ITEM_WATER_STONE:
    case ITEM_THUNDER_STONE:
    case ITEM_LEAF_STONE:
    case ITEM_MOON_STONE:
    case ITEM_SUN_STONE:
    case ITEM_LINKING_CORD:
    case ITEM_METAL_COAT:
    case ITEM_KINGS_ROCK:
    case ITEM_DRAGON_SCALE:
    case ITEM_UPGRADE:
    case ITEM_DUBIOUS_DISC:
    case ITEM_PROTECTOR:
    case ITEM_ELECTIRIZER:
    case ITEM_MAGMARIZER:
    case ITEM_PRISM_SCALE:
    case ITEM_REAPER_CLOTH:
    case ITEM_DEEP_SEA_TOOTH:
    case ITEM_DEEP_SEA_SCALE:
        return TRUE;
    default:
        return FALSE;
    }
}

static bool8 ConditionIsAllowed(const struct EvolutionParam *p)
{
    switch (p->condition)
    {
    case IF_GENDER:
    case IF_MIN_FRIENDSHIP:
    case IF_ATK_GT_DEF:
    case IF_ATK_EQ_DEF:
    case IF_ATK_LT_DEF:
    case IF_PID_UPPER_MODULO_10_GT:
    case IF_PID_UPPER_MODULO_10_EQ:
    case IF_PID_UPPER_MODULO_10_LT:
    case IF_PID_MODULO_100_GT:
    case IF_PID_MODULO_100_EQ:
    case IF_PID_MODULO_100_LT:
    case IF_NATURE:
    case IF_AMPED_NATURE:
    case IF_LOW_KEY_NATURE:
    case IF_NOT_REGION:
        return TRUE;
    case IF_HOLD_ITEM:
        return MfEvoItemIsKantoObtainable(p->arg1);
    default:
        return FALSE;
    }
}

bool8 MfEvolutionIsKantoAchievable(const struct Evolution *evo)
{
    u32 i;

    if (evo == NULL)
        return FALSE;

    switch (evo->method)
    {
    case EVO_LEVEL:
        if (evo->param > MAX_LEVEL)
            return FALSE;
        break;
    case EVO_TRADE:
        break;
    case EVO_ITEM:
        if (!MfEvoItemIsKantoObtainable(evo->param))
            return FALSE;
        break;
    default:
        return FALSE;
    }

    if (evo->params == NULL)
        return TRUE;

    for (i = 0; evo->params[i].condition != CONDITIONS_END; i++)
    {
        if (!ConditionIsAllowed(&evo->params[i]))
            return FALSE;
    }
    return TRUE;
}

#if MF_RANDOMIZER

#define MF_RANDOM_EVO_MAX 16
#define MF_RANDOM_EVO_COND_MAX 4

static EWRAM_DATA struct Evolution sRandomEvos[MF_RANDOM_EVO_MAX + 1] = {0};
static EWRAM_DATA struct EvolutionParam sRandomEvoParams[MF_RANDOM_EVO_MAX][MF_RANDOM_EVO_COND_MAX + 1] = {0};
static EWRAM_DATA u16 sCachedEvoSpecies = SPECIES_NONE;
static EWRAM_DATA u32 sCachedEvoSeed = 0;
static EWRAM_DATA u8 sCachedEvoFlags = 0;
static EWRAM_DATA bool8 sCachedEvoValid = FALSE;

static u8 EvoCacheFlags(const struct ModernRules *rules)
{
    return (rules->randomEvolution ? 1 : 0)
         | (rules->randomEvolutionMethods ? 2 : 0)
         | (rules->randomSimilar ? 4 : 0)
         | (rules->randomIncludeLegendaries ? 8 : 0);
}

static bool8 MethodIsPrimaryOk(u16 method)
{
    return method == EVO_LEVEL || method == EVO_ITEM || method == EVO_TRADE;
}

static u16 FallbackLevel(u16 method, u16 param)
{
    if (method == EVO_LEVEL && param > 0 && param <= MAX_LEVEL)
        return param;
    if (param > 0 && param <= MAX_LEVEL)
        return param;
    return MF_RANDOM_EVO_FALLBACK_LEVEL;
}

static bool8 ParamsHintNight(const struct EvolutionParam *params)
{
    u32 i;

    if (params == NULL)
        return FALSE;
    for (i = 0; params[i].condition != CONDITIONS_END; i++)
    {
        if (params[i].condition == IF_TIME && params[i].arg1 == TIME_NIGHT)
            return TRUE;
        if (params[i].condition == IF_NOT_TIME && params[i].arg1 != TIME_NIGHT)
            return TRUE;
    }
    return FALSE;
}

static bool8 ParamsHintDay(const struct EvolutionParam *params)
{
    u32 i;

    if (params == NULL)
        return FALSE;
    for (i = 0; params[i].condition != CONDITIONS_END; i++)
    {
        if (params[i].condition == IF_TIME && params[i].arg1 != TIME_NIGHT)
            return TRUE;
        if (params[i].condition == IF_NOT_TIME && params[i].arg1 == TIME_NIGHT)
            return TRUE;
    }
    return FALSE;
}

static bool8 ParamsNeedLocationRewrite(const struct EvolutionParam *params)
{
    u32 i;

    if (params == NULL)
        return FALSE;
    for (i = 0; params[i].condition != CONDITIONS_END; i++)
    {
        switch (params[i].condition)
        {
        case IF_IN_MAP:
        case IF_IN_MAPSEC:
        case IF_REGION:
        case IF_WEATHER:
        case IF_MIN_OVERWORLD_STEPS:
            return TRUE;
        default:
            break;
        }
    }
    return FALSE;
}

static const struct EvolutionParam *CopyAllowedParams(u32 slot, const struct EvolutionParam *src)
{
    u32 i;
    u32 count = 0;

    if (src == NULL)
        return NULL;

    for (i = 0; src[i].condition != CONDITIONS_END && count < MF_RANDOM_EVO_COND_MAX; i++)
    {
        if (!ConditionIsAllowed(&src[i]))
            continue;
        sRandomEvoParams[slot][count++] = src[i];
    }

    if (count == 0)
        return NULL;

    sRandomEvoParams[slot][count].condition = CONDITIONS_END;
    sRandomEvoParams[slot][count].arg1 = 0;
    sRandomEvoParams[slot][count].arg2 = 0;
    sRandomEvoParams[slot][count].arg3 = 0;
    return sRandomEvoParams[slot];
}

static void SanitizeEvoSlot(u32 slot, const struct Evolution *src)
{
    u16 method = src->method;
    u16 param = src->param;

    if (ParamsHintNight(src->params))
    {
        method = EVO_ITEM;
        param = ITEM_MOON_STONE;
    }
    else if (ParamsHintDay(src->params))
    {
        method = EVO_ITEM;
        param = ITEM_SUN_STONE;
    }
    else if (!MethodIsPrimaryOk(method) || ParamsNeedLocationRewrite(src->params))
    {
        if (method == EVO_ITEM && MfEvoItemIsKantoObtainable(param))
        {
            method = EVO_ITEM;
        }
        else
        {
            method = EVO_ITEM;
            param = ITEM_LINKING_CORD;
        }
    }
    else if (method == EVO_ITEM && !MfEvoItemIsKantoObtainable(param))
    {
        param = ITEM_LINKING_CORD;
    }
    else if (method == EVO_LEVEL && param > MAX_LEVEL)
    {
        param = FallbackLevel(method, 0);
    }

    sRandomEvos[slot].method = method;
    sRandomEvos[slot].param = param;
    sRandomEvos[slot].targetSpecies = src->targetSpecies;
    sRandomEvos[slot].params = CopyAllowedParams(slot, src->params);
}

static void FillRandomEvos(u32 seed, enum Species species, bool8 randomTargets, bool8 randomMethods, bool8 similar, bool8 legs)
{
    const struct Evolution *src;
    enum Species donor = species;
    u32 i;
    u32 count = 0;

    if (randomMethods)
        donor = MfSpeciesMapEx(species, seed, MF_RANDOM_CAT_EVO_METH, 0, similar, legs);

    src = MfGetVanillaSpeciesEvolutions(donor);
    if (src == NULL)
    {
        sRandomEvos[0].method = EVOLUTIONS_END;
        sRandomEvos[0].param = 0;
        sRandomEvos[0].targetSpecies = SPECIES_NONE;
        sRandomEvos[0].params = NULL;
        return;
    }

    for (i = 0; src[i].method != EVOLUTIONS_END && count < MF_RANDOM_EVO_MAX; i++)
    {
        enum Species target;

        if (src[i].method == EVO_NONE || src[i].method == EVO_SPLIT_FROM_EVO)
            continue;
        if (SanitizeSpeciesId(src[i].targetSpecies) == SPECIES_NONE)
            continue;

        SanitizeEvoSlot(count, &src[i]);
        target = sRandomEvos[count].targetSpecies;
        if (randomTargets)
        {
            target = MfSpeciesMapEx(target, seed, MF_RANDOM_CAT_EVO, 0, similar, legs);
            sRandomEvos[count].targetSpecies = target;
        }
        count++;
    }

    sRandomEvos[count].method = EVOLUTIONS_END;
    sRandomEvos[count].param = 0;
    sRandomEvos[count].targetSpecies = SPECIES_NONE;
    sRandomEvos[count].params = NULL;
}

const struct Evolution *MfMaybeRandomizeEvolutions(enum Species species, const struct Evolution *src)
{
    const struct ModernRules *rules;
    u32 seed;
    u8 flags;

    if (src == NULL)
        return src;

    rules = MfRules_GetActiveRules();
    if (!rules->randomEvolution && !rules->randomEvolutionMethods)
        return src;

    species = SanitizeSpeciesId(species);
    seed = rules->randomizerSeed;
    flags = EvoCacheFlags(rules);
    if (sCachedEvoValid
     && sCachedEvoSpecies == species
     && sCachedEvoSeed == seed
     && sCachedEvoFlags == flags)
        return sRandomEvos;

    FillRandomEvos(seed, species,
                   rules->randomEvolution,
                   rules->randomEvolutionMethods,
                   rules->randomSimilar && !rules->randomChaos,
                   rules->randomIncludeLegendaries || rules->randomChaos);
    sCachedEvoSpecies = species;
    sCachedEvoSeed = seed;
    sCachedEvoFlags = flags;
    sCachedEvoValid = TRUE;
    return sRandomEvos;
}

#else // !MF_RANDOMIZER

const struct Evolution *MfMaybeRandomizeEvolutions(enum Species species, const struct Evolution *src)
{
    (void)species;
    return src;
}

#endif // MF_RANDOMIZER
