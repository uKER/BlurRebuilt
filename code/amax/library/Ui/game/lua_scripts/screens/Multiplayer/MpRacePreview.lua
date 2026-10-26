GUI =
{
	finished		= false,
	player_list_id	= -1,
	player_nodes	= {},

	colours = {},
	can_invite = nil,
	view_gamercard = nil,
}

function Init()
	PlaySfxGraphicBack()
	AddSCUI_Elements( )
	Amax.ChangeUiCamera( "Sp_2", UIGlobals.CameraLerpTime, 0 )
	UIButtons.TimeLineActive( "Hide_Lobby", true )

	GUI.race_preview_id = SCUI.name_to_id["race_preview"] 
	GUI.colours[1] = UIButtons.GetXtVar( GUI.race_preview_id, "colour_first" )
	GUI.colours[2] = UIButtons.GetXtVar( GUI.race_preview_id, "colour_second" )
	GUI.colours[3] = UIButtons.GetXtVar( GUI.race_preview_id, "colour_third" )
	GUI.colours[4] = UIButtons.GetXtVar( GUI.race_preview_id, "colour_other" )

	GUI.player_list_id = SCUI.name_to_id["player_list"]
	GUI.race = Multiplayer.GetCurrentRace()

	for i = 1, 20 do
		GUI.player_nodes[i] = MpRacePreview_SetupPlayerNode( i )

		UIButtons.AddListItem( GUI.player_list_id, GUI.player_nodes[i], i )
	end

	MpRacePreview_RefreshStandings()

	StoreInfoLine()
	MpRacePreview_RefreshInfolines()
end

function PostInit()
	UIButtons.ChangePanel( SetupScreenTitle( UIText.MP_RACE_PREVIEW, SCUI.name_to_id["centre"], "route", "fe_icons" ), UIEnums.Panel._3DAA_WORLD, true )

	Mp_RefreshRaceOverview( SCUI.name_to_id["race_overview_node"], GUI.race )

	local city	= GameData.GetCityData( GUI.race.city ) 
	local route	= GameData.GetRouteData( GUI.race.route )
	MpRacePreview_UpdateTrackMap( city, route )
	-- setup laps and time
	if GUI.race.type == UIEnums.MpRaceType.Racing then
		UIButtons.ChangeText( SCUI.name_to_id["limit"], "LOBBY_PREVIEW_LAPS" )
		UIButtons.ChangeText( SCUI.name_to_id["timeout"], "LOBBY_PREVIEW_TIME" )
	else
		UIButtons.ChangeText( SCUI.name_to_id["limit"], "LOBBY_PREVIEW_TIME" )
	end

end

function MessageUpdate( message, arg1, arg2, arg3, arg4 )
	if SCUI_MessageUpdate( message, arg1, arg2, arg3, arg4 ) == true then
		return
	end
	
	if GUI.finished == true then
		return
	end

	if message == UIEnums.Message.MenuBack and arg2 == true then
		PlaySfxBack()
		PopScreen()

	elseif message == UIEnums.Message.MenuNext and arg2 == true then
		if NetServices.CanViewGamerCard( Profile.GetPrimaryPad() ) == true then
			local selection = UIButtons.GetSelection( GUI.player_list_id )
			local join_id = Select( GUI.preview.loading, GUI.preview.players_loading, GUI.preview.players_racing )[selection]

			NetServices.ViewGamerCard( Profile.GetPrimaryPad(), join_id, false, false )
		end
		
	elseif message == UIEnums.Message.ButtonY and arg2 == true then
		if Mp_XenonGameInviteAvailable() == true then
			if Mp_XenonPartyActive() == true then
				UIHardware.ShowParty360( Profile.GetPrimaryPad() )
			else
				UIHardware.ShowFriends360( Profile.GetPrimaryPad() )
			end
		end
	end
end

function FrameUpdate( time )
	if GUI.finished == true then
		return
	end

	if Multiplayer.RefreshRacePreview( GUI.race_preview_id ) == true then
		MpRacePreview_RefreshStandings()
	end

	if NetRace.RaceInProgress() == false or NetRace.HasRaceFinished() == true then
		PlaySfxBack()
		PopScreen()
	end
	
	MpRacePreview_RefreshInfolines()
end

function EnterEnd()
	PlaySfxGraphicNext()
	RestoreInfoLine()
	Amax.ChangeUiCamera( UIGlobals.CameraNames.MpLobby, UIGlobals.CameraLerpTime, 0 )
	
	UIButtons.TimeLineActive( "Hide_Lobby", false )
end

