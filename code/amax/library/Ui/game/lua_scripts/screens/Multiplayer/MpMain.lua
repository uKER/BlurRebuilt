GUI =
{
	finished = false,
	
	-- Carousel controls		
	active_context = UIEnums.Context.CarouselApp,
	active_selection = -1,
	over_selection = -1,
	
	launched_screen = false,
	
	branch_name = 
	{
		[UIEnums.RwMpBranch.Online]			= "MpOnline",
		[UIEnums.RwMpBranch.Splitscreen]	= "MpSplitscreenSignIn",
		[UIEnums.RwMpBranch.Options]		= "Options",
		[UIEnums.RwMpBranch.Quit]			= "Quit",
		[UIEnums.RwMpBranch.Help]			= "Help",
		[UIEnums.RwMpBranch.Stickers]		= "Stickers",
		[UIEnums.RwMpBranch.Lan]			= "MpLan",
	},
	help_text = 
	{
		[UIEnums.RwMpBranch.Online]			= UIText.RBU_TABHELP_ONLINE,
		[UIEnums.RwMpBranch.Splitscreen]	= UIText.RBU_TABHELP_PARTY_MODE,
		[UIEnums.RwMpBranch.Options]		= UIText.RBU_TABHELP_OPTIONS,
		[UIEnums.RwMpBranch.Quit]			= UIText.RBU_TABHELP_QUIT,
		[UIEnums.RwMpBranch.Help]			= UIText.RBU_TABHELP_HELP,
		[UIEnums.RwMpBranch.Stickers]		= UIText.RBU_TABHELP_STICKERS,
		[UIEnums.RwMpBranch.Lan]			= UIText.RBU_TABHELP_SYSTEM_LINK,
	},
	
	ShowUnrecoverableError = false,
	ShowRouterError = false,
	Ready =	false, -- used for game invites and party invites

	CarouselActive = false,
	
	node_id = {  },
}

function Init()
	net_LockoutFriendsOverlay( false )
	
	AddSCUI_Elements()

	if UIGlobals.Network.GameInviteState == UIEnums.MpGameInviteState.Initialising then
		UIGlobals.Multiplayer.LaunchScreen = UIEnums.MpLaunchScreen.None
	end
	
	if UIGlobals.Multiplayer.LaunchScreen == UIEnums.MpLaunchScreen.MultiplayerLobby or UIGlobals.Multiplayer.LaunchScreen == UIEnums.MpLaunchScreen.SplitscreenLobby then
		Amax.ChangeUiCamera( UIGlobals.CameraNames.MpLobby, 0, 0 )
	elseif UIGlobals.Multiplayer.LaunchScreen ~= UIEnums.MpLaunchScreen.None then
		Amax.ChangeUiCamera( UIGlobals.CameraNames.MpAppBase, 0, 0 )
	else
		Amax.ChangeUiCamera( UIGlobals.CameraNames.MpCarousel, 0, 0 )
	end

	if UIGlobals.Multiplayer.LaunchScreen == UIEnums.MpLaunchScreen.None then
		net_SetRichPresence(UIEnums.RichPresence.MpMain)
	end

	CheckRenderBuffer()
	LockController()
	UIScreen.SetScreenTimers( 0.3, 0.3 )
	
	if UIGlobals.Network.UnrecoverableRouterError == true then
		-- lost connection to platform servers. 
		GUI.ShowRouterError = true
	elseif UIGlobals.Network.UnrecoverableSessionError == true then
		-- some bad s**t has happened with the matching sessions so show error and flush network data
		GUI.ShowUnrecoverableError = true
	end

	UIGlobals.InitialLobby = true
	GUINetwork.load_started = false		-- if a host forces a peer to return to the party lobby this won't be reset until now.
	
	GUI.carousel_id = SCUI.name_to_id[ "Carousel" ]
	
	-- Set platform specific strings
	SetupInfoLine( UIText.INFO_OPEN_A, UIText.INFO_QUIT_B )
	
	EnableCarousel( false )

	GUI.camera_id = UIButtons.CloneXtGadgetByName( "SCUIBank", "Cam_Carousel" )
	UIButtons.SetParent( GUI.camera_id, SCUI.name_to_id[ "CameraDolly" ], UIEnums.Justify.MiddleCentre )

	-- Status bar
	local statusBarId = AddStatusBar( true )
	UIButtons.SetParent( statusBarId, GUI.camera_id, UIEnums.Justify.MiddleCentre )
