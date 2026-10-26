 --[[****************************************************************************************
	
	Network script helper functions. 
 
 ****************************************************************************************]]--
 
 GUINetwork = 
 {
	multiplayer_context		= 0,
	multiplayer_root		= "multiplayer\\mpmain.lua",
	multiplayer_party_root	= "multiplayer\\mponline.lua",
	load_started = false,
 }

function net_SetGroupPresence( id )
	-- only if we have an active online connection
	if NetServices.ConnectionStatusIsOnline() == true then
		if id ~= nil then
			Multiplayer.SetEventPresence( id )
		else
			Multiplayer.SetDefaultPresence()
		end
	end
end

function net_connection_CompleteConnect( )
	-- on ps3 and pc we re read the primary players profile since the code does not handle going to an online profile from an offline one.
	-- whould always have a valid offline profile on these platforms.
	if UIEnums.CurrentPlatform == UIEnums.Platform.PS3 or UIEnums.CurrentPlatform == UIEnums.Platform.PC then
		Profile.ReadPadProfile( UIGlobals.ProfilePressedStart )
	end

	-- start the async bandwidth evaluation.
	NetServices.StartBandwidthEvaluation( )
end
  
-- stop enumeration, flush sessions.
function net_FlushSessionEnumerator( )
	NetRace.EnumerateLanGames( false )
	NetServices.FlushEnumertedSessions( )
end

-- stop enumeration, but do not flush sessions.
function net_StopSessionEnumerator( )
	NetRace.EnumerateLanGames( false )
	NetServices.StopEnumertedSessions( )
end

function net_CloseAllSessions( )
	NetRace.Delete( )
	NetParty.Delete( )
	UIGlobals.Network.UnrecoverableSessionError = false
end

function net_DisconnectFromRace()
	if UIGlobals.Multiplayer.Disconnecting == true then
		return
	end
	
	UIGlobals.Multiplayer.Disconnecting = true -- will be flagged to false when the disconnection has finished
	
	NetRace.EnterDisconnecting()
	SetupCustomPopup( UIEnums.CustomPopups.MultiplayerDisconnectFromRace )
end

function net_Disconnecting()
	return UIGlobals.Multiplayer.Disconnecting
end

function net_StopScoreboardRequests( )
	NetServices.StopScoreboardRequests( )
end

function net_FlushEverything( ignore_presence )
	if ignore_presence == nil then
		net_EnableGlobalUpdateRichPresence( false )
	end
	net_StopScoreboardRequests( )
	net_FlushSessionEnumerator( )
	net_CloseAllSessions( )
	net_EnableGlobalUpdateRouterMonitoring( false )
	net_LockoutFriendsOverlay( false )
	net_EnableGlobalUpdatePartyCommands( false )
	
	UIGlobals.Network.UnrecoverableRouterError = false
		
	GUINetwork.load_started = false
end

function net_RouterAvailable( online_mode )
	local state = UIEnums.Network.Router.eConnectionStateNotActive
	local result = UIEnums.Network.Router.eConnectionResultNone
	local online = false
	state, result, online = NetServices.ConnectionStatus()
	
	if state == UIEnums.Network.Router.eConnectionStateComplete then
		if online == true then
			return true -- if we are online router is automatically available for offline play
		elseif online_mode == true then
			return false --we require online mode but router is connected for offline play only
		end			
		return true -- router is available for desired mode
	end
	
	return false -- connection not complete yet.
end

function net_CloseServiceConnection( )
	net_FlushEverything( )
	NetServices.Disconnect( )
	
	-- on pc we need to set a valid profile username
	Profile.SetDefaultUserName( )
end

function net_CanStartOnlineServiceConnection()
	if NetServices.OnlineConnectionPossible() == false then
		return false
	end
	
	if Profile.PadProfileOnline( Profile.GetPrimaryPad() ) == true and NetServices.UserHasAgeRestriction() == true then
		return false
	end
	
	-- do we currently have a valid online connection with demonware
	return ( NetServices.ConnectionStatusIsOnline() == false )
end

function net_StartServiceConnection( go_online, callback, spawn )
	if UserKickBackActive() == true then
		return
	end
	
	UIGlobals.server_connection.online_mode = go_online
	UIGlobals.server_connection.callback = callback
	UIGlobals.server_connection.active = true

	-- if a game invite is pending then we must be connected in online mode to play
	if NetServices.GameInvitePending() == true then
		UIGlobals.server_connection.online_mode = true
	end

	if spawn == true then
		PopupSpawn( UIEnums.CustomPopups.NetConnectionTask )
	else
		SetupCustomPopup( UIEnums.CustomPopups.NetConnectionTask )
	end
