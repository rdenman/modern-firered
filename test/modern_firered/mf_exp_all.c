#include "global.h"
#include "battle_util.h"
#include "event_data.h"
#include "item.h"
#include "mf_exp_all.h"
#include "mf_rules.h"
#include "constants/items.h"
#include "constants/vars.h"
#include "test/test.h"

static struct ModernRules *PrepareExpAllRules(bool8 enabled)
{
    struct ModernRules *save = MfRules_GetSaveRules();

    MfRules_ApplyDevDefaults(save);
    save->expAll = enabled;
    save->rulesLocked = TRUE;
    VarSet(VAR_EXP_ALL_ON, 0);
    return save;
}

static void ResetExpAllState(void)
{
    struct ModernRules *save = MfRules_GetSaveRules();

    save->expAll = FALSE;
    VarSet(VAR_EXP_ALL_ON, 0);
}

TEST("MF: Exp All is off unless the Features rule and var are both set")
{
    PrepareExpAllRules(FALSE);
    EXPECT_EQ((u32)MfExpAllIsEnabled(), (u32)FALSE);

    VarSet(VAR_EXP_ALL_ON, 1);
    EXPECT_EQ((u32)MfExpAllIsEnabled(), (u32)FALSE);

    PrepareExpAllRules(TRUE);
    EXPECT_EQ((u32)MfExpAllIsEnabled(), (u32)FALSE);

    VarSet(VAR_EXP_ALL_ON, 1);
    EXPECT_EQ((u32)MfExpAllIsEnabled(), (u32)TRUE);
    EXPECT_EQ((u32)IsGen6ExpShareEnabled(), (u32)TRUE);

    VarSet(VAR_EXP_ALL_ON, 0);
    EXPECT_EQ((u32)MfExpAllIsEnabled(), (u32)FALSE);
    EXPECT_EQ((u32)IsGen6ExpShareEnabled(), (u32)FALSE);
    ResetExpAllState();
}

TEST("MF: Exp All toggle flips the var only when the Features rule is on")
{
    PrepareExpAllRules(TRUE);
    EXPECT_EQ((u32)MfExpAllToggle(), (u32)TRUE);
    EXPECT_EQ(VarGet(VAR_EXP_ALL_ON), 1u);
    EXPECT_EQ((u32)MfExpAllToggle(), (u32)FALSE);
    EXPECT_EQ(VarGet(VAR_EXP_ALL_ON), 0u);

    PrepareExpAllRules(FALSE);
    EXPECT_EQ((u32)MfExpAllToggle(), (u32)FALSE);
    EXPECT_EQ(VarGet(VAR_EXP_ALL_ON), 1u);
    ResetExpAllState();
}

TEST("MF: Exp All New Game grant puts the Key Item in the bag and starts on")
{
    PrepareExpAllRules(FALSE);
    ClearBag();
    MfExpAllGrantOnNewGame();
    EXPECT_EQ((u32)CheckBagHasItem(ITEM_EXP_ALL, 1), (u32)FALSE);
    EXPECT_EQ(VarGet(VAR_EXP_ALL_ON), 0u);

    PrepareExpAllRules(TRUE);
    ClearBag();
    MfExpAllGrantOnNewGame();
    EXPECT_EQ((u32)CheckBagHasItem(ITEM_EXP_ALL, 1), (u32)TRUE);
    EXPECT_EQ(VarGet(VAR_EXP_ALL_ON), 1u);
    EXPECT_EQ((u32)MfExpAllIsEnabled(), (u32)TRUE);
    ResetExpAllState();
    ClearBag();
}