end

function PostInit()
	local init_selection = UIEnums.RwMpBranch.Online

	-- Help text bottom bar	
	GUI.help_text_id, GUI.bottom_help_id = SetupBottomHelpBar()
	UIButtons.ChangeText( GUI.help_text_id, GUI.help_text[ init_selection ] )

	local titles = 
	{
		[UIEnums.RwMpBranch.Online]			= UIText.MP_ONLINE_PLATFORM,
		[UIEnums.RwMpBranch.Splitscreen]	= UIText.CMN_PARTY_MODE,
		[UIEnums.RwMpBranch.Options]		= UIText.RBC_OPTIONS,
		[UIEnums.RwMpBranch.Quit]			= UIText.CMN_CSL_QUIT,
		[UIEnums.RwMpBranch.Help]			= UIText.RBC_HELP,
		[UIEnums.RwMpBranch.Stickers]		= UIText.RBC_STICKERS,
		[UIEnums.RwMpBranch.Lan]			= UIText.MP_LAN,
	}
	local shapes = 
	{
		[UIEnums.RwMpBranch.Online]			= "groups",
		[UIEnums.RwMpBranch.Splitscreen]	= "split_screen",
		[UIEnums.RwMpBranch.Options]		= "option cog",
		[UIEnums.RwMpBranch.Quit]			= "start",
		[UIEnums.RwMpBranch.Help]			= "help",
		[UIEnums.RwMpBranch.Stickers]		= "sticker",
		[UIEnums.RwMpBranch.Lan]			= "system_link",
	}
	local scenes = 
	{
		[UIEnums.RwMpBranch.Splitscreen]	= "fe_icons",
		[UIEnums.RwMpBranch.Lan]			= "fe_icons",
	}	
	-- This is the order items are added
	local order = 
	{
		UIEnums.RwMpBranch.Online,
		UIEnums.RwMpBranch.Splitscreen,
		UIEnums.RwMpBranch.Options,
		UIEnums.RwMpBranch.Quit,
		UIEnums.RwMpBranch.Help,
		UIEnums.RwMpBranch.Stickers,
		UIEnums.RwMpBranch.Lan,
	}
	
	-- Build carousel
	for _,i in ipairs( order ) do
		local addApp = true
		local branch_id = UIButtons.CloneXtGadgetByName( "SCUIBank", "CarouselBranch" )
		
		-- Title
		UIButtons.ChangeText( UIButtons.FindChildByName( branch_id, "Title" ), titles[ i ] )
		
		-- Shape
		local shape_id = UIButtons.FindChildByName( branch_id, "Shape" )
		if IsString( scenes[ i ] ) == true then
			UIShape.ChangeSceneName(shape_id, scenes[ i ] )
		end
		UIShape.ChangeObjectName( shape_id, shapes[ i ] )
		
		-- Attach
		UIButtons.SetParent( branch_id, GUI.carousel_id, UIEnums.Justify.None )
		GUI.node_id[ i ] = branch_id
	end
	
	-- Scan and assign values
	Amax.CarouselScan( GUI.carousel_id )
	for i,v in pairs(GUI.branch_name) do
		if IsNumber( GUI.node_id[ i ] ) == true then
			UIButtons.SetNodeValue( GUI.node_id[ i ], i )
		end
	end
	
	-- Hack - scale down the PlayStation(R)Network so maintain width doesn't degrade the text
	if UIEnums.CurrentPlatform == UIEnums.Platform.PS3 then
		UIButtons.ChangeScale( UIButtons.FindChildByName( GUI.node_id[ UIEnums.RwMpBranch.Online ], "Title" ), 0.7, 0.7 )
	end

	-- This one starts on
	UIButtons.PrivateTimeLineActive( GUI.node_id[ init_selection ], "Init", true, 1.0, true  )
	CarouselSetApp( GUI, SCUI, init_selection, true )
	
	-- Turn on 'new' Stickers
	if Amax.HasNewStickers() then
		UIButtons.SetActive( UIButtons.FindChildByName( GUI.node_id[ UIEnums.RwMpBranch.Stickers ], "NewIcon" ), true )
	end
	
	-- Hide if we're going somewhere else
	if UIGlobals.Multiplayer.LaunchScreen ~= UIEnums.MpLaunchScreen.None then
		UIButtons.PrivateTimeLineActive( GUI.bottom_help_id, "Hide_BottomHelp", true, 10.0, true )
		
		-- #33702 - Fix clunkyness when returning to a non-carousel screen
		UIButtons.PrivateTimeLineActive( GUI.carousel_id, "open_carousel", true, 10, true ) 
		CarouselOpenFinished()
	end

	FadeUpLoading()
