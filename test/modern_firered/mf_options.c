#include "global.h"
#include "event_data.h"
#include "mf_options.h"
#include "save.h"
#include "test/test.h"
#include "constants/flags.h"
#include "constants/pokemon.h"
#include "config/battle.h"

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
    EXPECT_EQ((u32)options.fastIntro, 1u);
    EXPECT_EQ((u32)options.ballPrompt, 1u);
    EXPECT_EQ((u32)options.typeHints, 1u);
    EXPECT_EQ((u32)options.unitImperial, 1u);
    EXPECT_EQ((u32)options.cursorMemory, 1u);
    EXPECT_EQ((u32)options.runPrompt, 0u);
    EXPECT_EQ((u32)options.bagSort, 0u);
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
    EXPECT_EQ((u32)save->fastIntro, 1u);
    EXPECT_EQ((u32)save->ballPrompt, 1u);
    EXPECT_EQ((u32)save->typeHints, 1u);
    EXPECT_EQ((u32)save->unitImperial, 1u);
    EXPECT_EQ((u32)save->cursorMemory, 1u);
}

TEST("MF: Options+ GetSave repairs unrecognized version")
{
    struct MfOptions *save = &gSaveBlock3Ptr->mfOptions;

    memset(save, 0xFF, sizeof(*save));
    EXPECT_EQ(MfOptions_GetSave()->version, MF_OPTIONS_VERSION);
    EXPECT_EQ(MfOptions_Get(MF_OPT_AUTORUN), FALSE);
}

TEST("MF: L toggles Options+ autorun")
{
    struct MfOptions *save = MfOptions_GetSave();
    u16 oldNewKeys = gMain.newKeys;

    MfOptions_ResetToDefaults(save);
    gMain.newKeys = 0;
    EXPECT_EQ(MfOptions_TryToggleAutorun(), FALSE);
    EXPECT_EQ(MfOptions_Get(MF_OPT_AUTORUN), FALSE);

    gMain.newKeys = L_BUTTON;
    EXPECT_EQ(MfOptions_TryToggleAutorun(), TRUE);
    EXPECT_EQ(MfOptions_Get(MF_OPT_AUTORUN), TRUE);

    gMain.newKeys = L_BUTTON;
    EXPECT_EQ(MfOptions_TryToggleAutorun(), TRUE);
    EXPECT_EQ(MfOptions_Get(MF_OPT_AUTORUN), FALSE);

    gMain.newKeys = oldNewKeys;
}

TEST("MF: L=A button mode is clamped to Normal")
{
    u8 oldMode = gSaveBlock2Ptr->optionsButtonMode;

    gSaveBlock2Ptr->optionsButtonMode = OPTIONS_BUTTON_MODE_L_EQUALS_A;
    EXPECT_EQ(MfOptions_Get(MF_OPT_BUTTON_MODE), OPTIONS_BUTTON_MODE_NORMAL);
    EXPECT_EQ((u32)gSaveBlock2Ptr->optionsButtonMode, (u32)OPTIONS_BUTTON_MODE_NORMAL);

    MfOptions_Set(MF_OPT_BUTTON_MODE, OPTIONS_BUTTON_MODE_L_EQUALS_A);
    EXPECT_EQ((u32)gSaveBlock2Ptr->optionsButtonMode, (u32)OPTIONS_BUTTON_MODE_NORMAL);

    MfOptions_Set(MF_OPT_BUTTON_MODE, OPTIONS_BUTTON_MODE_LR);
    EXPECT_EQ(MfOptions_Get(MF_OPT_BUTTON_MODE), OPTIONS_BUTTON_MODE_LR);

    gSaveBlock2Ptr->optionsButtonMode = oldMode;
}

