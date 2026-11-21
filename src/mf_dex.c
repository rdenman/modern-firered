#include "global.h"
#include "pokedex.h"
#include "pokemon.h"
#include "random.h"
#include "mf_dex.h"
#include "mf_evolution.h"
#include "mf_rules.h"
#include "constants/items.h"
#include "constants/moves.h"
#include "constants/regions.h"

// Post-Gen-3 species that only evolve from a regional form.
static const u16 sRegionalLineSpecies[] =
{
    SPECIES_PERRSERKER,
    SPECIES_SIRFETCHD,
    SPECIES_MR_RIME,
    SPECIES_CURSOLA,
    SPECIES_OBSTAGOON,
    SPECIES_RUNERIGUS,
    SPECIES_SNEASLER,
    SPECIES_OVERQWIL,
    SPECIES_CLODSIRE,
};

#define MF_RUN_EVO_MAX 16

static EWRAM_DATA struct Evolution sRunEvos[MF_RUN_EVO_MAX + 1] = {0};
static EWRAM_DATA u8 sListed[NUM_DEX_FLAG_BYTES] = {0};
static EWRAM_DATA u16 sMaxListed = 0;
static EWRAM_DATA u32 sListedKey = 0;
static EWRAM_DATA bool8 sListedValid = FALSE;

static bool8 IsRegionalForm(const struct SpeciesInfo *info)
{
    return info->isAlolanForm
        || info->isGalarianForm
        || info->isHisuianForm
        || info->isPaldeanForm;
}

static bool8 IsRegionalLineSpecies(enum Species species)
{
    u32 i;

    species = GET_BASE_SPECIES_ID(species);
    for (i = 0; i < ARRAY_COUNT(sRegionalLineSpecies); i++)
    {
        if (sRegionalLineSpecies[i] == species)
            return TRUE;
    }
    return FALSE;
}

bool8 MfSpecies_IsInRun(enum Species species)
{
    const struct SpeciesInfo *info;

    if (species == SPECIES_NONE || species >= NUM_SPECIES || !IsSpeciesEnabled(species))
        return FALSE;
    if (!IS_FRLG)
        return TRUE;

    info = &gSpeciesInfo[species];
    if (IsRegionalForm(info))
        return MfRules_HasRegionalForms();
    if (info->natDexNum <= NATIONAL_DEX_DEOXYS)
        return TRUE;
    if (IsRegionalLineSpecies(species))
        return MfRules_HasRegionalForms();
    return MfRules_HasModernEvos();
}

const struct Evolution *MfSpecies_FilterRunEvolutions(const struct Evolution *evolutions)
{
    u32 i;
    u32 count = 0;

    if (evolutions == NULL || !IS_FRLG)
        return evolutions;

    for (i = 0; evolutions[i].method != EVOLUTIONS_END; i++)
    {
        if (!MfSpecies_IsInRun(evolutions[i].targetSpecies))
            break;
    }
    if (evolutions[i].method == EVOLUTIONS_END)
        return evolutions;

    for (i = 0; evolutions[i].method != EVOLUTIONS_END && count < MF_RUN_EVO_MAX; i++)
    {
        if (MfSpecies_IsInRun(evolutions[i].targetSpecies))
            sRunEvos[count++] = evolutions[i];
    }
    sRunEvos[count].method = EVOLUTIONS_END;
    sRunEvos[count].param = 0;
    sRunEvos[count].targetSpecies = SPECIES_NONE;
    sRunEvos[count].params = NULL;
    return sRunEvos;
}

bool8 MfDex_IsActive(void)
{
    return IS_FRLG && MF_RULES_ENGINE;
}

// --- Listing -----------------------------------------------------------------
//
// The National Dex is fixed for the whole run: it depends only on the locked
// rules, never on story choices or catches (ADR 0124). Entries keep their real
// National Dex numbers; numbers outside the run are skipped, not shown blank.

static bool8 BitGet(const u8 *bits, u32 index)
{
    return (bits[index / 8] & (1 << (index % 8))) != 0;
}

static void BitSet(u8 *bits, u32 index)
{
    bits[index / 8] |= 1 << (index % 8);
}

