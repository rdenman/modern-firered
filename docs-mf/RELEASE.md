# Release builds and patches

S68. Product ROM is FireRed. **Never attach or commit a `.gba`.** Ship a BPS against a dump the player already owns.

## Build a release ROM (maintainers)

Do **not** run bare `make release` — that still builds Emerald. Always:

```bash
make firered RELEASE=1 -j$(sysctl -n hw.ncpu)
```

Output: `pokefirered-release.gba` and `pokefirered-release.elf` (gitignored).

This defines `RELEASE` / `NDEBUG`: overworld/battle debug menus compile out (`DISABLED_ON_RELEASE`), Quickstart is forced off, `src/mf_debug.c` is Cancel-only.

Confirm debug is gone:

```bash
python3 tools/mf/check_release_rom.py
```

That fails if `Debug_ShowMainMenu`, QA warps, or `CreateQuickstartHud` are still in the ELF.

## Patch (BPS)

Vanilla base: **Pokémon FireRed (USA, Rev 0)** / `BPRE-0`

| | |
| --- | --- |
| Size | 16777216 |
| SHA-1 | `41cb43a279208d5e14ee8ae47285c6bdae16dd85` |

Create (you supply the dump; the script refuses `--expect-vanilla` if the hash is wrong):

```bash
python3 tools/mf/bps.py create \
  --source /path/to/firered-usa-rev0.gba \
  --target pokefirered-release.gba \
  --output dist/modern-firered-1.0.0.bps \
  --expect-vanilla
```

Players apply with [Floating IPS](https://github.com/Alcaro/Flips) or:

```bash
python3 tools/mf/bps.py apply \
  --patch modern-firered-1.0.0.bps \
  --source /path/to/firered-usa-rev0.gba \
  --output pokefirered.gba
```

Open `pokefirered.gba` in mGBA. Do not redistribute that file.

Self-test (no ROMs): `python3 tools/mf/bps.py self-test`

## GitHub tags

Push a tag `v1.0.0` (or `vMAJOR.MINOR.PATCH`). Workflow [`.github/workflows/release.yml`](../.github/workflows/release.yml):

1. `make firered RELEASE=1`
2. `check_release_rom.py` + BPS self-test
3. Uploads **SHA-256 of the ROM** and changelog excerpt — **not** the ROM
4. BPS is produced on a machine that has the vanilla dump, then attached to the GitHub Release by hand (GitHub secrets cannot hold a 16MB ROM)

## Player notes

- Features: [PLAYER_FEATURES.md](./PLAYER_FEATURES.md)
- Changelog: [CHANGELOG.md](./CHANGELOG.md)
