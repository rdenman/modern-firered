# 0098 — Title Case FireRed labels and proper nouns

- **Type:** ux
- **Status:** Accepted
- **Date:** 2026-10-17
- **Story:** —
- **ME reference:** —
- **Expansion config:** —
- **Tool:** `tools/mf/title_case_text.py`

## Context

ADR 0097 Title-cased menu chrome. Species/move/item names were already Title Case from expansion. NPC sentences were already mixed case; leftover all-caps were place names, `POKéMON`/`POKéDEX`, speaker tags, trainer classes/names, mapsec popups, and similar labels. Players still saw `PALLET TOWN` next to `Bulbasaur`.

## Decision

Title Case FireRed-visible labels and proper nouns. Keep mixed-case sentence structure. Keep acronyms (`HP`, `PP`, `TM`, `HM`, `PC`, `TV`, `ID`, `IV`, `EV`, `OHKO`, Roman numerals, `RHH`). Always-on; no Options+ toggle.

In scope: `_Frlg` map scripts, `data/text/*.inc`, `data/scripts/*.inc`, mapsec names, FRLG trainer names, trainer classes, and remaining UI strings in `strings.c` / related screens.

Out of scope: Hoenn overworld scripts (not played on FireRed), item/move/ability data tables (already Title Case).

## Alternatives considered

- Font remap so lowercase glyphs draw as caps — rejected; it would shout dialogue too.
- Recase species/move/item tables back to all caps — rejected; fights every `RHH/master` merge.

## Consequences

Re-run `python3 tools/mf/title_case_text.py` after an upstream merge if FRLG script casing regresses, then spot-fix `S.S. Anne`, `IDNo.`, Roman numerals, and `RHH`. Extends ADR 0097.
