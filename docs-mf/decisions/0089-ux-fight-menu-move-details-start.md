# 0089 — Fight-menu move details on Start, no L/R hint

- **Type:** ux
- **Status:** Accepted
- **Date:** 2026-10-17
- **Story:** —
- **ME reference:** —
- **Expansion config:** `B_MOVE_DESCRIPTION_BUTTON`, `B_SHOW_MOVE_DESCRIPTION_HINT` in `include/config/battle.h`

## Context
Expansion shows an L (or R) overlay on the fight menu telling the player they can open move details. That sprite sits in the same corner as the last-used-ball prompt (R to throw), so the two fight for space. The overlay also has no Start graphic. Action-menu Start still swaps HP numbers; that is unchanged.

## Decision
- Keep in-battle move details (`B_SHOW_MOVE_DESCRIPTION` TRUE).
- Bind them to Start (`B_MOVE_DESCRIPTION_BUTTON` = `START_BUTTON`).
- Never show the L/R hint overlay (`B_SHOW_MOVE_DESCRIPTION_HINT` FALSE). Players discover Start themselves.
- Mega / Z / Dynamax toggle on the fight menu moves to L (skipped when Options is L=A). Unused in this product while megas are off (ADR 0083).

## Alternatives considered
- Keep L for details and show the overlay from battle 1 — rejected; it covers the ball prompt.
- Keep L, hide overlay only — rejected; L is easy to miss next to R-throw-ball, and L=A disables it.
- Select for details — rejected; Select is in-battle move swap (ADR 0087).

## Consequences
Next `RHH/master` merge will conflict on `B_MOVE_DESCRIPTION_BUTTON` and the new hint define. Re-apply after taking theirs. Gimmick Start users from vanilla expansion need L instead if gimmicks return.
