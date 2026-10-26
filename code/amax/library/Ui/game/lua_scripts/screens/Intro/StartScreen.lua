GUI =
{
	finished = false,
	loading = false,

	inactive_game_invite_wait = false,
	
	attract_timeout = 0,
	
	-- Screen times
	boot_start_time = 1.3,
	start_time		= 0.15,
	end_time		= 0.3,

	Mode = 
	{
		SinglePlayer	= 1,
		Multiplayer		= 2,
	},
	CurrentMode = 0,

	UiCameraName = 
	{
		UIGlobals.CameraNames.SpCarousel,
		UIGlobals.CameraNames.MpCarousel,
	},
	panel_items =
	{
		"_BoxGlow",
		"_Icon",
	},
	bright_panel_items = 
	{
		"_Text",
	},
	
	UiCameraTime = 0.5,

	debugdev_mode = true,
	
	entering_debug_menu = false,

	STAGE_PRESS_START	= 0,
	STAGE_LOADING		= 1,
	STAGE_MODE_SELECT	= 2,

	NetConnectionTaskCount = 0,

}

function Init()
	InfoLineFlush()	-- catch all start from scratch
	UIGlobals.OptionsTable = nil -- [PR] #37781 - clear options in case we sign out
	
	AddSCUI_Elements()
	SetupInfoLine( UIText.INFO_A_CONTINUE, UIText.INFO_QUIT_B )	-- [Blur2PC] retail Blur PC (T146)
	
	if UIGlobals.CurrentRegion == UIEnums.Region.NorthAmerica then
		UIButtons.ChangeText( SCUI.name_to_id[ "LegalText" ], UIText.CMN_LEGAL_US )
	end
	
	-- UIGlobals.ReturnMode = UIEnums.ReturnMode.QuitSpGame
	
	UIGlobals.ProfileBootFinished = false
	Camera_UseFrontend()

	Amax.SetGameMode( UIEnums.GameMode.Nothing )

	GUI.debugdev_mode = Amax.GetSetDebugDevMode()
	UIButtons.ChangeText( SCUI.name_to_id[ "DebugR" ], Select( GUI.debugdev_mode, UIText.DBG_DEVMODE_DEBUG, UIText.DBG_DEVMODE_MASTER ) )

	-- default state
	UIGlobals.Multiplayer.LaunchScreen = UIEnums.MpLaunchScreen.None
	UIGlobals.ConnectionType = UIEnums.ConnectionType.None
	
	GUI.allow_debug_buttons = ( UIGlobals.DevMode == true )
	UIButtons.SetActive( SCUI.name_to_id[ "DebugY" ], GUI.allow_debug_buttons )
	UIButtons.SetActive( SCUI.name_to_id[ "DebugL" ], GUI.allow_debug_buttons )
	UIButtons.SetActive( SCUI.name_to_id[ "DebugR" ], GUI.allow_debug_buttons )
	
	UIGlobals.ReturnToBlurb = nil
	
	--Make sure any single player stats sessions are deleted
	UIGlobals.FinishedCreatingNetworkSession = false; 
	UIGlobals.SuccessfullyCreatedNetworkSession = false;	
	Amax.DeleteStatsOnlyMatchingSession()
	--print("CLEANED UP AFTER NETWORK SESSION")
	
	-- reset this in case we get kicked
	UIGlobals.MpModShopKeyboardCheck = -1
	
	-- load in the default car if applicable
	local default_car_id, car_exists = Amax.GetShouldLoadDefaultCar()
	
	if default_car_id ~= nil and ( car_exists == false or UIGlobals.UserKickBackMode ~= UIEnums.UserKickBackMode.None ) then
		Amax.LoadCar( default_car_id, false, nil, nil, false )
	end
end

