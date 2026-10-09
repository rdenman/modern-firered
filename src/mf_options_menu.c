#include "global.h"
#include "mf_options_menu.h"
#include "mf_options.h"
#include "mf_nuzlocke.h"
#include "bg.h"
#include "gpu_regs.h"
#include "international_string_util.h"
#include "main.h"
#include "malloc.h"
#include "menu.h"
#include "palette.h"
#include "scanline_effect.h"
#include "sound.h"
#include "sprite.h"
#include "string_util.h"
#include "task.h"
#include "text.h"
#include "text_window.h"
#include "window.h"
#include "gba/m4a_internal.h"
#include "constants/rgb.h"
#include "constants/songs.h"

// S58 — data-driven Options+ shell (S18 windows/fonts; ME page split).

#if MF_OPTIONS_PLUS

enum
{
    WIN_TOPBAR,
    WIN_OPTIONS,
    WIN_DESCRIPTION,
};

enum MfOptionsMenuItemKind
{
    MF_OPTIONS_MENU_ITEM_VALUE,
    MF_OPTIONS_MENU_ITEM_FRAME,
    MF_OPTIONS_MENU_ITEM_NEXT,
    MF_OPTIONS_MENU_ITEM_EXIT,
};

enum MfOptionsMenuItemFlags
{
    MF_OPTIONS_MENU_FLAG_NONE = 0,
    MF_OPTIONS_MENU_FLAG_HARDCORE_LOCKS_SET = 1 << 0,
};

struct MfOptionsMenuChoice
{
    const u8 *label;
    const u8 *description;
};

struct MfOptionsMenuItem
{
    const u8 *label;
    u8 kind;
    u8 optionId;
    u8 choiceCount;
    u16 flags;
    const struct MfOptionsMenuChoice *choices;
};

struct MfOptionsMenuPage
{
    const u8 *title;
    const struct MfOptionsMenuItem *items;
    u8 itemCount;
};

#define Y_DIFF 16
#define OPTIONS_ON_SCREEN 5
#define MF_OPTIONS_MENU_MAX_ITEMS 16
#define CHOICE_LEFT_X 104
#define CHOICE_RIGHT_X 198

struct MfOptionsMenuState
{
    u8 page;
    u8 menuCursor;
    u8 scrollOffset;
    u8 selections[MF_OPTIONS_MENU_MAX_ITEMS];
};

static EWRAM_DATA struct MfOptionsMenuState *sMenu = NULL;

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

static const u8 sText_Slow[] = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}SLOW");
static const u8 sText_Mid[] = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}MID");
static const u8 sText_Fast[] = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}FAST");
static const u8 sText_Instant[] = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}INSTANT");
static const u8 sText_On[] = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}ON");
static const u8 sText_Off[] = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}OFF");
static const u8 sText_Shift[] = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}SHIFT");
static const u8 sText_Set[] = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}SET");
static const u8 sText_Mono[] = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}MONO");
static const u8 sText_Stereo[] = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}STEREO");
static const u8 sText_Normal[] = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}NORMAL");
static const u8 sText_LR[] = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}LR");
static const u8 sText_LA[] = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}L=A");
static const u8 sText_Metric[] = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}METRIC");
static const u8 sText_Imperial[] = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}IMPERIAL");
static const u8 sText_1x[] = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}1x");
static const u8 sText_2x[] = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}2x");
static const u8 sText_FramePrefix[] = _("{COLOR GREEN}{SHADOW LIGHT_GREEN}TYPE ");

