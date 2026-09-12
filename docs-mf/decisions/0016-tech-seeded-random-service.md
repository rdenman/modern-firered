# 0016 — Deterministic randomizer seed service (not OT ID)

- **Type:** tech
- **Status:** Accepted
- **Date:** 2026-09-12
- **Story:** S16
- **ME reference:** `RandomSeeded` / `RandomSeededModulo` in `src/random.c`; `TX_RANDOM_T_*` + `GetSpeciesRandomSeeded` in `src/pokemon.c` (category + map-based behavior)
- **Expansion config:** `MF_RANDOMIZER` (compile-out kill only)

## Context

S16 needs a pure `(seed, category, input) → output` mapping that survives soft-reset and save/load, with ME’s randomizer categories and map-based location keying. ME seeds via the player OT ID (`GetTrainerId`) and a single `ISO_RANDOMIZE1` step. We already store `ModernRules.randomizerSeed` (S12) and want gameplay remaps (S51+) to never depend on `Random()` call order.

## Decision

1. **Per-save seed:** `struct ModernRules.randomizerSeed` is the sole randomizer entropy. `MfRandom_EnsureSeed` assigns a non-zero `Random32()` value once at new-game init (`MfRules_InitNewGame`). `0` means unset. The seed is not derived from OT ID.
2. **Pure API:** `MfRandom_Hash` / `MfRandom_Modulo` take explicit seed + `enum MfRandomCategory` (ME `TX_RANDOM_T_*` numeric parity) + input id + location key. SplitMix32-style mixing; modulo uses short rejection sampling (ME-inspired) without touching the global RNG.
3. **Map-based:** `MfRandom_LocationKey` returns `mapsec` only for **wild** and **trainer** when `randomMapBased` is on; moves / ability / evo / evo-method / static always use location key `0` (ME parity).
4. **Wrappers:** `MfRandom_*Active` read the active rules seed and map-based flag. Species tables and constraint filters stay in S51+.
5. **Chaos:** ME’s chaos path calls `Random()` and breaks determinism. Our S16 service stays pure; if chaos is wired later, it must be an explicit opt-out at the call site, not inside `MfRandom_*`.

## Alternatives considered

- Match ME and seed from OT ID — rejected; we already budgeted `randomizerSeed`, and OT ID conflates identity with remap entropy.
- Stream/`LocalRandom` state advanced per call — rejected; ordering would break determinism across soft-reset paths.
- Defer seed assignment until first randomizer use — rejected; new-game commit should freeze entropy before any remap.

## Consequences

- S51+ must call `MfRandom_Modulo` / `*Active` (never `Random()`) for remaps.
- Changing the hash algorithm is a soft save-compat break for randomizer runs (same seed, different mappings) — treat as `MF_RULES_VERSION` / documented migration if it ever changes.
- Unit tests live in `test/modern_firered/mf_random.c`; filter with `TESTS='MF: random'`.
