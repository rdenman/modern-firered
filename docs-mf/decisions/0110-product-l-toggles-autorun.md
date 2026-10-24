# 0110 — Overworld L toggles Autorun; drop L=A

- **Type:** product
- **Status:** Accepted
- **Date:** 2026-10-24
- **Story:** —
- **ME reference:** —
- **Expansion config:** `MF_OPTIONS_PLUS`

## Context

Autorun already lives on Options+ (ADR 0065): run by default, hold B to walk. Opening Options to flip it is slow when you want a stretch of walking. L is free on the FireRed overworld (R is DexNav). Vanilla Button Mode still offered L=A, which would steal that tap.

## Decision

1. **L on the overworld** flips the existing `MfOptions.autorun` bit and plays `SE_SELECT`. Surf autorun and bikes are unchanged. Hold B still inverts while the option is on.
2. **No extra Options+ row.** The QoL Autorun setting and the hotkey are the same bit.
3. **Remove L=A.** Options+ Button Mode is Normal or LR. Saved `OPTIONS_BUTTON_MODE_L_EQUALS_A` clamps to Normal. The `main.c` L→A remap is compiled out when Options+ is on. Run Prompt L no longer cares about L=A (ADR 0070).

## Alternatives considered

- Bind R, or both shoulders — rejected; R starts DexNav and is last-used Ball in battle.
- A QoL “L toggles Autorun” Off/On — rejected; another menu trip is what the hotkey is for.
- Keep L=A and skip the toggle when that mode is set — rejected; the product does not need L=A.

## Consequences

Upstream: one-line call in `field_control_avatar.c`; L=A remap gated in `main.c`. Dead L=A branches in battle/PC/etc. stay so the next RHH merge stays small.
