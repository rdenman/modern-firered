# 0079 — Celio gives MysticTicket and AuroraTicket after the Sapphire

- **Type:** product
- **Status:** Accepted
- **Date:** 2026-10-11
- **Story:** S73
- **ME reference:** — (Hoenn Mystery Gift / Lilycove harbor)
- **Expansion config:** —

## Context

Navel Rock and Birth Island exist in stock FRLG but the tickets that unlock the Vermilion seagallop destinations are Mystery Gift only. `PROJECT.md` allows a narrow Celio Sapphire payoff so a normal playthrough can reach Lugia and Deoxys without linking. Mystery Gift copy points at Lilycove; FireRed’s sailor is in Vermilion.

## Decision

1. **Always-on.** No Features or Options+ toggle. After the Network Machine links to Hoenn, Celio gives `ITEM_MYSTIC_TICKET` and `ITEM_AURORA_TICKET` and sets `FLAG_ENABLE_SHIP_NAVEL_ROCK`, `FLAG_ENABLE_SHIP_BIRTH_ISLAND`, `FLAG_RECEIVED_MYSTIC_TICKET`, and `FLAG_RECEIVED_AURORA_TICKET`.
2. **Celio is the giver**, not the sailor. Vermilion already branches on item+flag; we do not reimplement the ferry.
3. **Bag-full fallback.** The Sapphire scene still consumes the gem (vanilla). If either ticket does not fit in Key Items, Celio keeps offering them on scene 6 and 7 talks until both `FLAG_RECEIVED_*` flags are set. A second successful talk does not duplicate items.
4. **Copy** points at the Vermilion seagallop sailor, not Lilycove. Mystery Gift scripts are untouched.

## Alternatives considered

- **Features-page toggle** — rejected; the story and `PROJECT.md` want a normal playthrough to reach the islands.
- **Sailor as giver** — rejected; the sailor only checks tickets, and Celio’s Sapphire scene is the one allowed story beat.
- **Refuse to take the Sapphire until two Key Item slots are free** — rejected; removing the Sapphire already frees a slot, and a retry talk covers the second ticket.

## Consequences

- Upstream touch: `data/maps/OneIsland_PokemonCenter_1F_Frlg/scripts.inc` only. Do not edit Mystery Gift or Vermilion sailor scripts.
- QA warps under **Modern FireRed → QA scenes…**: Celio Sapphire, Celio ticket retry, Vermilion tickets.
- Islands 1–7 and Rainbow Pass stay vanilla.
