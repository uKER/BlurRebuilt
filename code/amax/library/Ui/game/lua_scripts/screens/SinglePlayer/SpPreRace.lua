GUI =
{
	scoreboard_has_read = false,
	leaderboard_initialised = false,
	finished = false,
}

function Init()
	net_LockoutFriendsOverlay( true )
	
	AddSCUI_Elements( )
	DeferCam_Init( "Sp_1" )
	
	-- [Bone] - 37855 able to go to another screen before DeferCam has finished
	UIScreen.SetScreenTimers( 0.3, UIGlobals.screen_time.default_end )
	
	StoreInfoLine()
	SetupInfoLine()

	SpMiniLeaderboard_Init()
	
	GUI.boss_unlocked = UIGlobals.Sp.BossInfo[ UIGlobals.Sp.CurrentTier ].unlocked
end

function PostInit()
	UIButtons.TimeLineActive( "Hide_EventSelect", true )
	
	-- Update the boss demands panel
	local pr_event_id = SCUI.name_to_id[ "PreEvent" ]
	local eventId = UIGlobals.Sp.TierInfo[ UIGlobals.Sp.CurrentTier ].events[ UIGlobals.Sp.CurrentEvent ]
	local event = SinglePlayer.EventInfo( eventId )
	local amax_event = GameData.GetEvent( event.eventId )
	
	-- Owner
	GUI.pr_owner_node = SCUI.name_to_id[ "Owner_Node" ]

	-- Primary
	local star_root_id = SCUI.name_to_id[ "PrimaryLight_Root" ]

	GUI.pr_primary_stars = { }
	for i = 1, 5, 1 do
		GUI.pr_primary_stars[ i ] = Sp_CreateStar( nil, i, nil, false, false )
		UIButtons.SetParent( GUI.pr_primary_stars[ i ][ 1 ], star_root_id, UIEnums.Justify.MiddleCentre )
		UIButtons.SetParent( GUI.pr_primary_stars[ i ][ 2 ], star_root_id, UIEnums.Justify.MiddleCentre )
	end
	
	-- Boss
	star_root_id = SCUI.name_to_id[ "BossLight_Root" ]
	local boss_stars = Sp_EventStateToStars( state, true )
	GUI.pr_boss_stars = { }
	for i = 1, SP_MaxStars(), 1 do
		GUI.pr_boss_stars[ i ] = Sp_CreateStar( nil, i, nil, false, false )
		UIButtons.SetParent( GUI.pr_boss_stars[ i ][ 1 ], star_root_id, UIEnums.Justify.MiddleCentre )
		UIButtons.SetParent( GUI.pr_boss_stars[ i ][ 2 ], star_root_id, UIEnums.Justify.MiddleCentre )
	end
	
	-- Secondary
	star_root_id = SCUI.name_to_id[ "SecondaryLight_Root" ]
	GUI.pr_secondary_star = Sp_CreateStar( nil, nil, nil, true, false )
	UIButtons.SetParent( GUI.pr_secondary_star[ 1 ], star_root_id, UIEnums.Justify.MiddleCentre )
	UIButtons.SetParent( GUI.pr_secondary_star[ 2 ], star_root_id, UIEnums.Justify.MiddleCentre )
	-- Golden Fan Demand
	star_root_id = SCUI.name_to_id[ "GoldenFanDemand_Root" ]
	GUI.pr_gfd_star = Sp_CreateStar( nil, nil, nil, false, true )
	UIButtons.SetParent( GUI.pr_gfd_star[ 1 ], star_root_id, UIEnums.Justify.MiddleCentre )
	UIButtons.SetParent( GUI.pr_gfd_star[ 2 ], star_root_id, UIEnums.Justify.MiddleCentre )
	
	-- ### Vista ###
	UIButtons.ChangeTexture( { filename = amax_event.route_tag }, 1, SCUI.name_to_id[ "VP_Image" ] )
	
	Sp_UpdateEventPreview( SCUI.name_to_id[ "VP_Root" ], event, amax_event, eventId )
	if SP_ChangeRestrictionText( SCUI.name_to_id[ "Restriction_Title" ], SCUI.name_to_id[ "Restriction_Text" ], event ) ~= true then
		local frame_id = SCUI.name_to_id[ "VP_RacesFrame" ]
		local inner_id = SCUI.name_to_id[ "VP_BoxInner" ]
		local sx, sy = UIButtons.GetSize( frame_id )
		sy = sy - 3
		UIButtons.ChangeSize( frame_id, sx, sy )
		UIButtons.ChangeSize( inner_id, sx, sy )
	end
	
	--UIButtons.SetActive( GUI.pr_owner_node, Sp_UpdateOwnerState( GUI.pr_owner_node, Amax.EventOwnerState( eventId ), eventId ) )
	
	UpdateRival(eventId)
	
	-- Store for possible clone
	GUI.city_tag = amax_event.city_tag
	GUI.route_tag = amax_event.route_tag

	-- ### Goals ###
	local is_boss_race = ( event.kind == "boss" )
	local num_stars = Sp_EventStateToStars( event.state )
	
	-- Update stars
	if is_boss_race == true then
		-- Boss event
		UIButtons.SetActive( SCUI.name_to_id[ "BossStars" ], true )
		UIButtons.SetActive( SCUI.name_to_id[ "RegularStars" ], false )
		UIButtons.ChangeText( SCUI.name_to_id[ "BossInfo" ], "SPL_EVENT_" .. eventId .. "_PRIMARY_GOAL" )
		
		for i = 1, event.maxStars, 1 do
			if (i <= event.totalStars) then
				Sp_ActivateStar( GUI.pr_boss_stars[ i ], true )
				SpSetStarTexture( GUI.pr_boss_stars[ i ][ 1 ], true, event.state, false, false )
			else
				Sp_ActivateStar( GUI.pr_boss_stars[ i ], false )
				SpSetStarTexture( GUI.pr_boss_stars[ i ][ 2 ], false, nil, false, false )
			end
		end
	else
		-- Regular event
		UIButtons.ChangeText( SCUI.name_to_id[ "PrimaryInfo" ],			"SPL_EVENT_" .. eventId .. "_PRIMARY_GOAL_" .. Sp_EventStateToNumber( Sp_EventStateToNextState( event.state ) ) )
		UIButtons.ChangeText( SCUI.name_to_id[ "SecondaryInfo" ],		"SPL_EVENT_" .. eventId .. "_SECONDARY_GOAL" )
	--	UIButtons.ChangeText( SCUI.name_to_id[ "GoldenFanDemandInfo" ], "SPL_EVENT_" .. eventId .. "_GFD" )
	
		UIButtons.SetActive( SCUI.name_to_id[ "BossStars" ], false )
		UIButtons.SetActive( SCUI.name_to_id[ "RegularStars" ], true )
		
		for i=1,5,1 do
			if (i <= num_stars) then
				Sp_ActivateStar( GUI.pr_primary_stars[ i ], true )
				SpSetStarTexture( GUI.pr_primary_stars[ i ][ 1 ], true, event.state, false, false )
			else
				Sp_ActivateStar( GUI.pr_primary_stars[ i ], false )
				SpSetStarTexture( GUI.pr_primary_stars[ i ][ 2 ], false, nil, false, false )
			end
		end
	end
	
	if is_boss_race == false and Profile.PadProfileOnline( Profile.GetPrimaryPad() ) == false or (UIEnums.CurrentPlatform == UIEnums.Platform.PS3 and LSP.IsConnected() == false) then
		UIButtons.SetActive( SCUI.name_to_id[ "OfflineBestBranch" ], true )
		UIButtons.ChangeText( SCUI.name_to_id[ "BestPrimaryInfo" ],			"SPL_EVENT_" .. eventId .. "_PRIMARY_BEST" )
		UIButtons.ChangeText( SCUI.name_to_id[ "BestSecondaryInfo" ],		"SPL_EVENT_" .. eventId .. "_SECONDARY_BEST" )
	end
	
	Sp_ActivateStar( GUI.pr_secondary_star, event.fanParComplete )
	Sp_ActivateStar( GUI.pr_gfd_star, event.fanRunComplete )	
	
	GUI.leaderboard_initialised = SpMiniLeaderboard_PostInit( SCUI.name_to_id[ "LeaderboardBranch" ], true )