end

function FrameUpdate( time )
	local ftime, selection = UIButtons.GetSelection( GUI.carousel_id, 2 )
	
	if selection ~= GUI.over_selection then 
		local name = GUI.branch_name[ selection ]
		-- Update help text
		if GUI.over_selection ~= -1 then
			UIButtons.PrivateTimeLineActive( GUI.node_id[ GUI.over_selection ], "Selected", false )
		end
		UIButtons.PrivateTimeLineActive( GUI.node_id[ selection ], "Selected", true, 4.7 )
		
		if GUI.launched_screen == false then
			UIButtons.TimeLineActive( "HelpFade", true, 0 )
		else
			GUI.launched_screen = false	-- reset
		end
		
		if IsNumber( GUI.help_text_id ) == true then
			local help_text = UIText.RBU_LOCKED
			local is_locked = CarouselIsAppLocked( selection )
			if is_locked == false then
				 help_text = GUI.help_text[ selection ]
			end
			UIButtons.ChangeText( GUI.help_text_id, help_text )
		end
		
		-- Change the colours of objects : TODO lerp these colours
		if GUI.over_selection ~= -1 then
			CarouselSetApp( GUI, SCUI, GUI.over_selection, false )
		end
		
		-- New
		CarouselSetApp( GUI, SCUI, selection, true )
		
		GUI.over_selection = selection
	end
end

function MessageUpdate( message, arg1, arg2, arg3, arg4 )
	
	if SCUI_MessageUpdate( message, arg1, arg2, arg3, arg4 ) == true then
		return
	end
	
	-- #34414
	if message == UIEnums.MiscMessage.ServiceConnectionLost or message == UIEnums.MiscMessage.ServiceConnectionEstablished then
		UIButtons.ChangeText( UIButtons.FindChildByName( GUIBank.MpStatusBar.branch_id, "ProfileName" ), "PROFILE_PAD_NAME" )
	end

	if IsControllerLocked() == true then
		return
	end
	
	if GUI.active_selection == -1 then

		if message == UIEnums.Message.PopupEnd and arg2 == UIEnums.CustomPopups.TrophiesNotInstalled and arg4 ~= 1 then
			OpenApp( UIEnums.RwMpBranch.Stickers )
			return
		end
		
		if message == UIEnums.Message.MenuNext and arg2 == true then
			local ftime, selection = UIButtons.GetSelection( GUI.carousel_id, 2 )

			if selection == UIEnums.RwMpBranch.Quit then
				SetupCustomPopup( UIEnums.CustomPopups.ExitMultiplayer )

			elseif selection == UIEnums.RwMpBranch.Online then
				RemoveBlaggedProfiles( )
				
				local ret = NetServices.CheckEntryRequirements( true )
				if ret == 0 then
					net_MpEnter( UIEnums.GameMode.Online )
					
					-- [WillB] #37499 TCR:R041 - Need to present the user with a message informing them 
					if NetServices.CanUseChat() == false then
						SetupCustomPopup( UIEnums.CustomPopups.PS3NoVoiceAllowed, MpMain_EnterOnline )
					else
						MpMain_EnterOnline()
					end
					
				elseif ret == 1 then
					SetupCustomPopup( UIEnums.CustomPopups.MultiplayerTooManyProfiles )
				elseif ret == 2 then
					SetupCustomPopup( UIEnums.CustomPopups.MultiplayerOnlineAccountRequired )
				elseif ret == 3 then
					SetupCustomPopup( UIEnums.CustomPopups.MultiplayerInvalidPrivileges )
				elseif ret == 4 then
					SetupCustomPopup( UIEnums.CustomPopups.MultiplayerOnlineConnectionLost )
				end
							
			elseif selection == UIEnums.RwMpBranch.Lan then
				RemoveBlaggedProfiles()
				
				-- must have a valid network connection to play on LAN
				if NetServices.NetworkConnectionActive() == false then
					SetupCustomPopup( UIEnums.CustomPopups.InvalidNetworkConnection )
				else
					local ret = NetServices.CheckEntryRequirements( false )
					if ret == 1 then
						SetupCustomPopup( UIEnums.CustomPopups.MultiplayerTooManyProfiles )
					else
						net_MpEnter( UIEnums.GameMode.SystemLink )
						net_StartServiceConnection( false, ConnectionCallback )
					end
				end

			elseif selection == UIEnums.RwMpBranch.Splitscreen then
				-- [Blur2PC] Retail marks a fresh sign-in so it can keep the primary input device.
				UIGlobals.splitscreen_enter = true
				net_MpEnter( UIEnums.GameMode.SplitScreen )
				OpenApp( selection )
				-- store this as this is the only pad we're allowed to save on in splitscreen
				if UIGlobals.splitscreen_primary_pad_original == -1 then
					UIGlobals.splitscreen_primary_pad_original = Profile.GetPrimaryPad()
				end
				
			elseif selection == UIEnums.RwMpBranch.Stickers then
				local installed = Amax.TrophiesInstalled()
				
				if installed == false then
					SetupCustomPopup( UIEnums.CustomPopups.TrophiesNotInstalled )
				else
					OpenApp( selection )
				end
				
			else -- options
				OpenApp( selection )
			end
		elseif message == UIEnums.Message.MenuBack and arg2 == true then
			SetupCustomPopup( UIEnums.CustomPopups.ExitMultiplayer )
		end
	else
		if message == UIEnums.Message.MenuBack and arg2 == true then
			local ftime, selection = UIButtons.GetSelection( GUI.carousel_id, 2 )
			
			-- Close  App
			if CloseApp( selection, nil, arg1 ) == true then
				Amax.ChangeUiCamera( UIGlobals.CameraNames.MpCarousel, UIGlobals.CameraLerpTime, 0 )
				net_SetRichPresence(UIEnums.RichPresence.MpMain)
			end
		end
	end
