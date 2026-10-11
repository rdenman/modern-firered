# Rule interaction matrix (S65)

What happens when two (or more) start-of-run rules are on together. The menu uses this table: **forbid** reverts the last change and shows why; **warn** keeps the combo and replaces the description pane (SAVE also asks “anyway”). **Allow** is silent.

Checker: `MfRules_EvaluatePair` / `MfRules_FindConflict` in `src/mf_rules_conflicts.c`. ADR [0078](./decisions/0078-product-rule-interaction-matrix.md).

Existing intra-page gates (Custom-only gamemode rows, Nuzlocke sub-options, PC heal, Mirror Thief, randomizer master / chaos / balancing) are unchanged — they are not pairs.

| Pair | Action | Why |
| ---- | ------ | --- |
| Fairy monotype + Fairy Type Off | **Forbid** | `GetSpeciesType` has no Fairy slot; Oak cannot give a legal starter. |
| Monotype + species/type randomizer | **Warn** | Remaps ignore the type challenge; many wilds/trainers/statics are unusable. Catch still refuses off-type. |
| Monotype + Nuzlocke (Normal/Hardcore) | **Warn** | Off-type wilds do not consume the area encounter (ADR 0051). Sparse legal first-meets. |
| Nuzlocke (incl. Easy) + no Poké Centers | **Warn** | Healing is items and/or PC only. |
| Nuzlocke + no Centers + no PC heal | **Warn** | Overworld items only. Shown instead of the Centers-only warn when both apply. |
| Chaos + Nuzlocke (Normal/Hardcore) | **Warn** | Chaos salt can change a wild if you flee and re-enter; first-encounter is less stable. |
| Nuzlocke (incl. Easy) + EXP ×0 | **Warn** | No battle EXP; Rare Candy / Exp. Candy still work. |
| Evo Limit All + random evo / evo methods | **Warn** | Nothing evolves, so those randomizer rows do nothing. |
| Level cap + HARD MODE EXP | **Allow** | Designed: 60% EXP only while Level Cap is Hard (ADR 0044). |
| BST equalizer + modern/classic stats | **Allow** | Equalizer runs after the stat table pick (ADR 0029 / S49). |
| Party limit 1 + Nuzlocke | **Allow** | Solo Nuzlocke is a normal challenge. |
| Item randomizer + HMs / keys | **Allow** | Pins keep Cut/Surf/Strength and key items (ADR 0063). |
| Chaos + Balancing | **Allow** | Already exclusive in the Randomizer page (ADR 0025). |
| Random starter + monotype | **Allow** | Oak uses the monotype trio; random starter is skipped while monotype is on. |

## Adding a pair

1. Add an `MF_RULES_CONFLICT_*` id.
2. Put it in `sConflicts` (**forbids first**, then warns).
3. Implement `PairActive`.
4. Add a unit test in `test/modern_firered/mf_rules_conflicts.c`.
5. Update this table and ADR 0078 if the product call changes.
