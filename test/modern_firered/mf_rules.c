#include "global.h"
#include "mf_rules.h"
#include "save.h"
#include "test/test.h"

static void FillRulesWithPattern(struct ModernRules *rules)
{
    MfRules_ResetToEmpty(rules);

    rules->gamemodePreset = MF_GAMEMODE_MODERN;
    rules->rulesLocked = TRUE;

    rules->infiniteTms = TRUE;
    rules->survivePoison = TRUE;
    rules->synchronize = TRUE;
    rules->mints = TRUE;
    rules->modernSitrus = TRUE;
    rules->modernTypes = TRUE;
    rules->fairyTypes = TRUE;
    rules->modernStats = TRUE;
    rules->sturdy = TRUE;
    rules->modernMoves = TRUE;
    rules->legendaryAbilities = TRUE;
    rules->typeEffectiveness = TRUE;
    rules->alternateSpawns = 2;

    rules->shinyChance = 9;
    rules->wildItemDrops = TRUE;
    rules->rtcType = TRUE;
    rules->shinyColors = TRUE;
    rules->wonderTrade = TRUE;

    rules->randomStarter = TRUE;
    rules->randomWild = TRUE;
    rules->randomTrainer = TRUE;
    rules->randomStatic = TRUE;
    rules->randomSimilar = TRUE;
    rules->randomMapBased = TRUE;
    rules->randomIncludeLegendaries = TRUE;
    rules->randomType = TRUE;
    rules->randomMoves = TRUE;
    rules->randomAbilities = TRUE;
    rules->randomEvolution = TRUE;
    rules->randomEvolutionMethods = TRUE;
    rules->randomTypeEffectiveness = TRUE;
    rules->randomItems = TRUE;
    rules->randomChaos = TRUE;

    rules->nuzlocke = TRUE;
    rules->nuzlockeHardcore = TRUE;
    rules->nuzlockeSpeciesClause = TRUE;
    rules->nuzlockeShinyClause = TRUE;
    rules->nuzlockeNicknaming = TRUE;
    rules->nuzlockeDeletion = TRUE;

    rules->partyLimit = 3;
    rules->levelCap = 2;
    rules->expMultiplier = 2;
    rules->noItemPlayer = TRUE;
    rules->noItemTrainer = TRUE;
    rules->noEvs = TRUE;
    rules->scalingIvs = 2;
    rules->scalingEvs = 3;
    rules->maxPartyIvs = 2;
    rules->pokeCenterLimit = 1;
    rules->lockDifficulty = TRUE;
    rules->escapeRopeDig = TRUE;
    rules->hardExp = TRUE;
    rules->catchRate = 5;

    rules->evoLimit = 2;
    rules->baseStatEqualizer = 3;
    rules->lessEscapes = TRUE;
    rules->mirror = TRUE;
    rules->mirrorThief = TRUE;
    rules->noPcHeal = TRUE;
    rules->monotype = 12;
    rules->expensiveShops = 4;

    rules->randomizerSeed = 0xA5A5F00Du;
    rules->nuzlockeEncounterFlags[0] = 0xAAu;
    rules->nuzlockeEncounterFlags[MF_NUZLOCKE_ENCOUNTER_FLAG_BYTES - 1] = 0x55u;
}

TEST("MF: ModernRules fits save budget")
{
    EXPECT_LE(sizeof(struct ModernRules), MF_RULES_MAX_BYTES);
    EXPECT_LE(sizeof(struct SaveBlock3), SAVE_BLOCK_3_CHUNK_SIZE * NUM_SECTORS_PER_SLOT);
}

TEST("MF: rules pack/unpack round-trip")
{
    struct ModernRules original;
    struct ModernRules restored;
    u8 blob[MF_RULES_SERIALIZED_SIZE];

    FillRulesWithPattern(&original);
    memset(blob, 0xFF, sizeof(blob));
    memset(&restored, 0x00, sizeof(restored));

    MfRules_Pack(&original, blob);
    MfRules_Unpack(blob, &restored);

    EXPECT_EQ(memcmp(&original, &restored, sizeof(original)), 0);
    EXPECT_EQ(restored.version, MF_RULES_VERSION);
    EXPECT_EQ((u32)restored.gamemodePreset, (u32)MF_GAMEMODE_MODERN);
    EXPECT_EQ(restored.randomizerSeed, 0xA5A5F00Du);
    EXPECT_EQ((u32)restored.shinyChance, 9u);
    EXPECT_EQ((u32)restored.monotype, 12u);
    EXPECT_EQ(restored.nuzlockeEncounterFlags[0], 0xAAu);
    EXPECT_EQ(restored.nuzlockeEncounterFlags[MF_NUZLOCKE_ENCOUNTER_FLAG_BYTES - 1], 0x55u);
}

TEST("MF: rules reset sets version and monotype off")
{
    struct ModernRules rules;

    memset(&rules, 0xA5, sizeof(rules));
    MfRules_ResetToEmpty(&rules);

    EXPECT_EQ(rules.version, MF_RULES_VERSION);
    EXPECT_EQ((u32)rules.monotype, 31u);
    EXPECT_EQ((u32)rules.rulesLocked, (u32)FALSE);
    EXPECT_EQ(rules.randomizerSeed, 0u);
    EXPECT_EQ((u32)rules.nuzlocke, (u32)FALSE);
    EXPECT_EQ(rules.nuzlockeEncounterFlags[0], 0u);
}

TEST("MF: SaveBlock3 embeds ModernRules after dexNavChain")
{
    // Layout lock: older saves keep reading dexNavChain at the same offset;
    // mfRules is appended so pre-S12 saves still load.
    EXPECT_EQ(offsetof(struct SaveBlock3, dexNavChain), 0u);
    EXPECT_EQ(offsetof(struct SaveBlock3, mfRules), 4u);
}
