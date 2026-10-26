GUI =
{
	finished = false,
	stringNo = 0,

	NetTimer	= 0,
	NetTimeStep = 0.5,
	
	CooldownTime	= 2,
	GarageTimer		= 2,
	ModShopTimer	= 2,

	CanStart			= false,
	CanViewOptions		= true,

	SubScreenActive		= nil,
	SubScreensFlushed	= false,

	current_item = 1,
	
	CurrentList = 0,
	CurrentEvent = 0,
	PlaylistSize = 1,
	RaceOverviewID = -1,

	EnableColour = "Support_0",
	DisabledColour = "Support_3",
	SelectedSlot = -1,

	Players = {},
	MaxPlayers = 20,

	RaceFinished	= false,
	RaceIndex		= -1,

	ChangeTable = 
	{
		view	= nil,
		options = nil,
		invite	= nil,
	},

	TableMap =
	{
		"view",
		"options",
		"invite",
	},

	NewItemsAvailible = false,

	TeamCountdown = 0,
	CountdownTime = 0,
	CountdownTimeSfx = 0,
	
	TimerPrefix			= {},
	CountdownDefaults	= {},
	
	Disconnecting	= false,
	Migrating		= false,

	CanViewMap		= false,
	
	CanExit = function( _GUI )
		return false
	end,
}

function Init()
	AddSCUI_Elements( )
	Amax.ChangeUiCamera( UIGlobals.CameraNames.MpLobby, UIGlobals.CameraLerpTime, 0 )
	net_SetRichPresence( UIEnums.RichPresence.MpLobby )

	net_LockoutFriendsOverlay( false )
	
	-- we can release any enumerated session data now as it's no longer needed
	net_FlushSessionEnumerator()

	-- Network setup.
	NetRace.EnterLobby()
	
	UIScreen.SetScreenTimers( 0.3, 0.3 )
		
	UISystem.PlaySound( UIEnums.SoundEffect.LobbyJoin )
	
	StoreInfoLine()
	SetupInfoLine()

	GUI.TimerPrefix = 
	{
		[UIEnums.MpLobbyState.FindingPlayers]	= UIText.MP_CMN_NOWT,
		[UIEnums.MpLobbyState.Waiting]			= UIText.MP_CMN_NOWT,
		[UIEnums.MpLobbyState.WaitingToVote]	= "MPL_TIMER_VOTE_START",
		[UIEnums.MpLobbyState.Voting]			= "MPL_TIMER_VOTE_TIME",
		[UIEnums.MpLobbyState.Countdown]		= "MPL_TIMER_COUNTDOWN",
		[UIEnums.MpLobbyState.TeamCountdown]	= "MPL_TIMER_COUNTDOWN",
		[UIEnums.MpLobbyState.TeamBalanceWait]	= "MPL_TIMER_COUNTDOWN",
		[UIEnums.MpLobbyState.WaitingForHost]	= UIText.MP_CMN_NOWT,
		[UIEnums.MpLobbyState.Ready]			= UIText.MP_CMN_NOWT,
	}

	GUI.CountdownDefaults = 
	{
		[UIEnums.MpLobbyState.FindingPlayers]	= 0,
		[UIEnums.MpLobbyState.Waiting]			= 0,
		[UIEnums.MpLobbyState.WaitingToVote]	= 3,
		[UIEnums.MpLobbyState.Voting]			= 10,
		[UIEnums.MpLobbyState.Countdown]		= 40,
		[UIEnums.MpLobbyState.TeamCountdown]	= 10,
		[UIEnums.MpLobbyState.TeamBalanceWait]	= 0,
		[UIEnums.MpLobbyState.WaitingForHost]	= 0,
		[UIEnums.MpLobbyState.Ready]			= 0,
	}

	-- store this
	UIGlobals.Multiplayer.InitialLobby = NetRace.IsFirstRace()
	UIGlobals.Multiplayer.InLobby = true

	if Amax.GetPlayMode() ~= UIEnums.PlayMode.Playlist then
		Multiplayer.CacheNetRaceSettings() -- TODO_MP_REFACTOR
	end

	-- current lobby state
	if NetRace.IsHost() == true then
		if Amax.IsGameModeRanked() == true then
			UIGlobals.Multiplayer.LobbyState = Select( UIGlobals.Multiplayer.InitialLobby, UIEnums.MpLobbyState.FindingPlayers, UIEnums.MpLobbyState.Waiting )
		else
			UIGlobals.Multiplayer.LobbyState = Select( UIGlobals.Multiplayer.InitialLobby, UIEnums.MpLobbyState.WaitingForHost, UIEnums.MpLobbyState.Waiting )
		end
	else	
		-- [WillB] #37637 - We could join the lobby just as it loads causing us to miss the start race packet, and we would be stuck in countdown state
		MpLobby_ValidateState()
		
		-- whats the lobby state?
		UIGlobals.Multiplayer.LobbyState, UIGlobals.Multiplayer.LobbyTimer = NetRace.GetLobbyState()
	end

	GUI.RaceIndex = Multiplayer.GetCurrentRaceIndex()

	print( "-----------Enter Lobby-------------" )
	print( "Lobby MpLobbyState", UIGlobals.Multiplayer.LobbyState )
	show_table( UIEnums.MpLobbyState, 1 )

	GUI.RaceOverviewID = SCUI.name_to_id[ "node_current_race" ]
	
	-- Just incase mpmain isnt running.
	if UIGlobals.Multiplayer.InitialLobby == true and IsFunction( EnableCarousel ) == true then
		EnableCarousel( false )
	end

	GUI.Lobby		= SCUI.name_to_id[ "lobby" ]
	GUI.TimeoutID	= SCUI.name_to_id[ "lobby_info" ]
	
	-- Create the player lists
	GUI.PlayersLeft = SCUI.name_to_id[ "left_list" ]
	GUI.PlayersRight = SCUI.name_to_id[ "right_list" ]
	
	GUI.MaxPlayers = NetRace.MaxPlayerCount()
	
	local index = 1
	local half_players = GUI.MaxPlayers / 2
	
	-- create left side player list
	for i = 1, half_players do
		local root_id = UIButtons.CloneXtGadgetByName( "Multiplayer\\MpLobby.lua", "mp_player_left" )

		GUI.Players[index] = SetupPlayer( root_id, index - 1 )
		
		UIButtons.AddListItem( GUI.PlayersLeft, root_id, index )
		index = index + 1
	end
	
	-- create right side player list
	for i = 1, half_players do
		local root_id = UIButtons.CloneXtGadgetByName( "Multiplayer\\MpLobby.lua", "mp_player_right" )

		GUI.Players[index] = SetupPlayer( root_id, index - 1 )
		
		UIButtons.AddListItem( GUI.PlayersRight, root_id, index )
		index = index + 1
	end
	
	-- enable left, disable right
	GUI.CurrentList = GUI.PlayersLeft
	
	-- only valid in online mode
	if Amax.GetGameMode() == UIEnums.GameMode.Online then
		net_EnableGlobalUpdateRichPresence( true )
	end

	GUI.Slots = Multiplayer.GetLobbySlots( GUI.Lobby )

	if UIGlobals.IsIngame == true then
		GUI.camera_id = UIButtons.CloneXtGadgetByName( "SCUIBank", "Cam_Carousel" )
		UIButtons.SetParent( GUI.camera_id, SCUI.name_to_id[ "CameraDolly" ], UIEnums.Justify.MiddleCentre )
	end

	if NetRace.IsPrivateGame() == true then
		UIButtons.ChangeText( SCUI.name_to_id["title"], UIText.MP_PRIVATE_LOBBY )
	end

	if Amax.GetPlayMode() == UIEnums.PlayMode.CustomRace or Amax.GetGameMode() == UIEnums.GameMode.SystemLink then
		GUI.CanViewMap = false
	else
		GUI.CanViewMap = true
	end
	
	-- create lobby title and players data
	if Amax.GetPlayMode() == UIEnums.PlayMode.Playlist then
		UIButtons.ChangeText( SCUI.name_to_id[ "title" ], "MPL_PLAYLIST_NAME" .. Multiplayer.GetCurrentEventIndex() )

		local event = Multiplayer.GetPlaylist()
		UIShape.ChangeObjectName( SCUI.name_to_id[ "title_icon" ], event.icon_name )		
	else
		UIButtons.ChangeText( SCUI.name_to_id[ "title" ], UIText.MP_CUSTOM_GAME )		
	end
	
	-- [Bone] #37613 - Make system link lobby start button readable
	if UIEnums.CurrentPlatform == UIEnums.Platform.PS3 then
		UIButtons.ChangeScale( SCUI.name_to_id[ "start_button" ], 2.2, 2.2, 2.2 )
	end
