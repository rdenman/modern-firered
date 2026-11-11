#include "global.h"
#include "event_data.h"
#include "malloc.h"
#include "pokedex.h"
#include "pokemon.h"
#include "random.h"
#include "trade.h"
#include "wild_encounter.h"
#include "mf_dex.h"
#include "mf_encounters.h"
#include "mf_evolution.h"
#include "mf_monotype.h"
#include "mf_party_random.h"
#include "mf_rules.h"
#include "mf_species_map.h"
#include "constants/items.h"
#include "constants/moves.h"
#include "constants/regions.h"
#include "constants/trade.h"

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

// FireRed scripted battles that can be caught (Ghost Marowak cannot).
static const u16 sStaticBattleSpecies[] =
{
    SPECIES_MEWTWO,
    SPECIES_MOLTRES,
    SPECIES_ZAPDOS,
    SPECIES_ARTICUNO,
    SPECIES_ELECTRODE,
    SPECIES_SNORLAX,
    SPECIES_HYPNO,
};

// FireRed givemon gifts that are always on offer: Celadon Eevee, Silph Lapras,
// Route 4 Magikarp, Old Amber, and the FireRed Game Corner prizes.
static const u16 sGiftSpecies[] =
{
    SPECIES_EEVEE,
    SPECIES_LAPRAS,
    SPECIES_MAGIKARP,
    SPECIES_AERODACTYL,
    SPECIES_ABRA,
    SPECIES_CLEFAIRY,
    SPECIES_DRATINI,
    SPECIES_SCYTHER,
    SPECIES_PORYGON,
};

#define MF_RUN_EVO_MAX 16

static EWRAM_DATA struct Evolution sRunEvos[MF_RUN_EVO_MAX + 1] = {0};
static EWRAM_DATA u8 sListed[NUM_DEX_FLAG_BYTES] = {0};
static EWRAM_DATA u32 sListedKey = 0;
static EWRAM_DATA bool8 sListedValid = FALSE;

struct MfDexBuild
{
    u8 obtained[ROUND_BITS_TO_BYTES(NUM_SPECIES)];
    u8 wildDone[ROUND_BITS_TO_BYTES(NUM_SPECIES)];
    u16 preEvo[NUM_SPECIES];
    u16 queue[NUM_SPECIES];
    u16 queueHead;
    u16 queueTail;
    u8 monotype;
    bool8 ditto;
};

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

// --- Obtainability -----------------------------------------------------------

static bool8 BitGet(const u8 *bits, u32 index)
{
    return (bits[index / 8] & (1 << (index % 8))) != 0;
}

static void BitSet(u8 *bits, u32 index)
{
    bits[index / 8] |= 1 << (index % 8);
}

static void Mark(struct MfDexBuild *b, enum Species species)
{
    species = SanitizeSpeciesId(species);
    if (species == SPECIES_NONE || species == SPECIES_EGG || !MfSpecies_IsInRun(species))
        return;
    if (BitGet(b->obtained, species))
        return;
    BitSet(b->obtained, species);
    b->queue[b->queueTail++] = species;
}

static void MarkCatchable(struct MfDexBuild *b, enum Species species)
{
    if (MfRules_IsGiftOnly())
        return;
    if (MfIsMonotypePartyLegalValue(species, b->monotype))
        Mark(b, species);
}

