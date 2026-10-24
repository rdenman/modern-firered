#include "global.h"
#include "item.h"
#include "mf_evolution.h"
#include "mf_moves.h"
#include "mf_move_ability.h"
#include "mf_rules.h"
#include "pokemon.h"
#include "constants/items.h"
#include "constants/pokemon.h"

// Sparse stone evo: later-level entries below this count inherit the pre-evo
// table. Full post-evo lines (Vaporeon, Sunflora) sit well above it.
#define MF_STONE_OWN_MOVESET_LATER_MIN 4
#define MF_STONE_PRE_EVO_MAX 192
#define MF_STONE_LEARNSET_MAX 72

struct MfStonePreEvo
{
    u16 target;
    u16 preEvo;
};

struct MfClassicLevelUpPtr
{
    u16 species;
    const struct LevelUpMove *learnset;
};

struct MfClassicU16LearnsetPtr
{
    u16 species;
    const u16 *learnset;
};

#include "data/mf_classic_level_up_learnsets.h"
#include "data/mf_classic_teachable_learnsets.h"
#include "data/mf_classic_egg_moves.h"

static EWRAM_DATA struct MfStonePreEvo sStonePreEvos[MF_STONE_PRE_EVO_MAX] = {0};
static EWRAM_DATA u16 sStonePreEvoCount = 0;
static EWRAM_DATA bool8 sStonePreEvoMapReady = FALSE;

static EWRAM_DATA struct LevelUpMove sStoneMergedLearnset[MF_STONE_LEARNSET_MAX + 1] = {0};
static EWRAM_DATA u16 sStoneMergedSpecies = SPECIES_NONE;
static EWRAM_DATA bool8 sStoneMergedModern = FALSE;
static EWRAM_DATA bool8 sStoneMergedValid = FALSE;

static const struct LevelUpMove *FindClassicLevelUp(enum Species species)
{
    s32 lo = 0;
    s32 hi = (s32)ARRAY_COUNT(sClassicLevelUpLearnsets) - 1;

    while (lo <= hi)
    {
        s32 mid = (lo + hi) / 2;
        u16 midSpecies = sClassicLevelUpLearnsets[mid].species;

        if (midSpecies == species)
            return sClassicLevelUpLearnsets[mid].learnset;
        if (midSpecies < species)
            lo = mid + 1;
        else
            hi = mid - 1;
    }
    return NULL;
}

static const u16 *FindClassicU16Learnset(const struct MfClassicU16LearnsetPtr *table, u32 count, enum Species species)
{
    s32 lo = 0;
    s32 hi = (s32)count - 1;

    while (lo <= hi)
    {
        s32 mid = (lo + hi) / 2;
        u16 midSpecies = table[mid].species;

        if (midSpecies == species)
            return table[mid].learnset;
        if (midSpecies < species)
            lo = mid + 1;
        else
            hi = mid - 1;
    }
    return NULL;
}

static const struct LevelUpMove *ModernLevelUpLearnset(enum Species species)
{
    const struct LevelUpMove *learnset = gSpeciesInfo[species].levelUpLearnset;

    if (learnset == NULL)
        return gSpeciesInfo[SPECIES_NONE].levelUpLearnset;
    return learnset;
}

static const u16 *ModernTeachableLearnset(enum Species species)
{
    const u16 *learnset = gSpeciesInfo[species].teachableLearnset;

    if (learnset == NULL)
        return gSpeciesInfo[SPECIES_NONE].teachableLearnset;
    return learnset;
}

static const u16 *ModernEggMoves(enum Species species)
{
    const u16 *learnset = gSpeciesInfo[species].eggMoveLearnset;

    if (learnset == NULL)
        return gSpeciesInfo[SPECIES_NONE].eggMoveLearnset;
    return learnset;
}

static const struct LevelUpMove *RawLevelUpLearnset(enum Species species)
{
    const struct LevelUpMove *classic;

    if (MfRules_HasModernMoves())
        return ModernLevelUpLearnset(species);

    classic = FindClassicLevelUp(species);
    if (classic != NULL)
        return classic;
    return ModernLevelUpLearnset(species);
}

