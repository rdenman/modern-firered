# 0117 — Fainted participants still receive battle EXP

- **Type:** product
- **Status:** Accepted
- **Date:** 2026-11-02
- **Story:** —
- **ME reference:** —
- **Expansion config:** —

## Context

`Cmd_getexp` only awarded EXP to `IsValidForBattle` party mons, which requires HP > 0, and skipped `EmitExpUpdate` when HP was 0. A wild KO in the same turn as the winner fainting (Rocky Helmet, Rough Skin, Iron Barbs, Aftermath, recoil) therefore granted no EXP. With item/ability randomizer that is an intermittent wild-battle miss.

Vanilla still pays the Pokémon that got the KO, even if it is already fainted.

## Decision

1. **Eligibility uses `IsValidForBattleButDead`.** Eggs and empty slots stay skipped; HP 0 does not.
2. **`EmitExpUpdate` still runs at 0 HP** so the party EXP field is written. Level-up while fainted keeps current HP at 0 (`CalculateMonStats`); Dynamax HP add is unused in wilds.

## Alternatives considered

- **Leave it** — rejected; it is a real “no EXP” wild battle and is worse with randomizer contact-punish items/abilities.
- **Apply EXP in `Cmd_getexp` without the controller** — rejected; the existing update path already writes party EXP and records the test bar.

## Consequences

- Upstream touch: `battle_script_commands.c` (getter filter + HP check) and a declaration in `battle_controllers.h`.
- Nuzlocke: a dead lead still gets the EXP; bench mons still need participation or Exp. All / held Share.
