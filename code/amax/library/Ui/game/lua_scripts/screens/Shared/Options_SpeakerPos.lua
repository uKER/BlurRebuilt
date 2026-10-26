GUI =
{
	finished = false,	
	carousel_branch = "Options",
	
	GetSelections = function()
		UIGlobals.OptionsTable.speaker_fl, UIGlobals.OptionsTable.speaker_fr, UIGlobals.OptionsTable.speaker_rr, UIGlobals.OptionsTable.speaker_rl = Amax.Speakers( GUI.SpeakersId )
	end,
	
	HaveChangedSelections = function()
		local fl, fr, rr, rl = Amax.Speakers( GUI.SpeakersId )
		return ( GUI.StartingOptionsTable.speaker_fl ~= fl or
				GUI.StartingOptionsTable.speaker_fr ~= fr or
				GUI.StartingOptionsTable.speaker_rr ~= rr or
				GUI.StartingOptionsTable.speaker_rl ~= rl )
	end,

	CanExit = function( _GUI )
		return false
	end,
}

function Init()
	AddSCUI_Elements()
	
	GUI.SpeakersId = SCUI.name_to_id[ "Speakers" ]
	
	-- Set the gadgets positions from the table
	Amax.Speakers( GUI.SpeakersId, UIGlobals.OptionsTable.speaker_fl, UIGlobals.OptionsTable.speaker_fr, UIGlobals.OptionsTable.speaker_rr, UIGlobals.OptionsTable.speaker_rl )
	
	StoreInfoLine()
	SetupInfoLine( UIText.INFO_A_CONFIRM, UIText.INFO_B_CANCEL, UIText.INFO_RESET_TO_DEFAULT, UIText.INFO_LS_SPEAKER, UIText.INFO_RS_ANGLE  )

	GUI.StartingOptionsTable = CloneTable( UIGlobals.OptionsTable )
	
	SetupScreenTitle( UIText.CMN_OPT_AUDIO_SPEAKERS, SCUI.name_to_id["Dummy"], "small_speaker" )
	SetupBottomHelpBar( UIText.CMN_HLP_AUDIO_SPEAKERS_SCREEN )
end

function PostInit()
end

function StartLoop( time )
	--
end

function MessageUpdate( message, arg1, arg2, arg3, arg4 )
	if SCUI_MessageUpdate( message, arg1, arg2, arg3, arg4 ) == true then
		return
	end
	
	if message == UIEnums.Message.PopupNext then
		if arg2 == UIEnums.CustomPopups.DiscardOptionsChanges and arg3 == UIEnums.PopupOptions.Yes then
			ResetToStarting()
			GoScreen("Shared\\Options_AdvancedAudio.lua")
		elseif arg2 == UIEnums.CustomPopups.ResetToDefaults and arg3 == UIEnums.PopupOptions.Yes then
			ResetToDefaults()
		end
	end

	if message == UIEnums.Message.MenuNext then
		PlaySfxNext()
		GUI.GetSelections() -- save changes
		GoScreen("Shared\\Options_AdvancedAudio.lua")
	elseif message == UIEnums.Message.MenuBack and arg2 == true then
		if GUI.HaveChangedSelections() == true then
			SetupCustomPopup( UIEnums.CustomPopups.DiscardOptionsChanges )
		else
			PlaySfxBack()
			GoScreen("Shared\\Options_AdvancedAudio.lua")
		end
	elseif message == UIEnums.Message.StickLeftX or message == UIEnums.Message.StickRightY then
		GUI.do_update_options = true
	elseif message == UIEnums.Message.ButtonX then
		SetupCustomPopup( UIEnums.CustomPopups.ResetToDefaults )
	end
end

function FrameUpdate( time )
	--
	if GUI.do_update_options == true then
		Amax.UpdateAudioOptions( UIGlobals.OptionsTable )
		GUI.do_update_options = false
	end
end

function Render()
	--
end

function EnterEnd()
	RestoreInfoLine()
end

function EndLoop( time )
	--
end

function End()
end

function ResetToStarting()
	Amax.Speakers( GUI.SpeakersId, GUI.StartingOptionsTable.speaker_fl, GUI.StartingOptionsTable.speaker_fr, GUI.StartingOptionsTable.speaker_rr, GUI.StartingOptionsTable.speaker_rl )
	Amax.UpdateAudioOptions( GUI.StartingOptionsTable )
end

function ResetToDefaults()
	Amax.Speakers( GUI.SpeakersId, UIGlobals.DefaultOptionsTable.speaker_fl, UIGlobals.DefaultOptionsTable.speaker_fr, UIGlobals.DefaultOptionsTable.speaker_rr, UIGlobals.DefaultOptionsTable.speaker_rl )
	GUI.StartingOptionsTable = CloneTable( UIGlobals.OptionsTable )
	--[mtaylor] 37081 (Audio updates when options are reset to default)
	Amax.UpdateAudioOptions( UIGlobals.OptionsTable )
end