static bool8 ItemIsEvolutionStone(u16 item)
{
    if (item == ITEM_NONE || item >= ITEMS_COUNT)
        return FALSE;
    return gItemsInfo[item].sortType == ITEM_TYPE_EVOLUTION_STONE;
}

static void SortStonePreEvos(void)
{
    u32 i, j;

    for (i = 1; i < sStonePreEvoCount; i++)
    {
        struct MfStonePreEvo key = sStonePreEvos[i];

        for (j = i; j > 0 && sStonePreEvos[j - 1].target > key.target; j--)
            sStonePreEvos[j] = sStonePreEvos[j - 1];
        sStonePreEvos[j] = key;
    }
}

static void RecordStonePreEvo(enum Species target, enum Species preEvo)
{
    u32 i;

    if (target == SPECIES_NONE || preEvo == SPECIES_NONE)
        return;

    for (i = 0; i < sStonePreEvoCount; i++)
    {
        if (sStonePreEvos[i].target == target)
        {
            // Prefer the lowest species id (base form over costume forms).
            if (preEvo < sStonePreEvos[i].preEvo)
                sStonePreEvos[i].preEvo = preEvo;
            return;
        }
    }

    if (sStonePreEvoCount >= MF_STONE_PRE_EVO_MAX)
        return;

    sStonePreEvos[sStonePreEvoCount].target = target;
    sStonePreEvos[sStonePreEvoCount].preEvo = preEvo;
    sStonePreEvoCount++;
}

static void EnsureStonePreEvoMap(void)
{
    enum Species from;
    u32 i;
    const struct Evolution *evolutions;

    if (sStonePreEvoMapReady)
        return;

    sStonePreEvoCount = 0;
    for (from = SPECIES_BULBASAUR; from < NUM_SPECIES; from++)
    {
        if (!IsSpeciesEnabled(from))
            continue;
        evolutions = MfGetVanillaSpeciesEvolutions(from);
        if (evolutions == NULL)
            continue;
        for (i = 0; evolutions[i].method != EVOLUTIONS_END; i++)
        {
            enum Species target = evolutions[i].targetSpecies;

            if (evolutions[i].method != EVO_ITEM)
                continue;
            if (!ItemIsEvolutionStone(evolutions[i].param))
                continue;
            if (!IsSpeciesEnabled(target))
                continue;
            RecordStonePreEvo(target, from);
        }
    }
    SortStonePreEvos();
    sStonePreEvoMapReady = TRUE;
}

static enum Species FindStonePreEvo(enum Species species)
{
    s32 lo;
    s32 hi;

    EnsureStonePreEvoMap();
    lo = 0;
    hi = (s32)sStonePreEvoCount - 1;
    while (lo <= hi)
    {
        s32 mid = (lo + hi) / 2;
        u16 midTarget = sStonePreEvos[mid].target;

        if (midTarget == species)
            return sStonePreEvos[mid].preEvo;
        if (midTarget < species)
            lo = mid + 1;
        else
            hi = mid - 1;
    }
    return SPECIES_NONE;
}

static u32 CountLaterLevelMoves(const struct LevelUpMove *learnset)
{
    u32 i;
    u32 later = 0;

    if (learnset == NULL)
        return 0;
    for (i = 0; learnset[i].move != LEVEL_UP_MOVE_END; i++)
    {
        if (learnset[i].level > 1)
            later++;
    }
    return later;
}

static bool8 LearnsetHasExact(const struct LevelUpMove *learnset, enum Move move, u16 level)
{
    u32 i;

    for (i = 0; learnset[i].move != LEVEL_UP_MOVE_END; i++)
    {
        if (learnset[i].move == move && learnset[i].level == level)
            return TRUE;
    }
    return FALSE;
}

static bool8 LearnsetHasMoveAnyLevel(const struct LevelUpMove *learnset, enum Move move)
{
    u32 i;

    for (i = 0; learnset[i].move != LEVEL_UP_MOVE_END; i++)
    {
        if (learnset[i].move == move)
            return TRUE;
    }
    return FALSE;
}

