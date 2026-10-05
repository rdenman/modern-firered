#ifndef GUARD_MF_STATS_H
#define GUARD_MF_STATS_H

#include "global.h"
#include "constants/pokemon.h"

// Runtime base-stat helpers (S29 / S49). GetSpeciesBase* route here so
// POKéMON STATS and BST EQUALIZER apply before dex / summary / battle.

u32 MfGetSpeciesBaseStat(enum Species species, u32 statIndex);

// Map Challenges BST EQUALIZER mode 0–3 → Off / 100 / 255 / 500.
u32 MfGetBstEqualizerTarget(u8 mode);

// Scale `count` stats so they sum to `target`, keeping relative weights
// (largest remainder). Used by tests and MfGetSpeciesBaseStat.
void MfNormalizeStatsToBst(u32 *stats, u32 count, u32 target);

// Recalculate the player party’s stored stats after a mid-run modernStats
// or BST-equalizer debug toggle so summary / battle match the active rule.
void MfRecalculatePartyStats(void);

#endif // GUARD_MF_STATS_H
