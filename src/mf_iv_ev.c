#include "global.h"
#include "battle.h"
#include "event_data.h"
#include "mf_iv_ev.h"
#include "mf_rules.h"
#include "pokemon.h"
#include "random.h"
#include "constants/battle.h"
#include "constants/pokemon.h"

// ME sIV_Table / sEV_Table, keyed by badge count (0..8).
static const u8 sTrainerIvTable[MF_TRAINER_IV_EV_BADGE_STAGES] =
{
    7, 10, 13, 16, 19, 22, 25, 28, 31,
};

static const u8 sTrainerEvTable[MF_TRAINER_IV_EV_BADGE_STAGES] =
{
    12, 24, 36, 48, 60, 72, 80, 100, 128,
};

static u8 MfClampBadgeStage(u8 badgeCount)
{
    if (badgeCount >= MF_TRAINER_IV_EV_BADGE_STAGES)
        return MF_TRAINER_IV_EV_BADGE_STAGES - 1;
    return badgeCount;
}

static u8 MfCountBadges(void)
{
    u8 count = 0;
    u32 flag;

    for (flag = FLAG_BADGE01_GET; flag < FLAG_BADGE01_GET + NUM_BADGES; flag++)
    {
        if (FlagGet(flag))
            count++;
    }
    return count;
}

u8 MfResolveTrainerIVs(u8 mode, u8 badgeCount)
{
    if (mode == MF_TRAINER_IVS_SCALE)
        return sTrainerIvTable[MfClampBadgeStage(badgeCount)];
    if (mode == MF_TRAINER_IVS_HARD)
        return MAX_PER_STAT_IVS;
    return 0;
}

u8 MfResolveTrainerEVs(u8 mode, u8 badgeCount)
{
    if (mode == MF_TRAINER_EVS_SCALE)
        return sTrainerEvTable[MfClampBadgeStage(badgeCount)];
    if (mode == MF_TRAINER_EVS_HARD)
        return MF_TRAINER_EVS_HARD_VALUE;
    if (mode == MF_TRAINER_EVS_EXTREME)
        return MAX_PER_STAT_EVS;
    return 0;
}

u8 MfGetCurrentTrainerIVs(void)
{
    return MfResolveTrainerIVs(MfRules_GetScalingIvs(), MfCountBadges());
}

u8 MfGetCurrentTrainerEVs(void)
{
    return MfResolveTrainerEVs(MfRules_GetScalingEvs(), MfCountBadges());
}

bool8 MfArePlayerEvsDisabled(void)
{
    // ME: NoEVs && !FLAG_IS_CHAMPION. FireRed's championship flag is Hall of Fame.
    return MfRules_HasNoEvs() && !FlagGet(FLAG_SYS_GAME_CLEAR);
}

bool8 MfShouldCapEVItems(void)
{
    return B_EV_ITEMS_CAP || MfArePlayerEvsDisabled();
}

static void MfSetAllBoxIvs(struct BoxPokemon *mon, u8 iv)
{
    u32 i;

    for (i = 0; i < NUM_STATS; i++)
        SetBoxMonData(mon, MON_DATA_HP_IV + i, &iv);
}

void MfApplyPlayerPartyIvsToBoxMon(struct BoxPokemon *mon)
{
    u8 mode = MfRules_GetMaxPartyIvs();
    u32 i;

    if (mode == MF_PLAYER_IVS_MAX)
    {
        MfSetAllBoxIvs(mon, MAX_PER_STAT_IVS);
        return;
    }

    if (mode != MF_PLAYER_IVS_HP)
        return;

    for (i = 0; i < NUM_STATS; i++)
    {
        u8 iv = (Random() & 1) ? 30 : 31;
        SetBoxMonData(mon, MON_DATA_HP_IV + i, &iv);
    }
}

void MfApplyPlayerPartyIvsToMon(struct Pokemon *mon)
{
    MfApplyPlayerPartyIvsToBoxMon(&mon->box);
}

void MfApplyPlayerIvEvRulesToMon(struct Pokemon *mon)
{
    u8 zero = 0;
    u32 i;

    MfApplyPlayerPartyIvsToMon(mon);
    if (!MfArePlayerEvsDisabled())
        return;

    for (i = 0; i < NUM_STATS; i++)
        SetMonData(mon, MON_DATA_HP_EV + i, &zero);
}

static bool8 MfTrainerIvEvScalingExcluded(void)
{
    return (gBattleTypeFlags & (BATTLE_TYPE_FRONTIER
                              | BATTLE_TYPE_EREADER_TRAINER
                              | BATTLE_TYPE_TRAINER_HILL)) != 0;
}

static void MfApplyTrainerIvsToMon(struct Pokemon *mon, u8 iv)
{
    u32 i;

    for (i = 0; i < NUM_STATS; i++)
        SetMonData(mon, MON_DATA_HP_IV + i, &iv);
}

static void MfApplyTrainerEvsToMon(struct Pokemon *mon, u8 ev)
{
    // ME: HP, Speed, and the stronger of Atk/SpAtk and Def/SpDef.
    SetMonData(mon, MON_DATA_HP_EV, &ev);
    SetMonData(mon, MON_DATA_SPEED_EV, &ev);
    if (GetMonData(mon, MON_DATA_ATK) > GetMonData(mon, MON_DATA_SPATK))
        SetMonData(mon, MON_DATA_ATK_EV, &ev);
    else
        SetMonData(mon, MON_DATA_SPATK_EV, &ev);
    if (GetMonData(mon, MON_DATA_DEF) > GetMonData(mon, MON_DATA_SPDEF))
        SetMonData(mon, MON_DATA_DEF_EV, &ev);
    else
        SetMonData(mon, MON_DATA_SPDEF_EV, &ev);
}

void MfApplyTrainerIvEvScaling(struct Pokemon *party, u32 count)
{
    u8 ivMode = MfRules_GetScalingIvs();
    u8 evMode = MfRules_GetScalingEvs();
    u8 iv;
    u8 ev;
    u32 i;

    if (MfTrainerIvEvScalingExcluded())
        return;
    if (ivMode == MF_TRAINER_IVS_OFF && evMode == MF_TRAINER_EVS_OFF)
        return;

    iv = MfGetCurrentTrainerIVs();
    ev = MfGetCurrentTrainerEVs();

    for (i = 0; i < count; i++)
    {
        if (ivMode != MF_TRAINER_IVS_OFF)
            MfApplyTrainerIvsToMon(&party[i], iv);
        if (evMode != MF_TRAINER_EVS_OFF)
        {
            CalculateMonStats(&party[i]);
            MfApplyTrainerEvsToMon(&party[i], ev);
        }
        if (ivMode != MF_TRAINER_IVS_OFF || evMode != MF_TRAINER_EVS_OFF)
            CalculateMonStats(&party[i]);
    }
}
