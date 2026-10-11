# 0072 — Catch-to-box, no Gen 7 party-swap prompt

- **Type:** product
- **Status:** Accepted
- **Date:** 2026-10-11
- **Story:** S72
- **ME reference:** —
- **Expansion config:** `B_CATCH_SWAP_INTO_PARTY` default `GEN_LATEST` → `GEN_3`; `B_CATCH_SWAP_CHECK_HMS` left at `TRUE`

## Context
Expansion’s Gen 7+ catch flow asks whether to swap a party member for the new catch when the party is full. FireRed/Gen 3 sent the catch to the PC with no prompt. S07 kept a `GEN_LATEST` battle baseline (ADR 0007) and said later undesirable Gen 9/7 behaviors get a targeted pin, not a global downgrade.

`Cmd_givecaughtmon` already treats “full” as `CalculatePlayerPartyCount() >= MfGetMaxPartySize()` (S40).

## Decision
Pin `B_CATCH_SWAP_INTO_PARTY` to `GEN_3` so the swap yes/no never appears. Always-on; no Options+ or rules toggle. Do not reimplement give-caught-mon. Leave `B_CATCH_SWAP_CHECK_HMS` alone (dead while the swap prompt is off).

## Alternatives considered
- Keep `GEN_LATEST` swap UX — rejected; the story is explicitly Gen 3 catch-to-box.
- Runtime Options+ toggle — rejected; story says always-on.
- Also pin `B_CATCH_SWAP_CHECK_HMS` — rejected; unused unless swap is on.

## Consequences
Next `RHH/master` merge will conflict on this `#define`. Re-apply after taking theirs. S40 party-limit runs still send overflow catches to the Box because fullness already uses `MfGetMaxPartySize()`.
