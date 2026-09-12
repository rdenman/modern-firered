# Manual QA — S12 Rules data model & save storage

Rules blob in `SaveBlock3.mfRules`; pack/unpack covered by `make check TESTS='MF: rules'`. This checklist covers save/load persistence in mGBA.

---

## Story

- **ID:** S12
- **Feature / rule under test:** `struct ModernRules` write → save → reload → read back
- **Build:** `pokefirered.gba` from `make firered -j$(sysctl -n hw.ncpu)` (non-release)
- **Date / tester:** …

## Setup

- **Rules / presets at new game:** N/A (no menu yet; S14/S18). Use debug or a temporary poke via future S17 — for now verify via mGBA memory or that new game leaves a valid empty rules blob.
- **How to reach the scenario quickly:** Title → New Game (or Continue on a pre-S12 save) → save in-game → soft-reset → Continue
- **Save state slot (if used):** …

## Checklist

| # | Step | Expected | Pass? |
| - | ---- | -------- | ----- |
| 1 | Boot `pokefirered.gba` in mGBA | Title screen; no hang | |
| 2 | Start a New Game and reach the overworld (Quickstart OK) | Game runs; `ClearSav3` + `MfRules_ResetToEmpty` left rules with `version == 1` | |
| 3 | Save the game (in-game Save) | Save completes | |
| 4 | Soft-reset (or close/reopen ROM) and Continue | Save loads; overworld resumes | |
| 5 | (Optional) In mGBA Tools → Memory viewer, find `gSaveblock3` / `mfRules.version` | `version` is `1` after new game; survives save/load | |
| 6 | Load a save created **before** this ROM (if you have one) | Save still loads; party/progress intact; rules fields are whatever was in unused SB3 bytes (typically 0) until S64 | |

## Negative / edge cases

| # | Step | Expected | Pass? |
| - | ---- | -------- | ----- |
| N1 | Build with `MF_RULES_ENGINE=0` still links (storage always present) | ROM boots; empty rules struct still in SaveBlock3 | |

## Display consistency (if types/stats/moves/items change)

- N/A

## Evidence

- Screenshots / notes: …
- Automated coverage: `make check TESTS='MF: rules'` / `MF: ModernRules` / SaveBlock3 size test □

## Follow-ups

- S13 accessors; S14 new-game presets; S64 migration for `version`
- Bugs found: …
---
