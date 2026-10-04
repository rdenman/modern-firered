#include "global.h"
#include "test/test.h"
#include "mf_catch.h"
#include "mf_rules.h"

static struct ModernRules *PrepareRules(void)
{
    struct ModernRules *save = MfRules_GetSaveRules();

    MfRules_ApplyDevDefaults(save);
    save->rulesLocked = TRUE;
    save->catchRate = MF_CATCH_RATE_DEFAULT;
    save->lessEscapes = FALSE;
    save->escapeRopeDig = FALSE;
    return save;
}

TEST("MF: catch rate scales 1x / 0.5x / 2x / 3x")
{
    EXPECT_EQ(MfScaleCatchRate(100, MF_CATCH_RATE_DEFAULT), 100);
    EXPECT_EQ(MfScaleCatchRate(100, MF_CATCH_RATE_HALF), 50);
    EXPECT_EQ(MfScaleCatchRate(100, MF_CATCH_RATE_2X), 200);
    EXPECT_EQ(MfScaleCatchRate(100, MF_CATCH_RATE_3X), 255);

    EXPECT_EQ(MfScaleCatchRate(45, MF_CATCH_RATE_HALF), 22);
    EXPECT_EQ(MfScaleCatchRate(5, MF_CATCH_RATE_HALF), MF_CATCH_RATE_FLOOR);
    EXPECT_EQ(MfScaleCatchRate(3, MF_CATCH_RATE_HALF), MF_CATCH_RATE_FLOOR);
    EXPECT_EQ(MfScaleCatchRate(1, MF_CATCH_RATE_HALF), MF_CATCH_RATE_FLOOR);

    EXPECT_EQ(MfScaleCatchRate(200, MF_CATCH_RATE_2X), MF_CATCH_RATE_CAP);
    EXPECT_EQ(MfScaleCatchRate(90, MF_CATCH_RATE_3X), MF_CATCH_RATE_CAP);
    EXPECT_EQ(MfScaleCatchRate(80, MF_CATCH_RATE_3X), 240);

    EXPECT_EQ(MfScaleCatchRate(100, 4), 100);
    EXPECT_EQ(MfScaleCatchRate(100, 255), 100);
}

TEST("MF: live catch-rate rule and escape bans")
{
    struct ModernRules *save = PrepareRules();

    save->catchRate = MF_CATCH_RATE_DEFAULT;
    EXPECT_EQ(MfApplyCatchRateRule(90), 90);
    EXPECT(MfAreEscapeRopeAndDigAllowed());
    EXPECT(!MfLessEscapesFleeSucceeds(100, 200, 0, 255));

    save->catchRate = MF_CATCH_RATE_HALF;
    EXPECT_EQ(MfApplyCatchRateRule(90), 45);

    save->catchRate = MF_CATCH_RATE_2X;
    EXPECT_EQ(MfApplyCatchRateRule(90), 180);

    save->catchRate = MF_CATCH_RATE_3X;
    EXPECT_EQ(MfApplyCatchRateRule(90), 255);

    save->escapeRopeDig = TRUE;
    EXPECT(!MfAreEscapeRopeAndDigAllowed());
}

TEST("MF: less-escapes flee always rolls and extra tries help")
{
    u32 equalSpeed = MfComputeFleeChanceVar(100, 100, 0);
    u32 faster = MfComputeFleeChanceVar(200, 100, 0);

    EXPECT_EQ(equalSpeed, 128u);
    EXPECT_EQ(faster, 256u);
    // Foe speed 0 is treated as 1 so we never divide by zero.
    EXPECT_EQ(MfComputeFleeChanceVar(100, 0, 0), 12800u);

    // Faster still fails a high roll on try 0; guaranteed after enough tries.
    EXPECT(!MfLessEscapesFleeSucceeds(200, 100, 0, MF_LESS_ESCAPES_ROLL_MASK));
    EXPECT(MfLessEscapesFleeSucceeds(200, 100, 0, 0));
    EXPECT(MfLessEscapesFleeSucceeds(100, 100, 13, MF_LESS_ESCAPES_ROLL_MASK));
    EXPECT(!MfLessEscapesFleeSucceeds(100, 100, 12, MF_LESS_ESCAPES_ROLL_MASK));
}
