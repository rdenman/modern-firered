# 0114 — Running Shoes from New Game

- **Type:** product
- **Status:** Accepted
- **Date:** 2026-11-01
- **Story:** —
- **ME reference:** —
- **Expansion config:** —

## Context
Stock FireRed withholds `FLAG_SYS_B_DASH` until Oak’s aide intercepts the player leaving Pewter after Brock. Autorun and indoor running already assume shoes; waiting until the first gym is leftover friction.

## Decision
1. Set `FLAG_SYS_B_DASH` in `MfRules_InitNewGame` (every new save) and in `MfSave_MigrateOnLoad` (existing saves that never met the aide).
2. Remove the Pewter aide object, the three east-exit coord triggers, and the gift script. Beating Brock no longer unhides that NPC.

## Alternatives considered
- **Keep the aide, pre-set the flag** — the scene would still fire and “give” shoes the player already has.
- **Hide the NPC only** — leftover scripts and the east-exit intercept would still exist.

## Consequences
- Pewter map/scripts diverge from upstream FRLG; re-apply after `RHH/master` mapjson merges.
- Debug **Toggle shoes** still clears the flag for movement tests.
- Pallet running-shoes manual text is unchanged.
