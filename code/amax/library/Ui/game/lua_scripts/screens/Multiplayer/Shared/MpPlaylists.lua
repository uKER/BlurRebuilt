GUI =
{
	finished = false,
	
	Selection = nil,
	PlaylistNodes = {},
	
	show_total_count	= nil,
	show_count			= nil,

	Options =
	{
		Scoreboards = 1,
	},

	CanExit = function( _GUI )
		return false
	end,

	CurrentEventSelectionIsLocked = false,
}

function Init()
	AddSCUI_Elements()
	
	Amax.ChangeUiCamera( "Mp_2", UIGlobals.CameraLerpTime, 0 )
	
	StoreInfoLine()
	SetupInfoLine()

	Multiplayer.CachePlaylists( Amax.GetGameMode() )
	UIGlobals.Splitscreen.can_vote = false -- reset
	
	GUI.Events = Multiplayer.GetPlaylists( Amax.IsGameModeRanked() ) -- returns a stripped down version so we dont flood lua with 100's of races
	GUI.MenuId = SCUI.name_to_id[ "menu" ]

	-- and the rest
	for i, v in ipairs( GUI.Events ) do
		GUI.PlaylistNodes[i] = SetupPlaylist( v, i )
	end

	if Amax.IsGameModeRanked() == true then
		SetupInfoLine( UIText.INFO_FIND_GAME, UIText.INFO_B_BACK, UIText.MP_MATCHMAKING_OPTIONS )
		
		if NetServices.ConnectionStatusIsOnline() == true then
			NetServices.AllowPlayerCountUpdate()
		end
	else
		SetupInfoLine( UIText.INFO_A_SELECT, UIText.INFO_B_BACK )
	end
end

function PostInit()

	-- update playlist details with first item
	if #GUI.Events > 0 then
		local selection = UIButtons.GetSelectionIndex( GUI.MenuId )+1
		GUI.CurrentEventSelectionIsLocked = GUI.Events[selection].locked
	else
		GUI.CurrentEventSelectionIsLocked = true
	end

	UIButtons.SetActive( SCUI.name_to_id["playlist_node"], false )

	if NetServices.ConnectionStatusIsOnline() == false or Amax.IsGameModeRanked() == false then
		UIButtons.SetActive( SCUI.name_to_id["online_players_title"], false, true )
	else
		MpPlaylist_ShowTotalPlayerCount()
		--MpPlaylist_ShowPlaylistPlayerCount( 0 )
	end
end