end

function EnterEnd( )
	for i,v in pairs( GUI.node_id ) do 
		if i ~= UIEnums.RwMpBranch.Quit then
			UIButtons.PrivateTimeLineActive( v, "End", true, 0 )
		else
			UIButtons.PrivateTimeLineActive( UIButtons.FindChildByName( v, "Title" ), "End", true, 0 )
			UIButtons.PrivateTimeLineActive( UIButtons.FindChildByName( v, "Shape" ), "End", true, 0 )
		end
	end
end

function End()
	--
	EndScreen( UIEnums.Context.CarouselApp )
end

-- called externally from a timeline
function MpMain_TabsOpen()
	if UIScreen.IsContextActive( UIEnums.Context.CarouselApp ) == false then
		UISystem.PlaySound( UIEnums.SoundEffect.TabsOpen )
	end
end

-- called externally from a timeline
function MpMain_TabsClose()
	if UIScreen.IsContextActive( UIEnums.Context.CarouselApp ) == false then
		UISystem.PlaySound( UIEnums.SoundEffect.TabsClose )
	end
end

-- This is used as a callback so be carefull what local variables you try to access here
function MpMain_EnterOnline()
	-- if its the PS3 and we arnt signed into to PSN, then the connection logic needs to be ran to force the PSN sign in.
	local start_service_connection = ( NetServices.ConnectionStatusIsOnline() == false )
	if UIEnums.CurrentPlatform == UIEnums.Platform.PS3 and Profile.PadProfileOnline( Profile.GetPrimaryPad() ) == false then
		start_service_connection = true
	end
					
	-- dont try and connect again if we already have a online connection to the demonware
	if start_service_connection == true then
		net_StartServiceConnection( true, ConnectionCallback )
	else
		net_StartStorageDownload( StorageCallback )
	end
end


function ConnectionCallback( result )
	local _GUI = ContextTable[ UIEnums.Context.Main ].GUI
	if result == true then
		local ftime, selection = UIButtons.GetSelection( _GUI.carousel_id, 2 )
		
		-- only download the storage data if entering xbox live
		if selection == UIEnums.RwMpBranch.Online then
			net_StartStorageDownload( StorageCallback )
		else
			OpenApp( selection )
		end
	end
end

function StorageCallback( result )
	local _GUI = ContextTable[ UIEnums.Context.Main ].GUI

	if result == true then
		local ftime, selection = UIButtons.GetSelection( _GUI.carousel_id, 2 )
		OpenApp( selection )
	end
