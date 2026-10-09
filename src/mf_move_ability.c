#include "global.h"
#include "mf_move_ability.h"
#include "mf_random.h"
#include "mf_rules.h"
#include "mf_species_map.h"
#include "move.h"
#include "pokemon.h"
#include "constants/abilities.h"
#include "constants/moves.h"
#include "constants/pokemon.h"

#if MF_RANDOMIZER

#define MF_RANDOM_LEARNSET_MAX 64
#define MF_MOVE_POOL_MAX MOVES_COUNT
#define MF_ABILITY_POOL_MAX ABILITIES_COUNT

static EWRAM_DATA u16 sMovePool[MF_MOVE_POOL_MAX] = {0};
static EWRAM_DATA u16 sDmgMovePool[MF_MOVE_POOL_MAX] = {0};
static EWRAM_DATA u16 sAbilityPool[MF_ABILITY_POOL_MAX] = {0};
static EWRAM_DATA u16 sMovePoolCount = 0;
static EWRAM_DATA u16 sDmgMovePoolCount = 0;
static EWRAM_DATA u16 sAbilityPoolCount = 0;
static EWRAM_DATA bool8 sPoolsReady = FALSE;

static EWRAM_DATA struct LevelUpMove sRandomLearnset[MF_RANDOM_LEARNSET_MAX + 1] = {0};
static EWRAM_DATA u16 sCachedLearnsetSpecies = SPECIES_NONE;
static EWRAM_DATA u32 sCachedLearnsetSeed = 0;
static EWRAM_DATA bool8 sCachedLearnsetValid = FALSE;

static bool8 MfAbilityIsFormLocked(enum Ability ability)
{
    switch (ability)
    {
    case ABILITY_WONDER_GUARD:
    case ABILITY_FORECAST:
    case ABILITY_FLOWER_GIFT:
    case ABILITY_MULTITYPE:
    case ABILITY_ILLUSION:
    case ABILITY_IMPOSTER:
    case ABILITY_ZEN_MODE:
    case ABILITY_STANCE_CHANGE:
    case ABILITY_SHIELDS_DOWN:
    case ABILITY_SCHOOLING:
    case ABILITY_DISGUISE:
    case ABILITY_BATTLE_BOND:
    case ABILITY_POWER_CONSTRUCT:
    case ABILITY_RKS_SYSTEM:
    case ABILITY_GULP_MISSILE:
    case ABILITY_ICE_FACE:
    case ABILITY_HUNGER_SWITCH:
    case ABILITY_AS_ONE_ICE_RIDER:
    case ABILITY_AS_ONE_SHADOW_RIDER:
    case ABILITY_ZERO_TO_HERO:
    case ABILITY_COMMANDER:
    case ABILITY_EMBODY_ASPECT_TEAL_MASK:
    case ABILITY_EMBODY_ASPECT_HEARTHFLAME_MASK:
    case ABILITY_EMBODY_ASPECT_WELLSPRING_MASK:
    case ABILITY_EMBODY_ASPECT_CORNERSTONE_MASK:
    case ABILITY_TERA_SHIFT:
    case ABILITY_TERA_SHELL:
    case ABILITY_TERAFORM_ZERO:
        return TRUE;
    default:
        return FALSE;
    }
}

static bool8 MfMoveIsPoolCandidate(enum Move move)
{
    if (move == MOVE_NONE || move == MOVE_STRUGGLE || move == MOVE_SKETCH)
        return FALSE;
    if (move >= MOVES_COUNT)
        return FALSE;
    if (GetMoveEffect(move) == EFFECT_PLACEHOLDER)
        return FALSE;
    if (GetMovePP(move) == 0)
        return FALSE;
    return TRUE;
}

static bool8 MfMoveIsDamaging(enum Move move)
{
    return MfMoveIsPoolCandidate(move)
        && GetMoveCategory(move) != DAMAGE_CATEGORY_STATUS;
}

static bool8 AbilityPoolContains(enum Ability ability)
{
    u16 i;

    for (i = 0; i < sAbilityPoolCount; i++)
    {
        if (sAbilityPool[i] == ability)
            return TRUE;
    }
    return FALSE;
}

