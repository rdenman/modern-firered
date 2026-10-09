#include "global.h"
#include "mf_random.h"
#include "mf_rules.h"
#include "mf_types.h"
#include "pokemon.h"
#include "battle_main.h"

#include "data/mf_type_effectiveness_improved.h"

#define MF_RANDOM_CHART_FLAG_IMPROVED  0x01
#define MF_RANDOM_CHART_FLAG_READY     0x80

// Pre-Gen-6 typings for species that gained Fairy via P_UPDATED_TYPES >= GEN_6.
// Spec: ME GetTypeBySpecies when tx_Mode_Fairy_Types == 0 (types_old). Cotonee /
// Whimsicott are the same official Gen-6 retype class and are included for
// TESTING builds that keep Gen 5 families. Pure Fairy introductions (Sylveon,
// Alolan Ninetales, megas that were always Fairy) are intentionally left alone.

struct MfTypePair
{
    u16 species;
    u8 types[2];
};

static const struct MfTypePair sFairyTypeFallbacks[] =
{
    { SPECIES_CLEFFA,     { TYPE_NORMAL,  TYPE_NORMAL  } },
    { SPECIES_CLEFAIRY,   { TYPE_NORMAL,  TYPE_NORMAL  } },
    { SPECIES_CLEFABLE,   { TYPE_NORMAL,  TYPE_NORMAL  } },
    { SPECIES_IGGLYBUFF,  { TYPE_NORMAL,  TYPE_NORMAL  } },
    { SPECIES_JIGGLYPUFF, { TYPE_NORMAL,  TYPE_NORMAL  } },
    { SPECIES_WIGGLYTUFF, { TYPE_NORMAL,  TYPE_NORMAL  } },
    { SPECIES_TOGEPI,     { TYPE_NORMAL,  TYPE_NORMAL  } },
    { SPECIES_TOGETIC,    { TYPE_NORMAL,  TYPE_FLYING  } },
    { SPECIES_TOGEKISS,   { TYPE_NORMAL,  TYPE_FLYING  } },
    { SPECIES_AZURILL,    { TYPE_NORMAL,  TYPE_NORMAL  } },
    { SPECIES_MARILL,     { TYPE_WATER,   TYPE_WATER   } },
    { SPECIES_AZUMARILL,  { TYPE_WATER,   TYPE_WATER   } },
    { SPECIES_SNUBBULL,   { TYPE_NORMAL,  TYPE_NORMAL  } },
    { SPECIES_GRANBULL,   { TYPE_NORMAL,  TYPE_NORMAL  } },
    { SPECIES_MIME_JR,    { TYPE_PSYCHIC, TYPE_PSYCHIC } },
    { SPECIES_MR_MIME,    { TYPE_PSYCHIC, TYPE_PSYCHIC } },
    { SPECIES_RALTS,      { TYPE_PSYCHIC, TYPE_PSYCHIC } },
    { SPECIES_KIRLIA,     { TYPE_PSYCHIC, TYPE_PSYCHIC } },
    { SPECIES_GARDEVOIR,  { TYPE_PSYCHIC, TYPE_PSYCHIC } },
    { SPECIES_MAWILE,     { TYPE_STEEL,   TYPE_STEEL   } },
    { SPECIES_COTTONEE,   { TYPE_GRASS,   TYPE_GRASS   } },
    { SPECIES_WHIMSICOTT, { TYPE_GRASS,   TYPE_GRASS   } },
};

