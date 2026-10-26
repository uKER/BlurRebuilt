GUI =
{
	finished = false,
	
	num_options = -1,	-- set in PostInit
	
	node_list_id = -1,
	current_option_selection = -1,
	
	switch_icon_ids = {  },
	switch_rb_ids = {  },
	switch_node_list_ids = {  },
	switch_text_ids = {  },
	
	current_switch_selection = {  },
	
	choosing_switch = false,
}

function Init()
	PlaySfxGraphicNext()

	AddSCUI_Elements()
	
	StoreInfoLine()
	SetupInfoLine()
	
	GUI.camera_id = UIButtons.CloneXtGadgetByName( "SCUIBank", "Cam_Carousel" )
	UIButtons.SetParent( GUI.camera_id, SCUI.name_to_id[ "CameraDolly" ], UIEnums.Justify.MiddleCentre )
	
	UIScreen.BlockInputToContext( true )
	UIScreen.SetScreenTimers( 0.3, 0.3 )
end

function PostInit()
	UIButtons.TimeLineActive( "stage1", true, 0 )
	
	local screen_id = SCUI.name_to_id[ "screen" ]
	
	SetupScreenTitle( UIText.FDE_FRIEND_DEMAND, screen_id, "message", "common_icons", nil, UIEnums.Justify.TopCentre, true, nil, UIEnums.Panel._3DAA_LIGHT, nil, UIEnums.Justify.TopCentre )
	GUI.help_text_id = SetupBottomHelpBar( UIText.FDE_CREATE_HELP1, nil, 4, true )
	
	-- add options
	GUI.node_list_id = SCUI.name_to_id[ "node_list_options" ]
	
	local list_height = 0
	GUI.num_options = FriendDemand.GetParamCount()
	
	for i = 1, GUI.num_options, 1 do
		local node_id = UIButtons.CloneXtGadgetByName( "Ingame\\CreateFriendDemand.lua", "_option_node" )
		local backing_id = UIButtons.FindChildByName( node_id, "_option_backing" )
		local text_title_id = UIButtons.FindChildByName( node_id, "_option_text_title" )
		local text_info_id = UIButtons.FindChildByName( node_id, "_option_text_info" )
		
		-- option icon
		local icon_id = UIButtons.FindChildByName( node_id, "_option_icon" )
		local param_type = FriendDemand.GetParamType( i - 1 )
		local icon_info = Fd_GetParamIconInfo( param_type )
		
		UIButtons.ChangeTexture( icon_info.tex, 0, icon_id )
		UIButtons.ChangeEffectIndex( icon_id, icon_info.effect )
		UIButtons.ChangeColour( icon_id, icon_info.colour )		
		
		UIButtons.ChangeText( text_title_id, "GAME_FRIEND_DEMAND_PARAM_TITLE_" .. ( i - 1 ) )
		UIButtons.ChangeText( text_info_id, "GAME_FRIEND_DEMAND_PARAM_VAR_" .. ( i - 1 ) )
		
		if ( i % 2 ) == 0 then
			UIButtons.SetActive( backing_id, false )
		end
		
		local sx, sy, sz = UIButtons.GetSize( node_id )
		
		if i == GUI.num_options then
			local last_size_y = 7
			UIButtons.ChangeSize( node_id, sx, last_size_y, sz )
			list_height = list_height + last_size_y
		else
			list_height = list_height + sy
		end
		
		GUI.switch_icon_ids[ i ] = UIButtons.FindChildByName( node_id, "_option_modifier_icon" )
		
		-- add switches
		GUI.switch_rb_ids[ i ] = UIButtons.FindChildByName( node_id, "_option_switch_rb" )
		GUI.switch_node_list_ids[ i ] = UIButtons.FindChildByName( node_id, "_option_switch_node_list" )
		GUI.switch_text_ids[ i ] = UIButtons.FindChildByName( node_id, "_option_switch_info" )
		
		GUI.current_switch_selection[ i ] = -1
		
		local node_id_less = UIButtons.CloneXtGadgetByName( "Ingame\\CreateFriendDemand.lua", "_option_switch_node_less" )
		local icon_id_less = UIButtons.FindChildByName( node_id_less, "_option_switch_icon_less" )
		local info_less = Fd_GetModifierIconInfo( UIEnums.FriendDemandModifer.LessThan )
		UIButtons.ChangeTexture( info_less.tex, 0, icon_id_less )
		UIButtons.ChangeEffectIndex( icon_id_less, info_less.effect )
		UIButtons.AddListItem( GUI.switch_node_list_ids[ i ], node_id_less, UIEnums.FriendDemandModifer.LessThan )		
		
		local node_id_equal = UIButtons.CloneXtGadgetByName( "Ingame\\CreateFriendDemand.lua", "_option_switch_node_equal" )
		local icon_id_equal = UIButtons.FindChildByName( node_id_equal, "_option_switch_icon_equal" )
		local info_equal = Fd_GetModifierIconInfo( UIEnums.FriendDemandModifer.Equals )
		UIButtons.ChangeTexture( info_equal.tex, 0, icon_id_equal )
		UIButtons.ChangeEffectIndex( icon_id_equal, info_equal.effect )
		UIButtons.AddListItem( GUI.switch_node_list_ids[ i ], node_id_equal, UIEnums.FriendDemandModifer.Equals )
		
		local node_id_more = UIButtons.CloneXtGadgetByName( "Ingame\\CreateFriendDemand.lua", "_option_switch_node_more" )
		local icon_id_more = UIButtons.FindChildByName( node_id_more, "_option_switch_icon_more" )
		local info_more = Fd_GetModifierIconInfo( UIEnums.FriendDemandModifer.GreaterThan )
		UIButtons.ChangeTexture( info_more.tex, 0, icon_id_more )
		UIButtons.ChangeEffectIndex( icon_id_more, info_more.effect )
		UIButtons.AddListItem( GUI.switch_node_list_ids[ i ], node_id_more, UIEnums.FriendDemandModifer.GreaterThan )
		
		-- lock out switches
		local param_usage = FriendDemand.GetParamUsage( i - 1 )
		local num_switches = 0
		
		if param_usage == UIEnums.FriendDemandParamUse.LessThanMoreThan then
			UIButtons.SetNodeItemLocked( GUI.switch_node_list_ids[ i ], UIEnums.FriendDemandModifer.Equals, true, true )
			num_switches = 2
		
		elseif param_usage == UIEnums.FriendDemandParamUse.LessThanMoreThanEquals then
			num_switches = 3
			
		elseif param_usage == UIEnums.FriendDemandParamUse.EqualsMoreThan then
			UIButtons.SetNodeItemLocked( GUI.switch_node_list_ids[ i ], UIEnums.FriendDemandModifer.LessThan, true, true )
			num_switches = 2
		
		elseif param_usage == UIEnums.FriendDemandParamUse.LessThanEquals then
			UIButtons.SetNodeItemLocked( GUI.switch_node_list_ids[ i ], UIEnums.FriendDemandModifer.GreaterThan, true, true )
			num_switches = 2
		else
			-- none - equals only fail safe
			UIButtons.SetNodeItemLocked( GUI.switch_node_list_ids[ i ], UIEnums.FriendDemandModifer.LessThan, true, true )
			UIButtons.SetNodeItemLocked( GUI.switch_node_list_ids[ i ], UIEnums.FriendDemandModifer.GreaterThan, true, true )
			num_switches = 1
		end
		
		-- shift to centre
		local sx, _, _ = UIButtons.GetSize( node_id_less )
		local offset_x = ( ( num_switches - 1 ) / 2 ) * sx
		local px, py, pz = UIButtons.GetPosition( GUI.switch_node_list_ids[ i ] )
		UIButtons.ChangePosition( GUI.switch_node_list_ids[ i ], px - offset_x, py, pz )
		
		-- finished option
		UIButtons.AddListItem( GUI.node_list_id, node_id, i )
	end
	
	-- confirm
	local node_id = UIButtons.CloneXtGadgetByName( "Ingame\\CreateFriendDemand.lua", "_setup_node" )
	local box_id = SetupSelectionBox( node_id, UIText.FDE_CREATE_CHALLENGE, 5 )
	UIButtons.AddListItem( GUI.node_list_id, node_id, GUI.num_options + 1 )
	
	local _, sy, _ = UIButtons.GetSize( box_id )
	list_height = list_height + sy
	
	-- shift so we're in the middle
	local offset_y = -4
	local px, py, pz = UIButtons.GetPosition( GUI.node_list_id )
	UIButtons.ChangePosition( GUI.node_list_id, px, py + ( list_height / 2 ) + offset_y, pz )
	
	FdCreate_UpdateSwitches()
	FdCreate_SetupInfoLines()
	FdCreate_SetInitialSelections()
