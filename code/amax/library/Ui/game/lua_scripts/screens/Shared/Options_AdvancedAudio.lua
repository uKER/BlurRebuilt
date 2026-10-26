GUI =
{
	finished = false,	
	carousel_branch = "Options",

	GetSelections = function()
		UIGlobals.OptionsTable.room		= UIButtons.GetSelection( GUI.RoomSizeId )
		UIGlobals.OptionsTable.music_mode	= UIButtons.GetSelection( GUI.MusicModeId )
		UIGlobals.OptionsTable.lfe		= UIButtons.GetSelection( GUI.BoostId )
	end,
	
	SetSelections = function()
		UIButtons.SetSelection( GUI.RoomSizeId, UIGlobals.OptionsTable.room )
		UIButtons.SetSelection( GUI.MusicModeId, UIGlobals.OptionsTable.music_mode )
		UIButtons.SetSelection( GUI.BoostId, UIGlobals.OptionsTable.lfe )
	end,
	
	HaveChangedSelections = function()
		return ( GUI.StartingOptionsTable.room ~= UIButtons.GetSelection( GUI.RoomSizeId ) or 
				GUI.StartingOptionsTable.music_mode ~= UIButtons.GetSelection( GUI.MusicModeId ) or
				GUI.StartingOptionsTable.lfe ~= UIButtons.GetSelection( GUI.BoostId ) )
	end,

	UpdateInfoLine = function()
		local selection = UIButtons.GetSelectionIndex( GUI.MessageListId )
		if GUI.selection ~= selection then
			GUI.selection = selection 
			if selection == 3 then
				SetupInfoLine(UIText.INFO_A_SELECT, UIText.INFO_B_CANCEL, UIText.INFO_RESET_TO_DEFAULT)
			else
				SetupInfoLine(UIText.INFO_A_CONFIRM, UIText.INFO_B_CANCEL, UIText.INFO_RESET_TO_DEFAULT)
			end
		end
	end,
	
	bottom_help_text = 
	{
		UIText.CMN_HLP_ADVANCED_AUDIO_ROOM,
		UIText.CMN_HLP_ADVANCED_AUDIO_MODE,
		UIText.CMN_HLP_ADVANCED_AUDIO_LFE,
		UIText.CMN_HLP_ADVANCED_AUDIO_SPEAKERS,
	},
	
	CanExit = function( _GUI )
		return false
	end,
}

function Init()
	AddSCUI_Elements()
	
	GUI.RoomSizeId		= SCUI.name_to_id[ "Sdr_RoomSize" ]
	GUI.MusicModeId		= SCUI.name_to_id[ "Sdr_MusicMode" ]
	GUI.BoostId			= SCUI.name_to_id[ "Sdr_Boost" ]
	GUI.SpeakersId		= SCUI.name_to_id[ "Nde_Speaker" ]
	
	GUI.MessageListId	= SCUI.name_to_id[ "OptionsList" ]
	
	UIButtons.AddItem( GUI.RoomSizeId, 0, UIText.CMN_OPT_AUDIO_SMALL, false )
	UIButtons.AddItem( GUI.RoomSizeId, 1, UIText.CMN_OPT_AUDIO_MEDIUM, false )
	UIButtons.AddItem( GUI.RoomSizeId, 2, UIText.CMN_OPT_AUDIO_LARGE, false )
	UIButtons.AddItem( GUI.RoomSizeId, 3, UIText.CMN_OPT_AUDIO_AUDITORIUM, false )
	
	UIButtons.AddItem( GUI.MusicModeId, 0, UIText.CMN_OPT_AUDIO_STEREO, false )
	UIButtons.AddItem( GUI.MusicModeId, 1, UIText.CMN_OPT_AUDIO_QUAD, false )
	UIButtons.AddItem( GUI.MusicModeId, 2, UIText.CMN_OPT_AUDIO_SURROUND, false )
	
	-- Add ten items for LFE Boost
	for i = 0, 9, 1 do
		UIButtons.AddItem( GUI.BoostId, i, i, false )
	end
	
	-- Get the options from the game
	GUI.SetSelections()
	
	StoreInfoLine()
	
	GUI.StartingOptionsTable = CloneTable( UIGlobals.OptionsTable )

	SetupScreenTitle( UIText.CMN_OPT_ADVANCED_AUDIO_OPTIONS, SCUI.name_to_id["Dummy"], "sub" )
