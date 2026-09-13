#include "global.h"
#include "mf_rules_menu.h"
#include "mf_rules.h"
#include "bg.h"
#include "gpu_regs.h"
#include "international_string_util.h"
#include "main.h"
#include "malloc.h"
#include "menu.h"
#include "overworld.h"
#include "palette.h"
#include "scanline_effect.h"
#include "sound.h"
#include "sprite.h"
#include "task.h"
#include "text.h"
#include "text_window.h"
#include "window.h"
#include "constants/rgb.h"
#include "constants/songs.h"

// S18 — data-driven rules menu shell (FR option_menu fonts/palettes).
// S19 — new-game / mid-run entry points.
// S20 — Gamemode page (ME order; EXTRA LEGEND. dropped — no new maps).
// S21 — Features page (FR subset; Hoenn/Frontier/WT/RTC exclusions — ADR 0021).
// S22 — Nuzlocke page (Off/Easy/Normal/Hard; sub-options gated — ADR 0022).

#if MF_RULES_ENGINE

enum
{
    WIN_TOPBAR,
    WIN_OPTIONS,
    WIN_DESCRIPTION,
};

enum MfRulesMenuItemKind
{
    MF_RULES_MENU_ITEM_BOOL,
    MF_RULES_MENU_ITEM_VALUE,
    MF_RULES_MENU_ITEM_NEXT,
    MF_RULES_MENU_ITEM_EXIT,
};

enum MfRulesMenuItemFlags
{
    MF_RULES_MENU_FLAG_NONE = 0,
    MF_RULES_MENU_FLAG_REQUIRES_CUSTOM = 1 << 0,   // editable only when GAMEMODE=Custom
    MF_RULES_MENU_FLAG_REQUIRES_NUZLOCKE = 1 << 1, // editable only on Normal/Hardcore
};

struct MfRulesMenuChoice
{
    const u8 *label;
    const u8 *description;
};

struct MfRulesMenuItem
{
    const u8 *label;
    u8 kind;
    u8 ruleId; // MfRuleBool or MfRuleValue
    u8 choiceCount;
    u8 flags;
    const struct MfRulesMenuChoice *choices;
};

struct MfRulesMenuPage
{
    const u8 *title;
    const struct MfRulesMenuItem *items;
    u8 itemCount;
};

#define Y_DIFF 16
#define OPTIONS_ON_SCREEN 5
#define MF_RULES_MENU_MAX_ITEMS 24
#define CHOICE_LEFT_X 104
#define CHOICE_RIGHT_X 198

struct MfRulesMenuState
{
    u8 page;
    u8 menuCursor;
    u8 scrollOffset;
    u8 selections[MF_RULES_MENU_MAX_ITEMS];
};

static EWRAM_DATA struct MfRulesMenuState *sMenu = NULL;

static const struct WindowTemplate sWinTemplates[] =
{
    [WIN_TOPBAR] = {
        .bg = 1,
        .tilemapLeft = 0,
        .tilemapTop = 0,
        .width = 30,
        .height = 2,
        .paletteNum = 1,
        .baseBlock = 2
    },
    [WIN_OPTIONS] = {
        .bg = 0,
        .tilemapLeft = 2,
        .tilemapTop = 3,
        .width = 26,
        .height = 10,
        .paletteNum = 1,
        .baseBlock = 62
    },
    [WIN_DESCRIPTION] = {
        .bg = 1,
        .tilemapLeft = 2,
        .tilemapTop = 15,
        .width = 26,
        .height = 4,
        .paletteNum = 1,
        .baseBlock = 500
    },
    DUMMY_WIN_TEMPLATE
};

static const struct BgTemplate sBgTemplates[] =
{
    {
        .bg = 0,
        .charBaseIndex = 1,
        .mapBaseIndex = 30,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 1,
        .baseTile = 0
    },
    {
        .bg = 1,
        .charBaseIndex = 1,
        .mapBaseIndex = 31,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 0,
        .baseTile = 0
    },
};

static const u16 sBgPal[] = {RGB(17, 18, 31)};
static const u16 sTextPal[] = INCGFX_U16("graphics/interface/option_menu_text.pal", ".gbapal");

// --- Shared choice chrome ---

static const u8 sText_Off[] = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}OFF");
static const u8 sText_On[] = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}ON");
static const u8 sText_Classic[] = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}CLASSIC");
static const u8 sText_Modern[] = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}MODERN");
static const u8 sText_Custom[] = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}CUSTOM");
static const u8 sText_Orig[] = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}ORIG");
static const u8 sText_New[] = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}NEW");
static const u8 sText_Post[] = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}POST");
static const u8 sText_Original[] = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}ORIGINAL");
static const u8 sText_ModernLong[] = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}MODERN");
static const u8 sText_Gen6[] = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}GEN VI+");
static const u8 sText_Improved[] = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}IMPROVED");
static const u8 sText_Shiny8192[] = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}8192");
static const u8 sText_Shiny4096[] = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}4096");
static const u8 sText_Shiny2048[] = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}2048");
static const u8 sText_Shiny1024[] = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}1024");
static const u8 sText_Shiny512[] = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}512");
static const u8 sText_Easy[] = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}EASY");
static const u8 sText_Normal[] = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}NORMAL");
static const u8 sText_Hard[] = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}HARD");
static const u8 sText_Cemetery[] = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}CEMETERY");
static const u8 sText_Release[] = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}RELEASE");

// --- Gamemode descriptions (ME copy, FR-adapted where needed) ---

