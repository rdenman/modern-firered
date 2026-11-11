# 0121 — Gift Only challenge: no catching, gifts and prizes only

- **Type:** product
- **Status:** Accepted
- **Date:** 2026-11-10
- **Story:** —
- **ME reference:** — (FR-only; ME has no gift-only mode)
- **Expansion config:** —

## Context
Players asked for a "gift run": the team comes only from Pokémon the game hands you (starter, Magikarp salesman, Dojo Hitmon, Eevee, Lapras, fossils, Game Corner prizes, Togepi egg) instead of catching. Without rule support, the run breaks in three places. Oak's Aides and the National Dex unlock are gated on catch counts. A Squirtle start has no Cut user before Celadon. Some other rules make the team pool empty or let you acquire Pokémon another way.

## Decision
New Challenges-page option **Gift Only** (Off/On, default Off). The bit is `giftOnly`, taken from `paddingTail` bit 6, with `MF_RULES_VERSION` 10 and old saves defaulting to Off. Like other Challenges rules, it is CORE and locks when the new-game rules are committed.

| Area | Behavior |
|---|---|
| Wild, static and legendary Pokémon | Uncatchable. Bag ball use and the Safari Zone BALL command are refused with "Gift Only: Pokémon can't be caught on this adventure!". Battles still happen for EXP. Shinies are not exempt. |
| Balls, marts, Safari Zone | Unchanged. Balls stay in the bag and in marts, and the Safari Zone stays open. |
| Allowed sources | Everything that goes through `givemon`/`giveegg`: starter, NPC gifts, Game Corner prizes, fossils, the Togepi egg, in-game trades, and Day Care breeding. |
| Catch Rewards (ADR 0119) | In **Catches** mode the requirement is waived. Aides hand over their item with a short "Prof. Oak asked me to give you this anyway" line, and Oak's National Dex check passes. Badges and Levels modes still apply as chosen, because they are reachable. |
| Bag HMs | Gift Only joins Nuzlocke, Mirror, Monotype and Random Moves in letting the **lead** field-use any HM in the bag (`MfFieldHm_OverwriteActive`). |
| Randomizer | No new hook. Gifts, prizes and fossils already route through `MfStaticGiftSpecies`, so **Static Pokémon** randomizes them. |
| One Type Only | **Forbid** (rules conflict). Too few gift species for a single-type team. |
| Mirror Thief | **Forbid** (rules conflict). Thief keeps the enemy party, which is a non-gift source. Mirror without Thief is allowed. |
| Nuzlocke | Allowed. Area locks never matter because nothing is catchable, and faint rules still apply. |

The check sits before the Nuzlocke and Monotype checks in both ball paths, so the Gift Only message wins.

## Alternatives considered
- **Tiered setting (Strict / +Legendaries / +All statics)**: rejected in favor of a single strict mode.
- **Hide balls from marts and close the Safari Zone**: rejected. A refusal message is clearer, and those changes would need map and mart edits.
- **Lower the dex thresholds instead of waiving them**: rejected. The reachable owned count depends on the starter and the evolutions, so any fixed number is arbitrary.
- **Allow Monotype with a warning**: rejected. Most types have zero or one gift species, which is an unwinnable run.

## Consequences
- Upstream edits: one-line hooks in `src/item_use.c` (ball throw state + message) and `src/battle_main.c` (Safari ball), plus a battle string id, a battle script, and an `item_use.h` enum value.
- In-game trades stay allowed, but most of them ask for wild-only species, so in practice only a few are reachable (for example, Abra from the Game Corner for the Mr. Mime trade). Randomized trades (ADR 0118) still ask for wild species.
- Tests: `test/modern_firered/mf_gift_only.c`.
