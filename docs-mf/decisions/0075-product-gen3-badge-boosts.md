# 0075 — Gen 3 gym badge stat boosts

- **Type:** product
- **Status:** Accepted
- **Date:** 2026-10-11
- **Story:** —
- **ME reference:** —
- **Expansion config:** `B_BADGE_BOOST` in `include/config/battle.h` (`GEN_LATEST` → `GEN_3`)

## Context
Expansion’s default is Gen 4+: gym badges do not raise battle stats. FRLG applied a 1.1× boost to the player’s Pokémon for Attack (Boulder), Speed (Thunder), Defense (Soul), and Sp. Atk/Sp. Def (Volcano). A default Modern run therefore felt weaker immediately after Brock. S07 kept a `GEN_LATEST` battle baseline (ADR 0007) and said later undesirable Gen 9/4 behaviors get a targeted pin.

`B_FLAG_BADGE_BOOST_*` already map to the FRLG badge flags (`FLAG_BADGE01_GET` Attack, `FLAG_BADGE03_GET` Speed, `FLAG_BADGE05_GET` Defense, `FLAG_BADGE07_GET` Sp. Atk and Sp. Def). Boosts still skip link, frontier, secret-base, and opponent sides.

## Decision
Pin `B_BADGE_BOOST` to `GEN_3`. Always-on; no Options+ or rules toggle. Leave the `B_FLAG_BADGE_BOOST_*` mappings and `B_FRIENDSHIP_BOOST` alone.

## Alternatives considered
- Keep `GEN_LATEST` (no boosts) as a modern balance choice — rejected; default Kanto should match FRLG badge pacing.
- Runtime Options+ / rules toggle — rejected; not a player-facing rule, and the complaint is the default feel.
- `GEN_2` (1.125× plus type-matching gym boost) — rejected; the comment notes expansion does not implement the extra type boost, and the request is Gen 3.

## Consequences
Next `RHH/master` merge will conflict on this `#define`. Re-apply after taking theirs. Player damage and bulk after the matching badges are ~10% higher than Gen 4+; trainers are unchanged.
