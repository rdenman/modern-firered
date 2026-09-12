#ifndef GUARD_MF_RULES_H
#define GUARD_MF_RULES_H

// Modern FireRed save-backed rules (S12). Field checklist mirrors ME's
// tx_randomizer_and_challenges options (Gamemode / Features / Randomizer /
// Nuzlocke / Difficulty / Challenges). Packed bitfields match ME's density;
// storage lives in SaveBlock3 (see ADR 0012).

#include "gba/types.h"
#include "constants/region_map_sections.h"

// Bump when the on-save layout of struct ModernRules changes (S64 migrates).
#define MF_RULES_VERSION 1

// Soft ceiling so Nuzlocke flags + seed never silently balloon SaveBlock3.
// Hard ceiling remains SAVE_BLOCK_3_CHUNK_SIZE * NUM_SECTORS_PER_SLOT (1624).
#define MF_RULES_MAX_BYTES 128

#define MF_NUZLOCKE_ENCOUNTER_FLAG_BYTES ((MAPSEC_COUNT + 7) / 8)

// Gamemode page preset (S14 fills Classic / Modern / Custom vectors).
enum MfGamemodePreset
{
    MF_GAMEMODE_CLASSIC = 0,
    MF_GAMEMODE_MODERN = 1,
    MF_GAMEMODE_CUSTOM = 2,
};

// Packed player rules for one save. Do not reorder fields after S12 ships —
// additive changes only, via MF_RULES_VERSION + S64.
struct ModernRules
{
    /*0x00*/ u16 version;
    /*0x02*/ u8 gamemodePreset:2; // MfGamemodePreset
    /*0x02*/ u8 rulesLocked:1;    // S15 mid-run lock
    /*0x02*/ u8 padding0:5;

    /*0x03*/ // --- Gamemode (ME tx_Mode_*) ---
             u8 infiniteTms:1;
             u8 survivePoison:1;
             u8 synchronize:1;
             u8 mints:1;
             u8 modernSitrus:1;       // ME tx_Mode_New_Citrus
             u8 modernTypes:1;
             u8 fairyTypes:1;
             u8 modernStats:1;

    /*0x04*/ u8 sturdy:1;
             u8 modernMoves:1;
             u8 legendaryAbilities:1;
             u8 newLegendaries:1;     // ME parity; FR map content still out of scope
             u8 typeEffectiveness:1;  // ME tx_Mode_TypeEffectiveness
             u8 alternateSpawns:2;    // ME tx_Mode_Encounters
             u8 paddingGamemode:1;

    /*0x05*/ // --- Features (ME tx_Features_*) ---
             u8 shinyChance:4;
             u8 wildItemDrops:1;
             u8 easierFeebas:1;       // Hoenn leftover; unused in FR
             u8 rtcType:1;
             u8 shinyColors:1;

    /*0x06*/ u8 wonderTrade:1;
             u8 unlimitedWonderTrade:1;
             u8 frontierBans:1;       // ME parity; FR has no Frontier
             u8 paddingFeatures:5;

    /*0x07*/ // --- Randomizer (ME tx_Random_*) ---
             u8 randomStarter:1;
             u8 randomWild:1;
             u8 randomTrainer:1;
             u8 randomStatic:1;
             u8 randomSimilar:1;
             u8 randomMapBased:1;
             u8 randomIncludeLegendaries:1;
             u8 randomType:1;

    /*0x08*/ u8 randomMoves:1;
             u8 randomAbilities:1;
             u8 randomEvolution:1;
             u8 randomEvolutionMethods:1;
             u8 randomTypeEffectiveness:1;
             u8 randomItems:1;
             u8 randomChaos:1;
             u8 paddingRandom:1;

    /*0x09*/ // --- Nuzlocke (ME tx_Challenges_Nuzlocke* / tx_Nuzlocke_*) ---
             u8 nuzlocke:1;
             u8 nuzlockeHardcore:1;
             u8 nuzlockeEasy:1;       // ME tx_Nuzlocke_EasyMode / mini
             u8 nuzlockeSpeciesClause:1;
             u8 nuzlockeShinyClause:1;
             u8 nuzlockeNicknaming:1;
             u8 nuzlockeDeletion:1;
             u8 paddingNuzlocke:1;

    /*0x0A*/ // --- Difficulty (ME tx_Challenges_* / tx_Difficulty_*) ---
             u8 partyLimit:3;         // 0 = 6; value is (6 - maxParty)
             u8 levelCap:2;
             u8 expMultiplier:2;
             u8 noItemPlayer:1;

    /*0x0B*/ u8 noItemTrainer:1;
             u8 noEvs:1;
             u8 scalingIvs:2;
             u8 scalingEvs:2;
             u8 maxPartyIvs:2;

    /*0x0C*/ u8 pokeCenterLimit:2;    // 0 unlimited, 1 none (ME naming)
             u8 lockDifficulty:1;     // ME tx_Features_LimitDifficulty
             u8 escapeRopeDig:1;
             u8 hardExp:1;
             u8 catchRate:3;

    /*0x0D*/ // --- Challenges (ME tx_Challenges_*) ---
             u8 evoLimit:2;
             u8 baseStatEqualizer:2;
             u8 lessEscapes:1;
             u8 mirror:1;
             u8 mirrorThief:1;
             u8 noPcHeal:1;

    /*0x0E*/ u8 monotype:5;           // ME OneTypeChallenge; 31 = off
             u8 expensiveShops:3;

    /*0x0F*/ u8 paddingTail;

    /*0x10*/ u32 randomizerSeed;

    /*0x14*/ u8 nuzlockeEncounterFlags[MF_NUZLOCKE_ENCOUNTER_FLAG_BYTES];
};

typedef char mf_rules_fits_in_budget[(sizeof(struct ModernRules) <= MF_RULES_MAX_BYTES) ? 1 : -1];
typedef char mf_nuzlocke_flags_cover_mapsecs[(MF_NUZLOCKE_ENCOUNTER_FLAG_BYTES * 8 >= MAPSEC_COUNT) ? 1 : -1];

#define MF_RULES_SERIALIZED_SIZE (sizeof(struct ModernRules))

void MfRules_ResetToEmpty(struct ModernRules *rules);
void MfRules_Pack(const struct ModernRules *rules, u8 *out);
void MfRules_Unpack(const u8 *in, struct ModernRules *rules);
struct ModernRules *MfRules_GetSaveRules(void);

#endif // GUARD_MF_RULES_H
