# 0105 — Only Oak’s Poké Ball grants stay vanilla

- **Type:** product
- **Status:** Accepted
- **Date:** 2026-10-21
- **Story:** — (S57 follow-up)
- **ME reference:** ME 3.5 keeps starting Poké Balls vanilla
- **Expansion config:** `MF_RANDOMIZER`; runtime `randomItems`

## Context

ADR 0063 pinned every `ITEM_POKE_BALL` (identity and never a dest). That also froze Pewter and other field Poké Balls. The wanted behavior is catching balls from Oak, then letting overworld Poké Ball pickups shuffle like other non-TMs.

## Decision

1. **`ITEM_POKE_BALL` is a pool candidate.** Field `finditem` / hidden tiles remap it. Other non-TM chests may roll a Poké Ball.
2. **Oak’s lab grants stay identity.** `MfScriptRemapGrantItem` skips remap when the source is `ITEM_POKE_BALL` and the current map is `MAP_PALLET_TOWN_PROFESSOR_OAKS_LAB` (dex-scene five and the Route 22 refill).
3. **Marts unchanged** (ADR 0088). Shop Poké Balls stay buyable.

## Alternatives considered

- **Keep the global pin** — rejected; field Poké Balls never shuffled.
- **Pin by quantity 5 only** — rejected; map is the real Oak scene, and a 5-stack elsewhere should still shuffle.

## Consequences

- Same seed + ITEMS mappings change (Poké Ball enters the non-TM pool; ADR 0016).
- Oak’s five (and refill) are still Poké Balls even if Items is on.
