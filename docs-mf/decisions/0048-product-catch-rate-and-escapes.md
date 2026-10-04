# 0048 — Catch rate multiplier and escape restrictions

- **Type:** product
- **Status:** Accepted
- **Date:** 2026-10-03
- **Story:** S45
- **ME reference:** `tx_Difficulty_CatchRate` in `src/battle_script_commands.c`; `tx_Challenges_LessEscapes` in `src/battle_util.c`; `tx_Difficulty_EscapeRopeDig` in `src/item_use.c`
- **Expansion config:** `B_INCAPACITATED_CATCH_BONUS` / `B_LOW_LEVEL_CATCH_BONUS` / `B_MISSING_BADGE_CATCH_MALUS` stay as compile-time bonuses; the Difficulty row is an extra species-rate scale on top

## Context

S45 wires CATCH RATE, LESS ESCAPES, and ESC. ROPE / DIG. Expansion has no global catch multiplier. ME halves/doubles/triples the species catch rate (floor 3, cap 255) before ball math. ME’s less-escapes path always uses the speed formula and compares against `Random() & 512` (0 or 512) with a `u8` chance var, so extra run attempts cannot guarantee a flee. ESC. ROPE / DIG is Yes=allowed (0) / No=banned (1). Dungeons still have walking exits; Fly and Teleport stay legal.

## Decision

1. **CATCH RATE** scales the species (or Safari) catch rate after lookup and before ball bonuses: 0 = 1×, 1 = ½ (floor 3), 2 = 2× (cap 255), 3 = 3× (cap 255). Unknown values are 1×. Do not port ME’s `catchRate_hard` / Options Hard legendary extra cut.
2. **LESS ESCAPES** always rolls, even when faster. Chance var is `u32`: `(playerSpeed * 128) / foeSpeed + runTries * 30` vs `Random() & 0x1FF`. Enough attempts always succeed. Smoke Ball, Run Away, Ghost, and trainer/forfeit paths stay unchanged.
3. **ESC. ROPE / DIG** bans field Escape Rope and field Dig only. Battle Dig as an attack is unchanged. Escape Rope on a map that would allow leaving prints a dedicated refusal; Dig still uses “Can't use that here.”
4. **Softlock path:** walking out, Fly, Teleport, and white-out remain. Flee is never a hard lock.

## Alternatives considered

- Match ME’s `Random() & 512` — rejected; it is 50/50 forever with a `u8` chance var.
- Hide Escape Rope in the bag — rejected; shown-and-refused matches S43 and ME.
- Ban battle Dig / Teleport / Fly — rejected; ME only gates the field escape tools.

## Consequences

- Upstream one-liners: `ComputeCaptureOdds` in `battle_script_commands.c`, `TryRunFromBattle` in `battle_util.c`, `CanUseDigOrEscapeRopeOnCurMap` / `ItemUseOutOfBattle_EscapeRope` in `item_use.c`.
- Helpers live in `mf_catch.c`.
- Hall of Fame does not auto-clear these rows; edit Difficulty (unless LOCK DIFFICULTY) or debug.
