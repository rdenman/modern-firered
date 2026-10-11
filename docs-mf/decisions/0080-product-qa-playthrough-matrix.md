# 0080 — Playthrough QA matrix configs and Surge checkpoint

- **Type:** product
- **Status:** Accepted
- **Date:** 2026-10-11
- **Story:** S66
- **ME reference:** — (ME has no documented playthrough matrix)
- **Expansion config:** —

## Context

S66 needs a repeatable set of runs, not ad-hoc toggle clicking. The story names five configurations and a floor of Brock → Surge (Elite Four ideal). ADR 0078 also asks Nuzlocke + no Centers and monotype + randomizer to stay in the play set. Chaos + Nuzlocke is a warn combo, not a required row. Full championships on every row would block branding (S67) for weeks.

## Decision

1. **Required five:** Classic, Modern default, full randomizer, Hardcore Nuzlocke, Fire monotype. Fillers live in `MfQa_FillConfig` so tests and debug apply the same bits as the written log.
2. **Full randomizer** means every remap category + Similar + map-based + legendaries in pool. **Chaos stays off** so first encounters and marts stay playable; chaos remains a warn combo with Nuzlocke, not this row.
3. **Monotype is Fire** on Modern (Fairy Type On). Classic + Fairy monotype is forbidden (ADR 0078) and is not a matrix row.
4. **Required in-game checkpoint is Surge.** Elite Four is ideal, not gating. Celio tickets are always-on: one S73 QA-warp pass covers C6 for the ROM, not per config.
5. **ADR 0078 extras** (Normal Nuzlocke + no Centers; Fire monotype + wild/trainer random) are first-class rows in the same log, still Surge.
6. **S66 is complete** when the matrix is documented, configs have automated no-FORBID (and expected WARN) coverage, debug apply exists, and the gym table is the living log. S68 still expects a human short playthrough before a release patch.

## Alternatives considered

- **Chaos on in “full randomizer”** — rejected; salt makes Nuzlocke-style routing and mart checks noisy; ADR 0063 already covers chaos separately.
- **Require Elite Four on all five before S67** — rejected; Surge proves gym, HM, and early-route composition. Championship remains ideal in the log.
- **Skip extra 0078 rows** — rejected; those were explicitly left playable for this story.

## Consequences

- Debug **QA matrix…** overwrites save rules and locks them (inspector can unlock).
- Follow-up bugs from gym walks go in new stories, not silent scope in S66.
- Log: [`docs-mf/QA_MATRIX.md`](../QA_MATRIX.md).
