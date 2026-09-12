#include "global.h"
#include "debug.h"
#include "mf_debug.h"
#include "mf_rules.h"
#include "sound.h"
#include "constants/songs.h"

// Placeholder until S17 hangs the full rules inspector here.
static void MfDebug_Action_RulesStub(u8 taskId)
{
    PlaySE(SE_SELECT);
    (void)taskId;
}

// S15: session unlock so locked rules can be written in non-release builds.
// S17 will fold this into the inspector UI.
static void MfDebug_Action_ToggleRulesUnlock(u8 taskId)
{
    PlaySE(SE_SELECT);
    (void)taskId;

#if MF_RULES_ENGINE
    if (MfRules_DebugHasUnlockOverride())
        MfRules_DebugSetUnlockOverride(FALSE);
    else if (!MfRules_DebugSetUnlockOverride(TRUE))
        PlaySE(SE_FAILURE);
#endif
}

const struct DebugMenuOption gMfDebugMenuOptions[] =
{
    { COMPOUND_STRING("Rules inspector…"), MfDebug_Action_RulesStub },
    { COMPOUND_STRING("Unlock rules (dbg)"), MfDebug_Action_ToggleRulesUnlock },
    { COMPOUND_STRING("Cancel"),           DebugAction_Cancel },
    { NULL }
};
