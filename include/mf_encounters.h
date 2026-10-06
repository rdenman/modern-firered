#ifndef GUARD_MF_ENCOUNTERS_H
#define GUARD_MF_ENCOUNTERS_H

#include "wild_encounter.h"

// Gamemode ENCOUNTERS (ME tx_Mode_Encounters / alternateSpawns):
#define MF_ENCOUNTERS_VANILLA   0
#define MF_ENCOUNTERS_MODERN    1
#define MF_ENCOUNTERS_POSTGAME  2

// TRUE when the active rules should use the modern wild tables right now
// (Modern always; Postgame only after FLAG_SYS_GAME_CLEAR).
bool32 MfShouldUseModernWildEncounters(void);

// Active wild header table for the current rules (FR modern vs stock).
const struct WildPokemonHeader *MfGetActiveWildMonHeaders(void);

// S52 — remap a wild-table species for a mapsec (identity if WILD POKéMON is off).
// Unown letters stay Unown so Ruins of Alph stay completable.
enum Species MfWildEncounterSpecies(enum Species species, u16 mapsec);

// Same using the current overworld mapsec.
enum Species MfWildEncounterSpeciesHere(enum Species species);

enum Species MfWildSlotSpecies(const struct WildPokemon *slot, u16 mapsec);

u16 MfWildHeaderMapsec(const struct WildPokemonHeader *header);

// Scripted wilds, fossils/legendaries via setwildbattle, event mons.
enum Species MfStaticEncounterSpecies(enum Species species);

// Player gifts (givemon). Identity while the party is empty so Oak's starter stays S53.
enum Species MfStaticGiftSpecies(enum Species species);

#endif // GUARD_MF_ENCOUNTERS_H
