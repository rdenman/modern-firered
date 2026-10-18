@ VAR_0x8004 in = vanilla gift species. Buffers the static remap into STR_VAR_1
@ without changing the givemon source (do not givemon VAR_0x8004 after this).
Common_EventScript_BufferStaticGiftSpecies::
	specialvar VAR_0x8004, MfScript_StaticGiftSpecies
	bufferspeciesname STR_VAR_1, VAR_0x8004
	return

@ Skip the optional Yes/No when Nuzlocke NICKNAMES is on (S38 gift path).
Common_EventScript_AskNicknameGiftMon::
	bufferspeciesname STR_VAR_1, VAR_TEMP_TRANSFERRED_SPECIES
	specialvar VAR_RESULT, MfNuzlocke_ScriptIsNicknamingActive
	goto_if_eq VAR_RESULT, TRUE, Common_EventScript_AskNicknameGiftMonForced
	msgbox gText_NicknameThisPokemon, MSGBOX_YESNO
	return

Common_EventScript_AskNicknameGiftMonForced::
	setvar VAR_RESULT, YES
	return

@ VAR_0x8004 here is used by ChangePokemonNickname
Common_EventScript_GetGiftMonPartySlot::
	getpartysize
	subvar VAR_RESULT, 1
	copyvar VAR_0x8004, VAR_RESULT
	return

Common_EventScript_NameReceivedBoxMon::
	fadescreen FADE_TO_BLACK
	setvar VAR_0x8004, PC_MON_CHOSEN
	special ChangePokemonNickname
	lock
	faceplayer
	return

Common_EventScript_TransferredToPC::
	bufferboxname STR_VAR_1, VAR_PC_BOX_TO_SEND_MON
	bufferspeciesname STR_VAR_2, VAR_TEMP_TRANSFERRED_SPECIES
	call_if_unset FLAG_SYS_PC_LANETTE, EventScript_TransferredSomeonesPC
	call_if_set FLAG_SYS_PC_LANETTE, EventScript_TransferredLanettesPC
	return

EventScript_TransferredSomeonesPC::
	specialvar VAR_RESULT, ShouldShowBoxWasFullMessage
	goto_if_eq VAR_RESULT, TRUE, EventScript_SomeonesPCBoxFull
	msgbox gText_PkmnTransferredSomeonesPC, MSGBOX_DEFAULT
	return

EventScript_SomeonesPCBoxFull::
	specialvar VAR_RESULT, GetPCBoxToSendMon
	bufferboxname STR_VAR_3, VAR_RESULT
	msgbox gText_PkmnTransferredSomeonesPCBoxFull, MSGBOX_DEFAULT
	return

EventScript_TransferredLanettesPC::
	specialvar VAR_RESULT, ShouldShowBoxWasFullMessage
	goto_if_eq VAR_RESULT, TRUE, EventScript_LanettesPCBoxFull
	msgbox gText_PkmnTransferredLanettesPC, MSGBOX_DEFAULT
	return

EventScript_LanettesPCBoxFull::
	specialvar VAR_RESULT, GetPCBoxToSendMon
	bufferboxname STR_VAR_3, VAR_RESULT
	msgbox gText_PkmnTransferredLanettesPCBoxFull, MSGBOX_DEFAULT
	return

Common_EventScript_NoMoreRoomForPokemon::
	msgbox gText_NoMoreRoomForPokemon, MSGBOX_DEFAULT
	release
	end
