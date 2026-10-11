# Changelog

## Unreleased

- Item randomization dests are FRLG/RSE items plus later-gen evolution items (Linking Cord, Electirizer, …). Mega Stones, Z-Crystals, and later competitive holds are out of the pool.
- Mega Evolutions compiled out of the FireRed ROM (primals still on).
- Features **REGIONAL FORMS** (default Off). Alolan/Galarian/Hisuian/Paldean forms stay in the ROM but are out of the species pool unless this is On. Existing saves migrate the option On so in-progress randomizers do not reshuffle.

## 1.0.0

First player release. Patch a clean **FireRed (USA, Rev 0)** dump — never share the `.gba`.

- Rules menu at New Game (Classic / Modern / Custom plus Features, Randomizer, Nuzlocke, Difficulty, Challenges).
- Options+ QoL (autorun, Joy speed, fast battles, STAB hints, bag extras).
- Nuzlocke, randomizer, monotype, level caps, and the rest of the in-game toggles; see [PLAYER_FEATURES.md](./PLAYER_FEATURES.md).
- Celio gives MysticTicket and AuroraTicket after the Sapphire.
- Save-backed rules with migration; version stamp **v1.0.0** in Options+.
- Debug menu and Quickstart are **off** in this build (`make firered RELEASE=1`).

**Requires:** Pokémon FireRed (USA) revision 0, 16,777,216 bytes, SHA-1 `41cb43a279208d5e14ee8ae47285c6bdae16dd85`.

**How to apply:** [RELEASE.md](./RELEASE.md).
