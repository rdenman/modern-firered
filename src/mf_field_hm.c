#include "global.h"
#include "item.h"
#include "mf_field_hm.h"
#include "mf_rules.h"
#include "party_menu.h"
#include "pokemon.h"
#include "constants/items.h"
#include "constants/moves.h"

static enum Item MfFieldHm_ItemForMove(enum Move move)
{
    switch (move)
    {
    case MOVE_CUT:        return ITEM_HM_CUT;
    case MOVE_FLY:        return ITEM_HM_FLY;
    case MOVE_SURF:       return ITEM_HM_SURF;
    case MOVE_STRENGTH:   return ITEM_HM_STRENGTH;
    case MOVE_FLASH:      return ITEM_HM_FLASH;
    case MOVE_ROCK_SMASH: return ITEM_HM_ROCK_SMASH;
    case MOVE_WATERFALL:  return ITEM_HM_WATERFALL;
    case MOVE_DIVE:       return ITEM_HM_DIVE;
    default:              return ITEM_NONE;
    }
}

bool8 MfFieldHm_OverwriteActive(void)
{
    const struct ModernRules *r = MfRules_GetActiveRules();

    return r->nuzlocke
        || r->nuzlockeEasy
        || r->mirror
        || r->randomMoves
        || MfRules_IsMonotypeActive();
}

bool32 MfFieldHm_BagHasMove(enum Move move)
{
    enum Item item = MfFieldHm_ItemForMove(move);

    if (item == ITEM_NONE)
        return FALSE;
    return CheckBagHasItem(item, 1);
}

bool32 MfFieldHm_MonCanUse(struct Pokemon *mon, enum Move move, u8 slotId)
{
    enum Species species;

    if (mon == NULL)
        return FALSE;
    if (GetMonData(mon, MON_DATA_IS_EGG))
        return FALSE;
    species = GetMonData(mon, MON_DATA_SPECIES);
    if (species == SPECIES_NONE)
        return FALSE;
    if (MonKnowsMove(mon, move))
        return TRUE;
    if (!MfFieldHm_BagHasMove(move))
        return FALSE;
    if (MfRules_HasBagHms() && CanLearnTeachableMove(species, move))
        return TRUE;
    if (slotId == 0 && MfFieldHm_OverwriteActive())
        return TRUE;
    return FALSE;
}

u8 MfFieldHm_FindPartySlot(enum Move move)
{
    u32 i;

    for (i = 0; i < PARTY_SIZE; i++)
    {
        struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][i];

        if (GetMonData(mon, MON_DATA_SPECIES) == SPECIES_NONE)
            break;
        if (MfFieldHm_MonCanUse(mon, move, i))
            return i;
    }
    return PARTY_SIZE;
}
