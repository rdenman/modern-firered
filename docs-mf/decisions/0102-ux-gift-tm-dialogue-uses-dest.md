# 0102 — Gift TM dialogue names the dest TM and move

- **Type:** ux
- **Status:** Accepted
- **Date:** 2026-10-21
- **Story:** — (S57 follow-up)
- **ME reference:** —
- **Expansion config:** `MF_RANDOMIZER`; runtime `randomItems`

## Context

S57 remaps gym and NPC TM grants through `MfScriptRemapGrantItem`, but FRLG scripts still hardcode “TM39 contains Rock Tomb.” With ITEMS on, Brock can hand over TM05 while still saying Rock Tomb.

Flavor that does not name the TM (Blaine’s “don’t waste it on Water Pokémon”) is fine to leave.

## Decision

1. `MfScriptBufferGiftTmNames` hashes the vanilla source TM with the same grant site as the item remap, then fills `STR_VAR_1` with the dest item name (`TM05`) and `STR_VAR_2` with that TM’s move.
2. Gym leaders and other FRLG gift TMs (Mr. Psychic, Route 12 Return, Cerulean Dig, Celadon roof drinks) interpolate those vars in receive/explain lines.
3. Celadon roof `additem` now remaps through the grant helper so the bag and the explain line agree.
4. Game Corner TM coin prizes stay vanilla (shop-like; ADR 0088).

## Alternatives considered

- **Only rewrite Brock** — rejected; every named gift TM has the same mismatch.
- **Rewrite type-flavor lines** — rejected; player asked to keep them.

## Consequences

- FRLG `*_Frlg` scripts use `{STR_VAR_1}` / `{STR_VAR_2}` for TM number and move. Vanilla ITEMS-off still prints the original names.
- Subsequent “talk again” explains (Misty, Sabrina, Blaine, Mr. Psychic, Route 12) re-buffer so they stay in sync.
