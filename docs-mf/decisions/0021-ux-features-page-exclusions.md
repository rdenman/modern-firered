# 0021 — Features page: FR subset and deliberate ME omissions

- **Type:** ux
- **Status:** Accepted
- **Date:** 2026-09-12
- **Story:** S21
- **ME reference:** `MENUITEM_FEATURES_*` / `sText_*` in `src/tx_rac_menu.c`
- **Expansion config:** —

## Context

ME’s Features page includes several options that do not apply to stock Kanto FireRed, or that `PROJECT.md` defers. S21 must ship a trimmed page and record each omission so the gaps read as intentional, not unfinished.

## Decision

1. **Ship** (ME order among kept rows): `SHINY CHANCE` (1/8192…1/512), `SHINY COLORS` (Original/Modern), `ITEM DROP` (Off/On), then `NEXT`.
2. **Exclude from the menu** (fields remain in `ModernRules` for save parity / debug, default off / unused):
   - **`CLOCK TYPE`** — FireRed has no RTC; day/night is optional-later per `PROJECT.md`.
   - **`FRONTIER BANS`** — no Battle Frontier in FR (`PROJECT.md` out of scope).
   - **`EASIER FEEBAS`** — Hoenn Route 119 content; no Kanto equivalent.
   - **`WONDERTRADE` / `UNLIMITED WT`** — deferred (no FR Wonder Trade stations yet).
3. **`SHINY COLORS` labels** use Original/Modern (matching ME descriptions) rather than ME’s Off/On draw labels.
4. Features rows are always editable (no Custom gate); they are independent of the Gamemode preset.

## Alternatives considered

- Keep excluded rows greyed / no-op — rejected; implies systems we will not ship.
- Drop excluded fields from `ModernRules` — rejected; save layout is frozen post-S12; debug already exposes them.
- ME Features→Randomizer page order — rejected; STORIES.md Phase 3 order is Gamemode → Features → Nuzlocke → …

## Consequences

- S33–S34 wire shiny chance / item drops / shiny colors only.
- Debug Features page still lists Feebas / RTC / WT / Frontier for inspection; menu does not.
- Stub CONTINUE page remains after Features until S22–S25.
