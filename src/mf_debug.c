#include "global.h"
#include "debug.h"
#include "mf_debug.h"
#include "mf_qa_matrix.h"
#include "mf_iv_ev.h"
#include "mf_level_cap.h"
#include "mf_nuzlocke.h"
#include "mf_rules.h"
#include "mf_rules_conflicts.h"
#include "mf_rules_menu.h"
#include "mf_options.h"
#include "mf_options_menu.h"
#include "mf_shiny.h"
#include "mf_species_map.h"
#include "mf_move_ability.h"
#include "mf_encounters.h"
#include "mf_party_random.h"
#include "mf_stats.h"
#include "mf_types.h"
#include "mf_items.h"
#include "battle_util.h"
#include "event_data.h"
#include "field_player_avatar.h"
#include "field_screen_effect.h"
#include "item.h"
#include "main.h"
#include "overworld.h"
#include "pokemon.h"
#include "random.h"
#include "script.h"
#include "sound.h"
#include "string_util.h"
#include "constants/flags.h"
#include "constants/items.h"
#include "constants/maps.h"
#include "constants/songs.h"
#include "constants/vars.h"
#include "gba/isagbprint.h"

// S17 — paged rules inspector under overworld debug → Modern FireRed…
// Live labels rebuild into EWRAM (DEBUG_MAX_MENU_ITEMS = 20; name width 26).
// Under NDEBUG / make release the inspector compiles out (menu is Cancel-only).

#if !defined(NDEBUG) && MF_RULES_ENGINE

#define MF_DEBUG_NAME_WIDTH 26
#define MF_DEBUG_PAGE_SLOTS 20

enum MfDebugPage
{
    MF_DEBUG_PAGE_META,
    MF_DEBUG_PAGE_GAMEMODE,
    MF_DEBUG_PAGE_FEATURES,
    MF_DEBUG_PAGE_RANDOMIZER,
    MF_DEBUG_PAGE_NUZLOCKE,
    MF_DEBUG_PAGE_DIFFICULTY,
    MF_DEBUG_PAGE_CHALLENGES,
    MF_DEBUG_PAGE_COUNT,
};

enum MfDebugEntryKind
{
    MF_DEBUG_KIND_BOOL,
    MF_DEBUG_KIND_VALUE,
    MF_DEBUG_KIND_UNLOCK,   // session override
    MF_DEBUG_KIND_SEED,     // reroll randomizerSeed
    MF_DEBUG_KIND_VERSION,  // read-only
};

struct MfDebugEntry
{
    const u8 *label;
    u8 kind;
    u8 id;    // MfRuleBool / MfRuleValue when applicable
    u8 max;   // value cycle max (inclusive)
};

static EWRAM_DATA u8 sMfDebugNames[MF_DEBUG_PAGE_SLOTS][MF_DEBUG_NAME_WIDTH];
static EWRAM_DATA struct DebugMenuOption sMfDebugPageOptions[MF_DEBUG_PAGE_SLOTS + 1];
static EWRAM_DATA enum MfDebugPage sMfDebugCurrentPage;

static void MfDebug_Action_ToggleEntry(u8 taskId, void *params);

static const struct MfDebugEntry sMetaEntries[] =
{
    { COMPOUND_STRING("Locked"),   MF_DEBUG_KIND_BOOL,    MF_RULE_BOOL_RULES_LOCKED, 0 },
    { COMPOUND_STRING("Unlock"),   MF_DEBUG_KIND_UNLOCK,  0, 0 },
    { COMPOUND_STRING("LockDiff"), MF_DEBUG_KIND_BOOL,    MF_RULE_BOOL_LOCK_DIFFICULTY, 0 },
    { COMPOUND_STRING("Preset"),   MF_DEBUG_KIND_VALUE,   MF_RULE_VAL_GAMEMODE_PRESET, 3 },
    { COMPOUND_STRING("Version"),  MF_DEBUG_KIND_VERSION, 0, 0 },
    { COMPOUND_STRING("RerollSeed"), MF_DEBUG_KIND_SEED,  0, 0 },
};

static const struct MfDebugEntry sGamemodeEntries[] =
{
    { COMPOUND_STRING("InfTMs"),     MF_DEBUG_KIND_BOOL,  MF_RULE_BOOL_INFINITE_TMS, 0 },
    { COMPOUND_STRING("SurvPsn"),    MF_DEBUG_KIND_BOOL,  MF_RULE_BOOL_SURVIVE_POISON, 0 },
    { COMPOUND_STRING("Sync"),      MF_DEBUG_KIND_BOOL,  MF_RULE_BOOL_SYNCHRONIZE, 0 },
    { COMPOUND_STRING("Mints"),     MF_DEBUG_KIND_BOOL,  MF_RULE_BOOL_MINTS, 0 },
    { COMPOUND_STRING("Sitrus"),    MF_DEBUG_KIND_BOOL,  MF_RULE_BOOL_MODERN_SITRUS, 0 },
    { COMPOUND_STRING("Types"),     MF_DEBUG_KIND_BOOL,  MF_RULE_BOOL_MODERN_TYPES, 0 },
    { COMPOUND_STRING("Fairy"),     MF_DEBUG_KIND_BOOL,  MF_RULE_BOOL_FAIRY_TYPES, 0 },
    { COMPOUND_STRING("Stats"),     MF_DEBUG_KIND_BOOL,  MF_RULE_BOOL_MODERN_STATS, 0 },
    { COMPOUND_STRING("Sturdy"),    MF_DEBUG_KIND_BOOL,  MF_RULE_BOOL_STURDY, 0 },
    { COMPOUND_STRING("Moves"),     MF_DEBUG_KIND_BOOL,  MF_RULE_BOOL_MODERN_MOVES, 0 },
    { COMPOUND_STRING("LegAbil"),   MF_DEBUG_KIND_BOOL,  MF_RULE_BOOL_LEGENDARY_ABILITIES, 0 },
    { COMPOUND_STRING("NewLeg"),    MF_DEBUG_KIND_BOOL,  MF_RULE_BOOL_NEW_LEGENDARIES, 0 },
    { COMPOUND_STRING("TypeEff"),   MF_DEBUG_KIND_BOOL,  MF_RULE_BOOL_TYPE_EFFECTIVENESS, 0 },
    { COMPOUND_STRING("Spawns"),    MF_DEBUG_KIND_VALUE, MF_RULE_VAL_ALTERNATE_SPAWNS, 3 },
};

