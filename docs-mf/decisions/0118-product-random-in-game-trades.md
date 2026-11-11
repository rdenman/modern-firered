# 0118 — Randomizer TRADES: random offers, wild-catchable requests

- **Type:** product
- **Status:** Accepted
- **Date:** 2026-11-10
- **Story:** —
- **ME reference:** — (ME has no trade randomizer)
- **Expansion config:** `MF_RANDOMIZER`; runtime `randomTrades` (`MF_TX_RANDOM_TRADES` default FALSE)

## Context

Players want the NPC in-game trades (Cerulean Jynx, Route 2 Mr. Mime, Underground Path Nidoran, etc.) to be randomized along with the rest of a randomizer run. Both sides of a trade change. The risk is the request side: a fully random request (e.g. Treecko) is often impossible to get in FireRed, especially when WILD POKéMON is off.

## Decision

1. New Randomizer row **Trades** (Off/On), gated on the master toggle. The bit is `randomTrades`, taken from `paddingTail` bit 3, with `MF_RULES_VERSION` 7 and old saves defaulting it to Off.
2. **Offered species** = `MfSpeciesMapEx(vanilla, seed, MF_RANDOM_CAT_TRADE, tradeId + 1, …)`. A new hash category keeps trades independent of the wild/static remaps. BALANCING and LEGENDARIES apply as they do for the other species randomizers.
3. **Requested species** = a species from the *active* wild tables (land, water, Rock Smash, and fishing, including Sevii and Modern/Postgame tables), after the WILD remap for that area. The request is therefore always catchable under the current rules. The pick hashes a flat slot index, samples up to 32 slots, and skips empty slots, Unown, and the offered species. Under BALANCING it prefers a slot whose evo stage matches the vanilla request, and otherwise falls back to the first valid sample.
4. **Every species is unique across all trades**, offered and requested alike. All trades are resolved together in table order, and each pick skips species already used: offers retry the remap, then fall back to a pool scan; requests retry slot samples, then fall back to a linear scan of the wild slots. The result is cached in EWRAM and keyed on the seed, the active wild tables, and the relevant randomizer flags.
5. Nickname, OT, IVs, personality, held item, and mail words stay vanilla. The mail's species icon follows the offered species.
6. Trades turn on the BALANCING/LEGENDARIES rows (`MfRules_RandomizerSpeciesActive`) and unlock the National Dex (`MfRules_RunNeedsNationalDex`). CHAOS does not re-roll trades.

## Alternatives considered

- Fully random requests from the species pool — rejected because some trades would be impossible to complete.
- Random requests only when WILD is on — rejected because Trades-only runs would get half-randomized trades.
- Uniform choice over unique wild species — rejected. It needs every wild slot remapped per call, thousands of `MfSpeciesMapEx` calls, which is a visible stall when talking to the NPC. Slot sampling costs at most 32 remaps, but favors species that fill many slots.

## Consequences

- The request may need a fishing rod, Surf, Rock Smash, the Safari Zone, or a Sevii area.
- With Encounters = Postgame, the request can change after the Elite Four because the active wild tables change.
- With Map-based on, the requested species is a specific area's remap. The player has to catch it where it actually appears, not just any species with the same vanilla name.
- Upstream touch: `src/trade.c` calls `MfTradeRandom_*` where it reads `sIngameTrades` species. It also exposes `GetInGameTradeVanillaSpecies` (declared in `include/trade.h`) so the resolver can see every trade.