function StartScreen_SelectSp( no_sound )
	if GUI.CurrentMode == GUI.Mode.SinglePlayer then
		return
	end
	GUI.CurrentMode = GUI.Mode.SinglePlayer
	
	if no_sound ~= true then
		UISystem.PlaySound( UIEnums.SoundEffect.CarouselLeft )
	end
	
	Amax.ChangeUiCamera( GUI.UiCameraName[ GUI.CurrentMode ], 0.5, 0 )

	if UIGlobals.DevMode == true then
		UIButtons.SetActive( SCUI.name_to_id[ "DebugL" ], true )
	end
	
	for i,v in ipairs( GUI.panel_items ) do
		-- Turn off MP
		UIButtons.ChangeColour( SCUI.name_to_id[ "MP" .. v ], "Main_2" )
		-- Turn on SP
		UIButtons.ChangeColour( SCUI.name_to_id[ "SP" .. v ], "Main_1" )
	end

	for i,v in ipairs( GUI.bright_panel_items ) do
		-- Turn off MP
		UIButtons.ChangeColour( SCUI.name_to_id[ "MP" .. v ], "Main_2" )
		-- Turn on SP
		UIButtons.ChangeColour( SCUI.name_to_id[ "SP" .. v ], "Main_0" )
	end
	
	UIButtons.PrivateTimeLineActive( SCUI.name_to_id[ "SP_Icon" ], "Selected", true, 4.7, true )
	UIButtons.PrivateTimeLineActive( SCUI.name_to_id[ "MP_Icon" ], "Selected", false )

	SetupInfoLine( UIText.INFO_A_SELECT, UIText.INFO_B_BACK, "GAME_SHARE_BUTTON" )
end

function StartScreen_SelectMp( no_sound )
	if GUI.CurrentMode == GUI.Mode.Multiplayer then
		return
	end
	
	GUI.CurrentMode = GUI.Mode.Multiplayer
	
	if no_sound ~= true then
		UISystem.PlaySound( UIEnums.SoundEffect.CarouselRight )
	end
	
	Amax.ChangeUiCamera( GUI.UiCameraName[ GUI.CurrentMode ], 0.5, 0 )

	for i,v in ipairs( GUI.panel_items ) do
		-- Turn off SP
		UIButtons.ChangeColour( SCUI.name_to_id[ "SP" .. v ], "Main_2" )
		-- Turn on MP	
		UIButtons.ChangeColour( SCUI.name_to_id[ "MP" .. v ], "Main_1" )
	end
	
	UIButtons.PrivateTimeLineActive( SCUI.name_to_id[ "SP_Icon" ], "Selected", false )
	UIButtons.PrivateTimeLineActive( SCUI.name_to_id[ "MP_Icon" ], "Selected", true, 4.7, true )
	
	for i,v in ipairs( GUI.bright_panel_items ) do
		-- Turn off SP
		UIButtons.ChangeColour( SCUI.name_to_id[ "SP" .. v ], "Main_2" )
		-- Turn on MP	
		UIButtons.ChangeColour( SCUI.name_to_id[ "MP" .. v ], "Main_0" )
	end

	
	StartScreen_ChangeUiCamera( GUI.Mode.Multiplayer )
				
	if UIGlobals.DevMode == true then
		UIButtons.SetActive( SCUI.name_to_id[ "DebugL" ], false )
	end
	
	SetupInfoLine( UIText.INFO_A_SELECT, UIText.INFO_B_BACK, "GAME_SHARE_BUTTON" )
end