end

function PostInit()
	UpdateLoadout()
	UpdateCar()
	
	UIButtons.SetSelected( GUI.PlayersLeft, true )
	UIButtons.SetSelected( GUI.PlayersRight, false )

	-- turn off the clones
	UIButtons.SetActive( SCUI.name_to_id[ "mp_player_left" ], false )
	UIButtons.SetActive( SCUI.name_to_id[ "mp_player_right" ], false )

	MpLobby_RefreshCurrentRace()
	RefreshLobby()
	
	UpdateLobbyInfo( true )
	MpLobby_ShowNew( false )

	if NetRace.IsHost() == false and UIGlobals.Multiplayer.LobbyState == UIEnums.MpLobbyState.Voting then
		StartVoting()
	end
	GetSetLobbyCar()
	
	-- [WillB] #37785 TCR:R041 - Need to present the user with a message informing them 
	if UIGlobals.Network.VoiceChatRestricted == true then
		SetupCustomPopup( UIEnums.CustomPopups.PS3NoVoiceAllowed )
	end
end


function MessageUpdate( message, arg1, arg2, arg3, arg4 )
		
	if SCUI_MessageUpdate( message, arg1, arg2, arg3, arg4 ) == true then
		return
	end

	if message == UIEnums.GameMessage.RefreshLobby then
		MpLobby_RefreshCurrentRace()
		GetSetLobbyCar()
		UpdateCar()
		UpdateLoadout()

		if NetRace.IsHost() == false and arg1 == 0 then
			SetupCustomPopup( UIEnums.CustomPopups.HostChangedSettings )
		end
	end
	
	if SubScreenActive() == true then
		return
	end
	
	if message == UIEnums.Message.MenuNext then
		local selection = UIButtons.GetSelection( GUI.CurrentList )
		if GUI.Slots[ selection ].local_player == false and GUI.Slots[ selection ].empty == false then
			UIGlobals.Multiplayer.SelectPlayerJoinRef = GUI.Slots[ selection ].ref
			SetupCustomPopup( UIEnums.CustomPopups.ViewPlayer )
		end
		
	elseif message == UIEnums.Message.MenuBack then
		SetupCustomPopup( UIEnums.CustomPopups.ExitRaceLobby )
		
	elseif message == UIEnums.Message.ButtonRight and arg2 == true then
		if CanMoveRight() == true then
			UIButtons.SetSelected( GUI.PlayersLeft, false )
			UIButtons.SetSelected( GUI.PlayersRight, true )
			
			-- set selection to what is was on the othersize
			UIButtons.SetSelectionByIndex( GUI.PlayersRight, UIButtons.GetSelectionIndex( GUI.PlayersLeft ) )
			
			GUI.CurrentList = GUI.PlayersRight
		end
	elseif message == UIEnums.Message.ButtonLeft and arg2 == true then
		if CanMoveLeft() == true then
			UIButtons.SetSelected( GUI.PlayersLeft, true )
			UIButtons.SetSelected( GUI.PlayersRight, false )
			
			-- set selection to what is was on the othersize
			UIButtons.SetSelectionByIndex( GUI.PlayersLeft, UIButtons.GetSelectionIndex( GUI.PlayersRight ) )
		
			GUI.CurrentList = GUI.PlayersLeft
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

	-- cant vote yet
	if Amax.GetPlayMode() == UIEnums.PlayMode.Playlist and ( UIGlobals.Multiplayer.LobbyState == UIEnums.MpLobbyState.WaitingToVote or UIGlobals.Multiplayer.LobbyState == UIEnums.MpLobbyState.Voting ) then
		return -- dont allow other messages
	end

	if message == UIEnums.Message.ButtonStart and MpLobby_HostCanPressStart() == true and arg2 == true then
		NetRace.LaunchCustomRace()
		UIGlobals.Multiplayer.LobbyState = UIEnums.MpLobbyState.Ready

	elseif GUI.CanViewMap == true and message == UIEnums.Message.ButtonRightShoulder and NetRace.RaceInProgress() == true and NetRace.HasRaceFinished() == false then
		PushScreen( "Multiplayer\\MpRacePreview.lua" )
	
	elseif message == UIEnums.Message.ButtonX and MpLobby_CanViewMenu() == true and arg2 == true then
		SetupCustomPopup( UIEnums.CustomPopups.LobbyOptions )

	elseif message == UIEnums.Message.PopupNext and arg2 == UIEnums.CustomPopups.LobbyOptions then
		
		if arg3 == UIEnums.LobbyOptions.CarSelect then
			-- setup class filter for mini garage
			UIGlobals.GarageMiniClassFilter = NetRace.GetGarageMiniClassFilters()
			--PushScreen( "Shared\\GarageMini.lua" )
			PushScreen( "Shared\\Garage.lua" )
		
		elseif arg3 == UIEnums.LobbyOptions.ModShop then
			PushScreen( "Multiplayer\\ModShop\\MpModShop.lua" )

		elseif arg3 == UIEnums.LobbyOptions.Challenges then
			PushScreen( "Multiplayer\\Challenges\\MpChallenges.lua" )

		elseif arg3 == UIEnums.LobbyOptions.SessionScoreboard then
			PushScreen( "Multiplayer\\MpShowSessionLeaderboard.lua" )

		elseif arg3 == UIEnums.LobbyOptions.RaceSummery then
			PushScreen( "Multiplayer\\MpShowRaceSummary.lua" )

		elseif GUI.CanViewMap == true and arg3 == UIEnums.LobbyOptions.RacePreview then
			PushScreen( "Multiplayer\\MpRacePreview.lua" )

		elseif arg3 == UIEnums.LobbyOptions.Settings then
			PlaySfxGraphicBack()
			PushScreen( "Multiplayer\\Shared\\MpCustomRace.lua" )
		end

	elseif message == UIEnums.Message.ButtonLeftShoulder and arg2 == true then

		if Amax.CanShareInLobby() == true and Amax.CanUseShare() == true then
			if GUI.Slots[ UIButtons.GetSelectionIndex( GUI.CurrentList ) + 1 ].empty == false then
				
				UIGlobals.SharingOptionsChosen=-1

				if UIButtons.GetSelection( GUI.CurrentList ) == 1 then --player
					SetupCustomPopup( UIEnums.CustomPopups.MPLobbyLocalSharingOptions )
				else
					UIGlobals.ShareFromWhatPopup=-1
					SetupCustomPopup( UIEnums.CustomPopups.SharingOptions )
				end
			end
		end

	elseif message == UIEnums.Message.PopupNext and arg2 == UIEnums.CustomPopups.SharingOptions then

		if arg3 == UIEnums.ShareOptions.Facebook then

			if UIGlobals.SharingOptionsChosen == -1 then

				Amax.CreateBlurb(UIEnums.SocialNetworkingItemType.MPRelativePlayerPositionAbove, 1, UIButtons.GetSelection( GUI.CurrentList ))
				--Amax.CreateBlurb(UIEnums.SocialNetworkingItemType.MPRelativePlayerPositionBelow, 1, UIButtons.GetSelection( GUI.CurrentList ))

			else

				local join_ref=GUI.Slots[ UIButtons.GetSelectionIndex( GUI.CurrentList ) + 1 ].ref

				if UIGlobals.SharingOptionsChosen == UIEnums.MPLobbyLocalSharingOptions.Position then
					Amax.CreateBlurb(UIEnums.SocialNetworkingItemType.MPFinishingPosition, 1, join_ref )
				elseif UIGlobals.SharingOptionsChosen == UIEnums.MPLobbyLocalSharingOptions.Rank then
					Amax.CreateBlurb(UIEnums.SocialNetworkingItemType.MPPlayersRank, 1, -1)
				else
					Amax.CreateBlurb(UIEnums.SocialNetworkingItemType.MPRaceAward, 1, UIGlobals.SharingOptionsChosen-UIEnums.MPLobbyLocalSharingOptions.Award1)
				end
			end

			StoreScreen( UIEnums.ScreenStorage.FE_SOCIAL_NETWORK, "Multiplayer\\MpLobby.lua" )
			PushScreen( "Shared\\Facebook.lua", UIEnums.Context.Blurb )
		
		elseif arg3 == UIEnums.ShareOptions.Twitter then

			if UIGlobals.SharingOptionsChosen == -1 then

				Amax.CreateBlurb(UIEnums.SocialNetworkingItemType.MPRelativePlayerPositionAbove, 0, UIButtons.GetSelection( GUI.CurrentList ))
				--Amax.CreateBlurb(UIEnums.SocialNetworkingItemType.MPRelativePlayerPositionBelow, 0, UIButtons.GetSelection( GUI.CurrentList ))

			else

				local join_ref=GUI.Slots[ UIButtons.GetSelectionIndex( GUI.CurrentList ) + 1 ].ref

				if UIGlobals.SharingOptionsChosen == UIEnums.MPLobbyLocalSharingOptions.Position then
					Amax.CreateBlurb(UIEnums.SocialNetworkingItemType.MPFinishingPosition, 0, join_ref)
				elseif UIGlobals.SharingOptionsChosen == UIEnums.MPLobbyLocalSharingOptions.Rank then
					Amax.CreateBlurb(UIEnums.SocialNetworkingItemType.MPPlayersRank, 0, -1)
				else
					Amax.CreateBlurb(UIEnums.SocialNetworkingItemType.MPRaceAward, 0, UIGlobals.SharingOptionsChosen-UIEnums.MPLobbyLocalSharingOptions.Award1)
				end

			end

			StoreScreen( UIEnums.ScreenStorage.FE_SOCIAL_NETWORK, "Multiplayer\\MpLobby.lua" )
			PushScreen( "Shared\\Twitter.lua", UIEnums.Context.Blurb )

		elseif arg3 == UIEnums.ShareOptions.Blurb then

			if UIGlobals.SharingOptionsChosen == -1 then

				Amax.CreateBlurb(UIEnums.SocialNetworkingItemType.MPRelativePlayerPositionAbove, 2, UIButtons.GetSelection( GUI.CurrentList ))

			else

				local join_ref=GUI.Slots[ UIButtons.GetSelectionIndex( GUI.CurrentList ) + 1 ].ref

				if UIGlobals.SharingOptionsChosen == UIEnums.MPLobbyLocalSharingOptions.Position then
					Amax.CreateBlurb(UIEnums.SocialNetworkingItemType.MPFinishingPosition, 2, join_ref)
				elseif UIGlobals.SharingOptionsChosen == UIEnums.MPLobbyLocalSharingOptions.Rank then
					Amax.CreateBlurb(UIEnums.SocialNetworkingItemType.MPPlayersRank, 2, -1)
				else
					Amax.CreateBlurb(UIEnums.SocialNetworkingItemType.MPRaceAward, 2, UIGlobals.SharingOptionsChosen-UIEnums.MPLobbyLocalSharingOptions.Award1)
				end

			end
			
		end		
	end
