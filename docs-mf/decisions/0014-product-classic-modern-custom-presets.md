# 0014 — Classic / Modern / Custom gamemode preset tables

- **Type:** product
- **Status:** Accepted
- **Date:** 2026-09-12
- **Story:** S14
- **ME reference:** `DrawChoices_Mode_Classic_Modern_Selector` in `src/tx_rac_menu.c`; `TX_*` defaults in `include/tx_randomizer_and_challenges.h`
- **Expansion config:** `MF_DEFAULT_GAMEMODE_PRESET`, `MF_TX_*` in `include/config/modern_firered.h`

## Context

S14 needs a coherent rules blob on every new game before the Phase 3 menu exists, and exact Classic/Modern/Custom vectors for later menu bulk-set (S20). ME only bulk-sets the **Gamemode** page; Features / Randomizer / Nuzlocke / Difficulty / Challenges come from its compile-time `TX_*` block. We also ship Phase 1 modernization always-on via accessors when the engine is off (ADR 0013) — that null vector is **not** the same as the Modern preset (Modern turns on Synchronize, Mints, Sturdy, Legendary Abilities, alternate spawns).

## Decision

### New-game path

1. `NewGameInitData()` calls `MfRules_InitNewGame()` (replaces bare `MfRules_ResetToEmpty`).
2. Init applies `MfRules_ApplyDevDefaults()` (all pages from `MF_TX_*`), then `MfRules_ApplyGamemodePreset(MF_DEFAULT_GAMEMODE_PRESET)`.
3. Default `MF_DEFAULT_GAMEMODE_PRESET` is **Modern (1)** so skipping the menu keeps Phase 1’s modern Kanto feel until S19 wires the UI. Override via `MF_CPPFLAGS` for test builds.

### Gamemode preset table (fields Classic/Modern force)

| Rule field | Classic | Modern | Custom |
| ---------- | ------- | ------ | ------ |
| `alternateSpawns` | 0 | 1 | leave / `MF_TX_MODE_ALTERNATE_SPAWNS` |
| `infiniteTms` | off | on | leave / `MF_TX_*` |
| `survivePoison` | off | on | leave |
| `synchronize` | off | on | leave |
| `mints` | off | on | leave |
| `modernSitrus` | off | on | leave |
| `modernTypes` | off | on | leave |
| `fairyTypes` | off | on | leave |
| `modernStats` | off | on | leave |
| `sturdy` | off | on | leave |
| `modernMoves` | off | on | leave |
| `legendaryAbilities` | off | on | leave |
| `newLegendaries` | **off** | **off** | leave (`MF_TX` default off) |
| `typeEffectiveness` | off | on | leave |

Custom does not rewrite Gamemode fields — it only sets `gamemodePreset = CUSTOM` so the menu can edit (ME behavior). Switching Classic → Custom keeps Classic zeros until the player changes them.

### Non-gamemode pages

Always seeded from `MF_TX_*` (ME parity): e.g. `randomSimilar` / `randomMapBased` on, Nuzlocke clauses on but Nuzlocke itself off, monotype `31`, difficulty/challenges off. Presets do not touch these.

### Deviations from ME

- **`newLegendaries` always off** in Classic and Modern — PROJECT.md forbids new legendary maps; ME’s Modern sets it on and raises `FLAG_EXTRA_LEGENDARIES`.
- **Default skip-menu preset is Modern**, not ME’s menu-default Classic / all-zero `TX_MODE_*` — we already shipped Phase 1 modernization and have no menu yet (S19).

## Alternatives considered

- Default new games to Classic — rejected; would regress Phase 1 feel until the player opens a menu that does not exist yet.
- Make Modern identical to `gMfRulesPhase1Defaults` — rejected; ME’s Modern is the product spec for the Gamemode page (Synchronize/Mints/Sturdy/etc. on); Phase 1 null defaults stay the compile-out / bad-version fallback only.
- Pack presets only as code with no `MF_TX_*` knobs — rejected; story asks for ME-style compile defaults for test ROMs.

## Consequences

- S20 can call `MfRules_ApplyGamemodePreset` when the player cycles Classic/Modern/Custom.
- S19 must run the menu **before** anything rule-dependent is populated, or re-apply the chosen preset after the menu commits. S15 currently locks inside `InitNewGame` (skip-menu); S19 must remove that commit so the menu can edit, and S26 SAVE must call `MfRules_CommitAndLock()` (ADR 0015).
- Changing `MF_TX_*` or `MF_DEFAULT_GAMEMODE_PRESET` is a rebuild-only knob; it does not migrate existing saves.
- Unit tests lock Classic/Modern gamemode vectors and `InitNewGame` → save contents (`make check TESTS='MF: rules'`).
