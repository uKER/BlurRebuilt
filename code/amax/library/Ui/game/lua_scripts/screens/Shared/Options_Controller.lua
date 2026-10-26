GUI =
{
	finished = false,	
	carousel_branch = "Options",
	selection = -1,
	initial_selection = -1,
	
	bottom_bar_text_id = nil,

	CanExit = function( _GUI )
		return false
	end,
}

function Init()
	AddSCUI_Elements()
	
	GUI.OptionsId = SCUI.name_to_id[ "OptionsList" ]

	GUI.configs_table = Amax.GetControllerConfigs()
	if IsTable( GUI.configs_table ) == true then
		for i,v in ipairs( GUI.configs_table ) do
			UIButtons.AddItem( GUI.OptionsId, i, v.name, false )
		end
	end

	StoreInfoLine()
	SetupInfoLine(UIText.INFO_A_CONFIRM, UIText.INFO_B_CANCEL)
	
	if UIEnums.CurrentPlatform == UIEnums.Platform.PS3 then
		SetupScreenTitle( UIText.CMN_CONTROLS_TITLE, SCUI.name_to_id[ "Dummy" ], "ps3_controller_options" )
	else
		SetupScreenTitle( UIText.CMN_CONTROLS_TITLE, SCUI.name_to_id[ "Dummy" ], "controller_options" )
	end
	
	GUI.initial_selection = UIGlobals.OptionsTable.ctrl_index
	GUI.bottom_bar_text_id, GUI.bottom_bar_root_id = SetupBottomHelpBar( GUI.configs_table[UIGlobals.OptionsTable.ctrl_index + 1].name )
	
	UIButtons.SetParent( SCUI.name_to_id[ "arrow_left" ],	GUI.bottom_bar_root_id, UIEnums.Justify.MiddleCentre )
	UIButtons.SetParent( SCUI.name_to_id[ "arrow_right" ],	GUI.bottom_bar_root_id, UIEnums.Justify.MiddleCentre )
end

function PostInit()
	local ctrl_parent = SCUI.name_to_id[ "Bra_360" ]
	
	if UIEnums.CurrentPlatform == UIEnums.Platform.PS3 then
		print("Ps3")
		ctrl_parent = SCUI.name_to_id[ "Bra_Ps3" ]
	elseif UIEnums.CurrentPlatform == UIEnums.Platform.Xenon or UIEnums.CurrentPlatform == UIEnums.Platform.PC then
		print("360")
		ctrl_parent = SCUI.name_to_id[ "Bra_360" ]
	end
	
	UIButtons.SetActive( ctrl_parent, true )
		
	
	GUI.text_table = 
	{
		UIButtons.FindChildByName( ctrl_parent, "Txt_AButton" ),
		UIButtons.FindChildByName( ctrl_parent, "Txt_BButton" ),
		UIButtons.FindChildByName( ctrl_parent, "Txt_XButton" ),
		UIButtons.FindChildByName( ctrl_parent, "Txt_YButton" ),
		UIButtons.FindChildByName( ctrl_parent, "Txt_LTrigger" ),
		UIButtons.FindChildByName( ctrl_parent, "Txt_RTrigger" ),
		UIButtons.FindChildByName( ctrl_parent, "Txt_LBumper" ),
		UIButtons.FindChildByName( ctrl_parent, "Txt_RBumper" ),
		UIButtons.FindChildByName( ctrl_parent, "Txt_RStickButton" ),
	}
	
	GUI.parent_line_table = 
	{
		UIButtons.FindChildByName( ctrl_parent, "Gfx_LineA_AButton" ),
		UIButtons.FindChildByName( ctrl_parent, "Gfx_LineA_BButton" ),
		UIButtons.FindChildByName( ctrl_parent, "Gfx_LineA_XButton" ),
		UIButtons.FindChildByName( ctrl_parent, "Gfx_LineA_YButton" ),
		UIButtons.FindChildByName( ctrl_parent, "Gfx_LineA_LTrigger" ),
		UIButtons.FindChildByName( ctrl_parent, "Gfx_LineA_RTrigger" ),
		UIButtons.FindChildByName( ctrl_parent, "Gfx_LineA_LBumper" ),
		UIButtons.FindChildByName( ctrl_parent, "Gfx_LineA_RBumper" ),
		UIButtons.FindChildByName( ctrl_parent, "Gfx_LineA_RStick" ),
	}

	UIButtons.SetSelection( GUI.OptionsId, UIGlobals.OptionsTable.ctrl_index + 1 )
	UpdateControllerButtons()
