# 0001 — Track `.python-version` and document the FireRed toolchain in `BUILDING.md`

- **Type:** tech
- **Status:** Accepted
- **Date:** 2026-09-11
- **Story:** S01
- **ME reference:** —
- **Expansion config:** —

## Context

S01 requires a reproducible FireRed build with a pinned toolchain. Upstream already tracks `.python-version` (`3.12`). Install steps live in `INSTALL.md` / `docs/install/`, but they default to Emerald (`make`) and do not pin the compiler version we verified on macOS or call out the Emerald↔FireRed object-file hazard clearly for this fork.

## Decision

- Keep the tracked `.python-version` pin at **3.12**; do not untrack it.
- Add repo-root **`BUILDING.md`** as the Modern FireRed build entry point: FireRed command, toolchain versions (devkitARM / gcc 16.1.0, no agbcc, Python 3.12), and the `make clean` switch hazard.
- Leave upstream `INSTALL.md` intact for OS install; link to it from `BUILDING.md` instead of forking those guides.

## Alternatives considered

- Document only in `README.md` / `AGENTS.md` — rejected; agents already have a short build blurb, but humans need a durable pin table without bloating either file.
- Vendor or hard-pin a specific `dkp-pacman` package revision in-repo — rejected; pinning the observed `arm-none-eabi-gcc` version is enough for agents and CI to compare against.
- Re-enable `agbcc` for “classic” builds — rejected; expansion deprecated it and Modern FireRed only supports the modern toolchain.

## Consequences

- Fresh clones follow `BUILDING.md` after OS install.
- S02 can reuse the same command in CI.
- Future toolchain bumps should update the pin table in `BUILDING.md` (and this ADR’s pin note if the change is load-bearing).