static const u8 sDesc_TextSlow[] = _("Text prints slowly.");
static const u8 sDesc_TextMid[] = _("Text prints at medium speed.");
static const u8 sDesc_TextFast[] = _("Text prints quickly.");
static const u8 sDesc_TextInstant[] = _("Text appears instantly.");
static const u8 sDesc_SceneOn[] = _("Show battle animations.");
static const u8 sDesc_SceneOff[] = _("Skip battle animations.");
static const u8 sDesc_StyleShift[] = _("Ask before sending out the\nnext Pokémon.");
static const u8 sDesc_StyleSet[] = _("Send out the next Pokémon\nwithout asking.");
static const u8 sDesc_StyleHardcore[] = _("Hardcore Nuzlocke locks\nBattle Style to SET.");
static const u8 sDesc_Mono[] = _("Single-channel sound.");
static const u8 sDesc_Stereo[] = _("Two-channel sound.");
static const u8 sDesc_BtnNormal[] = _("Help system on L and R.");
static const u8 sDesc_BtnLR[] = _("L and R are used as extra\nbuttons.");
static const u8 sDesc_BtnLA[] = _("L works like A.");
static const u8 sDesc_Frame[] = _("Change the window frame.\nLeft/Right pick TYPE 1-20.");
static const u8 sDesc_Next[] = _("Go to the next page.\nL/R also switch pages.");
static const u8 sDesc_Exit[] = _("Close Options and keep\nyour settings.");
static const u8 sDesc_AutorunOn[] = _("Run by default. Hold B to walk.");
static const u8 sDesc_AutorunOff[] = _("Hold B to run, as usual.");
static const u8 sDesc_AutorunSurfOn[] = _("Surf at run speed by default.");
static const u8 sDesc_AutorunSurfOff[] = _("Surf at the usual speed.");
static const u8 sDesc_JoyOn[] = _("Poké Center healing is much\nfaster.");
static const u8 sDesc_JoyOff[] = _("Poké Center healing uses the\nfull animation.");
static const u8 sDesc_SkipOn[] = _("Skip the title intro movie\non boot.");
static const u8 sDesc_SkipOff[] = _("Play the title intro as usual.");
static const u8 sDesc_Metric[] = _("Heights and weights use\nmetric units.");
static const u8 sDesc_Imperial[] = _("Heights and weights use\nimperial units.");
static const u8 sDesc_FastBatOn[] = _("Speed up battle messages.");
static const u8 sDesc_FastBatOff[] = _("Battle message speed uses\nthe ROM default.");
static const u8 sDesc_FastIntroOn[] = _("Faster battle intro slides.");
static const u8 sDesc_FastIntroOff[] = _("Battle intros use the ROM\ndefault.");
static const u8 sDesc_Anim1x[] = _("Battle animation wait time\nat 1x.");
static const u8 sDesc_Anim2x[] = _("Battle animation wait time\nat 2x.");
static const u8 sDesc_BallOn[] = _("Show the last-ball prompt.");
static const u8 sDesc_BallOff[] = _("Do not show the last-ball\nprompt.");
static const u8 sDesc_HintsOn[] = _("Show STAB and effectiveness\nin the move menu.");
static const u8 sDesc_HintsOff[] = _("No STAB or effectiveness\nhints on moves.");
static const u8 sDesc_CursorOn[] = _("Menus remember your last\ncursor.");
static const u8 sDesc_CursorOff[] = _("Menus start at the first\nitem each visit.");
static const u8 sDesc_RunPromptOn[] = _("L/R can confirm Run.");
static const u8 sDesc_RunPromptOff[] = _("No L/R Run shortcut.");

static const struct MfOptionsMenuChoice sChoicesTextSpeed[] =
{
    {sText_Slow, sDesc_TextSlow},
    {sText_Mid, sDesc_TextMid},
    {sText_Fast, sDesc_TextFast},
    {sText_Instant, sDesc_TextInstant},
};

static const struct MfOptionsMenuChoice sChoicesScene[] =
{
    {sText_On, sDesc_SceneOn},
    {sText_Off, sDesc_SceneOff},
};

static const struct MfOptionsMenuChoice sChoicesStyle[] =
{
    {sText_Shift, sDesc_StyleShift},
    {sText_Set, sDesc_StyleSet},
};

static const struct MfOptionsMenuChoice sChoicesSound[] =
{
    {sText_Mono, sDesc_Mono},
    {sText_Stereo, sDesc_Stereo},
};