static void EnsurePools(void)
{
    u32 i;

    if (sPoolsReady)
        return;

    sMovePoolCount = 0;
    sDmgMovePoolCount = 0;
    sAbilityPoolCount = 0;

    for (i = 1; i < MOVES_COUNT && sMovePoolCount < MF_MOVE_POOL_MAX; i++)
    {
        enum Move move = (enum Move)i;

        if (!MfMoveIsPoolCandidate(move))
            continue;
        sMovePool[sMovePoolCount++] = move;
        if (MfMoveIsDamaging(move) && sDmgMovePoolCount < MF_MOVE_POOL_MAX)
            sDmgMovePool[sDmgMovePoolCount++] = move;
    }

    MfSpeciesMap_EnsurePool();
    for (i = 0; i < MfSpeciesMap_GetPoolCount(); i++)
    {
        enum Species species = MfSpeciesMap_GetPoolSpecies(i);
        u8 slot;

        for (slot = 0; slot < NUM_ABILITY_SLOTS; slot++)
        {
            enum Ability ability = gSpeciesInfo[species].abilities[slot];

            if (ability == ABILITY_NONE || MfAbilityIsFormLocked(ability))
                continue;
            if (AbilityPoolContains(ability))
                continue;
            if (sAbilityPoolCount < MF_ABILITY_POOL_MAX)
                sAbilityPool[sAbilityPoolCount++] = ability;
        }
    }

    sPoolsReady = TRUE;
}

static u32 MoveInputId(enum Species species, enum Move move)
{
    return ((u32)species << 16) | (u16)move;
}

static u32 AbilityInputId(enum Species species, u8 slot)
{
    return ((u32)species << 2) | (u32)slot;
}

enum Move MfMapMove(u32 seed, enum Species species, enum Move move)
{
    u16 idx;

    if (move == MOVE_NONE || move == LEVEL_UP_MOVE_END)
        return move;

    EnsurePools();
    if (sMovePoolCount == 0)
        return move;

    idx = MfRandom_Modulo(seed, MF_RANDOM_CAT_MOVES, MoveInputId(species, move), 0, sMovePoolCount);
    return (enum Move)sMovePool[idx];
}

enum Ability MfMapAbility(u32 seed, enum Species species, u8 slot)
{
    const struct SpeciesInfo *info = &gSpeciesInfo[SanitizeSpeciesId(species)];
    enum Ability vanilla;
    u16 idx;

    if (slot >= NUM_ABILITY_SLOTS)
        return ABILITY_NONE;

    vanilla = info->abilities[slot];
    if (vanilla == ABILITY_NONE)
        return ABILITY_NONE;
    // Form-locked abilities stay on the species that already has them (Shedinja
    // Wonder Guard, Castform Forecast). They are never in the assignment pool.
    if (MfAbilityIsFormLocked(vanilla))
        return vanilla;

    EnsurePools();
    if (sAbilityPoolCount == 0)
        return vanilla;

    idx = MfRandom_Modulo(seed, MF_RANDOM_CAT_ABILITY, AbilityInputId(species, slot), 0, sAbilityPoolCount);
    return (enum Ability)sAbilityPool[idx];
}

static void FillRandomLearnset(u32 seed, enum Species species, const struct LevelUpMove *src)
{
    u32 i;
    u32 count = 0;
    s32 replace = -1;
    s32 firstPositive = -1;

    for (i = 0; src[i].move != LEVEL_UP_MOVE_END && count < MF_RANDOM_LEARNSET_MAX; i++)
    {
        sRandomLearnset[count].level = src[i].level;
        sRandomLearnset[count].move = MfMapMove(seed, species, src[i].move);
        if (src[i].level > 0 && firstPositive < 0)
            firstPositive = (s32)count;
        if (src[i].level > 0 && src[i].level <= MF_RANDOM_LOW_MOVE_LEVEL && replace < 0)
            replace = (s32)count;
        count++;
    }

    sRandomLearnset[count].move = LEVEL_UP_MOVE_END;
    sRandomLearnset[count].level = 0;

    if (count > 0 && !MfLearnsetHasDamagingMoveByLevel(sRandomLearnset, MF_RANDOM_LOW_MOVE_LEVEL))
    {
        if (replace < 0)
            replace = (firstPositive >= 0) ? firstPositive : 0;
        EnsurePools();
        if (sDmgMovePoolCount > 0)
        {
            u16 idx = MfRandom_Modulo(seed, MF_RANDOM_CAT_MOVES,
                                      MoveInputId(species, MOVE_POUND) ^ 0xDADA0000u,
                                      0, sDmgMovePoolCount);
            sRandomLearnset[replace].move = (enum Move)sDmgMovePool[idx];
        }
        else
        {
            sRandomLearnset[replace].move = MOVE_TACKLE;
        }
    }
}

