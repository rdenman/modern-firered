# 0011 — Battle & overworld speed baseline

- **Type:** product
- **Status:** Accepted
- **Date:** 2026-09-12
- **Story:** S11
- **ME reference:** Options+ `FAST INTRO` / `BATTLE SPEED` / `FAST BATTLES` (`options_plus_menu.c`); Gamemode `SURVIVE POISON` (`tx_rac_menu.c`)
- **Expansion config:** `B_FAST_*`, `B_WAIT_TIME_MULTIPLIER` in `include/config/battle.h`; `TEXT_SPEED_*` in `include/config/text.h`; `OW_RUNNING_INDOORS`, `OW_POISON_DAMAGE` in `include/config/overworld.h`

## Context

Phase 1 should make the ROM feel fast before any Options+ or rules menu exists. Expansion already ships several battle-pace booleans at `TRUE` and leaves wait-time / slide / overworld text at vanilla-ish defaults. S61 will later expose intro/anim speed as player options; S34 will gate poison as ME’s SURVIVE POISON. S11 must choose always-on compile-time defaults that are clearly faster than vanilla without stealing those later toggles.

## Decision

- **Battle intro / bars:** keep `B_FAST_INTRO_PKMN_TEXT`, `B_FAST_HP_DRAIN`, and `B_FAST_EXP_GROW` at `TRUE`. Set `B_FAST_INTRO_NO_SLIDE` to `TRUE` (ME Fast Intro On — skip the slide-in).
- **Battle text pauses:** set `B_WAIT_TIME_MULTIPLIER` to `8` (vanilla `16`). Roughly half the pause length between battle messages; still readable, clearly snappier. S61 may lower this further at runtime for Anim Speed / Fast Battles.
- **Overworld / Options text:** leave `TEXT_SPEED_INSTANT` at `FALSE` and leave Slow/Mid/Fast modifiers at `1`. Instant would override the player Options menu entirely; Phase 10 owns Even Faster Joy and further text QoL.
- **Indoor running:** leave `OW_RUNNING_INDOORS` at `GEN_LATEST` (already Gen4+ indoor run). No code change required; confirmed `bike.c` only blocks indoor run when the define equals `GEN_3`.
- **Poison:** leave `OW_POISON_DAMAGE` at `GEN_LATEST` (Gen5+: no overworld poison damage at all). Document as the Phase 1 baseline and a future **Gamemode SURVIVE POISON** runtime gate in **S34**. ME’s On = survive at 1 HP (closer to Gen4); Off = can faint (Gen3). Expansion’s Gen5+ “no damage” is stricter than ME On; S34 should decide how to map the three expansion gens onto ME’s two-way toggle.

## Alternatives considered

- `B_WAIT_TIME_MULTIPLIER` `4` — rejected as always-on; too aggressive before Options+ can dial it back, and more likely to feel “broken” on first boot.
- `TEXT_SPEED_INSTANT` `TRUE` — rejected; removes Options text-speed agency and is hard to undo mid-run without a flag.
- Downgrade `OW_POISON_DAMAGE` to `GEN_3` for FRLG authenticity — rejected; conflicts with modern baseline and ME Modern’s survive-poison intent.
- Force `OW_RUNNING_INDOORS` to an explicit `GEN_4` — unnecessary; `GEN_LATEST` already enables the Gen4+ path.

## Consequences

- Upstream edits to `battle.h` (`B_FAST_INTRO_NO_SLIDE`, `B_WAIT_TIME_MULTIPLIER`) are merge surfaces; re-apply after `RHH/master` if defaults flip.
- S61 must treat these compile-time values as the “on / fast” baseline when wiring runtime Options+.
- S34 poison gate should not assume Gen5+ “no damage” is identical to ME SURVIVE POISON On.
- Config lock: `test/modern_firered/mf_speed_baseline.c`. Manual: [`docs-mf/manual-qa-s11-battle-ow-speed.md`](../manual-qa-s11-battle-ow-speed.md).