TEST("MF: Options+ autorun inverts hold-B to run")
{
    struct MfOptions *save = MfOptions_GetSave();

    MfOptions_ResetToDefaults(save);
    EXPECT_EQ(MfOptions_WantsToRun(0), FALSE);
    EXPECT_EQ(MfOptions_WantsToRun(B_BUTTON), TRUE);

    MfOptions_Set(MF_OPT_AUTORUN, TRUE);
    EXPECT_EQ(MfOptions_WantsToRun(0), TRUE);
    EXPECT_EQ(MfOptions_WantsToRun(B_BUTTON), FALSE);
}

TEST("MF: Options+ autorun surf inverts hold-B for extra surf speed")
{
    struct MfOptions *save = MfOptions_GetSave();

    MfOptions_ResetToDefaults(save);
    EXPECT_EQ(MfOptions_WantsSurfBoost(0), FALSE);
    EXPECT_EQ(MfOptions_WantsSurfBoost(B_BUTTON), TRUE);

    MfOptions_Set(MF_OPT_AUTORUN_SURF, TRUE);
    EXPECT_EQ(MfOptions_WantsSurfBoost(0), TRUE);
    EXPECT_EQ(MfOptions_WantsSurfBoost(B_BUTTON), FALSE);
}

TEST("MF: Even Faster Joy FAST shortens machine delays")
{
    struct MfOptions *save = MfOptions_GetSave();

    MfOptions_ResetToDefaults(save);
    EXPECT_EQ(MfOptions_Get(MF_OPT_EVEN_FASTER_JOY), MF_OPTION_JOY_OFF);
    EXPECT_EQ(MfOptions_JoyAnimDelay(25), 25);
    EXPECT_EQ(MfOptions_JoySkipFanfareWait(), FALSE);

    MfOptions_Set(MF_OPT_EVEN_FASTER_JOY, MF_OPTION_JOY_FAST);
    EXPECT_EQ(MfOptions_Get(MF_OPT_EVEN_FASTER_JOY), MF_OPTION_JOY_FAST);
    EXPECT_EQ(MfOptions_JoyAnimDelay(25), 6);
    EXPECT_EQ(MfOptions_JoyAnimDelay(3), 1);
    EXPECT_EQ(MfOptions_JoySkipFanfareWait(), TRUE);

    MfOptions_Set(MF_OPT_EVEN_FASTER_JOY, MF_OPTION_JOY_ON);
    EXPECT_EQ(MfOptions_Get(MF_OPT_EVEN_FASTER_JOY), MF_OPTION_JOY_ON);
    EXPECT_EQ(MfOptions_JoyAnimDelay(25), 25);
    EXPECT_EQ(MfOptions_JoySkipFanfareWait(), FALSE);
    EXPECT_EQ(MfOptions_ScriptGetEvenFasterJoy(), MF_OPTION_JOY_ON);
}

TEST("MF: S61 wait scale, skip intro, slide, and ball prompt")
{
    struct MfOptions *save = MfOptions_GetSave();

    MfOptions_ResetToDefaults(save);
    EXPECT_EQ(MfOptions_SkipBattleIntroSlide(), TRUE);
    EXPECT_EQ(MfOptions_ShowBallPrompt(), TRUE);
    EXPECT_EQ(MfOptions_ShouldSkipBootIntro(), FALSE);
    EXPECT_EQ(MfOptions_ScaleBattleWait(32), 32);
    EXPECT_EQ(MfOptions_ScaleBattleWait(8), 8);

    MfOptions_Set(MF_OPT_ANIM_SPEED, MF_OPTION_ANIM_2X);
    EXPECT_EQ(MfOptions_ScaleBattleWait(32), 16);
    EXPECT_EQ(MfOptions_ScaleBattleWait(1), 1);

    MfOptions_Set(MF_OPT_FAST_BATTLES, TRUE);
    EXPECT_EQ(MfOptions_ScaleBattleWait(32), 0);
    EXPECT_EQ(MfOptions_ScaleBattleWait(8), 0);

    MfOptions_Set(MF_OPT_FAST_INTRO, FALSE);
    MfOptions_Set(MF_OPT_BALL_PROMPT, FALSE);
    MfOptions_Set(MF_OPT_SKIP_INTRO, TRUE);
    EXPECT_EQ(MfOptions_SkipBattleIntroSlide(), FALSE);
    EXPECT_EQ(MfOptions_ShowBallPrompt(), FALSE);
    EXPECT_EQ(MfOptions_ShouldSkipBootIntro(), TRUE);
}

