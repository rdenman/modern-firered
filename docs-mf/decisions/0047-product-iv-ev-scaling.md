# 0047 — Runtime IV/EV scaling

- **Type:** product
- **Status:** Accepted
- **Date:** 2026-10-03
- **Story:** S44
- **ME reference:** `GetCurrentTrainerIVs` / `GetCurrentTrainerEVs` / `sIV_Table` / `sEV_Table` in `src/tx_randomizer_and_challenges.c`; `CreateBoxMon` PLAYER IVs and `MonGainEVs` PLAYER EVs in `src/pokemon.c`; trainer overwrite in `src/battle_main.c`
- **Expansion config:** `B_EV_CAP_TYPE` (`EV_CAP_NONE`), `B_EV_CAP_VARIABLE` (8), `B_EV_ITEMS_CAP` (`FALSE`)

## Context

Difficulty already stores PLAYER IVs (`maxPartyIvs` Off / Max / HP), PLAYER EVs (`noEvs`), TRAINER IVs (`scalingIvs` Off / Scale / Hard), and TRAINER EVs (`scalingEvs` Off / Scale / Hard / Extrem). Expansion’s EV cap in `src/caps.c` is compile-time and off. Flipping `B_EV_CAP_TYPE` would bind Emerald and every rule-off save.

## Decision

1. **Leave `include/config/caps.h` EV defaults unchanged.** `GetCurrentEVCap` returns 0 when PLAYER EVs is On, until Hall of Fame (`FLAG_SYS_GAME_CLEAR`). Off (and post-HoF) keeps the compile-time path (`MAX_TOTAL_EVS`). Vitamins and wings use that same cap (`MfShouldCapEVItems`), so they cannot bypass the rule.
2. **Trainer IV/EV tables match ME**, keyed by badge count 0–8 (same badge flags as S41):

   | Badges | Scale IVs | Scale EVs |
   | --- | --- | --- |
   | 0 | 7 | 12 |
   | 1 | 10 | 24 |
   | 2 | 13 | 36 |
   | 3 | 16 | 48 |
   | 4 | 19 | 60 |
   | 5 | 22 | 72 |
   | 6 | 25 | 80 |
   | 7 | 28 | 100 |
   | 8 | 31 | 128 |

   Hard IVs are 31. Hard EVs are 128. Extrem EVs are 252 (`MAX_PER_STAT_EVS`). Off leaves authored trainer IVs/EVs. Applied IVs go to every stat; applied EVs go to HP, Speed, and the higher of Atk/SpAtk and Def/SpDef (ME).
3. **PLAYER IVs at creation** (`SetBoxMonIVs` / packed-IV helpers / `CreateMonFromTemplate`): Max writes 31 in every stat; HP writes 30 or 31 per stat so Hidden Power types still vary. Off leaves the usual random or scripted IVs. FireRed’s Oak starter is `givemon`, which writes IVs in the template path and never calls `SetBoxMonIVs`. Wilds created through those helpers also get the rule, matching ME’s `CreateBoxMon` hook.
4. **Do not scale Frontier / e-Reader / Trainer Hill** parties. Hall of Fame lifts PLAYER EVs only; trainer scale stays badge-based after the Elite Four.
5. **Summary IV/EV pages stay raw stored values.** Caps bind at write/gain time, so a new-game run shows 31s or 0 EVs without a second display transform.

## Alternatives considered

- Flip `B_EV_CAP_TYPE` to `EV_CAP_NO_GAIN` — rejected; always-on and ignores Off / HoF.
- Wipe existing EVs in `CalculateMonStats` when PLAYER EVs is On — rejected; ME only blocks gain, and mid-run toggles would silently change stats.
- Force trainer EVs to 0 when TRAINER EVs is Off — rejected; “expected” means the party file, which may already have EVs.

## Consequences

- Upstream touch: `caps.c` (EV cap gate), `pokemon.c` (player IVs + EV items), `battle_setup.c` (trainer overwrite after generation). ME patched `battle_main.c`; expansion builds trainer parties in `CreateNPCTrainerPartyFromTrainer`.
- S53 trainer randomization should apply after this overwrite, or compose by calling `MfApplyTrainerIvEvScaling` last.
- The debug inspector shows live trainer IV/EV after ScaleIV / ScaleEV (`1/7` is Scale with no badges).