static const u8 sDesc_Gamemode_Classic[] = _("Vanilla-like preset.\nNote: All selections are permanent.");
static const u8 sDesc_Gamemode_Modern[] = _("Modernized preset.\nNote: All selections are permanent.");
static const u8 sDesc_Gamemode_Custom[] = _("Choose your own rules.\nNote: All selections are permanent.");
static const u8 sDesc_Spawns_Vanilla[] = _("Use Vanilla wild encounters.\nUnchanged from original FireRed.");
static const u8 sDesc_Spawns_Modern[] = _("Use Modern wild encounters.\nGen 1-3 remaps on existing routes.");
static const u8 sDesc_Spawns_Post[] = _("Vanilla tables, with more species\navailable after the League.");
static const u8 sDesc_TMs_Off[] = _("TMs are not reusable.\nLike in the original.");
static const u8 sDesc_TMs_On[] = _("TMs are reusable.\nRecommended for Modern FireRed.");
static const u8 sDesc_Poison_Off[] = _("Your Pokémon will faint if they are\nPoisoned in the overworld.");
static const u8 sDesc_Poison_On[] = _("Your Pokémon will survive the Poison\nstatus with 1HP.");
static const u8 sDesc_Sync_Old[] = _("Synchronize works as in Gen III.\n50% chance to copy nature.");
static const u8 sDesc_Sync_New[] = _("Synchronize works as in Gen VIII+.\n100% chance to copy nature.");
static const u8 sDesc_Mints_Off[] = _("Nature Mints stay scarce until\npostgame (or debug).");
static const u8 sDesc_Mints_On[] = _("Nature Mints can be stocked after a\nbadge threshold (shop wiring later).");
static const u8 sDesc_Sitrus_Off[] = _("Sitrus Berry restores 30HP.\nSame as Gen III.");
static const u8 sDesc_Sitrus_On[] = _("Sitrus Berry restores 25% of\ntotal HP. Same as Gen IV and up.");
static const u8 sDesc_Types_Off[] = _("Original {PKMN} typings. Doesn't include\n{PKMN} that got Fairy in Gen VI.");
static const u8 sDesc_Types_On[] = _("Pokémon have modified typings\nto make them more viable.");
static const u8 sDesc_Fairy_Off[] = _("Fairy Type isn't added to Pokémon\nthat got it in Gen VI.");
static const u8 sDesc_Fairy_On[] = _("Fairy Type is added / changed to\ncertain Pokémon, as in Gen VI.");
static const u8 sDesc_Stats_Off[] = _("Original Gen III Pokémon stats and\nabilities.");
static const u8 sDesc_Stats_On[] = _("Modified stats and abilities to make\ncertain Pokémon more viable.");
static const u8 sDesc_Sturdy_Off[] = _("Sturdy works as in Gen III. Only\nnegates OHKO moves (Guillotine, etc.).");
static const u8 sDesc_Sturdy_On[] = _("Sturdy works as in Gen V+.\nPokémon survive lethal hits with 1HP.");
static const u8 sDesc_Moves_Off[] = _("No new Moves, original Movepool for\nall Pokémon.");
static const u8 sDesc_Moves_On[] = _("Modern movepools for all {PKMN},\nplus updated Egg and TM moves.");
static const u8 sDesc_LegAbil_Off[] = _("Pressure stays as the main ability\nof some legendaries.");
static const u8 sDesc_LegAbil_On[] = _("Legendaries with the Pressure ability\nget a better one instead.");
static const u8 sDesc_Chart_Gen6[] = _("Type effectiveness from Gen VI!\nGhost / Dark do x1 to Steel.");
static const u8 sDesc_Chart_Improved[] = _("Rebalanced type effectiveness\nfor certain types.");
static const u8 sDesc_Next[] = _("Continue to later rule pages.\nB returns to the previous page.");
static const u8 sDesc_NextFeatures[] = _("Continue to Nuzlocke options.\nB returns to the previous page.");
static const u8 sDesc_NextNuzlocke[] = _("Continue to difficulty options.\nB returns to the previous page.");
static const u8 sDesc_Exit[] = _("Confirm these rules and continue.\nB returns to the previous page.");
static const u8 sDesc_LockedCustom[] = _("Select GAMEMODE Custom to edit\nthis option.");
static const u8 sDesc_LockedNuzlocke[] = _("Only usable with Nuzlocke!");

// --- Features descriptions (ME copy; excluded options documented in ADR 0021) ---

static const u8 sDesc_ShinyChance_8192[] = _("Very low chance of Shiny encounter.\nDefault chance from Generation III.");
static const u8 sDesc_ShinyChance_4096[] = _("Low chance of Shiny encounter.\nDefault chance from Generation VI+.");
static const u8 sDesc_ShinyChance_2048[] = _("Decent chance of Shiny encounter.");
static const u8 sDesc_ShinyChance_1024[] = _("High chance of Shiny encounter.");
static const u8 sDesc_ShinyChance_512[] = _("Very high chance of Shiny encounter.");
static const u8 sDesc_ItemDrop_Off[] = _("Wild Pokémon items will be only\nobtainable via capture or Thief.");
static const u8 sDesc_ItemDrop_On[] = _("Wild Pokémon will drop their hold\nitem after defeating them.");
static const u8 sDesc_ShinyColors_Original[] = _("Original shiny color palette for all\nPokémon. Default.");
static const u8 sDesc_ShinyColors_Modern[] = _("Some shiny Pokémon have brand new\ncolor palettes (when assets exist).");

// --- Nuzlocke descriptions (ME copy) ---

static const u8 sDesc_Nuzlocke_Off[] = _("Nuzlocke mode is disabled.");
static const u8 sDesc_Nuzlocke_Easy[] = _("Fainted {PKMN} can't be used anymore!\nNo more rules are enforced.");
static const u8 sDesc_Nuzlocke_Normal[] = _("One catch per route! Fainted Pokémon\ncan't be used anymore.");
static const u8 sDesc_Nuzlocke_Hard[] = _("Same rules as Normal but also\ndeletes Save on battle loss!");
static const u8 sDesc_Dupes_Off[] = _("The player always has to catch the\nfirst Pokémon per route.");
static const u8 sDesc_Dupes_On[] = _("Only not prior caught Pokémon count\nas first encounter. Recommended!");
static const u8 sDesc_ShinyClause_Off[] = _("The player can only catch a shiny\nPokémon if it's the first encounter.");
static const u8 sDesc_ShinyClause_On[] = _("The player can always catch shiny\nPokémon. Recommended!");
static const u8 sDesc_Nicknames_Off[] = _("Nicknames are optional.");
static const u8 sDesc_Nicknames_On[] = _("Forces the player to nickname every\nPokémon. Recommended!");
static const u8 sDesc_Fainting_Cemetery[] = _("Fainted Pokémon are sent to the PC\nafter battle and can't be retrieved.");
static const u8 sDesc_Fainting_Release[] = _("Fainted Pokémon are released after\nbattle!");

