#include "global.h"
#include "mf_evolution.h"
#include "mf_rules.h"
#include "pokemon.h"

static const u8 sText_EvolutionLimit[] = _(
    "The evolution limit won't let\n"
    "this Pokémon evolve.{PAUSE_UNTIL_PRESS}");

bool8 MfIsEvolutionBlockedByLimitValue(enum Species species, u8 evoLimit, bool8 hasPreEvolution)
{
    (void)species;

    if (evoLimit == MF_EVO_LIMIT_NONE)
        return TRUE;
    if (evoLimit == MF_EVO_LIMIT_FIRST)
        return hasPreEvolution;
    return FALSE;
}

bool8 MfSpeciesHasPreEvolution(enum Species species)
{
    species = GET_BASE_SPECIES_ID(SanitizeSpeciesId(species));
    if (species == SPECIES_NONE)
        return FALSE;
    return GetSpeciesPreEvolution(species) != SPECIES_NONE;
}

bool8 MfIsEvolutionBlockedByLimit(enum Species species)
{
    u8 limit = MfRules_GetEvoLimit();

    if (limit == MF_EVO_LIMIT_OFF)
        return FALSE;
    if (limit == MF_EVO_LIMIT_NONE)
        return TRUE;
    return MfIsEvolutionBlockedByLimitValue(species, limit, MfSpeciesHasPreEvolution(species));
}

bool8 MfIsEvolutionItemBlockedByLimit(enum Species species, enum Item item)
{
    const struct Evolution *evolutions;
    u32 i;

    if (!MfIsEvolutionBlockedByLimit(species))
        return FALSE;

    evolutions = GetSpeciesEvolutions(species);
    if (evolutions == NULL)
        return FALSE;

    for (i = 0; evolutions[i].method != EVOLUTIONS_END; i++)
    {
        if (evolutions[i].method == EVO_ITEM && evolutions[i].param == item)
            return TRUE;
    }
    return FALSE;
}

const u8 *MfGetEvolutionLimitMessage(void)
{
    return sText_EvolutionLimit;
}
