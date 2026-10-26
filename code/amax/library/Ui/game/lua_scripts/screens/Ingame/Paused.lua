GUI =
{
	finished = false,
	-- Carousel branches
	branch_name = 
	{
		[UIEnums.RwGameBranch.Paused]	= "Paused",
		[UIEnums.RwGameBranch.Retry]	= "Retry",
		[UIEnums.RwGameBranch.Help]		= "Help",
		[UIEnums.RwGameBranch.Options]	= "Options",
		[UIEnums.RwGameBranch.Photo]	= "Photo",
		[UIEnums.RwGameBranch.Quit]		= "Quit",
		[UIEnums.RwGameBranch.ReturnToLobby] = "ReturnToLobby",
	},
	help_text = 
	{
		[UIEnums.RwGameBranch.Paused]	= UIText.CMN_CSLHELP_PAUSED,
		[UIEnums.RwGameBranch.Retry]	= UIText.CMN_CSLHELP_RETRY,
		[UIEnums.RwGameBranch.Help]		= UIText.CMN_CSLHELP_HELP,
		[UIEnums.RwGameBranch.Options]	= UIText.CMN_CSLHELP_OPTIONS,
		[UIEnums.RwGameBranch.Photo]	= UIText.CMN_CSLHELP_PHOTO_MODE,
		[UIEnums.RwGameBranch.Quit]		= UIText.CMN_CSLHELP_QUIT,
		[UIEnums.RwGameBranch.ReturnToLobby] = UIText.CMN_CSLHELP_RETURN_TO_LOBBY,
	},
	locked_table = 
	{
	},	
	quit_and_win = false,
	going_photo = false,
	resume_game = false,
	wait_retry = false,
	
	-- Carousel controls		
	active_context = UIEnums.Context.CarouselApp,
	active_selection = -1,
	over_selection = -1,
	
	-- Local functions
	ResumeGame = function( )
		GUI.resume_game = true
		UIButtons.TimeLineActive( "start", false )
		UIButtons.TimeLineActive( "end_bg", true )
		GoScreen("Ingame\\HUD.lua")
	end,
}

