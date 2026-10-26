GUI =
{
	finished = false,
	timeout = 5.75,
	
	friend_demand = false,
}

function Init()
	UIScreen.SetScreenTimers( 1, UIGlobals.screen_time.default_end ) -- [Bone] #36322
	
	net_LockoutFriendsOverlay( true )
	
	AddSCUI_Elements()
	Amax.StartAnimatedCamera( UIEnums.AnimatedCameraSequenceType.MpWinner, false, 0, 0 )
	
	UIGlobals.FriendDemandSent = false
	
	-- nuke the wrecked screen if it's up
	local wrecked_context = UIEnums.Context.SpWrecked
	
	if UIScreen.IsContextActive( wrecked_context ) == true then
		UIScreen.SetScreenTimers( 0, 0, wrecked_context )
		EndScreen( wrecked_context )
	end
end

function PostInit()
	if UIEnums.CurrentPlatform == UIEnums.Platform.PC then
		-- [Blur2PC] Extend full-screen covers without resizing the result/preview cutout.
		UIButtons.ChangeSizeX( SCUI.name_to_id[ "LoadingTex" ], SCREEN_WIDTH )
	end

	local root_id = SCUI.name_to_id[ "FinishedDummy" ]
	local result = Amax.SP_GetLevelResult()
	if IsTable( result ) ~= true then
		return
	end
	
	--result.state = "none"
	--result.state = "bronze"
	--result.state = "silver"
	--result.state = "gold"

	if Amax.SP_IsStreetRaceFD() == true or Amax.SP_IsDestructionFD() == true or Amax.SP_IsCheckpointFD() == true or Amax.SP_IsFanDemandFD() == true or Amax.SP_IsBossBattleFD() == true then
		GUI.friend_demand = true
	end
	
	local place_id = SCUI.name_to_id[ "Pos_Place" ]
	
	UIButtons.ChangeColour( SCUI.name_to_id[ "Pos_Place" ], GetResultColour( result.state ) )
	
	-- don't show top string if failed in checkpoint
	if ( Amax.SP_IsCheckpointRace() == true or Amax.SP_IsCheckpointFD() == true ) and result.state == "none" then
		-- do nothing
	else
		UIButtons.ChangeText( place_id, "HUD_SP_RACE_RESULT" )
	end
	
	if GUI.friend_demand == false then
		local event_id = UIGlobals.Sp.TierInfo[ UIGlobals.Sp.CurrentTier ].events[ UIGlobals.Sp.CurrentEvent ]
		UIButtons.ChangeText( SCUI.name_to_id[ "medal_result_text" ], "SPL_EVENT_" .. event_id .. "_PRIMARY_GOAL_" .. Sp_EventStateToNumber( result.state ) )
	else
		local fd_info = FriendDemand.GetInfo( UIGlobals.FriendDemandAttemptingIndex, false, true )
		UIButtons.ChangeText( SCUI.name_to_id[ "medal_result_text" ], "SPL_EVENT_" .. fd_info.eventid_sp .. "_PRIMARY_GOAL_" .. Sp_EventStateToNumber( result.state ) .. "_" .. fd_info.difficulty )
	end

	local finished_id = SCUI.name_to_id[ "Finished" ]
	
	if GUI.friend_demand == true then
		if FriendDemand.IsComplete() == 0 then
			UIButtons.ChangeText( finished_id, UIText.FDE_FAILED )
		elseif FriendDemand.IsComplete() == 1 then
			UIButtons.ChangeText( finished_id, UIText.FDE_PASSED )
			Amax.CheckStickerProgress( UIEnums.StickerType.BeatFriendDemand )
		elseif FriendDemand.IsComplete() == 2 then
			UIButtons.ChangeText( finished_id, UIText.FDE_KO )
			Amax.CheckStickerProgress( UIEnums.StickerType.BeatFriendDemand )
		end
	else
		UIButtons.ChangeText( finished_id, Select( ( result.state == "none" ), UIText.FDE_FAILED, UIText.FDE_PASSED ) )
	end
	
	-- swirls
	UIButtons.SetActive( SCUI.name_to_id[ "swirl_" .. result.state ], true )
	
	-- primary medals
	SpFinished_CreatePrimaryLights( SCUI.name_to_id[ "light_dummy" ], result.state )
	
	UIButtons.TimeLineActive( "show_result", true )
	UIButtons.TimeLineActive( "show_finished", true, 0 )
	
	-- sounds
	if Amax.SP_IsDestructionRace() == true or Amax.SP_IsDestructionFD() == true then
		UISystem.PlaySound( Select( ( result.state == "none" ), UIEnums.SoundEffect.DestructionFail, UIEnums.SoundEffect.DestructionPass ) )
	elseif Amax.SP_IsBossRace() == true or Amax.SP_IsBossBattleFD() == true then
		UISystem.PlaySound( Select( ( result.state == "none" ), UIEnums.SoundEffect.BossRaceLose, UIEnums.SoundEffect.BossRaceWin ) )
	else
		UISystem.PlaySound( Select( ( result.state == "none" ), UIEnums.SoundEffect.RaceTargetFailed, UIEnums.SoundEffect.RaceTargetPass ) )		
	end
	
	if result.state ~= "none" then
		UISystem.PlaySound( UIEnums.SoundEffect.PostRacePassedLightTrail )
	end
	
	UIGlobals.DoResults = true
end

function SpFinished_CreatePrimaryLights( parent, state )
	local is_boss_race = ( Amax.SP_IsBossRace() == true or Amax.SP_IsBossBattleFD() == true )
	local num_stars = SP_MaxStars( is_boss_race )
	local num_stars_complete = Sp_EventStateToStars( state, is_boss_race )
	
	local star_sx = 0
	
	for i = 1, num_stars, 1 do
		local star_id = Sp_CreateStar( ( i <= num_stars_complete ), i, state, nil, nil, 12 )
		UIButtons.SetParent( star_id, parent, UIEnums.Justify.TopRight )
		
		UIButtons.ChangePanel( star_id, UIEnums.Panel._2DAA, true )
		UIButtons.ChangeJustification( star_id, UIEnums.Justify.MiddleCentre )
		UIButtons.ChangeScale( star_id, 12, 12 )
		
		if i == 1 then
			star_sx, _, _ = UIButtons.GetSize( star_id )
		end
		
		local offset = ( num_stars - 1 ) * star_sx * 13 * 0.5
	
		local px, py, pz = UIButtons.GetPosition( star_id )
		UIButtons.ChangePosition( star_id, px - offset, py, pz )
	end
end

function StartLoop( time )
	--
end

function MessageUpdate( message, arg1, arg2, arg3, arg4 )
	if SCUI_MessageUpdate( message, arg1, arg2, arg3, arg4 ) == true then
		return
	end
	
	if message == UIEnums.Message.MenuNext then
		SpFinished_End()
	end
end

function FrameUpdate( time )
	if GUI.finished == true then
		return
	end
	
	GUI.timeout = GUI.timeout - time
	
	if GUI.timeout <= 0 then
		SpFinished_End()
	end
end

function SpFinished_End()
	if GUI.friend_demand == true then
		GoScreen( "SinglePlayer\\Ingame\\SpFriendDemand.lua" )
	else
		GoScreen( "SinglePlayer\\Ingame\\SpRaceAwards.lua" )
	end
end

function EnterEnd()
	--
end

function EndLoop( time )
	--
end

function End()
	--
end
