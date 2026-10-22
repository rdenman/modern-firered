#include "global.h"
#include "event_data.h"
#include "mf_rules.h"
#include "mf_save.h"
#include "pokemon.h"
#include "test/test.h"
#include "constants/pokedex.h"
#include "constants/species.h"

static void ResetNationalDexAndRules(void)
{
    struct ModernRules *save = MfRules_GetSaveRules();

    DisableNationalPokedex();
    MfRules_ResetToEmpty(save);
    save->version = MF_RULES_VERSION;
}

TEST("MF: National Dex is not required for Classic vanilla")
{
    ResetNationalDexAndRules();
    EXPECT_EQ((u32)MfRules_RunNeedsNationalDex(), (u32)FALSE);
    MfTryEnableNationalDexForRules();
    EXPECT_EQ((u32)IsNationalPokedexEnabled(), (u32)FALSE);
}

TEST("MF: species randomizer and modern encounters need National Dex")
{
    struct ModernRules *save = MfRules_GetSaveRules();

    ResetNationalDexAndRules();
    save->randomizerEnabled = TRUE;
    save->randomItems = TRUE;
    EXPECT_EQ((u32)MfRules_RunNeedsNationalDex(), (u32)FALSE);

    save->randomWild = TRUE;
    EXPECT_EQ((u32)MfRules_RunNeedsNationalDex(), (u32)TRUE);

    ResetNationalDexAndRules();
    save->randomizerEnabled = TRUE;
    save->randomEvolution = TRUE;
    EXPECT_EQ((u32)MfRules_RunNeedsNationalDex(), (u32)TRUE);

    ResetNationalDexAndRules();
    save->alternateSpawns = 1;
    EXPECT_EQ((u32)MfRules_RunNeedsNationalDex(), (u32)TRUE);
}

TEST("MF: out-of-regional species hide until National Dex; load enables it")
{
    struct ModernRules *save = MfRules_GetSaveRules();

    ResetNationalDexAndRules();
    EXPECT_EQ(SpeciesToPokedexNum(SPECIES_CHIKORITA), 0xFFFFu);
    EXPECT(SpeciesToPokedexNum(SPECIES_TREECKO) != 0 && SpeciesToPokedexNum(SPECIES_TREECKO) != 0xFFFFu);

    save->randomizerEnabled = TRUE;
    save->randomWild = TRUE;
    MfSave_MigrateOnLoad();
    EXPECT_EQ((u32)IsNationalPokedexEnabled(), (u32)TRUE);
    EXPECT_EQ(SpeciesToPokedexNum(SPECIES_CHIKORITA), (u16)NATIONAL_DEX_CHIKORITA);

    DisableNationalPokedex();
}

TEST("MF: new-game commit on Modern enables National Dex")
{
    struct ModernRules *save = MfRules_GetSaveRules();

    ResetNationalDexAndRules();
    MfRules_DebugSetUnlockOverride(FALSE);
    MfRules_ApplyDevDefaults(save);
    MfRules_ApplyGamemodePreset(save, MF_GAMEMODE_MODERN);
    MfRules_CommitAndLock();
    EXPECT_EQ((u32)MfRules_RunNeedsNationalDex(), (u32)TRUE);
    EXPECT_EQ((u32)IsNationalPokedexEnabled(), (u32)TRUE);

    DisableNationalPokedex();
    save->rulesLocked = FALSE;
}
