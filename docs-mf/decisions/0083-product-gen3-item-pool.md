# 0083 — Gen 3 item randomizer pool; megas off

- **Type:** product
- **Status:** Accepted
- **Date:** 2026-10-11
- **Story:** S57 follow-up (—)
- **ME reference:** ME randomizes the full expansion item table; we do not copy that
- **Expansion config:** `MF_SPECIES_MEGA_EVOLUTIONS` (was TRUE in ADR 0008)

## Context

Expansion’s item enum is ~1000 entries (Mega Stones, Z-Crystals, Gems, Life Orb, Tera Shards, later balls, TM51–100, …). S57 remapped field grants into that whole table, so ITEMS runs felt like a later-gen dump instead of FireRed. Mega Evolutions were on per ADR 0008 but have no Kanto obtain path, so Mega Stones were dead weight in the pool.

## Decision

1. **Random dest pool** is FRLG/RSE items (balls, medicine, vitamins, Gen 3 berries, TM01–50, Gen 3 held items including Choice Band / Leftovers / Shell Bell) plus **later-gen evolution items** used by Gen 1–3 lines and their cross-evos (Ice/Shiny/Dusk/Dawn Stones, Electirizer, Magmarizer, Protector, Dubious Disc, Reaper Cloth, Prism Scale, Oval Stone, Razor Claw/Fang, Linking Cord, Black Augurite). Gen 4 baby incenses stay (breeding). Peat Block / Metal Alloy / Leader’s Crest / Galarica Cuff-Wreath are out (ADR 0103).
2. **Still out of the pool:** Mega Stones, Z-Crystals, Gems, Plates, Drives, Memories, primal orbs, later competitive holds (Life Orb, Focus Sash, Assault Vest, Choice Specs/Scarf, …), later balls, mints, Ability Capsule/Patch, Exp Candies, TM51–100, type berries, contest mail/scarves, Galar/Paldea evo junk for families we don’t enable (including Peat Block, Metal Alloy, Leader’s Crest).
3. **Do not delete item data** from `src/data/items.h`. Debug Give and expansion tests still need the IDs. Vanilla Kanto marts/field scripts were already Gen 3.
4. **Megas off** in `mf_species_enabled.h` (`MF_SPECIES_MEGA_EVOLUTIONS FALSE`). Primals stay on. Revises ADR 0008’s mega line only.

## Alternatives considered

- **Keep megas, only strip stones from the pool** — rejected; forms with no obtain path are ROM cost for nothing.
- **Allowlist every FRLG ID only, no cross-evo items** — rejected; Magnezone / Weavile / Togekiss / etc. would be unobtainable under ITEMS and awkward even when ITEMS is off if we later hide the items.
- **Strip later-gen rows from `gItemsInfo`** — rejected; huge upstream merge surface for no gameplay win once the pool is filtered.

## Consequences

- Same seed + ITEMS no longer maps Potion onto Assault Vest / Venusaurite. Mapping breaks vs S57 (ADR 0016).
- Nature mints remain shop-gated (S10 / mint rule), not random dests.
- FireRed ROM drops Mega sprites/cries (`!TESTING` only; TESTELF keeps expansion megas).
