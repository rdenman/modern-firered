# 0092 — Item randomizer starts before the Pokédex

- **Type:** product
- **Status:** Accepted
- **Date:** 2026-10-17
- **Story:** —
- **ME reference:** `IsRandomItemsActivated()` waits for adventure/dex — we diverge
- **Expansion config:** `MF_RANDOMIZER`; runtime `randomItems`

## Context

ADR 0063 copied ME’s starting-item skip: `MfItemMapActive` was identity until `FLAG_SYS_POKEDEX_GET`. That left the bedroom PC Potion, the Route 1 mart sample, and the Viridian Cut-tree ball as vanilla on the Parcel run. Mapping rules (pins, marts, pool) were fine; only the gate was too late.

## Decision

1. **ITEMS remaps as soon as the rule is on.** No Pokédex flag. Hash, pins, and mart identity stay as in ADR 0063 / 0088 / 0083.
2. **Oak’s five `ITEM_POKE_BALL` stay vanilla** via the existing Poké Ball pin.
3. **Route 1 clerk** uses `giveitem` so the sample goes through `MfScriptRemapGrantItem` (it previously `additem`’d a hardcoded Potion).
4. **Bedroom PC Potion** is written in `NewGameInitData` before rules SAVE. `MfRules_CommitAndLock` calls `MfRemapNewGamePCItems`, which rewrites that slot in place if it is still Potion ×1.

## Alternatives considered

- **Keep the dex gate, special-case early gifts** — rejected; every pre-dex `finditem` / hidden would still be vanilla.
- **Remap all PC slots at lock** — rejected; a debug re-commit would double-map deposited field items.

## Consequences

- Same seed: a field Potion is the same replacement on Route 1 as later in Kanto.
- Players can withdraw a remapped PC item before leaving Pallet.
- ME’s dex skip is an intentional divergence.
