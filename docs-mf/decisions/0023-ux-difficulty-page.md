# 0023 — Difficulty page: ME order, Yes/No item bans, clear PLAYER IVs labels

- **Type:** ux
- **Status:** Accepted
- **Date:** 2026-09-13
- **Story:** S23
- **ME reference:** `MENUITEM_DIFFICULTY_*` / `sText_*` / `DrawChoices_*` in `src/tx_rac_menu.c`
- **Expansion config:** —

## Context

S23 adds the Difficulty page as pure UI over existing `ModernRules` difficulty fields. ME’s live enum order differs from the story bullet list, several rows use Yes/No (not Off/On) because the save bit means “ban”, and PLAYER IVs draws Yes/No/No(HP) while descriptions read Off/On/HP. Pokécenter is in ME’s difficulty data enums but lives on the Challenges page UI.

## Decision

1. **Page order** follows ME’s `MENUITEM_DIFFICULTY_*` enum: LOCK DIFFICULTY → PARTY LIMIT → LEVEL CAP → EXP. MULTIPLIER → HARD MODE EXP. → CATCH RATE → PLAYER ITEMS → TRAINER ITEMS → PLAYER IVs → TRAINER IVs → PLAYER EVs → TRAINER EVs → LESS ESCAPES → ESC. ROPE / DIG → NEXT.
2. **Omit Pokécenter** here; it remains Challenges (S24) via `pokeCenterLimit` / `MF_RULE_VAL_POKECENTER_LIMIT`.
3. **PLAYER/TRAINER ITEMS** and **ESC. ROPE / DIG** use Yes/No labels mapped 0/1 onto `noItem*` / `escapeRopeDig` (Yes = allowed), matching ME field semantics and our Off=0 shell.
4. **PLAYER EVs** uses Off/On for `noEvs` (ME descriptions; ME’s Yes/No draw is inconsistent with those descriptions).
5. **PLAYER IVs** labels are Off / Max / HP (values 0/1/2), not ME’s Yes/No/No(HP), so the chrome matches the descriptions.
6. **HARD MODE EXP.** labels are Default / Normal (`hardExp` 0/1), matching ME.
7. **`EnsureWritable`** also accepts `MF_RULE_EDIT_DIFFICULTY` so mid-run edits work when `lockDifficulty` is off without forcing the debug unlock override (ADR 0015).

## Alternatives considered

- Story-bullet option order — rejected; ME live enum is the UX spec (same as S20).
- Keep ME’s Yes/No/No(HP) for PLAYER IVs — rejected; conflicts with description Off/On/HP meaning.
- Put Pokécenter on Difficulty for story completeness — rejected; ME Challenges page owns it; S24 covers it.

## Consequences

- S40–S45 difficulty wiring can trust these value indices.
- Stub CONTINUE remains after Difficulty until S24–S25.
- LOCK DIFFICULTY stays meta (`MF_RULE_EDIT_META`); editable only while rules are unlocked (new game) or via debug override.
