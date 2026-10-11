# 0081 — Version stamp in Options+; credits there and in README

- **Type:** ux
- **Status:** Accepted
- **Date:** 2026-10-11
- **Story:** S67
- **ME reference:** — (ME title art; no matching Options credits row)
- **Expansion config:** `MF_RELEASE_VERSION_*` in `include/config/modern_firered.h`; `EXPANSION_VERSION_*` in `include/constants/expansion.h`

## Context

S67 needs a player-visible Modern FireRed version and required RHH attribution (with expansion version) plus Modern Emerald as inspiration. The FRLG title uses all four backgrounds for logo, Charizard, copyright, and border; painting a text window there fights those tilemaps. Story allows title **or** Options. `MF_VERSION` is the config-header contract (ADR 0004), not a player stamp. S68 will cut a real 1.0.0 patch.

## Decision

1. **Player stamp** is `MF_RELEASE_VERSION_*`, currently **0.1.0**. Distinct from `MF_VERSION` and `MF_RULES_VERSION`. Bump to 1.0.0 in S68.
2. **In-game:** Options+ top bar right-aligns `v0.1.0` on every page. Battle Options gains a **CREDITS** row whose description is “Based off RHH expansion *x.y.z*. Inspired by Modern Emerald.” Expansion digits come from `EXPANSION_VERSION_*` so merges stay accurate.
3. **Title screen** stays stock FRLG (Charizard / copyright). No logo redraw, no story/map edits (ADR 0076 already skipped the RHH splash).
4. **README** describes Modern FireRed first and keeps the required `Based off RHH's pokeemerald-expansion 1.17.2 …` block plus a CREDITS.md pointer.

## Alternatives considered

- **Paint version on the FRLG title BGs** — rejected; all four BGs are in use; a window would steal tiles from the logo or Charizard.
- **Reuse `MF_VERSION` as the player stamp** — rejected; that counter is the config ABI, not a release.
- **Ship as 1.0.0 now** — rejected; S68 is the release patch.

## Consequences

- In-game credits track `include/constants/expansion.h` automatically; README’s copy-paste block is updated when that header moves.
- Debug **Options menu…** is enough to QA the stamp without a New Game.
