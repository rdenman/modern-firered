# 0056 — Trainer remap before IV/EV; Oak trio via S51

- **Type:** product
- **Status:** Accepted
- **Date:** 2026-10-05
- **Story:** S53
- **ME reference:** `CreateNPCTrainerParty` + `GetSpeciesRandomSeeded(..., TX_RANDOM_T_TRAINER, trainerNum)` in ME `battle_main.c`; `PickRandomStarter` / `GetStarterPokemon` in ME `pokemon.c` / `starter_choose.c`
- **Expansion config:** `MF_RANDOMIZER` (compile-out only)

## Context

S53 must randomize trainer parties and Oak's three balls without flattening gym levels. ME remaps species at `CreateMon` and skips authored moves so the replacement gets a level-up set. It also hashes `trainerNum` into the pick, so two Pidgey trainers differ. Starters in ME are an OT-seeded shuffle of evo-0 (or the full list), not `GetSpeciesRandomSeeded`, and Birch has no rival type-advantage rule. FireRed's rival takes a fixed type-advantage ball. S44 already overwrites trainer IVs/EVs after generation; S50 copies the foe party after `CreateNPCTrainerParty`. ADR 0051 says monotype Oak remap wins over a later starter randomizer.

## Decision

1. **Trainers** — `MfRandomizeTrainerMon` on a `TrainerMon` copy inside `CreateNPCTrainerPartyFromTrainer`, **before** `GenerateMonFromTrainerMon`. Level, party size, and held item stay. When the species changes, authored moves / ability / forced gender are cleared so `CreateMon` uses the new species' default set. `MfApplyTrainerIvEvScaling` still runs last (ADR 0047). Mirror then copies that party (ADR 0053).
2. **Trainer hash** — `MfSpeciesMapEx` with `MF_RANDOM_CAT_TRAINER`. Location key is the S16 map-based mapsec (or 0) XOR a CRC16 of the trainer struct (ME's `trainerNum` extra offset). Frontier / e-Reader / Trainer Hill stay identity, same skip as S44.
3. **Starters** — `randomStarter` remaps Bulbasaur / Squirtle / Charmander through `MfSpeciesMapEx` (`STATIC` category, slot as location key) so Balancing / Legendaries / the S08 pool apply. The trio is unique. Seed is S16, not OT ID. `MfStaticGiftSpecies` still skips an empty party, so Oak `givemon` uses this remap only.
4. **Rival** — keep the vanilla ball pairing when that remapped species is still super-effective against the player's remapped starter (any attacking type vs the dual-type product). If it is not, and the leftover ball is, use that leftover species. Otherwise keep the original pairing. `VAR_STARTER_MON` stays the ball index so later rival teams still branch.
5. **Monotype** — if ONE TYPE ONLY is on, skip starter randomization (ME `if / else if`).

## Alternatives considered

- **ME `PickRandomStarter` shuffle of `sRandomSpeciesEvo0`** — rejected; it ignores S51 BST/HM/legend rules and the S16 seed.
- **Keep authored trainer moves on the new species** — rejected; ME skips them, and gym TMs on a random species are nonsense.
- **Always walk the rival to a new ball when swapping species** — rejected; extra Oak script branches for a rare case.

## Consequences

- Upstream: `CreateNPCTrainerPartyFromTrainer` (one copy + remap) and the Oak lab confirm special. Logic lives in `mf_party_random.c`.
- Changing the trainer CRC or starter slot keys is a mapping break (treat like ADR 0016).
- Chaos (S57) still goes through `MfSpeciesMapEx`; do not call `Random()` here.
