# Poké Vial — Research & Planning Notes

Research notes for the optional **Poké Vial** in Modern FireRed (portable, charge-limited full-party heal). Implemented as **S76**; product rules: [`docs-mf/decisions/0109-product-pokevial.md`](docs-mf/decisions/0109-product-pokevial.md).

Primary source: **Inclement Emerald (IE)** by Buffel Saft, [BuffelSaft/pokeemerald](https://github.com/BuffelSaft/pokeemerald) `master` (cloned 2026-10-24). Author-facing copy is the [PokéCommunity thread (v1.13)](https://www.pokecommunity.com/threads/pok%C3%A9mon-inclement-emerald-a-decomp-difficulty-hack-version-1-13.457039/). IE has **no start-of-run toggle** for this item; it is always on.

---

## 1. Goal

A start-of-run option that, when On, gives the player a reusable Key Item that fully heals the party **out of battle**, with a small charge count refilled at Pokémon Centers — IE’s QoL, on **stock Kanto** (no new maps, no Route 111/133 nurse quests).

Accepted product rules: [`docs-mf/decisions/0109-product-pokevial.md`](docs-mf/decisions/0109-product-pokevial.md).

Out of scope unless separately approved: IE’s Chansey/“Blob” vial-upgrade quests, PSF’s bag dose-icon UI, Radical Red’s 6-charge / Select-menu variant.

---

## 2. Overall difficulty

| Piece | Difficulty | Main cost |
|---|---|---|
| Rules-menu toggle + accessor | Easy | One padding bit + `MF_RULES_VERSION` bump ([`docs-mf/SAVE_VERSIONING.md`](./docs-mf/SAVE_VERSIONING.md)) |
| Item table, icon, strings | Easy | New `ITEM_*`, graphics, Key Items pocket |
| Field use → `HealPlayerParty` | Easy | `ItemUseOutOfBattle_*` + charge check |
| Charge vars + Center refill | Easy–Medium | Nurse script (incl. Even Faster Joy path) |
| New-game grant | Easy | `AddBagItem` after rules lock if the toggle is On |
| Challenge interactions | Medium | No-Center Joy still refills; Nuzlocke skips fainted |
| Badge-scaled max charges | Easy | `max(1, ceil(badges/3))` cap 3; no new NPCs |
| Optional dose indicator in bag | Medium | New art states; not in IE |

Every behavior needs on/off paths so testing roughly doubles.

---

## 3. IE player-facing facts

Cited from IE source and Buffel Saft’s thread. Secondary sources are marked.

### What it is / what using it does

Reusable **Key Item**. Using it on the field plays `SE_USE_ITEM`, calls `HealPlayerParty()`, then decrements remaining charges by 1.

[`src/item_use.c:ItemUseOnFieldCB_PokeVial`](https://github.com/BuffelSaft/pokeemerald/blob/master/src/item_use.c) (IE):

```c
PlaySE(SE_USE_ITEM);
HealPlayerParty();
VarSet(VAR_POKE_VIAL_CHARGES, VarGet(VAR_POKE_VIAL_CHARGES) - 1);
DisplayItemMessageOnField(taskId, gText_UsedPokeVial, Task_CloseCantUseKeyItemMessage);
```

`HealPlayerParty` in IE restores **HP to max, all move PP (with bonuses), and status** for every slot in `gPlayerPartyCount`. There is **no egg skip** and **no “already healthy” check**. Fainted mons are revived (HP is written to max). Eggs get the same writes; an egg’s max HP is 0, so this is a no-op in practice.

[`src/script_pokemon_util.c:HealPlayerParty`](https://github.com/BuffelSaft/pokeemerald/blob/master/src/script_pokemon_util.c)

Use text: `"{PLAYER} used the Poké Vial.\nAll Pokémon were fully restored!"` — [`src/strings.c`](https://github.com/BuffelSaft/pokeemerald/blob/master/src/strings.c) `gText_UsedPokeVial`.

Author copy: “can be used out of battle to fully heal your entire party.” ([PC thread](https://www.pokecommunity.com/threads/pok%C3%A9mon-inclement-emerald-a-decomp-difficulty-hack-version-1-13.457039/))

### Where / when the player gets it

**Not** from Oak and **not** in the bag at frame 1. `NewGameInitData` does not give the item or set the vars ([`src/new_game.c:NewGameInitData`](https://github.com/BuffelSaft/pokeemerald/blob/master/src/new_game.c)).

The **first Pokémon Center nurse** who runs `Common_EventScript_PkmnCenterNurse` checks `checkitem ITEM_POKE_VIAL, 1`. If missing: rookie-trainer dialogue, `giveitem ITEM_POKE_VIAL, 1`, `VAR_POKE_VIAL_MAX_CHARGES = 1`, `VAR_POKE_VIAL_CHARGES = 1`, then explains and asks to rest.

[`data/scripts/pkmn_center_nurse.inc`](https://github.com/BuffelSaft/pokeemerald/blob/master/data/scripts/pkmn_center_nurse.inc) + [`data/text/pkmn_center_nurse.inc`](https://github.com/BuffelSaft/pokeemerald/blob/master/data/text/pkmn_center_nurse.inc) `gText_AreYouARookieTrainer` / `gText_ExplainPokeVial`.

In practice that is the first Center after Birch (Oldale), not Pallet.

### Key item vs regular / pocket / consumed

| Field | Value | Source |
|---|---|---|
| Name | `"Poké Vial"` (space, é) | [`src/data/items.h`](https://github.com/BuffelSaft/pokeemerald/blob/master/src/data/items.h) `[ITEM_POKE_VIAL]` |
| Constant | `ITEM_POKE_VIAL` (`LAST_TMHM_INDEX + 43`) | [`include/constants/items.h`](https://github.com/BuffelSaft/pokeemerald/blob/master/include/constants/items.h) |
| Pocket | `POCKET_KEY_ITEMS` | same item struct |
| Importance | `1` (key item; not dumped; registerable) | same |
| Price | `0` | same |
| Type | `ITEM_USE_FIELD` | same |
| Field func | `ItemUseOutOfBattle_PokeVial` | same |
| Battle usage / func | omitted → 0 / NULL | same; C designated init |
| Consumed | No (`RemoveBagItem` never called) | `ItemUseOnFieldCB_PokeVial` |

Bag description: `"Potent medicine that fully restores all Pokémon."` — [`src/data/text/item_descriptions.h`](https://github.com/BuffelSaft/pokeemerald/blob/master/src/data/text/item_descriptions.h) `sPokeVialDesc`.

### Charge model

Two saved **script vars** (not bag quantity, not a SaveBlock bitfield):

| Var | Address | Meaning |
|---|---|---|
| `VAR_POKE_VIAL_CHARGES` | `0x4091` | uses left |
| `VAR_POKE_VIAL_MAX_CHARGES` | `0x409B` | cap after each refill |

[`include/constants/vars.h`](https://github.com/BuffelSaft/pokeemerald/blob/master/include/constants/vars.h)

**IE does not display remaining charges in the bag.** Empty vs full is only learned from the empty message or by counting uses. (PSF’s later recreation added a dose icon; that is **not** IE — see §8.)

Progression of max charges:

| Max | When | Source |
|---|---|---|
| 1 | First Center grant | `pkmn_center_nurse.inc` |
| 2 | Route 111 Chansey (“Blob”) quest complete | [`data/maps/Route111/scripts.inc:Route111_EventScript_UpgradeVialHideNurse`](https://github.com/BuffelSaft/pokeemerald/blob/master/data/maps/Route111/scripts.inc) |
| 3 | Route 133 nurse Surf-HM quest | [`data/maps/Route133/scripts.inc:Route133_EventScript_NurseTeachAudino`](https://github.com/BuffelSaft/pokeemerald/blob/master/data/maps/Route133/scripts.inc) |

Author thread says “upgraded with an extra charge later” (singular). Code has **two** upgrades, to a max of **3**. See §9.

### How charges refill

| Event | Refills? | Source |
|---|---|---|
| Nurse actually heals the party | **Yes** — `copyvar VAR_POKE_VIAL_CHARGES, VAR_POKE_VIAL_MAX_CHARGES` at the start of `EventScript_PkmnCenterNurse_HealPkmn` | `pkmn_center_nurse.inc` |
| Nurse talk, player says No | **No** (heal script never runs) | same |
| First grant, player declines rest | Item + 1/1 charges already set; no extra refill | same |
| Whiteout / blackout | **Yes** — `DoWhiteOut` sets charges = max, then heals and warps | [`src/overworld.c:DoWhiteOut`](https://github.com/BuffelSaft/pokeemerald/blob/master/src/overworld.c) |
| Gym badges | **No** | no `VAR_POKE_VIAL` writes outside nurse / those two quests / whiteout |
| Pokémon Storage PC | **No** | no hits in storage code |
| Other `special HealPlayerParty` (secret base, Frontier, Route 101 Birch, Mossdeep house, …) | **No** | refill is **not** inside `HealPlayerParty`; only the nurse script + `DoWhiteOut` |

Nurse explanation: “every time you heal up at a Pokémon Center, we’ll refill it for free.” (`gText_ExplainPokeVial`)

### Battle vs field

**Field only.** Nurse text: “you can’t use it in battles.” Item has no `battleUseFunc`. Challenge difficulty in IE already blocks **Bag** in trainer battles (`DIFFICULTY_CHALLENGE` in [`src/battle_main.c`](https://github.com/BuffelSaft/pokeemerald/blob/master/src/battle_main.c) `B_ACTION_USE_ITEM`); the vial is a Key Item anyway.

From the bag, `ItemUseOutOfBattle_PokeVial` sets `sItemUseOnFieldCB` and uses `SetUpItemUseOnFieldCallback` (fade to field). Registered key-item use (Select) works; empty and success both have a `tUsingRegisteredKeyItem` branch.

### Zero charges

Does **not** heal. Prints `"The Poké Vial is empty.\nRefill it at a Pokémon Center."` (`gText_PokeVialEmpty`) and returns. Charges stay 0; the item stays in the bag.

### Healthy party / all fainted / eggs

No pre-check. A healthy party still **spends a charge**. All fainted: full revive + PP + status. Eggs: iterated, harmless. Empty party: `HealPlayerParty` loops `gPlayerPartyCount` (0) then still decrements a charge.

### Sprite / name

| | |
|---|---|
| In-game name | **Poké Vial** |
| Constant | `ITEM_POKE_VIAL` / funcs `*PokeVial*` |
| Icon | `graphics/items/icons/poke_vial.4bpp.lz` |
| Palette | `graphics/items/icon_palettes/poke_vial.gbapal.lz` |
| Tables | [`src/data/graphics/items.h`](https://github.com/BuffelSaft/pokeemerald/blob/master/src/data/graphics/items.h), [`src/data/item_icon_table.h`](https://github.com/BuffelSaft/pokeemerald/blob/master/src/data/item_icon_table.h) |

PSF credits **Horo** for a Pokévial icon and **BuffelSaft** for providing the sprite to that branch ([PSF wiki](https://github.com/PokemonSanFran/pokeemerald/wiki/pokevial)). Treat as fan art; credit if reused.

### Options / toggle in IE

**Always on.** Birch-speech difficulty (Normal / Hard / Challenge) does not gate the item. No config `#define`. Challenge only blocks in-battle Bag; field vial still works.

---

## 4. Design / challenge interactions in IE

IE is not a Nuzlocke engine. There is no no-Poké-Center challenge, no item-ban flag for the overworld, and no “does this count as a healing item” classifier. The vial is a Key Item that calls the same heal as the nurse.

| Question | IE answer |
|---|---|
| Counts as a battle healing item? | No — not usable in battle |
| Consumes a medicine? | No |
| Auto-use on whiteout to skip the warp? | **No.** Whiteout still warps; it **refills** the vial |
| Hardcore / Challenge | Challenge = no Bag in trainer fights; vial unchanged |
| Nuzlocke | Not present |

---

## 5. IE code map

| Concern | Location |
|---|---|
| Item ID | `include/constants/items.h` `ITEM_POKE_VIAL` |
| Item struct | `src/data/items.h` `[ITEM_POKE_VIAL]` |
| Use (field) | `src/item_use.c` `ItemUseOutOfBattle_PokeVial`, `ItemUseOnFieldCB_PokeVial` |
| Prototype | `include/item_use.h` |
| Heal effect | `src/script_pokemon_util.c` `HealPlayerParty` (special in `data/specials.inc`) |
| Deduct charge | `ItemUseOnFieldCB_PokeVial` |
| Refill | `data/scripts/pkmn_center_nurse.inc` `EventScript_PkmnCenterNurse_HealPkmn`; `src/overworld.c` `DoWhiteOut` |
| Grant | same nurse script, first visit |
| Max-charge upgrades | `data/maps/Route111/scripts.inc`, `data/maps/Route133/scripts.inc` |
| Save | `VAR_POKE_VIAL_CHARGES`, `VAR_POKE_VIAL_MAX_CHARGES` in `SaveBlock1` vars |
| Strings | `src/strings.c`, `data/text/pkmn_center_nurse.inc`, `src/data/text/item_descriptions.h` |
| Graphics | `graphics/items/icons/poke_vial.4bpp.lz` (+ palette) |
| Config flag | **none** |

---

## 6. Codebase map (this repo)

### Toggle

- [`include/mf_rules.h`](./include/mf_rules.h) `struct ModernRules`. Free bits: `padding0:5`, `paddingGamemode:1`, `paddingFeatures:4`, `paddingNuzlocke:1`, `paddingTail:5`. Prefer a Features bit (`paddingFeatures`) — this is QoL, not a Nuzlocke/Difficulty hammer.
- Additive-only + bump `MF_RULES_VERSION` (currently **4**). Follow [`docs-mf/SAVE_VERSIONING.md`](./docs-mf/SAVE_VERSIONING.md). Default **Off** so old saves do not gain it (`if (version < N) leave 0`).
- Menu: [`src/mf_rules_menu.c`](./src/mf_rules_menu.c) `sFeaturesPageItems` (Shiny Chance / Colors / Item Drop / Regional Forms). Challenges (`pokeCenterLimit`) is the other plausible page if planning treats it as a healing rule.
- Accessors: [`docs-mf/RULES_ACCESSORS.md`](./docs-mf/RULES_ACCESSORS.md). Grant must happen **after** rules lock (same S19 ordering as other new-game gifts).

### Item / bag / heal APIs

- Item enum: [`include/constants/items.h`](./include/constants/items.h) — append before `ITEMS_COUNT` (today ends at mega stones ~873).
- Table: [`src/data/items.h`](./src/data/items.h) `gItemsInfo[]`. Mirror Tera Orb’s key-item pattern (`.importance = 1`, `POCKET_KEY_ITEMS`, `price = 0`) but with a real `fieldUseFunc`.
- Use callbacks: [`include/item_use.h`](./include/item_use.h) / [`src/item_use.c`](./src/item_use.c).
- Party heal: [`src/script_pokemon_util.c:HealPlayerParty`](./src/script_pokemon_util.c) → [`src/pokemon.c:HealPokemon`](./src/pokemon.c) (HP, status, PP). Same effect as IE. **Also** may heal Boxes when `OW_PC_HEAL >= GEN_8 && MfShouldHealOnPcDeposit()` — do **not** call raw `HealPlayerParty` from the vial if boxed mons must stay unrestored; call `HealPokemon` on the party only.
- Nurse: [`data/scripts/pkmn_center_nurse_frlg.inc`](./data/scripts/pkmn_center_nurse_frlg.inc) (`EventScript_PkmnCenterNurse_Frlg`, Even Faster Joy path, challenge-blocked path). Whiteout heal: `EventScript_AfterWhiteOutHeal_Frlg`.
- Whiteout C: [`src/overworld.c:DoWhiteOut`](./src/overworld.c) — already `MfNuzlocke_OnWhiteOut()` then `HealPlayerParty()`. IE refills charges here; copy that only if planning wants whiteout to restock the vial.

### Existing challenge hooks

| Rule | Where | Vial implication |
|---|---|---|
| No Poké Center | `MfIsPokecenterHealingBlocked` / `MfIsPokecenterChallengeActivated` in [`src/mf_items.c`](./src/mf_items.c); nurse early-out in `pkmn_center_nurse_frlg.inc`; ADR [0049](./docs-mf/decisions/0049-product-pokecenter-pc-heal-shops.md) | ADR 0109: Joy still **refills the vial**, then refuses party heal. Whiteout still heals the party and refills. |
| No PC heal | `MfShouldHealOnPcDeposit`; ADR 0049 | PC was never an IE refill source |
| Player item ban | `MfIsPlayerBattleItemAllowed`; ADR [0046](./docs-mf/decisions/0046-product-battle-item-bans.md); Hardcore seeds `noItemPlayer` (ADR 0040) | Battle Bag only. Field Key Item is unaffected unless we choose to treat it as a healing item. |
| Nuzlocke faint | S36 / `MfNuzlocke_OnWhiteOut`; Cemetery vs Release | Vial is field-only, so it cannot revive mid-battle. Post-battle, fainted mons are already boxed/released. Residual risk: using it **before** faint handling if we ever allow in-battle use. |
| Item randomizer | `MfIsItemProgressionProtected` already returns TRUE for `POCKET_KEY_ITEMS` ([`src/mf_items.c`](./src/mf_items.c)) | A Key Item vial will not enter the random pool and will not be remapped. |

### Charge storage here

IE used two Hoenn story vars. This tree has spare vars `VAR_UNUSED_0x40F7`–`0x40FF` in [`include/constants/vars.h`](./include/constants/vars.h). Alternatively pack dose/max into `ModernRules` (needs new bytes) or a small `SaveBlock1` field. Vars are the IE-faithful, cheapest option.

---

## 7. Decisions (planning)

Accepted in ADR [0109](docs-mf/decisions/0109-product-pokevial.md):

| Topic | Call |
|---|---|
| Toggle | Features Off/On, default **Off** (all presets) |
| Grant | In the bag at New Game if On. No Joy/Oak cutscene. |
| Charge cap | `max(1, ceil(badges / 3))`, cap 3 → 0–3 badges = 1, 4–6 = 2, 7–8 = 3. Refill (Joy / whiteout) sets remaining to that max. Badge get does not add a charge in the field. |
| Spend a charge | Only if at least one non-egg would be restored (HP, status, or PP). Nuzlocke ignores fainted for that check; a fully healthy living party (or only-fainted party) refuses and keeps the charge. Off-Nuzlocke, reviving fainted **does** spend. |
| Nuzlocke heal | Living only (`HP > 0`); **no revive**. |
| Dose UI | Bag description suffix ` (remaining/max)`, e.g. `(1/2)`. |
| Name | **Poké Vial** (IE). |
| No Poké Centers | Joy still talks: refill vial, then “can’t heal the party (settings).” No `HealPlayerParty`. Vial Off = today’s refuse-only. |
| Whiteout | Refill to current max (IE). |
| Battle | Field only (IE). |
| Randomizer | Key Item, protected. |

Still open:

1. **Refuse copy** when nobody needs it (Dad’s advice vs a custom “already healthy” line).
2. **Hardcore / `noItemPlayer`:** field Key Item stays allowed (ADR 0109) unless we later treat it as a banned healing item.
3. **Heal pads** (Tower 5F, Ember Spa): refill or not? IE = nurse + whiteout only.
4. **Even Faster Joy:** refill on the instant heal path; grant is no longer Joy’s job.

---

## 8. Related implementations (not IE; do not copy blindly)

**PokemonSanFran `pokevial` branch** — [wiki](https://github.com/PokemonSanFran/pokeemerald/wiki/pokevial), [PC thread](https://www.pokecommunity.com/threads/pok%C3%A9vial.500216/). Inspired by IE / Radical Red / Temtem. Differences from IE:

- SaveBlock1 bitfield (`Dose` / `Size`, max 15), not script vars
- **Refill inside `HealPlayerParty`** (every full-party heal, not just nurse + whiteout)
- Bag **dose-percentage icons**
- Optional skip of party cutscene (`POKEVIAL_SKIP_CUTSCENE`)
- Credits BuffelSaft (name + sprite), Karathan (refactor), Horo (icon)

**Radical Red** (community, secondary): Teacher in Viridian Trainer School; often described as **6 charges**, Center refill, Select/L use. Not verified against RR source here. Changelog notes a “Nurse outside Victory Road not refilling PokeVial” bug — RR wired extra refill NPCs that IE does not have.

---

## 9. Unknowns / contradictions

| Topic | Sources disagree | Trust |
|---|---|---|
| Later upgrades | Author: “an extra charge” (one). Code: max **3** via Route 111 then Route 133. Fandom wiki: “heal once, recharge at a Center” (no upgrades). | **Code** |
| Charge UI | IE: none. PSF: icon. RR community: “you would know from the text.” | IE = none |
| Refill on any party heal | PSF yes (`HealPlayerParty`). IE no (nurse script + `DoWhiteOut` only). | **IE code** |
| Badge refill | Some community memory of “+1 per gym.” **Zero** `VAR_POKE_VIAL` writes on badge get in IE. | **IE code** — not badges |
| Always-on vs optional | IE always on. This repo wants a toggle. | Product decision |
| Does using it “count as a healing item”? | Unspecified in IE (no such rule). | Decision |
| Egg / empty-party / healthy-party | Not documented in-game; inferred from `HealPlayerParty` + no guards. | Code inference; worth a unit test |
| Numeric `ITEM_POKE_VIAL` | Offset `LAST_TMHM_INDEX + 43` on IE’s TM table; will **not** match this repo’s enum | Assign a new id here |
| Sprite license | No formal license; PSF says BuffelSaft provided it | Credit; confirm before shipping |

---

## 10. Suggested story order (draft)

A starting point for the backlog; refine during planning. Do not add these to `STORIES.md` until that session. Highest unused story numbers today are in the S70s; these can be `S76+` or a `PV*` prefix like Yellow’s `Y*`.

1. **PV01 — Toggle:** Features bit, accessor, menu row, default Off, version bump (ADR 0109).
2. **PV02 — Item + field use:** `ITEM_POKE_VIAL`, icon, strings, Key Items pocket, party-only heal, Nuzlocke skip-fainted, needs-heal check (no wasted charge), empty / already-healthy messages. Tests: healthy / PP-only / fainted-only Nuzlocke / fainted Off-Nuzlocke / egg / zero charges.
3. **PV03 — Charges + bag `(remaining/max)` description + refill:** remaining var; max from `ceil(badges/3)`; nurse heal, Even Faster Joy, no-Center Joy refill-without-heal, whiteout refill.
4. **PV04 — Grant:** `AddBagItem` after rules lock if On; remaining = max. Off = never given.
5. **PV05 — Challenge pass:** no-Center copy, `noItemPlayer`, item randomizer protection. Tests in `test/modern_firered/`.

Each story: clean `make firered`, in-game check with the toggle **on and off**, and unit tests where logic exists.

---

## 11. Sources

**Primary (IE)**

- https://github.com/BuffelSaft/pokeemerald (master, 2026-10-24)
- `include/constants/items.h`, `include/constants/vars.h`, `include/item_use.h`
- `src/data/items.h`, `src/data/text/item_descriptions.h`, `src/data/item_icon_table.h`, `src/data/graphics/items.h`
- `src/item_use.c` (`ItemUseOutOfBattle_PokeVial`, `ItemUseOnFieldCB_PokeVial`)
- `src/script_pokemon_util.c` (`HealPlayerParty`)
- `src/overworld.c` (`DoWhiteOut`)
- `src/new_game.c` (`NewGameInitData`)
- `src/strings.c`, `data/text/pkmn_center_nurse.inc`, `data/scripts/pkmn_center_nurse.inc`
- `data/maps/Route111/scripts.inc`, `data/maps/Route133/scripts.inc`
- https://github.com/BuffelSaft/pokeemerald/blob/master/README.md
- https://www.pokecommunity.com/threads/pok%C3%A9mon-inclement-emerald-a-decomp-difficulty-hack-version-1-13.457039/

**Secondary (inspired-by / community; verified against IE code above)**

- https://github.com/PokemonSanFran/pokeemerald/wiki/pokevial
- https://www.pokecommunity.com/threads/pok%C3%A9vial.500216/
- https://inclement-emerald.fandom.com/wiki/Hack_Changes (incomplete vs code)
- Radical Red player threads / changelogs (different game)
