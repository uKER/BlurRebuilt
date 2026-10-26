GUI =
{
	finished = false,
	DisableLoading = false,
	RaceLoaded = false,

	DelayLoadTimer = 0,
	DelayLoadTime = 0.5,
	LoadStarted = false,

	MessageTextIDs = {},
	MessageSettings =
	{
		StartTime	= -2,	-- start delay when wall changes
		EndTime		= 2,	-- time to wait at the end before resetting
		MinTime		= 2.5,	-- min display time
		MaxTime		= 6,	-- max display time
		Delta		= 0.16,	-- time per char
		TimeLimit	= 0,	-- time until next post is shown
		Timer		= 0,	-- current time

		CurrentPost	= 1,
		NumberPosts = 0,
	},


	CanExit = function( _GUI )
		return false
	end,
}

function Init()
	if UIGlobals.LoadFromDebug == false then
		Profile.AllowProfileChanges( true )
		Profile.ActOnProfileChanges( false )
	end
	
	UIScreen.SetScreenTimers(0,0)

	-- Clear ingame tempories
	UIGlobals.Ingame = { }
	ClearMultiplayerGlobals()
			
	AddSCUI_Elements( )
	SetupInfoLine()

	-- determine the current game mode
	show_table( UIEnums.GameMode )
	print( "Current Game Mode", Amax.GetGameMode() )

	GUI.RacerList = SCUI.name_to_id[ "racers_list" ]
	GUI.MessageList = SCUI.name_to_id[ "messages_list" ]

	local racers = nil
	local messages = nil
	local event = nil
	
	Amax.SetLoadStateIntoRace()

	if Amax.GetGameMode() == UIEnums.GameMode.Online then
		--racers = NetRace.GetLobbyMembers()
	
	elseif Amax.GetGameMode() == UIEnums.GameMode.Debug then
		event = Amax.GetDebugEvent()

		UIButtons.SetActive( SCUI.name_to_id[ "target" ], false )
		UIButtons.SetActive( SCUI.name_to_id[ "racers" ], false )

		for i = 1, 3 do
			UIButtons.SetActive( SCUI.name_to_id[ "objective_" .. i ], false )
		end
	end
	
	if Amax.AmaxWorldLayerExists() == true then
		print("LoadingGame : Waiting to dump amax world layer")
		Amax.SendMessage(UIEnums.GameFlowMessage.DumpLevel)
		GUI.LevelDumped = false
	else
		GUI.LevelDumped = true
	end
	
	if UIGlobals.LaunchMode == UIEnums.LaunchMode.Demo then
		local turn_it_off = 
		{
			"racers",
			"target",
			"objective_1",
			"objective_2",
			"objective_3",
		}
	
		for i,v in ipairs( turn_it_off ) do
			UIButtons.SetActive( SCUI.name_to_id[ v ], false )
		end
	end	
end

function PostInit()
	-- add bleep bleep
	AddTransmitter( Screen.safe.left + 32, 424, 32, 32, UIEnums.Panel._2DAA )
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
	
	if message == UIEnums.GameFlowMessage.LevelDumped then
		-- Finished waiting for dump
		print("LoadingGame : World layer dumped")
		GUI.LevelDumped = true

	elseif message == UIEnums.GameFlowMessage.RaceLoaded then
		print("LoadingGame : Race loaded")
		Amax.FillStreamer()
		GUI.RaceLoaded = true
	end	
end

function FrameUpdate( time )

	-- need to hang the load to let the groups end animation finish
	if GUI.LevelDumped == true then
		if GUI.LoadStarted == false then 
			GUI.DelayLoadTimer = GUI.DelayLoadTimer + time

			if GUI.DelayLoadTimer >= GUI.DelayLoadTime then
				GUI.LoadStarted = true
				StartLoad()
			end
		end
	end

	if GUI.RaceLoaded == true then
		if Amax.WaitForStreamer() == false then
			GUI.RaceLoaded = false
			
			local next_screen = "EndScreen"

			if Amax.GetGameMode() == UIEnums.GameMode.SinglePlayer then
				next_screen = "Ingame\\IntroVista.lua"
				
			elseif Amax.GetGameMode() == UIEnums.GameMode.Online then
				next_screen = "Multiplayer\\Ingame\\MpWaitingForPlayers.lua"

			elseif Amax.GetGameMode() == UIEnums.GameMode.Debug then
				next_screen = "Ingame\\HUD.lua"
				Amax.SendMessage(UIEnums.GameFlowMessage.RaceStarted)
			end
			
			if GUI.DisableLoading == false then
				UIGlobals.IsLoading = false
				UIGlobals.IsIngame = true

				Amax.SendMessage( UIEnums.GameFlowMessage.StartGameRendering )
				StartIngameMusic()
			end

			-- pass 0 so that main.lua is closed if we are running on a sub context
			GoScreen( next_screen , 0)
		end
	end

	if GUI.MessageSettings.NumberPosts > 0 and GUI.MessageSettings.CurrentPost <= GUI.MessageSettings.NumberPosts then
		GUI.MessageSettings.Timer = GUI.MessageSettings.Timer + time

		if GUI.MessageSettings.Timer > GUI.MessageSettings.TimeLimit then
			GUI.MessageSettings.Timer = 0
			GUI.MessageSettings.TimeLimit = Clamp( GUI.MessageSettings.Delta * UIButtons.GetStaticTextLength( GUI.MessageTextIDs[GUI.MessageSettings.CurrentPost] ), GUI.MessageSettings.MinTime, GUI.MessageSettings.MaxTime )

			GUI.MessageSettings.CurrentPost = GUI.MessageSettings.CurrentPost + 1
			UIButtons.SetSelectionByIndex( GUI.MessageList, GUI.MessageSettings.CurrentPost - 1 )
			print( "next post", GUI.MessageSettings.CurrentPost - 1 )
		end
	end
end

function Render()
	--
end

function EndLoop( time )
	--
end

function End()
	
	if	UIGlobals.SuccessfullyCreatedNetworkSession == true then
		Amax.StartPlayStatsOnlyMatchingSession()
		UIGlobals.NetworkSessionStarted = true;
		print("Started The Network Session")
	end

	if UIGlobals.LoadFromDebug == false then
		Profile.ForceProfileUpdate()
		Profile.ActOnProfileChanges( true )	
	end
end
