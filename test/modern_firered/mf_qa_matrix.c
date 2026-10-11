#include "global.h"
#include "mf_nuzlocke.h"
#include "mf_qa_matrix.h"
#include "mf_rules.h"
#include "mf_rules_conflicts.h"
#include "test/test.h"
#include "constants/pokemon.h"

static bool8 HasForbid(const struct ModernRules *r)
{
    const struct MfRulesConflict *c = MfRules_FindConflict(r);

    return c != NULL && c->action == MF_RULES_CONFLICT_FORBID;
}

TEST("MF: QA matrix required configs have no forbid")
{
    u32 i;
    struct ModernRules r;

    for (i = 0; i < MF_QA_REQUIRED_COUNT; i++)
    {
        MfQa_FillConfig(i, &r);
        EXPECT(!HasForbid(&r));
        EXPECT_EQ(r.version, MF_RULES_VERSION);
        EXPECT_EQ((u32)r.newLegendaries, FALSE);
    }
}

TEST("MF: QA Classic is vanilla gamemode with randomizer and Nuzlocke off")
{
    struct ModernRules r;

    MfQa_FillConfig(MF_QA_CLASSIC, &r);
    EXPECT_EQ((u32)r.gamemodePreset, (u32)MF_GAMEMODE_CLASSIC);
    EXPECT_EQ((u32)r.fairyTypes, FALSE);
    EXPECT_EQ((u32)r.modernStats, FALSE);
    EXPECT_EQ((u32)r.randomizerEnabled, FALSE);
    EXPECT_EQ((u32)r.nuzlocke, FALSE);
    EXPECT_EQ((u32)r.nuzlockeHardcore, FALSE);
    EXPECT_EQ((u32)r.monotype, 31);
}

TEST("MF: QA Modern matches InitNewGame gamemode fields")
{
    struct ModernRules filled;
    struct ModernRules *save = MfRules_GetSaveRules();

    MfRules_InitNewGame();
    MfQa_FillConfig(MF_QA_MODERN, &filled);
    filled.randomizerSeed = save->randomizerSeed;
    filled.rulesLocked = save->rulesLocked;
    EXPECT_EQ(memcmp(save, &filled, sizeof(struct ModernRules)), 0);
    EXPECT_EQ((u32)filled.gamemodePreset, (u32)MF_GAMEMODE_MODERN);
    EXPECT((u32)filled.fairyTypes);
}

TEST("MF: QA full randomizer enables remaps without chaos")
{
    struct ModernRules r;
    const struct MfRulesConflict *c;

    MfQa_FillConfig(MF_QA_FULL_RANDOMIZER, &r);
    EXPECT((u32)r.randomizerEnabled);
    EXPECT((u32)r.randomWild);
    EXPECT((u32)r.randomTrainer);
    EXPECT((u32)r.randomStarter);
    EXPECT((u32)r.randomItems);
    EXPECT((u32)r.randomType);
    EXPECT((u32)r.randomMoves);
    EXPECT_EQ((u32)r.randomChaos, FALSE);
    EXPECT_EQ((u32)r.nuzlocke, FALSE);
    c = MfRules_FindConflict(&r);
    EXPECT_EQ(c, NULL);
}

TEST("MF: QA Hardcore Nuzlocke seeds Set extras")
{
    struct ModernRules r;
    struct MfNuzlockeTierBundle bundle;

    MfQa_FillConfig(MF_QA_HARDCORE_NUZLOCKE, &r);
    MfNuzlocke_FillTierBundle(MF_NUZLOCKE_HARDCORE, &bundle);
    EXPECT((u32)r.nuzlocke);
    EXPECT((u32)r.nuzlockeHardcore);
    EXPECT((u32)r.noItemPlayer);
    EXPECT((u32)r.levelCap >= bundle.seedLevelCapIfOff);
    EXPECT(!HasForbid(&r));
}

TEST("MF: QA Fire monotype keeps Fairy On")
{
    struct ModernRules r;

    MfQa_FillConfig(MF_QA_MONOTYPE, &r);
    EXPECT_EQ((u32)r.monotype, (u32)TYPE_FIRE);
    EXPECT((u32)r.fairyTypes);
    EXPECT_EQ(MfRules_FindConflict(&r), NULL);
}

TEST("MF: QA Nuzlocke plus no Centers warns")
{
    struct ModernRules r;
    const struct MfRulesConflict *c;

    MfQa_FillConfig(MF_QA_NUZLOCKE_NO_CENTER, &r);
    EXPECT((u32)r.nuzlocke);
    EXPECT_EQ((u32)r.pokeCenterLimit, 1);
    c = MfRules_FindConflict(&r);
    EXPECT_NE(c, NULL);
    EXPECT_EQ(c->id, MF_RULES_CONFLICT_NUZLOCKE_NO_CENTER);
    EXPECT_EQ((u32)c->action, (u32)MF_RULES_CONFLICT_WARN);
}

TEST("MF: QA monotype plus randomizer warns")
{
    struct ModernRules r;
    const struct MfRulesConflict *c;

    MfQa_FillConfig(MF_QA_MONOTYPE_RANDOM, &r);
    EXPECT_EQ((u32)r.monotype, (u32)TYPE_FIRE);
    EXPECT((u32)r.randomizerEnabled);
    EXPECT((u32)r.randomWild);
    c = MfRules_FindConflict(&r);
    EXPECT_NE(c, NULL);
    EXPECT_EQ(c->id, MF_RULES_CONFLICT_MONOTYPE_RANDOM);
    EXPECT_EQ((u32)c->action, (u32)MF_RULES_CONFLICT_WARN);
}

TEST("MF: QA config pack unpack round-trip")
{
    struct ModernRules src;
    struct ModernRules dst;
    u8 blob[MF_RULES_SERIALIZED_SIZE_CURRENT];

    MfQa_FillConfig(MF_QA_FULL_RANDOMIZER, &src);
    MfRules_Pack(&src, blob);
    MfRules_Unpack(blob, &dst);
    EXPECT_EQ(memcmp(&src, &dst, sizeof(src)), 0);
}
