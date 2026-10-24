# 0111 — Bag HMs (field use without teaching)

- **Type:** product
- **Status:** Accepted
- **Date:** 2026-10-24
- **Story:** —
- **ME reference:** README HM QoL + `HMsOverwriteOptionActive()` in `src/tx_randomizer_and_challenges.c` / `ScrCmd_checkpartymove`
- **Expansion config:** —

## Context
Vanilla (and this ROM) required a party member that **knew** an HM to Cut/Surf/Fly/etc. ME’s always-on QoL lets a compatible learner use the move if the HM is in the bag and the badge is earned. A separate anti-softlock (`HMsOverwriteOptionActive`) lets the **lead** use bag HMs under Nuzlocke / Mirror / Monotype / Random Moves. We want both, as a Gamemode toggle (Classic Off / Modern On), not ME’s always-on with no menu row.

## Decision
1. **Gamemode `Bag HMs` Off/On** (`bagHms`, was `paddingGamemode`). Custom-gated, next to Reusable TMs. Classic Off, Modern On. Custom seed `MF_TX_MODE_BAG_HMS` (FALSE). Phase 1 / engine-off defaults On (Modern). No `MF_RULES_VERSION` bump; old saves keep the bit at 0 (Off).
2. **When On:** badge + HM in the bag + a non-egg that `CanLearnTeachableMove` that HM → party menu and `checkfieldmove` / Surf water prompt succeed. Teaching still works and still occupies a moveslot.
3. **Overwrite** (independent of the toggle): Nuzlocke (Easy/Normal/Hardcore) **or** Mirror **or** Monotype **or** Random Moves → party **slot 0** can use a bag HM even if it cannot learn it. Skip eggs. Not Random TMs / Items / Chaos / Themed Gyms.
4. All eight HMs (Cut, Fly, Surf, Strength, Flash, Rock Smash, Waterfall, Dive). Bag items stay teachers, not a “use from bag” key-item path. No fake flyer.

## Alternatives considered
- Always-on like ME — rejected; user wanted a start-of-run toggle with Classic authenticity.
- Features page / mid-run edit — rejected; this is a Gamemode convenience like Reusable TMs.
- Whole-party overwrite — rejected; ME is lead-only.
- Skip the challenge overwrite — rejected; it is the anti-softlock for Nuzlocke / monotype / random moves.

## Consequences
`ScrCmd_checkfieldmove`, `PartyHasMonWithSurf`, and party-menu field actions call `MfFieldHm_*`. Next `RHH/master` merge will conflict on those three sites. Compatibility uses the run’s current teachable table (classic vs modern movepool). Players can still forget HMs (ADR 0106) and reteach from the bag.
