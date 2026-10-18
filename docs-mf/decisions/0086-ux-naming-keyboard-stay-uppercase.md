# 0086 — Naming keyboard stays uppercase unless the player swaps case

- **Type:** ux
- **Status:** Superseded by [0096](0096-ux-naming-keyboard-auto-lowercase.md)
- **Date:** 2026-10-17
- **Story:** —
- **ME reference:** —
- **Expansion config:** `AUTO_LOWERCASE_KEYBOARD` in `include/config/general.h` (`GEN_LATEST` → `GEN_3`)

## Context

Expansion’s Gen 6+ naming screen switches from uppercase to lowercase after the first letter. That fights FireRed’s all-caps names and forces an extra page swap whenever the player wants `BULBASAUR` instead of `Bulbasaur`. The page button and SELECT still exist for people who want mixed case.

## Decision

Pin `AUTO_LOWERCASE_KEYBOARD` to `GEN_3` so the keyboard stays on the uppercase page until the player explicitly swaps. Always-on; no Options+ or rules toggle.

## Alternatives considered

- Keep Gen 6+ auto-lowercase as a “modern” default — rejected; it is the opposite of the desired naming feel.
- Options+ toggle — rejected; this is a one-time keyboard preference, not a mid-run rule.

## Consequences

Superseded by [0096](0096-ux-naming-keyboard-auto-lowercase.md) once Title Case species names were treated as the naming default.

Next `RHH/master` merge will conflict on this `#define` if upstream keeps `GEN_LATEST`. Re-apply after taking theirs. `SwapKeyboardToLowerAfterFirstCapitalLetter` stays in `naming_screen.c` but returns immediately when the pin is below `GEN_6`.
