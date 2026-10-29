# Modern FireRed — decision records

Short ADRs for non-trivial tech / product / UX choices. Upstream’s `docs/` is their mdbook; keep ours under `docs-mf/` to avoid merge collisions.

## Template

Create `NNNN-<type>-<slug>.md` with the next zero-padded `NNNN`. Types: `tech` | `product` | `ux`.

```markdown
# NNNN — <Title>

- **Type:** tech | product | ux
- **Status:** Accepted
- **Date:** YYYY-MM-DD
- **Story:** S## (or —)
- **ME reference:** <ME file or option> (or —)
- **Expansion config:** <macro + default> (or —)

## Context
Why a decision was needed; the forces at play.

## Decision
What was decided, stated plainly.

## Alternatives considered
- <option> — why not chosen.

## Consequences
Trade-offs, follow-ups, upstream-merge implications, and any affected stories or future work.
```

Append a row to the index below when you add a record.

## Index

| ID | Type | Title | Story | Date |
| -- | ---- | ----- | ----- | ---- |
| 0001 | tech | Track `.python-version` and document the FireRed toolchain in `BUILDING.md` | S01 | 2026-09-11 |
| 0002 | tech | Slim CI to FireRed + Emerald test suite | S02 | 2026-09-11 |
| 0003 | tech | Runtime rules layer over compile-time expansion configs | S03 | 2026-09-11 |
| 0004 | tech | MF_ config namespace via global.h | S04 | 2026-09-11 |
| 0005 | tech | MF debug submenu via public DebugMenuOption | S05 | 2026-09-11 |
| 0006 | tech | Emerald TESTELF for MF pure helpers; FR wiring stays manual | S06 | 2026-09-11 |
| 0007 | tech | Keep GEN_LATEST battle baseline (no Kanto downgrade) | S07 | 2026-09-11 |
| 0008 | product | Species families Gen 1–3 + cross-gen Kanto evolutions | S08 | 2026-09-11 |
| 0009 | ux | Summary screen IV/EV: always-on, raw values, tileset labels | S09 | 2026-09-12 |
| 0010 | product | Item QoL baseline: reusable TMs, held Exp Share, deferred mint shops, Gen4+ Sitrus | S10 | 2026-09-12 |
| 0011 | product | Battle & OW speed: no-slide intro, wait×8, Options text intact, Gen5+ poison | S11 | 2026-09-12 |
| 0012 | tech | Store ModernRules in SaveBlock3 | S12 | 2026-09-12 |
| 0013 | tech | Rule accessor null defaults = Phase 1 baseline | S13 | 2026-09-12 |
| 0014 | product | Classic / Modern / Custom gamemode preset tables | S14 | 2026-09-12 |
| 0015 | product | Rule immutability & mid-run lock semantics | S15 | 2026-09-12 |
| 0016 | tech | Deterministic randomizer seed service (not OT ID) | S16 | 2026-09-12 |
| 0017 | tech | Debug rules inspector via paged mf_debug menus | S17 | 2026-09-12 |
| 0018 | tech | Data-driven FR rules menu shell | S18 | 2026-09-12 |
| 0019 | tech | Rules menu after NewGameInitData, before overworld | S19 | 2026-09-12 |
| 0020 | ux | Gamemode page: ME order, no EXTRA LEGEND., Custom-gated edits | S20 | 2026-09-12 |
| 0021 | ux | Features page: FR subset and deliberate ME omissions | S21 | 2026-09-12 |
| 0022 | ux | Nuzlocke page: tier packing, Easy mini-mode gate | S22 | 2026-09-13 |
| 0023 | ux | Difficulty page: ME order, Yes/No item bans, clear PLAYER IVs labels | S23 | 2026-09-13 |
| 0024 | ux | Challenges page: ME order, monotype Off+types, gated PC heal / Mirror Thief | S24 | 2026-09-13 |
| 0025 | ux | Randomizer page: master gate, nested ME conditions, live clear/seed | S25 | 2026-09-13 |
| 0026 | ux | SAVE confirm + start-menu rules viewer | S26 | 2026-09-13 |
| 0027 | tech | Runtime Fairy retypes via GetSpeciesType fallback table | S27 | 2026-09-13 |
| 0028 | tech | Dual static type charts + modern-type overlay | S28 | 2026-09-14 |
| 0029 | tech | Classic Gen-3 base-stat fallback table | S29 | 2026-09-25 |
| 0030 | tech | FRLG classic movepool dual tables | S30 | 2026-09-25 |
| 0031 | tech | Runtime Gamemode item & ability behavior gates | S31 | 2026-09-26 |
| 0032 | tech | Runtime ENCOUNTERS mode via FR modern wild tables | S32 | 2026-09-26 |
| 0033 | tech | Runtime SHINY CHANCE tiers over SHINY_ODDS | S33 | 2026-09-27 |
| 0034 | tech | Wild ITEM DROP via battle-end held-item grant | S34 | 2026-09-27 |
| 0035 | product | Solo trade evolutions always-on (bag use + Celadon stock) | S69 | 2026-09-27 |
| 0036 | product | Per-mapsec Nuzlocke encounter locking | S35 | 2026-09-27 |
| 0037 | product | Nuzlocke faint handling: Cemetery / Release after battle | S36 | 2026-09-29 |
| 0038 | product | Nuzlocke DUPES & SHINY clauses (ME parity, no re-roll) | S37 | 2026-09-29 |
| 0039 | product | Nuzlocke forced nicknaming (ME NICKNAMES) | S38 | 2026-09-29 |
| 0040 | product | Nuzlocke difficulty tier bundles (Off/Easy/Normal/Hardcore) | S39 | 2026-09-30 |
| 0041 | tech | Party limit: enforce on add paths without capping CalculatePlayerPartyCount | S40 | 2026-09-30 |
| 0042 | tech | Runtime Kanto level caps | S41 | 2026-09-30 |
| 0043 | tech | CI test gate is MF: tests only | — | 2026-09-30 |
| 0044 | product | EXP multiplier and hard-mode EXP | S42 | 2026-10-01 |
| 0045 | product | Scaled EXP toggle | S70 | 2026-10-02 |
| 0046 | product | Battle item bans: Bag shown-and-refused, balls exempt | S43 | 2026-10-02 |
| 0047 | product | Runtime IV/EV scaling | S44 | 2026-10-03 |
| 0048 | product | Catch rate multiplier and escape restrictions | S45 | 2026-10-03 |
| 0049 | product | Poké Center ban, PC heal, and shop price scaling | S46 | 2026-10-04 |
| 0050 | product | Evolution limit (Off / First / All) | S47 | 2026-10-04 |
| 0051 | product | Monotype: either type, catch refuse, deterministic Oak remap | S48 | 2026-10-04 |
| 0052 | product | BST equalizer scales totals, after modern stats | S49 | 2026-10-05 |
| 0053 | product | Mirror Mode copies the foe onto the player; Thief keeps it | S50 | 2026-10-05 |
| 0054 | product | Species mapping: S08 pool, evo-stage+BST, HM preserve | S51 | 2026-10-05 |
| 0055 | tech | Wild tables remap at read/create; statics skip Oak | S52 | 2026-10-05 |
| 0056 | product | Trainer remap before IV/EV; Oak trio via S51 | S53 | 2026-10-05 |
| 0057 | product | Themed trainer parties: per-trainer hashed type | S74 | 2026-10-06 |
| 0058 | product | Kanto gym / E4 / Champion theme groups | S75 | 2026-10-08 |
| 0060 | product | Level-up remap + form-locked ability ban; TMs stay vanilla | S54 | 2026-10-08 |
| 0061 | product | Evolution remap + Kanto-reachable methods | S55 | 2026-10-09 |
| 0062 | product | Species types remap last; chart is a baked axis permutation | S56 | 2026-10-09 |
| 0063 | product | Item remap with key/HM pins; chaos salt only on encounters | S57 | 2026-10-09 |
| 0064 | tech | Options+ paged shell and SaveBlock3 extras | S58 | 2026-10-09 |
| 0065 | product | Autorun inverts B on foot and Surf, not on bike | S59 | 2026-10-09 |
| 0066 | tech | Named QA warps and movement dump for test-story | — | 2026-10-10 |
| 0067 | ux | Even Faster Joy Off / Fast / On on FireRed Centers | S60 | 2026-10-10 |
| 0068 | product | Battle/intro speed Options+ (wait, slide, skip, ball) | S61 | 2026-10-11 |
| 0069 | ux | STAB (+) and SEEN effectiveness/type icons | S62 | 2026-10-11 |
| 0070 | ux | Bag sort, cursor memory, run prompt, and units | S63 | 2026-10-11 |
| 0071 | product | B-to-Run always-on via expansion config | S71 | 2026-10-11 |
| 0072 | product | Catch-to-box, no Gen 7 party-swap prompt | S72 | 2026-10-11 |
| 0073 | product | Vanilla FRLG mart prices and Escape Rope | — | 2026-10-11 |
| 0074 | product | Gen 3 trainer EXP multiplier | — | 2026-10-11 |
| 0075 | product | Gen 3 gym badge stat boosts | — | 2026-10-11 |
| 0076 | product | Skip the RHH expansion intro | — | 2026-10-11 |
| 0077 | tech | Rules save migration and unrecognized versions | S64 | 2026-10-11 |
| 0078 | product | Rule interaction matrix: one forbid, otherwise warn | S65 | 2026-10-11 |
| 0079 | product | Celio gives MysticTicket and AuroraTicket after the Sapphire | S73 | 2026-10-11 |
| 0080 | product | Playthrough QA matrix configs and Surge checkpoint | S66 | 2026-10-11 |
| 0081 | ux | Version stamp in Options+; credits there and in README | S67 | 2026-10-11 |
| 0082 | tech | Ship BPS patches, never the ROM; tag-only release CI | S68 | 2026-10-11 |
| 0083 | product | Gen 3 item randomizer pool; megas off | S57 | 2026-10-11 |
| 0084 | product | Regional forms behind a Features toggle | — | 2026-10-11 |
| 0085 | product | Nuzlocke forced nicknaming on gift Pokémon | — | 2026-10-17 |
| 0086 | ux | Naming keyboard stays uppercase unless the player swaps case *(superseded by 0096)* | — | 2026-10-17 |
| 0087 | product | In-battle Select move rearrangement | — | 2026-10-17 |
| 0088 | product | Mart stock stays vanilla under item randomizer | — | 2026-10-17 |
| 0089 | ux | Fight-menu move details on Start, no L/R hint | — | 2026-10-17 |
| 0090 | ux | Effectiveness as green/red move names | — | 2026-10-17 |
| 0091 | ux | Gift nickname/obtain text uses the remapped species | — | 2026-10-17 |
| 0092 | product | Item randomizer starts before the Pokédex | — | 2026-10-17 |
| 0093 | product | Rival starter is the Oak ball; balancing never leaves its evo stage | S53 | 2026-10-17 |
| 0094 | ux | Gen 3 GBA species sprites as FireRed default | — | 2026-10-17 |
| 0095 | product | Item randomizer shuffles the Gen 3 pool | — | 2026-10-17 |
| 0096 | ux | Naming keyboard auto-lowercases after the first capital | — | 2026-10-17 |
| 0097 | ux | Title Case menu option labels | — | 2026-10-17 |
| 0098 | ux | Title Case FireRed labels and proper nouns | — | 2026-10-17 |
| 0098 | product | Item randomizer rolls per chest, collisions allowed | — | 2026-10-17 |
| 0099 | product | Fossils stay vanilla under item randomizer | — | 2026-10-17 |
| 0100 | ux | Start-menu Rules is debug-only | — | 2026-10-21 |
| 0101 | product | Item randomizer keeps TMs as TMs | — | 2026-10-21 |
| 0102 | ux | Gift TM dialogue names the dest TM and move | — | 2026-10-21 |
| 0102 | ux | Show Hidden Power’s IV type on the forget-move screen | — | 2026-10-21 |
| 0103 | product | Celadon stocks Kanto-reachable cross-evo items | — | 2026-10-21 |
| 0104 | product | National Dex when a run can produce non-Kanto species | — | 2026-10-21 |
| 0105 | product | TM randomizer remaps TM and tutor dest moves | — | 2026-10-21 |
| 0105 | product | Only Oak’s Poké Ball grants stay vanilla under item random | — | 2026-10-21 |
| 0106 | product | Forget HMs like any other move | — | 2026-10-24 |
| 0107 | product | Celadon stocks Moon and Sun Stones | — | 2026-10-24 |
| 0108 | product | Stone evolutions keep learning pre-evo moves | — | 2026-10-24 |
| 0109 | product | Poké Vial: Features toggle, badge charges, Nuzlocke, no-Center refill | — | 2026-10-24 |
| 0110 | product | Overworld L toggles Autorun; drop L=A | — | 2026-10-24 |
| 0111 | product | Bag HMs: field use without teaching + challenge lead overwrite | — | 2026-10-24 |
| 0112 | ux | Randomized TM bag descriptions follow dest | — | 2026-10-26 |
| 0113 | product | Bag pocket capacities (Items 72 / Balls 28 / Berries 68) | — | 2026-10-28 |