static bool8 SpeciesCanLearnMove(enum Species species, u16 move)
{
    const struct LevelUpMove *levelUp = GetSpeciesLevelUpLearnset(species);
    const u16 *list;
    u32 i;

    if (levelUp != NULL)
    {
        for (i = 0; levelUp[i].move != LEVEL_UP_MOVE_END; i++)
        {
            if (levelUp[i].move == move)
                return TRUE;
        }
    }
    list = GetSpeciesTeachableLearnset(species);
    if (list != NULL)
    {
        for (i = 0; list[i] != MOVE_UNAVAILABLE; i++)
        {
            if (list[i] == move)
                return TRUE;
        }
    }
    list = GetSpeciesEggMoves(species);
    if (list != NULL)
    {
        for (i = 0; list[i] != MOVE_UNAVAILABLE; i++)
        {
            if (list[i] == move)
                return TRUE;
        }
    }
    return FALSE;
}

static bool8 ConditionIsRunAchievable(enum Species species, const struct EvolutionParam *p)
{
    switch (p->condition)
    {
    case IF_HOLD_ITEM:
        return MfEvoItemIsKantoObtainable(p->arg1);
    case IF_KNOWS_MOVE:
    case IF_USED_MOVE_X_TIMES:
        return SpeciesCanLearnMove(species, p->arg1);
    case IF_REGION:
        return p->arg1 == REGION_KANTO;
    case IF_NOT_REGION:
        return p->arg1 != REGION_KANTO;
    case IF_IN_MAP:
    case IF_IN_MAPSEC:
    case IF_WEATHER:
    case IF_MIN_BEAUTY:
    case IF_MIN_COOLNESS:
    case IF_MIN_SMARTNESS:
    case IF_MIN_TOUGHNESS:
    case IF_MIN_CUTENESS:
    case IF_TRADE_PARTNER_SPECIES:
    case IF_DEFEAT_X_WITH_ITEMS:
    case IF_BAG_ITEM_COUNT:
        return FALSE;
    default:
        return TRUE;
    }
}

static bool8 EvolutionIsRunAchievable(enum Species species, const struct Evolution *evo)
{
    u32 i;

    switch (evo->method)
    {
    case EVO_LEVEL:
        if (evo->param > MAX_LEVEL)
            return FALSE;
        break;
    case EVO_ITEM:
        if (!MfEvoItemIsKantoObtainable(evo->param))
            return FALSE;
        break;
    case EVO_TRADE:
    case EVO_SPLIT_FROM_EVO:
    case EVO_LEVEL_BATTLE_ONLY:
    case EVO_BATTLE_END:
        break;
    default:
        return FALSE;
    }

    if (evo->params == NULL)
        return TRUE;
    for (i = 0; evo->params[i].condition != CONDITIONS_END; i++)
    {
        if (!ConditionIsRunAchievable(species, &evo->params[i]))
            return FALSE;
    }
    return TRUE;
}

// Species remaps that hand the player Pokémon (starters, wild, statics, trades,
// evolutions) can produce anything in the randomizer pool, so the whole run
// species set is listed. Trainer-only remaps don't count.
static bool8 RunRandomizesObtainableSpecies(void)
{
    const struct ModernRules *r = MfRules_GetActiveRules();

    if (!MF_RANDOMIZER || !r->randomizerEnabled)
        return FALSE;
    return r->randomStarter
        || r->randomWild
        || r->randomStatic
        || r->randomTrades
        || r->randomEvolution
        || r->randomEvolutionMethods;
}

// Gen 1–3 species are always listed (regional forms only reach the player
// through the randomizer pool).
static bool8 IsCoreSpecies(enum Species species)
{
    const struct SpeciesInfo *info = &gSpeciesInfo[species];

    return info->natDexNum >= NATIONAL_DEX_BULBASAUR
        && info->natDexNum <= NATIONAL_DEX_DEOXYS
        && !IsRegionalForm(info);
}

