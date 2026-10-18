# 0063 — Item remap with key/HM pins; chaos salt only on encounters

- **Type:** product
- **Status:** Accepted
- **Date:** 2026-10-09
- **Story:** S57
- **ME reference:** `IsRandomItemsActivated()` (ME skips until adventure/dex; we do not — ADR 0092); ITEMS description “Key items are excluded”; CHAOS MODE “every above chosen option” with items not chaos-eligible in the menu
- **Expansion config:** `MF_RANDOMIZER` (compile-out only); runtime `randomItems` / `randomChaos`

## Context

S57 must randomize field and hidden grants without blocking Kanto (HMs, key items) and must define chaos so it re-rolls encounters without corrupting the stable S16 maps (items, types, moves, dex-facing wild tables). ME chaos calls `Random()` inside species remaps. ADR 0016 forbids putting that inside `MfRandom_*`. Display consistency forbids re-rolling types/moves per battle. Poké marts later stayed vanilla (ADR 0088).

## Decision

1. **ITEMS.** Field grants hash per chest into the non-dummy, non-protected **Gen 3 + evo-exception** pool (ADR 0083 / 0098). `MfItemMapActive` is identity unless ITEMS is on. Timing no longer waits for the Pokédex (ADR 0092). Oak’s five `ITEM_POKE_BALL` stay vanilla (ME 3.5).
2. **Protected (identity, never dest).** Key-item pocket, HM01–HM08, `ITEM_POKE_BALL`, fossils (ADR 0099), and `GetItemImportance != 0` except TMs. TMs may remap.
3. **Hooks.** `Std_ObtainItem` / `Std_FindItem` / `giveitem_msg` remap `VAR_0x8000` once; hidden items remap `VAR_0x8005`. Not `AddBagItem` (would double-remap and scramble PC/Nuzlocke returns). Poké marts do **not** remap (ADR 0088).
4. **CHAOS.** Still disables Balancing and includes legendaries on the stable species mapper. In addition, `MfRandom_TakeChaosSalt` (S16 mix, never `Random()`) is XOR’d into **wild CreateWildMon** (`MfWildBattleSpecies`) and **trainer party** remaps only. `MfWildEncounterSpecies` / dex / item maps ignore the salt. Types, moves, abilities, evos, statics stay seed-stable.

## Alternatives considered

- **Remap inside `AddBagItem`** — rejected; bag moves and debug Give would remap twice or at all.
- **ME `Random()` per chaos call** — rejected; ADR 0016 / Phase 9 determinism. Salt is an explicit call-site opt-out.
- **Chaos re-roll of items/types** — rejected; menu already excludes items from chaos eligibility; per-battle types would desync summary vs battle.

## Consequences

- Same seed + ITEMS: the same chest is always the same replacement; two Potion balls can differ. Mart Potion is still Potion (ADR 0088).
- Chaos grass/trainer parties change each encounter this boot; save/load resets the nonce (EWRAM). Dex area lists stay on the stable map.
- Changing the item pool or category id is a mapping break (ADR 0016).
