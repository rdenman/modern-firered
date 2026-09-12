# 0005 — MF debug submenu via public DebugMenuOption

- **Type:** tech
- **Status:** Accepted
- **Date:** 2026-09-11
- **Story:** S05
- **ME reference:** —
- **Expansion config:** `DEBUG_OVERWORLD_MENU` / `DEBUG_BATTLE_MENU` (`DISABLED_ON_RELEASE`); `ENABLE_QUICKSTART` (`TRUE`, forced off on release)

## Context

S05 needs a Modern FireRed hang-point under expansion’s overworld debug menu so S17 can attach rules inspection without growing `src/debug.c`. Expansion keeps `struct DebugMenuOption` and submenu open/cancel helpers private to `debug.c`, so an `mf_` menu table cannot live in its own translation unit without a small public API.

## Decision

- Move `struct DebugMenuOption` into `include/debug.h` and export `DebugAction_OpenSubMenu` / `DebugAction_Cancel`.
- Own the Modern FireRed submenu in `src/mf_debug.c` / `include/mf_debug.h` (`gMfDebugMenuOptions`), with a stub **Rules inspector…** entry for S17.
- Touch upstream `src/debug.c` only to `#include "mf_debug.h"` and add one main-menu row: **Modern FireRed…** → `DebugAction_OpenSubMenu` + `gMfDebugMenuOptions`.
- Document Quickstart (**Select** on the FR title screen), overworld debug (**R**+**Start**), battle debug (**Select**), and the mGBA pre-Oak save-state workflow in `docs-mf/DEBUG.md`. Do not change the upstream `DISABLED_ON_RELEASE` / Quickstart defaults.

## Alternatives considered

- Inline the MF submenu table inside `src/debug.c` — rejected; S17 would keep expanding an upstream file and fight every RHH debug-menu merge.
- Duplicate `DebugMenuOption` layout in `mf_debug` without exporting it — rejected; fragile ABI drift.
- New start-menu entry instead of the overworld combo — rejected; `DEBUG_OVERWORLD_IN_MENU` is already `FALSE`, and the combo is the expansion default we confirmed for FR.

## Consequences

- After upstream rewrites `include/debug.h` or the main debug menu table, re-apply the public struct/API and the **Modern FireRed…** row (called out in `UPSTREAM.md`).
- Release builds still compile `mf_debug.o`, but the overworld menu and Quickstart are unreachable (`DISABLED_ON_RELEASE` / `QUICKSTART` false) — matching S17’s “absent from `make release`” expectation for the inspector UI.
- S17 should extend `gMfDebugMenuOptions` (and helpers in `mf_debug.c`) rather than adding more rows to `sDebugMenu_Actions_Main`.
