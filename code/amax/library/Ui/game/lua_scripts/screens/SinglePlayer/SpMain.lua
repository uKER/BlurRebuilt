GUI =
{
	-- Screen control
	finished = false,
	-- Carousel branches (I could generate these)
	branch_name = 
	{
		[UIEnums.RwBranch.Races]	= "Races",
		[UIEnums.RwBranch.Garage]	= "Garage",
		[UIEnums.RwBranch.Quit]		= "Quit",
		[UIEnums.RwBranch.Help]		= "Help",
		[UIEnums.RwBranch.Photos]	= "Photos",
		[UIEnums.RwBranch.Blurb]	= "Blurb",
		[UIEnums.RwBranch.Stickers]	= "Stickers",
		[UIEnums.RwBranch.Options]	= "Options",
		[UIEnums.RwBranch.Leaderboards]	= "Leaderboards",
	},
	screen_name = 
	{
		[UIEnums.RwBranch.Races]	= "SinglePlayer\\SpTierSelect.lua",
		[UIEnums.RwBranch.Garage]	= "Shared\\Garage.lua",
		[UIEnums.RwBranch.Quit]		= nil,
		[UIEnums.RwBranch.Help]		= "Shared\\Help.lua",
		[UIEnums.RwBranch.Photos]	= "Profile\\AsyncEnumerate.lua", --"Photos\\Photos.lua",
		[UIEnums.RwBranch.Blurb]	= "Shared\\BlurbMainMenu.lua",
		[UIEnums.RwBranch.Stickers]	= "Shared\\Stickers.lua",
		[UIEnums.RwBranch.Options]	= "Shared\\Options.lua",
		[UIEnums.RwBranch.Leaderboards]	= "SinglePlayer\\SpCarouselLeaderboard.lua",
	},
	help_text = 
	{
		[UIEnums.RwBranch.Races]	= UIText.RBU_TABHELP_RACES,
		[UIEnums.RwBranch.Garage]	= UIText.RBU_TABHELP_GARAGE,
		[UIEnums.RwBranch.Quit]		= UIText.RBU_TABHELP_QUIT,
		[UIEnums.RwBranch.Help]		= UIText.RBU_TABHELP_HELP,
		[UIEnums.RwBranch.Photos]	= UIText.RBU_TABHELP_PHOTOS,
		[UIEnums.RwBranch.Blurb]	= UIText.RBU_TABHELP_BLURB,
		[UIEnums.RwBranch.Stickers]	= UIText.RBU_TABHELP_STICKERS,
		[UIEnums.RwBranch.Options]	= UIText.RBU_TABHELP_OPTIONS,
		[UIEnums.RwBranch.Leaderboards]	= UIText.RBU_TABHELP_LEADERBOARDS,
	},
		
	-- Carousel controls		
	active_context = UIEnums.Context.CarouselApp,
	active_selection = -1,
	over_selection = -1,

	unlocks = {  },

	movie_names = 
	{
	--[[
		"TierSelect_01",
		"TierSelect_02",
		"TierSelect_03",
		"TierSelect_04",
		"TierSelect_05",
		"TierSelect_06",
		"TierSelect_07",
		"TierSelect_08",
		"TierSelect_09",
	]]--
	},
	
	has_new_mail = false,
	
	node_id = {  },
	
}

