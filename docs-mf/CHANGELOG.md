# Changelog

## Unreleased

- Challenges **Gift Only** (default Off): no Pokémon can be caught. Build your team from the starter, gifts, Game Corner prizes, fossils, trades, and eggs. Oak's Aides and the National Dex skip catch counts (unless Catch Rewards is Badges/Levels), and the lead can use bag HMs. Can't be combined with One Type Only or Mirror Thief.

- Features **Catch Rewards** (Catches default / Badges / Levels). Badges: Oak's Aides need 3–7 badges (HM05 → Exp. Share), Oak's National Dex needs 8. Levels: a party Pokémon at Lv. 20/30/40/50/60, National Dex Lv. 60. In Catches mode the Aides count the National Dex once you have it.
- Randomizer **Trades** (default Off): NPC in-game trades offer a random Pokémon and ask for one you can catch in the wild under your current rules. No species repeats across trades. Nicknames stay vanilla.
- Features **Exp. All** (default Off). Key Item in the bag at New Game; use it to share Exp. with the whole party or turn it off. Oak’s Aide held Exp. Share is unchanged.
- Running Shoes from New Game (and on load for older saves). Oak’s Pewter delivery NPC and the post-Brock intercept are gone.
- Gamemode **Bag HMs** (Classic Off, Modern On): compatible party Pokémon can use a bag HM in the field without teaching it. Nuzlocke / Mirror / Monotype / Random Moves also let the lead use bag HMs (anti-softlock).
- Features **Poké Vial** (default Off). Key Item in the bag at New Game; field-only party heal with `ceil(badges/3)` charges (1–3). Refills at Nurse Joy and on whiteout. No-Center runs still get a vial refill from Joy. Nuzlocke uses cannot revive.
- Overworld **L** toggles Autorun (same bit as Options+). Button Mode no longer includes L=A.
- HM moves can be forgotten from summary, level-up, TMs, and evolution like any other move. The Move Deleter is optional.
- Randomizer **TMs** (default Off): each TM01–50, FRLG tutor, and Cape Brink ultimate teaches a unique random move. Who can learn it is still the vanilla teachable set. HMs stay vanilla.
- National Dex unlocks at the start of a run (and on load for existing saves) when Encounters are Modern/Postgame or a species randomizer is on, so Johto/Hoenn Pokémon get real dex numbers and dex entries instead of 000.
- Teaching Hidden Power shows that Pokémon’s IV type on the forget-move screen; the TM bag entry stays Normal.
- In-game TM/HM labels stay fully capitalized (`TM01`, not `Tm01`).
- Celadon 4F also sells Moon/Sun Stones (Moon Stone buy/sell matches Sun Stone) plus Ice/Shiny/Dusk/Dawn Stones, Oval Stone, Razor Claw, and Razor Fang.
- Item randomization dests are FRLG/RSE items plus later-gen evolution items that this ROM can use (Linking Cord, Electirizer, Dawn Stone, …). Mega Stones, Z-Crystals, later competitive holds, Peat Block, Metal Alloy, Leader’s Crest, and Galarica Cuff/Wreath are out of the pool.
- Item randomizer keeps TM pickups as TM01–50 and never maps a non-TM chest onto a TM.
- Field Poké Balls shuffle under Items; Oak’s five (and his Route 22 refill) stay Poké Balls. Marts still sell vanilla balls.
- Gym and other gift TM dialogue names the TM and move you actually received.
- Mega Evolutions compiled out of the FireRed ROM (primals still on).
- Features **REGIONAL FORMS** (default Off). Alolan/Galarian/Hisuian/Paldean forms stay in the ROM but are out of the species pool unless this is On. Existing saves migrate the option On so in-progress randomizers do not reshuffle.

## 1.0.0

First player release. Patch a clean **FireRed (USA, Rev 0)** dump — never share the `.gba`.

- Rules menu at New Game (Classic / Modern / Custom plus Features, Randomizer, Nuzlocke, Difficulty, Challenges).
- Options+ QoL (autorun, Joy speed, fast battles, STAB hints, bag extras).
- Nuzlocke, randomizer, monotype, level caps, and the rest of the in-game toggles; see [PLAYER_FEATURES.md](./PLAYER_FEATURES.md).
- Celio gives MysticTicket and AuroraTicket after the Sapphire.
- Save-backed rules with migration; version stamp **v1.0.0** in Options+.
- Debug menu and Quickstart are **off** in this build (`make firered RELEASE=1`).

**Requires:** Pokémon FireRed (USA) revision 0, 16,777,216 bytes, SHA-1 `41cb43a279208d5e14ee8ae47285c6bdae16dd85`.

**How to apply:** [RELEASE.md](./RELEASE.md).
