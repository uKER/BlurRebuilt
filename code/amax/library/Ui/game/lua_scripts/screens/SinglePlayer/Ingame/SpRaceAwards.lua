GUI =
{
	finished = false,
	loading_ui = false,
	
	stage = 
	{
		goals = 0,
		leaderboard = 1,
	},
	
	current_stage = 0,
	
	light_ids = 
	{
		event = 
		{
			primary = {  },
			primary_blob = {  },
			fan_par = {  },
			fan_par_blob = nil,
			fan_run = {  },
			fan_run_blob = nil,
		},
		result = 
		{
			primary = {  },
			fan_par = {  },
			fan_run = {  },
		},
	},
	
	light_effect_ids = {  },
	
	light_change_active = nil,
	light_change_timer = 0,
	light_change_timer_next = 0,
	light_change_current = 1,
	
	timer = 0,
	
	allow_next_and_share = false,

	CanExit = function( _GUI )
		return false
	end,
	
	WriteFailed = false,
	FinishedWriting = false,
	
	LeaderboardWriteTimer = 1,
	
	LeaderboardsValid = false,
	
	FinishPostRaceLeaderboardScreen = false,
	leaderboard_initialised = false,
}

function SPRaceAwardLeaderboard( result )
	if result == true then
		UIGlobals.StartPostRaceLeaderboardWrite=true
		UIGlobals.SPRA_UpdateRival = true
	end
end

function Init()
	UIScreen.SetScreenTimers( 1, UIGlobals.screen_time.default_end ) -- [PR] #35572
	
	AddSCUI_Elements()
	SetupFadeToBlack( false, true )
	
	GUI.event_id = UIGlobals.Sp.TierInfo[ UIGlobals.Sp.CurrentTier ].events[ UIGlobals.Sp.CurrentEvent ]
	GUI.result = Amax.SP_GetLevelResult()
	
	GUI.LeaderboardsValid = false
	
	if GUI.result.state ~= "none" then
		if UIEnums.CurrentPlatform == UIEnums.Platform.Xenon then
			if UIGlobals.SuccessfullyCreatedNetworkSession == true then
				GUI.LeaderboardsValid = true
				
				--we've already wrote them, so don't do it again	
				if UIGlobals.SPPostRaceLeaderboardDone == false then
					print("Xenon")
					Amax.StartSPScoreboardWrite()
				end
				
				SpMiniLeaderboard_Init()
			end
		end
		
		if UIEnums.CurrentPlatform == UIEnums.Platform.PS3 then
			if LSP.IsConnected() == true then
				print("Starting write ...")
				GUI.LeaderboardsValid = true
				
				--we've already wrote them, so don't do it again	
				if UIGlobals.SPPostRaceLeaderboardDone == false then
					print("PS3")
					Amax.StartSPScoreboardWrite()
				end
				
				SpMiniLeaderboard_Init()
			end
		end
	else
		if UIGlobals.NetworkSessionStarted == true then
			Amax.EndPlayStatsOnlyMatchingSession()
			UIGlobals.NetworkSessionStarted = false
			print("Network Session Ended")
		end
	end

	SetupInfoLine()
	
	if UIGlobals.FriendDemandSent == true then
		UIButtons.SetActive( SCUI.name_to_id[ "background" ], true )
	end
	
	if UIEnums.CurrentPlatform ~= UIEnums.Platform.PC and UIHardware.WideScreen() == false then	-- [Blur2PC] retail PC has no 4:3 rescale here
		UIButtons.ChangeScale( SCUI.name_to_id[ "centre" ], 1.35, 1.35, 1.35 )
	end
end

