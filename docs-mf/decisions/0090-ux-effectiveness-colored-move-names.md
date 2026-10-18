# 0090 — Effectiveness as green/red move names

- **Type:** ux
- **Status:** Accepted
- **Date:** 2026-10-17
- **Story:** —
- **ME reference:** Options+ effectiveness hint (`src/battle_controller_player.c` / `optionTypeEffective`); names, not ME's extra STAB window
- **Expansion config:** `B_SHOW_EFFECTIVENESS` `SHOW_EFFECTIVENESS_SEEN`

## Context
S62 (ADR 0069) used expansion's PP-slot matchup symbols (circles, triangles, star, X) plus a `TYPE/xxx+` STAB mark. The symbols are hard to read on the FireRed fight menu, and STAB is obvious from the type line.

## Decision
- With **EFFECTIVENESS** On, color all four fight-menu **move names** using **pre-June 2026 ME** (`d7b8a0b`, before [e61a565](https://github.com/resetes12/pokeemerald/commit/e61a565) swapped to ↑/↓/×). That build printed `TYPE/FIRE` on windows 24/25/26: SE fg `#7BCD73` / shadow `#ACE6AC`; resisted (and we use the same for immune) fg `#FF0000` / shadow `#FFA462`. Neutral, status, and unseen stay gray. Current ME’s orange resisted color is not used.
- Always show **PP**. Do not replace the PP label with icons.
- No STAB mark on the type line or anywhere else.
- Doubles still pick the stronger foe matchup on the fight menu; while choosing a target, names recolor for that battler.
- Options+ item is **EFFECTIVENESS** (was STAB/EFFECTIVE). Default On and the SEEN/type-icon mapping from ADR 0069 stay.

## Alternatives considered
- Keep expansion symbols — rejected; unreadable vs name color.
- Color only the highlighted move — rejected; coloring all four is the point of a glance.
- Separate STAB color or `+` — rejected; not needed.

## Consequences
- Upstream: `src/battle_controller_player.c` (name coloring, no PP icons), `src/battle_message.c` (`B_WIN_PP` always uses the PP palette), `graphics/battle_interface/text.pal` (ME’s fight-menu colors at indices 5–9). Re-apply after `RHH/master` if they change fight-menu text.
- ADR 0069's SEEN gate and HP-bar type icons still apply; its PP-icon and `TYPE/+` display is superseded here.
