CeladonCity_DepartmentStore_4F_Frlg_MapScripts::
	.byte 0

CeladonCity_DepartmentStore_4F_EventScript_Man::
	msgbox CeladonCity_DepartmentStore_4F_Text_GettingPokeDollAsPresent, MSGBOX_NPC
	end

CeladonCity_DepartmentStore_4F_EventScript_Youngster::
	msgbox CeladonCity_DepartmentStore_4F_Text_CanRunAwayWithPokeDoll, MSGBOX_NPC
	end

CeladonCity_DepartmentStore_4F_EventScript_FloorSign::
	msgbox CeladonCity_DepartmentStore_4F_Text_FloorSign, MSGBOX_SIGN
	end

CeladonCity_DepartmentStore_4F_EventScript_Clerk::
	lock
	faceplayer
	message gText_HowMayIServeYou
	waitmessage
	pokemart CeladonCity_DepartmentStore_4F_Items
	msgbox gText_PleaseComeAgain
	release
	end

	.align 2
@ S69 / ADR 0103: Linking Cord, trade-held, and Kanto-reachable cross-evo items.
@ Always listed (no shopCriteria) — bag-use is compile always-on (ADR 0035).
CeladonCity_DepartmentStore_4F_Items::
	.2byte ITEM_POKE_DOLL
	.2byte ITEM_RETRO_MAIL
	.2byte ITEM_FIRE_STONE
	.2byte ITEM_THUNDER_STONE
	.2byte ITEM_WATER_STONE
	.2byte ITEM_LEAF_STONE
	.2byte ITEM_ICE_STONE
	.2byte ITEM_SHINY_STONE
	.2byte ITEM_DUSK_STONE
	.2byte ITEM_DAWN_STONE
	.2byte ITEM_LINKING_CORD
	.2byte ITEM_METAL_COAT
	.2byte ITEM_KINGS_ROCK
	.2byte ITEM_DRAGON_SCALE
	.2byte ITEM_UPGRADE
	.2byte ITEM_DUBIOUS_DISC
	.2byte ITEM_PROTECTOR
	.2byte ITEM_ELECTIRIZER
	.2byte ITEM_MAGMARIZER
	.2byte ITEM_PRISM_SCALE
	.2byte ITEM_REAPER_CLOTH
	.2byte ITEM_DEEP_SEA_TOOTH
	.2byte ITEM_DEEP_SEA_SCALE
	.2byte ITEM_OVAL_STONE
	.2byte ITEM_RAZOR_CLAW
	.2byte ITEM_RAZOR_FANG
	.2byte ITEM_NONE
	release
	end

CeladonCity_DepartmentStore_4F_Text_GettingPokeDollAsPresent::
	.string "I'm getting a present for my\n"
	.string "girlfriend.\p"
	.string "I guess a Poké Doll will be it.\n"
	.string "It's the popular thing.$"

CeladonCity_DepartmentStore_4F_Text_CanRunAwayWithPokeDoll::
	.string "I heard something useful.\p"
	.string "If a wild Pokémon appears, you can\n"
	.string "distract it with a Poké Doll.\p"
	.string "You can run away while the wild\n"
	.string "Pokémon is distracted.$"

CeladonCity_DepartmentStore_4F_Text_FloorSign::
	.string "Express yourself with gifts!\n"
	.string "4F: Wise Man Gifts\p"
	.string "Evolution Special!\n"
	.string "Stones and Linking Cords!$"
