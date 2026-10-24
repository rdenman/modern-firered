# 0106 — Forget HMs like any other move

- **Type:** product
- **Status:** Accepted
- **Date:** 2026-10-24
- **Story:** —
- **ME reference:** —
- **Expansion config:** `P_CAN_FORGET_HIDDEN_MOVE` default `FALSE` → `TRUE`

## Context
Vanilla and this ROM previously locked HM moves on the summary, level-up, TM, and evolution forget screens. The only way to drop them was Fuchsia’s Move Deleter, plus a last-Surf guard even there. That friction is the same in Classic, Modern, Nuzlocke, and randomizer. Expansion already implements free forget behind `P_CAN_FORGET_HIDDEN_MOVE`.

## Decision
Flip `P_CAN_FORGET_HIDDEN_MOVE` to `TRUE`. Always-on in every game mode; no rules or Options+ toggle. Do not reimplement forget checks. HMs stay in the bag and can be retaught. Field use still requires a party member that currently knows the move.

## Alternatives considered
- Runtime Gamemode / Options+ toggle — rejected; the request is all modes, always.
- Reimplement in `mf_*` — rejected; `CannotForgetMove` and the last-Surf Deleter special already honor the config.

## Consequences
Next `RHH/master` merge will conflict on this `#define` if upstream keeps `FALSE`. Re-apply after taking theirs. Players can softlock field progression by forgetting Cut/Surf/Strength with no other learner; that is accepted. The Move Deleter still works for non-HM moves.