end

function FrameUpdate( time )
	-- if we are trying to shut down the lobby then its best to stop the update from running as the game modes and net race are shutting down
	-- and its not a good idea to start invoking calls on games modes that might not be valid any more.
	if GUI.finished == true or UIGlobals.Multiplayer.NotEnoughPlayersKick == true or GUI.Disconnecting == true then
		return
	end
	
	-- catch the local player disconnecting and flag it
	local disconnecting = NetRace.LocalPlayerDisconnecting()
	if disconnecting == true then
		GUI.Disconnecting = disconnecting
		return -- no further processing
	end
	
	-- catch the mirgration state changing so we can provide feedback for the player
	local migrating = NetRace.IsSessionMigrating()
	if migrating ~= GUI.Migrating then
		GUI.Migrating = migrating
		UpdateLobbyInfo( nil, true )
	end

	UpdateVoiceInfo()

	-- update lobby selection
	if GUI.SubScreenActive ~= SubScreenActive() then
		GUI.SubScreenActive = SubScreenActive()

		EnablePlayerList( not GUI.SubScreenActive )
	end

	-- update lobby state every 500ms
	if NetRace.IsHost() == true then
		GUI.NetTimer = GUI.NetTimer + time
		if GUI.NetTimer >= GUI.NetTimeStep then
			NetRace.SendLobbyState( UIGlobals.Multiplayer.LobbyState, UIGlobals.Multiplayer.LobbyTimer )
			GUI.NetTimer = 0
		end
	else 	
		-- [WillB] #37637 - We could join the lobby just as it loads causing us to miss the start race packet, and we would be stuck in countdown state
		MpLobby_ValidateState()
		
		-- clients check state in case its changed
		local state, time = NetRace.GetLobbyState()
		
		-- if we are migrating, then keep the timer to the last known value. Check we are in sync
		if math.abs( UIGlobals.Multiplayer.LobbyTimer - time ) >= 1.5 then
			UIGlobals.Multiplayer.LobbyTimer = time
		end

		if UIGlobals.Multiplayer.LobbyState ~= state then
			print( "LOBBY - Change state", state )

			-- voting has just finished
			if UIGlobals.Multiplayer.LobbyState == UIEnums.MpLobbyState.Voting then
				EndVoting()
			end

			-- add any special client logic here
			if state == UIEnums.MpLobbyState.WaitingToVote then
				UIScreen.CancelPopup()
				FlushSubscreens()

			elseif state == UIEnums.MpLobbyState.Voting then
				StartVoting()
			end

			-- change the state
			UIGlobals.Multiplayer.LobbyState = state
			UIGlobals.Multiplayer.LobbyTimer = time
			UpdateLobbyInfo()
		end
		
		-- uber fail safe incase the client has managed to get the wrong race index
		local race_index = Multiplayer.GetCurrentRaceIndex()
		if GUI.RaceIndex ~= race_index then
			print("Resyncing lobby race index ", race_index)
			GUI.RaceIndex = race_index
			MpLobby_RefreshCurrentRace()
		end
	end

	-- update timer
	if MpLobby_CanCountdown() == true and GUI.Migrating == false then
		UIGlobals.Multiplayer.LobbyTimer = UIGlobals.Multiplayer.LobbyTimer - time
		
		local new_time = math.ceil( UIGlobals.Multiplayer.LobbyTimer )
		UIButtons.ChangeText( GUI.TimeoutID, GUI.TimerPrefix[UIGlobals.Multiplayer.LobbyState] .. new_time )
		
		-- Only want the sfx on the countdown stage
		if UIGlobals.Multiplayer.LobbyState == UIEnums.MpLobbyState.Countdown then
			if new_time ~= GUI.CountdownTimeSfx then
				GUI.CountdownTimeSfx = new_time
				
				-- dont play the sound on zero
				if new_time > 0 then
					if new_time <= 3 then
						UISystem.PlaySound( UIEnums.SoundEffect.LobbyCountDown03 )
					elseif new_time <= 10 then
						UISystem.PlaySound( UIEnums.SoundEffect.LobbyCountDown10 )
					end
				end
			end
		end

		if UIGlobals.Multiplayer.LobbyTimer <= 0 then
			UIGlobals.Multiplayer.LobbyTimer = 0
		end
	end
	
	-- update loadout
	if UIGlobals.Multiplayer.LobbyUpdateLoadout == true then
		UpdateLoadout()
		UIGlobals.Multiplayer.LobbyUpdateLoadout = false
	end

	-- update car select
	if UIGlobals.Multiplayer.LobbyUpdateCar then
		UpdateCar()
		UIGlobals.Multiplayer.LobbyUpdateCar = false
	end

	if UIButtons.GetSelectionIndex( GUI.CurrentList ) + 1 ~= GUI.SelectedSlot then
		GUI.SelectedSlot = UIButtons.GetSelectionIndex( GUI.CurrentList ) + 1
	end

	if Multiplayer.RefreshLobby( GUI.Lobby ) == true then
		RefreshLobby()
	end

	-- clients and host should check this logic for finding player timeouts
	if UIGlobals.Multiplayer.LobbyState == UIEnums.MpLobbyState.FindingPlayers then
		-- enough player now found so continue as normal
		if MpLobby_PlayersValid() == true then
			if NetRace.IsHost() == true then
				UIGlobals.Multiplayer.LobbyState = UIEnums.MpLobbyState.Countdown
				UpdateLobbyInfo()
			end
		end
	end

	-- only the host can control the lobby state and peers state
	if NetRace.IsHost() == true then
		-- voting/countdown hasnt started or the host hasnt pressed start yet
		if UIGlobals.Multiplayer.LobbyState == UIEnums.MpLobbyState.Waiting then

			if Amax.GetPlayMode() == UIEnums.PlayMode.Playlist then
				if NetRace.CanStartVoting() == true then
					if Multiplayer.IsVotingEnabled() == true then
						UIGlobals.Multiplayer.LobbyState = UIEnums.MpLobbyState.WaitingToVote

						-- close any screens that are open
						UIScreen.CancelPopup()
						FlushSubscreens()
					else
						if Amax.IsGameModeRanked() == true then	
							UIGlobals.Multiplayer.LobbyState = UIEnums.MpLobbyState.Countdown
						else
							UIGlobals.Multiplayer.LobbyState = UIEnums.MpLobbyState.WaitingForHost
						end
					end
					UpdateLobbyInfo()
				end
			else
				UIGlobals.Multiplayer.LobbyState = UIEnums.MpLobbyState.WaitingForHost
				UpdateLobbyInfo()
			end

		elseif UIGlobals.Multiplayer.LobbyState == UIEnums.MpLobbyState.WaitingToVote then
			if UIGlobals.Multiplayer.LobbyTimer <= 0 then
				StartVoting()
			end

		elseif UIGlobals.Multiplayer.LobbyState == UIEnums.MpLobbyState.Voting then
			if UIGlobals.Multiplayer.VotingFinished == true then
				EndVoting()
			end

		elseif UIGlobals.Multiplayer.LobbyState == UIEnums.MpLobbyState.Countdown then
			-- we need to pause the timer if its a team game
			local next_state = Select( NetRace.IsTeamGame(), UIEnums.MpLobbyState.TeamBalanceWait, UIEnums.MpLobbyState.FindingPlayers )
		
			-- if we ever run out of players then we need to wait for more
			if MpLobby_PlayersValid() == false then
				GUI.TeamCountdown = UIGlobals.Multiplayer.LobbyTimer -- save away the current timer so we dont have to start from zero again
			
				UIGlobals.Multiplayer.LobbyState = next_state
				UpdateLobbyInfo()
			else
				if UIGlobals.Multiplayer.LobbyTimer <= 0 then
					UIGlobals.Multiplayer.LobbyState = UIEnums.MpLobbyState.Ready
					UpdateLobbyInfo()
				end
			end			
			
		elseif UIGlobals.Multiplayer.LobbyState == UIEnums.MpLobbyState.TeamBalanceWait then
			if MpLobby_PlayersValid() == true then
				UIGlobals.Multiplayer.LobbyState = UIEnums.MpLobbyState.Countdown
				UpdateLobbyInfo()
				
				-- set time back to what it last was unless its under 10
				if GUI.TeamCountdown < GUI.CountdownDefaults[UIEnums.MpLobbyState.TeamCountdown] then
					UIGlobals.Multiplayer.LobbyTimer = GUI.CountdownDefaults[UIEnums.MpLobbyState.TeamCountdown]
				else
					UIGlobals.Multiplayer.LobbyTimer = GUI.TeamCountdown
				end
			end

		elseif UIGlobals.Multiplayer.LobbyState == UIEnums.MpLobbyState.Ready then
			if Amax.IsGameModeRanked() == true then
				-- we need to pause the timer if its a team game
				local next_state = Select( NetRace.IsTeamGame(), UIEnums.MpLobbyState.TeamBalanceWait, UIEnums.MpLobbyState.FindingPlayers )
		
				-- if we ever run out of players then we need to wait for more
				if MpLobby_PlayersValid() == false then
					UIGlobals.Multiplayer.LobbyState = next_state
					UpdateLobbyInfo()
				else	
					if NetRace.IsHost() == true and NetRace.CanStartCustomRace() then
						NetRace.LaunchCustomRace()
					end
				end
			end
			UIGlobals.Multiplayer.LobbyTimer = 0
		end
	end
	
	-- host and client should check to see if the race has now finished, and update the UI
	if UIGlobals.Multiplayer.LobbyState == UIEnums.MpLobbyState.Waiting then	
		local finished = NetRace.HasRaceFinished()
		if finished ~= GUI.RaceFinished then
			GUI.RaceFinished = finished
			UpdateLobbyInfo()
		end
	end

	-- can start
	if Amax.IsGameModeRanked() == false then
		local can_start = MpLobby_HostCanPressStart()
		if GUI.CanStart ~= can_start then
			GUI.CanStart = can_start
			UpdateLobbyInfo()
		end
	end

	-- Ready to load the race - make sure we arnt disconnecting
	if NetRace.IsStartRaceDataValid() == true then

		-- Fail safe incase mod shop or garage is open
		if SubScreenActive() == true then
			if GUI.SubScreensFlushed == false then
				GUI.SubScreensFlushed = true
				FlushSubscreens()
			end
		else
			local race = Multiplayer.GetCurrentRace()
			if IsTable( race ) == true then
				Amax.LoadTextureClone( race.thumbnail )
				UIButtons.TimeLineActive( "lobby_fade", true )
			end
			
			-- force it to finish
			StartLobbyNetworkLoad()
			GUI.finished = true
		end
	end

	-- stop it overriding
	if SubScreenActive() == false then
		Lobby_RefreshInfoLine()
	end

	-- check for any new items
	local new_table = Multiplayer.GetNewUnlocks()
	local new_cars = Multiplayer.GetNewCarsForClass( NetRace.GetCarClass() ) -- TODO_MP_REFACTOR this wont work when brought to release

	-- if any of these are new
	local newItems = ( new_cars or new_table.mods or new_table.challenges )

	if GUI.NewItemsAvailible ~= newItems then
		GUI.NewItemsAvailible = newItems
		MpLobby_ShowNew( GUI.NewItemsAvailible )
	end

	-- so mod shop and garage know to display timer
	UIGlobals.Multiplayer.CountdownStarted = ( UIGlobals.Multiplayer.LobbyState == UIEnums.MpLobbyState.Countdown )
