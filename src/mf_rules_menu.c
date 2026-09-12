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

// S18 — data-driven rules menu shell. Layout mirrors ME's rac menu
// (top bar + scrolling options + description) but uses FR option_menu
// fonts/palettes/window frames. Pages are tables; S20–S25 replace the demo.

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

// --- Demo strings (throwaway; S20+ replace) ---

static const u8 sText_Off[] = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}OFF");
static const u8 sText_On[] = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}ON");
static const u8 sText_Original[] = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}ORIG");
static const u8 sText_Alt[] = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}ALT");
static const u8 sText_New[] = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}NEW");
static const u8 sText_Low[] = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}LOW");
static const u8 sText_Mid[] = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}MID");
static const u8 sText_High[] = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}HIGH");
static const u8 sText_Max[] = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}MAX");
static const u8 sText_Ultra[] = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}ULTRA");

static const u8 sDesc_Off[] = _("Disabled for this demo option.");
static const u8 sDesc_On[] = _("Enabled for this demo option.");
static const u8 sDesc_Spawns_Orig[] = _("Original encounter tables.");
static const u8 sDesc_Spawns_Alt[] = _("Alternate encounter tables.");
static const u8 sDesc_Spawns_New[] = _("Post-game style encounter tables.");
static const u8 sDesc_Shiny_Low[] = _("Shiny chance: low (demo).");
static const u8 sDesc_Shiny_Mid[] = _("Shiny chance: mid (demo).");
static const u8 sDesc_Shiny_High[] = _("Shiny chance: high (demo).");
static const u8 sDesc_Shiny_Max[] = _("Shiny chance: max (demo).");
static const u8 sDesc_Shiny_Ultra[] = _("Shiny chance: ultra (demo).");
static const u8 sDesc_Next[] = _("Continue to the next demo page.");
static const u8 sDesc_Exit[] = _("Leave the rules menu demo.");

static const struct MfRulesMenuChoice sChoicesOffOn[] =
{
    { sText_Off, sDesc_Off },
    { sText_On,  sDesc_On  },
};

static const struct MfRulesMenuChoice sChoicesSpawns[] =
{
    { sText_Original, sDesc_Spawns_Orig },
    { sText_Alt,      sDesc_Spawns_Alt  },
    { sText_New,      sDesc_Spawns_New  },
};

static const struct MfRulesMenuChoice sChoicesShiny[] =
{
    { sText_Low,   sDesc_Shiny_Low   },
    { sText_Mid,   sDesc_Shiny_Mid   },
    { sText_High,  sDesc_Shiny_High  },
    { sText_Max,   sDesc_Shiny_Max   },
    { sText_Ultra, sDesc_Shiny_Ultra },
};

static const struct MfRulesMenuChoice sChoicesNext[] =
{
    { NULL, sDesc_Next },
};

static const struct MfRulesMenuChoice sChoicesExit[] =
{
    { NULL, sDesc_Exit },
};

static const struct MfRulesMenuItem sDemoPage0Items[] =
{
    { COMPOUND_STRING("INFINITE TMS"),   MF_RULES_MENU_ITEM_BOOL,  MF_RULE_BOOL_INFINITE_TMS,   2, sChoicesOffOn },
    { COMPOUND_STRING("SURVIVE POISON"), MF_RULES_MENU_ITEM_BOOL,  MF_RULE_BOOL_SURVIVE_POISON, 2, sChoicesOffOn },
    { COMPOUND_STRING("SYNCHRONIZE"),    MF_RULES_MENU_ITEM_BOOL,  MF_RULE_BOOL_SYNCHRONIZE,    2, sChoicesOffOn },
    { COMPOUND_STRING("NATURE MINTS"),   MF_RULES_MENU_ITEM_BOOL,  MF_RULE_BOOL_MINTS,          2, sChoicesOffOn },
    { COMPOUND_STRING("FAIRY TYPE"),     MF_RULES_MENU_ITEM_BOOL,  MF_RULE_BOOL_FAIRY_TYPES,    2, sChoicesOffOn },
    { COMPOUND_STRING("ENCOUNTERS"),     MF_RULES_MENU_ITEM_VALUE, MF_RULE_VAL_ALTERNATE_SPAWNS, 3, sChoicesSpawns },
    { COMPOUND_STRING("SHINY CHANCE"),   MF_RULES_MENU_ITEM_VALUE, MF_RULE_VAL_SHINY_CHANCE,    5, sChoicesShiny },
    { COMPOUND_STRING("NEXT"),           MF_RULES_MENU_ITEM_NEXT,  0,                           1, sChoicesNext },
};

