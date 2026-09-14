# 0026 — SAVE confirm + start-menu rules viewer

- **Type:** ux
- **Status:** Accepted
- **Date:** 2026-09-13
- **Story:** S26
- **ME reference:** `SAVE` / permanence notes in `src/tx_rac_menu.c`; `src/tx_rac_viewer.c` (`CB2_InitChallengeViewer`, `Special_OpenChallengeViewer`)
- **Expansion config:** —

## Context

Phase 3 ends with committing the new-game rule set and giving players a mid-run way to re-read it. ADR 0015 already reserved `MfRules_CommitAndLock()` for S26 SAVE. ME warns “All selections are permanent” on Gamemode descriptions and writes on SAVE without a Yes/No dialog; our story asks for an explicit confirmation. ME’s viewer is a separate ~1.4k-line CB2 opened from a special/script — we need an FR menu surface with no new maps/NPCs.

## Decision

1. **SAVE row** replaces EXIT on the Randomizer page. A opens a confirm step: description shows “All selections are permanent” with **A: Save / B: Cancel** (no Yes/No overlay — that popup fights this screen’s WIN0 darken blend). A calls `MfRules_CommitAndLock()` then fades to `savedCallback`; B returns to the menu.
2. **Read-only viewer** reuses the rules-menu shell (`sReadOnlyViewer`): same pages/labels/values, no writes, full-color draw, L/R (and D-pad L/R) page wrap, A/B exit. Entry: `CB2_InitMfRulesViewer`.
3. **Start menu RULES** (between SAVE and OPTION) opens the viewer. Bump `sCurrentStartMenuActions` to 10 so DexNav + RULES still fit. Debug also gets **Rules viewer…** (editable **Rules menu…** stays for S15 unlock testing).

## Alternatives considered

- ME-style SAVE with no confirm step — rejected; story requires confirmation reflecting permanence.
- `CreateYesNoMenu` overlay — rejected; WIN0 highlight darken + std-frame tiles produced an unreadable dark box on this shell.
- Separate `mf_rules_viewer.c` duplicating ME’s per-page draw tables — rejected; label/value drift vs the live menu.
- Trainer card / Pokédex hook — rejected; start menu is the least invasive FR surface and matches “existing menu.”

## Consequences

- New games leave the overworld with `rulesLocked == TRUE` after SAVE Yes.
- Upstream touch: `src/start_menu.c` only (action enum, 10-slot list, RULES callback).
- Mid-run Difficulty edits (when `lockDifficulty` is off) remain a future surface; the viewer stays read-only.