static const struct MfRulesMenuChoice sChoicesGamemode[] =
{
    { sText_Classic, sDesc_Gamemode_Classic },
    { sText_Modern,  sDesc_Gamemode_Modern  },
    { sText_Custom,  sDesc_Gamemode_Custom  },
};

static const struct MfRulesMenuChoice sChoicesSpawns[] =
{
    { sText_Orig, sDesc_Spawns_Vanilla },
    { sText_New,  sDesc_Spawns_Modern },
    { sText_Post, sDesc_Spawns_Post   },
};

static const struct MfRulesMenuChoice sChoicesOffOn[] =
{
    { sText_Off, sDesc_TMs_Off },
    { sText_On,  sDesc_TMs_On  },
};

static const struct MfRulesMenuChoice sChoicesPoison[] =
{
    { sText_Off, sDesc_Poison_Off },
    { sText_On,  sDesc_Poison_On  },
};

static const struct MfRulesMenuChoice sChoicesSync[] =
{
    { sText_Original,   sDesc_Sync_Old },
    { sText_ModernLong, sDesc_Sync_New },
};

static const struct MfRulesMenuChoice sChoicesMints[] =
{
    { sText_Off, sDesc_Mints_Off },
    { sText_On,  sDesc_Mints_On  },
};

static const struct MfRulesMenuChoice sChoicesSitrus[] =
{
    { sText_Original,   sDesc_Sitrus_Off },
    { sText_ModernLong, sDesc_Sitrus_On  },
};

static const struct MfRulesMenuChoice sChoicesTypes[] =
{
    { sText_Original,   sDesc_Types_Off },
    { sText_ModernLong, sDesc_Types_On  },
};

static const struct MfRulesMenuChoice sChoicesFairy[] =
{
    { sText_Original,   sDesc_Fairy_Off },
    { sText_ModernLong, sDesc_Fairy_On  },
};

static const struct MfRulesMenuChoice sChoicesStats[] =
{
    { sText_Original,   sDesc_Stats_Off },
    { sText_ModernLong, sDesc_Stats_On  },
};

static const struct MfRulesMenuChoice sChoicesSturdy[] =
{
    { sText_Original,   sDesc_Sturdy_Off },
    { sText_ModernLong, sDesc_Sturdy_On  },
};

static const struct MfRulesMenuChoice sChoicesMoves[] =
{
    { sText_Original,   sDesc_Moves_Off },
    { sText_ModernLong, sDesc_Moves_On  },
};

static const struct MfRulesMenuChoice sChoicesLegAbil[] =
{
    { sText_Off, sDesc_LegAbil_Off },
    { sText_On,  sDesc_LegAbil_On  },
};

static const struct MfRulesMenuChoice sChoicesTypeChart[] =
{
    { sText_Gen6,     sDesc_Chart_Gen6     },
    { sText_Improved, sDesc_Chart_Improved },
};

static const struct MfRulesMenuChoice sChoicesNext[] =
{
    { NULL, sDesc_Next },
};

static const struct MfRulesMenuChoice sChoicesNextFeatures[] =
{
    { NULL, sDesc_NextFeatures },
};

static const struct MfRulesMenuChoice sChoicesNextNuzlocke[] =
{
    { NULL, sDesc_NextNuzlocke },
};

static const struct MfRulesMenuChoice sChoicesExit[] =
{
    { NULL, sDesc_Exit },
};

static const struct MfRulesMenuChoice sChoicesShinyChance[] =
{
    { sText_Shiny8192, sDesc_ShinyChance_8192 },
    { sText_Shiny4096, sDesc_ShinyChance_4096 },
    { sText_Shiny2048, sDesc_ShinyChance_2048 },
    { sText_Shiny1024, sDesc_ShinyChance_1024 },
    { sText_Shiny512,  sDesc_ShinyChance_512  },
};

static const struct MfRulesMenuChoice sChoicesItemDrop[] =
{
    { sText_Off, sDesc_ItemDrop_Off },
    { sText_On,  sDesc_ItemDrop_On  },
};

// ME draws Off/On; descriptions are Original/Modern — prefer the descriptive labels.
static const struct MfRulesMenuChoice sChoicesShinyColors[] =
{
    { sText_Original,   sDesc_ShinyColors_Original },
    { sText_ModernLong, sDesc_ShinyColors_Modern   },
};

static const struct MfRulesMenuChoice sChoicesNuzlocke[] =
{
    { sText_Off,    sDesc_Nuzlocke_Off    },
    { sText_Easy,   sDesc_Nuzlocke_Easy   },
    { sText_Normal, sDesc_Nuzlocke_Normal },
    { sText_Hard,   sDesc_Nuzlocke_Hard   },
};

static const struct MfRulesMenuChoice sChoicesDupes[] =
{
    { sText_Off, sDesc_Dupes_Off },
    { sText_On,  sDesc_Dupes_On  },
};

static const struct MfRulesMenuChoice sChoicesShinyClause[] =
{
    { sText_Off, sDesc_ShinyClause_Off },
    { sText_On,  sDesc_ShinyClause_On  },
};

static const struct MfRulesMenuChoice sChoicesNicknames[] =
{
    { sText_Off, sDesc_Nicknames_Off },
    { sText_On,  sDesc_Nicknames_On  },
};

static const struct MfRulesMenuChoice sChoicesFainting[] =
{
    { sText_Cemetery, sDesc_Fainting_Cemetery },
    { sText_Release,  sDesc_Fainting_Release  },
};

