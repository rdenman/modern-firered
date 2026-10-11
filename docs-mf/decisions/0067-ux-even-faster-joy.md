# 0067 — Even Faster Joy: Off / Fast / On on FireRed Centers

- **Type:** ux
- **Status:** Accepted
- **Date:** 2026-10-10
- **Story:** S60
- **ME reference:** `src/options_plus_menu.c` EVEN FASTER JOY; `data/scripts/pkmn_center_nurse.inc` `FLAG_EVEN_FASTER_JOY`
- **Expansion config:** `MF_OPTIONS_PLUS`

## Context

ME’s option is On/Off. On skips the yes/no and the machine (`HealPlayerParty` + `SE_M_MORNING_SUN` + bow). Off still says “heals you fast, but with the usual animation.” FireRed’s nurse script is a separate `_Frlg` file (prompt, `VAR_LAST_TALKED`, no Pokerus/Gold Card). S58 already reserved a 2-bit `evenFasterJoy` field. The story asked for a middle “fast but animated” setting.

## Decision

1. **Three values** on the S58 bitfield: **Off** (vanilla FR), **Fast** (yes/no + machine, ¼ glow delays, do not wait for `MUS_HEAL`, skip “few seconds” / “thank you for waiting”), **On** (ME skip: no prompt, no machine, heal + short SE + bow). After On, the player **spins to face down** (Center exit). Default Off.
2. Hook **`data/scripts/pkmn_center_nurse_frlg.inc` only**. Do not copy ME’s flag; scripts call `MfOptions_ScriptGetEvenFasterJoy` (also logs `=== MF joy heal ===`). Challenge block (S46) still wins.
3. Speed the glow **only when the Center heal SE is playing**, so Hall of Fame recording stays vanilla. Whiteout uses `TakeAndHeal`: On skips the machine there too.

## Alternatives considered

- **On/Off only, matching ME’s menu** — rejected; the story wants the animated middle, and the 2-bit field already allowed it.
- **ME `FLAG_EVEN_FASTER_JOY`** — rejected; Options+ already lives in `SaveBlock3.mfOptions`.
- **Skip yes/no on Fast** — rejected; Fast should still look like a Center visit.

## Consequences

- Menu labels are Off / Fast / On (Off-first, unlike ME putting On on the left).
- Spa / Tower tile heals are unchanged (already a fade, not Joy).
- `src/field_effect.c` calls `mf_` delay helpers (merge-touch on glow timers).
