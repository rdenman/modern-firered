# 0066 — Named QA warps and movement dump for test-story

- **Type:** tech
- **Status:** Accepted
- **Date:** 2026-10-10
- **Story:** —
- **ME reference:** —
- **Expansion config:** —

## Context

test-story kept dying in the Pallet bedroom on overworld stories (autorun, Surf, bike). Manual verification said “walk Route 1” / “warp to water.” The agent used **Fly to map…** (city names) because **Warp to map warp…** is a numeric group/num/warp picker, and **Route 1 has zero warp events**. Fly at 4× lands on ocean gaps. Movement was scored from dash sprites, which are unreadable at 4×. Options dump already worked; motion had no dump.

## Decision

- Add **Modern FireRed → QA scenes…** with coordinate warps (not warp-event IDs): Route 1, Pallet town, Bedroom off the NES, Route 19 water (surfable tile so arrival is surfing), Viridian Center. Warps set `FLAG_SYS_B_DASH`. **Mount Mach bike** gives `ITEM_BICYCLE` and mounts. **Toggle shoes** for the shoes bit.
- Add **Movement dump** (`=== MF movement dump ===`): `wantRun` / `wantRunB` / `wantSurf` / `shoes` / avatar flags / map. test-story must use this, not sprites.
- Harness `reachDebug()`. Ban Fly, Pallet stairs, and walking from the bedroom unless the story *is* that UI.
- implement-story Manual verification must name the QA row and dump line.

## Alternatives considered

- Teach the agent to Fly better (`mapMove`) — rejected; still vision-heavy and Route 1 still has no warp id.
- Native pokemon-agent RAM `/state` — rejected; FireRed reader is a stub and expansion offsets differ.
- HTTP façade on the WASM harness — useful later; does not replace named warps + dumps.

## Consequences

- Overworld QoL stories (S59-style) can pass without playing Kanto.
- Add more QA rows when a story needs a new map (e.g. a gym tile) rather than telling the agent to walk there.
- Release builds still compile the QA table out with the rest of the MF debug menu (`NDEBUG`).
