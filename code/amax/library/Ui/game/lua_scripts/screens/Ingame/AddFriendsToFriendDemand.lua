GUI =
{
	finished = false,
	
	node_list_id = -1,
	node_list_friends_id = -1,
	
	current_option_selection = -1,
	current_friend_selection = -1,
	
	friend_text_ids = {  },
	friend_icon_ids = {  },
	friend_list = {  },
	friend_list_text_ids = {  },
	box_ids = {  },
	
	choosing_friend = false,
	num_friends_chosen = 0,
}

function Init()
	AddSCUI_Elements()
	SetupInfoLine()
	
	GUI.camera_id = UIButtons.CloneXtGadgetByName( "SCUIBank", "Cam_Carousel" )
	UIButtons.SetParent( GUI.camera_id, SCUI.name_to_id[ "CameraDolly" ], UIEnums.Justify.MiddleCentre )
	
	GUI.friend_list[ 1 ] = UIGlobals.FriendDemandFriends[ 1 ]
	GUI.friend_list[ 2 ] = UIGlobals.FriendDemandFriends[ 2 ]
	GUI.friend_list[ 3 ] = UIGlobals.FriendDemandFriends[ 3 ]
end

function PostInit()
	local screen_id = SCUI.name_to_id[ "screen" ]
	local screen_friends_id = SCUI.name_to_id[ "screen_friends" ]
	
	SetupScreenTitle( UIText.FDE_FRIEND_DEMAND, screen_id, "message", "common_icons", nil, UIEnums.Justify.TopCentre, true, nil, UIEnums.Panel._3DAA_LIGHT, nil, UIEnums.Justify.TopCentre )
	local text_id, root_id = SetupBottomHelpBar( UIText.FDE_HELP_CHOOSE_FRIENDS, nil, nil, true )
	GUI.bottom_bar_add_id = root_id
	
	SetupScreenTitle( UIText.FDE_FRIEND_DEMAND, screen_friends_id, "message", "common_icons", nil, UIEnums.Justify.TopCentre, true, nil, UIEnums.Panel._3DAA_LIGHT, nil, UIEnums.Justify.TopCentre )
	local text_id, root_id = SetupBottomHelpBar( UIText.FDE_HELP_CHOOSE_FRIEND, nil, nil, true )
	GUI.bottom_bar_choose_id = root_id
	
	UIButtons.PrivateTimeLineActive( GUI.bottom_bar_choose_id, "Hide_BottomHelp", true, 1.0, true )
	
	-- add options
	GUI.node_list_id = SCUI.name_to_id[ "node_list_options" ]
	
	-- friends
	for i = 1, 3, 1 do
		local node_id = UIButtons.CloneXtGadgetByName( "Ingame\\AddFriendsToFriendDemand.lua", "_friend_node" )
		local dummy = UIButtons.FindChildByName( node_id, "_friend_dummy" )
		
		if i == 3 then
			local sx, sy, sz = UIButtons.GetSize( node_id )
			UIButtons.ChangeSize( node_id, sx, sy + 3, sz )
		end
		
		local box_id, text_id = SetupSelectionBox( dummy, UIText.FDE_FRIEND_SLOT_EMPTY, nil, nil, nil, nil, UIEnums.Justify.MiddleLeft, true )
		
		GUI.box_ids[ i ] = box_id
		GUI.friend_text_ids[ i ] = text_id
		GUI.friend_icon_ids[ i ] = UIButtons.FindChildByName( node_id, "_friend_pic" )
		
		if GUI.friend_list[ i ] ~= nil then
			UIButtons.ChangeText( GUI.friend_text_ids[ i ], "GAME_FRIEND_DEMAND_FRIEND_" .. GUI.friend_list[ i ] )
		end
		
		UIButtons.AddListItem( GUI.node_list_id, node_id, i )
	end
	
	-- confirm
	local node_id = UIButtons.CloneXtGadgetByName( "Ingame\\AddFriendsToFriendDemand.lua", "_setup_node" )
	GUI.box_ids[ 4 ] = SetupSelectionBox( node_id, UIText.FDE_CREATE_CHALLENGE )
	UIButtons.AddListItem( GUI.node_list_id, node_id, 4 )
	
	-- stencil read on
	local stencil_read_on = UIButtons.AddButton( SCUI.elements[ SCUI.name_to_index[ "_friends_stencil_read_on" ] ] )
	UIButtons.SetParent( stencil_read_on, screen_friends_id, UIEnums.Justify.MiddleCentre )
	
	-- add friends list
	GUI.node_list_friends_id = UIButtons.AddButton( SCUI.elements[ SCUI.name_to_index[ "_node_list_friends" ] ] )
	UIButtons.SetParent( GUI.node_list_friends_id, screen_friends_id, UIEnums.Justify.MiddleCentre )
	
	for i = 1, Amax.GetNumFriends(), 1 do
		local node_friend_id = UIButtons.CloneXtGadgetByName( "Ingame\\AddFriendsToFriendDemand.lua", "_friend_list_node" )
		
		GUI.friend_list_text_ids[ i ] = UIButtons.FindChildByName( node_friend_id, "_friend_list_name" )
		
		local id=Amax.GetFriendsIDFromIndex(i-1)
		
		UIButtons.ChangeText( GUI.friend_list_text_ids[ i ], "GAME_FRIEND_DEMAND_FRIEND_" .. id )
		
		UIButtons.AddListItem( GUI.node_list_friends_id, node_friend_id, id )
	end
	
	-- stencil read off
	local stencil_read_off = UIButtons.AddButton( SCUI.elements[ SCUI.name_to_index[ "_friends_stencil_read_off" ] ] )
	UIButtons.SetParent( stencil_read_off, screen_friends_id, UIEnums.Justify.MiddleCentre )
	
	-- all unlocked by default
	for i = 1, Amax.GetNumFriends(), 1 do
	
		local id=Amax.GetFriendsIDFromIndex(i-1)
		
		UIButtons.SetNodeItemLocked( GUI.node_list_friends_id, id, false )
	end

	-- friends off by default
	UIButtons.SetSelected( GUI.node_list_friends_id, false )
	
	FdAddFriends_SetupInfoLines()
	FdAddFriends_UpdateGamerpics()
