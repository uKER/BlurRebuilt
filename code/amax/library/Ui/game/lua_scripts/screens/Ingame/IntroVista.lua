GUI =
{
	finished = false,
	timer = 0.3,
	stage_time = 5.0,
	pack_stage_time = 3.0,
	show_skip = false,
	allow_skip = false,
	
	stage = 0,
}

function Init()
	AddSCUI_Elements( )
	Amax.SetGlobalFade(1)

	UIScreen.SetScreenTimers( 0, 0 )

	if IsSplitScreen() == true then
		Amax.SetNumViewports(1)
	end
	
	Amax.FadeUpIngameAudio()
	Amax.StartAnimatedCamera( UIEnums.AnimatedCameraSequenceType.RouteVista, true )
	
	GUI.allow_skip = (Amax.GetGameMode() == UIEnums.GameMode.SinglePlayer)

	-- [Bone] #34942 Set future camera
	if Amax.IsRaceMpDestruction() == true then
		Amax.StartAnimatedCamera( UIEnums.AnimatedCameraSequenceType.RaceStart4, true, -1, 0, true )
	else
		local seqType, vehicleIndex = IntroVista_SecondSequence()
		Amax.StartAnimatedCamera( seqType, true, vehicleIndex, 0, true )
	end
	
	local t = Amax.GetAnimatedCameraParams( UIEnums.AnimatedCameraSequenceType.RouteVista )
	if IsTable( t ) == true then
		local x = XtToScreenSpaceX( t.px )
		local y = XtToScreenSpaceY( t.py )
		UIButtons.ChangePosition( SCUI.name_to_id[ "Vista" ], x, y )
		UIButtons.ChangePosition( SCUI.name_to_id[ "WriteFadeCutoutStencil0" ], SCREEN_LEFT, y )
		UIButtons.ChangePosition( SCUI.name_to_id[ "TopSetStencil128" ], SCREEN_LEFT, y )
		
		y = y + 2
		UIButtons.ChangeSize( SCUI.name_to_id[ "WriteFadeCutoutStencil0" ], 857, y )
		
		local tex =
		{
			filename = "LOADING_TEXTURE",
			pos = { u = 0, v = y/480 },
			size = { u = 1, v = 1 },
		}
	
		UIButtons.ChangeTexture( tex, 0, SCUI.name_to_id[ "Bottom" ] )
		
		tex.size.v = tex.pos.v
		tex.pos.v = 0
		UIButtons.ChangeTexture( tex, 0, SCUI.name_to_id[ "WriteFadeCutoutStencil0" ] )
		
		-- Change stage time
		if t.time > 0 then
			GUI.stage_time = t.time
		end
		
		GUI.anim_camera = t
	end
	
	-- MP ONLY
	if Amax.IsNetworkRace() == true then
		local race = Multiplayer.GetCurrentRace()
		if IsTable( race ) == true then
			-- UIButtons.ChangeText( SCUI.name_to_id[ "CityText" ],		race.city_name )
			-- UIButtons.ChangeText( SCUI.name_to_id[ "RouteText" ],		race.route_name )
			-- UIShape.ChangeObjectName( SCUI.name_to_id[ "Icon" ],		race.city_icon )
		end
	elseif Amax.GetGameMode() == UIEnums.GameMode.SinglePlayer then
--		if IsTable( UIGlobals.Sp ) == true then
--			local eventId = UIGlobals.Sp.TierInfo[ UIGlobals.Sp.CurrentTier ].events[ UIGlobals.Sp.CurrentEvent ]
--			local event = SinglePlayer.EventInfo( eventId )
--			if IsTable( event ) == true then
--				local amax_event = GameData.GetEvent( event.eventId )
				
				-- UIButtons.ChangeText( SCUI.name_to_id[ "CityText" ],		amax_event.city_name )
				-- UIButtons.ChangeText( SCUI.name_to_id[ "RouteText" ],		amax_event.route_name )
				-- UIShape.ChangeObjectName( SCUI.name_to_id[ "Icon" ],		amax_event.city_icon_name )
--			end
--		end
	end
end