end

function CarouselOpenFinished( )
	-- Sorcery safe
	local _GUI = ContextTable[ UIEnums.Context.Main ].GUI

	if ErrorMode() == true then
		if _GUI.ShowRouterError == true then
			_GUI.ShowRouterError = false
			SetupCustomPopup( UIEnums.CustomPopups.MultiplayerRouterError )
			
		elseif _GUI.ShowUnrecoverableError == true then
			_GUI.ShowUnrecoverableError = false
			SetupCustomPopup( UIEnums.CustomPopups.MultiplayerUnrecoverableError )
		end
		
		net_CloseAllSessions( )
		EnableCarousel( true )
	else

		-- could be a game invite
		if UIGlobals.Network.GameInviteState == UIEnums.MpGameInviteState.Initialising then
			net_StartStorageDownload()

		-- could be a return to party lobby
		elseif UIGlobals.Network.ReturnPartyLobbyState == UIEnums.MpReturnPartyLobbyState.Initialising then
			UIGlobals.Network.ReturnPartyLobbyState = UIEnums.MpReturnPartyLobbyState.PreProcessing

		-- might need to return to a screen
		elseif LaunchScreen( UIGlobals.Multiplayer.LaunchScreen ) == false then
			EnableCarousel( true )
			LeaveMultiplayerMode()
		end

		-- Were we forced here?
		if UIGlobals.IdlePlayerKicked == true then
			SetupCustomPopup( UIEnums.CustomPopups.IdlePlayerKicked )
		end
	end

	UnlockController()
	UIButtons.TimeLineActive( "fade_apps", true )
	GUI.Ready = true
end


function EnableCarousel( enable )
	-- Sorcery safe
	local _GUI = ContextTable[ UIEnums.Context.Main ].GUI
	
	-- [WillB] #37705 - Make this safe to be called from anywhere
	if IsTable( _GUI ) == false or IsNumber( _GUI.carousel_id ) == false then
		return
	end

	if enable ~= nil then
		UIButtons.SetSelected( _GUI.carousel_id, enable )
	end
	_GUI.CarouselActive = enable
end

function OpenApp( selection, context, screen_override )
	local _GUI = ContextTable[ UIEnums.Context.Main ].GUI
	local _SCUI = ContextTable[ UIEnums.Context.Main ].SCUI

	if UIScreen.GetScreenActive( _GUI.active_context ) == true then
		return
	end
	
	if context == nil then
		context = _GUI.active_context
	end

	-- Make safe to be called from Sorcery
	UIButtons.TimeLineActive( "HelpFade", false )
	local name = _GUI.branch_name[ selection ]
	if IsString( name ) == true then
		if screen_override ~= nil then
			UIScreen.SetNextScreen( screen_override, context )
		else
			--if selection == UIEnums.RwMpBranch.Options or selection == UIEnums.RwMpBranch.Twitter or selection == UIEnums.RwMpBranch.Facebook then
			if selection == UIEnums.RwMpBranch.Options or selection == UIEnums.RwMpBranch.Help or selection == UIEnums.RwMpBranch.Stickers then
				UIScreen.SetNextScreen( "Shared\\" .. name .. ".lua", context )
			else
				UIScreen.SetNextScreen( "Multiplayer\\" .. name .. ".lua", context )
			end
		end
		
		-- if we're viewing stickers, we may have a "new" icon to turn off
		if selection == UIEnums.RwMpBranch.Stickers then
			UIButtons.SetActive( UIButtons.FindChildByName( GUI.node_id[ UIEnums.RwMpBranch.Stickers ], "NewIcon" ), false )
		end

		PlaySfxNext()
	
		_GUI.active_selection = selection
		EnableCarousel( false )
		
		if selection == UIEnums.RwMpBranch.Options or selection == UIEnums.RwMpBranch.Help or selection == UIEnums.RwMpBranch.Online or selection == UIEnums.RwMpBranch.Splitscreen or selection == UIEnums.RwMpBranch.Lan then
			PlaySfxGraphicNext()
		end
	end
	
	UIButtons.PrivateTimeLineActive( _GUI.node_id[ selection ], "Open", true, 0 )
	UIButtons.PrivateTimeLineActive( _GUI.bottom_help_id, "Hide_BottomHelp", true )
