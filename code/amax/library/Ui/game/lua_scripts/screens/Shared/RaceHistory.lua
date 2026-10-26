GUI =
{
	finished = false,	
	carousel_branch = "RaceHistory",

	save_on_exit = false,
	
	dummy_id = 0,
	
	index = 1,
	grid_index = 0,
	num_rows = 20,
	
	node_list_id = 0,
	node_ids = {},
	award_node_ids = {},
	
	race_history = {},
	
	showing_awards = false,
	
	CanExit = function( _GUI )
		return false
	end,
	
	award_index = 0,
}

function Init()
	PlaySfxGraphicNext()
	
	AddSCUI_Elements()
	DeferCam_Init( UIGlobals.CameraNames.MpLobby )

	CarouselApp_SetScreenTimers()
	
	net_SetRichPresence( UIEnums.RichPresence.RaceHistory )
	
	StoreInfoLine()
		
	UIButtons.TimeLineActive( "garage_start", true )
	
	GUI.race_history = Multiplayer.GetRaceHistory()
	
	local history_count = 0
	for i = 1, 10 do
		if GUI.race_history[i].num_cars > 0 then
			history_count = history_count + 1
		end
	end

	UIButtons.SetActive( SCUI.name_to_id["award_left_arrow"], false )
	UIButtons.SetActive( SCUI.name_to_id["award_right_arrow"], false )

	if history_count == 0 then
		SetupCustomPopup( UIEnums.CustomPopups.NoHistory )
	elseif history_count == 1 then
		UIButtons.SetActive( SCUI.name_to_id["left_arrow"], false )
		UIButtons.SetActive( SCUI.name_to_id["right_arrow"], false )
	end	
	
end

function PostInit()
	GUI.dummy_id = SCUI.name_to_id["screen"]
	
	-- set up table titles
	local node_list_title_id = SCUI.name_to_id[ "node_list_title" ]
	GUI.node_title_id = UIButtons.CloneXtGadgetByName( "Shared\\RaceHistory.lua", "_node_title" )
	UIButtons.AddListItem( node_list_title_id, GUI.node_title_id, 1 )
	
	local name_title_id = UIButtons.FindChildByName( GUI.node_title_id, "_text_title_gamertag" )
	UIButtons.ChangeText( name_title_id, UIText.MP_SCOREBOARD_NAME )
	
	-- set up table entry nodes
	GUI.node_list_id = UIButtons.FindChildByName( GUI.dummy_id, "node_list" )
	
	for i = 1, GUI.num_rows do
		GUI.node_ids[i] = UIButtons.CloneXtGadgetByName( "Shared\\RaceHistory.lua", "_node_player" )
		UIButtons.SetActive( UIButtons.FindChildByName( GUI.node_ids[i], "_background_box" ), (i%2) == 1 )
		UIButtons.AddListItem( GUI.node_list_id, GUI.node_ids[i] )
	end
	
	GUI.award_list_id = SCUI.name_to_id["awards_node_list"]
	for i=1, 5 do
		GUI.award_node_ids[i] = InitAward()
		UIButtons.AddListItem( GUI.award_list_id, GUI.award_node_ids[i] )
	end
	
	BuildTable(GUI.index)
		
	RaceHistory_SetupInfoLines()	
end

