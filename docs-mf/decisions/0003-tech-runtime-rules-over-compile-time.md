# 0003 — Runtime rules layer over compile-time expansion configs

- **Type:** tech
- **Status:** Accepted
- **Date:** 2026-09-11
- **Story:** S03
- **ME reference:** `tx_randomizer_and_challenges` / start-of-game options pages (spec only)
- **Expansion config:** many `B_*` / `P_*` / item / summary toggles under `include/config/` (compile-time)

## Context

Modern Emerald exposes Gamemode, Features, Randomizer, Nuzlocke, Difficulty, and Challenges as **player choices at new game**. pokeemerald-expansion already implements many of the same *effects* as compile-time `#define`s (`GEN_LATEST`, phys/spec, reusable TMs, summary IV/EV, etc.).

If Modern FireRed only flips those macros, every run gets the same rules and we cannot match ME’s modular start-of-run UX. If we ignore expansion configs and reimplement everything from scratch, we duplicate battle/Pokémon logic and fight upstream forever.

We need an architectural bet **before** Phase 2+ builds the rules engine: how compile-time expansion and a runtime rules layer relate.

## Decision

- Treat expansion’s compile-time configs as the **baseline capability and default ROM behavior** (Phase 1 flips what should always be modern/playable).
- Implement a **runtime rules layer** (save-backed flags + accessors, ME’s `tx_*` as UX/algorithm reference) that can override or specialize behavior per save when a player-facing menu option exists.
- Player-toggleable options must resolve through **runtime accessors**, not new `#if` forks of gameplay paths. Compile-time `MF_*` master switches may disable whole unfinished subsystems; they are not substitutes for per-run rules.
- Prefer enabling/configuring an existing expansion primitive, then wrapping it with a runtime check, over rewriting the primitive.

## Alternatives considered

- Compile-time only (flip expansion configs, no start-of-game menu) — rejected; fails PROJECT.md’s ME-like options requirement.
- Runtime only (ignore expansion configs, reimplement mechanics) — rejected; huge duplicate surface and merge pain versus RHH.
- Dual ROMs / build flavors per preset — rejected; not how ME works and explodes CI/support.

## Consequences

- Phase 1 may turn features “always on” in the ROM; later phases make the same call sites respect save rules where ME allows a toggle (documented as intentional revisit, not accidental rework).
- S12+ owns save layout; S13/S14 own accessors/presets; gameplay wiring stories call accessors on hot paths cheaply.
- Upstream merges that change a config default still land in `include/config/*.h`; our runtime layer must not assume a frozen compile-time value without an accessor.
- ADR numbering: STORIES.md historically called this “ADR-0001”; S01/S02 already used 0001/0002 for toolchain/CI — this record is **0003**.
