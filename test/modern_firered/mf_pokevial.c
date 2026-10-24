#include "global.h"
#include "event_data.h"
#include "mf_pokevial.h"
#include "mf_rules.h"
#include "pokemon.h"
#include "constants/battle.h"
#include "constants/moves.h"
#include "constants/pokemon.h"
#include "constants/species.h"
#include "constants/vars.h"
#include "test/test.h"

static struct ModernRules *PrepareVialRules(bool8 nuzlocke)
{
    struct ModernRules *save = MfRules_GetSaveRules();

    MfRules_ApplyDevDefaults(save);
    save->pokeVial = TRUE;
    save->nuzlocke = nuzlocke;
    save->nuzlockeEasy = FALSE;
    save->nuzlockeHardcore = FALSE;
    save->rulesLocked = TRUE;
    return save;
}

static void ClearPlayerParty(void)
{
    ZeroPlayerPartyMons();
}

static struct Pokemon *MakePartyMon(enum Species species)
{
    struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][gPartiesCount[B_TRAINER_PLAYER]];

    CreateMon(mon, species, 5, 32, OTID_STRUCT_PRESET(0));
    CalculateMonStats(mon);
    GiveMonInitialMoveset(mon);
    gPartiesCount[B_TRAINER_PLAYER]++;
    return mon;
}

TEST("MF: poke vial max charges follow ceil(badges/3) with a floor of 1")
{
    EXPECT_EQ(MfPokeVialMaxChargesFromBadgeCount(0), 1);
    EXPECT_EQ(MfPokeVialMaxChargesFromBadgeCount(1), 1);
    EXPECT_EQ(MfPokeVialMaxChargesFromBadgeCount(3), 1);
    EXPECT_EQ(MfPokeVialMaxChargesFromBadgeCount(4), 2);
    EXPECT_EQ(MfPokeVialMaxChargesFromBadgeCount(6), 2);
    EXPECT_EQ(MfPokeVialMaxChargesFromBadgeCount(7), 3);
    EXPECT_EQ(MfPokeVialMaxChargesFromBadgeCount(8), 3);
}

TEST("MF: poke vial refuses a fully healthy party")
{
    PrepareVialRules(FALSE);
    ClearPlayerParty();
    MakePartyMon(SPECIES_BULBASAUR);
    VarSet(VAR_POKE_VIAL_CHARGES, 1);

    EXPECT(!MfPokeVialPartyNeedsHeal());
    EXPECT_EQ((u32)MfPokeVialWouldUse(), (u32)MF_POKEVIAL_USE_NOTHING);
    EXPECT_EQ((u32)MfPokeVialTryUse(), (u32)MF_POKEVIAL_USE_NOTHING);
    EXPECT_EQ(MfPokeVialGetRemaining(), 1);
}

TEST("MF: poke vial spends a charge to restore PP")
{
    struct Pokemon *mon;
    u8 pp = 1;

    PrepareVialRules(FALSE);
    ClearPlayerParty();
    mon = MakePartyMon(SPECIES_BULBASAUR);
    SetMonData(mon, MON_DATA_PP1, &pp);
    VarSet(VAR_POKE_VIAL_CHARGES, 1);

    EXPECT(MfPokeVialPartyNeedsHeal());
    EXPECT_EQ((u32)MfPokeVialTryUse(), (u32)MF_POKEVIAL_USE_OK);
    EXPECT_EQ(MfPokeVialGetRemaining(), 0);
    EXPECT_GT(GetMonData(mon, MON_DATA_PP1), 1);
}

TEST("MF: poke vial nuzlocke skips fainted mons and keeps the charge")
{
    struct Pokemon *mon;
    u16 hp = 0;

    PrepareVialRules(TRUE);
    ClearPlayerParty();
    mon = MakePartyMon(SPECIES_BULBASAUR);
    SetMonData(mon, MON_DATA_HP, &hp);
    VarSet(VAR_POKE_VIAL_CHARGES, 1);

    EXPECT(!MfPokeVialPartyNeedsHeal());
    EXPECT_EQ((u32)MfPokeVialTryUse(), (u32)MF_POKEVIAL_USE_NOTHING);
    EXPECT_EQ(MfPokeVialGetRemaining(), 1);
    EXPECT_EQ(GetMonData(mon, MON_DATA_HP), 0);
}

TEST("MF: poke vial off-nuzlocke revives fainted mons")
{
    struct Pokemon *mon;
    u16 hp = 0;

    PrepareVialRules(FALSE);
    ClearPlayerParty();
    mon = MakePartyMon(SPECIES_BULBASAUR);
    SetMonData(mon, MON_DATA_HP, &hp);
    VarSet(VAR_POKE_VIAL_CHARGES, 1);

    EXPECT(MfPokeVialPartyNeedsHeal());
    EXPECT_EQ((u32)MfPokeVialTryUse(), (u32)MF_POKEVIAL_USE_OK);
    EXPECT_EQ(MfPokeVialGetRemaining(), 0);
    EXPECT_GT(GetMonData(mon, MON_DATA_HP), 0);
}

TEST("MF: poke vial empty and disabled refuse without healing")
{
    struct Pokemon *mon;
    u8 pp = 1;
    struct ModernRules *save;

    save = PrepareVialRules(FALSE);
    ClearPlayerParty();
    mon = MakePartyMon(SPECIES_BULBASAUR);
    SetMonData(mon, MON_DATA_PP1, &pp);

    VarSet(VAR_POKE_VIAL_CHARGES, 0);
    EXPECT_EQ((u32)MfPokeVialWouldUse(), (u32)MF_POKEVIAL_USE_EMPTY);

    save->pokeVial = FALSE;
    VarSet(VAR_POKE_VIAL_CHARGES, 1);
    EXPECT_EQ((u32)MfPokeVialWouldUse(), (u32)MF_POKEVIAL_USE_DISABLED);
    EXPECT_EQ(GetMonData(mon, MON_DATA_PP1), 1);
}
