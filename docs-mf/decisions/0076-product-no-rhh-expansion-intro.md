# 0076 — Skip the RHH expansion intro

- **Type:** product
- **Status:** Accepted
- **Date:** 2026-10-11
- **Story:** —
- **ME reference:** —
- **Expansion config:** `EXPANSION_INTRO` in `include/config/general.h` (`TRUE` → `FALSE`)

## Context
Expansion plays a custom pokeemerald-expansion splash after the vanilla copyright screen. That is not FireRed. The flag is compile-time only; there is no rules or Options+ path for it.

## Decision
Pin `EXPANSION_INTRO` to `FALSE`. The vanilla FRLG copyright screen goes straight into the Game Freak / title sequence. Always-on; no toggle.

## Alternatives considered
- Keep the RHH splash as a credit — rejected; it is Emerald-expansion branding on a FireRed ROM.
- Runtime Options+ skip — rejected; not a player-facing rule.

## Consequences
Next `RHH/master` merge will conflict on this `#define` if upstream keeps `TRUE`. Re-apply after taking theirs. `src/expansion_intro.c` stays in the tree but is compiled out.
