#include "global.h"
#include "config/modern_firered.h"
#include "mf_nuzlocke.h"
#include "mf_qa_matrix.h"
#include "mf_random.h"
#include "constants/pokemon.h"

static void EnableFullRandomizer(struct ModernRules *r)
{
    r->randomizerEnabled = TRUE;
    r->randomStarter = TRUE;
    r->randomWild = TRUE;
    r->randomTrainer = TRUE;
    r->randomStatic = TRUE;
    r->randomSimilar = TRUE;
    r->randomMapBased = TRUE;
    r->randomIncludeLegendaries = TRUE;
    r->randomType = TRUE;
    r->randomMoves = TRUE;
    r->randomAbilities = TRUE;
    r->randomEvolution = TRUE;
    r->randomEvolutionMethods = TRUE;
    r->randomTypeEffectiveness = TRUE;
    r->randomItems = TRUE;
    r->randomChaos = FALSE;
    r->randomThemedTrainers = TRUE;
}

static void EnableWildTrainerRandomizer(struct ModernRules *r)
{
    r->randomizerEnabled = TRUE;
    r->randomWild = TRUE;
    r->randomTrainer = TRUE;
    r->randomStatic = TRUE;
    r->randomSimilar = TRUE;
    r->randomMapBased = TRUE;
}

static void ApplyFullNuzlockeClauses(struct ModernRules *r)
{
    r->nuzlockeSpeciesClause = MF_TX_NUZLOCKE_SPECIES_CLAUSE;
    r->nuzlockeShinyClause = MF_TX_NUZLOCKE_SHINY_CLAUSE;
    r->nuzlockeNicknaming = MF_TX_NUZLOCKE_NICKNAMING;
    r->nuzlockeDeletion = MF_TX_NUZLOCKE_DELETION;
}

static void ApplyNormalNuzlocke(struct ModernRules *r)
{
    r->nuzlocke = TRUE;
    r->nuzlockeHardcore = FALSE;
    r->nuzlockeEasy = FALSE;
    ApplyFullNuzlockeClauses(r);
}

static void ApplyHardcoreNuzlocke(struct ModernRules *r)
{
    r->nuzlocke = TRUE;
    r->nuzlockeHardcore = TRUE;
    r->nuzlockeEasy = FALSE;
    ApplyFullNuzlockeClauses(r);
    MfNuzlocke_ApplyHardcoreDifficultySeeds(r);
}

void MfQa_FillConfig(enum MfQaMatrixConfig id, struct ModernRules *rules)
{
    if (rules == NULL)
        return;

    MfRules_ApplyDevDefaults(rules);
    MfRules_ApplyGamemodePreset(rules, (id == MF_QA_CLASSIC) ? MF_GAMEMODE_CLASSIC : MF_GAMEMODE_MODERN);

    switch (id)
    {
    case MF_QA_CLASSIC:
    case MF_QA_MODERN:
        break;
    case MF_QA_FULL_RANDOMIZER:
        EnableFullRandomizer(rules);
        break;
    case MF_QA_HARDCORE_NUZLOCKE:
        ApplyHardcoreNuzlocke(rules);
        break;
    case MF_QA_MONOTYPE:
        rules->monotype = TYPE_FIRE;
        break;
    case MF_QA_NUZLOCKE_NO_CENTER:
        ApplyNormalNuzlocke(rules);
        rules->pokeCenterLimit = 1;
        break;
    case MF_QA_MONOTYPE_RANDOM:
        rules->monotype = TYPE_FIRE;
        EnableWildTrainerRandomizer(rules);
        break;
    case MF_QA_CONFIG_COUNT:
        break;
    }
}

const char *MfQa_GetName(enum MfQaMatrixConfig id)
{
    switch (id)
    {
    case MF_QA_CLASSIC:            return "Classic";
    case MF_QA_MODERN:             return "Modern";
    case MF_QA_FULL_RANDOMIZER:    return "Full rand";
    case MF_QA_HARDCORE_NUZLOCKE:  return "Hardcore";
    case MF_QA_MONOTYPE:           return "Monotype";
    case MF_QA_NUZLOCKE_NO_CENTER: return "Nuz noCtr";
    case MF_QA_MONOTYPE_RANDOM:    return "Mono+rand";
    case MF_QA_CONFIG_COUNT:       break;
    }
    return "current";
}

void MfQa_ApplyToSave(enum MfQaMatrixConfig id)
{
#if MF_RULES_ENGINE
    struct ModernRules *save = MfRules_GetSaveRules();

    MfQa_FillConfig(id, save);
    MfRandom_EnsureSeed(save);
    save->rulesLocked = TRUE;
#else
    (void)id;
#endif
}