// ME enum order (tx_rac_menu.c MENUITEM_MODE_*), minus EXTRA LEGEND.
static const struct MfRulesMenuItem sGamemodePageItems[] =
{
    { COMPOUND_STRING("GAMEMODE"),        MF_RULES_MENU_ITEM_VALUE, MF_RULE_VAL_GAMEMODE_PRESET,      3, MF_RULES_MENU_FLAG_NONE,            sChoicesGamemode  },
    { COMPOUND_STRING("ENCOUNTERS"),      MF_RULES_MENU_ITEM_VALUE, MF_RULE_VAL_ALTERNATE_SPAWNS,     3, MF_RULES_MENU_FLAG_REQUIRES_CUSTOM, sChoicesSpawns    },
    { COMPOUND_STRING("TYPE CHART"),      MF_RULES_MENU_ITEM_BOOL,  MF_RULE_BOOL_TYPE_EFFECTIVENESS, 2, MF_RULES_MENU_FLAG_REQUIRES_CUSTOM, sChoicesTypeChart },
    { COMPOUND_STRING("POKéMON STATS"),   MF_RULES_MENU_ITEM_BOOL,  MF_RULE_BOOL_MODERN_STATS,       2, MF_RULES_MENU_FLAG_REQUIRES_CUSTOM, sChoicesStats     },
    { COMPOUND_STRING("FAIRY TYPE"),      MF_RULES_MENU_ITEM_BOOL,  MF_RULE_BOOL_FAIRY_TYPES,        2, MF_RULES_MENU_FLAG_REQUIRES_CUSTOM, sChoicesFairy     },
    { COMPOUND_STRING("POKéMON TYPES"),   MF_RULES_MENU_ITEM_BOOL,  MF_RULE_BOOL_MODERN_TYPES,       2, MF_RULES_MENU_FLAG_REQUIRES_CUSTOM, sChoicesTypes     },
    { COMPOUND_STRING("{PKMN} MOVEPOOL"), MF_RULES_MENU_ITEM_BOOL,  MF_RULE_BOOL_MODERN_MOVES,       2, MF_RULES_MENU_FLAG_REQUIRES_CUSTOM, sChoicesMoves     },
    { COMPOUND_STRING("SYNCHRONIZE"),     MF_RULES_MENU_ITEM_BOOL,  MF_RULE_BOOL_SYNCHRONIZE,        2, MF_RULES_MENU_FLAG_REQUIRES_CUSTOM, sChoicesSync      },
    { COMPOUND_STRING("STURDY"),          MF_RULES_MENU_ITEM_BOOL,  MF_RULE_BOOL_STURDY,             2, MF_RULES_MENU_FLAG_REQUIRES_CUSTOM, sChoicesSturdy    },
    { COMPOUND_STRING("SITRUS BERRY"),    MF_RULES_MENU_ITEM_BOOL,  MF_RULE_BOOL_MODERN_SITRUS,      2, MF_RULES_MENU_FLAG_REQUIRES_CUSTOM, sChoicesSitrus    },
    { COMPOUND_STRING("LEGEN. ABILITIES"),MF_RULES_MENU_ITEM_BOOL,  MF_RULE_BOOL_LEGENDARY_ABILITIES,2, MF_RULES_MENU_FLAG_REQUIRES_CUSTOM, sChoicesLegAbil   },
    { COMPOUND_STRING("NATURE MINTS"),    MF_RULES_MENU_ITEM_BOOL,  MF_RULE_BOOL_MINTS,              2, MF_RULES_MENU_FLAG_REQUIRES_CUSTOM, sChoicesMints     },
    { COMPOUND_STRING("REUSABLE TMS"),    MF_RULES_MENU_ITEM_BOOL,  MF_RULE_BOOL_INFINITE_TMS,       2, MF_RULES_MENU_FLAG_REQUIRES_CUSTOM, sChoicesOffOn     },
    { COMPOUND_STRING("SURVIVE POISON"),  MF_RULES_MENU_ITEM_BOOL,  MF_RULE_BOOL_SURVIVE_POISON,     2, MF_RULES_MENU_FLAG_REQUIRES_CUSTOM, sChoicesPoison    },
    { COMPOUND_STRING("NEXT"),            MF_RULES_MENU_ITEM_NEXT,  0,                               1, MF_RULES_MENU_FLAG_NONE,            sChoicesNext      },
};

// ME MENUITEM_FEATURES_* order minus CLOCK / WT / FEEBAS / FRONTIER (ADR 0021).
static const struct MfRulesMenuItem sFeaturesPageItems[] =
{
    { COMPOUND_STRING("SHINY CHANCE"), MF_RULES_MENU_ITEM_VALUE, MF_RULE_VAL_SHINY_CHANCE,     5, MF_RULES_MENU_FLAG_NONE, sChoicesShinyChance },
    { COMPOUND_STRING("SHINY COLORS"), MF_RULES_MENU_ITEM_BOOL,  MF_RULE_BOOL_SHINY_COLORS,   2, MF_RULES_MENU_FLAG_NONE, sChoicesShinyColors },
    { COMPOUND_STRING("ITEM DROP"),    MF_RULES_MENU_ITEM_BOOL,  MF_RULE_BOOL_WILD_ITEM_DROPS, 2, MF_RULES_MENU_FLAG_NONE, sChoicesItemDrop    },
    { COMPOUND_STRING("NEXT"),         MF_RULES_MENU_ITEM_NEXT,  0,                            1, MF_RULES_MENU_FLAG_NONE, sChoicesNextFeatures },
};