static const struct MfOptionsMenuChoice sChoicesButton[] =
{
    {sText_Normal, sDesc_BtnNormal},
    {sText_LR, sDesc_BtnLR},
    {sText_LA, sDesc_BtnLA},
};

static const struct MfOptionsMenuChoice sChoicesNext[] =
{
    {NULL, sDesc_Next},
};

static const struct MfOptionsMenuChoice sChoicesExit[] =
{
    {NULL, sDesc_Exit},
};

static const struct MfOptionsMenuChoice sChoicesOnOffLaterRun[] =
{
    {sText_Off, sDesc_AutorunOff},
    {sText_On, sDesc_AutorunOn},
};

static const struct MfOptionsMenuChoice sChoicesOnOffSurf[] =
{
    {sText_Off, sDesc_AutorunSurfOff},
    {sText_On, sDesc_AutorunSurfOn},
};

static const struct MfOptionsMenuChoice sChoicesJoy[] =
{
    {sText_Off, sDesc_JoyOff},
    {sText_On, sDesc_JoyOn},
};

static const struct MfOptionsMenuChoice sChoicesSkip[] =
{
    {sText_Off, sDesc_SkipOff},
    {sText_On, sDesc_SkipOn},
};

static const struct MfOptionsMenuChoice sChoicesUnits[] =
{
    {sText_Metric, sDesc_Metric},
    {sText_Imperial, sDesc_Imperial},
};

static const struct MfOptionsMenuChoice sChoicesFastBat[] =
{
    {sText_Off, sDesc_FastBatOff},
    {sText_On, sDesc_FastBatOn},
};

static const struct MfOptionsMenuChoice sChoicesFastIntro[] =
{
    {sText_Off, sDesc_FastIntroOff},
    {sText_On, sDesc_FastIntroOn},
};

static const struct MfOptionsMenuChoice sChoicesAnim[] =
{
    {sText_1x, sDesc_Anim1x},
    {sText_2x, sDesc_Anim2x},
};

static const struct MfOptionsMenuChoice sChoicesBall[] =
{
    {sText_Off, sDesc_BallOff},
    {sText_On, sDesc_BallOn},
};

static const struct MfOptionsMenuChoice sChoicesHints[] =
{
    {sText_Off, sDesc_HintsOff},
    {sText_On, sDesc_HintsOn},
};

static const struct MfOptionsMenuChoice sChoicesCursor[] =
{
    {sText_Off, sDesc_CursorOff},
    {sText_On, sDesc_CursorOn},
};

static const struct MfOptionsMenuChoice sChoicesRunPrompt[] =
{
    {sText_Off, sDesc_RunPromptOff},
    {sText_On, sDesc_RunPromptOn},
};

static const struct MfOptionsMenuItem sMainPageItems[] =
{
    {COMPOUND_STRING("TEXT SPEED"), MF_OPTIONS_MENU_ITEM_VALUE, MF_OPT_TEXT_SPEED, 4, 0, sChoicesTextSpeed},
    {COMPOUND_STRING("BATTLE SCENE"), MF_OPTIONS_MENU_ITEM_VALUE, MF_OPT_BATTLE_SCENE, 2, 0, sChoicesScene},
    {COMPOUND_STRING("BATTLE STYLE"), MF_OPTIONS_MENU_ITEM_VALUE, MF_OPT_BATTLE_STYLE, 2, MF_OPTIONS_MENU_FLAG_HARDCORE_LOCKS_SET, sChoicesStyle},
    {COMPOUND_STRING("SOUND"), MF_OPTIONS_MENU_ITEM_VALUE, MF_OPT_SOUND, 2, 0, sChoicesSound},
    {COMPOUND_STRING("BUTTON MODE"), MF_OPTIONS_MENU_ITEM_VALUE, MF_OPT_BUTTON_MODE, 3, 0, sChoicesButton},
    {COMPOUND_STRING("FRAME"), MF_OPTIONS_MENU_ITEM_FRAME, MF_OPT_FRAME, WINDOW_FRAMES_COUNT, 0, NULL},
    {COMPOUND_STRING("NEXT"), MF_OPTIONS_MENU_ITEM_NEXT, 0, 0, 0, sChoicesNext},
};

