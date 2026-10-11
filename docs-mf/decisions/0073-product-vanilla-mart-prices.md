# 0073 — Vanilla FRLG mart prices and Escape Rope

- **Type:** product
- **Status:** Accepted
- **Date:** 2026-10-11
- **Story:** —
- **ME reference:** —
- **Expansion config:** `I_PRICE`, `I_KEY_ESCAPE_ROPE`, `I_SELL_VALUE_FRACTION` in `include/config/item.h` (`GEN_LATEST` → `GEN_3`)

## Context
Expansion defaults item buy prices to Gen 9 and treats Escape Rope as a Gen 8+ Key Item (`price` 0, `importance` 1). Pewter (and every other mart) then shows flattened Gen 7/8 costs (Potion ¥200, Repel ¥400) and a free Escape Rope that sells out after one copy. S07 kept a `GEN_LATEST` battle baseline (ADR 0007) and said later undesirable Gen 9/7/8 behaviors get a targeted pin. Default rules already leave shop multipliers at 1× (S46); this was compile-time item data, not a rules-menu setting.

## Decision
Pin `I_PRICE`, `I_KEY_ESCAPE_ROPE`, and `I_SELL_VALUE_FRACTION` to `GEN_3`. Mart buy prices match FRLG/RSE tables (Potion ¥300, Antidote ¥100, Repel ¥350, Escape Rope ¥550). Escape Rope stays a consumable Items-pocket good, not a Key Item. Sell value stays 1/2 of buy price. Always-on; no Options+ or rules toggle. Leave `I_PREMIER_BALL_BONUS`, heal amounts, and other item-gen knobs alone.

## Alternatives considered
- Keep `GEN_LATEST` prices as a modern QoL — rejected; default run should feel like FireRed marts.
- Runtime Options+ toggle — rejected; not a player-facing rule, and S46 already scales shops when Challenges ask for expensive marts.
- Also pin `I_HEALTH_RECOVERY` / Premier Ball bonus — rejected; those are not mart stock/cost.

## Consequences
Next `RHH/master` merge will conflict on these `#define`s. Re-apply after taking theirs. Awakening is ¥250 in expansion’s Gen 3 table (RSE); FRLG listed ¥200 — accepted as the expansion-native Gen 3 value rather than a one-off override. S46 multipliers still apply on top of these base prices.
