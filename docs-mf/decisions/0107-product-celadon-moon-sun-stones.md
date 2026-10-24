# 0107 — Celadon stocks Moon and Sun Stones

- **Type:** product
- **Status:** Accepted
- **Date:** 2026-10-24
- **Story:** —
- **ME reference:** —
- **Expansion config:** `I_PRICE` (ADR 0073)

## Context

ADR 0103 left Moon / Sun Stones as field/NPC-only (vanilla FRLG). Sun Stone is a single Six Island chest; Moon Stone is several Kanto copies plus a Two Island gift, but its Gen 3 buy price is ¥0 so it cannot be sold. Celadon 4F already sells the other Kanto stones, so Bellossom / Clefable / etc. were the odd ones out for a mid-game buy.

## Decision

1. **Celadon 4F** always lists Moon Stone and Sun Stone with Fire / Thunder / Water / Leaf.
2. **Moon Stone** uses the same `.price` expression as Sun Stone (`I_PRICE >= GEN_7` → ¥3000, else ¥2100). With ADR 0073 (`I_PRICE` / sell fraction Gen 3) that is ¥2100 buy and ¥1050 sell.
3. Field copies and the Two Island gift stay.

## Alternatives considered

- Keep Moon Stone unsellable (price 0) and only add Sun Stone to the mart — rejected; the ask was a buyable Moon Stone with matching sell value.
- Runtime shop toggle — rejected; other Celadon evo stock is always-on.

## Consequences

- Revises ADR 0103’s “Moon / Sun stay field/NPC” line.
- `MfEvoItemIsKantoObtainable` already included both; no sanitize/mapping break.
- Upstream `src/data/items.h` Moon Stone price is an MF pin; re-apply on `RHH/master` merge.