function PostInit()
	CheckRenderBuffer()

	if UIGlobals.ReturnMode == UIEnums.ReturnMode.QuitSpGame then
		-- Just backed out of single player
		GUI.current_stage = GUI.STAGE_MODE_SELECT
		
		UIScreen.SetScreenTimers( GUI.start_time, GUI.end_time )
		StartScreen_SelectSp( true )
		
		net_SetRichPresence( UIEnums.RichPresence.SignedIn )
		
	elseif UIGlobals.ReturnMode == UIEnums.ReturnMode.QuitMpGame then
		-- Just backed out of multiplayer
		GUI.current_stage = GUI.STAGE_MODE_SELECT
		UIScreen.SetScreenTimers( GUI.start_time, GUI.end_time )
		StartScreen_SelectMp( true )

		net_SetRichPresence( UIEnums.RichPresence.SignedIn )
		
	else
		-- From boot / user change kick
		GUI.fade_id = UIButtons.CloneXtGadgetByName( "SCUIBank", "Gfx_FadeUp" )
		Amax.ChangeUiCamera( "CameraInit", 0, 0 )
		Amax.ChangeUiCamera( "Start1a", 1, 0, 2 )
		
		GUI.current_stage = GUI.STAGE_PRESS_START
		
		Profile.SetPrimaryPad( -1 )
		Profile.Flush()
		
		StartScreen_ClearProfiles()
		StartScreen_DefaultProfileVars()
		
		Multiplayer.SplitscreenResetScores()
		UIGlobals.splitscreen_primary_pad_original = -1
		
		UISystem.PlaySound( UIEnums.SoundEffect.PressStartButtonScreen )
		
		net_SetRichPresence( UIEnums.RichPresence.Idle )

		GUI.CurrentMode = GUI.Mode.SinglePlayer
		UIScreen.SetScreenTimers( GUI.boot_start_time, GUI.end_time )
	end
	
	-- UserKickBackMode messages
	if UIGlobals.UserKickBackMode == UIEnums.UserKickBackMode.UserChanged then
		SetupCustomPopup( UIEnums.CustomPopups.ProfileKickedYou )
		
	elseif UIGlobals.UserKickBackMode == UIEnums.UserKickBackMode.MpBetaExpired then	
		SetupCustomPopup( UIEnums.CustomPopups.MpBetaExpired )
	
	elseif UIGlobals.UserKickBackMode == UIEnums.UserKickBackMode.InactiveGameInvite then
		SetupCustomPopup( UIEnums.CustomPopups.ActiveProfileChanged )
		GUI.inactive_game_invite_wait = true
		
	elseif UIGlobals.UserKickBackMode == UIEnums.UserKickBackMode.ProfileUploadFailed then
		SetupCustomPopup( UIEnums.CustomPopups.ProfileUploadFailed )
		
	elseif UIGlobals.UserKickBackMode == UIEnums.UserKickBackMode.XLSPConnectionLost then
		SetupCustomPopup( UIEnums.CustomPopups.NetConnectionError )
	
	elseif UIGlobals.UserKickBackMode == UIEnums.UserKickBackMode.ServiceConnectionLost then
		SetupCustomPopup( UIEnums.CustomPopups.ServiceConnectionLost )
		
	elseif UIGlobals.UserKickBackMode == UIEnums.UserKickBackMode.NetworkCableRemoved then
		SetupCustomPopup( UIEnums.CustomPopups.NetworkCableRemoved )
	end

	-- clear
	UIGlobals.ReturnMode = UIEnums.ReturnMode.None
	
	UIGlobals.UserKickBackMode = UIEnums.UserKickBackMode.None
	UIGlobals.UserKickBackActive = false
	CheckPumpkin()
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
		
	if GUI.entering_debug_menu == true then
		if message == UIEnums.GameFlowMessage.LevelDumped then
			GUI.entering_debug_menu = false
			Amax.SendMessage( UIEnums.GameFlowMessage.EnteredDebugMenu )
			Amax.SetGameMode( UIEnums.GameMode.Debug )
			GoScreen( "Debug\\DebugScreen.lua" )
		else
			return
		end
	end		
	
	if	message == UIEnums.Message.MenuNext or
		message == UIEnums.Message.MenuBack or
		message == UIEnums.Message.ButtonX or
		message == UIEnums.Message.ButtonY or
		message == UIEnums.Message.ButtonStart or
		message == UIEnums.Message.StickLeftX or
		message == UIEnums.Message.StickLeftY or
		message == UIEnums.Message.StickRightX or
		message == UIEnums.Message.StickRightY then
			GUI.attract_timeout	= 0
	end
	
	-- DEBUG
	if message == UIEnums.Message.ButtonRightShoulder and arg2 == true and GUI.allow_debug_buttons == true then
		if GUI.debugdev_mode == true then
			UIButtons.ChangeText(SCUI.name_to_id[ "DebugR" ], UIText.DBG_DEVMODE_MASTER)
			GUI.debugdev_mode = Amax.GetSetDebugDevMode(false)
		else
			UIButtons.ChangeText(SCUI.name_to_id[ "DebugR" ], UIText.DBG_DEVMODE_DEBUG)
			GUI.debugdev_mode = Amax.GetSetDebugDevMode(true)
		end
		
	elseif message == UIEnums.Message.ButtonY and arg2 == true and GUI.allow_debug_buttons == true then
		Profile.Setup( arg1, true )	-- just blagged them a profile
		UIGlobals.ProfileState[ arg1 ] = UIEnums.Profile.Blagged
		Profile.LockToPad( arg1 )
		Profile.SetPrimaryPad( arg1 )
		
		GameProfile.InitPrimary()
		
		Profile.AllowProfileChanges( false )
		Profile.ActOnProfileChanges( false )
		Profile.AllowAllPadInput( false )
		-- [Blur2PC] T230: apply the profile's controls (keyboard layout and pad configuration) to this slot;
		-- UI::SetupPlayerControls skips them while every pad drives the menus, as it did at InitPrimary above.
		GameProfile.ClearGameProfile( arg1, true )
		
		GUI.entering_debug_menu = true
		
		Amax.SendMessage(UIEnums.GameFlowMessage.DumpUI)
	end
	-- END DEBUG
	
	-- Handle each stage
	if GUI.current_stage == GUI.STAGE_PRESS_START then
		-- [Blur2PC] retail Blur PC (Blur.exe 1.2 bytecode, T146): back asks to quit the game, and only
		-- MenuNext continues. Esc sends both MenuBack and ButtonStart on PC, so the snapshot's ButtonStart
		-- test went forward on Esc.
		if message == UIEnums.Message.MenuBack then
			SetupCustomPopup( UIEnums.CustomPopups.KillApplication )
		elseif message == UIEnums.Message.MenuNext and UIScreen.IsContextActive( UIEnums.Context.LoadSave ) == false then
			UISystem.PlaySound( UIEnums.SoundEffect.Start )
			UISystem.PlaySound( UIEnums.SoundEffect.GraphicStartFlyForward )
			
			UIButtons.SetSelected( SCUI.name_to_id[ "OnShape" ], false )
			
			StartScreen_SelectSp( true )

			UIButtons.TimeLineActive( "Open", true )
			Sp_NewGame()
			Profile.Flush( )
			
			Amax.GameDataLogDisplaySettings()
			
			Amax.ChangeUiCamera( "Start1b", 8, 0 )

			-- [Blur2PC] retail Blur PC: keyboard and mouse (LastInputDevice 0 and 1) load pad 0's profile
			if UIGlobals.LastInputDevice < 2 then
				arg1 = 0
			end
			StartProfileLoad( arg1 )

			GUI.current_stage = GUI.STAGE_LOADING
			UIGlobals.ProfileBootFinished = false
			
			Profile.AllowProfileChanges( true )
			Profile.ActOnProfileChanges( false )
			Profile.AllowAllPadInput( false )
			
			net_SetRichPresence( UIEnums.RichPresence.SignedIn )

			Amax.InitTwitter()
			Amax.InitFacebook()
		end
	
	elseif GUI.current_stage == GUI.STAGE_MODE_SELECT then
		
		if message == UIEnums.Message.ButtonX and UIGlobals.DevMode == true then
			-- Debug - start single player with new profile data
			if selection == 0 then
				StartProfileNew(arg1)
				EnterSpRaceBook(true)
				StartAsyncSave()
			end

		elseif message == UIEnums.Message.ButtonLeftShoulder and arg2 == true then
			if Amax.CanUseShare() == true then
				UIGlobals.ShareFromWhatPopup=-1
				SetupCustomPopup( UIEnums.CustomPopups.SharingOptions )
			end
		
		elseif message == UIEnums.Message.PopupNext and arg2 == UIEnums.CustomPopups.SharingOptions then

			StoreScreen( UIEnums.ScreenStorage.FE_SOCIAL_NETWORK, "Intro\\StartScreen.lua" )

			if arg3 == UIEnums.ShareOptions.Facebook then
				Amax.CreateBlurb( UIEnums.SocialNetworkingItemType.PlayingBlur, 1, -1 )
				GoScreen( "Shared\\Facebook.lua", UIEnums.Context.Blurb )

			elseif arg3 == UIEnums.ShareOptions.Twitter then
				Amax.CreateBlurb( UIEnums.SocialNetworkingItemType.PlayingBlur, 0, -1 )
				GoScreen( "Shared\\Twitter.lua", UIEnums.Context.Blurb )
			elseif arg3 == UIEnums.ShareOptions.Blurb then

				Amax.CreateBlurb( UIEnums.SocialNetworkingItemType.PlayingBlur, 2, -1 )

			end	
			
		elseif message == UIEnums.Message.MenuNext and arg2 == true then	-- [Blur2PC] retail Blur PC: no ButtonStart (T146)
			if UIGlobals.ProfileState[ arg1 ] == UIEnums.Profile.PreLoad then
				SetupCustomPopup( UIEnums.CustomPopups.ProfilePreLoad )
				return
			end
			
			-- send this again as if you mash A the audio hasn't subscribed yet and misses the first message - this is fine to send twice
			UISystem.PlaySound( UIEnums.SoundEffect.Start )	

			-- Singleplayer
			if GUI.CurrentMode == GUI.Mode.SinglePlayer then
				Amax.SetGameMode( UIEnums.GameMode.SinglePlayer )

				PlaySfxNext()
				UIButtons.TimeLineActive( "Open_Sp", false )
				UIButtons.TimeLineActive( "Close_Mp", true )
				Amax.SetPlayingSpGameLatest(true)
				EnterSpRaceBook(true)

			-- Multiplayer
			elseif GUI.CurrentMode == GUI.Mode.Multiplayer then
				Amax.SetGameMode( UIEnums.GameMode.MultiPlayer )

				PlaySfxNext()
				UIButtons.TimeLineActive( "Open_Mp", false )
				UIButtons.TimeLineActive( "Close_Sp", true )
				Amax.SetPlayingSpGameLatest(false)
				EnterMpRaceBook()
			end

			-- allow the global update to run in the background
			net_EnableGlobalUpdate( true, true )
				
		elseif message == UIEnums.Message.MenuBack then
			-- [Bone] #37718 - Don't allow back button for the 0.5 seconds DefCam is active
			if IsTable( GUI.camera ) == true then
				return
			end
			-- Back to start screen
			UIButtons.TimeLineActive( "Open", false, 0 )
			UIButtons.TimeLineActive( "Selected", false )
			
			Amax.ChangeUiCamera( "Start1a", 1, 0 )
			
			GUI.current_stage = GUI.STAGE_PRESS_START
			SetupInfoLine( UIText.INFO_A_CONTINUE, UIText.INFO_QUIT_B )	-- [Blur2PC] retail Blur PC (T146)
			PlaySfxBack()
			UISystem.PlaySound( UIEnums.SoundEffect.GraphicStartFlyBackward )
			
			UIButtons.SetSelected( SCUI.name_to_id[ "OnShape" ], true )
			
			StartScreen_DefaultProfileVars()
			
			net_SetRichPresence( UIEnums.RichPresence.Idle )
			
			Multiplayer.SplitscreenResetScores()
			UIGlobals.splitscreen_primary_pad_original = -1
		elseif message == UIEnums.Message.ButtonLeft and arg2 == true and UIGlobals.LaunchMode ~= UIEnums.LaunchMode.Press and UIGlobals.LaunchMode ~= UIEnums.LaunchMode.E3Mp then
			StartScreen_SelectSp()
		elseif message == UIEnums.Message.ButtonRight and arg2 == true and UIGlobals.LaunchMode ~= UIEnums.LaunchMode.Press and UIGlobals.LaunchMode ~= UIEnums.LaunchMode.E3Mp then
			StartScreen_SelectMp()
		end
	end
