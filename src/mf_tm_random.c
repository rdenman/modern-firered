#include "global.h"
#include "event_data.h"
#include "item.h"
#include "line_break.h"
#include "mf_items.h"
#include "mf_random.h"
#include "mf_rules.h"
#include "mf_tm_random.h"
#include "move.h"
#include "string_util.h"
#include "text.h"
#include "constants/items.h"
#include "constants/moves.h"
#include "gba/isagbprint.h"

static EWRAM_DATA enum Move sTutorSourceMove = MOVE_NONE;

enum Move MfTutorSourceMove(void)
{
    return sTutorSourceMove;
}

enum Move MfTutorLearnCheckMove(enum Move destMove)
{
    if (sTutorSourceMove != MOVE_NONE)
        return sTutorSourceMove;
    return destMove;
}

enum Move MfTmLearnCheckMove(enum Item itemId, enum Move destMove)
{
    enum Move vanilla = GetItemTMHMMoveId(itemId);

    if (vanilla != MOVE_NONE)
        return vanilla;
    return destMove;
}

#if MF_RANDOMIZER

static const u16 sTutorSourceMoves[MF_TM_RANDOM_TUTOR_COUNT] =
{
    MOVE_DOUBLE_EDGE,
    MOVE_THUNDER_WAVE,
    MOVE_ROCK_SLIDE,
    MOVE_EXPLOSION,
    MOVE_MEGA_PUNCH,
    MOVE_MEGA_KICK,
    MOVE_DREAM_EATER,
    MOVE_SOFT_BOILED,
    MOVE_SUBSTITUTE,
    MOVE_SWORDS_DANCE,
    MOVE_SEISMIC_TOSS,
    MOVE_COUNTER,
    MOVE_METRONOME,
    MOVE_MIMIC,
    MOVE_BODY_SLAM,
};

static const u16 sCapeSourceMoves[MF_TM_RANDOM_CAPE_COUNT] =
{
    MOVE_FRENZY_PLANT,
    MOVE_BLAST_BURN,
    MOVE_HYDRO_CANNON,
};

#define MF_TM_RANDOM_POOL_MAX MOVES_COUNT
#define MF_TM_RANDOM_SHUFFLE_TAG 0x544D0000u
#define MF_TM_BAG_DESC_WIDTH  102
#define MF_TM_BAG_DESC_LINES  3
#define MF_TM_BAG_DESC_SIZE   256

static EWRAM_DATA u8 sTmDescBuffer[MF_TM_BAG_DESC_SIZE] = {0};
static EWRAM_DATA u16 sDestBySlot[MF_TM_RANDOM_SLOT_COUNT] = {0};
static EWRAM_DATA u16 sShufflePool[MF_TM_RANDOM_POOL_MAX] = {0};
static EWRAM_DATA u32 sCachedSeed = 0;
static EWRAM_DATA bool8 sCacheValid = FALSE;

static bool32 MfIsTeachDestCandidate(enum Move move)
{
    if (move == MOVE_NONE || move == MOVE_STRUGGLE)
        return FALSE;
    if (move >= MOVES_COUNT)
        return FALSE;
    if (GetMoveEffect(move) == EFFECT_PLACEHOLDER)
        return FALSE;
    if (GetMovePP(move) == 0)
        return FALSE;
    return TRUE;
}

static void EnsureTeachMap(u32 seed)
{
    u16 count = 0;
    u16 i;
    enum Move move;

    if (sCacheValid && sCachedSeed == seed)
        return;

    for (move = 1; move < MOVES_COUNT && count < MF_TM_RANDOM_POOL_MAX; move++)
    {
        if (MfIsTeachDestCandidate(move))
            sShufflePool[count++] = move;
    }

    if (count > 1)
    {
        for (i = count - 1; i > 0; i--)
        {
            u16 j = MfRandom_Modulo(seed, MF_RANDOM_CAT_MOVES, MF_TM_RANDOM_SHUFFLE_TAG + i, 0, i + 1);
            u16 tmp = sShufflePool[i];

            sShufflePool[i] = sShufflePool[j];
            sShufflePool[j] = tmp;
        }
    }

    for (i = 0; i < MF_TM_RANDOM_SLOT_COUNT; i++)
        sDestBySlot[i] = (i < count) ? sShufflePool[i] : MOVE_TACKLE;

    sCachedSeed = seed;
    sCacheValid = TRUE;
}

static s16 TutorSlotForSource(enum Move sourceMove)
{
    u16 i;

    for (i = 0; i < MF_TM_RANDOM_TUTOR_COUNT; i++)
    {
        if (sTutorSourceMoves[i] == sourceMove)
            return MF_TM_RANDOM_TM_COUNT + i;
    }
    for (i = 0; i < MF_TM_RANDOM_CAPE_COUNT; i++)
    {
        if (sCapeSourceMoves[i] == sourceMove)
            return MF_TM_RANDOM_TM_COUNT + MF_TM_RANDOM_TUTOR_COUNT + i;
    }
    return -1;
}

static enum Move MapTeachSlot(u32 seed, u16 slot)
{
    EnsureTeachMap(seed);
    if (slot >= MF_TM_RANDOM_SLOT_COUNT)
        return MOVE_NONE;
    return sDestBySlot[slot];
}

enum Move MfGetTmHmTeachMove(enum Item itemId)
{
    const struct ModernRules *rules;
    u16 tmIndex;