// ME modern typings (tx_Mode_Modern_Types). Expansion keeps vanilla / Fairy-on
// types in gSpeciesInfo; when modernTypes is on we overlay ME's balance retypes.
// Snubbull/Granbull: ME uses types_new (Fairy/Normal) when modern is on, else
// Fairy/Fairy from species_info when Fairy is on.
static const struct MfTypePair sModernTypeOverlays[] =
{
    { SPECIES_ARBOK,      { TYPE_POISON,   TYPE_DARK     } },
    { SPECIES_PARASECT,   { TYPE_BUG,      TYPE_GHOST    } },
    { SPECIES_GOLDUCK,    { TYPE_WATER,    TYPE_PSYCHIC  } },
    { SPECIES_KINGLER,    { TYPE_WATER,    TYPE_STEEL    } },
    { SPECIES_MEGANIUM,   { TYPE_GRASS,    TYPE_FAIRY    } },
    { SPECIES_TYPHLOSION, { TYPE_FIRE,     TYPE_GROUND   } },
    { SPECIES_FERALIGATR, { TYPE_WATER,    TYPE_DRAGON   } },
    { SPECIES_NOCTOWL,    { TYPE_PSYCHIC,  TYPE_FLYING   } },
    { SPECIES_SUNFLORA,   { TYPE_GRASS,    TYPE_FIRE     } },
    { SPECIES_STANTLER,   { TYPE_NORMAL,   TYPE_PSYCHIC  } },
    { SPECIES_SNUBBULL,   { TYPE_FAIRY,    TYPE_NORMAL   } },
    { SPECIES_GRANBULL,   { TYPE_FAIRY,    TYPE_NORMAL   } },
    { SPECIES_GROVYLE,    { TYPE_GRASS,    TYPE_DRAGON   } },
    { SPECIES_SCEPTILE,   { TYPE_GRASS,    TYPE_DRAGON   } },
    { SPECIES_MASQUERAIN, { TYPE_BUG,      TYPE_WATER    } },
    { SPECIES_DELCATTY,   { TYPE_NORMAL,   TYPE_FAIRY    } },
    { SPECIES_GULPIN,     { TYPE_POISON,   TYPE_NORMAL   } },
    { SPECIES_SWALOT,     { TYPE_POISON,   TYPE_NORMAL   } },
    { SPECIES_LUVDISC,    { TYPE_WATER,    TYPE_FAIRY    } },
    { SPECIES_HUNTAIL,    { TYPE_WATER,    TYPE_DARK     } },
    { SPECIES_GOREBYSS,   { TYPE_WATER,    TYPE_PSYCHIC  } },
    { SPECIES_ELECTIVIRE, { TYPE_ELECTRIC, TYPE_FIGHTING } },
    { SPECIES_YANMEGA,    { TYPE_BUG,      TYPE_DRAGON   } },
};

static const u8 *FindTypePair(const struct MfTypePair *table, u32 count, enum Species species)
{
    u32 i;

    for (i = 0; i < count; i++)
    {
        if (table[i].species == species)
            return table[i].types;
    }
    return NULL;
}

bool8 MfTypeIsCombatType(enum Type type, bool8 includeFairy)
{
    if (type == TYPE_NONE || type == TYPE_MYSTERY || type == TYPE_STELLAR)
        return FALSE;
    if (type >= NUMBER_OF_MON_TYPES)
        return FALSE;
    if (type == TYPE_FAIRY && !includeFairy)
        return FALSE;
    return TRUE;
}

u8 MfFillCombatTypes(enum Type *out, bool8 includeFairy)
{
    u8 count = 0;
    u8 type;

    if (out == NULL)
        return 0;

    for (type = 0; type < NUMBER_OF_MON_TYPES; type++)
    {
        if (MfTypeIsCombatType(type, includeFairy))
            out[count++] = type;
    }
    return count;
}

static enum Type RemapSpeciesType(enum Species species, u8 slot, enum Type base)
{
#if MF_RANDOMIZER
    const struct ModernRules *rules = MfRules_GetActiveRules();
    enum Type pool[NUMBER_OF_MON_TYPES];
    u8 count;
    u32 inputId;

    if (!rules->randomType)
        return base;
    if (!MfTypeIsCombatType(base, TRUE))
        return base;

    count = MfFillCombatTypes(pool, rules->fairyTypes);
    if (count == 0)
        return base;

    // ME: RandomSeededModulo(type + typeNum + species, pool). Slot is 1-based.
    inputId = ((u32)species << 16) | ((u32)base << 8) | (u32)(slot + 1);
    return pool[MfRandom_Modulo(rules->randomizerSeed, MF_RANDOM_CAT_TYPE, inputId, 0, count)];
#else
    (void)species;
    (void)slot;
    return base;
#endif
}

enum Type MfGetSpeciesType(enum Species species, u8 slot)
{
    enum Species sanitized = SanitizeSpeciesId(species);
    const u8 *types;
    enum Type base;

    if (slot > 1)
        slot = 1;

