#include "global.h"
#include "mf_options.h"
#include "save.h"
#include "test/test.h"

TEST("MF: Options+ struct fits SaveBlock3 budget")
{
    EXPECT_EQ(sizeof(struct MfOptions), 4u);
    EXPECT_LE(sizeof(struct SaveBlock3), SAVE_BLOCK_3_CHUNK_SIZE * NUM_SECTORS_PER_SLOT);
    EXPECT_EQ(offsetof(struct SaveBlock3, mfOptions),
              offsetof(struct SaveBlock3, mfRules) + sizeof(struct ModernRules));
}

TEST("MF: Options+ reset seeds version and vanilla-off extras")
{
    struct MfOptions options;

    memset(&options, 0xA5, sizeof(options));
    MfOptions_ResetToDefaults(&options);

    EXPECT_EQ(options.version, MF_OPTIONS_VERSION);
    EXPECT_EQ((u32)options.autorun, 0u);
    EXPECT_EQ((u32)options.autorunSurf, 0u);
    EXPECT_EQ((u32)options.evenFasterJoy, 0u);
    EXPECT_EQ((u32)options.skipIntro, 0u);
    EXPECT_EQ((u32)options.fastBattles, 0u);
    EXPECT_EQ((u32)options.animSpeed, 0u);
}

TEST("MF: Options+ Get/Set vanilla fields stay on SaveBlock2")
{
    u8 oldSpeed = gSaveBlock2Ptr->optionsTextSpeed;
    u8 oldScene = gSaveBlock2Ptr->optionsBattleSceneOff;
    u8 oldStyle = gSaveBlock2Ptr->optionsBattleStyle;

    MfOptions_Set(MF_OPT_TEXT_SPEED, OPTIONS_TEXT_SPEED_INSTANT);
    MfOptions_Set(MF_OPT_BATTLE_SCENE, TRUE);
    MfOptions_Set(MF_OPT_BATTLE_STYLE, OPTIONS_BATTLE_STYLE_SET);

    EXPECT_EQ((u32)gSaveBlock2Ptr->optionsTextSpeed, (u32)OPTIONS_TEXT_SPEED_INSTANT);
    EXPECT_EQ((u32)gSaveBlock2Ptr->optionsBattleSceneOff, (u32)TRUE);
    EXPECT_EQ((u32)gSaveBlock2Ptr->optionsBattleStyle, (u32)OPTIONS_BATTLE_STYLE_SET);
    EXPECT_EQ(MfOptions_Get(MF_OPT_TEXT_SPEED), OPTIONS_TEXT_SPEED_INSTANT);
    EXPECT_EQ(MfOptions_Get(MF_OPT_BATTLE_SCENE), TRUE);
    EXPECT_EQ(MfOptions_Get(MF_OPT_BATTLE_STYLE), OPTIONS_BATTLE_STYLE_SET);

    gSaveBlock2Ptr->optionsTextSpeed = oldSpeed;
    gSaveBlock2Ptr->optionsBattleSceneOff = oldScene;
    gSaveBlock2Ptr->optionsBattleStyle = oldStyle;
}

TEST("MF: Options+ extras persist on SaveBlock3")
{
    struct MfOptions *save = MfOptions_GetSave();

    MfOptions_ResetToDefaults(save);
    MfOptions_Set(MF_OPT_AUTORUN, TRUE);
    MfOptions_Set(MF_OPT_EVEN_FASTER_JOY, MF_OPTION_JOY_ON);
    MfOptions_Set(MF_OPT_ANIM_SPEED, MF_OPTION_ANIM_2X);
    MfOptions_Set(MF_OPT_TYPE_HINTS, TRUE);

    EXPECT_EQ(MfOptions_Get(MF_OPT_AUTORUN), TRUE);
    EXPECT_EQ(MfOptions_Get(MF_OPT_EVEN_FASTER_JOY), MF_OPTION_JOY_ON);
    EXPECT_EQ(MfOptions_Get(MF_OPT_ANIM_SPEED), MF_OPTION_ANIM_2X);
    EXPECT_EQ(MfOptions_Get(MF_OPT_TYPE_HINTS), TRUE);
    EXPECT_EQ((u32)save->autorun, (u32)TRUE);
    EXPECT_EQ((u32)save->evenFasterJoy, (u32)MF_OPTION_JOY_ON);
}

TEST("MF: Options+ InitNewGame repairs junk version")
{
    struct MfOptions *save = &gSaveBlock3Ptr->mfOptions;

    memset(save, 0xFF, sizeof(*save));
    MfOptions_InitNewGame();
    EXPECT_EQ(save->version, MF_OPTIONS_VERSION);
    EXPECT_EQ((u32)save->autorun, 0u);
}

TEST("MF: Options+ GetSave repairs unrecognized version")
{
    struct MfOptions *save = &gSaveBlock3Ptr->mfOptions;

    memset(save, 0xFF, sizeof(*save));
    EXPECT_EQ(MfOptions_GetSave()->version, MF_OPTIONS_VERSION);
    EXPECT_EQ(MfOptions_Get(MF_OPT_AUTORUN), FALSE);
}
