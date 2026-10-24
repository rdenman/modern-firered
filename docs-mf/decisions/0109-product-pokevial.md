# 0109 — Poké Vial: Features toggle, badge charges, Nuzlocke, no-Center refill

- **Type:** product
- **Status:** Accepted
- **Date:** 2026-10-24
- **Story:** S76 (research: [`POKEVIAL.md`](../../POKEVIAL.md))
- **ME reference:** — (Inclement Emerald Key Item, not Modern Emerald)
- **Expansion config:** —

## Context

Inclement Emerald’s Poké Vial is an always-on Key Item: field-only full-party heal, 1–3 charges, refill at Nurse Joy and on whiteout. Modern FireRed wants that QoL on stock Kanto, behind a toggle, without IE’s Chansey/Audino upgrade quests. Open questions were page/default, charge curve, Nuzlocke revives, no-Center refill, whiteout, grant, wasted uses, and dose UI.

Research and IE code map: [`POKEVIAL.md`](../../POKEVIAL.md). No-Center nurse currently early-outs with a refuse and never talks further (ADR 0049).

## Decision

1. **Toggle.** Features page `Poké Vial` Off/On. Default **Off** for Classic, Modern, and Custom (and for saves that predate the bit). One `ModernRules` padding bit (`pokeVial`). Locked after SAVE like other Features rows (ADR 0015); debug inspector can still flip it.

2. **Charge cap.** Remaining charges are saved. Max is derived from **gym badge count**, not IE’s Route 111/133 quests:

   `max(1, ceil(badges / 3))`, capped at 3.

   | Badges | Max charges |
   | ------ | ----------- |
   | 0–3 | 1 |
   | 4–6 | 2 |
   | 7–8 | 3 |

   Integer form: `max = (badges + 2) / 3`; if `max < 1` then `1`; if `max > 3` then `3`.

   Refill (Joy or whiteout) sets remaining = that max. Earning a badge does not add a charge in the field; the next refill restocks to the new cap.

3. **Nuzlocke.** The vial stays legal when Nuzlocke ≠ Off (including Easy / Hardcore). Use restores HP / PP / status on **living** party mons only (`HP > 0`). Fainted mons are skipped — no field revive. Off-Nuzlocke uses the full IE heal (including fainted).

4. **No Poké Centers.** Joy still talks. If the vial is On, she **refills charges**, then says she cannot heal the party because of the run’s settings. She does not call `HealPlayerParty`. If the vial is Off, keep today’s refuse-only line (ADR 0049).

5. **Whiteout.** Copy IE: `DoWhiteOut` refills remaining to the current badge-derived max. Party heal / Nuzlocke whiteout handling is unchanged.

6. **Grant.** If the toggle is On at New Game (after rules SAVE / lock), the vial is already in the Key Items pocket with remaining = current max (1 at 0 badges). No Oak/Joy give-cutscene and no extra flavor text. Off = never given. Mid-run Off (debug): Use refuses; do not have to strip the item.

7. **When a use counts.** Do **not** spend a charge if nobody would actually be restored. A party mon “needs the vial” if it is not an egg and any of: HP < max, status ≠ none, or any move PP < max PP (with bonuses).
   - **Nuzlocke ≠ Off:** ignore fainted mons (`HP == 0`) for both the check and the heal. If every living mon is already full HP / PP / status, refuse and keep the charge (a party of only fainted mons also refuses).
   - **Nuzlocke Off:** fainted mons count (a revive is a real restore). If at least one mon would change, spend one charge and apply the heal.

8. **Dose UI.** Bag description only — no extra icons. Append ` (remaining/max)` to the item description, e.g. `(1/2)` when one of two charges is left. `GetItemDescription` already has a per-item hook (classic Sitrus).

Field-only (no in-battle use). Key Item, not in the item-randomizer pool. Name: **Poké Vial** (IE spelling) unless a later call changes it.

## Alternatives considered

- **Modern default On** — rejected; Features QoL stays opt-in like regional forms, and old saves must not gain the item.
- **Fixed 1 charge, or IE’s two Hoenn quests** — rejected; quests are new map content. Badge `ceil(n/3)` gives the same 1→2→3 curve on stock Kanto.
- **`ceil((badges+2)/3)` without the 0-badge clamp, or 1 + floor(badges/3)** — rejected; those disagree with the requested table (0–3 → 1, 4–6 → 2, 7+ → 3).
- **Force vial Off under Nuzlocke** — rejected; the option should still work as a between-fights heal.
- **IE `HealPlayerParty` under Nuzlocke** — rejected; that revives and breaks the run type.
- **No-Center: no Joy interaction / whiteout-only refill** — rejected; walking in for a refill without a free party heal is the intended compromise.
- **Skip whiteout refill** — rejected; matches IE and avoids a 0-charge trap after a wipe.
- **First-Center / Oak grant + explain box** — rejected; no extra story. Bag-at-start if the Features row is On.
- **IE always spend a charge** — rejected; wasted uses feel bad. One mon needing only PP is enough to spend.
- **PSF percent bag icons** — rejected; `(current/max)` on the description is enough.

## Consequences

- ADR 0049’s nurse blocked script grows a vial-refill branch; the no-Center **party** heal ban stays.
- Do not call `HealPlayerParty` from the vial (this ROM may also heal Boxes). Party-only `HealPokemon`, and skip `HP == 0` when Nuzlocke faint rules are active. Unit-test the needs-heal predicate (healthy / PP-only / fainted-only Nuzlocke / fainted Off-Nuzlocke / eggs).
- Grant after rules lock (`AddBagItem` + remaining = max), same ordering as other new-game gifts (S19). Joy scripts do not give the item; they only refill (and, under no-Center, refill without healing).
- Features menu (ADR 0021) gains a row. `MF_RULES_VERSION` 5 names `pokeVial`; migrate forces Off so v1 padding garbage cannot enable it.
- Hardcore / `noItemPlayer`: field vial stays legal (battle item ban only).
- Heal pads (Tower 5F, Ember Spa) do **not** refill.
- Empty vs already-healthy: two refuse messages, no charge spent.
