# Modern FireRed — decision records

Short ADRs for non-trivial tech / product / UX choices. Upstream’s `docs/` is their mdbook; keep ours under `docs-mf/` to avoid merge collisions.

## Template

Create `NNNN-<type>-<slug>.md` with the next zero-padded `NNNN`. Types: `tech` | `product` | `ux`.

```markdown
# NNNN — <Title>

- **Type:** tech | product | ux
- **Status:** Accepted
- **Date:** YYYY-MM-DD
- **Story:** S## (or —)
- **ME reference:** <ME file or option> (or —)
- **Expansion config:** <macro + default> (or —)

## Context
Why a decision was needed; the forces at play.

## Decision
What was decided, stated plainly.

## Alternatives considered
- <option> — why not chosen.

## Consequences
Trade-offs, follow-ups, upstream-merge implications, and any affected stories or future work.
```

Append a row to the index below when you add a record.

## Index

| ID | Type | Title | Story | Date |
| -- | ---- | ----- | ----- | ---- |
| 0001 | tech | Track `.python-version` and document the FireRed toolchain in `BUILDING.md` | S01 | 2026-09-11 |
| 0002 | tech | Slim CI to FireRed + Emerald test suite | S02 | 2026-09-11 |
| 0003 | tech | Runtime rules layer over compile-time expansion configs | S03 | 2026-09-11 |
| 0004 | tech | MF_ config namespace via global.h | S04 | 2026-09-11 |
| 0005 | tech | MF debug submenu via public DebugMenuOption | S05 | 2026-09-11 |
| 0006 | tech | Emerald TESTELF for MF pure helpers; FR wiring stays manual | S06 | 2026-09-11 |
| 0007 | tech | Keep GEN_LATEST battle baseline (no Kanto downgrade) | S07 | 2026-09-11 |
