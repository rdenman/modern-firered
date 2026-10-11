---
name: test-story
description: >-
  Execute a story's Manual verification in the local WASM mGBA harness and
  report pass/fail only. Pair with implement-story in the same chat: after
  that skill's summary, run test-story. Use when the user says test-story,
  test this story, or asks to run in-game QA. Local agent only — not cloud.
  Do not edit code, stories, or this skill while testing.
---

# Test a Story

Local IDE agent only. **Read-only:** no source, config, story-status, or skill edits. Report pass/fail + friction.

Cloud agents cannot see `127.0.0.1` or gitignored `pokefirered.gba`.

## 1. Load the plan

In this order:

1. This chat’s latest **Manual verification** from `implement-story` (same conversation).
2. Else the user-named story’s **Acceptance** / **Tests** in `STORIES.md`.
3. If the section is `N/A` (tooling/docs/`make check` only): report N/A, do not boot the emulator.

Turn each bullet into a case: setup, action, expected.

**If a case needs a map, water, bike, or indoor tile and Manual verification does not name a QA row → BLOCK that case immediately.** Do not invent Fly, stairs, or walking Kanto.

Oracles: mGBA dump lines (`=== MF … ===`). **Never** pass/fail movement from walk vs dash sprites at 4×.

## 2. Harness lifecycle

Port **8765**. Use **uv**, not raw `python3`.

```bash
# already up?
lsof -iTCP:8765 -sTCP:LISTEN

# start (background). Remember the PID.
tools-mf/harness/run.sh
```

`run.sh` fetches vendor WASM if missing and runs unbuffered `python3 -u server.py` (`PYTHONUNBUFFERED=1`). Ready when stdout has `MF harness: http://127.0.0.1:8765/` **or** `lsof` shows the port — do not wait forever on stdout alone if you started an older `run.sh`.

- If **you** started it → when the report is done, kill the **listener** (`lsof -t -iTCP:8765 -sTCP:LISTEN`), not only the `run.sh` wrapper (`uv` often leaves `python3` bound).
- If it was **already** listening → reuse it, **do not kill**.
- ROM missing: `make firered -j$(sysctl -n hw.ncpu)` is allowed (build artifact only). Do not change C/headers/Makefile.

## 3. Drive the page

1. IDE browser → `http://127.0.0.1:8765/`
2. CDP `Runtime.evaluate` on `window.__mfHarness` only. Do not click the HTML pad.
3. Read **`window.__mfHarness.logs`**. Ignore the on-page `<pre>`.
4. Speed stays **4×**. Never `setFastForwardMultiplier(1)`.
5. Screenshot `#canvas` after pulses. `browser_snapshot` does not show the game.

```js
const h = window.__mfHarness
await h.reachDebug()           // boot + Quickstart + off NES + R+Start
// screenshot: debug list? if NES/Font Test, B until overworld, then openDebug()
// listMove down until "Modern FireRed…", A
await h.pulse('a')             // QA scenes… is the first MF row
const hit = h.logs.filter((t) => /=== MF /.test(t))
```

Buttons: `a` `b` `start` `select` `up` `down` `left` `right` `l` `r`. Helpers: `h.reachDebug()`, `h.listMove('down', n)` / `h.mapMove` (50ms tap, 350ms gap), `h.rulesNext()`, `h.quickstart()`, `h.openDebug()`.

Pulse-and-screenshot. Never hold a button for multiple seconds. Do **not** grep `mgba.js` / `_buttonPress`. Do not screenshot after the first title Select — call `quickstart()` then screenshot.

**D-pad at 4× (lists):** default 200ms `up`/`down`/`left`/`right` key-repeats. Use `listMove` / `pulse('down', 50, 350)` on debug lists. Screenshot the highlighted name before A. **A and B stay at the 200ms default.**

**Banned for QA (unless the story is that UI):** **Fly to map…**, Pallet stairs, walking Route 1 from the bedroom, BAG to use a bike. Route 1 has **no warp events** — numeric Warp cannot land there. Use **Modern FireRed → QA scenes…**.

