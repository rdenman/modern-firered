#ifndef GUARD_MF_TRADE_RANDOM_H
#define GUARD_MF_TRADE_RANDOM_H

// Randomizer TRADES: NPC in-game trade species. Deterministic via S16.
// Offered = S51 remap (BALANCING / LEGENDARIES apply). Requested = a species
// from the active wild tables after the WILD remap, so it is always catchable.
// Every offered and requested species is unique across all trades.
// Identity when the rule is off. See ADR 0118.

#include "gba/types.h"
#include "constants/species.h"

#define MF_TRADE_RANDOM_MAX_TRADES 16

bool8 MfTradeRandom_IsActive(void);

enum Species MfTradeRandom_OfferedSpecies(enum Species vanillaOffered, u8 tradeId);
enum Species MfTradeRandom_RequestedSpecies(enum Species vanillaRequested,
                                            enum Species vanillaOffered,
                                            u8 tradeId);

#endif // GUARD_MF_TRADE_RANDOM_H
