# 0019 — Rules menu after NewGameInitData, before overworld

- **Type:** tech
- **Status:** Accepted
- **Date:** 2026-09-12
- **Story:** S19
- **ME reference:** `Task_NewGameBirchSpeech_ChooseChallenge` / `CB2_InitTxRandomizerChallengesMenu` in ME `main_menu.c` + `tx_rac_menu.c`; `Task_ChooseChallenge_NoNewGame`
- **Expansion config:** —

## Context

ME opens the RAC menu mid–Birch speech (before naming). FireRed’s new-game path is Oak speech (`oak_speech.c`) ending in `CB2_NewGame`, which calls `NewGameInitData()` → `ClearSav3()` then `MfRules_InitNewGame()`. Writing rules to `SaveBlock3` *before* that clear would wipe the player’s choices. Story S19 also requires the menu after naming and before the overworld, with the smallest upstream call sites.

## Decision

1. **Split** `CB2_NewGame` into `CB2_PrepareNewGameData()` (clear/init including unlocked presets) and `CB2_ContinueNewGame()` (play-time start + map load). Bare `CB2_NewGame` still does both for any leftover callers.
2. **Orchestrate** in `CB2_MfRules_BeginNewGame`: Prepare → open `CB2_InitMfRulesMenu` with `savedCallback = CB2_ContinueNewGame`.
3. **Hook** with one-line replacements: Oak speech free-resources, Birch cleanup (non-FRLG), and quickstart skip-intro all call `CB2_MfRules_BeginNewGame` instead of `CB2_NewGame`.
4. **`Task_MfRulesMenu_NoNewGame`**: ME mid-run entry equivalent (no `NewGameInitData`); returns to field. Debug “Rules menu demo…” opens the same CB2 path.
5. **`MfRules_InitNewGame` no longer commits/locks** (ADR 0015 / 0014); S26 SAVE owns `MfRules_CommitAndLock`. Cancel/B/EXIT keeps the S14 Modern (or `MF_DEFAULT_GAMEMODE_PRESET`) seed.

## Alternatives considered

- Menu before `NewGameInitData`, stash choices in EWRAM — rejected; duplicates the save blob and risks desync with ClearSav3.
- Preserve `mfRules` across `ClearSav3` like the rival name — rejected; more upstream save churn for no gain once Prepare-then-menu works.
- Insert ME-style mid–Oak-speech dialogue before naming — rejected; FR story says after naming; Oak speech is already long.

## Consequences

- Until S26, new games leave `rulesLocked == FALSE` (Phase 3 menus can still edit mid-run via TrySet / debug unlock).
- `CB2_ContinueNewGame` zeros `gMain.state` so the rules menu’s state machine does not break map load.
- Upstream merge touchpoints: `overworld.c` (split), `oak_speech.c` / `main_menu.c` / `quickstart.c` (one-line callback each).
