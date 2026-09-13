# 0025 — Randomizer page: master gate, nested ME conditions, live clear/seed

- **Type:** ux
- **Status:** Accepted
- **Date:** 2026-09-13
- **Story:** S25
- **ME reference:** `MENUITEM_RANDOM_*` / `CheckConditions` / SAVE clear path in `src/tx_rac_menu.c`; `IsRandomizerActivated` in `src/tx_randomizer_and_challenges.c`
- **Expansion config:** —

## Context

S25 adds the Randomizer page as pure UI over existing `ModernRules` randomizer fields. ME keeps a UI-only master (`sel_randomizer[OFF_ON]`) until SAVE; our menu writes live via `TrySetBool`. ME also defaults `Similar`/`MapBased` to TRUE in the TX block (and forces `MapBased` on SAVE when the master is on), inverts BALANCING Off/On indices, and gates BALANCING / LEGENDARIES / CHAOS on nested conditions. `MfRules_IsRandomizerActive` already exists and deliberately ignores Similar/MapBased alone.

## Decision

1. **Page order** follows ME’s Randomizer enum: RANDOMIZER → STARTER → WILD → TRAINER → STATIC → BALANCING → LEGENDARIES → TYPE → MOVES → ABILITIES → EVOLUTIONS → EVO LINES → EFFECTIVENESS → ITEMS → CHAOS MODE → EXIT (SAVE deferred to S26). Page sits last in our Phase 3 order (after Challenges), not ME’s mid-menu slot.
2. **Master bit** uses the spare `randomizerEnabled` field (`MF_RULE_BOOL_RANDOMIZER_ENABLED`). Turning **Off** clears every `random*` remap bit (including Items — ME’s SAVE omit is treated as a bug). Turning **On** seeds `randomSimilar` / `randomMapBased` from `MF_TX_RANDOM_SIMILAR` / `MF_TX_RANDOM_MAP_BASED`.
3. **Gating** mirrors ME `CheckConditions`:
   - most remaps → `REQUIRES_RANDOMIZER` (master on)
   - LEGENDARIES → species remaps on
   - BALANCING → species remaps on and Chaos off
   - CHAOS → any chaos-eligible remap on (Items excluded, as in ME)
4. **BALANCING** uses Off=0 / On=1 → `randomSimilar` (reject ME’s On-left inverted indices; same as ADR 0022).
5. **CHAOS MODE** labels Off / Chaos; enabling Chaos clears Balancing (`randomSimilar = FALSE`), matching ME’s draw side effect.
6. **`MfRules_IsRandomizerActive`** stays “any remapping feature” (not master alone, not Similar/MapBased). Master Off clears remaps so the helper goes false. ApplyDevDefaults keeps Similar/MapBased **off** until the master is enabled so map-based RNG keys stay inert.

## Alternatives considered

- UI-only master with no save bit — rejected; live writes need durable gate state when all remaps are off.
- Treat master On alone as “activated” — rejected; wiring should require an actual remap feature.
- Keep ME Off/Random choice labels — rejected; Off/On matches the rest of the FR shell (Chaos keeps Off/Chaos).

## Consequences

- S51+ randomizer wiring can keep using `MfRules_IsRandomizerActive` / per-flag accessors; menu players must enable the master first.
- Debug Randomizer page exposes `RMaster` plus raw flags (including MapBased, which has no menu row).
- No MAP BASED menu row (ME also omits it; value is seeded with the master).