end

function MessageUpdate( message, arg1, arg2, arg3, arg4 )
	if SCUI_MessageUpdate( message, arg1, arg2, arg3, arg4 ) == true then
		return
	end

	if message == UIEnums.Message.MenuNext and arg2 == true then
		if UIGlobals.SpSelectMode == UIGlobals.SpSelectMode_PreRace then
			PlaySfxNext()
			PushScreen( "Shared\\Garage.lua" )
			
			UIGlobals.SpSelectMode = UIGlobals.SpSelectMode_Garage
			
			Amax.SetSPEvent(UIGlobals.Sp.TierInfo[ UIGlobals.Sp.CurrentTier ].events[ UIGlobals.Sp.CurrentEvent ])
						
			UIButtons.TimeLineActive( "Hide_PreRace", true )

			Amax.LoadTextureClone( GUI.route_tag, 0 )
		end
	elseif message == UIEnums.Message.MenuBack and arg2 == true then
		if UIGlobals.SpSelectMode == UIGlobals.SpSelectMode_PreRace then
			PlaySfxBack()
			UISystem.PlaySound( UIEnums.SoundEffect.GraphicBackward )
			SpPopToEventSelect()
		end
		
		-- PR - garage handles itself now
		
	elseif message == UIEnums.Message.ButtonX then 
		
		if Profile.PadProfileOnline( Profile.GetPrimaryPad() ) == false or (UIEnums.CurrentPlatform == UIEnums.Platform.PS3 and LSP.IsConnected() == false) then
			return
		end
		
		if	Profile.PadProfileOnline( Profile.GetPrimaryPad() ) == true then
			if GUI.scoreboard_has_read == true then
				UISystem.PlaySound( UIEnums.SoundEffect.Filter )
				PlaySfxGraphicBack()
				PushScreen( "SinglePlayer\\SpShowLeaderboard.lua" )
			end
		end

	elseif message == UIEnums.MiscMessage.FriendsUpdated then
		UIGlobals.UpdateEventOwnersPreRace = true
	end