static const struct MfDebugEntry sFeaturesEntries[] =
{
    { COMPOUND_STRING("ShinyCh"),   MF_DEBUG_KIND_VALUE, MF_RULE_VAL_SHINY_CHANCE, 5 },
    { COMPOUND_STRING("Drops"),     MF_DEBUG_KIND_BOOL,  MF_RULE_BOOL_WILD_ITEM_DROPS, 0 },
    { COMPOUND_STRING("Feebas"),    MF_DEBUG_KIND_BOOL,  MF_RULE_BOOL_EASIER_FEEBAS, 0 },
    { COMPOUND_STRING("RTC"),       MF_DEBUG_KIND_BOOL,  MF_RULE_BOOL_RTC_TYPE, 0 },
    { COMPOUND_STRING("ShinyCol"),  MF_DEBUG_KIND_BOOL,  MF_RULE_BOOL_SHINY_COLORS, 0 },
    { COMPOUND_STRING("WTrade"),    MF_DEBUG_KIND_BOOL,  MF_RULE_BOOL_WONDER_TRADE, 0 },
    { COMPOUND_STRING("UnlimWT"),   MF_DEBUG_KIND_BOOL,  MF_RULE_BOOL_UNLIMITED_WONDER_TRADE, 0 },
    { COMPOUND_STRING("FrBans"),    MF_DEBUG_KIND_BOOL,  MF_RULE_BOOL_FRONTIER_BANS, 0 },
    { COMPOUND_STRING("Regionals"), MF_DEBUG_KIND_BOOL,  MF_RULE_BOOL_REGIONAL_FORMS, 0 },
};

static const struct MfDebugEntry sRandomizerEntries[] =
{
    { COMPOUND_STRING("RMaster"),   MF_DEBUG_KIND_BOOL, MF_RULE_BOOL_RANDOMIZER_ENABLED, 0 },
    { COMPOUND_STRING("RStart"),    MF_DEBUG_KIND_BOOL, MF_RULE_BOOL_RANDOM_STARTER, 0 },
    { COMPOUND_STRING("RWild"),     MF_DEBUG_KIND_BOOL, MF_RULE_BOOL_RANDOM_WILD, 0 },
    { COMPOUND_STRING("RTrain"),    MF_DEBUG_KIND_BOOL, MF_RULE_BOOL_RANDOM_TRAINER, 0 },
    { COMPOUND_STRING("RTheme"),    MF_DEBUG_KIND_BOOL, MF_RULE_BOOL_RANDOM_THEMED_TRAINERS, 0 },
    { COMPOUND_STRING("RStatic"),   MF_DEBUG_KIND_BOOL, MF_RULE_BOOL_RANDOM_STATIC, 0 },
    { COMPOUND_STRING("RSimilar"),  MF_DEBUG_KIND_BOOL, MF_RULE_BOOL_RANDOM_SIMILAR, 0 },
    { COMPOUND_STRING("RMap"),      MF_DEBUG_KIND_BOOL, MF_RULE_BOOL_RANDOM_MAP_BASED, 0 },
    { COMPOUND_STRING("RLegs"),     MF_DEBUG_KIND_BOOL, MF_RULE_BOOL_RANDOM_INCLUDE_LEGENDARIES, 0 },
    { COMPOUND_STRING("RType"),     MF_DEBUG_KIND_BOOL, MF_RULE_BOOL_RANDOM_TYPE, 0 },
    { COMPOUND_STRING("RMoves"),    MF_DEBUG_KIND_BOOL, MF_RULE_BOOL_RANDOM_MOVES, 0 },
    { COMPOUND_STRING("RAbil"),     MF_DEBUG_KIND_BOOL, MF_RULE_BOOL_RANDOM_ABILITIES, 0 },
    { COMPOUND_STRING("REvo"),      MF_DEBUG_KIND_BOOL, MF_RULE_BOOL_RANDOM_EVOLUTION, 0 },
    { COMPOUND_STRING("REvoM"),     MF_DEBUG_KIND_BOOL, MF_RULE_BOOL_RANDOM_EVOLUTION_METHODS, 0 },
    { COMPOUND_STRING("RTypeE"),    MF_DEBUG_KIND_BOOL, MF_RULE_BOOL_RANDOM_TYPE_EFFECTIVENESS, 0 },
    { COMPOUND_STRING("RItems"),    MF_DEBUG_KIND_BOOL, MF_RULE_BOOL_RANDOM_ITEMS, 0 },
    { COMPOUND_STRING("RChaos"),    MF_DEBUG_KIND_BOOL, MF_RULE_BOOL_RANDOM_CHAOS, 0 },
};

static const struct MfDebugEntry sNuzlockeEntries[] =
{
    { COMPOUND_STRING("Nuzlocke"),  MF_DEBUG_KIND_BOOL, MF_RULE_BOOL_NUZLOCKE, 0 },
    { COMPOUND_STRING("Hardcore"),  MF_DEBUG_KIND_BOOL, MF_RULE_BOOL_NUZLOCKE_HARDCORE, 0 },
    { COMPOUND_STRING("Easy"),      MF_DEBUG_KIND_BOOL, MF_RULE_BOOL_NUZLOCKE_EASY, 0 },
    { COMPOUND_STRING("Species"),   MF_DEBUG_KIND_BOOL, MF_RULE_BOOL_NUZLOCKE_SPECIES_CLAUSE, 0 },
    { COMPOUND_STRING("ShinyCl"),   MF_DEBUG_KIND_BOOL, MF_RULE_BOOL_NUZLOCKE_SHINY_CLAUSE, 0 },
    { COMPOUND_STRING("Nickname"),  MF_DEBUG_KIND_BOOL, MF_RULE_BOOL_NUZLOCKE_NICKNAMING, 0 },
    { COMPOUND_STRING("Deletion"),  MF_DEBUG_KIND_BOOL, MF_RULE_BOOL_NUZLOCKE_DELETION, 0 },
};

