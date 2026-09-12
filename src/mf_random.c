#include "global.h"
#include "mf_random.h"
#include "random.h"

// SplitMix32 finalizer — cheap, avalanche-friendly, no mutable stream state.
static u32 MfRandom_Mix(u32 x)
{
    x += 0x9E3779B9u;
    x = (x ^ (x >> 16)) * 0x85EBCA6Bu;
    x = (x ^ (x >> 13)) * 0xC2B2AE35u;
    return x ^ (x >> 16);
}

u32 MfRandom_Hash(u32 seed, enum MfRandomCategory category, u32 inputId, u16 locationKey)
{
    u32 x = seed;

    // Category salt so the same input id remaps differently per domain.
    x ^= 0xA5A5A5A5u * ((u32)category + 1u);
    x = MfRandom_Mix(x);
    x ^= inputId;
    x = MfRandom_Mix(x);
    x ^= ((u32)locationKey << 16) | (u32)locationKey;
    x = MfRandom_Mix(x);
    return x;
}

u16 MfRandom_Modulo(u32 seed, enum MfRandomCategory category, u32 inputId, u16 locationKey, u16 bound)
{
    u32 h;
    u32 limit;
    u8 i;

    if (bound <= 1)
        return 0;

    // ME RandomSeededModulo-style rejection: avoid low-bit bias when bound is not
    // a power of two. Advance deterministically via Mix — never Random().
    limit = 0xFFFFFFFFu - (0xFFFFFFFFu % (u32)bound);
    h = MfRandom_Hash(seed, category, inputId, locationKey);
    i = 0;
    while (h >= limit && i < 4)
    {
        i++;
        h = MfRandom_Mix(h ^ (0x9E3779B9u * (u32)i));
    }

    return (u16)(h % bound);
}

u16 MfRandom_LocationKey(enum MfRandomCategory category, u16 mapsec, bool8 mapBasedEnabled)
{
    if (!mapBasedEnabled)
        return 0;

    // ME: only wild + trainer set mapBased from the rule; moves/ability/evo/static
    // keep locationKey 0 so an area's species remap does not reshuffle learnsets.
    switch (category)
    {
    case MF_RANDOM_CAT_WILD:
    case MF_RANDOM_CAT_TRAINER:
        return mapsec;
    default:
        return 0;
    }
}

u32 MfRandom_GenerateNewSeed(void)
{
    u32 seed;

    // 0 means "unset" in ModernRules; never persist it as a live seed.
    do
        seed = Random32();
    while (seed == 0);

    return seed;
}

void MfRandom_EnsureSeed(struct ModernRules *rules)
{
    if (rules == NULL)
        return;

    if (rules->randomizerSeed == 0)
        rules->randomizerSeed = MfRandom_GenerateNewSeed();
}

u32 MfRandom_HashActive(enum MfRandomCategory category, u32 inputId, u16 mapsec)
{
    const struct ModernRules *rules = MfRules_GetActiveRules();
    u16 locationKey = MfRandom_LocationKey(category, mapsec, rules->randomMapBased);

    return MfRandom_Hash(rules->randomizerSeed, category, inputId, locationKey);
}

u16 MfRandom_ModuloActive(enum MfRandomCategory category, u32 inputId, u16 mapsec, u16 bound)
{
    const struct ModernRules *rules = MfRules_GetActiveRules();
    u16 locationKey = MfRandom_LocationKey(category, mapsec, rules->randomMapBased);

    return MfRandom_Modulo(rules->randomizerSeed, category, inputId, locationKey, bound);
}
