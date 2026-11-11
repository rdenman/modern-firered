#ifndef GUARD_MF_DEX_H
#define GUARD_MF_DEX_H

// Run-accurate species set and National Dex. See ADR 0120.
//
// MfSpecies_IsInRun: the species may exist this run. Gen 1–3 always; regional
// forms (and their post-Gen-3 evolutions) need Regional Forms; other post-Gen-3
// evolutions/babies of Gen 1–3 lines need Gamemode Evolutions = Modern.
//
// MfDex_*: National Dex entries the player can still obtain this run (wild,
// static, gift, trade, evolution, breeding — after randomizer remaps), shown
// renumbered 1..N. Caught entries are always listed. FRLG only; other builds
// list every enabled species under its real number.

#include "gba/types.h"
#include "pokemon.h"
#include "constants/pokedex.h"
#include "constants/species.h"

bool8 MfSpecies_IsInRun(enum Species species);

// Drops evolutions whose target is not in this run. May return a shared
// buffer that the next call overwrites (same contract as the randomizer).
const struct Evolution *MfSpecies_FilterRunEvolutions(const struct Evolution *evolutions);

// TRUE when the dex uses run numbering (FRLG with the rules engine).
bool8 MfDex_IsActive(void);

// Rebuilds the cached entry set if rules, story choices, roamers or caught
// flags changed. Call once before a batch of the reads below.
void MfDex_Refresh(void);

bool8 MfDex_IsListed(enum NationalDexOrder nationalNum);
u16 MfDex_GetCount(void);
// 3, or 4 when the National Dex shown has more than 999 entries.
u8 MfDex_GetNationalDigitCount(void);
// 1..N for listed entries; 0 otherwise.
u16 MfDex_GetRunNumber(enum NationalDexOrder nationalNum);
// Seen/caught entries among the listed ones (FLAG_GET_SEEN / FLAG_GET_CAUGHT).
u16 MfDex_CountListed(u8 caseId);
// Every listed non-mythical entry is caught.
bool8 MfDex_HasAllListedCaught(void);

#endif // GUARD_MF_DEX_H