static const struct MfDebugEntry sDifficultyEntries[] =
{
    { COMPOUND_STRING("PartyLim"),  MF_DEBUG_KIND_VALUE, MF_RULE_VAL_PARTY_LIMIT, 7 },
    { COMPOUND_STRING("LvlCap"),    MF_DEBUG_KIND_VALUE, MF_RULE_VAL_LEVEL_CAP, 3 },
    { COMPOUND_STRING("ExpMult"),   MF_DEBUG_KIND_VALUE, MF_RULE_VAL_EXP_MULTIPLIER, 3 },
    { COMPOUND_STRING("ScaledExp"), MF_DEBUG_KIND_BOOL,  MF_RULE_BOOL_SCALED_EXP, 0 },
    { COMPOUND_STRING("NoItemP"),   MF_DEBUG_KIND_BOOL,  MF_RULE_BOOL_NO_ITEM_PLAYER, 0 },
    { COMPOUND_STRING("NoItemT"),   MF_DEBUG_KIND_BOOL,  MF_RULE_BOOL_NO_ITEM_TRAINER, 0 },
    { COMPOUND_STRING("NoEVs"),     MF_DEBUG_KIND_BOOL,  MF_RULE_BOOL_NO_EVS, 0 },
    { COMPOUND_STRING("ScaleIV"),   MF_DEBUG_KIND_VALUE, MF_RULE_VAL_SCALING_IVS, 3 },
    { COMPOUND_STRING("ScaleEV"),   MF_DEBUG_KIND_VALUE, MF_RULE_VAL_SCALING_EVS, 3 },
    { COMPOUND_STRING("MaxPIV"),    MF_DEBUG_KIND_VALUE, MF_RULE_VAL_MAX_PARTY_IVS, 3 },
    { COMPOUND_STRING("RopeDig"),   MF_DEBUG_KIND_BOOL,  MF_RULE_BOOL_ESCAPE_ROPE_DIG, 0 },
    { COMPOUND_STRING("HardExp"),   MF_DEBUG_KIND_BOOL,  MF_RULE_BOOL_HARD_EXP, 0 },
    { COMPOUND_STRING("Catch"),     MF_DEBUG_KIND_VALUE, MF_RULE_VAL_CATCH_RATE, 7 },
    { COMPOUND_STRING("LessEsc"),   MF_DEBUG_KIND_BOOL,  MF_RULE_BOOL_LESS_ESCAPES, 0 },
};

static const struct MfDebugEntry sChallengeEntries[] =
{
    { COMPOUND_STRING("PCLimit"),   MF_DEBUG_KIND_VALUE, MF_RULE_VAL_POKECENTER_LIMIT, 3 },
    { COMPOUND_STRING("EvoLim"),    MF_DEBUG_KIND_VALUE, MF_RULE_VAL_EVO_LIMIT, 3 },
    { COMPOUND_STRING("BSE"),       MF_DEBUG_KIND_VALUE, MF_RULE_VAL_BASE_STAT_EQUALIZER, 3 },
    { COMPOUND_STRING("Mirror"),    MF_DEBUG_KIND_BOOL,  MF_RULE_BOOL_MIRROR, 0 },
    { COMPOUND_STRING("Thief"),     MF_DEBUG_KIND_BOOL,  MF_RULE_BOOL_MIRROR_THIEF, 0 },
    { COMPOUND_STRING("NoPCHeal"),  MF_DEBUG_KIND_BOOL,  MF_RULE_BOOL_NO_PC_HEAL, 0 },
    { COMPOUND_STRING("Monotype"),  MF_DEBUG_KIND_VALUE, MF_RULE_VAL_MONOTYPE, 31 },
    { COMPOUND_STRING("Shops"),     MF_DEBUG_KIND_VALUE, MF_RULE_VAL_EXPENSIVE_SHOPS, 7 },
};

struct MfDebugPageDef
{
    const struct MfDebugEntry *entries;
    u8 count;
};

static const struct MfDebugPageDef sPages[MF_DEBUG_PAGE_COUNT] =
{
    [MF_DEBUG_PAGE_META]        = { sMetaEntries,        ARRAY_COUNT(sMetaEntries) },
    [MF_DEBUG_PAGE_GAMEMODE]    = { sGamemodeEntries,    ARRAY_COUNT(sGamemodeEntries) },
    [MF_DEBUG_PAGE_FEATURES]    = { sFeaturesEntries,    ARRAY_COUNT(sFeaturesEntries) },
    [MF_DEBUG_PAGE_RANDOMIZER]  = { sRandomizerEntries,  ARRAY_COUNT(sRandomizerEntries) },
    [MF_DEBUG_PAGE_NUZLOCKE]    = { sNuzlockeEntries,    ARRAY_COUNT(sNuzlockeEntries) },
    [MF_DEBUG_PAGE_DIFFICULTY]  = { sDifficultyEntries,  ARRAY_COUNT(sDifficultyEntries) },
    [MF_DEBUG_PAGE_CHALLENGES]  = { sChallengeEntries,   ARRAY_COUNT(sChallengeEntries) },
};

static bool8 MfDebug_EnsureWritable(void)
{
    if (MfRules_DebugHasUnlockOverride())
        return TRUE;
    return MfRules_DebugSetUnlockOverride(TRUE);
}

static bool8 MfDebug_TrySetBool(enum MfRuleBool id, bool8 value)
{
    if (MfRules_TrySetBool(id, value))
        return TRUE;
    if (!MfDebug_EnsureWritable())
        return FALSE;
    return MfRules_TrySetBool(id, value);
}

static bool8 MfDebug_TrySetValue(enum MfRuleValue id, u8 value)
{
    if (MfRules_TrySetValue(id, value))
        return TRUE;
    if (!MfDebug_EnsureWritable())
        return FALSE;
    return MfRules_TrySetValue(id, value);
}

