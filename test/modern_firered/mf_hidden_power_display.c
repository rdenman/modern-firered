#include "global.h"
#include "battle_main.h"
#include "config/pokemon.h"
#include "move.h"
#include "pokemon.h"
#include "test/test.h"
#include "constants/abilities.h"
#include "constants/battle.h"
#include "constants/hold_effects.h"
#include "constants/moves.h"
#include "constants/species.h"

static void SetIvs(struct Pokemon *mon, u32 hp, u32 atk, u32 def, u32 speed, u32 spAtk, u32 spDef)
{
    SetMonData(mon, MON_DATA_HP_IV, &hp);
    SetMonData(mon, MON_DATA_ATK_IV, &atk);
    SetMonData(mon, MON_DATA_DEF_IV, &def);
    SetMonData(mon, MON_DATA_SPEED_IV, &speed);
    SetMonData(mon, MON_DATA_SPATK_IV, &spAtk);
    SetMonData(mon, MON_DATA_SPDEF_IV, &spDef);
}

TEST("MF: Hidden Power TM type stays Normal; per-mon type uses IVs")
{
    struct Pokemon mon;

    EXPECT_EQ(GetMoveType(MOVE_HIDDEN_POWER), TYPE_NORMAL);
    EXPECT(!P_SHOW_DYNAMIC_TYPES);

    CreateMon(&mon, SPECIES_PIKACHU, 5, 0, OTID_STRUCT_PRESET(0));

    // All-odd IVs → last Hidden Power type (Dark).
    SetIvs(&mon, 31, 31, 31, 31, 31, 31);
    EXPECT_EQ(GetDynamicMoveType(&mon, MOVE_HIDDEN_POWER, 0, ABILITY_NONE, HOLD_EFFECT_NONE, MON_OUTSIDE_BATTLE), TYPE_DARK);

    // Smogon HP Fire (31/31/30/30/31/30).
    SetIvs(&mon, 31, 31, 30, 30, 30, 31);
    EXPECT_EQ(GetDynamicMoveType(&mon, MOVE_HIDDEN_POWER, 0, ABILITY_NONE, HOLD_EFFECT_NONE, MON_OUTSIDE_BATTLE), TYPE_FIRE);
}