end

function FrameUpdate( time )
	DeferCam_Update( time )
	
	if GUI.finished == false and GUI.current_stage == GUI.STAGE_PRESS_START and UIScreen.IsPopupActive() == false then
		GUI.attract_timeout = GUI.attract_timeout + time
		if GUI.attract_timeout > 30 then
			-- Come back here after showing please
			StoreScreen( UIEnums.ScreenStorage.LOAD_NEXT )
			GoScreen("Intro\\AttractSplash.lua")
		end	
	else
		GUI.attract_timeout = 0
	end
	
	if GUI.entering_debug_menu == true then
		return
	end

	-- if we have a game invite waiting
	if GUI.inactive_game_invite_wait == true then
		GUI.inactive_game_invite_wait = UIScreen.IsPopupActive()
	end
	
	if GUI.current_stage == GUI.STAGE_PRESS_START and GUI.inactive_game_invite_wait == false then
		if NetServices.GameInvitePending() == true then
			local pad_index = NetServices.FindGameInvitePadIndex()

			-- lets pretent that the player actually pressed start... tee hee
			if pad_index ~= -1 then
				UIScreen.CancelPopup()
				ContextTable[ UIEnums.Context.Main ].MessageUpdate( UIEnums.Message.ButtonStart, pad_index, true )
			end
		elseif UIGlobals.ActUponFriendChallenges == true and UIGlobals.PendingFriendChallengeInfo.Pending == true then
			if UIGlobals.PendingFriendChallengeInfo.Pad ~= -1 then
				UIScreen.CancelPopup()
				ContextTable[ UIEnums.Context.Main ].MessageUpdate( UIEnums.Message.ButtonStart, UIGlobals.PendingFriendChallengeInfo.Pad, true )
			end
		end
	end
	
	if GUI.current_stage == GUI.STAGE_LOADING then
		if UIGlobals.ProfileBootFinished == true then
			GUI.current_stage = GUI.STAGE_MODE_SELECT
			
			if Amax.HasBeenPlayingSpGameLatest() == true then
				--select single player
				StartScreen_SelectSp( true )
			else
				--select multiplayer
				StartScreen_SelectMp( true )
			end
		
			Profile.AllowProfileChanges( true )
			Profile.ActOnProfileChanges( true )
			Profile.AllowAllPadInput( false )
			Profile.GetPrimaryProfilePicture() -- fine to call on all platforms as the function body only exists on PS3
			
			Amax.GetAchievementDetails()
					
			CheckPumpkin()
			
			SetupInfoLine( UIText.INFO_A_SELECT, UIText.INFO_B_BACK, "GAME_SHARE_BUTTON" )
			
			-- this doesnt get flagged until we leave the screen but in the case or accepted a game invite while demonware is down, then it needs to reconnect
			if NetServices.GameInvitePending() == true and UIGlobals.Network.GlobalUpdateEnabled == false then
				net_EnableGlobalUpdate( true, true )
			end
			
			-- [Feature Creep] #37023 - Play glossy video first time you play
			local enum = UIEnums.SPVideoConfig.VIDEO_GLOSSY_INTRO
			if Amax.CanPlayGameModeVideo( enum ) == true then
				Amax.SetVideoPlayedBitInProfile( enum ) 
				UIGlobals.ActiveSPMovie[1].Filename, UIGlobals.ActiveSPMovie[1].Skipable, UIGlobals.ActiveSPMovie[1].FullScreen = Amax.GetGameModeVideoFile( enum )
				UIGlobals.SpMovieScreenShouldSave = false
				
				PushScreen( "SinglePlayer\\SpMovieScreen.lua" )
				DeferCam_Init( GUI.UiCameraName[ GUI.Mode.SinglePlayer ], 0.5 )
				return
			end

			if Amax.CanShowPreviouslyOnBlur() == true and UIGlobals.CanShowPreviouslyOnBlur == true then
				--get the friend demands
				FriendDemand.RetrieveFromServer(true)
				UIGlobals.CanShowPreviouslyOnBlur = false
				PushScreen( "Intro\\PreviouslyOnBlur.lua" )
			else
				Amax.ChangeUiCamera( GUI.UiCameraName[ GUI.CurrentMode ], 2, 0 )
			end
			
		end
	end
	