static void MfDebug_FormatEntryName(u8 *dest, const struct MfDebugEntry *entry)
{
    u8 *end;
    u8 value;

    end = StringCopy(dest, entry->label);
    end = StringAppend(end, COMPOUND_STRING(":"));

    switch (entry->kind)
    {
    case MF_DEBUG_KIND_BOOL:
        value = MfRules_GetBool(entry->id);
        ConvertIntToDecimalStringN(end, value, STR_CONV_MODE_LEFT_ALIGN, 1);
        break;
    case MF_DEBUG_KIND_VALUE:
        value = MfRules_GetValue(entry->id);
        if (entry->id == MF_RULE_VAL_LEVEL_CAP)
        {
            // Mode, then the live cap (14 with Normal and no badges; 100 if lifted).
            end = ConvertIntToDecimalStringN(end, value, STR_CONV_MODE_LEFT_ALIGN, 1);
            end = StringAppend(end, COMPOUND_STRING("/"));
            ConvertIntToDecimalStringN(end, MfGetPartyLevelCap(), STR_CONV_MODE_LEFT_ALIGN, 3);
        }
        else if (entry->id == MF_RULE_VAL_SCALING_IVS)
        {
            end = ConvertIntToDecimalStringN(end, value, STR_CONV_MODE_LEFT_ALIGN, 1);
            end = StringAppend(end, COMPOUND_STRING("/"));
            ConvertIntToDecimalStringN(end, MfGetCurrentTrainerIVs(), STR_CONV_MODE_LEFT_ALIGN, 2);
        }
        else if (entry->id == MF_RULE_VAL_SCALING_EVS)
        {
            end = ConvertIntToDecimalStringN(end, value, STR_CONV_MODE_LEFT_ALIGN, 1);
            end = StringAppend(end, COMPOUND_STRING("/"));
            ConvertIntToDecimalStringN(end, MfGetCurrentTrainerEVs(), STR_CONV_MODE_LEFT_ALIGN, 3);
        }
        else if (entry->max >= 10)
        {
            ConvertIntToDecimalStringN(end, value, STR_CONV_MODE_LEFT_ALIGN, 2);
        }
        else
        {
            ConvertIntToDecimalStringN(end, value, STR_CONV_MODE_LEFT_ALIGN, 1);
        }
        break;
    case MF_DEBUG_KIND_UNLOCK:
        ConvertIntToDecimalStringN(end, MfRules_DebugHasUnlockOverride(), STR_CONV_MODE_LEFT_ALIGN, 1);
        break;
    case MF_DEBUG_KIND_VERSION:
        ConvertIntToDecimalStringN(end, MfRules_GetActiveRules()->version, STR_CONV_MODE_LEFT_ALIGN, 2);
        break;
    case MF_DEBUG_KIND_SEED:
        ConvertIntToHexStringN(end, MfRules_GetRandomizerSeed(), STR_CONV_MODE_LEADING_ZEROS, 8);
        break;
    }
}

static void MfDebug_ApplyEntry(const struct MfDebugEntry *entry)
{
    u8 cur;
    bool8 prevModernStats;

    switch (entry->kind)
    {
    case MF_DEBUG_KIND_BOOL:
        prevModernStats = MfRules_HasModernStats();
        if (!MfDebug_TrySetBool(entry->id, !MfRules_GetBool(entry->id)))
        {
            PlaySE(SE_FAILURE);
            break;
        }
        // Stored party stats must track the active base-stat table (S29).
        if (entry->id == MF_RULE_BOOL_MODERN_STATS
            && prevModernStats != MfRules_HasModernStats())
            MfRecalculatePartyStats();
        break;
    case MF_DEBUG_KIND_VALUE:
        cur = MfRules_GetValue(entry->id);
        cur = (cur >= entry->max) ? 0 : (cur + 1);
        if (!MfDebug_TrySetValue(entry->id, cur))
        {
            PlaySE(SE_FAILURE);
            break;
        }
        if (entry->id == MF_RULE_VAL_BASE_STAT_EQUALIZER)
            MfRecalculatePartyStats();
        break;
    case MF_DEBUG_KIND_UNLOCK:
        if (MfRules_DebugHasUnlockOverride())
            MfRules_DebugSetUnlockOverride(FALSE);
        else if (!MfRules_DebugSetUnlockOverride(TRUE))
            PlaySE(SE_FAILURE);
        break;
    case MF_DEBUG_KIND_SEED:
        if (!MfRules_DebugRerollSeed())
            PlaySE(SE_FAILURE);
        break;
    case MF_DEBUG_KIND_VERSION:
        PlaySE(SE_FAILURE);
        break;
    }
}

static void MfDebug_RebuildPageNames(enum MfDebugPage page)
{
    const struct MfDebugPageDef *def = &sPages[page];
    u32 i;

    for (i = 0; i < def->count; i++)
    {
        MfDebug_FormatEntryName(sMfDebugNames[i], &def->entries[i]);
        sMfDebugPageOptions[i].text = sMfDebugNames[i];
        sMfDebugPageOptions[i].action = MfDebug_Action_ToggleEntry;
        sMfDebugPageOptions[i].actionParams = (void *)&def->entries[i];
    }

    sMfDebugPageOptions[def->count].text = COMPOUND_STRING("Cancel");
    sMfDebugPageOptions[def->count].action = DebugAction_Cancel;
    sMfDebugPageOptions[def->count].actionParams = NULL;
    sMfDebugPageOptions[def->count + 1].text = NULL;
    sMfDebugPageOptions[def->count + 1].action = NULL;
    sMfDebugPageOptions[def->count + 1].actionParams = NULL;
}

static void MfDebug_Action_ToggleEntry(u8 taskId, void *params)
{
    const struct MfDebugEntry *entry = params;

    MfDebug_ApplyEntry(entry);
    MfDebug_RebuildPageNames(sMfDebugCurrentPage);
    Debug_RefreshCurrentMenu(taskId);
}

static void MfDebug_Action_OpenPage(u8 taskId, void *params)
{
    enum MfDebugPage page = (enum MfDebugPage)(uintptr_t)params;

    sMfDebugCurrentPage = page;
    MfDebug_RebuildPageNames(page);
    DebugAction_OpenSubMenu(taskId, sMfDebugPageOptions);
}

