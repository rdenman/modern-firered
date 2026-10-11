#include "global.h"
#include "mf_options.h"
#include "event_data.h"
#include "field_player_avatar.h"
#include "gba/isagbprint.h"
#include "string_util.h"
#include "constants/flags.h"

#if MF_OPTIONS_PLUS

static void Sanitize(struct MfOptions *options)
{
    if (options == NULL)
        return;
    if (options->version != MF_OPTIONS_VERSION)
        MfOptions_ResetToDefaults(options);
}

void MfOptions_ResetToDefaults(struct MfOptions *options)
{
    if (options == NULL)
        return;
    memset(options, 0, sizeof(*options));
    options->version = MF_OPTIONS_VERSION;
}

void MfOptions_InitNewGame(void)
{
    MfOptions_ResetToDefaults(&gSaveBlock3Ptr->mfOptions);
}

struct MfOptions *MfOptions_GetSave(void)
{
    struct MfOptions *options = &gSaveBlock3Ptr->mfOptions;

    Sanitize(options);
    return options;
}

u8 MfOptions_Get(u8 id)
{
    struct MfOptions *o;

    if (gSaveBlock2Ptr == NULL)
        return 0;

    switch (id)
    {
    case MF_OPT_TEXT_SPEED:
        return gSaveBlock2Ptr->optionsTextSpeed;
    case MF_OPT_BATTLE_SCENE:
        return gSaveBlock2Ptr->optionsBattleSceneOff;
    case MF_OPT_BATTLE_STYLE:
        return gSaveBlock2Ptr->optionsBattleStyle;
    case MF_OPT_SOUND:
        return gSaveBlock2Ptr->optionsSound;
    case MF_OPT_BUTTON_MODE:
        return gSaveBlock2Ptr->optionsButtonMode;
    case MF_OPT_FRAME:
        return gSaveBlock2Ptr->optionsWindowFrameType;
    default:
        break;
    }

    if (gSaveBlock3Ptr == NULL)
        return 0;

    o = MfOptions_GetSave();
    switch (id)
    {
    case MF_OPT_AUTORUN:          return o->autorun;
    case MF_OPT_AUTORUN_SURF:     return o->autorunSurf;
    case MF_OPT_EVEN_FASTER_JOY:  return o->evenFasterJoy;
    case MF_OPT_SKIP_INTRO:       return o->skipIntro;
    case MF_OPT_UNIT_IMPERIAL:    return o->unitImperial;
    case MF_OPT_FAST_BATTLES:     return o->fastBattles;
    case MF_OPT_FAST_INTRO:       return o->fastIntro;
    case MF_OPT_ANIM_SPEED:       return o->animSpeed;
    case MF_OPT_BALL_PROMPT:      return o->ballPrompt;
    case MF_OPT_TYPE_HINTS:       return o->typeHints;
    case MF_OPT_CURSOR_MEMORY:    return o->cursorMemory;
    case MF_OPT_RUN_PROMPT:       return o->runPrompt;
    default:                      return 0;
    }
}