end

function StartLoop( time )
	DeferCam_Update( time )
end

function FrameUpdate( time )
	DeferCam_Update( time )
	
	-- BUG: 37063	
	if GUI.leaderboard_initialised == true then
		SpMiniLeaderboard_FrameUpdate( time, false )
	end
	
	if UIGlobals.SpViewingLeaderboard ~= true and UIGlobals.SpViewingGarage ~= true and UIGlobals.OnMovieScreenInEventSelect ~= true and UIGlobals.ViewingFriendsList ~= true then
		if Profile.PadProfileOnline( Profile.GetPrimaryPad() ) == false or (UIEnums.CurrentPlatform == UIEnums.Platform.PS3 and LSP.IsConnected() == false) or GUI.leaderboard_initialised == false then
			SetupInfoLine( UIText.INFO_A_SELECT, UIText.INFO_B_BACK )
		else
			SetupInfoLine( UIText.INFO_A_SELECT, UIText.INFO_B_BACK, UIText.INFO_X_EXPAND_LEADERBOARD )
		end
	end
	
	if UIGlobals.UpdateEventOwnersPreRace == true then
		
		UIGlobals.UpdateEventOwnersPreRace = false
		UpdateRival(UIGlobals.Sp.TierInfo[ UIGlobals.Sp.CurrentTier ].events[ UIGlobals.Sp.CurrentEvent ])
		
	end
end

function Render()
	--
end

function EnterEnd()
	--
	RestoreInfoLine()	

	UIButtons.TimeLineActive( "Hide_EventSelect", false )
end

function EndLoop( time )
	--
end

function End()
	net_LockoutFriendsOverlay( false )
end

function UpdateRival(eventId)

	if Profile.PadProfileOnline( Profile.GetPrimaryPad() ) == false or (UIEnums.CurrentPlatform == UIEnums.Platform.PS3 and LSP.IsConnected() == false) then
		UIButtons.SetActive( SCUI.name_to_id[ "_background_box" ], false)
		UIButtons.SetActive( SCUI.name_to_id[ "_text_title_rival" ], false)
		UIButtons.SetActive( SCUI.name_to_id[ "Owner_Help" ], false)
		UIButtons.SetActive( SCUI.name_to_id[ "Owner_AvatarFrame" ], false)
		UIButtons.SetActive( SCUI.name_to_id[ "Owner_Gamertag" ], false)
		UIButtons.SetActive( SCUI.name_to_id[ "Owner_Score" ], false)
		
		return
	end

	if Amax.HasSinglePlayerRival() == true then

		local event=SinglePlayer.EventInfo( UIGlobals.Sp.TierInfo[ UIGlobals.Sp.CurrentTier ].events[ UIGlobals.Sp.CurrentEvent ] )
		
		if event.new == true then
		
			UIButtons.SetActive( SCUI.name_to_id[ "Owner_Help" ], true)
			UIButtons.ChangeText( SCUI.name_to_id[ "Owner_Help" ], UIText.CMN_RIVAL_EVENT_NEW )

			UIButtons.SetActive( SCUI.name_to_id[ "Owner_AvatarFrame" ], false)
			UIButtons.SetActive( SCUI.name_to_id[ "Owner_Gamertag" ], false)
			UIButtons.SetActive( SCUI.name_to_id[ "Owner_Score" ], false)
			
		else
		
			UIButtons.SetActive( SCUI.name_to_id[ "Owner_Help" ], false)
			UIButtons.SetActive( SCUI.name_to_id[ "Owner_AvatarFrame" ], true)
			UIButtons.SetActive( SCUI.name_to_id[ "Owner_Gamertag" ], true)
			UIButtons.SetActive( SCUI.name_to_id[ "Owner_Score" ], true)
			
			local gp_map=Profile.GetRemoteGamerPictureMap()
			if gp_map[0] ~= nil then
				UIButtons.ChangeTexture( { filename = "REMOTE_GAMERPIC_" .. gp_map[0] }, 1, SCUI.name_to_id[ "Owner_AvatarFrame" ] )
			end
			
			UIButtons.ChangeText( SCUI.name_to_id[ "Owner_Gamertag" ], "SPL_EVENT_" .. eventId .. "_RIVAL_NAME" )
			UIButtons.ChangeText( SCUI.name_to_id[ "Owner_Score" ], "SPL_EVENT_" .. eventId .. "_RIVAL_SCORE" )
			
		end
	else

		UIButtons.SetActive( SCUI.name_to_id[ "Owner_Help" ], true)
		UIButtons.ChangeText( SCUI.name_to_id[ "Owner_Help" ], UIText.CMN_NO_RIVAL_SET )

		UIButtons.SetActive( SCUI.name_to_id[ "Owner_AvatarFrame" ], false)
		UIButtons.SetActive( SCUI.name_to_id[ "Owner_Gamertag" ], false)
		UIButtons.SetActive( SCUI.name_to_id[ "Owner_Score" ], false)
	
	end
end