function PostInit()
	if UIEnums.CurrentPlatform == UIEnums.Platform.PC then
		-- [Blur2PC] Extend the authored 16:9 composition horizontally. The two
		-- bottom panes attach to TopSetStencil128, so widening that parent also
		-- keeps their centres aligned with the viewport. Preserve all heights.
		local left = 320 - SCREEN_WIDTH / 2
		for _,name in ipairs( { "TopSetStencil128", "WriteBlackStencil128",
			"Bottom", "Bottom2", "LoadingBG", "WriteFadeCutoutStencil0" } ) do
			UIButtons.ChangeSizeX( SCUI.name_to_id[ name ], SCREEN_WIDTH )
		end
		UIButtons.ChangePosition( SCUI.name_to_id[ "TopSetStencil128" ], left )
		UIButtons.ChangePosition( SCUI.name_to_id[ "WriteFadeCutoutStencil0" ], left )

		-- Hole and FadeCutoutStencil inherit Vista's transform. Stretch their
		-- existing horizontal swipe with the viewport, leaving vertical motion,
		-- the camera and timeline durations untouched.
		local stretch = SCREEN_WIDTH / (480 * 16 / 9)
		local vista = SCUI.name_to_id[ "Vista" ]
		local x = UIButtons.GetPosition( vista )
		local sx,sy,sz = UIButtons.GetScale( vista )
		UIButtons.ChangePosition( vista, 320 + (x - 320) * stretch )
		UIButtons.ChangeScale( vista, sx * stretch, sy, sz )
	end

	if IsTable( GUI.anim_camera ) == true then
		UIButtons.TimeLineActive( GUI.anim_camera.anim, true, true )
	end
end

function StartLoop( time )
	--
	
end
 
function MessageUpdate( message, arg1, arg2, arg3, arg4 )
	if SCUI_MessageUpdate( message, arg1, arg2, arg3, arg4 ) == true then
		return
	end
	
	-- [Feature creep] #36329 - skip *some* of the intro vista in SP
	if message == UIEnums.Message.MenuNext then
		if GUI.allow_skip == true then
			if GUI.stage == 1 and GUI.show_skip == true then
				IntroVista_GoSecondSequence()
				UISystem.PlaySound( UIEnums.SoundEffect.SkipVista )
			end
		end
	end
end

function FrameUpdate( time )
	--
	if GUI.timer > 0 then
		GUI.timer = GUI.timer - time

		-- [Feature creep] #36329 - skip *some* of the intro vista in SP
		if GUI.allow_skip == true then
			if GUI.show_skip == false and GUI.stage == 1 and GUI.timer < (GUI.stage_time-0.7)  then -- 0.7 is *some* time for the streamer to load the pack shot
				GUI.show_skip = true
				SetupInfoLine( UIText.INFO_SKIP_A )
			end
		end
		
		if GUI.timer <= 0 then
			if GUI.stage == 0 then
				Amax.StartAnimatedCamera( 0, false )
				GUI.timer = GUI.stage_time
				GUI.stage = 1
			elseif GUI.stage == 1 then
				IntroVista_GoSecondSequence()
			else
				GoScreen("Ingame\\IntroRace.lua")
			end
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
	--
end

function IntroVista_SecondSequence()
	if Amax.GetGameMode() == UIEnums.GameMode.SinglePlayer then	
		if Amax.SP_IsBossRace() == true or Amax.SP_IsBossBattleFD() == true then
			return UIEnums.AnimatedCameraSequenceType.RivalSeq01, 1
		end	
	end
	return UIEnums.AnimatedCameraSequenceType.RoutePack, -1
end

function IntroVista_GoSecondSequence()
	GUI.show_skip = false
	SetupInfoLine()
	
	-- Start pack shot
	if Amax.IsNetworkRace() == true then
		Amax.SendMessage( UIEnums.GameFlowMessage.BeginRollingStart )
	else
		UIScreen.AddMessage( UIEnums.CutScene.StartScriptedCamera )
	end
	
	UIButtons.TimeLineActive( "route", true )
	
	if Amax.IsRaceMpDestruction() == true then
		GoScreen("Ingame\\IntroRace.lua")
	else
		GUI.timer = GUI.pack_stage_time
		GUI.stage = 2
		
		-- Start pack / rival shot
		local seqType, vehicleIndex = IntroVista_SecondSequence()
		Amax.StartAnimatedCamera( seqType, false, vehicleIndex, 0 )
		-- [Bone] #34942 Set future camera
		Amax.StartAnimatedCamera( UIEnums.AnimatedCameraSequenceType.RaceStart4, true, 0, 0, true )
	end
end
