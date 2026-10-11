#ifndef GUARD_MF_QA_MATRIX_H
#define GUARD_MF_QA_MATRIX_H

// S66 — playthrough QA configs. Fills a ModernRules blob; does not lock or seed
// unless MfQa_ApplyToSave is used (debug). See docs-mf/QA_MATRIX.md.

#include "gba/types.h"
#include "mf_rules.h"

enum MfQaMatrixConfig
{
    MF_QA_CLASSIC = 0,       // Classic vanilla gamemode; other pages TX off
    MF_QA_MODERN,            // New-game Modern default
    MF_QA_FULL_RANDOMIZER,   // Modern + all remaps; chaos off (playable)
    MF_QA_HARDCORE_NUZLOCKE, // Modern + Hardcore Nuzlocke bundle
    MF_QA_MONOTYPE,          // Modern + ONE TYPE FIRE
    MF_QA_NUZLOCKE_NO_CENTER, // ADR 0078 extra: Normal Nuzlocke + no Centers
    MF_QA_MONOTYPE_RANDOM,   // ADR 0078 extra: Fire monotype + wild/trainer rand
    MF_QA_CONFIG_COUNT,
};

#define MF_QA_REQUIRED_COUNT 5 // Classic … Monotype (story minimum)

void MfQa_FillConfig(enum MfQaMatrixConfig id, struct ModernRules *rules);
const char *MfQa_GetName(enum MfQaMatrixConfig id);
// Writes save, ensures seed, locks. Recalc party in the debug caller.
void MfQa_ApplyToSave(enum MfQaMatrixConfig id);

#endif // GUARD_MF_QA_MATRIX_H