end


function ForceCloseApp()
	-- Make safe to be called from elsewhere
	local _GUI = ContextTable[ UIEnums.Context.Main ].GUI
	local _SCUI = ContextTable[ UIEnums.Context.Main ].SCUI
	
	if IsNumber( _GUI.active_context ) == true then
		if IsTable( ContextTable[ _GUI.active_context ] ) == true then
			local ActiveGUI = ContextTable[ _GUI.active_context ].GUI
		
			if IsTable( ActiveGUI ) then
				ActiveGUI.finished = true
				
				-- [Bone] #36001 vanishing panels 
				if IsNumber( _GUI.active_selection ) and _GUI.active_selection ~= -1 then
					UIButtons.PrivateTimeLineActive( _GUI.node_id[ _GUI.active_selection ], "Open", false, 1, true )
				end				

				EndScreen( _GUI.active_context )
				PlaySfxBack()
				
				EndActive()
				FlushSubscreens() -- nuke the subscreens
				
				LeaveMultiplayerMode()
				
				-- #25152
				UIButtons.TimeLineActive( "HelpFade", true, 0 )
				
				-- [WillB] #35521
				Amax.ChangeUiCamera( UIGlobals.CameraNames.MpCarousel, 0, 0 )
				UIButtons.PrivateTimeLineActive( _GUI.bottom_help_id, "Hide_BottomHelp", false )
			end
		end
	end
end


function ErrorMode( )
	if GUI.ShowUnrecoverableError == true or GUI.ShowRouterError == true then
		return true
	end
	return false
end

function LeaveMultiplayerMode( )
	net_MpLeave( )
	net_FlushEverything( )

	Amax.SetGameMode( UIEnums.GameMode.MultiPlayer )
end


function LaunchScreen( launch_screen )
	local _GUI = ContextTable[ UIEnums.Context.Main ].GUI
	local result = false
	local screen_name = nil
	local branch = nil
	
	if launch_screen ~= UIEnums.MpLaunchScreen.None and launch_screen ~= UIEnums.MpLaunchScreen.SplitscreenLobby then
		
		-- open the correct screen on the active context
		if launch_screen == UIEnums.MpLaunchScreen.MultiplayerLobby then
			screen_name = "Multiplayer\\MpLobby.lua"
		elseif launch_screen == UIEnums.MpLaunchScreen.LanBrowser then
			screen_name = "Multiplayer\\MpLan.lua"
		else
			screen_name = "Multiplayer\\MpOnline.lua"
		end
		
		if Amax.GetGameMode() == UIEnums.GameMode.SystemLink then
			branch = UIEnums.RwMpBranch.Lan
		else
			branch = UIEnums.RwMpBranch.Online
		end
	
	elseif launch_screen == UIEnums.MpLaunchScreen.SplitscreenLobby then
		if Amax.GetGameMode() ~= UIEnums.GameMode.SplitScreen then
			print( "***Invalid GameMode***" )
			show_table( UIEnums.GameMode, 1 )
			return false
		end

		screen_name = "Multiplayer\\MpSplitscreenLobby.lua"
		branch = UIEnums.RwMpBranch.Splitscreen
	end

	-- load the correct screen
	if screen_name ~= nil and branch ~= nil then
		-- Start the 'set back' animation	
		UIButtons.SetSelection( _GUI.carousel_id, branch )
		UIButtons.PrivateTimeLineActive( _GUI.node_id[ branch ], "Open", true, 0 )

		-- Bug #27069 make sure carousel is disabled
		EnableCarousel( false )
		
		-- change selection and current screen
		-- Bug #37445 : Panels dissappear
		-- _GUI.active_selection = Amax.GetGameMode() -- TODO : fix me upper
		_GUI.active_selection = branch

		GoScreen( screen_name, _GUI.active_context )
		UIButtons.TimeLineActive( "HelpFade", false )
		_GUI.launched_screen = true
		result = true
		
		UIButtons.PrivateTimeLineActive( _GUI.bottom_help_id, "Hide_BottomHelp", true, 10.0, true )
	end

	-- reset
	UIGlobals.Multiplayer.LaunchScreen = UIEnums.MpLaunchScreen.None
	return result
end


-----------------------------------------------------------------------------------------
-- Party lobby helper functions
-----------------------------------------------------------------------------------------