end

function EnterEnd()
	--
end

function EndLoop( time )
	--
end

function End()
	-- Debug : UNSAFE!
	if GUI.current_stage == GUI.STAGE_LOADING then
		print("Warning cancelled load unsafely")
		EndScreen( UIEnums.Context.LoadSave )
	end
	
	StartScreen_DefaultProfileVars = nil
	StartScreen_ChangeUiCamera = nil
end

function StartProfileLoad(arg1)
	UIGlobals.ProfilePressedStart = arg1
	Profile.SetPrimaryPad( UIGlobals.ProfilePressedStart )
	Profile.LockToPad( arg1 )

	GameProfile.InitPrimary()
	
	Amax.SetGameMode( UIEnums.GameMode.SinglePlayer )
	
	-- clear all cached profiles - again - for if we've signed in on this screen
	StartScreen_ClearProfiles()
	
	UIGlobals.ProfilePrimaryPad = -1		-- clear
	UIGlobals.ProfilesFound = 0				-- reset
	
	-- reset to only load in the one profile - the person that presses start
	UIGlobals.ProfileLookingFor = -1
	
	PushScreen( "Profile\\Boot.lua" )
end

function EnterMpRaceBook()	
	Amax.SetUICarToMultiplayer(true)
	Amax.SendMessage(UIEnums.GameFlowMessage.LoadSPProfileCar)
	
	GoScreen( GUINetwork.multiplayer_root )

	Amax.SetLoadStateIntoUI()
	LSP.GetLinkCode()
