# 0104 — National Dex when a run can produce non-Kanto species

- **Type:** product
- **Status:** Accepted
- **Date:** 2026-10-21
- **Story:** —
- **ME reference:** —
- **Expansion config:** —

## Context

FireRed keeps the Kanto dex until Oak’s post–Elite Four upgrade. Seen/caught flags already use National numbers, but the summary and Pokédex list stay regional. Randomized or Modern-encounter species outside 001–151 mapped to Kanto order 0, so the summary printed **000** and those entries were missing from the dex.

## Decision

1. **Unlock National Dex** (`EnableNationalPokedex`) when the run can produce a non-Kanto species: Encounters ≠ Vanilla, or randomizer master plus starter / wild / trainer / static / evolution remaps.
2. **When:** new-game SAVE (`MfRules_CommitAndLock`), save load (`MfSave_MigrateOnLoad` so existing randomizer saves recover), debug rule writes, and QA matrix apply. Do not turn National Dex back off.
3. **Classic vanilla** stays Kanto-only until Oak’s usual scene. Hall of Fame already skips that scene when National Dex is on.
4. **`SpeciesToPokedexNum`** treats regional order 0 as not in this dex (`0xFFFF`) so a Magnezone on Classic no longer shows **000**.

## Alternatives considered

- Always-on National Dex — rejected; spoils Sevii and skips Oak on Classic.
- Summary-only National numbers — rejected; the Pokédex still could not open those species.
- Unlock only for the randomizer — rejected; Modern encounter tables already include Johto/Hoenn families.

## Consequences

- Modern preset (Encounters = Modern) starts with National Dex. Item/move/ability-only randomizers on Classic do not.
- Existing saves pick this up on load; no new-game required.
- Upstream `pokemon.c`: regional 0 is invalid for summary/trade “in regional dex” checks.
