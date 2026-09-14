#ifndef GUARD_MF_RULES_MENU_H
#define GUARD_MF_RULES_MENU_H

// S18 — reusable FireRed-styled rules menu shell (data-driven pages).
// S19 — new-game hook + mid-run entry. S20 — Gamemode; S21 — Features;
// S22 — Nuzlocke; S23 — Difficulty; S24 — Challenges; S25 — Randomizer.
// S26 — SAVE confirm/lock + read-only mid-run viewer.

void CB2_InitMfRulesMenu(void);

// Read-only viewer of the active rule set (ME tx_rac_viewer spirit). Start menu RULES.
void CB2_InitMfRulesViewer(void);

// After Oak/Birch speech (or quickstart): init save + presets, show menu, then overworld.
void CB2_MfRules_BeginNewGame(void);

// ME Task_ChooseChallenge_NoNewGame equivalent: open menu without NewGameInitData.
void Task_MfRulesMenu_NoNewGame(u8 taskId);

#endif // GUARD_MF_RULES_MENU_H