end

function StartScreen_DefaultProfileVars()
	--print( "StartScreen_DefaultProfileVars" )
	
	UIGlobals.FileParams = {  }
	UIGlobals.ScreenOptions = { }
		
	UIGlobals.DebugEvent = nil
	UIGlobals.LoadFromDebug = false
	UIGlobals.FadeUpLoading = nil
	
	UIGlobals.FriendDemandFilterFriend = 0
	UIGlobals.FriendDemandAttemptFromMessage = false
	
	Profile.AllowProfileChanges( true )
	Profile.ActOnProfileChanges( false )
	Profile.AllowAllPadInput( true )

	net_EnableGlobalUpdate( false )
	net_CloseServiceConnection( )
end

function StartScreen_ClearProfiles()
	for i = 0, 3, 1 do
		Profile.ClearProfile( i )
		Profile.ReadPadProfile( i )
		GameProfile.ClearGameProfile( i )
		
		UIGlobals.ProfileState[ i ] = UIEnums.Profile.None
		UIGlobals.LoadProfile[ i ] = false
	end
end

function StartScreen_ChangeUiCamera( camera_index )
	Amax.ChangeUiCamera(  GUI.UiCameraName[ camera_index ], GUI.UiCameraTime, 0 )
	
	if GUI.UiCameraTime ~= 0.5 then
		GUI.UiCameraTime = 0.5
	end
end

function CheckPumpkin()

	--show the pumpkin if we have finished the game
	local shouldDisplayPumpkin = Amax.ShouldDisplayPumpkin()
	
	local SPIcon = SCUI.name_to_id[ "SP_Icon" ]
	
	if shouldDisplayPumpkin == true then		
		UIShape.ChangeSceneName( SPIcon, "fe_icons" )
		UIShape.ChangeObjectName( SPIcon, "bizarre" )
	else		
		UIShape.ChangeSceneName( SPIcon, "common_icons" )
		UIShape.ChangeObjectName( SPIcon, "bio" )	
	end
	
end

-- called externally from a timeline
function StartScreen_StartLogo()
	if GUI.current_stage == GUI.STAGE_PRESS_START then
		UIButtons.SetSelected( SCUI.name_to_id[ "OnShape" ], true )
	end
end
