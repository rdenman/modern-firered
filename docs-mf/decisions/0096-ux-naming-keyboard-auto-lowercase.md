# 0096 — Naming keyboard auto-lowercases after the first capital

- **Type:** ux
- **Status:** Accepted
- **Date:** 2026-10-17
- **Story:** —
- **ME reference:** —
- **Expansion config:** `AUTO_LOWERCASE_KEYBOARD` in `include/config/general.h` (`GEN_LATEST`)

## Context

Species names in expansion are Title Case (`Bulbasaur`), not FireRed all-caps (`BULBASAUR`). ADR 0086 pinned the naming keyboard to stay uppercase so players could type all-caps nicknames without swapping pages. That now works against the strings the rest of the UI shows.

## Decision

Leave `AUTO_LOWERCASE_KEYBOARD` at `GEN_LATEST` so after the first uppercase letter the keyboard swaps to lowercase (Gen 6+). Always-on; no Options+ or rules toggle. Page and SELECT still swap case when the player wants all caps.

## Alternatives considered

- Keep ADR 0086’s Gen 3 pin — rejected; default nicknames and species names are Title Case.
- Options+ toggle — rejected; this is a one-time keyboard preference, not a mid-run rule.

## Consequences

Supersedes [0086](0086-ux-naming-keyboard-stay-uppercase.md). `SwapKeyboardToLowerAfterFirstCapitalLetter` in `naming_screen.c` actually runs. Upstream default is already `GEN_LATEST`, so this pin should not fight `RHH/master` merges.
