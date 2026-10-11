# 0070 — Bag sort, cursor memory, run prompt, and units

- **Type:** ux
- **Status:** Accepted
- **Date:** 2026-10-11
- **Story:** S63
- **ME reference:** `src/options_plus_menu.c` (CURSOR MEMORY, RUN PROMPT, UNITS); expansion `SortItemsInBag`
- **Expansion config:** `UNITS` (`UNITS_IMPERIAL`) remains the compile fallback when Options+ is off

## Context
S63 needed bag sorting to be discoverable, a remembered sort preference, CURSOR MEMORY across menu visits, RUN PROMPT (L/R), button mode, and UNIT SYSTEM — without ME follower/music/background options. Expansion already had START-to-sort and session `gBagPosition` cursor/scroll. R is the last-used Ball button (S61 BALL PROMPT). Vanilla FR measurements are imperial.

## Decision
- **Bag sort:** Keep START-to-sort. CLOSE BAG description shows `{START} Sort items`. Last sort type is stored in `MfOptions.bagSort` (2 bits, not an Options+ row) and pre-selected the next time START is pressed. Item order already persists in the bag save; do not re-sort on open (that would fight SELECT-swap).
- **CURSOR MEMORY (default On):** Bag pocket/scroll/cursor stay in `gBagPosition` while On; Off resets them on each new bag open (`ITEMMENULOCATION_LAST` keep-alive is unchanged). Field party from the start menu uses `keepCursorPos` when On. Battle single-target select restores the last target for the same move slot (ME’s cursor-memory meaning).
- **RUN PROMPT (default Off):** In wild battles, L confirms Run unless Button Mode is L=A. R also confirms Run only when Ball Prompt is Off so it does not steal last-used Ball. Safari uses the same helper. Trainer battles are unchanged. B-to-Run remains S71.
- **UNIT SYSTEM:** Runtime `MfOptions_UseImperialUnits()` drives Pokédex (and size-record) height/weight. Menu 0 is Metric, 1 is Imperial. Default Imperial to match `UNITS_IMPERIAL`; v3 saves migrate both imperial and cursor memory On.
- **BUTTON MODE:** Already on the vanilla Options+ page (S58); no extra row.

## Alternatives considered
- Auto-sort the bag on open using the remembered type — rejected; it would undo manual SELECT order.
- ME L+R visual flee prompt / QUICK RUN hold — rejected; FR already uses R for balls and S71 covers B-to-Run.
- Compile-time `UNITS` only — rejected; player-facing options must be runtime.

## Consequences
`MF_OPTIONS_VERSION` is 4. `padding0` became `bagSort`. Upstream diffs: `item_menu.c`, `party_menu.c`, `pokedex.c`, `battle_controller_player.c`, `battle_controller_safari.c`.
