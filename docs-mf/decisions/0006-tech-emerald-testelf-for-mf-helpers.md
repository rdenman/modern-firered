# 0006 — Emerald TESTELF for MF pure helpers; FR wiring stays manual

- **Type:** tech
- **Status:** Accepted
- **Date:** 2026-09-11
- **Story:** S06
- **ME reference:** —
- **Expansion config:** —

## Context

S06 needs a permanent place for Modern FireRed unit tests and a clear split between what `make check` can prove and what must be played in mGBA. Expansion’s runner (`TESTELF`) is Emerald-targeted; CI already runs that path (ADR 0002). Building a second FireRed TESTELF would duplicate tooling and fight upstream.

## Decision

1. Keep using Emerald `make check` as the only automated suite.
2. Put MF tests under `test/modern_firered/` with names prefixed `MF:` so they filter cleanly.
3. Unit-test pure `mf_*` helpers there; cover FR-only gameplay wiring with the committed manual QA template (`docs-mf/manual-qa-template.md`).
4. Document the split in `docs-mf/TESTING.md`. Do not treat `make firered check` as supported.

## Alternatives considered

- FireRed-targeted TESTELF / CI job — rejected; high merge and maintenance cost for little gain over manual QA on FR wiring.
- Host-side C unit tests outside the ROM — rejected; would diverge from expansion’s macros, RNG, and types.
- Skip automated MF tests until rules exist — rejected; S06’s goal is to lock the path before Phase 2.

## Consequences

- Rules math, accessors, and randomizer mapping should stay Emerald-safe so TESTELF links them.
- Gameplay-wiring stories (Phases 4–9) must fill the manual QA template; green `make check` alone is not enough.
- Upstream merges that change `test/` discovery still pick up `test/*/*.c` — no Makefile fork required unless RHH narrows the wildcard.
