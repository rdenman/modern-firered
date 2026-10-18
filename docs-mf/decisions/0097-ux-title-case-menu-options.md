# 0097 — Title Case menu option labels

- **Type:** ux
- **Status:** Accepted
- **Date:** 2026-10-17
- **Story:** —
- **ME reference:** —
- **Expansion config:** —

## Context

Expansion already stores species, move, and ability names in Title Case (`Bulbasaur`). Battle commands are mixed case (`Battle` / `Bag` / `Pokémon` / `Run`). Start menu, Options+, rules, bag, party, PC, and shop option labels were still Gen 3 all-caps (`POKéDEX`, `TEXT SPEED`). ADR 0096 made the naming keyboard match Title Case; the rest of the menu chrome did not.

## Decision

Selectable menu option labels and their choice values use Title Case. Keep acronyms (`TM`, `HM`, `IV`, `EV`, `BST`, `HP`, `PC`, `LR`, `L=A`) and numbers as-is. Always-on; no Options+ or rules toggle.

In scope: title/continue, start menu, Options / Options+, rules menu (labels and values), Yes/No, shop Buy/Sell/Quit, bag pockets and item actions, party actions, player PC, and Pokémon Storage.

Out of scope at the time: remaining all-caps nouns in NPC text and map names (done in [0098](0098-ux-title-case-frlg-text.md)). Item/move/ability names were already Title Case.

## Alternatives considered

- Convert every all-caps UI string in `strings.c` — rejected; too broad, fights remaining Gen 3 dialogue voice.
- Options+ toggle — rejected; this is chrome, not a mid-run rule.

## Consequences

Upstream `src/strings.c` and menu files will conflict on `RHH/master` merges; re-apply Title Case on the option labels, not on untouched dialogue. Some menus (Dex, summary stats) still look all-caps until a later pass.
