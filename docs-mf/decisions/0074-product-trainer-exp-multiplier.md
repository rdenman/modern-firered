# 0074 — Gen 3 trainer EXP multiplier

- **Type:** product
- **Status:** Accepted
- **Date:** 2026-10-11
- **Story:** —
- **ME reference:** —
- **Expansion config:** `B_TRAINER_EXP_MULTIPLIER` in `include/config/battle.h` (`GEN_LATEST` → `GEN_3`)

## Context
Expansion Gen 7+ dropped the 1.5× EXP bonus for trainer battles. Kanto’s gym/route pace was built with that bonus; without it, trainers pay the same as wilds. S07 kept a `GEN_LATEST` battle baseline (ADR 0007) and said later undesirable Gen 9/7 behaviors get a targeted pin. The Difficulty `EXP. MULTIPLIER` (S42 / ADR 0044) is a separate player rule on the already-calculated pool.

## Decision
Pin `B_TRAINER_EXP_MULTIPLIER` to `GEN_3`. Trainer battles apply ×1.5 (`* 150 / 100`) after the species yield divisor and before S42’s multiplier. Always-on; no Options+ or rules toggle. Leave `B_SPLIT_EXP` on Gen 9. Leave `B_SCALED_EXP` as the S70 runtime toggle (ADR 0045).

## Alternatives considered
- Keep Gen 7+ (no trainer bonus) as modern QoL — rejected; default Kanto pace assumes the bonus.
- Fold 1.5× into Difficulty `EXP. MULTIPLIER` — rejected; that row is an extra scale the player chooses, not the vanilla trainer formula.
- Runtime toggle — rejected; not a player-facing rule.

## Consequences
Default ×1 Difficulty runs still get 1.5× from trainers and 1× from wilds. Choosing ×1.5 / ×2 / ×0 stacks on top of that pool. Next `RHH/master` merge will conflict on this `#define`; re-apply after taking theirs.
