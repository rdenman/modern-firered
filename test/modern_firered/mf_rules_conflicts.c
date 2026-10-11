#include "global.h"
#include "mf_evolution.h"
#include "mf_exp.h"
#include "mf_monotype.h"
#include "mf_rules.h"
#include "mf_rules_conflicts.h"
#include "test/test.h"
#include "constants/pokemon.h"

static void EmptyRules(struct ModernRules *r)
{
    MfRules_ResetToEmpty(r);
}

TEST("MF: default rules have no conflicts")
{
    struct ModernRules r;

    EmptyRules(&r);
    EXPECT_EQ(MfRules_FindConflict(&r), NULL);
    EXPECT_EQ((u32)MfRules_EvaluatePair(&r, MF_RULES_CONFLICT_FAIRY_MONOTYPE), (u32)MF_RULES_CONFLICT_ALLOW);
}

TEST("MF: Fairy monotype with Fairy Off is forbidden")
{
    struct ModernRules r;

    EmptyRules(&r);
    r.monotype = TYPE_FAIRY;
    r.fairyTypes = FALSE;
    EXPECT_EQ((u32)MfRules_EvaluatePair(&r, MF_RULES_CONFLICT_FAIRY_MONOTYPE), (u32)MF_RULES_CONFLICT_FORBID);
    EXPECT_EQ(MfRules_FindConflict(&r)->id, MF_RULES_CONFLICT_FAIRY_MONOTYPE);
    EXPECT_EQ((u32)MfRules_FindConflict(&r)->action, (u32)MF_RULES_CONFLICT_FORBID);

    r.fairyTypes = TRUE;
    EXPECT_EQ((u32)MfRules_EvaluatePair(&r, MF_RULES_CONFLICT_FAIRY_MONOTYPE), (u32)MF_RULES_CONFLICT_ALLOW);
    EXPECT_EQ(MfRules_FindConflict(&r), NULL);
}

TEST("MF: monotype plus species randomizer warns")
{
    struct ModernRules r;

    EmptyRules(&r);
    r.monotype = TYPE_FIRE;
    r.randomizerEnabled = TRUE;
    r.randomWild = TRUE;
    EXPECT_EQ((u32)MfRules_EvaluatePair(&r, MF_RULES_CONFLICT_MONOTYPE_RANDOM), (u32)MF_RULES_CONFLICT_WARN);

    r.randomWild = FALSE;
    EXPECT_EQ((u32)MfRules_EvaluatePair(&r, MF_RULES_CONFLICT_MONOTYPE_RANDOM), (u32)MF_RULES_CONFLICT_ALLOW);
}

TEST("MF: monotype plus Nuzlocke warns")
{
    struct ModernRules r;

    EmptyRules(&r);
    r.monotype = TYPE_WATER;
    r.nuzlocke = TRUE;
    EXPECT_EQ((u32)MfRules_EvaluatePair(&r, MF_RULES_CONFLICT_MONOTYPE_NUZLOCKE), (u32)MF_RULES_CONFLICT_WARN);

    r.nuzlocke = FALSE;
    r.nuzlockeEasy = TRUE;
    EXPECT_EQ((u32)MfRules_EvaluatePair(&r, MF_RULES_CONFLICT_MONOTYPE_NUZLOCKE), (u32)MF_RULES_CONFLICT_ALLOW);
}

TEST("MF: Nuzlocke plus no Poké Center warns")
{
    struct ModernRules r;

    EmptyRules(&r);
    r.nuzlocke = TRUE;
    r.pokeCenterLimit = 1;
    EXPECT_EQ((u32)MfRules_EvaluatePair(&r, MF_RULES_CONFLICT_NUZLOCKE_NO_CENTER), (u32)MF_RULES_CONFLICT_WARN);

    r.noPcHeal = TRUE;
    EXPECT_EQ((u32)MfRules_EvaluatePair(&r, MF_RULES_CONFLICT_NUZLOCKE_NO_HEAL), (u32)MF_RULES_CONFLICT_WARN);
    EXPECT_EQ(MfRules_FindConflict(&r)->id, MF_RULES_CONFLICT_NUZLOCKE_NO_HEAL);

    r.nuzlocke = FALSE;
    EXPECT_EQ((u32)MfRules_EvaluatePair(&r, MF_RULES_CONFLICT_NUZLOCKE_NO_CENTER), (u32)MF_RULES_CONFLICT_ALLOW);
}