static void MfDebug_Action_Dump(u8 taskId)
{
    (void)taskId;
    PlaySE(SE_SELECT);
    MfRules_DebugDump();
}

static void MfDebug_Action_DumpUsedAreas(u8 taskId)
{
    (void)taskId;
    PlaySE(SE_SELECT);
    MfNuzlocke_DebugDumpUsedAreas();
}

static void MfDebug_Action_DumpFaintPlan(u8 taskId)
{
    (void)taskId;
    PlaySE(SE_SELECT);
    MfNuzlocke_DebugDumpFaintPlan();
}

static void MfDebug_Action_ToggleFaintDryRun(u8 taskId)
{
    (void)taskId;
    PlaySE(SE_SELECT);
    MfNuzlocke_SetFaintDryRun(!MfNuzlocke_GetFaintDryRun());
}

static void MfDebug_Action_ToggleForceShiny(u8 taskId)
{
    (void)taskId;
    PlaySE(SE_SELECT);
    MfDebug_SetForceShiny(!MfDebug_GetForceShiny());
}

// Instant statistical check — no wild encounters. Open mGBA Tools → View Logs.
#define MF_DEBUG_SHINY_ROLLS 10000

static void MfDebug_Action_ShinyRollTest(u8 taskId)
{
    u32 i;
    u32 hits = 0;
    u32 tier = MfRules_GetShinyChance();
    u32 threshold = MfGetShinyOddsThreshold();
    u32 expected; // approximate: threshold * N / 65536
    static const u16 sDenoms[MF_SHINY_CHANCE_COUNT] = { 8192, 4096, 2048, 1024, 512 };
    u16 denom = (tier < MF_SHINY_CHANCE_COUNT) ? sDenoms[tier] : 8192;

    (void)taskId;
    PlaySE(SE_SELECT);

    for (i = 0; i < MF_DEBUG_SHINY_ROLLS; i++)
    {
        // Same 16-bit shinyValue space as GET_SHINY_VALUE (0..65535).
        if (MfIsShinyValue(Random()))
            hits++;
    }

    expected = (threshold * MF_DEBUG_SHINY_ROLLS) / 65536u;

    DebugPrintfLevel(MGBA_LOG_WARN, "=== MF shiny roll test ===");
    DebugPrintfLevel(MGBA_LOG_WARN, "tier=%u (1/%u) threshold=%u rolls=%u",
        tier, denom, threshold, MF_DEBUG_SHINY_ROLLS);
    DebugPrintfLevel(MGBA_LOG_WARN, "hits=%u expected~%u (rate 1/%u vs target 1/%u)",
        hits, expected,
        hits ? (MF_DEBUG_SHINY_ROLLS / hits) : 0,
        denom);
}

static void MfDebug_Action_DumpSpeciesMap(u8 taskId)
{
    static const u16 sSamples[] = {
        SPECIES_BULBASAUR, SPECIES_CHARMANDER, SPECIES_SQUIRTLE,
        SPECIES_PIDGEY, SPECIES_MAGIKARP, SPECIES_ONIX,
        SPECIES_MACHOP, SPECIES_LAPRAS, SPECIES_MEWTWO,
    };
    const struct ModernRules *rules = MfRules_GetActiveRules();
    u32 i;
    bool8 similar = rules->randomSimilar && !rules->randomChaos;
    bool8 legs = rules->randomIncludeLegendaries || rules->randomChaos;

    (void)taskId;
    PlaySE(SE_SELECT);
    DebugPrintfLevel(MGBA_LOG_WARN, "=== MF species map ===");
    DebugPrintfLevel(MGBA_LOG_WARN, "seed=%08X similar=%u legs=%u map=%u pool=%u",
        rules->randomizerSeed, similar, legs, rules->randomMapBased,
        MfSpeciesMap_GetPoolCount());
    for (i = 0; i < ARRAY_COUNT(sSamples); i++)
    {
        enum Species src = sSamples[i];
        enum Species dest = MfSpeciesMapEx(src,
                                           rules->randomizerSeed,
                                           MF_RANDOM_CAT_WILD,
                                           0,
                                           similar,
                                           legs);
        DebugPrintfLevel(MGBA_LOG_WARN, "%u -> %u (stage %u bst %u hm %u)",
            src, dest,
            MfSpeciesMap_GetEvoStage(dest),
            MfSpeciesMap_GetRawBst(dest),
            MfSpeciesMap_GetKantoHmMask(dest));
    }
    DebugPrintfLevel(MGBA_LOG_WARN, "here Pidgey wild=%u static=%u gift=%u trainer=%u mapsec=%u",
        MfWildEncounterSpeciesHere(SPECIES_PIDGEY),
        MfStaticEncounterSpecies(SPECIES_PIDGEY),
        MfStaticGiftSpecies(SPECIES_PIDGEY),
        MfTrainerEncounterSpecies(SPECIES_PIDGEY, 1),
        gMapHeader.regionMapSectionId);
    {
        enum Species trio[MF_OAK_STARTER_COUNT];

        MfFillRandomOakStarters(trio);
        DebugPrintfLevel(MGBA_LOG_WARN, "oak starters %u %u %u (rival vs first %u)",
            trio[0], trio[1], trio[2],
            MfPickRivalStarterSpecies(trio[0], trio[2], trio[1]));
    }
    DebugPrintfLevel(MGBA_LOG_WARN, "RMoves=%u RAbil=%u Bulba moves %u/%u/%u abil %u/%u/%u",
        rules->randomMoves, rules->randomAbilities,
        GetSpeciesLevelUpLearnset(SPECIES_BULBASAUR)[0].move,
        GetSpeciesLevelUpLearnset(SPECIES_BULBASAUR)[1].move,
        GetSpeciesLevelUpLearnset(SPECIES_BULBASAUR)[2].move,
        GetSpeciesAbility(SPECIES_BULBASAUR, 0),
        GetSpeciesAbility(SPECIES_BULBASAUR, 1),
        GetSpeciesAbility(SPECIES_BULBASAUR, 2));
    DebugPrintfLevel(MGBA_LOG_WARN, "Magikarp L1 %u (dmgLow=%u) Shedinja abil %u",
        GetSpeciesLevelUpLearnset(SPECIES_MAGIKARP)[0].move,
        MfLearnsetHasDamagingMoveByLevel(GetSpeciesLevelUpLearnset(SPECIES_MAGIKARP), MF_RANDOM_LOW_MOVE_LEVEL),
        GetSpeciesAbility(SPECIES_SHEDINJA, 0));
    DebugPrintfLevel(MGBA_LOG_WARN, "REvo=%u REvoM=%u Charmander %u/%u Magneton %u/%u",
        rules->randomEvolution, rules->randomEvolutionMethods,
        GetSpeciesEvolutions(SPECIES_CHARMANDER)[0].method,
        GetSpeciesEvolutions(SPECIES_CHARMANDER)[0].targetSpecies,
        GetSpeciesEvolutions(SPECIES_MAGNETON)[0].method,
        GetSpeciesEvolutions(SPECIES_MAGNETON)[0].targetSpecies);
    DebugPrintfLevel(MGBA_LOG_WARN, "RType=%u RTypeE=%u Charizard %u/%u Clefairy %u GroundxRock=%u",
        rules->randomType, rules->randomTypeEffectiveness,
        GetSpeciesType(SPECIES_CHARIZARD, 0),
        GetSpeciesType(SPECIES_CHARIZARD, 1),
        GetSpeciesType(SPECIES_CLEFAIRY, 0),
        GetTypeModifier(TYPE_GROUND, TYPE_ROCK));
    DebugPrintfLevel(MGBA_LOG_WARN, "RItems=%u RChaos=%u Potion=%u Nugget=%u HMCut=%u PokeBall=%u",
        rules->randomItems, rules->randomChaos,
        MfItemMapActive(ITEM_POTION),
        MfItemMapActive(ITEM_NUGGET),
        MfItemMapActive(ITEM_HM_CUT),
        MfItemMapActive(ITEM_POKE_BALL));
}

