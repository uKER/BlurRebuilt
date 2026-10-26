GUI =
{
	finished = false,	
	carousel_branch = "Options",

	diff = 0,
	initial_diff = 0,
	
	node_ids = { }, 
	
	difficulty_strings = {
		UIText.CMN_DIFFICULTY_1,
		UIText.CMN_DIFFICULTY_2,
		UIText.CMN_DIFFICULTY_3,
		UIText.CMN_DIFFICULTY_4,
		UIText.CMN_DIFFICULTY_5,
						},
						
	difficulty_icons = {
		"ai_difficulty_low",
		"ai_difficutly_mid",
		"ai_difficulty",
						},
	
	difficulty_explanations = {
		UIText.CMN_HLP_OPT_DIFFICULTY1,
		UIText.CMN_HLP_OPT_DIFFICULTY2,
		UIText.CMN_HLP_OPT_DIFFICULTY3,
		UIText.CMN_HLP_OPT_DIFFICULTY4,
		UIText.CMN_HLP_OPT_DIFFICULTY5,
								},
		
	CanExit = function( _GUI )
		return false
	end,
}

function Init()
	AddSCUI_Elements()
	
	GUI.MessageListId = SCUI.name_to_id[ "OptionsList" ]
	
	GUI.DifficultyId		= SCUI.name_to_id[ "DifficultyList" ]
	
	GUI.diff = UIGlobals.OptionsTable.difficulty
	GUI.initial_diff = GUI.diff
	
	SetupScreenTitle( UIText.CMN_DIFFICULTY, SCUI.name_to_id["Dummy"], "ai_difficulty" )
	GUI.bottom_help_text_id = SetupBottomHelpBar( GUI.difficulty_explanations[UIGlobals.OptionsTable.difficulty + 1] )
	
	StoreInfoLine()
	SetupInfoLine(UIText.INFO_A_CONFIRM, UIText.INFO_B_CANCEL)
end

function PostInit()
	for i=2,4 do
		GUI.node_ids[i] = UIButtons.CloneXtGadgetByName( "Shared\\Options_DrivingAssists.lua", "Nde_Difficulty" )
		UIButtons.SetActive( GUI.node_ids[i], true, true )
		UIButtons.AddListItem( GUI.DifficultyId, GUI.node_ids[i], i-1 )
		UIButtons.ChangeText( UIButtons.FindChildByName( GUI.node_ids[i], "txt_difficulty" ), GUI.difficulty_strings[i] )
		UIShape.ChangeObjectName( UIButtons.FindChildByName( GUI.node_ids[i], "Shp_Difficulty" ), GUI.difficulty_icons[i-1] )
	end
	
	UIButtons.SetSelection( GUI.DifficultyId, UIGlobals.OptionsTable.difficulty )
	
	-- set up arrows
	if GUI.diff ~= 1 then
		UIButtons.TimeLineActive( "left_fade", true )
	end
	
	if GUI.diff ~= 3 then
		UIButtons.TimeLineActive( "right_fade", true )
	end
		
end

function StartLoop( time )
	--
end

function MessageUpdate( message, arg1, arg2, arg3, arg4 )
	if SCUI_MessageUpdate( message, arg1, arg2, arg3, arg4 ) == true then
		return
	end
	
	if message == UIEnums.Message.PopupNext and arg2 == UIEnums.CustomPopups.DiscardOptionsChanges and arg3 == UIEnums.PopupOptions.Yes then
		GUI.diff = GUI.initial_diff
		GoBack( true )
	end

	if message == UIEnums.Message.MenuNext and arg2 == true then
		PlaySfxNext()

		local difficulty = UIButtons.GetSelection( GUI.DifficultyId )
				
		if UIGlobals.OptionsTable.difficulty ~= difficulty then
			Amax.GameDataLogDifficultySelected(UIGlobals.OptionsTable.difficulty, difficulty)
			UIGlobals.OptionsTable.difficulty = difficulty
			UIGlobals.SaveOptions = true
		end

		GoScreen("Shared\\Options.lua")
	end
	
	if message == UIEnums.Message.MenuBack and arg2 == true then
		if GUI.initial_diff == GUI.diff then
			GoBack()
		else
			SetupCustomPopup( UIEnums.CustomPopups.DiscardOptionsChanges )
		end
	end
end

function FrameUpdate( time )
	local diff = UIButtons.GetSelection( GUI.DifficultyId )

	if GUI.diff ~= diff then
		local from = GUI.diff
		GUI.diff = diff
		UIButtons.ChangeText( GUI.bottom_help_text_id, GUI.difficulty_explanations[diff + 1] )
		
		-- arrows
		if diff == 1 then
			UIButtons.TimeLineActive( "left_fade", false )
			UIButtons.TimeLineActive( "move_left", true, 0 )
		elseif diff == 3 then
			UIButtons.TimeLineActive( "right_fade", false )
			UIButtons.TimeLineActive( "move_right", true, 0 )
		else
			UIButtons.TimeLineActive( "right_fade", true )
			UIButtons.TimeLineActive( "left_fade", true )
			if from == 1 then
				UIButtons.TimeLineActive( "move_right", true, 0 )
			elseif from == 3 then
				UIButtons.TimeLineActive( "move_left", true, 0 )
			end
		end

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

function GoBack( no_sound )
	if no_sound ~= true then
		PlaySfxBack()
	end
	
	GoScreen("Shared\\Options.lua")
end
