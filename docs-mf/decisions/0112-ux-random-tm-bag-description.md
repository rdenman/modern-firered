# 0112 — Randomized TM bag descriptions follow dest

- **Type:** ux
- **Status:** Accepted
- **Date:** 2026-10-26
- **Story:** —
- **ME reference:** —
- **Expansion config:** `MF_RANDOMIZER`; runtime `randomTms`

## Context

ADR 0105 remaps TM01–50 dest moves and already makes bag **names** and TM type tint follow dest. Bag / shop / PC descriptions still came from `gItemsInfo` for the TM number, so TM13 could read Ice Beam while teaching something else. “Flavor paragraphs stay vanilla” in 0105 meant NPC tutor chatter, not the item menu.

## Decision

1. `GetItemDescription` uses dest when TMs is on and the slot remapped.
2. If dest is a vanilla TM/HM move, reuse that item’s bag-formatted description.
3. Otherwise copy `GetMoveDescription(dest)`, strip its summary-screen breaks, and `BreakStringAutomatic` into the bag’s 3-line × 102px box at `FONT_NORMAL`. If that still overflows, re-break at `FONT_NARROW`, then `FONT_NARROWER`, and prepend a font control so bag/shop `FONT_NORMAL` printers honor it.
4. HMs, TM51–100, and TMs-off stay vanilla.

## Alternatives considered

- Always `GetMoveDescription` as-is — rejected; summary-screen copy overflows the bag window; TM items already have three-line bag text.
- Shrink-only (`GetFontIdToFit`) — rejected; those lines are already `FONT_NARROWER` at ~152px.
- Leave vanilla flavor on the TM number — rejected; contradicts the dest name/tint and confuses teaching.

## Consequences

- One-line hook in `GetItemDescription`; lookup lives in `MfGetTmItemDescription`. Non-TM dests use a static RAM buffer (same pattern as Poké Vial).
- Shop, PC, and overworld item text that share `GetItemDescription` stay in sync with the bag.