static void MfDebug_Action_DumpOptions(u8 taskId)
{
    (void)taskId;
    PlaySE(SE_SELECT);
    MfOptions_DebugDump();
}

static void MfDebug_Action_DumpMovement(u8 taskId)
{
    (void)taskId;
    PlaySE(SE_SELECT);
    MfOptions_DebugDumpMovement();
}

static void MfDebug_WarpTo(u8 taskId, u16 map, s8 x, s8 y)
{
    PlaySE(SE_SELECT);
    FlagSet(FLAG_SYS_B_DASH);
    Debug_CloseMenuFull(taskId);
    SetWarpDestination(MAP_GROUP(map), MAP_NUM(map), WARP_ID_NONE, x, y);
    DoWarp();
    ResetInitialPlayerAvatarState();
    ScriptContext_Stop();
}

static void MfDebug_Action_WarpRoute1(u8 taskId)
{
    // Route 1 has no warp events; numeric "Warp to map warp" cannot land here.
    MfDebug_WarpTo(taskId, MAP_ROUTE1, 10, 20);
}

static void MfDebug_Action_WarpPallet(u8 taskId)
{
    MfDebug_WarpTo(taskId, MAP_PALLET_TOWN, 6, 9);
}

static void MfDebug_Action_WarpBedroom(u8 taskId)
{
    // 6,7 is south of the NES (6,5) so A does not play Font Test.
    MfDebug_WarpTo(taskId, MAP_PALLET_TOWN_PLAYERS_HOUSE_2F, 6, 7);
}

static void MfDebug_Action_WarpRoute19Water(u8 taskId)
{
    // Surfable tile: arrival sets PLAYER_AVATAR_FLAG_SURFING.
    MfDebug_WarpTo(taskId, MAP_ROUTE19, 12, 22);
}

static void MfDebug_Action_WarpViridianCenter(u8 taskId)
{
    MfDebug_WarpTo(taskId, MAP_VIRIDIAN_CITY_POKEMON_CENTER_1F, 4, 5);
}

static void MfDebug_Action_MountMachBike(u8 taskId)
{
    PlaySE(SE_SELECT);
    FlagSet(FLAG_SYS_B_DASH);
    AddBagItem(ITEM_BICYCLE, 1);
    Debug_CloseMenuFull(taskId);
    SetPlayerAvatarTransitionFlags(PLAYER_AVATAR_FLAG_MACH_BIKE);
}

static void MfDebug_Action_ToggleShoes(u8 taskId)
{
    (void)taskId;
    FlagToggle(FLAG_SYS_B_DASH);
    PlaySE(FlagGet(FLAG_SYS_B_DASH) ? SE_PC_LOGIN : SE_PC_OFF);
}

static void MfDebug_PrepSeviiPostgame(void)
{
    FlagSet(FLAG_SYS_POKEDEX_GET);
    FlagSet(FLAG_SYS_NATIONAL_DEX);
    EnableNationalPokedex();
    FlagSet(FLAG_SYS_B_DASH);
    FlagSet(FLAG_SYS_SEVII_MAP_123);
    FlagSet(FLAG_SYS_SEVII_MAP_4567);
    FlagSet(FLAG_WORLD_MAP_VERMILION_CITY);
    FlagSet(FLAG_WORLD_MAP_ONE_ISLAND);
    FlagClear(FLAG_HIDE_ONE_ISLAND_POKECENTER_CELIO);
    VarSet(VAR_MAP_SCENE_VERMILION_CITY, 3);
}

static void MfDebug_Action_WarpCelioSapphire(u8 taskId)
{
    MfDebug_PrepSeviiPostgame();
    FlagSet(FLAG_RECOVERED_SAPPHIRE);
    FlagClear(FLAG_RECEIVED_MYSTIC_TICKET);
    FlagClear(FLAG_RECEIVED_AURORA_TICKET);
    FlagClear(FLAG_ENABLE_SHIP_NAVEL_ROCK);
    FlagClear(FLAG_ENABLE_SHIP_BIRTH_ISLAND);
    RemoveBagItem(ITEM_MYSTIC_TICKET, 1);
    RemoveBagItem(ITEM_AURORA_TICKET, 1);
    AddBagItem(ITEM_SAPPHIRE, 1);
    VarSet(VAR_MAP_SCENE_ONE_ISLAND_POKEMON_CENTER_1F, 5);
    MfDebug_WarpTo(taskId, MAP_ONE_ISLAND_POKEMON_CENTER_1F, 15, 7);
}

