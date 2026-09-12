# 0012 — Store ModernRules in SaveBlock3

- **Type:** tech
- **Status:** Accepted
- **Date:** 2026-09-12
- **Story:** S12
- **ME reference:** `SaveBlock1` bitfields + `NuzlockeEncounterFlags[9]` at ~0x3D88 in ME `include/global.h`
- **Expansion config:** `include/config/save.h` `FREE_*` (not used for this)

## Context

S12 needs a persistent home for `struct ModernRules` (all six ME option pages, a `version` field, randomizer seed, and per-mapsec Nuzlocke flags). ME packs flags into `SaveBlock1`; expansion already documents `SaveBlock3` as a low-conflict extension blob (max **1624** bytes = 116×14 sector chunks) and offers `FREE_*` reclaim in SaveBlock1/2 (up to ~3790 bytes).

Budget for our blob (bitfields + `u32` seed + `ceil(MAPSEC_COUNT/8)` ≈ 27 flag bytes) is well under 128 bytes — far below either ceiling — so the choice is about **merge cost and save compatibility**, not capacity.

## Decision

- Define `struct ModernRules` in `include/mf_rules.h` (ME field checklist, packed bitfields, `MF_RULES_VERSION`).
- Store it as **`SaveBlock3.mfRules`**, appended after existing SB3 fields (`dexNavChain`, etc.) so pre-S12 saves still map the old SB3 prefix correctly.
- Keep a soft `MF_RULES_MAX_BYTES` (128) assert on the rules struct; rely on expansion’s existing `SaveBlock3FreeSpace` assert for the 1624-byte hard cap.
- Do **not** flip `FREE_*` macros to carve SaveBlock1/2 space for rules.
- Provide `MfRules_Pack` / `MfRules_Unpack` / `MfRules_ResetToEmpty` / `MfRules_GetSaveRules` in `src/mf_rules.c`; call reset from `NewGameInitData`.

## Alternatives considered

- **Pack into SaveBlock1 like ME** — rejected; maximizes merge conflicts with RHH on a hot upstream struct and fights ASLR saveblock layout.
- **Reclaim via `FREE_MYSTERY_GIFT` / other `FREE_*`** — rejected for v1; frees enough bytes but couples rules storage to disabling Mystery Gift (and other optional systems) and still edits SaveBlock1/2 layout.
- **Separate flash sector** — rejected; needs save-format surgery beyond S12 scope.

## Consequences

- Growing SaveBlock3 is the intentional upstream touch for S12 (`include/global.h` + `test/save.c` size lock). Documented in `UPSTREAM.md` if needed on merge.
- Pre-S12 saves still **load**; bytes under `mfRules` may be uninitialized flash junk until S64 treats `version != MF_RULES_VERSION` as “migrate / reset.” Gameplay must not trust rules until S13 accessors define defaults.
- Nuzlocke flags cover full `MAPSEC_COUNT` (Emerald TESTELF + FR), not Kanto-only — avoids FR/Emerald struct skew in `make check`.
- Hoenn-only ME leftovers (`easierFeebas`, `frontierBans`, `newLegendaries`) remain as unused fields for checklist parity; product stories may ignore them.
