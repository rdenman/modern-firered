#ifndef GUARD_MF_IV_EV_H
#define GUARD_MF_IV_EV_H

// S44 — PLAYER IVs / PLAYER EVs / TRAINER IVs / TRAINER EVs.
// ME: GetCurrentTrainerIVs / GetCurrentTrainerEVs in src/tx_randomizer_and_challenges.c.

#include "gba/types.h"

#define MF_TRAINER_IVS_OFF    0
#define MF_TRAINER_IVS_SCALE  1
#define MF_TRAINER_IVS_HARD   2

#define MF_TRAINER_EVS_OFF      0
#define MF_TRAINER_EVS_SCALE    1
#define MF_TRAINER_EVS_HARD     2
#define MF_TRAINER_EVS_EXTREME  3

#define MF_PLAYER_IVS_OFF  0
#define MF_PLAYER_IVS_MAX  1
#define MF_PLAYER_IVS_HP   2

#define MF_TRAINER_IV_EV_BADGE_STAGES 9
#define MF_TRAINER_EVS_HARD_VALUE     128

struct Pokemon;
struct BoxPokemon;

// Pure lookups. badgeCount above 8 uses the 8-badge row.
// Off IVs return 0 (caller leaves authored IVs). Hard IVs are always 31.
u8 MfResolveTrainerIVs(u8 mode, u8 badgeCount);
u8 MfResolveTrainerEVs(u8 mode, u8 badgeCount);

// Live ME GetCurrentTrainerIVs / GetCurrentTrainerEVs.
u8 MfGetCurrentTrainerIVs(void);
u8 MfGetCurrentTrainerEVs(void);

// PLAYER EVs: no battle or item EV gain until Hall of Fame.
bool8 MfArePlayerEvsDisabled(void);
bool8 MfShouldCapEVItems(void);

// PLAYER IVs at creation. Max = all 31; HP = each stat 30 or 31.
void MfApplyPlayerPartyIvsToBoxMon(struct BoxPokemon *mon);
void MfApplyPlayerPartyIvsToMon(struct Pokemon *mon);
// IVs plus PLAYER EVs wipe. Use after any path that writes IVs/EVs itself.
void MfApplyPlayerIvEvRulesToMon(struct Pokemon *mon);

// Overwrite NPC trainer IVs/EVs after party generation (skips Frontier / e-Reader / Trainer Hill).
void MfApplyTrainerIvEvScaling(struct Pokemon *party, u32 count);

#endif // GUARD_MF_IV_EV_H