bool8 MfLearnsetHasDamagingMoveByLevel(const struct LevelUpMove *learnset, u8 maxLevel)
{
    u32 i;
    bool8 anyAtLevel = FALSE;

    if (learnset == NULL)
        return FALSE;

    for (i = 0; learnset[i].move != LEVEL_UP_MOVE_END; i++)
    {
        if (learnset[i].level == 0 || learnset[i].level > maxLevel)
            continue;
        anyAtLevel = TRUE;
        if (MfMoveIsDamaging(learnset[i].move))
            return TRUE;
    }

    if (anyAtLevel)
        return FALSE;

    // No slots at/under maxLevel: require the first real slot to be damaging.
    for (i = 0; learnset[i].move != LEVEL_UP_MOVE_END; i++)
    {
        if (learnset[i].level == 0)
            continue;
        return MfMoveIsDamaging(learnset[i].move);
    }
    return FALSE;
}

const struct LevelUpMove *MfMaybeRandomizeLevelUpLearnset(enum Species species, const struct LevelUpMove *src)
{
    const struct ModernRules *rules;
    u32 seed;

    if (src == NULL)
        return src;

    rules = MfRules_GetActiveRules();
    if (!rules->randomMoves)
        return src;

    species = SanitizeSpeciesId(species);
    seed = rules->randomizerSeed;
    if (sCachedLearnsetValid && sCachedLearnsetSpecies == species && sCachedLearnsetSeed == seed)
        return sRandomLearnset;

    FillRandomLearnset(seed, species, src);
    sCachedLearnsetSpecies = species;
    sCachedLearnsetSeed = seed;
    sCachedLearnsetValid = TRUE;
    return sRandomLearnset;
}

enum Ability MfGetSpeciesAbility(enum Species species, u8 slot)
{
    species = SanitizeSpeciesId(species);
    if (slot >= NUM_ABILITY_SLOTS)
        return ABILITY_NONE;

    if (MfRules_GetActiveRules()->randomAbilities)
        return MfMapAbility(MfRules_GetRandomizerSeed(), species, slot);

    return gSpeciesInfo[species].abilities[slot];
}

#else // !MF_RANDOMIZER

enum Move MfMapMove(u32 seed, enum Species species, enum Move move)
{
    (void)seed;
    (void)species;
    return move;
}

enum Ability MfMapAbility(u32 seed, enum Species species, u8 slot)
{
    (void)seed;
    if (slot >= NUM_ABILITY_SLOTS)
        return ABILITY_NONE;
    return gSpeciesInfo[SanitizeSpeciesId(species)].abilities[slot];
}

const struct LevelUpMove *MfMaybeRandomizeLevelUpLearnset(enum Species species, const struct LevelUpMove *src)
{
    (void)species;
    return src;
}

enum Ability MfGetSpeciesAbility(enum Species species, u8 slot)
{
    if (slot >= NUM_ABILITY_SLOTS)
        return ABILITY_NONE;
    return gSpeciesInfo[SanitizeSpeciesId(species)].abilities[slot];
}

bool8 MfLearnsetHasDamagingMoveByLevel(const struct LevelUpMove *learnset, u8 maxLevel)
{
    u32 i;

    if (learnset == NULL)
        return FALSE;
    for (i = 0; learnset[i].move != LEVEL_UP_MOVE_END; i++)
    {
        if (learnset[i].level == 0 || learnset[i].level > maxLevel)
            continue;
        if (GetMoveCategory(learnset[i].move) != DAMAGE_CATEGORY_STATUS)
            return TRUE;
    }
    return FALSE;
}

#endif // MF_RANDOMIZER