void MfOptions_Set(u8 id, u8 value)
{
    struct MfOptions *o;

    if (gSaveBlock2Ptr == NULL)
        return;

    switch (id)
    {
    case MF_OPT_TEXT_SPEED:
        gSaveBlock2Ptr->optionsTextSpeed = value;
        return;
    case MF_OPT_BATTLE_SCENE:
        gSaveBlock2Ptr->optionsBattleSceneOff = value;
        return;
    case MF_OPT_BATTLE_STYLE:
        gSaveBlock2Ptr->optionsBattleStyle = value;
        return;
    case MF_OPT_SOUND:
        gSaveBlock2Ptr->optionsSound = value;
        return;
    case MF_OPT_BUTTON_MODE:
        gSaveBlock2Ptr->optionsButtonMode = value;
        return;
    case MF_OPT_FRAME:
        gSaveBlock2Ptr->optionsWindowFrameType = value;
        return;
    default:
        break;
    }

    if (gSaveBlock3Ptr == NULL)
        return;

    o = MfOptions_GetSave();
    switch (id)
    {
    case MF_OPT_AUTORUN:          o->autorun = value; break;
    case MF_OPT_AUTORUN_SURF:     o->autorunSurf = value; break;
    case MF_OPT_EVEN_FASTER_JOY:  o->evenFasterJoy = value; break;
    case MF_OPT_SKIP_INTRO:       o->skipIntro = value; break;
    case MF_OPT_UNIT_IMPERIAL:    o->unitImperial = value; break;
    case MF_OPT_FAST_BATTLES:     o->fastBattles = value; break;
    case MF_OPT_FAST_INTRO:       o->fastIntro = value; break;
    case MF_OPT_ANIM_SPEED:       o->animSpeed = value; break;
    case MF_OPT_BALL_PROMPT:      o->ballPrompt = value; break;
    case MF_OPT_TYPE_HINTS:       o->typeHints = value; break;
    case MF_OPT_CURSOR_MEMORY:    o->cursorMemory = value; break;
    case MF_OPT_RUN_PROMPT:       o->runPrompt = value; break;
    default:                      break;
    }
}

bool8 MfOptions_WantsToRun(u16 heldKeys)
{
    bool8 bHeld = (heldKeys & B_BUTTON) != 0;

    if (MfOptions_Get(MF_OPT_AUTORUN))
        return !bHeld;
    return bHeld;
}

bool8 MfOptions_WantsSurfBoost(u16 heldKeys)
{
    bool8 bHeld = (heldKeys & B_BUTTON) != 0;

    if (MfOptions_Get(MF_OPT_AUTORUN_SURF))
        return !bHeld;
    return bHeld;
}

void MfOptions_FillMovementProbe(struct MfMovementProbe *out)
{
    if (out == NULL)
        return;
    memset(out, 0, sizeof(*out));
    out->shoes = FlagGet(FLAG_SYS_B_DASH);
    out->autorun = MfOptions_Get(MF_OPT_AUTORUN);
    out->autorunSurf = MfOptions_Get(MF_OPT_AUTORUN_SURF);
    out->wantRun = MfOptions_WantsToRun(0);
    out->wantRunB = MfOptions_WantsToRun(B_BUTTON);
    out->wantSurf = MfOptions_WantsSurfBoost(0);
    out->wantSurfB = MfOptions_WantsSurfBoost(B_BUTTON);
    out->onFoot = (gPlayerAvatar.flags & PLAYER_AVATAR_FLAG_ON_FOOT) != 0;
    out->machBike = (gPlayerAvatar.flags & PLAYER_AVATAR_FLAG_MACH_BIKE) != 0;
    out->acroBike = (gPlayerAvatar.flags & PLAYER_AVATAR_FLAG_ACRO_BIKE) != 0;
    out->surfing = (gPlayerAvatar.flags & PLAYER_AVATAR_FLAG_SURFING) != 0;
    out->dash = (gPlayerAvatar.flags & PLAYER_AVATAR_FLAG_DASH) != 0;
    out->allowRunning = gMapHeader.allowRunning;
    out->mapType = gMapHeader.mapType;
    if (gSaveBlock1Ptr != NULL)
    {
        out->mapGroup = gSaveBlock1Ptr->location.mapGroup;
        out->mapNum = gSaveBlock1Ptr->location.mapNum;
    }
    PlayerGetDestCoords(&out->x, &out->y);
}

