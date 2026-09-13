# 0022 — Nuzlocke page: tier packing, Easy mini-mode gate

- **Type:** ux
- **Status:** Accepted
- **Date:** 2026-09-13
- **Story:** S22
- **ME reference:** `MENUITEM_NUZLOCKE_*` / `DrawChoices_Challenges_Nuzlocke` / `CheckConditions` in `src/tx_rac_menu.c`
- **Expansion config:** —

## Context

ME’s Nuzlocke master control is a four-way Off/Easy/Normal/Hard selector that packs three save bits (`tx_Challenges_Nuzlocke`, `tx_Challenges_NuzlockeHardcore`, `tx_Nuzlocke_EasyMode`). Easy is “mini mode”: faint retirement only, with DUPES/SHINY/NICKNAMES/FAINTING disabled. Our rules struct already stores those bits separately; the menu needs a single value id and gating that match ME without inventing gameplay yet (Phase 3 UI-only).

## Decision

1. **`MF_RULE_VAL_NUZLOCKE_MODE`** (`MfNuzlockeMode`: Off/Easy/Normal/Hardcore) packs/unpacks the three bitfields via `MfRules_TrySetValue`. Choice labels use ME’s **Hard** (not “Hardcore”).
2. **Sub-options** (`DUPES CLAUSE`, `SHINY CLAUSE`, `NICKNAMES`, `FAINTING`) use `MF_RULES_MENU_FLAG_REQUIRES_NUZLOCKE`, editable only when Normal or Hardcore (`MfRules_NuzlockeSubOptionsActive`). Off **and** Easy grey them out; description shows “Only usable with Nuzlocke!” (ME disabled copy).
3. **Clause side effects on mode change** (ME save/draw spirit, adapted to live writes): leaving full Nuzlocke clears clause/deletion flags; entering Normal/Hard from Off/Easy seeds `MF_TX_NUZLOCKE_*` recommended defaults. Normal↔Hardcore keeps current clause values.
4. **FAINTING** labels are Cemetery / Release (deletion false/true), matching ME. Clause toggles stay Off/On (not ME’s inverted On-left draw order) so bool indices stay consistent with the rest of the menu.
5. Page order remains STORIES.md Phase 3: Gamemode → Features → **Nuzlocke** → stub CONTINUE.

## Alternatives considered

- Three separate bool rows for Nuzlocke/Easy/Hardcore — rejected; ME’s four-way control is the UX spec.
- Gate sub-options only when Off (story wording) — rejected; ME also disables them on Easy mini mode, and the story asks for mini mode.
- Keep ME’s On/Off inverted selection indices — rejected; conflicts with our Off=0/On=1 shell and debug inspector.

## Consequences

- S35–S39 / S36 faint handling can read `MfRules_GetValue(MF_RULE_VAL_NUZLOCKE_MODE)` or the typed bool helpers.
- Debug Nuzlocke page still exposes individual bools; the rules menu is the player-facing packed control.
- Stub CONTINUE remains after Nuzlocke until S23–S25.
