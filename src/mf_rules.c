#include "global.h"
#include "mf_rules.h"

// Null / compile-out defaults: vanilla Kanto progression + Phase 1 always-on
// modernization (phys/spec+Fairy+modern types/stats/moves/chart, reusable TMs,
// Gen4+ Sitrus, Gen5+ OW poison). Challenges / randomizer / Nuzlocke stay off.
// See ADR 0013 and docs-mf/RULES_ACCESSORS.md.
const struct ModernRules gMfRulesPhase1Defaults = {
    .version = MF_RULES_VERSION,
    .gamemodePreset = MF_GAMEMODE_MODERN,
    .rulesLocked = FALSE,

    .infiniteTms = TRUE,
    .survivePoison = TRUE,
    .synchronize = FALSE,
    .mints = FALSE,
    .modernSitrus = TRUE,
    .modernTypes = TRUE,
    .fairyTypes = TRUE,
    .modernStats = TRUE,
    .sturdy = FALSE,
    .modernMoves = TRUE,
    .legendaryAbilities = FALSE,
    .newLegendaries = FALSE,
    .typeEffectiveness = TRUE,
    .alternateSpawns = 0,

    .shinyChance = 0,
    .wildItemDrops = FALSE,
    .easierFeebas = FALSE,
    .rtcType = FALSE,
    .shinyColors = FALSE,
    .wonderTrade = FALSE,
    .unlimitedWonderTrade = FALSE,
    .frontierBans = FALSE,

    .randomStarter = FALSE,
    .randomWild = FALSE,
    .randomTrainer = FALSE,
    .randomStatic = FALSE,
    .randomSimilar = FALSE,
    .randomMapBased = FALSE,
    .randomIncludeLegendaries = FALSE,
    .randomType = FALSE,
    .randomMoves = FALSE,
    .randomAbilities = FALSE,
    .randomEvolution = FALSE,
    .randomEvolutionMethods = FALSE,
    .randomTypeEffectiveness = FALSE,
    .randomItems = FALSE,
    .randomChaos = FALSE,

    .nuzlocke = FALSE,
    .nuzlockeHardcore = FALSE,
    .nuzlockeEasy = FALSE,
    .nuzlockeSpeciesClause = FALSE,
    .nuzlockeShinyClause = FALSE,
    .nuzlockeNicknaming = FALSE,
    .nuzlockeDeletion = FALSE,

    .partyLimit = 0,
    .levelCap = 0,
    .expMultiplier = 0,
    .noItemPlayer = FALSE,
    .noItemTrainer = FALSE,
    .noEvs = FALSE,
    .scalingIvs = 0,
    .scalingEvs = 0,
    .maxPartyIvs = 0,
    .pokeCenterLimit = 0,
    .lockDifficulty = FALSE,
    .escapeRopeDig = FALSE,
    .hardExp = FALSE,
    .catchRate = 0,

    .evoLimit = 0,
    .baseStatEqualizer = 0,
    .lessEscapes = FALSE,
    .mirror = FALSE,
    .mirrorThief = FALSE,
    .noPcHeal = FALSE,
    .monotype = 31,
    .expensiveShops = 0,

    .randomizerSeed = 0,
};

void MfRules_ResetToEmpty(struct ModernRules *rules)
{
    if (rules == NULL)
        return;

    memset(rules, 0, sizeof(*rules));
    rules->version = MF_RULES_VERSION;
    rules->monotype = 31; // ME TX_CHALLENGE_TYPE_OFF
}

void MfRules_Pack(const struct ModernRules *rules, u8 *out)
{
    if (rules == NULL || out == NULL)
        return;

    memcpy(out, rules, sizeof(struct ModernRules));
}

void MfRules_Unpack(const u8 *in, struct ModernRules *rules)
{
    if (in == NULL || rules == NULL)
        return;

    memcpy(rules, in, sizeof(struct ModernRules));
}

struct ModernRules *MfRules_GetSaveRules(void)
{
    return &gSaveBlock3Ptr->mfRules;
}

const struct ModernRules *MfRules_GetActiveRules(void)
{
#if !MF_RULES_ENGINE
    return &gMfRulesPhase1Defaults;
#else
    {
        const struct ModernRules *rules = MfRules_GetSaveRules();

        if (rules->version != MF_RULES_VERSION)
            return &gMfRulesPhase1Defaults;
        return rules;
    }
#endif
}

