# 0017 — Debug rules inspector via paged mf_debug menus

- **Type:** tech
- **Status:** Accepted
- **Date:** 2026-09-12
- **Story:** S17
- **ME reference:** `PrintTXSaveData()` in `src/tx_randomizer_and_challenges.c`
- **Expansion config:** `DEBUG_OVERWORLD_MENU` (`DISABLED_ON_RELEASE`); stripped further under `NDEBUG`

## Context

S17 needs every `ModernRules` field visible and editable without replaying the Phase 3 menu, while honoring S15’s mid-run lock (debug override only) and keeping upstream `src/debug.c` thin. Expansion’s debug list is capped at **20 items** with **26-character** labels, so a flat dump of ~70 fields does not fit one submenu.

## Decision

1. Own the inspector entirely in `src/mf_debug.c`, opened from **Modern FireRed… → Rules inspector…**.
2. Page by ME category (Meta / Gamemode / Features / Randomizer / Nuzlocke / Difficulty / Challenges). Rebuild live `Label:value` strings into EWRAM before open and after each edit.
3. Export `Debug_RefreshCurrentMenu` from `include/debug.h` so toggles can regenerate list labels without growing more private debug helpers.
4. First failed `TrySet*` auto-enables `MfRules_DebugSetUnlockOverride` so locked runs are editable in one press.
5. **Dump (mGBA)** calls `MfRules_DebugDump()` (`DebugPrintfLevel`), matching ME’s console dump spirit without a separate viewer UI.
6. Under `NDEBUG` or `!MF_RULES_ENGINE`, `gMfDebugMenuOptions` is Cancel-only — the inspector UI is absent from `make release` even if the object file still links.

## Alternatives considered

- Flat single list of all rules — rejected; exceeds `DEBUG_MAX_MENU_ITEMS` (20).
- Custom full-screen UI outside the debug menu — rejected; more code and diverges from S05’s hang-point.
- Require a separate **Unlock rules** step before every edit — rejected; Meta still exposes Unlock, but auto-override on edit matches “debug builds bypass S15”.

## Consequences

- Upstream merge: keep `Debug_RefreshCurrentMenu` next to the other public debug helpers if `debug.h` / `debug.c` are rewritten. It must pass `Debug_GetCurrentCallbackMenu()` into the generator — `NULL` is safe for FLAGS/TRAINERS generators but **not** for `GenerateListBasicMenu` (S17 A-press crash).
- Label abbreviations are debug-only; Phase 3 player-facing strings stay ME-accurate.
- Follow-up stories (S18+) can use Dump / inspector to verify menu writes without new tooling.
