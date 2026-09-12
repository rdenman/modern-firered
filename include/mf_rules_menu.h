#ifndef GUARD_MF_RULES_MENU_H
#define GUARD_MF_RULES_MENU_H

// S18 — reusable FireRed-styled rules menu shell (data-driven pages).
// S19 — new-game hook + mid-run entry. Page tables land in S20–S25.

void CB2_InitMfRulesMenu(void);

// After Oak/Birch speech (or quickstart): init save + presets, show menu, then overworld.
void CB2_MfRules_BeginNewGame(void);

// ME Task_ChooseChallenge_NoNewGame equivalent: open menu without NewGameInitData.
void Task_MfRulesMenu_NoNewGame(u8 taskId);

#endif // GUARD_MF_RULES_MENU_H
