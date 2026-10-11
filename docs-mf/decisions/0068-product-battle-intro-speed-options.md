# 0068 — Battle and intro speed Options+

- **Type:** product
- **Status:** Accepted
- **Date:** 2026-10-11
- **Story:** S61
- **ME reference:** Options+ `FAST BATTLES` / `ANIM SPEED` / `FAST INTRO` / `SKIP INTRO` / `BALL PROMPT` (`src/options_plus_menu.c`)
- **Expansion config:** `B_WAIT_TIME_MULTIPLIER` 8, `B_FAST_INTRO_NO_SLIDE` TRUE, `B_LAST_USED_BALL` TRUE (`include/config/battle.h`)

## Context

S11 baked a faster battle baseline (no-slide intro, wait ×8, last-used Ball). S61 has to expose those knobs as mid-run Options+ without turning the compile-time `#define`s into player toggles. ME stores inverted On=0 flags on SaveBlock2; we already persist extras on `SaveBlock3.mfOptions` (ADR 0064). Battle scripts bake `B_WAIT_TIME_*` at assemble time, so a runtime multiplier cannot be a `#define`.

## Decision

1. **FAST INTRO** (default **On**) gates `MfOptions_SkipBattleIntroSlide()` at the existing `B_FAST_INTRO_NO_SLIDE` sites. Off restores the slide-in. `B_FAST_INTRO_PKMN_TEXT` stays always-on from S11.
2. **ANIM SPEED** 1x keeps baked waits (×8). 2x halves `waitmessage` / `pause` via `MfOptions_ScaleBattleWait`. We did not double sprite anims, HP, or EXP bars (ME does); those already use S11 `B_FAST_HP_DRAIN` / `B_FAST_EXP_GROW`.
3. **FAST BATTLES** On sets those waits to 0 (ME skip-all-delays). Off does not add ME’s A/B mash-to-skip; mash skip fights the “read this line” pause.
4. **SKIP INTRO** On, after copyright loads a save, jumps to the main menu (`CB2_InitMainMenu`) — Game Freak movie and title (including Quickstart Select) are skipped on boot and soft-reset. Requires a saved option; empty/corrupt saves still play the intro.
5. **BALL PROMPT** (default **On**) gates expansion’s last-used Ball UI/throw (`CanThrowLastUsedBall`). `B_LAST_USED_BALL` stays `TRUE`.
6. **v1 → v2:** unrecognized extras still reset; version 1 saves turn FAST INTRO and BALL PROMPT on without wiping autorun/Joy.

## Alternatives considered

- **Rewrite `B_WAIT_TIME_*` as functions** — rejected; battle scripts need immediates.
- **ME A/B skip on Fast Battles Off** — rejected; too easy to skip send-out text by accident.
- **Default FAST INTRO Off** — rejected; would undo S11 for new and migrated saves (ADR 0011).
- **Skip only the GF movie, keep the title** — rejected; ME and the story skip the title on soft-reset.

## Consequences

- Upstream call sites: `intro.c` (boot skip), `battle_intro.c`, `battle_main.c`, `battle_controllers.c`, `battle_script_commands.c`, `battle_interface.c`. Re-apply if `RHH/master` changes those checks.
- Compile-time `B_FAST_INTRO_NO_SLIDE` remains TRUE for S11’s config lock; runtime Off still shows the slide.
- Soft-reset skip only works after the player **saves** with SKIP INTRO On. Title-screen Quickstart is unavailable on that path — Continue from the main menu instead.
