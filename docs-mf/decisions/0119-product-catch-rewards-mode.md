# 0119 — Catch Rewards: catches, badges, or party level

- **Type:** product
- **Status:** Accepted
- **Date:** 2026-11-10
- **Story:** —
- **ME reference:** — (FR-only; ME has no Aide rewards)
- **Expansion config:** —

## Context
Oak's Aides gate HM05, Everstone, Itemfinder, Amulet Coin, and Exp. Share behind Pokédex catch counts (10/20/30/40/50). Oak gates the National Dex behind 60 caught. On One Type Only (monotype) runs the player cannot reasonably catch that many species, so these rewards become near-unreachable.

## Decision
New Features-page option **Catch Rewards** (`catchRewards`, 2 bits from `paddingTail`, rules v8):

| Reward | Catches (default) | Badges | Levels (highest party mon) |
|---|---|---|---|
| HM05 (Route 2) | 10 | 3 | 20 |
| Everstone (Route 10) | 20 | 4 | 30 |
| Itemfinder (Route 11) | 30 | 5 | 40 |
| Amulet Coin (Route 16) | 40 | 6 | 50 |
| Exp. Share (Route 15) | 50 | 7 | 60 |
| National Dex (Oak) | 60 | 8 | 60 |

- Levels counts the **party only** (eggs excluded), not PC boxes.
- Badges/Levels skip the vanilla YES/NO "have you caught enough?" prompt; a shared Aide script states the requirement, checks it, and hands off to each Aide's original give/no-room path.
- Catch mode keeps vanilla scripts untouched, and counts the National Dex once it is enabled.
- Oak's National Dex check follows the option in both the Pallet Town cutscene and the lab talk path.
- Thresholds live in `src/mf_catch_rewards.c`; scripts call `MfCatchReward_GetMode` / `MfCatchReward_Check`.

## Alternatives considered
- Auto-switch on monotype only — less flexible; badges/levels also help randomizer and Nuzlocke runs.
- Count PC boxes for Levels — makes the gate trivial to cheese and costs a box scan for little gain.
- Exp. Share at Lv. 50 — rejected for a clean +10 ladder.

## Consequences
- Rule is CORE: locked after new-game commit.
- Old saves migrate to Catches (v8 `if (version < 8)`).
- Upstream script edits: five Aide maps, Pallet Town, Oak's Lab (small inserted branches + one label each).