static bool8 StoneEvoMoveIsExtra(const struct LevelUpMove *pre, enum Move move, u16 level)
{
    if (LearnsetHasExact(pre, move, level))
        return FALSE;
    // SV dumps the pre-evo movepool at level 1; skip those copies.
    if (level == 1 && LearnsetHasMoveAnyLevel(pre, move))
        return FALSE;
    return TRUE;
}

static void SortMergedLearnset(struct LevelUpMove *learnset, u32 count)
{
    u32 i, j;

    for (i = 1; i < count; i++)
    {
        struct LevelUpMove key = learnset[i];

        for (j = i; j > 0 && learnset[j - 1].level > key.level; j--)
            learnset[j] = learnset[j - 1];
        learnset[j] = key;
    }
}

static const struct LevelUpMove *MaybeInheritStoneLearnset(enum Species species, const struct LevelUpMove *evoLearnset)
{
    enum Species preEvo;
    const struct LevelUpMove *preLearnset;
    u32 count = 0;
    u32 i;
    bool8 modern = MfRules_HasModernMoves();

    if (evoLearnset == NULL)
        return evoLearnset;

    preEvo = FindStonePreEvo(species);
    if (preEvo == SPECIES_NONE)
        return evoLearnset;

    // Real post-evo lines (Eevee, Sunflora, Gallade, …) keep their own table.
    if (CountLaterLevelMoves(evoLearnset) >= MF_STONE_OWN_MOVESET_LATER_MIN)
        return evoLearnset;

    if (sStoneMergedValid && sStoneMergedSpecies == species && sStoneMergedModern == modern)
        return sStoneMergedLearnset;

    preLearnset = RawLevelUpLearnset(preEvo);
    if (preLearnset == NULL)
        return evoLearnset;

    for (i = 0; preLearnset[i].move != LEVEL_UP_MOVE_END && count < MF_STONE_LEARNSET_MAX; i++)
        sStoneMergedLearnset[count++] = preLearnset[i];

    for (i = 0; evoLearnset[i].move != LEVEL_UP_MOVE_END && count < MF_STONE_LEARNSET_MAX; i++)
    {
        if (!StoneEvoMoveIsExtra(preLearnset, evoLearnset[i].move, evoLearnset[i].level))
            continue;
        sStoneMergedLearnset[count++] = evoLearnset[i];
    }

    SortMergedLearnset(sStoneMergedLearnset, count);
    sStoneMergedLearnset[count].move = LEVEL_UP_MOVE_END;
    sStoneMergedLearnset[count].level = 0;
    sStoneMergedSpecies = species;
    sStoneMergedModern = modern;
    sStoneMergedValid = TRUE;
    return sStoneMergedLearnset;
}

const struct LevelUpMove *MfGetSpeciesLevelUpLearnset(enum Species species)
{
    enum Species sanitized = SanitizeSpeciesId(species);
    const struct LevelUpMove *raw = RawLevelUpLearnset(sanitized);

    return MfMaybeRandomizeLevelUpLearnset(sanitized, MaybeInheritStoneLearnset(sanitized, raw));
}

const u16 *MfGetSpeciesTeachableLearnset(enum Species species)
{
    enum Species sanitized = SanitizeSpeciesId(species);
    const u16 *classic;

    if (MfRules_HasModernMoves())
        return ModernTeachableLearnset(sanitized);

    classic = FindClassicU16Learnset(sClassicTeachableLearnsets,
                                     ARRAY_COUNT(sClassicTeachableLearnsets),
                                     sanitized);
    if (classic != NULL)
        return classic;

    return ModernTeachableLearnset(sanitized);
}

const u16 *MfGetSpeciesEggMoves(enum Species species)
{
    enum Species sanitized = SanitizeSpeciesId(species);
    const u16 *classic;

    if (MfRules_HasModernMoves())
        return ModernEggMoves(sanitized);

    classic = FindClassicU16Learnset(sClassicEggMoveLearnsets,
                                     ARRAY_COUNT(sClassicEggMoveLearnsets),
                                     sanitized);
    if (classic != NULL)
        return classic;

    return ModernEggMoves(sanitized);
}
