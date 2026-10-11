#ifndef GUARD_MF_OPTIONS_H
#define GUARD_MF_OPTIONS_H

// S58 — Options+ preferences (mid-run, unlike ModernRules). Vanilla fields
// stay in SaveBlock2; extra QoL bits live in SaveBlock3.mfOptions.

#include "gba/types.h"

#define MF_OPTIONS_VERSION 4
#define MF_OPTIONS_MAX_BYTES 8

enum MfOptionJoy
{
    MF_OPTION_JOY_OFF = 0,
    MF_OPTION_JOY_FAST = 1, // machine anim, shorter waits
    MF_OPTION_JOY_ON = 2,   // skip prompt + machine (ME EVEN FASTER JOY)
};

enum MfOptionAnimSpeed
{
    MF_OPTION_ANIM_1X = 0,
    MF_OPTION_ANIM_2X = 1,
};

enum MfOptionId
{
    // SaveBlock2 vanilla
    MF_OPT_TEXT_SPEED,
    MF_OPT_BATTLE_SCENE, // 0 On, 1 Off (matches optionsBattleSceneOff)
    MF_OPT_BATTLE_STYLE,
    MF_OPT_SOUND,
    MF_OPT_BUTTON_MODE,
    MF_OPT_FRAME,
    // SaveBlock3 extras (wired in S59–S63)
    MF_OPT_AUTORUN,
    MF_OPT_AUTORUN_SURF,
    MF_OPT_EVEN_FASTER_JOY,
    MF_OPT_SKIP_INTRO,
    MF_OPT_UNIT_IMPERIAL,
    MF_OPT_FAST_BATTLES,
    MF_OPT_FAST_INTRO,
    MF_OPT_ANIM_SPEED,
    MF_OPT_BALL_PROMPT,
    MF_OPT_TYPE_HINTS,
    MF_OPT_CURSOR_MEMORY,
    MF_OPT_RUN_PROMPT,
    MF_OPT_BAG_SORT,
    MF_OPT_COUNT,
};

struct MfOptions
{
    /*0x00*/ u8 version;
    /*0x01*/ u8 autorun:1;
             u8 autorunSurf:1;
             u8 evenFasterJoy:2;
             u8 skipIntro:1;
             u8 unitImperial:1;
             u8 fastBattles:1;
             u8 fastIntro:1;
    /*0x02*/ u8 animSpeed:2;
             u8 ballPrompt:1;
             u8 typeHints:1;
             u8 cursorMemory:1;
             u8 runPrompt:1;
             u8 bagSort:2;
    /*0x03*/ u8 padding1;
};

typedef char mf_options_fits_in_budget[(sizeof(struct MfOptions) <= MF_OPTIONS_MAX_BYTES) ? 1 : -1];
typedef char mf_options_packed_size[(sizeof(struct MfOptions) == 4) ? 1 : -1];

void MfOptions_ResetToDefaults(struct MfOptions *options);
void MfOptions_InitNewGame(void);
struct MfOptions *MfOptions_GetSave(void);

u8 MfOptions_Get(u8 id);
void MfOptions_Set(u8 id, u8 value);

bool8 MfOptions_WantsToRun(u16 heldKeys);
bool8 MfOptions_WantsSurfBoost(u16 heldKeys);

u8 MfOptions_JoyAnimDelay(u8 frames);
bool8 MfOptions_JoySkipFanfareWait(void);
u16 MfOptions_ScriptGetEvenFasterJoy(void);

bool8 MfOptions_ShouldSkipBootIntro(void);
bool8 MfOptions_SkipBattleIntroSlide(void);
bool8 MfOptions_ShowBallPrompt(void);
u16 MfOptions_ScaleBattleWait(u16 frames);

bool8 MfOptions_ShowTypeHints(void);
u8 MfOptions_GetShowTypesMode(void);
bool8 MfOptions_IsStabType(u8 moveType, u8 type1, u8 type2);

bool8 MfOptions_UseImperialUnits(void);
bool8 MfOptions_RememberCursors(void);
bool8 MfOptions_RunPromptPressed(void);

// Snapshot for mGBA dump / tests. Do not score run/surf from sprites.
struct MfMovementProbe
{
    u8 shoes;
    u8 autorun;
    u8 autorunSurf;
    u8 wantRun;
    u8 wantRunB;
    u8 wantSurf;
    u8 wantSurfB;
    u8 onFoot;
    u8 machBike;
    u8 acroBike;
    u8 surfing;
    u8 dash;
    u8 allowRunning;
    u8 mapType;
    u8 mapGroup;
    u8 mapNum;
    s16 x;
    s16 y;
};

void MfOptions_FillMovementProbe(struct MfMovementProbe *out);
void MfOptions_DebugDump(void);
void MfOptions_DebugDumpMovement(void);

#endif // GUARD_MF_OPTIONS_H