end

function EnterEnd()
	UISystem.PlaySound( UIEnums.SoundEffect.LobbyLeave )
	RestoreInfoLine()
end

function End()
	UIGlobals.Multiplayer.InLobby = false
	FlushSubscreens()
end


function EnablePlayerList( active )
	UIButtons.SetSelected( GUI.CurrentList, active )
end

function UpdateLobbyInfo( post_init, finish_migration )
	local lobby_info_icon = SCUI.name_to_id[ "lobby_info_icon" ]
	local lobby_state = SCUI.name_to_id[ "lobby_state" ]
	local lobby_info = SCUI.name_to_id[ "lobby_info" ]
	local summary_title = SCUI.name_to_id[ "summery_title" ]
	local rb_button = SCUI.name_to_id[ "button" ]

	UIButtons.ChangeColour( lobby_info, "Support_2" )
	UIButtons.SetActive( rb_button, false )

	if UIGlobals.Multiplayer.LobbyState == UIEnums.MpLobbyState.FindingPlayers then
		UIShape.ChangeSceneName( lobby_info_icon, "fe_icons" )
		UIShape.ChangeObjectName( lobby_info_icon, "join_game" )

		UIButtons.ChangeColour( lobby_info_icon, "Support_3" )
		UIButtons.ChangeColour( lobby_info, "Support_3" )
		UIButtons.ChangeText( lobby_state, UIText.MP_PLEASE_WAIT )
		UIButtons.ChangeText( lobby_info, UIText.MP_WAITING_FOR_PLAYERS )
		UIButtons.ChangeText( summary_title, UIText.MP_NEXT_RACE )

		UpdateCar( true )

	elseif UIGlobals.Multiplayer.LobbyState == UIEnums.MpLobbyState.Waiting then
		if GUI.RaceFinished == true then
			UIShape.ChangeSceneName( lobby_info_icon, "common_icons" )
			UIShape.ChangeObjectName( lobby_info_icon, "end_of_race" )

			UIButtons.ChangeColour( lobby_info_icon, "Support_1" )
			UIButtons.ChangeText( lobby_state, UIText.MP_PLEASE_WAIT )
			UIButtons.ChangeText( lobby_info, UIText.MP_FINISHED )
			UIButtons.ChangeText( summary_title, UIText.MP_RACE_SUMMARY )
		else
			UIShape.ChangeSceneName( lobby_info_icon, "fe_icons" )
			UIShape.ChangeObjectName( lobby_info_icon, "racing" )

			UIButtons.ChangeColour( lobby_info_icon, "Support_3" )
			UIButtons.ChangeColour( lobby_info, "Support_3" )
			UIButtons.ChangeText( lobby_info, UIText.MP_RACE_IN_PROGRESS )
			UIButtons.ChangeText( summary_title, UIText.MP_RACE_SUMMARY )

			if NetRace.RaceInProgress() == true then
				if GUI.CanViewMap == true then
					UIButtons.ChangeText( lobby_state, UIText.MP_RACE_PREVIEW )				
					UIButtons.SetActive( rb_button, true )
				end
			else
				UIButtons.ChangeText( lobby_state, UIText.MP_PLEASE_WAIT )
			end
		end

		UpdateCar( true )
	
	elseif UIGlobals.Multiplayer.LobbyState == UIEnums.MpLobbyState.WaitingToVote then
		UIShape.ChangeSceneName( lobby_info_icon, "common_icons" )
		UIShape.ChangeObjectName( lobby_info_icon, "vote" )

		UIButtons.ChangeColour( lobby_info_icon, "Main_White" )
		UIButtons.ChangeText( lobby_state, UIText.MP_PLEASE_WAIT )
		UIButtons.ChangeText( summary_title, UIText.MP_RACE_SUMMARY )

		UpdateCar( true )
	
	elseif UIGlobals.Multiplayer.LobbyState == UIEnums.MpLobbyState.Countdown then
		UIShape.ChangeSceneName( lobby_info_icon, "fe_icons" )
		UIShape.ChangeObjectName( lobby_info_icon, "steering_wheel" )

		UIButtons.ChangeColour( lobby_info_icon, "Support_0" )
		UIButtons.ChangeColour( lobby_info, "Support_0" )
		UIButtons.ChangeText( lobby_state, UIText.MP_GET_READY_TO_RACE )
		UIButtons.ChangeText( summary_title, UIText.MP_NEXT_RACE )

		UpdateCar()
	
	elseif UIGlobals.Multiplayer.LobbyState == UIEnums.MpLobbyState.WaitingForHost then
		if Amax.IsGameModeRanked() == false then

			UIShape.ChangeSceneName( lobby_info_icon, "fe_icons" )
			UIShape.ChangeObjectName( lobby_info_icon, "steering_wheel" )
			UpdateCar()

			UIButtons.ChangeColour( lobby_info_icon, "Support_0" )
			UIButtons.ChangeText( lobby_state, UIText.MP_GET_READY_TO_RACE )
			UIButtons.ChangeText( summary_title, UIText.MP_NEXT_RACE )

			-- the host cant press start if a player is connecting
			if NetRace.IsHost() == true then
				UIButtons.ChangeText( GUI.TimeoutID, Select( MpLobby_HostCanPressStart(), UIText.CMN_NOWT, UIText.MP_WAITING_FOR_PLAYERS ) )
			else
				UIButtons.ChangeText( GUI.TimeoutID, UIText.MP_WAITING_FOR_HOST )
			end
		end
		
	elseif UIGlobals.Multiplayer.LobbyState == UIEnums.MpLobbyState.TeamBalanceWait then
		UIShape.ChangeSceneName( lobby_info_icon, "fe_icons" )
		UIShape.ChangeObjectName( lobby_info_icon, "join_game" )

		UIButtons.ChangeColour( lobby_info_icon, "Support_3" )
		UIButtons.ChangeColour( lobby_info, "Support_3" )
		UIButtons.ChangeText( lobby_state, UIText.MP_PLEASE_WAIT )
		UIButtons.ChangeText( lobby_info, UIText.MP_BALANCING_TEAMS )
		UIButtons.ChangeText( summary_title, UIText.MP_NEXT_RACE )
		
		UpdateCar()
		
	elseif UIGlobals.Multiplayer.LobbyState == UIEnums.MpLobbyState.Ready then
		UIButtons.ChangeColour( lobby_info, "Support_0" )
		UpdateCar()
	end
	
	-- migration takes priorty over everything
	if GUI.Migrating == true then
		local lobby_info_icon = SCUI.name_to_id[ "lobby_info_icon" ]
		local lobby_state = SCUI.name_to_id[ "lobby_state" ]
		local lobby_info = SCUI.name_to_id[ "lobby_info" ]
	
		UIShape.ChangeSceneName( lobby_info_icon, "fe_icons" )
		UIShape.ChangeObjectName( lobby_info_icon, "join_game" )

		UIButtons.ChangeColour( lobby_info_icon, "Support_3" )
		UIButtons.ChangeColour( lobby_info, "Support_3" )
		UIButtons.ChangeText( lobby_state, UIText.MP_PLEASE_WAIT )
		UIButtons.ChangeText( lobby_info, UIText.MP_HOST_MIGRATING )
	end

	if NetRace.IsHost() == true and finish_migration ~= true then
		UIGlobals.Multiplayer.LobbyTimer = GUI.CountdownDefaults[UIGlobals.Multiplayer.LobbyState]
	end

	if post_init ~= true then
		PlaySfxToggle()
	end
