# Manual QA checklist template

Copy this into a story’s working notes or paste filled bullets under that story’s **Tests** / acceptance follow-up when gameplay wiring cannot be covered by `make check` (see [`TESTING.md`](./TESTING.md)).

Replace bracketed placeholders. Delete rows that do not apply.

---

## Story

- **ID:** S##
- **Feature / rule under test:** …
- **Build:** `pokefirered.gba` from `make firered -j$(sysctl -n hw.ncpu)` (non-release unless noted)
- **Date / tester:** …

## Setup

- **Rules / presets at new game:** (e.g. Gamemode Modern, Nuzlocke Normal, …) — or “N/A (compile-time only)”
- **How to reach the scenario quickly:** Quickstart / pre-Oak save state / debug warp / cheat start — see [`DEBUG.md`](./DEBUG.md)
- **Save state slot (if used):** …

## Checklist

| # | Step | Expected | Pass? |
| - | ---- | -------- | ----- |
| 1 | … | … | |
| 2 | … | … | |
| 3 | … | … | |

## Negative / edge cases

| # | Step | Expected | Pass? |
| - | ---- | -------- | ----- |
| N1 | Rule off / master switch compiled out | Vanilla-plus-Phase-1 behavior | |
| N2 | … | … | |

## Display consistency (if types/stats/moves/items change)

- [ ] Battle UI matches the change
- [ ] Summary screen matches
- [ ] Pokédex (if shown) matches

## Evidence

- Screenshots / notes: …
- Automated coverage already green: `make check` □ / N/A □

## Follow-ups

- Bugs found: …
- Decision / ADR needed?: …
---
