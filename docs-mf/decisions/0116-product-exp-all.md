# 0116 — Exp. All: Features toggle, Key Item, held Exp. Share stays

- **Type:** product
- **Status:** Accepted
- **Date:** 2026-11-02
- **Story:** —
- **ME reference:** —
- **Expansion config:** `I_EXP_SHARE_ITEM` / `I_EXP_SHARE_FLAG` stay `GEN_5` / `0` (ADR 0010)

## Context

Players wanted a party-wide Exp. All they can turn on or off during a run, chosen at New Game, without replacing FireRed’s held Exp. Share (Oak’s Aide, 50 species). Expansion’s Gen 6 Exp. Share would convert `ITEM_EXP_SHARE` into a Key Item toggle and break that Aide reward. `ITEM_EXP_ALL` was only a Gen I alias for the held Share.

## Decision

1. **Features `Exp. All` Off/On.** Default **Off** for Classic, Modern, Custom, Phase 1 defaults, and older saves. One `ModernRules` padding bit (`expAll`). Locked after SAVE like other Features rows (ADR 0015); debug inspector can still flip it.

2. **Separate Key Item.** `ITEM_EXP_ALL` is a real item (id 875), not an alias of `ITEM_EXP_SHARE`. Key Items pocket, field-use toggle, Exp. Share icon. No hold effect.

3. **Grant.** If the toggle is On at New Game (after rules SAVE / lock), the item is already in Key Items and the effect starts **on**. Off = never given. Mid-run Off (debug): Use refuses; do not strip the item.

4. **Mid-run toggle.** Using the item sets/clears `VAR_EXP_ALL_ON` (same save-var pattern as Poké Vial charges). Party-wide EXP is on only when **both** the Features rule and that var are set. Held Exp. Share still works independently (one mon holds it). A VAR is used instead of `I_EXP_SHARE_FLAG` so FireRed and the Emerald TESTELF share one id, and so held Exp. Share stays Gen 5.

5. **Do not flip `I_EXP_SHARE_ITEM`.** Aide script, item data, and tests keep Gen 5 held Exp. Share. Battle code reuses `IsGen6ExpShareEnabled()` with an `MfExpAllIsEnabled()` check so we do not duplicate EXP-split logic.

## Alternatives considered

- **Gen 6 Exp. Share via `I_EXP_SHARE_ITEM`** — rejected; turns the Aide reward into a Key Item and removes held-Share behavior.
- **Always-on party EXP with no item** — rejected; the player asked for a bag toggle throughout the run.
- **Give held Exp. Share at New Game** — rejected; that is Exp. Share, not Exp. All, and would skip the Aide reward’s meaning.

## Consequences

- `MF_RULES_VERSION` 6 names `expAll`; migrate forces Off so leftover Features padding cannot enable it.
- Existing saves and default presets do not start with the item.
- Oak’s Aide 50-species Exp. Share is unchanged.
- Features menu (ADR 0021) gains a row. Hardcore / `noItemPlayer`: field toggle stays legal (battle item ban only).
