#ifndef GUARD_MF_MIRROR_H
#define GUARD_MF_MIRROR_H

// S50 — Challenges MIRROR MODE / MIRROR THIEF (ME tx_Challenges_Mirror*).
// Trainer (and double) battles put a copy of the foe's party on the player.
// Thief keeps that copy after battle; otherwise the original party is restored.
// See ADR 0053.

#include "gba/types.h"

struct Pokemon;

// Trainer battles, or doubles (including wild doubles). Not link/recorded.
bool8 MfMirror_IsBattleEligible(u32 battleTypeFlags);

// Mirror on, not thief: restore the backed-up player party at battle end.
bool8 MfMirror_ShouldRestorePlayerParty(bool8 mirror, bool8 thief);

// Fill outPlayer from opponent A, then opponent B in the second half if twoOpponents.
void MfMirror_BuildPlayerPartyFromOpponents(const struct Pokemon *opponentA,
                                            const struct Pokemon *opponentB,
                                            bool8 twoOpponents,
                                            struct Pokemon *outPlayer);

void MfMirror_OnBattleStart(u32 battleTypeFlags);
void MfMirror_OnBattleEnd(u32 battleTypeFlags);

#endif // GUARD_MF_MIRROR_H
