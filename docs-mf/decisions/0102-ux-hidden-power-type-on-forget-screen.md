# 0102 — Show Hidden Power’s IV type on the forget-move screen

- **Type:** ux
- **Status:** Accepted
- **Date:** 2026-10-21
- **Story:** —
- **ME reference:** —
- **Expansion config:** `P_SHOW_DYNAMIC_TYPES` (left `FALSE`)

## Context

Hidden Power is Normal in item data. Its battle type comes from the user’s IVs. The bag TM pane has no Pokémon yet, so Normal is correct there. Once a Pokémon is chosen, the forget-move summary still showed Normal, which hid the type you were actually teaching.

Expansion can show every dynamic type (`P_SHOW_DYNAMIC_TYPES`), including Weather Ball and Natural Gift, on summary and in battle. That is wider than this request.

## Decision

Always resolve Hidden Power through `CheckDynamicMoveType` on the Pokémon summary move icons (existing moves and the new-move / forget-move row). Leave `P_SHOW_DYNAMIC_TYPES` off. The TM/HM bag info pane keeps `GetMoveType` (Normal).

## Alternatives considered

- Flip `P_SHOW_DYNAMIC_TYPES` — would also retint Weather Ball, Natural Gift, and battle move types.
- Bag TM type per currently hovered party member — no selected mon on that screen.

## Consequences

Teaching Hidden Power shows the IV type next to the new-move slot. Known Hidden Power on the summary moves page matches. Battle fight-menu type text stays Normal until someone opts into the expansion flag.
