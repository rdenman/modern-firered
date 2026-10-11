#include "global.h"
#include "constants/characters.h"
#include "mf_branding.h"
#include "test/test.h"

// Smoke test: MF configs are visible in the Emerald TESTELF (`make check`).
// Keeps the test/modern_firered/ path exercised in CI.
TEST("MF: MF_VERSION is defined and current")
{
    EXPECT_EQ(MF_VERSION, 1);
}

TEST("MF: player-facing release stamp is 1.0.0")
{
    u8 buf[16];

    EXPECT_EQ(MF_RELEASE_VERSION_MAJOR, 1);
    EXPECT_EQ(MF_RELEASE_VERSION_MINOR, 0);
    EXPECT_EQ(MF_RELEASE_VERSION_PATCH, 0);
    MfBranding_WriteShortVersion(buf);
    EXPECT_EQ(buf[0], CHAR_v);
}

TEST("MF: credits mention RHH expansion version")
{
    u8 credits[96];
    u8 exp[16];
    u32 i;
    bool8 found = FALSE;

    MfBranding_WriteCredits(credits);
    MfBranding_AppendExpansionVersion(exp);
    for (i = 0; credits[i] != EOS; i++)
    {
        u32 j = 0;

        while (exp[j] != EOS && credits[i + j] == exp[j])
            j++;
        if (exp[j] == EOS)
        {
            found = TRUE;
            break;
        }
    }
    EXPECT(found);
}

TEST("MF: master switches default on")
{
    EXPECT_EQ(MF_RULES_ENGINE, TRUE);
    EXPECT_EQ(MF_RANDOMIZER, TRUE);
    EXPECT_EQ(MF_NUZLOCKE, TRUE);
    EXPECT_EQ(MF_OPTIONS_PLUS, TRUE);
}