function MessageUpdate( message, arg1, arg2, arg3, arg4 )	

	if SCUI_MessageUpdate( message, arg1, arg2, arg3, arg4 ) == true then
		return
	end
	
	-- disable new icons
	if Amax.IsGameModeRanked() == true then
		Multiplayer.EventViewed( UIButtons.GetSelectionIndex( GUI.MenuId ) )
		Mp_HideItemNew( GUI.MenuId, UIButtons.GetSelectionIndex( GUI.MenuId ) );
	end

	if message == UIEnums.Message.MenuNext and arg2 == true then
		local selection_index = UIButtons.GetSelection( GUI.MenuId )

		if GUI.Events[selection_index].not_available == true or ( Amax.IsGameModeRanked() == true and GUI.Events[selection_index].locked == true ) then
			PlaySfxError()
		else
			PlaySfxNext()
			Multiplayer.SetCurrentEventIndex( GUI.Events[selection_index].playlist_index )

			if Amax.GetGameMode() == UIEnums.GameMode.Online then

				if Amax.IsGameModeRanked() == true then
					-- [WillB] #35660 - Fixed up player counts for demonware groups
					net_SetGroupPresence( GUI.Events[selection_index].playlist_index )

					NetRace.SetRaceParameters( Playlist_OnlineSetupTable( true ) )
					NetRace.SetOnlineSessionEnumerationParameters( Playlist_OnlineSetupTable( true ) )

					SetupCustomPopup( UIEnums.CustomPopups.MultiplayerSearchOnlineGame )
				else
					Amax.SetupRace( Playlist_OnlineSetupTable( false ) )
					SetupCustomPopup( UIEnums.CustomPopups.MultiplayerCreateOnlineGame )
				end
			
			elseif Amax.GetGameMode() == UIEnums.GameMode.SystemLink then
				if NetRace.CreateLanServer( Playlist_LanSetupTable() ) == true then
					GoSubScreen( "Multiplayer\\MpLobby.lua" )
				else
					print( "LAN - Failed to create game" )
				end	

			elseif Amax.GetGameMode() == UIEnums.GameMode.SplitScreen then
				Amax.SetupRace( Playlist_SplitScreenSetupTable() )
				PlaySfxGraphicNext()
				GoScreen( "Multiplayer\\MpSplitscreenLobby.lua" )
			end
		end

	elseif message == UIEnums.Message.ButtonX and arg2 == true then
		if Amax.IsGameModeRanked() == true then
			SetupCustomPopup( UIEnums.CustomPopups.MatchMakingOptions )
		end

	elseif message == UIEnums.Message.ButtonLeft and arg2 == true then
		UIButtons.TimeLineActive( "move_left", true, 0.0 )
		local selection_index = UIButtons.GetSelectionIndex( GUI.MenuId ) + 1 
		if selection_index-1 >= 1 then
			Ranked_RefreshInfoLine(GUI.Events[selection_index-1].locked )
		end

	elseif message == UIEnums.Message.ButtonRight and arg2 == true then
		UIButtons.TimeLineActive( "move_right", true, 0.0 )
		local selection_index = UIButtons.GetSelectionIndex( GUI.MenuId ) + 1
		if selection_index+1 <= #GUI.Events then
			Ranked_RefreshInfoLine(GUI.Events[selection_index+1].locked )
		end
		
	elseif message == UIEnums.Message.MenuBack and arg2 == true then
		PlaySfxBack()
		PlaySfxGraphicBack()

		if Amax.IsGameModeRanked() == true then
			net_SetGroupPresence()
			GoScreen( "Multiplayer\\MpOnline.lua" )
		else
			GoScreen( "Multiplayer\\Shared\\MpModeSelect.lua" )
		end
	end
end

function FrameUpdate( time )

	-- has the selection changed
	local selection = UIButtons.GetSelectionIndex( GUI.MenuId )
	if selection ~= GUI.Selection then

		if GUI.Selection ~= nil then
			Amax.ChangeChildrenLayer( GUI.PlaylistNodes[GUI.Selection+1], 1 )
		end
		GUI.Selection = selection
		Amax.ChangeChildrenLayer( GUI.PlaylistNodes[GUI.Selection+1], 2 )

		if selection == 0 then
			UIButtons.TimeLineActive( "left_fade", false )
			UIButtons.TimeLineActive( "right_fade", true )

		elseif selection == #GUI.Events-1 then
			UIButtons.TimeLineActive( "left_fade", true )
			UIButtons.TimeLineActive( "right_fade", false )
		else
			UIButtons.TimeLineActive( "left_fade", true )
			UIButtons.TimeLineActive( "right_fade", true )
		end

		GUI.show_count = nil
	end

	-- update counts
	if Amax.IsGameModeRanked() == true then
		MpPlaylist_ShowTotalPlayerCount()
		--MpPlaylist_ShowPlaylistPlayerCount( GUI.Selection )
	end
end

function EnterEnd()
	RestoreInfoLine()
end

function End()
	--StoreScreenSelection( GUI, "MenuId", true )
	
	NetServices.StopPlayerCountUpdate()
end

function MpPlaylist_ShowTotalPlayerCount()
	if NetServices.ConnectionStatusIsOnline() == false then
		return
	end

	-- only show total players if there are people playing
	local show_total_count = NetServices.ShowTotalPlayerCount()
	if show_total_count ~= GUI.show_total_count then
		GUI.show_total_count = show_total_count

		UIButtons.SetActive( SCUI.name_to_id["online_players_title"], show_total_count, true )
	end
end

function MpPlaylist_ShowPlaylistPlayerCount( playlist_index )
	-- only show playlist players if there are people playing
	local show_count = NetServices.ShowPlaylistPlayerCount( playlist_index )
	if show_count ~= GUI.show_count then
		GUI.show_count = show_count

		if GUI.Events[playlist_index+1].locked == false then
			UIButtons.SetActive( UIButtons.FindChildByName( GUI.PlaylistNodes[playlist_index+1], "players_title" ), show_count, true )
		end
	end