TEST("MF: chaos plus Nuzlocke warns")
{
    struct ModernRules r;

    EmptyRules(&r);
    r.nuzlocke = TRUE;
    r.randomizerEnabled = TRUE;
    r.randomChaos = TRUE;
    EXPECT_EQ((u32)MfRules_EvaluatePair(&r, MF_RULES_CONFLICT_CHAOS_NUZLOCKE), (u32)MF_RULES_CONFLICT_WARN);

    r.nuzlocke = FALSE;
    EXPECT_EQ((u32)MfRules_EvaluatePair(&r, MF_RULES_CONFLICT_CHAOS_NUZLOCKE), (u32)MF_RULES_CONFLICT_ALLOW);
}

TEST("MF: Nuzlocke plus EXP x0 warns")
{
    struct ModernRules r;

    EmptyRules(&r);
    r.nuzlockeEasy = TRUE;
    r.expMultiplier = MF_EXP_MULT_0X;
    EXPECT_EQ((u32)MfRules_EvaluatePair(&r, MF_RULES_CONFLICT_NUZLOCKE_EXP_ZERO), (u32)MF_RULES_CONFLICT_WARN);

    r.expMultiplier = MF_EXP_MULT_1X;
    EXPECT_EQ((u32)MfRules_EvaluatePair(&r, MF_RULES_CONFLICT_NUZLOCKE_EXP_ZERO), (u32)MF_RULES_CONFLICT_ALLOW);
}

TEST("MF: Evo Limit All plus random evo warns")
{
    struct ModernRules r;

    EmptyRules(&r);
    r.evoLimit = MF_EVO_LIMIT_NONE;
    r.randomizerEnabled = TRUE;
    r.randomEvolutionMethods = TRUE;
    EXPECT_EQ((u32)MfRules_EvaluatePair(&r, MF_RULES_CONFLICT_EVO_ALL_RANDOM_EVO), (u32)MF_RULES_CONFLICT_WARN);

    r.randomEvolutionMethods = FALSE;
    r.randomEvolution = TRUE;
    EXPECT_EQ((u32)MfRules_EvaluatePair(&r, MF_RULES_CONFLICT_EVO_ALL_RANDOM_EVO), (u32)MF_RULES_CONFLICT_WARN);

    r.randomizerEnabled = FALSE;
    EXPECT_EQ((u32)MfRules_EvaluatePair(&r, MF_RULES_CONFLICT_EVO_ALL_RANDOM_EVO), (u32)MF_RULES_CONFLICT_ALLOW);
}

TEST("MF: level cap plus hard EXP and BST plus modern stats are allowed")
{
    struct ModernRules r;

    EmptyRules(&r);
    r.levelCap = 2;
    r.hardExp = FALSE;
    EXPECT_EQ(MfRules_FindConflict(&r), NULL);

    EmptyRules(&r);
    r.modernStats = TRUE;
    r.baseStatEqualizer = 3;
    EXPECT_EQ(MfRules_FindConflict(&r), NULL);
}

TEST("MF: forbid outranks concurrent warns")
{
    struct ModernRules r;

    EmptyRules(&r);
    r.monotype = TYPE_FAIRY;
    r.fairyTypes = FALSE;
    r.nuzlocke = TRUE;
    r.randomizerEnabled = TRUE;
    r.randomChaos = TRUE;
    EXPECT_EQ(MfRules_FindConflict(&r)->id, MF_RULES_CONFLICT_FAIRY_MONOTYPE);
    EXPECT_EQ((u32)MfRules_FindConflict(&r)->action, (u32)MF_RULES_CONFLICT_FORBID);
}
