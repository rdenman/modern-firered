#include "global.h"
#include "mf_rules.h"
#include "mf_types.h"
#include "pokemon.h"

// Pre-Gen-6 typings for species that gained Fairy via P_UPDATED_TYPES >= GEN_6.
// Spec: ME GetTypeBySpecies when tx_Mode_Fairy_Types == 0 (types_old). Cotonee /
// Whimsicott are the same official Gen-6 retype class and are included for
// TESTING builds that keep Gen 5 families. Pure Fairy introductions (Sylveon,
// Alolan Ninetales, megas that were always Fairy) are intentionally left alone.

struct MfFairyTypeFallback
{
    u16 species;
    u8 types[2];
};

static const struct MfFairyTypeFallback sFairyTypeFallbacks[] =
{
    { SPECIES_CLEFFA,     { TYPE_NORMAL,  TYPE_NORMAL  } },
    { SPECIES_CLEFAIRY,   { TYPE_NORMAL,  TYPE_NORMAL  } },
    { SPECIES_CLEFABLE,   { TYPE_NORMAL,  TYPE_NORMAL  } },
    { SPECIES_IGGLYBUFF,  { TYPE_NORMAL,  TYPE_NORMAL  } },
    { SPECIES_JIGGLYPUFF, { TYPE_NORMAL,  TYPE_NORMAL  } },
    { SPECIES_WIGGLYTUFF, { TYPE_NORMAL,  TYPE_NORMAL  } },
    { SPECIES_TOGEPI,     { TYPE_NORMAL,  TYPE_NORMAL  } },
    { SPECIES_TOGETIC,    { TYPE_NORMAL,  TYPE_FLYING  } },
    { SPECIES_TOGEKISS,   { TYPE_NORMAL,  TYPE_FLYING  } },
    { SPECIES_AZURILL,    { TYPE_NORMAL,  TYPE_NORMAL  } },
    { SPECIES_MARILL,     { TYPE_WATER,   TYPE_WATER   } },
    { SPECIES_AZUMARILL,  { TYPE_WATER,   TYPE_WATER   } },
    { SPECIES_SNUBBULL,   { TYPE_NORMAL,  TYPE_NORMAL  } },
    { SPECIES_GRANBULL,   { TYPE_NORMAL,  TYPE_NORMAL  } },
    { SPECIES_MIME_JR,    { TYPE_PSYCHIC, TYPE_PSYCHIC } },
    { SPECIES_MR_MIME,    { TYPE_PSYCHIC, TYPE_PSYCHIC } },
    { SPECIES_RALTS,      { TYPE_PSYCHIC, TYPE_PSYCHIC } },
    { SPECIES_KIRLIA,     { TYPE_PSYCHIC, TYPE_PSYCHIC } },
    { SPECIES_GARDEVOIR,  { TYPE_PSYCHIC, TYPE_PSYCHIC } },
    { SPECIES_MAWILE,     { TYPE_STEEL,   TYPE_STEEL   } },
    { SPECIES_COTTONEE,   { TYPE_GRASS,   TYPE_GRASS   } },
    { SPECIES_WHIMSICOTT, { TYPE_GRASS,   TYPE_GRASS   } },
};

static const u8 *FindFairyTypeFallback(enum Species species)
{
    u32 i;

    for (i = 0; i < ARRAY_COUNT(sFairyTypeFallbacks); i++)
    {
        if (sFairyTypeFallbacks[i].species == species)
            return sFairyTypeFallbacks[i].types;
    }
    return NULL;
}

enum Type MfGetSpeciesType(enum Species species, u8 slot)
{
    enum Species sanitized = SanitizeSpeciesId(species);
    const u8 *fallback;

    if (slot > 1)
        slot = 1;

    if (!MfRules_HasFairyTypes())
    {
        fallback = FindFairyTypeFallback(sanitized);
        if (fallback != NULL)
            return fallback[slot];
    }

    return gSpeciesInfo[sanitized].types[slot];
}
