# 0088 — Mart stock stays vanilla under item randomizer

- **Type:** product
- **Status:** Accepted
- **Date:** 2026-10-17
- **Story:** —
- **ME reference:** ME remaps poké marts with `IsRandomItemsActivated()` — we diverge
- **Expansion config:** `MF_RANDOMIZER`; runtime `randomItems`

## Context

S57 / ADR 0063 remapped poké mart lists through `MfMaybeRandomizeShopStock` so shop Potion matched field Potion. That made staples (Potions, status heals, Repels) unreliable to buy, which is too punishing next to randomized wilds and trainers.

## Decision

1. **Poké marts never remap.** `MfMaybeRandomizeShopStock` is identity. `CreatePokemartMenu` still calls it so the seam stays in one place.
2. **ITEMS still remaps** field balls, hidden items, and NPC `giveitem` grants (key items / HMs / Poké Balls stay pinned per ADR 0063).
3. **No extra rules-menu toggle.** Vanilla marts are always on when ITEMS is on.

## Alternatives considered

- **Keep ME shop remap** — rejected; buying the right healing items is part of a playable Kanto run.
- **Protect only Potions / status heals** — rejected; the rest of the vanilla list (balls stay pinned anyway, Escape Rope, evo items, mints via criteria) is the stock players expect.
- **Separate RANDOM SHOPS flag** — rejected; default play must keep marts static; a hidden extra axis is not needed.

## Consequences

- Same seed: a Route 1 Potion ball may not be Potion; Viridian still sells Potion.
- Celadon 4F Linking Cord / held evo items and mint-gated 2F stock stay obtainable (ADR 0035 / 0031).
- Chaos salt still does not touch items (ADR 0063).
