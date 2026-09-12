#ifndef GUARD_DEBUG_H
#define GUARD_DEBUG_H

// Shared by upstream debug.c and Modern FireRed mf_debug menus (S05 / S17).
struct DebugMenuOption
{
    const u8 *text;
    const void *action;
    const void *actionParams;
};

void Debug_ShowMainMenu(void);
void DebugAction_OpenSubMenu(u8 taskId, const struct DebugMenuOption *items);
void DebugAction_Cancel(u8 taskId);

extern const u8 Debug_FlagsAndVarNotSetBattleConfigMessage[];
const u8 *GetWeatherName(u32 weatherId);
const struct Trainer* GetDebugAiTrainer(void);

void DebugNative_GetAbilityNames(void);
void DebugNative_Party_SetFriendship(void);

extern EWRAM_DATA bool8 gIsDebugBattle;
extern EWRAM_DATA u64 gDebugAIFlags;

#endif // GUARD_DEBUG_H
