#include "global.h"
#include "debug.h"
#include "mf_debug.h"
#include "sound.h"
#include "constants/songs.h"

// Placeholder until S17 hangs the rules inspector here.
static void MfDebug_Action_RulesStub(u8 taskId)
{
    PlaySE(SE_SELECT);
    (void)taskId;
}

const struct DebugMenuOption gMfDebugMenuOptions[] =
{
    { COMPOUND_STRING("Rules inspector…"), MfDebug_Action_RulesStub },
    { COMPOUND_STRING("Cancel"),           DebugAction_Cancel },
    { NULL }
};