static void MfDebug_Action_WarpCelioTicketRetry(u8 taskId)
{
    MfDebug_PrepSeviiPostgame();
    FlagSet(FLAG_RECOVERED_SAPPHIRE);
    FlagSet(FLAG_IS_CHAMPION);
    FlagClear(FLAG_RECEIVED_MYSTIC_TICKET);
    FlagClear(FLAG_RECEIVED_AURORA_TICKET);
    FlagClear(FLAG_ENABLE_SHIP_NAVEL_ROCK);
    FlagClear(FLAG_ENABLE_SHIP_BIRTH_ISLAND);
    RemoveBagItem(ITEM_MYSTIC_TICKET, 1);
    RemoveBagItem(ITEM_AURORA_TICKET, 1);
    VarSet(VAR_MAP_SCENE_ONE_ISLAND_POKEMON_CENTER_1F, 6);
    MfDebug_WarpTo(taskId, MAP_ONE_ISLAND_POKEMON_CENTER_1F, 15, 7);
}

static void MfDebug_Action_WarpVermilionTickets(u8 taskId)
{
    MfDebug_PrepSeviiPostgame();
    FlagSet(FLAG_IS_CHAMPION);
    FlagSet(FLAG_RECEIVED_MYSTIC_TICKET);
    FlagSet(FLAG_RECEIVED_AURORA_TICKET);
    FlagSet(FLAG_ENABLE_SHIP_NAVEL_ROCK);
    FlagSet(FLAG_ENABLE_SHIP_BIRTH_ISLAND);
    AddBagItem(ITEM_MYSTIC_TICKET, 1);
    AddBagItem(ITEM_AURORA_TICKET, 1);
    VarSet(VAR_MAP_SCENE_ONE_ISLAND_POKEMON_CENTER_1F, 6);
    MfDebug_WarpTo(taskId, MAP_VERMILION_CITY, 24, 32);
}

static const struct DebugMenuOption sMfDebugQaOptions[] =
{
    { COMPOUND_STRING("Route 1 (run)"), MfDebug_Action_WarpRoute1 },
    { COMPOUND_STRING("Pallet town"), MfDebug_Action_WarpPallet },
    { COMPOUND_STRING("Bedroom (off NES)"), MfDebug_Action_WarpBedroom },
    { COMPOUND_STRING("Route 19 water"), MfDebug_Action_WarpRoute19Water },
    { COMPOUND_STRING("Viridian Center"), MfDebug_Action_WarpViridianCenter },
    { COMPOUND_STRING("Mount Mach bike"), MfDebug_Action_MountMachBike },
    { COMPOUND_STRING("Toggle shoes"), MfDebug_Action_ToggleShoes },
    { COMPOUND_STRING("Celio Sapphire"), MfDebug_Action_WarpCelioSapphire },
    { COMPOUND_STRING("Celio ticket retry"), MfDebug_Action_WarpCelioTicketRetry },
    { COMPOUND_STRING("Vermilion tickets"), MfDebug_Action_WarpVermilionTickets },
    { COMPOUND_STRING("Cancel"), DebugAction_Cancel },
    { NULL }
};

static void MfDebug_Action_OpenQa(u8 taskId)
{
    PlaySE(SE_SELECT);
    DebugAction_OpenSubMenu(taskId, sMfDebugQaOptions);
}

static void MfDebug_DumpQaConfig(enum MfQaMatrixConfig id)
{
    const struct ModernRules *r = MfRules_GetActiveRules();
    const struct MfRulesConflict *c = MfRules_FindConflict(r);
    const char *name = MfQa_GetName(id);

    DebugPrintfLevel(MGBA_LOG_WARN, "=== MF QA matrix === %s id=%u locked=%u seed=%08X",
        name, id, r->rulesLocked, r->randomizerSeed);
    DebugPrintfLevel(MGBA_LOG_WARN, "preset=%u fairy=%u nuz=%u hard=%u mono=%u ranz=%u chaos=%u centers=%u",
        r->gamemodePreset, r->fairyTypes, r->nuzlocke, r->nuzlockeHardcore,
        r->monotype, r->randomizerEnabled, r->randomChaos, r->pokeCenterLimit);
    if (c == NULL)
        DebugPrintfLevel(MGBA_LOG_WARN, "conflict=none");
    else
        DebugPrintfLevel(MGBA_LOG_WARN, "conflict=%u action=%u", c->id, c->action);
}

static void MfDebug_Action_ApplyQa(u8 taskId, void *params)
{
    enum MfQaMatrixConfig id = (enum MfQaMatrixConfig)((uintptr_t)params - 1);

    (void)taskId;
    if (id >= MF_QA_CONFIG_COUNT)
        return;
    MfQa_ApplyToSave(id);
    MfRecalculatePartyStats();
    PlaySE(SE_PC_LOGIN);
    MfDebug_DumpQaConfig(id);
}

static void MfDebug_Action_DumpQa(u8 taskId)
{
    (void)taskId;
    PlaySE(SE_SELECT);
    MfDebug_DumpQaConfig(MF_QA_CONFIG_COUNT);
}