TEST("MF: Options+ v1 migrates FAST INTRO and BALL PROMPT on")
{
    struct MfOptions *save = &gSaveBlock3Ptr->mfOptions;

    memset(save, 0, sizeof(*save));
    save->version = 1;
    save->autorun = TRUE;
    EXPECT_EQ(MfOptions_GetSave()->version, MF_OPTIONS_VERSION);
    EXPECT_EQ(MfOptions_Get(MF_OPT_AUTORUN), TRUE);
    EXPECT_EQ(MfOptions_Get(MF_OPT_FAST_INTRO), TRUE);
    EXPECT_EQ(MfOptions_Get(MF_OPT_BALL_PROMPT), TRUE);
    EXPECT_EQ(MfOptions_Get(MF_OPT_TYPE_HINTS), TRUE);
    EXPECT_EQ(MfOptions_Get(MF_OPT_UNIT_IMPERIAL), TRUE);
    EXPECT_EQ(MfOptions_Get(MF_OPT_CURSOR_MEMORY), TRUE);
}

TEST("MF: Options+ v2 migrates type hints on")
{
    struct MfOptions *save = &gSaveBlock3Ptr->mfOptions;

    memset(save, 0, sizeof(*save));
    save->version = 2;
    save->fastIntro = TRUE;
    save->ballPrompt = TRUE;
    save->autorun = TRUE;
    EXPECT_EQ(MfOptions_GetSave()->version, MF_OPTIONS_VERSION);
    EXPECT_EQ(MfOptions_Get(MF_OPT_AUTORUN), TRUE);
    EXPECT_EQ(MfOptions_Get(MF_OPT_TYPE_HINTS), TRUE);
    EXPECT_EQ(MfOptions_Get(MF_OPT_UNIT_IMPERIAL), TRUE);
    EXPECT_EQ(MfOptions_Get(MF_OPT_CURSOR_MEMORY), TRUE);
}

TEST("MF: Options+ v3 migrates imperial units and cursor memory on")
{
    struct MfOptions *save = &gSaveBlock3Ptr->mfOptions;

    memset(save, 0, sizeof(*save));
    save->version = 3;
    save->fastIntro = TRUE;
    save->ballPrompt = TRUE;
    save->typeHints = TRUE;
    save->autorun = TRUE;
    EXPECT_EQ(MfOptions_GetSave()->version, MF_OPTIONS_VERSION);
    EXPECT_EQ(MfOptions_Get(MF_OPT_AUTORUN), TRUE);
    EXPECT_EQ(MfOptions_Get(MF_OPT_UNIT_IMPERIAL), TRUE);
    EXPECT_EQ(MfOptions_Get(MF_OPT_CURSOR_MEMORY), TRUE);
    EXPECT_EQ(MfOptions_UseImperialUnits(), TRUE);
    EXPECT_EQ(MfOptions_RememberCursors(), TRUE);
}