static const struct MfOptionsMenuItem sQolPageItems[] =
{
    {COMPOUND_STRING("AUTORUN"), MF_OPTIONS_MENU_ITEM_VALUE, MF_OPT_AUTORUN, 2, 0, sChoicesOnOffLaterRun},
    {COMPOUND_STRING("AUTORUN SURF"), MF_OPTIONS_MENU_ITEM_VALUE, MF_OPT_AUTORUN_SURF, 2, 0, sChoicesOnOffSurf},
    {COMPOUND_STRING("EVEN FASTER JOY"), MF_OPTIONS_MENU_ITEM_VALUE, MF_OPT_EVEN_FASTER_JOY, 2, 0, sChoicesJoy},
    {COMPOUND_STRING("SKIP INTRO"), MF_OPTIONS_MENU_ITEM_VALUE, MF_OPT_SKIP_INTRO, 2, 0, sChoicesSkip},
    {COMPOUND_STRING("UNIT SYSTEM"), MF_OPTIONS_MENU_ITEM_VALUE, MF_OPT_UNIT_IMPERIAL, 2, 0, sChoicesUnits},
    {COMPOUND_STRING("NEXT"), MF_OPTIONS_MENU_ITEM_NEXT, 0, 0, 0, sChoicesNext},
};

static const struct MfOptionsMenuItem sBattlePageItems[] =
{
    {COMPOUND_STRING("FAST BATTLES"), MF_OPTIONS_MENU_ITEM_VALUE, MF_OPT_FAST_BATTLES, 2, 0, sChoicesFastBat},
    {COMPOUND_STRING("ANIM SPEED"), MF_OPTIONS_MENU_ITEM_VALUE, MF_OPT_ANIM_SPEED, 2, 0, sChoicesAnim},
    {COMPOUND_STRING("FAST INTRO"), MF_OPTIONS_MENU_ITEM_VALUE, MF_OPT_FAST_INTRO, 2, 0, sChoicesFastIntro},
    {COMPOUND_STRING("BALL PROMPT"), MF_OPTIONS_MENU_ITEM_VALUE, MF_OPT_BALL_PROMPT, 2, 0, sChoicesBall},
    {COMPOUND_STRING("STAB/EFFECTIVE"), MF_OPTIONS_MENU_ITEM_VALUE, MF_OPT_TYPE_HINTS, 2, 0, sChoicesHints},
    {COMPOUND_STRING("CURSOR MEMORY"), MF_OPTIONS_MENU_ITEM_VALUE, MF_OPT_CURSOR_MEMORY, 2, 0, sChoicesCursor},
    {COMPOUND_STRING("RUN PROMPT"), MF_OPTIONS_MENU_ITEM_VALUE, MF_OPT_RUN_PROMPT, 2, 0, sChoicesRunPrompt},
    {COMPOUND_STRING("EXIT"), MF_OPTIONS_MENU_ITEM_EXIT, 0, 0, 0, sChoicesExit},
};

static const struct MfOptionsMenuPage sPages[] =
{
    {COMPOUND_STRING("OPTIONS"), sMainPageItems, ARRAY_COUNT(sMainPageItems)},
    {COMPOUND_STRING("QOL OPTIONS"), sQolPageItems, ARRAY_COUNT(sQolPageItems)},
    {COMPOUND_STRING("BATTLE OPTIONS"), sBattlePageItems, ARRAY_COUNT(sBattlePageItems)},
};

static const struct MfOptionsMenuPage *CurrentPage(void)
{
    return &sPages[sMenu->page];
}

