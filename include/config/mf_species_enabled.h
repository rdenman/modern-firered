#ifndef GUARD_CONFIG_MF_SPECIES_ENABLED_H
#define GUARD_CONFIG_MF_SPECIES_ENABLED_H

// Modern FireRed species-range policy (S08 / ADR 0008).
// Included immediately after upstream species_enabled.h so family macros that
// expand P_GEN_* / form switches pick up these values. Do not edit upstream
// P_GEN_* defaults in species_enabled.h — override here instead.
//
// Intent: Kanto-focused data (Gen 1–3 families + cross-gen Kanto-line
// evolutions/forms). Not a full National Dex / Gen 9 dump (see PROJECT.md).
// Full reference: docs-mf/SPECIES_BASELINE.md
//
// TESTING builds skip the P_* overrides so Emerald TESTELF keeps expansion’s
// full species set for upstream coverage (test.h also force-enables families).
// The FireRed ROM (`make firered`, TESTING=0) is the policy surface.

// Policy stamp for tests / docs (always defined).
#define MF_SPECIES_FAMILIES_MAX_GEN      3
#define MF_SPECIES_CROSS_EVOS            TRUE
#define MF_SPECIES_REGIONAL_FORMS        TRUE
#define MF_SPECIES_MEGA_EVOLUTIONS       TRUE
#define MF_SPECIES_PRIMAL_REVERSIONS     TRUE
#define MF_SPECIES_ULTRA_BURST_FORMS     FALSE
#define MF_SPECIES_GIGANTAMAX_FORMS      FALSE
#define MF_SPECIES_TERA_FORMS            FALSE
#define MF_SPECIES_FUSION_FORMS          FALSE // Kyurem / Necrozma / Calyrex — Gen 5/7/8 families
#define MF_SPECIES_PIKACHU_EXTRA_FORMS   FALSE

#if !TESTING

// --- Generation families -------------------------------------------------
#undef P_GEN_1_POKEMON
#define P_GEN_1_POKEMON                  TRUE
#undef P_GEN_2_POKEMON
#define P_GEN_2_POKEMON                  TRUE
#undef P_GEN_3_POKEMON
#define P_GEN_3_POKEMON                  TRUE
#undef P_GEN_4_POKEMON
#define P_GEN_4_POKEMON                  FALSE
#undef P_GEN_5_POKEMON
#define P_GEN_5_POKEMON                  FALSE
#undef P_GEN_6_POKEMON
#define P_GEN_6_POKEMON                  FALSE
#undef P_GEN_7_POKEMON
#define P_GEN_7_POKEMON                  FALSE
#undef P_GEN_8_POKEMON
#define P_GEN_8_POKEMON                  FALSE
#undef P_GEN_9_POKEMON
#define P_GEN_9_POKEMON                  FALSE

// Regional dex lists Magnezone / Sylveon / Annihilape etc. with Kanto mons.
#undef P_NEW_EVOS_IN_REGIONAL_DEX
#define P_NEW_EVOS_IN_REGIONAL_DEX       TRUE

// --- Cross-gen evolutions (Kanto / enabled-family lines) -----------------
#undef P_CROSS_GENERATION_EVOS
#define P_CROSS_GENERATION_EVOS          MF_SPECIES_CROSS_EVOS
#undef P_GEN_2_CROSS_EVOS
#define P_GEN_2_CROSS_EVOS               P_CROSS_GENERATION_EVOS
#undef P_GEN_3_CROSS_EVOS
#define P_GEN_3_CROSS_EVOS               P_CROSS_GENERATION_EVOS
#undef P_GEN_4_CROSS_EVOS
#define P_GEN_4_CROSS_EVOS               P_CROSS_GENERATION_EVOS
#undef P_GEN_6_CROSS_EVOS
#define P_GEN_6_CROSS_EVOS               P_CROSS_GENERATION_EVOS
#undef P_GEN_8_CROSS_EVOS
#define P_GEN_8_CROSS_EVOS               P_CROSS_GENERATION_EVOS
#undef P_GEN_9_CROSS_EVOS
#define P_GEN_9_CROSS_EVOS               P_CROSS_GENERATION_EVOS

// --- Regional forms of enabled families (e.g. Alolan Rattata) ------------
#undef P_REGIONAL_FORMS
#define P_REGIONAL_FORMS                 MF_SPECIES_REGIONAL_FORMS
#undef P_ALOLAN_FORMS
#define P_ALOLAN_FORMS                   P_REGIONAL_FORMS
#undef P_GALARIAN_FORMS
#define P_GALARIAN_FORMS                 P_REGIONAL_FORMS
#undef P_HISUIAN_FORMS
#define P_HISUIAN_FORMS                  P_REGIONAL_FORMS
#undef P_PALDEAN_FORMS
#define P_PALDEAN_FORMS                  P_REGIONAL_FORMS

// --- Battle gimmick forms ------------------------------------------------
#undef P_MEGA_EVOLUTIONS
#define P_MEGA_EVOLUTIONS                MF_SPECIES_MEGA_EVOLUTIONS
#undef P_PRIMAL_REVERSIONS
#define P_PRIMAL_REVERSIONS              MF_SPECIES_PRIMAL_REVERSIONS
#undef P_ULTRA_BURST_FORMS
#define P_ULTRA_BURST_FORMS              MF_SPECIES_ULTRA_BURST_FORMS
#undef P_GIGANTAMAX_FORMS
#define P_GIGANTAMAX_FORMS               MF_SPECIES_GIGANTAMAX_FORMS
#undef P_TERA_FORMS
#define P_TERA_FORMS                     MF_SPECIES_TERA_FORMS
#undef P_FUSION_FORMS
#define P_FUSION_FORMS                   MF_SPECIES_FUSION_FORMS
#undef P_GEN_9_MEGA_EVOLUTIONS
#define P_GEN_9_MEGA_EVOLUTIONS          FALSE // ZA megas; Gen 9 families off

// Cosplay / Cap Pikachu — not used in Kanto progression.
#undef P_PIKACHU_EXTRA_FORMS
#define P_PIKACHU_EXTRA_FORMS            MF_SPECIES_PIKACHU_EXTRA_FORMS
#undef P_COSPLAY_PIKACHU_FORMS
#define P_COSPLAY_PIKACHU_FORMS          P_PIKACHU_EXTRA_FORMS
#undef P_CAP_PIKACHU_FORMS
#define P_CAP_PIKACHU_FORMS              P_PIKACHU_EXTRA_FORMS

#endif // !TESTING

#endif // GUARD_CONFIG_MF_SPECIES_ENABLED_H