end

function StartLoop( time )
	--
end

function MessageUpdate( message, arg1, arg2, arg3, arg4 )
	if SCUI_MessageUpdate( message, arg1, arg2, arg3, arg4 ) == true then
		return
	end
	
	if message == UIEnums.MiscMessage.FriendsUpdated then
		for i=1,3,1 do
			UIGlobals.FriendDemandFriends[ i ] = nil
			if GUI.friend_list[ i ] ~= nil then
				if FriendDemand.IsFriend(GUI.friend_list[ i ]) == true then
					UIGlobals.FriendDemandFriends[ i ] = GUI.friend_list[ i ]
				end
			end
		end
		
		GoScreen("Ingame\\AddFriendsToFriendDemand.lua")
	end
	
	if message == UIEnums.Message.MenuNext then
		if GUI.choosing_friend == false then
			if GUI.current_option_selection == 4 then
				-- confirm
				if GUI.friend_list[ 1 ] ~= nil or GUI.friend_list[ 2 ] ~= nil or GUI.friend_list[ 3 ] ~= nil then
					-- set friends
					UIGlobals.FriendDemandFriends[ 1 ] = GUI.friend_list[ 1 ]
					UIGlobals.FriendDemandFriends[ 2 ] = GUI.friend_list[ 2 ]
					UIGlobals.FriendDemandFriends[ 3 ] = GUI.friend_list[ 3 ]
					
					FriendDemand.Create()
					GoScreen( "Ingame\\AddTextToFriendDemand.lua" )
					PlaySfxNext()
				else
					SetupCustomPopup( UIEnums.CustomPopups.FriendDemandNoFriendsChosen )
				end
			else
				-- add/change friend
				local num_friends = Amax.GetNumFriends()
				local num_friends_in_list = num_friends
				
				for i = 0, num_friends - 1, 1 do
					
					local id=Amax.GetFriendsIDFromIndex(i)
				
					UIButtons.SetNodeItemLocked( GUI.node_list_friends_id, id, false, true )
					
					if id == GUI.friend_list[ 1 ] or id == GUI.friend_list[ 2 ] or id == GUI.friend_list[ 3 ] then
						UIButtons.ChangeColour( GUI.friend_list_text_ids[ i + 1 ], "Main_Black" )
						UIButtons.SetNodeItemLocked( GUI.node_list_friends_id, id, true, true )
						num_friends_in_list = num_friends_in_list - 1
					else
						UIButtons.ChangeColour( GUI.friend_list_text_ids[ i + 1 ], "Main_White" )
					end
				end
				
				if GUI.num_friends_chosen < num_friends and num_friends_in_list > 0 then
					UIButtons.PrivateTimeLineActive( GUI.bottom_bar_add_id, "Hide_BottomHelp", true )
					UIButtons.PrivateTimeLineActive( GUI.bottom_bar_choose_id, "Hide_BottomHelp", false )
					
					UIButtons.SetSelected( GUI.node_list_friends_id, true )
					UIButtons.SetSelected( GUI.node_list_id, false )
					
					GUI.choosing_friend = true
					UIButtons.TimeLineActive( "stage1", true )
					FdAddFriends_SetupInfoLines()
					
					PlaySfxNext()
					PlaySfxGraphicNext()
				else
					PlaySfxError()
				end
			end
		else
			PlaySfxNext()
			PlaySfxGraphicBack()
			
			-- confirm friend
			local friend_index = UIButtons.GetSelection( GUI.node_list_friends_id )
			
			-- clear current gamerpic before adding new
			Profile.ClearRemotePictureByRef( GUI.current_option_selection - 1 )
			
			FriendDemand.AddFriend( GUI.current_option_selection - 1, friend_index )
			
			if GUI.friend_list[ GUI.current_option_selection ] == nil then	-- new friend
				GUI.num_friends_chosen = GUI.num_friends_chosen + 1
			end
			
			GUI.friend_list[ GUI.current_option_selection ] = friend_index

			UIButtons.SetSelected( GUI.node_list_friends_id, false )
			UIButtons.SetSelected( GUI.node_list_id, true )
			
			UIButtons.ChangeText( GUI.friend_text_ids[ GUI.current_option_selection ], "GAME_FRIEND_DEMAND_FRIEND_" .. GUI.friend_list[ GUI.current_option_selection ] )
			UIButtons.PrivateTimeLineActive( GUI.bottom_bar_add_id, "Hide_BottomHelp", false )
			UIButtons.PrivateTimeLineActive( GUI.bottom_bar_choose_id, "Hide_BottomHelp", true )
			
			GUI.choosing_friend = false
			UIButtons.TimeLineActive( "stage1", false )
			FdAddFriends_SetupInfoLines()
			FdAddFriends_UpdateGamerpics()
		end
		
	elseif message == UIEnums.Message.MenuBack then
		if GUI.choosing_friend == true then
			UIButtons.SetSelected( GUI.node_list_friends_id, false )
			UIButtons.SetSelected( GUI.node_list_id, true )
			
			UIButtons.PrivateTimeLineActive( GUI.bottom_bar_add_id, "Hide_BottomHelp", false )
			UIButtons.PrivateTimeLineActive( GUI.bottom_bar_choose_id, "Hide_BottomHelp", true )
			
			GUI.choosing_friend = false
			UIButtons.TimeLineActive( "stage1", false )
			FdAddFriends_SetupInfoLines()
			PlaySfxGraphicBack()
		else
			GoScreen( GetStoredScreen( UIEnums.ScreenStorage.FE_FRIEND_DEMAND ) )
		end
		
		PlaySfxBack()
		
	elseif message == UIEnums.Message.ButtonX then
		-- remove
		if GUI.choosing_friend == false and GUI.current_option_selection ~= 4 then
			FriendDemand.AddFriend( GUI.current_option_selection - 1, -1 )
			GUI.friend_list[ GUI.current_option_selection ] = nil
			
			UIButtons.ChangeText( GUI.friend_text_ids[ GUI.current_option_selection ], UIText.FDE_FRIEND_SLOT_EMPTY )
			GUI.num_friends_chosen = GUI.num_friends_chosen - 1
			FdAddFriends_SetupInfoLines()
			FdAddFriends_UpdateGamerpics()
			PlaySfxError()
		end
		
	elseif message == UIEnums.Message.ButtonY then
		-- gamercard
		if UIEnums.CurrentPlatform == UIEnums.Platform.Xenon then
			if GUI.choosing_friend == false and GUI.current_option_selection ~= 4 then
				if GUI.friend_list[ GUI.current_option_selection ] ~= nil then
					FriendDemand.ShowFriendGamerCard( GUI.friend_list[ GUI.current_option_selection ] )
				end

			elseif GUI.choosing_friend == true then
				local friend_index = UIButtons.GetSelection( GUI.node_list_friends_id )
				FriendDemand.ShowFriendGamerCard( friend_index )
			end
		end
	end
