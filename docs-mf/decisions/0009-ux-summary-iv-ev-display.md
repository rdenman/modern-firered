# 0009 — Summary screen IV/EV: always-on, raw values, tileset labels

- **Type:** ux
- **Status:** Accepted
- **Date:** 2026-09-12
- **Story:** S09
- **ME reference:** ME summary skills page cycles Stats ↔ IVs ↔ EVs with numeric values (A-button style in ME’s forked summary); we use expansion’s equivalent cycle rather than copying ME C
- **Expansion config:** `P_SUMMARY_SCREEN_IV_EV_INFO` / `P_SUMMARY_SCREEN_IV_EV_BOX_ONLY` / `P_SUMMARY_SCREEN_IV_EV_VALUES` / `P_SUMMARY_SCREEN_IV_EV_TILESET` / `P_FLAG_SUMMARY_SCREEN_IV_EV_INFO` in `include/config/summary_screen.h`

## Context

S09 asks players to see IVs/EVs without external tools. Expansion already implements this behind compile-time knobs (all default `FALSE` except hyper-train display). Choices: party vs box-only, letter grades vs raw numbers, vanilla vs IV/EV tileset for the STATS/IVs/EVs header, and whether to gate on a flag for later Options+.

FireRed historically had a different summary UI than Emerald, but this expansion tree shares `src/pokemon_summary_screen.c` for both targets (no `FIRERED` layout fork). Enabling the Emerald-proven configs is therefore the FR path as well; layout risk is “does the shared UI look right on a FR ROM,” not a separate FR codepath.

## Decision

- Set `P_SUMMARY_SCREEN_IV_EV_INFO` to `TRUE` — always available (party and box).
- Leave `P_SUMMARY_SCREEN_IV_EV_BOX_ONLY` at `FALSE`.
- Set `P_SUMMARY_SCREEN_IV_EV_VALUES` to `TRUE` — show numeric IVs/EVs (matches ME’s useful raw readout; letter grades are too coarse for Nuzlocke/Difficulty later).
- Set `P_SUMMARY_SCREEN_IV_EV_TILESET` to `TRUE` — swap the skills-page “STATS” graphic to IVs/EVs so the mode is obvious without relying only on the A-prompt.
- Leave `P_FLAG_SUMMARY_SCREEN_IV_EV_INFO` at `0` and do not introduce an Options+ toggle yet (Phase 10 / S58+ may bind one; until then this is an always-on Phase 1 QoL baseline).
- Leave `P_SUMMARY_SCREEN_IV_ONLY` / `P_SUMMARY_SCREEN_EV_ONLY` at `FALSE` so the full Stats → IVs → EVs cycle remains.

## Alternatives considered

- Box-only (`P_SUMMARY_SCREEN_IV_EV_BOX_ONLY TRUE`) — rejected; PROJECT.md lists summary IV/EV as high-priority QoL for normal play, not a PC judge substitute.
- Letter grades (`P_SUMMARY_SCREEN_IV_EV_VALUES FALSE`) — rejected; grades hide training detail players need; raw numbers match ME’s practical display.
- Keep vanilla tileset — workable (A-prompt still says IVs/EVs/STATS) but weaker legibility; tileset swap is the expansion-supported path.
- Flag-gated from day one — deferred; Phase 1 baseline is compile-time always-on, same pattern as phys/spec and Fairy until Options+ exists.

## Consequences

- Editing upstream `summary_screen.h` defaults creates a known merge surface on `RHH/master`; re-apply or reconcile if upstream flips these macros.
- After changing `P_SUMMARY_SCREEN_IV_EV_TILESET`, a `make clean` may be required so the alternate graphics object is rebuilt (upstream comment on that macro).
- S45 (IV/EV caps under Difficulty) must keep this display honest when caps land.
- Phase 10 can switch `P_SUMMARY_SCREEN_IV_EV_INFO` off and drive visibility via `P_FLAG_SUMMARY_SCREEN_IV_EV_INFO` without redesigning the skills-page cycle.
