# 0065 — Autorun inverts B on foot and Surf, not on bike

- **Type:** product
- **Status:** Accepted
- **Date:** 2026-10-09
- **Story:** S59
- **ME reference:** `src/field_player_avatar.c` (`optionsautoRun` / `optionsAutorunSurf`); Options+ “Run without pressing B” / “Surf faster without pressing B”
- **Expansion config:** `MF_OPTIONS_PLUS`

## Context

S58 already stores `autorun` and `autorunSurf` on SaveBlock3. S59 has to invert run/walk without breaking indoor/metatile run bans, running shoes (`FLAG_SYS_B_DASH`), Acro bike wheelies (B), or vanilla Surf speed. ME’s movement C treats the save bits backwards from its own option copy (autorun off walks on B). Kanto has no Dive, so ME’s dive autorun stays omitted (ADR 0064).

## Decision

1. **On foot:** `MfOptions_WantsToRun` inverts B when autorun is on (run by default, hold B to walk). Shoes, `IsRunningDisallowed`, follower-through-door, and ORAS dowsing still gate the dash. Indoor running stays the S11 `OW_RUNNING_INDOORS` baseline.
2. **Surf:** vanilla is already run-speed (`PlayerWalkFast`). Extra speed is `PlayerWalkFaster`. Autorun Surf **off**: hold B to boost. **On:** boost by default, hold B for usual Surf speed. No Dive hook.
3. **Bike:** no invert. Mach speed is its own ramp; Acro still uses B for wheelies/bunny hops.
4. Do **not** copy ME’s inverted land/Surf C; match ME’s labels and this story’s acceptance.

## Alternatives considered

- **Copy ME’s `IsPlayerTryingToRun` literally** — rejected; it implements the opposite of “run without pressing B.”
- **Slow vanilla Surf to walk speed so autorun Surf can restore run speed** — rejected; would nerf every Surf until the option is on.
- **Invert B on Mach/Acro** — rejected; fights wheelie input.

## Consequences

- `src/field_player_avatar.c` is a one-line call site (`MfOptions_WantsToRun` / `MfOptions_WantsSurfBoost`). Bike code is untouched.
- With Options+ compiled in, holding B while Surfing is faster even if Autorun Surf is off (ME Off copy). `MF_OPTIONS_PLUS=0` keeps vanilla Surf (no boost).
- Follow-up: S60 Joy, S61 battle/intro speed; no new save fields.
