# 0004 — MF_ config namespace via global.h

- **Type:** tech
- **Status:** Accepted
- **Date:** 2026-09-11
- **Story:** S04
- **ME reference:** —
- **Expansion config:** —

## Context

Every later story needs one obvious place for Modern FireRed compile-time knobs, isolated from upstream `include/config/*.h` so RHH merges do not fight our toggles. We also need kill-switches for unfinished subsystems (rules engine, randomizer, Nuzlocke, Options+) without inventing player-facing `#if` forks (ADR 0003).

## Decision

- Own all Modern FireRed compile-time configs in `include/config/modern_firered.h` under the `MF_` prefix, with `MF_VERSION` as a stamp for this header’s contract.
- Include that header from **`include/global.h`** immediately after `config/general.h`, via `#include "config/modern_firered.h"`. One upstream edit site; every game TU that includes `global.h` sees `MF_*`.
- Do **not** include it from `general.h`: `tools/mid2agb` and some asm preprocess pipelines pull `general.h` without a usable `-I include` / including-file search path, so either include form breaks one of those consumers.
- Ship four master switches defaulting to `TRUE`: `MF_RULES_ENGINE`, `MF_RANDOMIZER`, `MF_NUZLOCKE`, `MF_OPTIONS_PLUS`. Each is wrapped in `#ifndef` so a build can override via `MF_CPPFLAGS='-DMF_…=0'` (Makefile hook) without editing the file or replacing `CPPFLAGS`.
- Document naming and override usage in `docs-mf/UPSTREAM.md`.
- Add a one-line `MF_CPPFLAGS` append in the Makefile so overrides keep `-I include` and other expansion flags.
## Alternatives considered

- Include from `general.h` with `"config/modern_firered.h"` — rejected; `tools/mid2agb` includes `general.h` by relative path without `-I include`, so the nested `config/` include fails tool builds.
- Include from `general.h` with `"modern_firered.h"` (same directory) — rejected; the asm preprocess pipe (`arm-none-eabi-cpp` on stdin) fails to resolve that form for `data/sound_data.s`.
- Scatter `MF_*` into existing upstream config headers — rejected; maximizes merge conflict surface.
- Default master switches `FALSE` until subsystems land — rejected; later stories should compile features in by default and only flip a switch to quarantine breakage.
- No `#ifndef` overrides — rejected; S04’s “build with each switch off” test and local triage are cheaper with command-line overrides.
- Document `CPPFLAGS+=-DMF_…=0` — rejected after verification; command-line `CPPFLAGS` replaces the Makefile’s value and drops `-I include`, breaking the build. `MF_CPPFLAGS` via `override CPPFLAGS +=` is the safe hook.

## Consequences

- After upstream rewrites `global.h` near the early config includes, re-apply the `#include "config/modern_firered.h"` line (called out in `UPSTREAM.md`).
- After upstream rewrites the Makefile `CPPFLAGS` block, re-apply the `MF_CPPFLAGS` hook (near the other Modern FireRed Makefile note for `mf_%.o`).
- Subsystem code must wrap behind the matching `MF_*` master switch; player options still go through runtime accessors.
- `MF_VERSION` is not the save rules version — S12 owns that field separately.
- TUs that only include `config/general.h` (e.g. mid2agb) do not see `MF_*`; that is intentional — those tools do not need Modern FireRed switches.
