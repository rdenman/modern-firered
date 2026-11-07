#include "global.h"
#include "event_data.h"
#include "mf_monotype.h"
#include "mf_random.h"
#include "mf_rules.h"
#include "new_game.h"
#include "pokemon.h"
#include "constants/pokemon.h"
#include "constants/species.h"

#include "data/mf_monotype_oak_starters.h"

static const u8 sText_MonotypeBlocked[] = _(
    "This Pokémon's type isn't allowed\n"
    "in the One Type challenge!{PAUSE_UNTIL_PRESS}");

bool8 MfSpeciesMatchesType(enum Species species, u8 type)
{
    species = SanitizeSpeciesId(species);
    if (species == SPECIES_NONE || species == SPECIES_EGG)
        return FALSE;
    return GetSpeciesType(species, 0) == type
        || GetSpeciesType(species, 1) == type;
}

bool8 MfIsMonotypePartyLegalValue(enum Species species, u8 monotype)
{
    if (monotype == MF_MONOTYPE_OFF)
        return TRUE;
    species = SanitizeSpeciesId(species);
    if (species == SPECIES_NONE)
        return TRUE;
    return MfSpeciesMatchesType(species, monotype);
}

bool8 MfIsMonotypePartyLegal(enum Species species)
{
    return MfIsMonotypePartyLegalValue(species, MfRules_GetMonotype());
}

// Fisher–Yates keyed by full OT ID. ME ShuffleListU16 reused one ISO_RANDOMIZE1
// draw for every swap, which pinned Bug to Caterpie/Weedle/Paras. See ADR 0115.
static void MfShuffleSpeciesList(enum Species *list, u16 count, u8 slot)
{
    u32 otId = GetTrainerId(gSaveBlock2Ptr->playerTrainerId);
    u16 i;

    for (i = count - 1; i > 0; i--)
    {
        u16 j = MfRandom_Modulo(otId, MF_RANDOM_CAT_MONOTYPE_STARTER, i, slot, i + 1);
        enum Species tmp = list[j];
        list[j] = list[i];
        list[i] = tmp;
    }
}

static enum Species MfPickMonotypeStarter(u8 monotype, u8 slot, const enum Species chosen[3])
{
    u16 i;
    enum Species stemp[ARRAY_COUNT(sMfMonotypeEvo0)];
    enum Species species = SPECIES_NONE;

    memcpy(stemp, sMfMonotypeEvo0, sizeof(sMfMonotypeEvo0));
    MfShuffleSpeciesList(stemp, ARRAY_COUNT(sMfMonotypeEvo0), slot);
    for (i = 0; i < ARRAY_COUNT(sMfMonotypeEvo0); i++)
    {
        species = stemp[i];
        if (!IsSpeciesEnabled(species))
            continue;
        if (!MfSpeciesMatchesType(species, monotype))
            continue;
        if (species == chosen[0] || species == chosen[1] || species == chosen[2])
            continue;
        return species;
    }
    return chosen[1];
}

void MfFillMonotypeOakStarters(u8 monotype, enum Species trio[3])
{
    u8 slot;

    trio[0] = SPECIES_NONE;
    trio[1] = SPECIES_NONE;
    trio[2] = SPECIES_NONE;

    if (monotype == MF_MONOTYPE_OFF || monotype >= NUMBER_OF_MON_TYPES)
        return;

    for (slot = 0; slot < 3; slot++)
        trio[slot] = MfPickMonotypeStarter(monotype, slot, trio);
}

enum Species MfFindNthMonotypeStarter(u8 monotype, u8 slot)
{
    enum Species trio[3];

    if (slot > 2)
        slot = 0;
    MfFillMonotypeOakStarters(monotype, trio);
    return trio[slot];
}

enum Species MfResolveMonotypeStarterSpecies(enum Species chosen, u8 slot)
{
    enum Species resolved;
    u8 monotype = MfRules_GetMonotype();

    if (monotype == MF_MONOTYPE_OFF)
        return chosen;

    resolved = MfFindNthMonotypeStarter(monotype, slot);
    if (resolved == SPECIES_NONE)
        return chosen;
    return resolved;
}

u16 MfResolveOakStarterForMonotype(void)
{
    enum Species chosen;
    enum Species resolved;
    u8 slot;

    if (!MfRules_IsMonotypeActive())
        return FALSE;

    chosen = VarGet(VAR_TEMP_2);
    slot = VarGet(VAR_TEMP_5);
    resolved = MfResolveMonotypeStarterSpecies(chosen, slot);
    if (resolved == chosen)
        return FALSE;
    VarSet(VAR_TEMP_2, resolved);
    return TRUE;
}

bool8 MfIsMonotypeCaptureBlocked(void)
{
    enum Species species;

    if (!MfRules_IsMonotypeActive())
        return FALSE;
    species = GetMonData(&gParties[B_TRAINER_OPPONENT_A][0], MON_DATA_SPECIES);
    return !MfIsMonotypePartyLegal(species);
}

const u8 *MfGetMonotypeBlockedMessage(void)
{
    return sText_MonotypeBlocked;
}
