# 0087 — In-battle Select move rearrangement

- **Type:** product
- **Status:** Accepted
- **Date:** 2026-10-17
- **Story:** —
- **ME reference:** —
- **Expansion config:** `B_MOVE_REARRANGEMENT_IN_BATTLE` in `include/config/battle.h` (`GEN_LATEST` → `GEN_3`)

## Context
FRLG lets the player press Select on the fight menu to swap two move slots (and their PP) mid-battle. Expansion 1.11 disabled that for Gen 4+ (`B_MOVE_REARRANGEMENT_IN_BATTLE` default `GEN_LATEST`). The fight-menu handler in `src/battle_controller_player.c` still implements the swap; the config gate was the only thing blocking it. Move descriptions are on Start (ADR 0089), so Select is free.

S07 kept a `GEN_LATEST` battle baseline (ADR 0007) and said later undesirable Gen 9/4 behaviors get a targeted pin.

## Decision
Pin `B_MOVE_REARRANGEMENT_IN_BATTLE` to `GEN_3`. Always-on, matching vanilla FRLG: Select on a move, highlight another, Select or A to swap. Link battles still refuse the swap (existing expansion check). No Options+ or rules toggle.

## Alternatives considered
- Keep `GEN_LATEST` (no in-battle swaps) as modern QoL — rejected; players expect the FRLG Select behavior.
- Runtime Options+ / rules toggle — rejected; this is vanilla UI, not a challenge rule.

## Consequences
Next `RHH/master` merge will conflict on this `#define`. Re-apply after taking theirs. Summary-screen / party move order is unchanged.