end

function StartLoop( time )
	--
end

function MessageUpdate( message, arg1, arg2, arg3, arg4 )
	if SCUI_MessageUpdate( message, arg1, arg2, arg3, arg4 ) == true then
		return
	end
	
	if message == UIEnums.Message.PopupNext and arg2 == UIEnums.CustomPopups.DiscardOptionsChanges and arg3 == UIEnums.PopupOptions.Yes then
		GoBack( true ) -- all changes are done on leaving the screen so no need to "revert" any values
	end

	if message == UIEnums.Message.MenuNext and arg2 == true then
		PlaySfxNext()
		
		local ctrl_index = UIButtons.GetSelection( GUI.OptionsId ) - 1

		if UIGlobals.OptionsTable.ctrl_index ~= ctrl_index then
			Amax.GameDataLogControllerLayoutChanged(UIGlobals.OptionsTable.ctrl_index, ctrl_index)
			UIGlobals.OptionsTable.ctrl_index = ctrl_index
			UIGlobals.SaveOptions = true
		end
		
		GoScreen("Shared\\Options.lua")
	end
	
	if message == UIEnums.Message.MenuBack and arg2 == true then
		if GUI.initial_selection ==  UIButtons.GetSelection( GUI.OptionsId ) - 1 then
			GoBack()
		else
			SetupCustomPopup( UIEnums.CustomPopups.DiscardOptionsChanges )
		end
	end
end

function UpdateControllerButtons()
	local selection = UIButtons.GetSelection( GUI.OptionsId )
	if GUI.selection == selection then
		return
	end
	
	if IsTable( GUI.configs_table ) == false then
		return
	end
	
	if IsTable( GUI.configs_table[ selection ] ) == false then
		return
	end
	
	UIButtons.ChangeText( GUI.bottom_bar_text_id, GUI.configs_table[selection].name )
	
	-- animate arrows appropriately (amazing alliteration!)
	if selection == 1 then
		UIButtons.TimeLineActive( "left_fade", false )
	elseif selection == 3 then
		UIButtons.TimeLineActive( "right_fade", false )
	else
		UIButtons.TimeLineActive( "left_fade", true )
		UIButtons.TimeLineActive( "right_fade", true )
	end
	
	if GUI.selection < selection  and GUI.selection ~= -1 then
		UIButtons.TimeLineActive( "move_right", true, 0 )
	else
		UIButtons.TimeLineActive( "move_left", true, 0 )
	end
	
	local ct = GUI.configs_table[ selection ]
	local t = 
	{
		ct.ButtonsA,
		ct.ButtonsB,
		ct.ButtonsX,
		ct.ButtonsY,
		ct.ButtonsLT,
		ct.ButtonsRT,
		ct.ButtonsLB,
		ct.ButtonsRB,
		ct.ButtonsRThumb,
	}

	for i,v in ipairs( GUI.text_table ) do
		if v == 0  or t[i] == 32767 then		-- invalid lod id - see Loc.h
			UIButtons.SetActive( v, false )
			UIButtons.SetActive( GUI.parent_line_table[i], false, true )
		else
			UIButtons.SetActive( v, true )
			UIButtons.SetActive( GUI.parent_line_table[i], true, true )
			UIButtons.ChangeText( v, t[ i ] )
		end
	end
	
	GUI.selection = selection
end

function FrameUpdate( time )
	--
	UpdateControllerButtons()
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