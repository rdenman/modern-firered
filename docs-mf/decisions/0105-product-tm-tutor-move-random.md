# 0105 — TM randomizer remaps TM and tutor dest moves

- **Type:** product
- **Status:** Accepted
- **Date:** 2026-10-21
- **Story:** —
- **ME reference:** — (ME does not split TM dests from Items)
- **Expansion config:** `MF_RANDOMIZER`; runtime `randomTms`

## Context

**Items** remaps which item/TM *number* you find (ADR 0101 keeps TMs as TMs). Players also want the *move* each TM and tutor teaches to be random, including junk and signature moves, without collapsing Kanto HM progression or TM/tutor learn eligibility.

## Decision

1. **New Randomizer row `TMs`**, default Off, not implied by **Items** or **Moves**.
2. **68 unique dests** from the full real-move pool (not a TM↔TM shuffle): TM01–50, 15 FRLG tutors, Cape Brink’s three ultimates. Collisions across those slots are forbidden. Unused pool moves simply never appear.
3. **Pool in:** any move with a name and PP, including Tail Whip, Helping Hand, signatures, Sketch, and Cut-the-move. **Out:** `MOVE_NONE`, Struggle, placeholders / 0 PP, Z / Max / G-Max (`move >= MOVES_COUNT`).
4. **HMs** still teach their field moves. **TM51–100** stay vanilla.
5. **Eligibility** is the vanilla teachable set for that TM/tutor slot. **Already-knows** checks the dest.
6. Mapping is **seed-global** (ignore map-based and Chaos). Fisher–Yates the dest pool with `MF_RANDOM_CAT_MOVES` and a TM-specific input tag.
7. **UI:** names, TM type tint, and bag descriptions follow dest (ADR 0112). Tutor NPC flavor chatter stays vanilla except the move name. Debug **TM map dump** lists the 68 dests; no player spoiler sheet.
8. **`randomTms`** consumes `paddingTail` bit 2. `MF_RULES_VERSION` 4; default Off so old saves do not turn it on.

## Alternatives considered

- **Shuffle vanilla TM01–50 moves among TMs** — rejected; dests can be any real move.
- **Hash with collisions** — rejected; dests must be unique across the 68 slots.
- **Dest learnsets** — rejected; too many dests would be unlearnable. Original slot eligibility stays.
- **Genericize tutor flavor** — rejected; players understand randomized chatter.

## Consequences

- Hook `ItemIdToBattleMoveId` / bag TM palette / gift TM names through `MfGetTmHmTeachMove`. Bag descriptions follow dest (ADR 0112). Tutor scripts buffer dest into `STR_VAR_1` before dialogue.
- **Items** + **TMs:** names follow the TM number you actually received, then that TM’s dest move.
- Cape Brink flags still key off the vanilla ultimate, not the dest.