TEST("MF: type hints default On; STAB helper uses live types")
{
    struct MfOptions *save = MfOptions_GetSave();

    MfOptions_ResetToDefaults(save);
    EXPECT_EQ(MfOptions_ShowTypeHints(), TRUE);
    EXPECT_EQ(MfOptions_GetShowTypesMode(), SHOW_TYPES_SEEN);
    EXPECT_EQ(MfOptions_IsStabType(TYPE_FIRE, TYPE_FIRE, TYPE_FLYING), TRUE);
    EXPECT_EQ(MfOptions_IsStabType(TYPE_WATER, TYPE_FIRE, TYPE_FLYING), FALSE);
    EXPECT_EQ(MfOptions_IsStabType(TYPE_MYSTERY, TYPE_MYSTERY, TYPE_MYSTERY), FALSE);

    MfOptions_Set(MF_OPT_TYPE_HINTS, FALSE);
    EXPECT_EQ(MfOptions_ShowTypeHints(), FALSE);
    EXPECT_EQ(MfOptions_GetShowTypesMode(), SHOW_TYPES_NEVER);
    EXPECT_EQ(MfOptions_IsStabType(TYPE_FIRE, TYPE_FIRE, TYPE_FLYING), FALSE);
}

TEST("MF: S63 units, cursor memory, bag sort, run prompt")
{
    struct MfOptions *save = MfOptions_GetSave();

    MfOptions_ResetToDefaults(save);
    EXPECT_EQ(MfOptions_UseImperialUnits(), TRUE);
    EXPECT_EQ(MfOptions_RememberCursors(), TRUE);
    EXPECT_EQ(MfOptions_Get(MF_OPT_RUN_PROMPT), FALSE);
    EXPECT_EQ(MfOptions_Get(MF_OPT_BAG_SORT), 0);
    EXPECT_EQ(MfOptions_RunPromptPressed(), FALSE);

    MfOptions_Set(MF_OPT_UNIT_IMPERIAL, FALSE);
    MfOptions_Set(MF_OPT_CURSOR_MEMORY, FALSE);
    MfOptions_Set(MF_OPT_RUN_PROMPT, TRUE);
    MfOptions_Set(MF_OPT_BAG_SORT, 2);
    EXPECT_EQ(MfOptions_UseImperialUnits(), FALSE);
    EXPECT_EQ(MfOptions_RememberCursors(), FALSE);
    EXPECT_EQ(MfOptions_Get(MF_OPT_RUN_PROMPT), TRUE);
    EXPECT_EQ(MfOptions_Get(MF_OPT_BAG_SORT), 2);
}

TEST("MF: movement probe matches autorun and avatar flags")
{
    struct MfOptions *save = MfOptions_GetSave();
    struct MfMovementProbe probe;
    u8 oldShoes = FlagGet(FLAG_SYS_B_DASH);
    u8 oldFlags = gPlayerAvatar.flags;

    MfOptions_ResetToDefaults(save);
    FlagSet(FLAG_SYS_B_DASH);
    gPlayerAvatar.flags = PLAYER_AVATAR_FLAG_ON_FOOT;
    MfOptions_FillMovementProbe(&probe);
    EXPECT_EQ(probe.shoes, TRUE);
    EXPECT_EQ(probe.wantRun, FALSE);
    EXPECT_EQ(probe.wantRunB, TRUE);
    EXPECT_EQ(probe.onFoot, TRUE);
    EXPECT_EQ(probe.surfing, FALSE);

    MfOptions_Set(MF_OPT_AUTORUN, TRUE);
    MfOptions_Set(MF_OPT_AUTORUN_SURF, TRUE);
    gPlayerAvatar.flags = PLAYER_AVATAR_FLAG_SURFING;
    MfOptions_FillMovementProbe(&probe);
    EXPECT_EQ(probe.wantRun, TRUE);
    EXPECT_EQ(probe.wantRunB, FALSE);
    EXPECT_EQ(probe.wantSurf, TRUE);
    EXPECT_EQ(probe.wantSurfB, FALSE);
    EXPECT_EQ(probe.surfing, TRUE);
    EXPECT_EQ(probe.onFoot, FALSE);

    gPlayerAvatar.flags = oldFlags;
    if (oldShoes)
        FlagSet(FLAG_SYS_B_DASH);
    else
        FlagClear(FLAG_SYS_B_DASH);
}
