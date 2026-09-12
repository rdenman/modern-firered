#include "global.h"
#include "test/test.h"

// Smoke test: MF configs are visible in the Emerald TESTELF (`make check`).
// Keeps the test/modern_firered/ path exercised in CI.
TEST("MF: MF_VERSION is defined and current")
{
    EXPECT_EQ(MF_VERSION, 1);
}

TEST("MF: master switches default on")
{
    EXPECT_EQ(MF_RULES_ENGINE, TRUE);
    EXPECT_EQ(MF_RANDOMIZER, TRUE);
    EXPECT_EQ(MF_NUZLOCKE, TRUE);
    EXPECT_EQ(MF_OPTIONS_PLUS, TRUE);
}
