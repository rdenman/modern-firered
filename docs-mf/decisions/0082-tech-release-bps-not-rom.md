# 0082 — Ship BPS patches, never the ROM; tag-only release CI

- **Type:** tech
- **Status:** Accepted
- **Date:** 2026-10-11
- **Story:** S68
- **ME reference:** —
- **Expansion config:** `RELEASE` / `NDEBUG`; `DISABLED_ON_RELEASE`; `QUICKSTART` false when `RELEASE`

## Context

S68 needs a player distribution. Uploading `pokefirered-release.gba` would be distributing a copyrighted work. Bare `make release` still builds Emerald (Makefile default). ADR 0002 dropped an always-on release CI job; we still want a tag-time check that debug/Quickstart are gone.

## Decision

1. **Canonical command:** `make firered RELEASE=1` (not bare `make release`).
2. **Artifact:** a BPS against FireRed USA Rev 0 (`BPRE-0`, SHA-1 `41cb43a279208d5e14ee8ae47285c6bdae16dd85`). Tool: `tools/mf/bps.py`. GitHub secrets cannot hold a 16MB dump, so CI does not create the BPS.
3. **Tag workflow** `.github/workflows/release.yml` on `v*`: build the release ROM, `check_release_rom.py` (nm must not see debug/Quickstart symbols), upload SHA-256 + docs, **never** the `.gba`.
4. **Player stamp** `MF_RELEASE_VERSION_*` is **1.0.0** for this cut.

## Alternatives considered

- **UPS instead of BPS** — rejected; BPS carries source/target checksums so a wrong dump fails closed.
- **CI uploads the ROM to a private release** — rejected; still a ROM on GitHub.
- **Vanilla ROM as a GitHub secret** — rejected; secret size cap.
- **Keep shipping debug Quickstart in “release”** — rejected; the story and `DISABLED_ON_RELEASE` exist to strip it.

## Consequences

- Maintainers who own a legal dump run `bps.py create --expect-vanilla` and attach the `.bps` to the GitHub Release by hand.
- `src/quickstart.c` is wrapped in `#if QUICKSTART` so release ELFs do not keep HUD/entry symbols for `check_release_rom.py`. `arm-none-eabi-nm` is resolved via `DEVKITARM` / `/opt/devkitpro/devkitARM` when it is not on `PATH`.
- Next `RHH/master` merge may revive upstream’s Emerald `release` job in `build.yml`; keep it off (ADR 0002) and keep our tag workflow.
- Docs: [`docs-mf/RELEASE.md`](../RELEASE.md).
