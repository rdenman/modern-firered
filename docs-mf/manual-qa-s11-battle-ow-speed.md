# Manual QA — S11 Battle & overworld speed baseline

Fast battle intro (no slide), faster HP/EXP bars, half battle text-wait multiplier, indoor running, Gen5+ no OW poison damage. Config lock: `make check TESTS='MF: speed'`.

---

## Story

- **ID:** S11
- **Feature / rule under test:** Always-on battle/OW pace before Options+
- **Build:** `pokefirered.gba` from `make firered -j$(sysctl -n hw.ncpu)` (non-release)
- **Date / tester:** …

## Setup

- **Rules / presets at new game:** N/A (compile-time only)
- **How to reach the scenario quickly:** Quickstart (title **Select**) → Route 1 grass for a wild battle; enter a building for indoor run; poison via battle then walk if testing poison (expect no OW ticks at Gen5+)
- **Save state slot (if used):** …

## Checklist

| # | Step | Expected | Pass? |
| - | ---- | -------- | ----- |
| 1 | Start a wild battle on Route 1 | No trainer/wild slide-in; battle UI appears quickly | |
| 2 | Use a damaging move and watch HP bar | HP drains noticeably faster than Gen3 vanilla | |
| 3 | Win and gain EXP (or force level-up with debug) | EXP bar fills quickly; no softlock | |
| 4 | Mash through several battle messages | Pauses feel ~half as long as vanilla (`B_WAIT_TIME_MULTIPLIER` 8 vs 16) | |
| 5 | Enter a house / Pokémon Center and hold B to run | Running works indoors | |
| 6 | Options → Text Speed still Slow / Mid / Fast | Changing Options still affects overworld dialogue (instant not forced) | |

## Negative / edge cases

| # | Step | Expected | Pass? |
| - | ---- | -------- | ----- |
| N1 | Full wild battle to conclusion (catch or faint) | Completes with no animation glitches or softlocks | |
| N2 | (Optional) Poison a party mon in battle, then walk in OW | No poison step damage / faint script (Gen5+); note S34 will toggle this | |

## Display consistency (if types/stats/moves/items change)

- N/A

## Notes / bugs

-
