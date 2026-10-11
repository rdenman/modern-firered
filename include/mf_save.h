#ifndef GUARD_MF_SAVE_H
#define GUARD_MF_SAVE_H

#include "gba/types.h"

// Exact SaveBlock3 size for this ROM. Bump with test/save.c T_SAVEBLOCK3_SIZE
// when mfRules grows or upstream adds SB3 fields. Soft/hard caps still apply
// (MF_RULES_MAX_BYTES / 1624). See docs-mf/SAVE_VERSIONING.md.
#define MF_SAVEBLOCK3_SIZE 56

void MfSave_MigrateOnLoad(void);
void MfSave_RelocateMfOptions(u16 oldRulesSize);

#endif // GUARD_MF_SAVE_H