void MfOptions_DebugDumpMovement(void)
{
    struct MfMovementProbe p;

    MfOptions_FillMovementProbe(&p);
    DebugPrintfLevel(MGBA_LOG_WARN, "=== MF movement dump ===");
    DebugPrintfLevel(MGBA_LOG_WARN, "map g=%u n=%u xy=%d,%d type=%u allowRun=%u",
        p.mapGroup, p.mapNum, p.x, p.y, p.mapType, p.allowRunning);
    DebugPrintfLevel(MGBA_LOG_WARN, "shoes=%u dash=%u foot=%u mach=%u acro=%u surf=%u",
        p.shoes, p.dash, p.onFoot, p.machBike, p.acroBike, p.surfing);
    DebugPrintfLevel(MGBA_LOG_WARN, "autorun=%u surfOpt=%u wantRun=%u wantRunB=%u wantSurf=%u wantSurfB=%u",
        p.autorun, p.autorunSurf, p.wantRun, p.wantRunB, p.wantSurf, p.wantSurfB);
}

void MfOptions_DebugDump(void)
{
    DebugPrintfLevel(MGBA_LOG_WARN, "=== MF options dump ===");
    DebugPrintfLevel(MGBA_LOG_WARN, "sb2: text=%u sceneOff=%u style=%u sound=%u btn=%u frame=%u",
        MfOptions_Get(MF_OPT_TEXT_SPEED),
        MfOptions_Get(MF_OPT_BATTLE_SCENE),
        MfOptions_Get(MF_OPT_BATTLE_STYLE),
        MfOptions_Get(MF_OPT_SOUND),
        MfOptions_Get(MF_OPT_BUTTON_MODE),
        MfOptions_Get(MF_OPT_FRAME));
    DebugPrintfLevel(MGBA_LOG_WARN, "plus: run=%u surf=%u joy=%u skip=%u unitImp=%u fastBat=%u fastIn=%u",
        MfOptions_Get(MF_OPT_AUTORUN),
        MfOptions_Get(MF_OPT_AUTORUN_SURF),
        MfOptions_Get(MF_OPT_EVEN_FASTER_JOY),
        MfOptions_Get(MF_OPT_SKIP_INTRO),
        MfOptions_Get(MF_OPT_UNIT_IMPERIAL),
        MfOptions_Get(MF_OPT_FAST_BATTLES),
        MfOptions_Get(MF_OPT_FAST_INTRO));
    DebugPrintfLevel(MGBA_LOG_WARN, "plus: anim=%u ball=%u hints=%u cursor=%u runPrompt=%u ver=%u",
        MfOptions_Get(MF_OPT_ANIM_SPEED),
        MfOptions_Get(MF_OPT_BALL_PROMPT),
        MfOptions_Get(MF_OPT_TYPE_HINTS),
        MfOptions_Get(MF_OPT_CURSOR_MEMORY),
        MfOptions_Get(MF_OPT_RUN_PROMPT),
        (gSaveBlock3Ptr != NULL) ? gSaveBlock3Ptr->mfOptions.version : 0);
}

#else // !MF_OPTIONS_PLUS

void MfOptions_ResetToDefaults(struct MfOptions *options)
{
    if (options == NULL)
        return;
    memset(options, 0, sizeof(*options));
}

void MfOptions_InitNewGame(void)
{
}

struct MfOptions *MfOptions_GetSave(void)
{
    return &gSaveBlock3Ptr->mfOptions;
}

u8 MfOptions_Get(u8 id)
{
    (void)id;
    return 0;
}

void MfOptions_Set(u8 id, u8 value)
{
    (void)id;
    (void)value;
}

bool8 MfOptions_WantsToRun(u16 heldKeys)
{
    return (heldKeys & B_BUTTON) != 0;
}

bool8 MfOptions_WantsSurfBoost(u16 heldKeys)
{
    (void)heldKeys;
    return FALSE;
}

void MfOptions_FillMovementProbe(struct MfMovementProbe *out)
{
    if (out == NULL)
        return;
    memset(out, 0, sizeof(*out));
    out->wantRunB = TRUE;
}

void MfOptions_DebugDump(void)
{
}

void MfOptions_DebugDumpMovement(void)
{
}

#endif
