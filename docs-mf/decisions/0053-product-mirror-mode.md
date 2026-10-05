# 0053 — Mirror Mode copies the foe onto the player; Thief keeps it

- **Type:** product
- **Status:** Accepted
- **Date:** 2026-10-05
- **Story:** S50
- **ME reference:** `tx_Challenges_Mirror` / `tx_Challenges_Mirror_Thief` in `src/battle_main.c` and Challenges descriptions in `src/tx_rac_menu.c`
- **Expansion config:** —

## Context

S50’s backlog line said opponents would copy the player’s party, with Thief stealing those Pokémon. ME’s menu copy and battle code do the opposite: the **player** receives a copy of the **enemy** party. PROJECT.md only names “mirror (± thief)”. ME is the spec for option semantics.

ME also has a leftover “all battles” description, but the control is Off/On (ADR 0024). Apply is gated on `BATTLE_TYPE_TRAINER || BATTLE_TYPE_DOUBLE`. Thief skips the party backup so the stolen team stays after battle.

STORIES.md asked how this behaves before the first catch and with the randomizer. S53 is not built yet.

## Decision

1. **Direction** — follow ME, not the inverted story sentence. Trainer battles (and doubles, including wild doubles) replace the player party with a struct copy of the foe party (species, level, moves, items, IVs, OT). Single battles copy opponent A, including empty slots. Two-opponent battles copy A into slots 0–2 and B into 3–5.
2. **Thief off** — backup the real party in EWRAM and restore it at `HandleEndTurn_FinishBattle` **before** Nuzlocke faint handling, so borrowed faints do not cemetery the real team.
3. **Thief on** — no backup; the player keeps the foe team (still with that trainer’s OT). Nuzlocke then sees the stolen party.
4. **Pre-first-catch** — no special case. Whatever is in the player party is overwritten for the fight (usually the starter); non-thief restores it.
5. **Randomizer** — copy happens after `CreateNPCTrainerParty` (in `CB2_InitBattleInternal`). S53 trainer mapping will be visible automatically if it mutates the foe party first.
6. **Skip** — link/recorded, the test runner, and debug battles (`gIsDebugBattle`). ME wrapped apply in `TX_DEBUG_SYSTEM_ENABLE`; that would turn the challenge off in release, so we do not copy that.
7. **HM overwrite** — ME’s `HMsOverwriteOptionActive` (lead can use bag HMs) is **not** ported here. Non-thief restores HMs. Thief can drop field moves; a later field-move / combo story can add the bag-HM shortcut if it becomes a Kanto softlock.

## Alternatives considered

- **Opponents copy the player** (story wording) — rejected; contradicts ME labels (“the player gets a copy of the enemy's party”).
- **Species-only clone with player levels** — rejected; ME copies the whole `Pokemon` struct.
- **Wild singles** — rejected; ME’s leftover “all battles” string is unused; Off/On is trainer+double only.
- **Port HM overwrite in S50** — rejected; it also covers Nuzlocke / monotype / random moves and is a party-menu change, not team construction.

## Consequences

- Upstream: one-line start/end hooks in `battle_main.c`. Logic lives in `mf_mirror.c`.
- S53 should keep generating the foe party before battle start so Mirror sees the randomized team.
- S65 should treat Mirror Thief + Nuzlocke as a dangerous combo (stolen team, then cemetery).