end


function Lobby_RefreshInfoLine()

	local lobby_selection = UIButtons.GetSelection( GUI.CurrentList )

	-- get new values
	local change_table = 
	{
		view	= ( GUI.Slots[ lobby_selection ].local_player == false and GUI.Slots[ lobby_selection ].empty == false ),
		options = MpLobby_CanViewMenu(),
		invite	= Mp_XenonGameInviteAvailable(),
	}

	-- check for changes
	local update = false
	for i, v in pairs( GUI.TableMap ) do
		if change_table[v] ~= GUI.ChangeTable[v] then
			update = true
			break
		end
	end

	-- [WillB] #37608 - Dont show start button while migrating
	UIButtons.SetActive( SCUI.name_to_id["press_start"], ( GUI.CanStart == true and GUI.Migrating == false ), true )
	
	-- update?
	if update == false then
		return
	end

	-- store current state
	GUI.ChangeTable.view		= ( GUI.Slots[ lobby_selection ].local_player == false and GUI.Slots[ lobby_selection ].empty == false )
	GUI.ChangeTable.options		= MpLobby_CanViewMenu()
	GUI.ChangeTable.invite		= Mp_XenonGameInviteAvailable()

	-- change info lines
	if GUI.ChangeTable.view == true then
		SetupInfoLine( UIText.INFO_VIEW_PLAYER, UIText.INFO_EXIT_LOBBY )
	else
		SetupInfoLine( UIText.INFO_EXIT_LOBBY )
	end

	if GUI.ChangeTable.options == true then
		PushInfoLine( UIText.INFO_LOBBY_OPTIONS )
	end
	if GUI.ChangeTable.invite == true then
		PushInfoLine( "LOBBY_INVITE_BUTTON" )
	end

	if Amax.CanShareInLobby() == true and Amax.CanUseShare() == true then
		PushInfoLine( UIText.INFO_LB_SHARE_PREVIOUS_RESULTS )
	end
