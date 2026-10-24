#include "global.h"
#include "event_data.h"
#include "item.h"
#include "mf_pokevial.h"
#include "mf_rules.h"
#include "pokemon.h"
#include "string_util.h"
#include "text.h"
#include "constants/battle.h"
#include "constants/characters.h"
#include "constants/flags.h"
#include "constants/items.h"
#include "constants/moves.h"
#include "constants/pokemon.h"
#include "constants/vars.h"

static const u8 sPokeVialDescBase[] = _(
    "Potent medicine\n"
    "that fully restores\n"
    "all Pokémon.");

static const u8 sText_PokeVialEmpty[] = _(
    "The Poké Vial is empty.\n"
    "Refill it at a Pokémon Center.{PAUSE_UNTIL_PRESS}");

static const u8 sText_PokeVialNothing[] = _(
    "Your Pokémon are already\n"
    "healthy.{PAUSE_UNTIL_PRESS}");

static const u8 sText_UsedPokeVial[] = _(
    "{PLAYER} used the Poké Vial.\n"
    "The party was restored!{PAUSE_UNTIL_PRESS}");

static u8 sPokeVialDescBuffer[64];

static bool32 MfPokeVialSkipFainted(void)
{
    return MfRules_GetValue(MF_RULE_VAL_NUZLOCKE_MODE) != MF_NUZLOCKE_OFF;
}

static u8 MfPokeVialCountBadges(void)
{
    u8 count = 0;
    u16 flag;

    for (flag = FLAG_BADGE01_GET; flag < FLAG_BADGE01_GET + NUM_BADGES; flag++)
    {
        if (FlagGet(flag))
            count++;
    }
    return count;
}

u8 MfPokeVialMaxChargesFromBadgeCount(u8 badges)
{
    u8 max;

    if (badges > NUM_BADGES)
        badges = NUM_BADGES;
    max = (badges + 2) / 3;
    if (max < 1)
        max = 1;
    if (max > MF_POKEVIAL_MAX_CHARGES)
        max = MF_POKEVIAL_MAX_CHARGES;
    return max;
}

u8 MfPokeVialGetMaxCharges(void)
{
    return MfPokeVialMaxChargesFromBadgeCount(MfPokeVialCountBadges());
}

u8 MfPokeVialGetRemaining(void)
{
    u8 remaining = (u8)VarGet(VAR_POKE_VIAL_CHARGES);
    u8 max = MfPokeVialGetMaxCharges();

    if (remaining > max)
        remaining = max;
    return remaining;
}

static bool32 MfPokeVialMonNeedsHeal(struct Pokemon *mon)
{
    u32 i;
    u8 ppBonuses;
    u16 species;
    u16 hp;
    u32 status;

    species = GetMonData(mon, MON_DATA_SPECIES);
    if (species == SPECIES_NONE)
        return FALSE;
    if (GetMonData(mon, MON_DATA_IS_EGG))
        return FALSE;

    hp = GetMonData(mon, MON_DATA_HP);
    if (MfPokeVialSkipFainted() && hp == 0)
        return FALSE;

    if (hp < GetMonData(mon, MON_DATA_MAX_HP))
        return TRUE;

    status = GetMonData(mon, MON_DATA_STATUS);
    if (status != STATUS1_NONE)
        return TRUE;

    ppBonuses = GetMonData(mon, MON_DATA_PP_BONUSES);
    for (i = 0; i < MAX_MON_MOVES; i++)
    {
        u16 move = GetMonData(mon, MON_DATA_MOVE1 + i);
        u8 pp;
        u8 maxPP;

        if (move == MOVE_NONE)
            continue;
        pp = GetMonData(mon, MON_DATA_PP1 + i);
        maxPP = CalculatePPWithBonus(move, ppBonuses, i);
        if (pp < maxPP)
            return TRUE;
    }
    return FALSE;
}

bool32 MfPokeVialPartyNeedsHeal(void)
{
    u32 i;

    for (i = 0; i < PARTY_SIZE; i++)
    {
        if (MfPokeVialMonNeedsHeal(&gParties[B_TRAINER_PLAYER][i]))
            return TRUE;
    }
    return FALSE;
}

static void MfPokeVialHealParty(void)
{
    u32 i;
    bool32 skipFainted = MfPokeVialSkipFainted();

    for (i = 0; i < PARTY_SIZE; i++)
    {
        struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][i];

        if (GetMonData(mon, MON_DATA_SPECIES) == SPECIES_NONE)
            continue;
        if (GetMonData(mon, MON_DATA_IS_EGG))
            continue;
        if (skipFainted && GetMonData(mon, MON_DATA_HP) == 0)
            continue;
        HealPokemon(mon);
    }
}

enum MfPokeVialUseResult MfPokeVialWouldUse(void)
{
    if (!MfRules_HasPokeVial())
        return MF_POKEVIAL_USE_DISABLED;
    if (MfPokeVialGetRemaining() == 0)
        return MF_POKEVIAL_USE_EMPTY;
    if (!MfPokeVialPartyNeedsHeal())
        return MF_POKEVIAL_USE_NOTHING;
    return MF_POKEVIAL_USE_OK;
}

enum MfPokeVialUseResult MfPokeVialTryUse(void)
{
    enum MfPokeVialUseResult result = MfPokeVialWouldUse();

    if (result != MF_POKEVIAL_USE_OK)
        return result;

    MfPokeVialHealParty();
    VarSet(VAR_POKE_VIAL_CHARGES, MfPokeVialGetRemaining() - 1);
    return MF_POKEVIAL_USE_OK;
}

void MfPokeVialGrantOnNewGame(void)
{
    if (!MfRules_HasPokeVial())
        return;
    if (!CheckBagHasItem(ITEM_POKE_VIAL, 1))
        AddBagItem(ITEM_POKE_VIAL, 1);
    VarSet(VAR_POKE_VIAL_CHARGES, MfPokeVialGetMaxCharges());
}

u16 MfPokeVialRefill(void)
{
    if (!MfRules_HasPokeVial())
        return FALSE;
    if (!CheckBagHasItem(ITEM_POKE_VIAL, 1))
        return FALSE;
    VarSet(VAR_POKE_VIAL_CHARGES, MfPokeVialGetMaxCharges());
    return TRUE;
}

const u8 *MfPokeVialGetDescription(void)
{
    u8 *end;

    StringCopy(sPokeVialDescBuffer, sPokeVialDescBase);
    end = sPokeVialDescBuffer;
    while (*end != EOS)
        end++;
    *end++ = CHAR_SPACE;
    *end++ = CHAR_LEFT_PAREN;
    ConvertIntToDecimalStringN(end, MfPokeVialGetRemaining(), STR_CONV_MODE_LEFT_ALIGN, 1);
    while (*end != EOS)
        end++;
    *end++ = CHAR_SLASH;
    ConvertIntToDecimalStringN(end, MfPokeVialGetMaxCharges(), STR_CONV_MODE_LEFT_ALIGN, 1);
    while (*end != EOS)
        end++;
    *end++ = CHAR_RIGHT_PAREN;
    *end = EOS;
    return sPokeVialDescBuffer;
}

const u8 *MfPokeVialGetUseMessage(enum MfPokeVialUseResult result)
{
    switch (result)
    {
    case MF_POKEVIAL_USE_EMPTY:
        return sText_PokeVialEmpty;
    case MF_POKEVIAL_USE_NOTHING:
        return sText_PokeVialNothing;
    case MF_POKEVIAL_USE_OK:
        return sText_UsedPokeVial;
    case MF_POKEVIAL_USE_DISABLED:
    default:
        return NULL;
    }
}
