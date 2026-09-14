#ifndef GUARD_MF_TYPES_H
#define GUARD_MF_TYPES_H

#include "global.h"

// Runtime type helpers (S27+). GetSpeciesType routes here so Fairy / modern-type
// rules stay out of upstream species_info tables.

enum Type MfGetSpeciesType(enum Species species, u8 slot);

#endif // GUARD_MF_TYPES_H