static const struct MfOptionsMenuItem *CurrentItem(void)
{
    return &CurrentPage()->items[sMenu->menuCursor];
}

static bool8 ItemIsEditable(const struct MfOptionsMenuItem *item)
{
    if (item->kind == MF_OPTIONS_MENU_ITEM_NEXT || item->kind == MF_OPTIONS_MENU_ITEM_EXIT)
        return TRUE;
    if ((item->flags & MF_OPTIONS_MENU_FLAG_HARDCORE_LOCKS_SET) && MfNuzlocke_ForcesSetBattleStyle())
        return FALSE;
    return TRUE;
}

static void ReloadFrameGfx(u8 frame)
{
    LoadBgTiles(1, GetWindowFrameTilesPal(frame)->tiles, 0x120, 0x1A2);
    LoadPalette(GetWindowFrameTilesPal(frame)->pal, BG_PLTT_ID(7), PLTT_SIZE_4BPP);
}

static void LoadPageSelections(void)
{
    const struct MfOptionsMenuPage *page = CurrentPage();
    u8 i;

    for (i = 0; i < page->itemCount; i++)
    {
        const struct MfOptionsMenuItem *item = &page->items[i];
        u8 value;

        if (item->kind != MF_OPTIONS_MENU_ITEM_VALUE && item->kind != MF_OPTIONS_MENU_ITEM_FRAME)
        {
            sMenu->selections[i] = 0;
            continue;
        }

        value = MfOptions_Get(item->optionId);
        if (item->choiceCount != 0 && value >= item->choiceCount)
            value = 0;
        if ((item->flags & MF_OPTIONS_MENU_FLAG_HARDCORE_LOCKS_SET) && MfNuzlocke_ForcesSetBattleStyle())
            value = OPTIONS_BATTLE_STYLE_SET;
        sMenu->selections[i] = value;
    }
}

static void WriteSelection(u8 itemIndex)
{
    const struct MfOptionsMenuItem *item = &CurrentPage()->items[itemIndex];
    u8 value = sMenu->selections[itemIndex];

    if (item->kind != MF_OPTIONS_MENU_ITEM_VALUE && item->kind != MF_OPTIONS_MENU_ITEM_FRAME)
        return;
    if (!ItemIsEditable(item))
        return;

    MfOptions_Set(item->optionId, value);
    if (item->optionId == MF_OPT_SOUND)
        SetPokemonCryStereo(value);
    if (item->optionId == MF_OPT_FRAME)
        ReloadFrameGfx(value);
}

static void DrawChoiceText(const u8 *text, u8 x, u8 y, bool8 selected, bool8 active)
{
    u8 dst[24];
    u16 i;

    for (i = 0; *text != EOS && i < ARRAY_COUNT(dst) - 1; i++)
        dst[i] = *(text++);
    dst[i] = EOS;

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

static void DrawFrameChoices(u8 value, u8 y, bool8 active)
{
    u8 text[16];
    u8 n = value + 1;

    StringCopy(text, sText_FramePrefix);
    ConvertIntToDecimalStringN(gStringVar1, n, STR_CONV_MODE_LEFT_ALIGN, 2);
    StringAppend(text, gStringVar1);
    DrawChoiceText(text,
                   GetStringRightAlignXOffset(FONT_NORMAL, text, CHOICE_RIGHT_X),
                   y, TRUE, active);
}

static void DrawItemChoices(u8 itemIndex, u8 y, bool8 active)
{
    const struct MfOptionsMenuItem *item = &CurrentPage()->items[itemIndex];
    u8 value = sMenu->selections[itemIndex];
    u8 styles[2] = {0};

    if (item->kind == MF_OPTIONS_MENU_ITEM_NEXT || item->kind == MF_OPTIONS_MENU_ITEM_EXIT)
        return;

    if (item->kind == MF_OPTIONS_MENU_ITEM_FRAME)
    {
        DrawFrameChoices(value, y, active);
        return;
    }

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
        DrawChoiceText(item->choices[value].label,
                       GetStringRightAlignXOffset(FONT_NORMAL, item->choices[value].label, CHOICE_RIGHT_X),
                       y, TRUE, active);
    }
}