end

function FrameUpdate( time )
	local current_option_selection = UIButtons.GetSelection( GUI.node_list_id )
	
	if current_option_selection ~= GUI.current_option_selection then
		if GUI.current_option_selection ~= -1 then
			UIButtons.PrivateTimeLineActive( GUI.box_ids[ GUI.current_option_selection ], "flash", false )
		end
		
		GUI.current_option_selection = current_option_selection
		
		UIButtons.PrivateTimeLineActive( GUI.box_ids[ GUI.current_option_selection ], "flash", true, 0 )
		
		FdAddFriends_SetupInfoLines()
	end
	
	local current_friend_selection = UIButtons.GetSelectionIndex( GUI.node_list_friends_id )
	
	if current_friend_selection ~= GUI.current_friend_selection then
		UIButtons.PrivateTimeLineActive( SCUI.name_to_id[ "arrow_top" ], "more", ( current_friend_selection ~= 0 ) )
		UIButtons.PrivateTimeLineActive( SCUI.name_to_id[ "arrow_top" ], "push", ( current_friend_selection < GUI.current_friend_selection ), 0 )
		UIButtons.PrivateTimeLineActive( SCUI.name_to_id[ "arrow_bottom" ], "more", ( current_friend_selection ~= ( Amax.GetNumFriends() - 1 ) ) )
		UIButtons.PrivateTimeLineActive( SCUI.name_to_id[ "arrow_bottom" ], "push", ( current_friend_selection > GUI.current_friend_selection ), 0 )
		
		GUI.current_friend_selection = current_friend_selection
	end
