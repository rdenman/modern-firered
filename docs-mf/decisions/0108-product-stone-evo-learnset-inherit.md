# 0108 — Stone evolutions keep learning pre-evo moves

- **Type:** product
- **Status:** Accepted
- **Date:** 2026-10-24
- **Story:** —
- **ME reference:** —
- **Expansion config:** —

## Context

Gen 9 (and FRLG) stone evolutions usually dump moves at level 0/1 and stop learning as they level. Players still want to stone-evolve without locking the pre-evo’s later STAB. A few lines (Eevee, Sunflora) have a real post-evo learnset that should win.

## Decision

Always-on, no rules toggle. `MfGetSpeciesLevelUpLearnset` merges for a species that:

1. Evolves from something via an `ITEM_TYPE_EVOLUTION_STONE` (`EVO_ITEM`), using the vanilla evo graph.
2. Has fewer than 4 later-level (`level > 1`) entries in its own table.

The merged table is the pre-evo level-up list plus extras from the evolved form: evolution moves (level 0), token later-level moves (Raichu Thunderbolt at 5, classic Ninetales Fire Spin at 45), and unique level-1 moves the pre-evo never learns. Level-1 copies of the pre-evo movepool are skipped.

If the evolved form already has a real later-level line (≥ 4 entries above level 1), keep that table unchanged.

## Alternatives considered

- **Relearner-only (`P_PRE_EVO_MOVES`)** — does not teach on level-up; rejected.
- **Always inherit, even Eevee/Sunflora** — would replace those unique lines with Eevee/Sunkern; rejected.
- **Player toggle** — extra rules UI for a QoL default everyone wanted; rejected.

## Consequences

- Dex, summary, level-up, and the move relearner all see the merged table (same accessor).
- ROM unchanged aside from the merge code; merge happens in EWRAM like move-randomizer learnsets.
- Linking Cord / held-item bag evos are not stones and are not merged.
- Evolution randomizer does not change who “is” a stone species for learnsets (vanilla graph).
