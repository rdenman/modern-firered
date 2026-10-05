#include "global.h"
#include "mf_rules.h"
#include "mf_stats.h"
#include "pokemon.h"

struct MfClassicBaseStats
{
    u16 species;
    u8 stats[NUM_STATS]; // STAT_HP … STAT_SPDEF
};

#include "data/mf_classic_base_stats.h"

static const u16 sBstEqualizerTargets[] = { 0, 100, 255, 500 };

static const u8 *FindClassicBaseStats(enum Species species)
{
    s32 lo = 0;
    s32 hi = (s32)ARRAY_COUNT(sClassicBaseStats) - 1;

    while (lo <= hi)
    {
        s32 mid = (lo + hi) / 2;
        u16 midSpecies = sClassicBaseStats[mid].species;

        if (midSpecies == species)
            return sClassicBaseStats[mid].stats;
        if (midSpecies < species)
            lo = mid + 1;
        else
            hi = mid - 1;
    }
    return NULL;
}

static u32 ModernBaseStat(enum Species species, u32 statIndex)
{
    const struct SpeciesInfo *info = &gSpeciesInfo[species];

    switch (statIndex)
    {
    case STAT_HP:     return info->baseHP;
    case STAT_ATK:    return info->baseAttack;
    case STAT_DEF:    return info->baseDefense;
    case STAT_SPEED:  return info->baseSpeed;
    case STAT_SPATK:  return info->baseSpAttack;
    case STAT_SPDEF:  return info->baseSpDefense;
    }
    return 0;
}

static u32 LookupBaseStat(enum Species species, u32 statIndex, bool8 modernStats)
{
    const u8 *classic;

    if (modernStats)
        return ModernBaseStat(species, statIndex);

    classic = FindClassicBaseStats(species);
    if (classic != NULL)
        return classic[statIndex];

    return ModernBaseStat(species, statIndex);
}

u32 MfGetBstEqualizerTarget(u8 mode)
{
    if (mode >= ARRAY_COUNT(sBstEqualizerTargets))
        return 0;
    return sBstEqualizerTargets[mode];
}

void MfNormalizeStatsToBst(u32 *stats, u32 count, u32 target)
{
    u32 i;
    u32 total = 0;
    u32 assigned = 0;
    u32 leftover;
    u32 scaled[NUM_STATS];
    u32 remainders[NUM_STATS];
    u32 orig[NUM_STATS];
    bool8 gotExtra[NUM_STATS] = {0};

    if (stats == NULL || count == 0 || count > NUM_STATS)
        return;

    for (i = 0; i < count; i++)
    {
        orig[i] = stats[i];
        total += stats[i];
    }

    if (total == 0 || target == 0)
        return;

    for (i = 0; i < count; i++)
    {
        scaled[i] = (stats[i] * target) / total;
        remainders[i] = (stats[i] * target) % total;
        assigned += scaled[i];
    }

    leftover = target - assigned;
    while (leftover > 0)
    {
        s32 best = -1;

        for (i = 0; i < count; i++)
        {
            if (gotExtra[i])
                continue;
            if (best < 0
                || remainders[i] > remainders[best]
                || (remainders[i] == remainders[best] && orig[i] > orig[best])
                || (remainders[i] == remainders[best] && orig[i] == orig[best] && i < (u32)best))
                best = i;
        }
        if (best < 0)
            break;
        scaled[best]++;
        gotExtra[best] = TRUE;
        leftover--;
    }

    for (i = 0; i < count; i++)
    {
        if (orig[i] > 0 && scaled[i] == 0)
        {
            s32 donor = -1;
            u32 j;

            scaled[i] = 1;
            for (j = 0; j < count; j++)
            {
                if (j == i)
                    continue;
                if (scaled[j] > 1 && (donor < 0 || scaled[j] > scaled[donor]))
                    donor = j;
            }
            if (donor >= 0)
                scaled[donor]--;
        }
        if (scaled[i] > 255)
            scaled[i] = 255;
    }

    assigned = 0;
    for (i = 0; i < count; i++)
        assigned += scaled[i];

    while (assigned < target)
    {
        s32 best = -1;

        for (i = 0; i < count; i++)
        {
            if (scaled[i] >= 255)
                continue;
            if (best < 0 || orig[i] > orig[best])
                best = i;
        }
        if (best < 0)
            break;
        scaled[best]++;
        assigned++;
    }

    while (assigned > target)
    {
        s32 best = -1;

        for (i = 0; i < count; i++)
        {
            if (scaled[i] <= 1)
                continue;
            if (best < 0 || scaled[i] > scaled[best])
                best = i;
        }
        if (best < 0)
            break;
        scaled[best]--;
        assigned--;
    }

    for (i = 0; i < count; i++)
        stats[i] = scaled[i];
}

static void FillEqualizedStats(enum Species species, bool8 modernStats, u32 target, u32 *out)
{
    u32 i;
    u32 rest[NUM_STATS - 1];

    for (i = 0; i < NUM_STATS; i++)
        out[i] = LookupBaseStat(species, i, modernStats);

    // Keep 1 HP (Shedinja) so Wonder Guard isn't paired with a real HP stat.
    // Don't call HasShedinjaHPHandling — it reads GetSpeciesBaseHP (recursion).
    if (out[STAT_HP] == 1 && target > 1)
    {
        for (i = 0; i < NUM_STATS - 1; i++)
            rest[i] = out[i + 1];
        MfNormalizeStatsToBst(rest, NUM_STATS - 1, target - 1);
        out[STAT_HP] = 1;
        for (i = 0; i < NUM_STATS - 1; i++)
            out[i + 1] = rest[i];
        return;
    }

    MfNormalizeStatsToBst(out, NUM_STATS, target);
}

u32 MfGetSpeciesBaseStat(enum Species species, u32 statIndex)
{
    enum Species sanitized = SanitizeSpeciesId(species);
    u8 mode;
    u32 target;
    bool8 modernStats;
    static enum Species sCacheSpecies;
    static u8 sCacheMode;
    static bool8 sCacheModern;
    static u32 sCacheStats[NUM_STATS];
    static bool8 sCacheValid;

    if (statIndex >= NUM_STATS)
        return 0;

    mode = MfRules_GetBaseStatEqualizer();
    target = MfGetBstEqualizerTarget(mode);
    modernStats = MfRules_HasModernStats();

    if (target == 0)
        return LookupBaseStat(sanitized, statIndex, modernStats);

    if (!sCacheValid
        || sCacheSpecies != sanitized
        || sCacheMode != mode
        || sCacheModern != modernStats)
    {
        FillEqualizedStats(sanitized, modernStats, target, sCacheStats);
        sCacheSpecies = sanitized;
        sCacheMode = mode;
        sCacheModern = modernStats;
        sCacheValid = TRUE;
    }

    return sCacheStats[statIndex];
}

void MfRecalculatePartyStats(void)
{
    u32 i;

    for (i = 0; i < PARTY_SIZE; i++)
    {
        if (GetMonData(&gParties[B_TRAINER_PLAYER][i], MON_DATA_SPECIES) != SPECIES_NONE)
            CalculateMonStats(&gParties[B_TRAINER_PLAYER][i]);
    }
}
