#include "global.h"
#include "item.h"
#include "mf_field_hm.h"
#include "mf_rules.h"
#include "pokemon.h"
#include "constants/items.h"
#include "constants/moves.h"
#include "constants/pokemon.h"
#include "constants/species.h"
#include "test/test.h"

static void ResetFieldHmState(void)
{
    struct ModernRules *save = MfRules_GetSaveRules();

    MfRules_ApplyDevDefaults(save);
    MfRules_ApplyGamemodePreset(save, MF_GAMEMODE_CLASSIC);
    save->rulesLocked = TRUE;
    ClearBag();
    ZeroPlayerPartyMons();
}

static void MakePartyMon(u8 slot, enum Species species)
{
    CreateMon(&gParties[B_TRAINER_PLAYER][slot], species, 5, 32, OTID_STRUCT_PRESET(slot + 1));
}

TEST("MF: Bag HMs Classic Off, Modern On")
{
    struct ModernRules rules;

    MfRules_ApplyDevDefaults(&rules);
    MfRules_ApplyGamemodePreset(&rules, MF_GAMEMODE_CLASSIC);
    EXPECT_EQ((u32)rules.bagHms, (u32)FALSE);

    MfRules_ApplyGamemodePreset(&rules, MF_GAMEMODE_MODERN);
    EXPECT_EQ((u32)rules.bagHms, (u32)TRUE);
}

TEST("MF: Bag HMs lets a compatible mon use an untaught HM from the bag")
{
    ASSUME(CanLearnTeachableMove(SPECIES_BULBASAUR, MOVE_CUT));
    ASSUME(!CanLearnTeachableMove(SPECIES_MAGIKARP, MOVE_CUT));

    ResetFieldHmState();
    MakePartyMon(0, SPECIES_BULBASAUR);
    EXPECT_EQ(MfFieldHm_FindPartySlot(MOVE_CUT), PARTY_SIZE);

    AddBagItem(ITEM_HM_CUT, 1);
    EXPECT_EQ(MfFieldHm_FindPartySlot(MOVE_CUT), PARTY_SIZE);

    MfRules_GetSaveRules()->bagHms = TRUE;
    EXPECT_EQ(MfFieldHm_FindPartySlot(MOVE_CUT), 0);

    ZeroPlayerPartyMons();
    MakePartyMon(0, SPECIES_MAGIKARP);
    EXPECT_EQ(MfFieldHm_FindPartySlot(MOVE_CUT), PARTY_SIZE);

    ResetFieldHmState();
}

TEST("MF: knowing an HM still works without Bag HMs or the item")
{
    enum Move cut = MOVE_CUT;

    ResetFieldHmState();
    MakePartyMon(0, SPECIES_MAGIKARP);
    SetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_MOVE1, &cut);
    EXPECT_EQ(MfFieldHm_FindPartySlot(MOVE_CUT), 0);
    ResetFieldHmState();
}

TEST("MF: Nuzlocke overwrite lets the lead use a bag HM it cannot learn")
{
    ASSUME(!CanLearnTeachableMove(SPECIES_MAGIKARP, MOVE_CUT));
    ASSUME(CanLearnTeachableMove(SPECIES_BULBASAUR, MOVE_CUT));

    ResetFieldHmState();
    MakePartyMon(0, SPECIES_MAGIKARP);
    MakePartyMon(1, SPECIES_BULBASAUR);
    AddBagItem(ITEM_HM_CUT, 1);
    MfRules_GetSaveRules()->nuzlockeEasy = TRUE;
    EXPECT(MfFieldHm_OverwriteActive());
    EXPECT_EQ(MfFieldHm_FindPartySlot(MOVE_CUT), 0);

    MfRules_GetSaveRules()->nuzlockeEasy = FALSE;
    MfRules_GetSaveRules()->bagHms = TRUE;
    EXPECT_EQ(MfFieldHm_FindPartySlot(MOVE_CUT), 1);
    ResetFieldHmState();
}

TEST("MF: overwrite skips an egg lead and ignores Random TMs")
{
    u8 isEgg = TRUE;
    enum Type electric = TYPE_ELECTRIC;

    ASSUME(!CanLearnTeachableMove(SPECIES_MAGIKARP, MOVE_CUT));

    ResetFieldHmState();
    MakePartyMon(0, SPECIES_MAGIKARP);
    SetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_IS_EGG, &isEgg);
    AddBagItem(ITEM_HM_CUT, 1);
    MfRules_GetSaveRules()->nuzlocke = TRUE;
    EXPECT_EQ(MfFieldHm_FindPartySlot(MOVE_CUT), PARTY_SIZE);

    ResetFieldHmState();
    MakePartyMon(0, SPECIES_MAGIKARP);
    AddBagItem(ITEM_HM_CUT, 1);
    MfRules_GetSaveRules()->randomTms = TRUE;
    EXPECT(!MfFieldHm_OverwriteActive());
    EXPECT_EQ(MfFieldHm_FindPartySlot(MOVE_CUT), PARTY_SIZE);

    MfRules_GetSaveRules()->randomTms = FALSE;
    MfRules_GetSaveRules()->monotype = electric;
    EXPECT(MfFieldHm_OverwriteActive());
    EXPECT_EQ(MfFieldHm_FindPartySlot(MOVE_CUT), 0);
    ResetFieldHmState();
}
