# Manual QA — S09 Summary screen IV/EV display

Filled from [`manual-qa-template.md`](./manual-qa-template.md). Feature is compile-time config only; not covered by `make check` on FR layout.

## Story

- **ID:** S09
- **Feature / rule under test:** Skills-page Stats → IVs → EVs cycle (`P_SUMMARY_SCREEN_IV_EV_*`)
- **Build:** `pokefirered.gba` from `make firered -j$(sysctl -n hw.ncpu)` (non-release)
- **Date / tester:** 2026-09-12 / (fill on play)

## Setup

- **Rules / presets at new game:** N/A (compile-time only)
- **How to reach the scenario quickly:** Title → Select (Quickstart) → overworld with starter; open party → Summary → skills page (second page). Optional: debug Utilities → Cheat Start for more party members.
- **Save state slot (if used):** …

## Checklist

| # | Step | Expected | Pass? |
| - | ---- | -------- | ----- |
| 1 | Open Summary → skills page | Stats shown; A-prompt reads **IVs**; header graphic says **STATS** (tileset) | |
| 2 | Press **A** | Numbers become IVs (0–31); prompt → **EVs**; header → **IVs**; no tile/text corruption | |
| 3 | Press **A** again | Numbers become EVs (0–252); prompt → **STATS**; header → **EVs** | |
| 4 | Press **A** again | Back to battle stats (HP current/max); prompt → **IVs**; header → **STATS** | |
| 5 | Repeat on 2–3 different species / levels | Same cycle; values look plausible for each mon | |
| 6 | PC → Summary on a boxed mon | Same IV/EV cycle available (not box-only) | |

## Negative / edge cases

| # | Step | Expected | Pass? |
| - | ---- | -------- | ----- |
| N1 | Info page / moves pages | No stray IVs/EVs prompt; rename/cancel and move Info/Switch still work | |
| N2 | Nature-colored stats | Colors apply on **STATS** mode only; IV/EV numbers stay neutral | |

## Display consistency (if types/stats/moves/items change)

- N/A for S09 (display of existing IV/EV fields only)

## Evidence

- Screenshots / notes: (attach skills-page STATS / IVs / EVs frames)
- Automated coverage already green: `make check` □ / N/A ☑ for FR layout (Emerald TESTELF shares the same summary code path)

## Follow-ups

- Bugs found: …
- Decision / ADR: [`decisions/0009-ux-summary-iv-ev-display.md`](./decisions/0009-ux-summary-iv-ev-display.md)
