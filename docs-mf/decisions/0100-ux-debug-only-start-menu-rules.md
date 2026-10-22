# 0100 — Start-menu Rules is debug-only

- **Type:** ux
- **Status:** Accepted
- **Date:** 2026-10-21
- **Story:** — (revises S26 / ADR 0026)
- **ME reference:** —
- **Expansion config:** `DISABLED_ON_RELEASE`

## Context

S26 / ADR 0026 put a read-only **Rules** row on the start menu (between SAVE and OPTION) so players could re-read the active set mid-run. That also ships in `make firered RELEASE=1` production ROMs. The start menu is already crowded, and the viewer is a debugging aid more than a player-facing control.

## Decision

**Rules** appears on the start menu only in non-release builds (`#if MF_RULES_ENGINE && DISABLED_ON_RELEASE`). Release ROMs omit the row. The read-only viewer (`CB2_InitMfRulesViewer`) stays reachable from overworld debug → **Modern FireRed… → Rules viewer…** on dev builds.

## Alternatives considered

- Keep the player-facing start-menu row (ADR 0026) — rejected; it is not wanted in production.
- Compile the entire viewer out of release — unnecessary; debug already strips the overworld menu, and the start-menu callback is gone.

## Consequences

- `src/start_menu.c` is still the only upstream file for this hook.
- Players in a release ROM have no in-game rules recap. Revisit if that becomes a support problem.
- ADR 0026’s SAVE confirm/lock behavior is unchanged.