static void BuildPreEvolutions(struct MfDexBuild *b)
{
    u32 i, j;

    for (i = SPECIES_BULBASAUR; i < NUM_SPECIES; i++)
    {
        const struct Evolution *evos;

        if (!MfSpecies_IsInRun(i))
            continue;
        evos = MfGetVanillaSpeciesEvolutions(i);
        if (evos == NULL)
            continue;
        for (j = 0; evos[j].method != EVOLUTIONS_END; j++)
        {
            enum Species target = SanitizeSpeciesId(evos[j].targetSpecies);

            if (MfSpecies_IsInRun(target) && b->preEvo[target] == SPECIES_NONE)
                b->preEvo[target] = i;
        }
    }
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

static void ProcessEvolutions(struct MfDexBuild *b, enum Species species)
{
    const struct Evolution *evos;
    u8 limit = MfRules_GetEvoLimit();
    u32 i;

    if (limit == MF_EVO_LIMIT_NONE)
        return;
    if (limit == MF_EVO_LIMIT_FIRST && b->preEvo[GET_BASE_SPECIES_ID(species)] != SPECIES_NONE)
        return;

    evos = GetSpeciesEvolutions(species);
    if (evos == NULL)
        return;
    for (i = 0; evos[i].method != EVOLUTIONS_END; i++)
    {
        if (EvolutionIsRunAchievable(species, &evos[i]))
            Mark(b, evos[i].targetSpecies);
    }
}

// Day Care eggs hatch the line's base stage (GetEggSpecies). Male-only and
// genderless parents need a Ditto.
static void ProcessBreeding(struct MfDexBuild *b, enum Species species)
{
    const struct SpeciesInfo *info = &gSpeciesInfo[species];
    enum Species egg = species;
    u32 guard;

    if (info->eggGroups[0] == EGG_GROUP_NO_EGGS_DISCOVERED || info->eggGroups[0] == EGG_GROUP_DITTO)
        return;
    if (!b->ditto && (info->genderRatio == MON_GENDERLESS || info->genderRatio == MON_MALE))
        return;
    if (!MfIsMonotypePartyLegalValue(species, b->monotype))
        return;

    for (guard = 0; guard < 5 && b->preEvo[egg] != SPECIES_NONE; guard++)
        egg = b->preEvo[egg];
    if (!MfIsMonotypePartyLegalValue(egg, b->monotype))
        return;

    Mark(b, egg);
    if (egg == SPECIES_NIDORAN_F)
        Mark(b, SPECIES_NIDORAN_M);
    else if (egg == SPECIES_ILLUMISE)
        Mark(b, SPECIES_VOLBEAT);
    else if (P_NIDORAN_M_DITTO_BREED >= GEN_5 && egg == SPECIES_NIDORAN_M)
        Mark(b, SPECIES_NIDORAN_F);
    else if (P_NIDORAN_M_DITTO_BREED >= GEN_5 && egg == SPECIES_VOLBEAT)
        Mark(b, SPECIES_ILLUMISE);
}

static void RunClosure(struct MfDexBuild *b)
{
    while (b->queueHead < b->queueTail)
    {
        enum Species species = b->queue[b->queueHead++];

        ProcessEvolutions(b, species);
        ProcessBreeding(b, species);
    }
}

static bool8 HeaderIsFirstForMap(const struct WildPokemonHeader *headers, u32 index)
{
    u32 i;

    for (i = 0; i < index; i++)
    {
        if (headers[i].mapGroup == headers[index].mapGroup && headers[i].mapNum == headers[index].mapNum)
            return FALSE;
    }
    return TRUE;
}

static void MarkWildList(struct MfDexBuild *b, const struct WildPokemonInfo *info, u32 size, u16 mapsec)
{
    u32 i;

    if (info == NULL || info->wildPokemon == NULL)
        return;
    for (i = 0; i < size; i++)
    {
        enum Species src = SanitizeSpeciesId(info->wildPokemon[i].species);

        if (src == SPECIES_NONE || BitGet(b->wildDone, src))
            continue;
        BitSet(b->wildDone, src);
        MarkCatchable(b, MfWildEncounterSpecies(src, mapsec));
    }
}

// Only the first header per map is ever used (Altering Cave variants need
// Mystery Gift).
static void MarkWildHeaders(struct MfDexBuild *b, const struct WildPokemonHeader *headers, bool8 perMap)
{
    u32 i;

    for (i = 0; headers[i].mapGroup != MAP_GROUP(MAP_UNDEFINED); i++)
    {
        const struct WildEncounterTypes *types = &headers[i].encounterTypes[TIME_OF_DAY_DEFAULT];
        u16 mapsec;

        if (!HeaderIsFirstForMap(headers, i))
            continue;
        if (perMap)
            memset(b->wildDone, 0, sizeof(b->wildDone));
        mapsec = MfWildHeaderMapsec(&headers[i]);
        MarkWildList(b, types->landMonsInfo, NUM_LAND_MONS_ENCOUNTER_SLOTS, mapsec);
        MarkWildList(b, types->waterMonsInfo, NUM_WATER_MONS_ENCOUNTER_SLOTS, mapsec);
        MarkWildList(b, types->rockSmashMonsInfo, NUM_ROCK_SMASH_MONS_ENCOUNTER_SLOTS, mapsec);
        MarkWildList(b, types->fishingMonsInfo, NUM_FISHING_MONS_ENCOUNTER_SLOTS, mapsec);
    }
}

static void MarkWildSeeds(struct MfDexBuild *b)
{
    const struct ModernRules *rules = MfRules_GetActiveRules();
    bool8 perMap = MfSpeciesMap_CategoryRemaps(MF_RANDOM_CAT_WILD) && rules->randomMapBased;
    u8 mode = MfRules_GetAlternateSpawns();
    u32 i;

    if (MfRules_IsGiftOnly())
        return;
    if (MfSpeciesMap_CategoryRemaps(MF_RANDOM_CAT_WILD) && rules->randomChaos)
    {
        for (i = 0; i < MfSpeciesMap_GetPoolCount(); i++)
            MarkCatchable(b, MfSpeciesMap_GetPoolSpecies(i));
        return;
    }

    if (mode != MF_ENCOUNTERS_MODERN)
    {
        memset(b->wildDone, 0, sizeof(b->wildDone));
        MarkWildHeaders(b, MfGetWildMonHeaders(FALSE), perMap);
    }
    if (mode != MF_ENCOUNTERS_VANILLA)
    {
        memset(b->wildDone, 0, sizeof(b->wildDone));
        MarkWildHeaders(b, MfGetWildMonHeaders(TRUE), perMap);
    }
}

static void MarkScriptedSeeds(struct MfDexBuild *b)
{
    enum Species trio[MF_OAK_STARTER_COUNT];
    u32 i;

    for (i = 0; i < ARRAY_COUNT(sStaticBattleSpecies); i++)
        MarkCatchable(b, MfStaticEncounterSpecies(sStaticBattleSpecies[i]));
    for (i = 0; i < ARRAY_COUNT(sGiftSpecies); i++)
        Mark(b, MfStaticEncounterSpecies(sGiftSpecies[i]));
    Mark(b, SPECIES_TOGEPI);

    if (!FlagGet(FLAG_GOT_FOSSIL_FROM_MT_MOON) || FlagGet(FLAG_GOT_HELIX_FOSSIL))
        Mark(b, MfStaticEncounterSpecies(SPECIES_OMANYTE));
    if (!FlagGet(FLAG_GOT_FOSSIL_FROM_MT_MOON) || FlagGet(FLAG_GOT_DOME_FOSSIL))
        Mark(b, MfStaticEncounterSpecies(SPECIES_KABUTO));
    if (!FlagGet(FLAG_GOT_HITMON_FROM_DOJO))
    {
        Mark(b, MfStaticEncounterSpecies(SPECIES_HITMONLEE));
        Mark(b, MfStaticEncounterSpecies(SPECIES_HITMONCHAN));
    }
    if (!FlagGet(FLAG_SYS_POKEMON_GET))
    {
        if (MfRules_IsMonotypeActive())
            MfFillMonotypeOakStarters(MfRules_GetMonotype(), trio);
        else
            MfFillRandomOakStarters(trio);
        for (i = 0; i < MF_OAK_STARTER_COUNT; i++)
            Mark(b, trio[i]);
    }

    for (i = INGAME_TRADE_MR_MIME; i <= INGAME_TRADE_SEEL; i++)
        Mark(b, GetInGameTradeOfferedSpecies(i));

    for (i = 0; i < ARRAY_COUNT(gSaveBlock1Ptr->roamer); i++)
    {
        if (gSaveBlock1Ptr->roamer[i].active)
            MarkCatchable(b, gSaveBlock1Ptr->roamer[i].species);
    }
}

static void MarkCaughtSeeds(struct MfDexBuild *b)
{
    u32 i;

    for (i = 1; i <= NATIONAL_DEX_COUNT; i++)
    {
        if (GetSetPokedexFlag(i, FLAG_GET_CAUGHT))
            Mark(b, NationalPokedexNumToSpecies(i));
    }
}

static void ListSpecies(enum Species species)
{
    u16 dexNum = gSpeciesInfo[species].natDexNum;

    if (dexNum != NATIONAL_DEX_NONE && dexNum <= NATIONAL_DEX_COUNT)
        BitSet(sListed, dexNum - 1);
}

static void RebuildListed(void)
{
    struct MfDexBuild *b = AllocZeroed(sizeof(*b));
    u32 i;
    u16 resumeFrom;

    memset(sListed, 0, sizeof(sListed));
    if (b == NULL)
    {
        for (i = SPECIES_BULBASAUR; i < NUM_SPECIES; i++)
        {
            if (MfSpecies_IsInRun(i))
                ListSpecies(i);
        }
        return;
    }

    b->monotype = MfRules_GetMonotype();
    BuildPreEvolutions(b);
    MarkWildSeeds(b);
    MarkScriptedSeeds(b);
    MarkCaughtSeeds(b);

    b->ditto = BitGet(b->obtained, SPECIES_DITTO);
    RunClosure(b);
    if (!b->ditto && BitGet(b->obtained, SPECIES_DITTO))
    {
        // Ditto turned up mid-pass: retry breeding for everything already seen.
        b->ditto = TRUE;
        resumeFrom = b->queueHead;
        for (i = 0; i < resumeFrom; i++)
            ProcessBreeding(b, b->queue[i]);
        RunClosure(b);
    }

    for (i = 0; i < b->queueTail; i++)
        ListSpecies(b->queue[i]);
    Free(b);
}

static u32 ComputeListedKey(void)
{
    const struct ModernRules *rules = MfRules_GetActiveRules();
    u32 key = Crc32B((const u8 *)rules, offsetof(struct ModernRules, nuzlockeEncounterFlags));
    u32 i;

    key = key * 31 + Crc32B(gSaveBlock1Ptr->dexCaught, NUM_DEX_FLAG_BYTES);
    key = key * 31 + (FlagGet(FLAG_SYS_POKEMON_GET)
                   | (FlagGet(FLAG_GOT_FOSSIL_FROM_MT_MOON) << 1)
                   | (FlagGet(FLAG_GOT_HELIX_FOSSIL) << 2)
                   | (FlagGet(FLAG_GOT_DOME_FOSSIL) << 3)
                   | (FlagGet(FLAG_GOT_HITMON_FROM_DOJO) << 4));
    for (i = 0; i < ARRAY_COUNT(gSaveBlock1Ptr->roamer); i++)
        key = key * 31 + (gSaveBlock1Ptr->roamer[i].active ? gSaveBlock1Ptr->roamer[i].species : 0);
    return key;
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

static u32 CountBits(u8 value)
{
    u32 count = 0;

    while (value != 0)
    {
        value &= value - 1;
        count++;
    }
    return count;
}

static u8 ListedByte(u32 index)
{
    return sListed[index] | gSaveBlock1Ptr->dexCaught[index];
}

bool8 MfDex_IsListed(enum NationalDexOrder nationalNum)
{
    if (nationalNum == NATIONAL_DEX_NONE || nationalNum > NATIONAL_DEX_COUNT)
        return FALSE;
    if (!MfDex_IsActive())
        return TRUE;
    if (!sListedValid)
        MfDex_Refresh();
    return (ListedByte((nationalNum - 1) / 8) & (1 << ((nationalNum - 1) % 8))) != 0;
}

u16 MfDex_GetRunNumber(enum NationalDexOrder nationalNum)
{
    u32 bit;
    u32 i;
    u16 count = 0;

    if (!MfDex_IsListed(nationalNum))
        return 0;
    if (!MfDex_IsActive())
        return nationalNum;

    bit = nationalNum - 1;
    for (i = 0; i < bit / 8; i++)
        count += CountBits(ListedByte(i));
    count += CountBits(ListedByte(bit / 8) & ((1 << (bit % 8)) - 1));
    return count + 1;
}

u16 MfDex_GetCount(void)
{
    u32 i;
    u16 count = 0;

    if (!MfDex_IsActive())
        return NATIONAL_DEX_COUNT;
    if (!sListedValid)
        MfDex_Refresh();
    for (i = 0; i < NUM_DEX_FLAG_BYTES; i++)
        count += CountBits(ListedByte(i));
    return count;
}

u8 MfDex_GetNationalDigitCount(void)
{
    return MfDex_GetCount() > 999 ? 4 : 3;
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