bool8 MfRules_GetBool(enum MfRuleBool id)
{
    const struct ModernRules *r = MfRules_GetActiveRules();

    switch (id)
    {
    case MF_RULE_BOOL_RULES_LOCKED:               return r->rulesLocked;
    case MF_RULE_BOOL_INFINITE_TMS:               return r->infiniteTms;
    case MF_RULE_BOOL_SURVIVE_POISON:             return r->survivePoison;
    case MF_RULE_BOOL_SYNCHRONIZE:                return r->synchronize;
    case MF_RULE_BOOL_MINTS:                      return r->mints;
    case MF_RULE_BOOL_MODERN_SITRUS:              return r->modernSitrus;
    case MF_RULE_BOOL_MODERN_TYPES:               return r->modernTypes;
    case MF_RULE_BOOL_FAIRY_TYPES:                return r->fairyTypes;
    case MF_RULE_BOOL_MODERN_STATS:               return r->modernStats;
    case MF_RULE_BOOL_STURDY:                     return r->sturdy;
    case MF_RULE_BOOL_MODERN_MOVES:               return r->modernMoves;
    case MF_RULE_BOOL_LEGENDARY_ABILITIES:        return r->legendaryAbilities;
    case MF_RULE_BOOL_NEW_LEGENDARIES:            return r->newLegendaries;
    case MF_RULE_BOOL_TYPE_EFFECTIVENESS:         return r->typeEffectiveness;
    case MF_RULE_BOOL_WILD_ITEM_DROPS:            return r->wildItemDrops;
    case MF_RULE_BOOL_EASIER_FEEBAS:              return r->easierFeebas;
    case MF_RULE_BOOL_RTC_TYPE:                   return r->rtcType;
    case MF_RULE_BOOL_SHINY_COLORS:               return r->shinyColors;
    case MF_RULE_BOOL_WONDER_TRADE:               return r->wonderTrade;
    case MF_RULE_BOOL_UNLIMITED_WONDER_TRADE:     return r->unlimitedWonderTrade;
    case MF_RULE_BOOL_FRONTIER_BANS:              return r->frontierBans;
    case MF_RULE_BOOL_RANDOM_STARTER:             return r->randomStarter;
    case MF_RULE_BOOL_RANDOM_WILD:                return r->randomWild;
    case MF_RULE_BOOL_RANDOM_TRAINER:             return r->randomTrainer;
    case MF_RULE_BOOL_RANDOM_STATIC:              return r->randomStatic;
    case MF_RULE_BOOL_RANDOM_SIMILAR:             return r->randomSimilar;
    case MF_RULE_BOOL_RANDOM_MAP_BASED:           return r->randomMapBased;
    case MF_RULE_BOOL_RANDOM_INCLUDE_LEGENDARIES: return r->randomIncludeLegendaries;
    case MF_RULE_BOOL_RANDOM_TYPE:                return r->randomType;
    case MF_RULE_BOOL_RANDOM_MOVES:               return r->randomMoves;
    case MF_RULE_BOOL_RANDOM_ABILITIES:           return r->randomAbilities;
    case MF_RULE_BOOL_RANDOM_EVOLUTION:           return r->randomEvolution;
    case MF_RULE_BOOL_RANDOM_EVOLUTION_METHODS:   return r->randomEvolutionMethods;
    case MF_RULE_BOOL_RANDOM_TYPE_EFFECTIVENESS:  return r->randomTypeEffectiveness;
    case MF_RULE_BOOL_RANDOM_ITEMS:               return r->randomItems;
    case MF_RULE_BOOL_RANDOM_CHAOS:               return r->randomChaos;
    case MF_RULE_BOOL_NUZLOCKE:                   return r->nuzlocke;
    case MF_RULE_BOOL_NUZLOCKE_HARDCORE:          return r->nuzlockeHardcore;
    case MF_RULE_BOOL_NUZLOCKE_EASY:              return r->nuzlockeEasy;
    case MF_RULE_BOOL_NUZLOCKE_SPECIES_CLAUSE:    return r->nuzlockeSpeciesClause;
    case MF_RULE_BOOL_NUZLOCKE_SHINY_CLAUSE:      return r->nuzlockeShinyClause;
    case MF_RULE_BOOL_NUZLOCKE_NICKNAMING:        return r->nuzlockeNicknaming;
    case MF_RULE_BOOL_NUZLOCKE_DELETION:          return r->nuzlockeDeletion;
    case MF_RULE_BOOL_NO_ITEM_PLAYER:             return r->noItemPlayer;
    case MF_RULE_BOOL_NO_ITEM_TRAINER:            return r->noItemTrainer;
    case MF_RULE_BOOL_NO_EVS:                     return r->noEvs;
    case MF_RULE_BOOL_LOCK_DIFFICULTY:            return r->lockDifficulty;
    case MF_RULE_BOOL_ESCAPE_ROPE_DIG:            return r->escapeRopeDig;
    case MF_RULE_BOOL_HARD_EXP:                   return r->hardExp;
    case MF_RULE_BOOL_LESS_ESCAPES:               return r->lessEscapes;
    case MF_RULE_BOOL_MIRROR:                     return r->mirror;
    case MF_RULE_BOOL_MIRROR_THIEF:               return r->mirrorThief;
    case MF_RULE_BOOL_NO_PC_HEAL:                 return r->noPcHeal;
    case MF_RULE_BOOL_COUNT:                      break;
    }
    return FALSE;
}

u8 MfRules_GetValue(enum MfRuleValue id)
{
    const struct ModernRules *r = MfRules_GetActiveRules();

    switch (id)
    {
    case MF_RULE_VAL_GAMEMODE_PRESET:      return r->gamemodePreset;
    case MF_RULE_VAL_ALTERNATE_SPAWNS:     return r->alternateSpawns;
    case MF_RULE_VAL_SHINY_CHANCE:         return r->shinyChance;
    case MF_RULE_VAL_PARTY_LIMIT:          return r->partyLimit;
    case MF_RULE_VAL_LEVEL_CAP:            return r->levelCap;
    case MF_RULE_VAL_EXP_MULTIPLIER:       return r->expMultiplier;
    case MF_RULE_VAL_SCALING_IVS:          return r->scalingIvs;
    case MF_RULE_VAL_SCALING_EVS:          return r->scalingEvs;
    case MF_RULE_VAL_MAX_PARTY_IVS:        return r->maxPartyIvs;
    case MF_RULE_VAL_POKECENTER_LIMIT:     return r->pokeCenterLimit;
    case MF_RULE_VAL_CATCH_RATE:           return r->catchRate;
    case MF_RULE_VAL_EVO_LIMIT:            return r->evoLimit;
    case MF_RULE_VAL_BASE_STAT_EQUALIZER:  return r->baseStatEqualizer;
    case MF_RULE_VAL_MONOTYPE:             return r->monotype;
    case MF_RULE_VAL_EXPENSIVE_SHOPS:      return r->expensiveShops;
    case MF_RULE_VAL_COUNT:                break;
    }
    return 0;
}