function Init()
	net_LockoutFriendsOverlay( false )
	
	AddSCUI_Elements( )
	UIScreen.SetScreenTimers( 0.3, 0.3 )
	Amax.ChangeUiCamera( UIGlobals.CameraNames.SpCarousel, 0, 0 )

	SetupInfoLine( UIText.INFO_OPEN_A, UIText.INFO_QUIT_B, "GAME_SHARE_BUTTON" )
	net_SetRichPresence(UIEnums.RichPresence.SpMain)

	-- Note intialise all the movie slots here
	for i, v in ipairs( GUI.movie_names ) do
		UISystem.InitMovie( i, "ui\\movies\\" .. v .. ".bik", false )
		UISystem.PauseMovie( i, true )
	end

	
	--[[	
	UIGlobals.Sp = 
	{
		CurrentTier = 1,
		CurrentEvent = 1,
	}
	UIGlobals.SpReturnMode = UIGlobals.SpSelectMode_Event
	]]--
	
	LockController()
	
	UIGlobals.IsIngame = false

	GUI.carousel_id = SCUI.name_to_id[ "Carousel" ]
	
	GUI.events_new_count_id = SCUI.name_to_id[ "EventsNewCount" ]
	GUI.events_new_id = SCUI.name_to_id[ "EventsNew" ]
	
	UIButtons.SetSelected( GUI.carousel_id, false )
	GUI.over_selection = UIEnums.RwBranch.Races
	
	GUI.camera_id = UIButtons.CloneXtGadgetByName( "SCUIBank", "Cam_Carousel" )
	UIButtons.SetParent( GUI.camera_id, SCUI.name_to_id[ "CameraDolly" ], UIEnums.Justify.MiddleCentre )
	
--	UIButtons.SetSelectionByIndex(GUI.carousel_id, GUI.over_selection ) 

	-- Status bar
	local statusBarId = AddStatusBar()
	UIButtons.SetParent( statusBarId, GUI.camera_id, UIEnums.Justify.MiddleCentre )
	
end


