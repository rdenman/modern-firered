# Modern FireRed

A **playable FireRed** hack: Modern Emerald–style start-of-run rules (gamemode, randomizer, Nuzlocke, difficulty, challenges) and QoL on **stock Kanto**. Not a story or map rewrite.

**Version:** 1.0.0 — [player features](docs-mf/PLAYER_FEATURES.md) · [changelog](docs-mf/CHANGELOG.md) · [how to patch](docs-mf/RELEASE.md)

- **Scope:** [PROJECT.md](./PROJECT.md)
- **Build:** [BUILDING.md](./BUILDING.md) — `make firered -j$(sysctl -n hw.ncpu)` → `pokefirered.gba`
- **Backlog:** [STORIES.md](./STORIES.md)
- **Agent notes:** [AGENTS.md](./AGENTS.md)
- **Upstream merges:** [docs-mf/UPSTREAM.md](./docs-mf/UPSTREAM.md)
- **Decisions:** [docs-mf/decisions/](./docs-mf/decisions/)

Players: apply the BPS to a FireRed (USA, Rev 0) dump you own ([RELEASE.md](docs-mf/RELEASE.md)). Do not share the patched `.gba`.

Dev builds: `make firered -j$(sysctl -n hw.ncpu)` → `pokefirered.gba`. New Game opens the rules menu; **Options** shows **v1.0.0** and a **CREDITS** row. Release (no debug / Quickstart): `make firered RELEASE=1`.

## Credits

If you use this ROM, credit the engine it is built on:

```
Based off RHH's pokeemerald-expansion 1.17.2 https://github.com/rh-hideout/pokeemerald-expansion/
```

Please consider [crediting all expansion contributors](CREDITS.md).

Gameplay and menu design are inspired by [Modern Emerald](https://github.com/resetes12/pokeemerald) (`resetes12/pokeemerald`). Vanilla FireRed/LeafGreen belong to Game Freak / Nintendo / The Pokémon Company.

This project is **not** an official Pokémon product.

## For contributors (upstream engine)

Modern FireRed is a fork of **`pokeemerald-expansion`**. Bare `make` still builds Emerald. FireRed is always `make firered`.

- Install and build the engine: [INSTALL.md](INSTALL.md)
- Expansion feature list: [FEATURES.md](FEATURES.md)
- Docs: [pokeemerald-expansion documentation](https://rh-hideout.github.io/pokeemerald-expansion/)
- Contributing to upstream: [CONTRIBUTING.md](CONTRIBUTING.md)
- Community: [ROM Hacking Hideout Discord](https://discord.gg/6CzjAG6GZk)
