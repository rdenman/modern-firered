#ifndef GUARD_MF_TM_RANDOM_H
#define GUARD_MF_TM_RANDOM_H

#include "global.h"
#include "item.h"
#include "move.h"

#define MF_TM_RANDOM_TM_COUNT     50
#define MF_TM_RANDOM_TUTOR_COUNT  15
#define MF_TM_RANDOM_CAPE_COUNT   3
#define MF_TM_RANDOM_SLOT_COUNT   (MF_TM_RANDOM_TM_COUNT + MF_TM_RANDOM_TUTOR_COUNT + MF_TM_RANDOM_CAPE_COUNT)

// Vanilla TM/HM move, except TM01–50 dest when TMs randomizer is on (HMs never remap).
enum Move MfGetTmHmTeachMove(enum Item itemId);

// Tutor / Cape Brink: dest for a vanilla teach move. Identity if unknown or TMs off.
enum Move MfMapTutorTeachMove(enum Move sourceMove);
enum Move MfTutorSourceMove(void);
enum Move MfTutorLearnCheckMove(enum Move destMove);
enum Move MfTmLearnCheckMove(enum Item itemId, enum Move destMove);

// VAR_0x8005 = vanilla tutor move → dest. STR_VAR_1 = dest name. Always buffers.
void MfScriptRemapTutorMove(void);
enum Move MfApplyTutorRemap(enum Move sourceMove);

void MfTmRandom_DebugDump(void);

#endif // GUARD_MF_TM_RANDOM_H
