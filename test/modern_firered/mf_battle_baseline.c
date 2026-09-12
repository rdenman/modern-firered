#include "global.h"
#include "test/battle.h"
#include "test/test.h"

// S07: lock Phase 1 battle baseline (phys/spec, Fairy, updated types).
// Shared Emerald TESTELF path — same configs/data as the FireRed ROM for these macros.

TEST("MF: battle baseline configs are GEN_LATEST modern")
{
    EXPECT(GEN_LATEST == GEN_9);
    EXPECT(B_PHYSICAL_SPECIAL_SPLIT >= GEN_4);
    EXPECT(P_UPDATED_TYPES >= GEN_6);
    EXPECT(B_UPDATED_TYPE_MATCHUPS >= GEN_6);
    EXPECT(B_UPDATED_MOVE_TYPES >= GEN_2);
}

TEST("MF: Kanto species use updated / Fairy typings")
{
    EXPECT_EQ(GetSpeciesType(SPECIES_CLEFAIRY, 0), TYPE_FAIRY);
    EXPECT_EQ(GetSpeciesType(SPECIES_CLEFAIRY, 1), TYPE_FAIRY);
    EXPECT_EQ(GetSpeciesType(SPECIES_CLEFABLE, 0), TYPE_FAIRY);
    EXPECT_EQ(GetSpeciesType(SPECIES_JIGGLYPUFF, 0), TYPE_NORMAL);
    EXPECT_EQ(GetSpeciesType(SPECIES_JIGGLYPUFF, 1), TYPE_FAIRY);
    EXPECT_EQ(GetSpeciesType(SPECIES_MR_MIME, 0), TYPE_PSYCHIC);
    EXPECT_EQ(GetSpeciesType(SPECIES_MR_MIME, 1), TYPE_FAIRY);
    EXPECT_EQ(GetSpeciesType(SPECIES_MAGNEMITE, 0), TYPE_ELECTRIC);
    EXPECT_EQ(GetSpeciesType(SPECIES_MAGNEMITE, 1), TYPE_STEEL);
}

TEST("MF: split move categories and types for Kanto-relevant moves")
{
    EXPECT_EQ(GetMoveType(MOVE_BITE), TYPE_DARK);
    EXPECT_EQ(GetMoveCategory(MOVE_BITE), DAMAGE_CATEGORY_PHYSICAL);
    EXPECT_EQ(GetMoveType(MOVE_CRUNCH), TYPE_DARK);
    EXPECT_EQ(GetMoveCategory(MOVE_CRUNCH), DAMAGE_CATEGORY_PHYSICAL);
    EXPECT_EQ(GetMoveType(MOVE_SHADOW_BALL), TYPE_GHOST);
    EXPECT_EQ(GetMoveCategory(MOVE_SHADOW_BALL), DAMAGE_CATEGORY_SPECIAL);
    // Pre-split, Dark is special — Bite would follow the type. Split uses move category.
    EXPECT_EQ((u32)gTypesInfo[TYPE_DARK].damageCategory, (u32)DAMAGE_CATEGORY_SPECIAL);
    EXPECT(GetMoveCategory(MOVE_BITE) != (enum DamageCategory)gTypesInfo[TYPE_DARK].damageCategory);
}

TEST("MF: Fairy type chart matchups")
{
    EXPECT_EQ(gTypeEffectivenessTable[TYPE_DRAGON][TYPE_FAIRY], UQ_4_12(0.0));
    EXPECT_EQ(gTypeEffectivenessTable[TYPE_FAIRY][TYPE_DRAGON], UQ_4_12(2.0));
    EXPECT_EQ(gTypeEffectivenessTable[TYPE_POISON][TYPE_FAIRY], UQ_4_12(2.0));
    EXPECT_EQ(gTypeEffectivenessTable[TYPE_STEEL][TYPE_FAIRY], UQ_4_12(2.0));
    EXPECT_EQ(gTypeEffectivenessTable[TYPE_FAIRY][TYPE_STEEL], UQ_4_12(0.5));
    EXPECT_EQ(gTypeEffectivenessTable[TYPE_DRAGON][TYPE_DRAGON], UQ_4_12(2.0));
}

SINGLE_BATTLE_TEST("MF: Fairy typing blocks Dragon damage on Clefairy")
{
    GIVEN {
        ASSUME(GetSpeciesType(SPECIES_CLEFAIRY, 0) == TYPE_FAIRY);
        ASSUME(GetMoveType(MOVE_DRAGON_CLAW) == TYPE_DRAGON);
        ASSUME(gTypeEffectivenessTable[TYPE_DRAGON][TYPE_FAIRY] == UQ_4_12(0.0));
        PLAYER(SPECIES_CLEFAIRY);
        OPPONENT(SPECIES_DRAGONITE);
    } WHEN {
        TURN { MOVE(opponent, MOVE_DRAGON_CLAW); }
    } SCENE {
        NOT ANIMATION(ANIM_TYPE_MOVE, MOVE_DRAGON_CLAW, opponent);
        MESSAGE("It doesn't affect Clefairy…");
    }
}

SINGLE_BATTLE_TEST("MF: Fairy move is super effective against Dragonite")
{
    GIVEN {
        ASSUME(GetMoveType(MOVE_MOONBLAST) == TYPE_FAIRY);
        ASSUME(GetSpeciesType(SPECIES_DRAGONITE, 0) == TYPE_DRAGON);
        ASSUME(gTypeEffectivenessTable[TYPE_FAIRY][TYPE_DRAGON] == UQ_4_12(2.0));
        PLAYER(SPECIES_CLEFABLE);
        OPPONENT(SPECIES_DRAGONITE);
    } WHEN {
        TURN { MOVE(player, MOVE_MOONBLAST); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_MOONBLAST, player);
        HP_BAR(opponent);
        MESSAGE("It's super effective!");
    }
}

SINGLE_BATTLE_TEST("MF: Bite (Dark) uses physical Attack under the split", s16 damage)
{
    enum Item item;

    PARAMETRIZE { item = ITEM_NONE; }
    PARAMETRIZE { item = ITEM_CHOICE_BAND; }

    GIVEN {
        ASSUME(B_PHYSICAL_SPECIAL_SPLIT >= GEN_4);
        ASSUME(GetMoveType(MOVE_BITE) == TYPE_DARK);
        ASSUME(GetMoveCategory(MOVE_BITE) == DAMAGE_CATEGORY_PHYSICAL);
        ASSUME(GetItemHoldEffect(ITEM_CHOICE_BAND) == HOLD_EFFECT_CHOICE_BAND);
        PLAYER(SPECIES_WOBBUFFET) { Item(item); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_BITE); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_BITE, player);
        HP_BAR(opponent, captureDamage: &results[i].damage);
    } FINALLY {
        // Choice Band only boosts physical damage — proves Bite is not type-based Special.
        EXPECT_MUL_EQ(results[0].damage, Q_4_12(1.5), results[1].damage);
    }
}