function PostInit()
	if UIEnums.CurrentPlatform == UIEnums.Platform.PC then
		-- [Blur2PC] Cover the viewport while leaving the garage for a race.
		UIButtons.ChangeSizeX( SCUI.name_to_id[ "FadeDown_Loading" ], SCREEN_WIDTH )
	end

	-- Help text bottom bar	
	GUI.help_text_id, GUI.bottom_help_id = SetupBottomHelpBar()
	UIButtons.ChangeText( GUI.help_text_id, GUI.help_text[ GUI.over_selection ] )

	local titles = 
	{
		[UIEnums.RwBranch.Races]		= UIText.RBC_RACES,
		[UIEnums.RwBranch.Garage]		= UIText.RBC_GARAGE,
		[UIEnums.RwBranch.Quit]			= UIText.CMN_CSL_QUIT,
		[UIEnums.RwBranch.Help]			= UIText.RBC_HELP,
		[UIEnums.RwBranch.Photos]		= UIText.RBC_PHOTOS,
		[UIEnums.RwBranch.Blurb]		= UIText.FDE_DEMANDS,
		[UIEnums.RwBranch.Stickers]		= UIText.RBC_STICKERS,
		[UIEnums.RwBranch.Options]		= UIText.RBC_OPTIONS,
		[UIEnums.RwBranch.Leaderboards]	= UIText.SP_CAROUSEL_LEADERBOARD,
	}
	local shapes = 
	{
		[UIEnums.RwBranch.Races]		= "bio",
		[UIEnums.RwBranch.Garage]		= "garage",
		[UIEnums.RwBranch.Quit]			= "start",
		[UIEnums.RwBranch.Help]			= "help",
		[UIEnums.RwBranch.Photos]		= "photo",
		[UIEnums.RwBranch.Blurb]		= "message",
		[UIEnums.RwBranch.Stickers]		= "sticker",
		[UIEnums.RwBranch.Options]		= "option cog",
		[UIEnums.RwBranch.Leaderboards]	= "leaderboards",
	}
	local scenes = 
	{
		[UIEnums.RwBranch.Leaderboards]	= "fe_icons",
	}
	-- This is the order items are added
	local order = 
	{
		UIEnums.RwBranch.Races,
		UIEnums.RwBranch.Stickers,
		UIEnums.RwBranch.Garage,
		UIEnums.RwBranch.Photos,
		UIEnums.RwBranch.Options,
		UIEnums.RwBranch.Quit,
		UIEnums.RwBranch.Leaderboards,
		UIEnums.RwBranch.Help,
		UIEnums.RwBranch.Blurb,
	}
	
	local isSignedIn = true
	if UIEnums.CurrentPlatform == UIEnums.Platform.Xenon then
		if UIGlobals.ProfileState[ Profile.GetPrimaryPad() ] ~= UIEnums.Profile.GamerProfile then
			isSignedIn = false
		end
	end
	
	for _,i in ipairs( order ) do
		local addApp = true
		
		if i == UIEnums.RwBranch.Photos then
			if isSignedIn == false then
				addApp = false
			end
		elseif i == UIEnums.RwBranch.Leaderboards then
			if isSignedIn == false then
				addApp = false
			end
		elseif i == UIEnums.RwBranch.Blurb then
			if isSignedIn == false then
				addApp = false
			end
		end
		
		if addApp == true then
			local branch_id = UIButtons.CloneXtGadgetByName( "SCUIBank", Select( (i==UIEnums.RwBranch.Garage), "Garage_CarouselBranch", "CarouselBranch" ) )
		
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
	end
	
	-- Scan and assign values
	Amax.CarouselScan( GUI.carousel_id )
	for i,v in pairs(GUI.branch_name) do
		if IsNumber( GUI.node_id[ i ] ) == true then
			UIButtons.SetNodeValue( GUI.node_id[ i ], i )
		end
	end
	
	-- This one starts on
	UIButtons.PrivateTimeLineActive( GUI.node_id[ UIEnums.RwBranch.Races ], "Init", true, 10.0, true  )
	
	GUI.locked_table = {} -- GameProfile.RaceBookAppsLocked()

	-- Turn off locked items
	for i,v in ipairs( GUI.locked_table ) do
		local rb = GUI.node_id[ v ]
		UIButtons.SetParent( rb, nil )
		UIButtons.SetActive( rb, false )
	end
	
	Amax.CarouselScan( GUI.carousel_id )
	UIButtons.SetSelection( GUI.carousel_id, GUI.over_selection )
	CarouselSetApp( GUI, SCUI, GUI.over_selection, true )
	
	if UIGlobals.FriendDemandAttemptFromMessage == true then
		
		UIGlobals.ReturnToBlurb = nil

		net_FlushEverything(false)
		
		Amax.SetGameMode( UIEnums.GameMode.SinglePlayer )

		EnterSpRaceBook(false)		
		
		if FriendDemand.HasPrivilege() == true then
			GUI.active_selection = UIEnums.RwBranch.Blurb
			UIButtons.SetSelection( GUI.carousel_id, GUI.active_selection )
			UIGlobals.FriendDemandFilter = 2	--new challenges
			OpenApp( UIEnums.RwBranch.Blurb, nil, "Shared\\Blurb.lua" )
		else
			SetupCustomPopup( UIEnums.CustomPopups.FriendChallengeNoPrivilege )
			UIGlobals.FriendDemandAttemptFromMessage = false
		end

	elseif UIGlobals.ReturnToBlurb == true then
		GUI.active_selection = UIEnums.RwBranch.Blurb
		UIButtons.SetSelection( GUI.carousel_id, GUI.active_selection )
		OpenApp( UIEnums.RwBranch.Blurb, nil, "Shared\\BlurbMainMenu.lua" )
		
		
	else
		-- any unlocks?
		GUI.unlocks = Sp_UnlocksFilter( SinglePlayer.ProcessUnlocks() )
		--[[
		GUI.unlocks = 
		{
			{
				kind = "event",
				tier = 2,
			}
		}
		]]--
		
		local showing_video = false
		if #GUI.unlocks > 0 then
			--Play the right Videos First
			--queue up all the boss beaten videos that you have unlocked
			for i, v in ipairs( GUI.unlocks ) do
				if v.kind == "boss-defeated" then
					showing_video = true
					SpMain_ProcessBossBeatenVideos( v ) -- Boss Beaten
				end
			end
			-- [Bone] #35739 - **Sake** I said comment out all the code, not just the movie player
			--[[
			--queue up all the boss unlocked videos that you have unlocked
			for i, v in ipairs( GUI.unlocks ) do
				if v.kind == "event" then
					showing_video = true
					-- Boss Unlocked. Handle in sptierselect.lua
				end
			end
			]]--
			--queue up all the tier unlocked videos that you have unlocked
			for i, v in ipairs( GUI.unlocks ) do
				if v.kind == "tier" then
					showing_video = true
					SpMain_ProcessTierUnlockVideos( v ) -- Tier Unlocked
				end
			end
			-- OpenApp( UIEnums.RwBranch.Races, nil, "SinglePlayer\\SpUnlocks.lua" )

			-- Note if (#GUI.unlocks>0 and showing_videos = false) then we've got unhandles unlocks
			-- such as cars and rank-up
			if showing_video == false then
				SinglePlayer.ClearUnlocks()
			end
		end
		
		if Sp_ReturnFromGame() == true and showing_video == false then
			OpenApp( UIEnums.RwBranch.Races, nil, "SinglePlayer\\SpTierSelect.lua" )
		end
	end
		
	Amax.CheckSinglePlayerStickers()
	
	if Amax.ShouldDisplayPumpkin() == true then
		local shape_id = UIButtons.FindChildByName( GUI.node_id[ UIEnums.RwBranch.Races ], "Shape" )
		UIShape.ChangeSceneName( shape_id, "fe_icons" )
		UIShape.ChangeObjectName( shape_id, "bizarre" )
	end
	
	-- Turn on 'new' Blurbs
	GUI.has_new_mail = false
	if FriendDemand.IsThereNewMail() == true then
		GUI.has_new_mail = true
		UIButtons.SetActive( UIButtons.FindChildByName( GUI.node_id[ UIEnums.RwBranch.Blurb ], "NewIcon" ), true )
	end

	-- Turn on 'new' Stickers [ToDo]
	if Amax.HasNewStickers() then
		UIButtons.SetActive( UIButtons.FindChildByName( GUI.node_id[ UIEnums.RwBranch.Stickers ], "NewIcon" ), true )
	end
	
	FadeUpLoading()
end

function CarouselOpenFinished( )
	-- Sorcery safe
	local _GUI = ContextTable[ UIEnums.Context.Main ].GUI
	
	if UIGlobals.FriendDemandAttemptFromMessage ~= true and UIGlobals.ReturnToBlurb ~= true then
		UIButtons.SetSelected( _GUI.carousel_id, true )
	end
	
	UIGlobals.ReturnToBlurb = nil

	UnlockController()
end

function StartLoop( time )
	--
end

function MessageUpdate( message, arg1, arg2, arg3, arg4 )
	if SCUI_MessageUpdate( message, arg1, arg2, arg3, arg4 ) == true then
		return
	end

	-- #34414
	if message == UIEnums.MiscMessage.ServiceConnectionLost or message == UIEnums.MiscMessage.ServiceConnectionEstablished then
		UIButtons.ChangeText( UIButtons.FindChildByName( GUIBank.SpStatusBar.branch_id, "ProfileName" ), "PROFILE_PAD_NAME" )
	end

	if IsControllerLocked() == true then
		return
	end
	
	if SubScreenActive() == true then
		return
	end
	
	if GUI.active_selection == -1 then
		
		if message == UIEnums.Message.PopupEnd and arg2 == UIEnums.CustomPopups.TrophiesNotInstalled and arg4 ~= 1 then
			OpenApp( UIEnums.RwBranch.Stickers )
			return
		end
	
		if message == UIEnums.Message.ButtonLeftShoulder and arg2 == true then

			if Amax.CanUseShare() == true then
				SetupCustomPopup( UIEnums.CustomPopups.SpMainSelectSharingOptions )
			end

		elseif message == UIEnums.Message.PopupNext and arg2 == UIEnums.CustomPopups.SharingOptions then

			if arg3 == UIEnums.ShareOptions.Facebook then
				
				Amax.CreateBlurb(UIEnums.SocialNetworkingItemType.FanStatus + UIGlobals.SharingOptionsChosen, 1, -1)
				
				StoreScreen( UIEnums.ScreenStorage.FE_SOCIAL_NETWORK, "SinglePlayer\\SpMain.lua" )
				GoScreen( "Shared\\Facebook.lua", UIEnums.Context.Blurb )
			
			elseif arg3 == UIEnums.ShareOptions.Twitter then

				Amax.CreateBlurb(UIEnums.SocialNetworkingItemType.FanStatus + UIGlobals.SharingOptionsChosen, 0, -1)

				StoreScreen( UIEnums.ScreenStorage.FE_SOCIAL_NETWORK, "SinglePlayer\\SpMain.lua" )
				GoScreen( "Shared\\Twitter.lua", UIEnums.Context.Blurb )
				
			elseif arg3 == UIEnums.ShareOptions.Blurb then

				Amax.CreateBlurb(UIEnums.SocialNetworkingItemType.FanStatus + UIGlobals.SharingOptionsChosen, 2, -1)
				
			end		
		end
	
		if message == UIEnums.Message.MenuNext and arg2 == true then
			local ftime, selection = UIButtons.GetSelection( GUI.carousel_id, 2 )
	
			if selection == UIEnums.RwBranch.Stickers then
				local installed = Amax.TrophiesInstalled()
				
				if installed == false then
					SetupCustomPopup( UIEnums.CustomPopups.TrophiesNotInstalled )
				else
					OpenApp( selection )
				end
					
			elseif selection == UIEnums.RwBranch.Quit then
				SetupCustomPopup( UIEnums.CustomPopups.ExitApplication )
				
			elseif selection == UIEnums.RwBranch.Blurb then
				if Profile.PadProfileOnline( Profile.GetPrimaryPad() ) == false then
					if UIEnums.CurrentPlatform == UIEnums.Platform.PS3 then
						net_StartServiceConnection(true, nil, false)
					else
						if net_CanReconnectToDemonware() == false then
							SetupCustomPopup( UIEnums.CustomPopups.MultiplayerOnlineConnectionLost )
						else
							net_StartServiceConnection(true, nil, false)
						end
					end
				else
					if NetServices.UserHasAgeRestriction() == true then
						SetupCustomPopup( UIEnums.CustomPopups.FailedAgeCheck )
					elseif LSP.IsConnected() == false then
						SetupCustomPopup( UIEnums.CustomPopups.ContentServerGeneralError )
					elseif FriendDemand.HasPrivilege() == false then
						SetupCustomPopup( UIEnums.CustomPopups.FriendChallengeNoPrivilege )
					elseif Amax.GetNumFriends() == 0 then
						SetupCustomPopup( UIEnums.CustomPopups.FriendChallengeNoFriends )
					else
						OpenApp( selection )
					end
				end

			elseif selection == UIEnums.RwBranch.Leaderboards then
			
				if Profile.PadProfileOnline( Profile.GetPrimaryPad() ) == false then
					if UIEnums.CurrentPlatform == UIEnums.Platform.PS3 then
						net_StartServiceConnection(true, nil, false)
					else
						if net_CanReconnectToDemonware() == false then
							SetupCustomPopup( UIEnums.CustomPopups.MultiplayerOnlineConnectionLost )
						else
							net_StartServiceConnection(true, nil, false)
						end
					end
				else
					if NetServices.UserHasAgeRestriction() == true then
						SetupCustomPopup( UIEnums.CustomPopups.FailedAgeCheck )
					elseif LSP.IsConnected() == false and UIEnums.CurrentPlatform == UIEnums.Platform.PS3 then
						SetupCustomPopup( UIEnums.CustomPopups.ContentServerGeneralError )
					else
						-- Open App
						if CarouselIsAppLocked( selection ) == false then
							OpenApp( selection )
						else
							UISystem.PlaySound( UIEnums.SoundEffect.Error )
						end
					end
				end
			else
				-- Open App
				if CarouselIsAppLocked( selection ) == false then
					OpenApp( selection )
				else
					UISystem.PlaySound( UIEnums.SoundEffect.Error )
				end
				
				-- if we're viewing stickers, we may have a "new" icon to turn off [todo: universal solution for any selection?]
				if selection == UIEnums.RwBranch.Stickers then
					UIButtons.SetActive( UIButtons.FindChildByName( GUI.node_id[ UIEnums.RwBranch.Stickers ], "NewIcon" ), false )
				end
			end
		elseif message == UIEnums.Message.MenuBack and arg2 == true then
			SetupCustomPopup( UIEnums.CustomPopups.ExitApplication )
		end
	else
		if message == UIEnums.Message.MenuBack and arg2 == true then
			local ftime, selection = UIButtons.GetSelection( GUI.carousel_id, 2 )
			
			-- Close  App
			if CloseApp( selection ) == true then
				-- Light paint
				UIButtons.SetActive( SCUI.name_to_id["OpenLP_1"], true )
				UIButtons.SetActive( SCUI.name_to_id["OpenLP_2"], true )
				-- Store in case going back from the garage
				Amax.ChangeUiSplineCamera( 0, 0, nil, nil, nil, nil, nil, 0 )
				-- Move
				Amax.ChangeUiCamera( UIGlobals.CameraNames.SpCarousel, UIGlobals.CameraLerpTime, 0 )
				net_SetRichPresence(UIEnums.RichPresence.SpMain)
			end
		end
	end
end

function FrameUpdate( time )

	-- Check The network session has been created successfully
	if UIGlobals.FinishedCreatingNetworkSession == false then
		UIGlobals.FinishedCreatingNetworkSession, UIGlobals.SuccessfullyCreatedNetworkSession = Amax.ContinueCreateStatsOnlyMatchingSession()
		print("SUCCESSFULLY CREATED NETWORK SESSION", UIGlobals.SuccessfullyCreatedNetworkSession )
	end

	local ftime, selection = UIButtons.GetSelection( GUI.carousel_id, 2 )

	if selection ~= GUI.active_selection and GUI.active_selection ~= -1 then
		EndActive()
	end
	
	if selection ~= GUI.over_selection then 
		local name = GUI.branch_name[ selection ]
		
		UIButtons.PrivateTimeLineActive( GUI.node_id[ GUI.over_selection ], "Selected", false )
		UIButtons.PrivateTimeLineActive( GUI.node_id[ selection ], "Selected", true, 4.7 )
		
		-- Update help text
		UIButtons.TimeLineActive( "HelpFade", true, 0 )
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
	
	if FriendDemand.IsThereNewMail() == true and GUI.has_new_mail == false then
		GUI.has_new_mail = true
		UIButtons.SetActive( UIButtons.FindChildByName( GUI.node_id[ UIEnums.RwBranch.Blurb ], "NewIcon" ), true )
	end
	
	if FriendDemand.IsThereNoMail() == true and GUI.has_new_mail == true then
		GUI.has_new_mail = false
		UIButtons.SetActive( UIButtons.FindChildByName( GUI.node_id[ UIEnums.RwBranch.Blurb ], "NewIcon" ), false )
	end
	
	--EqualizerTestCycle( time )
end

function Render()
	--
end

function EnterEnd( )
	for i,v in pairs( GUI.node_id ) do 
		if i ~= UIEnums.RwBranch.Quit then
			UIButtons.PrivateTimeLineActive( v, "End", true, 0 )
		else
			UIButtons.PrivateTimeLineActive( UIButtons.FindChildByName( v, "Title" ), "End", true, 0 )
			UIButtons.PrivateTimeLineActive( UIButtons.FindChildByName( v, "Shape" ), "End", true, 0 )
		end
	end

	for i, v in ipairs( GUI.movie_names ) do
		UISystem.DestroyMovie( i )
	end
end

function EndLoop( time )
	--
end

function End()
	-- Clear the boot mode
	EndScreen( UIEnums.Context.CarouselApp )
	
	FlushSubscreens() -- remove the extra stack screens if they exist
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
	
	if screen_override ~= nil then
		UIScreen.SetNextScreen( screen_override, context )
	else
		UIScreen.SetNextScreen( GUI.screen_name[ selection ], context )
	end
	
	PlaySfxNext()
	CarouselSetApp( _GUI, _SCUI, selection, false )

	_GUI.active_selection = selection
	UIButtons.SetSelected( _GUI.carousel_id, false )
	
	UIButtons.PrivateTimeLineActive( _GUI.node_id[ selection ], "Open", true, 0 )
	UIButtons.PrivateTimeLineActive( _GUI.bottom_help_id, "Hide_BottomHelp", true )

	UIButtons.SetActive( _SCUI.name_to_id["OpenLP_1"], false )
	UIButtons.SetActive( _SCUI.name_to_id["OpenLP_2"], false )
	
	if selection == UIEnums.RwBranch.Races or selection == UIEnums.RwBranch.Options or selection == UIEnums.RwBranch.Help or selection == UIEnums.RwBranch.Blurb or selection == UIEnums.RwBranch.Photos or selection == UIEnums.RwBranch.Leaderboards then
		PlaySfxGraphicNext()
	end
end

function RaceWorld_UpdateProfileBar(ignore_vehicles)
	-- this can be called from different contexts
	local _SCUI = ContextTable[ UIEnums.Context.Main ].SCUI
	
	local cash_text_id = _SCUI.name_to_id[ "cash_text" ]
	local cash_icon_id = _SCUI.name_to_id[ "cash_icon" ]
	local  car_text_id = _SCUI.name_to_id[ "car_text" ]
	local  car_icon_id = _SCUI.name_to_id[ "car_icon" ]
	
	UIButtons.ChangePosition( cash_text_id, 0, 0, 0 )
	UIButtons.ChangePosition( cash_icon_id, 0, 0, 0 )
	UIButtons.ChangePosition(  car_text_id, 0, 0, 0 )
	UIButtons.ChangePosition(  car_icon_id, 0, 0, 0 )
	
	-- TODO - with the new garage
	--[[
	if ignore_vehicles ~= true then
		-- PR - don't really like grabbing the whole garage
		local vehicles = UIGlobals.SpGarageList
		local vehicle_id = Amax.GetVehicleByID()
		
		for i, v in ipairs( vehicles ) do
			if v.id == vehicle_id then
				UIButtons.ChangeText( car_text_id, v.name_loc_id )
				UIShape.ChangeObjectName( car_icon_id, v.icon )
				break
			end
		end
	end
	--]]
	
	local cash_icon_sx = UIButtons.GetSize( cash_icon_id )
	if IsNumber( cash_icon_sx ) == false then
		cash_icon_sx = 0
	end
	
	local cash_length = UIButtons.GetStaticTextLength( cash_text_id )
	local  car_length = UIButtons.GetStaticTextLength( car_text_id )
	
	UIButtons.ChangePosition( cash_icon_id, -cash_length - 0.5, 0, 0 )
	UIButtons.ChangePosition(  car_text_id, -cash_length - 0.5 - cash_icon_sx - 1, 0, 0 )
	UIButtons.ChangePosition(  car_icon_id, -cash_length - 0.5 - cash_icon_sx - 1 - car_length - 0.5, 0.5, 0 )
end

function SpMain_ProcessTierUnlockVideos( t )
--	print( "Unlock Tier Video" )
	local tier = t.tier

--	show_table( t )

	if tier ~= nil then
		if IsTable( UIGlobals.Sp ) == false then
			UIGlobals.Sp = { }
		end
		UIGlobals.SpReturnMode = UIGlobals.SpSelectMode_Tier
		UIGlobals.Sp.CurrentTier = tier
		
		PlaySPUnlockMovieFullScreen(UIEnums.SPVideoConfig.VIDEO_TIER_UNLOCKS, (tier-1) )	
	end
end						  

function SpMain_ProcessBossBeatenVideos( v )
--	print( "Boss Beaten Video" )

--	show_table( v )

	local tier = v.tier

	if tier ~= nil then
		PlaySPUnlockMovieFullScreen(UIEnums.SPVideoConfig.VIDEO_BOSS_BEATEN, (tier-1) )
		
		local tier_info = SinglePlayer.TierInfo()
		
		if tier_info ~= nil and tier == #tier_info then
			--Roll the end credits
			UIGlobals.SPRollCredits = true	
			UIButtons.SetActive(GUI.help_text_id, false)			
			UIButtons.SetActive(GUI.bottom_help_id, false)				
		end
	end
end

-- called externally from a timeline
function SpMain_TabsOpen()
	if UIScreen.IsContextActive( UIEnums.Context.CarouselApp ) == false then
		UISystem.PlaySound( UIEnums.SoundEffect.TabsOpen )
	end
end

-- called externally from a timeline
function SpMain_TabsClose()
	if UIScreen.IsContextActive( UIEnums.Context.CarouselApp ) == false then
		UISystem.PlaySound( UIEnums.SoundEffect.TabsClose )
	end
end
