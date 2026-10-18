@ Matches enum MfOptionJoy in include/mf_options.h
.set MF_JOY_FAST, 1
.set MF_JOY_ON, 2

EventScript_AfterWhiteOutHeal_Frlg::
	lockall
	textcolor NPC_TEXT_COLOR_FEMALE
	msgbox gText_FirstShouldRestoreMonsHealth
	call EventScript_PkmnCenterNurse_TakeAndHealPkmn_Frlg
	call_if_unset FLAG_DEFEATED_BROCK, EventScript_AfterWhiteOutHealMsgPreFirstBoss
	call_if_set FLAG_DEFEATED_BROCK, EventScript_AfterWhiteOutHealMsg
	applymovement VAR_LAST_TALKED, Movement_Bow
	waitmovement 0
	fadedefaultbgm
	releaseall
	end

EventScript_PkmnCenterNurse_Frlg::
	specialvar VAR_RESULT, MfIsPokecenterChallengeActivated
	goto_if_eq VAR_RESULT, TRUE, EventScript_PkmnCenterNurse_ChallengeBlocked_Frlg
	specialvar VAR_RESULT, MfOptions_ScriptGetEvenFasterJoy
	goto_if_eq VAR_RESULT, MF_JOY_ON, EventScript_PkmnCenterNurse_HealInstant_Frlg
	goto EventScript_PkmnCenterNurse_OfferHeal_Frlg

EventScript_PkmnCenterNurse_ChallengeBlocked_Frlg::
	msgbox Text_PkmnCenterChallenge_NoHealing_Frlg
	return

EventScript_PkmnCenterNurse_HealInstant_Frlg::
	incrementgamestat GAME_STAT_USED_POKECENTER
	special HealPlayerParty
	waitse
	playse SE_M_MORNING_SUN
	applymovement VAR_LAST_TALKED, Movement_BowInstant
	applymovement OBJ_EVENT_ID_PLAYER, Movement_PlayerSpinFaceDown
	waitmovement 0
	return

EventScript_PkmnCenterNurse_OfferHeal_Frlg::
	message Text_WelcomeWantToHealPkmn_Frlg
	waitmessage
	multichoice 19, 8, MULTI_YESNO, FALSE | (TRUE << 1)
	switch VAR_RESULT
	case 0, EventScript_PkmnCenterNurse_HealPkmn_Frlg
	case 1 EventScript_PkmnCenterNurse_Goodbye_Frlg
	case MULTI_B_PRESSED, EventScript_PkmnCenterNurse_Goodbye_Frlg
	end

EventScript_FieldHealBlocked_Frlg::
	msgbox Text_PkmnCenterChallenge_NoHealing_Frlg
	releaseall
	end

EventScript_PkmnCenterNurse_HealPkmn_Frlg::
	incrementgamestat GAME_STAT_USED_POKECENTER
	specialvar VAR_RESULT, MfOptions_ScriptGetEvenFasterJoy
	goto_if_eq VAR_RESULT, MF_JOY_FAST, EventScript_PkmnCenterNurse_HealPkmnFast_Frlg
	message Text_TakeYourPkmnForFewSeconds_Frlg
	waitmessage
	call EventScript_PkmnCenterNurse_TakeAndHealPkmn_Frlg
	goto EventScript_PkmnCenterNurse_CheckTrainerTowerAndUnionRoom_Frlg
	end

EventScript_PkmnCenterNurse_HealPkmnFast_Frlg::
	call EventScript_PkmnCenterNurse_TakeAndHealPkmn_Frlg
	applymovement VAR_LAST_TALKED, Movement_Bow
	waitmovement 0
	msgbox Text_WeHopeToSeeYouAgain_Frlg
	return

EventScript_PkmnCenterNurse_TakeAndHealPkmn_Frlg::
	specialvar VAR_RESULT, MfOptions_ScriptGetEvenFasterJoy
	goto_if_eq VAR_RESULT, MF_JOY_ON, EventScript_PkmnCenterNurse_TakeAndHealInstant_Frlg
	applymovement VAR_LAST_TALKED, Common_Movement_WalkInPlaceFasterLeft
	waitmovement 0
	dofieldeffect FLDEFF_POKECENTER_HEAL
	waitfieldeffect FLDEFF_POKECENTER_HEAL
	applymovement VAR_LAST_TALKED, Common_Movement_WalkInPlaceFasterDown
	waitmovement 0
	special HealPlayerParty
	return

EventScript_PkmnCenterNurse_TakeAndHealInstant_Frlg::
	special HealPlayerParty
	return

EventScript_PkmnCenterNurse_CheckTrainerTowerAndUnionRoom_Frlg::
	specialvar VAR_RESULT, IsPlayerNotInTrainerTowerLobby
	goto_if_eq VAR_RESULT, FALSE, EventScript_PkmnCenterNurse_ReturnPkmn_Frlg
	specialvar VAR_RESULT, BufferUnionRoomPlayerName
	copyvar VAR_0x8008, VAR_RESULT
	goto_if_eq VAR_0x8008, 0, EventScript_PkmnCenterNurse_ReturnPkmn_Frlg
	goto_if_eq VAR_0x8008, 1, EventScript_PkmnCenterNurse_PlayerWaitingInUnionRoom_Frlg
	end

EventScript_PkmnCenterNurse_ReturnPkmn_Frlg::
	message Text_RestoredPkmnToFullHealth_Frlg
	waitmessage
	applymovement VAR_LAST_TALKED, Movement_Bow
	waitmovement 0
	msgbox Text_WeHopeToSeeYouAgain_Frlg
	return

EventScript_PkmnCenterNurse_PlayerWaitingInUnionRoom_Frlg::
	goto_if_set FLAG_NURSE_UNION_ROOM_REMINDER, EventScript_PkmnCenterNurse_ReturnPkmn_Frlg
	msgbox Text_RestoredPkmnToFullHealth_Frlg
	setflag FLAG_NURSE_UNION_ROOM_REMINDER
	message Text_PlayerIsPlayingRightNowGoForIt_Frlg
	waitmessage
	applymovement VAR_LAST_TALKED, Movement_Bow
	waitmovement 0
	msgbox Text_WeHopeToSeeYouAgain_Frlg
	return

EventScript_PkmnCenterNurse_Goodbye_Frlg::
	msgbox Text_WeHopeToSeeYouAgain_Frlg
	return

Movement_Bow::
	nurse_joy_bow
	delay_4
	step_end

Movement_BowInstant::
	nurse_joy_bow
	step_end

@ Facing Joy is up; spin to face the Center door (down).
Movement_PlayerSpinFaceDown::
	face_left
	face_up
	face_right
	face_down
	step_end

Text_WelcomeWantToHealPkmn_Frlg::
	.string "Welcome to our Pokémon Center!\p"
	.string "Would you like me to heal your\n"
	.string "Pokémon back to perfect health?$"

Text_TakeYourPkmnForFewSeconds_Frlg::
	.string "Okay, I'll take your Pokémon for a\n"
	.string "few seconds.$"

Text_WeHopeToSeeYouAgain_Frlg::
	.string "We hope to see you again!$"

Text_RestoredPkmnToFullHealth_Frlg::
	.string "Thank you for waiting.\n"
	.string "We've restored your Pokémon to\l"
	.string "full health.$"

Text_PkmnCenterChallenge_NoHealing_Frlg::
	.string "I'm sorry, but I can't treat your\n"
	.string "Pokémon on this challenge.\p"
	.string "Please recover with items instead.$"