    if (MfIsHmItem(itemId))
        return GetItemTMHMMoveId(itemId);
    if (itemId < ITEM_TM01 || itemId > ITEM_TM50)
        return GetItemTMHMMoveId(itemId);

    rules = MfRules_GetActiveRules();
    if (!rules->randomTms)
        return GetItemTMHMMoveId(itemId);

    tmIndex = itemId - ITEM_TM01;
    return MapTeachSlot(rules->randomizerSeed, tmIndex);
}

static bool32 MoveDescFitsBag(u8 *str, u32 fontId)
{
    return GetStringWidth(fontId, str, 0) <= MF_TM_BAG_DESC_WIDTH
        && CountLineBreaks(str) < MF_TM_BAG_DESC_LINES;
}

// Summary-screen copy is two long FONT_NARROWER lines. Reflow into the bag's
// 3×FONT_NORMAL box; drop to NARROW / NARROWER only if it still overflows.
static const u8 *FormatMoveAsBagDescription(enum Move dest)
{
    static const u8 sFonts[] = { FONT_NORMAL, FONT_NARROW, FONT_NARROWER };
    u32 i;
    u8 *end;

    for (i = 0; i < ARRAY_COUNT(sFonts); i++)
    {
        StringCopy(sTmDescBuffer, GetMoveDescription(dest));
        StripLineBreaks(sTmDescBuffer);
        BreakStringAutomatic(sTmDescBuffer, MF_TM_BAG_DESC_WIDTH, MF_TM_BAG_DESC_LINES,
                             sFonts[i], HIDE_SCROLL_PROMPT);
        if (i + 1 < ARRAY_COUNT(sFonts) && !MoveDescFitsBag(sTmDescBuffer, sFonts[i]))
            continue;
        if (sFonts[i] != FONT_NORMAL)
        {
            for (end = sTmDescBuffer; *end != EOS; end++)
                ;
            PrependFontIdToFit(sTmDescBuffer, end, FONT_NORMAL, MF_TM_BAG_DESC_WIDTH);
        }
        return sTmDescBuffer;
    }
    return sTmDescBuffer;
}

const u8 *MfGetTmItemDescription(enum Item itemId)
{
    enum Move dest = MfGetTmHmTeachMove(itemId);
    enum Item destTm;

    if (dest == MOVE_NONE || dest == GetItemTMHMMoveId(itemId))
        return NULL;

    destTm = GetTMHMItemIdFromMoveId(dest);
    if (destTm != ITEM_NONE)
        return gItemsInfo[destTm].description;
    return FormatMoveAsBagDescription(dest);
}

enum Move MfMapTutorTeachMove(enum Move sourceMove)
{
    const struct ModernRules *rules = MfRules_GetActiveRules();
    s16 slot;

    if (!rules->randomTms)
        return sourceMove;

    slot = TutorSlotForSource(sourceMove);
    if (slot < 0)
        return sourceMove;
    return MapTeachSlot(rules->randomizerSeed, (u16)slot);
}

enum Move MfApplyTutorRemap(enum Move sourceMove)
{
    sTutorSourceMove = sourceMove;
    return MfMapTutorTeachMove(sourceMove);
}

void MfScriptRemapTutorMove(void)
{
    enum Move dest = MfApplyTutorRemap(gSpecialVar_0x8005);

    gSpecialVar_0x8005 = dest;
    StringCopy(gStringVar1, GetMoveName(dest));
}

void MfTmRandom_DebugDump(void)
{
    const struct ModernRules *rules = MfRules_GetActiveRules();
    u16 i;

    DebugPrintfLevel(MGBA_LOG_WARN, "=== MF TM/tutor map === TMs=%u seed=0x%08X",
                     rules->randomTms, rules->randomizerSeed);
    if (!rules->randomTms)
        return;

    EnsureTeachMap(rules->randomizerSeed);
    for (i = 0; i < MF_TM_RANDOM_TM_COUNT; i++)
        DebugPrintfLevel(MGBA_LOG_WARN, "TM%02u -> %u", i + 1, sDestBySlot[i]);
    for (i = 0; i < MF_TM_RANDOM_TUTOR_COUNT; i++)
        DebugPrintfLevel(MGBA_LOG_WARN, "tutor %u -> %u", sTutorSourceMoves[i], sDestBySlot[MF_TM_RANDOM_TM_COUNT + i]);
    for (i = 0; i < MF_TM_RANDOM_CAPE_COUNT; i++)
        DebugPrintfLevel(MGBA_LOG_WARN, "cape %u -> %u",
                         sCapeSourceMoves[i],
                         sDestBySlot[MF_TM_RANDOM_TM_COUNT + MF_TM_RANDOM_TUTOR_COUNT + i]);
}

#else // !MF_RANDOMIZER

enum Move MfGetTmHmTeachMove(enum Item itemId)
{
    return GetItemTMHMMoveId(itemId);
}

const u8 *MfGetTmItemDescription(enum Item itemId)
{
    return NULL;
}

enum Move MfMapTutorTeachMove(enum Move sourceMove)
{
    return sourceMove;
}

enum Move MfApplyTutorRemap(enum Move sourceMove)
{
    sTutorSourceMove = sourceMove;
    return sourceMove;
}

void MfScriptRemapTutorMove(void)
{
    sTutorSourceMove = gSpecialVar_0x8005;
    StringCopy(gStringVar1, GetMoveName(gSpecialVar_0x8005));
}

void MfTmRandom_DebugDump(void)
{
}

#endif // MF_RANDOMIZER
