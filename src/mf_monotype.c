#include "global.h"
#include "event_data.h"
#include "mf_monotype.h"
#include "mf_rules.h"
#include "new_game.h"
#include "pokemon.h"
#include "random.h"
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

// ME RandomSeeded(u16 value, TRUE): ISO_RANDOMIZE1(low16(OT ID) + value) >> 16.
// ShuffleListU16 reuses that one draw for every swap (same seed each iteration).
static u16 MfMeRandomSeeded(u16 value)
{
    u16 otId = GetTrainerId(gSaveBlock2Ptr->playerTrainerId);

    return ISO_RANDOMIZE1(otId + value) >> 16;
}

static void MfShuffleSpeciesList(enum Species *list, u16 count, u32 seed)
{
    u16 i;
    u16 k = MfMeRandomSeeded(seed);

    for (i = count - 1; i > 0; i--)
    {
        u16 j = k % (i + 1);
        enum Species tmp = list[j];
        list[j] = list[i];
        list[i] = tmp;
    }
}

static enum Species MfPickMeMonotypeStarter(u8 monotype, u8 slot, const enum Species chosen[3])
{
    u16 i;
    enum Species stemp[ARRAY_COUNT(sMfMonotypeEvo0)];
    enum Species species = SPECIES_NONE;

    memcpy(stemp, sMfMonotypeEvo0, sizeof(sMfMonotypeEvo0));
    MfShuffleSpeciesList(stemp, ARRAY_COUNT(sMfMonotypeEvo0), (slot + 13) * 12289);
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
        trio[slot] = MfPickMeMonotypeStarter(monotype, slot, trio);
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
    slot = VarGet(VAR_TEMP_1);
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