end

-- downloads the user profile and title storage data. User profile will only download
-- if it doesnt exist
function net_StartStorageDownload( callback )
	UIGlobals.StorageDownloadCallback = callback

	SetupCustomPopup( UIEnums.CustomPopups.StorageDownload )
end


function net_SetRichPresence( ePresence )
	UIGlobals.Network.CurrentRichPresence = ePresence
	NetServices.SetRichPresence( ePresence )
end

function net_MpEnter( eMode )
	if eMode == UIEnums.GameMode.Online then
		Amax.SetGameMode( UIEnums.GameMode.Online )
		net_EnableGlobalUpdateRichPresence( true )
		net_SetRichPresence( UIEnums.RichPresence.OnlineMain )

	elseif eMode == UIEnums.GameMode.SplitScreen then
		Amax.SetGameMode( UIEnums.GameMode.SplitScreen )
		net_EnableGlobalUpdateRichPresence( false )

	elseif eMode == UIEnums.GameMode.SystemLink then
		Amax.SetGameMode( UIEnums.GameMode.SystemLink )
		net_EnableGlobalUpdateRichPresence( false )
		net_SetRichPresence( UIEnums.RichPresence.LAN )
	else
		print( "ERROR: Current GameMode is not supported", eMode )
		return
	end
end

function net_MpEnterPlayMode( eMode, bRanked )
	-- only these are valid
	if eMode == UIEnums.PlayMode.Nothing or eMode == UIEnums.PlayMode.CustomRace or eMode == UIEnums.PlayMode.Playlist then
		Amax.SetPlayMode( eMode )
	else
		print( "ERROR: Current PlayMode is not supported", eMode )
		return
	end

	if bRanked == nil then
		bRanked = false
	end

	Amax.SetOnlineRanked( bRanked )
end

function net_MpLeave( )
	UIGlobals.ConnectionType = UIEnums.ConnectionType.None

	Amax.SetGameMode( UIEnums.GameMode.Nothing )
	net_MpEnterPlayMode( UIEnums.PlayMode.Nothing, false )
	
	net_EnableGlobalUpdatePartyCommands( false )
	net_EnableGlobalUpdateRichPresence( false )
		
	UpdateMpStatusBar()
end

function net_EnableGlobalUpdate( enable, retain_invite )
	UIGlobals.Network.GlobalUpdateEnabled = enable

	if retain_invite == nil or retain_invite == false then
		UIGlobals.Network.GameInviteState = UIEnums.MpGameInviteState.Polling
	end
end

function net_GameInviteFailed( )
	UIGlobals.Network.GameInviteState = UIEnums.MpGameInviteState.Polling
	net_FlushEverything( )
	
	-- i shouldnt be here but i might cause knock on effects by not being here???
	--net_MpLeave( )
end

function net_EnableGlobalUpdateRichPresence( enable )
	UIGlobals.Network.RichPresenceUpdateEnabled = enable
end

function net_EnableGlobalUpdateRouterMonitoring( enable )
	UIGlobals.Network.RouterMonitoringEnabled = enable
end

function net_LockoutFriendsOverlay( lock )
	local old = UIGlobals.Network.LockoutFriendsOverlay
	UIGlobals.Network.LockoutFriendsOverlay = lock
	return old
end

function net_SetPartyLobbyMatchingDelay( )
	UIGlobals.Network.PartyLobbyMatchingDelay = 5
end

function net_EnableGlobalUpdatePartyCommands( enable )
	UIGlobals.Network.GlobalUpdatePartyCommands = enable
end


function net_EnterPartyLobby( clear_invite )
	NetRace.Delete( )
	NetParty.EnterLobby( )
	NetParty.EnableMatchingQoS( true )
	NetParty.ReceivedReturnToLobby( true )			-- clear the internal flag now it's been processed.	

	net_EnableGlobalUpdatePartyCommands( false )	-- going back to party lobby so disable command processing within the global update.	
	net_EnableGlobalUpdateRouterMonitoring( true ) 	-- on certain platform we need to make sure the network router stays connected to demonware for online play.
	
	UIGlobals.Network.ReturnPartyLobbyState = UIEnums.MpReturnPartyLobbyState.Polling
	
	if clear_invite == true then
		NetParty.ClearRaceInvite( )
	end
end

function net_ExitPartyLobby()
	NetRace.Delete( )
	NetParty.EnableMatchingQoS( false )

	net_EnableGlobalUpdatePartyCommands( false )
	net_FlushSessionEnumerator( )
	net_CloseAllSessions( )
