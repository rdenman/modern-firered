# 0007 — Keep GEN_LATEST battle baseline (no Kanto downgrade)

- **Type:** tech
- **Status:** Accepted
- **Date:** 2026-09-11
- **Story:** S07
- **ME reference:** Gamemode Modern — phys/spec split, Fairy typing, modern type chart (compile-time stand-in until Phase 4)
- **Expansion config:** `GEN_LATEST` (`GEN_9`); `B_PHYSICAL_SPECIAL_SPLIT`; `P_UPDATED_TYPES`; `B_UPDATED_TYPE_MATCHUPS` (all default `GEN_LATEST`)

## Context

Phase 1 flips expansion configs to always-on modern defaults before the rules engine can toggle them. Expansion already defaults most battle/Pokémon knobs to `GEN_LATEST` (`GEN_9`). S07 asks whether FireRed overrides any of that, whether Fairy is actually “on” (it has no single macro), and whether any Gen 9 mechanic should be carved out for a Kanto game.

## Decision

- **Keep `GEN_LATEST` at Gen 9** as the Phase 1 ROM baseline. Do not blanket-set it (or the S07 macros) to Gen 3/4 for “vanilla Kanto.”
- **Phys/spec:** rely on `B_PHYSICAL_SPECIAL_SPLIT` defaulting to `GEN_LATEST` (`>= GEN_4`). No FR-specific override exists; leave `include/config/battle.h` untouched.
- **Fairy:** document the multi-config set in [`docs-mf/BATTLE_BASELINE.md`](../BATTLE_BASELINE.md) rather than inventing an `MF_FAIRY` compile switch. Required pieces: `TYPE_FAIRY`, Fairy chart/`gTypesInfo` entries, `B_UPDATED_TYPE_MATCHUPS >= GEN_6`, `P_UPDATED_TYPES >= GEN_6`.
- **Exceptions:** none for phys/spec, Fairy typing, or the type chart in S07. Future undesirable Gen 9 behaviors get their own ADR and a targeted macro change — not a global `GEN_LATEST` downgrade.
- **Pinning:** regression tests under `test/modern_firered/mf_battle_baseline.c` assert the thresholds and Kanto spot-checks; we do not `#define` over upstream config headers (keeps `RHH/master` merges cheap per `UPSTREAM.md`).

## Alternatives considered

- Downgrade `GEN_LATEST` to Gen 3/4 for FRLG authenticity — rejected; conflicts with PROJECT.md modernization goals and ME’s Modern preset intent.
- Add `MF_*` compile mirrors that force phys/spec and Fairy — rejected for S07; would duplicate upstream knobs and create merge noise. Phase 4 runtime gates are the right toggle surface.
- Explicitly edit `B_PHYSICAL_SPECIAL_SPLIT` / `P_UPDATED_TYPES` to `GEN_LATEST` in-tree — no-op vs current defaults; skipped to avoid fake ownership of upstream lines.

## Consequences

- FireRed battles already use move-based categories, Fairy matchups, and Gen-6+ Kanto retypes with zero config edits in this story.
- Phase 4 (S27–S28) will revisit the same call sites for runtime Fairy / types / chart toggles; this ADR is the compile-time baseline those stories wrap.
- Upstream merges that change defaults must be checked against `BATTLE_BASELINE.md`; failing MF tests mean re-apply or ADR a deliberate exception.
- Adjacent Gen 9 behaviors that touch Kanto feel but are out of S07 scope (badge stat boosts off, trainer EXP multiplier, etc.) remain at `GEN_LATEST` until a later story or ADR says otherwise.
