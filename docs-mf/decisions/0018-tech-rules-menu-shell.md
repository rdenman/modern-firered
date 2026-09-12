# 0018 — Data-driven FR rules menu shell

- **Type:** tech
- **Status:** Accepted
- **Date:** 2026-09-12
- **Story:** S18
- **ME reference:** `src/tx_rac_menu.c` (layout, NEXT paging, description pane); `TX_MENU_ITEMS_PER_PAGE` in `tx_randomizer_and_challenges.h`
- **Expansion config:** —

## Context

Phase 3 needs a reusable options screen before six real pages (S20–S25) and the new-game hook (S19). ME’s rac menu is Emerald-styled and code-heavy (per-item draw/input function tables). FireRed’s `src/option_menu.c` is the house style for fonts, palettes, and window frames. Story S18 also asks that a page be a **data table**, not new C per option.

## Decision

- Own the shell in `src/mf_rules_menu.c` / `include/mf_rules_menu.h` (`CB2_InitMfRulesMenu`).
- Layout: ME-like three windows (title / scrolling options / description), **5 rows visible** (`OPTIONS_ON_SCREEN`), FR `option_menu_text.pal` + player window-frame tiles.
- Page model: `MfRulesMenuPage` → items with kind (`BOOL` / `VALUE` / `NEXT` / `EXIT`), label, choice labels+descriptions, and a rules accessor id. Adding a page is mostly table data.
- Choice chrome: 2-value rows use FR-style left/right OFF/ON; 3+ values show the **active** label right-aligned (overflow-safe; ME’s multi-slot sliding chrome deferred).
- Demo entry: overworld debug → **Modern FireRed…** → **Rules menu demo…**. Export `Debug_CloseMenuFull` so `mf_debug` can tear down and launch a CB2 without growing `debug.c` further. New-game insertion stays S19.

Note: ME’s `TX_MENU_ITEMS_PER_PAGE 6` is unused; the live rac menu draws **5** rows (`OPTIONS_ON_SCREEN`). We match the live layout.

## Alternatives considered

- Port ME’s per-item function tables verbatim — rejected; fights the data-table requirement and Emerald APIs.
- Extend FR `option_menu.c` in place — rejected; high upstream merge cost; Options+ (S58) may share the shell later via `mf_` reuse.
- Six empty real pages instead of a throwaway demo — deferred; S20–S25 own real labels/descriptions.

## Consequences

- S20–S25 plug page tables into this shell; S19 calls `CB2_InitMfRulesMenu` from the new-game path.
- Mid-run demo auto-enables the S15 unlock override when rules are locked (same spirit as S17).
- After upstream rewrites `debug.h` destroy helpers, re-export `Debug_CloseMenuFull` (called out in `UPSTREAM.md`).
