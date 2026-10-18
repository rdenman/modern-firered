# 0085 — Nuzlocke forced nicknaming on gift Pokémon

- **Type:** product
- **Status:** Accepted
- **Date:** 2026-10-17
- **Story:** — (follow-up to S38)
- **ME reference:** `IsNuzlockeNicknamingActive` (ME does not force gift nicknames)
- **Expansion config:** `MF_NUZLOCKE`; runtime `nuzlockeNicknaming`

## Context

S38 / ADR 0039 wired Nuzlocke `NICKNAMES` for wild catch and egg hatch, matching ME. FireRed gifts (Oak's starter, fossils, Magikarp, Eevee, Lapras, Hitmon, Game Corner prizes) still used the vanilla Yes/No `gText_NicknameThisPokemon` prompt, so a Nuzlocke run could keep the species name on the starter.

## Decision

1. Shared script `Common_EventScript_AskNicknameGiftMon` replaces that Yes/No. When `MfNuzlocke_IsNicknamingActive()` is true it sets `VAR_RESULT` to YES and skips the box; otherwise vanilla Yes/No is unchanged.
2. Every FireRed `givemon` gift that already offered a nickname uses this helper (party and PC paths). Name Rater and other rename flows are untouched.
3. Empty names are still refused by the existing naming-screen hook (`KeyboardKeyHandler_OK`).

## Alternatives considered

- **Auto-assign a generated nickname with no screen** — rejected; catch/hatch already open the naming screen, and the menu copy is “Forces the player to nickname every Pokémon.”
- **Patch `GiveScriptedMonToPlayer` in C** — rejected; `givemon` is used by tests and debug, and would open naming without a script waitstate. Script-level Yes/No skip matches the existing catch/hatch pattern.
- **Edit only Oak's lab** — rejected; the same optional prompt exists on every Kanto gift.

## Consequences

- FR map scripts (`*_Frlg`) plus `data/scripts/pc_transfer.inc` and a `u16` special wrapper. Emerald gift maps stay vanilla.
- ADR 0039's “gifts unchanged” bullet is superseded for FireRed gifts.