// ME MENUITEM_NUZLOCKE_* (ADR 0022). Easy = mini mode; sub-options need Normal/Hard.
static const struct MfRulesMenuItem sNuzlockePageItems[] =
{
    { COMPOUND_STRING("NUZLOCKE"),     MF_RULES_MENU_ITEM_VALUE, MF_RULE_VAL_NUZLOCKE_MODE,             4, MF_RULES_MENU_FLAG_NONE,             sChoicesNuzlocke    },
    { COMPOUND_STRING("DUPES CLAUSE"), MF_RULES_MENU_ITEM_BOOL,  MF_RULE_BOOL_NUZLOCKE_SPECIES_CLAUSE,  2, MF_RULES_MENU_FLAG_REQUIRES_NUZLOCKE, sChoicesDupes       },
    { COMPOUND_STRING("SHINY CLAUSE"), MF_RULES_MENU_ITEM_BOOL,  MF_RULE_BOOL_NUZLOCKE_SHINY_CLAUSE,    2, MF_RULES_MENU_FLAG_REQUIRES_NUZLOCKE, sChoicesShinyClause },
    { COMPOUND_STRING("NICKNAMES"),    MF_RULES_MENU_ITEM_BOOL,  MF_RULE_BOOL_NUZLOCKE_NICKNAMING,      2, MF_RULES_MENU_FLAG_REQUIRES_NUZLOCKE, sChoicesNicknames   },
    { COMPOUND_STRING("FAINTING"),     MF_RULES_MENU_ITEM_BOOL,  MF_RULE_BOOL_NUZLOCKE_DELETION,        2, MF_RULES_MENU_FLAG_REQUIRES_NUZLOCKE, sChoicesFainting    },
    { COMPOUND_STRING("NEXT"),         MF_RULES_MENU_ITEM_NEXT,  0,                                     1, MF_RULES_MENU_FLAG_NONE,             sChoicesNextNuzlocke },
};

// Stub until S23–S25 land; EXIT still commits the new-game flow (S26 adds SAVE).
static const struct MfRulesMenuItem sStubContinueItems[] =
{
    { COMPOUND_STRING("EXIT"), MF_RULES_MENU_ITEM_EXIT, 0, 1, MF_RULES_MENU_FLAG_NONE, sChoicesExit },
};

static const struct MfRulesMenuPage sPages[] =
{
    { COMPOUND_STRING("GAMEMODE"),  sGamemodePageItems,  ARRAY_COUNT(sGamemodePageItems)  },
    { COMPOUND_STRING("FEATURES"),  sFeaturesPageItems,  ARRAY_COUNT(sFeaturesPageItems)  },
    { COMPOUND_STRING("NUZLOCKE"),  sNuzlockePageItems,  ARRAY_COUNT(sNuzlockePageItems)  },
    { COMPOUND_STRING("CONTINUE"),  sStubContinueItems,  ARRAY_COUNT(sStubContinueItems)  },
};

static void MainCB2(void);
static void VBlankCB(void);
static void Task_FadeIn(u8 taskId);
static void Task_ProcessInput(u8 taskId);
static void Task_FadeOut(u8 taskId);
static void HighlightItem(void);
static void DrawTopBar(void);
static void DrawDescription(void);
static void DrawAllOptions(void);
static void DrawBgWindowFrames(void);
static void LoadPageSelections(void);
static void WriteSelection(u8 itemIndex);
static bool8 EnsureWritable(void);
static bool8 ItemIsEditable(const struct MfRulesMenuItem *item);
static const struct MfRulesMenuPage *CurrentPage(void);
static const struct MfRulesMenuItem *CurrentItem(void);
static u8 CurrentValue(void);

static void MainCB2(void)
{
    RunTasks();
    AnimateSprites();
    BuildOamBuffer();
    UpdatePaletteFade();
}

static void VBlankCB(void)
{
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
}

static const struct MfRulesMenuPage *CurrentPage(void)
{
    return &sPages[sMenu->page];
}

static const struct MfRulesMenuItem *CurrentItem(void)
{
    return &CurrentPage()->items[sMenu->menuCursor];
}

static u8 CurrentValue(void)
{
    return sMenu->selections[sMenu->menuCursor];
}

static bool8 ItemIsEditable(const struct MfRulesMenuItem *item)
{
    if (item->kind == MF_RULES_MENU_ITEM_NEXT || item->kind == MF_RULES_MENU_ITEM_EXIT)
        return TRUE;
    if (item->flags & MF_RULES_MENU_FLAG_REQUIRES_CUSTOM)
        return MfRules_GetValue(MF_RULE_VAL_GAMEMODE_PRESET) == MF_GAMEMODE_CUSTOM;
    if (item->flags & MF_RULES_MENU_FLAG_REQUIRES_NUZLOCKE)
        return MfRules_NuzlockeSubOptionsActive();
    return TRUE;
}

static bool8 EnsureWritable(void)
{
    if (MfRules_CanEdit(MF_RULE_EDIT_CORE))
        return TRUE;
    if (MfRules_DebugHasUnlockOverride())
        return TRUE;
    return MfRules_DebugSetUnlockOverride(TRUE);
}

static void LoadPageSelections(void)
{
    const struct MfRulesMenuPage *page = CurrentPage();
    u8 i;

    for (i = 0; i < page->itemCount; i++)
    {
        const struct MfRulesMenuItem *item = &page->items[i];

        switch (item->kind)
        {
        case MF_RULES_MENU_ITEM_BOOL:
            sMenu->selections[i] = MfRules_GetBool(item->ruleId) ? 1 : 0;
            break;
        case MF_RULES_MENU_ITEM_VALUE:
            sMenu->selections[i] = MfRules_GetValue(item->ruleId);
            if (sMenu->selections[i] >= item->choiceCount)
                sMenu->selections[i] = 0;
            break;
        default:
            sMenu->selections[i] = 0;
            break;
        }
    }
}

static void WriteSelection(u8 itemIndex)
{
    const struct MfRulesMenuItem *item = &CurrentPage()->items[itemIndex];
    u8 value = sMenu->selections[itemIndex];

    if (item->kind != MF_RULES_MENU_ITEM_BOOL && item->kind != MF_RULES_MENU_ITEM_VALUE)
        return;
    if (!ItemIsEditable(item))
    {
        PlaySE(SE_FAILURE);
        return;
    }
    if (!EnsureWritable())
    {
        PlaySE(SE_FAILURE);
        return;
    }

    if (item->kind == MF_RULES_MENU_ITEM_BOOL)
    {
        if (!MfRules_TrySetBool(item->ruleId, value != 0))
            PlaySE(SE_FAILURE);
    }
    else if (!MfRules_TrySetValue(item->ruleId, value))
    {
        PlaySE(SE_FAILURE);
    }
    else if (item->ruleId == MF_RULE_VAL_GAMEMODE_PRESET
          || item->ruleId == MF_RULE_VAL_NUZLOCKE_MODE)
    {
        // Classic/Modern bulk-set; Nuzlocke mode seeds/clears clauses (ADR 0020/0022).
        LoadPageSelections();
    }
}

