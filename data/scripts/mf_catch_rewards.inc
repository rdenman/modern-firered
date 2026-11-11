@ Catch Rewards rule (ADR 0119). Call after the Aide's GetAideRequestInfo.
@ In: VAR_0x8004 = MF_CATCH_REWARD_*, STR_VAR_2 = reward item name.
@ Out: VAR_RESULT FALSE = run the vanilla catch-count flow, TRUE = give the reward.
@ Ends the script itself when a badge/level requirement is not met.
MfAide_EventScript_CheckRequirement::
	specialvar VAR_RESULT, MfCatchReward_GetMode
	goto_if_eq VAR_RESULT, MF_CATCH_REWARDS_BADGES, MfAide_EventScript_CheckBadges
	goto_if_eq VAR_RESULT, MF_CATCH_REWARDS_LEVELS, MfAide_EventScript_CheckLevels
	goto_if_eq VAR_RESULT, MF_CATCH_REWARDS_WAIVED, MfAide_EventScript_Waived
	setvar VAR_RESULT, FALSE
	return

MfAide_EventScript_Waived::
	msgbox MfAide_Text_GiftOnlyWaived
	setvar VAR_RESULT, TRUE
	return

MfAide_EventScript_CheckBadges::
	specialvar VAR_0x8005, MfCatchReward_Check
	msgbox MfAide_Text_GiveItemIfEnoughBadges
	goto_if_ne VAR_0x8005, MF_CATCH_REWARD_MET, MfAide_EventScript_NotEnoughBadges
	msgbox MfAide_Text_EnoughBadges
	setvar VAR_RESULT, TRUE
	return

MfAide_EventScript_NotEnoughBadges::
	msgbox MfAide_Text_NotEnoughBadges
	release
	end

MfAide_EventScript_CheckLevels::
	specialvar VAR_0x8005, MfCatchReward_Check
	msgbox MfAide_Text_GiveItemIfHighEnoughLevel
	goto_if_ne VAR_0x8005, MF_CATCH_REWARD_MET, MfAide_EventScript_LevelTooLow
	msgbox MfAide_Text_HighEnoughLevel
	setvar VAR_RESULT, TRUE
	return

MfAide_EventScript_LevelTooLow::
	msgbox MfAide_Text_LevelTooLow
	release
	end

MfAide_Text_GiftOnlyWaived::
	.string "Hi! I'm one of Prof. Oak's Aides.\p"
	.string "Prof. Oak told me you can't catch\n"
	.string "Pokémon on your journey.\p"
	.string "So he asked me to give you\n"
	.string "the {STR_VAR_2} anyway!$"

MfAide_Text_GiveItemIfEnoughBadges::
	.string "Hi! I'm one of Prof. Oak's Aides.\p"
	.string "If you've earned {STR_VAR_1} Badges,\n"
	.string "I'm supposed to give you\l"
	.string "the {STR_VAR_2}.\p"
	.string "Let me see your Badges…$"

MfAide_Text_EnoughBadges::
	.string "Great! You have {STR_VAR_3} Badges!\p"
	.string "Congratulations!\n"
	.string "Here you go!$"

MfAide_Text_NotEnoughBadges::
	.string "Uh-oh! You don't have enough\n"
	.string "Badges yet!\p"
	.string "You need {STR_VAR_1} Badges\n"
	.string "if you want the {STR_VAR_2}.$"

MfAide_Text_GiveItemIfHighEnoughLevel::
	.string "Hi! I'm one of Prof. Oak's Aides.\p"
	.string "If one of your Pokémon is at\n"
	.string "least Lv. {STR_VAR_1}, I'm supposed to\l"
	.string "give you the {STR_VAR_2}.\p"
	.string "Let me see your party…$"

MfAide_Text_HighEnoughLevel::
	.string "Great! Your Pokémon has\n"
	.string "reached Lv. {STR_VAR_3}!\p"
	.string "Congratulations!\n"
	.string "Here you go!$"

MfAide_Text_LevelTooLow::
	.string "Uh-oh! Your strongest Pokémon\n"
	.string "is only Lv. {STR_VAR_3}!\p"
	.string "You need one at Lv. {STR_VAR_1}\n"
	.string "if you want the {STR_VAR_2}.$"
