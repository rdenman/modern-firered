# 0020 — Gamemode page: ME order, no EXTRA LEGEND., Custom-gated edits

- **Type:** ux
- **Status:** Accepted
- **Date:** 2026-09-12
- **Story:** S20
- **ME reference:** `MENUITEM_MODE_*` / `sText_*` / `CheckConditions` in `src/tx_rac_menu.c`
- **Expansion config:** —

## Context

S20 replaces the S18 throwaway demo with the real Gamemode page. ME’s live item order differs from the story’s shorthand list, and `EXTRA LEGEND.` requires new map content forbidden by `PROJECT.md` (already forced off in ADR 0014). Sub-options must only be editable under Custom, matching ME’s `CheckConditions`.

## Decision

1. **Page order** follows ME’s `MENUITEM_MODE_*` enum (Gamemode → Encounters → Type Chart → Stats → Fairy → Types → Movepool → Synchronize → Sturdy → Sitrus → Legen. Abilities → Mints → Reusable TMs → Survive Poison → NEXT), not the story bullet’s abbreviated order.
2. **Drop `EXTRA LEGEND.`** from the menu. The `newLegendaries` field remains in `ModernRules` for save parity but stays off in Classic/Modern and is unreachable from this UI.
3. **`MF_RULES_MENU_FLAG_REQUIRES_CUSTOM`** greys dependent rows and blocks L/R/A cycling unless `gamemodePreset == Custom`. Descriptions still show the current value (ME shows value text when disabled mode descs are empty).
4. **`MfRules_TrySetValue(MF_RULE_VAL_GAMEMODE_PRESET)`** calls `MfRules_ApplyGamemodePreset` so Classic/Modern bulk-set from the menu and the S17 debug Preset control (cycle count raised to 3 for Custom).
5. Stub **CONTINUE / EXIT** page remains until S21–S25; EXIT still leaves the new-game flow (SAVE/lock is S26).

## Alternatives considered

- Story-bullet option order — rejected; ME live enum is the UX spec.
- Keep EXTRA LEGEND. as a no-op toggle — rejected; implies content we will not ship.
- Auto-switch to Custom when editing a greyed row — rejected; ME requires explicit Custom first.

## Consequences

- S21+ pages plug into the same `flags` / page-table model.
- Players must pick Custom before tweaking individual Gamemode switches.
- Debug **Rules menu…** (renamed from demo) opens the real Gamemode page mid-run.
