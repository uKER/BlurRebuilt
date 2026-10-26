GUI =	
{
	finished = false,
	carousel_branch = "MpLan",

	nodes = {},

	state = 
	{
		searching	= 1,
		finished	= 2,
	},

	current_state	= 1,
	total_games		= 0,

	items_table =
	{
		"index",
		"name",
		"players",
		"mode",
	},

	CanExit = function( _GUI )
		PlaySfxGraphicBack()
		
		-- [WillB] #37769 - Need to reconnect to demonware in online mode if we previously was connected.
		if net_CanReconnectToDemonware() == true then
			net_StartServiceConnection( true, nil )
		end
		
		return true
	end,
}

function Init()
	AddSCUI_Elements()
	DeferCam_Init( UIGlobals.CameraNames.MpAppBase )

	StoreInfoLine()
	SetupInfoLine( UIText.INFO_B_BACK, UIText.INFO_Y_HOST_GAME )

	GUI.lobby_list_id = SCUI.name_to_id["list"]

	-- add a max number of items
	for i = 1, 10 do
		GUI.nodes[i] = UIButtons.CloneXtGadgetByName( "Multiplayer\\MpLan.lua", "lobby_node" )

		UIButtons.SetActive( UIButtons.FindChildByName( GUI.nodes[i], "background_box" ), Select( ( i % 2 ) == 1, true, false ) )
		UIButtons.AddListItem( GUI.lobby_list_id, GUI.nodes[i], i - 1 )
	end

	Multiplayer.CachePlaylists( UIEnums.GameMode.SystemLink )
	NetRace.EnumerateLanGames( true )
end

function PostInit()
	UIButtons.SetActive( SCUI.name_to_id["lobby_node"], false )
	
	UIButtons.ChangePanel( SetupScreenTitle( UIText.MP_LAN, SCUI.name_to_id["locator"], "system_link", "fe_icons" ), UIEnums.Panel._3DAA_WORLD, true )
	
	if UIGlobals.Multiplayer.LobbyConnectionLost == true then
		UIGlobals.Multiplayer.LobbyConnectionLost = false
		SetupCustomPopup( UIEnums.CustomPopups.LostLobbyConnection )
	end
end

function MessageUpdate( message, arg1, arg2, arg3, arg4 )
	if SCUI_MessageUpdate( message, arg1, arg2, arg3, arg4 ) == true then
		return
	end

	if message == UIEnums.Message.MenuNext then

		if GUI.sessions_found > 0 then
			PlaySfxNext()

			-- check the game we are trying to join
			UIGlobals.Network.LanEnumeratedGameIndexToJoin = UIButtons.GetSelection( GUI.lobby_list_id )
			local code, pack_version = NetRace.CheckLanServerNetworkVersion( UIGlobals.Network.LanEnumeratedGameIndexToJoin )

			-- check that we have to same build
			if code == UIEnums.Network.Version.eNetVersionSame then

				-- check that the system link pack versions are compatable
				if pack_version == UIEnums.Network.Version.eNetVersionSame then
					SetupCustomPopup( UIEnums.CustomPopups.MultiplayerJoinLanGame )

				elseif pack_version == UIEnums.Network.Version.eNetVersionOlder then
					UIGlobals.Network.JoinErrorCode = UIEnums.Network.JoinErrors.eJoinErrorPlaylistVerHost
					SetupCustomPopup( UIEnums.CustomPopups.MultiplayerJoinLanGameFailed )

				elseif pack_version == UIEnums.Network.Version.eNetVersionNewer then
					UIGlobals.Network.JoinErrorCode = UIEnums.Network.JoinErrors.eJoinErrorPlaylistVerClient
					SetupCustomPopup( UIEnums.CustomPopups.MultiplayerJoinLanGameFailed )
				end
			else                                                                                                                                                           
				UIGlobals.Network.JoinVersionErrorCode = code
				SetupCustomPopup( UIEnums.CustomPopups.MultiplayerVersionError )
			end
		end

	elseif message == UIEnums.Message.ButtonX then
		if GUI.current_state == GUI.state.finished then
			NetRace.EnumerateLanGames( false ) -- stop the old
			MpLan_RefreshLobbyList( 0 )

			-- search again
			NetRace.EnumerateLanGames( true )
			SetupInfoLine( UIText.INFO_B_BACK, UIText.INFO_Y_HOST_GAME )

			GUI.current_state = GUI.state.searching
			UIButtons.SetActive( SCUI.name_to_id["searching_branch"], true )
			
			UISystem.PlaySound( UIEnums.SoundEffect.Filter )
		end

	elseif message == UIEnums.Message.ButtonY then
		GoScreen( "Multiplayer\\Shared\\MpModeSelect.lua" )
		PlaySfxNext()
		PlaySfxGraphicNext()
	end
end

function StartLoop( time )
	DeferCam_Update( time )
end

function FrameUpdate( time )
	DeferCam_Update( time )
	
	-- searching
	if GUI.current_state == GUI.state.searching then

		local sessions = NetRace.NumLanServersEnumerated()
		if sessions ~= GUI.sessions_found then
			GUI.sessions_found = sessions

			-- show sessions
			for i = 1, sessions do
				MpLan_SetupGame( GUI.nodes[i], i )
			end
		end
		
		local finished = not NetRace.IsEnumeratingLanGames()
		if finished == true then
			GUI.current_state = GUI.state.finished
			UIButtons.SetActive( SCUI.name_to_id["searching_branch"], false )

			print( "LAN : Finished, games found ", sessions )

			MpLan_RefreshLobbyList( sessions )

			if sessions > 0 then
				SetupInfoLine( UIText.INFO_JOIN_GAME, UIText.INFO_B_BACK, UIText.INFO_X_REFRESH, UIText.INFO_Y_HOST_GAME )
			else
				SetupCustomPopup( UIEnums.CustomPopups.NoLanGames )
				SetupInfoLine( UIText.INFO_B_BACK, UIText.INFO_X_REFRESH, UIText.INFO_Y_HOST_GAME )
			end
		end
	end
end


function EnterEnd()
	RestoreInfoLine()
end

function End()
	NetRace.EnumerateLanGames( false )
end

function MpLan_RefreshLobbyList( sessions )

	-- turn on new nodes and off with the old
	for i, v in ipairs( GUI.nodes ) do
		if i > sessions then
			UIButtons.LockNode( v )

			-- change text back to '--'
			for j, w in pairs( GUI.items_table ) do
				UIButtons.ChangeText( UIButtons.FindChildByName( v, w ), UIText.CMN_DASH )
			end
		else
			UIButtons.UnlockNode( v )	
		end
	end

	UIButtons.SetSelectionByIndex( GUI.lobby_list_id, 0 )
	UIButtons.SetSelected( GUI.lobby_list_id, ( sessions >= 1 ) )
end


function MpLan_SetupGame( node_id, index )

	local index_id		= UIButtons.FindChildByName( node_id, "index" )
	local name_id		= UIButtons.FindChildByName( node_id, "name" )
	local players_id	= UIButtons.FindChildByName( node_id, "players" )
	local mode_id		= UIButtons.FindChildByName( node_id, "mode" )

	UIButtons.ChangeText( index_id, "GAME_NUM_" .. index )
	UIButtons.ChangeText( name_id, "LAN_HOST" .. index - 1 )
	UIButtons.ChangeText( players_id, "LAN_PLAYERS" .. index - 1 )
	UIButtons.ChangeText( mode_id, "LAN_MODE" .. index - 1 )
end