end

function StartLoop( time )
	--
end

function MessageUpdate( message, arg1, arg2, arg3, arg4 )
	if SCUI_MessageUpdate( message, arg1, arg2, arg3, arg4 ) == true then
		return
	end
	
	if GUI.choosing_switch == false then
		if message == UIEnums.Message.MenuNext then
			if GUI.current_option_selection == ( GUI.num_options + 1 ) then
				-- confirm
				PopScreen()
			else
				GUI.choosing_switch = true
				UIButtons.SetSelected( GUI.node_list_id, false )
				UIButtons.SetSelected( GUI.switch_node_list_ids[ GUI.current_option_selection ], true )
				UIButtons.PrivateTimeLineActive( GUI.switch_rb_ids[ GUI.current_option_selection ], "option_switch_on", true, 0 )
			end
			
			PlaySfxNext()
		
		elseif message == UIEnums.Message.MenuBack then
			PlaySfxBack()
			PopScreen()
		
		elseif message == UIEnums.Message.ButtonX then
			if UIGlobals.FriendDemandSwitchChosen > -1 then
				Fd_ClearExtraChallenge()				
				FdCreate_UpdateSwitches()
				FdCreate_SetupInfoLines()
				PlaySfxError()
			end
		end
		
	elseif GUI.choosing_switch == true then
		if message == UIEnums.Message.MenuNext or message == UIEnums.Message.MenuBack then
			GUI.choosing_switch = false
			UIButtons.SetSelected( GUI.node_list_id, true )
			UIButtons.SetSelected( GUI.switch_node_list_ids[ GUI.current_option_selection ], false )
			UIButtons.PrivateTimeLineActive( GUI.switch_rb_ids[ GUI.current_option_selection ], "option_switch_on", false )
			
			if message == UIEnums.Message.MenuNext then
				local switch_selection = UIButtons.GetSelection( GUI.switch_node_list_ids[ GUI.current_option_selection ] )
				
				-- clear current				
				for i = 1, GUI.num_options, 1 do
					FriendDemand.ChangeSwitch( i - 1, 0 )
				end
				
				-- set new
				FriendDemand.ChangeSwitch( GUI.current_option_selection - 1, switch_selection )
				UIGlobals.FriendDemandSwitchChosen = GUI.current_option_selection - 1
				
				FdCreate_UpdateSwitches()
				FdCreate_SetupInfoLines()
				PlaySfxNext()
			else
				PlaySfxBack()
			end
		end
	end	
