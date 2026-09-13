# 0024 — Challenges page: ME order, monotype Off+types, gated PC heal / Mirror Thief

- **Type:** ux
- **Status:** Accepted
- **Date:** 2026-09-13
- **Story:** S24
- **ME reference:** `MENUITEM_CHALLENGES_*` / `MENUITEM_DIFFICULTY_POKECENTER` / `DrawChoices_Challenges_*` in `src/tx_rac_menu.c`
- **Expansion config:** —

## Context

S24 adds the Challenges page as pure UI over existing `ModernRules` challenge fields. ME places Pokécenter on this page (despite a difficulty-shaped field name), gates PC heal on centers being allowed and Mirror Thief on Mirror Mode, and stores monotype as a type id with `TX_CHALLENGE_TYPE_OFF` (31). ME also offers a Random monotype slot resolved at SAVE time; our menu writes live, and expansion’s type enum includes NONE / MYSTERY / STELLAR that are not useful monotype targets.

## Decision

1. **Page order** follows ME’s Challenges enum: POKéCENTER → PC HEALS {PKMN} → ULTRA EXPENSIVE! → EVO LIMIT → ONE TYPE ONLY → BST EQUALIZER → MIRROR MODE → MIRROR THIEF → NEXT (SAVE deferred to S26).
2. **POKéCENTER** and **PC HEALS** use Yes/No labels mapped 0/1 onto `pokeCenterLimit` / `noPcHeal` (Yes = allowed / heal), matching ME field semantics.
3. **PC HEALS** uses `MF_RULES_MENU_FLAG_REQUIRES_POKECENTER` (editable only when `pokeCenterLimit == 0`). **MIRROR THIEF** uses `MF_RULES_MENU_FLAG_REQUIRES_MIRROR`; turning Mirror off clears Thief (ME draw side effect).
4. **ONE TYPE ONLY** cycles Off + the 18 real types (Normal–Fairy), skipping NONE / MYSTERY / STELLAR. Menu index maps to stored type id via `sMonotypeStoredValues`; Off stores `MF_TX_CHALLENGE_TYPE_OFF` (31). No Random slot — live writes have no SAVE-time resolve, and the story asks for types plus Off.
5. **EVO LIMIT** labels Off / First / All (values 0/1/2). **BST EQUALIZER** Off / 100 / 255 / 500. **ULTRA EXPENSIVE!** Off / x5 / x10 / x50! (values 0–3).
6. **MIRROR MODE** is Off/On on the existing 1-bit field (ME draw is Off/On despite a leftover “All” description string).

## Alternatives considered

- ME’s selection-index monotype + Random at SAVE — rejected; mismatches live `TrySetValue` writes and expansion’s TYPE_NONE=0 layout.
- Include Stellar / Mystery — rejected; not meaningful capture-type restrictions for this challenge.
- Put Pokécenter on Difficulty to match the struct comment cluster — rejected; ME Challenges UI owns it (ADR 0023).

## Consequences

- S46–S50 challenge wiring can trust these indices and the 31=Off monotype sentinel already used by accessors.
- Stub CONTINUE remains after Challenges until S25 (Randomizer).
- Debug Challenges page still edits raw fields (including monotype 0–31); the rules menu is the player-facing mapped control.
