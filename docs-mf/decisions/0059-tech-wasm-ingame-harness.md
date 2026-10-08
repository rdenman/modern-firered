# 0059 — Local WASM mGBA harness for agent in-game QA

- **Type:** tech
- **Status:** Accepted
- **Date:** 2026-10-08
- **Story:** —
- **ME reference:** —
- **Expansion config:** —

## Context

Story acceptance is mostly overworld/menu wiring that `make check TESTS='MF:'` (Emerald TESTELF) cannot see. Desktop mGBA is manual. We needed a way for a **local** Cursor agent to boot `pokefirered.gba` and read debug dumps.

A cloud agent cannot use this: it has no laptop `localhost`, no gitignored ROM, and no uncommitted working tree.

## Decision

Ship `tools-mf/harness/`: Python static server with COOP/COEP, `GET /rom` → repo-root `pokefirered.gba`, pinned local `@thenick775/mgba-wasm` (not CDN), `window.__mfHarness` (pulse 200ms, `openDebug`, `waitLog`). Default speed **4×**. Agents screenshot the canvas and assert `__mfHarness.logs` (Dump mGBA), not the DOM log pane.

Do not copy the ROM on compile. Do not change the Makefile. Do not commit `.gba` or `vendor/`.

## Alternatives considered

- HTML macro buttons as the agent UI — rejected; canvas is opaque to snapshots; CDP on the JS API worked in spikes.
- Always 32× — rejected; open-loop holds skipped the title; 4× + 200ms pulses reached Pallet and debug.
- Native mGBA + Lua — fallback if WASM isolation had failed in the IDE browser; it did not.
- Cloud agent + CI ROM artifact — extra toolchain and no inner-loop uncommitted testing.

## Consequences

- Local in-game QA can follow `.agents/skills/test-story`.
- `videoFrameEndedCallback` never fired in spikes; pulse is wall-clock, not GBA frames.
- Select on title needs ~200ms; 80ms often misses.
- BIOS/DMA spam is huge; keep a large JS log buffer and filter the visible pane.