end

function net_CanReconnectToDemonware( )
	if NetServices.NetworkConnectionActive() == true and NetServices.OnlineConnectionPossible() == true then
		return true
	else
		return false
	end
end

--[[ basically if we loose connection to demonware whilst playing online mode. 
	 signing in / out of profiles should be of a higher priority than this	
--]]
function net_GlobalUpdate_RouterConnection( )	
	if UIGlobals.Network.RouterMonitoringEnabled == true then
		if UIEnums.CurrentPlatform == UIEnums.Platform.PS3 or UIEnums.CurrentPlatform == UIEnums.Platform.PC then
				
			local state = UIEnums.Network.Router.eConnectionStateNotActive
			local result = UIEnums.Network.Router.eConnectionResultNone
			local online = false
			state, result, online = NetServices.ConnectionStatus()
			
			if state == UIEnums.Network.Router.eConnectionStateFailed then		
							
				net_EnableGlobalUpdatePartyCommands( false )
				
				UIGlobals.Network.RouterMonitoringEnabled = false
				UIGlobals.Network.UnrecoverableRouterError = true
				
				Profile.CloseFriends( )
			end
		end
	end
	return false
end


--[[ has an error occurred with the matching sessions that we cant recover from 
--]]
function net_GlobalUpdate_MatchingSessions( )
	if UIGlobals.Network.UnrecoverableSessionError == false	then
		if NetRace.UnrecoverableError( ) == true or NetParty.UnrecoverableError( ) == true then
				
			if UIScreen.IsPopupActive( ) == true then
				UIScreen.CancelPopup( )
			end
			
			if GUINetwork.load_started == false then	
				Profile.CloseFriends( )
				
				net_EnableGlobalUpdatePartyCommands( false )
				
				-- [WillB] #36793 this is safe to call from a non-network game
				net_FlushSessionEnumerator()
				net_CloseAllSessions()
				
				if UIGlobals.IsIngame == true then
									
					-- load ui and go directly to the MP root
					Amax.SendMessage(UIEnums.GameFlowMessage.QuitRace)
					
					StoreScreen( UIEnums.ScreenStorage.FE_RETURN, GUINetwork.multiplayer_root ) 

					if UserKickBackActive() == false then
						GoScreen( "Loading\\LoadingUi.lua" )
					end
				else
					GoScreen( GUINetwork.multiplayer_root, GUINetwork.multiplayer_context )
				end
				
				GUINetwork.load_started = true
				
				-- mpMain.lua will pickup on this global and show popup..
				UIGlobals.Network.UnrecoverableSessionError = true
			end
			
		end
	end
	
	return false
end


--[[	if this function is enabled assume we are in online mode and need to update.
		ps3 and pc need to update each frame to make join in progress function correctly
		xbox360 handles this through its operating system 
--]]			
function net_GlobalUpdate_RichPresence( )	
	if UIEnums.CurrentPlatform == UIEnums.Platform.PS3 or UIEnums.CurrentPlatform == UIEnums.Platform.PC then
		NetServices.SetRichPresence( UIGlobals.Network.CurrentRichPresence )
	end
	
	return false
end


-- is it possible to use the friends overlay at this time
function net_CheckFriendsAvailability( )
	if UIEnums.CurrentPlatform == UIEnums.Platform.PS3 or UIEnums.CurrentPlatform == UIEnums.Platform.PC then
		local ok = false
		
		if UIGlobals.IsLoading == false and UIGlobals.Network.GlobalUpdateEnabled == true and UIGlobals.Network.GameInviteState == UIEnums.MpGameInviteState.Polling then
			local state = UIEnums.Network.Router.eConnectionStateNotActive
			local result = UIEnums.Network.Router.eConnectionResultNone
			local online = false
			state, result, online = NetServices.ConnectionStatus()
			
			if state == UIEnums.Network.Router.eConnectionStateComplete and online == true then	
				if UIGlobals.Network.LockoutFriendsOverlay == false then
					ok = true
				end
			end
		end
		
		if UIScreen.IsPopupActive() == true then
			ok = false
		end
		
		if Amax.IsEnumeratingFriends() == true then
			ok = false
		end
		
		if ok == false then
			Profile.CloseFriends()
		end
		
		Profile.SetFriendsActive( ok )
	end
end