static void DrawChoiceText(const u8 *text, u8 x, u8 y, bool8 selected, bool8 active)
{
    u8 dst[24];
    u16 i;

    for (i = 0; *text != EOS && i < ARRAY_COUNT(dst) - 1; i++)
        dst[i] = *(text++);
    dst[i] = EOS;

    // Embedded COLOR/SHADOW codes: bytes 2 and 5 after {COLOR}/{SHADOW}.
    if (i > 5)
    {
        if (!active)
        {
            dst[2] = TEXT_COLOR_DARK_GRAY;
            dst[5] = TEXT_COLOR_LIGHT_GRAY;
        }
        else if (selected)
        {
            dst[2] = TEXT_COLOR_RED;
            dst[5] = TEXT_COLOR_LIGHT_RED;
        }
    }

    AddTextPrinterParameterized(WIN_OPTIONS, FONT_NORMAL, dst, x, y + 1, TEXT_SKIP_DRAW, NULL);
}

static void DrawItemChoices(u8 itemIndex, u8 y, bool8 active)
{
    const struct MfRulesMenuItem *item = &CurrentPage()->items[itemIndex];
    u8 value = sMenu->selections[itemIndex];
    u8 styles[2] = {0};

    if (item->kind == MF_RULES_MENU_ITEM_NEXT || item->kind == MF_RULES_MENU_ITEM_EXIT)
        return;

    if (item->choiceCount == 2)
    {
        styles[value] = 1;
        DrawChoiceText(item->choices[0].label, CHOICE_LEFT_X, y, styles[0], active);
        DrawChoiceText(item->choices[1].label,
                       GetStringRightAlignXOffset(FONT_NORMAL, item->choices[1].label, CHOICE_RIGHT_X),
                       y, styles[1], active);
    }
    else if (item->choiceCount > 0 && value < item->choiceCount)
    {
        // 3+ choices: show active value only (overflow-safe vs ME multi-slot chrome).
        DrawChoiceText(item->choices[value].label,
                       GetStringRightAlignXOffset(FONT_NORMAL, item->choices[value].label, CHOICE_RIGHT_X),
                       y, TRUE, active);
    }
}

static void DrawItemRow(u8 itemIndex, u8 y)
{
    const struct MfRulesMenuItem *item = &CurrentPage()->items[itemIndex];
    bool8 active = ItemIsEditable(item);
    u8 color[3];

    if (active)
    {
        AddTextPrinterParameterized(WIN_OPTIONS, FONT_NORMAL, item->label, 8, y + 1, TEXT_SKIP_DRAW, NULL);
    }
    else
    {
        color[0] = TEXT_COLOR_TRANSPARENT;
        color[1] = TEXT_COLOR_DARK_GRAY;
        color[2] = TEXT_COLOR_LIGHT_GRAY;
        AddTextPrinterParameterized4(WIN_OPTIONS, FONT_NORMAL, 8, y + 1, 0, 0, color, TEXT_SKIP_DRAW, item->label);
    }
    DrawItemChoices(itemIndex, y, active);
}

static void DrawAllOptions(void)
{
    const struct MfRulesMenuPage *page = CurrentPage();
    u8 i;
    u8 rows = page->itemCount - sMenu->scrollOffset;

    if (rows > OPTIONS_ON_SCREEN)
        rows = OPTIONS_ON_SCREEN;

    FillWindowPixelBuffer(WIN_OPTIONS, PIXEL_FILL(1));
    for (i = 0; i < rows; i++)
        DrawItemRow(sMenu->scrollOffset + i, i * Y_DIFF);
    CopyWindowToVram(WIN_OPTIONS, COPYWIN_FULL);
}

static void DrawTopBar(void)
{
    FillWindowPixelBuffer(WIN_TOPBAR, PIXEL_FILL(1));
    AddTextPrinterParameterized(WIN_TOPBAR, FONT_NORMAL, CurrentPage()->title, 8, 1, TEXT_SKIP_DRAW, NULL);
    CopyWindowToVram(WIN_TOPBAR, COPYWIN_FULL);
}

static void DrawDescription(void)
{
    const struct MfRulesMenuItem *item = CurrentItem();
    u8 value = CurrentValue();
    const u8 *desc = sDesc_LockedCustom;
    u8 color[3];

    if (!ItemIsEditable(item) && (item->flags & MF_RULES_MENU_FLAG_REQUIRES_NUZLOCKE))
    {
        // ME shows "Only usable with Nuzlocke!" when sub-options are gated.
        desc = sDesc_LockedNuzlocke;
    }
    else if (item->choices != NULL)
    {
        if (item->kind == MF_RULES_MENU_ITEM_NEXT || item->kind == MF_RULES_MENU_ITEM_EXIT)
            desc = item->choices[0].description;
        else if (value < item->choiceCount)
            desc = item->choices[value].description;
        else if (!ItemIsEditable(item))
            desc = sDesc_LockedCustom;
    }

    color[0] = TEXT_COLOR_TRANSPARENT;
    color[1] = TEXT_COLOR_DARK_GRAY;
    color[2] = TEXT_COLOR_LIGHT_GRAY;

    FillWindowPixelBuffer(WIN_DESCRIPTION, PIXEL_FILL(1));
    AddTextPrinterParameterized4(WIN_DESCRIPTION, FONT_NORMAL, 8, 1, 0, 0, color, TEXT_SKIP_DRAW, desc);
    CopyWindowToVram(WIN_DESCRIPTION, COPYWIN_FULL);
}