end

function EnterEnd()
	--
end

function EndLoop( time )
	--
end

function End()
	--
end

function FdAddFriends_SetupInfoLines()
	if GUI.current_option_selection == 4 then
		-- confirm
		if GUI.friend_list[ 1 ] ~= nil or GUI.friend_list[ 2 ] ~= nil or GUI.friend_list[ 3 ] ~= nil then
			SetupInfoLine( UIText.INFO_A_CONTINUE, UIText.INFO_B_BACK )
		else
			SetupInfoLine( UIText.INFO_B_BACK )
		end
	else
		if GUI.choosing_friend == true then
			SetupInfoLine( UIText.INFO_A_ADD_FRIEND, UIText.INFO_B_BACK, Select( ( UIEnums.CurrentPlatform == UIEnums.Platform.Xenon ), UIText.INFO_Y_VIEW_GAMER_CARD, nil ) )
		else
			if GUI.friend_list[ GUI.current_option_selection ] ~= nil then
				-- add/change friend
				local num_friends = Amax.GetNumFriends()
				
				for i = 1, 3, 1 do
					if GUI.friend_list[ i ] ~= nil then
						num_friends = num_friends - 1
					end
				end
				
				if num_friends > 0 then
					SetupInfoLine( UIText.INFO_A_CHANGE_FRIEND, UIText.INFO_B_BACK, UIText.INFO_REMOVE_FRIEND, Select( ( UIEnums.CurrentPlatform == UIEnums.Platform.Xenon ), UIText.INFO_Y_VIEW_GAMER_CARD, nil ) )
				else
					SetupInfoLine( UIText.INFO_B_BACK, UIText.INFO_REMOVE_FRIEND, Select( ( UIEnums.CurrentPlatform == UIEnums.Platform.Xenon ), UIText.INFO_Y_VIEW_GAMER_CARD, nil ) )
				end
			else
				local num_friends = Amax.GetNumFriends()
				local num_friends_selected = 0
				
				for i = 1, 3, 1 do
					if GUI.friend_list[ i ] ~= nil then
						num_friends_selected = num_friends_selected + 1
					end
				end
				
				if num_friends_selected < num_friends then
					SetupInfoLine( UIText.INFO_A_ADD_FRIEND, UIText.INFO_B_BACK )
				else
					SetupInfoLine( UIText.INFO_B_BACK )
				end
			end
		end
	end
end

function FdAddFriends_UpdateGamerpics()
	local map = Profile.GetRemoteGamerPictureMap()
	
	for i, v in ipairs( GUI.friend_icon_ids ) do
		if GUI.friend_list[ i ] ~= nil and map[ i - 1 ] ~= nil then
			UIButtons.ChangeTexture( { filename = "REMOTE_GAMERPIC_" .. map[ i - 1 ] }, 1, v )
		else
			UIButtons.ChangeTexture( { filename = "default_gamerpic" }, 1, v )
		end
	end
end