end

function SetupPlaylist( playlist, index )
	local node_id = UIButtons.CloneXtGadgetByName( "Multiplayer\\Shared\\MpPlaylists.lua", "playlist_node" )
	local text_id = UIButtons.FindChildByName( node_id, "name" )
	local desc_id = UIButtons.FindChildByName( node_id, "desc" )
	local image_id = UIButtons.FindChildByName( node_id, "image" )
	local icon_id = UIButtons.FindChildByName( node_id, "icon" )
	local new_id = UIButtons.FindChildByName( node_id, "new" )
	local lock_id = UIButtons.FindChildByName( node_id, "lock" )

	UIButtons.ChangeText( text_id, "MPL_PLAYLIST_NAME" .. playlist.playlist_index )

	-- unless its unavailable, then it can only be locked in rank games
	if playlist.not_available == true or ( Amax.IsGameModeRanked() == true and playlist.locked == true ) then
		UIButtons.ChangeText( desc_id, Select( playlist.not_available, UIText.MP_PLAYLIST_NOT_AVAILABLE, "MPL_UNLOCK_AT_RANK" .. playlist.rank ) )
		UIButtons.ChangeEffectIndex( image_id, UIEnums.EffectIndex.GreyScale )

		UIButtons.ChangeColour( text_id, "Dark_White" )
		UIButtons.ChangeColour( desc_id, "Dark_white" )
		UIButtons.ChangeColour( icon_id, "Dark_White" )

		UIButtons.SetActive( lock_id, true )
	else
		-- only show new in ranked mode
		UIButtons.SetActive( new_id, ( Amax.IsGameModeRanked() == true and playlist.new == true ) )
		UIButtons.ChangeText( desc_id, "MPL_PLAYLIST_DESC" .. playlist.playlist_index )
	end

	UIShape.ChangeObjectName( icon_id, playlist.icon_name )

	if playlist.image_name ~= nil then
		UIButtons.ChangeTexture( { filename = playlist.image_name }, 0, image_id )
	end

	-- hide online stats when we arnt in online ranked mode
	if NetServices.ConnectionStatusIsOnline() == false or Amax.IsGameModeRanked() == false or playlist.locked == true then
		UIButtons.SetActive( UIButtons.FindChildByName( node_id, "players_title" ), false, true )
	else
		UIButtons.ChangeText( UIButtons.FindChildByName( node_id, "players" ), "MPL_ACTIVE_PLAYERS" .. playlist.playlist_index )
	end

	UIButtons.AddListItem( GUI.MenuId, node_id, index )

	return node_id
end

function Ranked_RefreshInfoLine( locked )

	if Amax.IsGameModeRanked() == false then
		return
	end

	-- Is the new info line different to the current info line?
	if GUI.CurrentEventSelectionIsLocked == locked then
		return 
	else
		-- Clear old
		SetupInfoLine()
		
		if locked == true then
			SetupInfoLine( UIText.INFO_B_BACK, UIText.MP_MATCHMAKING_OPTIONS )
		else
			SetupInfoLine( UIText.INFO_FIND_GAME, UIText.INFO_B_BACK, UIText.MP_MATCHMAKING_OPTIONS )
		end

		-- Store new event locked status
		GUI.CurrentEventSelectionIsLocked = locked
	end
end


-----------------------------------------------------------------------------------------
-- Mode specific setup functions
-----------------------------------------------------------------------------------------
function Playlist_OnlineSetupTable( public_game )
	local settings = 
	{
		num_players = 1,
		public_game = public_game,

		-- TODO_MP_REFACTOR
		world_tour = true, -- this means playlist
		user_created_group = false,
	}

	return settings
end

function Playlist_LanSetupTable( public_game )
	local settings = 
	{
		num_players = 1,
		public_game = public_game,

		-- TODO_MP_REFACTOR
		world_tour = true, -- this means playlist
		user_created_group = false,
	}

	return settings
end

function Playlist_SplitScreenSetupTable()
	local settings =
	{
		num_players = #UIGlobals.Splitscreen.players
	}

	return settings
end
