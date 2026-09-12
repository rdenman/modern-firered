# Debug & fast-iteration harness

How to reach new-game and overworld debug flows in under 15 seconds on a FireRed (`pokefirered.gba`) build. Non-release only — `make release` turns these off via `DISABLED_ON_RELEASE` / `QUICKSTART`.

Configs confirmed for this fork:

| Knob | Header | Dev default | How to open |
| ---- | ------ | ----------- | ----------- |
| Overworld debug menu | `include/config/debug.h` → `DEBUG_OVERWORLD_MENU` | `DISABLED_ON_RELEASE` (on) | Hold **R**, press **Start** in the overworld |
| Battle debug menu | same → `DEBUG_BATTLE_MENU` | `DISABLED_ON_RELEASE` (on) | **Select** in battle |
| Quickstart | `include/config/quickstart.h` → `ENABLE_QUICKSTART` | `TRUE` (forced off on release) | **Select** on the FireRed title screen (HUD icon top-right) |

Modern FireRed’s own submenu lives under overworld debug → **Modern FireRed…** (`src/mf_debug.c`):

| Entry | What it does |
| ----- | ------------ |
| **Rules inspector…** | Paged view of every `ModernRules` field (Meta / Gamemode / Features / Randomizer / Nuzlocke / Difficulty / Challenges). **A** toggles bools or cycles multi-value fields; locked rules auto-enable the S15 session unlock override. **Dump (mGBA)** prints the full struct to the mGBA console (ME `PrintTXSaveData` spirit). |
| **Cancel** | Close |

Under `make release` (`NDEBUG`) the inspector pages compile out — the MF submenu is Cancel-only, and the overworld debug menu itself is off via `DISABLED_ON_RELEASE`.

## Checklist — cold boot → overworld debug (< 15s)

1. Open `pokefirered.gba` in mGBA (File → Load ROM…), or restart emulation if already loaded.
2. Wait for the title screen (Skip intro with A/Start if the copyright/Game Freak screens are still playing).
3. Press **Select** (Quickstart). Naming / Oak intro are skipped; you land in the overworld (Pallet bedroom).
4. Hold **R** and press **Start**. The expansion debug menu opens.
5. Open **Modern FireRed…** → **Rules inspector…**. Browse a page (e.g. Gamemode), confirm values, toggle one with **A**, and optionally **Dump (mGBA)** (Tools → View Logs). **B** backs up a level; Cancel closes.

Optional: Utilities → Cheat Start (or FRLG equivalent) if you need badges / party for a later story’s scenario.

## Checklist — save state just before Oak’s speech

Use this when iterating on the start-of-run rules menu (Phase 3) without replaying the title every time.

1. Cold boot `pokefirered.gba` in mGBA.
2. On the title screen press **A** or **Start** (not Select) → Continue/New Game → **New Game**.
3. Mash through the controls guide / Pikachu intro until Professor Oak appears and begins speaking (first Oak dialogue box, before gender).
4. mGBA: **File → Save State → Slot N** (or keyboard shortcut for a save-state slot). Name/note the slot “pre-Oak”.
5. To re-test: **File → Load State → Slot N**. You should be back at that Oak line within a second.

After the rules menu is wired into Oak speech, re-capture the state one screen *before* that menu so you can enter it cleanly each run.

## Checklist — confirm release build strips debug

```bash
make firered RELEASE=1 -j$(sysctl -n hw.ncpu)
# equivalent: make release firered -j$(sysctl -n hw.ncpu)
```

On the resulting ROM: title Select must not Quickstart; overworld R+Start must not open the debug menu.

## Related

- Build command: [`BUILDING.md`](../BUILDING.md)
- Upstream merge notes for debug hook sites: [`UPSTREAM.md`](./UPSTREAM.md)
- Decision: `docs-mf/decisions/0005-tech-mf-debug-submenu-hook.md`
- Decision: `docs-mf/decisions/0017-tech-debug-rules-inspector.md`