static const struct DebugMenuOption sMfDebugQaMatrixOptions[] =
{
    { COMPOUND_STRING("Apply Classic"), MfDebug_Action_ApplyQa, (void *)(uintptr_t)(MF_QA_CLASSIC + 1) },
    { COMPOUND_STRING("Apply Modern"), MfDebug_Action_ApplyQa, (void *)(uintptr_t)(MF_QA_MODERN + 1) },
    { COMPOUND_STRING("Apply Full rand"), MfDebug_Action_ApplyQa, (void *)(uintptr_t)(MF_QA_FULL_RANDOMIZER + 1) },
    { COMPOUND_STRING("Apply Hardcore"), MfDebug_Action_ApplyQa, (void *)(uintptr_t)(MF_QA_HARDCORE_NUZLOCKE + 1) },
    { COMPOUND_STRING("Apply Monotype"), MfDebug_Action_ApplyQa, (void *)(uintptr_t)(MF_QA_MONOTYPE + 1) },
    { COMPOUND_STRING("Apply Nuz noCtr"), MfDebug_Action_ApplyQa, (void *)(uintptr_t)(MF_QA_NUZLOCKE_NO_CENTER + 1) },
    { COMPOUND_STRING("Apply Mono+rand"), MfDebug_Action_ApplyQa, (void *)(uintptr_t)(MF_QA_MONOTYPE_RANDOM + 1) },
    { COMPOUND_STRING("Dump QA config"), MfDebug_Action_DumpQa },
    { COMPOUND_STRING("Cancel"), DebugAction_Cancel },
    { NULL }
};

static void MfDebug_Action_OpenQaMatrix(u8 taskId)
{
    PlaySE(SE_SELECT);
    DebugAction_OpenSubMenu(taskId, sMfDebugQaMatrixOptions);
}

static void MfDebug_Action_OpenOptionsMenu(u8 taskId)
{
    PlaySE(SE_SELECT);
    Debug_CloseMenuFull(taskId);
    CleanupOverworldWindowsAndTilemaps();
    gMain.savedCallback = CB2_ReturnToField;
    gMain.state = 0;
    SetMainCallback2(CB2_InitMfOptionsMenu);
}

static void MfDebug_Action_OpenRulesMenu(u8 taskId)
{
    PlaySE(SE_SELECT);
    Debug_CloseMenuFull(taskId);
    CleanupOverworldWindowsAndTilemaps();
    // Same destination as Task_MfRulesMenu_NoNewGame (ME mid-run entry).
    gMain.savedCallback = CB2_ReturnToField;
    gMain.state = 0;
    SetMainCallback2(CB2_InitMfRulesMenu);
}

static void MfDebug_Action_OpenRulesViewer(u8 taskId)
{
    PlaySE(SE_SELECT);
    Debug_CloseMenuFull(taskId);
    CleanupOverworldWindowsAndTilemaps();
    gMain.savedCallback = CB2_ReturnToField;
    gMain.state = 0;
    SetMainCallback2(CB2_InitMfRulesViewer);
}

static const struct DebugMenuOption sMfDebugInspectorOptions[] =
{
    { COMPOUND_STRING("Meta…"),        MfDebug_Action_OpenPage, (void *)(uintptr_t)MF_DEBUG_PAGE_META },
    { COMPOUND_STRING("Gamemode…"),    MfDebug_Action_OpenPage, (void *)(uintptr_t)MF_DEBUG_PAGE_GAMEMODE },
    { COMPOUND_STRING("Features…"),    MfDebug_Action_OpenPage, (void *)(uintptr_t)MF_DEBUG_PAGE_FEATURES },
    { COMPOUND_STRING("Randomizer…"),  MfDebug_Action_OpenPage, (void *)(uintptr_t)MF_DEBUG_PAGE_RANDOMIZER },
    { COMPOUND_STRING("Nuzlocke…"),    MfDebug_Action_OpenPage, (void *)(uintptr_t)MF_DEBUG_PAGE_NUZLOCKE },
    { COMPOUND_STRING("Difficulty…"),  MfDebug_Action_OpenPage, (void *)(uintptr_t)MF_DEBUG_PAGE_DIFFICULTY },
    { COMPOUND_STRING("Challenges…"),  MfDebug_Action_OpenPage, (void *)(uintptr_t)MF_DEBUG_PAGE_CHALLENGES },
    { COMPOUND_STRING("Dump (mGBA)"),  MfDebug_Action_Dump },
    { COMPOUND_STRING("Used areas…"), MfDebug_Action_DumpUsedAreas },
    { COMPOUND_STRING("Faint plan…"), MfDebug_Action_DumpFaintPlan },
    { COMPOUND_STRING("Faint dry-run"), MfDebug_Action_ToggleFaintDryRun },
    { COMPOUND_STRING("Cancel"),       DebugAction_Cancel },
    { NULL }
};

static void MfDebug_Action_OpenInspector(u8 taskId)
{
    PlaySE(SE_SELECT);
    DebugAction_OpenSubMenu(taskId, sMfDebugInspectorOptions);
}

const struct DebugMenuOption gMfDebugMenuOptions[] =
{
    { COMPOUND_STRING("QA scenes…"), MfDebug_Action_OpenQa },
    { COMPOUND_STRING("QA matrix…"), MfDebug_Action_OpenQaMatrix },
    { COMPOUND_STRING("Movement dump"), MfDebug_Action_DumpMovement },
    { COMPOUND_STRING("Rules inspector…"), MfDebug_Action_OpenInspector },
    { COMPOUND_STRING("Rules menu…"), MfDebug_Action_OpenRulesMenu },
    { COMPOUND_STRING("Options menu…"), MfDebug_Action_OpenOptionsMenu },
    { COMPOUND_STRING("Options dump"), MfDebug_Action_DumpOptions },
    { COMPOUND_STRING("Rules viewer…"), MfDebug_Action_OpenRulesViewer },
    { COMPOUND_STRING("Shiny roll test"), MfDebug_Action_ShinyRollTest },
    { COMPOUND_STRING("Force shiny"), MfDebug_Action_ToggleForceShiny },
    { COMPOUND_STRING("Species map dump"), MfDebug_Action_DumpSpeciesMap },
    { COMPOUND_STRING("Used areas…"), MfDebug_Action_DumpUsedAreas },
    { COMPOUND_STRING("Faint plan…"), MfDebug_Action_DumpFaintPlan },
    { COMPOUND_STRING("Faint dry-run"), MfDebug_Action_ToggleFaintDryRun },
    { COMPOUND_STRING("Cancel"),           DebugAction_Cancel },
    { NULL }
};

#else // NDEBUG or !MF_RULES_ENGINE

const struct DebugMenuOption gMfDebugMenuOptions[] =
{
    { COMPOUND_STRING("Cancel"), DebugAction_Cancel },
    { NULL }
};

#endif