end


function SetupPlayer( root_id, index )
	if index == 0 then
		UIButtons.ChangeColour( UIButtons.FindChildByName( root_id, "gamertag" ), "Support_4" )
		UIButtons.ChangeColour( UIButtons.FindChildByName( root_id, "frame" ), "Support_4" )
	end

	return root_id
end

function RefreshLobby()

	-- setup team view
	if NetRace.IsTeamGame() == true and Multiplayer.TeamDataValid( GUI.Lobby ) == true then
		local half_players = GUI.MaxPlayers / 2
		local player_team = NetRace.GetLocalPlayerTeam()
		
		local left_colour = Select( player_team == 0, "team_a", "team_b" )
		local right_colour = Select( player_team == 0, "team_b", "team_a" )
		
		UIButtons.SetActive( SCUI.name_to_id["left_team_icon"], true, true )
		UIButtons.SetActive( SCUI.name_to_id["right_team_icon"], true, true )
		
		UIShape.ChangeObjectName( SCUI.name_to_id["left_team_icon"], Select( player_team == 0, "team_a", "team_b" ) )
		UIShape.ChangeObjectName( SCUI.name_to_id["right_team_icon"], Select( player_team == 0, "team_b", "team_a" ) )
		
		UIButtons.ChangeText( SCUI.name_to_id["left_team_name"], Select( player_team == 0, UIText.MP_TEAM_A_NAME, UIText.MP_TEAM_B_NAME ) )
		UIButtons.ChangeText( SCUI.name_to_id["right_team_name"], Select( player_team == 0, UIText.MP_TEAM_B_NAME, UIText.MP_TEAM_A_NAME ) )
		 
		UIButtons.ChangeColour( SCUI.name_to_id["left_team_icon"], UIGlobals.TeamColours[left_colour] )
		UIButtons.ChangeColour( SCUI.name_to_id["left_team_name"], UIGlobals.TeamColours[left_colour] )
		UIButtons.ChangeColour( SCUI.name_to_id["right_team_icon"], UIGlobals.TeamColours[right_colour] )
		UIButtons.ChangeColour( SCUI.name_to_id["right_team_name"], UIGlobals.TeamColours[right_colour] )
		
		-- build team a
		for i = 2, half_players do
			UIButtons.ChangeColour( UIButtons.FindChildByName( GUI.Players[i], "gamertag" ), UIGlobals.TeamColours[left_colour] )
			UIButtons.ChangeColour( UIButtons.FindChildByName( GUI.Players[i], "frame" ), UIGlobals.TeamColours[left_colour] )
		end
		
		-- build team b
		for i = half_players+1, GUI.MaxPlayers do
			UIButtons.ChangeColour( UIButtons.FindChildByName( GUI.Players[i], "gamertag" ), UIGlobals.TeamColours[right_colour] )
			UIButtons.ChangeColour( UIButtons.FindChildByName( GUI.Players[i], "frame" ), UIGlobals.TeamColours[right_colour] )
		end
	end

	GUI.Slots			= Multiplayer.GetLobbySlots( GUI.Lobby )
	GUI.GamerPictures	= Profile.GetRemoteGamerPictureMap()
		
	for i, v in ipairs( GUI.Slots ) do
	
		local node_id = GUI.Players[i]
	
		local empty_id		= UIButtons.FindChildByName( node_id, "empty" )
		local gamerpic_id	= UIButtons.FindChildByName( node_id, "gamer_picture" )
		local info_id		= UIButtons.FindChildByName( node_id, "info" )
		local rank_id		= UIButtons.FindChildByName( node_id, "rank" )
		local rank_icon_id	= UIButtons.FindChildByName( node_id, "rank_icon" )
		local party_id		= UIButtons.FindChildByName( node_id, "party" )

		local name_id		= UIButtons.FindChildByName( node_id, "gamertag" )
		local frame_id		= UIButtons.FindChildByName( node_id, "frame" )

		-- turn on or off other gadgets
		UIButtons.SetActive( info_id, not v.empty, true )
		UIButtons.SetActive( rank_icon_id, not v.empty, true, false )
		UIButtons.SetActive( empty_id, v.empty )
		UIButtons.SetActive( party_id, v.party_member )
		
		UIButtons.ChangeText( name_id, "LOBBY_PLAYER_NAME" .. v.ref )
		UIButtons.ChangeText( rank_id, "LOBBY_PLAYER_RANK" .. v.ref )
		UIButtons.ChangeText( info_id, "LOBBY_PLAYER_INFO" .. v.ref )

		-- show party icons
		if v.party_member == true then
			if v.party_host == true then
				UIButtons.ChangeColour( party_id, "Main_0" )
			else
				UIButtons.ChangeColour( party_id, "Support_1" )
			end			
		end

		-- turn effects on/off
		if v.racing == true then
			UIButtons.PrivateTimeLineActive( node_id, "player_active", false )
			UIButtons.PrivateTimeLineActive( node_id, "player_racing", true )
		else
			UIButtons.PrivateTimeLineActive( node_id, "player_active" , not v.empty )
			UIButtons.PrivateTimeLineActive( node_id, "player_racing" , false )
		end

		-- need to refresh the string length
		UIButtons.GetStaticTextLength( rank_id, true )

		-- active player
		if v.empty == false then
			if i == 1 then
				LocalGamerPicture( gamerpic_id, Profile.GetPrimaryPad() )
			else
				RemoteGamerPicture( gamerpic_id, GUI.GamerPictures[v.ref] )
			end

			-- set rank icon
			Mp_RankIcon( rank_icon_id, v.rank, v.legend )
		else
			RemoteGamerPicture( gamerpic_id, nil ) -- default
		end

		-- lock or unlock the lists
		if i <= 10 then
			UIButtons.SetNodeItemLocked( GUI.PlayersLeft, i - 1, v.empty )
		else
			UIButtons.SetNodeItemLocked( GUI.PlayersRight, i - 11, v.empty )
		end
	end

	-- can always highlight the left hand side
	if GUI.CurrentList ~= GUI.PlayersLeft and CanMoveRight() == false then
		UIButtons.SetSelected( GUI.PlayersLeft, true )
		UIButtons.SetSelected( GUI.PlayersRight, false )
	
		GUI.CurrentList = GUI.PlayersLeft
	end
