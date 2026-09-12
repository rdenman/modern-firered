# Manual QA — S10 Item & progression QoL

Reusable TMs, Exp Share policy, nature mint usability, Sitrus Gen4+ heal. Config lock: `make check TESTS='MF: item'`.

---

## Story

- **ID:** S10
- **Feature / rule under test:** Reusable TMs; held Exp Share unchanged; mints usable; Sitrus 25% heal
- **Build:** `pokefirered.gba` from `make firered -j$(sysctl -n hw.ncpu)` (non-release)
- **Date / tester:** …

## Setup

- **Rules / presets at new game:** N/A (compile-time only)
- **How to reach the scenario quickly:** Quickstart (title **Select**) → overworld debug (**R+Start**) → **Give item XYZ…** / **Fill Pocket…** as needed — see [`DEBUG.md`](./DEBUG.md)
- **Save state slot (if used):** …

## Checklist

| # | Step | Expected | Pass? |
| - | ---- | -------- | ----- |
| 1 | Debug → Give a TM (e.g. TM05 / Roar) ×1 and a second party mon that can learn it | TM appears in TM/HM pocket | |
| 2 | Teach the TM to party slot 1 | Move learned; TM **still in bag** (quantity unchanged) | |
| 3 | Teach the same TM to party slot 2 | Second teach succeeds; TM **still in bag** | |
| 4 | Confirm Exp. Share (if obtained via Oak’s aide or Give item) is a **held** item, not a Key Item party-wide toggle | Can give to one mon; rest of party does not auto-share without holding | |
| 5 | Debug → Give Adamant Mint → use on a mon with a different nature (e.g. Hardy) | Prompt names Adamant; after confirm, summary **nature line says Adamant** (not Hardy); Attack tinted up / Sp. Atk down if nature colors on; mint consumed | |
| 6 | Give a mon a Sitrus Berry, damage below the heal threshold in battle, wait for hold proc (or use from bag) | Heals **~25% max HP**, not flat 30 | |

## Negative / edge cases

| # | Step | Expected | Pass? |
| - | ---- | -------- | ----- |
| N1 | Try to sell a TM from the bag | Cannot sell (importance / key-like) | |
| N2 | Confirm no Celadon / mart mint stock yet | Mints not in vanilla FR shops (S34 follow-up) | |

## Display consistency (if types/stats/moves/items change)

- [ ] Summary nature matches after mint
- [ ] N/A for TM/Exp Share beyond bag quantity

## Evidence

- Screenshots / notes: …
- Automated coverage already green: `make check` □ / N/A □

## Follow-ups

- Bugs found: …
- Decision / ADR: [`decisions/0010-product-item-qol-baseline.md`](./decisions/0010-product-item-qol-baseline.md)
- S34: runtime gates + mint shop placement
---
