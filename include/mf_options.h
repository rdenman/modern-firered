#ifndef GUARD_MF_OPTIONS_H
#define GUARD_MF_OPTIONS_H

// S58 — Options+ preferences (mid-run, unlike ModernRules). Vanilla fields
// stay in SaveBlock2; extra QoL bits live in SaveBlock3.mfOptions.

#include "gba/types.h"

#define MF_OPTIONS_VERSION 1
#define MF_OPTIONS_MAX_BYTES 8

enum MfOptionJoy
{
    MF_OPTION_JOY_OFF = 0,
    MF_OPTION_JOY_ON = 1,
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
             u8 padding0:2;
    /*0x03*/ u8 padding1;
};

typedef char mf_options_fits_in_budget[(sizeof(struct MfOptions) <= MF_OPTIONS_MAX_BYTES) ? 1 : -1];
typedef char mf_options_packed_size[(sizeof(struct MfOptions) == 4) ? 1 : -1];

void MfOptions_ResetToDefaults(struct MfOptions *options);
void MfOptions_InitNewGame(void);
struct MfOptions *MfOptions_GetSave(void);

u8 MfOptions_Get(u8 id);
void MfOptions_Set(u8 id, u8 value);

void MfOptions_DebugDump(void);

#endif // GUARD_MF_OPTIONS_H