static void HighlightItem(void)
{
    u8 row = sMenu->menuCursor - sMenu->scrollOffset;

    SetGpuReg(REG_OFFSET_WIN0H, WIN_RANGE(16, DISPLAY_WIDTH - 16));
    SetGpuReg(REG_OFFSET_WIN0V, WIN_RANGE(row * Y_DIFF + 24, row * Y_DIFF + 40));
}

#define TILE_TOP_CORNER_L 0x1A2
#define TILE_TOP_EDGE     0x1A3
#define TILE_TOP_CORNER_R 0x1A4
#define TILE_LEFT_EDGE    0x1A5
#define TILE_RIGHT_EDGE   0x1A7
#define TILE_BOT_CORNER_L 0x1A8
#define TILE_BOT_EDGE     0x1A9
#define TILE_BOT_CORNER_R 0x1AA

static void DrawBgWindowFrames(void)
{
    // Options list (matches ME rac menu placement)
    FillBgTilemapBufferRect(1, TILE_TOP_CORNER_L,  1,  2,  1,  1,  7);
    FillBgTilemapBufferRect(1, TILE_TOP_EDGE,      2,  2, 26,  1,  7);
    FillBgTilemapBufferRect(1, TILE_TOP_CORNER_R, 28,  2,  1,  1,  7);
    FillBgTilemapBufferRect(1, TILE_LEFT_EDGE,     1,  3,  1, 10,  7);
    FillBgTilemapBufferRect(1, TILE_RIGHT_EDGE,   28,  3,  1, 10,  7);
    FillBgTilemapBufferRect(1, TILE_BOT_CORNER_L,  1, 13,  1,  1,  7);
    FillBgTilemapBufferRect(1, TILE_BOT_EDGE,      2, 13, 26,  1,  7);
    FillBgTilemapBufferRect(1, TILE_BOT_CORNER_R, 28, 13,  1,  1,  7);

    // Description pane
    FillBgTilemapBufferRect(1, TILE_TOP_CORNER_L,  1, 14,  1,  1,  7);
    FillBgTilemapBufferRect(1, TILE_TOP_EDGE,      2, 14, 26,  1,  7);
    FillBgTilemapBufferRect(1, TILE_TOP_CORNER_R, 28, 14,  1,  1,  7);
    FillBgTilemapBufferRect(1, TILE_LEFT_EDGE,     1, 15,  1,  4,  7);
    FillBgTilemapBufferRect(1, TILE_RIGHT_EDGE,   28, 15,  1,  4,  7);
    FillBgTilemapBufferRect(1, TILE_BOT_CORNER_L,  1, 19,  1,  1,  7);
    FillBgTilemapBufferRect(1, TILE_BOT_EDGE,      2, 19, 26,  1,  7);
    FillBgTilemapBufferRect(1, TILE_BOT_CORNER_R, 28, 19,  1,  1,  7);

    CopyBgTilemapBufferToVram(1);
}

static void BeginExit(u8 taskId)
{
    BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
    gTasks[taskId].func = Task_FadeOut;
}

static void GoToPage(u8 page)
{
    if (page >= ARRAY_COUNT(sPages))
        return;

    sMenu->page = page;
    sMenu->menuCursor = 0;
    sMenu->scrollOffset = 0;
    LoadPageSelections();
    DrawTopBar();
    DrawAllOptions();
    DrawDescription();
    HighlightItem();
}

static void GoToNextPage(void)
{
    if (sMenu->page + 1 >= ARRAY_COUNT(sPages))
        return;

    GoToPage(sMenu->page + 1);
}

static void GoToPrevPage(void)
{
    if (sMenu->page == 0)
        return;

    GoToPage(sMenu->page - 1);
}

static void CycleValue(s8 delta)
{
    const struct MfRulesMenuItem *item = CurrentItem();
    u8 count;
    s16 next;

    if (item->kind != MF_RULES_MENU_ITEM_BOOL && item->kind != MF_RULES_MENU_ITEM_VALUE)
        return;
    if (!ItemIsEditable(item))
    {
        PlaySE(SE_FAILURE);
        return;
    }

    count = item->choiceCount;
    if (count == 0)
        return;

    next = (s16)sMenu->selections[sMenu->menuCursor] + delta;
    if (next < 0)
        next = count - 1;
    else if (next >= count)
        next = 0;

    sMenu->selections[sMenu->menuCursor] = (u8)next;
    WriteSelection(sMenu->menuCursor);
    DrawAllOptions();
    DrawDescription();
    PlaySE(SE_SELECT);
}

static void MoveCursor(s8 delta)
{
    const struct MfRulesMenuPage *page = CurrentPage();
    s16 next = (s16)sMenu->menuCursor + delta;

    if (next < 0)
        next = page->itemCount - 1;
    else if (next >= page->itemCount)
        next = 0;

    sMenu->menuCursor = (u8)next;

    if (sMenu->menuCursor < sMenu->scrollOffset)
        sMenu->scrollOffset = sMenu->menuCursor;
    else if (sMenu->menuCursor >= sMenu->scrollOffset + OPTIONS_ON_SCREEN)
        sMenu->scrollOffset = sMenu->menuCursor - OPTIONS_ON_SCREEN + 1;

    DrawAllOptions();
    DrawDescription();
    HighlightItem();
    PlaySE(SE_SELECT);
}

static void Task_FadeIn(u8 taskId)
{
    if (!gPaletteFade.active)
        gTasks[taskId].func = Task_ProcessInput;
}

