# 0099 — Fossils stay vanilla under item randomizer

- **Type:** product
- **Status:** Accepted
- **Date:** 2026-10-17
- **Story:** —
- **ME reference:** —
- **Expansion config:** `I_KEY_FOSSILS` (`GEN_LATEST` — fossils are Items-pocket, not Keys)

## Context

ADR 0063 pinned key-item pocket and `importance != 0`. Expansion’s `I_KEY_FOSSILS >= GEN_4` puts Old Amber / Helix / Dome in the regular Items pocket with price and no importance, so the Pewter museum grant remapped. Those fossils are unique revive items for Aerodactyl / Omanyte / Kabuto.

## Decision

1. **All fossil item IDs** (`ITEM_HELIX_FOSSIL`–`ITEM_FOSSILIZED_DINO`) are progression-protected: identity source, never a random dest.
2. **Do not flip `I_KEY_FOSSILS`.** They can stay in the Items pocket; only the randomizer pin changes.

## Alternatives considered

- **Set `I_KEY_FOSSILS` to Gen 3** — rejected for this bug; that would move them back to Keys globally and is a separate UX call.
- **Pin only Old Amber** — rejected; Mt. Moon Helix/Dome have the same problem.

## Consequences

- Museum Old Amber and Mt. Moon fossils stay themselves. They no longer appear as chest dests.
- Pool membership for those IDs is now via the protected filter, not the old Helix–Claw allowlist.
