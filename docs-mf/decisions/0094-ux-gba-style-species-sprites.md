# 0094 — Gen 3 GBA species sprites as FireRed default

- **Type:** ux
- **Status:** Accepted
- **Date:** 2026-10-17
- **Story:** —
- **ME reference:** — (ME uses expansion’s Gen 4/5-style sheets)
- **Expansion config:** `P_GBA_STYLE_SPECIES_GFX` / `_ICONS` / `_FOOTPRINTS` (upstream default `FALSE`)

## Context

Expansion ships modern (Gen 4/5-style) front/back sprites, party icons, and footprints, with original GBA art still in-tree as `*_gba` assets. Modern FireRed is a Kanto ROM; the Gen 3 sheets match FRLG/RSE better. Regional forms and cross-gen evos have no GBA originals.

## Decision

1. **FireRed ROM** (`!TESTING`) sets `P_GBA_STYLE_SPECIES_GFX`, `_ICONS`, and `_FOOTPRINTS` to `TRUE` via `mf_species_enabled.h` (`MF_SPECIES_GBA_STYLE_GFX`). Do not edit upstream `pokemon.h` defaults.
2. **Compile-time only.** No runtime / Features toggle. Classic, Modern, and REGIONAL FORMS share the same baked sheets.
3. **Later-gen species keep modern art** (Alolan forms, Magnezone, Sylveon, Kleavor, …). That mix is accepted.
4. **TESTELF** keeps expansion’s Gen 4/5-style default so upstream sprite tests stay on their usual assets.

## Alternatives considered

- **Runtime swap** — rejected; both sheet sets in the ROM is a large payload for a preference that is not a rules-page concern.
- **Tie GBA art to REGIONAL FORMS Off** — rejected; species availability and sprite style are independent (ADR 0084).
- **GFX only, keep modern icons/footprints** — rejected; party/summary should match battle art.
- **Also force `P_TWO_FRAME_FRONT_SPRITES FALSE`** — deferred; GBA sheets still use the two-frame layout expansion expects.

## Consequences

- Opponent Gen 4+ ground shadows stay off while GBA gfx is on (upstream already gates `B_ENEMY_MON_SHADOW_STYLE` on `P_GBA_STYLE_SPECIES_GFX == FALSE`).
- Overworld follower/object-event mons still use the modern OW sheets (no GBA OW variants in-tree). Followers remain out of product scope.
- A later `RHH/master` merge that touches `P_GBA_STYLE_*` in `pokemon.h` is a no-op for FR as long as the MF override file still `#undef`s them.
