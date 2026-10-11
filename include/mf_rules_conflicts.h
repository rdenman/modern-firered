#ifndef GUARD_MF_RULES_CONFLICTS_H
#define GUARD_MF_RULES_CONFLICTS_H

// S65 — rule-pair legality. See docs-mf/RULE_INTERACTIONS.md and ADR 0078.

#include "gba/types.h"
#include "mf_rules.h"

enum MfRulesConflictAction
{
    MF_RULES_CONFLICT_ALLOW = 0,
    MF_RULES_CONFLICT_WARN,
    MF_RULES_CONFLICT_FORBID,
};

enum MfRulesConflictId
{
    MF_RULES_CONFLICT_NONE = 0,
    MF_RULES_CONFLICT_FAIRY_MONOTYPE,       // FORBID: Fairy Off + Fairy monotype
    MF_RULES_CONFLICT_MONOTYPE_RANDOM,      // WARN: monotype + species/type remap
    MF_RULES_CONFLICT_MONOTYPE_NUZLOCKE,    // WARN: sparse legal encounters
    MF_RULES_CONFLICT_NUZLOCKE_NO_CENTER,   // WARN: Nuzlocke + no Poké Centers
    MF_RULES_CONFLICT_NUZLOCKE_NO_HEAL,     // WARN: Nuzlocke + no Centers + no PC heal
    MF_RULES_CONFLICT_CHAOS_NUZLOCKE,       // WARN: chaos salt vs first-encounter
    MF_RULES_CONFLICT_NUZLOCKE_EXP_ZERO,    // WARN: Nuzlocke + EXP ×0
    MF_RULES_CONFLICT_EVO_ALL_RANDOM_EVO,   // WARN: can't evolve + random evo options
    MF_RULES_CONFLICT_COUNT,
};

struct MfRulesConflict
{
    enum MfRulesConflictId id;
    enum MfRulesConflictAction action;
    const u8 *message;
};

enum MfRulesConflictAction MfRules_EvaluatePair(const struct ModernRules *rules, enum MfRulesConflictId id);
const struct MfRulesConflict *MfRules_FindConflict(const struct ModernRules *rules);

#endif // GUARD_MF_RULES_CONFLICTS_H