function End()
	UIButtons.SetActive( SCUI.name_to_id["player_node"], false, true )
end

function MpRacePreview_SetupPlayerNode( index )

	local node_id = UIButtons.CloneXtGadgetByName( "Multiplayer\\MpRacePreview.lua", "player_node" )
	UIButtons.SetActive( UIButtons.FindChildByName( node_id, "background_box" ), ( index % 2 ) == 1 )

	UIButtons.ChangeColour( UIButtons.FindChildByName( node_id, "position" ),	GUI.colours[Select( index > #GUI.colours, 4, index )] )
	UIButtons.ChangeColour( UIButtons.FindChildByName( node_id, "name" ),		GUI.colours[Select( index > #GUI.colours, 4, index )] )
	UIButtons.ChangeColour( UIButtons.FindChildByName( node_id, "state" ),		GUI.colours[Select( index > #GUI.colours, 4, index )] )

	UIButtons.ChangeText( UIButtons.FindChildByName( node_id, "position" ), "GAME_POS_" .. index )

	return node_id
end

function MpRacePreview_RefreshStandings()

	-- turn all nodes off
	for i, v in ipairs( GUI.player_nodes ) do
		UIButtons.LockNode( v )
	end

	GUI.preview = Multiplayer.GetRacePreviewData( GUI.race_preview_id )

	if GUI.preview.loading == true then
		for i, v in ipairs( GUI.preview.players_loading ) do
			UIButtons.ChangeText( UIButtons.FindChildByName( GUI.player_nodes[i], "name" ),		"LOBBY_PLAYER_NAME" .. v )
			UIButtons.ChangeText( UIButtons.FindChildByName( GUI.player_nodes[i], "state" ),	UIText.MP_LOADING )

			UIButtons.UnlockNode( GUI.player_nodes[i] )
		end
	else
		for i, v in ipairs( GUI.preview.players_racing ) do
			UIButtons.ChangeText( UIButtons.FindChildByName( GUI.player_nodes[i], "name" ),		"LOBBY_PREVIEW_NAME" .. i )
			UIButtons.ChangeText( UIButtons.FindChildByName( GUI.player_nodes[i], "state" ),	"LOBBY_PREVIEW_STATE" .. i )

			UIButtons.UnlockNode( GUI.player_nodes[i] )
		end
	end
end

function MpRacePreview_UpdateTrackMap( city_data, route_data )

	local map_route_id = SCUI.name_to_id["track_shape"]
	local map_start_id = SCUI.name_to_id["start_line"]
	
	UIButtons.SetActive( map_route_id, false )
	UIButtons.SetActive( map_start_id, false )

	if UIShape.ChangeSceneName( map_route_id, city_data.debug_tag .. "_game" ) == false then
		return
	end
	if UIShape.ChangeObjectName( map_route_id, route_data.debug_tag ) == false then
		if UIShape.ChangeObjectName( map_route_id, route_data.debug_tag ) == false then
			return
		end
	end
	
	UIButtons.SetActive( map_route_id, true )
	
	-- Change start line
	if UIShape.ChangeSceneName( map_start_id, city_data.debug_tag .. "_game" ) == false then
		return
	end
	if UIShape.ChangeObjectName( map_start_id, route_data.debug_tag .. "_Alt" ) == false then
		return
	end
	
	UIButtons.SetActive( map_start_id, true )
end

function MpRacePreview_RefreshInfolines()
	
	local selection = UIButtons.GetSelection( GUI.player_list_id )
	local join_id = Select( GUI.preview.loading, GUI.preview.players_loading, GUI.preview.players_racing )[selection]
	
	local view_gamercard = ( NetServices.CanViewGamerCard( Profile.GetPrimaryPad() ) == true and join_id < 50 )
	
	local refresh = false
	if view_gamercard ~= GUI.view_gamercard then
		GUI.view_gamercard = view_gamercard
		refresh = true
	end
	if GUI.can_invite ~= Mp_XenonGameInviteAvailable() then
		GUI.can_invite = Mp_XenonGameInviteAvailable()
		refresh = true
	end
	
	if refresh == true then
		-- need previleges to view a gamercard
		if NetServices.CanViewGamerCard( Profile.GetPrimaryPad() ) == true and join_id < 50 then
			SetupInfoLine( UIText.INFO_VIEW_GAMER_CARD, UIText.INFO_B_BACK )
		else
			SetupInfoLine( UIText.INFO_B_BACK )
		end
			
		if GUI.can_invite == true then
			PushInfoLine( "LOBBY_INVITE_BUTTON" )
		end
	end
end
