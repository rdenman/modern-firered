#ifndef GUARD_MF_DEX_H
#define GUARD_MF_DEX_H

// Run-accurate species set and National Dex. See ADR 0120.
//
// MfSpecies_IsInRun: the species may exist this run. Gen 1–3 always; regional
// forms (and their post-Gen-3 evolutions) need Regional Forms; other post-Gen-3
// evolutions/babies of Gen 1–3 lines need Gamemode Evolutions = Modern.
//
// MfDex_*: the run's National Dex, fixed for the whole run (ADR 0124). It
// depends only on the locked rules: Gen 1–3 always; post-Gen-3 species when the
// run can evolve into them, or the whole run species set when a species
// randomizer hands the player Pokémon. Entries keep real National Dex numbers.
// FRLG only; other builds list every enabled species.

#include "gba/types.h"
#include "pokemon.h"
#include "constants/pokedex.h"
#include "constants/species.h"

bool8 MfSpecies_IsInRun(enum Species species);

// Drops evolutions whose target is not in this run. May return a shared
// buffer that the next call overwrites (same contract as the randomizer).
const struct Evolution *MfSpecies_FilterRunEvolutions(const struct Evolution *evolutions);

// TRUE when the run dex applies (FRLG with the rules engine).
bool8 MfDex_IsActive(void);

// Rebuilds the cached entry set if the rules changed (a different save loaded).
// Cheap; call once before a batch of the reads below.
void MfDex_Refresh(void);

bool8 MfDex_IsListed(enum NationalDexOrder nationalNum);
// 3, or 4 when the highest listed National Dex number exceeds 999.
u8 MfDex_GetNationalDigitCount(void);
// Seen/caught entries among the listed ones (FLAG_GET_SEEN / FLAG_GET_CAUGHT).
u16 MfDex_CountListed(u8 caseId);
// Every listed non-mythical entry is caught.
bool8 MfDex_HasAllListedCaught(void);

#endif // GUARD_MF_DEX_H