end


function UpdateVoiceInfo()

	local voice_info = NetServices.GetVoiceMap( false )

	for i, v in ipairs( GUI.Slots ) do

		local voice_id	= UIButtons.FindChildByName( GUI.Players[i], "voice" )

		-- turn off by default
		UIButtons.SetActive( voice_id, false )

		if v.empty == false and voice_info[v.ref] ~= nil then

			-- turn icon on for speaking players
			if voice_info[v.ref].speaking == true then
				UIButtons.SetActive( voice_id, true )
				UIButtons.ChangeTextureUV( voice_id, 0, 0.5, 0 )
				
				--UIButtons.ChangeColour( voice_id, "Main_White" )				
				UIButtons.ChangeAlpha( voice_id, 1.0 )

			-- show mute icon
			elseif voice_info[v.ref].muted == true then
				UIButtons.SetActive( voice_id, true )
				UIButtons.ChangeTextureUV( voice_id, 0, 0, 0 )
				
				--UIButtons.ChangeColour( voice_id, "Main_White" )
				UIButtons.ChangeAlpha( voice_id, 1.0 )

			-- headset is plugged in
			elseif voice_info[v.ref].active == true then
				UIButtons.SetActive( voice_id, true )
				UIButtons.ChangeTextureUV( voice_id, 0, 0.5, 0 )
				
				--UIButtons.ChangeColour( voice_id, "Locked" )
				UIButtons.ChangeAlpha( voice_id, 0.45 )
			end
		end
	end
end


function UpdateLoadout()
	local loadout = Multiplayer.GetCurrentLoadout()
	
	local modshop_unlocked = Amax.IsGameModeRanked() == false or Multiplayer.GetFeatureUnlocked(UIEnums.MPFeatures.ModShop);
	UIButtons.SetActive( SCUI.name_to_id["loadout_not_unlocked"], not modshop_unlocked ) 
	
	if Multiplayer.CanChooseMods() == false or modshop_unlocked == false then
		UIButtons.SetActive( SCUI.name_to_id["loadout_not_available"], true ) 
		UIButtons.ChangeText( SCUI.name_to_id["loadout_not_available"], UIText.MP_NO_CAR_SELECTED )
		UIButtons.ChangeText( SCUI.name_to_id["loadout_name"], UIText.MP_MOD_SHOP )
		UIButtons.SetActive( SCUI.name_to_id["loadout_name"], true )
		UIButtons.SetActive( SCUI.name_to_id["loadout_panel"], true )
		UIButtons.SetActive( SCUI.name_to_id["mod_2"], false, true )
	else
		UIButtons.SetActive( SCUI.name_to_id["loadout_not_available"], false ) 
	
		if modshop_unlocked == true then
			UIButtons.SetActive( SCUI.name_to_id["mod_2"], true, true )
			if UIGlobals.Multiplayer.InLobby == true then
				UIButtons.ChangeText( SCUI.name_to_id["loadout_name"], "MPL_LOADOUT_NAME_LOBBY" .. loadout.type )
			else
				UIButtons.ChangeText( SCUI.name_to_id["loadout_name"], "MPL_LOADOUT_NAME" .. loadout.type )
			end

			-- each slot
			for i, v in ipairs( loadout.slots ) do
				local id = SCUI.name_to_id["mod_" .. i ]

				if v.empty == true then
					UIShape.ChangeSceneName( id, "common_icons" )
					UIShape.ChangeObjectName( id, "style_mixed" )
					UIButtons.ChangeColour( id, "Dark_White" )
				else
					UIShape.ChangeSceneName( id, "ability_icons" )
					UIShape.ChangeObjectName( id, v.icon_name )
					UIButtons.ChangeColour( id, UIGlobals.CategoryColour[v.pack] )
				end
			end
		else
			local feature_table = Multiplayer.GetFeatureUnlockedMap()
			UIButtons.ChangeText( SCUI.name_to_id["loadout_not_unlocked"], "MPL_UNLOCK_AT_RANK" .. feature_table[UIEnums.MPFeatures.ModShop] )
			UIButtons.ChangeText( SCUI.name_to_id["loadout_name"], UIText.MP_MOD_SHOP )

			UIButtons.SetActive( SCUI.name_to_id["loadout_name"], true )
			UIButtons.SetActive( SCUI.name_to_id["loadout_panel"], true )

			UIButtons.SetActive( SCUI.name_to_id["mod_2"], false, true )
		end
		
	end
end

function UpdateCar( hide )
	-- need a safe default
	if hide == nil then
		hide = false
	end

	local colour_id = SCUI.name_to_id[ "car_colour" ]
	local car_id = SCUI.name_to_id[ "car_icon" ]
	local car_man_id = SCUI.name_to_id[ "car_icon_man" ]
	local car_name_id = SCUI.name_to_id[ "car_name" ]
	local car_class_id = SCUI.name_to_id[ "car_class_icon" ]

	local car_siloette_id = SCUI.name_to_id[ "car_siloette" ]
	local car_unavailable_id = SCUI.name_to_id[ "car_unavailable" ]

	UIButtons.SetActive( car_class_id, not hide )
	UIButtons.SetActive( car_name_id, not hide )

	-- show question mark icon and unavailible
	UIButtons.SetActive( car_siloette_id, hide )
	UIButtons.SetActive( car_unavailable_id, hide )

	if hide == true then
		UIButtons.SetActive( car_id, false, true )
		UIButtons.SetActive( car_man_id, false )
	else 
		-- this wont run when it gets merged back into the release branch
		UIShape.ChangeObjectName( car_class_id, UIGlobals.ClassIcons[NetRace.GetCarClass()] ) -- TODO_MP_REFACTOR_FIX
		
		local car = Multiplayer.GetLobbyPlayerCar()

		if car == nil then
			return
		end
		
		UIButtons.SetActive( car_siloette_id, false )

		if car.colour_man == true then
			UIButtons.SetActive( car_id, false, true )
			UIButtons.SetActive( car_man_id, true )

			UIShape.ChangeSceneName( car_man_id, car.icon_sheet )
			UIShape.ChangeObjectName( car_man_id, car.icon_name )
		else
			UIButtons.SetActive( car_id, true, true )
			UIButtons.SetActive( car_man_id, false )
			UIButtons.ChangeColour( colour_id, "!" .. car.colour )

			UIShape.ChangeSceneName( car_id, car.icon_sheet )
			UIShape.ChangeObjectName( car_id, car.icon_name )
		end
	end
end

function MpLobby_DisablePopupActive( state )
	local _GUI = ContextTable[ UIEnums.Context.CarouselApp ].GUI
	if IsTable( _GUI ) == false then
		return
	end
	if IsTable( _GUI.Players ) == false then
		return
	end
	
	if state == false then
		for i, v in ipairs( _GUI.Players ) do
			local id = UIButtons.FindChildByName( v, "gamer_picture" )
			UIButtons.SetXtVar( id, "time_lines.0.label", "Popup_Active" )
			if UIScreen.IsPopupActive() == false then
				UIButtons.PrivateTimeLineActive( id, "Popup_Active", state )
			end
		end
	else
		for i, v in ipairs( _GUI.Players ) do
			local id = UIButtons.FindChildByName( v, "gamer_picture" )
			UIButtons.PrivateTimeLineActive( id, "Popup_Active", state )
			UIButtons.SetXtVar( id, "time_lines.0.label", "hacksaw" )
		end
	end
