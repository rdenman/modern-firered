# Playthrough QA matrix (S66)

Configs, checkpoints, and the written log for “does a run finish,” not only “does a toggle flip.”

Code: `MfQa_FillConfig` / `MfQa_ApplyToSave` in `src/mf_qa_matrix.c`. ADR [0080](./decisions/0080-product-qa-playthrough-matrix.md). Debug: **Modern FireRed… → QA matrix…**. Interactions: [RULE_INTERACTIONS.md](./RULE_INTERACTIONS.md). Fast paths: [DEBUG.md](./DEBUG.md).

## Configs

Required (story minimum). Each starts from `MfRules_ApplyDevDefaults` then overlays:

| ID | Name | Overlay |
| -- | ---- | ------- |
| 0 | **Classic** | Gamemode Classic (Fairy/modern stats/moves/types off). Randomizer and Nuzlocke off. |
| 1 | **Modern** | Same as `MfRules_InitNewGame` (default skip-menu preset). |
| 2 | **Full rand** | Modern + every randomizer remap on, Similar + map-based on, legendaries in the pool. **Chaos off** (playable; salt is not the “full” row). |
| 3 | **Hardcore** | Modern + Hardcore Nuzlocke (area lock, faint handling, Set style, level cap Normal, no player battle items, wipe on whiteout). |
| 4 | **Monotype** | Modern + ONE TYPE **FIRE** (`TYPE_FIRE` = 11). Fairy Type stays On so the combo is legal. |

ADR 0078 extras (same Surge checkpoint; warn at SAVE, still playable):

| ID | Name | Overlay |
| -- | ---- | ------- |
| 5 | **Nuz noCtr** | Modern + Normal Nuzlocke + no Poké Centers. |
| 6 | **Mono+rand** | Modern + Fire monotype + wild/trainer/static randomizer (Similar + map-based). |

`newLegendaries` stays **off** on every row (no new maps).

## Checkpoints

Play **New Game** (not only Quickstart) so Oak, the dex, and Nuzlocke `IsActive` match a real run. Quickstart + **QA matrix… → Apply …** is for save/load and combat smoke only.

| # | Checkpoint | Pass if |
| - | ---------- | ------- |
| C1 | Oak / Route 1 | Starter received; Route 1 wilds appear; catch or flee does not crash. Monotype: off-type balls refused. Nuzlocke: first Route 1 encounter consumes the area. |
| C2 | Brock | Pewter Gym completes; Boulder Badge; no softlock in Pewter/Mt. Moon approach. |
| C3 | Misty | Cerulean Gym completes. Cut path still exists (randomizer HM pins). |
| C4 | **Surge (required)** | Vermilion Gym completes. Story minimum. |
| C5 | Elite Four (ideal) | Champion; hall of fame. |
| C6 | Celio + tickets | After Sapphire: both tickets (or retry). **QA scenes… → Celio Sapphire / Vermilion tickets** — always-on, not per-config. |
| S1 | Save/load | In-game save after C1, C2, and C4; reset; Continue; rules and (Nuzlocke) used areas persist. |

Record softlocks, crashes, and balance outliers as follow-up stories. Do not invent map/story fixes here (`PROJECT.md`).

## How to apply a config

**Preferred (real run):** title → New Game → rules menu → match the overlay table → SAVE.

**Debug (smoke / save-load):** Quickstart → **R+Start → Modern FireRed… → QA matrix… → Apply \<name\>**. mGBA Warn log: `=== MF QA matrix ===`. Locks rules (inspector can still unlock). Recalculates party stats.

## Execution log

Build: `pokefirered.gba` from `make firered`. Tester / date as noted.

### Automated (2026-10-11, agent)

`make check TESTS='MF:'` including `test/modern_firered/mf_qa_matrix.c`.

| Config | No FORBID | Notes |
| ------ | --------- | ----- |
| Classic | pass | Fairy off, randomizer off, Nuzlocke off. |
| Modern | pass | Byte-match with `InitNewGame` (seed copied). |
| Full rand | pass | Chaos off; no conflict row. |
| Hardcore | pass | `noItemPlayer`, level cap seeded. |
| Monotype | pass | Fire + Fairy On. |
| Nuz noCtr | pass (WARN) | `NUZLOCKE_NO_CENTER`. |
| Mono+rand | pass (WARN) | `MONOTYPE_RANDOM`. |

Also green from earlier stories: species-map Cut/Surf/Strength pins; save migrate (S64); Celio tickets (S73 debug warps). ROM boots; no new `mf_*` warnings.

### In-game gyms (Brock → Surge)

Walk each required config to **C4**. Use this table; leave a cell empty until played.

| Config | C1 Oak/R1 | C2 Brock | C3 Misty | C4 Surge | S1 save/load | C5 E4 | Notes |
| ------ | --------- | -------- | -------- | -------- | ------------ | ----- | ----- |
| Classic | | | | | | | |
| Modern | | | | | | | |
| Full rand | | | | | | | |
| Hardcore | | | | | | | Do not white out (wipe). |
| Monotype | | | | | | | Fire catches only. |
| Nuz noCtr | | | | | | | Centers refuse heal. |
| Mono+rand | | | | | | | |

C6 (Celio / Vermilion sailor): S73 QA warps, once per ROM — not once per config.

## Follow-ups found

None blocking as of the automated pass. Altering Cave rotation stays parked in [SCOPE_CREEP.md](../SCOPE_CREEP.md).
