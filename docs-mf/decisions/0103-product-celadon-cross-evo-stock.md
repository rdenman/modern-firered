# 0103 — Celadon stocks Kanto-reachable cross-evo items

- **Type:** product
- **Status:** Accepted
- **Date:** 2026-10-21
- **Story:** — (S69 / S57 follow-up)
- **ME reference:** — (ME does not stock these on a Kanto mart)
- **Expansion config:** `I_USE_EVO_HELD_ITEMS_FROM_BAG`; runtime `randomItems`

## Context

ADR 0035 stocked Linking Cord and trade-held items at Celadon 4F, but later-gen stones and held items for enabled cross-evos (Gallade, Togekiss, Weavile, Glaceon, …) had no FireRed obtain path unless ITEMS remapped a chest onto them. ADR 0083 also left Peat Block / Metal Alloy / Leader’s Crest in the dest pool even though Ursaluna needs Hisui, Archaludon is Gen 8, and Kingambit is Gen 9.

## Decision

1. **Celadon 4F** also sells Ice / Shiny / Dusk / Dawn Stones, Oval Stone, Razor Claw, and Razor Fang. Same clerk, always listed.
2. **`MfEvoItemIsKantoObtainable`** matches that shop plus vanilla Fire/Water/Thunder/Leaf/Moon/Sun stones, so EVO LINES / EVOLUTIONS keep those methods instead of rewriting them to Linking Cord (revises ADR 0061’s “unobtainable stones” line).
3. **Item random dest pool** drops Peat Block, Metal Alloy, Leader’s Crest, and Galarica Cuff/Wreath. Debug Give still has the IDs (ADR 0083 §3).
4. **Still not stocked:** Black Augurite (Kleavor only with REGIONAL FORMS), Peat Block (Hisui region), Galarica items, Metal Alloy, Leader’s Crest. Black Augurite stays in the random dest pool.

## Alternatives considered

- **Stock Black Augurite too** — rejected; Kleavor is regional-only and the Features default is Off.
- **Leave later stones random-only** — rejected; ITEMS off made Gallade / Togekiss / Weavile / etc. dead lines.

## Consequences

- Same seed + ITEMS mappings change (smaller dest modulus; ADR 0016).
- Revises the Celadon list in ADR 0035 and the dest-pool list in ADR 0083.
- Moon / Sun stones stay field/NPC, not the shop (vanilla FRLG).