--[[ have we received a game invite from another player..
--]]
function net_GlobalUpdate_GameInvite( )

	if UIGlobals.Network.GameInviteState == UIEnums.MpGameInviteState.Polling then
		if NetServices.GameInvitePending( ) == false then
			return false
		end

		-- close any popups that may be active
		if UIScreen.IsPopupActive() == true then
			UIScreen.CancelPopup()
		end

		local pad_index = NetServices.FindGameInvitePadIndex()

		if Profile.GetPrimaryPad() ~= pad_index then
			ReturnToStartScreen( UIEnums.UserKickBackMode.InactiveGameInvite )
			return
		end		
		
		-- disable party updates
		net_EnableGlobalUpdatePartyCommands( false )
		
		-- we need to be in the multiplayer root screen.
		if UIScreen.GetCurrentScreen( GUINetwork.multiplayer_context ) ~= GUINetwork.multiplayer_root then
			print( "Booting Multiplayer mode" )

			FlushSubscreens()

			if UIGlobals.IsIngame == true then
				Amax.SendMessage(UIEnums.GameFlowMessage.QuitRace)
				Profile.CloseFriends( )
				StoreScreen( UIEnums.ScreenStorage.FE_RETURN, GUINetwork.multiplayer_root ) 

				GoScreen( "Loading\\LoadingUi.lua", UIEnums.Context.Main )
			else
				-- Need to close down single player and boot the MP root
				GoScreen( GUINetwork.multiplayer_root, GUINetwork.multiplayer_context )
				Amax.ChangeEnvironmentSettings( false )
			end
			UIGlobals.Network.GameInviteState = UIEnums.MpGameInviteState.Initialising
		else
			ForceCloseApp()
			UIGlobals.Network.GameInviteState = UIEnums.MpGameInviteState.Initialising

			net_StartStorageDownload() -- need to check to see if we should download the profile and tms
		end

	elseif UIGlobals.Network.GameInviteState == UIEnums.MpGameInviteState.PreProcessing then
		Profile.CloseFriends( )

		-- Start the invite
		Amax.GameInviteStarted( )
		RemoveBlaggedProfiles( )

		net_FlushEverything( )
		net_MpEnter( UIEnums.GameMode.Online )
		net_EnableGlobalUpdateRouterMonitoring( true )

		local popup_type = -1
		local ret = NetServices.CheckEntryRequirements( true )

		if ret == 0 then
			popup_type = UIEnums.CustomPopups.MultiplayerAcceptGameInvitation
		elseif ret == 1 then
			NetServices.ClearGameInvite( )
			popup_type =  UIEnums.CustomPopups.MultiplayerTooManyProfiles
		elseif ret == 2 then
			NetServices.ClearGameInvite( )
			popup_type =  UIEnums.CustomPopups.MultiplayerOnlineAccountRequired
		elseif ret == 3 then
			NetServices.ClearGameInvite( )
			popup_type = UIEnums.CustomPopups.MultiplayerInvalidPrivileges
		end

		if UIScreen.IsPopupActive() == true then
			PopupSpawn( popup_type )
		else
			SetupCustomPopup( popup_type )
		end

		UIGlobals.Network.GameInviteState = UIEnums.MpGameInviteState.Processing
	end

	return false 
end


function net_GlobalUpdate_RaceInvite()
	if NetParty.IsRunning( ) == false then
		net_EnableGlobalUpdatePartyCommands( false )
		return false
	end

	if UIGlobals.Network.RaceInviteState == UIEnums.MpRaceInviteState.Polling then
		if NetParty.MatchmakingSessionIdle() == false or NetParty.ReceivedRaceInvite() == false then
			return false
		end

		-- close any popups that may be active
		if UIScreen.IsPopupActive() == true then
			UIScreen.CancelPopup()
		end
		
		-- disable party updates
		NetParty.EnableMatchingQoS( false )
		
		Profile.CloseFriends( )
		Mp_ReturnToPartyLobby()

		print( "Booting party lobby" )
		UIGlobals.Network.RaceInviteState = UIEnums.MpRaceInviteState.Initialising

	elseif UIGlobals.Network.RaceInviteState == UIEnums.MpRaceInviteState.PreProcessing then
		print( "Accepting party race invite" )

		UIGlobals.Network.RaceInviteState = UIEnums.MpRaceInviteState.Processing
		SetupCustomPopup( UIEnums.CustomPopups.AcceptPartyRaceInvite )
	end

	return false
end



