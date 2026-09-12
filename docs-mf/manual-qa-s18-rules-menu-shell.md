# Manual QA — S18 Rules menu shell

Throwaway demo page for the FR-styled rules menu (`src/mf_rules_menu.c`). Automated coverage: N/A (UI). Build: `make firered`.

---

## Story

- **ID:** S18
- **Feature / rule under test:** Paged list, cursor, L/R value cycling, description pane, NEXT / EXIT
- **Build:** `pokefirered.gba` from `make firered -j$(sysctl -n hw.ncpu)` (non-release)
- **Date / tester:** …

## Setup

- **Rules / presets at new game:** N/A (menu not in new-game flow yet; S19)
- **How to reach the scenario quickly:** Quickstart → overworld → **R+Start** → **Modern FireRed…** → **Rules menu demo…** — see [`DEBUG.md`](./DEBUG.md)
- **Save state slot (if used):** …

## Checklist

| # | Step | Expected | Pass? |
| - | ---- | -------- | ----- |
| 1 | Open **Rules menu demo…** | Title “RULES DEMO 1/2”; list + description pane; no graphical garbage | |
| 2 | D-pad **Down** past row 5 | List scrolls; highlight stays on the selected row; no leftover text | |
| 3 | Highlight **INFINITE TMS**; press **←/→** | OFF/ON cycle; description updates; selected value turns red | |
| 4 | Highlight **ENCOUNTERS** / **SHINY CHANCE**; cycle | Right-aligned active label changes; description matches | |
| 5 | **A** on **NEXT** | Page 2 title “RULES DEMO 2/2”; STURDY + EXIT | |
| 6 | **A** on **EXIT** (or **B** earlier) | Fade out; overworld returns; no stuck windows / black screen | |
| 7 | Re-open **Rules inspector…** | Demo toggles (e.g. InfTMs / Fairy / Spawns / ShinyCh) match what you set | |

## Negative / edge cases

| # | Step | Expected | Pass? |
| - | ---- | -------- | ----- |
| N1 | Exit mid-page with **B** | Clean return to overworld | |
| N2 | Open demo when rules are locked; cycle a value | Auto unlock override; write succeeds (or SE_FAILURE only if unlock unavailable) | |

## Display consistency (if types/stats/moves/items change)

- N/A (UI shell only; no gameplay wiring)

## Evidence

- Screenshots (commit under `docs-mf/qa-s18/` when capturing): page 1 list, scrolled list, description change, page 2, return to overworld
- Automated coverage: N/A □

## Follow-ups

- S19 hooks this CB2 into new-game; S20–S25 replace demo tables
- Bugs found: …
---
