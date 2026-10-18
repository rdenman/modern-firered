# 0095 — Item randomizer shuffles the Gen 3 pool

- **Type:** product
- **Status:** Superseded by [0098](0098-product-item-random-per-chest.md)
- **Date:** 2026-10-17
- **Story:** —
- **ME reference:** ME hashes each source into the item pool (collisions allowed) — we diverge
- **Expansion config:** `MF_RANDOMIZER`; runtime `randomItems`

## Context

ADR 0063 hashed each source item independently into the Gen 3 pool, so every Potion became the same dest but two sources could share a dest and some pool items never appeared. Players wanted a permutation of that list, without later-gen junk (Life Orb, megas, Heal Ball, …) showing up as grants.

## Decision

1. **Shuffle.** `MfItemMapEx` Fisher–Yates shuffles the ADR 0083 candidate list with `MfRandom_Modulo(seed, MF_RANDOM_CAT_ITEMS, i, 0, i + 1)` and maps `pool[i] → shuffled[i]`. Same seed is stable; cache rebuilds when the seed changes.
2. **Pins and marts unchanged.** Key items, HMs, `ITEM_POKE_BALL`, and poké mart stock stay identity (ADR 0063 / 0088).
3. **No garbage dests.** Shuffle membership is the existing pool filter. A source that is not in the pool and not pinned lands on a pool dest rather than passing through (so a later-gen gift cannot grant Life Orb under ITEMS).
4. **Fixed points allowed.** A shuffle may leave Potion as Potion.

## Alternatives considered

- **Keep independent hashing** — rejected; duplicates and missing dests were the complaint.
- **Derangement (no item stays itself)** — rejected; extra code for little gain.
- **Shuffle `ITEMS_COUNT`** — rejected; that is how Life Orb and megas leak in.

## Consequences

- Same seed: each pool item appears as a field dest exactly once (among pool sources). Existing seeds change (ADR 0016 mapping break).
- Chaos still does not touch items.
