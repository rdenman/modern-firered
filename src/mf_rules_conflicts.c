#include "global.h"
#include "mf_rules_conflicts.h"
#include "mf_evolution.h"
#include "mf_exp.h"
#include "mf_monotype.h"
#include "constants/pokemon.h"

static const u8 sMsgFairyMonotype[] = _("Fairy Type is Off, so Fairy\nmonotype has no legal Pokémon.");
static const u8 sMsgMonotypeRandom[] = _("Monotype + randomizer: many\nencounters may be unusable.");
static const u8 sMsgMonotypeNuzlocke[] = _("Monotype Nuzlocke: off-type\nwilds do not count as a catch.");
static const u8 sMsgNuzlockeNoCenter[] = _("Nuzlocke without Poké Centers:\nheal with items or the PC.");
static const u8 sMsgNuzlockeNoHeal[] = _("Nuzlocke with no Centers or PC\nheals: overworld items only.");
static const u8 sMsgChaosNuzlocke[] = _("Chaos + Nuzlocke: fleeing can\nchange the first encounter.");
static const u8 sMsgNuzlockeExpZero[] = _("Nuzlocke + EXP ×0: no battle\nEXP. Candies still work.");
static const u8 sMsgEvoAllRandom[] = _("Evo Limit All: random evolution\noptions will not apply.");

static const struct MfRulesConflict sConflicts[] =
{
    { MF_RULES_CONFLICT_FAIRY_MONOTYPE,     MF_RULES_CONFLICT_FORBID, sMsgFairyMonotype },
    { MF_RULES_CONFLICT_CHAOS_NUZLOCKE,     MF_RULES_CONFLICT_WARN,   sMsgChaosNuzlocke },
    { MF_RULES_CONFLICT_MONOTYPE_NUZLOCKE,  MF_RULES_CONFLICT_WARN,   sMsgMonotypeNuzlocke },
    { MF_RULES_CONFLICT_MONOTYPE_RANDOM,    MF_RULES_CONFLICT_WARN,   sMsgMonotypeRandom },
    { MF_RULES_CONFLICT_NUZLOCKE_NO_HEAL,   MF_RULES_CONFLICT_WARN,   sMsgNuzlockeNoHeal },
    { MF_RULES_CONFLICT_NUZLOCKE_NO_CENTER, MF_RULES_CONFLICT_WARN,   sMsgNuzlockeNoCenter },
    { MF_RULES_CONFLICT_NUZLOCKE_EXP_ZERO,  MF_RULES_CONFLICT_WARN,   sMsgNuzlockeExpZero },
    { MF_RULES_CONFLICT_EVO_ALL_RANDOM_EVO, MF_RULES_CONFLICT_WARN,   sMsgEvoAllRandom },
};

static bool8 AnyNuzlocke(const struct ModernRules *r)
{
    return r->nuzlocke || r->nuzlockeEasy;
}

static bool8 SpeciesOrTypeRandom(const struct ModernRules *r)
{
    return r->randomizerEnabled
        && (r->randomStarter
         || r->randomWild
         || r->randomTrainer
         || r->randomStatic
         || r->randomType
         || r->randomTypeEffectiveness);
}

static bool8 PairActive(const struct ModernRules *r, enum MfRulesConflictId id)
{
    if (r == NULL)
        return FALSE;

    switch (id)
    {
    case MF_RULES_CONFLICT_FAIRY_MONOTYPE:
        return r->monotype == TYPE_FAIRY && !r->fairyTypes;
    case MF_RULES_CONFLICT_MONOTYPE_RANDOM:
        return r->monotype != MF_MONOTYPE_OFF && SpeciesOrTypeRandom(r);
    case MF_RULES_CONFLICT_MONOTYPE_NUZLOCKE:
        return r->monotype != MF_MONOTYPE_OFF && r->nuzlocke;
    case MF_RULES_CONFLICT_NUZLOCKE_NO_CENTER:
        return AnyNuzlocke(r) && r->pokeCenterLimit != 0;
    case MF_RULES_CONFLICT_NUZLOCKE_NO_HEAL:
        return AnyNuzlocke(r) && r->pokeCenterLimit != 0 && r->noPcHeal;
    case MF_RULES_CONFLICT_CHAOS_NUZLOCKE:
        return r->nuzlocke && r->randomizerEnabled && r->randomChaos;
    case MF_RULES_CONFLICT_NUZLOCKE_EXP_ZERO:
        return AnyNuzlocke(r) && r->expMultiplier == MF_EXP_MULT_0X;
    case MF_RULES_CONFLICT_EVO_ALL_RANDOM_EVO:
        return r->evoLimit == MF_EVO_LIMIT_NONE
            && r->randomizerEnabled
            && (r->randomEvolution || r->randomEvolutionMethods);
    case MF_RULES_CONFLICT_NONE:
    case MF_RULES_CONFLICT_COUNT:
        break;
    }
    return FALSE;
}

enum MfRulesConflictAction MfRules_EvaluatePair(const struct ModernRules *rules, enum MfRulesConflictId id)
{
    u32 i;

    if (!PairActive(rules, id))
        return MF_RULES_CONFLICT_ALLOW;

    for (i = 0; i < ARRAY_COUNT(sConflicts); i++)
    {
        if (sConflicts[i].id == id)
            return sConflicts[i].action;
    }
    return MF_RULES_CONFLICT_ALLOW;
}

const struct MfRulesConflict *MfRules_FindConflict(const struct ModernRules *rules)
{
    u32 i;

    for (i = 0; i < ARRAY_COUNT(sConflicts); i++)
    {
        if (PairActive(rules, sConflicts[i].id))
            return &sConflicts[i];
    }
    return NULL;
}
