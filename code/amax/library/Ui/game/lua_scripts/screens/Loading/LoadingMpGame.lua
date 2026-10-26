GUI =
{
	finished = false,
	DisableLoading = false,

	timer = 0,
	show_time = 0.28,

	tip_timer = 0,
	tip_change_time = 10,
	disconnect_time_out = 20,

	RaceLoaded = false,
	SentReady = false,
	DisconnectedIdlePlayers = true, -- [WillB] - Stop the disconnection during load from running. We can control the load time due to hard drive installs, PC spec etc.
	Disconnecting = false,

	LoadTimer = 0,
	LoadStarted = false,
}

function Init()
	if UIGlobals.LoadFromDebug == false then
		Profile.AllowProfileChanges( true )
		Profile.ActOnProfileChanges( false )
		UIGlobals.ActUponFriendChallenges = false
	end
	
	AddSCUI_Elements()
	SetupInfoLine( UIText.INFO_NEXT_TIP_RT )

	net_LockoutFriendsOverlay( true )
	
	if IsSplitScreen() == true then
		Splitscreen_ClearMessages()
	end
	
	UIScreen.SetScreenTimers( 0, 1 )

	Amax.SetLoadStateIntoRace()
	
	local race = Multiplayer.GetCurrentRace()
	local t = Amax.GetAnimatedCameraParams( UIEnums.AnimatedCameraSequenceType.RouteVista, race.route )
	if IsTable( t ) == true then
		UIButtons.ChangePosition( SCUI.name_to_id[ "VistaDestination" ], XtToScreenSpaceX( t.px, true ), XtToScreenSpaceY( t.py, true )  )
	end
	UIButtons.DummyLerpMatrixCopy( SCUI.name_to_id[ "VistaDestination" ], SCUI.name_to_id[ "VistaDummy" ] )
end

function PostInit()
	AddLoadingSegs()
	
	local race = Multiplayer.GetCurrentRace()
	local overview_id = SCUI.name_to_id["border"]
	Mp_RefreshRaceOverview( overview_id, race, Multiplayer.GetCurrentRaceIndex(), false )

	-- clone the background image
	if UIGlobals.IsQuickRestart == false then
		Amax.DeleteTextureBundle()
	end

	UIButtons.ChangeTexture( { filename = "LOADING_TEXTURE" }, 1, UIButtons.FindChildByName( overview_id, "thumbnail" ) )
	
	UpdateRouteShapes( "Shp_Route", 4, race.city, race.route, "Shp_StartLine" )

	-- turn the clone off. You have been... ERASED!
	UIButtons.SetActive( SCUI.name_to_id[ "toggle_branch" ], false )
	
	LoadingMp_HelpTextInit()
	LoadingMp_HelpTextNext()
end

function StartLoop( time )
	--
end

function MessageUpdate( message, arg1, arg2, arg3, arg4 )
	if SCUI_MessageUpdate( message, arg1, arg2, arg3, arg4 ) == true then
		return
	end

	if GUI.finished == true then
		return
	end
	
	if message == UIEnums.Message.ButtonRightTrigger then
		LoadingMp_HelpTextNext()
	end

	if message == UIEnums.GameFlowMessage.LevelDumped then
		Amax.SendMessage( UIEnums.GameFlowMessage.StartRaceLoad )
		print( "LEVEL DUMPED - START LOAD" )

	elseif message == UIEnums.GameFlowMessage.RaceLoaded then
		Amax.SetGlobalFade(1)
		Amax.FillStreamer()
		Amax.ExitNetworkLoad()
		
		GUI.RaceLoaded = true
		print( "RACE LOADED - FILLING STREAMER" )
	end	
end

function FrameUpdate( time )
	LoadingMp_HelpTextUpdate( time )
	
	if GUI.DisableLoading == true then
		return
	end

	-- TODO : remove once ASync loading works properly
	-- to stop the main thread blocking the screen transitions
	GUI.LoadTimer = GUI.LoadTimer + time
	if GUI.LoadStarted == false and GUI.LoadTimer > 1 then
		Amax.EnterNetworkLoad() -- no network calls/structures should be referenced between the and ExitNetworkLoad()

		-- stop the game rendering just in case
		Amax.SendMessage(UIEnums.GameFlowMessage.StopGameRendering)

		if Amax.AmaxWorldLayerExists() == true then
			print( "FrontendLobby load - DUMP LEVEL" )
			Amax.SendMessage(UIEnums.GameFlowMessage.DumpLevel)
		else
			print( "PostRaceLobby load - START RACE" )
			Amax.SendMessage(UIEnums.GameFlowMessage.StartRaceLoad)		
		end

		GUI.LoadStarted = true
		ClearMultiplayerGlobals()
	end
	
	-- catch the local player disconnecting and flag it
	local disconnecting = NetRace.LocalPlayerDisconnecting()
	if disconnecting == true then
		GUI.Disconnecting = disconnecting
	end
	
	-- dont let the logic update until we have loaded the race and we arnt filling the streamer or trying to disconnected because we
	-- lost connection to the lsp / xbox live
	if GUI.RaceLoaded == true and Amax.WaitForStreamer() == false then

		UIGlobals.IsLoading = false -- race has now loaded so flag this back
		
		-- incase a game invite is pending
		if NetServices.GameInvitePending() == true then
			UIGlobals.IsIngame = true -- this will allow networkbank to run the game invite logic
			return
		end

		-- network games need to sync up with peers
		if IsSplitScreen() == true then
			LeaveScreen()
		else
			if GUI.Disconnecting == false then
				-- inform peers that we have loaded
				if GUI.SentReady == false then
					LoadingMpGame_SendReady()

				-- wait until everybody else has loaded
				elseif NetRace.RaceStartSynchronised() == true then
					LoadingMpGame_StartRace()

				-- incase remote player loading threads have stalled. We will request them to be disconnected
				elseif GUI.DisconnectedIdlePlayers == false then
					GUI.disconnect_time_out = GUI.disconnect_time_out - time
					if GUI.disconnect_time_out <= 0 then
						LoadingMpGame_DisconnectIdlePlayers()
					end
				end
				
				-- [WillB] #37236 - check we are still online. There is a fringe case that you can sign out of PSN on the lobby at 0 and then the ui will ignore the "Sign out" message 
				-- because it came during the load and the system is suppose to store them until after the load.
				if UserKickBackActive() == false then
					if Profile.PadProfileOnline( Profile.GetPrimaryPad() ) == false then
						ServiceConnectionLost() -- safe to call for non online games as its checked internally
					
					elseif NetServices.ConnectionStatusIsOnline() == false then
						XLSPConnectionLost() -- safe to call for non online games as its checked internally
					end
				end
			end
		end
	end
