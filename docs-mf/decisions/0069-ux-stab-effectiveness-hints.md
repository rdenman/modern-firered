# 0069 — STAB and effectiveness Options+ hints

- **Type:** ux
- **Status:** Accepted
- **Date:** 2026-10-11
- **Story:** S62
- **ME reference:** Options+ `STAB/EFFECTIVE` (`src/options_plus_menu.c`); STAB icon on `B_WIN_STAB`
- **Expansion config:** `B_SHOW_TYPES` `SHOW_TYPES_NEVER`; `B_SHOW_EFFECTIVENESS` `SHOW_EFFECTIVENESS_SEEN`

## Context

Expansion already replaces PP with a matchup icon when `B_SHOW_EFFECTIVENESS` is not NEVER (SEEN by default) and can draw HP-bar type icons via `B_SHOW_TYPES` (NEVER). There is no STAB flag. ME’s single On/Off option shows both STAB and effectiveness. S56/S27 change types and the chart at runtime, so the hint path must use the same live types and table as damage.

## Decision

1. **STAB/EFFECTIVE** (default **On**) is `MfOptions.typeHints`. Off hides effectiveness and type icons. On shows expansion’s **SEEN** effectiveness (unseen species stay blank) and maps type icons to **SHOW_TYPES_SEEN**.
2. **STAB** is a `+` after `TYPE/` on the move menu when the displayed move type matches `gBattleMons` types (already Fairy/modern/TYPE-random). Status moves never get `+`. No extra window (ME’s `B_WIN_STAB` would fight FR’s layout).
3. Effectiveness still goes through `CalcTypeEffectivenessMultiplier` → `MfGetTypeEffectivenessTable()` (S28/S56). Compile `B_SHOW_EFFECTIVENESS` stays SEEN; compile `B_SHOW_TYPES` stays NEVER so upstream default is unchanged when Options+ is off.
4. **v2 → v3** turns type hints On so existing saves keep the SEEN matchup icons they already had.

## Alternatives considered

- **Dedicated STAB window like ME** — rejected; FR move windows have no spare slot.
- **ALWAYS show foe types/matchups** — rejected; spoils unseen wilds; SEEN matches expansion’s effectiveness default.
- **Default Off** — rejected; would hide SEEN matchups that expansion already shows.

## Consequences

- Upstream: `src/battle_controller_player.c` (effectiveness gates + TYPE/+), `src/type_icons.c` (show-types mode). Re-apply if `RHH/master` adds more `B_SHOW_EFFECTIVENESS` sites.
- Doubles still pick the stronger foe matchup (upstream `CheckTargetTypeEffectiveness`).
