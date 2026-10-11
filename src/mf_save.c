#include "global.h"
#include "mf_save.h"
#include "mf_options.h"
#include "mf_rules.h"
#include "save.h"

STATIC_ASSERT(sizeof(struct SaveBlock3) == MF_SAVEBLOCK3_SIZE, MfSaveBlock3SizeLock);
STATIC_ASSERT(sizeof(struct SaveBlock3) <= SAVE_BLOCK_3_CHUNK_SIZE * NUM_SECTORS_PER_SLOT, MfSaveBlock3ChunkCap);

void MfSave_RelocateMfOptions(u16 oldRulesSize)
{
    u8 *base;
    u8 *src;
    u8 *dst;

    if (gSaveBlock3Ptr == NULL)
        return;
    if (oldRulesSize == 0 || oldRulesSize >= sizeof(struct ModernRules))
        return;

    base = (u8 *)&gSaveBlock3Ptr->mfRules;
    src = base + oldRulesSize;
    dst = base + sizeof(struct ModernRules);
    memmove(dst, src, sizeof(struct MfOptions));
    memset(base + oldRulesSize, 0, sizeof(struct ModernRules) - oldRulesSize);
}

void MfSave_MigrateOnLoad(void)
{
    struct ModernRules *rules;
    u16 version;
    u16 oldSize;

    if (gSaveBlock3Ptr == NULL)
        return;

#if MF_RULES_ENGINE
    rules = &gSaveBlock3Ptr->mfRules;
    version = rules->version;
    oldSize = MfRules_SerializedSizeForVersion(version);
    if (oldSize != 0 && oldSize < sizeof(struct ModernRules))
    {
        MfSave_RelocateMfOptions(oldSize);
        rules->version = version;
    }
    MfRules_Migrate(rules);
#endif

#if MF_OPTIONS_PLUS
    MfOptions_Migrate(&gSaveBlock3Ptr->mfOptions);
#endif
}