**Nested menus:** `B` closes one submenu. Screenshot the parent before counting downs. **Do not openDebug while a message box is up** (NES, Font Test) — A until gone, or warp **Bedroom (off NES)**. Utilities: screenshot the row before A. **Font Test** and **Watch credits** are long scripts — never A those.

**Logs:** `logs` caps at 4000 and drops from the front. GBA BIOS/DMA lines are **not** stored (they used to evict MF dumps in seconds). `waitLog(re, ms, { fromIndex: logs.length })` is still racy if the buffer wrapped — scrape `/=== MF /` **after** `pulse('a')`, not during a 30ms A-hold. Do not treat leftover BIOS/DMA text as failure.

### Reach overworld (when the plan needs it)

Prefer **`await h.reachDebug()`** once, then screenshot.

If that misses: `boot()` → pulse `start` until PRESS START (Continue/New Game → `b`) → `quickstart()` → **once per page** `rulesNext()` until Pallet. **First overworld input is `pulse('down')`** (off the NES) → `openDebug()`. Do not take the stairs.

Quickstart’s start menu often has **no POKéMON**. Party/summary: **Utilities → Cheat start**, or **Give X → Pokémon (Basic)**.

### Debug map (R+Start)

Main: Utilities, PC/Bag, Party, Give X, Player, Scripts, Trainers, Encounters, Flags & Vars, Sound, ROM Info, **Modern FireRed**, Cancel. MF is near the **bottom** — pulse `down` until the screenshot shows **Modern FireRed…**, don’t count blindly.

| Need | Path |
| --- | --- |
| Outdoor run / autorun | Modern FireRed → QA scenes… → **Route 1 (run)** → Movement dump → `wantRun=` / `wantRunB=` / `shoes=` |
| Indoor (off NES) | QA scenes… → **Bedroom (off NES)** |
| Pallet outdoors | QA scenes… → **Pallet town** |
| Surf | QA scenes… → **Route 19 water** → Movement dump → `surf=1` `wantSurf=` |
| Bike (Mach, no invert) | QA scenes… → Route 1, then **Mount Mach bike** → dump `mach=1` (B must not change `wantRun` logic; bike ignores autorun) |
| Running shoes | QA scenes… → **Toggle shoes** or Flags → Toggle Running Shoes |
| Center | QA scenes… → **Viridian Center** |
| Rules dump | Rules inspector → Dump (mGBA) → `=== MF rules dump ===` |
| Options dump | Options dump → `=== MF options dump ===` |
| Movement dump | Movement dump → `=== MF movement dump ===` |
| Items / money | Give X → Give item XYZ… / Max Money |
| Pokémon | Give X → Pokémon (Basic) |
| Badges | Flags & Vars → All badges |
| Skip early-game lock | Utilities → Cheat start |
| Set a rule | Rules inspector → page → A to cycle |

Oracle: dump logs. `docs-mf/DEBUG.md` for row names.

## 4. Report (always)

Kill the server you started, then report. Do not patch the game to make a case pass.

```markdown
## Test-story report
- **Story:** S##
- **Overall:** PASS | FAIL | BLOCKED
- **Harness:** started + killed | reused existing :8765

### Cases
| # | Step | Result | Evidence |
| - | ---- | ------ | -------- |
| 1 | … | PASS/FAIL | screenshot and/or log line |

### Failures
- Case #: what you saw vs expected

### Friction
What took too long or had to be rediscovered (menu hunting, missed pulses, unclear Manual verification, missing debug shortcut). Be specific so we can bake it into this skill later.
- …
```

**Friction** is required even on PASS. Skip only if truly none (`none`).

## Don’t

- Edit code, `STORIES.md` status, ADRs, or this skill
- Cloud / Browser-agent against this URL
- Fly to map…, Pallet stairs, or sprite-based run/walk judgments
- Multi-second button holds or dropping to 1×
- Treat BIOS/DMA log spam as failure
- Commit `pokefirered.gba` or `vendor/mgba.*`