static void DrawItemRow(u8 itemIndex, u8 y)
{
    const struct MfOptionsMenuItem *item = &CurrentPage()->items[itemIndex];
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
    const struct MfOptionsMenuPage *page = CurrentPage();
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
    const struct MfOptionsMenuItem *item = CurrentItem();
    u8 value = sMenu->selections[sMenu->menuCursor];
    const u8 *desc = sDesc_Exit;
    u8 color[3];

    if (!ItemIsEditable(item) && (item->flags & MF_OPTIONS_MENU_FLAG_HARDCORE_LOCKS_SET))
        desc = sDesc_StyleHardcore;
    else if (item->kind == MF_OPTIONS_MENU_ITEM_FRAME)
        desc = sDesc_Frame;
    else if (item->choices != NULL)
    {
        if (item->kind == MF_OPTIONS_MENU_ITEM_NEXT || item->kind == MF_OPTIONS_MENU_ITEM_EXIT)
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
    SetGpuRegBits(REG_OFFSET_DISPCNT, DISPCNT_WIN0_ON);
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
    FillBgTilemapBufferRect(1, TILE_TOP_CORNER_L,  1,  2,  1,  1,  7);
    FillBgTilemapBufferRect(1, TILE_TOP_EDGE,      2,  2, 26,  1,  7);
    FillBgTilemapBufferRect(1, TILE_TOP_CORNER_R, 28,  2,  1,  1,  7);
    FillBgTilemapBufferRect(1, TILE_LEFT_EDGE,     1,  3,  1, 10,  7);
    FillBgTilemapBufferRect(1, TILE_RIGHT_EDGE,   28,  3,  1, 10,  7);
    FillBgTilemapBufferRect(1, TILE_BOT_CORNER_L,  1, 13,  1,  1,  7);
    FillBgTilemapBufferRect(1, TILE_BOT_EDGE,      2, 13, 26,  1,  7);
    FillBgTilemapBufferRect(1, TILE_BOT_CORNER_R, 28, 13,  1,  1,  7);

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

static void Task_FadeIn(u8 taskId);
static void Task_ProcessInput(u8 taskId);
static void Task_FadeOut(u8 taskId);

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
        GoToPage(0);
    else
        GoToPage(sMenu->page + 1);
}

static void GoToPrevPage(void)
{
    if (sMenu->page == 0)
        GoToPage(ARRAY_COUNT(sPages) - 1);
    else
        GoToPage(sMenu->page - 1);
}

static void CycleValue(s8 delta)
{
    const struct MfOptionsMenuItem *item = CurrentItem();
    u8 count;
    s16 next;

    if (item->kind != MF_OPTIONS_MENU_ITEM_VALUE && item->kind != MF_OPTIONS_MENU_ITEM_FRAME)
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
    const struct MfOptionsMenuPage *page = CurrentPage();
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
    const struct MfOptionsMenuItem *item = CurrentItem();

    if (JOY_NEW(A_BUTTON))
    {
        if (item->kind == MF_OPTIONS_MENU_ITEM_NEXT)
        {
            PlaySE(SE_SELECT);
            GoToNextPage();
        }
        else if (item->kind == MF_OPTIONS_MENU_ITEM_EXIT)
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
    else if (JOY_NEW(L_BUTTON))
    {
        PlaySE(SE_SELECT);
        GoToPrevPage();
    }
    else if (JOY_NEW(R_BUTTON))
    {
        PlaySE(SE_SELECT);
        GoToNextPage();
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

void CB2_InitMfOptionsMenu(void)
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
        MfOptions_GetSave();
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

#else // !MF_OPTIONS_PLUS

void CB2_InitMfOptionsMenu(void)
{
    SetMainCallback2(gMain.savedCallback);
}

#endif
