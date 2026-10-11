# 0077 — Rules save migration and unrecognized versions

- **Type:** tech
- **Status:** Accepted
- **Date:** 2026-10-11
- **Story:** S64
- **ME reference:** — (ME has no versioned `tx_*` migrator)
- **Expansion config:** —

## Context

S12 stored `ModernRules.version` and S13 treated any mismatch as Phase 1 defaults. Shipping an update that adds fields would otherwise make every existing save play as a fresh Modern run (or, worse, read new fields as flash junk). S70/S74 already stuffed bits into `paddingTail` without a bump because S64 did not exist yet. We needed a load-time path, a policy for saves from a *newer* ROM, and a size lock so a field cannot grow `SaveBlock3` unnoticed.

## Decision

1. **`MF_RULES_VERSION` is 2.** Version 1 is the S12–S63 48-byte layout. v1→v2 does not grow the struct; it clears unused padding bits (future named fields default to 0) and stamps version 2. Player-facing fields, seed, and Nuzlocke flags are preserved.
2. **Migrate on Continue only** (`MfSave_MigrateOnLoad` from `LoadGameSave` when `SAVE_STATUS_OK`). Accessors stay read-only. If rules size grew, `mfOptions` is `memmove`d to the new offset before field defaults run.
3. **Unrecognized** (`0`, unknown, or **newer than this ROM**): do not write the rules blob. Gameplay uses Phase 1 defaults. Writers already no-op when `version` mismatches, so a future save is not clobbered.
4. **Additive-only** from here: append or consume padding; never reorder. Documented in [`docs-mf/SAVE_VERSIONING.md`](../SAVE_VERSIONING.md).
5. **Exact size locks:** `sizeof(ModernRules) == 48`, `sizeof(SaveBlock3) == 56` (`MF_SAVEBLOCK3_SIZE`), plus the existing 128-byte soft cap and 1624-byte chunk assert.

## Alternatives considered

- **Treat older versions as Phase 1 forever** — rejected; that bricks the player’s chosen Nuzlocke/randomizer on an update.
- **Overwrite unrecognized blobs with Phase 1** — rejected; a save from a newer ROM must still load if the player goes back.
- **Migrate inside `GetActiveRules`** — rejected; that would write save RAM from battle/overworld.

## Consequences

- Existing v1 saves Continue with the same rules; leftover padding is 0 so the next padding-bit feature defaults Off unless its migrator sets it.
- A save made on a future ROM plays this ROM as Phase 1 until the player returns to the newer build.
- The next field that needs new *bytes* must bump `MF_RULES_SERIALIZED_SIZE_CURRENT`, `MF_SAVEBLOCK3_SIZE`, and `T_SAVEBLOCK3_SIZE` together.
- Upstream touch: one call in `src/save.c` `LoadGameSave`.
