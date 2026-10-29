# 0113 — Bag pocket capacities (Items 72 / Balls 28 / Berries 68)

- **Type:** product
- **Status:** Accepted
- **Date:** 2026-10-28
- **Story:** —
- **ME reference:** —
- **Expansion config:** `BAG_*_COUNT` in `include/constants/global.h`; `FREE_EXTRA_SEEN_FLAGS_SAVEBLOCK1`, `FREE_ENIGMA_BERRY`, `FREE_LINK_BATTLE_RECORDS` in `include/config/save.h`

## Context

Unique item *types* per pocket are limited by SaveBlock1. Stack size is already `MAX_BAG_ITEM_CAPACITY` (999). Players were hitting the Items pocket (30 slots — smaller than vanilla FRLG’s 42). Doubling every pocket does not fit.

FireRed SaveBlock1 had only **120 bytes** free (~30 slots) because of FRLG-only fields (rival name, Route 5 Day Care, Trainer Tower). Emerald’s 304-byte gap does not apply to the product ROM.

## Decision

Pocket sizes:

| Pocket | Was | Now |
|---|---|---|
| Items | 30 | 72 |
| Key Items | 30 | 30 |
| Poké Balls | 16 | 28 (all `POKEBALL_COUNT` types) |
| TM/HM | 64 | 64 (this hack’s list is 50 TMs + 8 HMs) |
| Berries | 46 | 68 (all berry indices) |

That is +76 slots (304 bytes). To make it fit on FireRed, reclaim unused SaveBlock1 systems (192 bytes):

- `FREE_EXTRA_SEEN_FLAGS_SAVEBLOCK1` — leftover dex-flag filler (52)
- `FREE_ENIGMA_BERRY` — e-Reader Enigma Berry (52)
- `FREE_LINK_BATTLE_RECORDS` — wireless link-battle record table (88)

Leave Mystery Gift, ramScript, Match Call / VS Seeker, Union Room, and Trainer Tower allocated.

Existing saves are not compatible.

## Alternatives considered

- Double every pocket — exceeds SaveBlock1 even after reclaim.
- Items-only 30 → 60 using the native 30-slot gap — leaves Balls/Berries unable to hold every type.
- Reclaim Mystery Gift / ramScript — still rejected (ADR 0012); those systems are larger than needed and more entangled.

## Consequences

- Upstream merge conflict on bag counts and three `FREE_*` defaults.
- Link Battle Records screen and e-Reader Enigma Berry data are gone.
- Further bag growth needs more SaveBlock1 reclaim.
- `test/save.c` `T_SAVEBLOCK1_SIZE` is 15680 on the Emerald TESTELF (bag +304, reclaim −192). FireRed sits a few bytes under the 15872 sector cap.
