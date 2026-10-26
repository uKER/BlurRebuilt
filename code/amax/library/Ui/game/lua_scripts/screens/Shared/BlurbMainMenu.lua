GUI =
{
	finished = false,	
	carousel_branch = "Blurb",
	
	node_list_id = -1,
	
	node_ids = {  },
	pic_ids = {  },
	
	current_selection = -1,
	num_items = -1,
	
	current_sort = -1,
	
	pic_wait_time = 0.3,
	pic_wait_timer = 0,
	
	pic_requested = {  },
	
	CanExit = function( _GUI )
		PlaySfxGraphicBack()
		return true
	end,
}

function Init()
	AddSCUI_Elements()
	Amax.ChangeUiCamera( "Sp_2", UIGlobals.CameraLerpTime, 0 )

	CarouselApp_SetScreenTimers()
	net_SetRichPresence( UIEnums.RichPresence.FriendChallenges )
	
	StoreInfoLine()
	
	if FriendDemand.HasOldChallenges() == 0 then
		SetupInfoLine( UIText.INFO_A_SELECT, UIText.INFO_B_BACK, UIText.INFO_X_SORT )
	else
		SetupInfoLine( UIText.INFO_A_SELECT, UIText.INFO_B_BACK, UIText.INFO_X_SORT, UIText.INFO_Y_REMOVE_OLD_CHALLENGES )
	end
end

function PostInit()
	local screen_id = SCUI.name_to_id[ "screen" ]
	
	if UIGlobals.FriendDemandCurrentSort ~= nil and UIGlobals.FriendDemandCurrentSort ~= -1 then
		GUI.current_sort = UIGlobals.FriendDemandCurrentSort
	else
		GUI.current_sort = UIEnums.FriendDemandSort.New
	end
	
	SetupScreenTitle( UIText.FDE_FRIEND_DEMANDS, screen_id, "message", "common_icons", 1, UIEnums.Justify.TopCentre, nil, nil, UIEnums.Panel._3DAA_WORLD, nil, UIEnums.Justify.TopCentre )
	GUI.help_text_id = SetupBottomHelpBar( UIText.FDE_INBOX_HELP, nil, 1, nil )
	
	local friends_sorted = FriendDemand.SortFriends( GUI.current_sort )
	GUI.num_items = #friends_sorted
	
	local friends_index_to_id = {  }

	GUI.node_list_id = SCUI.name_to_id[ "node_list" ]
	
	for i, v in ipairs( friends_sorted ) do
		local node_id = UIButtons.CloneXtGadgetByName( "SpSCUI", "friend_challenge_node" )
		
		local name_id = UIButtons.FindChildByName( node_id, "friend_name" )
		UIButtons.ChangeText( name_id, "GAME_FRIEND_DEMAND_FRIEND_" .. v )
		
		local new_id = UIButtons.FindChildByName( node_id, "friend_text_new_info" )
		UIButtons.ChangeText( new_id, "GAME_FRIEND_DEMAND_NEW_COUNT_" .. v )
		
		local accept_id = UIButtons.FindChildByName( node_id, "friend_text_accept_info" )
		UIButtons.ChangeText( accept_id, "GAME_FRIEND_DEMAND_ACTIVE_COUNT_" .. v )
		
		UIButtons.AddListItem( GUI.node_list_id, node_id, v )
		
		GUI.node_ids[ v ] = node_id
		GUI.pic_ids[ v ] = UIButtons.FindChildByName( node_id, "friend_pic" )
	end
	
	local friend_selection = UIGlobals.FriendDemandFilterFriend
	BlurbMainMenu_NextSort( true )
	UIButtons.SetSelection( GUI.node_list_id, friend_selection )
	
	ContextTable[ UIScreen.Context() ].FrameUpdate( 0 )	-- yeah
	
	BlurbMainMenu_InitialiseGamerpics()
	BlurbMainMenu_RequestGamerpics()
	BlurbMainMenu_UpdateGamerpics()
	
	UIGlobals.FriendDemandDoNotRefresh = true
	SetupCustomPopup( UIEnums.CustomPopups.FriendDemandRetrieving )
end

function StartLoop( time )
	--
end

