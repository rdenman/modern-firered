#include "global.h"
#include "test/test.h"
#include "config/summary_screen.h"

// S09: lock Phase 1 summary IV/EV baseline (always-on, raw values, tileset).
// Shared Emerald TESTELF path — same summary configs as the FireRed ROM.
// FR layout/legibility remains manual: docs-mf/manual-qa-s09-summary-iv-ev.md

TEST("MF: summary IV/EV display configs are always-on with raw values")
{
    EXPECT(P_SUMMARY_SCREEN_IV_EV_INFO);
    EXPECT(!P_SUMMARY_SCREEN_IV_EV_BOX_ONLY);
    EXPECT(P_SUMMARY_SCREEN_IV_EV_VALUES);
    EXPECT(P_SUMMARY_SCREEN_IV_EV_TILESET);
    EXPECT(!P_SUMMARY_SCREEN_IV_ONLY);
    EXPECT(!P_SUMMARY_SCREEN_EV_ONLY);
    EXPECT_EQ(P_FLAG_SUMMARY_SCREEN_IV_EV_INFO, 0);
}
