# 0126 — Residual KO of the last foe still gives EXP

- **Type:** tech
- **Status:** Accepted
- **Date:** 2026-11-22
- **Story:** —
- **ME reference:** —
- **Expansion config:** —

## Context
Players sometimes won a battle and got no EXP at all: no "gained Exp. Points" line and no bar. It happened in wild and trainer battles, Rival 1 in Oak's Lab included. Randomized moves and abilities make it more common because they spread poison and burn around.

Cause (upstream expansion, still in `RHH/master` on 2026-11-22): when end-of-turn damage KOs the last foe (poison, toxic, burn, Leech Seed, sandstorm and the like), `BattleScript_DoTurnDmg` runs `checkteamslost`, which sets `gBattleOutcome = B_OUTCOME_WON`. When `DoEndTurnEffects` runs again, it sees an outcome and jumps to `ENDTURN_TRAINER_A_SLIDES`. That jump goes past `ENDTURN_FAINTED_MON_ACTIONS`, the only place that runs `BattleScript_GiveExp` for an end-of-turn faint. A direct KO isn't affected because it gives EXP through `HandleAction_TryFinish`.

## Decision
When an outcome is set, skip to `ENDTURN_FAINTED_MON_ACTIONS` instead of `ENDTURN_TRAINER_A_SLIDES`. The fainted-mon handler then gives EXP for the residual KO the same way a mid-turn KO does. The Dynamax step that comes next already handles the battle being won.

## Alternatives considered
- **Give EXP from `HandleEndTurn_BattleWon`:** rejected. That would be a second EXP path apart from the fainted-mon state machine.
- **Drop `checkteamslost` from the residual script:** rejected. It changes when the battle ends for every residual effect, and that is riskier.

## Consequences
- Upstream touch: one condition in `src/battle_end_turn.c` `DoEndTurnEffects`. Good candidate to send upstream.
- Regression tests: `test/modern_firered/mf_exp_residual.c` (wild and trainer; poison, toxic, burn, Leech Seed, sandstorm; non-final foe paid once).