function MessageUpdate( message, arg1, arg2, arg3, arg4 )
	if SCUI_MessageUpdate( message, arg1, arg2, arg3, arg4 ) == true then
		return
	end

	if message == UIEnums.MiscMessage.FriendsUpdated then
		if Amax.GetNumFriends() == 0 then
			Amax.ChangeUiCamera( UIGlobals.CameraNames.SpCarousel, 1, 0 )
			UIButtons.TimeLineActive( "Hide_Carousel", false )
			CloseCurrentApp()
		else
			GoScreen( "Shared\\BlurbMainMenu.lua" )	-- just full refresh
		end
		
	elseif message == UIEnums.Message.PopupNext and arg2 == UIEnums.CustomPopups.FriendDemandRemoveOldChallenges then
	
		SetupInfoLine( UIText.INFO_A_SELECT, UIText.INFO_B_BACK, UIText.INFO_X_SORT )
	
	elseif message == UIEnums.Message.PopupNext and arg2 == UIEnums.CustomPopups.FriendDemandInboxOptions then
		if arg3 == 0 then	-- new
			UIGlobals.FriendDemandFilter = 2
			UIGlobals.FriendDemandFilterFriend = GUI.current_selection
			
			GoScreen( "Shared\\Blurb.lua" )
			BlurbMainMenu_RequestGamerpics()	-- make sure we're up-to-date
			
		elseif arg3 == 1 then	-- accepted
			UIGlobals.FriendDemandFilter = 1
			UIGlobals.FriendDemandFilterFriend = GUI.current_selection
			
			GoScreen( "Shared\\Blurb.lua" )
			BlurbMainMenu_RequestGamerpics()	-- make sure we're up-to-date
			
		elseif arg3 == 2 then	-- stats
			UIGlobals.FriendDemandFilterFriend = GUI.current_selection
			FriendDemand.GetTaleOfTheTape( UIGlobals.FriendDemandFilterFriend )

			GoScreen( "Shared\\BlurbStats.lua" )
			BlurbMainMenu_RequestGamerpics()	-- make sure we're up-to-date
		end
	end
	
	-- [Bone] #35640 - was able to press back here when controller was locked
	if IsControllerLocked() == true then
		return
	end

	-- Handle pad messages
	if message == UIEnums.Message.MenuNext then
		UIGlobals.FriendDemandFilterFriend = GUI.current_selection
		SetupCustomPopup( UIEnums.CustomPopups.FriendDemandInboxOptions )
		
	elseif message == UIEnums.Message.ButtonX then
		UISystem.PlaySound( UIEnums.SoundEffect.Filter )
		BlurbMainMenu_NextSort()
		
	elseif message == UIEnums.Message.ButtonY then
	
		if FriendDemand.HasOldChallenges() > 0 then
			SetupCustomPopup( UIEnums.CustomPopups.FriendDemandRemoveOldChallenges )
		end
	end
	
	--[[		
	elseif message == UIEnums.Message.ButtonRightShoulder then
		local current_index = UIButtons.GetSelectionIndex( GUI.node_list_id )
		local new_index = Clamp( current_index + 5, 0, GUI.num_items - 1 )
		UIButtons.SetSelectionByIndex( GUI.node_list_id, new_index )
		
		if current_index ~= new_index then
			PlaySfxRight()
		end
	
	elseif message == UIEnums.Message.ButtonLeftShoulder then
		local current_index = UIButtons.GetSelectionIndex( GUI.node_list_id )
		local new_index = Clamp( current_index - 5, 0, GUI.num_items - 1 )
		UIButtons.SetSelectionByIndex( GUI.node_list_id, new_index )
		
		if current_index ~= new_index then
			PlaySfxLeft()
		end
	end
	--]]
end

function FrameUpdate( time )
	local current_selection = UIButtons.GetSelection( GUI.node_list_id )
	
	if current_selection ~= GUI.current_selection then
		if GUI.current_selection ~= -1 then
			UIButtons.PrivateTimeLineActive( GUI.node_ids[ GUI.current_selection ], "flash", false )
		end
		
		GUI.current_selection = current_selection
		
		UIButtons.PrivateTimeLineActive( GUI.node_ids[ GUI.current_selection ], "flash", true, 0 )
		
		GUI.pic_wait_timer = 0	-- update gamerpics
	end
	
	-- check for gamerpic refresh
	if GUI.pic_wait_timer < GUI.pic_wait_time then
		GUI.pic_wait_timer = GUI.pic_wait_timer + time
		
		if GUI.pic_wait_timer > GUI.pic_wait_time then
			BlurbMainMenu_RequestGamerpics()
			BlurbMainMenu_UpdateGamerpics()
		end
	end
	
	if Amax.GetNumFriends() == 0 then
		Amax.ChangeUiCamera( UIGlobals.CameraNames.SpCarousel, 1, 0 )
		UIButtons.TimeLineActive( "Hide_Carousel", false )
		CloseCurrentApp()
	end
