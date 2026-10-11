# Save versioning (S64)

How Modern FireRed upgrades an existing save without bricking it.

See ADR [0077](./decisions/0077-tech-rules-save-migration.md). Layout: ADR [0012](./decisions/0012-tech-rules-save-storage.md). Gameplay fallback: ADR [0013](./decisions/0013-tech-rules-accessor-null-defaults.md).

## When migration runs

`LoadGameSave(SAVE_NORMAL)` calls `MfSave_MigrateOnLoad()` after a successful slot read. That:

1. If the rules blob is an **older recognized** version and the serialized size grew, `memmove`s `mfOptions` down so it still sits immediately after `mfRules`.
2. Runs `MfRules_Migrate` (field defaults + `version` stamp).
3. Runs `MfOptions_Migrate` (Options+ extras; already used on `MfOptions_GetSave`).

New games still go through `MfRules_InitNewGame` / `MfOptions_InitNewGame` and never need this path.

`MfRules_GetActiveRules()` stays **read-only**. It must not migrate (battle/overworld hot path). After a successful Continue, `version` matches `MF_RULES_VERSION` and accessors read the save. If `version` is still wrong, they return `gMfRulesPhase1Defaults`.

## Additive-only `struct ModernRules`

Once a version has shipped, do **not**:

- Reorder, remove, or resize existing fields
- Repurpose a named field for a different meaning
- Silently change `MF_RULES_VERSION` without a migrator step

Do:

1. Prefer unused padding bits (`padding0`, `paddingGamemode`, `paddingFeatures`, `paddingNuzlocke`, `paddingTail`). Zero is the default for those bits (v2 clears leftover padding on load).
2. If you need new bytes, **append** them at the end of `struct ModernRules` (after `nuzlockeEncounterFlags`). That shifts `SaveBlock3.mfOptions`; the load path relocates it when the size table says the old blob was shorter.
3. Bump `MF_RULES_VERSION`.
4. Set `MF_RULES_SERIALIZED_SIZE_CURRENT` to `sizeof(struct ModernRules)` (the compile-time size lock fails until you do).
5. Add the old version’s byte size to `MfRules_SerializedSizeForVersion` (keep every historical index 1…N).
6. Add `if (version < NEW)` in `MfRules_Migrate` that writes **non-zero** defaults. Bytes past the old size are already zeroed.
7. Update `MF_SAVEBLOCK3_SIZE` and `test/save.c` `T_SAVEBLOCK3_SIZE` together.

Never add a field in the middle. Never consume padding for a default-TRUE flag without an explicit `if (version < N)` that sets it.

Current: **`MF_RULES_VERSION` 3**. v3 names `regionalForms` from Features padding; v1/v2 migrate that bit On (ADR 0084). Blob size is still 48 bytes.

## Unrecognized / newer saves

| On-disk `version` | Load | Gameplay | Next save |
| ----------------- | ---- | -------- | --------- |
| `1` … current−1 | Migrate in place; stamp current | Migrated blob | Writes current layout |
| current | No-op | Save blob | Unchanged |
| `0` or unknown | **Do not write** | Phase 1 defaults | Leaves the junk/future bytes as loaded |
| **> current** (save from a newer ROM) | **Do not write** | Phase 1 defaults | Leaves the newer blob intact so that ROM can still read it |

Writers (`MfRules_TrySet*`, `CommitAndLock`) already refuse when `version != MF_RULES_VERSION`, so a future save is not silently rewritten as Phase 1.

Options+ is separate (`MF_OPTIONS_VERSION`). Unknown Options+ versions still reset extras to defaults via `MfOptions_Migrate` — that blob is 4 bytes of QoL, not a run’s rules.

## Size locks

| Check | Where | Why |
| ----- | ----- | --- |
| `sizeof(ModernRules) == MF_RULES_SERIALIZED_SIZE_CURRENT` (48) | `mf_rules.h` | Adding a field is a compile error until versioning is updated |
| `sizeof(ModernRules) <= MF_RULES_MAX_BYTES` (128) | `mf_rules.h` | Soft budget for flags + seed |
| `sizeof(SaveBlock3) == MF_SAVEBLOCK3_SIZE` (56) | `mf_save.c` | Options+ offset and SB3 layout |
| `sizeof(SaveBlock3) <= 1624` | `save.c` + `mf_save.c` | Flash chunk hard cap |
| `T_SAVEBLOCK3_SIZE` | `test/save.c` | Upstream-style exact lock |

## Tests

```bash
make check -j$(sysctl -n hw.ncpu) TESTS='MF: rules'
make check -j$(sysctl -n hw.ncpu) TESTS='MF: save'
```