end

function StartVoting()
	MpLobby_DisablePopupActive( true )
	
	print( "Start Voting" )

	if NetRace.IsHost() == true then
		Multiplayer.StartRaceVoting()
	end

	UIGlobals.Multiplayer.LobbyState = UIEnums.MpLobbyState.Voting
	UpdateLobbyInfo()

	UIButtons.SetActive( GUI.RaceOverviewID, false )
	UIButtons.SetActive( SCUI.name_to_id["summary_branch"], false )
	UIButtons.SetActive( SCUI.name_to_id["vehicle_branch"], false )
	UIButtons.SetActive( SCUI.name_to_id["info_branch"], false )
	UIButtons.SetActive( SCUI.name_to_id[ "loadout_branch" ], false )

	PushScreen( "Multiplayer\\Shared\\MpVoting.lua" )
end


function EndVoting()
	MpLobby_DisablePopupActive( false )
	print( "End Voting" )

	UIGlobals.Multiplayer.LobbyUpdateCar = true -- force car to update
	UIGlobals.Multiplayer.LobbyState = Select( Amax.IsGameModeRanked(), UIEnums.MpLobbyState.Countdown, UIEnums.MpLobbyState.WaitingForHost )
	UpdateLobbyInfo()

	GUI.RaceIndex = Multiplayer.GetCurrentRaceIndex()
	MpLobby_RefreshCurrentRace()

	UIButtons.SetActive( GUI.RaceOverviewID, true )
	UIButtons.SetActive( SCUI.name_to_id["summary_branch"], true )
	UIButtons.SetActive( SCUI.name_to_id["vehicle_branch"], true )
	UIButtons.SetActive( SCUI.name_to_id["info_branch"], true )
	UIButtons.SetActive( SCUI.name_to_id[ "loadout_branch" ], true )
	
	GetSetLobbyCar()	
	UpdateLoadout()
		
	PopScreen()
end

function GetSetLobbyCar()
	local car = Multiplayer.GetLobbyPlayerCar()
	
	if car == nil then
		return
	end
	
	Multiplayer.SetGarageCarByID( car.id )
	Amax.LoadCar( car.id, true, NetRace.IsTeamGame(), NetRace.GetLocalPlayerTeam() )
	GUI.viewing_id = car.id
end

function CanMoveLeft()
	local half_players = GUI.MaxPlayers / 2
	
	for i = 1, half_players do
		if GUI.Slots[i].empty == false then
			return true
		end
	end
	return false
end

function CanMoveRight()
	local half_players = GUI.MaxPlayers / 2
	
	for i = half_players+1, GUI.MaxPlayers do
		if GUI.Slots[i].empty == false then
			return true
		end
	end
	return false
end

function MpLobby_ShowHideMods( show )
	UIButtons.SetActive( SCUI.name_to_id[ "loadout_branch" ], show )
end

function MpLobby_CanViewMenu()
	if UIGlobals.Multiplayer.LobbyState == UIEnums.MpLobbyState.FindingPlayers or 
	   UIGlobals.Multiplayer.LobbyState == UIEnums.MpLobbyState.Waiting or 
	   UIGlobals.Multiplayer.LobbyState == UIEnums.MpLobbyState.Countdown or 
	   UIGlobals.Multiplayer.LobbyState == UIEnums.MpLobbyState.TeamBalanceWait or 
	   UIGlobals.Multiplayer.LobbyState == UIEnums.MpLobbyState.WaitingForHost then
		return true
	else
		return false
	end
end

function MpLobby_CanCountdown()
	if UIGlobals.Multiplayer.LobbyState == UIEnums.MpLobbyState.WaitingToVote or 
	   UIGlobals.Multiplayer.LobbyState == UIEnums.MpLobbyState.Countdown or
	   UIGlobals.Multiplayer.LobbyState == UIEnums.MpLobbyState.Voting then
		return true
	else
		return false
	end
end

function MpLobby_PlayersValid()
	local result = false
	if NetRace.IsTeamGame() == true then
		result = ( NetRace.EnoughPlayersFound() == true and NetRace.AreTeamsValid() == true and NetRace.AreTeamsBalanced() == true )
	else
		result = NetRace.EnoughPlayersFound()
	end
	return result
end

function MpLobby_HostCanPressStart()
	-- only the host can call this
	if NetRace.IsHost() == false then
		return false
	end

	if UIGlobals.Multiplayer.LobbyState == UIEnums.MpLobbyState.WaitingForHost and NetRace.CanStartCustomRace() == true and Amax.IsGameModeRanked() == false then
		return true
	else
		return false
	end
end

function MpLobby_ValidateState()
	if UIGlobals.Multiplayer.LobbyState ~= UIEnums.MpLobbyState.Countdown and UIGlobals.Multiplayer.LobbyState ~= UIEnums.MpLobbyState.Ready then
		return
	end
	
	-- if somebody isnt in the lobby and they arnt connecting, then the race is in progress, and we are in the wrong state.
	if NetRace.RaceInProgress() == true then
		NetRace.ResetLobbyState()
		print( "Lobby: Resetting lobby state as its got confused" )
	end
end

function MpLobby_ShowNew( show )
	if Amax.GetGameMode() == UIEnums.GameMode.SystemLink then
		return
	end

	UIButtons.SetActive( SCUI.name_to_id[ "new_branch" ], show )

	-- reset
	if show == false then
		GUI.NewItemsAvailible = false
	end
end

function MpLobby_CanViewMenu()
	if UIGlobals.Multiplayer.LobbyState == UIEnums.MpLobbyState.WaitingToVote or 
	   UIGlobals.Multiplayer.LobbyState == UIEnums.MpLobbyState.Voting then
		return false
	else
		return true
	end
end

function MpLobby_RefreshCurrentRace()
	local race = Multiplayer.GetCurrentRace()
	local race_index = Multiplayer.GetCurrentRaceIndex()

	local prefix = Select( (Amax.GetPlayMode() == UIEnums.PlayMode.Playlist), "MPL_PLAYLIST_RACE" .. race_index, "MPL_CUSTOM_RACE" )

	-- change strings
	UIButtons.ChangeText( UIButtons.FindChildByName( GUI.RaceOverviewID, "city" ),	prefix .. "_CITY" )
	UIButtons.ChangeText( UIButtons.FindChildByName( GUI.RaceOverviewID, "route" ), prefix .. "_ROUTE" )
	UIButtons.ChangeText( UIButtons.FindChildByName( GUI.RaceOverviewID, "mode" ),	prefix .. "_TYPE" )
	UIButtons.ChangeText( UIButtons.FindChildByName( GUI.RaceOverviewID, "class" ), prefix .. "_CLASS" )
	UIButtons.ChangeText( UIButtons.FindChildByName( GUI.RaceOverviewID, "end" ),	prefix .. Select( race.type == UIEnums.MpRaceType.Destruction, "_TIME", "_NUM_LAPS" ) )

	-- change icons
	UIShape.ChangeObjectName( UIButtons.FindChildByName( GUI.RaceOverviewID, "city_icon" ), Amax.GetCityIconName( race.city ) )
	Mp_RaceTypeIcon( UIButtons.FindChildByName( GUI.RaceOverviewID, "mode_icon" ), race )
end

