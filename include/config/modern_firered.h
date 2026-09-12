#ifndef GUARD_CONFIG_MODERN_FIRERED_H
#define GUARD_CONFIG_MODERN_FIRERED_H

// Modern FireRed compile-time knobs. All MF_* configs live here — do not add
// Modern FireRed toggles to upstream include/config/*.h headers.
//
// Naming:
//   MF_*     — compile-time configs in this file
//   mf_*     — runtime sources (src/mf_*.c) and headers (include/mf_*.h)
//
// Master switches below gate whole unfinished subsystems. Player-facing options
// still resolve at runtime through the rules accessors (see ADR 0003); these
// are compile-out kills, not substitutes for save-backed rules.
// Override any switch at build time without wiping Makefile CPPFLAGS, e.g.:
//   make firered MF_CPPFLAGS='-DMF_RULES_ENGINE=0'

// Bump when this header's contract changes in a way callers must notice.
#define MF_VERSION 1

#ifndef MF_RULES_ENGINE
#define MF_RULES_ENGINE              TRUE    // Save-backed rules engine + start-of-run menu
#endif

#ifndef MF_RANDOMIZER
#define MF_RANDOMIZER                TRUE    // Seeded randomizer subsystem
#endif

#ifndef MF_NUZLOCKE
#define MF_NUZLOCKE                  TRUE    // Nuzlocke encounter / faint handling
#endif

#ifndef MF_OPTIONS_PLUS
#define MF_OPTIONS_PLUS              TRUE    // Options+ QoL menu and settings
#endif

#endif // GUARD_CONFIG_MODERN_FIRERED_H