end

function EnterEnd()
	UIGlobals.FriendDemandDoNotRefresh = false
	UIGlobals.FriendDemandCurrentSort = GUI.current_sort
	RestoreInfoLine()	
end

function EndLoop( time )
	--
end

function End()	
	-- omgz. this is bad, but it's late and I don't know how to fix it up. will sort out later. heh
	UIScreen.CancelPopup()
	 --
end

function BlurbMainMenu_NextSort( initial )
	local sort_text_id = SCUI.name_to_id[ "sorting_info" ]
	
	if initial ~= true then
		if GUI.current_sort == UIEnums.FriendDemandSort.New then
			GUI.current_sort = UIEnums.FriendDemandSort.Active
			
		elseif GUI.current_sort == UIEnums.FriendDemandSort.Active then
			GUI.current_sort = UIEnums.FriendDemandSort.Alpha
			
		elseif GUI.current_sort == UIEnums.FriendDemandSort.Alpha then
			GUI.current_sort = UIEnums.FriendDemandSort.New
		end
	end
	
	-- update text
	if GUI.current_sort == UIEnums.FriendDemandSort.New then
		UIButtons.ChangeText( sort_text_id, UIText.FDE_SORTING_NEW_CHALLENGES )
		
	elseif GUI.current_sort == UIEnums.FriendDemandSort.Active then
		UIButtons.ChangeText( sort_text_id, UIText.FDE_SORTING_ACTIVE_CHALLENGES )
		
	elseif GUI.current_sort == UIEnums.FriendDemandSort.Alpha then
		UIButtons.ChangeText( sort_text_id, UIText.FDE_SORTING_ALPHA )
	end
	
	-- update sort keys ...
	local friends_sorted = FriendDemand.SortFriends( GUI.current_sort )
	GUI.num_items = #friends_sorted
	
	for i, v in ipairs( friends_sorted ) do
		UIButtons.SetNodeSortKey( GUI.node_list_id, v, i, true )
	end
	
	-- reset this
	UIGlobals.FriendDemandFilterFriend = friends_sorted[ 1 ]
	
	-- ... and re-sort
	UIButtons.NodeListSort( GUI.node_list_id )
	
	-- set default back to friend
	if UIGlobals.FriendDemandFilterFriend == 0 then
		UIButtons.SetSelectionByIndex( GUI.node_list_id, 0 )
	else
		UIButtons.SetSelection( GUI.node_list_id, UIGlobals.FriendDemandFilterFriend )
	end
	
	UIButtons.PrivateTimeLineActive( GUI.node_list_id, "new_sort", true, 0 )
end

function BlurbMainMenu_InitialiseGamerpics()
	-- make sure we keep ones we've already got
	local map = Profile.GetRemoteGamerPictureMap()
	
	for i, v in pairs( map ) do
		GUI.pic_requested[ i ] = true
	end
end

function BlurbMainMenu_RequestGamerpics()
	if GUI.pic_requested[ GUI.current_selection ] == nil then
		local count = 0
		local first = nil
		
		for i, v in pairs( GUI.pic_requested ) do
			count = count + 1
			
			if first == nil then
				first = i
			end
		end
		
		if count == 19 then
			-- full - just ditch the first - this might need to be a bit more clever - probably want to make sure we ditch one offscreen
			Profile.ClearRemotePictureByRef( first )
			GUI.pic_requested[ first ] = nil
		end
		
		FriendDemand.RequestRemotePicture( GUI.current_selection, GUI.current_selection )
		
		GUI.pic_requested[ GUI.current_selection ] = true
	end
end

function BlurbMainMenu_UpdateGamerpics()
	local map = Profile.GetRemoteGamerPictureMap()

	for i, v in pairs( GUI.pic_ids ) do
		if map[ i ] ~= nil then
			UIButtons.ChangeTexture( { filename = "REMOTE_GAMERPIC_" .. map[ i ] }, 1, v )
		else
			UIButtons.ChangeTexture( { filename = "default_gamerpic" }, 1, v )
		end
	end
end
