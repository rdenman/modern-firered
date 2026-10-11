#include "global.h"
#include "mf_options.h"
#include "mf_rules.h"
#include "mf_save.h"
#include "save.h"
#include "test/test.h"

TEST("MF: SaveBlock3 size is locked")
{
    EXPECT_EQ(sizeof(struct SaveBlock3), MF_SAVEBLOCK3_SIZE);
    EXPECT_LE(sizeof(struct SaveBlock3), SAVE_BLOCK_3_CHUNK_SIZE * NUM_SECTORS_PER_SLOT);
    EXPECT_EQ(offsetof(struct SaveBlock3, mfOptions),
              offsetof(struct SaveBlock3, mfRules) + sizeof(struct ModernRules));
}

TEST("MF: save migrate on load upgrades v1 rules and keeps Options+")
{
    struct ModernRules *rules = MfRules_GetSaveRules();
    struct MfOptions *options = &gSaveBlock3Ptr->mfOptions;

    MfRules_ResetToEmpty(rules);
    rules->version = MF_RULES_VERSION_V1;
    rules->infiniteTms = TRUE;
    rules->nuzlocke = TRUE;
    rules->scaledExp = TRUE;
    rules->paddingTail = 63;
    rules->randomizerSeed = 0x11223344u;

    MfOptions_ResetToDefaults(options);
    options->autorun = TRUE;
    options->version = MF_OPTIONS_VERSION;

    MfSave_MigrateOnLoad();

    EXPECT_EQ(rules->version, MF_RULES_VERSION);
    EXPECT_EQ((u32)rules->infiniteTms, (u32)TRUE);
    EXPECT_EQ((u32)rules->nuzlocke, (u32)TRUE);
    EXPECT_EQ((u32)rules->scaledExp, (u32)TRUE);
    EXPECT_EQ((u32)rules->paddingTail, 0u);
    EXPECT_EQ(rules->randomizerSeed, 0x11223344u);
    EXPECT_EQ(MfRules_GetActiveRules(), rules);
    EXPECT_EQ(options->version, MF_OPTIONS_VERSION);
    EXPECT_EQ((u32)options->autorun, (u32)TRUE);
}

TEST("MF: save relocate Options+ when rules blob grows")
{
    struct MfOptions opts;
    u8 *base;
    u16 oldSize = sizeof(struct ModernRules) - 4;

    MfRules_ResetToEmpty(MfRules_GetSaveRules());
    MfRules_GetSaveRules()->nuzlockeEncounterFlags[MF_NUZLOCKE_ENCOUNTER_FLAG_BYTES - 1] = 0xABu;
    MfOptions_ResetToDefaults(&gSaveBlock3Ptr->mfOptions);
    gSaveBlock3Ptr->mfOptions.autorun = TRUE;
    gSaveBlock3Ptr->mfOptions.bagSort = 2;
    opts = gSaveBlock3Ptr->mfOptions;

    base = (u8 *)&gSaveBlock3Ptr->mfRules;
    memcpy(base + oldSize, &opts, sizeof(opts));
    memset(&gSaveBlock3Ptr->mfOptions, 0xFF, sizeof(struct MfOptions));

    MfSave_RelocateMfOptions(oldSize);

    EXPECT_EQ(gSaveBlock3Ptr->mfOptions.version, opts.version);
    EXPECT_EQ((u32)gSaveBlock3Ptr->mfOptions.autorun, (u32)TRUE);
    EXPECT_EQ((u32)gSaveBlock3Ptr->mfOptions.bagSort, 2u);
    EXPECT_EQ(base[oldSize], 0u);
    EXPECT_EQ(base[oldSize + 1], 0u);
    EXPECT_EQ(base[oldSize + 2], 0u);
    EXPECT_EQ(base[oldSize + 3], 0u);
}

TEST("MF: save migrate on load does not rewrite a newer rules blob")
{
    struct ModernRules *rules = MfRules_GetSaveRules();

    MfRules_ResetToEmpty(rules);
    rules->version = MF_RULES_VERSION + 9;
    rules->nuzlocke = TRUE;
    rules->randomizerSeed = 0xDEADBEEFu;

    MfSave_MigrateOnLoad();

    EXPECT_EQ(rules->version, (u16)(MF_RULES_VERSION + 9));
    EXPECT_EQ((u32)rules->nuzlocke, (u32)TRUE);
    EXPECT_EQ(rules->randomizerSeed, 0xDEADBEEFu);
    EXPECT_EQ(MfRules_GetActiveRules(), &gMfRulesPhase1Defaults);

    rules->version = MF_RULES_VERSION;
}