static void Task_ProcessInput(u8 taskId)
{
    const struct MfRulesMenuItem *item = CurrentItem();

    if (JOY_NEW(A_BUTTON))
    {
        if (item->kind == MF_RULES_MENU_ITEM_NEXT)
        {
            PlaySE(SE_SELECT);
            GoToNextPage();
        }
        else if (item->kind == MF_RULES_MENU_ITEM_EXIT)
        {
            // Only the final confirm/SAVE row leaves the menu (not B).
            PlaySE(SE_SELECT);
            BeginExit(taskId);
        }
        else
        {
            CycleValue(1);
        }
    }
    else if (JOY_NEW(B_BUTTON))
    {
        // Previous page only — never discard the new-game rules flow.
        if (sMenu->page == 0)
        {
            PlaySE(SE_FAILURE);
        }
        else
        {
            PlaySE(SE_SELECT);
            GoToPrevPage();
        }
    }
    else if (JOY_NEW(DPAD_UP))
    {
        MoveCursor(-1);
    }
    else if (JOY_NEW(DPAD_DOWN))
    {
        MoveCursor(1);
    }
    else if (JOY_NEW(DPAD_LEFT))
    {
        CycleValue(-1);
    }
    else if (JOY_NEW(DPAD_RIGHT))
    {
        CycleValue(1);
    }
}

static void Task_FadeOut(u8 taskId)
{
    if (!gPaletteFade.active)
    {
        DestroyTask(taskId);
        FreeAllWindowBuffers();
        FREE_AND_SET_NULL(sMenu);
        SetMainCallback2(gMain.savedCallback);
    }
}

void CB2_InitMfRulesMenu(void)
{
    switch (gMain.state)
    {
    default:
    case 0:
        SetVBlankCallback(NULL);
        gMain.state++;
        break;
    case 1:
        DmaClearLarge16(3, (void *)(VRAM), VRAM_SIZE, 0x1000);
        DmaClear32(3, OAM, OAM_SIZE);
        DmaClear16(3, PLTT, PLTT_SIZE);
        SetGpuReg(REG_OFFSET_DISPCNT, 0);
        ResetBgsAndClearDma3BusyFlags(0);
        InitBgsFromTemplates(0, sBgTemplates, ARRAY_COUNT(sBgTemplates));
        ChangeBgX(0, 0, BG_COORD_SET);
        ChangeBgY(0, 0, BG_COORD_SET);
        ChangeBgX(1, 0, BG_COORD_SET);
        ChangeBgY(1, 0, BG_COORD_SET);
        InitWindows(sWinTemplates);
        DeactivateAllTextPrinters();
        SetGpuReg(REG_OFFSET_WIN0H, 0);
        SetGpuReg(REG_OFFSET_WIN0V, 0);
        SetGpuReg(REG_OFFSET_WININ, WININ_WIN0_BG0);
        SetGpuReg(REG_OFFSET_WINOUT, WINOUT_WIN01_BG0 | WINOUT_WIN01_BG1 | WINOUT_WIN01_CLR);
        SetGpuReg(REG_OFFSET_BLDCNT, BLDCNT_TGT1_BG0 | BLDCNT_EFFECT_DARKEN);
        SetGpuReg(REG_OFFSET_BLDALPHA, 0);
        SetGpuReg(REG_OFFSET_BLDY, 4);
        SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_WIN0_ON | DISPCNT_OBJ_ON | DISPCNT_OBJ_1D_MAP);
        ShowBg(0);
        ShowBg(1);
        gMain.state++;
        break;
    case 2:
        ResetPaletteFade();
        ScanlineEffect_Stop();
        ResetTasks();
        ResetSpriteData();
        gMain.state++;
        break;
    case 3:
        LoadBgTiles(1, GetWindowFrameTilesPal(gSaveBlock2Ptr->optionsWindowFrameType)->tiles, 0x120, 0x1A2);
        gMain.state++;
        break;
    case 4:
        LoadPalette(sBgPal, BG_PLTT_ID(0), sizeof(sBgPal));
        LoadPalette(GetWindowFrameTilesPal(gSaveBlock2Ptr->optionsWindowFrameType)->pal, BG_PLTT_ID(7), PLTT_SIZE_4BPP);
        gMain.state++;
        break;
    case 5:
        LoadPalette(sTextPal, BG_PLTT_ID(1), sizeof(sTextPal));
        gMain.state++;
        break;
    case 6:
        sMenu = AllocZeroed(sizeof(*sMenu));
        if (sMenu == NULL)
        {
            SetMainCallback2(gMain.savedCallback);
            return;
        }
        sMenu->page = 0;
        sMenu->menuCursor = 0;
        sMenu->scrollOffset = 0;
        LoadPageSelections();
        PutWindowTilemap(WIN_TOPBAR);
        PutWindowTilemap(WIN_OPTIONS);
        PutWindowTilemap(WIN_DESCRIPTION);
        DrawTopBar();
        DrawAllOptions();
        DrawDescription();
        DrawBgWindowFrames();
        HighlightItem();
        gMain.state++;
        break;
    case 7:
        CreateTask(Task_FadeIn, 0);
        BeginNormalPaletteFade(PALETTES_ALL, 0, 16, 0, RGB_BLACK);
        SetVBlankCallback(VBlankCB);
        SetMainCallback2(MainCB2);
        break;
    }
}

void CB2_MfRules_BeginNewGame(void)
{
    // NewGameInitData clears SaveBlock3 then seeds unlocked presets (S14).
    // Menu edits that blob; exit returns to CB2_ContinueNewGame → overworld.
    CB2_PrepareNewGameData();
    gMain.savedCallback = CB2_ContinueNewGame;
    gMain.state = 0;
    SetMainCallback2(CB2_InitMfRulesMenu);
}

void Task_MfRulesMenu_NoNewGame(u8 taskId)
{
    gMain.savedCallback = CB2_ReturnToField;
    gMain.state = 0;
    SetMainCallback2(CB2_InitMfRulesMenu);
    DestroyTask(taskId);
}

#else // !MF_RULES_ENGINE

void CB2_InitMfRulesMenu(void)
{
    SetMainCallback2(gMain.savedCallback);
}

void CB2_MfRules_BeginNewGame(void)
{
    SetMainCallback2(CB2_NewGame);
}

void Task_MfRulesMenu_NoNewGame(u8 taskId)
{
    SetMainCallback2(CB2_ReturnToField);
    DestroyTask(taskId);
}

#endif // MF_RULES_ENGINE
