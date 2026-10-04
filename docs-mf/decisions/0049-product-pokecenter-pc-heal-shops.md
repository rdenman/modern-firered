# 0049 — Poké Center ban, PC heal, and shop price scaling

- **Type:** product
- **Status:** Accepted
- **Date:** 2026-10-04
- **Story:** S46
- **ME reference:** `IsPokecenterChallengeActivated()`, `ItemId_GetPrice()` expensive multipliers, PC heal skips in `pokemon_storage_system.c`
- **Expansion config:** `OW_PC_HEAL` stays `GEN_LATEST` (unused for this challenge)

## Context

S46 wires ME’s Challenges rows `POKéCENTER`, `PC HEALS {PKMN}`, and `ULTRA EXPENSIVE!`. Expansion already compiles `OW_PC_HEAL` as Gen 8+ (no heal on deposit, nurse heals boxed mons). The player-facing option from S24 is Yes/No PC heal, default Yes. Whiteout must still restore the party or a no-Center run can softlock.

## Decision

1. **Nurse Joy and free heal pads** (Pokémon Tower 5F, Ember Spa) refuse healing when `pokeCenterLimit != 0`, with an explicit message. Whiteout, Oak’s Lab after the rival, Trainer Tower start/loss, Hall of Fame, debug heal, and link/minigame heals still restore the party.
2. **PC deposit heal** follows the Challenges options, not `OW_PC_HEAL`. Heal on deposit when Centers are allowed and `noPcHeal` is off. Banning Centers also disables PC heal (ME, and the S24 menu already gates the row). Nurse does not heal boxed Pokémon unless that same deposit-heal rule is on.
3. **Shop prices** multiply buy cost by 1 / 5 / 10 / 50 via `GetItemPrice`, so sell price (`GetItemPrice / ITEM_SELL_FACTOR`) stays a quarter of the listed buy price. Zero-price and key items stay unsellable. Scaled prices cap at `MAX_MONEY` so the 6-digit mart UI cannot overflow.
4. **No extra “no-Center” mart tables.** Kanto already sells Potions from Viridian; whiteout remains the completeness valve. ME’s badge-scaled `sShopInventories_PC` lists are Emerald-specific and are not ported.

## Alternatives considered

- Silent no-op at Nurse Joy — rejected; the story requires a clear in-game explanation.
- Also skip whiteout healing — rejected; that makes fainted-party progress unwinnable.
- Leave `OW_PC_HEAL` as the deposit gate — rejected; it would ignore the S24 Yes/No row and keep default Modern runs on Gen 8 no-heal despite the menu defaulting to Yes.
- Scale buy only, leave sell vanilla — rejected; pickup dumps would print money relative to inflated buy prices.

## Consequences

- Default Modern (PC HEALS Yes) now heals on deposit even though `OW_PC_HEAL` is still `GEN_LATEST`. Upstream merges that touch those `#if`s should keep calling `MfShouldHealOnPcDeposit()`.
- Expensive shops inflate found-item sell value by the same factor; the buy:sell ratio is unchanged, so marts are not a money printer.
- Follow-up if no-Center early game feels starved: a Kanto mart inventory bump, not a whiteout skip.
