# 0101 — Item randomizer keeps TMs as TMs

- **Type:** product
- **Status:** Accepted
- **Date:** 2026-10-21
- **Story:** — (S57 follow-up)
- **ME reference:** — (ME does not split the item pool this way)
- **Expansion config:** `MF_RANDOMIZER`; runtime `randomItems`

## Context

ADR 0098 hashes each chest into the full ADR 0083 pool, so a Potion ball can become TM29 and a TM ball can become a Nugget. That makes TM density on the map seed-dependent and turns early medicine pickups into late-game moves.

## Decision

1. **Two dest pools** from the same ADR 0083 allowlist: TM01–50, and everything else.
2. **TM source** (`MfIsTmItem`, TM01–TM100) hashes into the TM pool. Vanilla Kanto only grants TM01–50; later TMs are not dests (ADR 0083) but would still land on TM01–50 if remapped.
3. **Non-TM source** hashes into the non-TM pool. Never a TM dest.
4. **Pins unchanged.** Keys, HMs, Poké Balls, fossils, and marts stay identity.

## Alternatives considered

- **Keep one mixed pool** — rejected; TM count and early-route medicine swing too hard.
- **Full TM/HM pocket, including HMs** — rejected; HMs stay progression-protected (ADR 0063).
- **Include TM51–100 as dests** — rejected; still out of the Gen 3 pool (ADR 0083).

## Consequences

- Same seed + ITEMS mappings change (smaller modulus per class; ADR 0016).
- Kanto still has exactly as many TM pickups as vanilla; they just teach different TM01–50 moves.
- A Potion chest can still become a Rare Candy or Electirizer; it cannot become a TM.