function PostInit()
	if UIEnums.CurrentPlatform == UIEnums.Platform.PC then
		-- [Blur2PC] Extend full-screen covers without resizing the result/preview cutout.
		UIButtons.ChangeSizeX( SCUI.name_to_id[ "background" ], SCREEN_WIDTH )
		UIButtons.ChangeSizeX( SCUI.name_to_id[ "LoadingTex" ], SCREEN_WIDTH )
		UIButtons.ChangeSizeX( SCUI.name_to_id[ "LoadingFade" ], SCREEN_WIDTH )
	end

	if IsTable( GUI.result ) ~= true then
		return
	end

	GUI.is_boss_race = Amax.SP_IsBossRace()
	
	--GUI.is_boss_race = false
	
	--GUI.result.state = "silver"
	--GUI.result.secondary = false
	--GUI.result.gfd = false
	
	-- best - needs to be done before updating the save
	if GUI.result.state == "gold" and GUI.is_boss_race == false then
		UIButtons.SetActive( SCUI.name_to_id[ "best_branch" ], true )
		
		local best_text_id = SCUI.name_to_id[ "SpNextMedalInfo" ]
		local beating_best = false
		
		if Amax.SP_IsStreetRace() == true then
			Amax.ChangeText( best_text_id, "HUD_SP_RACE_STREET_DIFF_TO_MEDAL" )
			_, _, beating_best = Amax.SP_StreetRaceState()
			
		elseif Amax.SP_IsCheckpointRace() == true then
			Amax.ChangeText( best_text_id, "HUD_SP_RACE_CHECKPOINT_TIME_DIFF_TO_MEDAL" )
			_, _, _, beating_best = Amax.SP_CheckpointRaceState()
			
		elseif Amax.SP_IsDestructionRace() == true then
			Amax.ChangeText( best_text_id, "HUD_SP_RACE_DESTRUCTION_DIFF_TO_MEDAL" )
			_, _, _, beating_best = Amax.SP_DestructionRaceState()
		
		elseif Amax.SP_IsFanDemandRace() == true then
			Amax.ChangeText( best_text_id, "HUD_SP_RACE_FAN_DEMAND_DIFF_TO_MEDAL" )
			_, _, _, beating_best = Amax.SP_FanDemandRaceState()
		end
		
		if beating_best == false then
			local arrow_id = SCUI.name_to_id[ "SpNextArrow" ]
			UIButtons.ChangeColour( arrow_id, "Support_3" )
			UIButtons.ChangeOrientation( arrow_id, 0, 0, 180 )
		end
		
		GUI.beating_best = beating_best
	end
	
	-- update the game save
	GUI.event_info = SinglePlayer.EventInfo( GUI.event_id )

	if UIGlobals.DoResults == true then 
		UIGlobals.DoResults = false
		
		-- save our previous fan count
		UIGlobals.Sp_PreviousFans = SinglePlayer.GameInfo().fans
		
		SinglePlayer.CompleteEvent( GUI.event_id )
		
		if UIGlobals.FriendDemandSent == false then
			Amax.PostRaceSinglePlayer()
		end
	end
	
	--GUI.event_info.state = "none"
	--GUI.event_info.fanParComplete = false
	--GUI.event_info.fanRunComplete = false
	
	-- event box
	local event_node_id = UIButtons.CloneXtGadgetByName( "SpSCUI", "Event_Node" )
	UIButtons.ChangePanel( event_node_id, UIEnums.Panel._3DAA_0, true )
	UIButtons.SetParent( event_node_id, SCUI.name_to_id[ "event_node_rb" ], UIEnums.Justify.TopCentre )
	UIButtons.ChangeOrientation( event_node_id, 0, 0, 0 )
	SpRaceAwards_SetEventInfo( event_node_id )
	
	-- primary
	local primary_dummy = UIButtons.CloneXtGadgetByName( "SinglePlayer\\Ingame\\SpRaceAwards.lua", "_obj_dummy" )
	UIButtons.SetParent( primary_dummy, SCUI.name_to_id[ "primary_rb" ], UIEnums.Justify.MiddleCentre )
	
	SpRaceAwards_CreatePrimaryLights( UIButtons.FindChildByName( primary_dummy, "_obj_light_dummy" ), GUI.result.state )
	
	UIButtons.ChangeText( UIButtons.FindChildByName( primary_dummy, "_obj_title" ), UIText.SP_EVENT_PRIMARY )
	UIButtons.ChangeText( UIButtons.FindChildByName( primary_dummy, "_obj_info" ), "HUD_SP_RACE_RESULT" )
	
	UIShape.ChangeObjectName( UIButtons.FindChildByName( primary_dummy, "_obj_icon" ), Sp_KindToShape( GUI.event_info.kind ) )
	
	-- fan par
	local fan_par_dummy = UIButtons.CloneXtGadgetByName( "SinglePlayer\\Ingame\\SpRaceAwards.lua", "_obj_dummy" )
	UIButtons.SetParent( fan_par_dummy, SCUI.name_to_id[ "fan_par_rb" ], UIEnums.Justify.MiddleCentre )
	
	SpRaceAwards_CreateSecondaryLights( UIButtons.FindChildByName( fan_par_dummy, "_obj_light_dummy" ), GUI.result.secondary )
	
	UIButtons.ChangeText( UIButtons.FindChildByName( fan_par_dummy, "_obj_title" ), UIText.SP_EVENT_SECONDARY )
	UIButtons.ChangeText( UIButtons.FindChildByName( fan_par_dummy, "_obj_info" ), "SPL_EVENT_" .. GUI.event_id .. "_SECONDARY_GOAL" )
	UIButtons.ChangeText( UIButtons.FindChildByName( fan_par_dummy, "_obj_info_extra" ), "HUD_0_FANS_STRING" )
	
	UIShape.ChangeObjectName( UIButtons.FindChildByName( fan_par_dummy, "_obj_icon" ), "fan" )
	
	-- gfd
	local gfd_dummy = UIButtons.CloneXtGadgetByName( "SinglePlayer\\Ingame\\SpRaceAwards.lua", "_obj_dummy" )
	UIButtons.SetParent( gfd_dummy, SCUI.name_to_id[ "gfd_rb" ], UIEnums.Justify.MiddleCentre )
	
	SpRaceAwards_CreateGfdLights( UIButtons.FindChildByName( gfd_dummy, "_obj_light_dummy" ), GUI.result.gfd )
	
	UIButtons.ChangeText( UIButtons.FindChildByName( gfd_dummy, "_obj_title" ), UIText.SP_EVENT_GFD )
	UIButtons.ChangeText( UIButtons.FindChildByName( gfd_dummy, "_obj_info" ), UIText.SP_EVENT_GFD_INFO )
	
	UIShape.ChangeObjectName( UIButtons.FindChildByName( gfd_dummy, "_obj_icon" ), "fan_attack" )
	
	if GUI.result.state == "none" then
		-- failed
		UIButtons.SetActive( SCUI.name_to_id[ "failed_branch" ], true )
		UIButtons.SetActive( SCUI.name_to_id[ "event_node_rb" ], false )
		
		local city_icon_id = SCUI.name_to_id[ "city_icon" ]
		local px, py, pz = UIButtons.GetPosition( city_icon_id )
		UIButtons.ChangePosition( city_icon_id, px, 0, pz )
		
		SetupInfoLine( UIText.INFO_A_NEXT )
	else
		-- new unlocks
		if #Sp_UnlocksFilter( SinglePlayer.ProcessUnlocks() ) > 0 then
			UIButtons.SetActive( SCUI.name_to_id[ "new_branch" ], true )
			GUI.has_new_unlocks = true
		end
		
		UIButtons.TimeLineActive( "event_info_on", true, 0 )
	end
	
	-- build effect id table
	-- primary
	local num_stars = Sp_EventStateToStars( GUI.result.state, GUI.is_boss_race )
	for i = 1, num_stars, 1 do
		local star_num = num_stars - i + 1
		GUI.light_effect_ids[ #GUI.light_effect_ids + 1 ] = { GUI.light_ids.result.primary[ star_num ][ 1 ], GUI.light_ids.event.primary[ i ][ 1 ], true, false, false, GUI.light_ids.event.primary_blob[ i ] }
	end
	-- fan par
	if GUI.result.secondary == true then
		GUI.light_effect_ids[ #GUI.light_effect_ids + 1 ] = { GUI.light_ids.result.fan_par[ 1 ], GUI.light_ids.event.fan_par[ 1 ], false, true, false, GUI.light_ids.event.fan_par_blob }
	end
	-- fan run
	if GUI.result.gfd == true then
		GUI.light_effect_ids[ #GUI.light_effect_ids + 1 ] = { GUI.light_ids.result.fan_run[ 1 ], GUI.light_ids.event.fan_run[ 1 ], false, false, true, GUI.light_ids.event.fan_run_blob }
	end
	
	-- loading bleep
	GUI.transmitter_id = AddTransmitter( 24, 0, 32, 32, UIEnums.Panel._2DAA )
	UIButtons.SetParent( GUI.transmitter_id, SCUI.name_to_id[ "Txt_Loading" ], UIEnums.Justify.MiddleRight )
	UIButtons.SetActive( SCUI.name_to_id[ "Txt_Loading" ], false, true )

	if GUI.LeaderboardsValid == true then
		GUI.leaderboard_initialised = SpMiniLeaderboard_PostInit( SCUI.name_to_id[ "centre" ] )
	end
	
	local eventId = UIGlobals.Sp.TierInfo[ UIGlobals.Sp.CurrentTier ].events[ UIGlobals.Sp.CurrentEvent ]
	SpRaceAwards_UpdateRival(eventId)
end

function StartLoop( time )
	--
end

function MessageUpdate( message, arg1, arg2, arg3, arg4 )
	if SCUI_MessageUpdate( message, arg1, arg2, arg3, arg4 ) == true then
		return
	end
	
	if message == UIEnums.GameFlowMessage.UILoaded and UIGlobals.LoadFromDebug == true then
		GoScreen( GetStoredScreen( UIEnums.ScreenStorage.FE_RETURN ) )
	elseif message == UIEnums.GameFlowMessage.StartGameRendering then
		GoScreen( GetStoredScreen( UIEnums.ScreenStorage.FE_RETURN ) )
	end
	
	if SubScreenActive() == true or GUIBank.loading == true or GUI.loading_ui == true then
		return
	end
	if message == UIEnums.Message.PopupNext and arg2 == UIEnums.CustomPopups.FailedWriteToLeaderboards then
		UIGlobals.SPPostRaceLeaderboardDone = true	
		GUI.FinishedWriting = true
	elseif message == UIEnums.Message.PopupNext and arg2 == UIEnums.CustomPopups.SpPostRaceOptions then
		if arg3 == 0 then
			PushScreen( "Shared\\GarageMini.lua" )
		elseif arg3 == 1 then
			SpRaceAwards_BackToUiStart()
		elseif arg3 == 2 then
			if Profile.PadProfileOnline( Profile.GetPrimaryPad() ) == false then
				
				if net_CanReconnectToDemonware() == false then
					SetupCustomPopup( UIEnums.CustomPopups.MultiplayerOnlineConnectionLost )
				else
					net_StartServiceConnection(true, nil, false)
				end
				
			elseif LSP.IsConnected() == false then
				SetupCustomPopup( UIEnums.CustomPopups.ContentServerGeneralError )
			elseif FriendDemand.HasPrivilege() == false then
				SetupCustomPopup( UIEnums.CustomPopups.FriendChallengeNoPrivilege )
			elseif Amax.GetNumFriends() == 0 then
				SetupCustomPopup( UIEnums.CustomPopups.FriendChallengeNoFriends )
			else
				--friend challenge
				Fd_ClearExtraChallenge()
				FriendDemand.PopulateChallengeText()
				
				UIGlobals.FriendDemandFriends[1] = nil
				UIGlobals.FriendDemandFriends[2] = nil
				UIGlobals.FriendDemandFriends[3] = nil
				
				StoreScreen( UIEnums.ScreenStorage.FE_FRIEND_DEMAND )
				GoScreen( "Ingame\\AddFriendsToFriendDemand.lua" )
			end
		end
			
	elseif message == UIEnums.Message.PopupNext and arg2 == UIEnums.CustomPopups.SPLeaderboardPostRaceError then
	
		GUI.FinishPostRaceLeaderboardScreen=true
	
	elseif message == UIEnums.Message.MenuNext then
		if GUI.current_stage == GUI.stage.goals and GUI.result.state ~= "none" and GUI.allow_next_and_share == false and GUI.has_taken_photo == true then
			SpRaceAwards_Skip()
		else
			SetupCustomPopup( UIEnums.CustomPopups.SpPostRaceOptions )
		end
		
	elseif message == UIEnums.Message.ButtonLeftShoulder and GUI.allow_next_and_share == true then
		if GUI.result.state ~= "none" then
			if Amax.CanUseShare() == true then
				SetupCustomPopup( UIEnums.CustomPopups.SpPostRaceSharingOptions )
			end
		end

	elseif message == UIEnums.Message.PopupNext and arg2 == UIEnums.CustomPopups.SharingOptions then
		if arg3 == UIEnums.ShareOptions.Facebook then
			Amax.CreateBlurb( UIGlobals.SharingOptionsChosen, 1, GUI.event_id )
			StoreScreen( UIEnums.ScreenStorage.FE_SOCIAL_NETWORK, "SinglePlayer\\Ingame\\SpRaceAwards.lua" )
			GoScreen( "Shared\\Facebook.lua", UIEnums.Context.Blurb )
		
		elseif arg3 == UIEnums.ShareOptions.Twitter then

			Amax.CreateBlurb(UIGlobals.SharingOptionsChosen, 0, GUI.event_id)
			StoreScreen( UIEnums.ScreenStorage.FE_SOCIAL_NETWORK, "SinglePlayer\\Ingame\\SpRaceAwards.lua" )
			GoScreen( "Shared\\Twitter.lua", UIEnums.Context.Blurb )

		elseif arg3 == UIEnums.ShareOptions.Blurb then

			Amax.CreateBlurb(UIGlobals.SharingOptionsChosen, 2, GUI.event_id)

		end			
	end
end

function SpRaceAwards_DoLights()
	if GUI.light_change_active == nil and #GUI.light_effect_ids > 0 then
		GUI.light_change_active = true
	end
end

function FrameUpdate( time )

	if UIGlobals.SPRA_UpdateRival == true then
		local eventId = UIGlobals.Sp.TierInfo[ UIGlobals.Sp.CurrentTier ].events[ UIGlobals.Sp.CurrentEvent ]
		SpRaceAwards_UpdateRival(eventId)
		UIGlobals.SPRA_UpdateRival = false
	end
	
	if UIGlobals.StartPostRaceLeaderboardWrite == true then

		UIGlobals.StartPostRaceLeaderboardWrite = nil
		
		if UIGlobals.SPPostRaceLeaderboardDone == false then
			print("FrameUPdate")
			Amax.StartSPScoreboardWrite()
		end
		
		SpMiniLeaderboard_Init()
		GUI.leaderboard_initialised = SpMiniLeaderboard_PostInit( SCUI.name_to_id[ "centre" ] )
		
		local eventId = UIGlobals.Sp.TierInfo[ UIGlobals.Sp.CurrentTier ].events[ UIGlobals.Sp.CurrentEvent ]
		SpRaceAwards_UpdateRival(eventId)
		
		GUI.LeaderboardsValid = true
	end
	
	if GUI.FinishPostRaceLeaderboardScreen == false then	
		if UIEnums.CurrentPlatform == UIEnums.Platform.Xenon then
			if UIGlobals.SuccessfullyCreatedNetworkSession == false then
				if Profile.PadProfileOnline( Profile.GetPrimaryPad() ) == true then
					return
				end
			end			
		else -- PS3 and PC
			if LSP.IsConnected() == false then
				if net_CanReconnectToDemonware() == true then
					if GUI.LeaderboardsValid == false then 
						return
					end
				end
			end
		end
	end
	
	--end our session before we begin to read the leaderboard
	if  GUI.LeaderboardsValid == true then
	
		if  Amax.SPScooreboardWriteSuccess() == false then
		
			if GUI.WriteFailed ~= true then
				--Write failed
				SetupCustomPopup( UIEnums.CustomPopups.FailedWriteToLeaderboards )	
				GUI.WriteFailed = true	
			end
			
		elseif GUI.WriteFailed == false then
			if Amax.SPScoreboardWriteComplete() == true then

				if GUI.FinishedWriting == false then
					print("write Complete ...")
				end
				
				UIGlobals.SPPostRaceLeaderboardDone = true
				GUI.FinishedWriting = true	
			end
		end
		
		if UIGlobals.LeaderboardReTryPassed ~= nil then
			if UIGlobals.LeaderboardReTryPassed == true then
				UIGlobals.LeaderboardReTryPassed = nil
				GUI.FinishedWriting = true
			end
		end
		
		if GUI.FinishedWriting == true then
			GUI.LeaderboardWriteTimer = GUI.LeaderboardWriteTimer - time
			
			if GUI.LeaderboardWriteTimer <= 0 or GUI.WriteFailed == true then
				if UIGlobals.NetworkSessionStarted == true then
					Amax.EndPlayStatsOnlyMatchingSession()
					UIGlobals.NetworkSessionStarted = false
					print("Network Session Ended")
				end
			end
		end
	end
	
	if GUI.LeaderboardsValid == true then
		-- BUG: 37063	
		if GUI.leaderboard_initialised == true and GUI.FinishedWriting == true then
			SpMiniLeaderboard_FrameUpdate( time, true, false )
		end			
	end

	GUI.timer = GUI.timer + time
	
	if GUI.light_change_active == true then
		if GUI.light_change_timer > GUI.light_change_timer_next then
			GUI.light_change_timer_next = GUI.light_change_timer_next + 0.325
			SpRaceAwards_UpdateLight()
		end
		
		GUI.light_change_timer = GUI.light_change_timer + time
	end

	-- [Bone] #35472 - Commented out because of LUA memory hammering (table create gc)
	--[[	
	if SubScreenActive() == false then	
		if GUI.current_stage == GUI.stage.leaderboard then
			SetupInfoLine( UIText.INFO_A_NEXT, "GAME_SHARE_BUTTON" )
		end
	end
	]]--
end

function EnterEnd()
	--
end

function EndLoop( time )
	--
end

function End()
	if GUI.loading_ui == true then
		SpRaceAwards_BackToUiEnd()
	end
end

function SpRaceAwards_CreatePrimaryLights( parent, state )
	local num_stars = SP_MaxStars( GUI.is_boss_race )
	local num_stars_complete = Sp_EventStateToStars( state, GUI.is_boss_race )
	
	local star_sx = 0
	
	for i = 1, num_stars, 1 do
		local star_id = Sp_CreateStar( ( i <= num_stars_complete ), i, state, nil, nil, 12, true, false )
		
		for j, w in ipairs( star_id ) do
			UIButtons.SetParent( w, parent, UIEnums.Justify.TopRight )
			UIButtons.ChangePanel( w, UIEnums.Panel._3DAA_0, true )
			UIButtons.ChangeJustification( w, UIEnums.Justify.MiddleCentre )
			UIButtons.ChangeScale( w, 12, 12 )
			
			if i == 1 and j == 1 then
				star_sx, _, _ = UIButtons.GetSize( w )
			end
		end
		
		GUI.light_ids.result.primary[ #GUI.light_ids.result.primary + 1 ] = star_id
	end
	
	local offset = ( num_stars - 1 ) * star_sx * 13 * 0.5
	
	local px, py, pz = UIButtons.GetPosition( parent )
	UIButtons.ChangePosition( parent, px - offset, py, pz )
	
	if GUI.is_boss_race == true then
		UIButtons.SetActive( SCUI.name_to_id[ "fan_par_rb" ], false )
		UIButtons.SetActive( SCUI.name_to_id[ "gfd_rb" ], false )
		
		local primary_rb_id = SCUI.name_to_id[ "primary_rb" ]
		local px, py, pz = UIButtons.GetPosition( primary_rb_id )
		UIButtons.ChangePosition( primary_rb_id, 0, py, pz )
	end
end

function SpRaceAwards_CreateSecondaryLights( parent, pass )
	local star_id = Sp_CreateStar( pass, 1, nil, true, nil, nil, true, false )
	
	for i, v in ipairs( star_id ) do
		UIButtons.SetParent( v, parent, UIEnums.Justify.TopRight )
		UIButtons.ChangePanel( v, UIEnums.Panel._3DAA_0, true )
		UIButtons.ChangeJustification( v, UIEnums.Justify.MiddleCentre )
		UIButtons.ChangeScale( v, 12, 12 )
	end
	
	GUI.light_ids.result.fan_par = star_id
end

function SpRaceAwards_CreateGfdLights( parent, pass )
	local star_id = Sp_CreateStar( pass, 1, nil, nil, true, nil, true, false )
	
	for i, v in ipairs( star_id ) do
		UIButtons.SetParent( v, parent, UIEnums.Justify.TopRight )
		UIButtons.ChangePanel( v, UIEnums.Panel._3DAA_0, true )
		UIButtons.ChangeJustification( v, UIEnums.Justify.MiddleCentre )
		UIButtons.ChangeScale( v, 12, 12 )
	end
	
	GUI.light_ids.result.fan_run = star_id
end

function SpRaceAwards_SetEventInfo( node_id )
	UIButtons.SetActive( UIButtons.FindChildByName( node_id, "New" ), false )
	
	UIButtons.ChangeText( UIButtons.FindChildByName( node_id, "EventNumber" ), "GAME_NUM_" .. UIGlobals.Sp.CurrentEvent )
	UIButtons.ChangeText( UIButtons.FindChildByName( node_id, "KindName" ), Sp_KindToText( GUI.event_info.kind ) )
	
	UIShape.ChangeObjectName( UIButtons.FindChildByName( node_id, "KindIcon" ), Sp_KindToShape( GUI.event_info.kind ) )
	
	local star_root_id = UIButtons.FindChildByName( node_id, "Light_Root" )
	
	local num_stars_primary = SP_MaxStars( GUI.is_boss_race )
	
	for i = 1, num_stars_primary, 1 do
		local star_id, star_id_blob = Sp_CreateStar( ( i <= Sp_EventStateToStars( GUI.event_info.state, GUI.is_boss_race ) ), i, GUI.event_info.state, nil, nil, nil, true, true )
		
		for j, w in ipairs( star_id ) do
			UIButtons.ChangePanel( w, UIEnums.Panel._3DAA_0, true )
			UIButtons.SetParent( w, star_root_id, UIEnums.Justify.TopRight )
		end
		
		GUI.light_ids.event.primary[ #GUI.light_ids.event.primary + 1 ] = star_id
		GUI.light_ids.event.primary_blob[ #GUI.light_ids.event.primary_blob + 1 ] = star_id_blob
	end
	
	if GUI.is_boss_race == false then
		-- fan par
		local star_id, star_id_blob = Sp_CreateStar( GUI.event_info.fanParComplete, 6.5, nil, true, nil, nil, true, true )
		
		for j, w in ipairs( star_id ) do
			UIButtons.ChangePanel( w, UIEnums.Panel._3DAA_0, true )
			UIButtons.SetParent( w, star_root_id, UIEnums.Justify.TopRight )
		end
		
		GUI.light_ids.event.fan_par = star_id
		GUI.light_ids.event.fan_par_blob = star_id_blob
		
		-- fan_run
		local star_id, star_id_blob = Sp_CreateStar( GUI.event_info.fanRunComplete, 7.5, nil, nil, true, nil, true, true )
		
		for j, w in ipairs( star_id ) do
			UIButtons.ChangePanel( w, UIEnums.Panel._3DAA_0, true )
			UIButtons.SetParent( w, star_root_id, UIEnums.Justify.TopRight )
		end
		
		GUI.light_ids.event.fan_run = star_id
		GUI.light_ids.event.fan_run_blob = star_id_blob
	end
	
	local amax_event = GameData.GetEvent( GUI.event_info.eventId )
	
	local city_icon_id = SCUI.name_to_id[ "city_icon" ]
	local city_name_id = SCUI.name_to_id[ "city_name" ]
	local route_name_id = SCUI.name_to_id[ "route_name" ]
	
	UIShape.ChangeObjectName( city_icon_id, amax_event.city_icon_name )
	
	UIButtons.ChangeText( city_name_id, amax_event.city_name )
	UIButtons.ChangeText( route_name_id, amax_event.route_name )
	
	local length_city = UIButtons.GetStaticTextLength( city_name_id )
	local length_route = UIButtons.GetStaticTextLength( route_name_id )
	local length_max = math.max( length_city, length_route )
	
	local px, py, pz = UIButtons.GetPosition( city_icon_id )
	UIButtons.ChangePosition( city_icon_id, px - length_max / 2, py, pz )
	
	local box_frame = UIButtons.FindChildByName( node_id, "BoxFrame" )
	UIButtons.ChangeColour( box_frame, "Main_2" )
	
	local eventId = UIGlobals.Sp.TierInfo[ UIGlobals.Sp.CurrentTier ].events[ UIGlobals.Sp.CurrentEvent ]
	
	local eo_state=Amax.EventOwnerState(eventId)
	if eo_state == UIEnums.EventOwnerResult.Losing or eo_state == UIEnums.EventOwnerResult.Winning then
		local owner_node = UIButtons.CloneXtGadgetByName( "SpSCUI", "Owner_Node" )
		
		UIButtons.ChangePanel( owner_node, UIEnums.Panel._3DAA_0, true )
		
		UIButtons.SetParent( owner_node, UIButtons.FindChildByName( node_id, "BoxFrame" ), UIEnums.Justify.BottomLeft )
		Sp_UpdateOwnerState( owner_node, eo_state, eventId )
	end
end

-- called externally from a timeline
function SpRaceAwards_BestOn()
	if GUI.beating_best == nil then
		return
	end
	
	UISystem.PlaySound( Select( ( GUI.beating_best == true ), UIEnums.SoundEffect.PostRaceBest, UIEnums.SoundEffect.PostRaceNotBest ) )
end

-- called externally from a timeline
function SpRaceAwards_FailedOn()
	if GUI.result.state == "none" then
		UISystem.PlaySound( UIEnums.SoundEffect.PostRaceFailed )
	end
end

-- called externally from a timeline
function SpRaceAwards_NewOn()
	if GUI.has_new_unlocks == true then
		UISystem.PlaySound( UIEnums.SoundEffect.PostRaceNewUnlocks )
	end
end

function SpRaceAwards_TakePhoto()

	SpRaceAwards_CheckCanReconnectLeaderboard()
	
	if UIGlobals.FriendDemandSent == true then
		SetupCustomPopup( UIEnums.CustomPopups.SpPostRaceOptions )
	else
		Amax.TakePhoto( 1, false, false )
	end
	
	UIButtons.SetActive( SCUI.name_to_id[ "background" ], true )
	Amax.SendMessage( UIEnums.GameFlowMessage.Pause )
	UISystem.PlaySound( UIEnums.SoundEffect.EndOfRacePause )
	GUI.has_taken_photo = true
end

function SpRaceAwards_BackToUiStart()
	GUI.loading_ui = true
	
	Amax.SendMessage( UIEnums.GameFlowMessage.UnPause )
	UISystem.PlaySound( UIEnums.SoundEffect.EndOfRaceUnPause )	
	Amax.SendMessage( UIEnums.GameFlowMessage.QuitRace )
	
	--[[
	StopIngameMusic()
	Amax.SetLoadStateBackToUI()
	
	UIGlobals.Ingame = {  }
	GameProfile.ClearRaceDifficulty()

	if UIGlobals.LoadFromDebug == false then
		Profile.AllowProfileChanges( true )
		Profile.ActOnProfileChanges( false )
	end
	
	SpRaceAwards_TransmitterOn()
	--]]
	
	UIScreen.SetScreenTimers( 0, 0.3 )
	
	UIButtons.TimeLineActive( "end_alpha", true )
	UIButtons.TimeLineActive( "start_load", true )
	
	SetupInfoLine()
	
	--StartUiLoad()
	StartAsyncSave()
	GoScreen( "Loading\\LoadingUi.lua" )
end

function SpRaceAwards_BackToUiEnd()
	--[[
	UIGlobals.IsIngame = false
	
	Amax.CreateUiResources()
	
	if UIGlobals.LoadFromDebug == false then
		Profile.ForceProfileUpdate()
		Profile.ActOnProfileChanges( true )
	end
	--]]
end

--[[
function SpRaceAwards_TransmitterOn()
	UIButtons.SetActive( ContextTable[ UIEnums.Context.Main ].SCUI.name_to_id[ "Txt_Loading" ], true, true )
	UIButtons.TimeLineActive( "CSB_off_full", true )
	UIButtons.TimeLineActive( "start_load", true )
end
--]]

function SpRaceAwards_CanGoLeaderboard()
	return ( GUI.current_stage == GUI.stage.goals and Profile.PadProfileOnline( Profile.GetPrimaryPad() ) == true and GUI.result.state ~= "none" and GUI.LeaderboardsValid == true )
end

function SpRaceAwards_CheckCanReconnectLeaderboard()

	if UIEnums.CurrentPlatform == UIEnums.Platform.Xenon then
		if UIGlobals.SuccessfullyCreatedNetworkSession ~= true then
			if Profile.PadProfileOnline( Profile.GetPrimaryPad() ) == true then
				GUI.FinishPostRaceLeaderboardScreen = false
				SetupCustomPopup(UIEnums.CustomPopups.SPLeaderboardPostRaceError)
			end
		end			
	else --PS3 and PC
		if LSP.IsConnected() == false then
			if net_CanStartOnlineServiceConnection() == true then
				GUI.FinishPostRaceLeaderboardScreen = false
				
				SetupCustomPopup(UIEnums.CustomPopups.SPLeaderboardPostRaceError)
			end
		end
	end
	
end

-- called externally from a timeline
function SpRaceAwards_TryGoLeaderboard( force_for_offline )
	local can_go = SpRaceAwards_CanGoLeaderboard()
	
	if force_for_offline == true then
		if can_go == false and GUI.current_stage == GUI.stage.goals then
			SetupInfoLine( UIText.INFO_A_NEXT, "GAME_SHARE_BUTTON" )
			GUI.allow_next_and_share = true
		end
	else
		if can_go == true then
			SpRaceAwards_GoLeaderboard()
		
		elseif GUI.current_stage == GUI.stage.goals then
			SetupInfoLine( UIText.INFO_A_NEXT, "GAME_SHARE_BUTTON" )
			GUI.allow_next_and_share = true
		end
	end
end

function SpRaceAwards_GoLeaderboard()
	GUI.current_stage = GUI.stage.leaderboard
	
	UIButtons.TimeLineActive( "goals_off", true )
	UIButtons.TimeLineActive( "leaderboard_on", true )
	
	if GUI.result.state ~= "none" then
		SetupInfoLine( UIText.INFO_A_NEXT, "GAME_SHARE_BUTTON" )
		GUI.allow_next_and_share = true
	else
		SetupInfoLine( UIText.INFO_A_NEXT )
	end
end

function SpRaceAwards_UpdateLight( force_all, force_time )
	if force_time == nil then
		force_time = 0
	end

	local light_start = GUI.light_change_current
	local light_end = Select( force_all, #GUI.light_effect_ids, GUI.light_change_current )
	
	UISystem.PlaySound( UIEnums.SoundEffect.MedalIncrease )
	
	for i = light_start, light_end, 1 do
		UIButtons.PrivateTimeLineActive( GUI.light_effect_ids[ GUI.light_change_current ][ 1 ], "light_off", true, force_time, force_all )
		UIButtons.PrivateTimeLineActive( GUI.light_effect_ids[ GUI.light_change_current ][ 2 ], "light_on",  true, 0 )
		UIButtons.PrivateTimeLineActive( GUI.light_effect_ids[ GUI.light_change_current ][ 6 ], "light_on",  true, 0 )
		
		local higher_state = Select( ( Sp_EventStateToStars( GUI.result.state ) > Sp_EventStateToStars( GUI.event_info.state ) ), GUI.result.state, GUI.event_info.state )
		SpSetStarTexture( GUI.light_effect_ids[ GUI.light_change_current ][ 2 ], true, higher_state, GUI.light_effect_ids[ GUI.light_change_current ][ 4 ], GUI.light_effect_ids[ GUI.light_change_current ][ 5 ] )
		
		local colour = Sp_EventStateToColour( higher_state )
		
		if GUI.light_effect_ids[ GUI.light_change_current ][ 4 ] == true or GUI.light_effect_ids[ GUI.light_change_current ][ 5 ] == true then
			colour = Sp_EventStateToColour( "gold" )
		end
		
		UIButtons.ChangeColour( GUI.light_effect_ids[ GUI.light_change_current ][ 6 ], colour )
		
		GUI.light_change_current = GUI.light_change_current + 1
	end
			
	if GUI.light_change_current > #GUI.light_effect_ids then
		GUI.light_change_active = false
		UIButtons.TimeLineActive( "goals_darken", true, 0 )
	end
end

function SpRaceAwards_Skip()
	if GUI.has_skipped == true then
		return
	end
	
	if GUI.has_taken_photo ~= true then
		SpRaceAwards_TakePhoto()
	end
	
	local max_time = 100	-- super safe
	
	UIButtons.TimeLineActive( "CSB_off", true, max_time, true )
	UIButtons.TimeLineActive( "commands", true, max_time, true )
	UIButtons.TimeLineActive( "primary_on", true, max_time, true )
	UIButtons.TimeLineActive( "fan_par_on", true, max_time, true )
	UIButtons.TimeLineActive( "fan_run_on", true, max_time, true )
	UIButtons.TimeLineActive( "best_info_on", true, max_time, true )
	UIButtons.TimeLineActive( "fail_info_on", true, max_time, true )
	UIButtons.TimeLineActive( "city_info_on", true, max_time, true )
	
	if GUI.result.state ~= "none" then
		UIButtons.TimeLineActive( "event_info_on", true, max_time, true )
	end
	
	SpRaceAwards_UpdateLight( true, max_time )
	
	SpRaceAwards_TryGoLeaderboard( true )
	SpRaceAwards_TryGoLeaderboard()
	
	GUI.has_skipped = true
end

function SpRaceAwards_UpdateRival(eventId)
	
	if Profile.PadProfileOnline( Profile.GetPrimaryPad() ) == false or LSP.IsConnected() == false then
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
		UIButtons.ChangeText( SCUI.name_to_id[ "Owner_Help" ], UIText.CMN_NO_RIVAL_SET_GAME )

		UIButtons.SetActive( SCUI.name_to_id[ "Owner_AvatarFrame" ], false)
		UIButtons.SetActive( SCUI.name_to_id[ "Owner_Gamertag" ], false)
		UIButtons.SetActive( SCUI.name_to_id[ "Owner_Score" ], false)
	
	end
end
