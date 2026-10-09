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

Turn each bullet into a case: setup, action, expected. Prefer **debug menu** (warp, give, cheat start, inspector dump) over walking Kanto.

## 2. Harness lifecycle

Port **8765**. Use **uv**, not raw `python3`.

```bash
# already up?
lsof -iTCP:8765 -sTCP:LISTEN

# start (background). Remember the PID.
tools-mf/harness/run.sh
```

`run.sh` fetches vendor WASM if missing and runs unbuffered `python3 -u server.py` (`PYTHONUNBUFFERED=1`). Ready when stdout has `MF harness: http://127.0.0.1:8765/` **or** `lsof` shows the port — do not wait forever on stdout alone if you started an older `run.sh`.

- If **you** started it → **kill that PID when the report is done** (success or fail).
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
await h.boot()
await h.pulse('select')        // 200ms default
await h.openDebug()            // hold R, pulse Start, release R
await h.pulse('a')             // dump / confirm; then scrape
const hit = h.logs.filter((t) => /=== MF /.test(t))
```

Buttons: `a` `b` `start` `select` `up` `down` `left` `right` `l` `r`.

Pulse-and-screenshot. Never hold a button for multiple seconds. If Select on title does nothing, pulse again (200ms) — do **not** grep `mgba.js` / `_buttonPress`.

**Lists at 4×:** default 200ms `up`/`down`/`left`/`right` key-repeats and skips rows. Use `pulse('down', 50, 350)` (and screenshot before A) on debug and inspector lists. Leave 200ms for title Select.

**Logs:** `logs` caps at 4000 and drops from the front. GBA BIOS/DMA lines are **not** stored (they used to evict MF dumps in seconds). `waitLog(re, ms, { fromIndex: logs.length })` is still racy if the buffer wrapped — scrape `/=== MF /` **after** `pulse('a')`, not during a 30ms A-hold. Do not treat leftover BIOS/DMA text as failure.

### Reach overworld (when the plan needs it)

1. `boot()` → pulse `start` until **PRESS START**. If Continue/New Game, pulse `b`.
2. Pulse `select` (Quickstart).
3. Rules defaults: `up`+`a` on **NEXT** until **SAVE**, `a`, confirm `a`. Pallet bedroom (PC) = success. Confirm may skip by at 4×.
4. Quickstart faces the NES. **Do not mash A** (it plays the NES). `pulse('down')` once off the furniture, then `openDebug()`.

Quickstart’s start menu often has **no POKéMON** (Oak never ran). For party/summary cases: **Utilities → Cheat start**, or **Give X → Pokémon (Basic)**.

### Debug map (R+Start)

Main: Utilities, PC/Bag, Party, Give X, Player, Scripts, Trainers, Encounters, Flags & Vars, Sound, ROM Info, **Modern FireRed**, Cancel. MF is near the **bottom** — pulse `down` until the screenshot shows it, don’t count blindly.

| Need | Path |
| --- | --- |
| Rules dump | Modern FireRed → Rules inspector → Dump (mGBA) → assert `=== MF rules dump ===` |
| Warp / fly | Utilities → Fly to map… **or** Warp to map warp… |
| Items / money | Give X → Give item XYZ… / Max Money |
| Pokémon | Give X → Pokémon (Basic) |
| Badges / fly flags | Flags & Vars → All badges / Fly Flags |
| Skip early-game lock | Utilities → Cheat start |
| Set a rule | Modern FireRed → Rules inspector → page → A to cycle |

Oracle for rules: dump logs, not eyeballing the inspector. `docs-mf/DEBUG.md` for MF submenu rows.

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
- Multi-second button holds or dropping to 1×
- Treat BIOS/DMA log spam as failure
- Commit `pokefirered.gba` or `vendor/mgba.*`