    // Fairy-off first (ME order): official Gen-6 Fairy retypes revert even when
    // modernTypes would otherwise assign Fairy (e.g. Meganium is modern-only Fairy).
    if (!MfRules_HasFairyTypes())
    {
        types = FindTypePair(sFairyTypeFallbacks, ARRAY_COUNT(sFairyTypeFallbacks), sanitized);
        if (types != NULL)
            return RemapSpeciesType(sanitized, slot, types[slot]);
    }

    if (MfRules_HasModernTypes())
    {
        types = FindTypePair(sModernTypeOverlays, ARRAY_COUNT(sModernTypeOverlays), sanitized);
        if (types != NULL)
            return RemapSpeciesType(sanitized, slot, types[slot]);
    }

    base = gSpeciesInfo[sanitized].types[slot];
    return RemapSpeciesType(sanitized, slot, base);
}

#if MF_RANDOMIZER
static EWRAM_DATA uq4_12_t sRandomTypeChart[NUMBER_OF_MON_TYPES][NUMBER_OF_MON_TYPES] = {0};
static EWRAM_DATA u32 sRandomChartSeed = 0;
static EWRAM_DATA u8 sRandomChartFlags = 0;

static const uq4_12_t (*BaseTypeEffectivenessTable(void))[NUMBER_OF_MON_TYPES]
{
    if (MfRules_HasTypeEffectiveness())
        return sMfTypeEffectivenessImproved;
    return gTypeEffectivenessTable;
}

static void ShuffleCombatTypes(enum Type *list, u8 count, u32 seed)
{
    s32 i;

    for (i = (s32)count - 1; i > 0; i--)
    {
        u16 j = MfRandom_Modulo(seed, MF_RANDOM_CAT_TYPE_EFF, (u32)i, 0, (u16)(i + 1));
        enum Type tmp = list[i];
        list[i] = list[j];
        list[j] = tmp;
    }
}

static const uq4_12_t (*EnsureRandomTypeChart(void))[NUMBER_OF_MON_TYPES]
{
    const struct ModernRules *rules = MfRules_GetActiveRules();
    const uq4_12_t (*base)[NUMBER_OF_MON_TYPES];
    enum Type pool[NUMBER_OF_MON_TYPES];
    u8 inv[NUMBER_OF_MON_TYPES];
    u8 count;
    u8 flags;
    u8 atk;
    u8 def;
    u8 i;

    flags = MF_RANDOM_CHART_FLAG_READY;
    if (rules->typeEffectiveness)
        flags |= MF_RANDOM_CHART_FLAG_IMPROVED;

    if (sRandomChartFlags == flags && sRandomChartSeed == rules->randomizerSeed)
        return sRandomTypeChart;

    base = BaseTypeEffectivenessTable();
    // ME shuffles every real type including Fairy; Fairy-off only gates species remaps.
    count = MfFillCombatTypes(pool, TRUE);

    for (i = 0; i < NUMBER_OF_MON_TYPES; i++)
        inv[i] = i;

    // Identity combat list in enum order; shuffled[k] is the new label for the
    // k-th combat type. Inverse maps that label back onto the static chart.
    {
        enum Type shuffled[NUMBER_OF_MON_TYPES];

        memcpy(shuffled, pool, count * sizeof(enum Type));
        ShuffleCombatTypes(shuffled, count, rules->randomizerSeed);
        for (i = 0; i < count; i++)
            inv[shuffled[i]] = pool[i];
    }

    for (atk = 0; atk < NUMBER_OF_MON_TYPES; atk++)
    {
        for (def = 0; def < NUMBER_OF_MON_TYPES; def++)
            sRandomTypeChart[atk][def] = base[inv[atk]][inv[def]];
    }

    sRandomChartSeed = rules->randomizerSeed;
    sRandomChartFlags = flags;
    return sRandomTypeChart;
}
#endif // MF_RANDOMIZER

const uq4_12_t (*MfGetTypeEffectivenessTable(void))[NUMBER_OF_MON_TYPES]
{
#if MF_RANDOMIZER
    if (MfRules_GetActiveRules()->randomTypeEffectiveness)
        return EnsureRandomTypeChart();
#endif
    if (MfRules_HasTypeEffectiveness())
        return sMfTypeEffectivenessImproved;
    return gTypeEffectivenessTable;
}
