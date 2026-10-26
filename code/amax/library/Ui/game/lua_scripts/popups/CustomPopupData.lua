PopupGUI = {}	-- [WillB, Peter R] # - fix to stop PopupGUI getting garbarged collected at the end of the loading screen.

PopupInit			= function() print( "PopupInit - Blank" ) end
PopupPostInit		= function() print( "PopupPostInit - Blank" ) end
PopupStartLoop		= function() print( "PopupStartLoop - Blank" ) end
PopupMessageUpdate	= function() print( "PopupMessageUpdate - Blank" ) end
PopupFrameUpdate	= function() print( "PopupFrameUpdate - Blank" ) end
PopupRender			= function() print( "PopupRender - Blank" ) end
PopupEnterEnd		= function() print( "PopupEnterEnd - Blank" ) end
PopupEndLoop		= function() print( "PopupEndLoop - Blank" ) end
PopupEnd			= function() print( "PopupEnd - Blank" ) end

CustomPopup = 
{
	-- Should point at one of below tables
	CurrentData = nil,
	CurrentType = -1,
	Callback	= nil,
	
	-- Each popups data
	Data =
	{
		[UIEnums.CustomPopups.Default] =	-- Don't alter this - it's just to show what the options are
		{
			-- Optional stuff
			darken				= nil,			-- Darken the screen (nil == true)
			colour_style		= nil,			-- Overide frame colour
			icon_name			= nil,			-- Overide icon name
			pos					= nil,			-- { x, y }
			size				= nil,			-- { x, y }
			no_back				= nil,			-- Set to true to disable the back button
			no_next				= nil,			-- Set to true to disable the next button
			show_progress		= nil,			-- Display progress symbol
			
			-- Text ID's
			title_text_ID		= nil,			-- Message header
			message_text_ID		= nil,			-- Message info
			message_text_y		= nil,			-- override for position
			
			next_text_ID		= nil,			-- Next message IDS_
			back_text_ID		= nil,			-- Back message IDS_
			x_text_ID			= nil,			-- X message IDS_
			y_text_ID			= nil,			-- Y message IDS_
			
			timeout				= nil,			-- number of seconds after which to call the timeout_CB
			
			-- Strings for when CancelPopup is called (don't forget nil is a valid option)
			next_screen			= nil,			-- String containing next screen to go to
			prev_screen			= nil,			-- String containing prev screen to go to
			go_subscreen		= nil,			-- call GoSubScreen
			go_launch_screen	= nil,			-- call LaunchScreen
			
			-- Callbacks
			init_CB				= nil,			-- Called at the start of PopupInit()
			update_CB			= nil,			-- Called at the start of PopupFrameUpdate()
			message_CB			= nil,			-- Called at the start of PopupMessageUpdate() (if this returns true then PopupMessageUpdate bails)
			next_CB				= nil,			-- Called when the "BUTTON_NEXT" is accepted and we cancel the popup
			back_CB				= nil,			-- Called when the "BUTTON_BACK" is accepted and we cancel the popup
			timeout_CB			= nil,			-- Called when we timeout - based on timeout above
			end_CB				= nil,			-- called when we end

			do_end				= nil,			-- Set to true to disable the end screen
			
			busy				= nil,			-- Set to true to show *busy* icon
			no_effects			= nil,			-- Set to true if you don't want to slide on / slide off effect

			-- menu items
			options				= nil,			-- menu item list. Format is { value = 0, name = UIText.CMN_NOWT }
			default_option		= nil,			-- set the initial state of the above options list
			
			save_icon			= nil,			-- only use this once on the auto save warning popup
			
			silent				= nil,			-- don't show graphics etc
		},
		
		[UIEnums.CustomPopups.ExitApplication] =
		{
			darken			= true,

			title_text_ID	= UIText.POP_TITLE_WARNING,
			message_text_ID	= UIText.POP_QUIT_MSG,
			
			next_text_ID	= UIText.INFO_A_SELECT,
			back_text_ID	= UIText.INFO_B_BACK,

			options = 
			{
				{ value = UIEnums.PopupOptions.Yes,				name = UIText.POP_OPTION_YES },
				{ value = UIEnums.PopupOptions.No,				name = UIText.POP_OPTION_NO },
			},
			
			default_option	= UIEnums.PopupOptions.Yes,

			next_CB			= function()
			
								local option = UIButtons.GetSelection( SCUI_Popup.name_to_id[ "options" ] )

								if option == UIEnums.PopupOptions.Yes then
									UIScreen.SetScreenTimers( 0, 0.6, 0 )
									
									UIButtons.SetSelection( ContextTable[ UIEnums.Context.Main ].GUI.carousel_id, UIEnums.RwBranch.Quit )

									-- Run the off animation for the current application
									UIButtons.TimeLineActive( "start", false )
									--GameProfile.DebugSaveGame()
									PopupCancel( "Intro\\StartScreen.lua" )
									--Amax.SendMessage(UIEnums.GameFlowMessage.DumpUI)
									
									if UIGlobals.ReturnMode == UIEnums.ReturnMode.None then
										UIGlobals.ReturnMode = UIEnums.ReturnMode.QuitSpGame
									end
								end
							end,
		},
		
		[UIEnums.CustomPopups.ExitMultiplayer] =
		{
			darken			= true,
			colour_style	= "Main_Black",
			intensity		= 0.6,
			icon_name		= "groups",
			
			title_text_ID	= UIText.POP_TITLE_WARNING,
			message_text_ID	= UIText.POP_QUIT_MSG,
			
			next_text_ID	= UIText.INFO_A_SELECT,
			back_text_ID	= UIText.INFO_B_BACK,
			
			default_option	= UIEnums.PopupOptions.Yes,

			options = 
			{
				{ value = UIEnums.PopupOptions.Yes,				name = UIText.POP_OPTION_YES },
				{ value = UIEnums.PopupOptions.No,				name = UIText.POP_OPTION_NO },
			},

			next_CB			= function()
								local option = UIButtons.GetSelection( SCUI_Popup.name_to_id[ "options" ] )

								if option == UIEnums.PopupOptions.Yes then
									UIScreen.SetScreenTimers( 0, 0.6, 0 )
								
									net_MpLeave( )
									net_FlushEverything( )
									PopupCancel( "Intro\\StartScreen.lua" )
									
									UIButtons.SetSelection( ContextTable[ UIEnums.Context.Main ].GUI.carousel_id, UIEnums.RwMpBranch.Quit )

									--GameProfile.DebugSaveGame()
									--Amax.SendMessage(UIEnums.GameFlowMessage.DumpUI)
								
									if UIGlobals.ReturnMode == UIEnums.ReturnMode.None then
										UIGlobals.ReturnMode = UIEnums.ReturnMode.QuitMpGame
									end
								end
							end,
		},
		
		[UIEnums.CustomPopups.NoLanGames] =
		{
			darken			= true,
			colour_style	= "Main_Black",
			intensity		= 0.6,
			icon_name		= "groups",
			no_back			= true,
			title_text_ID	= UIText.MP_SEARCH_FINISHED,
			message_text_ID	= UIText.MP_NO_LAN_GAMES_FOUND,
			next_text_ID	= UIText.INFO_A_OK,
			
			init_CB			= function()
								NetRace.EnumerateLanGames( false )
							  end,
		},
		
		[UIEnums.CustomPopups.ExitRaceLobby] =
		{
			darken			= true,
			colour_style	= "Main_Black",
			intensity		= 0.6,
			icon_name		= "groups",

			title_text_ID	= UIText.POP_TITLE_WARNING,
			message_text_ID	= UIText.POP_LEAVE_LOBBY,
			
			next_text_ID	= UIText.INFO_A_SELECT,
			back_text_ID	= UIText.INFO_B_BACK,

			init_CB		=	function()
								if NetParty.IsRunning() == true and NetParty.IsHost() == true then
									CustomPopup.CurrentData.options = 
									{
										{ value = UIEnums.PopupOptions.Yes,				name = UIText.POP_OPTION_YES },
										{ value = UIEnums.PopupOptions.QuitWithParty,	name = UIText.POP_OPTION_QUIT_WITH_PARTY },
										{ value = UIEnums.PopupOptions.No,				name = UIText.POP_OPTION_NO },
									}
								else
									CustomPopup.CurrentData.options = 
									{
										{ value = UIEnums.PopupOptions.Yes,		name = UIText.POP_OPTION_YES },
										{ value = UIEnums.PopupOptions.No,		name = UIText.POP_OPTION_NO },
									}
								end
							end,

			next_CB		=	function()
								local option = UIButtons.GetSelection( SCUI_Popup.name_to_id[ "options" ] )

								if option == UIEnums.PopupOptions.QuitWithParty then
									UIGlobals.Multiplayer.DisconnectRaceAsPartyHost = true

									net_DisconnectFromRace()

								elseif option == UIEnums.PopupOptions.Yes then
									UIGlobals.Multiplayer.DisconnectRaceAsPartyHost = false

									net_DisconnectFromRace()
								end
							end,
		},

		[UIEnums.CustomPopups.ExitRace] =
		{
			darken			= true,
			
			title_text_ID	= UIText.POP_TITLE_WARNING,
			message_text_ID	= UIText.POP_LEAVE_RACE,
			
			next_text_ID	= UIText.INFO_A_SELECT,
			back_text_ID	= UIText.INFO_B_BACK,

			init_CB			= function()
								if Amax.IsGameModeNetworked() then
									CustomPopup.CurrentData.colour_style	= "Main_Black"
									CustomPopup.CurrentData.intensity		= 0.6
									CustomPopup.CurrentData.icon_name		= "groups"
								else
									CustomPopup.CurrentData.colour_style	= "Main_2"
									CustomPopup.CurrentData.icon_name		= "bio"
								end

								if NetParty.IsRunning() == true and NetParty.IsHost() == true then
									CustomPopup.CurrentData.options = 
									{
										{ value = UIEnums.PopupOptions.Yes,				name = UIText.POP_OPTION_YES },
										{ value = UIEnums.PopupOptions.QuitWithParty,	name = UIText.POP_OPTION_QUIT_WITH_PARTY },
										{ value = UIEnums.PopupOptions.No,				name = UIText.POP_OPTION_NO },
									}
								else
									CustomPopup.CurrentData.options = 
									{
										{ value = UIEnums.PopupOptions.Yes,		name = UIText.POP_OPTION_YES },
										{ value = UIEnums.PopupOptions.No,		name = UIText.POP_OPTION_NO },
									}
								end
							  end,
			
			next_CB			= function()
								local option = UIButtons.GetSelection( SCUI_Popup.name_to_id[ "options" ] )

								-- dont do anything
								if option == UIEnums.PopupOptions.No then
									return
								end

								if option == UIEnums.PopupOptions.QuitWithParty then
									UIGlobals.Multiplayer.DisconnectRaceAsPartyHost = true

								elseif option == UIEnums.PopupOptions.Yes then
									UIGlobals.Multiplayer.DisconnectRaceAsPartyHost = false
								end

								if Amax.IsGameModeNetworked() then
									Multiplayer.PostFlaggedDataEvents()
								
									net_DisconnectFromRace()
								else
									-- PR - sort out splitscreen pad for save after a race
									if IsSplitScreen() == true then
										Profile.SetPrimaryPad( UIGlobals.Splitscreen.primary_pad )
										Profile.LockToPad( UIGlobals.Splitscreen.primary_pad )

										UIGlobals.Splitscreen.can_vote = false
									else
										StartAsyncSave()
									end

									local is_fd = ( Amax.SP_IsStreetRaceFD() == true or Amax.SP_IsDestructionFD() == true or Amax.SP_IsCheckpointFD() == true or Amax.SP_IsFanDemandFD() == true or Amax.SP_IsBossBattleFD() == true )
									if is_fd == true then
										FriendDemand.QuitAttempt()
									end

									Amax.SendMessage(UIEnums.GameFlowMessage.QuitRace)
									PopupCancel( "Loading\\LoadingUi.lua" )
								end
							  end,
		},
		
		[UIEnums.CustomPopups.ReturnToLobby] =
		{
			darken			= true,
			
			title_text_ID	= UIText.POP_TITLE_WARNING,
			message_text_ID	= UIText.POP_LEAVE_TO_LOBBY,
			
			next_text_ID	= UIText.INFO_A_SELECT,
			back_text_ID	= UIText.INFO_B_BACK,

			options = 
					{
						{ value = UIEnums.PopupOptions.Yes,				name = UIText.POP_OPTION_YES },
						{ value = UIEnums.PopupOptions.No,				name = UIText.POP_OPTION_NO },
					},
			
			next_CB			= function()
								local option = UIButtons.GetSelection( SCUI_Popup.name_to_id[ "options" ] )

								-- dont do anything
								if option == UIEnums.PopupOptions.No then
									return
								elseif option == UIEnums.PopupOptions.Yes then
									NetRace.ForceEventTimeout()
								end

							  end,
		},
		
		[UIEnums.CustomPopups.ExitPartyLobby] =
		{
			darken			= true,

			colour_style	= "Main_Black",
			intensity		= 0.6,
			icon_name		= "groups",
			
			title_text_ID	= UIText.POP_TITLE_WARNING,
			message_text_ID	= UIText.POP_EXIT_PARTY_LOBBY,
			
			next_text_ID	= UIText.INFO_A_SELECT,
			back_text_ID	= UIText.INFO_B_BACK,

			options = 
			{
				{ value = UIEnums.PopupOptions.Yes,				name = UIText.POP_OPTION_YES },
				{ value = UIEnums.PopupOptions.No,				name = UIText.POP_OPTION_NO },
			},
			
			next_CB			= function()
								local option = UIButtons.GetSelection( SCUI_Popup.name_to_id[ "options" ] )

								if option == UIEnums.PopupOptions.Yes then
									PopupSpawn( UIEnums.CustomPopups.MultiplayerDisconnectFromParty )
								end
							  end,
		},
			
		[UIEnums.CustomPopups.MultiplayerDisconnectFromRace] =
		{
			darken			= true,
			colour_style	= "Main_Black",
			intensity		= 0.6,
			icon_name		= "groups",

			no_back			= true,
			no_next			= true,
			show_progress	= true,
			
			title_text_ID	= UIText.POP_TITLE_PLEASE_WAIT,
			message_text_ID	= UIText.MP_DISCONNECTING,
			
			init_CB			= function()
								PopupGUI.timer = 0
								PopupGUI.state = 0
								
								if NetParty.IsHost() == true and NetParty.IsActive() == true and UIGlobals.Multiplayer.DisconnectRaceAsPartyHost == true then
									NetParty.SendReturnToPartyLobby( )	-- if party host send msg to party peers to also disconnect from the race.
								end

								UIGlobals.Multiplayer.LaunchScreen = UIEnums.MpLaunchScreen.None -- reset
								NetRace.EnterDisconnecting( )		-- local players go into disconnecting state.

								if Amax.IsGameModeRanked() == true then
									net_SetGroupPresence()
								end
							end,
							  
			update_CB		= function(time)	
								PopupGUI.timer = PopupGUI.timer + time
								local move_on = false
								
								if PopupGUI.state == 0 then 
									if NetRace.IsPlayerMigrating( ) == false or PopupGUI.timer >= 5 then
										if UIGlobals.Multiplayer.DisconnectRaceAsPartyHost == false then
											if PopupGUI.timer >= 1.5 then
												PopupGUI.state = 1	
												PopupGUI.timer = 0
											end
										else
											if PopupGUI.timer >= 5 then
												if NetParty.AllPeersIdle( ) == true or PopupGUI.timer >= 10 then
													PopupGUI.state = 1
													PopupGUI.timer = 0
												end
											end
										end
									end
								else
									move_on = true
								end
								
								if move_on == true then
								
									if UIGlobals.IsIngame == true then
									
										-- party lobby
										if UIGlobals.Multiplayer.DisconnectRaceAsPartyHost == true then
											UIGlobals.Multiplayer.LaunchScreen = UIEnums.MpLaunchScreen.PartyLobby
										else
											net_FlushSessionEnumerator()
											net_CloseAllSessions()
											
											if Amax.GetGameMode() == UIEnums.GameMode.Online then
												UIGlobals.Multiplayer.LaunchScreen = UIEnums.MpLaunchScreen.MultiplayerRoot
											elseif Amax.GetGameMode() == UIEnums.GameMode.SystemLink then
												UIGlobals.Multiplayer.LaunchScreen = UIEnums.MpLaunchScreen.LanBrowser
											end
										end
										
										Amax.SendMessage(UIEnums.GameFlowMessage.QuitRace)
										
										PopupCancel( "Loading\\LoadingUi.lua" )
										
										if Amax.IsGameModeRanked() == true then
											-- Force a save as we go back to the UI	
											print( "LoadingUI Init() > Saving..." )
											StartAsyncSave( )											
										end
									
									elseif UIGlobals.IsIngame == false then	

										-- party lobby
										if UIGlobals.Multiplayer.DisconnectRaceAsPartyHost == false then
											net_FlushSessionEnumerator()
											net_CloseAllSessions()
										end
										
										if Amax.GetGameMode() == UIEnums.GameMode.SystemLink then
											PopupCancel( "Multiplayer\\MpLan.lua", true )
										else
											PopupCancel( "Multiplayer\\MpOnline.lua", true )
										end
									end
								end
							  end,
							  
			end_CB	=		function()
								UIGlobals.Multiplayer.Disconnecting = false
							end,
							 
							 
		},	
		
		[UIEnums.CustomPopups.MultiplayerDisconnectFromParty] =
		{
			darken			= true,
			no_back			= true,
			no_next			= true,
			show_progress	= true,

			colour_style	= "Main_Black",
			intensity		= 0.6,
			icon_name		= "groups",
			
			title_text_ID	= UIText.POP_TITLE_PLEASE_WAIT,
			message_text_ID	= UIText.MP_DISCONNECTING,
			
			init_CB			= function()
								PopupGUI.timer = 0
								NetParty.EnterDisconnecting( ) -- local players go into disconnecting state.
							  end,
							  
			update_CB		= function(time)	
								PopupGUI.timer = PopupGUI.timer + time
								if PopupGUI.timer > 3.5 then
								
									local _GUI = ContextTable[ UIEnums.Context.Main ].GUI
									if IsTable( _GUI ) == true then
										PopupCancel()										
										net_ExitPartyLobby()
									end
								end
							  end,
		},	
		
		[UIEnums.CustomPopups.MultiplayerJoinLanGame] =
		{
			darken			= true,
			no_back			= true,
			no_next			= true,
			colour_style	= "Main_Black",
			intensity		= 0.6,
			icon_name		= "groups",
			show_progress	= true,
			
			title_text_ID	= UIText.POP_TITLE_PLEASE_WAIT,
			message_text_ID	= UIText.POP_ATTEMPTING_TO_JOIN_GAME,
			
			init_CB			=	function()
									PopupGUI.state = 0
									PopupGUI.timer = 0
								end,
								
			update_CB			=	function(time)
			
									if PopupGUI.state == 0 then
										PopupGUI.timer = PopupGUI.timer + time
										if PopupGUI.timer >= 1.5 then
											PopupGUI.state = 1
											
											UIGlobals.Network.JoinErrorCode = UIEnums.Network.JoinErrors.eJoinErrorInvalid

											local started, custom_race, event_id = NetRace.JoinLanServer( UIGlobals.Network.LanEnumeratedGameIndexToJoin )
											if started == true then

												if custom_race == true then
													net_MpEnterPlayMode( UIEnums.PlayMode.CustomRace )
												else
													-- check we can play
													if Multiplayer.FindPlaylistFromID( event_id ) == true then
														net_MpEnterPlayMode( UIEnums.PlayMode.Playlist )
													else
														print( "LAN : Couldnt find playlist" )
														PopupSpawn( UIEnums.CustomPopups.MultiplayerJoinLanGameFailed )
													end
												end
											else
												PopupSpawn( UIEnums.CustomPopups.MultiplayerJoinLanGameFailed )
											end
										end
									else
										local complete = false
										local code = UIEnums.Network.JoinErrors.eJoinErrorInvalid
										complete, code = NetRace.MonitorJoinLanServer( )
										
										if complete == true then
											UIGlobals.Network.JoinErrorCode = code
										
											if code == UIEnums.Network.JoinErrors.eJoinErrorNone then
												PopupCancel( "Multiplayer\\MpLobby.lua", true )
											else
												PopupSpawn( UIEnums.CustomPopups.MultiplayerJoinLanGameFailed )
											end
										end
									end
								end,
		},	
		
		[UIEnums.CustomPopups.MultiplayerVersionError] =
		{
			darken			= true,
			no_back			= true,
			colour_style	= "Main_Black",
			intensity		= 0.6,
			icon_name		= "groups",

			title_text_ID	= UIText.POP_TITLE_ERROR,
			message_text_ID	= UIText.CMN_NOWT,
			next_text_ID	= UIText.INFO_A_OK,
						
			init_CB			=	function()
									-- switch on error codes.
									if UIGlobals.Network.JoinVersionErrorCode == UIEnums.Network.Version.eNetVersionOlder then
										CustomPopup.CurrentData.message_text_ID = UIText.MP_VERSION_OLDER
										
									elseif UIGlobals.Network.JoinVersionErrorCode == UIEnums.Network.Version.eNetVersionNewer then
										CustomPopup.CurrentData.message_text_ID = UIText.MP_VERSION_NEWER
									end
								end,

		},
		
		[UIEnums.CustomPopups.MultiplayerJoinLanGameFailed] =
		{
			darken			= true,
			no_back			= true,
			colour_style	= "Main_Black",
			intensity		= 0.6,
			icon_name		= "groups",
			
			title_text_ID	= UIText.POP_TITLE_ERROR,
			message_text_ID	= UIText.MP_JOIN_ERROR_DEFAULT,
			next_text_ID	= UIText.INFO_A_OK,
						
			init_CB			=	function()
									local errorString = UIText.MP_JOIN_ERROR_DEFAULT
									
									if UIGlobals.Network.JoinErrorCode == UIEnums.Network.JoinErrors.eJoinErrorNotEnoughOpenSlots then
										errorString = UIText.MP_JOIN_ERROR_NO_SLOTS
									elseif UIGlobals.Network.JoinErrorCode == UIEnums.Network.JoinErrors.eJoinErrorBannedFromSession then
										errorString = UIText.MP_JOIN_ERROR_BANNED
									elseif UIGlobals.Network.JoinErrorCode == UIEnums.Network.JoinErrors.eJoinErrorNoConnectionToHost then
										errorString = UIText.MP_JOIN_ERROR_HOST
									elseif UIGlobals.Network.JoinErrorCode == UIEnums.Network.JoinErrors.eJoinErrorNoCommunications then
										errorString = UIText.MP_JOIN_ERROR_PEERS
									elseif UIGlobals.Network.JoinErrorCode == UIEnums.Network.JoinErrors.eJoinErrorToManyTeams then
										errorString = UIText.MP_JOIN_ERROR_TEAM
									elseif UIGlobals.Network.JoinErrorCode == UIEnums.Network.JoinErrors.eJoinErrorToManyTeamMembers then
										errorString = UIText.MP_JOIN_ERROR_TEAM_MEMBERS
									elseif UIGlobals.Network.JoinErrorCode == UIEnums.Network.JoinErrors.eJoinErrorContentMismatch then
										errorString = UIText.MP_JOIN_ERROR_CONTENT
									elseif UIGlobals.Network.JoinErrorCode == UIEnums.Network.JoinErrors.eJoinErrorChampionshipStarted then
										errorString = UIText.MP_JOIN_ERROR_CHAMPIONSHIP_STARTED
									elseif UIGlobals.Network.JoinErrorCode == UIEnums.Network.JoinErrors.eJoinErrorRankedMatchStarted then
										errorString = UIText.MP_JOIN_ERROR_RANKED_STARTED
									elseif UIGlobals.Network.JoinErrorCode == UIEnums.Network.JoinErrors.eJoinErrorPartyClosed then
										errorString = UIText.MP_JOIN_ERROR_PARTY_CLOSED
									elseif UIGlobals.Network.JoinErrorCode == UIEnums.Network.JoinErrors.eJoinErrorPlaylistVerClient then
										errorString = UIText.MP_JOIN_ERROR_PLAYLIST_VERSION_CLIENT
									elseif UIGlobals.Network.JoinErrorCode == UIEnums.Network.JoinErrors.eJoinErrorPlaylistVerHost then
										errorString = UIText.MP_JOIN_ERROR_PLAYLIST_VERSION_HOST
									end
									-- Change message text
									CustomPopup.CurrentData.message_text_ID = errorString
									
									NetRace.Delete( )
								end,
		},
		[UIEnums.CustomPopups.MultiplayerTooManyProfiles] =
		{
			darken			= true,
			no_back			= true,
			colour_style	= "Main_Black",
			intensity		= 0.6,
			icon_name		= "groups",
			
			title_text_ID	= UIText.POP_TITLE_ALERT,
			message_text_ID	= UIText.MP_TOO_MANY_PLAYERS,
			next_text_ID	= UIText.INFO_A_OK,
			
			next_CB			=	function()
									if UIGlobals.Network.GameInviteState == UIEnums.MpGameInviteState.Processing then
										net_GameInviteFailed( )
										PopupCancel()
										EnableCarousel( true )
									end
								end,
		},
		
		[UIEnums.CustomPopups.MultiplayerInvalidPrivileges] =
		{
			darken			= true,
			no_back			= true,
			
			title_text_ID	= UIText.POP_TITLE_ALERT,
			message_text_ID	= UIText.MP_INVALID_PRIVILEGES,
			next_text_ID	= UIText.INFO_A_OK,
			
			next_CB			=	function()
									if UIGlobals.Network.GameInviteState == UIEnums.MpGameInviteState.Processing then
										net_GameInviteFailed( )
										PopupCancel()
										EnableCarousel( true )
									end
								end,
		},
		
		[UIEnums.CustomPopups.MultiplayerOnlineAccountRequired] =
		{
			darken			= true,
			no_back			= true,
			
			title_text_ID	= UIText.POP_TITLE_ALERT,
			message_text_ID	= UIText.MP_ONLINE_ACCOUNT_REQUIRED,
			next_text_ID	= UIText.INFO_A_OK,
			
			next_CB			=	function()
									if UIGlobals.Network.GameInviteState == UIEnums.MpGameInviteState.Processing then
										net_GameInviteFailed( )
										PopupCancel()
										EnableCarousel( true )
									end
								end,
		},
		
		[UIEnums.CustomPopups.MultiplayerCreateFailed] =
		{
			darken			= true,
			no_back			= true,
			colour_style	= "Main_Black",
			intensity		= 0.6,
			icon_name		= "groups",

			title_text_ID	= UIText.MP_MULTIPLAYER,
			message_text_ID	= UIText.MP_MULTIPLAYER_CREATE_FAILED,
			next_text_ID	= UIText.INFO_A_OK,
			next_CB			= function()
								net_MpLeave( )
								net_FlushEverything( )
								ForceCloseApp()
								end,
		},
		
		[UIEnums.CustomPopups.MultiplayerCreateFromPartyFailed] =
		{
			darken			= true,
			no_back			= true,
			colour_style	= "Main_Black",
			intensity		= 0.6,
			icon_name		= "groups",

			title_text_ID	= UIText.MP_MULTIPLAYER,
			message_text_ID	= UIText.MP_MULTIPLAYER_CREATE_FAILED,
			next_text_ID	= UIText.INFO_A_OK,
			
			init_CB			= function()
								NetRace.Delete()
							end,
								
			next_CB			= function()
								Mp_ReturnToPartyLobby( )
							end,
		},
				
		[UIEnums.CustomPopups.MultiplayerCreateOnlineGame] =
		{
			darken			= true,
			no_back			= true,
			no_next			= true,
			show_progress	= true,

			title_text_ID	= UIText.POP_TITLE_PLEASE_WAIT,
			message_text_ID	= UIText.POP_CREATING_GAME,
			timeout			= 35,
			
			timeout_CB		=	function()
									net_FlushSessionEnumerator( )
									net_CloseAllSessions( )
									PopupSpawn( UIEnums.CustomPopups.MultiplayerCreateFailed )
								end,
			
			init_CB			=	function()
									print( "ONLINE - Creating game..." )

									NetRace.Delete( )
									net_FlushSessionEnumerator()

									PopupGUI.state = UIEnums.MpOnlineGameCreation.InitialDelay
									PopupGUI.timer = 0

									if NetParty.IsRunning( ) == true then 
										PopupGUI.state = UIEnums.MpOnlineGameCreation.SyncPartyStart
									end	
								end,
			
			update_CB		=	function(time)
									PopupGUI.timer = PopupGUI.timer + time	

									if PopupGUI.state == UIEnums.MpOnlineGameCreation.SyncPartyStart then
										-- bit dirty but we are changing states so we reset the timeout timer to get the full timeout again.
										PopupGUI.timeout_timer = 0
											
										if NetParty.MatchmakingSessionIdle( ) == true and NetParty.AllPeersIdle( ) == true then
											PopupGUI.state = UIEnums.MpOnlineGameCreation.InitialDelay
										else
											if PopupGUI.timer > 10 then
												PopupGUI.timer = 0
												PopupGUI.state = UIEnums.MpOnlineGameCreation.Failed
												NetParty.SendReturnToPartyLobby( )
											end  
										end 
									
									elseif PopupGUI.state == UIEnums.MpOnlineGameCreation.InitialDelay then
										if PopupGUI.timer > 1 then
											if NetRace.CreateOnlineServer( ) == true then 
												print( "ONLINE - Game Created" )
												PopupGUI.state = UIEnums.MpOnlineGameCreation.Creating
											else
												PopupSpawn( UIEnums.CustomPopups.MultiplayerCreateFailed )
											end
										end	
										
									elseif PopupGUI.state == UIEnums.MpOnlineGameCreation.Creating then
										local finished = false
										local success = false
										finished, success = NetRace.ContinueCreateOnlineServer( )
										if finished == true then
											if success == true then
												PopupGUI.timer = 0
												if NetParty.IsRunning( ) == true then
													PopupGUI.state = UIEnums.MpOnlineGameCreation.SyncPartyEnd
													NetParty.ChangeLocalPlayersState( UIEnums.Network.PlayerStates.eNetPlayerStatePartyJoinRaceSuccessful )
													NetParty.SendJoinRaceSession( )	

													print( "ONLINE - Waiting for party peers" )
												else
													PopupGUI.state = UIEnums.MpOnlineGameCreation.Complete	
												end
											else
												if NetParty.IsRunning( ) == true then
													PopupGUI.timer = 0
													NetParty.SendReturnToPartyLobby( )
													PopupGUI.state = UIEnums.MpOnlineGameCreation.Failed
												else
													NetRace.Delete()
													PopupSpawn( UIEnums.CustomPopups.MultiplayerCreateFailed )
												end
											end
										end
									
									elseif PopupGUI.state == UIEnums.MpOnlineGameCreation.SyncPartyEnd then
										-- bit dirty but we are changing states so we reset the timeout timer to get the full timeout again.
										PopupGUI.timeout_timer = 0
										
										if PopupGUI.timer > 30 or NetParty.AnyPlayersFailedToJoinRace( ) == true then
											PopupGUI.timer = 0
											PopupGUI.state = UIEnums.MpOnlineGameCreation.Failed
											NetParty.SendReturnToPartyLobby( )
										else
											if NetParty.AllPlayersJoinedRace( ) == true then
												PopupGUI.state = UIEnums.MpOnlineGameCreation.Complete	
											end
										end
									
									elseif PopupGUI.state == UIEnums.MpOnlineGameCreation.Complete then
										PopupCancel( "Multiplayer\\MpLobby.lua", true )
									
									elseif PopupGUI.state == UIEnums.MpOnlineGameCreation.Failed then
										if NetParty.IsRunning( ) == true and PopupGUI.timer > 5 then
											PopupSpawn( UIEnums.CustomPopups.MultiplayerCreateFromPartyFailed )	
										end
									else	
										print("UIEnums.CustomPopups.MultiplayerCreateOnlineGame - unknown state")
										PopupCancel()
									end
								end,
		},
		[UIEnums.CustomPopups.MultiplayerJoinOnlineGame] =
		{
			darken			= true,
			no_next			= true,
			no_back			= true,
			show_progress	= true,

			title_text_ID	= UIText.POP_TITLE_PLEASE_WAIT,
			message_text_ID	= UIText.POP_ATTEMPTING_TO_JOIN_GAME,
			timeout			= 35,

			timeout_CB		=	function()
									NetRace.Delete( )
									net_FlushSessionEnumerator()
									PopupSpawn( UIEnums.CustomPopups.MultiplayerCreateOnlineGame )
								end,

			init_CB			=	function()
									UIGlobals.Network.JoinErrorCode = UIEnums.Network.JoinErrors.eJoinErrorInvalid
									PopupGUI.state = UIEnums.MpOnlineGameJoining.BandwidthScan
									PopupGUI.timer = 0

									print( "ONLINE - Joining game ", UIGlobals.Network.MatchingEnumeratedGameIndexToJoin )
								end,
								
			update_CB		=	function(time)
									PopupGUI.timer = PopupGUI.timer + time
									if PopupGUI.state == UIEnums.MpOnlineGameJoining.BandwidthScan then
										if NetServices.BandwidthEvaluationRunning( ) == false then
											
											-- bit dirty but we are changing states so we reset the timeout timer to get the full timeout again.
											PopupGUI.timeout_timer = 0
											
											PopupGUI.state = UIEnums.MpOnlineGameJoining.JoinStart
											
											if NetParty.IsRunning( ) == true then 
												PopupGUI.state = UIEnums.MpOnlineGameJoining.SyncPartyStart
											end					
										end
										
									elseif PopupGUI.state == UIEnums.MpOnlineGameJoining.SyncPartyStart then
										-- bit dirty but we are changing states so we reset the timeout timer to get the full timeout again.
										PopupGUI.timeout_timer = 0
											
										if NetParty.MatchmakingSessionIdle( ) == true and NetParty.AllPeersIdle( ) == true then
											NetParty.ChangeLocalPlayersState( UIEnums.Network.PlayerStates.eNetPlayerStatePartyJoinRace )
											PopupGUI.state = UIEnums.MpOnlineGameJoining.JoinStart

											print( "ONLINE - Waiting for party peers" )
										else
											if PopupGUI.timer > 10 then
												PopupGUI.timer = 0
												PopupGUI.state = UIEnums.MpOnlineGameJoining.Failed
												NetParty.SendReturnToPartyLobby( )
												PopupSpawn( UIEnums.CustomPopups.MultiplayerCreateOnlineGame )
											end  
										end 
										
									elseif PopupGUI.state == UIEnums.MpOnlineGameJoining.JoinStart then
										if PopupGUI.timer >= 1.5 then
											PopupGUI.state = UIEnums.MpOnlineGameJoining.JoinContinue

											if NetRace.JoinOnlineServer( UIGlobals.Network.MatchingEnumeratedGameIndexToJoin ) == false then
												PopupGUI.state = UIEnums.MpOnlineGameJoining.JoinStart
												
												-- try all 10 games, otherwise host a game
												UIGlobals.Network.MatchingEnumeratedGameIndexToJoin = UIGlobals.Network.MatchingEnumeratedGameIndexToJoin + 1

												if UIGlobals.Network.MatchingEnumeratedGameIndexToJoin >= UIGlobals.Network.MatchingNumSesionsEnumerated then
													UIGlobals.Network.MatchingEnumeratedGameIndexToJoin = 0
													PopupGUI.state = UIEnums.MpOnlineGameJoining.Failed
													
													PopupSpawn( UIEnums.CustomPopups.MultiplayerCreateOnlineGame )													
												else
													print( "ONLINE - Joining game ", UIGlobals.Network.MatchingEnumeratedGameIndexToJoin )
												end
											end
										end
									
									elseif PopupGUI.state == UIEnums.MpOnlineGameJoining.JoinContinue then
										local complete, code = NetRace.MonitorJoinOnlineServer( UIGlobals.Network.MatchingEnumeratedGameIndexToJoin )
										if complete == true then
											UIGlobals.Network.JoinErrorCode = code
										
											if code == UIEnums.Network.JoinErrors.eJoinErrorNone then 
												PopupGUI.timer = 0
												
												if NetParty.IsRunning( ) == true then												
													NetParty.ChangeLocalPlayersState( UIEnums.Network.PlayerStates.eNetPlayerStatePartyJoinRaceSuccessful )
													NetParty.SendJoinRaceSession( )	
													PopupGUI.state = UIEnums.MpOnlineGameJoining.SyncPartyEnd
												else
													PopupGUI.state = UIEnums.MpOnlineGameJoining.Complete
												end

												print( "ONLINE - Join successfull " )
											else
											
												-- bit dirty but we are changing states so we reset the timeout timer to get the full timeout again.
												PopupGUI.timeout_timer = 0
											
												-- try all 10 games, otherwise host a game
												UIGlobals.Network.MatchingEnumeratedGameIndexToJoin = UIGlobals.Network.MatchingEnumeratedGameIndexToJoin + 1

												if UIGlobals.Network.MatchingEnumeratedGameIndexToJoin >= UIGlobals.Network.MatchingNumSesionsEnumerated then
													UIGlobals.Network.MatchingEnumeratedGameIndexToJoin = 0
													PopupGUI.state = UIEnums.MpOnlineGameJoining.Failed
													
													PopupSpawn( UIEnums.CustomPopups.MultiplayerCreateOnlineGame )
												else
													print( "ONLINE - Joining game ", UIGlobals.Network.MatchingEnumeratedGameIndexToJoin )
													
													PopupGUI.timer = 0
													PopupGUI.state = UIEnums.MpOnlineGameJoining.JoinStart
												end	
											end
										end

									elseif PopupGUI.state == UIEnums.MpOnlineGameJoining.SyncPartyJoinNext then
										-- bit dirty but we are changing states so we reset the timeout timer to get the full timeout again.
										PopupGUI.timeout_timer = 0
												
										if PopupGUI.timer > 3 then
											NetRace.Delete( )
											PopupGUI.state = UIEnums.MpOnlineGameJoining.SyncPartyStart
											PopupGUI.timer = 0											
										end
	
									elseif PopupGUI.state == UIEnums.MpOnlineGameJoining.SyncPartyEnd then
										-- bit dirty but we are changing states so we reset the timeout timer to get the full timeout again.
										PopupGUI.timeout_timer = 0
										
										if PopupGUI.timer > 30 or NetParty.AnyPlayersFailedToJoinRace( ) == true then
											PopupGUI.timer = 0
											NetParty.SendReturnToPartyLobby( )
											NetParty.ChangeLocalPlayersState( UIEnums.Network.PlayerStates.eNetPlayerStatePartyBusy )
											
											-- try all 10 games, otherwise host a game
											UIGlobals.Network.MatchingEnumeratedGameIndexToJoin = UIGlobals.Network.MatchingEnumeratedGameIndexToJoin + 1

											if UIGlobals.Network.MatchingEnumeratedGameIndexToJoin >= UIGlobals.Network.MatchingNumSesionsEnumerated then
												UIGlobals.Network.MatchingEnumeratedGameIndexToJoin = 0
												PopupGUI.state = UIEnums.MpOnlineGameJoining.SyncPartyCreate
											else
												PopupGUI.state = UIEnums.MpOnlineGameJoining.SyncPartyJoinNext
											end
											
										else
											if NetParty.AllPlayersJoinedRace( ) == true then
												PopupGUI.state = UIEnums.MpOnlineGameJoining.Complete	
											end
										end
										
									elseif PopupGUI.state == UIEnums.MpOnlineGameJoining.SyncPartyCreate then
										-- bit dirty but we are changing states so we reset the timeout timer to get the full timeout again.
										PopupGUI.timeout_timer = 0
												
										if PopupGUI.timer > 3 then
											NetRace.Delete( )
											PopupSpawn( UIEnums.CustomPopups.MultiplayerCreateOnlineGame )	
										end
										
									elseif PopupGUI.state == UIEnums.MpOnlineGameJoining.Complete then
										PopupCancel( "Multiplayer\\MpLobby.lua", true )
									end
								end,
		},
		[UIEnums.CustomPopups.MultiplayerSearchOnlineGame] =
		{
			darken			= true,
			no_back			= true,
			no_next			= true,
			show_progress	= true,

			title_text_ID	= UIText.POP_TITLE_PLEASE_WAIT,
			message_text_ID	= UIText.POP_SEARCHING_FOR_GAMES,
			timeout			= 25,

			timeout_CB		=	function()
									if PopupGUI.state == UIEnums.MpOnlineGameEnumeration.Enumerating then
										net_StopSessionEnumerator( )
									else
										NetRace.Delete( )
										net_FlushSessionEnumerator()
										PopupSpawn( UIEnums.CustomPopups.MultiplayerCreateOnlineGame )
									end
								end,
								
			init_CB			=	function()
									PopupGUI.timer = 0
									PopupGUI.state = UIEnums.MpOnlineGameEnumeration.BandwidthScan

									print( "ONLINE - BandwidthScan" )
								end,

			update_CB		=	function(time)
									PopupGUI.timer = PopupGUI.timer + time	

									if PopupGUI.state == UIEnums.MpOnlineGameEnumeration.BandwidthScan then
										if NetServices.BandwidthEvaluationRunning( ) == false then
											if PopupGUI.timer > 1 then
												print( "ONLINE - Enumerating..." )
												PopupGUI.state = UIEnums.MpOnlineGameEnumeration.Enumerating
												
												-- bit dirty but we are changing states so we reset the timeout timer to get the full timeout again.
												PopupGUI.timeout_timer = 0

												if NetRace.StartOnlineSessionEnumeration( ) == false then
													PopupSpawn( UIEnums.CustomPopups.MultiplayerNoMatchingGames )
												end
											end
										end
										
									-- looking for games
									elseif PopupGUI.state == UIEnums.MpOnlineGameEnumeration.Enumerating then
										local search_finished = false
										local count = 0
										search_finished, count = NetRace.ContinueOnlineSessionEnumeration( )
										if search_finished == true then
											print( "ONLINE - Enumeration finished ", count )
											if count == 0 then
												PopupSpawn( UIEnums.CustomPopups.MultiplayerCreateOnlineGame )
											else
												UIGlobals.Network.MatchingEnumeratedGameIndexToJoin = 0
												UIGlobals.Network.MatchingNumSesionsEnumerated = count
												UIGlobals.ConnectionType = UIEnums.ConnectionType.CustomSearch

												PopupGUI.state = UIEnums.MpOnlineGameEnumeration.Complete
											end
										end
									elseif PopupGUI.state == UIEnums.MpOnlineGameEnumeration.Complete then
										PopupSpawn( UIEnums.CustomPopups.MultiplayerJoinOnlineGame )
									end
								end,
		},
		[UIEnums.CustomPopups.MultiplayerMatching] =
		{
			darken			= true,
			no_back			= true,
			no_next			= true,
			colour_style	= "Main_Black",
			intensity		= 0.6,
			icon_name		= "groups",
			show_progress	= true,

			title_text_ID	= UIText.POP_TITLE_PLEASE_WAIT,
			message_text_ID	= UIText.POP_SEARCHING_FOR_GAMES,
			timeout			= 25,
			
			timeout_CB		=	function()
									net_FlushSessionEnumerator( )
									PopupSpawn( UIEnums.CustomPopups.MultiplayerNoMatchingGames )
								end,
								
			init_CB			=	function()
									PopupGUI.state = UIEnums.MpOnlineGameEnumeration.BandwidthScan
									PopupGUI.timer = 0
								end,
			
			update_CB		=	function(time)
									PopupGUI.timer = PopupGUI.timer + time	
									
									if PopupGUI.state == UIEnums.MpOnlineGameEnumeration.BandwidthScan then
										if NetServices.BandwidthEvaluationRunning( ) == false then
											if PopupGUI.timer > 1 then
											
												-- bit dirty but we are changing states so we reset the timeout timer to get the full timeout again.
												PopupGUI.timeout_timer = 0
												
												PopupGUI.state = UIEnums.MpOnlineGameEnumeration.Enumerating
												
												if NetRace.StartOnlineSessionEnumeration( ) == false then
													PopupSpawn( UIEnums.CustomPopups.MultiplayerNoMatchingGames )
												end
											end
										end
										
									elseif PopupGUI.state == UIEnums.MpOnlineGameEnumeration.Enumerating then
										local search_finished = false
										local count = 0
										search_finished, count = NetRace.ContinueOnlineSessionEnumeration( )
										if search_finished == true then
											PopupGUI.state = UIEnums.MpOnlineGameEnumeration.Complete
											if count == 0 then
												PopupSpawn( UIEnums.CustomPopups.MultiplayerNoMatchingGames )
											else
												UIGlobals.Network.MatchingNumSesionsEnumerated = count
												PopupCancel( "Multiplayer\\Events\\MpLobbyList.lua", true )
											end
										end

									elseif PopupGUI.state == UIEnums.MpOnlineGameEnumeration.Complete then
									end
								end,
		},	
		
		[UIEnums.CustomPopups.MultiplayerQuickMatching] =
		{
			darken			= true,
			no_back			= true,
			no_next			= true,
			colour_style	= "Main_Black",
			intensity		= 0.6,
			icon_name		= "groups",
			show_progress	= true,

			title_text_ID	= UIText.POP_TITLE_PLEASE_WAIT,
			message_text_ID	= UIText.POP_SEARCHING_FOR_GAMES,
			timeout			= 25,

			timeout_CB		=	function()
									net_FlushSessionEnumerator( )
									net_CloseAllSessions( )
									PopupSpawn( UIEnums.CustomPopups.MultiplayerNoMatchingGames )
								end,
			
			init_CB			=	function()
									PopupGUI.state = UIEnums.MpOnlineQuickMatching.BandwidthScan
									PopupGUI.timer = 0
								end,
			
			update_CB		=	function(time)
									PopupGUI.timer = PopupGUI.timer + time	
									
									if PopupGUI.state == UIEnums.MpOnlineQuickMatching.BandwidthScan then
										if NetServices.BandwidthEvaluationRunning( ) == false then
											if PopupGUI.timer > 1 then
											
												-- bit dirty but we are changing states so we reset the timeout timer to get the full timeout again.
												PopupGUI.timeout_timer = 0
											
												PopupGUI.state = UIEnums.MpOnlineQuickMatching.Enumerating
												if NetRace.StartOnlineSessionEnumeration( ) == false then
													PopupSpawn( UIEnums.CustomPopups.MultiplayerNoMatchingGames )
												end
											end
										end
										
									elseif PopupGUI.state == UIEnums.MpOnlineQuickMatching.Enumerating then
										local search_finished = false
										local count = 0
										search_finished, count = NetRace.ContinueOnlineSessionEnumeration( )
										if search_finished == true then
											if count == 0 then
												PopupSpawn( UIEnums.CustomPopups.MultiplayerNoMatchingGames )
											else
											
												-- bit dirty but we are changing states so we reset the timeout timer to get the full timeout again.
												PopupGUI.timeout_timer = 0
											
												UIGlobals.Network.MatchingNumSesionsEnumerated = count
												PopupGUI.state = UIEnums.MpOnlineQuickMatching.JoinStart
											end
										end
									
									elseif PopupGUI.state == UIEnums.MpOnlineQuickMatching.JoinStart then 
										if UIGlobals.Network.MatchingEnumeratedGameIndexToJoin >= UIGlobals.Network.MatchingNumSesionsEnumerated then
											PopupSpawn( UIEnums.CustomPopups.MultiplayerJoinMatchingGameFailed )
										else
											local join_in_race = true
										
											if join_in_race == true then
												if NetRace.JoinOnlineServer( UIGlobals.Network.MatchingEnumeratedGameIndexToJoin ) == false then
													-- failed to start join procedure try nect session or show join error if no more sessions
													UIGlobals.Network.JoinErrorCode = UIEnums.Network.JoinErrors.eJoinErrorInvalid
													UIGlobals.Network.MatchingEnumeratedGameIndexToJoin = UIGlobals.Network.MatchingEnumeratedGameIndexToJoin + 1
												else
													PopupGUI.state = UIEnums.MpOnlineQuickMatching.JoinContinue
												end
											else
												UIGlobals.Network.JoinErrorCode = UIEnums.Network.JoinErrors.eJoinErrorInvalid
												UIGlobals.Network.MatchingEnumeratedGameIndexToJoin = UIGlobals.Network.MatchingEnumeratedGameIndexToJoin + 1
												if UIGlobals.Network.MatchingEnumeratedGameIndexToJoin >= UIGlobals.Network.MatchingNumSesionsEnumerated then
													PopupSpawn( UIEnums.CustomPopups.MultiplayerNoMatchingGames )
												end
											end
										end
									
									elseif PopupGUI.state == UIEnums.MpOnlineQuickMatching.JoinContinue then
										local complete = false
										local code = UIEnums.Network.JoinErrors.eJoinErrorInvalid
										complete, code = NetRace.MonitorJoinOnlineServer( UIGlobals.Network.MatchingEnumeratedGameIndexToJoin )
										
										if complete == true then
											UIGlobals.Network.JoinErrorCode = code
										
											if code == UIEnums.Network.JoinErrors.eJoinErrorNone then 
												PopupGUI.state = UIEnums.MpOnlineQuickMatching.Complete
												PopupCancel( "Multiplayer\\MpLobby.lua", true )
											else
												NetRace.Delete( )
												
												-- try join next session if it exists
												PopupGUI.state = UIEnums.MpOnlineQuickMatching.JoinStart
												UIGlobals.Network.MatchingEnumeratedGameIndexToJoin = UIGlobals.Network.MatchingEnumeratedGameIndexToJoin + 1
												
												-- bit dirty but we are changing states so we reset the timeout timer to get the full timeout again.
												PopupGUI.timeout_timer = 0
											end
										end
									
									elseif PopupGUI.state == UIEnums.MpOnlineQuickMatching.Complete then
										-- bit dirty but we are changing states so we reset the timeout timer to get the full timeout again.
										PopupGUI.timeout_timer = 0
									end
								end,
		},	
		
		[UIEnums.CustomPopups.MultiplayerNoMatchingGames] =
		{
			darken			= true,
			no_back			= true,
			colour_style	= "Main_Black",
			intensity		= 0.6,
			icon_name		= "groups",

			title_text_ID	= UIText.MP_SEARCH_FINISHED,
			message_text_ID	= UIText.MP_NO_MATCHING_GAMES_FOUND,
			next_text_ID	= UIText.INFO_A_OK,
			
			init_CB			=	function()
									NetServices.FlushEnumertedSessions( )
								end,
		},
		
		[UIEnums.CustomPopups.MultiplayerJoinMatchingGame] =
		{
			darken			= true,
			no_back			= true,
			no_next			= true,
			colour_style	= "Main_Black",
			intensity		= 0.6,
			icon_name		= "groups",

			title_text_ID	= UIText.POP_TITLE_PLEASE_WAIT,
			message_text_ID	= UIText.POP_ATTEMPTING_TO_JOIN_GAME,
			timeout			= 25,

			timeout_CB		=	function()
									net_FlushSessionEnumerator( )
									net_CloseAllSessions( )
									UIGlobals.Network.JoinErrorCode = UIEnums.Network.JoinErrors.eJoinErrorInvalid
									PopupSpawn( UIEnums.CustomPopups.MultiplayerJoinMatchingGameFailed )
								end,
			
			init_CB			=	function()
									UIGlobals.Network.JoinErrorCode = UIEnums.Network.JoinErrors.eJoinErrorInvalid
									PopupGUI.state = UIEnums.MpOnlineMatching.BandwidthScan
									PopupGUI.timer = 0
								end,
								
		update_CB			=	function(time)
									PopupGUI.timer = PopupGUI.timer + time
									if PopupGUI.state == UIEnums.MpOnlineMatching.BandwidthScan then
										if NetServices.BandwidthEvaluationRunning( ) == false then
										
											-- bit dirty but we are changing states so we reset the timeout timer to get the full timeout again.
											PopupGUI.timeout_timer = 0
											
											if NetParty.IsRunning( ) == false then
												PopupGUI.state = UIEnums.MpOnlineMatching.JoinStart
											else
												if NetParty.AllPeersIdle( ) == true and NetParty.MatchmakingSessionIdle( ) == true then
													PopupGUI.state = UIEnums.MpOnlineMatching.JoinStart
												else
													if PopupGUI.timer > 20 then
														PopupGUI.timer = 0
														NetRace.Delete( )
														PopupSpawn( UIEnums.CustomPopups.MultiplayerJoinMatchingGameFailed )	
													end
												end
											end
										end
										
									elseif PopupGUI.state == UIEnums.MpOnlineMatching.JoinStart then
										
										-- bit dirty but we no longer need the popup to auto timeout.
										PopupGUI.timeout_timer = 0
										
										if PopupGUI.timer >= 1.5 then
											PopupGUI.state = UIEnums.MpOnlineMatching.JoinContinue
											
											if NetRace.JoinOnlineServer( UIGlobals.Network.MatchingEnumeratedGameIndexToJoin ) == false then
												PopupSpawn( UIEnums.CustomPopups.MultiplayerJoinMatchingGameFailed )
											end
										end
										
									elseif PopupGUI.state == UIEnums.MpOnlineMatching.JoinContinue then
										local complete = false
										local code = UIEnums.Network.JoinErrors.eJoinErrorInvalid
										complete, code = NetRace.MonitorJoinOnlineServer( UIGlobals.Network.MatchingEnumeratedGameIndexToJoin )
										
										if complete == true then
											UIGlobals.Network.JoinErrorCode = code
										
											if code == UIEnums.Network.JoinErrors.eJoinErrorNone then 
												local _GUI = ContextTable[ UIEnums.Context.Main ].GUI
												if IsTable( _GUI ) == true then
													if NetParty.IsRunning( ) == true then
													
														-- bit dirty but we no longer need the popup to auto timeout.
														PopupGUI.timeout_timer = 0
										
														PopupGUI.state = UIEnums.MpOnlineMatching.WaitingForParty
														NetParty.ChangeLocalPlayersState( UIEnums.Network.PlayerStates.eNetPlayerStatePartyJoinRaceSuccessful )
														NetParty.SendJoinRaceSession( )	
														PopupGUI.timer = 0
													else
														PopupGUI.state = UIEnums.MpOnlineMatching.Complete
													end
												end
											else
												PopupSpawn( UIEnums.CustomPopups.MultiplayerJoinMatchingGameFailed )
											end
										end
										
									elseif PopupGUI.state == UIEnums.MpOnlineMatching.WaitingForParty then
										
										-- bit dirty but we no longer need the popup to auto timeout.
										PopupGUI.timeout_timer = 0
											
										if PopupGUI.timer > 20  or NetParty.AnyPlayersFailedToJoinRace( ) == true then
											PopupGUI.timer = 0
											NetParty.SendReturnToPartyLobby( )
											PopupGUI.state = UIEnums.MpOnlineMatching.PartyJoinFailure
										else
											if NetParty.AllPlayersJoinedRace( ) == true then
												PopupGUI.state = UIEnums.MpOnlineMatching.Complete	
											end
										end
									
									elseif PopupGUI.state == UIEnums.MpOnlineMatching.PartyJoinFailure then
										-- bit dirty but we no longer need the popup to auto timeout.
										PopupGUI.timeout_timer = 0
										
										if PopupGUI.timer > 5 then
											NetRace.Delete( )
											UIGlobals.Network.JoinErrorCode = UIEnums.Network.JoinErrors.eJoinErrorNoCommunications
											PopupSpawn( UIEnums.CustomPopups.MultiplayerJoinMatchingGameFailed )
										end
										
									elseif PopupGUI.state == UIEnums.MpOnlineMatching.Complete then
										PopupCancel( "Multiplayer\\MpLobby.lua", true )
									end
									
								end,
		},	
		
		[UIEnums.CustomPopups.MultiplayerJoinMatchingGameFailed] =
		{
			darken			= true,
			no_back			= true,
			colour_style	= "Main_Black",
			intensity		= 0.6,
			icon_name		= "groups",

			title_text_ID	= UIText.POP_TITLE_ERROR,
			message_text_ID	= UIText.MP_JOIN_ERROR_DEFAULT,
			next_text_ID	= UIText.INFO_A_OK,
									
			init_CB			=	function()
									local errorString = UIText.MP_JOIN_ERROR_DEFAULT
									
									if UIGlobals.Network.JoinErrorCode == UIEnums.Network.JoinErrors.eJoinErrorNotEnoughOpenSlots then
										errorString = UIText.MP_JOIN_ERROR_NO_SLOTS
									elseif UIGlobals.Network.JoinErrorCode == UIEnums.Network.JoinErrors.eJoinErrorBannedFromSession then
										errorString = UIText.MP_JOIN_ERROR_BANNED
									elseif UIGlobals.Network.JoinErrorCode == UIEnums.Network.JoinErrors.eJoinErrorNoConnectionToHost then
										errorString = UIText.MP_JOIN_ERROR_HOST
									elseif UIGlobals.Network.JoinErrorCode == UIEnums.Network.JoinErrors.eJoinErrorNoCommunications then
										errorString = UIText.MP_JOIN_ERROR_PEERS
									elseif UIGlobals.Network.JoinErrorCode == UIEnums.Network.JoinErrors.eJoinErrorToManyTeams then
										errorString = UIText.MP_JOIN_ERROR_TEAM
									elseif UIGlobals.Network.JoinErrorCode == UIEnums.Network.JoinErrors.eJoinErrorToManyTeamMembers then
										errorString = UIText.MP_JOIN_ERROR_TEAM_MEMBERS
									elseif UIGlobals.Network.JoinErrorCode == UIEnums.Network.JoinErrors.eJoinErrorContentMismatch then
										errorString = UIText.MP_JOIN_ERROR_CONTENT
									elseif UIGlobals.Network.JoinErrorCode == UIEnums.Network.JoinErrors.eJoinErrorChampionshipStarted then
										errorString = UIText.MP_JOIN_ERROR_CHAMPIONSHIP_STARTED
									elseif UIGlobals.Network.JoinErrorCode == UIEnums.Network.JoinErrors.eJoinErrorRankedMatchStarted then
										errorString = UIText.MP_JOIN_ERROR_RANKED_STARTED
									elseif UIGlobals.Network.JoinErrorCode == UIEnums.Network.JoinErrors.eJoinErrorPartyClosed then
										errorString = UIText.MP_JOIN_ERROR_PARTY_CLOSED
									end
									-- Change message text
									CustomPopup.CurrentData.message_text_ID = errorString
									
									if NetRace.OnlineJoinFailedNameAvailable() == true then
										CustomPopup.CurrentData.message_text_ID = "MATCHING_JOIN_ERROR_RACE"
									end
									
									NetRace.Delete( )
									
									if NetParty.IsRunning( ) == true then
										NetParty.ChangeLocalPlayersState( UIEnums.Network.PlayerStates.eNetPlayerStatePartyBusy )
									end
								end,
								
			next_CB			=	function()
									--NetServices.FlushEnumertedSessions( )
								end,

		},
		
		[UIEnums.CustomPopups.MultiplayerAcceptGameInvitation] =
		{
			darken			= true,
			no_next			= true,
			no_back			= true,
			colour_style	= "Main_Black",
			intensity		= 0.6,
			icon_name		= "groups",
			show_progress	= true,

			title_text_ID	= UIText.POP_TITLE_PLEASE_WAIT,
			message_text_ID	= UIText.POP_ATTEMPTING_TO_JOIN_GAME,
			timeout			= 25,

			timeout_CB		=	function()
									print( "GAME INVITE - Timed out" )

									net_FlushSessionEnumerator( )
									net_CloseAllSessions( )
									PopupSpawn( UIEnums.CustomPopups.MultiplayerAcceptGameInvitationFailed )
								end,
	
			init_CB			=	function()	
									UIGlobals.Network.JoinErrorCode = UIEnums.Network.JoinErrors.eJoinErrorInvalid
									UIGlobals.Network.GameInviteType = UIEnums.MpGameInviteType.Unknown
									PopupGUI.state = UIEnums.MpOnlineQuickMatching.BandwidthScan
									PopupGUI.timer = 0
									NetServices.StartBandwidthEvaluation( )
								end,
								
			update_CB		=	function(time)
									PopupGUI.timer = PopupGUI.timer + time	
		 
									if PopupGUI.state == UIEnums.MpOnlineQuickMatching.BandwidthScan then
										if NetServices.BandwidthEvaluationRunning( ) == false then
											if PopupGUI.timer > 1 then
												
												-- bit dirty but we are changing states so we reset the timeout timer to get the full timeout again.
												PopupGUI.timeout_timer = 0
												
												PopupGUI.state = UIEnums.MpOnlineQuickMatching.Enumerating
												if NetServices.StartOnlineSessionEnumerationFromInvitation( ) == false then
													print( "GAME INVITE - Failed to start online session enumeration" )
													PopupSpawn( UIEnums.CustomPopups.MultiplayerAcceptGameInvitationFailed )
												end
											end
										end
									
									elseif PopupGUI.state == UIEnums.MpOnlineQuickMatching.Enumerating then
										local finished = false
										local error = false
										local race_session = false
										local ranked_mode = false
										local custom_race = false
										local event_id = 0
										local version_num_diff = 0		-- 1 = your playlist ver < host; 2 = host playlist ver < yours;
										
										finished, error, race_session, ranked_mode, custom_race, event_id, version_num_diff = NetServices.ContinueOnlineSessionEnumerationFromInvitation( )
										
										if finished == true then
											if error == true then
												if version_num_diff == 1 then
													-- 1 = your playlist ver < host
													UIGlobals.Network.JoinErrorCode = UIEnums.Network.JoinErrors.eJoinErrorPlaylistVerHost
												elseif version_num_diff == 2 then
													-- 2 = host playlist ver < yours
													UIGlobals.Network.JoinErrorCode = UIEnums.Network.JoinErrors.eJoinErrorPlaylistVerClient
												end
												
												PopupSpawn( UIEnums.CustomPopups.MultiplayerAcceptGameInvitationFailed )
											else											
												-- bit dirty but we are changing states so we reset the timeout timer to get the full timeout again.
												PopupGUI.timeout_timer = 0
											
												PopupGUI.state = UIEnums.MpOnlineQuickMatching.JoinStart
												UIGlobals.Network.MatchingEnumeratedGameIndexToJoin = 0

												if race_session == true then
													UIGlobals.Network.GameInviteType = UIEnums.MpGameInviteType.Race

													if custom_race == true then
														net_MpEnterPlayMode( UIEnums.PlayMode.CustomRace )
													else
														-- if ranked then we need to find it in sorcery. If that fails then stop and fail
														Multiplayer.CachePlaylists( UIEnums.GameMode.Online )

														-- check we can play
														if Multiplayer.FindPlaylistFromID( event_id ) == true then
															net_MpEnterPlayMode( UIEnums.PlayMode.Playlist, ranked_mode )
														else
															print( "Invite failed because you cant play the event" )
															PopupSpawn( UIEnums.CustomPopups.MultiplayerAcceptGameInvitationFailed )
															PopupGUI.state = UIEnums.MpOnlineQuickMatching.Failed
														end
													end
												else
													UIGlobals.Network.GameInviteType = UIEnums.MpGameInviteType.Party
												end

												net_MpEnter( UIEnums.GameMode.Online )
											end
										end
									
									elseif PopupGUI.state == UIEnums.MpOnlineQuickMatching.JoinStart then
										if UIGlobals.Network.GameInviteType == UIEnums.MpGameInviteType.Race then
											if NetRace.JoinOnlineServer( UIGlobals.Network.MatchingEnumeratedGameIndexToJoin ) == false then
												print( "GAME INVITE - Failed to join online server" )
												PopupSpawn( UIEnums.CustomPopups.MultiplayerAcceptGameInvitationFailed )
											else
												PopupGUI.state = UIEnums.MpOnlineQuickMatching.JoinContinue
											end
										elseif UIGlobals.Network.GameInviteType == UIEnums.MpGameInviteType.Party then
											if NetParty.JoinOnlineServer( UIGlobals.Network.MatchingEnumeratedGameIndexToJoin ) == false then
												PopupSpawn( UIEnums.CustomPopups.MultiplayerAcceptGameInvitationFailed )
											else
												PopupGUI.state = UIEnums.MpOnlineQuickMatching.JoinContinue
											end
										else
											print( "GAME INVITE - Invalid invite type" )
											PopupSpawn( UIEnums.CustomPopups.MultiplayerAcceptGameInvitationFailed )
										end
									
									elseif PopupGUI.state == UIEnums.MpOnlineQuickMatching.JoinContinue then
										if UIGlobals.Network.GameInviteType == UIEnums.MpGameInviteType.Race then
											local complete = false
											local code = UIEnums.Network.JoinErrors.eJoinErrorInvalid
											complete, code = NetRace.MonitorJoinOnlineServer( UIGlobals.Network.MatchingEnumeratedGameIndexToJoin )
								
											if complete == true then
												UIGlobals.Network.JoinErrorCode = code
										
												if code == UIEnums.Network.JoinErrors.eJoinErrorNone then 
													PopupGUI.state = UIEnums.MpOnlineQuickMatching.Complete
													UIGlobals.Network.GameInviteState = UIEnums.MpGameInviteState.Polling
													PopupCancel( UIEnums.MpLaunchScreen.MultiplayerLobby, false, true )
													
													-- [WillB] #37785 TCR:R041 - Need to present the user with a message informing them 
													UIGlobals.Network.VoiceChatRestricted = ( NetServices.CanUseChat() == false )
												else
													PopupSpawn( UIEnums.CustomPopups.MultiplayerAcceptGameInvitationFailed )
												end
											end
											
										elseif UIGlobals.Network.GameInviteType == UIEnums.MpGameInviteType.Party then
											local complete = false
											local code = UIEnums.Network.JoinErrors.eJoinErrorInvalid
											complete, code = NetParty.MonitorJoinOnlineServer( UIGlobals.Network.MatchingEnumeratedGameIndexToJoin )
								
											if complete == true then
												UIGlobals.Network.JoinErrorCode = code
										
												if code == UIEnums.Network.JoinErrors.eJoinErrorNone then 
													PopupGUI.state = UIEnums.MpOnlineQuickMatching.Complete
													UIGlobals.Network.GameInviteState = UIEnums.MpGameInviteState.Polling
													PopupCancel( UIEnums.MpLaunchScreen.PartyLobby, false, true )
													
													-- [WillB] #37785 TCR:R041 - Need to present the user with a message informing them 
													UIGlobals.Network.VoiceChatRestricted = ( NetServices.CanUseChat() == false )
												else
													PopupSpawn( UIEnums.CustomPopups.MultiplayerAcceptGameInvitationFailed )
												end
											end
											
										else
											PopupSpawn( UIEnums.CustomPopups.MultiplayerAcceptGameInvitationFailed )
										end
									
									elseif PopupGUI.state == UIEnums.MpOnlineQuickMatching.Complete then
									
									end
								end,

		},
		
		[UIEnums.CustomPopups.MultiplayerAcceptGameInvitationFailed] =
		{
			darken			= true,
			no_back			= true,
			colour_style	= "Main_Black",
			intensity		= 0.6,
			icon_name		= "groups",

			title_text_ID	= UIText.POP_TITLE_ERROR,
			message_text_ID	= UIText.MP_GAME_INVITATION_FAILED,
			next_text_ID	= UIText.INFO_A_OK,
						
			init_CB			=	function()
									NetServices.ClearGameInvite( )
			
									local errorString = UIText.MP_JOIN_ERROR_DEFAULT

									if UIGlobals.Network.JoinErrorCode == UIEnums.Network.JoinErrors.eJoinErrorNotEnoughOpenSlots then
										errorString = UIText.MP_JOIN_ERROR_NO_SLOTS
									elseif UIGlobals.Network.JoinErrorCode == UIEnums.Network.JoinErrors.eJoinErrorBannedFromSession then
										errorString = UIText.MP_JOIN_ERROR_BANNED
									elseif UIGlobals.Network.JoinErrorCode == UIEnums.Network.JoinErrors.eJoinErrorNoConnectionToHost then
										errorString = UIText.MP_JOIN_ERROR_HOST
									elseif UIGlobals.Network.JoinErrorCode == UIEnums.Network.JoinErrors.eJoinErrorNoCommunications then
										errorString = UIText.MP_JOIN_ERROR_PEERS
									elseif UIGlobals.Network.JoinErrorCode == UIEnums.Network.JoinErrors.eJoinErrorToManyTeams then
										errorString = UIText.MP_JOIN_ERROR_TEAM
									elseif UIGlobals.Network.JoinErrorCode == UIEnums.Network.JoinErrors.eJoinErrorToManyTeamMembers then
										errorString = UIText.MP_JOIN_ERROR_TEAM_MEMBERS
									elseif UIGlobals.Network.JoinErrorCode == UIEnums.Network.JoinErrors.eJoinErrorContentMismatch then
										errorString = UIText.MP_JOIN_ERROR_CONTENT
									elseif UIGlobals.Network.JoinErrorCode == UIEnums.Network.JoinErrors.eJoinErrorChampionshipStarted then
										errorString = UIText.MP_JOIN_ERROR_CHAMPIONSHIP_STARTED
									elseif UIGlobals.Network.JoinErrorCode == UIEnums.Network.JoinErrors.eJoinErrorRankedMatchStarted then
										errorString = UIText.MP_JOIN_ERROR_RANKED_STARTED
									elseif UIGlobals.Network.JoinErrorCode == UIEnums.Network.JoinErrors.eJoinErrorPartyClosed then
										errorString = UIText.MP_JOIN_ERROR_PARTY_CLOSED
									elseif UIGlobals.Network.JoinErrorCode == UIEnums.Network.JoinErrors.eJoinErrorPlaylistVerClient then
										errorString = UIText.MP_JOIN_ERROR_PLAYLIST_VERSION_CLIENT
									elseif UIGlobals.Network.JoinErrorCode == UIEnums.Network.JoinErrors.eJoinErrorPlaylistVerHost then
										errorString = UIText.MP_JOIN_ERROR_PLAYLIST_VERSION_HOST
									end
									
									-- Change message text
									CustomPopup.CurrentData.message_text_ID = errorString
									
									-- auto size by default
									CustomPopup.CurrentData.size = nil 
									
									if UIGlobals.Network.GameInviteType == UIEnums.MpGameInviteType.Race then
										if NetRace.OnlineJoinFailedNameAvailable() == true then
											CustomPopup.CurrentData.size =	{ 
																			x = 240,
																			y = 80,
																			}
											CustomPopup.CurrentData.message_text_ID = "MATCHING_JOIN_ERROR_RACE"
										end
									elseif UIGlobals.Network.GameInviteType == UIEnums.MpGameInviteType.Party then
										if NetParty.OnlineJoinFailedNameAvailable() == true then
											CustomPopup.CurrentData.size =	{  
																			x = 240,
																			y = 80,
																			}
											CustomPopup.CurrentData.message_text_ID = "MATCHING_JOIN_ERROR_PARTY"
										end
									end
								end,
								
			next_CB			=	function()
									net_GameInviteFailed( )
									PopupCancel()
									EnableCarousel( true )
								end,
		},
		
		[UIEnums.CustomPopups.MultiplayerUnrecoverableError] =
		{
			darken			= true,
			no_back			= true,
			colour_style	= "Main_Black",
			intensity		= 0.6,
			icon_name		= "groups",

			title_text_ID	= UIText.MP_BIG_ERROR_TITLE,
			message_text_ID	= UIText.MP_BIG_ERROR,
			next_text_ID	= UIText.INFO_A_OK,
		},
		
		[UIEnums.CustomPopups.MultiplayerRouterError] =
		{
			darken			= true,
			no_back			= true,
			colour_style	= "Main_Black",
			intensity		= 0.6,
			icon_name		= "groups",

			title_text_ID	= UIText.MP_ROUTER_ERROR_TITLE,
			message_text_ID	= UIText.MP_ROUTER_ERROR,
			next_text_ID	= UIText.INFO_A_OK,
		},
		
		[UIEnums.CustomPopups.MultiplayerCreateOnlineParty] =
		{
			darken			= true,
			no_back			= true,
			no_next			= true,
			colour_style	= "Main_Black",
			intensity		= 0.6,
			icon_name		= "groups",
			show_progress	= true,
			timeout			= 15,
			
			title_text_ID	= UIText.POP_TITLE_PLEASE_WAIT,
			message_text_ID	= UIText.POP_CREATING_PARTY,
			
			timeout_CB		=	function()
									net_CloseAllSessions( )
									PopupSpawn( UIEnums.CustomPopups.MultiplayerCreateFailed )
								end,
			
			init_CB			=	function()
									PopupGUI.state = UIEnums.MpOnlineGameCreation.BandwidthScan
									PopupGUI.timer = 0
								end,
			
			update_CB		=	function(time)
									PopupGUI.timer = PopupGUI.timer + time	
									if PopupGUI.state == UIEnums.MpOnlineGameCreation.BandwidthScan then
										if NetServices.BandwidthEvaluationRunning( ) == false then
											
											-- bit dirty but we are changing states so we reset the timeout timer to get the full timeout again.
											PopupGUI.timeout_timer = 0
												
											PopupGUI.state = UIEnums.MpOnlineGameCreation.InitialDelay
										end
																		
									elseif PopupGUI.state == UIEnums.MpOnlineGameCreation.InitialDelay then
										if PopupGUI.timer > 1 then
											if NetParty.CreateOnlineServer( ) == true then 
												PopupGUI.state = UIEnums.MpOnlineGameCreation.Creating
											else
												PopupSpawn( UIEnums.CustomPopups.MultiplayerCreateFailed )
											end
										end	
										
									elseif PopupGUI.state == UIEnums.MpOnlineGameCreation.Creating then
										local finished = false
										local success = false
										finished, success = NetParty.ContinueCreateOnlineServer( )
										if finished == true then
											if success == true then
												PopupGUI.state = UIEnums.MpOnlineGameCreation.Complete	
											else
												NetRace.Delete( )
												PopupSpawn( UIEnums.CustomPopups.MultiplayerCreateFailed )
											end
										end
										
									elseif PopupGUI.state == UIEnums.MpOnlineGameCreation.Complete then
										net_EnterPartyLobby( false )
										PopupCancel()
									else		
										print("UIEnums.CustomPopups.MultiplayerCreateOnlineGame - unknown state")
									end
								end,
		},	
		[UIEnums.CustomPopups.MultiplayerPartyTooBig] =
		{
			darken			= true,
			no_back			= true,
			colour_style	= "Main_Black",
			intensity		= 0.6,
			icon_name		= "groups",
			
			title_text_ID	= UIText.MP_PARTY_TOO_BIG_TITLE,
			message_text_ID	= UIText.MP_PARTY_TOO_BIG,
			next_text_ID	= UIText.INFO_A_OK,
		},
		[UIEnums.CustomPopups.DeleteGroup] =
		{
			darken			= true,
			colour_style	= "Main_Black",
			intensity		= 0.6,
			icon_name		= "groups",

			title_text_ID	= UIText.POP_DELETE_GROUP,
			message_text_ID	= UIText.POP_DELETE_GROUP_MSG,
			
			next_text_ID	= UIText.INFO_YES_A,
			back_text_ID	= UIText.INFO_NO_B,
			
			next_CB			=	function()					
									RemoveUserGroup()
								end,
		},
		[UIEnums.CustomPopups.SaveNewEvent] =
		{
			darken			= true,
			colour_style	= "Main_Black",
			intensity		= 0.6,
			icon_name		= "groups",

			title_text_ID	= UIText.POP_TITLE_WARNING,
			message_text_ID	= UIText.POP_SAVE_NEW_EVENT_MSG,
			
			next_text_ID	= UIText.INFO_YES_A,
			back_text_ID	= UIText.INFO_NO_B,
			
			
			next_CB			=	function()
									RemoveUserEvent()
									EndEventCreation()
								end,
		},
		[UIEnums.CustomPopups.MaximumEventsReached] =
		{
			darken			= true,
			no_back			= true,
			colour_style	= "Main_Black",
			intensity		= 0.6,
			icon_name		= "groups",

			title_text_ID	= UIText.POP_TITLE_ALERT,
			message_text_ID	= UIText.POP_MAX_EVENTS_REACHED_MSG,
			
			next_text_ID	= UIText.INFO_A_OK,
		},
		[UIEnums.CustomPopups.NoUserGroups] =
		{
			darken			= true,
			no_back			= true,
			colour_style	= "Main_Black",
			intensity		= 0.6,
			icon_name		= "groups",

			title_text_ID	= UIText.POP_NO_USER_GROUPS,
			message_text_ID	= UIText.POP_NO_USER_GROUPS_MSG,
			
			next_text_ID	= UIText.INFO_A_OK,
		},
		[UIEnums.CustomPopups.BuyUpgrade] =
		{
			darken			= true,
			
			title_text_ID	= UIText.POP_TITLE_INFO,
			message_text_ID	= UIText.RBU_SHOP_BUY_UPGRADE_POPUP_TEXT,
			
			next_text_ID	= UIText.INFO_YES_A,
			back_text_ID	= UIText.INFO_NO_B,
			
			next_CB			= function()
								Shop_BuyUpgrade()
							  end,
		},
		[UIEnums.CustomPopups.MultiplayerNoEvents] =
		{
			darken			= true,
			no_back			= true,
			colour_style	= "Main_Black",
			intensity		= 0.6,
			icon_name		= "groups",

			title_text_ID	= UIText.POP_NO_EVENT,
			message_text_ID	= UIText.POP_NO_EVENT_MSG,
			
			next_text_ID	= UIText.INFO_A_OK,
		},
		[UIEnums.CustomPopups.MultiplayerLanSearching] =
		{
			darken			= true,
			no_next			= true,
			no_back			= true,
			colour_style	= "Main_Black",
			intensity		= 0.6,
			icon_name		= "groups",

			title_text_ID	= UIText.POP_TITLE_PLEASE_WAIT,
			message_text_ID	= UIText.POP_SEARCHING_FOR_GAMES,
			
			init_CB			=	function()
									NetRace.EnumerateLanGames( true )
									PopupGUI.timer = 0
								end,

			update_CB		=	function(time)
									PopupGUI.timer = PopupGUI.timer + time

									if NetRace.IsEnumeratingLanGames() == true then
										return
									end

									-- check we have a result.
									if PopupGUI.timer > 1 then
										if NetRace.NumLanServersEnumerated() == 0 then
											PopupSpawn( UIEnums.CustomPopups.NoLanGames )
										else
											PopupCancel( "Multiplayer\\Events\\MpLobbyList.lua", true )
										end
									end
								end,
		},		
		[UIEnums.CustomPopups.MultiplayerE3Searching] =
		{
			darken			= true,
			no_next			= true,
			no_back			= true,
			colour_style	= "Main_Black",
			intensity		= 0.6,
			icon_name		= "groups",

			title_text_ID	= UIText.POP_TITLE_PLEASE_WAIT,
			message_text_ID	= UIText.POP_SEARCHING_FOR_GAMES,
		},
		[UIEnums.CustomPopups.ProfilePreLoad] =
		{
			darken			= true,
			title_text_ID	= UIText.POP_PROFILE_PRELOAD_TITLE,
			message_text_ID	= UIText.POP_PROFILE_PRELOAD_MSG,
			
			next_text_ID	= UIText.INFO_A_OK,
			no_back			= true,
		},
		[UIEnums.CustomPopups.ProfileKickedYou] =
		{
			darken			= true,
			title_text_ID	= UIText.POP_PROFILE_KICKED_TITLE,
			message_text_ID	= UIText.POP_PROFILE_KICKED_MSG,
			
			next_text_ID	= UIText.INFO_A_OK,
			no_back			= true,
		},
		[UIEnums.CustomPopups.MessagesAlert] =
		{
			darken			= true,
			title_text_ID	= UIText.IDS_CMN_NOWT,
			message_text_ID	= UIText.IDS_CMN_NOWT,
			
			next_text_ID	= UIText.INFO_A_OK,
			no_back			= true,
			
			init_CB			= function()
								CustomPopup.CurrentData.title_text_ID = UIGlobals.MessagesAlert.title_id
								CustomPopup.CurrentData.message_text_ID = UIGlobals.MessagesAlert.message_id
							end,
		},
		[UIEnums.CustomPopups.OptionsChangeStorageDevice] =
		{
			title_text_ID	= UIText.POP_TITLE_WARNING,
			message_text_ID	= UIText.POP_STORAGE_MSG,
			
			next_text_ID	= UIText.INFO_YES_A,
			back_text_ID	= UIText.INFO_NO_B,
			
			next_CB			= function()
								-- store the new device id
								Profile.StoreDeviceId( Profile.GetPrimaryPad() )
								-- reset save flag
								UIGlobals.DoNotSave[ Profile.GetPrimaryPad() ] = false
							end,			
		},
		[UIEnums.CustomPopups.GroupsCarDoNotHaveOne] =
		{
			darken			= true,
			title_text_ID	= UIText.POP_GROUPS_NO_CAR_TITLE,
			message_text_ID	= UIText.POP_GROUPS_NO_CAR_MSG,
			
			next_text_ID	= UIText.INFO_A_OK,
			no_back			= true,
		},
		[UIEnums.CustomPopups.ContentServerData] =
		{
			show_progress	= true,

			darken			= true,
			no_back			= true,
			no_next			= true,
			colour_style	= "Main_Black",
			intensity		= 0.6,
			
			title_text_ID	= UIText.POP_TITLE_PLEASE_WAIT,
			message_text_ID	= "GAME_CONTENT_SERVER_INFO",
		},
		[UIEnums.CustomPopups.SplitscreenResetScores] =
		{
			darken			= true,
			colour_style	= "Main_Black",
			intensity		= 0.6,
			icon_name		= "groups",
			
			next_text_ID	= UIText.INFO_YES_A,
			back_text_ID	= UIText.INFO_NO_B,
			
			title_text_ID	= UIText.POP_TITLE_WARNING,
			message_text_ID	= UIText.POP_SS_RESET_SCORES_MSG,
			
			next_CB			= function()
								Multiplayer.SplitscreenResetScores()
							end,
		},
		[UIEnums.CustomPopups.ServiceConnectionLost] = 
		{
			darken			= true,
			title_text_ID	= UIText.POP_TITLE_ALERT,
			message_text_ID	= UIText.POP_DISCONNECTED_FROM_LIVE,
			
			next_text_ID	= UIText.INFO_A_OK,
			no_back			= true,

			init_CB			= function()
								if UIEnums.CurrentPlatform == UIEnums.Platform.Xenon then
									CustomPopup.CurrentData.message_text_ID = UIText.POP_DISCONNECTED_FROM_LIVE
								elseif UIEnums.CurrentPlatform == UIEnums.Platform.PC then
									CustomPopup.CurrentData.message_text_ID = UIText.POP_DISCONNECTED_FROM_INTERNET
								elseif UIEnums.CurrentPlatform == UIEnums.Platform.PS3 then
									CustomPopup.CurrentData.message_text_ID = UIText.POP_DISCONNECTED_FROM_PSN	
								end
							  end,
		},
		[UIEnums.CustomPopups.GarageBuyVehicle] =
		{
			darken			= true,
			
			next_text_ID	= UIText.INFO_YES_A,
			back_text_ID	= UIText.INFO_NO_B,
			
			title_text_ID	= UIText.POP_TITLE_INFO,
			message_text_ID	= UIText.POP_GARAGE_BUY_MSG,
			
			next_CB			= function()
								Garage_BuyCar()
							end,
		},
		[UIEnums.CustomPopups.GarageMiniBuyVehicle] =
		{
			darken			= true,
			
			next_text_ID	= UIText.INFO_YES_A,
			back_text_ID	= UIText.INFO_NO_B,
			
			title_text_ID	= UIText.POP_TITLE_INFO,
			message_text_ID	= UIText.POP_GARAGE_BUY_MSG,
			
			next_CB			= function()
								GarageMini_BuyCar()
							end,
		},
		[UIEnums.CustomPopups.UploadUserEvent] = 
		{
			darken			= true,
			title_text_ID	= UIText.POP_TITLE_INFO,
			message_text_ID	= UIText.POP_UPLOADING_EVENT,
			colour_style	= "Main_Black",
			intensity		= 0.6,
			icon_name		= "groups",

			no_next			= true,
			no_back			= true,
			show_progress	= true,

			init_CB			= function()
								PopupGUI.timer = 0
								PopupGUI.state = UIEnums.LspState.Ready

								LSP.Enable( true )
								LSP.SetFileName( "Event.bin" )
								LSP.SetFileSlot( 6 + EventCreator.GetCurrentEventSlot() )
								LSP.SetUserIndex( Profile.GetPrimaryPad() )
							  end,

			update_CB		= function(time)
								PopupGUI.timer = PopupGUI.timer + time	

								if PopupGUI.state == UIEnums.LspState.Ready then
									PopupGUI.state = UIEnums.LspState.Uploading
									LSP.UploadUserEvent()
								
								elseif PopupGUI.state == UIEnums.LspState.Uploading then
									local busy, error = LSP.PumpCurrentTask()

									if error == true then
										PopupGUI.state = UIEnums.LspState.Finished
										PopupSpawn( UIEnums.CustomPopups.ContentServerError )

									elseif busy == false then
										LSP.FinishUserEventUpload()
										PopupGUI.state = UIEnums.LspState.Finished
									end

								elseif PopupGUI.state == UIEnums.LspState.Finished then
									if PopupGUI.timer > 2 then
										Back()

										EventCreator.CreateSocialCommentary(EventCreator.GetCurrentEventSlot())

										StartAsyncSave()
										PopupCancel()
									end
								end
							  end
		},
		[UIEnums.CustomPopups.UploadUserGroup] = 
		{
			darken			= true,
			title_text_ID	= UIText.POP_TITLE_INFO,
			message_text_ID	= UIText.POP_UPLOADING_GROUP,
			colour_style	= "Main_Black",
			intensity		= 0.6,
			icon_name		= "groups",

			no_next			= true,
			no_back			= true,
			show_progress	= true,

			init_CB			= function()
								PopupGUI.timer = 0
								PopupGUI.state = UIEnums.LspState.Ready

								LSP.Enable( true )
								LSP.SetFileName( "Group.bin" )
								LSP.SetFileSlot( 5 + GroupCreator.GetCurrentGroupSlot() )
								LSP.SetUserIndex( Profile.GetPrimaryPad() )
							  end,

			update_CB		= function(time)
								PopupGUI.timer = PopupGUI.timer + time	

								if PopupGUI.state == UIEnums.LspState.Ready then
									PopupGUI.state = UIEnums.LspState.Uploading
									LSP.UploadUserGroup()
								
								elseif PopupGUI.state == UIEnums.LspState.Uploading then
									local busy, error = LSP.PumpCurrentTask()

									if error == true then
										PopupGUI.state = UIEnums.LspState.Finished
										PopupSpawn( UIEnums.CustomPopups.ContentServerError )

									elseif busy == false then
										LSP.FinishUserGroupUpload()
										PopupGUI.state = UIEnums.LspState.Finished
									end

								elseif PopupGUI.state == UIEnums.LspState.Finished then
									if PopupGUI.timer > 2 then
										Back()
										StartAsyncSave()
										PopupCancel()
									end
								end
							  end
		},
		[UIEnums.CustomPopups.UserEventSearch] = 
		{
			darken			= true,
			title_text_ID	= UIText.POP_TITLE_INFO,
			message_text_ID	= UIText.POP_SEARCHING_EVENTS,
			colour_style	= "Main_Black",
			intensity		= 0.6,
			icon_name		= "groups",

			no_next			= true,
			no_back			= true,
			show_progress	= true,

			init_CB			= function()
								PopupGUI.timer = 0
								PopupGUI.state = UIEnums.LspState.Ready

								LSP.Enable(true)
								LSP.SetUserIndex(Profile.GetPrimaryPad())

								LSP.ClearTags()
								LSP.AddTag( 0, UIEnums.ContentServerType.Event )
							  end,

			update_CB		= function(time)
								PopupGUI.timer = PopupGUI.timer + time	

								if PopupGUI.state == UIEnums.LspState.Ready then
									PopupGUI.state = UIEnums.LspState.Searching
									LSP.SearchEvents( UIGlobals.Multiplayer.EventFilter )
								
								elseif PopupGUI.state == UIEnums.LspState.Searching then
									local busy, error = LSP.PumpCurrentTask()

									if error == true then
										PopupGUI.state = UIEnums.LspState.Finished
										PopupSpawn( UIEnums.CustomPopups.ContentServerError )
			
									elseif busy == false then
										show_table( LSP.GetReturnedSearchData(), 1 )
										PopupGUI.state = UIEnums.LspState.Finished
									end

								elseif PopupGUI.state == UIEnums.LspState.Finished then
									if PopupGUI.timer > 2 then
										if LSP.GetNumSearchEventsResult( UIGlobals.Multiplayer.EventFilter ) > 0 then
											PopupSpawn( UIEnums.CustomPopups.DownloadUserEvents )
										else
											PopupSpawn( UIEnums.CustomPopups.ContentServerNoEventsFound )
										end
									end
								end
							  end
		},
		[UIEnums.CustomPopups.DownloadUserEvents] = 
		{
			darken			= true,
			title_text_ID	= UIText.POP_TITLE_INFO,
			message_text_ID	= UIText.POP_DOWNLOADING_EVENTS,
			colour_style	= "Main_Black",
			intensity		= 0.6,
			icon_name		= "groups",

			no_next			= true,
			no_back			= true,
			show_progress	= true,

			init_CB			= function()
								PopupGUI.timer = 0
								PopupGUI.state = UIEnums.LspState.Ready

								LSP.Enable(true)
								LSP.SetUserIndex(Profile.GetPrimaryPad())
							  end,

			update_CB		= function(time)
								PopupGUI.timer = PopupGUI.timer + time	

								if PopupGUI.state == UIEnums.LspState.Ready then
									PopupGUI.state = UIEnums.LspState.MetaData
									LSP.GetEventMetaData( UIGlobals.Multiplayer.EventFilter )
								
								elseif PopupGUI.state == UIEnums.LspState.MetaData then
									local busy, error = LSP.PumpCurrentTask()

									if error == true then
										PopupGUI.state = UIEnums.LspState.Finished
										PopupSpawn( UIEnums.CustomPopups.ContentServerError )
			
									elseif busy == false then
										PopupGUI.state = UIEnums.LspState.Downloading
										LSP.DownloadUserEvents()
									end

								elseif PopupGUI.state == UIEnums.LspState.Downloading then
									local busy, error = LSP.PumpCurrentTask()
									if error == true then
										PopupSpawn( UIEnums.CustomPopups.ContentServerError )

									elseif busy == false then
										PopupGUI.state = UIEnums.LspState.Finished
									end

								elseif PopupGUI.state == UIEnums.LspState.Finished then
									if PopupGUI.timer > 2 then
										if LSP.GetNumDownloadedFiles() > 0 then
											PopupCancel( "Multiplayer\\Events\\MpEventsList.lua", true )
										else
											print( "LSP download ", LSP.GetNumDownloadedFiles() )
											PopupSpawn( UIEnums.CustomPopups.ContentServerError )
										end
									end
								end
							  end
		},
		[UIEnums.CustomPopups.ContentServerError] =
		{
			darken			= true,
			no_back			= true,
			colour_style	= "Main_Black",
			intensity		= 0.6,
			icon_name		= "groups",
			
			next_text_ID	= UIText.INFO_A_OK,
			
			title_text_ID	= UIText.POP_TITLE_ERROR,
			message_text_ID	= UIText.POP_CONTENT_SERVER_ERROR,
		},
		[UIEnums.CustomPopups.ContentServerCantUpload] =
		{
			darken			= true,
			no_back			= true,
			colour_style	= "Main_Black",
			intensity		= 0.6,
			
			next_text_ID	= UIText.INFO_A_OK,
			
			title_text_ID	= UIText.POP_TITLE_ERROR,
			message_text_ID	= UIText.POP_CONTENT_SERVER_CANT_UPLOAD,
		},
		[UIEnums.CustomPopups.ContentServerNoEventsFound] =
		{
			darken			= true,
			no_back			= true,
			colour_style	= "Main_Black",
			intensity		= 0.6,
			icon_name		= "groups",
			
			next_text_ID	= UIText.INFO_A_OK,
			
			title_text_ID	= UIText.POP_TITLE_ERROR,
			message_text_ID	= UIText.POP_CONTENT_SERVER_NO_EVENTS_FOUND,
		},
		[UIEnums.CustomPopups.MultiplayerRateItem] =
		{
			darken			= true,
			colour_style	= "Main_Black",
			intensity		= 0.6,
			icon_name		= "groups",
			
			next_text_ID	= UIText.INFO_SUBMIT_A,
			back_text_ID	= UIText.INFO_IGNORE_X,
			x_text_ID		= UIText.CMN_NOWT,
			
			title_text_ID	= "MPL_EVENT_NAME0",
			message_text_ID	= UIText.POP_RATE_GROUP,

			init_CB			= function()
								PopupGUI.stars = {}
								PopupGUI.rating = 3
								for i = 1, 5 do
									PopupGUI.stars[i] = SCUI_Popup.name_to_id["star_" .. i ]
									UIButtons.SetActive( PopupGUI.stars[i], true )
									
									if i > 3 then
										UIButtons.ChangeColour( PopupGUI.stars[i], "Main_Black" )
									end
								end

								if UIGlobals.Multiplayer.RateEvent == true then
									UIGlobals.Multiplayer.RateEvent = false
								end

								-- can we add as favourite?
								if Multiplayer.IsFavouriteEvent( 0 ) == false then
									CustomPopup.CurrentData.x_text_ID = UIText.INFO_TAG_FAVOURITE_EVENT
								else
									CustomPopup.CurrentData.x_text_ID = nil
								end
							  end,

			message_CB		= function(message,arg1,arg2)
								if message == UIEnums.Message.ButtonLeft or message == UIEnums.Message.ButtonDown then
									if PopupGUI.rating > 0 then
										UIButtons.ChangeColour( PopupGUI.stars[PopupGUI.rating], "Main_Black" )
										PopupGUI.rating = PopupGUI.rating - 1
									end

								elseif message == UIEnums.Message.ButtonRight or message == UIEnums.Message.ButtonUp then
									if PopupGUI.rating < 5 then
										PopupGUI.rating = PopupGUI.rating + 1
										UIButtons.ChangeColour( PopupGUI.stars[PopupGUI.rating], "Support_4" )
									end
							
								elseif message == UIEnums.Message.ButtonX then
									if Multiplayer.IsFavouriteEvent( 0 ) == false then
										PlaySfxNext()

										-- just incase we 
										if Multiplayer.AddFavouriteEvent( 0 ) == false then
											PopupSpawn( UIEnums.CustomPopups.MaxFavourties )
										end

										-- need to resize x to fix the gimpness
										local is_english = ( UIGlobals.CurrentLanguage == UIEnums.Language.English )
										
										if is_english == false then
											UIButtons.ChangeSize( SCUI_Popup.name_to_id[ "info_1_o" ], 0, 0, 0 )
										else
											UIButtons.ChangeSize( SCUI_Popup.name_to_id[ "info_1" ], 0, 0, 0 )
										end
									end
								end
							  end,

			next_CB			= function()
								print( "GROUP RATING", PopupGUI.rating )
								LSP.VoteUserEvent( PopupGUI.rating )	

								PopupSpawn( UIEnums.CustomPopups.SubmittingEventRating )
							  end,
		},
		[UIEnums.CustomPopups.NoUserEvents] =
		{
			darken			= true,
			no_back			= true,
			colour_style	= "Main_Black",
			intensity		= 0.6,
			icon_name		= "groups",
			
			next_text_ID	= UIText.INFO_A_OK,
			
			title_text_ID	= UIText.POP_TITLE_PLEASE_WAIT,
			message_text_ID	= UIText.POP_SUBMITTING_EVENT_RATING,
		},
		[UIEnums.CustomPopups.NoUserEvents] =
		{
			darken			= true,
			no_back			= true,
			colour_style	= "Main_Black",
			intensity		= 0.6,
			icon_name		= "groups",
			
			next_text_ID	= UIText.INFO_A_OK,
			
			title_text_ID	= UIText.POP_TITLE_ERROR,
			message_text_ID	= UIText.POP_NO_USER_EVENTS,
		},
		[UIEnums.CustomPopups.DeleteEvent] =
		{
			darken			= true,
			colour_style	= "Main_Black",
			intensity		= 0.6,
			icon_name		= "groups",

			title_text_ID	= UIText.POP_ALERT,
			message_text_ID	= UIText.POP_DELETE_EVENT_MSG,
			
			next_text_ID	= UIText.INFO_YES_A,
			back_text_ID	= UIText.INFO_NO_B,
			
			next_CB			=	function()					
									RemoveUserEvent()
									StartAsyncSave()
								end,
		},
		[UIEnums.CustomPopups.NoFavouriteEvents] =
		{
			darken			= true,
			colour_style	= "Main_Black",
			intensity		= 0.6,
			icon_name		= "groups",
			no_back			= true,
			title_text_ID	= UIText.POP_ERROR,
			message_text_ID	= UIText.POP_NO_FAVOURITE_EVENTS,
			next_text_ID	= UIText.INFO_A_OK,
		},
		[UIEnums.CustomPopups.NoRecentEvents] =
		{
			darken			= true,
			colour_style	= "Main_Black",
			intensity		= 0.6,
			icon_name		= "groups",
			no_back			= true,
			title_text_ID	= UIText.POP_ERROR,
			message_text_ID	= UIText.POP_NO_RECENT_EVENTS,
			next_text_ID	= UIText.INFO_A_OK,
		},
		[UIEnums.CustomPopups.VerifyEventName] =
		{
			darken			= true,
			colour_style	= "Main_Black",
			intensity		= 0.6,
			icon_name		= "groups",
			no_back			= true,
			no_next			= true,
			show_progress	= true,
			title_text_ID	= UIText.POP_TITLE_PLEASE_WAIT,
			message_text_ID	= UIText.POP_VERIFYING_EVENT_NAME,

			init_CB			= function()
								PopupGUI.timer = 0
								PopupGUI.state = UIEnums.PopupState.Ready
							  end,

			update_CB		= function(time)
								PopupGUI.timer = PopupGUI.timer + time	

								if PopupGUI.state == UIEnums.PopupState.Ready then
									PopupGUI.state = UIEnums.PopupState.Busy
									EventCreator.VerifyEventName()
									
								elseif PopupGUI.state == UIEnums.PopupState.Busy then
									local busy, valid = EventCreator.PollVerifyString()
									if busy == false then
										if valid == false then
											PopupSpawn( UIEnums.CustomPopups.InvalidEventName )
										else
											PopupSpawn( UIEnums.CustomPopups.VerifyEventDesc )
										end
									end

								elseif PopupGUI.state == UIEnums.PopupState.Finished then
								end
							  end,
		},
  		[UIEnums.CustomPopups.VerifyEventDesc] =
		{
			darken			= true,
			colour_style	= "Main_Black",
			intensity		= 0.6,
			icon_name		= "groups",
			no_back			= true,
			no_next			= true,
			show_progress	= true,
			title_text_ID	= UIText.POP_TITLE_PLEASE_WAIT,
			message_text_ID	= UIText.POP_VERIFYING_EVENT_DESC,

			init_CB			= function()
								PopupGUI.timer = 0
								PopupGUI.state = UIEnums.PopupState.Ready
							  end,

			update_CB		= function(time)
								PopupGUI.timer = PopupGUI.timer + time	

								if PopupGUI.state == UIEnums.PopupState.Ready then
									PopupGUI.state = UIEnums.PopupState.Busy
									EventCreator.VerifyEventDesc()
									
								elseif PopupGUI.state == UIEnums.PopupState.Busy then
									local busy, valid = EventCreator.PollVerifyString()
									if busy == false then
										if valid == false then
											PopupSpawn( UIEnums.CustomPopups.InvalidEventDesc )
										else
											PopupSpawn( UIEnums.CustomPopups.UploadUserEvent )
										end
									end

								elseif PopupGUI.state == UIEnums.PopupState.Finished then
								end
							  end,
		},
		[UIEnums.CustomPopups.InvalidEventName] =
		{
			darken			= true,
			colour_style	= "Main_Black",
			intensity		= 0.6,
			icon_name		= "groups",
			no_back			= true,
			title_text_ID	= UIText.POP_TITLE_ALERT,
			message_text_ID	= UIText.POP_INVALID_EVENT_NAME,
			next_text_ID	= UIText.INFO_A_OK,
		},
		[UIEnums.CustomPopups.InvalidEventDesc] =
		{
			darken			= true,
			colour_style	= "Main_Black",
			intensity		= 0.6,
			icon_name		= "groups",
			no_back			= true,
			title_text_ID	= UIText.POP_TITLE_ALERT,
			message_text_ID	= UIText.POP_INVALID_EVENT_DESC,
			next_text_ID	= UIText.INFO_A_OK,
		},
		[UIEnums.CustomPopups.SubmittingEventRating] =
		{
			darken			= true,
			colour_style	= "Main_Black",
			intensity		= 0.6,
			icon_name		= "groups",
			no_back			= true,
			no_next			= true,
			show_progress	= true,
			title_text_ID	= UIText.POP_TITLE_PLEASE_WAIT,
			message_text_ID	= UIText.POP_SUBMITTING_EVENT_RATING,

			init_CB			= function()
								PopupGUI.timer = 0
								PopupGUI.state = UIEnums.PopupState.Ready
							  end,

			update_CB		= function(time)
								PopupGUI.timer = PopupGUI.timer + time	

								if PopupGUI.state == UIEnums.PopupState.Ready then
									PopupGUI.state = UIEnums.PopupState.Busy
									
								elseif PopupGUI.state == UIEnums.PopupState.Busy then
									local busy, error = LSP.CheckCurrentTask()
									
									if error == true then
										PopupSpawn( UIEnums.CustomPopups.ContentServerError )
									elseif busy == false then
										PopupGUI.state = UIEnums.PopupState.Finished
									end

		  						elseif PopupGUI.state == UIEnums.PopupState.Finished then
									if PopupGUI.timer > 2 then
										PopupSpawn( UIEnums.CustomPopups.EventRatingComplete )
									end
								end
							  end,

		},
		[UIEnums.CustomPopups.EventRatingComplete] =
		{
			darken			= true,
			colour_style	= "Main_Black",
			intensity		= 0.6,
			icon_name		= "groups",
			no_back			= true,
			title_text_ID	= UIText.POP_TITLE_INFO,
			message_text_ID	= UIText.POP_RATING_COMPLETE,
			next_text_ID	= UIText.INFO_A_OK,
		},

		[UIEnums.CustomPopups.TwitterUserUnknown] =
		{
			darken			= true,
			no_back			= true,
			
			title_text_ID	= UIText.POP_TITLE_TWITTER,
			message_text_ID	= UIText.POP_TWITTER_LOGIN_ERROR,
			
			next_text_ID	= UIText.INFO_A_OK,
			next_CB			= function()
								--UIGlobals.TwitterClose = true
							end,
		},
		[UIEnums.CustomPopups.SaveEventChanges] =
		{
			darken			= true,
			colour_style	= "Main_Black",
			intensity		= 0.6,
			icon_name		= "groups",

			title_text_ID	= UIText.POP_TITLE_ALERT,
			message_text_ID	= UIText.POP_SAVE_EVENT_MSG,
			
			next_text_ID	= UIText.INFO_YES_A,
			back_text_ID	= UIText.INFO_B_CANCEL,
			
			next_CB			=	function()
									SaveEvent()
								end,
								
			back_CB			=	function()
								end,
		},
		[UIEnums.CustomPopups.ConfirmRemoveRace] =
		{
			darken			= true,
			colour_style	= "Main_Black",
			intensity		= 0.6,
			icon_name		= "groups",

			title_text_ID	= UIText.POP_TITLE_ALERT,
			message_text_ID	= UIText.POP_REMOVE_EVENT_RACE,
			
			next_text_ID	= UIText.INFO_YES_A,
			back_text_ID	= UIText.INFO_NO_B,
			
			
			next_CB			=	function()	
									RemoveEventRace() -- called from script
								end,
								
			back_CB			=	function()
								end,
		},
		[UIEnums.CustomPopups.ContentServerGeneralError] =
		{
			darken			= true,
			no_back			= true,
			colour_style	= "Main_Black",
			intensity		= 0.6,
			
			next_text_ID	= UIText.INFO_A_OK,
			
			title_text_ID	= UIText.POP_TITLE_ERROR,
			message_text_ID	= UIText.POP_CONTENT_SERVER_GENERAL_ERROR,
			x_text_ID		= UIText.INFO_TRY_RECONNECT,

			init_CB			=	function()
									if net_CanReconnectToDemonware() == false then
										CustomPopup.CurrentData.x_text_ID = nil
										PopupGUI.ShowingX=false
									else
										CustomPopup.CurrentData.x_text_ID = UIText.INFO_TRY_RECONNECT
										PopupGUI.ShowingX=true
									end
								end,

			message_CB		=	function( message,arg1,arg2 )
									if message == UIEnums.Message.ButtonX then
										if net_CanReconnectToDemonware() == true then
											PlaySfxNext()
											net_StartServiceConnection(true, nil, true)
										end
									end
								end,

			update_CB		=	function(time)
									if net_CanReconnectToDemonware() == false then
										if PopupGUI.ShowingX == true then
											PopupSpawn(UIEnums.CustomPopups.ContentServerGeneralError)
										end
									else
										if PopupGUI.ShowingX == false then
											PopupSpawn(UIEnums.CustomPopups.ContentServerGeneralError)
										end
									end
								end,

		},
		[UIEnums.CustomPopups.ContentServerUploadBandwidthError] =
		{
			darken			= true,
			no_back			= true,
			colour_style	= "Main_Black",
			intensity		= 0.6,
			
			next_text_ID	= UIText.INFO_A_OK,
			
			title_text_ID	= UIText.POP_TITLE_ERROR,
			message_text_ID	= UIText.POP_CONTENT_SERVER_UPLOAD_BANDWIDTH_ERROR,
		},
		[UIEnums.CustomPopups.ContentServerDownloadBandwidthError] =
		{
			darken			= true,
			no_back			= true,
			colour_style	= "Main_Black",
			intensity		= 0.6,
			
			next_text_ID	= UIText.INFO_A_OK,
			
			title_text_ID	= UIText.POP_TITLE_ERROR,
			message_text_ID	= UIText.POP_CONTENT_SERVER_DOWNLOAD_BANDWIDTH_ERROR,
		},
		[UIEnums.CustomPopups.ThankyouForVoting] =
		{
			darken			= true,
			no_back			= true,
			colour_style	= "Main_Black",
			intensity		= 0.6,
			
			next_text_ID	= UIText.INFO_A_OK,
			
			title_text_ID	= UIText.POP_TITLE_INFO,
			message_text_ID	= UIText.POP_THANK_YOU_FOR_VOTING,

			next_CB			= function()
									UIGlobals.FinishedVoting=true
								end,
		},
		[UIEnums.CustomPopups.ContentServerNoResultsFromSearch] =
		{
			darken			= true,
			no_back			= true,
			colour_style	= "Main_Black",
			intensity		= 0.6,
			
			next_text_ID	= UIText.INFO_A_OK,
			
			title_text_ID	= UIText.POP_TITLE_INFO,
			message_text_ID	= UIText.POP_SEARCH_RETURNED_NO_RESULTS,

			next_CB			= function()
								PopupCancel( "Photos\\PhotoSearchParams.lua", true )
							end,
		},
		[UIEnums.CustomPopups.ContentServerNoUserPhotos] =
		{
			darken			= true,
			no_back			= true,
			colour_style	= "Main_Black",
			intensity		= 0.6,
			
			next_text_ID	= UIText.INFO_A_OK,
			
			title_text_ID	= UIText.POP_TITLE_INFO,
			message_text_ID	= UIText.POP_SEARCH_NO_USER_PHOTOS,

			next_CB			= function()
								PopupCancel( "Photos\\Photos.lua", true )
							end,
		},
		[UIEnums.CustomPopups.ContentServerLeaderboardError] =
		{
			darken			= true,
			no_back			= true,
			colour_style	= "Main_Black",
			intensity		= 0.6,
			
			next_text_ID	= UIText.INFO_A_OK,
			
			title_text_ID	= UIText.POP_TITLE_INFO,
			message_text_ID	= UIText.POP_SEARCH_LEADERBOARD_ERROR,

			next_CB			= function()
								PopupCancel( "Photos\\Photos.lua", true )
							end,
		},
		[UIEnums.CustomPopups.AlreadyVoted] =
		{
			darken			= true,
			no_back			= true,
			colour_style	= "Main_Black",
			intensity		= 0.6,
			
			next_text_ID	= UIText.INFO_A_OK,
			
			title_text_ID	= UIText.POP_TITLE_INFO,
			message_text_ID	= UIText.POP_ALREADY_VOTED,

			next_CB			= function()
								PopupCancel( "Photos\\ViewPhoto.lua", true )
							end,
		},	
		[UIEnums.CustomPopups.OverwritePhotoServerSlot] =
		{
			darken			= true,
			colour_style	= "Main_Black",
			intensity		= 0.6,

			title_text_ID	= UIText.POP_TITLE_ALERT,
			message_text_ID	= UIText.POP_OVERWRITE_SERVER_PHOTO_SLOT,
			
			next_text_ID	= UIText.INFO_YES_A,
			back_text_ID	= UIText.INFO_NO_B,
			
			next_CB			=	function()	
									local caro_GUI =  ContextTable[ UIGlobals.PhotoSlotContext ].GUI
									DoUpload(UIButtons.GetSelection( caro_GUI.node_list_id ), caro_GUI) -- called from script
								end,
		},
		[UIEnums.CustomPopups.RetryRace] =
		{
			darken			= true,
			
			title_text_ID	= UIText.POP_TITLE_WARNING,
			message_text_ID	= UIText.POP_RETRY_RACE,
			
			next_text_ID	= UIText.INFO_A_SELECT,
			back_text_ID	= UIText.INFO_B_BACK,

			options = 
			{
				{ value = UIEnums.PopupOptions.Yes,				name = UIText.POP_OPTION_YES },
				{ value = UIEnums.PopupOptions.No,				name = UIText.POP_OPTION_NO },
			},
		},
		[UIEnums.CustomPopups.TwitterPostError] =
		{
			darken			= true,
			
			title_text_ID	= UIText.POP_TITLE_TWITTER,
			message_text_ID	= UIText.POP_TWITTER_POST_ERROR,
			
			next_text_ID	= UIText.INFO_A_OK,
			x_text_ID		= UIText.INFO_X_AUTHENTICATE,

			message_CB		=	function( message,arg1,arg2 )
									if message == UIEnums.Message.ButtonX then
										PlaySfxNext()
										UIGlobals.TwitterBackToTweetScreen = true
										PopupCancel( "Shared\\TwitterAuthenticate.lua", nil, nil, UIEnums.Context.Blurb )
									end
								end,

			next_CB			= function()
									UIGlobals.TwitterBackToTweetScreen = false
									UIGlobals.TwitterClose = true
								end,
		},
		[UIEnums.CustomPopups.TwitterConnectionError] =
		{
			darken			= true,
			
			title_text_ID	= UIText.POP_TITLE_TWITTER,
			message_text_ID	= UIText.POP_TWITTER_CONNECTION_ERROR,
			
			next_text_ID	= UIText.INFO_A_OK,
			next_CB			= function()
								UIGlobals.TwitterClose = true
							end,
		},
		[UIEnums.CustomPopups.MaxEventRaces] =
		{
			darken			= true,
			no_back			= true,
			colour_style	= "Main_Black",
			intensity		= 0.6,
			
			title_text_ID	= UIText.POP_TITLE_ALERT,
			message_text_ID	= UIText.POP_MAX_EVENT_RACES,
			
			next_text_ID	= UIText.INFO_A_OK,
		},
		[UIEnums.CustomPopups.TwitterPosted] =
		{
			darken			= true,
			
			title_text_ID	= UIText.POP_TITLE_TWITTER,
			message_text_ID	= UIText.POP_TWITTER_POSTED,
			
			next_text_ID	= UIText.INFO_A_OK,
			next_CB			= function()
								UIGlobals.TwitterClose = true
							end,
		},
		[UIEnums.CustomPopups.EventOptions] =
		{
			darken			= true,
			colour_style	= "Main_Black",
			intensity		= 0.6,
			icon_name		= "groups",
			
			title_text_ID	= UIText.POP_TITLE_INFO,
			message_text_ID	= UIText.POP_EVENT_OPTIONS,
			
			next_text_ID	= UIText.INFO_HOST_PRIVATE_EVENT,
			back_text_ID	= UIText.INFO_B_CANCEL,

			next_CB			=	function()
									HostGame( false )
								end,
		},
		[UIEnums.CustomPopups.NetConnectionTask] =
		{
			darken			= true,
			no_back			= true,
			no_next			= true,
			colour_style	= nil,
			icon_name		= nil,
			show_progress	= true,
			
			title_text_ID	= UIText.POP_TITLE_PLEASE_WAIT,
			message_text_ID	= UIText.CMN_NOWT,
			
			init_CB			=	function()
									print( "net connection start" )
									PopupGUI.profile_download_failed=false
									CustomPopup.CurrentData.darken = UIGlobals.NetConnectionDarken
																	
									UIGlobals.Network.AgeConnectionError = false
									
									PopupGUI.State = UIEnums.PopupState.Ready
									PopupGUI.Timer = 0

									if UIGlobals.server_connection.online_mode == true then
										CustomPopup.CurrentData.message_text_ID = UIText.RBC_SERVER_CONNECTION_ONLINE
									else
										CustomPopup.CurrentData.message_text_ID = UIText.RBC_SERVER_CONNECTION_OFFLINE
									end

									if IsMultiplayerMode() == true then
										print( "is multiplayer" )
										CustomPopup.CurrentData.colour_style	= "Main_Black"
										CustomPopup.CurrentData.intensity		= 0.6
										CustomPopup.CurrentData.icon_name		= "groups"
									else
										print( "is singleplayer" )
										CustomPopup.CurrentData.colour_style	= "Main_2"
										CustomPopup.CurrentData.icon_name		= "bio"
									end
								end,

			update_CB		=	function( time )
									if PopupGUI.State == UIEnums.PopupState.Ready then
										-- get current connection status.
										local state = UIEnums.Network.Router.eConnectionStateNotActive
										local result = UIEnums.Network.Router.eConnectionResultNone
										local online = false
										state, result, online = NetServices.ConnectionStatus()

										-- [WillB] #36172 Not anymore because peer proxy causes offline and online incompatability
										-- we can keep the online connection active if we only want an offline connection, no point disconnecting.
										--if state == UIEnums.Network.Router.eConnectionStateComplete and online == true and UIGlobals.server_connection.online_mode == false then
										--	net_connection_CompleteConnect()
										--	PopupGUI.State = UIEnums.PopupState.Finished
										--else
											PopupGUI.State = UIEnums.PopupState.Busy
											net_FlushEverything( true ) -- clean networking but ignore current rich presence states

											if NetServices.Connect( UIGlobals.server_connection.online_mode ) == false then
												PopupGUI.State = UIEnums.PopupState.Failed
											end
										--end

									elseif PopupGUI.State == UIEnums.PopupState.Busy then
										-- get current connection status.
										local state = UIEnums.Network.Router.eConnectionStateNotActive
										local result = UIEnums.Network.Router.eConnectionResultNone
										local online = false
										state, result, online = NetServices.ConnectionStatus()	

										if state == UIEnums.Network.Router.eConnectionStateComplete then
											net_connection_CompleteConnect()
											
											PopupGUI.State = UIEnums.PopupState.ServerTime
											
										elseif state == UIEnums.Network.Router.eConnectionStateFailed or state == UIEnums.Network.Router.eConnectionStateNotActive then
											if result == UIEnums.Network.Router.eConnectionResultNoServiceProvider then
												
												if UIGlobals.server_connection.online_mode == true then
													net_CloseServiceConnection()
													UIGlobals.server_connection.active = false
													
													print( "net connection error" )
													PopupSpawn( UIEnums.CustomPopups.NetConnectionErrorNoOnline )
												else
													PopupGUI.State = UIEnums.PopupState.SilentExit
													PopupCancel()
													UIGlobals.server_connection.active = false

													-- callback for success
													print( "net connection success" )
													if UIGlobals.server_connection.callback ~= nil then
														UIGlobals.server_connection.callback( true )
													end
												end
												
											elseif result == UIEnums.Network.Router.eConnectionResultAgeError then
												UIGlobals.Network.AgeConnectionError = true
												PopupGUI.State = UIEnums.PopupState.Failed
												net_CloseServiceConnection()

											elseif result == UIEnums.Network.Router.eConnectionResultInvalidUserName then
												-- todo show correct message for pc
												PopupGUI.State = UIEnums.PopupState.Failed
											elseif result == UIEnums.Network.Router.eConnectionResultInvalidPassword then
												-- todo show correct message for pc
												PopupGUI.State = UIEnums.PopupState.Failed
											else
												PopupGUI.State = UIEnums.PopupState.Failed
											end
										end								
										
									elseif PopupGUI.State == UIEnums.PopupState.ServerTime then

										local finished = NetServices.StartServerTimeStampTask();
												
										if finished == true then
											PopupGUI.State = UIEnums.PopupState.Finished
											LSP.Enable(true);
											UIGlobals.DoPublisherCheck=true
										end

									elseif PopupGUI.State == UIEnums.PopupState.Failed then
										PopupGUI.Timer = PopupGUI.Timer + time

										if PopupGUI.Timer > 0 then
											LSP.Enable(false)
											net_CloseServiceConnection()
											UIGlobals.server_connection.active = false

											if NetServices.MpBetaExpired() == true then
												PopupGUI.Timer = 0
												MultiplayerBetaExpired( )
											elseif PopupGUI.profile_download_failed==true then
												PopupSpawn( UIEnums.CustomPopups.ProfileDownloadFailed )
											else
												print( "net connection error" )
												PopupSpawn( UIEnums.CustomPopups.NetConnectionError )
											end

											-- clear the game invite since we cant join a game without being connected
											NetServices.ClearGameInvite()
											net_GameInviteFailed()
										end

									elseif PopupGUI.State == UIEnums.PopupState.Finished then
										PopupGUI.Timer = PopupGUI.Timer + time

										if PopupGUI.Timer > 1.5 then -- this is needed so that the system link connection displays

											if UIGlobals.DoPublisherCheck == true then
												Amax.TwitterCheckPublisherFile()
												Amax.FacebookCheckPublisherFile()
												
												UIGlobals.DoPublisherCheck=nil
											end
		
											local twitter_check_done=Amax.TwitterPublisherFileCheckFinished()
											local facebook_check_done=Amax.FacebookPublisherFileCheckFinished()

											if UIGlobals.server_connection.online_mode == false or ( twitter_check_done == true and facebook_check_done == true ) then
												
												if UIGlobals.server_connection.online_mode == true then
													Amax.StartFriendEnumeration()
												end

												if NetServices.MpBetaExpired() == true then
													LSP.Enable(false)
													net_CloseServiceConnection()
													UIGlobals.server_connection.active = false
													PopupGUI.Timer = 0
													MultiplayerBetaExpired( )
												else
													net_EnableGlobalUpdate( true )

													PopupCancel()
													UIGlobals.server_connection.active = false

													-- callback for success
													print( "net connection success" )
													if UIGlobals.server_connection.callback ~= nil then
														UIGlobals.server_connection.callback( true )
													end
												end
											end
										end
									elseif PopupGUI.State == UIEnums.PopupState.SilentExit then
									end
								end,
		},
		[UIEnums.CustomPopups.NetConnectionError] =
		{
			darken			= true,
			no_back			= true,
			colour_style	= "Main_Black",
			intensity		= 0.6,
			icon_name		= "groups",
			
			title_text_ID	= UIText.POP_TITLE_ERROR,
			message_text_ID	= UIText.CMN_NOWT,
			
			next_text_ID	= UIText.INFO_A_OK,
			
			end_CB			=	function()
									-- [WillB] #37705 - Need to re-enable carousel after failure
									if IsFunction( EnableCarousel ) == true then
										EnableCarousel( true )
									end
								end,

			init_CB			=	function()
									if UIGlobals.server_connection.online_mode == true then
										if UIGlobals.Network.AgeConnectionError == false then
											-- "Unable to connect to the Blur game servers."
											CustomPopup.CurrentData.message_text_ID = UIText.RBC_SERVER_CONNECTION_FAILED 
											-- 24967
											if ( UIEnums.CurrentPlatform == UIEnums.Platform.PS3 ) and ( Profile.PadProfileOnline( Profile.GetPrimaryPad() ) == false ) then
												-- "You must be signed in to PlayStation�Network to access online features."
												CustomPopup.CurrentData.message_text_ID = UIText.RBC_SERVER_CONNECTION_FAILED_ONLINE_REQUIRED
											end
										else
											CustomPopup.CurrentData.message_text_ID = UIText.RBC_SERVER_CONNECTION_FAILED_AGE_CHECK
										end
									else
										CustomPopup.CurrentData.message_text_ID = UIText.RBC_SERVER_CONNECTION_FAILED
									end
								end,

			next_CB			=	function()
									-- callback for failed	
									if UIGlobals.server_connection.callback ~= nil then
										UIGlobals.server_connection.callback( false )
									end
								end,
		},	
		
		[UIEnums.CustomPopups.ProfileUploadFailed] =
		{
			darken			= true,
			no_back			= true,
			colour_style	= "Main_Black",
			intensity		= 0.6,
			icon_name		= "groups",
			
			title_text_ID	= UIText.POP_TITLE_ERROR,
			message_text_ID	= UIText.PRO_UPLOAD_FAILED,
			
			next_text_ID	= UIText.INFO_A_OK,
		},	
		
		[UIEnums.CustomPopups.ProfileDownloadFailed] =
		{
			darken			= true,
			no_back			= true,
			colour_style	= "Main_Black",
			intensity		= 0.6,
			icon_name		= "groups",
			
			title_text_ID	= UIText.POP_TITLE_ERROR,
			message_text_ID	= UIText.PRO_DOWNLOAD_FAILED,
			
			next_text_ID	= UIText.INFO_A_OK,
			
			next_CB			=	function()
									-- callback for failed	
									if UIGlobals.server_connection.callback ~= nil then
										UIGlobals.server_connection.callback( false )
									end
								end,			
		},	

		
		[UIEnums.CustomPopups.NetConnectionErrorNoOnline] =
		{
			darken			= true,
			no_back			= true,
			colour_style	= "Main_Black",
			intensity		= 0.6,
			icon_name		= "groups",
			
			title_text_ID	= UIText.POP_TITLE_ERROR,
			message_text_ID	= UIText.RBC_SERVER_CONNECTION_FAILED_ONLINE_REQUIRED,
			
			next_text_ID	= UIText.INFO_A_OK,

			init_CB			=	function()
								end,

			next_CB			=	function()
									-- callback for failed	
									if UIGlobals.server_connection.callback ~= nil then
										UIGlobals.server_connection.callback( false )
									end
								end,
		},	
		[UIEnums.CustomPopups.LeaderboardNoScore] =
		{
			darken			= true,
			colour_style	= "Main_Black",
			intensity		= 0.6,
			icon_name		= "groups",
			no_back			= true,
			title_text_ID	= UIText.POP_TITLE_INFO,
			message_text_ID	= UIText.POP_LEADERBOARDS_NO_SCORE,
			next_text_ID	= UIText.INFO_A_OK,
		},
		[UIEnums.CustomPopups.LeaderboardNoFriends] =
		{
			darken			= true,
			colour_style	= "Main_Black",
			intensity		= 0.6,
			icon_name		= "groups",
			no_back			= true,
			title_text_ID	= UIText.POP_TITLE_INFO,
			message_text_ID	= UIText.POP_LEADERBOARDS_NO_FRIENDS,
			next_text_ID	= UIText.INFO_A_OK,
		},
		[UIEnums.CustomPopups.ConfirmLegendMode] =
		{
			darken			= true,
			title_text_ID	= UIText.POP_TITLE_LEGEND,
			message_text_ID	= "GAME_LEGEND_RESET",

			next_text_ID	= UIText.INFO_A_SELECT,
			back_text_ID	= UIText.INFO_B_BACK,
			options			= 		
							{
								{ value = UIEnums.PopupOptions.Yes,		name = UIText.POP_OPTION_LEGEND },
								{ value = UIEnums.PopupOptions.No,		name = UIText.POP_OPTION_CANCEL },
							},
								
			end_CB			= function()
								if PopupGUI.send_arg3 == UIEnums.PopupOptions.Yes then
								    Multiplayer.LegendModeReset()
								    StartAsyncSave()
								    PopupCancel( "Multiplayer\\MpOnline.lua", true )
								end
							  end,
		},
		[UIEnums.CustomPopups.MaxFavourties] =
		{
			darken			= true,
			colour_style	= "Main_Black",
			intensity		= 0.6,
			icon_name		= "groups",
			title_text_ID	= UIText.POP_TITLE_ALERT,
			message_text_ID	= UIText.POP_MAXED_FAVOURITES_REACHED,
			next_text_ID	= UIText.INFO_A_OK,
		},
		[UIEnums.CustomPopups.TwitterNotAvailable] =
		{
			darken			= true,
			
			title_text_ID	= UIText.POP_TITLE_TWITTER,
			message_text_ID	= UIText.POP_TWITTER_NOT_AVAILABLE,
			
			next_text_ID	= UIText.INFO_A_OK,
			x_text_ID		= UIText.INFO_TRY_RECONNECT,

			init_CB			=	function()
									if net_CanReconnectToDemonware() == false then
										CustomPopup.CurrentData.x_text_ID = nil
									else
										CustomPopup.CurrentData.x_text_ID = UIText.INFO_TRY_RECONNECT
									end
									
									if UIEnums.CurrentPlatform == UIEnums.Platform.PS3 then
										CustomPopup.CurrentData.message_text_ID = UIText.POP_TWITTER_NOT_AVAILABLE_PSN	
									end
									
								end,

			message_CB		=	function( message,arg1,arg2 )
									if message == UIEnums.Message.ButtonX then
										if net_CanReconnectToDemonware() == true then
											PlaySfxNext()
											net_StartServiceConnection(true, nil, true)
										end
									end
								end,
		},
		[UIEnums.CustomPopups.PS3NoVoiceAllowed] =
		{
			darken			= true,
			no_back			= true,
			
			title_text_ID	= UIText.POP_TITLE_ALERT,
			message_text_ID	= UIText.POP_PS3_NO_VOICE_ALLOWED,
			
			next_text_ID	= UIText.INFO_A_OK,
			
			init_CB			= function()
								UIGlobals.Network.VoiceChatRestricted = false
							end,
			
			next_CB			= function()
								if IsFunction( CustomPopup.Callback ) == true then
									CustomPopup.Callback()
								end
							end,
		},
		[UIEnums.CustomPopups.EventNameOverwrite] =
		{
			darken			= true,
			colour_style	= "Main_Black",
			intensity		= 0.6,
			icon_name		= "groups",

			title_text_ID	= UIText.POP_TITLE_ALERT,
			message_text_ID	= UIText.POP_OVERWRITE_EVENT,
			
			next_text_ID	= UIText.INFO_YES_A,
			back_text_ID	= UIText.INFO_NO_B,
			
			next_CB			= function()
								PopupSpawn( UIEnums.CustomPopups.VerifyEventName )
							  end,
		},
		[UIEnums.CustomPopups.EventNoRaces] =
		{
			darken			= true,
			no_back			= true,
			colour_style	= "Main_Black",
			intensity		= 0.6,
			icon_name		= "groups",

			title_text_ID	= UIText.POP_TITLE_ALERT,
			message_text_ID	= UIText.POP_NO_EVENT_RACES,
			
			next_text_ID	= UIText.INFO_A_OK,
		},
		[UIEnums.CustomPopups.NoLocalPhotos] =
		{
			darken			= true,
			no_back			= true,
			colour_style	= "Main_Black",
			intensity		= 0.6,
			
			next_text_ID	= UIText.INFO_A_OK,
			
			title_text_ID	= UIText.POP_TITLE_INFO,
			message_text_ID	= UIText.POP_NO_LOCAL_PHOTOS,

			next_CB			= function()
								PopupCancel() -- "Photos\\Photos.lua", true )
							end,
		},
		[UIEnums.CustomPopups.FacebookUserUnknown] =
		{
			darken			= true,
			no_back			= true,
			
			title_text_ID	= UIText.POP_TITLE_FACEBOOK,
			message_text_ID	= UIText.POP_FACEBOOK_LOGIN_ERROR,
			
			next_text_ID	= UIText.INFO_A_OK,
			next_CB			= function()
								--UIGlobals.FacebookClose = true
							end,
		},
		
		[UIEnums.CustomPopups.FacebookPostError] =
		{
			darken			= true,
			no_back			= true,		
			
			title_text_ID	= UIText.POP_TITLE_FACEBOOK,
			message_text_ID	= UIText.POP_FACEBOOK_POST_ERROR,
			
			next_text_ID	= UIText.INFO_A_OK,
			x_text_ID		= UIText.INFO_X_AUTHENTICATE,

			message_CB		=	function( message,arg1,arg2 )
									if message == UIEnums.Message.ButtonX then
										PlaySfxNext()
										UIGlobals.FacebookBackToStoryScreen = true
										PopupCancel( "Shared\\FacebookAuthenticate.lua", nil, nil, UIEnums.Context.Blurb )
									end
								end,

			next_CB			= function()
									UIGlobals.FacebookBackToStoryScreen = false
									UIGlobals.FacebookClose = true
								end,
		},
		[UIEnums.CustomPopups.FacebookConnectionError] =
		{
			darken			= true,
			no_back			= true,	
			
			title_text_ID	= UIText.POP_TITLE_FACEBOOK,
			message_text_ID	= UIText.POP_FACEBOOK_CONNECTION_ERROR,
			
			next_text_ID	= UIText.INFO_A_OK,
			next_CB			= function()
								UIGlobals.FacebookClose = true
							end,
		},
		[UIEnums.CustomPopups.FacebookPosted] =
		{
			darken			= true,
			no_back			= true,	
			
			title_text_ID	= UIText.POP_TITLE_FACEBOOK,
			message_text_ID	= UIText.POP_FACEBOOK_POSTED,
			
			next_text_ID	= UIText.INFO_A_OK,
			next_CB			= function()
								UIGlobals.FacebookClose = true
							end,
		},
		[UIEnums.CustomPopups.FacebookNotAvailable] =
		{
			darken			= true,
			
			title_text_ID	= UIText.POP_TITLE_FACEBOOK,
			message_text_ID	= UIText.POP_FACEBOOK_NOT_AVAILABLE,
			
			next_text_ID	= UIText.INFO_A_OK,
			x_text_ID		= UIText.INFO_TRY_RECONNECT,

			init_CB			=	function()
									if net_CanReconnectToDemonware() == false then
										CustomPopup.CurrentData.x_text_ID = nil
									else
										CustomPopup.CurrentData.x_text_ID = UIText.INFO_TRY_RECONNECT
									end

									if UIEnums.CurrentPlatform == UIEnums.Platform.PS3 then
										CustomPopup.CurrentData.message_text_ID = UIText.POP_FACEBOOK_NOT_AVAILABLE_PSN	
									end
								end,

			message_CB		=	function( message,arg1,arg2 )
									if message == UIEnums.Message.ButtonX then
										if net_CanReconnectToDemonware() == true then
											PlaySfxNext()
											net_StartServiceConnection(true, nil, true)
										end
									end
								end,
		},
		[UIEnums.CustomPopups.EventNotUploaded] =
		{
			darken			= true,
			no_back			= true,
			colour_style	= "Main_Black",
			intensity		= 0.6,
			icon_name		= "groups",

			title_text_ID	= UIText.POP_TITLE_ALERT,
			message_text_ID	= UIText.POP_EVENT_NOT_UPLOADED,
			
			next_text_ID	= UIText.INFO_A_OK,
		},
		[UIEnums.CustomPopups.EventNoCarClasses] =
		{
			darken			= true,
			no_back			= true,
			colour_style	= "Main_Black",
			intensity		= 0.6,
			icon_name		= "groups",

			title_text_ID	= UIText.POP_TITLE_ALERT,
			message_text_ID	= UIText.POP_NO_CAR_CLASSES,
			
			next_text_ID	= UIText.INFO_A_OK,
		},
		[UIEnums.CustomPopups.EventSaveWarning] =
		{
			darken			= true,
			no_back			= true,
			colour_style	= "Main_Black",
			intensity		= 0.6,
			icon_name		= "groups",

			title_text_ID	= UIText.POP_TITLE_ALERT,
			message_text_ID	= UIText.POP_SAVE_EVENT,
			
			next_text_ID	= UIText.INFO_A_OK,
			back_text_ID	= UIText.INFO_B_CANCEL,

			next_CB			=	function()
									SaveEvent()
								end,
		},
		[UIEnums.CustomPopups.UploadPhotoToFacebook] =
		{
			darken			= true,
			show_progress	= true,
			
			no_next			= true,
			no_back			= true,

			title_text_ID	= UIText.POP_TITLE_FACEBOOK,
			message_text_ID	= UIText.POP_FACEBOOK_UPLOADING,

			init_CB			=	function()
									UIGlobals.FacebookPhotoUploadStage = 0
									UIGlobals.FacebookUploadingPhoto = true

									-- upload photo
									LSP.Enable( true )
									LSP.SetFileName( "Photo.jpg" )
									LSP.SetFileSlot( 0 )
									LSP.SetUserIndex( Profile.GetPrimaryPad() )
									
									local ok=LSP.UploadCurrentPhoto()
									if ok == false then
										PopupSpawn( UIEnums.CustomPopups.ContentServerCantUpload )
									end

								end,

			update_CB		= function(time)
			
								--uploading to the content server
								if UIGlobals.FacebookPhotoUploadStage == 0 then
								
									local still_pumping,error = LSP.PumpCurrentTask()

									if still_pumping == false then
										if error ~= 0 then
											if error == 2003 then
												PopupSpawn( UIEnums.CustomPopups.ContentServerUploadBandwidthError )
											else
												if Profile.PadProfileOnline( Profile.GetPrimaryPad() ) == false then
													PopupSpawn( UIEnums.CustomPopups.MultiplayerOnlineConnectionLost )
												else
													PopupSpawn( UIEnums.CustomPopups.ContentServerGeneralError )
												end
											end	
										else
											
											UIGlobals.FacebookPhotoUploadStage = 1
											Amax.UploadPhotoToFacebook()
										
										end								
									end
								
								else
				
									PopupGUI.timer = PopupGUI.timer + time
									
									local still_pumping, error = Amax.PumpFacebookCurrentTask()
									
									if PopupGUI.timer > 2 and still_pumping == false then

										if error == 0 then
											PopupSpawn( UIEnums.CustomPopups.FacebookPhotoUploadComplete )
										elseif error==3601 or error==3602 then
											UIGlobals.AuthenticateFromPhoto=true
											PopupSpawn( UIEnums.CustomPopups.FacebookPostError )
										elseif error == 3605 then
											PopupSpawn( UIEnums.CustomPopups.FacebookPhotoAlbumFull )
										else
											PopupSpawn( UIEnums.CustomPopups.ContentServerGeneralError )
										end

									end
								end
								
							  end,
		},
		[UIEnums.CustomPopups.FacebookPhotoUploadComplete] =
		{
			darken			= true,
			
			title_text_ID	= UIText.POP_TITLE_FACEBOOK,
			message_text_ID	= UIText.POP_FACEBOOK_PHOTO_UPLOADED,
			
			next_text_ID	= UIText.INFO_A_OK,
		},
		[UIEnums.CustomPopups.FacebookPhotoAlbumFull] =
		{
			darken			= true,
			
			title_text_ID	= UIText.POP_TITLE_FACEBOOK,
			message_text_ID	= UIText.POP_FACEBOOK_ALBUM_FULL,
			
			next_text_ID	= UIText.INFO_A_OK,
		},
		[UIEnums.CustomPopups.ConfirmModConfig] =
		{
			darken			= true,
			colour_style	= "Main_Black",
			intensity		= 0.6,
			icon_name		= "groups",

			title_text_ID	= UIText.POP_TITLE_ALERT,
			message_text_ID	= UIText.POP_SAVE_MOD_CHANGES,
			
			next_text_ID	= UIText.INFO_YES_A,
			back_text_ID	= UIText.INFO_B_CANCEL,

			next_CB			=	function()
									UIGlobals.ModShopFlag = UIEnums.ModShopFlags.Discard
								end,
		},
		[UIEnums.CustomPopups.DiscardEventSettings] =
		{
			darken			= true,
			colour_style	= "Main_Black",
			intensity		= 0.6,
			icon_name		= "groups",

			title_text_ID	= UIText.POP_TITLE_WARNING,
			message_text_ID	= UIText.POP_DISCARD_EVENT_SETTINGS,
			
			next_text_ID	= UIText.INFO_YES_A,
			back_text_ID	= UIText.INFO_B_CANCEL,
			
			next_CB			=	function()
									DiscardEventSettings( true )
								end,
		},
		[UIEnums.CustomPopups.DiscardEventPlaylist] =
		{
			darken			= true,
			colour_style	= "Main_Black",
			intensity		= 0.6,
			icon_name		= "groups",

			title_text_ID	= UIText.POP_TITLE_WARNING,
			message_text_ID	= UIText.POP_DISCARD_EVENT_PLAYLIST,
			
			next_text_ID	= UIText.INFO_YES_A,
			back_text_ID	= UIText.INFO_B_CANCEL,
			
			next_CB			=	function()
									DiscardEventPlaylist( true )
								end,
		},
		[UIEnums.CustomPopups.DiscardEventPlaylistRace] =
		{
			darken			= true,
			colour_style	= "Main_Black",
			intensity		= 0.6,
			icon_name		= "groups",

			title_text_ID	= UIText.POP_TITLE_WARNING,
			message_text_ID	= UIText.POP_DISCARD_EVENT_RACE,
			
			next_text_ID	= UIText.INFO_YES_A,
			back_text_ID	= UIText.INFO_B_CANCEL,
			
			next_CB			=	function()
									DiscardEventRace( true )
								end,
		},
		[UIEnums.CustomPopups.IdlePlayerKicked] =
		{
			darken			= true,
			title_text_ID	= UIText.POP_TITLE_WARNING,
			message_text_ID	= UIText.POP_IDLE_PLAYER_KICKED_MSG,
			
			next_text_ID	= UIText.INFO_A_OK,
			no_back			= true,
			colour_style	= "Main_Black",
			intensity		= 0.6,
			icon_name		= "groups",

			init_CB			=	function()
									UIGlobals.IdlePlayerKicked = false -- flag has serverd its purpose!
								end,
		},
		[UIEnums.CustomPopups.FacebookLegal] =
		{
			darken			= true,
			
			title_text_ID	= UIText.POP_TITLE_INFO,
			message_text_ID	= UIText.POP_FACEBOOK_LEGAL,
			
			next_text_ID	= UIText.INFO_A_ACCEPT,
			back_text_ID	= UIText.INFO_B_DECLINE,

								
			back_CB			=	function()
									Amax.FacebookUnAuthenticate()
									UIGlobals.FacebookClose = true
								end,
		},
		[UIEnums.CustomPopups.TwitterLegal] =
		{
			darken			= true,
			
			title_text_ID	= UIText.POP_TITLE_INFO,
			message_text_ID	= UIText.POP_TWITTER_LEGAL,
			
			next_text_ID	= UIText.INFO_A_ACCEPT,
			back_text_ID	= UIText.INFO_B_DECLINE,

								
			back_CB			=	function()
									Amax.TwitterUnAuthenticate()
									UIGlobals.TwitterClose = true
								end,
		},
		[UIEnums.CustomPopups.TwitterPost] =
		{
			darken			= true,
			
			title_text_ID	= UIText.POP_TITLE_INFO,
			message_text_ID	= UIText.POP_TWITTER_POST,
			
			next_text_ID	= UIText.INFO_YES_A,
			back_text_ID	= UIText.INFO_NO_B,

			next_CB			=	function()
									SendTweet()
									PopupSpawn( UIEnums.CustomPopups.TwitterPosting )
								end,
		},
		[UIEnums.CustomPopups.DeleteTwitterPost] =
		{
			darken			= true,
			
			title_text_ID	= UIText.POP_TITLE_INFO,
			message_text_ID	= UIText.POP_TWITTER_DELETE_POST,
			
			next_text_ID	= UIText.INFO_YES_A,
			back_text_ID	= UIText.INFO_NO_B,

			next_CB			=	function()
									DeleteTweet()
								end,
		},
		[UIEnums.CustomPopups.FacebookPost] =
		{
			darken			= true,
			
			title_text_ID	= UIText.POP_TITLE_INFO,
			message_text_ID	= UIText.POP_FACEBOOK_POST,
			
			next_text_ID	= UIText.INFO_YES_A,
			back_text_ID	= UIText.INFO_NO_B,

			next_CB			=	function()
									SendStory()
									UIGlobals.SendFacebookStory=true
									PopupSpawn( UIEnums.CustomPopups.FacebookPosting )
								end,
		},
		[UIEnums.CustomPopups.DeleteFacebookPost] =
		{
			darken			= true,
			
			title_text_ID	= UIText.POP_TITLE_INFO,
			message_text_ID	= UIText.POP_FACEBOOK_DELETE_POST,
			
			next_text_ID	= UIText.INFO_YES_A,
			back_text_ID	= UIText.INFO_NO_B,

			next_CB			=	function()
									DeleteStory()
								end,
		},
		[UIEnums.CustomPopups.TwitterPosting] =
		{
			show_progress	= true,

			darken			= true,
			no_back			= true,
			no_next			= true,
			
			title_text_ID	= UIText.POP_TITLE_PLEASE_WAIT,
			message_text_ID	= UIText.POP_SOCIAL_UPLOADING,
		},
		[UIEnums.CustomPopups.FacebookPosting] =
		{
			show_progress	= true,

			darken			= true,
			no_back			= true,
			no_next			= true,
			
			title_text_ID	= UIText.POP_TITLE_PLEASE_WAIT,
			message_text_ID	= UIText.POP_SOCIAL_UPLOADING,
		},
		[ UIEnums.CustomPopups.CheckForDLC ] =
		{
			silent			= ( UIEnums.CurrentPlatform == UIEnums.Platform.Xenon ),
			show_progress	= true,

			darken			= false,
			no_back			= true,
			no_next			= true,
			
			title_text_ID	= UIText.POP_TITLE_PLEASE_WAIT,
			message_text_ID	= UIText.POP_DLC_CHECKING,
			
			init_CB			= function()
								PopupGUI.timer = 0
								
								print( "check for DLC start" )
								GameProfile.StartDLC()
							end,
							
			update_CB		= function( time )
								PopupGUI.timer = PopupGUI.timer + time
								
								local finished, error = GameProfile.ContinueDLC()
								
								if finished == true and PopupGUI.timer >= 0 then
									if error == true then
										print( "check for DLC error" )
										PopupSpawn( UIEnums.CustomPopups.ErrorWithDLC )
									else
										print( "check for DLC finished" )
										PopupCancel()
										
										if IsFunction( UIGlobals.CallbackDLC ) == true then
											UIGlobals.CallbackDLC( true )
										end
									end
								end
							end,
		},
		[ UIEnums.CustomPopups.ErrorWithDLC ] =
		{
			darken			= true,
			no_back			= true,
			
			next_text_ID	= UIText.INFO_A_OK,
			
			title_text_ID	= UIText.POP_TITLE_ERROR,
			message_text_ID	= UIText.POP_DLC_ERROR,
			
			next_CB			= function()
								if IsFunction( UIGlobals.CallbackDLC ) == true then
									UIGlobals.CallbackDLC( false )
								end
							end,
		},

		[UIEnums.CustomPopups.SaveNewRace] =
		{
			darken			= true,
			colour_style	= "Main_Black",
			intensity		= 0.6,
			icon_name		= "groups",

			title_text_ID	= UIText.POP_TITLE_WARNING,
			message_text_ID	= UIText.POP_SAVE_NEW_RACE_MSG,
			
			next_text_ID	= UIText.INFO_YES_A,
			back_text_ID	= UIText.INFO_B_CANCEL,
			
			
			next_CB			=	function()
									SaveEvent()
								end,
		},
		[UIEnums.CustomPopups.DiscardEventChanges] =
		{
			darken			= true,
			colour_style	= "Main_Black",
			intensity		= 0.6,
			icon_name		= "groups",

			title_text_ID	= UIText.POP_TITLE_WARNING,
			message_text_ID	= UIText.POP_DISCARD_EVENT_CHANGES,
			
			next_text_ID	= UIText.INFO_YES_A,
			back_text_ID	= UIText.INFO_B_CANCEL,
			
			
			next_CB			=	function()
									RestoreUserEvent()
								end,
		},
		[UIEnums.CustomPopups.ModShopOptions] =
		{
			darken			= true,
			colour_style	= "Main_Black",
			intensity		= 0.6,
			icon_name		= "groups",

			title_text_ID	= UIText.POP_TITLE_OPTIONS,
			next_text_ID	= UIText.INFO_A_SELECT,
			back_text_ID	= UIText.INFO_B_BACK,
			
			init_CB			=	function()
									CustomPopup.CurrentData.options	= 
									{
										{ value = UIEnums.ModShopOptions.Use,		name = UIText.POP_OPTION_USE_LOADOUT, locked = not UIGlobals.Multiplayer.CanEquipMods },
										{ value = UIEnums.ModShopOptions.Edit,		name = UIText.POP_OPTION_EDIT_LOADOUT },
									}
									
									if UIGlobals.Multiplayer.InLobby == false or Amax.IsGameModeRanked() == true then									
										CustomPopup.CurrentData.options[#CustomPopup.CurrentData.options + 1] = 
										{ 
											value = UIEnums.ModShopOptions.Rename,
											name = UIText.POP_OPTION_RENAME_LOADOUT 
										}
									end
								end,
		},
		[UIEnums.CustomPopups.MatchMakingOptions] =
		{
			darken			= true,
			colour_style	= "Main_Black",
			intensity		= 0.6,
			icon_name		= "groups",

			title_text_ID	= UIText.POP_TITLE_MATCHMAKING_OPTIONS,
			message_text_ID	= UIText.CMN_NOWT,
			
			next_text_ID	= UIText.INFO_A_SELECT,
			back_text_ID	= UIText.INFO_B_BACK,

			init_CB			=	function()
									local menu_id = SCUI_Popup.name_to_id[ "options" ]

									-- change frame size to fit items
									CustomPopup.CurrentData.size = { x = 256, y = 64, z = 0 }

									-- region
									local region_node_id = UIButtons.CloneXtGadgetByName( "TheCustomPopup.lua", "menu_node" )
									UIButtons.ChangeSize( region_node_id, 0, 32 ,0 )
									UIButtons.ChangeSize( UIButtons.FindChildByName( region_node_id, "frame" ), 232, 32 ,0 )
									UIButtons.ChangeSize( UIButtons.FindChildByName( region_node_id, "frame_fill" ), 232, 32 ,0 )
									UIButtons.ChangeSize( UIButtons.FindChildByName( region_node_id, "selector" ), 232, 32 ,0 )
									UIButtons.ChangeSize( UIButtons.FindChildByName( region_node_id, "selector_fill" ), 232, 32 ,0 )
									UIButtons.SetActive( UIButtons.FindChildByName( region_node_id, "option_name" ), false )

									local region_slider_id = SCUI_Popup.name_to_id[ "region_rule_slider" ]
									
									UIButtons.SetActive( region_slider_id, true )
									UIButtons.SetActive( SCUI_Popup.name_to_id["region_title"], true )

									UIButtons.SetParent( region_slider_id, region_node_id, UIEnums.Justify.TopLeft )
									UIButtons.AddItem( region_slider_id, UIEnums.MatchingRule.Any, UIText.MP_REGION_ANY, false )
									UIButtons.AddItem( region_slider_id, UIEnums.MatchingRule.Preferred, UIText.MP_REGION_PREFERRED, false )
									UIButtons.AddItem( region_slider_id, UIEnums.MatchingRule.Enforced, UIText.MP_REGION_REQUIRED, false )

									UIButtons.AddListItem( menu_id, region_node_id, 0 )

									-- set default
									local rule = Amax.GetMultiplayerOptions()
									UIButtons.SetSelection( region_slider_id, rule )
								end,

			next_CB		=	function()
								-- set options
								local region_id = SCUI_Popup.name_to_id[ "region_rule_slider" ]

								-- update player options and network settings
								Amax.SetMultiplayerOptions( UIButtons.GetSelection( region_id ) )
								NetServices.SetMatchingRegionInfo()
							end,
		},
		[UIEnums.CustomPopups.LobbyOptions] =
		{
			darken			= true,
			colour_style	= "Main_Black",
			intensity		= 0.6,
			icon_name		= "groups",

			title_text_ID	= UIText.POP_TITLE_GARAGE,
			next_text_ID	= UIText.INFO_A_SELECT,
			back_text_ID	= UIText.INFO_B_BACK,
			
			init_CB			=	function()
									CustomPopup.CurrentData.options = {}

									local new_cars = Multiplayer.GetNewCarsForClass( NetRace.GetCarClass() ) -- TODO_MP_REFACTOR this wont work when brought to release
									local car_select_locked = ( UIGlobals.Multiplayer.LobbyState ~= UIEnums.MpLobbyState.Countdown and UIGlobals.Multiplayer.LobbyState ~= UIEnums.MpLobbyState.WaitingForHost )
									local mod_shop_locked = Multiplayer.CanChooseMods() == false or ( Amax.IsGameModeRanked() == true and Multiplayer.GetFeatureUnlocked( UIEnums.MPFeatures.ModShop ) == false )
									local race_summary_locked = (Multiplayer.GetRaceResultsValid() == false)

									-- add each table item
									table.insert( CustomPopup.CurrentData.options, { value = UIEnums.LobbyOptions.CarSelect,	name = UIText.MP_CAR_SELECT,	locked = car_select_locked } )
									table.insert( CustomPopup.CurrentData.options, { value = UIEnums.LobbyOptions.ModShop,		name = UIText.MP_MOD_SHOP,		locked = mod_shop_locked } )

									if Amax.GetPlayMode() == UIEnums.PlayMode.CustomRace and NetRace.IsHost() == true then
										table.insert( CustomPopup.CurrentData.options, { value = UIEnums.LobbyOptions.Settings,	name = UIText.MP_GAME_SETTINGS } )
									end

									if Amax.GetGameMode() == UIEnums.GameMode.Online then
										table.insert( CustomPopup.CurrentData.options, { value = UIEnums.LobbyOptions.Challenges, name = UIText.MP_CHALLENGES } )
									end
									
									table.insert( CustomPopup.CurrentData.options, { value = UIEnums.LobbyOptions.SessionScoreboard,	name = UIText.MP_SESSION_LEADERBOARD } )

									if NetRace.RaceInProgress() == true then
										if NetRace.HasRaceFinished() == false and Amax.GetGameMode() ~= UIEnums.GameMode.SystemLink and Amax.GetPlayMode() ~= UIEnums.PlayMode.CustomRace then											
											table.insert( CustomPopup.CurrentData.options, { value = UIEnums.LobbyOptions.RacePreview, name = UIText.MP_RACE_PREVIEW } )
										end
									else
										if Amax.IsGameModeOnline() == true then --dont want to be able to view the race summary screen in system link
											table.insert( CustomPopup.CurrentData.options, { value = UIEnums.LobbyOptions.RaceSummery,	name = UIText.MP_RACE_SUMMERY, locked =  race_summary_locked} )
										end
									end
								end,
		},
		[UIEnums.CustomPopups.ViewPlayer] =
		{
			darken			= true,
			colour_style	= "Main_Black",
			intensity		= 0.6,
			icon_name		= "groups",

			title_text_ID	= UIText.CMN_NOWT,
			next_text_ID	= UIText.INFO_A_SELECT,
			back_text_ID	= UIText.INFO_B_BACK,

			options			= {}, -- need to fill this out in the init

			init_CB			=	function()

									if UIGlobals.Multiplayer.InPartyLobby == true then
										CustomPopup.CurrentData.title_text_ID = "LOBBY_PARTY_PLAYER_VIEWING" .. UIGlobals.Multiplayer.SelectPlayerJoinRef
									else
										CustomPopup.CurrentData.title_text_ID = "LOBBY_PLAYER_VIEWING" .. UIGlobals.Multiplayer.SelectPlayerJoinRef
									end

									CustomPopup.CurrentData.options = {}

									-- only 360 has player review and view gamercard
									if UIEnums.CurrentPlatform == UIEnums.Platform.Xenon then
										table.insert( CustomPopup.CurrentData.options, { value = UIEnums.PlayerOptions.GamerCard,	name = UIText.MP_VIEW_GAMERCARD,		locked = not NetServices.CanViewGamerCard( Profile.GetPrimaryPad() ) } )
										table.insert( CustomPopup.CurrentData.options, { value = UIEnums.PlayerOptions.Review,		name = UIText.MP_SUBMIT_PLAYER_REVIEW,	locked = not NetServices.CanSubmitPlayerReview( Profile.GetPrimaryPad() ) } )
									end

									-- Cant mute yourself
									if NetServices.QuickMuteAvailable( Profile.GetPrimaryPad(), UIGlobals.Multiplayer.SelectPlayerJoinRef, UIGlobals.Multiplayer.InPartyLobby ) == true then
										-- toggle text
										local muted = NetServices.IsRemotePlayerQuickMuted( Profile.GetPrimaryPad(), UIGlobals.Multiplayer.SelectPlayerJoinRef, UIGlobals.Multiplayer.InPartyLobby )
									
										table.insert( CustomPopup.CurrentData.options, { value = UIEnums.PlayerOptions.Voice, name = Select( muted, UIText.MP_UNMUTE, UIText.MP_MUTE ) } )
									end
								end,

			next_CB			=	function()
	
									local option = UIButtons.GetSelection( SCUI_Popup.name_to_id[ "options" ] )

									if option == UIEnums.PlayerOptions.GamerCard and NetServices.CanViewGamerCard( Profile.GetPrimaryPad() ) == true then
										NetServices.ViewGamerCard( Profile.GetPrimaryPad(), UIGlobals.Multiplayer.SelectPlayerJoinRef, UIGlobals.Multiplayer.InPartyLobby, UIGlobals.Multiplayer.ViewingResults )
									
									elseif option == UIEnums.PlayerOptions.Review and NetServices.CanSubmitPlayerReview( Profile.GetPrimaryPad() ) == true then
										NetServices.SubmitPlayerReview( Profile.GetPrimaryPad(), UIGlobals.Multiplayer.SelectPlayerJoinRef, UIGlobals.IsIngame, UIGlobals.Multiplayer.InPartyLobby )
									
									elseif option == UIEnums.PlayerOptions.Voice then
										NetServices.ToggleRemotePlayerQuickMute( Profile.GetPrimaryPad(), UIGlobals.Multiplayer.SelectPlayerJoinRef, UIGlobals.Multiplayer.InPartyLobby )
									end
								end,
		},
		[UIEnums.CustomPopups.MpBetaExpired] = 
		{
			darken			= true,
			title_text_ID	= UIText.POP_TITLE_ERROR,
			message_text_ID	= UIText.POP_BETA_EXPIRED,
			
			next_text_ID	= UIText.INFO_A_OK,
			no_back			= true,
		},

		[UIEnums.CustomPopups.AcceptPartyRaceInvite] = 
		{
			darken			= true,
			colour_style	= "Main_Black",
			intensity		= 0.6,
			icon_name		= "groups",
			no_back			= true,
			no_next			= true,
			show_progress	= true,

			title_text_ID	= UIText.POP_TITLE_PLEASE_WAIT,
			message_text_ID	= UIText.POP_ATTEMPTING_TO_JOIN_GAME,
			timeout			= 25,

			init_CB			=	function()
									PopupGUI.timer = 0
									PopupGUI.state = UIEnums.MpOnlineMatching.JoinStart

									print( "PARTY RACE INVITE" )
								end,

			update_CB		=	function(time)
									PopupGUI.timer = PopupGUI.timer + time
									
									local  run_main_logic = true
									 
									if NetParty.IsHost( ) == true then -- if a peer has migrated to host then fail the join and force everyone back to the party lobby.
										run_main_logic = false
										NetRace.Delete( )
										NetParty.SendReturnToPartyLobby( )
										
										UIGlobals.Network.RaceInviteState = UIEnums.MpRaceInviteState.Polling
										net_EnterPartyLobby( true )
										PopupCancel()
										
									elseif NetParty.ReceivedReturnToLobby( false ) == true then -- has the host informed us to return to the party lobby while joining.
										run_main_logic = false
										NetRace.Delete( )
										
										UIGlobals.Network.RaceInviteState = UIEnums.MpRaceInviteState.Polling
										net_EnterPartyLobby( true )
										PopupCancel()
									end

									if run_main_logic == true then
										if PopupGUI.state == UIEnums.MpOnlineMatching.JoinStart then
											if PopupGUI.timer >= 1.5 then
												
												if NetRace.JoinOnlineServer( 0 ) == true then
													-- if its a playlist then find and store it
													local ranked, custom_race, event_id = NetParty.GetRaceInviteData()
													if custom_race == true then
														net_MpEnterPlayMode( UIEnums.PlayMode.CustomRace )

														PopupGUI.state = UIEnums.MpOnlineMatching.JoinContinue
														NetParty.ChangeLocalPlayersState( UIEnums.Network.PlayerStates.eNetPlayerStatePartyJoinRace )
													else
														-- if ranked then we need to find it in sorcery. If that fails then stop and fail
														Multiplayer.CachePlaylists( UIEnums.GameMode.Online )

														-- check we can play
														if Multiplayer.FindPlaylistFromID( event_id ) == true then
															net_MpEnterPlayMode( UIEnums.PlayMode.Playlist, ranked )

															PopupGUI.state = UIEnums.MpOnlineMatching.JoinContinue
															NetParty.ChangeLocalPlayersState( UIEnums.Network.PlayerStates.eNetPlayerStatePartyJoinRace )
														else
															print( "Invite failed because you cant play the event" )
															PopupGUI.state = UIEnums.MpOnlineMatching.PartyJoinFailure
															NetParty.ChangeLocalPlayersState( UIEnums.Network.PlayerStates.eNetPlayerStatePartyJoinRaceFailed )	
														end
													end
												else
													PopupGUI.state = UIEnums.MpOnlineMatching.PartyJoinFailure
													NetParty.ChangeLocalPlayersState( UIEnums.Network.PlayerStates.eNetPlayerStatePartyJoinRaceFailed )								
												end
											end

										elseif PopupGUI.state == UIEnums.MpOnlineMatching.JoinContinue then
											local complete = false
											local code = UIEnums.Network.JoinErrors.eJoinErrorInvalid
											complete, code = NetRace.MonitorJoinOnlineServer( 0 )
											
											if complete == true then
												if code == UIEnums.Network.JoinErrors.eJoinErrorNone then 
													NetParty.ChangeLocalPlayersState( UIEnums.Network.PlayerStates.eNetPlayerStatePartyJoinRaceSuccessful )
													PopupGUI.state = UIEnums.MpOnlineMatching.Complete
												else
													NetParty.ChangeLocalPlayersState( UIEnums.Network.PlayerStates.eNetPlayerStatePartyJoinRaceFailed )								
													PopupGUI.state = UIEnums.MpOnlineMatching.PartyJoinFailure
												end
											end
										
										elseif PopupGUI.state == UIEnums.MpOnlineMatching.Complete then

											if NetParty.AllPlayersJoinedRace() == true then
												net_EnableGlobalUpdatePartyCommands( true )
												NetParty.ClearRaceInvite( )
												UIGlobals.Network.RaceInviteState = UIEnums.MpRaceInviteState.Polling

												PopupCancel( "Multiplayer\\MpLobby.lua", true )
											end
										
										elseif PopupGUI.state == UIEnums.MpOnlineMatching.PartyJoinFailure then
											-- host playe knows we failed and should inform us to go back to the party lobby asap.
										end
									end
								end,
		},
		[UIEnums.CustomPopups.SharingOptions] =
		{
			darken			= true,
			colour_style	= "Main_Black",
			intensity		= 0.6,

			title_text_ID	= UIText.POP_TITLE_SHARE,
			
			next_text_ID	= UIText.INFO_A_SELECT,
			back_text_ID	= UIText.INFO_B_BACK,
			
			init_CB			= function()
			
								CustomPopup.CurrentData.options = {}
								
								UIGlobals.FacebookUploadingPhoto = false

								local locked_opt=false
								if Amax.GetFacebookTimeout() > 0 then
									locked_opt=true
								end
								if Amax.FacebookHasPublisherFile() == true then
									UIGlobals.FacebookLocked=locked_opt
									table.insert( CustomPopup.CurrentData.options, { value = UIEnums.ShareOptions.Facebook,	name = "GAME_FACEBOOK_PUBLISH_INFO", locked = locked_opt } )
								end
								
								locked_opt=false
								if Amax.GetTwitterTimeout() > 0 then
									locked_opt=true
								end
								if Amax.TwitterHasPublisherFile() == true then
									UIGlobals.TwitterLocked=locked_opt
									table.insert( CustomPopup.CurrentData.options, { value = UIEnums.ShareOptions.Twitter,	name = "GAME_TWITTER_PUBLISH_INFO", locked = locked_opt } )
								end
								
								if UIEnums.CurrentPlatform == UIEnums.Platform.Xenon then
									if Profile.PadProfileOnline( Profile.GetPrimaryPad() ) == true then
										table.insert( CustomPopup.CurrentData.options, { value = UIEnums.ShareOptions.Blurb,	name = UIText.RBC_SYSTEM_MESSAGE, locked = false } )
									end
								elseif UIEnums.CurrentPlatform == UIEnums.Platform.PS3 then
									if Profile.PadProfileOnline( Profile.GetPrimaryPad() ) == true or net_CanReconnectToDemonware() == true then
										table.insert( CustomPopup.CurrentData.options, { value = UIEnums.ShareOptions.Blurb,	name = UIText.RBC_PS3_SYSTEM_MESSAGE, locked = false } )
									end
								end
		
							end,
							
			update_CB		=	function()
			
									local index=0
									
									if Amax.FacebookHasPublisherFile() == true then
										if Amax.GetFacebookTimeout() > 0 then
											UIButtons.SetNodeItemLocked( SCUI_Popup.name_to_id[ "options" ], UIEnums.ShareOptions.Facebook, true, true )
										else
											UIButtons.SetNodeItemLocked( SCUI_Popup.name_to_id[ "options" ], UIEnums.ShareOptions.Facebook, false, true )

											if UIGlobals.FacebookLocked == true then
												local id=UIButtons.GetNodeID(SCUI_Popup.name_to_id[ "options" ], index)
												local option_id = UIButtons.FindChildByName( id, "option_name" )
												UIButtons.ChangeColour( option_id, "!255 0 0 0")
												
												UIGlobals.FacebookLocked = false
											end
										end
										
										index=index+1
									end
									
									if Amax.TwitterHasPublisherFile() == true then
										if Amax.GetTwitterTimeout() > 0 then
											UIButtons.SetNodeItemLocked( SCUI_Popup.name_to_id[ "options" ], UIEnums.ShareOptions.Twitter, true, true )
										else
											UIButtons.SetNodeItemLocked( SCUI_Popup.name_to_id[ "options" ], UIEnums.ShareOptions.Twitter, false, true )

											if UIGlobals.TwitterLocked == true then
												local id=UIButtons.GetNodeID(SCUI_Popup.name_to_id[ "options" ], index)
												local option_id = UIButtons.FindChildByName( id, "option_name" )
												UIButtons.ChangeColour( option_id, "!255 0 0 0")
												
												UIGlobals.TwitterLocked = false
											end
										end			
									end
									
								end,
							
			next_CB		=	function()
								local option = UIButtons.GetSelection( SCUI_Popup.name_to_id[ "options" ] )
									if UIEnums.CurrentPlatform == UIEnums.Platform.PS3 and option == UIEnums.ShareOptions.Blurb then
										if Profile.PadProfileOnline( Profile.GetPrimaryPad() ) == false and net_CanReconnectToDemonware() == true then
											net_StartServiceConnection(true, nil, true)
										end
									end
								end,
								
			back_CB			=	function()
									if UIGlobals.ShareFromWhatPopup ~= -1 then
										PopupSpawn(UIGlobals.ShareFromWhatPopup)
									end
								end,
		},
		[UIEnums.CustomPopups.MPOnlineSharingOptions] =
		{
			darken			= true,
			colour_style	= "Main_Black",
			intensity		= 0.6,

			title_text_ID	= UIText.POP_TITLE_WHAT_SHARE,
			
			next_text_ID	= UIText.INFO_A_SELECT,
			back_text_ID	= UIText.INFO_B_BACK,
			
			init_CB			=	function()
									CustomPopup.CurrentData.options = 
									{
										{ value = UIEnums.MPOnlineSharingOptions.Rank,			name = UIText.MP_SCOREBOARD_RANK },
										{ value = UIEnums.MPOnlineSharingOptions.EventRating,	name = UIText.MP_STAT_HELP_RATING },
										{ value = UIEnums.MPOnlineSharingOptions.TotalWins,		name = UIText.MP_STAT_HELP_TOTAL_WINS },
										{ value = UIEnums.MPOnlineSharingOptions.PowerUpRatio,	name = UIText.MP_STAT_HELP_POWERUPS },
										{ value = UIEnums.MPOnlineSharingOptions.TotalRaces,	name = UIText.MP_STAT_HELP_TOTAL_RACES },
										{ value = UIEnums.MPOnlineSharingOptions.TotalPlaytime,	name = UIText.MP_STAT_HELP_PLAYTIME },
										{ value = UIEnums.MPOnlineSharingOptions.TotalFans,		name = UIText.MP_STAT_HELP_TOTAL_FANS },
										{ value = UIEnums.MPOnlineSharingOptions.LegendRank,	name = UIText.MP_STAT_HELP_LEGEND_RANK },
									}
								end,


			next_CB		=	function()
								local option = UIButtons.GetSelection( SCUI_Popup.name_to_id[ "options" ] )
								UIGlobals.SharingOptionsChosen=option
								UIGlobals.ShareFromWhatPopup=UIEnums.CustomPopups.MPOnlineSharingOptions
								PopupSpawn(UIEnums.CustomPopups.SharingOptions)
							end,
		},
		[UIEnums.CustomPopups.MPLobbyLocalSharingOptions] =
		{
			darken			= true,
			colour_style	= "Main_Black",
			intensity		= 0.6,

			title_text_ID	= UIText.POP_TITLE_WHAT_SHARE,
			
			next_text_ID	= UIText.INFO_A_SELECT,
			back_text_ID	= UIText.INFO_B_BACK,
			
			next_CB		=	function()
								local option = UIButtons.GetSelection( SCUI_Popup.name_to_id[ "options" ] )
								UIGlobals.SharingOptionsChosen=option
								UIGlobals.ShareFromWhatPopup=UIEnums.CustomPopups.MPLobbyLocalSharingOptions
								PopupSpawn(UIEnums.CustomPopups.SharingOptions)
							end,

			options			= 
			{
				{ value = UIEnums.MPLobbyLocalSharingOptions.Position,	name = UIText.MP_LAST_RACE_POSITION },
				{ value = UIEnums.MPLobbyLocalSharingOptions.Rank,		name = UIText.MP_SCOREBOARD_RANK },
			},
		},	
		[UIEnums.CustomPopups.TwitterAuthenticated] =
		{
			darken			= true,
			
			no_back			= true,
			
			title_text_ID	= UIText.POP_TITLE_TWITTER,
			message_text_ID	= UIText.POP_TWITTER_AUTHENTICATED,
			
			next_text_ID	= UIText.INFO_A_OK,
			next_CB			= function()
								UIGlobals.TwitterClose = true
							end,
		},
		[UIEnums.CustomPopups.SendingFriendDemand] =
		{
			show_progress	= true,

			darken			= true,
			no_back			= true,
			no_next			= true,
			
			title_text_ID	= UIText.POP_TITLE_PLEASE_WAIT,
			message_text_ID	= UIText.POP_UPLOADING,

			init_CB			=	function()
									PopupGUI.timer = 0
									FriendDemand.Send()
								end,

			update_CB		=	function(time)

									PopupGUI.timer = PopupGUI.timer + time

									local busy, error = FriendDemand.PumpSend()

									if busy == false and PopupGUI.timer > 2 then
										if error == 0 then
											PopupSpawn( UIEnums.CustomPopups.FriendDemandSent )
											if UIGlobals.FriendDemandIsResend == true then
												Amax.CheckStickerProgress( UIEnums.StickerType.SendChallengeBack )
											else
												Amax.CheckStickerProgress( UIEnums.StickerType.FirstFriendDemand )
											end
										else
											PopupSpawn( UIEnums.CustomPopups.FriendDemandSendingError )
										end
									end
								end,
		},
		[UIEnums.CustomPopups.FriendDemandSent] = 
		{
			darken			= true,
			title_text_ID	= UIText.POP_FRIEND_DEMAND,
			message_text_ID	= UIText.POP_FRIEND_DEMAND_POSTED,
			
			next_text_ID	= UIText.INFO_A_OK,
			no_back			= true,

			init_CB			=	function()
									UIGlobals.FriendDemandSent = true
								end,
		},
		[UIEnums.CustomPopups.FriendDemandSendingError] = 
		{
			darken			= true,
			title_text_ID	= UIText.POP_FRIEND_DEMAND,
			message_text_ID	= UIText.POP_FRIEND_DEMAND_ERROR,
			
			next_text_ID	= UIText.INFO_A_OK,
			x_text_ID		= UIText.INFO_TRY_RECONNECT,
			
			no_back			= true,

			init_CB			=	function()
									if net_CanReconnectToDemonware() == false then
										CustomPopup.CurrentData.x_text_ID = nil
									else
										CustomPopup.CurrentData.x_text_ID = UIText.INFO_TRY_RECONNECT
									end
								end,

			message_CB		=	function( message,arg1,arg2 )
									if message == UIEnums.Message.ButtonX then
										if net_CanReconnectToDemonware() == true then
											PlaySfxNext()
											net_StartServiceConnection(true, FriendChallengeReloadScreen, true)
										end
									end
								end,
		},
		[UIEnums.CustomPopups.FriendDemandRetrieving] =
		{
			show_progress	= true,

			darken			= true,
			no_back			= true,
			no_next			= true,
			
			title_text_ID	= UIText.POP_FRIEND_DEMAND,
			message_text_ID	= UIText.POP_RETRIEVING,

			init_CB			=	function()
									PopupGUI.timer = 0
									FriendDemand.RetrieveFromServer(false)
								end,

			update_CB		=	function(time)

									PopupGUI.timer = PopupGUI.timer + time

									if PopupGUI.timer > 1 then
										local busy, error = FriendDemand.PumpSend()

										if busy == false then
											if error == 0 then
												if UIGlobals.FriendDemandDoNotRefresh ~= true then
													UIGlobals.RefreshFriendDemandScreen = true
												end
												
												if FriendDemand.MessagesStillOnServer() == true then
													PopupSpawn( UIEnums.CustomPopups.FriendDemandMoreOnServer )
												else
													PopupCancel()
												end
											else
												UIGlobals.FriendDemandAttemptFromMessage = false
												if net_CanReconnectToDemonware() == false then
													PopupSpawn( UIEnums.CustomPopups.MultiplayerOnlineConnectionLost )
												else
													PopupSpawn( UIEnums.CustomPopups.ContentServerGeneralError )
												end
											end
											
											UIGlobals.FriendDemandDoNotRefresh = false
										end
									end
								end,
		},
		[UIEnums.CustomPopups.FriendDemandMoreOnServer] = 
		{
			darken			= true,
			title_text_ID	= UIText.POP_FRIEND_DEMAND,
			message_text_ID	= UIText.POP_FRIEND_DEMAND_MORE_ON_SERVER,
			
			next_text_ID	= UIText.INFO_A_OK,
			no_back			= true,
		},
		[UIEnums.CustomPopups.FriendDemandRetrievingError] = 
		{
			darken			= true,
			title_text_ID	= UIText.POP_FRIEND_DEMAND,
			message_text_ID	= UIText.POP_FRIEND_DEMAND_RETRIEVING_ERROR,
			
			next_text_ID	= UIText.INFO_A_OK,
			no_back			= true,
		},
		[UIEnums.CustomPopups.FriendDemandDelete] =
		{
			darken			= true,
			
			title_text_ID	= UIText.POP_FRIEND_DEMAND,
			message_text_ID	= UIText.POP_DELETE_FRIEND_DEMAND,
			
			next_text_ID	= UIText.INFO_YES_A,
			back_text_ID	= UIText.INFO_NO_B,

			next_CB			= function()
			
									if UIGlobals.FriendDemandCurrentType == UIEnums.FriendDemandMessageType.Local then
									
										FriendDemand.Remove(UIGlobals.CurrentFriendDemandMessageIndex)
										StartAsyncSave()
										UIGlobals.RefreshFriendDemandScreen = true
										
									else
									
										PopupSpawn( UIEnums.CustomPopups.FriendDemandDeleting )
										
									end
								end,
		},
		[UIEnums.CustomPopups.FriendDemandDeleting] =
		{
			show_progress	= true,

			darken			= true,
			no_back			= true,
			no_next			= true,
			
			title_text_ID	= UIText.POP_FRIEND_DEMAND,
			message_text_ID	= UIText.POP_DELETING_FRIEND_DEMAND,

			init_CB			=	function()
									PopupGUI.timer = 0
									FriendDemand.DeleteMessage(UIGlobals.CurrentFriendDemandMessageIndex)
								end,

			update_CB		=	function(time)

									PopupGUI.timer = PopupGUI.timer + time

									if PopupGUI.timer > 1 then
										local busy, error = FriendDemand.PumpSend()

										if busy == false then
											if error == 0 then
												PopupSpawn( UIEnums.CustomPopups.FriendDemandRetrieving )
											else
												if net_CanReconnectToDemonware() == false then
													PopupSpawn( UIEnums.CustomPopups.MultiplayerOnlineConnectionLost )
												else
													PopupSpawn( UIEnums.CustomPopups.ContentServerGeneralError )
												end
											end
										end
									end
								end,
		},
		[UIEnums.CustomPopups.SpChangeDifficulty] = 
		{
			darken			= true,
			title_text_ID	= UIText.CMN_DIFFICULTY,
			next_text_ID	= UIText.INFO_A_SELECT,
			back_text_ID	= UIText.INFO_B_BACK,
			
			options			= 
			{
--				{ value = 0, name = UIText.CMN_DIFFICULTY_1 },
				{ value = 1, name = UIText.CMN_DIFFICULTY_2 },
				{ value = 2, name = UIText.CMN_DIFFICULTY_3 },
				{ value = 3, name = UIText.CMN_DIFFICULTY_4 },
--				{ value = 4, name = UIText.CMN_DIFFICULTY_5 },
			},
			init_CB		=	function()
					CustomPopup.CurrentData.default_option	= UIGlobals.OptionsTable.difficulty
				end,
				
			next_CB		= function()
					local difficulty_selected = UIButtons.GetSelection( SCUI_Popup.name_to_id[ "options" ] )
					local difficulty = UIGlobals.OptionsTable.difficulty
					
					if difficulty_selected ~= difficulty then
						UIGlobals.OptionsTable.difficulty = difficulty_selected
						Amax.Options( UIGlobals.OptionsTable )
						
						Amax.GameDataLogDifficultySelected(difficulty, difficulty_selected)
						
						print( "Difficulty changed to", difficulty_selected )
						
						StartAsyncSave()
					end
				end,
		},
		[UIEnums.CustomPopups.MultiplayerOnlineConnectionLost] =
		{
			darken			= true,
			no_back			= true,

			title_text_ID	= UIText.POP_TITLE_ALERT,
			message_text_ID	= UIText.POP_NO_CONNECTION_TO_LIVE,

			next_text_ID	= UIText.INFO_A_OK,
			
			init_CB			= function()
								if UIEnums.CurrentPlatform == UIEnums.Platform.Xenon then
									CustomPopup.CurrentData.message_text_ID = UIText.POP_NO_CONNECTION_TO_LIVE
								elseif UIEnums.CurrentPlatform == UIEnums.Platform.PC then
									CustomPopup.CurrentData.message_text_ID = UIText.POP_NO_CONNECTION_TO_INTERNET
								elseif UIEnums.CurrentPlatform == UIEnums.Platform.PS3 then
									CustomPopup.CurrentData.message_text_ID = UIText.POP_NO_CONNECTION_TO_PSN	
								end
							  end,
			
		},
		[UIEnums.CustomPopups.DiscardModshopLoadoutChanges] =
		{
			darken			= true,
			colour_style	= "Main_Black",

			title_text_ID	= UIText.POP_TITLE_WARNING,
			message_text_ID	= UIText.POP_DISCARD_LOADOUT_CHANGES,
			
			next_text_ID	= UIText.INFO_A_OK,
			no_back			= true,
		},
		[ UIEnums.CustomPopups.AutoSaveWarning ] = 
		{
			darken			= true,
			no_back			= true,

			title_text_ID	= UIText.POP_TITLE_WARNING,
			message_text_ID	= UIText.POP_AUTO_SAVE_WARNING,

			next_text_ID	= UIText.INFO_A_OK,
			
			save_icon		= true,
		},
		[ UIEnums.CustomPopups.ActiveProfileChanged ] = 
		{
			darken			= true,
			title_text_ID	= UIText.POP_PROFILE_KICKED_TITLE,
			message_text_ID	= UIText.POP_INACTIVE_PROFILE_CHANGED_MSG,
			
			next_text_ID	= UIText.INFO_A_OK,
			no_back			= true,
		},
		[UIEnums.CustomPopups.SpPostRaceSharingOptions] =
		{
			darken			= true,
			colour_style	= "Main_Black",
			intensity		= 0.6,

			title_text_ID	= UIText.POP_TITLE_WHAT_SHARE,
			
			next_text_ID	= UIText.INFO_A_SELECT,
			back_text_ID	= UIText.INFO_B_BACK,

			init_CB			=	function()

									CustomPopup.CurrentData.options = {}
																			
									local event=SinglePlayer.EventInfo( UIGlobals.Sp.TierInfo[ UIGlobals.Sp.CurrentTier ].events[ UIGlobals.Sp.CurrentEvent ] )
									
									CustomPopup.CurrentData.options[#CustomPopup.CurrentData.options + 1] = 
									{ 
										value = UIEnums.SocialNetworkingItemType.EventStars,
										name = UIText.SP_EVENT_STARS_EARNT 
									}
									
									if event.fanParComplete == true then
										CustomPopup.CurrentData.options[#CustomPopup.CurrentData.options + 1] = 
										{ 
											value = UIEnums.SocialNetworkingItemType.FanParComplete,
											name = UIText.SP_FAN_PAR_COMPLETE 
										}
									end
									
									if event.fanRunComplete == true then
										CustomPopup.CurrentData.options[#CustomPopup.CurrentData.options + 1] = 
										{ 
											value = UIEnums.SocialNetworkingItemType.GoldenFanDemandComplete,
											name = UIText.SP_FAN_RUN_COMPLETE 
										}
									end
								end,

			next_CB		=	function()
								local option = UIButtons.GetSelection( SCUI_Popup.name_to_id[ "options" ] )
								
								UIGlobals.SharingOptionsChosen=option
								UIGlobals.ShareFromWhatPopup=UIEnums.CustomPopups.SpPostRaceSharingOptions
								PopupSpawn(UIEnums.CustomPopups.SharingOptions)
							end,								
		},
		[UIEnums.CustomPopups.AcceptingFriendDemand] =
		{
			show_progress	= true,

			darken			= true,
			no_back			= true,
			no_next			= true,
			
			title_text_ID	= UIText.POP_FRIEND_DEMAND,
			message_text_ID	= UIText.POP_FRIEND_DEMAND_ACCEPTING,

			init_CB			=	function()
			
									if UIGlobals.FriendDemandAttemptFromServer == false then
										CustomPopup.CurrentData.message_text_ID	= UIText.POP_FRIEND_DEMAND_SAVING
									end
									
									PopupGUI.timer = 0
									FriendDemand.SaveMessageToSlot(UIGlobals.CurrentFriendDemandMessageIndex, UIGlobals.CurrentFriendFreeSlot)
									FriendDemand.DeleteMessage(UIGlobals.CurrentFriendDemandMessageIndex)
								end,

			update_CB		=	function(time)

									PopupGUI.timer = PopupGUI.timer + time

									if PopupGUI.timer > 2 then
										local busy, error = FriendDemand.PumpSend()

										if busy == false then

											if error == 0 then
											
												FriendDemand.Accepted(UIGlobals.FriendDemandFilterFriend)
												StartAsyncSave()

												if UIGlobals.FriendDemandAttemptFromServer == true then
													UIGlobals.RefreshFriendDemandScreen = true
													PopupCancel( )
												else
													PopupSpawn( UIEnums.CustomPopups.AcceptedFriendDemand )
												end
												
												Amax.CheckStickerProgress( UIEnums.StickerType.AcceptFriendDemand )
											else
												UIGlobals.FriendDemandAttemptFromServer = false
												FriendDemand.RemoveMessageFromSlot(UIGlobals.CurrentFriendFreeSlot)

												if net_CanReconnectToDemonware() == false then
													PopupSpawn( UIEnums.CustomPopups.MultiplayerOnlineConnectionLost )
												else
													PopupSpawn( UIEnums.CustomPopups.ContentServerGeneralError )
												end
											end
										end
									end
								end,
		},
		[ UIEnums.CustomPopups.AcceptedFriendDemand ] = 
		{
			darken			= false,
			no_back			= true,

			title_text_ID	= UIText.POP_FRIEND_DEMAND,
			message_text_ID	= UIText.POP_FRIEND_DEMAND_ACCEPTED,

			next_text_ID	= UIText.INFO_A_OK,

			next_CB			=	function()
									UIGlobals.RefreshFriendDemandScreen = true
								end,
		},
		[UIEnums.CustomPopups.SpTierSelectSharingOptions] =
		{
			darken			= true,
			colour_style	= "Main_Black",
			intensity		= 0.6,

			title_text_ID	= UIText.POP_TITLE_WHAT_SHARE,
			
			next_text_ID	= UIText.INFO_A_SELECT,
			back_text_ID	= UIText.INFO_B_BACK,
			
			init_CB			=	function()
									
									CustomPopup.CurrentData.options = 
									{
										{ value = UIEnums.SpTierSelectSharingOptions.FanStatusRank,			name = UIText.SP_FAN_STATUS_RANK },
										{ value = UIEnums.SpTierSelectSharingOptions.CompletionPercentage,	name = UIText.SP_COMPLETION_PERCENTAGE },
										{ value = UIEnums.SpTierSelectSharingOptions.NumberOfFans,			name = UIText.SP_NUMBER_OF_FANS },
										{ value = UIEnums.SpTierSelectSharingOptions.NumberOfStars,			name = UIText.SP_NUMBER_OF_STARS },
										{ value = UIEnums.SpTierSelectSharingOptions.NumberOfCars,			name = UIText.SP_NUMBER_OF_CARS },
										{ value = UIEnums.SpTierSelectSharingOptions.BossesBeat,			name = UIText.SP_BOSSES_BEAT },
									}
									
									if UIGlobals.ShareShowTier == true then
										table.insert( CustomPopup.CurrentData.options, { value = UIEnums.SpTierSelectSharingOptions.TierComplete,	name = UIText.SP_TIER_COMPLETE } )
									end
								end,

			next_CB		=	function()
								local option = UIButtons.GetSelection( SCUI_Popup.name_to_id[ "options" ] )
								UIGlobals.SharingOptionsChosen=option
								UIGlobals.ShareFromWhatPopup=UIEnums.CustomPopups.SpTierSelectSharingOptions
								PopupSpawn(UIEnums.CustomPopups.SharingOptions)
							end,
		},
		[UIEnums.CustomPopups.SpMainSelectSharingOptions] =
		{
			darken			= true,
			colour_style	= "Main_Black",
			intensity		= 0.6,

			title_text_ID	= UIText.POP_TITLE_WHAT_SHARE,
			
			next_text_ID	= UIText.INFO_A_SELECT,
			back_text_ID	= UIText.INFO_B_BACK,
			
			init_CB			=	function()
									CustomPopup.CurrentData.options = 
									{
										--{ value = UIEnums.SpTierSelectSharingOptions.FanStatusRank,			name = UIText.SP_FAN_STATUS_RANK },
										--{ value = UIEnums.SpTierSelectSharingOptions.CompletionPercentage,	name = UIText.SP_COMPLETION_PERCENTAGE },
										{ value = UIEnums.SpTierSelectSharingOptions.NumberOfFans,			name = UIText.SP_NUMBER_OF_FANS },
										{ value = UIEnums.SpTierSelectSharingOptions.NumberOfStars,			name = UIText.SP_NUMBER_OF_STARS },
										--{ value = UIEnums.SpTierSelectSharingOptions.NumberOfCars,			name = UIText.SP_NUMBER_OF_CARS },
										--{ value = UIEnums.SpTierSelectSharingOptions.NumberOfMods,			name = UIText.SP_NUMBER_OF_MODS },
										--{ value = UIEnums.SpTierSelectSharingOptions.BossesBeat,			name = UIText.SP_BOSSES_BEAT },
										--{ value = UIEnums.SpTierSelectSharingOptions.TierComplete,			name = UIText.SP_TIER_COMPLETE },
									}
								end,

			next_CB		=	function()
								local option = UIButtons.GetSelection( SCUI_Popup.name_to_id[ "options" ] )
								UIGlobals.SharingOptionsChosen=option
								UIGlobals.ShareFromWhatPopup=UIEnums.CustomPopups.SpMainSelectSharingOptions
								PopupSpawn(UIEnums.CustomPopups.SharingOptions)
							end,
		},
		[UIEnums.CustomPopups.SpEventSelectSharingOptions] =
		{
			darken			= true,
			colour_style	= "Main_Black",
			intensity		= 0.6,

			title_text_ID	= UIText.POP_TITLE_WHAT_SHARE,
			
			next_text_ID	= UIText.INFO_A_SELECT,
			back_text_ID	= UIText.INFO_B_BACK,

			init_CB			=	function()
									CustomPopup.CurrentData.options = {}
									
									CustomPopup.CurrentData.options[#CustomPopup.CurrentData.options + 1] = 
									{ 
										value = UIEnums.SocialNetworkingItemType.EventStars,
										name = UIText.SP_EVENT_STARS_EARNT 
									}
									
									local event=SinglePlayer.EventInfo( UIGlobals.Sp.TierInfo[ UIGlobals.Sp.CurrentTier ].events[ UIGlobals.Sp.CurrentEvent ] )
									
									if event.fanParComplete == true then
										CustomPopup.CurrentData.options[#CustomPopup.CurrentData.options + 1] = 
										{ 
											value = UIEnums.SocialNetworkingItemType.FanParComplete,
											name = UIText.SP_FAN_PAR_COMPLETE 
										}
									end
									
									if event.fanRunComplete == true then
										CustomPopup.CurrentData.options[#CustomPopup.CurrentData.options + 1] = 
										{ 
											value = UIEnums.SocialNetworkingItemType.GoldenFanDemandComplete,
											name = UIText.SP_FAN_RUN_COMPLETE 
										}
									end
								end,

			next_CB		=	function()
								local option = UIButtons.GetSelection( SCUI_Popup.name_to_id[ "options" ] )
								UIGlobals.SharingOptionsChosen=option
								UIGlobals.ShareFromWhatPopup=UIEnums.CustomPopups.SpEventSelectSharingOptions
								PopupSpawn(UIEnums.CustomPopups.SharingOptions)
							end,								
		},
		[UIEnums.CustomPopups.NoHistory] =
		{
			darken			= true,
			no_back			= true,
			colour_style	= "Main_Black",
			intensity		= 0.6,
			
			next_text_ID	= UIText.INFO_A_OK,
			
			title_text_ID	= UIText.POP_TITLE_INFO,
			message_text_ID	= UIText.POP_NO_RACE_HISTORY,

			next_CB			= function()
								UIGlobals.RaceHistoryClose=true
							end,
		},
		[UIEnums.CustomPopups.OnlinePhotoSharingOptions] =
		{
			darken			= true,
			colour_style	= "Main_Black",
			intensity		= 0.6,

			title_text_ID	= UIText.POP_TITLE_SHARE,
			
			next_text_ID	= UIText.INFO_A_SELECT,
			back_text_ID	= UIText.INFO_B_BACK,
			
			init_CB			= function()

								CustomPopup.CurrentData.options = {}
								
								if Amax.FacebookHasPublisherFile() == true then
									table.insert( CustomPopup.CurrentData.options, { value = UIEnums.ShareOptions.FacebookAlbum,	name = UIText.RBC_FACEBOOK_ALBUM } )
								end

								table.insert( CustomPopup.CurrentData.options, { value = UIEnums.ShareOptions.Blurb,	name = UIText.RBC_AGORA_UPLOAD } )

							end,
		},		
		[UIEnums.CustomPopups.FacebookAuthenticated] =
		{
			darken			= true,
			
			no_back			= true,
			
			title_text_ID	= UIText.POP_TITLE_FACEBOOK,
			message_text_ID	= UIText.POP_FACEBOOK_AUTHENTICATED,
			
			next_text_ID	= UIText.INFO_A_OK,
			next_CB			= function()
								UIGlobals.FacebookClose = true
							end,
		},
		[ UIEnums.CustomPopups.SpPostRaceOptions ] =
		{
			darken			= true,
			
			title_text_ID	= UIText.POP_TITLE_OPTIONS,

			next_text_ID	= UIText.INFO_A_SELECT,
			back_text_ID	= UIText.INFO_B_BACK,
			
			init_CB			=	function()
									CustomPopup.CurrentData.options = {  }
									
									local is_fd = ( Amax.SP_IsStreetRaceFD() == true or Amax.SP_IsDestructionFD() == true or Amax.SP_IsCheckpointFD() == true or Amax.SP_IsFanDemandFD() == true or Amax.SP_IsBossBattleFD() == true )
									local level_result = Amax.SP_GetLevelResult()
									
									local do_challenge = false
									local do_rechallenge = false
									local do_retry = false
									
									if is_fd == true then
										if FriendDemand.IsComplete() > 0 and UIGlobals.FriendDemandSent == false then
											do_rechallenge = true
										elseif UIGlobals.FriendDemandSent == false then
											do_retry = true
										end
									else
										if UIGlobals.FriendDemandSent == false then
											if level_result.state ~= "none" and Amax.GetNumFriends() > 0 then
												do_challenge = true
											end
										end
										
										do_retry = true	
									end
									
									CustomPopup.CurrentData.options[ #CustomPopup.CurrentData.options + 1 ] = { value = 1, name = UIText.POP_OPTION_RETRUN_TO_EVENT_SELECT, }
									
									if do_challenge == true then
										CustomPopup.CurrentData.options[ #CustomPopup.CurrentData.options + 1 ] = { value = 2, name = UIText.FDE_CHALLENGE_A_FRIEND, }
									end
									
									if do_rechallenge == true then
										CustomPopup.CurrentData.options[ #CustomPopup.CurrentData.options + 1 ] = { value = 2, name = UIText.FDE_RECHALLENGE_A_FRIEND, }
									end
									
									if do_retry == true then
										CustomPopup.CurrentData.options[ #CustomPopup.CurrentData.options + 1 ] = { value = 0, name = UIText.POP_OPTION_RETRY, }
									end
									
									if level_result.state == "none" and do_retry == true then									
										CustomPopup.CurrentData.default_option = 0
									elseif do_rechallenge == true then
										CustomPopup.CurrentData.default_option = 2
									else
										CustomPopup.CurrentData.default_option = 1
									end
								end,
		},
		[UIEnums.CustomPopups.HostChangedSettings] =
		{
			darken			= true,
			
			title_text_ID	= UIText.POP_TITLE_ALERT,
			message_text_ID	= UIText.POP_HOST_CHANGED_SETTINGS,
			
			next_text_ID	= UIText.INFO_A_OK,
		},
		[UIEnums.CustomPopups.SsRetry] =
		{
			darken			= true,
			
			title_text_ID	= UIText.POP_TITLE_WARNING,
			message_text_ID	= UIText.POP_SS_RETRY,
			
			next_text_ID	= UIText.INFO_A_SELECT,
			back_text_ID	= UIText.INFO_B_BACK,
			
			options			= 
			{
				{ value = UIEnums.PopupOptions.Yes,		name = UIText.POP_OPTION_YES, },
				{ value = UIEnums.PopupOptions.No,		name = UIText.POP_OPTION_NO, },
			},
		},
		[UIEnums.CustomPopups.TaleOfTheTapeSharingOptions] =
		{
			darken			= true,
			colour_style	= "Main_Black",
			intensity		= 0.6,

			title_text_ID	= UIText.POP_TITLE_WHAT_SHARE,
			
			next_text_ID	= UIText.INFO_A_SELECT,
			back_text_ID	= UIText.INFO_B_BACK,
			
			init_CB			=	function()
									CustomPopup.CurrentData.options = 
									{
										{ value = UIEnums.TaleOfTheTapeSharingOptions.Accepted,			name = UIText.FDE_SENT },
										{ value = UIEnums.TaleOfTheTapeSharingOptions.Won,				name = UIText.FDE_WON },
										{ value = UIEnums.TaleOfTheTapeSharingOptions.KOs,				name = UIText.FDE_KOS },
										{ value = UIEnums.TaleOfTheTapeSharingOptions.AvgAttempts,		name = UIText.FDE_AVG_ATTEMPTS },
									}
								end,

			next_CB		=	function()
								local option = UIButtons.GetSelection( SCUI_Popup.name_to_id[ "options" ] )
								UIGlobals.SharingOptionsChosen=option
								UIGlobals.ShareFromWhatPopup=UIEnums.CustomPopups.TaleOfTheTapeSharingOptions
								PopupSpawn(UIEnums.CustomPopups.SharingOptions)
							end,
		},		
		[UIEnums.CustomPopups.FriendDemandOptions] =
		{
			darken			= true,
			colour_style	= "Main_Black",
			intensity		= 0.6,

			title_text_ID	= UIText.POP_FRIEND_DEMAND,
			
			next_text_ID	= UIText.INFO_A_SELECT,
			back_text_ID	= UIText.INFO_B_BACK,
			
			init_CB			=	function()
			
									if UIGlobals.FriendDemandCurrentType == UIEnums.FriendDemandMessageType.Local then
										CustomPopup.CurrentData.options = 
										{
											{ value = UIEnums.FriendDemandOptions.Attempt,			name = UIText.FDE_TRY_NOW },
											{ value = UIEnums.FriendDemandOptions.Delete,			name = UIText.FDE_DELETE },
										}
									else
										CustomPopup.CurrentData.options = 
										{
											{ value = UIEnums.FriendDemandOptions.Attempt,			name = UIText.FDE_ATTEMPT },
											{ value = UIEnums.FriendDemandOptions.Store,			name = UIText.FDE_STORE },
											{ value = UIEnums.FriendDemandOptions.Delete,			name = UIText.FDE_DELETE },
										}
									end
								end,
		},
		[UIEnums.CustomPopups.FriendCompare] =
		{
			darken			= true,
			colour_style	= "Main_Black",
			intensity		= 0.6,
			icon_name		= "groups",

			title_text_ID	= UIText.POP_FRIEND_COMPARE,
			message_text_ID	= UIText.POP_FRIEND_COMPARE_MSG,
			
			next_text_ID	= UIText.INFO_YES_A,
			back_text_ID	= UIText.INFO_NO_B,
			
			next_CB			=	function()					
									Amax.SetFriendCompare(UIGlobals.CompareToFriendRow)
									Amax.CheckStickerProgress( UIEnums.StickerType.LeaderboardChallengeFriend )
									
									Amax.RequestRivalRemotePicture( )
									
									UIGlobals.UpdateEventOwners = true
									UIGlobals.UpdateEventOwnersPreRace = true
									PopupSpawn(UIEnums.CustomPopups.FriendCompareChosen)
								end,
		},
		[UIEnums.CustomPopups.NetworkCableRemoved] = 
		{
			darken			= true,
			title_text_ID	= UIText.POP_TITLE_ALERT,
			message_text_ID	= UIText.POP_NETWORK_CABLE_REMOVED,
			
			next_text_ID	= UIText.INFO_A_OK,
			no_back			= true,
		},
		[UIEnums.CustomPopups.SSLobbySharingOptions] =
		{
			darken			= true,
			colour_style	= "Main_Black",
			intensity		= 0.6,

			title_text_ID	= UIText.POP_TITLE_WHAT_SHARE,
			
			next_text_ID	= UIText.INFO_A_SELECT,
			back_text_ID	= UIText.INFO_B_BACK,
			
			init_CB			=	function()
									CustomPopup.CurrentData.options = 
									{
										{ value = UIEnums.SocialNetworkingItemType.SSWins,				name = UIText.MP_SCOREBOARD_NEW_TOTAL_WINS },
										{ value = UIEnums.SocialNetworkingItemType.SSPoints,			name = UIText.MP_SCOREBOARD_POINTS },
										{ value = UIEnums.SocialNetworkingItemType.SSTotalGames,		name = UIText.MP_STAT_HELP_TOTAL_RACES },
										{ value = UIEnums.SocialNetworkingItemType.SSTotalPlaytime,		name = UIText.MP_STAT_HELP_PLAYTIME },
									}
								end,

			next_CB		=	function()
								local option = UIButtons.GetSelection( SCUI_Popup.name_to_id[ "options" ] )
								UIGlobals.SharingOptionsChosen=option
								UIGlobals.ShareFromWhatPopup=UIEnums.CustomPopups.SSLobbySharingOptions
								PopupSpawn(UIEnums.CustomPopups.SharingOptions)
							end,
		},
		[UIEnums.CustomPopups.LeaderboardReadError] =
		{
			darken			= true,
			no_back			= true,
			
			next_text_ID	= UIText.INFO_A_OK,
			
			title_text_ID	= UIText.POP_TITLE_ALERT,
			message_text_ID	= UIText.POP_LEADERBOARD_READ_ERROR,
		},
		[UIEnums.CustomPopups.FriendDemandCustomAttemptError] = 
		{
			darken			= true,
			title_text_ID	= UIText.POP_TITLE_ERROR,
			message_text_ID	= UIText.POP_FRIEND_DEMAND_CUSTOM_POPUP_ERROR,
			
			next_text_ID	= UIText.INFO_A_OK,
			no_back			= true,
		},
		[UIEnums.CustomPopups.FriendDemandCustomAttemptPadError] = 
		{
			darken			= true,
			title_text_ID	= UIText.POP_TITLE_ERROR,
			message_text_ID	= UIText.POP_FRIEND_DEMAND_CUSTOM_POPUP_PAD_ERROR,
			
			next_text_ID	= UIText.INFO_A_OK,
			no_back			= true,
		},
		[UIEnums.CustomPopups.ContentServerSearching] =
		{
			show_progress	= true,

			darken			= true,
			no_back			= true,
			no_next			= true,
			colour_style	= "Main_Black",
			intensity		= 0.6,
			
			title_text_ID	= UIText.POP_TITLE_PLEASE_WAIT,
			message_text_ID	= UIText.POP_SEARCHING,
		},
		[UIEnums.CustomPopups.DiscardOptionsChanges] =
		{
			darken			= true,
			colour_style	= "Main_Black",
			intensity		= 0.6,
			icon_name		= "options cog",

			title_text_ID	= UIText.POP_TITLE_WARNING,
			message_text_ID	= UIText.POP_DISCARD_OPTIONS,
			
			next_text_ID	= UIText.INFO_A_SELECT,
			back_text_ID	= UIText.INFO_B_BACK,

			options = 
			{
				{ value = UIEnums.PopupOptions.Yes,				name = UIText.POP_OPTION_YES },
				{ value = UIEnums.PopupOptions.No,				name = UIText.POP_OPTION_NO },
			},
		},
		[UIEnums.CustomPopups.ResetToDefaults] =
		{
			darken			= true,
			colour_style	= "Main_Black",
			intensity		= 0.6,
			icon_name		= "options cog",

			title_text_ID	= UIText.POP_TITLE_WARNING,
			message_text_ID	= UIText.POP_RESET_DEFAULT,
			
			next_text_ID	= UIText.INFO_A_SELECT,
			back_text_ID	= UIText.INFO_B_BACK,

			options = 
			{
				{ value = UIEnums.PopupOptions.Yes,				name = UIText.POP_OPTION_YES },
				{ value = UIEnums.PopupOptions.No,				name = UIText.POP_OPTION_NO },
			},
		},
		[UIEnums.CustomPopups.FriendCompareChosen] =
		{
			darken			= true,
			no_back			= true,
			
			next_text_ID	= UIText.INFO_A_OK,
			
			title_text_ID	= UIText.POP_FRIEND_COMPARE,
			message_text_ID	= "GAME_FRIEND_COMPARE_MSG",

			init_CB			= function()
								StartAsyncSave()
							end,
		},
		[UIEnums.CustomPopups.FriendChallengeNoFriends] =
		{
			darken			= true,
			colour_style	= "Main_Black",
			intensity		= 0.6,
			icon_name		= "groups",
			no_back			= true,
			title_text_ID	= UIText.POP_TITLE_INFO,
			message_text_ID	= UIText.POP_FRIEND_CHALLENGE_NO_FRIENDS,
			next_text_ID	= UIText.INFO_A_OK,
		},
		[UIEnums.CustomPopups.FriendDemandInboxOptions] = 
		{
			darken			= true,
			
			next_text_ID	= UIText.INFO_A_SELECT,
			back_text_ID	= UIText.INFO_B_BACK,
			
			title_text_ID	= UIText.FDE_FRIEND_DEMANDS,
			
			options = 
			{
				{ value = 0,	name = UIText.FDE_MENU_NOT_ACCEPTED },
				{ value = 1,	name = UIText.FDE_MENU_ACCEPTED },
				{ value = 2,	name = UIText.FDE_MENU_RE_CHALLENGES },
			},
			
			init_CB			= function()
								CustomPopup.CurrentData.title_text_ID = "GAME_FRIEND_DEMAND_FRIEND_" .. UIGlobals.FriendDemandFilterFriend
								
								CustomPopup.CurrentData.options[ 1 ].locked = ( FriendDemand.GetFriendNewCount( UIGlobals.FriendDemandFilterFriend ) == 0 )
								CustomPopup.CurrentData.options[ 2 ].locked = ( FriendDemand.GetFriendActiveCount( UIGlobals.FriendDemandFilterFriend ) == 0 )
							end,
		},
		[UIEnums.CustomPopups.FriendDemandNoFreeSlots] = 
		{
			darken			= true,
			
			no_back			= true,
			next_text_ID	= UIText.INFO_A_OK,
			
			title_text_ID	= UIText.POP_TITLE_WARNING,
			message_text_ID	= UIText.POP_FRIEND_CHALLENGE_NO_FREE_SLOTS,
		},
		[UIEnums.CustomPopups.FriendDemandNoBlurbs] = 
		{
			darken			= true,
			
			no_back			= true,
			next_text_ID	= UIText.INFO_A_OK,
			
			title_text_ID	= UIText.POP_TITLE_WARNING,
			
			init_CB			= function()
								if UIGlobals.FriendDemandFilter == 3 then
									CustomPopup.CurrentData.message_text_ID = UIText.POP_FRIEND_CHALLENGE_NO_BLURBS_EVENT
								else
									CustomPopup.CurrentData.message_text_ID = UIText.POP_FRIEND_CHALLENGE_NO_BLURBS_FRIEND
								end
							end,
		},
		[UIEnums.CustomPopups.FriendChallengeNoPrivilege] =
		{
			darken			= true,
			no_back			= true,
			
			next_text_ID	= UIText.INFO_A_OK,
			
			title_text_ID	= UIText.POP_TITLE_INFO,
			message_text_ID	= UIText.POP_NO_CONTENT_PRIVILEGE,
		},
		[UIEnums.CustomPopups.FriendDemandNotOnServer] =
		{
			darken			= true,
			no_back			= true,
			
			next_text_ID	= UIText.INFO_A_OK,
			
			title_text_ID	= UIText.FDE_FRIEND_DEMANDS,
			message_text_ID	= UIText.FDE_FRIEND_DEMANDS_DOESNT_EXIST,
		},
		[UIEnums.CustomPopups.InvalidNetworkConnection] = 
		{
			darken			= true,
			title_text_ID	= UIText.POP_TITLE_WARNING,
			message_text_ID	= UIText.POP_NO_NETWORK_CONNECTION,
			
			next_text_ID	= UIText.INFO_A_OK,
			no_back			= true,
		},
		[UIEnums.CustomPopups.FriendDemandRemoveOldChallenges] =
		{
			darken			= true,
			
			title_text_ID	= UIText.POP_FRIEND_DEMAND,
			message_text_ID	= UIText.POP_FRIEND_DEMAND_REMOVE_OLD,
			
			next_text_ID	= UIText.INFO_YES_A,
			back_text_ID	= UIText.INFO_NO_B,

			next_CB			= function()
									FriendDemand.CleanOutOldChallenges()
									StartAsyncSave()
								end,
		},		
		[UIEnums.CustomPopups.SPLeaderboardPostRaceError] =
		{
			darken			= true,
			no_back			= true,
			colour_style	= "Main_Black",
			intensity		= 0.6,
			
			next_text_ID	= UIText.INFO_A_OK,
			
			title_text_ID	= UIText.POP_TITLE_ERROR,
			message_text_ID	= UIText.POP_CONTENT_SERVER_GENERAL_ERROR,
			x_text_ID		= UIText.INFO_TRY_RECONNECT,

			message_CB		=	function( message,arg1,arg2 )
									if message == UIEnums.Message.ButtonX then
										PlaySfxNext()
										if UIEnums.CurrentPlatform == UIEnums.Platform.Xenon then
											UIGlobals.server_connection.callback = SPRaceAwardLeaderboard
											PopupSpawn( UIEnums.CustomPopups.ReconnectToLeaderboards )											
										else
											net_StartServiceConnection(true, SPRaceAwardLeaderboard, true)
										end
									end
								end,
		},
		[UIEnums.CustomPopups.StorageDownload] =
		{
			darken			= true,
			no_back			= true,
			no_next			= true,
			show_progress	= true,
			
			title_text_ID	= UIText.POP_TITLE_PLEASE_WAIT,
			message_text_ID	= UIText.POP_STORAGE_DOWNLOAD,
			
			init_CB			=	function()
									NetServices.InitTimeOutTimer()
			
									PopupGUI.Timer = 0
									
									PopupGUI.task = UIEnums.NetTaskType.TitleStorage
									PopupGUI.state = UIEnums.NetTaskState.Ready
									
									-- [WillB] -- Fail safe incase we get into multiplayer without being in online via a game invite.
									net_MpEnter( UIEnums.GameMode.Online )
								end,
								
			end_CB			=	function()
									-- if we have a game invite waiting, then allow it to continue
									if UIGlobals.Network.GameInviteState == UIEnums.MpGameInviteState.Initialising then
										UIGlobals.Network.GameInviteState = UIEnums.MpGameInviteState.PreProcessing
									end
								end,
							  
			update_CB		=	function( time )
									PopupGUI.Timer = PopupGUI.Timer + time
									
									-- title storage
									if PopupGUI.task == UIEnums.NetTaskType.TitleStorage then
										-- start the download task
										if PopupGUI.state == UIEnums.NetTaskState.Ready then
											-- dont allow use to start until we have finished saving
											if UIScreen.IsContextActive( UIEnums.Context.LoadSave ) == false then
												PopupGUI.state = UIEnums.NetTaskState.Busy
											end
											
										-- pump the task
										elseif PopupGUI.state == UIEnums.NetTaskState.Busy then
											local state = NetServices.TitleStorageStatus()
											
											if state == UIEnums.TitleStorageState.Finished then
												print( "Storage Download : TMS Complete " )
												PopupGUI.task = UIEnums.NetTaskType.UserStorage
												PopupGUI.state = UIEnums.NetTaskState.Ready
										
											elseif state == UIEnums.TitleStorageState.Failed then
												print( "Storage Download : TMS Failed " )
												PopupGUI.state = UIEnums.NetTaskState.Failed
												UIGlobals.StorageFailType = UIEnums.StorageFailType.TitleFailed									
											end
											
										-- failed
										elseif PopupGUI.state == UIEnums.NetTaskState.Failed then
											PopupSpawn( UIEnums.CustomPopups.StorageDownloadFailed )			
										end
									
									-- user storage
									elseif PopupGUI.task == UIEnums.NetTaskType.UserStorage then
										-- start the download task
										if PopupGUI.state == UIEnums.NetTaskState.Ready then
											-- no point downloading it if its valid
											local downloaded = NetServices.UserProfileDownloaded()
											if downloaded == true then
												print( "Storage Download : Profile already downloaded" )
												PopupGUI.state = UIEnums.NetTaskState.Complete
											else
												local result = NetServices.StartProfileDowload()
												
												if result ~= UIEnums.StorageReaderState.InProgress then
													print( "Storage Download : Start Profile Failed " )
													PopupGUI.state = UIEnums.NetTaskState.Failed
													UIGlobals.StorageFailType = UIEnums.StorageFailType.UserFailed
												else
													PopupGUI.state = UIEnums.NetTaskState.Busy
												end
											end
											
										-- pump the task
										elseif PopupGUI.state == UIEnums.NetTaskState.Busy then
											local state = NetServices.ContinueProfileDownload()
							
											if state == UIEnums.StorageReaderState.Failed then
												print( "Storage Download : Profile Failed " )
												PopupGUI.state = UIEnums.NetTaskState.Failed
												UIGlobals.StorageFailType = UIEnums.StorageFailType.UserFailed									
												
											elseif state == UIEnums.StorageReaderState.Corrupted then
												print( "Storage Download : Profile Corrupted " )
												PopupGUI.state = UIEnums.NetTaskState.Failed
												UIGlobals.StorageFailType = UIEnums.StorageFailType.UserCorrupted		
												
											elseif state == UIEnums.StorageReaderState.VersionError then
												print( "Storage Download : Profile Version Error " )
												PopupGUI.state = UIEnums.NetTaskState.Failed
												UIGlobals.StorageFailType = UIEnums.StorageFailType.UserCorrupted								
												
											elseif state == UIEnums.StorageReaderState.FileDoesNotExist then
												print( "Storage Download : Profile new user " )
												PopupGUI.state = UIEnums.NetTaskState.Complete -- fine to continue. We are a new user
												StartAsyncSave()
											
											elseif state == UIEnums.StorageReaderState.Successful then
												print( "Storage Download : Profile Complete " )
												PopupGUI.state = UIEnums.NetTaskState.Complete
												NetServices.ApplyDownloadedProfile() -- need to make the changes take effect
											end
											
										-- task failed
										elseif PopupGUI.state == UIEnums.NetTaskState.Failed then
											PopupSpawn( UIEnums.CustomPopups.StorageDownloadFailed )			

										-- task complete
										elseif PopupGUI.state == UIEnums.NetTaskState.Complete then
											if PopupGUI.Timer >= 1.5 then -- must show a popup for at least 2 seconds
												PopupCancel()
												
												-- callback function
												if UIGlobals.StorageDownloadCallback ~= nil then
													UIGlobals.StorageDownloadCallback( true )
												end
											end
										end
									end
								end,						  
		},		
		[UIEnums.CustomPopups.StorageDownloadFailed] = 
		{
			darken			= true,
			title_text_ID	= UIText.POP_TITLE_WARNING,
			message_text_ID	= UIText.POP_STORAGE_DOWNLOAD_FAILED,
			
			next_text_ID	= UIText.INFO_A_OK,
			no_back			= true,
			
			end_CB			=	function()
									-- [WillB] #37705 - Need to re-enable carousel after failure
									if IsFunction( EnableCarousel ) == true then
										EnableCarousel( true )
									end
								end,
			
			init_CB			=	function()
									local messages =
									{
										[UIEnums.StorageFailType.UserFailed]	= UIText.POP_STORAGE_DOWNLOAD_FAILED,
										[UIEnums.StorageFailType.UserCorrupted] = UIText.POP_STORAGE_DOWNLOAD_CORRUPTED,
										[UIEnums.StorageFailType.TitleFailed]	= UIText.POP_STORAGE_DOWNLOAD_FAILED,
									}
									
									--Bug 37624, allow cancel to pass TCR #030
									if UIGlobals.StorageFailType == UIEnums.StorageFailType.UserCorrupted then
										CustomPopup.CurrentData.no_back = false
										CustomPopup.CurrentData.back_text_ID = UIText.INFO_B_CANCEL
									end
									
									CustomPopup.CurrentData.message_text_ID = messages[UIGlobals.StorageFailType]
									NetServices.ResetMultiplayerProfile() -- call to clear up any data in memory
									
									-- clean up any game invite processing
									UIGlobals.Network.GameInviteState = UIEnums.MpGameInviteState.Polling
									NetServices.ClearGameInvite( )
									net_FlushEverything( ) 
								end,
								
			next_CB			=	function()
									-- still allow the player into xbox live since they will have a fresh profile
									if UIGlobals.StorageFailType == UIEnums.StorageFailType.UserCorrupted then
										if UIGlobals.StorageDownloadCallback ~= nil then
											UIGlobals.StorageDownloadCallback( true )
											StartAsyncSave()
										end
									else
										-- cant enter xbox live without a profile or latest TMS data
										if UIGlobals.StorageDownloadCallback ~= nil then
											UIGlobals.StorageDownloadCallback( false )
										end
									end
								end,
		},	
		[UIEnums.CustomPopups.FriendDemandNoFriendsChosen] = 
		{
			darken			= true,
			no_back			= true,

			title_text_ID	= UIText.POP_FRIEND_DEMAND,
			message_text_ID	= UIText.POP_FRIEND_DEMAND_NO_FRIENDS_CHOSEN,

			next_text_ID	= UIText.INFO_A_OK,
		},
		[UIEnums.CustomPopups.LostLobbyConnection] =
		{
			darken			= true,
			no_back			= true,
			
			next_text_ID	= UIText.INFO_A_OK,
			
			title_text_ID	= UIText.POP_TITLE_ALERT,
			message_text_ID	= UIText.POP_LOST_CONNECTION_LOBBY,
		},
		[UIEnums.CustomPopups.DisconnectFromFacebook] =
		{
			darken			= true,
			
			title_text_ID	= UIText.POP_TITLE_FACEBOOK,
			message_text_ID	= UIText.POP_FACEBOOK_UN_REGISTERED,
			
			next_text_ID	= UIText.INFO_YES_A,
			back_text_ID	= UIText.INFO_NO_B,

			next_CB			=	function()
									UIGlobals.fb_disconnecting=true
									Amax.FacebookUnRegister()
									PopupSpawn( UIEnums.CustomPopups.DisconnectingFromFacebook )
								end,
		},		
		[UIEnums.CustomPopups.DisconnectingFromFacebook] =
		{
			show_progress	= true,

			darken			= true,
			no_back			= true,
			no_next			= true,
			
			title_text_ID	= UIText.POP_TITLE_FACEBOOK,
			message_text_ID	= UIText.POP_FACEBOOK_UN_REGISTERING,
		},
		[UIEnums.CustomPopups.FacebookDisconnected] =
		{
			darken			= true,
			no_back			= true,
			
			next_text_ID	= UIText.INFO_A_OK,
			
			title_text_ID	= UIText.POP_TITLE_FACEBOOK,
			message_text_ID	= UIText.POP_FACEBOOK_DISCONNECTED,
		},
		[UIEnums.CustomPopups.TrophiesNotInstalled] =
		{
			darken			= true,
			
			back_text_ID	= UIText.INFO_B_CANCEL,
			no_next			= true,
			
			show_progress	= true,
			
			title_text_ID	= UIText.POP_TITLE_PLEASE_WAIT,
			message_text_ID	= UIText.POP_TROPHIES_NOT_INSTALLED,
			
			update_CB		= function()
								local finished = Amax.TrophiesInstalled()
								
								if finished == true then
									PopupCancel()
								end
							end
		},
		[UIEnums.CustomPopups.GarageFirstModUnlocked] =
		{
			darken			= true,
			no_back			= true,
			
			next_text_ID	= UIText.INFO_A_OK,
			
			title_text_ID	= UIText.POP_TITLE_INFO,
			message_text_ID	= UIText.POP_GARAGE_FIRST_MOD_UNLOCKED,
		},
		[UIEnums.CustomPopups.ReconnectToLeaderboards] =
		{
			darken			= true,
			no_back			= true,
			no_next			= true,
			colour_style	= nil,
			icon_name		= nil,
			show_progress	= true,
			
			title_text_ID	= UIText.POP_TITLE_PLEASE_WAIT,
			message_text_ID	= UIText.CMN_NOWT,
			
			init_CB			=	function()
									CustomPopup.CurrentData.darken = UIGlobals.NetConnectionDarken
							
									PopupGUI.FinishedCreatingNetworkSession = false
									PopupGUI.reading_leaderboard = false
									PopupGUI.server_timeout = 0
									PopupGUI.end_timer = 1.5
									
									if Profile.PadProfileOnline( Profile.GetPrimaryPad() ) == true then									
										CustomPopup.CurrentData.message_text_ID = UIText.RBC_SERVER_CONNECTION_ONLINE
										
										if UIGlobals.SuccessfullyCreatedNetworkSession == false then
											Amax.StartCreateStatsOnlyMatchingSession()
										end
	
										PopupGUI.State = UIEnums.PopupState.Busy
									else
										CustomPopup.CurrentData.message_text_ID = UIText.RBC_SERVER_CONNECTION_OFFLINE	
										PopupGUI.State = UIEnums.PopupState.Failed								
									end
									
								end,

			update_CB		=	function( time )
									if PopupGUI.State == UIEnums.PopupState.Busy then
									
										if Profile.PadProfileOnline( Profile.GetPrimaryPad() ) ~= true then
											PopupGUI.State = UIEnums.PopupState.Failed
											return
										end
										
										if PopupGUI.FinishedCreatingNetworkSession == false and UIGlobals.SuccessfullyCreatedNetworkSession == false then
										
											PopupGUI.FinishedCreatingNetworkSession, UIGlobals.SuccessfullyCreatedNetworkSession = Amax.ContinueCreateStatsOnlyMatchingSession()
											print("SUCCESSFULLY CREATED NETWORK SESSION", UIGlobals.SuccessfullyCreatedNetworkSession )
										
										elseif UIGlobals.SuccessfullyCreatedNetworkSession == true then
											
											if UIGlobals.NetworkSessionStarted == false then

												UIGlobals.NetworkSessionStarted = Amax.StartPlayStatsOnlyMatchingSession()											
												print("Network Session Started: ", UIGlobals.NetworkSessionStarted )
												
												--begin to read the leaderboard
												if UIGlobals.NetworkSessionStarted == true then
													PopupGUI.reading_leaderboard = true
													print("Reading the scoreboard")
													Amax.StartFriendScoreboardRead( 0 )
												end
											end
											
											if UIGlobals.NetworkSessionStarted == false then
												PopupGUI.State = UIEnums.PopupState.Failed
												return	
											else
											
												if PopupGUI.reading_leaderboard == true then
													local complete, rows, row_me, rank_me = Amax.ScoreboardReadComplete()

													PopupGUI.server_timeout = PopupGUI.server_timeout + time

													if PopupGUI.server_timeout > 15 then														
														PopupGUI.State = UIEnums.PopupState.Failed
														return
													end
													
													if complete == true then
														PopupGUI.reading_leaderboard = false
														print("Finished Reading Leaderboard", rows, row_me, rank_me)
														UIGlobals.SPPostRaceLeaderboardDone = false
														PopupGUI.State = UIEnums.PopupState.Finished
														CustomPopup.CurrentData.message_text_ID = UIText.RBC_COMPLETED
														if UIGlobals.server_connection.callback ~= nil then
															UIGlobals.server_connection.callback( true )
														end
													end		
												end	
																						
											end
										else
											PopupGUI.State = UIEnums.PopupState.Failed
											return
										end				
												
									elseif PopupGUI.State == UIEnums.PopupState.Failed then
										
										print("Failed")
										PopupSpawn(UIEnums.CustomPopups.SPLeaderboardPostRaceError)										

									elseif PopupGUI.State == UIEnums.PopupState.Finished then
											
										PopupGUI.end_timer = PopupGUI.end_timer - time
										if PopupGUI.end_timer <= 0 then
											PopupCancel()
										end	
																			
									end
								end,
		},
		[UIEnums.CustomPopups.FacebookAuthenticating] =
		{
			show_progress	= true,

			darken			= true,
			no_back			= true,
			no_next			= true,
			
			title_text_ID	= UIText.POP_TITLE_FACEBOOK,
			message_text_ID	= UIText.POP_PLEASE_WAIT,
		},
		[UIEnums.CustomPopups.TwitterAuthenticating] =
		{
			show_progress	= true,

			darken			= true,
			no_back			= true,
			no_next			= true,
			
			title_text_ID	= UIText.POP_TITLE_TWITTER,
			message_text_ID	= UIText.POP_PLEASE_WAIT,
		},
		[UIEnums.CustomPopups.FailedAgeCheck] =
		{
			darken			= true,
			no_back			= true,
			
			next_text_ID	= UIText.INFO_A_OK,
			
			title_text_ID	= UIText.POP_TITLE_INFO,
			message_text_ID	= UIText.RBC_SERVER_CONNECTION_FAILED_AGE_CHECK,
		},				
		[UIEnums.CustomPopups.FailedWriteToLeaderboards] =
		{
			darken			= true,
			no_back			= true,
			colour_style	= "Main_Black",
			intensity		= 0.6,
			
			next_text_ID	= UIText.INFO_A_OK,
			
			title_text_ID	= UIText.POP_TITLE_ERROR,
			message_text_ID	= UIText.POP_CONTENT_SERVER_GENERAL_ERROR,
			x_text_ID		= UIText.INFO_TRY_RECONNECT,

			message_CB		=	function( message,arg1,arg2 )
									if message == UIEnums.Message.ButtonX then
										PlaySfxNext()
										PopupSpawn( UIEnums.CustomPopups.RetryWriteToLeaderboards )											
									end
								end,
		},
		[UIEnums.CustomPopups.RetryWriteToLeaderboards] =
		{
			darken			= true,
			no_back			= true,
			no_next			= true,
			colour_style	= nil,
			icon_name		= nil,
			show_progress	= true,
			
			title_text_ID	= UIText.POP_TITLE_PLEASE_WAIT,
			message_text_ID	= UIText.CMN_NOWT,
			
			init_CB			=	function()
			
									Amax.StartSPScoreboardWrite()
									PopupGUI.State = UIEnums.PopupState.Busy									
									CustomPopup.CurrentData.message_text_ID = UIText.RBC_SERVER_CONNECTION_ONLINE
									PopupGUI.TimeOut = 15
									
								end,

			update_CB		=	function( time )
			
									if PopupGUI.State == UIEnums.PopupState.Busy then									
										
										PopupGUI.TimeOut = PopupGUI.TimeOut - time
									
										if  Amax.SPScooreboardWriteSuccess( ) == false or PopupGUI.TimeOut <= 0 then
											PopupGUI.State = UIEnums.PopupState.Failed
										end										
									
										if Amax.SPScoreboardWriteComplete( true ) == true then
											UIGlobals.SPPostRaceLeaderboardDone = true																			
											PopupGUI.State = UIEnums.PopupState.Finished										
										end											
												
									elseif PopupGUI.State == UIEnums.PopupState.Failed then
									
											PopupSpawn( UIEnums.CustomPopups.FailedWriteToLeaderboards )
											UIGlobals.SPPostRaceLeaderboardDone = true							
										
									elseif PopupGUI.State == UIEnums.PopupState.Finished then	
																		
											UIGlobals.LeaderboardReTryPassed = true
											PopupCancel()			
																			
									end
								end,
		},
		[UIEnums.CustomPopups.FriendDemandRemovedFriend] =
		{
			darken			= true,
			no_back			= true,
			
			next_text_ID	= UIText.INFO_A_OK,
			
			title_text_ID	= UIText.POP_TITLE_INFO,
			message_text_ID	= UIText.RBC_REMOVED_FRIEND,
		},		
		[UIEnums.CustomPopups.FriendDemandNotFriend] =
		{
			darken			= true,
			no_back			= true,

			next_text_ID	= UIText.INFO_A_OK,

			title_text_ID	= UIText.POP_TITLE_INFO,
			message_text_ID	= UIText.RBC_FC_NOT_FRIEND,
		},

		-- [Blur2PC] retail Blur PC (ported from the Blur.exe 1.2 bytecode): keep the new video settings?
		-- Reverts after 10 seconds without an answer. The message is the GAME_OPTION_COUNTDOWN<n> variable
		-- text (POP_CONFIRM_VIDEO_SETTINGS with the seconds left).
		[UIEnums.CustomPopups.ConfirmVideoSettings] =
		{
			darken			= true,

			next_text_ID	= UIText.INFO_YES_A,
			back_text_ID	= UIText.INFO_NO_B,

			title_text_ID	= UIText.POP_TITLE_ALERT,
			message_text_ID	= UIText.CMN_NOWT,

			init_CB			= function()
								PopupGUI.timer = 10
								CustomPopup.CurrentData.message_text_ID = "GAME_OPTION_COUNTDOWN" .. PopupGUI.timer
							end,

			update_CB		= function( time )
								UIButtons.ChangeText( SCUI_Popup.name_to_id.message, "GAME_OPTION_COUNTDOWN" .. math.ceil( PopupGUI.timer ) )
								PopupGUI.timer = PopupGUI.timer - time
								if PopupGUI.timer <= 0 then
									PopupGUI.timer = 0
									PopupCancel()
									OptionsGraphics_RevertSettings()
								end
							end,

			next_CB			= function()
								OptionsGraphics_BackupSettings()
							end,

			back_CB			= function()
								OptionsGraphics_RevertSettings()
							end,
		},

		-- [Blur2PC] retail Blur PC (ported from the Blur.exe 1.2 bytecode, T146): Esc / B on the start
		-- screen asks to quit; Yes closes the game window.
		[UIEnums.CustomPopups.KillApplication] =
		{
			darken			= true,

			title_text_ID	= UIText.POP_TITLE_ALERT,
			message_text_ID	= UIText.POP_QUIT_MSG,

			next_text_ID	= UIText.INFO_YES_A,
			back_text_ID	= UIText.INFO_NO_B,

			next_CB			= function()
								UISystem.SystemQuitGame( true )
							end,
		},
	},
}

function SetupCustomPopup( index, callback )
	if UIScreen.IsPopupActive() == true then
		PopupSpawn( index )
	else
		if index == nil then
			print("SetupCustomPopup : Error, the index passed was a nil")
			CustomPopup.CurrentData = CustomPopup.Data[ UIEnums.CustomPopups.Default ]
			CustomPopup.CurrentType = UIEnums.CustomPopups.Default
			CustomPopup.Callback	= nil
		else
			CustomPopup.CurrentData = CustomPopup.Data[ index ]
			CustomPopup.CurrentType = index
			CustomPopup.Callback	= callback
		
			if CustomPopup.CurrentData == nil then
				print("SetupCustomPopup : Error index was invalid")
				CustomPopup.CurrentData = CustomPopup.Data[ UIEnums.CustomPopups.Default ]
				CustomPopup.CurrentType = UIEnums.CustomPopups.Default
			end
		end
		
		-- Store where I was called from
		CustomPopup.CallingGUI = GUI
		CustomPopup.CallingSCUI = SCUI

		UIScreen.AddPopup("TheCustomPopup.lua")
	end
end

function PopupSpawn( id )
	if IsNumber( id ) == false then
		print("PopupSpawn : Error id not a number" )
		return
	end

	-- disable it
	if UIGlobals.UserKickBackMode ~= UIEnums.UserKickBackMode.None then
		print("PopupSpawn : Disabled, Returning to start screen." )
		return
	end
	
	PopupGUI.spawn_next = id

	UIScreen.CancelPopup()
end

function PopupCancel( next_screen, subscreen, launch_screen, context )
	PopupGUI.cancel = true
	PopupGUI.finished = true
	PopupGUI.next_screen = next_screen
	PopupGUI.go_subscreen = subscreen
	PopupGUI.go_launch_screen = launch_screen
	PopupGUI.context = context
	PopupGUI.timeout_timer = 0
end