end

function LoadingMpGame_SendReady()
	print( "LoadingMpGame.lua : PLAYER READY SENT" )

	if UIGlobals.LoadFromDebug == false then
		Profile.ForceProfileUpdate()
		Profile.ActOnProfileChanges( true )	
		UIGlobals.ActUponFriendChallenges = true
	end

	NetRace.EnterSyncAfterRaceLoad()
	GUI.SentReady = true
	UIGlobals.IsIngame = true
end

function LoadingMpGame_StartRace()
	print("LoadingMpGame.lua : EVERYONES READY! LETS GO!")

	NetRace.StartPlay( )
	LeaveScreen()
end

function LoadingMpGame_DisconnectIdlePlayers()
	print("LoadingMpGame.lua : TIME OUT - Requesting disconnection of idle players!")
	
	NetRace.DisconnectIdleLoadingPlayers()
	GUI.DisconnectedIdlePlayers = true
end

function Render()
	--
end

function EndLoop( time )
	--
end

function LeaveScreen()
	GoScreen( "Ingame\\IntroVista.lua" )
end

function EnterEnd()
	UIButtons.SetActive( SCUI.name_to_id[ "hint_text" ], false )
	UIButtons.TimeLineActive( "Hide_BottomHelp", true )
end

function End()
	if GUI.DisableLoading == true then
		return
	end

	UIGlobals.IsIngame = true
	UIGlobals.IsLoading = false
	UIGlobals.IsQuickRestart = false

	Amax.SendMessage( UIEnums.GameFlowMessage.StartGameRendering )
	
	-- 21869 : OMFG Audio team say we can't start the music and stop it again on the same frame [sigh]
	if UserKickBackActive() == false then
		StartIngameMusic()
	end

	if IsSplitScreen() == true then
		if UIGlobals.LoadFromDebug == false then
			Profile.ForceProfileUpdate()
			Profile.ActOnProfileChanges( true )	
			UIGlobals.ActUponFriendChallenges = true
		end
	end
end

function SetupToggle( icon, icon_sheet, text, colour )
	local root_id = UIButtons.CloneXtGadgetByName( "Loading\\LoadingMpGame.lua", "toggle_branch" )

	local icon_id	= UIButtons.FindChildByName( root_id, "toggle_icon" )
	local name_id	= UIButtons.FindChildByName( root_id, "toggle_text" )

	UIShape.ChangeSceneName( icon_id, icon_sheet )
	UIShape.ChangeObjectName( icon_id, icon )

	UIButtons.ChangeText( name_id, text )
	UIButtons.ChangeColour( icon_id, colour )

	return root_id
end

function LoadingMp_HelpTextInit()
	local tips_table = Select( IsSplitScreen(), UIGlobals.ls_help_text_ss_ids, UIGlobals.ls_help_text_ids )
	
	if tips_table ~= nil and #tips_table > 1 then
		return
	end
	
	if IsSplitScreen() == true then
		UIGlobals.ls_help_text_ss_ids = {  }
	else	
		UIGlobals.ls_help_text_ids = {  }
	end
	
	UIGlobals.ls_num_help = 100
	
	tips_table = Select( IsSplitScreen(), UIGlobals.ls_help_text_ss_ids, UIGlobals.ls_help_text_ids )
	
	for i = 1, UIGlobals.ls_num_help, 1 do
		tips_table[ #tips_table + 1 ] = Select( IsSplitScreen(), UIText[ "MLHP_SS_" .. i ], UIText[ "MHLP_" .. i ] )
	end
end

function LoadingMp_HelpTextNext()
	local tips_table = Select( IsSplitScreen(), UIGlobals.ls_help_text_ss_ids, UIGlobals.ls_help_text_ids )
	
	local next_text_id = table.remove( tips_table, 1 ) --math.random( #tips_table ) )
	local text_id = SCUI.name_to_id[ "hint_text" ]
	
	UIButtons.ChangeText( text_id, next_text_id )
	UIButtons.TimeLineActive( "hint_text_next", true, 0 )
	
	-- Scale bar to fit text
	local x, y = UIButtons.GetSize( text_id )
	local num_lines = UIButtons.GetStaticTextHeight( text_id ) / y
	UIButtons.ChangeScale( SCUI.name_to_id[ "motd_dummy" ], 1, (num_lines) + 0.3 )
	
	if #tips_table == 0 then
		LoadingMp_HelpTextInit()
	end
	
	PlaySfxToggle()
	
	GUI.tip_timer = 0
end

function LoadingMp_HelpTextUpdate( time )
	GUI.tip_timer = GUI.tip_timer + time
	
	if GUI.tip_timer >= GUI.tip_change_time then
		LoadingMp_HelpTextNext()
	end
end