end

function FrameUpdate( time )
	-- option list
	local current_option_selection = UIButtons.GetSelection( GUI.node_list_id )
	
	if current_option_selection ~= GUI.current_option_selection then
		GUI.current_option_selection = current_option_selection
		
		FdCreate_SetupInfoLines()
	end
	
	-- switch lists
	for i, v in ipairs( GUI.switch_node_list_ids ) do
		local current_switch_selection = UIButtons.GetSelection( v )
	
		if current_switch_selection ~= GUI.current_switch_selection[ i ] then
			GUI.current_switch_selection[ i ] = current_switch_selection
			
			-- update text			
			if current_switch_selection == UIEnums.FriendDemandModifer.LessThan then
				UIButtons.ChangeText( GUI.switch_text_ids[ i ], UIText.FDE_LESS_THAN )
				
			elseif current_switch_selection == UIEnums.FriendDemandModifer.GreaterThan then
				UIButtons.ChangeText( GUI.switch_text_ids[ i ], UIText.FDE_GREATER_THAN )
				
			elseif current_switch_selection == UIEnums.FriendDemandModifer.Equals then
				UIButtons.ChangeText( GUI.switch_text_ids[ i ], UIText.FDE_EQUALS )
			end
		end
	end
end

function EnterEnd()
	RestoreInfoLine()
	UIButtons.TimeLineActive( "stage1", false )
	UIGlobals.FdAddText_DoUpdate = true
	FriendDemand.Create()
	PlaySfxGraphicBack()
end

function EndLoop( time )
	--
end

function End()
	--
end

function FdCreate_UpdateSwitches()
	for i = 1, GUI.num_options, 1 do
		local switch_option = FriendDemand.GetSwitch( i - 1 )
		UIButtons.SetActive( GUI.switch_icon_ids[ i ], ( switch_option ~= UIEnums.FriendDemandModifer.NotUsed ) )
		
		if switch_option ~= UIEnums.FriendDemandModifer.NotUsed then
			local info = Fd_GetModifierIconInfo( switch_option )
			
			UIButtons.ChangeTexture( info.tex, 0, GUI.switch_icon_ids[ i ] )
			UIButtons.ChangeEffectIndex( GUI.switch_icon_ids[ i ], info.effect )
		end
	end
end

function FdCreate_SetupInfoLines()
	if GUI.current_option_selection == ( GUI.num_options + 1 ) then
		if UIGlobals.FriendDemandSwitchChosen > -1 then
			SetupInfoLine( UIText.INFO_A_CONFIRM, UIText.INFO_B_BACK, UIText.INFO_X_CLEAR_EXTRA_CHALLENGE )
		else
			SetupInfoLine( UIText.INFO_A_CONFIRM, UIText.INFO_B_BACK )
		end
	else
		if GUI.choosing_switch == true or ( GUI.choosing_switch == false and UIGlobals.FriendDemandSwitchChosen == -1 ) then
			SetupInfoLine( UIText.INFO_A_SELECT, UIText.INFO_B_BACK )
		else
			SetupInfoLine( UIText.INFO_A_SELECT, UIText.INFO_B_BACK, UIText.INFO_X_CLEAR_EXTRA_CHALLENGE )
		end
	end
end

function FdCreate_SetInitialSelections()
	if UIGlobals.FriendDemandSwitchChosen > -1 then
		UIButtons.SetSelection( GUI.node_list_id, UIGlobals.FriendDemandSwitchChosen + 1 )
	end
	
	for i = 1, GUI.num_options, 1 do
		local switch_option = FriendDemand.GetSwitch( i - 1 )
		
		if switch_option ~= UIEnums.FriendDemandModifer.NotUsed then
			UIButtons.SetSelection( GUI.switch_node_list_ids[ i ], switch_option )
		end
	end
end
