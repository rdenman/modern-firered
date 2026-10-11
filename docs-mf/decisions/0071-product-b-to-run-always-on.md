# 0071 — B-to-Run always-on via expansion config

- **Type:** product
- **Status:** Accepted
- **Date:** 2026-10-11
- **Story:** S71
- **ME reference:** — (ME uses a separate RUN PROMPT / QUICK RUN; not this flag)
- **Expansion config:** `B_QUICK_MOVE_CURSOR_TO_RUN` default `FALSE` → `TRUE`

## Context
S71 wants B on the wild/Safari action menu to jump the cursor to Run. Expansion already implements that behind `B_QUICK_MOVE_CURSOR_TO_RUN`. S61 kept B-to-Run off Options+. S63 RUN PROMPT is L/R confirm, not this shortcut.

## Decision
Flip `B_QUICK_MOVE_CURSOR_TO_RUN` to `TRUE`. Always-on; no Options+ or rules toggle. Do not reimplement the cursor move. Trainer B (including double-battle partner cancel) stays on the existing controller branches.

## Alternatives considered
- Options+ toggle — rejected; S61 already reserved this as always-on.
- Reimplement in `mf_*` — rejected; the two upstream sites are the whole feature.

## Consequences
Next `RHH/master` merge will conflict on this `#define` if upstream keeps `FALSE`. Re-apply after taking theirs. B-to-Run is compile-time, unlike player-facing Options+ (runtime). S72-style gen pins are a similar exception to “leave expansion defaults.”