static const struct MfRulesMenuItem sDemoPage1Items[] =
{
    { COMPOUND_STRING("STURDY"), MF_RULES_MENU_ITEM_BOOL, MF_RULE_BOOL_STURDY, 2, sChoicesOffOn },
    { COMPOUND_STRING("EXIT"),   MF_RULES_MENU_ITEM_EXIT, 0,                   1, sChoicesExit },
};

static const struct MfRulesMenuPage sDemoPages[] =
{
    { COMPOUND_STRING("RULES DEMO 1/2"), sDemoPage0Items, ARRAY_COUNT(sDemoPage0Items) },
    { COMPOUND_STRING("RULES DEMO 2/2"), sDemoPage1Items, ARRAY_COUNT(sDemoPage1Items) },
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
    return &sDemoPages[sMenu->page];
}

static const struct MfRulesMenuItem *CurrentItem(void)
{
    return &CurrentPage()->items[sMenu->menuCursor];
}

static u8 CurrentValue(void)
{
    return sMenu->selections[sMenu->menuCursor];
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
}

static void DrawChoiceText(const u8 *text, u8 x, u8 y, bool8 selected)
{
    u8 dst[24];
    u16 i;

    for (i = 0; *text != EOS && i < ARRAY_COUNT(dst) - 1; i++)
        dst[i] = *(text++);
    dst[i] = EOS;

    // FR option_menu: selected choice turns red (bytes after COLOR/SHADOW codes).
    if (selected && i > 5)
    {
        dst[2] = TEXT_COLOR_RED;
        dst[5] = TEXT_COLOR_LIGHT_RED;
    }

    AddTextPrinterParameterized(WIN_OPTIONS, FONT_NORMAL, dst, x, y + 1, TEXT_SKIP_DRAW, NULL);
}

static void DrawItemChoices(u8 itemIndex, u8 y)
{
    const struct MfRulesMenuItem *item = &CurrentPage()->items[itemIndex];
    u8 value = sMenu->selections[itemIndex];
    u8 styles[2] = {0};

    if (item->kind == MF_RULES_MENU_ITEM_NEXT || item->kind == MF_RULES_MENU_ITEM_EXIT)
        return;

    if (item->choiceCount == 2)
    {
        styles[value] = 1;
        DrawChoiceText(item->choices[0].label, CHOICE_LEFT_X, y, styles[0]);
        DrawChoiceText(item->choices[1].label,
                       GetStringRightAlignXOffset(FONT_NORMAL, item->choices[1].label, CHOICE_RIGHT_X),
                       y, styles[1]);
    }
    else if (item->choiceCount > 0 && value < item->choiceCount)
    {
        // 3+ choices: show active value only (overflow-safe vs ME multi-slot chrome).
        DrawChoiceText(item->choices[value].label,
                       GetStringRightAlignXOffset(FONT_NORMAL, item->choices[value].label, CHOICE_RIGHT_X),
                       y, TRUE);
    }
}

static void DrawItemRow(u8 itemIndex, u8 y)
{
    const struct MfRulesMenuItem *item = &CurrentPage()->items[itemIndex];

    AddTextPrinterParameterized(WIN_OPTIONS, FONT_NORMAL, item->label, 8, y + 1, TEXT_SKIP_DRAW, NULL);
    DrawItemChoices(itemIndex, y);
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
    const u8 *desc = sDesc_Off;
    u8 color[3];

    if (item->choices != NULL)
    {
        if (item->kind == MF_RULES_MENU_ITEM_NEXT || item->kind == MF_RULES_MENU_ITEM_EXIT)
            desc = item->choices[0].description;
        else if (value < item->choiceCount)
            desc = item->choices[value].description;
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

static void GoToNextPage(void)
{
    if (sMenu->page + 1 >= ARRAY_COUNT(sDemoPages))
        return;

    sMenu->page++;
    sMenu->menuCursor = 0;
    sMenu->scrollOffset = 0;
    LoadPageSelections();
    DrawTopBar();
    DrawAllOptions();
    DrawDescription();
    HighlightItem();
}

static void CycleValue(s8 delta)
{
    const struct MfRulesMenuItem *item = CurrentItem();
    u8 count;
    s16 next;

    if (item->kind != MF_RULES_MENU_ITEM_BOOL && item->kind != MF_RULES_MENU_ITEM_VALUE)
        return;

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
        PlaySE(SE_SELECT);
        BeginExit(taskId);
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

#else // !MF_RULES_ENGINE

void CB2_InitMfRulesMenu(void)
{
    SetMainCallback2(gMain.savedCallback);
}

#endif // MF_RULES_ENGINE
