# 0098 — Item randomizer rolls per chest, collisions allowed

- **Type:** product
- **Status:** Accepted
- **Date:** 2026-10-17
- **Story:** —
- **ME reference:** ME maps by source item id — we diverge
- **Expansion config:** `MF_RANDOMIZER`; runtime `randomItems`

## Context

ADR 0095 shuffled the Gen 3 item *list*, so every Potion grant still became the same dest. The wanted behavior is each pickup (item ball, hidden tile, NPC `giveitem`, bedroom PC) rolling independently into that pool. Two chests may share a dest.

## Decision

1. **Site key.** `MfItemMapExAt(item, seed, siteId)` hashes `(seed, MF_RANDOM_CAT_ITEMS, siteId, itemId)` into the ADR 0083 pool.
2. **Object grants** (`giveitem` / `finditem`): `siteId = (mapGroup << 24) | (mapNum << 16) | lastTalked`.
3. **Hidden items:** `siteId = 0x80000000 | hiddenFlag` (`VAR_0x8004` as set before `EventScript_HiddenItemScript`).
4. **Bedroom PC Potion:** dedicated `0x7F000001` so it does not collide with a map object.
5. **Pins / marts / garbage dests unchanged.** Key items, HMs, Poké Balls, and mart stock stay identity. Destinations are still pool candidates only (later-gen junk cannot appear).

## Alternatives considered

- **Keep type-wide shuffle (ADR 0095)** — rejected; every Potion ball stayed identical.
- **Unique dest per chest (global permutation of pickups)** — rejected; needs a full grant census and forbids duplicate dests.

## Consequences

- Same seed + same chest is stable. Same vanilla item on two maps can differ.
- ADR 0095’s bijection-over-item-ids no longer applies.
- Changing how `siteId` is packed is a mapping break (ADR 0016).