function MessageUpdate( message, arg1, arg2, arg3, arg4 )
	if SCUI_MessageUpdate( message, arg1, arg2, arg3, arg4 ) == true then
		return
	end
	
	if GUI.finished == true then
		return
	end
	
	if message == UIEnums.Message.MenuNext then
		if NetServices.CanViewGamerCard( Profile.GetPrimaryPad() ) == true and GUI.showing_awards == false then
			NetServices.ViewRaceSummaryGamerCard( Profile.GetPrimaryPad(), SelectedPlayerIndex(), GUI.race_history[GUI.index].index )			
		end
	
	elseif message == UIEnums.Message.MenuBack then
		if GUI.showing_awards == true then

			UIButtons.SetActive( SCUI.name_to_id["award_left_arrow"], false )
			UIButtons.SetActive( SCUI.name_to_id["award_right_arrow"], false )

			UIButtons.TimeLineActive( "hide_table", false )
			for i=1,5 do
				UIButtons.PrivateTimeLineActive( GUI.award_node_ids[i], "activate_award", false )
			end
			
			RaceHistory_SetupInfoLines()
			
		else
			GoScreen( "Multiplayer\\MpOnline.lua" )
		end
		
		PlaySfxBack()
		
		UIButtons.SetActive( GUI.node_list_id, true )
		GUI.showing_awards = false

	elseif message == UIEnums.Message.ButtonX then
		
		if GUI.showing_awards == false and GUI.race_history[GUI.index].num_awards > 0 then
		
			GUI.award_index=0
			
			SetupAwards( GUI.index )
			UIButtons.TimeLineActive( "hide_table", true )
			
			local options = {  }

			options[ #options + 1 ] = UIText.INFO_B_BACK
			
			if Amax.CanUseShare() == true then
				options[ #options + 1 ] = "GAME_SHARE_BUTTON"
			end
				
			SetupInfoLine( unpack( options ) )
			
			UIButtons.SetActive( GUI.node_list_id, false )
			GUI.showing_awards = true
			
			for i=1,5 do
				UIButtons.PrivateTimeLineActive( GUI.award_node_ids[i], "chose_award", false )
			end
			
			if GUI.race_history[GUI.index].num_awards > 1 then
				UIButtons.SetActive( SCUI.name_to_id["award_left_arrow"], true )
				UIButtons.SetActive( SCUI.name_to_id["award_right_arrow"], true )

				UIButtons.PrivateTimeLineActive( GUI.award_node_ids[GUI.award_index + 1], "chose_award", true )
			end
			
			PlaySfxNext()
		end
		
	elseif message == UIEnums.Message.ButtonLeftShoulder and arg2 == true then

		UIGlobals.ShareFromWhatPopup=-1
		
		if Amax.CanUseShare() == true then
			SetupCustomPopup( UIEnums.CustomPopups.SharingOptions )
		end
		
	elseif message == UIEnums.Message.ButtonLeft and arg2 == true then
		if GUI.showing_awards == true then
		
			for i=1,5 do
				UIButtons.PrivateTimeLineActive( GUI.award_node_ids[i], "chose_award", false )
			end
		
			GUI.award_index=GUI.award_index-1
			if GUI.award_index == -1 then
				GUI.award_index=GUI.race_history[GUI.index].num_awards-1
			end
			
			if GUI.race_history[GUI.index].num_awards > 1 then
				UIButtons.PrivateTimeLineActive( GUI.award_node_ids[GUI.award_index + 1], "chose_award", true )
			end
			
			UIButtons.TimeLineActive( "move_award_left", true, 0 )
			
			PlaySfxLeft()

			return
		end
		
		repeat
			GUI.index=GUI.index-1
			
			if GUI.index == 0 then
				GUI.index=10
			end
		until GUI.race_history[GUI.index].num_cars > 0
		
		BuildTable(GUI.index)
		
		if UIButtons.GetSelectionIndex( GUI.node_list_id ) > GUI.race_history[GUI.index].num_cars - 1 then
			UIButtons.SetSelectionByIndex( GUI.node_list_id, GUI.race_history[GUI.index].num_cars - 1 )
		end
		
		RaceHistory_SetupInfoLines()
		
		UIButtons.TimeLineActive( "move_left", true, 0 )
		
		PlaySfxLeft()

	elseif message == UIEnums.Message.ButtonRight and arg2 == true then
		if GUI.showing_awards == true then

			for i=1,5 do
				UIButtons.PrivateTimeLineActive( GUI.award_node_ids[i], "chose_award", false )
			end

			GUI.award_index=GUI.award_index+1
			if GUI.award_index == GUI.race_history[GUI.index].num_awards then
				GUI.award_index=0
			end
			
			if GUI.race_history[GUI.index].num_awards > 1 then
				UIButtons.PrivateTimeLineActive( GUI.award_node_ids[GUI.award_index + 1], "chose_award", true )
			end

			UIButtons.TimeLineActive( "move_award_right", true, 0 )
			
			PlaySfxRight()
			
			return
		end
		
		repeat
			GUI.index=GUI.index+1
			
			if GUI.index == 11 then
				GUI.index=1
			end
		until GUI.race_history[GUI.index].num_cars > 0

		BuildTable(GUI.index)
		
		if UIButtons.GetSelectionIndex( GUI.node_list_id ) > GUI.race_history[GUI.index].num_cars - 1 then
			UIButtons.SetSelectionByIndex( GUI.node_list_id, GUI.race_history[GUI.index].num_cars - 1 )
		end
		
		RaceHistory_SetupInfoLines()
		
		UIButtons.TimeLineActive( "move_right", true, 0 )
		
		PlaySfxRight()

	elseif message == UIEnums.Message.PopupNext and arg2 == UIEnums.CustomPopups.SharingOptions then

		if arg3 == UIEnums.ShareOptions.Facebook then
			
			if GUI.showing_awards == true then
			
				Amax.CreateBlurb(UIEnums.SocialNetworkingItemType.MPRaceAward, 1, GUI.award_index)
				
			else

				local pos=SelectedPlayerIndex()
				local my_pos=GUI.race_history[GUI.index].my_pos
				
				if pos == my_pos then
					Amax.CreateBlurb(UIEnums.SocialNetworkingItemType.MPFinishingPosition, 1, pos)
				elseif pos > my_pos then
					Amax.CreateBlurb(UIEnums.SocialNetworkingItemType.MPRelativePlayerPositionAbove, 1, pos)
				elseif pos < my_pos then
					Amax.CreateBlurb(UIEnums.SocialNetworkingItemType.MPRelativePlayerPositionBelow, 1, pos)
				end
				
			end
			
			StoreScreen( UIEnums.ScreenStorage.FE_SOCIAL_NETWORK, "Shared\\RaceHistory.lua" )
			GoScreen( "Shared\\Facebook.lua", UIEnums.Context.Blurb )
		
		elseif arg3 == UIEnums.ShareOptions.Twitter then

			if GUI.showing_awards == true then
			
				Amax.CreateBlurb(UIEnums.SocialNetworkingItemType.MPRaceAward, 0, GUI.award_index)

			else
			
				local pos=SelectedPlayerIndex()
				local my_pos=GUI.race_history[GUI.index].my_pos
				
				if pos == my_pos then
					Amax.CreateBlurb(UIEnums.SocialNetworkingItemType.MPFinishingPosition, 0, pos)
				elseif pos > my_pos then
					Amax.CreateBlurb(UIEnums.SocialNetworkingItemType.MPRelativePlayerPositionAbove, 0, pos)
				elseif pos < my_pos then
					Amax.CreateBlurb(UIEnums.SocialNetworkingItemType.MPRelativePlayerPositionBelow, 0, pos)
				end
				
			end

			StoreScreen( UIEnums.ScreenStorage.FE_SOCIAL_NETWORK, "Shared\\RaceHistory.lua" )
			GoScreen( "Shared\\Twitter.lua", UIEnums.Context.Blurb )

		elseif arg3 == UIEnums.ShareOptions.Blurb then

			if GUI.showing_awards == true then
			
				Amax.CreateBlurb(UIEnums.SocialNetworkingItemType.MPRaceAward, 2, GUI.award_index)

			else
			
				local pos=SelectedPlayerIndex()
				local my_pos=GUI.race_history[GUI.index].my_pos
				
				if pos == my_pos then
					Amax.CreateBlurb(UIEnums.SocialNetworkingItemType.MPFinishingPosition, 2, pos)
				elseif pos > my_pos then
					Amax.CreateBlurb(UIEnums.SocialNetworkingItemType.MPRelativePlayerPositionAbove, 2, pos)
				elseif pos < my_pos then
					Amax.CreateBlurb(UIEnums.SocialNetworkingItemType.MPRelativePlayerPositionBelow, 2, pos)
				end
				
			end

		end		

	end
end

function StartLoop( time )
	DeferCam_Update( time )
end

function FrameUpdate( time )
	DeferCam_Update( time )
	if UIGlobals.RaceHistoryClose == true then
		UIGlobals.RaceHistoryClose = false
		
		GoScreen( "Multiplayer\\MpOnline.lua" )

		--CloseCurrentApp()
		Amax.ChangeUiCamera( UIGlobals.CameraNames.MpOnline, UIGlobals.CameraLerpTime, 0 )
	end
end

function Render()
	--
end

function EnterEnd()
	RestoreInfoLine()
	PlaySfxGraphicBack()
end

function EndLoop( time )
	UIButtons.TimeLineActive( "garage_start", false )
end

function End()
	if GUI.save_on_exit == true then
		StartAsyncSave()
	end
end

function SelectedPlayerIndex()
	return UIButtons.GetSelectionIndex( GUI.node_list_id ) + 1
end

function BuildTable(index)
	Multiplayer.LookingAtRaceHistory( GUI.race_history[index].index )
	
	-- heading text
	UIButtons.ChangeText( UIButtons.FindChildByName( GUI.dummy_id, "city" ), "GAME_RACE_HISTORY_CITY_" .. GUI.race_history[index].index+1 )
	UIButtons.ChangeText( UIButtons.FindChildByName( GUI.dummy_id, "route" ), "GAME_RACE_HISTORY_ROUTE_" .. GUI.race_history[index].index+1 )
	UIButtons.ChangeText( UIButtons.FindChildByName( GUI.dummy_id, "time" ), "GAME_RACE_HISTORY_TIME_" .. GUI.race_history[index].index+1 )
	UIButtons.ChangeText( UIButtons.FindChildByName( GUI.dummy_id, "mode" ), "GAME_RACE_HISTORY_MODE_" .. GUI.race_history[index].index+1 )
	
	local race = { type = 0, } -- tabulate so we may use the nice function in MpGUIBank
	race.type = GUI.race_history[index].type
	
	-- Total points in Motor Mash (as opposed to total time)
	if race.type == 3 then
		UIButtons.ChangeText( UIButtons.FindChildByName( GUI.node_title_id, "_text_title_value" ), UIText.MP_SCOREBOARD_NEW_POINTS_PER_RACE )
	else
		UIButtons.ChangeText( UIButtons.FindChildByName( GUI.node_title_id, "_text_title_value" ), UIText.MP_SCOREBOARD_TOTAL_TIME )
	end
	
	-- heading icons
	UIShape.ChangeObjectName( UIButtons.FindChildByName( GUI.dummy_id, "city_icon" ), Amax.GetCityIconName( GUI.race_history[index].city ) )
	Mp_RaceTypeIcon( UIButtons.FindChildByName( GUI.dummy_id, "mode_icon" ), race )
	
	for i=1, GUI.num_rows do
		local rank_icon_id = UIButtons.FindChildByName( GUI.node_ids[i], "_rank_icon" )
		local txt_rank_id = UIButtons.FindChildByName( GUI.node_ids[i], "_text_rank" )
		local isNodePopulated = i <= GUI.race_history[index].num_cars
		if isNodePopulated == true then
			local player = Multiplayer.GetRaceHistoryVehicle( GUI.race_history[index].index, i )
			
			if player ~= nil then
				Mp_RankIcon( rank_icon_id, player.rank, player.legend )
				UIButtons.ChangeText( txt_rank_id, "GAME_NUM_" .. player.rank + 1 )
			end
		end
		
		UIButtons.SetActive( rank_icon_id, isNodePopulated == true )
		UIButtons.SetActive( txt_rank_id, isNodePopulated == true )
		UIButtons.SetNodeItemLocked( GUI.node_list_id, i-1, isNodePopulated == false )
		
		local highlightEntry = GUI.race_history[index].my_pos == i

		local txt_pos_id = UIButtons.FindChildByName( GUI.node_ids[i], "_text_position" )
		local txt_name_id = UIButtons.FindChildByName( GUI.node_ids[i], "_text_gamertag" )
		local txt_value_id = UIButtons.FindChildByName( GUI.node_ids[i], "_text_value" )
		UIButtons.ChangeText( txt_pos_id, "GAME_RACE_HISTORY_GRID_" .. GUI.race_history[index].index+1 .. "_POS_" .. i )
		UIButtons.ChangeText( txt_name_id, "GAME_RACE_HISTORY_GRID_" .. GUI.race_history[index].index+1 .. "_NAME_" .. i )
		UIButtons.ChangeText( txt_value_id, "GAME_RACE_HISTORY_GRID_" .. GUI.race_history[index].index+1 .. "_TIME_" .. i )
		UIButtons.ChangeColour( txt_pos_id, GetTextColour( highlightEntry ) )
		UIButtons.ChangeColour( txt_name_id, GetTextColour( highlightEntry ) )
		UIButtons.ChangeColour( txt_value_id, GetTextColour( highlightEntry ) )
	end
	
end

function InitAward()
	local node_id = UIButtons.CloneXtGadgetByName( "Shared\\RaceHistory.lua", "award_node" )
	return node_id
end

function SetupAwards( index )
	-- activate and populate award panes
	for i=1, GUI.race_history[index].num_awards do
		PopulateAwardInfo( i, index )
	end
	
	-- deactivate unused award panes
	for i=GUI.race_history[index].num_awards + 1, 5 do
		UIButtons.SetActive( GUI.award_node_ids[i], false )
	end
	
	UIButtons.ChangePosition( GUI.award_list_id, (GUI.race_history[index].num_awards - 1) * -8.75 )
end

function PopulateAwardInfo( award, index )
	UIButtons.SetActive( GUI.award_node_ids[award], true )
	UIButtons.PrivateTimeLineActive( GUI.award_node_ids[award], "activate_award", true, 0 )

	local name_id = UIButtons.FindChildByName( GUI.award_node_ids[award], "name" )
	local desc_id = UIButtons.FindChildByName( GUI.award_node_ids[award], "desc" )
	local icon_id = UIButtons.FindChildByName( GUI.award_node_ids[award], "icon" )
	local amount_id = UIButtons.FindChildByName( GUI.award_node_ids[award], "amount" )
	local gamerpic_id = UIButtons.FindChildByName( GUI.award_node_ids[award], "gamerpic" )
	local rank_id = UIButtons.FindChildByName( GUI.award_node_ids[award], "rank" )
	local rank_icon_id = UIButtons.FindChildByName( GUI.award_node_ids[award], "rank_icon" )

	UIButtons.ChangeText( name_id, "GAME_RACE_HISTORY_AWARD_" .. GUI.race_history[index].index+1 .. "_NAME_" .. award - 1 )
	UIButtons.ChangeText( desc_id, "GAME_RACE_HISTORY_AWARD_" .. GUI.race_history[index].index+1 .. "_DESC_" .. award - 1 )
	UIButtons.ChangeText( amount_id, "GAME_RACE_HISTORY_AWARD_" .. GUI.race_history[index].index+1 .. "_AMOUNT_" .. award - 1 )
	LocalGamerPicture( gamerpic_id, Profile.GetPrimaryPad() )

	UIShape.ChangeObjectName( icon_id, Multiplayer.GetRaceHistoryAwardIconNames( GUI.race_history[index].index )[award] )
	
	local progression = Multiplayer.ProfileProgression()
		
	if IsTable( progression ) == true then
		Mp_RankIcon( rank_icon_id, progression.level - 1, progression.legend )	
		UIButtons.ChangeText( rank_id, "GAME_NUM_" .. progression.level )
	end
end

function GetTextColour( highlighted )
	if highlighted == true then
		return "Support_4"
	else
		return "Support_1"
	end
end

function RaceHistory_SetupInfoLines()

	local options = {  }

	if NetServices.CanViewGamerCard( Profile.GetPrimaryPad() ) == true then
		options[ #options + 1 ] = UIText.INFO_VIEW_GAMER_CARD
	end

	options[ #options + 1 ] = UIText.INFO_B_BACK
	
	if GUI.race_history[GUI.index].num_awards > 0 then
		options[ #options + 1 ] = UIText.INFO_X_SHOW_AWARDS
	end
	
	if Amax.CanUseShare() == true then
		options[ #options + 1 ] = "GAME_SHARE_BUTTON"
	end
		
	SetupInfoLine( unpack( options ) )
end