end

function PostInit()
	local box_id, text_id = SetupSelectionBox( GUI.SpeakersId, UIText.CMN_OPT_AUDIO_SPEAKERS, nil, nil, nil, nil, UIEnums.Justify.MiddleCentre )
	UIButtons.ChangeScale( box_id, 1.4, 1.4 )
	UIButtons.ChangePosition( box_id, 0, -13 ) -- cough
	
	UIButtons.NodeListScan( GUI.MessageListId )
	local s = GetScreenOtions().selection
	if IsNumber( s ) == true then
		UIButtons.SetSelectionByIndex( GUI.MessageListId, s )
		GUI.bottom_text_id = SetupBottomHelpBar( GUI.bottom_help_text[s+1] )
	else
		UIButtons.SetSelectionByIndex( GUI.MessageListId, 0 )
		GUI.bottom_text_id = SetupBottomHelpBar( GUI.bottom_help_text[1] )
	end
	
	GUI.UpdateInfoLine()

	GUI.do_update_options = true
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
			GoScreen("Shared\\Options_Audio.lua")
		elseif arg2 == UIEnums.CustomPopups.ResetToDefaults and arg3 == UIEnums.PopupOptions.Yes then
			ResetToDefaults()
		end
	end

	if message == UIEnums.Message.MenuNext and arg2 == true then
		if UIButtons.GetSelectionIndex( GUI.MessageListId ) == 3 then
			PlaySfxNext()
			GoScreen("Shared\\Options_SpeakerPos.lua")
		else
			PlaySfxNext()
		
			-- Send the options back to the game
			GUI.GetSelections()
			GoScreen("Shared\\Options_Audio.lua")
		end
		
		return
	end
	
	if message == UIEnums.Message.MenuBack and arg2 == true then
		if GUI.HaveChangedSelections() == true then
			SetupCustomPopup( UIEnums.CustomPopups.DiscardOptionsChanges )
		else
			PlaySfxBack()
			GoScreen("Shared\\Options_Audio.lua")
		end
	elseif message == UIEnums.Message.ButtonLeft or message == UIEnums.Message.ButtonRight then
		GUI.do_update_options = true
	elseif message == UIEnums.Message.ButtonDown or message == UIEnums.Message.ButtonUp then
		GUI.do_update_options = true
	elseif message == UIEnums.Message.ButtonX then
		SetupCustomPopup( UIEnums.CustomPopups.ResetToDefaults )
	end
end

function FrameUpdate( time )
	GUI.UpdateInfoLine()

	--
	if GUI.do_update_options == true then
		GUI.GetSelections()
		
		local selected = UIButtons.GetSelectionIndex( GUI.MessageListId )
		UIButtons.ChangeText( GUI.bottom_text_id, GUI.bottom_help_text[ selected + 1 ] )
		UIButtons.TimeLineActive( "HelpFade", true, 0.5 )

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
	GetScreenOtions().selection = UIButtons.GetSelectionIndex( GUI.MessageListId )
end

function ResetToStarting()
	UIGlobals.OptionsTable.room			= GUI.StartingOptionsTable.room	
	UIGlobals.OptionsTable.music_mode	= GUI.StartingOptionsTable.music_mode
	UIGlobals.OptionsTable.lfe			= GUI.StartingOptionsTable.lfe	
	Amax.UpdateAudioOptions( UIGlobals.OptionsTable )
end

function ResetToDefaults()
	UIGlobals.OptionsTable.room			= UIGlobals.DefaultOptionsTable.room	
	UIGlobals.OptionsTable.music_mode	= UIGlobals.DefaultOptionsTable.music_mode
	UIGlobals.OptionsTable.lfe			= UIGlobals.DefaultOptionsTable.lfe	

	GUI.SetSelections()
	GUI.StartingOptionsTable = CloneTable( UIGlobals.OptionsTable )
	--[mtaylor] 37081 (Audio updates when options are reset to default)
	Amax.UpdateAudioOptions( UIGlobals.OptionsTable )
end