--[[	host issues commands to peers, if we are not on the party lobby script then we can pick them up here and
		process them
--]]
function net_GlobalUpdate_PartyCommands( )
	if NetParty.IsRunning( ) == false then
		net_EnableGlobalUpdatePartyCommands( false )
		return false
	end

	if UIGlobals.Network.GlobalUpdatePartyCommands == false then
		return false
	end

	if UIGlobals.IsLoading == false and UIGlobals.Network.GlobalUpdateEnabled == true and UIGlobals.Network.GameInviteState == UIEnums.MpGameInviteState.Polling then

		if UIGlobals.Network.ReturnPartyLobbyState == UIEnums.MpReturnPartyLobbyState.Polling then
			if NetParty.ReceivedReturnToLobby( false ) == false then
				return false
			end

			print( "Returning to lobby" )

			-- close any popups that may be active
			if UIScreen.IsPopupActive() == true then
				UIScreen.CancelPopup()
			end
			
			NetRace.Delete( ) 
			
			-- we need to be in the multiplayer root screen.
			if UIScreen.GetCurrentScreen( GUINetwork.multiplayer_context ) ~= GUINetwork.multiplayer_root then
				print( "Booting Multiplayer mode" )

				-- just in case
				--Amax.SendMessage(UIEnums.GameFlowMessage.DumpUI)
				
				if UIGlobals.IsIngame == true then
					Amax.SendMessage(UIEnums.GameFlowMessage.QuitRace)
					Profile.CloseFriends( )
					GUINetwork.load_started = false

					GoScreen( "Loading\\LoadingUi.lua" )
				else
					-- Need to close down single player and boot the MP root
					GoScreen( GUINetwork.multiplayer_root, GUINetwork.multiplayer_context )
				end
				UIGlobals.Network.ReturnPartyLobbyState = UIEnums.MpReturnPartyLobbyState.Initialising
			else
				UIGlobals.Network.ReturnPartyLobbyState = UIEnums.MpReturnPartyLobbyState.PreProcessing
			end

		elseif UIGlobals.Network.ReturnPartyLobbyState == UIEnums.MpReturnPartyLobbyState.PreProcessing then
			UIGlobals.Network.ReturnPartyLobbyState = UIEnums.MpReturnPartyLobbyState.Polling

			net_EnableGlobalUpdatePartyCommands( false )	-- going back to party lobby so disable command processing from now on.		
			NetParty.ReceivedReturnToLobby( true )			-- clear the internal flag now it's been processed.	

			LaunchScreen( UIEnums.MpLaunchScreen.PartyLobby )
		end
	end

	return false
end

function net_GlobalUpdate_InviteReconnectToServers( )
	-- must be connected in online mode to be able to accept a pending game invite.
	if NetServices.GameInvitePending() == false or UIGlobals.Network.GameInviteState ~= UIEnums.MpGameInviteState.Polling or NetServices.ConnectionStatusIsOnline() == true then
		return false
	end

	-- check that its for the primary player, otherwise the invite will throw us back to the start screen
	if Profile.GetPrimaryPad() ~= NetServices.FindGameInvitePadIndex() then
		return false
	end	

	print( "NET - Reconnecting to servers" )

	-- close any popups that may be active
	if UIScreen.IsPopupActive() == true then
		UIScreen.CancelPopup()
	end

	-- connect to blur servers
	net_StartServiceConnection( true, nil )

	return true
end

function net_GlobalUpdate( )

	-- rich presence update enabled. mainly for online mode ps3 and pc
	if UIGlobals.Network.RichPresenceUpdateEnabled == true then
		net_GlobalUpdate_RichPresence( )
	end

	-- The UI is currently kicking the player back to the start screen
	if UIGlobals.UserKickBackMode ~= UIEnums.UserKickBackMode.None then
		return
	end

	-- no logic runs while loading.. or when update is flagged as being disabled. or when connecting to server
	if UIGlobals.IsLoading == true or UIGlobals.Network.GlobalUpdateEnabled == false or UIGlobals.server_connection.active == true then
		return true
	end

	-- busy trying to reconnect us to the server
	if net_GlobalUpdate_InviteReconnectToServers( ) == true then
		return false
	end
	
	-- check router connection if required ps3 and pc may disconnect from the platform servers, this would force us to disconnect and go to
	-- a specific ui screen
	if net_GlobalUpdate_RouterConnection( ) == true then
		return false
	end
	
	-- check matching sessions for errors
	if net_GlobalUpdate_MatchingSessions( ) == true then
		return false
	end

	-- check for game invites that need processing.	
	if net_GlobalUpdate_GameInvite( ) == true then
		return false
	end

	-- check for commands from host if required.
	if net_GlobalUpdate_RaceInvite( ) == true then
		return false
	end
	
	if net_GlobalUpdate_PartyCommands( ) == true then
		return false
	end
			
	return true
end