// Unrandomized runs list post-Gen-3 species only when an achievable evolution
// reaches them from a listed species. Breeding-only babies (Munchlax, Happiny,
// Mime Jr., …) need incense Kanto doesn't have, so they stay out.
static void ReachByEvolution(u8 *reached)
{
    bool8 changed;
    u32 i, j;

    do
    {
        changed = FALSE;
        for (i = SPECIES_BULBASAUR; i < NUM_SPECIES; i++)
        {
            const struct Evolution *evos;

            if (!BitGet(reached, i))
                continue;
            evos = MfGetVanillaSpeciesEvolutions(i);
            if (evos == NULL)
                continue;
            for (j = 0; evos[j].method != EVOLUTIONS_END; j++)
            {
                enum Species target = SanitizeSpeciesId(evos[j].targetSpecies);

                if (BitGet(reached, target) || !MfSpecies_IsInRun(target))
                    continue;
                if (!EvolutionIsRunAchievable(i, &evos[j]))
                    continue;
                BitSet(reached, target);
                changed = TRUE;
            }
        }
    } while (changed);
}

static void RebuildListed(void)
{
    u8 reached[ROUND_BITS_TO_BYTES(NUM_SPECIES)];
    bool8 randomized = RunRandomizesObtainableSpecies();
    u32 i;

    memset(reached, 0, sizeof(reached));
    for (i = SPECIES_BULBASAUR; i < NUM_SPECIES; i++)
    {
        if (MfSpecies_IsInRun(i) && (randomized || IsCoreSpecies(i)))
            BitSet(reached, i);
    }
    if (!randomized)
        ReachByEvolution(reached);

    memset(sListed, 0, sizeof(sListed));
    sMaxListed = 0;
    for (i = SPECIES_BULBASAUR; i < NUM_SPECIES; i++)
    {
        u16 dexNum;

        if (!BitGet(reached, i))
            continue;
        dexNum = gSpeciesInfo[i].natDexNum;
        if (dexNum == NATIONAL_DEX_NONE || dexNum > NATIONAL_DEX_COUNT)
            continue;
        BitSet(sListed, dexNum - 1);
        if (dexNum > sMaxListed)
            sMaxListed = dexNum;
    }
}

// Rules lock at New Game, so this only changes when a different save loads.
static u32 ComputeListedKey(void)
{
    const struct ModernRules *rules = MfRules_GetActiveRules();

    return Crc32B((const u8 *)rules, offsetof(struct ModernRules, nuzlockeEncounterFlags));
}

void MfDex_Refresh(void)
{
    u32 key;

    if (!MfDex_IsActive())
        return;
    key = ComputeListedKey();
    if (sListedValid && key == sListedKey)
        return;
    RebuildListed();
    sListedKey = key;
    sListedValid = TRUE;
}

// --- Queries -----------------------------------------------------------------

bool8 MfDex_IsListed(enum NationalDexOrder nationalNum)
{
    u32 bit;

    if (nationalNum == NATIONAL_DEX_NONE || nationalNum > NATIONAL_DEX_COUNT)
        return FALSE;
    if (!MfDex_IsActive())
        return TRUE;
    if (!sListedValid)
        MfDex_Refresh();
    bit = nationalNum - 1;
    // Caught entries are listed by construction; the caught check is only a
    // safety net (debug gifts) so nothing the player owns is ever hidden.
    return BitGet(sListed, bit) || BitGet(gSaveBlock1Ptr->dexCaught, bit);
}

u8 MfDex_GetNationalDigitCount(void)
{
    if (!MfDex_IsActive())
        return NATIONAL_DEX_COUNT > 999 ? 4 : 3;
    if (!sListedValid)
        MfDex_Refresh();
    return sMaxListed > 999 ? 4 : 3;
}

u16 MfDex_CountListed(u8 caseId)
{
    u32 i;
    u16 count = 0;

    for (i = 1; i <= NATIONAL_DEX_COUNT; i++)
    {
        if (MfDex_IsListed(i) && GetSetPokedexFlag(i, caseId))
            count++;
    }
    return count;
}

bool8 MfDex_HasAllListedCaught(void)
{
    u32 i;

    for (i = 1; i <= NATIONAL_DEX_COUNT; i++)
    {
        enum Species species;

        if (!MfDex_IsListed(i) || GetSetPokedexFlag(i, FLAG_GET_CAUGHT))
            continue;
        species = NationalPokedexNumToSpecies(i);
        if (gSpeciesInfo[species].isMythical && !gSpeciesInfo[species].dexForceRequired)
            continue;
        return FALSE;
    }
    return TRUE;
}
