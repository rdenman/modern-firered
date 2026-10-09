# 0064 — Options+ paged shell and SaveBlock3 extras

- **Type:** tech
- **Status:** Accepted
- **Date:** 2026-10-09
- **Story:** S58
- **ME reference:** `src/options_plus_menu.c` (Main / Battle / Sound pages)
- **Expansion config:** `MF_OPTIONS_PLUS`

## Context

S58 replaces FireRed’s single-screen Options with an ME-style paged menu that still has to persist vanilla settings and leave room for S59–S63. ME packs extra flags into SaveBlock2. Expansion’s SaveBlock2 is a high-conflict, size-locked struct; Options+ bits would fight every `RHH/master` merge. The S18 rules menu already owns the FR three-window layout we want, but its item table is wired to `MfRules_*` accessors and lock flags.

## Decision

1. **Menu** lives in `src/mf_options_menu.c` (S18 windows, palettes, 5 visible rows, data tables). Vanilla `CB2_InitOptionMenu` redirects here when `MF_OPTIONS_PLUS` is on (title screen and Start menu both keep working). **B** or **EXIT** leaves; **L/R** and **NEXT** change pages. Writes are live, like cycling Frame in vanilla.
2. **Vanilla fields stay on SaveBlock2** (`optionsTextSpeed` and friends). Extra QoL bits go in **`SaveBlock3.mfOptions`** (`struct MfOptions`, 4 bytes, `MF_OPTIONS_VERSION`), appended after `mfRules` so pre-S58 saves still map rules.
3. **Pages (FR subset of ME):** OPTIONS (vanilla + INSTANT text), QOL (autorun / Joy / skip intro / units), BATTLE (fast battles / anim / intro / ball / STAB / cursor / run prompt). Drop followers, music packs, battle terrain, difficulty, font, dive autorun — out of `PROJECT.md` or unused in Kanto.
4. **Gameplay of extra rows is not wired in S58.** Accessors persist so S59–S63 only hook behavior. Hardcore Nuzlocke still greys Battle Style to SET (ADR 0040).

## Alternatives considered

- **Extend `src/option_menu.c` in place** — rejected; huge upstream diff and no description pane.
- **Share `mf_rules_menu.c` via a generic widget** — rejected for S58; the rules shell is lock/preset-specific. Clone the layout instead of a risky extract.
- **Store extras in SaveBlock2 padding** — rejected; only ~20 spare bits, and SB2 stays merge-hostile.
- **Ship vanilla page only, add QoL rows later** — rejected; the story asks for pages for the new options, and adding rows later reshuffles save field meaning.

## Consequences

- `test/save.c` SaveBlock3 size lock is 56 (was 52). Unrecognized `mfOptions.version` resets extras to off/vanilla.
- S59–S63 read `MfOptions_Get` / `MfOptions_Set`; do not add parallel save fields.
- Sound page extras from ME are omitted; Stereo stays on the main page.