function Init()
	Splitscreen_AddSplits()
	
	SetupMenuOptions()
	UnlockController()
	
	-- disable in online and system link
	if IsSplitScreen() == true then
		GUI.locked_table[ #GUI.locked_table+1 ] = UIEnums.RwGameBranch.Photo
		UIButtons.ChangeText( SCUI.name_to_id["RetryTitle"], UIText.MP_SS_RETRY )

	elseif IsNetworkRace() == true then
		GUI.locked_table[ #GUI.locked_table+1 ] = UIEnums.RwGameBranch.Photo
		GUI.locked_table[ #GUI.locked_table+1 ] = UIEnums.RwGameBranch.Retry
	end
	
	if NetRace.CanReturnPlayersToLobby() == false then
		GUI.locked_table[ #GUI.locked_table+1 ] = UIEnums.RwGameBranch.ReturnToLobby
	end
		
	AddSCUI_Elements( )

	GUI.quit_and_win = UIGlobals.DevMode
	--in debug remove the quit and win, as we have nothing to win
	if Amax.GetGameMode() == UIEnums.GameMode.Debug or GUI.network_race == true then
		GUI.quit_and_win = false
	end
	-- Remove button if not available
	UIButtons.SetActive(SCUI.name_to_id[ "QuitAndWin" ],GUI.quit_and_win)
	
	GUI.network_race = IsNetworkRace( )
	
	GUI.carousel_id = SCUI.name_to_id[ "Carousel" ]

	UIScreen.SetScreenTimers( 0.3, 0.6 )

	-- Don't allow un-pause in the retry menu
	if UIGlobals.Ingame.RetryMenu == true then
		GUI.locked_table[ #GUI.locked_table+1 ] = UIEnums.RwGameBranch.Paused
		GUI.locked_table[ #GUI.locked_table+1 ] = UIEnums.RwGameBranch.Photo
		
		UIButtons.SetSelection( GUI.carousel_id, 1 )
	end

	UIGlobals.IsQuickRestart = false
	
	-- friends overlay is enabled for the duration of the pause screen.
	net_LockoutFriendsOverlay( false )

	GUI.camera_id = UIButtons.CloneXtGadgetByName( "SCUIBank", "Cam_Carousel" )
	UIButtons.SetParent( GUI.camera_id, SCUI.name_to_id[ "CameraDolly" ], UIEnums.Justify.MiddleCentre )
	
	local statusBarId = UIButtons.CloneXtGadgetByName( "SCUIBank", "Dmy_StatusBar" )
	UIButtons.SetParent( statusBarId, GUI.camera_id, UIEnums.Justify.MiddleCentre )
	
	UIScreen.AddMessageNow( UIEnums.GameMessage.SetLocalPlayerIndex, player_index )
end

function PostInit()
	if UIEnums.CurrentPlatform == UIEnums.Platform.PC then
		-- [Blur2PC] Keep pause-menu dimming exactly viewport-sized at any aspect.
		UIButtons.ChangeSize( SCUI.name_to_id[ "DarkBG" ], SCREEN_WIDTH, 480 )
		-- [Blur2PC] The GoLoad transition uses its own purple cover.
		UIButtons.ChangeSizeX( SCUI.name_to_id[ "LoadingBG" ], SCREEN_WIDTH )
	end

	local titles = 
	{
		[UIEnums.RwGameBranch.Paused]		= UIText.CMN_CSL_PAUSED,
		[UIEnums.RwGameBranch.Retry]		= UIText.CMN_CSL_RETRY,
		[UIEnums.RwGameBranch.Help]			= UIText.CMN_CSL_HELP,
		[UIEnums.RwGameBranch.Options]		= UIText.CMN_CSL_OPTIONS,
		[UIEnums.RwGameBranch.Photo]		= UIText.CMN_CSL_PHOTO_MODE,
		[UIEnums.RwGameBranch.Quit]			= UIText.CMN_CSL_QUIT,
		[UIEnums.RwGameBranch.ReturnToLobby]= UIText.CMN_CSL_RETURN_TO_LOBBY,
	}
	local shapes = 
	{
		[UIEnums.RwGameBranch.Paused]		= "pause",
		[UIEnums.RwGameBranch.Retry]		= "retry inverted",
		[UIEnums.RwGameBranch.Help]			= "help",
		[UIEnums.RwGameBranch.Options]		= "option cog",
		[UIEnums.RwGameBranch.Photo]		= "photo",
		[UIEnums.RwGameBranch.Quit]			= "start",
		[UIEnums.RwGameBranch.ReturnToLobby]= "end_of_race",
	}
	
	-- This is the order items are added
	local order = 
	{
		UIEnums.RwGameBranch.Paused,
		UIEnums.RwGameBranch.Retry,
		UIEnums.RwGameBranch.ReturnToLobby,
		UIEnums.RwGameBranch.Help,
		UIEnums.RwGameBranch.Options,
		UIEnums.RwGameBranch.Photo,
		UIEnums.RwGameBranch.Quit,
	}

	-- Build carousel
	GUI.node_id = { }
	for _,i in ipairs( order ) do
		if CarouselIsAppLocked( i ) ~= true then
			local addApp = true
			local branch_id = UIButtons.CloneXtGadgetByName( "SCUIBank", "CarouselBranch" )

			-- Title
			UIButtons.ChangeText( UIButtons.FindChildByName( branch_id, "Title" ), titles[ i ] )
			
			-- Shape
			UIShape.ChangeObjectName( UIButtons.FindChildByName( branch_id, "Shape" ), shapes[ i ] )
			
			-- Attach
			UIButtons.SetParent( branch_id, GUI.carousel_id, UIEnums.Justify.None )
			GUI.node_id[ i ] = branch_id
		end
	end

	-- Scan and assign values
	Amax.CarouselScan( GUI.carousel_id )
	for i,v in pairs(GUI.branch_name) do
		if IsNumber( GUI.node_id[ i ] ) == true then
			UIButtons.SetNodeValue( GUI.node_id[ i ], i )
		end
	end
	
	-- This one starts on
	local init_selection = UIEnums.RwGameBranch.Paused
	UIButtons.PrivateTimeLineActive( GUI.node_id[ init_selection ], "Init", true, 1.0, true  )
	CarouselSetApp( GUI, SCUI, init_selection, true )


	-- Set to UI camera panel	
	UIButtons.ChangePanel( GUI.carousel_id, UIEnums.Panel._3DAA_LIGHT, true )

	GUI.help_text_id, GUI.bottom_help_id = SetupBottomHelpBar()
end

function StartLoop( time )
	--
end

function MessageUpdate( message, arg1, arg2, arg3, arg4 )
	if SCUI_MessageUpdate( message, arg1, arg2 ) == true then
		return
	end
	
	if SubScreenActive() == true or GUIBank.loading == true then
		return
	end

	if GUI.wait_retry == true then
		if message == UIEnums.GameFlowMessage.LevelDumped then
			UIGlobals.Ingame = { }
			StartGameLoad()
		end
	end
	
	if GUI.wait_retry == true then
		return
	end
	
	if GUI.finished == true then
		return
	end

	if GUI.active_selection ~= -1 then
		if message == UIEnums.Message.MenuBack then
			local selection = GUI.active_selection
			if CloseApp( selection ) == true then
				UIButtons.PrivateTimeLineActive( GUI.node_id[ selection ], "Open", false, 2 )
				UIButtons.TimeLineActive( "light_fade", false )
			end
		end		
	elseif (message == UIEnums.Message.ButtonStart or message == UIEnums.Message.MenuBack) and arg2 == true and UIGlobals.Ingame.RetryMenu ~= true then
		net_LockoutFriendsOverlay( true )
		GUI.ResumeGame()
		PlaySfxBack()
	elseif message == UIEnums.Message.ButtonX and arg2 == true then
		-- DEBUG : Quit and win
		if GUI.quit_and_win == true then
			net_LockoutFriendsOverlay( true )
			GoScreen( "Debug\\QuitandWin.lua" )
			UIButtons.TimeLineActive( "status_bar", false )
		end
	elseif message == UIEnums.Message.MenuNext and arg2 == true then
		local ftime, selection = UIButtons.GetSelection( GUI.carousel_id, 2 )

		if CarouselIsAppLocked( selection ) == true then
			PlaySfxError()
		else
			if UIScreen.GetScreenActive( GUI.active_context ) == true then
				return
			end

			PlaySfxNext()
			UIButtons.PrivateTimeLineActive( GUI.bottom_help_id, "Hide_BottomHelp", true )
			

			if selection == UIEnums.RwGameBranch.Paused and UIGlobals.Ingame.RetryMenu ~= true then 
				-- Paused
				net_LockoutFriendsOverlay( true )
				GUI.ResumeGame()				
			elseif selection == UIEnums.RwGameBranch.Retry then 
				-- Retry
				if IsSplitScreen() == true then
					SetupCustomPopup( UIEnums.CustomPopups.SsRetry )

				elseif GUI.network_race == false then
					SetupCustomPopup( UIEnums.CustomPopups.RetryRace )
				end
			elseif selection == UIEnums.RwGameBranch.Options then 

				UIButtons.TimeLineActive( "HelpFade", false )
				UIButtons.TimeLineActive( "light_fade", true )
				UIButtons.TimeLineActive( "stage1", true )
				
				-- Options
				GUI.active_selection = UIEnums.RwGameBranch.Options
				UIScreen.SetNextScreen("Shared\\Options.lua", GUI.active_context )
				UIButtons.SetSelected( GUI.carousel_id, false )
			elseif selection == UIEnums.RwGameBranch.Help then 

				UIButtons.TimeLineActive( "HelpFade", false )
				UIButtons.TimeLineActive( "light_fade", true )
				UIButtons.TimeLineActive( "stage1", true )
				
				-- Help
				GUI.active_selection = UIEnums.RwGameBranch.Help
				UIScreen.SetNextScreen("Shared\\Help.lua", GUI.active_context )
				UIButtons.SetSelected( GUI.carousel_id, false )
			elseif selection == UIEnums.RwGameBranch.Photo then 
				-- Photo
--				UIButtons.TimeLineActive( "start", false )
				UIButtons.TimeLineActive( "end_bg", true )
				
				net_LockoutFriendsOverlay( true )
				
				GUI.going_photo = true
				UIScreen.SetNextScreen("Ingame\\PhotoMode.lua")
			elseif selection == UIEnums.RwGameBranch.Quit then
				-- Quit
				SetupCustomPopup( UIEnums.CustomPopups.ExitRace )
			elseif selection == UIEnums.RwGameBranch.ReturnToLobby then
				SetupCustomPopup( UIEnums.CustomPopups.ReturnToLobby )
			end
		end
	end
	
	if message == UIEnums.Message.PopupNext and arg2 == UIEnums.CustomPopups.RetryRace and arg3 == UIEnums.PopupOptions.Yes then

		if Amax.SP_IsStreetRaceFD() == true or Amax.SP_IsDestructionFD() == true or Amax.SP_IsCheckpointFD() == true or Amax.SP_IsFanDemandFD() == true or Amax.SP_IsBossBattleFD() == true then
			FriendDemand.ReAttempt()

			Paused_EnableCarousel( false )

			GameProfile.RetryingEvent()
			StopIngameMusic()
			
			Amax.SendMessage( UIEnums.GameFlowMessage.StopGameRendering )
			Amax.SendMessage( UIEnums.GameFlowMessage.StopGameUpdate )

			UIGlobals.IsQuickRestart = true
			
			UIGlobals.Ingame = {  }
			GoLoadingScreen( "Loading\\LoadingSpGame.lua" )

		elseif UIGlobals.LoadFromDebug == true and IsTable( UIGlobals.Sp ) == false then
			-- just restart - no mini-garage
			Paused_EnableCarousel( false )
			Paused_TransmitterOn()
			
			GameProfile.RetryingEvent()
			StopIngameMusic()
			
			Amax.SendMessage( UIEnums.GameFlowMessage.StopGameRendering )
			Amax.SendMessage( UIEnums.GameFlowMessage.StopGameUpdate )

			UIGlobals.IsQuickRestart = true
			
			UIGlobals.Ingame = {  }
			--StartGameLoad()
			GoLoadingScreen("Loading\\LoadingGame.lua")
		else
			UIGlobals.GarageMiniFromPause = true
			PushScreen( "Shared\\GarageMini.lua" )
			StartAsyncSave()
		end

	elseif message == UIEnums.Message.PopupNext and arg2 == UIEnums.CustomPopups.SsRetry and arg3 == UIEnums.PopupOptions.Yes then
		if IsSplitScreen() == true then
			Amax.SendMessage( UIEnums.GameFlowMessage.QuitRace )
			Amax.SendMessage( UIEnums.GameFlowMessage.StopGameRendering )
			Amax.SendMessage( UIEnums.GameFlowMessage.StopGameUpdate )

			UIGlobals.IsQuickRestart = true
			UIGlobals.Ingame = { }

			StopIngameMusic()
			GoLoadingScreen( "Loading\\LoadingMpGame.lua" )
		end
	end
end

function FrameUpdate( time )
	if GUI.finished == true then
		return
	end
	
	-- Catch the race finished message and store it
	if IsSplitScreen() == false and Multiplayer.RaceFinished() == true then
		GoScreen("Multiplayer\\Ingame\\MpFinished.lua")
	end
	
	local ftime, selection = UIButtons.GetSelection( GUI.carousel_id, 2 )
	
	if selection ~= GUI.over_selection then 
		local name = GUI.branch_name[ selection ]
		
		if GUI.over_selection ~= -1 then
			UIButtons.PrivateTimeLineActive( GUI.node_id[ GUI.over_selection ], "Selected", false )
		end
		UIButtons.PrivateTimeLineActive( GUI.node_id[ selection ], "Selected", true, 4.7 )
		
		-- Update help text
		if GUI.active_selection == -1 then
			UIButtons.TimeLineActive( "HelpFade", true, 0 )
		end

		local help_text = UIText.RBU_LOCKED
		local is_locked = CarouselIsAppLocked( selection )
		if is_locked == false then
			 help_text = GUI.help_text[ selection ]
		end
		
		UIButtons.ChangeText( GUI.help_text_id, help_text )
		
		-- Change the colours of objects : TODO lerp these colours
		if GUI.over_selection ~= -1 then
			CarouselSetApp( GUI, SCUI, GUI.over_selection, false )
		end
		
		-- New
		CarouselSetApp( GUI, SCUI, selection, true )
		
		GUI.over_selection = selection
	end
end

function Render()
	--
end

function EnterEnd( )
	UIButtons.TimeLineActive( "sc_end_",  true )
	UIButtons.TimeLineActive( "HelpFade", false )
	
	if GUI.resume_game == false and GUI.going_photo == false then
		UIButtons.TimeLineActive( "GoLoad", true )
	end
end

function EndLoop( time )
	--
end

function End()
	EndScreen( UIEnums.Context.CarouselApp )
	--
	Amax.SendMessage(UIEnums.GameFlowMessage.UnPause)

	if GUI.resume_game == true then
		Camera_UseInGame()
	end
end

function IsNetworkRace( )
	local data  = {}
	data = Amax.GetLevelData( )
	return data.NetworkRace
end

function Paused_EnableCarousel( enable )
	UIButtons.SetSelected( ContextTable[ UIEnums.Context.Main ].GUI.carousel_id, enable )
end

function Paused_TransmitterOn()
	UIButtons.SetActive( ContextTable[ UIEnums.Context.Main ].SCUI.name_to_id[ "Txt_Loading" ], true, true )
end

function CarouselOpenFinished( )
	-- Sorcery safe
	local _GUI = ContextTable[ UIEnums.Context.Main ].GUI
	
	UIButtons.SetSelected( _GUI.carousel_id, true )

	UnlockController()
	
	-- Turn fades on
	for i,v in pairs(GUI.branch_name) do
		UIButtons.TimeLineActive( "fade_" .. v, true )
	end
end